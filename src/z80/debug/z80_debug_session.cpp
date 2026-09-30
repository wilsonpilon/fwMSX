#include "z80_debug_session.h"

#include <cstdio>
#include <fstream>
#include <sstream>

#include "../../memmap/cpp/memory_system.h"
#include "../../vdp/cpp/vdp_device.h"
#include "../common/z80_state.h"
#include "z80_disasm.h"

namespace z80::debug {

namespace {

// Numeros aceitos nos comandos: decimal ("4096"), hex com prefixo "0x"
// ("0x1000") ou hex com prefixo "$" ("$1000", convencao classica de
// assembler/monitor de 8 bits). Sem prefixo => decimal.
bool ParseNumber(const std::string &s, unsigned long &out) {
    if (s.empty()) return false;
    try {
        size_t consumed = 0;
        if (s.size() > 1 && s[0] == '$') {
            out = std::stoul(s.substr(1), &consumed, 16);
            return consumed == s.size() - 1;
        }
        out = std::stoul(s, &consumed, 0); // 0 => detecta "0x" sozinho
        return consumed == s.size();
    } catch (...) {
        return false;
    }
}

bool ParseAddr(const std::string &s, uint16_t &out) {
    unsigned long v = 0;
    if (!ParseNumber(s, v) || v > 0xFFFF) return false;
    out = static_cast<uint16_t>(v);
    return true;
}

bool ParseByte(const std::string &s, uint8_t &out) {
    unsigned long v = 0;
    if (!ParseNumber(s, v) || v > 0xFF) return false;
    out = static_cast<uint8_t>(v);
    return true;
}

bool ParseU32(const std::string &s, uint32_t &out) {
    unsigned long v = 0;
    if (!ParseNumber(s, v)) return false;
    out = static_cast<uint32_t>(v);
    return true;
}

// Indice de slot (primario ou secundario): sempre 0..3 -- ver
// doc/memory-map-spec.md, secao 2.
bool ParseSlotIndex(const std::string &s, int &out) {
    unsigned long v = 0;
    if (!ParseNumber(s, v) || v > 3) return false;
    out = static_cast<int>(v);
    return true;
}

// Nomes aceitos no 4o argumento opcional de 'loadrom' -- Fase 3 do mapa
// de memoria (bank-switch), ver doc/memory-map-spec.md, secao 6. Sem
// esse argumento, 'loadrom' continua carregando ROM plana (comportamento
// da Fase 2, inalterado).
bool ParseMapperType(const std::string &s, MemMapMapperType &out) {
    if (s == "gen8") { out = MEMMAP_MAPPER_GEN8; return true; }
    if (s == "gen16") { out = MEMMAP_MAPPER_GEN16; return true; }
    if (s == "konami5") { out = MEMMAP_MAPPER_KONAMI5; return true; }
    if (s == "konami4") { out = MEMMAP_MAPPER_KONAMI4; return true; }
    if (s == "ascii8") { out = MEMMAP_MAPPER_ASCII8; return true; }
    if (s == "ascii16") { out = MEMMAP_MAPPER_ASCII16; return true; }
    return false;
}

std::string Hex4(uint16_t v) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%04X", v);
    return buf;
}

std::string Hex2(uint8_t v) {
    char buf[4];
    std::snprintf(buf, sizeof(buf), "%02X", v);
    return buf;
}

} // namespace

Z80DebugSession::Z80DebugSession(z80::IBus &bus, memmap::MemorySystem *memory_system, vdp::VdpDevice *vdp_device)
    : bus_(bus), cpu_(bus_), memory_system_(memory_system), vdp_device_(vdp_device) {}

void Z80DebugSession::DriveVdp(int cycles_consumed) {
    if (!vdp_device_) return;
    vdp_pending_cycles_ -= cycles_consumed;
    while (vdp_pending_cycles_ <= 0) {
        const VdpStepResult r = vdp_device_->Step();
        vdp_pending_cycles_ += r.next_period_cycles;
        if (r.irq_pending) cpu_.interrupt(Z80_INT_IRQ);
    }
}

