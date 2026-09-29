// Barramento do Z80 do fwMSX -- design proprio (BSD-3-Clause), nao
// adaptado do fMSX. fMSX usa funcoes globais externas (RdZ80/WrZ80/
// InZ80/OutZ80/PatchZ80/JumpZ80) que o host define uma unica vez por
// processo; aqui usamos uma struct de ponteiros de funcao + contexto
// (Z80Bus), permitindo mais de uma instancia de Z80State/bus no mesmo
// processo (util para testes automatizados). Ver doc/z80-core-spec.md,
// secao 3.2.
#pragma once

#include <stdint.h>

#include "../common/z80_state.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Z80Bus {
    void *ctx;

    uint8_t (*read)(void *ctx, uint16_t addr);
    void (*write)(void *ctx, uint16_t addr, uint8_t value);
    uint8_t (*in)(void *ctx, uint16_t port);
    void (*out)(void *ctx, uint16_t port, uint8_t value);

    // Gancho do opcode especial "ED FE" (equivalente ao PatchZ80 do
    // fMSX), usado para interceptar chamadas de BIOS (ex.: disco/fita)
    // sem emular o hardware real. Pode ser NULL (sem patch nenhum).
    void (*patch)(void *ctx, Z80State *state);

    // Gancho opcional chamado em todo JP/JR/CALL/RST/RET (equivalente ao
    // JumpZ80 do fMSX), usado por um host que precise trocar mapeamento
    // de banco de memoria ao mudar de PC. Pode ser NULL.
    void (*jump)(void *ctx, uint16_t pc);
} Z80Bus;

#ifdef __cplusplus
}
#endif
