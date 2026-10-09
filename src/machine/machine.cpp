#include "machine.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iterator>

#include "../memmap/cpp/rom_guess.h"
#include "../ppi/core/ppi_state.h"
#include "../vdp/core/vdp_render.h"
#include "../vdp/core/vdp_state.h"
#include "../z80/common/z80_state.h"

extern "C" void rom_crc32(const uint8_t *data, int32_t length, uint32_t *crc_out);

namespace machine {

namespace {

// CRC32 do conteudo de um arquivo (0 se vazio/ilegivel) -- usa o mesmo rom_crc32 (Assembly)
// do mapa de memoria.
uint32_t FileCrc32(const std::string &path) {
    if (path.empty()) return 0;
    std::ifstream f(path, std::ios::binary);
    if (!f) return 0;
    const std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    uint32_t crc = 0;
    rom_crc32(data.data(), static_cast<int32_t>(data.size()), &crc);
    return crc;
}

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
    case MEMMAP_MAPPER_MSXDOS2: return "MSXDOS2";
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
                if (item.size_kb == 32) {
                    const int next = p * 4 + s + 1;
                    if (next >= 16) {
                        error = where + "RAM de 32KB precisa da celula seguinte, que nao existe apos 3:3";
                        return false;
                    }
                    if (layout.cell[next / 4][next % 4].kind != SlotKind::Empty) {
                        error = where + "RAM de 32KB ocupa a celula seguinte (" + std::to_string(next / 4) + ":" +
                                std::to_string(next % 4) + "), que precisa estar vazia";
                        return false;
                    }
                }
                break;
            case SlotKind::Mapper:
                ++mappers;
                if (item.size_kb < 64 || item.size_kb > 4096 || (item.size_kb & (item.size_kb - 1)) != 0) {
                    error = where + "mapper de 64 KB a 4096 KB (potencia de 2)";
                    return false;
                }
                break;
            case SlotKind::Disk: ++disks; break;
            case SlotKind::FmPac: ++fmpacs; break;
            default: break;
            }
        }
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