std::string Z80DebugSession::ProcessCommand(const std::vector<std::string> &tokens) {
    if (tokens.empty()) return "";
    const std::string &cmd = tokens[0];

    if (cmd == "reset") return CmdReset();
    if (cmd == "regs") return CmdRegs();
    if (cmd == "step") return CmdStep(tokens);
    if (cmd == "run") return CmdRun(tokens);
    if (cmd == "break") return CmdBreak(tokens);
    if (cmd == "clear") return CmdClearBreak(tokens);
    if (cmd == "breaks") return CmdListBreaks();
    if (cmd == "mem") return CmdMem(tokens);
    if (cmd == "peek") return CmdPeek(tokens);
    if (cmd == "poke") return CmdPoke(tokens);
    if (cmd == "load") return CmdLoad(tokens);
    if (cmd == "fill") return CmdFill(tokens);
    if (cmd == "disasm") return CmdDisasm(tokens);
    if (cmd == "slots") return CmdSlots();
    if (cmd == "pages") return CmdPages();
    if (cmd == "slotmem") return CmdSlotMem(tokens);
    if (cmd == "slotpeek") return CmdSlotPeek(tokens);
    if (cmd == "slotpoke") return CmdSlotPoke(tokens);
    if (cmd == "loadrom") return CmdLoadRom(tokens);
    if (cmd == "vdpregs") return CmdVdpRegs();
    if (cmd == "vdpmem") return CmdVdpMem(tokens);
    if (cmd == "vdppeek") return CmdVdpPeek(tokens);
    if (cmd == "vdppoke") return CmdVdpPoke(tokens);
    if (cmd == "vdpstep") return CmdVdpStep(tokens);
    if (cmd == "help" || cmd == "?") return CmdHelp();

    return "comando desconhecido: '" + cmd + "' (digite 'help' para a lista)";
}

std::string Z80DebugSession::CmdReset() {
    cpu_.reset();
    return "CPU resetada (PC=0000).";
}

std::string Z80DebugSession::CmdRegs() const {
    std::ostringstream out;
    out << "PC=" << Hex4(cpu_.pc()) << " SP=" << Hex4(cpu_.sp()) << " AF=" << Hex4(cpu_.af())
        << " BC=" << Hex4(cpu_.bc()) << " DE=" << Hex4(cpu_.de()) << " HL=" << Hex4(cpu_.hl())
        << " IX=" << Hex4(cpu_.ix()) << " IY=" << Hex4(cpu_.iy()) << "\n";

    const uint8_t f = cpu_.af() & 0xFF;
    out << "Flags: S=" << ((f & Z80_S_FLAG) ? 1 : 0) << " Z=" << ((f & Z80_Z_FLAG) ? 1 : 0)
        << " H=" << ((f & Z80_H_FLAG) ? 1 : 0) << " P/V=" << ((f & Z80_P_FLAG) ? 1 : 0)
        << " N=" << ((f & Z80_N_FLAG) ? 1 : 0) << " C=" << ((f & Z80_C_FLAG) ? 1 : 0) << "\n";

    const uint8_t iff = cpu_.iff();
    int im = 0;
    if (iff & Z80_IFF_IM2) im = 2;
    else if (iff & Z80_IFF_IM1) im = 1;
    out << "IFF1=" << ((iff & Z80_IFF_1) ? 1 : 0) << " IFF2=" << ((iff & Z80_IFF_2) ? 1 : 0)
        << " IM=" << im << " HALT=" << ((iff & Z80_IFF_HALT) ? 1 : 0) << "\n";
    out << "I=" << Hex2(cpu_.state().i) << " R=" << Hex2(cpu_.state().r);
    return out.str();
}

std::string Z80DebugSession::CmdStep(const std::vector<std::string> &tokens) {
    uint32_t n = 1;
    if (tokens.size() > 1 && !ParseU32(tokens[1], n)) return "step: numero de passos invalido: '" + tokens[1] + "'";
    if (n == 0) return "step: nada a fazer (n=0)";

    std::ostringstream out;
    for (uint32_t i = 0; i < n; ++i) {
        const int leftover = cpu_.run(1);
        DriveVdp(1 - leftover);
        out << "step " << (i + 1) << ": PC=" << Hex4(cpu_.pc());
        if (breakpoints_.count(cpu_.pc())) {
            out << "  <-- breakpoint";
            return out.str();
        }
        if (i + 1 < n) out << "\n";
    }
    return out.str();
}

