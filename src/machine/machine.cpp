#include "machine.h"

#include <algorithm>
#include <fstream>
#include <iterator>

#include "../memmap/cpp/rom_guess.h"
#include "../ppi/core/ppi_state.h"
#include "../vdp/core/vdp_render.h"
#include "../vdp/core/vdp_state.h"
#include "../z80/common/z80_state.h"

namespace machine {

namespace {

// Arquivo de SRAM do cartucho: a ROM com a extensao trocada por .sav.
std::string SramPathFor(const std::string &cart_path) {
    const size_t slash = cart_path.find_last_of("/\\");
    const size_t dot = cart_path.find_last_of('.');
    const std::string stem = (dot != std::string::npos && (slash == std::string::npos || dot > slash))
                                 ? cart_path.substr(0, dot)
                                 : cart_path;
    return stem + ".sav";
}

bool ReadFile(const std::string &path, std::vector<uint8_t> &out, std::string &error) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }
    out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

uint32_t PackRgba(uint8_t r, uint8_t g, uint8_t b) {
    // Bytes na ordem R,G,B,A na memoria (little-endian: A<<24 | B<<16 | G<<8 | R).
    return 0xFF000000u | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | r;
}

} // namespace

namespace {

// Nome do mapper, como aparece em cart_info().
const char *MapperName(MemMapMapperType mapper) {
    switch (mapper) {
    case MEMMAP_MAPPER_GEN8: return "Gen8";
    case MEMMAP_MAPPER_GEN16: return "Gen16";
    case MEMMAP_MAPPER_KONAMI5: return "Konami5";
    case MEMMAP_MAPPER_KONAMI4: return "Konami4";
    case MEMMAP_MAPPER_ASCII8: return "ASCII8";
    case MEMMAP_MAPPER_ASCII16: return "ASCII16";
    case MEMMAP_MAPPER_FMPAC: return "FMPAC";
    default: return "ROM plana";
    }
}

// Imagem de uma ROM posta a partir da pagina pedida (0 = 0000h, 1 = 4000h). Um
// cartucho de 16KB com cabecalho "AB" cujo INIT/STATEMENT aponta para 8000h+
// vai para 8000h, como antes.
std::vector<uint8_t> PlainImage(const std::vector<uint8_t> &data, int page) {
    size_t base = page == 0 ? 0 : 0x4000;
    if (page != 0 && data.size() >= 16 && data[0] == 'A' && data[1] == 'B' && data.size() <= 0x4000) {
        unsigned high = 0;
        for (int off : {2, 4, 6, 8}) high = std::max<unsigned>(high, data[off + 1]);
        if (high >= 0x80) base = 0x8000;
    }
    std::vector<uint8_t> image(base, 0);
    image.insert(image.end(), data.begin(), data.end());
    while (image.size() % 0x2000) image.push_back(0);
    return image;
}

// Sub-ROM de 16KB (MSX2EXT) na pagina 0 da celula.
bool ReadSubRom(const std::string &path, std::vector<uint8_t> &out, std::string &error) {
    if (!ReadFile(path, out, error)) {
        error = "sub-ROM do MSX2: " + error;
        return false;
    }
    if (out.size() != 0x4000) {
        error = "sub-ROM do MSX2 '" + path + "': tamanho invalido (" + std::to_string(out.size()) + " bytes, esperava 16384)";
        return false;
    }
    return true;
}

// Carrega uma celula de ROM (BIOS, BASIC, cartucho). `info` recebe a descricao
// do primeiro cartucho (a BIOS nao entra).
bool LoadRomCell(memmap::MemorySystem &mem, int p, int s, const SlotItem &item, std::string &info, std::string &error) {
    const bool is_bios = p == 0 && s == 0;
    const std::string label = is_bios ? "BIOS" : "cartucho";
    std::vector<uint8_t> data;
    if (!ReadFile(item.path, data, error)) {
        error = label + ": " + error;
        return false;
    }
    std::string load_error;

    if (!item.path2.empty()) {
        // BIOS e BASIC em arquivos separados: 16KB na pagina 0 e 16KB na pagina 1.
        std::vector<uint8_t> second;
        std::string read_error;
        if (!ReadFile(item.path2, second, read_error)) {
            error = "BASIC: " + read_error;
            return false;
        }
        if (data.size() != 0x4000 || second.size() != 0x4000) {
            error = "BIOS + BASIC: cada arquivo precisa ter 16KB (tem " + std::to_string(data.size()) + " e " +
                    std::to_string(second.size()) + " bytes)";
            return false;
        }
        data.insert(data.end(), second.begin(), second.end());
        if (!mem.LoadRom(p, s, data.data(), data.size(), &load_error)) {
            error = label + ": " + load_error;
            return false;
        }
        return true;
    }

    MemMapMapperType mapper = item.mapper;
    const bool guessed = mapper == MEMMAP_MAPPER_NONE && data.size() > 0x8000;
    if (guessed) mapper = memmap::GuessMapper(data.data(), data.size());
    if (mapper == MEMMAP_MAPPER_NONE) {
        const std::vector<uint8_t> image = PlainImage(data, item.page);
        if (!mem.LoadRom(p, s, image.data(), image.size(), &load_error)) {
            error = label + ": " + load_error;
            return false;
        }
    } else if (!mem.LoadRom(p, s, data.data(), data.size(), &load_error, mapper)) {
        error = label + ": " + load_error;
        return false;
    }
    if (!is_bios && info.empty()) {
        info = std::to_string(data.size() / 1024) + "KB, " + MapperName(mapper) + (guessed ? " (detectado)" : "");
    }
    return true;
}

} // namespace