void SetCartridge(MachineConfig &config, const std::string &path, MemMapMapperType mapper) {
    SlotLayout layout = EffectiveLayout(config);
    layout.cell[1][0] = SlotItem{};
    if (!path.empty()) {
        layout.cell[1][0].kind = SlotKind::Rom;
        layout.cell[1][0].path = path;
        layout.cell[1][0].page = 1;
        layout.cell[1][0].mapper = mapper;
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

MemMapMapperType CartridgeMapper(const MachineConfig &config) {
    const SlotItem &item = EffectiveLayout(config).cell[1][0];
    return item.kind == SlotKind::Rom ? item.mapper : MEMMAP_MAPPER_NONE;
}

std::string FmPacPath(const MachineConfig &config) {
    const SlotItem &item = EffectiveLayout(config).cell[2][0];
    return item.kind == SlotKind::FmPac ? item.path : std::string();
}

namespace {

// Porta FCh-FFh de todos os mappers: escrita vai a todos; leitura vem do primeiro.
class MapperPorts : public z80::IBus {
public:
    explicit MapperPorts(std::vector<memmap::RamMapperDevice *> devices) : devices_(std::move(devices)) {}
    uint8_t read(uint16_t) override { return 0xFF; }
    void write(uint16_t, uint8_t) override {}
    uint8_t in(uint16_t port) override { return devices_.empty() ? 0xFF : devices_.front()->in(port); }
    void out(uint16_t port, uint8_t value) override {
        for (memmap::RamMapperDevice *d : devices_) d->out(port, value);
    }

private:
    std::vector<memmap::RamMapperDevice *> devices_;
};

} // namespace

// Portas de E/S ja' ocupadas pelo sistema (VDP, PSG, PPI, FM, RTC, mapper). Usado para
// recusar uma base de controladora de disco que colide com elas.
bool PortRangeUsed(int first, int last) {
    static const int kUsed[][2] = {{0x7C, 0x7D}, {0x98, 0x9B}, {0xA0, 0xA2}, {0xA8, 0xAB}, {0xB4, 0xB5}, {0xFC, 0xFF}};
    for (const auto &r : kUsed) {
        if (first <= r[1] && last >= r[0]) return true;
    }
    return false;
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
    m->disk_format_ = config.disk_format;
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
                if (item.size_kb == 32) {
                    // 32KB = 16KB na pagina 2 desta celula + 16KB na pagina 3 da celula seguinte.
                    const int next = p * 4 + s + 1;
                    mem.AllocateRamChunks(p, s, 4, 2);
                    mem.AllocateRamChunks(next / 4, next % 4, 6, 2);
                } else {
                    mem.AllocateRamTop(p, s, static_cast<size_t>(item.size_kb) * 1024);
                }
                break;
            case SlotKind::Mapper:
                mem.AllocateMapperRam(p, s, item.size_kb / 16);
                m->mappers_.push_back(std::make_unique<memmap::RamMapperDevice>(mem, p, s));
                break;
            case SlotKind::Disk: {
                // DISK.ROM (ou o driver de porta, DDX 3.0 / CDX-2) na pagina 1 (4000h-7FFFh); a sub-ROM do MSX2, se houver, na pagina 0.
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
                // Pela memoria: o WD2793 aparece em 7FF8h-7FFFh desta celula. Pelas portas: so' a ROM
                // do driver fica aqui; os registradores estao nas portas (ver abaixo).
                if (config.disk_access == DiskAccess::Memory) {
                    m->fdc_ = std::make_unique<fdc::FdcDevice>();
                    m->fdc_engine_ = &m->fdc_->fdc();
                    m->startup_.slot_bus->AttachMmio(p, s, m->fdc_.get());
                }
                break;
            }
            }
        }
    }
    m->cart_info_ = cart_info;
    m->bios_crc_ = FileCrc32(config.bios_path);
    m->cart_crc_ = FileCrc32(CartridgePath(config));

    // Portas FCh-FFh: todos os mappers do layout.
    if (!m->mappers_.empty()) {
        std::vector<memmap::RamMapperDevice *> devices;
        for (auto &d : m->mappers_) devices.push_back(d.get());
        m->mapper_ports_ = std::make_unique<MapperPorts>(devices);
        m->startup_.composite_bus->RegisterPortRange(0xFC, 0xFF, m->mapper_ports_.get());
    }

    // Controladora por portas (sem DISK.ROM): pedida pela configuracao, com celula Disco no layout
    // ou com discos pedidos. Os registradores vao para disk_port..disk_port+4.
    const bool has_disks = !config.disk_a.empty() || !config.disk_b.empty();
    bool has_disk_cell = false;
    for (int p = 0; p < 4; ++p) {
        for (int s = 0; s < 4; ++s) {
            if (layout.cell[p][s].kind == SlotKind::Disk) has_disk_cell = true;
        }
    }
    if (config.disk_access == DiskAccess::Port && (has_disk_cell || has_disks)) {
        if (PortRangeUsed(config.disk_port, config.disk_port + fdc::PortFdcDevice::kPorts - 1)) {
            error = "controladora de disco: portas " + std::to_string(config.disk_port) + ".." +
                    std::to_string(config.disk_port + fdc::PortFdcDevice::kPorts - 1) +
                    " ja' usadas por outro dispositivo (escolha outra base)";
            return nullptr;
        }
        m->port_fdc_ = std::make_unique<fdc::PortFdcDevice>(static_cast<uint8_t>(config.disk_port));
        m->fdc_engine_ = &m->port_fdc_->fdc();
        m->startup_.composite_bus->RegisterPortRange(config.disk_port,
                                                     config.disk_port + fdc::PortFdcDevice::kPorts - 1,
                                                     m->port_fdc_.get());
    }

    // Discos montados: precisam da interface de disquete (celula Disk ou acesso por portas).
    if (has_disks && !m->fdc_engine_) {
        error = "discos pedidos, mas o layout nao tem interface de disquete (celula Disco)";
        return nullptr;
    }
    if (m->fdc_engine_) {
        for (int d = 0; d < 2; ++d) fdc_attach(m->fdc_engine_, d, m->disks_[d].disk());
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

    // Fita (ver doc/tape-spec.md): o gancho de BIOS (modo rapido) sempre
    // existe no barramento; so' faz algo quando ha' fita inserida.
    m->tape_ = std::make_unique<tape::TapeEngine>(mem);
    m->startup_.slot_bus->AttachTapeHook(m->tape_.get());
    m->tape_->SetMode(config.tape_mode);
    if (!config.tape_path.empty() && !m->InsertTape(config.tape_path, error)) return nullptr;

    m->cpu_ = std::make_unique<z80::Z80Cpu>(m->startup_.Bus());
    return m;
}