std::string Z80DebugSession::CmdRun(const std::vector<std::string> &tokens) {
    uint32_t budget = 1000;
    if (tokens.size() > 1 && !ParseU32(tokens[1], budget)) return "run: orcamento de ciclos invalido: '" + tokens[1] + "'";

    uint32_t consumed = 0;
    const char *reason = "orcamento de ciclos esgotado";
    while (consumed < budget) {
        const int leftover = cpu_.run(1);
        const int step_cycles = 1 - leftover;
        consumed += static_cast<uint32_t>(step_cycles);
        DriveVdp(step_cycles);
        if (breakpoints_.count(cpu_.pc())) {
            reason = "breakpoint atingido";
            break;
        }
    }

    std::ostringstream out;
    out << "parado: " << reason << " (ciclos consumidos: " << consumed << ", PC=" << Hex4(cpu_.pc()) << ")";
    return out.str();
}

std::string Z80DebugSession::CmdBreak(const std::vector<std::string> &tokens) {
    if (tokens.size() < 2) return "uso: break <endereco>";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[1], addr)) return "break: endereco invalido: '" + tokens[1] + "'";
    breakpoints_.insert(addr);
    return "breakpoint em " + Hex4(addr);
}

std::string Z80DebugSession::CmdClearBreak(const std::vector<std::string> &tokens) {
    if (tokens.size() < 2) return "uso: clear <endereco>";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[1], addr)) return "clear: endereco invalido: '" + tokens[1] + "'";
    const size_t removed = breakpoints_.erase(addr);
    return removed ? ("breakpoint removido: " + Hex4(addr)) : ("nao havia breakpoint em " + Hex4(addr));
}

std::string Z80DebugSession::CmdListBreaks() const {
    if (breakpoints_.empty()) return "nenhum breakpoint definido.";
    std::ostringstream out;
    out << "breakpoints:";
    for (uint16_t addr : breakpoints_) out << " " << Hex4(addr);
    return out.str();
}

std::string Z80DebugSession::CmdMem(const std::vector<std::string> &tokens) const {
    if (tokens.size() < 2) return "uso: mem <endereco> [tamanho]";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[1], addr)) return "mem: endereco invalido: '" + tokens[1] + "'";
    uint32_t len = 16;
    if (tokens.size() > 2 && !ParseU32(tokens[2], len)) return "mem: tamanho invalido: '" + tokens[2] + "'";

    std::ostringstream out;
    for (uint32_t i = 0; i < len; ++i) {
        const uint16_t a = static_cast<uint16_t>(addr + i);
        if (i % 16 == 0) {
            if (i != 0) out << "\n";
            out << Hex4(a) << ": ";
        }
        out << Hex2(bus_.read(a)) << " ";
        if (a == 0xFFFF && i + 1 < len) break; // nao envolve alem de 0xFFFF
    }
    return out.str();
}

std::string Z80DebugSession::CmdPeek(const std::vector<std::string> &tokens) const {
    if (tokens.size() < 2) return "uso: peek <endereco>";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[1], addr)) return "peek: endereco invalido: '" + tokens[1] + "'";
    return Hex4(addr) + ": " + Hex2(bus_.read(addr));
}

