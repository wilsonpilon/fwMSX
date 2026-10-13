// fwMSX -- depurador: classificacao do fluxo de uma instrucao Z80. Ver dbg_flow.h.
#include "dbg_flow.h"

int dbg_flow_kind(const uint8_t *bytes) {
    const uint8_t op = bytes[0];
    if (op == 0xCD) return DBG_FLOW_CALL;           // CALL nn
    if ((op & 0xC7) == 0xC4) return DBG_FLOW_CALL;  // CALL cc,nn (C4 CC D4 DC E4 EC F4 FC)
    if ((op & 0xC7) == 0xC7) return DBG_FLOW_CALL;  // RST n (C7 CF D7 DF E7 EF F7 FF)
    if (op == 0x76) return DBG_FLOW_HALT;
    if (op == 0xED) {
        // ED B0..B3 (LDIR CPIR INIR OTIR) e ED B8..BB (LDDR CPDR INDR OTDR)
        if ((bytes[1] & 0xF4) == 0xB0) return DBG_FLOW_BLOCK;
    }
    return DBG_FLOW_PLAIN;
}
