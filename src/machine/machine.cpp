#include "machine.h"

#include <algorithm>
#include <fstream>
#include <iterator>

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
        bool ok;
        if (config.cart_mapper == MEMMAP_MAPPER_NONE) {
            // Cartucho de ROM plana: o cartucho real responde em 4000h-BFFFh,
            // mas LoadRom() poe a imagem em 0000h do slot -- entao preenche
            // 0000h-3FFFh com zeros antes (a BIOS procura o cabecalho "AB" em
            // 4000h).
            if (data.size() > 0x8000) {
                error = "cartucho de ROM plana tem no maximo 32KB (use um mapper para MegaROM)";
                return nullptr;
            }
            std::vector<uint8_t> image(0x4000, 0);
            image.insert(image.end(), data.begin(), data.end());
            while (image.size() % 0x2000) image.push_back(0);
            ok = m->startup_.memory_system->LoadRom(1, 0, image.data(), image.size(), &load_error);
        } else {
            ok = m->startup_.memory_system->LoadRom(1, 0, data.data(), data.size(), &load_error, config.cart_mapper);
        }
        if (!ok) {
            error = "cartucho: " + load_error;
            return nullptr;
        }
    }

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
    }
    ++frame_count_;
}

void Machine::Reset() {
    cpu_->reset();
    startup_.vdp_device->Reset();
    startup_.ppi_device->Reset();
    startup_.psg_device->Reset();
    vdp_pending_cycles_ = 0;
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

void Machine::ReleaseAllKeys() { ppi_key_release_all(&startup_.ppi_device->state()); }

void Machine::RenderFrame(std::vector<uint32_t> &rgba) const {
    const VdpState &v = startup_.vdp_device->state();
    const int width = vdp_render_width(&v);
    const int pad = (kFrameWidth - width) / 2;

    rgba.assign(static_cast<size_t>(kFrameWidth) * kFrameHeight,
                PackRgba(v.palette_r[v.regs[7] & 0x0F], v.palette_g[v.regs[7] & 0x0F], v.palette_b[v.regs[7] & 0x0F]));

    std::vector<VdpRgb888> row(static_cast<size_t>(kFrameWidth));
    for (int y = 0; y < kFrameHeight; ++y) {
        vdp_render_line(&v, y, row.data());
        uint32_t *dst = rgba.data() + static_cast<size_t>(y) * kFrameWidth + pad;
        for (int x = 0; x < width; ++x) dst[x] = PackRgba(row[x].r, row[x].g, row[x].b);
    }
}

} // namespace machine
