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
#include "../fdc/cpp/disk_format.h"
#include "../fdc/cpp/fdc_device.h"
#include "../fdc/cpp/fdc_port.h"
#include "../memmap/core/slot_state.h"
#include "../memmap/cpp/ram_mapper.h"
#include "../rtc/rtc_device.h"
#include "../scc/cpp/scc_device.h"
#include "../fm/cpp/fm_device.h"
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

// Acesso da controladora de disco: pela memoria (DISK.ROM no slot 3:1, como sempre) ou pelas
// portas (WD2793 em disk_port..disk_port+4, sem ROM). Ver doc/fdc-spec.md, secao 6.
enum class DiskAccess { Memory, Port };

// Layout de slots: cada combinacao primario:secundario (celula) tem um conteudo.
// Ver doc/slots-spec.md.
enum class SlotKind {
    Empty,   // nada ligado nesta celula
    Rom,     // ROM: BIOS (0:0), BASIC, cartucho (ROM plana ou MegaROM)
    SubRom,  // sub-ROM de 16KB na pagina 0 (MSX2EXT)
    Ram,     // RAM comum (16, 32 ou 64 KB)
    Mapper,  // RAM mapeada (memory mapper, portas FCh-FFh): 64 a 1024 KB
    Disk,    // interface de disquete: DISK.ROM na pagina 1 + controladora WD2793
    FmPac,   // FM-PAC: ROM de 16KB e SRAM de 8KB, com o OPLL
};

struct SlotItem {
    SlotKind kind = SlotKind::Empty;
    // Rom: ROM principal. SubRom: a sub-ROM. Disk: DISK.ROM. FmPac: FMPAC.ROM.
    std::string path;
    // Rom: segunda ROM de 16KB, que vai para a pagina 1 (ex.: BASIC, com a BIOS
    // na pagina 0). Disk: sub-ROM do MSX2 (16KB, pagina 0). Vazio = nenhuma.
    std::string path2;
    // Rom de 16KB ou 32KB: pagina 0 (0000h) ou 1 (4000h). Uma ROM de 32KB em pagina 0
    // ocupa 0000h-7FFFh (BIOS); em pagina 1 ocupa 4000h-BFFFh (cartucho).
    int page = 1;
    // Rom de cartucho: mapper. NONE = detecta pelo tamanho, como antes.
    MemMapMapperType mapper = MEMMAP_MAPPER_NONE;
    // Ram: 16, 32 ou 64. Mapper: 64, 128, 256, 512 ou 1024.
    int size_kb = 64;
};

struct SlotLayout {
    SlotItem cell[4][4]; // [primario][secundario]
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
    // FM-PAC (Panasonic, OPLL + SRAM de 8KB): ROM de 16KB no slot 2:0. Vazio = sem
    // FM-PAC. A ROM e' o FMPAC.ROM do fMSX. Ver doc/fm-spec.md, secao 4.
    std::string fmpac_rom_path;

    // Controladora de disco e formato dos drives (180, 360 ou 720 KB). Padrao: pela memoria e
    // formato automatico (aceita qualquer um dos tres tamanhos).
    DiskAccess disk_access = DiskAccess::Memory;
    int disk_port = 0xD0;
    fdc::DiskFormat disk_format = fdc::DiskFormat::Auto;

    // Layout de slots. Enquanto `layout_set` for false, o layout sai dos campos
    // acima (bios, cartucho, disco, sub-ROM, FM-PAC) pela regra padrao. Depois
    // de editado pelo menu, vale o `layout` e os campos acima sao ignorados.
    SlotLayout layout;
    bool layout_set = false;
};