bool Machine::InsertTape(const std::string &path, std::string &error) { return tape_->Insert(path, error); }
bool Machine::NewBlankTape(const std::string &path, std::string &error) { return tape_->NewBlank(path, error); }
void Machine::EjectTape() { tape_->Eject(); }
void Machine::RewindTape() { tape_->Rewind(); }
void Machine::SetTapeMode(tape::TapeMode mode) { tape_->SetMode(mode); }

namespace {

// Formato de save-state do fwMSX (ver doc/savestate-spec.md): cabecalho fixo
// (assinatura + versao + modelo + contador de quadros) seguido de secoes
// TLV (tag de 4 bytes + tamanho de 4 bytes + payload). TLV permite que uma
// versao futura adicione secoes novas sem quebrar leitores antigos (secao
// desconhecida e' simplesmente pulada) -- mesmo raciocinio do TZX (ver
// doc/tape-spec.md). NAO e' um formato portavel entre SO (os structs sao
// gravados quase crus, o layout de padding pode diferir entre MSVC e GCC) --
// um estado so' e' garantido carregar de volta no MESMO build que o salvou;
// ver a nota de escopo em Machine::SaveState() (machine.h).
constexpr char kStateMagic[8] = {'F', 'W', 'M', 'S', 'X', 'S', 'S', 'T'};
constexpr uint32_t kStateFormatVersion = 1;

void WriteRaw(std::ofstream &f, const void *data, std::size_t len) {
    f.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(len));
}

void WriteU32(std::ofstream &f, uint32_t v) { WriteRaw(f, &v, sizeof(v)); }
void WriteU64(std::ofstream &f, uint64_t v) { WriteRaw(f, &v, sizeof(v)); }

void WriteSection(std::ofstream &f, const char tag[4], const void *data, uint32_t len) {
    WriteRaw(f, tag, 4);
    WriteU32(f, len);
    if (len) WriteRaw(f, data, len);
}

bool TagIs(const char tag[4], const char *lit) { return std::memcmp(tag, lit, 4) == 0; }

} // namespace

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