std::string Z80DebugSession::CmdPoke(const std::vector<std::string> &tokens) {
    if (tokens.size() < 3) return "uso: poke <endereco> <byte>";
    uint16_t addr = 0;
    uint8_t value = 0;
    if (!ParseAddr(tokens[1], addr)) return "poke: endereco invalido: '" + tokens[1] + "'";
    if (!ParseByte(tokens[2], value)) return "poke: byte invalido: '" + tokens[2] + "'";
    bus_.write(addr, value);
    // Le de volta em vez de so ecoar `value`: numa sessao com mapa de
    // memoria real (--slots), a pagina atual pode nao ser gravavel (ex.:
    // ROM, ou um slot vazio) e a escrita e' descartada silenciosamente
    // (mesma regra do memmap_write) -- ler de volta revela isso em vez de
    // reportar um sucesso que nao aconteceu. Achado testando --z80dbg
    // --slots na mao (ver doc/memory-map-spec.md, Fase 1).
    //
    // Fase 3 (MegaROM): o valor tambem pode diferir do escrito quando a
    // pagina e' um mapper com bank-switch -- nesse caso a escrita foi
    // INTERPRETADA como comando de troca de banco (nao descartada!), e o
    // byte lido de volta e' o conteudo do banco recem-selecionado, nao o
    // numero de banco em si. A mensagem generica abaixo nao distingue os
    // dois casos (IBus::write() nao devolve essa informacao) -- redigida
    // de proposito pra nao afirmar "nao gravado" quando pode muito bem ter
    // sido um comando de mapper com efeito. Achado testando --z80dbg
    // --slots na mao apos a Fase 3 (ver doc/memory-map-spec.md).
    const uint8_t actual = bus_.read(addr);
    if (actual == value) return Hex4(addr) + " <- " + Hex2(value);
    return Hex4(addr) + " <- " + Hex2(value) + " (byte na pagina agora: " + Hex2(actual) +
           " -- ou a escrita foi descartada por pagina nao-gravavel, ou foi interpretada como comando "
           "de mapper/bank-switch; use 'slots'/'pages' pra checar)";
}

std::string Z80DebugSession::CmdLoad(const std::vector<std::string> &tokens) {
    if (tokens.size() < 3) return "uso: load <arquivo> <endereco>";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[2], addr)) return "load: endereco invalido: '" + tokens[2] + "'";

    std::ifstream file(tokens[1], std::ios::binary | std::ios::ate);
    if (!file) return "load: nao foi possivel abrir '" + tokens[1] + "'";

    const std::streamsize size = file.tellg();
    if (size < 0) return "load: falha ao ler tamanho de '" + tokens[1] + "'";
    if (static_cast<uint32_t>(addr) + static_cast<uint32_t>(size) > 0x10000u) {
        return "load: arquivo (" + std::to_string(size) + " bytes) nao cabe a partir de " + Hex4(addr) +
               " sem passar de FFFF";
    }

    file.seekg(0, std::ios::beg);
    for (std::streamsize i = 0; i < size; ++i) {
        char byte = 0;
        if (!file.get(byte)) return "load: erro de leitura em '" + tokens[1] + "' apos " + std::to_string(i) + " bytes";
        bus_.write(static_cast<uint16_t>(addr + i), static_cast<uint8_t>(byte));
    }
    return "carregado: " + std::to_string(size) + " byte(s) em " + Hex4(addr);
}

std::string Z80DebugSession::CmdFill(const std::vector<std::string> &tokens) {
    if (tokens.size() < 4) return "uso: fill <endereco> <tamanho> <byte>";
    uint16_t addr = 0;
    uint32_t len = 0;
    uint8_t value = 0;
    if (!ParseAddr(tokens[1], addr)) return "fill: endereco invalido: '" + tokens[1] + "'";
    if (!ParseU32(tokens[2], len)) return "fill: tamanho invalido: '" + tokens[2] + "'";
    if (!ParseByte(tokens[3], value)) return "fill: byte invalido: '" + tokens[3] + "'";
    if (static_cast<uint32_t>(addr) + len > 0x10000u) return "fill: intervalo passa de FFFF";

    for (uint32_t i = 0; i < len; ++i) bus_.write(static_cast<uint16_t>(addr + i), value);
    return "preenchido: " + std::to_string(len) + " byte(s) a partir de " + Hex4(addr) + " com " + Hex2(value);
}

std::string Z80DebugSession::CmdDisasm(const std::vector<std::string> &tokens) const {
    uint16_t addr = cpu_.pc();
    if (tokens.size() > 1 && !ParseAddr(tokens[1], addr)) return "disasm: endereco invalido: '" + tokens[1] + "'";
    uint32_t n = 10;
    if (tokens.size() > 2 && !ParseU32(tokens[2], n)) return "disasm: quantidade invalida: '" + tokens[2] + "'";

    const auto read = [this](uint16_t a) { return bus_.read(a); };

    std::ostringstream out;
    for (uint32_t i = 0; i < n; ++i) {
        const DisasmResult r = Disassemble(read, addr);
        out << Hex4(addr) << ": ";
        for (uint16_t k = 0; k < r.length; ++k) out << Hex2(bus_.read(static_cast<uint16_t>(addr + k))) << " ";
        for (uint16_t k = r.length; k < 4; ++k) out << "   ";
        out << " " << r.text;
        if (i + 1 < n) out << "\n";
        addr = static_cast<uint16_t>(addr + r.length);
    }
    return out.str();
}