SlotLayout DefaultLayout(const MachineConfig &config) {
    SlotLayout layout;
    const bool msx2 = config.model != Model::MSX1;
    const std::string bios_dir = [&] {
        const size_t slash = config.bios_path.find_last_of("/\\");
        return slash == std::string::npos ? std::string() : config.bios_path.substr(0, slash + 1);
    }();

    // BIOS (32KB: BIOS e BASIC) em 0:0, pagina 0.
    layout.cell[0][0].kind = SlotKind::Rom;
    layout.cell[0][0].path = config.bios_path;
    layout.cell[0][0].page = 0;

    // RAM: 64KB comuns em 3:2 no MSX1; mapper de 128KB em 3:2 no MSX2.
    if (msx2) {
        layout.cell[3][2].kind = SlotKind::Mapper;
        layout.cell[3][2].size_kb = 128;
    } else {
        layout.cell[3][2].kind = SlotKind::Ram;
        layout.cell[3][2].size_kb = 64;
    }

    // Sub-ROM do MSX2 e DISK.ROM. Com disco, a celula 3:1 tem o DISK.ROM na
    // pagina 1 e a sub-ROM do MSX2 na pagina 0.
    const std::string ext_path = config.ext_rom_path.empty()
                                     ? bios_dir + (config.model == Model::MSX2P ? "MSX2PEXT.ROM" : "MSX2EXT.ROM")
                                     : config.ext_rom_path;
    const bool want_disk = config.disk_interface || !config.disk_a.empty() || !config.disk_b.empty();
    if (want_disk) {
        layout.cell[3][1].kind = SlotKind::Disk;
        layout.cell[3][1].path = config.disk_rom_path.empty() ? bios_dir + "DISK.ROM" : config.disk_rom_path;
        if (msx2) layout.cell[3][1].path2 = ext_path;
    } else if (msx2) {
        layout.cell[3][1].kind = SlotKind::SubRom;
        layout.cell[3][1].path = ext_path;
    }

    if (!config.cart_path.empty()) {
        layout.cell[1][0].kind = SlotKind::Rom;
        layout.cell[1][0].path = config.cart_path;
        layout.cell[1][0].page = 1;
        layout.cell[1][0].mapper = config.cart_mapper;
    }
    if (!config.fmpac_rom_path.empty()) {
        layout.cell[2][0].kind = SlotKind::FmPac;
        layout.cell[2][0].path = config.fmpac_rom_path;
    }
    return layout;
}