bool Machine::SaveState(const std::string &path, std::string &error) const {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) {
        error = "nao foi possivel criar '" + path + "'";
        return false;
    }

    WriteRaw(f, kStateMagic, sizeof(kStateMagic));
    WriteU32(f, kStateFormatVersion);
    const uint8_t model_byte = static_cast<uint8_t>(model_);
    WriteRaw(f, &model_byte, 1);
    WriteU64(f, frame_count_);

    // Z80: so' os registradores de verdade -- iperiod/icount/ibackup/
    // irequest/iautoreset/trapbadops/user_data ficam de fora (bookkeeping
    // interno do core ou ponteiro do host, nunca "estado de jogo"; ver
    // Z80State em src/z80/common/z80_state.h).
    WriteSection(f, "Z80 ", &cpu_->state(), static_cast<uint32_t>(offsetof(Z80State, iperiod)));

    // VDP: tudo ate' scanline_snapshot (array derivado, recomputado a cada
    // RunFrame() -- nao faz sentido salvar, ver vdp_state.h).
    const VdpState &vdp = startup_.vdp_device->state();
    WriteSection(f, "VDP ", &vdp, static_cast<uint32_t>(offsetof(VdpState, scanline_snapshot)));

    // PSG: prefixo (r..env_hold) + sufixo (cycle_acc..box_count), pulando
    // joy[]/cassette_in -- "mundo externo" (joystick/fita), preservado do
    // estado AO VIVO ao carregar, nunca sobrescrito pelo save.
    {
        const PsgState &p = startup_.psg_device->state();
        const std::size_t prefix_len = offsetof(PsgState, joy);
        const std::size_t suffix_off = offsetof(PsgState, cycle_acc);
        const std::size_t suffix_len = sizeof(PsgState) - suffix_off;
        std::vector<uint8_t> buf(prefix_len + suffix_len);
        std::memcpy(buf.data(), &p, prefix_len);
        std::memcpy(buf.data() + prefix_len, reinterpret_cast<const uint8_t *>(&p) + suffix_off, suffix_len);
        WriteSection(f, "PSG ", buf.data(), static_cast<uint32_t>(buf.size()));
    }

    // SCC e OPLL: sempre existem (construidos incondicionalmente em
    // Create(), mesmo quando nao ha' cartucho SCC/FM-PAC -- ver o
    // comentario de scc_/fm_ em machine.h), sem campo "mundo externo"
    // nenhum a excluir.
    WriteSection(f, "SCC ", &scc_->state(), static_cast<uint32_t>(sizeof(SccState)));
    WriteSection(f, "OPLL", &fm_->state(), static_cast<uint32_t>(sizeof(Ym2413State)));

    // PPI: so' os registradores do chip (r/rout/rin); key_state (ultimo
    // campo) fica de fora -- teclas pressionadas sao entrada do mundo
    // externo, nao estado da maquina (ver doc/ppi-spec.md).
    {
        const PpiState &ppi = startup_.ppi_device->state();
        WriteSection(f, "PPI ", &ppi, static_cast<uint32_t>(offsetof(PpiState, key_state)));
    }

    // Fingerprint da midia: CRC32 da BIOS e do cartucho, para avisar ao carregar sobre outra midia.
    {
        uint32_t crcs[2] = {bios_crc_, cart_crc_};
        WriteSection(f, "MEDA", crcs, sizeof(crcs));
    }

    // Controladora de disco (pela memoria OU pelas portas, nunca as duas ao
    // mesmo tempo -- ver doc/fdc-spec.md): so' os registradores do WD2793,
    // ate' (sem incluir) `ptr`/`disk[]` (ponteiros para dentro da imagem
    // em disco, que continua a mesma -- recarregar o `ptr` exato de uma
    // transferencia em andamento nao e' suportado, ver a nota de escopo em
    // machine.h: uma transferencia em andamento no instante do save e'
    // abortada ao carregar, nunca deixada com um ponteiro invalido).
    if (fdc_) WriteSection(f, "FDCM", &fdc_->fdc(), static_cast<uint32_t>(offsetof(Fdc, ptr)));
    if (port_fdc_) WriteSection(f, "FDCP", &port_fdc_->fdc(), static_cast<uint32_t>(offsetof(Fdc, ptr)));

    // Fita: so' os campos pequenos que decidem o comportamento do proximo
    // TAPION/TAPOON (modo, protecao, modo de gravacao, ponto marcado, rele
    // do motor) -- a posicao exata de um pulso em andamento no modo Normal
    // fica de fora (mesma logica da nota do FDC acima).
    {
        const uint8_t mode_byte = static_cast<uint8_t>(tape_->mode());
        const uint8_t read_only_byte = tape_->read_only() ? 1 : 0;
        const uint8_t write_mode_byte = static_cast<uint8_t>(tape_->write_mode());
        const uint8_t motor_byte = tape_->motor_on() ? 1 : 0;
        const uint64_t marked = static_cast<uint64_t>(tape_->marked_file());
        uint8_t buf[4 + 8];
        buf[0] = mode_byte;
        buf[1] = read_only_byte;
        buf[2] = write_mode_byte;
        buf[3] = motor_byte;
        std::memcpy(buf + 4, &marked, 8);
        WriteSection(f, "TAPE", buf, sizeof(buf));
    }

    // Mapper(es) de RAM (MSX2, portas FCh-FFh): um por celula Mapper do
    // layout -- registrador de segmento (4 paginas) de cada um.
    {
        std::vector<uint8_t> buf;
        const uint32_t count = static_cast<uint32_t>(mappers_.size());
        buf.resize(4 + static_cast<std::size_t>(count) * 6);
        std::memcpy(buf.data(), &count, 4);
        std::size_t off = 4;
        for (const auto &m : mappers_) {
            buf[off + 0] = static_cast<uint8_t>(m->primary());
            buf[off + 1] = static_cast<uint8_t>(m->secondary());
            for (int page = 0; page < 4; ++page) buf[off + 2 + page] = m->segment(page);
            off += 6;
        }
        WriteSection(f, "MAPR", buf.data(), static_cast<uint32_t>(buf.size()));
    }

    // Conteudo de RAM: toda combinacao (primario,secundario) que e' RAM
    // comum ou RAM de mapper (SRAM de cartucho/FM-PAC fica de fora -- ja'
    // persiste sozinha no .sav, ver SaveSram() acima). RAM de mapper usa
    // MapperRamBase() (buffer inteiro, ate' 1024KB) porque PeekSlot so'
    // enxerga os 4 segmentos PAGINADOS agora, nao o mapper inteiro.
    {
        const memmap::MemorySystem &mem = *startup_.memory_system;
        for (int primary = 0; primary < MEMMAP_PRIMARY_SLOTS; ++primary) {
            for (int secondary = 0; secondary < MEMMAP_SECONDARY_SLOTS; ++secondary) {
                const int segments = mem.MapperSegments(primary, secondary);
                std::vector<uint8_t> payload;
                std::size_t size = 0;
                if (segments > 0) {
                    size = static_cast<std::size_t>(segments) * 0x4000;
                    const uint8_t *base = mem.MapperRamBase(primary, secondary);
                    payload.assign(base, base + size);
                } else if (mem.Describe(primary, secondary).kind == MEMMAP_KIND_RAM) {
                    size = mem.Describe(primary, secondary).size;
                    payload.resize(size);
                    for (std::size_t i = 0; i < size; ++i)
                        payload[i] = mem.PeekSlot(primary, secondary, static_cast<uint16_t>(i));
                } else {
                    continue;
                }
                std::vector<uint8_t> buf(2 + 4 + size);
                buf[0] = static_cast<uint8_t>(primary);
                buf[1] = static_cast<uint8_t>(secondary);
                const uint32_t size32 = static_cast<uint32_t>(size);
                std::memcpy(buf.data() + 2, &size32, 4);
                if (size) std::memcpy(buf.data() + 6, payload.data(), size);
                WriteSection(f, "RAM ", buf.data(), static_cast<uint32_t>(buf.size()));
            }
        }
    }

    if (!f) {
        error = "erro gravando '" + path + "'";
        return false;
    }
    return true;
}