std::string Z80DebugSession::CmdSlots() const {
    if (!memory_system_) return "slots: requer 'fwmsx --z80dbg --slots' (esta sessao nao tem mapa de memoria)";
    std::ostringstream out;
    out << "Slots (primario:secundario -> conteudo):";
    for (int primary = 0; primary < 4; ++primary) {
        for (int secondary = 0; secondary < 4; ++secondary) {
            const memmap::SlotDescriptor d = memory_system_->Describe(primary, secondary);
            out << "\n  " << primary << ":" << secondary << " -> ";
            switch (d.kind) {
                case MEMMAP_KIND_RAM:
                    out << "RAM (" << d.size << " bytes)";
                    break;
                case MEMMAP_KIND_ROM: {
                    char crc_buf[16];
                    std::snprintf(crc_buf, sizeof(crc_buf), "%08X", d.crc32);
                    out << "ROM (" << d.size << " bytes";
                    if (d.mapper_name[0] != '\0') out << ", " << d.mapper_name;
                    out << ", CRC32 " << crc_buf << ")";
                    break;
                }
                case MEMMAP_KIND_EMPTY:
                default:
                    out << "vazio";
                    break;
            }
        }
    }
    return out.str();
}

std::string Z80DebugSession::CmdPages() const {
    if (!memory_system_) return "pages: requer 'fwmsx --z80dbg --slots' (esta sessao nao tem mapa de memoria)";
    static const char *const kRanges[4] = {"0000-3FFF", "4000-7FFF", "8000-BFFF", "C000-FFFF"};
    const auto view = memory_system_->CurrentView();
    std::ostringstream out;
    out << "Paginas visiveis para a CPU agora:";
    for (int page = 0; page < 4; ++page) {
        out << "\n  " << kRanges[page] << ": slot " << view[page].primary << ":" << view[page].secondary
            << (view[page].writable ? " (gravavel)" : " (somente leitura)");
    }
    return out.str();
}

std::string Z80DebugSession::CmdSlotMem(const std::vector<std::string> &tokens) const {
    if (!memory_system_) return "slotmem: requer 'fwmsx --z80dbg --slots' (esta sessao nao tem mapa de memoria)";
    if (tokens.size() < 4) return "uso: slotmem <primario> <secundario> <endereco> [tamanho]";
    int primary = 0, secondary = 0;
    if (!ParseSlotIndex(tokens[1], primary)) return "slotmem: slot primario invalido: '" + tokens[1] + "'";
    if (!ParseSlotIndex(tokens[2], secondary)) return "slotmem: slot secundario invalido: '" + tokens[2] + "'";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[3], addr)) return "slotmem: endereco invalido: '" + tokens[3] + "'";
    uint32_t len = 16;
    if (tokens.size() > 4 && !ParseU32(tokens[4], len)) return "slotmem: tamanho invalido: '" + tokens[4] + "'";

    std::ostringstream out;
    for (uint32_t i = 0; i < len; ++i) {
        const uint16_t a = static_cast<uint16_t>(addr + i);
        if (i % 16 == 0) {
            if (i != 0) out << "\n";
            out << primary << ":" << secondary << " " << Hex4(a) << ": ";
        }
        out << Hex2(memory_system_->PeekSlot(primary, secondary, a)) << " ";
        if (a == 0xFFFF && i + 1 < len) break; // nao envolve alem de 0xFFFF
    }
    return out.str();
}

