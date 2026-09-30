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
    // de boot.
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] != "--slots") continue;
        startup.use_slots = true;
        if (i + 1 < args.size()) {
            startup.boot_rom_path = args[i + 1];
            startup.boot_rom_requested = true;
        }
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

    startup.slot_bus = std::make_unique<memmap::SlotMemoryBus>(*startup.memory_system);
    return startup;
}

} // namespace z80::debug