SlotLayout EffectiveLayout(const MachineConfig &config) {
    return config.layout_set ? config.layout : DefaultLayout(config);
}

bool ValidateLayout(const SlotLayout &layout, std::string &error) {
    const SlotItem &bios = layout.cell[0][0];
    if (bios.kind != SlotKind::Rom || bios.path.empty()) {
        error = "BIOS: falta a BIOS no slot 0:0 (o Z80 comeca la')";
        return false;
    }
    int mappers = 0, disks = 0, fmpacs = 0;
    for (int p = 0; p < 4; ++p) {
        for (int s = 0; s < 4; ++s) {
            const SlotItem &item = layout.cell[p][s];
            const std::string where = "slot " + std::to_string(p) + ":" + std::to_string(s) + ": ";
            switch (item.kind) {
            case SlotKind::Rom:
                if (item.page != 0 && item.page != 1) {
                    error = where + "pagina invalida (use 0 ou 1)";
                    return false;
                }
                break;
            case SlotKind::Ram:
                if (item.size_kb != 16 && item.size_kb != 32 && item.size_kb != 64) {
                    error = where + "RAM de 16, 32 ou 64 KB";
                    return false;
                }
                break;
            case SlotKind::Mapper:
                ++mappers;
                if (item.size_kb != 64 && item.size_kb != 128 && item.size_kb != 256 && item.size_kb != 512 &&
                    item.size_kb != 1024) {
                    error = where + "mapper de 64, 128, 256, 512 ou 1024 KB";
                    return false;
                }
                break;
            case SlotKind::Disk: ++disks; break;
            case SlotKind::FmPac: ++fmpacs; break;
            default: break;
            }
        }
    }
    if (mappers > 1) {
        error = "so' uma RAM mapeada (mapper) por maquina (portas FCh-FFh)";
        return false;
    }
    if (disks > 1) {
        error = "so' uma interface de disquete por maquina";
        return false;
    }
    if (fmpacs > 1) {
        error = "so' um FM-PAC por maquina";
        return false;
    }
    return true;
}

void SetCartridge(MachineConfig &config, const std::string &path) {
    SlotLayout layout = EffectiveLayout(config);
    layout.cell[1][0] = SlotItem{};
    if (!path.empty()) {
        layout.cell[1][0].kind = SlotKind::Rom;
        layout.cell[1][0].path = path;
        layout.cell[1][0].page = 1;
    }
    config.layout = layout;
    config.layout_set = true;
}

void SetFmPac(MachineConfig &config, const std::string &path) {
    SlotLayout layout = EffectiveLayout(config);
    layout.cell[2][0] = SlotItem{};
    if (!path.empty()) {
        layout.cell[2][0].kind = SlotKind::FmPac;
        layout.cell[2][0].path = path;
    }
    config.layout = layout;
    config.layout_set = true;
}

std::string CartridgePath(const MachineConfig &config) {
    const SlotItem &item = EffectiveLayout(config).cell[1][0];
    return item.kind == SlotKind::Rom ? item.path : std::string();
}

std::string FmPacPath(const MachineConfig &config) {
    const SlotItem &item = EffectiveLayout(config).cell[2][0];
    return item.kind == SlotKind::FmPac ? item.path : std::string();
}