bool Machine::LoadState(const std::string &path, std::string &error) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }

    char magic[8];
    if (!f.read(magic, sizeof(magic)) || std::memcmp(magic, kStateMagic, sizeof(magic)) != 0) {
        error = "'" + path + "' nao e' um estado salvo do fwMSX";
        return false;
    }
    uint32_t version = 0;
    if (!f.read(reinterpret_cast<char *>(&version), sizeof(version)) || version != kStateFormatVersion) {
        error = "'" + path + "': versao de formato de estado desconhecida";
        return false;
    }
    uint8_t model_byte = 0;
    if (!f.read(reinterpret_cast<char *>(&model_byte), 1)) {
        error = "'" + path + "': arquivo truncado";
        return false;
    }
    if (static_cast<Model>(model_byte) != model_) {
        error = "'" + path + "' foi salvo numa maquina diferente (MSX1/MSX2/MSX2+)";
        return false;
    }
    uint64_t saved_frame_count = 0;
    if (!f.read(reinterpret_cast<char *>(&saved_frame_count), sizeof(saved_frame_count))) {
        error = "'" + path + "': arquivo truncado";
        return false;
    }

    state_warning_.clear();
    memmap::MemorySystem &mem = *startup_.memory_system;
    char tag[4];
    while (f.read(tag, sizeof(tag))) {
        uint32_t len = 0;
        if (!f.read(reinterpret_cast<char *>(&len), sizeof(len))) break;
        std::vector<uint8_t> payload(len);
        if (len && !f.read(reinterpret_cast<char *>(payload.data()), len)) break;

        if (TagIs(tag, "Z80 ")) {
            if (payload.size() != offsetof(Z80State, iperiod)) continue;
            std::memcpy(&cpu_->state(), payload.data(), payload.size());
        } else if (TagIs(tag, "VDP ")) {
            if (payload.size() != offsetof(VdpState, scanline_snapshot)) continue;
            std::memcpy(&startup_.vdp_device->state(), payload.data(), payload.size());
        } else if (TagIs(tag, "PSG ")) {
            PsgState &p = startup_.psg_device->state();
            const std::size_t prefix_len = offsetof(PsgState, joy);
            const std::size_t suffix_off = offsetof(PsgState, cycle_acc);
            const std::size_t suffix_len = sizeof(PsgState) - suffix_off;
            if (payload.size() != prefix_len + suffix_len) continue;
            std::memcpy(&p, payload.data(), prefix_len);
            std::memcpy(reinterpret_cast<uint8_t *>(&p) + suffix_off, payload.data() + prefix_len, suffix_len);
        } else if (TagIs(tag, "SCC ")) {
            if (payload.size() != sizeof(SccState)) continue;
            std::memcpy(&scc_->state(), payload.data(), payload.size());
        } else if (TagIs(tag, "OPLL")) {
            if (payload.size() != sizeof(Ym2413State)) continue;
            std::memcpy(&fm_->state(), payload.data(), payload.size());
        } else if (TagIs(tag, "PPI ")) {
            if (payload.size() != offsetof(PpiState, key_state)) continue;
            std::memcpy(&startup_.ppi_device->state(), payload.data(), payload.size());
        } else if (TagIs(tag, "MEDA")) {
            if (payload.size() != 8) continue;
            uint32_t crcs[2];
            std::memcpy(crcs, payload.data(), 8);
            if (crcs[1] != cart_crc_) {
                state_warning_ = "o cartucho inserido agora e' diferente do que estava quando o estado foi salvo";
            } else if (crcs[0] != bios_crc_) {
                state_warning_ = "a BIOS atual e' diferente da que estava quando o estado foi salvo";
            }
        } else if (TagIs(tag, "FDCM")) {
            if (!fdc_ || payload.size() != offsetof(Fdc, ptr)) continue;
            std::memcpy(&fdc_->fdc(), payload.data(), payload.size());
            fdc_->fdc().wr_length = 0; // transferencia em andamento no save: abortada, nunca com ptr invalido
            fdc_->fdc().rd_length = 0;
            fdc_->fdc().trk_left = 0; // formatacao em andamento no save: abortada
        } else if (TagIs(tag, "FDCP")) {
            if (!port_fdc_ || payload.size() != offsetof(Fdc, ptr)) continue;
            std::memcpy(&port_fdc_->fdc(), payload.data(), payload.size());
            port_fdc_->fdc().wr_length = 0;
            port_fdc_->fdc().rd_length = 0;
            port_fdc_->fdc().trk_left = 0;
        } else if (TagIs(tag, "TAPE")) {
            if (payload.size() != 12) continue;
            tape_->SetMode(static_cast<tape::TapeMode>(payload[0]));
            tape_->SetReadOnly(payload[1] != 0);
            tape_->SetWriteMode(static_cast<tape::TapeWriteMode>(payload[2]));
            const bool motor = payload[3] != 0;
            tape_->SetMotor(motor);
            tape_motor_prev_ = motor;
            uint64_t marked = 0;
            std::memcpy(&marked, payload.data() + 4, 8);
            if (marked == static_cast<uint64_t>(tape::kNoMark)) {
                tape_->ClearMark();
            } else {
                tape_->SeekToFile(static_cast<std::size_t>(marked));
            }
        } else if (TagIs(tag, "MAPR")) {
            if (payload.size() < 4) continue;
            uint32_t count = 0;
            std::memcpy(&count, payload.data(), 4);
            if (payload.size() != 4 + static_cast<std::size_t>(count) * 6) continue;
            std::size_t off = 4;
            for (uint32_t i = 0; i < count; ++i) {
                const int primary = payload[off + 0];
                const int secondary = payload[off + 1];
                for (int page = 0; page < 4; ++page) mem.SetMapperSegment(primary, secondary, page, payload[off + 2 + page]);
                off += 6;
            }
        } else if (TagIs(tag, "RAM ")) {
            if (payload.size() < 6) continue;
            const int primary = payload[0];
            const int secondary = payload[1];
            uint32_t size = 0;
            std::memcpy(&size, payload.data() + 2, 4);
            if (payload.size() != 6 + static_cast<std::size_t>(size)) continue;
            const int segments = mem.MapperSegments(primary, secondary);
            if (segments > 0 && static_cast<std::size_t>(segments) * 0x4000 == size) {
                uint8_t *base = mem.MapperRamBase(primary, secondary);
                if (base) std::memcpy(base, payload.data() + 6, size);
            } else if (mem.Describe(primary, secondary).kind == MEMMAP_KIND_RAM &&
                       mem.Describe(primary, secondary).size == size) {
                for (uint32_t i = 0; i < size; ++i) mem.PokeSlot(primary, secondary, static_cast<uint16_t>(i), payload[6 + i]);
            }
        }
        // Tag desconhecida (versao futura): ja' consumida (payload lido
        // acima), so' ignorada aqui -- compatibilidade com leitores velhos.
    }

    frame_count_ = saved_frame_count;
    return true;
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

        // Fita (ver doc/tape-spec.md): o rele do motor e' o bit 4 da porta C
        // do PPI (AAh), ativo em ZERO (openMSX, MSXPPI::writeC1); so' o PPI
        // sabe disso, nao a Tape, por isso a sincronia e' feita aqui, como o
        // SyncSlot() do proprio PpiDevice faz para o slot primario.
        const bool motor_on = (startup_.ppi_device->state().rout[2] & 0x10) == 0;
        if (motor_on != tape_motor_prev_) {
            tape_motor_prev_ = motor_on;
            tape_->SetMotor(motor_on);
        }
        tape_->Advance(used);
        psg_set_cassette_in(&startup_.psg_device->state(), tape_->CassetteInLevel());
    }
    ++frame_count_;
}

