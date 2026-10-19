// fwMSX -- Mamute: execucao offline (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "mamute_exec.h"

#include <cstdio>

namespace mamute {

OfflineExec::OfflineExec(IMemory &mem) : bus_(mem), cpu_(bus_) { cpu_.reset(); }

std::string OfflineExec::Go(bool has_addr, uint16_t addr, const std::vector<uint16_t> &bps) {
    if (has_addr) cpu_.set_pc(addr);
    char buf[96];
    for (int n = 0; n < kMaxInstructions; ++n) {
        const uint16_t pc = cpu_.pc();
        if (n > 0) { // o breakpoint vale ANTES de executar a instrucao; quem parte de cima dele nao para de cara
            for (const uint16_t b : bps) {
                if (b == pc) {
                    std::snprintf(buf, sizeof(buf), "PARADO NO BREAKPOINT %04X (%d INSTRUCOES)", pc, n);
                    return buf;
                }
            }
        }
        if (cpu_.state().iff & Z80_IFF_HALT) {
            std::snprintf(buf, sizeof(buf), "PARADO EM HALT NO ENDERECO %04X (%d INSTRUCOES)", pc, n);
            return buf;
        }
        cpu_.run(1);
    }
    std::snprintf(buf, sizeof(buf), "LIMITE DE %d INSTRUCOES - PC=%04X", kMaxInstructions, cpu_.pc());
    return buf;
}

} // namespace mamute
