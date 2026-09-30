// Adaptado de fMSX (resource/fMSX/fMSX/MSX.c), Copyright (C) Marat
// Fayzullin 1994-2021 -- ver slot_state.h para a nota de atribuicao
// completa.
#include "slot_state.h"

#include <string.h>

// Pedaco de 8KB compartilhado por toda combinacao de slot ainda vazia --
// mesma ideia do EmptyRAM do fMSX (um unico bloco "somente leitura" para
// todo slot nao populado, em vez de alocar 8KB reais so pra devolver
// 0xFF). Diferente do fMSX, a protecao contra escrita aqui vem de
// chunk_writable=0 (checado explicitamente em memmap_write/poke_slot),
// nao de comparar o ponteiro contra um sentinela -- ver a nota de
// "diferencas deliberadas" no topo de slot_state.h.
static uint8_t g_empty_chunk[MEMMAP_CHUNK_SIZE];

static int ValidSlot(int primary, int secondary) {
    return primary >= 0 && primary < MEMMAP_PRIMARY_SLOTS && secondary >= 0 && secondary < MEMMAP_SECONDARY_SLOTS;
}

void memmap_init(SlotState *state) {
    memset(g_empty_chunk, MEMMAP_EMPTY_BYTE, sizeof(g_empty_chunk));
    memset(state, 0, sizeof(*state));

    for (int primary = 0; primary < MEMMAP_PRIMARY_SLOTS; ++primary) {
        for (int secondary = 0; secondary < MEMMAP_SECONDARY_SLOTS; ++secondary) {
            state->slot_kind[primary][secondary] = MEMMAP_KIND_EMPTY;
            state->slot_size[primary][secondary] = 0;
            for (int chunk_idx = 0; chunk_idx < MEMMAP_CHUNKS; ++chunk_idx) {
                state->chunk[primary][secondary][chunk_idx] = g_empty_chunk;
                state->chunk_writable[primary][secondary][chunk_idx] = 0;
            }
        }
    }

    // psl/ssl/ssl_reg/psl_reg ja' zerados pelo memset acima -- vista
    // ativa inicial = combinacao 0:0 (tudo vazio ainda, ate' algum
    // AllocateRam()/LoadRom() popular alguma coisa).
    for (int chunk_idx = 0; chunk_idx < MEMMAP_CHUNKS; ++chunk_idx) {
        state->active_view[chunk_idx] = state->chunk[0][0][chunk_idx];
        state->active_writable[chunk_idx] = state->chunk_writable[0][0][chunk_idx];
    }
}

void memmap_attach(SlotState *state, int primary, int secondary, uint8_t *data, size_t size, MemMapKind kind,
                    int writable) {
    if (!ValidSlot(primary, secondary)) return;

    const size_t chunk_count = size / MEMMAP_CHUNK_SIZE;
    for (size_t chunk_idx = 0; chunk_idx < (size_t)MEMMAP_CHUNKS; ++chunk_idx) {
        if (chunk_idx < chunk_count) {
            state->chunk[primary][secondary][chunk_idx] = data + chunk_idx * MEMMAP_CHUNK_SIZE;
            state->chunk_writable[primary][secondary][chunk_idx] = (uint8_t)(writable ? 1 : 0);
        } else {
            state->chunk[primary][secondary][chunk_idx] = g_empty_chunk;
            state->chunk_writable[primary][secondary][chunk_idx] = 0;
        }
    }
    state->slot_kind[primary][secondary] = kind;
    state->slot_size[primary][secondary] = chunk_count * MEMMAP_CHUNK_SIZE;

    // Se essa combinacao estiver na vista ativa agora (alguma pagina com
    // psl[pagina]==primary && ssl[pagina]==secondary), a vista precisa
    // ser recomputada -- caso contrario um AllocateRam() feito depois de
    // uma troca de slot para essa combinacao ficaria invisivel para a
    // CPU ate a proxima troca. Refazemos a partir do psl_reg atual (o
    // proprio caminho de memmap_switch_primary), que e' o jeito mais
    // simples de garantir consistencia sem duplicar a logica de
    // recomputo de vista aqui.
    memmap_switch_primary(state, state->psl_reg);
}