// Layout padrao (o mesmo de sempre) a partir dos campos de MachineConfig.
SlotLayout DefaultLayout(const MachineConfig &config);
// O layout que a maquina usa: o editado, se houver, senao o padrao.
SlotLayout EffectiveLayout(const MachineConfig &config);
// Recusa layouts que nao montam: BIOS em 0:0, uma so' RAM mapeada, tamanhos validos.
bool ValidateLayout(const SlotLayout &layout, std::string &error);
// Edicao pelos menus (marca layout_set): cartucho no slot 1:0, FM-PAC no slot 2:0.
void SetCartridge(MachineConfig &config, const std::string &path);
void SetFmPac(MachineConfig &config, const std::string &path);
// Caminhos atuais no layout (vazio = nenhum).
std::string CartridgePath(const MachineConfig &config);
std::string FmPacPath(const MachineConfig &config);

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
    // Primeiro mapper do layout (nullptr sem mapper). Ha' um por celula Mapper.
    memmap::RamMapperDevice *mapper() { return mappers_.empty() ? nullptr : mappers_.front().get(); }
    rtc::RtcDevice *rtc() { return rtc_.get(); }

    // Joystick das portas A (0) e B (1): mascara de PSG_JOY_* (1 = pressionado).
    void SetJoystick(int port, uint8_t bits);

    // Interface de disquete: true se o DISK.ROM esta no slot 3:1. Inserir/ejetar
    // disco em A: (0) / B: (1) a qualquer momento; as escritas do MSX vao
    // direto para o arquivo da imagem.
    bool has_disk_interface() const { return fdc_ != nullptr || port_fdc_ != nullptr; }
    bool InsertDisk(int drive, const std::string &path, std::string &error);
    void EjectDisk(int drive);
    const fdc::DiskImage &disk(int drive) const { return disks_[drive & 1]; }
    fdc::FdcDevice *fdc() { return fdc_.get(); }
    fdc::PortFdcDevice *port_fdc() { return port_fdc_.get(); }
    fdc::DiskFormat disk_format() const { return disk_format_; }
    bool disk_access_is_port() const { return port_fdc_ != nullptr; }

    // Resumo do cartucho carregado ("" se nao ha'): tamanho e mapper.
    const std::string &cart_info() const { return cart_info_; }

    // SRAM de cartucho (ASCII8/ASCII16) e do FM-PAC: arquivo .sav ao lado da ROM.
    // SaveSram() grava so' o que mudou desde a ultima gravacao.
    bool has_sram() const { return !sram_targets_.empty(); }
    bool sram_dirty() const;
    bool SaveSram(std::string &error);
    const std::string &sram_path() const { return sram_targets_.empty() ? empty_path_ : sram_targets_[0].path; }

    uint64_t frame_count() const { return frame_count_; }
    const VdpState &vdp_state() const { return startup_.vdp_device->state(); }
    PpiState &ppi_state() { return startup_.ppi_device->state(); }
    psg::PsgDevice &psg() { return *startup_.psg_device; }
    scc::SccDevice &scc() { return *scc_; }
    fm::FmDevice &fm() { return *fm_; }

    // Saida de audio ao vivo: PSG, SCC e FM somados (saturados em 16 bits). `out`
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
    // SRAM de cada cartucho/FM-PAC com memoria de bateria: slot e arquivo .sav.
    struct SramTarget {
        int primary;
        int secondary;
        std::string path;
    };
    std::vector<SramTarget> sram_targets_;
    std::string empty_path_;
    Model model_ = Model::MSX1;
    // Um mapper por celula Mapper do layout; as portas FCh-FFh chegam a todos (como o hardware).
    std::vector<std::unique_ptr<memmap::RamMapperDevice>> mappers_;
    std::unique_ptr<z80::IBus> mapper_ports_;
    std::unique_ptr<rtc::RtcDevice> rtc_;
    std::unique_ptr<fdc::FdcDevice> fdc_;
    std::unique_ptr<fdc::PortFdcDevice> port_fdc_;
    ::Fdc *fdc_engine_ = nullptr;  // o WD2793 em uso (por memoria ou por portas)
    fdc::DiskFormat disk_format_ = fdc::DiskFormat::Auto;
    std::unique_ptr<scc::SccDevice> scc_;
    std::unique_ptr<fm::FmDevice> fm_;
    fdc::DiskImage disks_[2];
    bool disk_read_only_ = false;
};

} // namespace machine
