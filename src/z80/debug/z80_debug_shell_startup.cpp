// fwMSX -- ver z80_debug_shell_startup.h. Sem replxx/stdin de proposito.
#include "z80_debug_shell_startup.h"

#include <cstdint>
#include <fstream>

namespace z80::debug {

Z80DebugShellStartup BuildZ80DebugShellStartup(const std::vector<std::string> &args) {
    Z80DebugShellStartup startup;
    startup.flat_bus = std::make_unique<FlatMemoryBus>();

    // Regra de sintaxe (ver z80_debug_shell_startup.h): so o token
    // IMEDIATAMENTE seguinte a "--slots" (se existir) vira caminho de ROM
    // de boot. "--vdp" e' um token independente (qualquer posicao).
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--vdp") {
            startup.use_vdp = true;
            continue;
        }
        if (args[i] == "--ppi") {
            startup.use_ppi = true;
            continue;
        }
        if (args[i] == "--psg") {
            startup.use_psg = true;
            continue;
        }
        if (args[i] != "--slots") continue;
        startup.use_slots = true;
        if (i + 1 < args.size() && args[i + 1] != "--vdp" && args[i + 1] != "--ppi" &&
            args[i + 1] != "--psg") {
            startup.boot_rom_path = args[i + 1];
            startup.boot_rom_requested = true;
        }
    }

    if (startup.use_vdp && !startup.use_slots) {
        startup.vdp_error = "--vdp requer --slots (ignorado nesta sessao)";
        startup.use_vdp = false;
    }

    if (startup.use_ppi && !startup.use_slots) {
        startup.ppi_error = "--ppi requer --slots (ignorado nesta sessao)";
        startup.use_ppi = false;
    }

    if (startup.use_psg && !startup.use_slots) {
        startup.psg_error = "--psg requer --slots (ignorado nesta sessao)";
        startup.use_psg = false;
    }

    if (!startup.use_slots) return startup;

    startup.memory_system = std::make_unique<memmap::MemorySystem>();

    if (startup.boot_rom_requested) {
        std::ifstream file(startup.boot_rom_path, std::ios::binary | std::ios::ate);
        if (!file) {
            startup.boot_rom_error = "nao foi possivel abrir '" + startup.boot_rom_path + "'";
        } else {
            const std::streamsize size = file.tellg();
            if (size < 0) {
                startup.boot_rom_error = "falha ao ler tamanho de '" + startup.boot_rom_path + "'";
            } else {
                file.seekg(0, std::ios::beg);
                std::vector<uint8_t> data(static_cast<std::size_t>(size));
                if (size > 0 && !file.read(reinterpret_cast<char *>(data.data()), size)) {
                    startup.boot_rom_error = "erro de leitura em '" + startup.boot_rom_path + "'";
                } else {
                    std::string load_error;
                    // Sempre ROM plana (mapper=MEMMAP_MAPPER_NONE, default de
                    // LoadRom()) -- igual uma BIOS carregada na mao (Fase 2),
                    // sem assumir nenhum tipo de MegaROM.
                    if (startup.memory_system->LoadRom(0, 0, data.data(), data.size(), &load_error)) {
                        startup.boot_rom_loaded = true;
                    } else {
                        startup.boot_rom_error = load_error;
                    }
                }
            }
        }
    }

    // Sem ROM de boot carregada (nenhuma pedida, ou pedida mas falhou) --
    // cai no estado "--slots" original: RAM plana de 64KB em 0:0, ja'
    // visivel por padrao (ver a nota historica em memory-map-spec.md,
    // Fase 1). "Avisa, nao trava" -- a sessao sempre inicia.
    if (!startup.boot_rom_loaded) {
        startup.memory_system->AllocateRam(0, 0, 0x10000);
    }

    // "--ppi" com BIOS carregada: layout MSX1 padrao (como o fMSX) -- RAM de
    // 64KB no slot primario 3. Sem isso a BIOS real nao tem onde montar
    // sua area de trabalho (a RAM em 0:0 so' existe quando NAO ha' ROM de
    // boot) e fica presa na rotina de limpeza/teste de RAM, mesmo com o PPI
    // funcionando -- ver doc/ppi-spec.md.
    if (startup.use_ppi && startup.boot_rom_loaded) {
        startup.memory_system->AllocateRam(3, 2, 0x10000);
        // Hardware MSX1: so' o slot 3 e' expandido (ver slot_state.h).
        startup.memory_system->state().msx1_subslot_rules = 1;
    }

    startup.slot_bus = std::make_unique<memmap::SlotMemoryBus>(*startup.memory_system);

    if (startup.use_vdp || startup.use_ppi || startup.use_psg) {
        startup.composite_bus = std::make_unique<z80::CompositeBus>(*startup.slot_bus);
        if (startup.use_ppi) {
            // Portas A8h-ABh (PPI i8255) -- a porta A8h deixa de ir
            // direto para o SlotMemoryBus: o PPI decide quando o slot
            // primario muda (ver ppi_device.h e doc/ppi-spec.md).
            startup.ppi_device = std::make_unique<ppi::PpiDevice>(*startup.memory_system);
            startup.composite_bus->RegisterPortRange(0xA8, 0xAB, startup.ppi_device.get());
        } else {
            // Porta A8h (slot primario) -- mesmo dispositivo de memoria, ja
            // que SlotMemoryBus::out() ja trata essa porta.
            startup.composite_bus->RegisterPort(0xA8, startup.slot_bus.get());
        }
        if (startup.use_psg) {
            // Portas A0h-A2h (PSG AY-3-8910) -- ver doc/psg-spec.md.
            startup.psg_device = std::make_unique<psg::PsgDevice>();
            startup.composite_bus->RegisterPortRange(0xA0, 0xA2, startup.psg_device.get());
        }
        if (startup.use_vdp) {
            // Portas 98h-9Bh (VDP) -- ver doc/vdp-spec.md, secao 3.3/6.
            startup.vdp_device = std::make_unique<vdp::VdpDevice>();
            startup.composite_bus->RegisterPortRange(0x98, 0x9B, startup.vdp_device.get());
        }
    }

    return startup;
}

} // namespace z80::debug
