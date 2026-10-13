// fwMSX -- depurador: teste de faixa de endereco (Assembly, NASM dual-ABI Win64/SysV). Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). Ver dbg_range.asm e doc/debug-spec.md.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 1 se `addr` esta' em [base, base+len) com volta em 16 bits (a faixa pode atravessar FFFFh -> 0000h); 0 se
// len == 0. E' o teste que cada acesso a memoria faz contra cada watchpoint ativo.
int dbg_addr_in_range(uint16_t addr, uint16_t base, uint16_t len);

#ifdef __cplusplus
}
#endif