std::unique_ptr<Machine> Machine::Create(const MachineConfig &config, std::string &error) {
    if (!config.layout_set && config.bios_path.empty()) {
        error = "nenhuma BIOS informada";
        return nullptr;
    }
    const SlotLayout layout = EffectiveLayout(config);
    if (!ValidateLayout(layout, error)) return nullptr;

    std::unique_ptr<Machine> m(new Machine());
    // Sem BIOS na sessao: o layout de slots monta tudo (BIOS, RAM, cartuchos...).
    m->startup_ = z80::debug::BuildZ80DebugShellStartup({"--slots", "--vdp", "--ppi", "--psg"});
    memmap::MemorySystem &mem = *m->startup_.memory_system;
    // A RAM padrao de 0:0 do startup sai: 0:0 e' da BIOS.
    mem.ClearSlot(0, 0);

    m->model_ = config.model;
    m->disk_read_only_ = config.disk_read_only;
    const bool msx2 = config.model != Model::MSX1;
    if (msx2) {
        // VDP V9938/V9958 e regras de subslot do MSX2 (qualquer slot pode ser expandido).
        m->startup_.vdp_device->SetModel(config.model == Model::MSX2P ? VDP_MODEL_MSX2P : VDP_MODEL_MSX2);
        m->rtc_ = std::make_unique<rtc::RtcDevice>();
        m->startup_.composite_bus->RegisterPortRange(0xB4, 0xB5, m->rtc_.get());
    }
    // MSX1 so' expande o slot 3 (regra de hardware); um subslot fora dele pede a regra do MSX2.
    bool expansion = msx2;
    for (int p = 0; p < 4; ++p) {
        for (int s = 1; s < 4; ++s) {
            if (p != 3 && layout.cell[p][s].kind != SlotKind::Empty) expansion = true;
        }
    }
    mem.state().msx1_subslot_rules = expansion ? 2 : 1;

    std::string cart_info;
    for (int p = 0; p < 4; ++p) {
        for (int s = 0; s < 4; ++s) {
            const SlotItem &item = layout.cell[p][s];
            switch (item.kind) {
            case SlotKind::Empty:
                break;
            case SlotKind::Rom:
            case SlotKind::FmPac: {
                if (item.kind == SlotKind::FmPac) {
                    std::vector<uint8_t> data;
                    if (!ReadFile(item.path, data, error)) {
                        error = "FM-PAC: " + error;
                        return nullptr;
                    }
                    std::string load_error;
                    if (!mem.LoadRom(p, s, data.data(), data.size(), &load_error, MEMMAP_MAPPER_FMPAC)) {
                        error = "FM-PAC: " + load_error;
                        return nullptr;
                    }
                } else if (!LoadRomCell(mem, p, s, item, cart_info, error)) {
                    return nullptr;
                }
                if (mem.HasSram(p, s)) {
                    SramTarget target{p, s, SramPathFor(item.path)};
                    std::vector<uint8_t> save;
                    std::string ignored;
                    if (ReadFile(target.path, save, ignored) && save.size() == mem.SramFileSize(p, s)) {
                        mem.LoadSram(p, s, save.data(), save.size());
                    }
                    m->sram_targets_.push_back(target);
                }
                break;
            }
            case SlotKind::SubRom: {
                std::vector<uint8_t> ext;
                if (!ReadSubRom(item.path, ext, error)) return nullptr;
                std::string load_error;
                if (!mem.LoadRom(p, s, ext.data(), ext.size(), &load_error)) {
                    error = "sub-ROM do MSX2: " + load_error;
                    return nullptr;
                }
                break;
            }
            case SlotKind::Ram:
                mem.AllocateRam(p, s, static_cast<size_t>(item.size_kb) * 1024);
                break;
            case SlotKind::Mapper:
                mem.AllocateMapperRam(p, s, item.size_kb / 16);
                m->mapper_ = std::make_unique<memmap::RamMapperDevice>(mem, p, s);
                m->startup_.composite_bus->RegisterPortRange(0xFC, 0xFF, m->mapper_.get());
                break;
            case SlotKind::Disk: {
                // DISK.ROM na pagina 1 (4000h-7FFFh); a sub-ROM do MSX2, se houver, na pagina 0.
                std::vector<uint8_t> rom;
                if (!ReadFile(item.path, rom, error)) {
                    error = "DISK.ROM: " + error;
                    return nullptr;
                }
                if (rom.empty() || rom.size() > 0x4000) {
                    error = "DISK.ROM: tamanho invalido (" + std::to_string(rom.size()) + " bytes)";
                    return nullptr;
                }
                std::vector<uint8_t> image(0x4000, 0);
                if (!item.path2.empty()) {
                    std::vector<uint8_t> ext;
                    if (!ReadSubRom(item.path2, ext, error)) return nullptr;
                    image = ext;
                }
                rom.resize(0x4000, 0);
                image.insert(image.end(), rom.begin(), rom.end());
                std::string load_error;
                if (!mem.LoadRom(p, s, image.data(), image.size(), &load_error)) {
                    error = "slot " + std::to_string(p) + ":" + std::to_string(s) + " (DISK.ROM): " + load_error;
                    return nullptr;
                }
                m->fdc_ = std::make_unique<fdc::FdcDevice>();
                m->startup_.slot_bus->AttachMmio(p, s, m->fdc_.get());
                break;
            }
            }
        }
    }
    m->cart_info_ = cart_info;

    // Discos montados: precisam da interface de disquete (celula Disk) no layout.
    const bool has_disks = !config.disk_a.empty() || !config.disk_b.empty();
    if (has_disks && !m->fdc_) {
        error = "discos pedidos, mas o layout nao tem interface de disquete (celula Disco)";
        return nullptr;
    }
    if (m->fdc_) {
        for (int d = 0; d < 2; ++d) fdc_attach(&m->fdc_->fdc(), d, m->disks_[d].disk());
        const std::string *paths[2] = {&config.disk_a, &config.disk_b};
        for (int d = 0; d < 2; ++d) {
            if (paths[d]->empty()) continue;
            if (!m->InsertDisk(d, *paths[d], error)) return nullptr;
        }
    }

    // SCC: o chip liga-se ao primeiro cartucho do layout (ou ao slot 1:0).
    int scc_p = 1, scc_s = 0;
    bool found = false;
    for (int p = 0; p < 4 && !found; ++p) {
        for (int s = 0; s < 4 && !found; ++s) {
            if (layout.cell[p][s].kind == SlotKind::Rom && !(p == 0 && s == 0)) {
                scc_p = p;
                scc_s = s;
                found = true;
            }
        }
    }
    m->scc_ = std::make_unique<scc::SccDevice>();
    m->startup_.slot_bus->AttachCart(scc_p, scc_s, m->scc_.get());

    // Chip FM (MSX-MUSIC): portas 7Ch/7Dh, sempre presentes como no fMSX.
    m->fm_ = std::make_unique<fm::FmDevice>();
    m->startup_.composite_bus->RegisterPortRange(0x7C, 0x7D, m->fm_.get());

    m->cpu_ = std::make_unique<z80::Z80Cpu>(m->startup_.Bus());
    return m;
}

