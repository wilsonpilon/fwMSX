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

std::unique_ptr<Machine> Machine::Create(const MachineConfig &config, std::string &error) {
    if (config.bios_path.empty()) {
        error = "nenhuma BIOS informada";
        return nullptr;
    }

    std::unique_ptr<Machine> m(new Machine());
    m->startup_ = z80::debug::BuildZ80DebugShellStartup({"--slots", config.bios_path, "--vdp", "--ppi", "--psg"});
    if (!m->startup_.boot_rom_loaded) {
        error = "BIOS: " + m->startup_.boot_rom_error;
        return nullptr;
    }

    if (!config.cart_path.empty()) {
        std::vector<uint8_t> data;
        if (!ReadFile(config.cart_path, data, error)) return nullptr;
        std::string load_error;
        MemMapMapperType mapper = config.cart_mapper;
        const bool guessed = mapper == MEMMAP_MAPPER_NONE && data.size() > 0x8000;
        if (guessed) mapper = memmap::GuessMapper(data.data(), data.size());
        const char *mapper_name = "ROM plana";
        switch (mapper) {
        case MEMMAP_MAPPER_GEN8: mapper_name = "Gen8"; break;
        case MEMMAP_MAPPER_GEN16: mapper_name = "Gen16"; break;
        case MEMMAP_MAPPER_KONAMI5: mapper_name = "Konami5"; break;
        case MEMMAP_MAPPER_KONAMI4: mapper_name = "Konami4"; break;
        case MEMMAP_MAPPER_ASCII8: mapper_name = "ASCII8"; break;
        case MEMMAP_MAPPER_ASCII16: mapper_name = "ASCII16"; break;
        default: break;
        }

        bool ok;
        if (mapper == MEMMAP_MAPPER_NONE) {
            // Cartucho de ROM plana: o cartucho real responde em 4000h-BFFFh,
            // mas LoadRom() poe a imagem em 0000h do slot -- entao preenche o
            // comeco com zeros (a BIOS procura o cabecalho "AB" em 4000h, e
            // em 8000h). Uma ROM de ate' 16KB cujo INIT/STATEMENT/DEVICE/TEXT
            // aponta para 8000h+ vai para 8000h (cartuchos de pagina 2).
            size_t base = 0x4000;
            if (data.size() >= 16 && data[0] == 'A' && data[1] == 'B' && data.size() <= 0x4000) {
                unsigned high = 0;
                for (int off : {2, 4, 6, 8}) high = std::max<unsigned>(high, data[off + 1]);
                if (high >= 0x80) base = 0x8000;
            }
            std::vector<uint8_t> image(base, 0);
            image.insert(image.end(), data.begin(), data.end());
            while (image.size() % 0x2000) image.push_back(0);
            ok = m->startup_.memory_system->LoadRom(1, 0, image.data(), image.size(), &load_error);
        } else {
            ok = m->startup_.memory_system->LoadRom(1, 0, data.data(), data.size(), &load_error, mapper);
        }
        if (!ok) {
            error = "cartucho: " + load_error;
            return nullptr;
        }
        m->cart_info_ = std::to_string(data.size() / 1024) + "KB, " + mapper_name + (guessed ? " (detectado)" : "");
    }

    m->model_ = config.model;
    m->disk_read_only_ = config.disk_read_only;
    const bool msx2 = config.model != Model::MSX1;
    const bool want_disk = config.disk_interface || !config.disk_a.empty() || !config.disk_b.empty();
    const std::string bios_dir = [&] {
        const size_t slash = config.bios_path.find_last_of("/\\");
        return slash == std::string::npos ? std::string() : config.bios_path.substr(0, slash + 1);
    }();

    // O slot 3:1 hospeda a sub-ROM do MSX2 (em 0000h-3FFFh) e a DISK.ROM (em
    // 4000h-7FFFh): uma unica ROM plana de 32KB, montada aqui.
    std::vector<uint8_t> slot31;

    if (msx2) {
        // VDP V9938 (128KB, comandos), RAM de 128KB com mapper (3:2), regras de
        // subslot do MSX2, e as portas do mapper (FCh-FFh) e do relogio (B4h/B5h).
        m->startup_.vdp_device->SetModel(config.model == Model::MSX2P ? VDP_MODEL_MSX2P : VDP_MODEL_MSX2);
        memmap::MemorySystem &mem = *m->startup_.memory_system;
        mem.state().msx1_subslot_rules = 2;
        mem.AllocateMapperRam(3, 2, 8);
        m->mapper_ = std::make_unique<memmap::RamMapperDevice>(mem, 3, 2);
        m->rtc_ = std::make_unique<rtc::RtcDevice>();
        m->startup_.composite_bus->RegisterPortRange(0xFC, 0xFF, m->mapper_.get());
        m->startup_.composite_bus->RegisterPortRange(0xB4, 0xB5, m->rtc_.get());

        const char *ext_name = config.model == Model::MSX2P ? "MSX2PEXT.ROM" : "MSX2EXT.ROM";
        std::string ext_path = config.ext_rom_path.empty() ? bios_dir + ext_name : config.ext_rom_path;
        std::vector<uint8_t> ext;
        if (!ReadFile(ext_path, ext, error)) {
            error = "sub-ROM do MSX2: " + error;
            return nullptr;
        }
        if (ext.size() != 0x4000) {
            error = "sub-ROM do MSX2 '" + ext_path + "': tamanho invalido (" + std::to_string(ext.size()) + " bytes, esperava 16384)";
            return nullptr;
        }
        slot31 = ext;
    }

    // Interface de disquete: DISK.ROM em 3:1 (como o fMSX), com a controladora
    // mapeada em 7FF8h-7FFFh daquele slot.
    if (want_disk) {
        std::string rom_path = config.disk_rom_path.empty() ? bios_dir + "DISK.ROM" : config.disk_rom_path;
        std::vector<uint8_t> rom;
        if (!ReadFile(rom_path, rom, error)) {
            error = "DISK.ROM: " + error;
            return nullptr;
        }
        if (rom.empty() || rom.size() > 0x4000) {
            error = "DISK.ROM: tamanho invalido (" + std::to_string(rom.size()) + " bytes)";
            return nullptr;
        }
        slot31.resize(0x4000, 0); // sem sub-ROM (MSX1): 0000h-3FFFh vazio
        slot31.insert(slot31.end(), rom.begin(), rom.end());
    }

    if (!slot31.empty()) {
        while (slot31.size() % 0x2000) slot31.push_back(0);
        std::string load_error;
        if (!m->startup_.memory_system->LoadRom(3, 1, slot31.data(), slot31.size(), &load_error)) {
            error = "slot 3:1 (sub-ROM/DISK.ROM): " + load_error;
            return nullptr;
        }
    }

    if (want_disk) {
        m->fdc_ = std::make_unique<fdc::FdcDevice>();
        m->startup_.slot_bus->AttachMmio(3, 1, m->fdc_.get());
        for (int d = 0; d < 2; ++d) fdc_attach(&m->fdc_->fdc(), d, m->disks_[d].disk());
        const std::string *paths[2] = {&config.disk_a, &config.disk_b};
        for (int d = 0; d < 2; ++d) {
            if (paths[d]->empty()) continue;
            if (!m->InsertDisk(d, *paths[d], error)) return nullptr;
        }
    }

    m->scc_ = std::make_unique<scc::SccDevice>();
    m->startup_.slot_bus->AttachCart(1, 0, m->scc_.get());

    m->cpu_ = std::make_unique<z80::Z80Cpu>(m->startup_.Bus());
    return m;
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
    }
    ++frame_count_;
}