std::string Z80DebugSession::CmdSlotPeek(const std::vector<std::string> &tokens) const {
    if (!memory_system_) return "slotpeek: requer 'fwmsx --z80dbg --slots' (esta sessao nao tem mapa de memoria)";
    if (tokens.size() < 4) return "uso: slotpeek <primario> <secundario> <endereco>";
    int primary = 0, secondary = 0;
    if (!ParseSlotIndex(tokens[1], primary)) return "slotpeek: slot primario invalido: '" + tokens[1] + "'";
    if (!ParseSlotIndex(tokens[2], secondary)) return "slotpeek: slot secundario invalido: '" + tokens[2] + "'";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[3], addr)) return "slotpeek: endereco invalido: '" + tokens[3] + "'";
    return std::to_string(primary) + ":" + std::to_string(secondary) + " " + Hex4(addr) + ": " +
           Hex2(memory_system_->PeekSlot(primary, secondary, addr));
}

std::string Z80DebugSession::CmdSlotPoke(const std::vector<std::string> &tokens) {
    if (!memory_system_) return "slotpoke: requer 'fwmsx --z80dbg --slots' (esta sessao nao tem mapa de memoria)";
    if (tokens.size() < 5) return "uso: slotpoke <primario> <secundario> <endereco> <byte>";
    int primary = 0, secondary = 0;
    if (!ParseSlotIndex(tokens[1], primary)) return "slotpoke: slot primario invalido: '" + tokens[1] + "'";
    if (!ParseSlotIndex(tokens[2], secondary)) return "slotpoke: slot secundario invalido: '" + tokens[2] + "'";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[3], addr)) return "slotpoke: endereco invalido: '" + tokens[3] + "'";
    uint8_t value = 0;
    if (!ParseByte(tokens[4], value)) return "slotpoke: byte invalido: '" + tokens[4] + "'";
    memory_system_->PokeSlot(primary, secondary, addr, value);
    // Mesma logica de CmdPoke: le de volta em vez de so ecoar `value`,
    // porque um slot vazio ou nao-gravavel descarta a escrita em
    // silencio (memmap_poke_slot) -- ver o comentario em CmdPoke.
    const uint8_t actual = memory_system_->PeekSlot(primary, secondary, addr);
    const std::string prefix = std::to_string(primary) + ":" + std::to_string(secondary) + " " + Hex4(addr);
    if (actual == value) return prefix + " <- " + Hex2(value);
    return prefix + " <- " + Hex2(value) + " (nao gravado -- slot nao gravavel; continua " + Hex2(actual) + ")";
}

std::string Z80DebugSession::CmdLoadRom(const std::vector<std::string> &tokens) {
    if (!memory_system_) return "loadrom: requer 'fwmsx --z80dbg --slots' (esta sessao nao tem mapa de memoria)";
    if (tokens.size() < 4) return "uso: loadrom <primario> <secundario> <arquivo> [mapper]";
    int primary = 0, secondary = 0;
    if (!ParseSlotIndex(tokens[1], primary)) return "loadrom: slot primario invalido: '" + tokens[1] + "'";
    if (!ParseSlotIndex(tokens[2], secondary)) return "loadrom: slot secundario invalido: '" + tokens[2] + "'";

    // Argumento opcional de mapper (Fase 3, bank-switch) -- sem ele,
    // carrega ROM plana (comportamento da Fase 2, inalterado).
    MemMapMapperType mapper = MEMMAP_MAPPER_NONE;
    if (tokens.size() > 4 && !ParseMapperType(tokens[4], mapper)) {
        return "loadrom: mapper desconhecido: '" + tokens[4] +
               "' (use gen8, gen16, konami5, konami4, ascii8 ou ascii16)";
    }

    std::ifstream file(tokens[3], std::ios::binary | std::ios::ate);
    if (!file) return "loadrom: nao foi possivel abrir '" + tokens[3] + "'";
    const std::streamsize size = file.tellg();
    if (size < 0) return "loadrom: falha ao ler tamanho de '" + tokens[3] + "'";
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(static_cast<size_t>(size));
    if (size > 0 && !file.read(reinterpret_cast<char *>(data.data()), size)) {
        return "loadrom: erro de leitura em '" + tokens[3] + "'";
    }

    std::string error;
    if (!memory_system_->LoadRom(primary, secondary, data.data(), data.size(), &error, mapper)) {
        return "loadrom: " + error;
    }
    const memmap::SlotDescriptor desc = memory_system_->Describe(primary, secondary);
    char crc_buf[16];
    std::snprintf(crc_buf, sizeof(crc_buf), "%08X", desc.crc32);
    std::string result = "carregado em " + std::to_string(primary) + ":" + std::to_string(secondary) + ": " +
                          std::to_string(data.size()) + " byte(s), CRC32 " + crc_buf;
    if (mapper != MEMMAP_MAPPER_NONE) result += std::string(", mapper ") + desc.mapper_name;
    return result;
}

