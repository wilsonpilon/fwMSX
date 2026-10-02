// fwMSX -- a maquina MSX1 completa (BIOS + slots + VDP + PPI/teclado + PSG +
// Z80) avancando em quadros, SEM nenhuma dependencia de janela/stdin.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/machine-spec.md.
//
// E' o que a janela (src/machine/gui/emu_window.cpp) e os testes
// (tests/z80/machine_test.cpp) usam: a janela so' chama RunFrame(),
// RenderFrame() e KeyDown()/KeyUp() -- toda a logica de maquina fica aqui,
// onde da' para testar sem GL.
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../memmap/core/slot_state.h"
#include "../z80/cpp/z80_cpu.h"
#include "../z80/debug/z80_debug_shell_startup.h"

namespace machine {

struct MachineConfig {
    std::string bios_path;
    // Cartucho opcional no slot 1 (vazio = so' BIOS + BASIC). Com mapper
    // MEMMAP_MAPPER_NONE e' ROM plana (ate' 32KB, posta em 4000h); com outro
    // mapper e' MegaROM (ver doc/memory-map-spec.md).
    std::string cart_path;
    MemMapMapperType cart_mapper = MEMMAP_MAPPER_NONE;
};

class Machine {
public:
    // Ciclos de Z80 por quadro NTSC: 262 linhas x 228 ciclos (59.92 Hz).
    static constexpr int kFrameCycles = 262 * 228;
    static constexpr double kFrameRate = 3579545.0 / kFrameCycles;
    static constexpr int kFrameWidth = 256;
    static constexpr int kFrameHeight = 192;

    // Devolve nullptr e preenche `error` se a BIOS/cartucho nao carregarem.
    static std::unique_ptr<Machine> Create(const MachineConfig &config, std::string &error);

    // Executa um quadro (kFrameCycles ciclos), avancando VDP e PSG e
    // entregando a interrupcao de VBlank ao Z80.
    void RunFrame();

    // Reset de maquina: CPU, VDP, PPI (teclas pressionadas continuam) e PSG.
    void Reset();

    // Teclado do MSX por nome (a-z, 0-9, shift, enter, space, f1-f5...; ver
    // ppi_key_name()). Devolvem false se o nome for desconhecido.
    bool KeyDown(const std::string &name);
    bool KeyUp(const std::string &name);
    void ReleaseAllKeys();

    // Renderiza o quadro atual em `rgba` (256x192, 4 bytes por pixel na
    // ordem R,G,B,A -- direto para glTexImage2D). Telas mais estreitas
    // (SCREEN 0, 240px) ficam centralizadas sobre a cor de fundo.
    void RenderFrame(std::vector<uint32_t> &rgba) const;

    uint64_t frame_count() const { return frame_count_; }
    const VdpState &vdp_state() const { return startup_.vdp_device->state(); }
    PpiState &ppi_state() { return startup_.ppi_device->state(); }
    psg::PsgDevice &psg() { return *startup_.psg_device; }
    z80::Z80Cpu &cpu() { return *cpu_; }
    memmap::MemorySystem &memory() { return *startup_.memory_system; }

private:
    Machine() = default;

    z80::debug::Z80DebugShellStartup startup_;
    std::unique_ptr<z80::Z80Cpu> cpu_;
    int vdp_pending_cycles_ = 0;
    uint64_t frame_count_ = 0;
};

} // namespace machine