bool Machine::SaveSram(std::string &error) {
    memmap::MemorySystem &mem = *startup_.memory_system;
    for (const SramTarget &t : sram_targets_) {
        if (!mem.HasSram(t.primary, t.secondary) || !mem.SramDirty(t.primary, t.secondary) || t.path.empty()) continue;
        const std::vector<uint8_t> image = mem.SramImage(t.primary, t.secondary);
        std::ofstream f(t.path, std::ios::binary | std::ios::trunc);
        if (!f || !f.write(reinterpret_cast<const char *>(image.data()), static_cast<std::streamsize>(image.size()))) {
            error = "nao foi possivel gravar '" + t.path + "'";
            return false;
        }
        mem.ClearSramDirty(t.primary, t.secondary);
    }
    return true;
}

bool Machine::sram_dirty() const {
    const memmap::MemorySystem &mem = *startup_.memory_system;
    for (const SramTarget &t : sram_targets_) {
        if (mem.HasSram(t.primary, t.secondary) && mem.SramDirty(t.primary, t.secondary)) return true;
    }
    return false;
}

void Machine::RunFrame() {
    int budget = kFrameCycles;
    while (budget > 0) {
        const int leftover = cpu_->run(1);
        const int used = 1 - leftover;
        budget -= used;

        // Mesmo escalonamento do VDP da sessao de depuracao (DriveVdp): avanca
        // um meio-scanline sempre que o periodo pendente se esgota e entrega a
        // interrupcao quando o VDP a sinaliza.
        vdp_pending_cycles_ -= used;
        while (vdp_pending_cycles_ <= 0) {
            const VdpStepResult r = startup_.vdp_device->Step();
            vdp_pending_cycles_ += r.next_period_cycles;
            if (r.irq_pending) cpu_->interrupt(Z80_INT_IRQ);
        }
        startup_.psg_device->Advance(used);
        scc_->Advance(used);
        fm_->Advance(used);
    }
    ++frame_count_;
}

