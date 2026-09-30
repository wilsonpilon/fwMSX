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

    // Gancho opcional (Fase 3, ver doc/z80-core-spec.md secao 3.4) usado
    // pelo despachante para acelerar LDIR/LDDR via Assembly
    // (src/z80/asm/block_ops.asm). Deve devolver um ponteiro direto para
    // `len` bytes de RAM do host que respaldem, de forma plana e sem
    // efeito colateral (sem I/O, sem cruzar fronteira de banco), os
    // enderecos Z80 [addr, addr+len), tanto para leitura quanto escrita;
    // ou NULL se nao existir esse mapeamento direto para o intervalo
    // pedido. O proprio ponteiro de funcao pode ser NULL, se o host nunca
    // suportar isso -- quem chama tem que checar as DUAS coisas (ponteiro
    // de funcao nao-nulo E retorno nao-nulo) antes de usar o caminho
    // rapido; a corretude do core nunca pode depender deste caminho ser
    // tomado, so o desempenho.
    uint8_t *(*ram_ptr)(void *ctx, uint16_t addr, uint16_t len);
} Z80Bus;

#ifdef __cplusplus
}
#endif