std::string Z80DebugSession::CmdVdpRegs() const {
    if (!vdp_device_) return "vdpregs: requer 'fwmsx --z80dbg --slots --vdp' (esta sessao nao tem VDP)";
    const VdpState &v = vdp_device_->state();
    std::ostringstream out;
    out << "Registradores (R#0-R#46):\n";
    for (int r = 0; r <= 46; ++r) {
        out << Hex2(v.regs[r]) << (r == 46 ? "" : " ");
        if (r % 16 == 15) out << "\n";
    }
    out << "\nStatus (S#0-S#9): ";
    for (int s = 0; s <= 9; ++s) out << Hex2(v.status[s]) << " ";
    out << "\nModo de tela (ScrMode): " << static_cast<int>(v.scr_mode)
        << "  IE0 (VBlank) habilitado: " << ((v.regs[1] & 0x20) ? 1 : 0)
        << "  IE1 (HBlank/coincidencia) habilitado: " << ((v.regs[0] & 0x10) ? 1 : 0)
        << "\nInterrupcao pendente (VDP): " << (v.irq_pending ? "sim" : "nao") << " (bits " << Hex2(v.irq_pending)
        << ")\nScanLine=" << v.scanline << " Drawing=" << v.drawing;
    return out.str();
}

std::string Z80DebugSession::CmdVdpMem(const std::vector<std::string> &tokens) const {
    if (!vdp_device_) return "vdpmem: requer 'fwmsx --z80dbg --slots --vdp' (esta sessao nao tem VDP)";
    if (tokens.size() < 2) return "uso: vdpmem <endereco> [tamanho]";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[1], addr)) return "vdpmem: endereco invalido: '" + tokens[1] + "'";
    uint32_t len = 16;
    if (tokens.size() > 2 && !ParseU32(tokens[2], len)) return "vdpmem: tamanho invalido: '" + tokens[2] + "'";

    const VdpState &v = vdp_device_->state();
    std::ostringstream out;
    for (uint32_t i = 0; i < len && (addr + i) < VDP_VRAM_SIZE; ++i) {
        const uint16_t a = static_cast<uint16_t>(addr + i);
        if (i % 16 == 0) {
            if (i != 0) out << "\n";
            out << Hex4(a) << ": ";
        }
        out << Hex2(v.vram[a]) << " ";
    }
    return out.str();
}

std::string Z80DebugSession::CmdVdpPeek(const std::vector<std::string> &tokens) const {
    if (!vdp_device_) return "vdppeek: requer 'fwmsx --z80dbg --slots --vdp' (esta sessao nao tem VDP)";
    if (tokens.size() < 2) return "uso: vdppeek <endereco>";
    uint16_t addr = 0;
    if (!ParseAddr(tokens[1], addr)) return "vdppeek: endereco invalido: '" + tokens[1] + "'";
    if (addr >= VDP_VRAM_SIZE) return "vdppeek: endereco alem da VRAM (" + std::to_string(VDP_VRAM_SIZE) + " bytes)";
    return Hex4(addr) + ": " + Hex2(vdp_device_->state().vram[addr]);
}

std::string Z80DebugSession::CmdVdpPoke(const std::vector<std::string> &tokens) {
    if (!vdp_device_) return "vdppoke: requer 'fwmsx --z80dbg --slots --vdp' (esta sessao nao tem VDP)";
    if (tokens.size() < 3) return "uso: vdppoke <endereco> <byte>";
    uint16_t addr = 0;
    uint8_t value = 0;
    if (!ParseAddr(tokens[1], addr)) return "vdppoke: endereco invalido: '" + tokens[1] + "'";
    if (!ParseByte(tokens[2], value)) return "vdppoke: byte invalido: '" + tokens[2] + "'";
    if (addr >= VDP_VRAM_SIZE) return "vdppoke: endereco alem da VRAM (" + std::to_string(VDP_VRAM_SIZE) + " bytes)";
    vdp_device_->state().vram[addr] = value;
    return Hex4(addr) + " <- " + Hex2(value);
}