void Machine::EnableLiveAudio(bool on) {
    startup_.psg_device->EnableLive(on);
    scc_->EnableLive(on);
    fm_->EnableLive(on);
}

void Machine::TakeLiveAudio(std::vector<int16_t> &out) {
    std::vector<int16_t> psg_samples, scc_samples, fm_samples;
    startup_.psg_device->TakeLive(psg_samples);
    scc_->TakeLive(scc_samples);
    fm_->TakeLive(fm_samples);

    // O FM soma ate' 9 canais: entra na metade para deixar folga ao PSG e ao SCC.
    const size_t n = std::max({psg_samples.size(), scc_samples.size(), fm_samples.size()});
    out.reserve(out.size() + n);
    for (size_t i = 0; i < n; ++i) {
        const int psg = i < psg_samples.size() ? psg_samples[i] : 0;
        const int scc = i < scc_samples.size() ? scc_samples[i] : 0;
        const int fm = i < fm_samples.size() ? fm_samples[i] / 2 : 0;
        out.push_back(static_cast<int16_t>(std::clamp(psg + scc + fm, -32768, 32767)));
    }
}

bool Machine::InsertDisk(int drive, const std::string &path, std::string &error) {
    if (!fdc_) {
        error = "esta maquina nao tem interface de disquete (use --disk ou --disk-interface)";
        return false;
    }
    drive &= 1;
    std::string load_error;
    if (!disks_[drive].Load(path, load_error, disk_read_only_)) {
        error = "disco " + std::string(1, static_cast<char>('A' + drive)) + ": " + load_error;
        return false;
    }
    fdc_attach(&fdc_->fdc(), drive, disks_[drive].disk());
    return true;
}

void Machine::EjectDisk(int drive) {
    drive &= 1;
    disks_[drive].Eject();
    if (fdc_) fdc_attach(&fdc_->fdc(), drive, disks_[drive].disk());
}

void Machine::Reset() {
    if (fdc_) fdc_reset(&fdc_->fdc());
    if (mapper_) mapper_->Reset();
    cpu_->reset();
    startup_.vdp_device->Reset();
    startup_.ppi_device->Reset();
    startup_.psg_device->Reset();
    scc_->Reset();
    fm_->Reset();
    vdp_pending_cycles_ = 0;
}

// Teclas MSX (com SHIFT quando preciso, layout internacional) para um caractere
// do --keys. Devolve false se o caractere nao tem tecla.
bool Machine::KeysForChar(char c, std::string &key, bool &shift) {
    shift = false;
    if (c >= 'a' && c <= 'z') { key = std::string(1, c); return true; }
    if (c >= 'A' && c <= 'Z') { key = std::string(1, static_cast<char>(c - 'A' + 'a')); shift = true; return true; }
    if (c >= '0' && c <= '9') { key = std::string(1, c); return true; }
    switch (c) {
    case ' ': key = "space"; return true;
    case '|': key = "enter"; return true; // ENTER (o '|' real nao e' digitavel aqui)
    case '.': case ',': case '/': case '-': case '=': case ';': case '\'': case '[': case ']': case '`':
        key = std::string(1, c);
        return true;
    // simbolos com SHIFT (MSX internacional)
    case '!': key = "1"; shift = true; return true;
    case '@': key = "2"; shift = true; return true;
    case '#': key = "3"; shift = true; return true;
    case '$': key = "4"; shift = true; return true;
    case '%': key = "5"; shift = true; return true;
    case '^': key = "6"; shift = true; return true;
    case '&': key = "6"; shift = true; return true;
    // Layout MSX: SHIFT+8 = '*', SHIFT+9 = '(', SHIFT+0 = ')', SHIFT+7 = aspa simples.
    case '*': key = "8"; shift = true; return true;
    case '(': key = "9"; shift = true; return true;
    case ')': key = "0"; shift = true; return true;
    case '_': key = "-"; shift = true; return true;
    case '+': key = "="; shift = true; return true;
    case ':': key = ";"; shift = true; return true;
    case '"': key = "'"; shift = true; return true;
    case '<': key = ","; shift = true; return true;
    case '>': key = "."; shift = true; return true;
    case '?': key = "/"; shift = true; return true;
    default: return false;
    }
}

