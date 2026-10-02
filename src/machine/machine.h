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

#include "../fdc/cpp/disk_image.h"
#include "../fdc/cpp/fdc_device.h"
#include "../memmap/core/slot_state.h"
#include "../z80/cpp/z80_cpu.h"
#include "../z80/debug/z80_debug_shell_startup.h"

namespace machine {

struct MachineConfig {
    std::string bios_path;
    // Cartucho opcional no slot 1 (vazio = so' BIOS + BASIC). Com mapper
    // MEMMAP_MAPPER_NONE (padrao): ate' 32KB e' ROM plana (em 4000h, ou em
    // 8000h se o cabecalho "AB" aponta o INIT para la'); acima de 32KB e'
    // MegaROM com o mapper detectado automaticamente (memmap::GuessMapper).
    // Com outro mapper, e' MegaROM desse mapper (ver doc/memory-map-spec.md).
    std::string cart_path;
    MemMapMapperType cart_mapper = MEMMAP_MAPPER_NONE;

    // Disco (ver doc/fdc-spec.md): a interface de disquete (DISK.ROM no slot
    // 3:1 + controladora WD2793) so' e' ligada quando ha' um disco em A:/B:
    // ou `disk_interface` e' true -- sem ela o MSX BASIC e' o "puro" (28815
    // bytes livres), com ela vira o Disk BASIC. `disk_rom_path` vazio = DISK.ROM
    // ao lado da BIOS.
    std::string disk_a;
    std::string disk_b;
    bool disk_interface = false;
    std::string disk_rom_path;
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

    // Joystick das portas A (0) e B (1): mascara de PSG_JOY_* (1 = pressionado).
    void SetJoystick(int port, uint8_t bits);

    // Interface de disquete: true se o DISK.ROM esta no slot 3:1. Inserir/ejetar
    // disco em A: (0) / B: (1) a qualquer momento; as escritas do MSX vao
    // direto para o arquivo da imagem.
    bool has_disk_interface() const { return fdc_ != nullptr; }
    bool InsertDisk(int drive, const std::string &path, std::string &error);
    void EjectDisk(int drive);
    const fdc::DiskImage &disk(int drive) const { return disks_[drive & 1]; }
    fdc::FdcDevice *fdc() { return fdc_.get(); }

    // Resumo do cartucho carregado ("" se nao ha'): tamanho e mapper.
    const std::string &cart_info() const { return cart_info_; }

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
    std::string cart_info_;
    std::unique_ptr<fdc::FdcDevice> fdc_;
    fdc::DiskImage disks_[2];
};

} // namespace machine
