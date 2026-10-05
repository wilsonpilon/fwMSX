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
#include "../memmap/cpp/ram_mapper.h"
#include "../rtc/rtc_device.h"
#include "../scc/cpp/scc_device.h"
#include "../z80/cpp/z80_cpu.h"
#include "../z80/debug/z80_debug_shell_startup.h"

namespace machine {

// Modelo da maquina: MSX1 (TMS9918, 64KB de RAM, MSX.ROM) ou MSX2 (V9938 com
// 128KB de VRAM e motor de comandos, RAM de 128KB com mapper, relogio RTC,
// MSX2.ROM + MSX2EXT.ROM). Ver doc/msx2-spec.md.
enum class Model { MSX1, MSX2, MSX2P };

// Tamanho da imagem de RenderFrame(): `width` x `height` pixels e quantas vezes
// cada linha deve ser repetida ao exibir (`y_scale`): 2 nas imagens de 512 de
// largura do MSX2, onde o pixel e' a metade da largura de um pixel de 256.
struct FrameSize {
    int width = 256;
    int height = 192;
    int y_scale = 1;
};

struct MachineConfig {
    Model model = Model::MSX1;
    // BIOS principal (MSX.ROM, ou MSX2.ROM no MSX2).
    std::string bios_path;
    // Sub-ROM do MSX2 (MSX2EXT.ROM, 16KB, no slot 3:1). Vazio = ao lado da BIOS.
    std::string ext_rom_path;
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
    // Discos inseridos (agora e depois, pelo menu) entram protegidos contra
    // gravacao: o MSX-DOS le normalmente e recusa escrever, e o arquivo da
    // imagem nunca e' alterado.
    bool disk_read_only = false;
};

class Machine {
public:
    // Ciclos de Z80 por quadro NTSC: 262 linhas x 228 ciclos (59.92 Hz).
    static constexpr int kFrameCycles = 262 * 228;
    static constexpr double kFrameRate = 3579545.0 / kFrameCycles;
    // Quadro com borda, como o fMSX (WIDTH 272 x HEIGHT 228): 8 pixels de borda de
    // cada lado; 18 linhas em cima e embaixo para 192 linhas (8 para 212). A borda
    // tem a cor de fundo (R#7). MSX2 sai com 16 de borda por lado (pixels dobrados).
    static constexpr int kFrameWidth = 272;
    static constexpr int kFrameHeight = 228;

    // Devolve nullptr e preenche `error` se a BIOS/cartucho nao carregarem.
    static std::unique_ptr<Machine> Create(const MachineConfig &config, std::string &error);

    // Executa um quadro (kFrameCycles ciclos), avancando VDP e PSG e
    // entregando a interrupcao de VBlank ao Z80.
    void RunFrame();

    // Reset de maquina: CPU, VDP, PPI (teclas pressionadas continuam) e PSG.
    void Reset();

    // Teclado do MSX por nome (a-z, 0-9, shift, enter, space, f1-f5...; ver
    // ppi_key_name()). Devolvem false se o nome for desconhecido.
    // Tecla(s) MSX para um caractere (letras, numeros, espaco, pontuacao comum com
    // SHIFT quando preciso; '|' = ENTER) -- usado pelo --keys e pelos testes.
    // Devolve false se o caractere nao tem tecla.
    static bool KeysForChar(char c, std::string &key, bool &shift);

    bool KeyDown(const std::string &name);
    bool KeyUp(const std::string &name);
    void ReleaseAllKeys();

    // Renderiza o quadro atual em `rgba` (4 bytes por pixel na ordem R,G,B,A --
    // direto para glTexImage2D) e devolve o tamanho. MSX1: sempre 256x192
    // (telas mais estreitas ficam centralizadas sobre a cor de fundo). MSX2:
    // 512 de largura x 192 ou 212 linhas (os modos de 256 pixels saem
    // dobrados; a imagem pede y_scale=2 ao exibir, ver FrameSize).
    FrameSize RenderFrame(std::vector<uint32_t> &rgba) const;

    // MSX2 e MSX2+ (V9938/V9958): o que a janela e o mapa de memoria tratam como 'MSX2'.
    bool is_msx2() const { return model_ != Model::MSX1; }
    // Mapper de RAM e relogio (so' no MSX2; nullptr no MSX1).
    memmap::RamMapperDevice *mapper() { return mapper_.get(); }
    rtc::RtcDevice *rtc() { return rtc_.get(); }

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
    scc::SccDevice &scc() { return *scc_; }

    // Saida de audio ao vivo: PSG e SCC somados (saturados em 16 bits). `out`
    // recebe as amostras acumuladas desde a ultima chamada.
    void EnableLiveAudio(bool on);
    void TakeLiveAudio(std::vector<int16_t> &out);

    z80::Z80Cpu &cpu() { return *cpu_; }
    memmap::MemorySystem &memory() { return *startup_.memory_system; }

private:
    Machine() = default;

    z80::debug::Z80DebugShellStartup startup_;
    std::unique_ptr<z80::Z80Cpu> cpu_;
    int vdp_pending_cycles_ = 0;
    uint64_t frame_count_ = 0;
    std::string cart_info_;
    Model model_ = Model::MSX1;
    std::unique_ptr<memmap::RamMapperDevice> mapper_;
    std::unique_ptr<rtc::RtcDevice> rtc_;
    std::unique_ptr<fdc::FdcDevice> fdc_;
    std::unique_ptr<scc::SccDevice> scc_;
    fdc::DiskImage disks_[2];
    bool disk_read_only_ = false;
};

} // namespace machine