std::string Z80DebugSession::CmdVdpStep(const std::vector<std::string> &tokens) {
    if (!vdp_device_) return "vdpstep: requer 'fwmsx --z80dbg --slots --vdp' (esta sessao nao tem VDP)";
    uint32_t n = 1;
    if (tokens.size() > 1 && !ParseU32(tokens[1], n)) return "vdpstep: numero de passos invalido: '" + tokens[1] + "'";
    if (n == 0) return "vdpstep: nada a fazer (n=0)";

    std::ostringstream out;
    for (uint32_t i = 0; i < n; ++i) {
        const VdpStepResult r = vdp_device_->Step();
        out << "vdpstep " << (i + 1) << ": ScanLine=" << vdp_device_->state().scanline
            << " proximo periodo=" << r.next_period_cycles << " irq_pending=" << (r.irq_pending ? "sim" : "nao");
        if (i + 1 < n) out << "\n";
    }
    return out.str();
}

std::string Z80DebugSession::CmdHelp() const {
    return "Comandos (enderecos/numeros: decimal, 0x-hex ou $-hex):\n"
           "  reset                 reseta a CPU\n"
           "  regs                  mostra registradores/flags/IFF\n"
           "  step [n]              executa n instrucoes (default 1), uma por vez\n"
           "  run [ciclos]          executa ate esgotar o orcamento de ciclos (default 1000)\n"
           "                        ou ate atingir um breakpoint\n"
           "  break <end>           define um breakpoint em <end>\n"
           "  clear <end>           remove o breakpoint em <end>\n"
           "  breaks                lista os breakpoints ativos\n"
           "  mem <end> [tam]       dump hexadecimal (default 16 bytes)\n"
           "  peek <end>            le um byte\n"
           "  poke <end> <byte>     escreve um byte\n"
           "  load <arquivo> <end>  carrega um arquivo binario na RAM em <end>\n"
           "  fill <end> <tam> <b>  preenche <tam> bytes com <b> a partir de <end>\n"
           "  disasm [end] [n]      desmonta n instrucoes (default: PC atual, 10)\n"
           "  slots                 lista as 16 combinacoes de slot (requer --slots)\n"
           "  pages                 mostra o que a CPU enxerga agora em cada pagina (requer --slots)\n"
           "  slotmem <p> <s> <end> [tam]    dump de uma combinacao especifica (requer --slots)\n"
           "  slotpeek <p> <s> <end>         le um byte de uma combinacao especifica (requer --slots)\n"
           "  slotpoke <p> <s> <end> <byte>  escreve numa combinacao especifica (requer --slots)\n"
           "  loadrom <p> <s> <arquivo> [mapper]  carrega uma ROM na combinacao (requer --slots)\n"
           "      sem [mapper]: ROM plana (8..64KB, sem bank-switch)\n"
           "      com [mapper]: MegaROM com bank-switch (ate 2MB) -- gen8, gen16,\n"
           "      konami5, konami4, ascii8 ou ascii16 (so a troca de banco de ROM;\n"
           "      SCC/SRAM nao suportados nesses dois ultimos, ver doc/memory-map-spec.md)\n"
           "  vdpregs               registradores/status/modo de tela do VDP (requer --vdp)\n"
           "  vdpmem <end> [tam]    dump da VRAM do VDP (requer --vdp)\n"
           "  vdppeek <end>         le um byte da VRAM do VDP (requer --vdp)\n"
           "  vdppoke <end> <byte>  escreve um byte na VRAM do VDP (requer --vdp)\n"
           "  vdpstep [n]           avanca a maquina de estados do VDP manualmente, sem "
           "rodar o Z80 (requer --vdp)\n"
           "  help                  esta mensagem";
}

} // namespace z80::debug