void Machine::EnableLiveAudio(bool on) {
    startup_.psg_device->EnableLive(on);
    scc_->EnableLive(on);
}

void Machine::TakeLiveAudio(std::vector<int16_t> &out) {
    std::vector<int16_t> psg_samples, scc_samples;
    startup_.psg_device->TakeLive(psg_samples);
    scc_->TakeLive(scc_samples);

    const size_t n = std::max(psg_samples.size(), scc_samples.size());
    out.reserve(out.size() + n);
    for (size_t i = 0; i < n; ++i) {
        const int sum = (i < psg_samples.size() ? psg_samples[i] : 0) + (i < scc_samples.size() ? scc_samples[i] : 0);
        out.push_back(static_cast<int16_t>(std::clamp(sum, -32768, 32767)));
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
    case '*': key = "'"; shift = true; return true;
    case '(': key = "8"; shift = true; return true;
    case ')': key = "9"; shift = true; return true;
    case '_': key = "-"; shift = true; return true;
    case '+': key = "="; shift = true; return true;
    case ':': key = ";"; shift = true; return true;
    case '"': key = "2"; shift = true; return true;
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
    std::vector<VdpRgb888> row(VDP_RENDER_MAX_WIDTH);

    FrameSize size;
    if (model_ == Model::MSX1) {
        // MSX1: 256x192, telas mais estreitas centralizadas sobre a borda
        const int pad = (kFrameWidth - width) / 2;
        size.width = kFrameWidth;
        size.height = kFrameHeight;
        rgba.assign(static_cast<size_t>(size.width) * size.height, border_px);
        for (int y = 0; y < size.height; ++y) {
            vdp_render_line(&v, y, row.data());
            uint32_t *dst = rgba.data() + static_cast<size_t>(y) * size.width + pad;
            for (int x = 0; x < width; ++x) dst[x] = PackRgba(row[x].r, row[x].g, row[x].b);
        }
        return size;
    }

    // MSX2: 512 pixels de largura. Modos de 256 (e o texto de 40 colunas, 240)
    // saem dobrados na horizontal; os de 512 (SCREEN 6/7, TEXT80 de 480) saem
    // como sao. O que sobra nas laterais e' a cor da borda.
    size.width = 512;
    size.height = lines;
    size.y_scale = 2;
    rgba.assign(static_cast<size_t>(size.width) * size.height, border_px);
    const int factor = width <= 256 ? 2 : 1;
    const int out_width = width * factor;
    const int pad = (size.width - out_width) / 2;
    for (int y = 0; y < lines; ++y) {
        vdp_render_line(&v, y, row.data());
        uint32_t *dst = rgba.data() + static_cast<size_t>(y) * size.width + pad;
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
