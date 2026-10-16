// fwasm: sintese dos pulsos da fita MSX (Assembly, NASM dual-ABI). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver wav_synth.asm.
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// kinds[i] = 0 (ciclo curto, 2400 Hz) ou 1 (ciclo longo, 1200 Hz). Escreve as amostras de 8 bits (0/255) em `out` e devolve
// quantas. `out` precisa ter ate' (soma dos meios-periodos em 1/16 de amostra)/16 + 2 bytes.
size_t fwasm_wav_render(const uint8_t *kinds, size_t n, uint8_t *out);

#ifdef __cplusplus
}
#endif