void Machine::EnableLiveAudio(bool on) {
    startup_.psg_device->EnableLive(on);
    scc_->EnableLive(on);
    fm_->EnableLive(on);
    tape_->EnableLive(on);
}

void Machine::TakeLiveAudio(std::vector<int16_t> &out) {
    std::vector<int16_t> psg_samples, scc_samples, fm_samples, tape_samples;
    startup_.psg_device->TakeLive(psg_samples);
    scc_->TakeLive(scc_samples);
    fm_->TakeLive(fm_samples);
    tape_->TakeLive(tape_samples);

    // O FM soma ate' 9 canais: entra na metade para deixar folga ao PSG e ao SCC.
    // A fita (modo normal) e' so' uma onda quadrada: entra baixa, igual ao
    // click do teclado real, para nao dominar a mistura.
    const size_t n = std::max({psg_samples.size(), scc_samples.size(), fm_samples.size(), tape_samples.size()});
    out.reserve(out.size() + n);
    for (size_t i = 0; i < n; ++i) {
        const int psg = i < psg_samples.size() ? psg_samples[i] : 0;
        const int scc = i < scc_samples.size() ? scc_samples[i] : 0;
        const int fm = i < fm_samples.size() ? fm_samples[i] / 2 : 0;
        const int tape_s = i < tape_samples.size() ? tape_samples[i] : 0;
        out.push_back(static_cast<int16_t>(std::clamp(psg + scc + fm + tape_s, -32768, 32767)));
    }
}