bool Machine::KeyDown(const std::string &name) {
    const int id = ppi_key_lookup(name.c_str());
    if (id == PPI_KEY_NONE) return false;
    ppi_key_set(&startup_.ppi_device->state(), id, 1);
    return true;
}

bool Machine::KeyUp(const std::string &name) {
    const int id = ppi_key_lookup(name.c_str());
    if (id == PPI_KEY_NONE) return false;
    ppi_key_set(&startup_.ppi_device->state(), id, 0);
    return true;
}

void Machine::SetJoystick(int port, uint8_t bits) { psg_set_joystick(&startup_.psg_device->state(), port, bits); }

void Machine::ReleaseAllKeys() { ppi_key_release_all(&startup_.ppi_device->state()); }

FrameSize Machine::RenderFrame(std::vector<uint32_t> &rgba) const {
    const VdpState &v = startup_.vdp_device->state();
    const VdpRgb888 border = vdp_render_border_color(&v);
    const uint32_t border_px = PackRgba(border.r, border.g, border.b);
    const int width = vdp_render_width(&v);
    const int lines = vdp_render_height(&v);
    // Borda vertical como RefreshBorder() do fMSX: FirstLine = 18 (192 linhas) ou 8 (212).
    const int top = lines == 212 ? 8 : 18;
    std::vector<VdpRgb888> row(VDP_RENDER_MAX_WIDTH);

    FrameSize size;
    size.height = kFrameHeight;
    if (model_ == Model::MSX1) {
        // MSX1: 256 pixels de tela + 8 de borda de cada lado; telas mais estreitas
        // (TEXT 40) ficam centralizadas dentro da area de 256.
        const int side = (kFrameWidth - 256) / 2;
        const int pad = side + (256 - width) / 2;
        size.width = kFrameWidth;
        rgba.assign(static_cast<size_t>(size.width) * size.height, border_px);
        for (int y = 0; y < lines; ++y) {
            vdp_render_line(&v, y, row.data());
            uint32_t *dst = rgba.data() + static_cast<size_t>(top + y) * size.width + pad;
            for (int x = 0; x < width; ++x) dst[x] = PackRgba(row[x].r, row[x].g, row[x].b);
        }
        return size;
    }

    // MSX2: 512 pixels de tela, com 16 de borda de cada lado (8 pixels de 256 dobrados).
    // Modos de 256 (e o texto de 40 colunas, 240) saem dobrados na horizontal; os de
    // 512 (SCREEN 6/7, TEXT80 de 480) saem como sao. Linhas dobradas na exibicao (y_scale).
    const int side = 16;
    size.width = 512 + 2 * side;
    size.y_scale = 2;
    rgba.assign(static_cast<size_t>(size.width) * size.height, border_px);
    const int factor = width <= 256 ? 2 : 1;
    const int out_width = width * factor;
    const int pad = side + (512 - out_width) / 2;
    for (int y = 0; y < lines; ++y) {
        vdp_render_line(&v, y, row.data());
        uint32_t *dst = rgba.data() + static_cast<size_t>(top + y) * size.width + pad;
        for (int x = 0; x < width; ++x) {
            const uint32_t px = PackRgba(row[x].r, row[x].g, row[x].b);
            if (factor == 2) {
                dst[x * 2] = px;
                dst[x * 2 + 1] = px;
            } else {
                dst[x] = px;
            }
        }
    }
    return size;
}

} // namespace machine
