// fwMSX -- declaracao C da rotina em Assembly tape_sum_cycles() (ver
// pulse_sum.asm). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t tape_sum_cycles(const uint32_t *pulses, uint32_t count);

#ifdef __cplusplus
}
#endif