bool Machine::InsertDisk(int drive, const std::string &path, std::string &error) {
    if (!fdc_engine_) {
        error = "esta maquina nao tem interface de disquete (use --disk ou --disk-interface)";
        return false;
    }
    drive &= 1;
    const std::string name = std::string(1, static_cast<char>('A' + drive));
    std::string load_error;
    if (!disks_[drive].Load(path, load_error, disk_read_only_)) {
        error = "disco " + name + ": " + load_error;
        return false;
    }
    // O formato configurado (180, 360 ou 720 KB) decide quais imagens o drive aceita.
    if (!fdc::SizeAllowed(disk_format_, disks_[drive].disk()->size)) {
        error = "disco " + name + ": o formato '" + fdc::FormatName(disk_format_) + "' nao aceita imagem de " +
                std::to_string(disks_[drive].disk()->size) + " bytes";
        disks_[drive].Eject();
        return false;
    }
    if (disk_format_ != fdc::DiskFormat::Auto) fdc::ApplyFormat(disks_[drive].disk(), *fdc::SpecFor(disk_format_));
    fdc_attach(fdc_engine_, drive, disks_[drive].disk());
    return true;
}

void Machine::EjectDisk(int drive) {
    drive &= 1;
    disks_[drive].Eject();
    if (fdc_engine_) fdc_attach(fdc_engine_, drive, disks_[drive].disk());
}

