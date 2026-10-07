// fwMSX -- declaracao C da funcao Fortran kcs_pulse_hz() (ver
// kcs_timing.f90). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

double kcs_pulse_hz(int64_t cpu_clock, int64_t pulse_t_states);

#ifdef __cplusplus
}
#endif