void memmap_switch_primary(SlotState *state, uint8_t value) {
    // Nota: nao ha' um "if (psl_reg == value) return;" aqui como no
    // PSlot() do fMSX -- memmap_attach() precisa poder forcar um
    // recomputo da vista ativa mesmo quando o valor do registrador nao
    // mudou (ver comentario acima). O fMSX nunca precisa disso porque
    // carregamento de ROM/RAM la' acontece antes de qualquer troca de
    // slot, no reset da maquina.
    state->psl_reg = value;
    for (int page = 0; page < MEMMAP_PAGES; ++page) {
        const int bit_shift = page * 2;
        const int chunk_idx = page * 2;
        const int ps = (value >> bit_shift) & 3;
        state->psl[page] = (uint8_t)ps;
        const int ss = (state->ssl_reg[ps] >> bit_shift) & 3;
        state->ssl[page] = (uint8_t)ss;
        state->active_view[chunk_idx] = state->chunk[ps][ss][chunk_idx];
        state->active_view[chunk_idx + 1] = state->chunk[ps][ss][chunk_idx + 1];
        state->active_writable[chunk_idx] = state->chunk_writable[ps][ss][chunk_idx];
        state->active_writable[chunk_idx + 1] = state->chunk_writable[ps][ss][chunk_idx + 1];
    }
}

void memmap_switch_secondary(SlotState *state, uint8_t value) {
    // Fase 1: nao existe ainda o conceito de "slot sem subslot" (cartucho
    // ou slot 0 em MSX1 -- o fMSX forca value=0 nesses casos em SSlot()).
    // Decisao explicita para esta fase: como nao ha' cartuchos nem modo
    // MSX1/MSX2 no fwMSX ainda (chegam nas Fases 2/3, ver
    // doc/memory-map-spec.md), todo slot primario aceita subslot
    // livremente por enquanto, em vez de adiantar uma regra que nao tem
    // nada a se aplicar. Revisitar quando LoadRom()/modo de maquina
    // existirem de verdade.
    const int active_primary = state->psl[3];
    state->ssl_reg[active_primary] = value;

    for (int page = 0; page < MEMMAP_PAGES; ++page) {
        if (state->psl[page] != active_primary) continue;
        const int bit_shift = page * 2;
        const int chunk_idx = page * 2;
        const int ss = (value >> bit_shift) & 3;
        state->ssl[page] = (uint8_t)ss;
        state->active_view[chunk_idx] = state->chunk[active_primary][ss][chunk_idx];
        state->active_view[chunk_idx + 1] = state->chunk[active_primary][ss][chunk_idx + 1];
        state->active_writable[chunk_idx] = state->chunk_writable[active_primary][ss][chunk_idx];
        state->active_writable[chunk_idx + 1] = state->chunk_writable[active_primary][ss][chunk_idx + 1];
    }
}

uint8_t memmap_read(const SlotState *state, uint16_t addr) {
    // TODO(FDC): o RdZ80 do fMSX intercepta 7FF8h/BFF8h/7F80h/7FB8h (e
    // variantes) para o controlador de disquete quando o slot 3:1 esta
    // visivel -- esse hardware nao existe ainda no fwMSX (ver
    // doc/memory-map-spec.md, secao 3.2). Omissao registrada de
    // proposito, nao esquecida.
    const int chunk_idx = addr >> 13;
    const int offset = addr & (MEMMAP_CHUNK_SIZE - 1);
    return state->active_view[chunk_idx][offset];
}

void memmap_write(SlotState *state, uint16_t addr, uint8_t value) {
    // TODO(FDC): mesma observacao de memmap_read() acima, para o lado de
    // escrita do WrZ80 do fMSX (comandos do FDC em 7FF8h/BFF8h/etc.).
    const int chunk_idx = addr >> 13;
    if (!state->active_writable[chunk_idx]) return;
    const int offset = addr & (MEMMAP_CHUNK_SIZE - 1);
    state->active_view[chunk_idx][offset] = value;
}

uint8_t memmap_peek_slot(const SlotState *state, int primary, int secondary, uint16_t addr) {
    if (!ValidSlot(primary, secondary)) return MEMMAP_EMPTY_BYTE;
    const int chunk_idx = addr >> 13;
    const int offset = addr & (MEMMAP_CHUNK_SIZE - 1);
    return state->chunk[primary][secondary][chunk_idx][offset];
}

void memmap_poke_slot(SlotState *state, int primary, int secondary, uint16_t addr, uint8_t value) {
    if (!ValidSlot(primary, secondary)) return;
    const int chunk_idx = addr >> 13;
    if (!state->chunk_writable[primary][secondary][chunk_idx]) return;
    const int offset = addr & (MEMMAP_CHUNK_SIZE - 1);
    state->chunk[primary][secondary][chunk_idx][offset] = value;
}