void Machine::Reset() {
    if (fdc_engine_) fdc_reset(fdc_engine_);
    for (auto &d : mappers_) d->Reset();
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
    case '&': key = "7"; shift = true; return true;
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
    // Copia por valor (nao referencia) -- cada linha e' desenhada depois de
    // "voltar no tempo" com vdp_apply_snapshot() (ver vdp_state.h), que
    // sobrescreve regs/paleta/cache de tabela da COPIA local a cada
    // iteracao; mexer na copia, nunca no estado de verdade da maquina,
    // evita qualquer risco de deixar o VDP "no passado" se o chamador
    // reler o estado depois (ex.: o depurador). vdp_render_border_color()/
    // width()/height() abaixo usam o estado FINAL do quadro (antes de
    // qualquer snapshot ser aplicado), igual antes.
    VdpState v = startup_.vdp_device->state();
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
            // Efeito de rastreio (ver doc/vdp-spec.md, secao 2): aplica o estado que
            // esta linha tinha DE VERDADE durante a execucao do quadro (capturado
            // por vdp_capture_snapshot() em vdp_step_scanline()), nao o estado final.
            if (y >= 0 && y < VDP_MAX_SCANLINES) vdp_apply_snapshot(&v, &v.scanline_snapshot[y]);
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
        // Efeito de rastreio (ver doc/vdp-spec.md, secao 2): aplica o estado que
        // esta linha tinha DE VERDADE durante a execucao do quadro (capturado
        // por vdp_capture_snapshot() em vdp_step_scanline()), nao o estado final.
        if (y >= 0 && y < VDP_MAX_SCANLINES) vdp_apply_snapshot(&v, &v.scanline_snapshot[y]);
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
