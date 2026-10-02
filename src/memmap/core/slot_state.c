// Adaptado de fMSX (resource/fMSX/fMSX/MSX.c -- inclui MapROM(), Fase 3),
// Copyright (C) Marat Fayzullin 1994-2021 -- ver slot_state.h para a nota
// de atribuicao completa.
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
    if (state->msx1_subslot_rules == 1 && active_primary <= 2) value = 0;
    if (state->msx1_subslot_rules == 2 && (active_primary == 1 || active_primary == 2)) value = 0;
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
    if (state->active_writable[chunk_idx]) {
        const int offset = addr & (MEMMAP_CHUNK_SIZE - 1);
        state->active_view[chunk_idx][offset] = value;
        return;
    }

    // Nao gravavel diretamente -- pode ser uma troca de banco MegaROM
    // (Fase 3). primary/secondary sao SEMPRE os da pagina que a propria
    // escrita esta acessando (mesmo raciocinio do MapROM() do fMSX: PS/SS
    // vem de PSL[A>>14]/SSL[A>>14], nao de uma combinacao escolhida a
    // parte) -- ver doc/memory-map-spec.md, secao 6.
    const int page = addr >> 14;
    if (memmap_try_bank_switch(state, state->psl[page], state->ssl[page], addr, value)) return;

    // Nem RAM gravavel nem troca de banco reconhecida -- descartada em
    // silencio, mesmo comportamento de antes da Fase 3.
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

void memmap_attach_megarom(SlotState *state, int primary, int secondary, uint8_t *data, size_t size,
                            MemMapMapperType mapper) {
    if (!ValidSlot(primary, secondary)) return;

    // Pedacos 0,1 (0000h-3FFFh) e 6,7 (C000h-FFFFh) ficam vazios --
    // hardware real de cartucho MSX classico (Konami/ASCII) so responde
    // em 4000h-BFFFh; nao ha nada plugado no restante do espaco de
    // enderecos dessa combinacao. Ver doc/memory-map-spec.md, secao 6.
    for (int chunk_idx = 0; chunk_idx < MEMMAP_CHUNKS; ++chunk_idx) {
        state->chunk[primary][secondary][chunk_idx] = g_empty_chunk;
        state->chunk_writable[primary][secondary][chunk_idx] = 0;
    }

    const size_t bank_count = size / MEMMAP_CHUNK_SIZE;

    state->slot_kind[primary][secondary] = MEMMAP_KIND_ROM;
    state->slot_size[primary][secondary] = size;
    state->slot_mapper[primary][secondary] = mapper;
    state->rom_base[primary][secondary] = data;
    state->rom_bank_mask[primary][secondary] = (uint8_t)(bank_count - 1);

    // Estado inicial: os 4 pedacos de 8KB (enderecos 4000h/6000h/8000h/A000h)
    // mostram os bancos 0,1,2,3 -- SetMegaROM(J,0,1,2,3) do fMSX (e o que
    // MegaROMs reais encontram ao ligar: o codigo de INIT de varios jogos
    // chama rotinas em 6000h-7FFFh ANTES de trocar qualquer banco, esperando
    // o banco 1 ali -- com todos os pedacos no banco 0 esse codigo executava
    // lixo, o que travava o Firebird). Os bancos sao mascarados pelo tamanho
    // da ROM (uma MegaROM de 16KB so' tem os bancos 0 e 1).
    for (int quarter = 0; quarter < 4; ++quarter) {
        const size_t bank = (size_t)quarter & (bank_count - 1);
        state->rom_bank[primary][secondary][quarter] = (uint8_t)bank;
        const int chunk_idx = quarter + 2;
        state->chunk[primary][secondary][chunk_idx] = data + (bank << 13);
        state->chunk_writable[primary][secondary][chunk_idx] = 0;
    }

    // Recomputa a vista ativa se essa combinacao ja estiver visivel agora
    // -- mesmo raciocinio de memmap_attach() (ver comentario la').
    memmap_switch_primary(state, state->psl_reg);
}

// Atualiza chunk[primary][secondary][chunk_idx] (sempre) e, so quando a
// pagina de chunk_idx atualmente mostra essa MESMA combinacao, tambem
// active_view/active_writable -- ver o comentario de
// memmap_try_bank_switch() em slot_state.h sobre por que essa checagem e'
// necessaria pra ASCII8/ASCII16 e inofensiva (sempre verdadeira) pros
// outros mappers.
static void RefreshChunk(SlotState *state, int primary, int secondary, int chunk_idx, uint8_t *ptr) {
    state->chunk[primary][secondary][chunk_idx] = ptr;
    state->chunk_writable[primary][secondary][chunk_idx] = 0;

    const int page = chunk_idx / 2;
    if (state->psl[page] != primary || state->ssl[page] != secondary) return;
    state->active_view[chunk_idx] = ptr;
    state->active_writable[chunk_idx] = 0;
}

void memmap_remap_ram_chunk(SlotState *state, int primary, int secondary, int chunk_idx, uint8_t *ptr) {
    if (!ValidSlot(primary, secondary) || chunk_idx < 0 || chunk_idx >= MEMMAP_CHUNKS) return;
    state->chunk[primary][secondary][chunk_idx] = ptr;
    state->chunk_writable[primary][secondary][chunk_idx] = 1;

    const int page = chunk_idx / 2;
    if (state->psl[page] != primary || state->ssl[page] != secondary) return;
    state->active_view[chunk_idx] = ptr;
    state->active_writable[chunk_idx] = 1;
}

int memmap_try_bank_switch(SlotState *state, int primary, int secondary, uint16_t addr, uint8_t value) {
    if (!ValidSlot(primary, secondary)) return 0;
    const MemMapMapperType mapper = state->slot_mapper[primary][secondary];
    if (mapper == MEMMAP_MAPPER_NONE) return 0;
    if (addr < 0x4000 || addr > 0xBFFF) return 0;

    const uint8_t mask = state->rom_bank_mask[primary][secondary];
    uint8_t *const rom = state->rom_base[primary][secondary];
    int quarter = -1; /* 0..3 -- indexa rom_bank[...][quarter] e chunk[...][quarter+2] */
    int bank = -1;    /* novo numero de banco de 8KB para 'quarter' */
    int wide = 0;     /* 1 = mapper de granularidade 16KB (atualiza quarter e quarter+1 juntos) */

    switch (mapper) {
        case MEMMAP_MAPPER_GEN8:
            /* Adaptado de MAP_GEN8 em MapROM() -- SCC (linha "if(J==2)
               SCCOn...") deliberadamente omitida, ver doc/
               memory-map-spec.md, secao 6. */
            if (addr < 0x4000 || addr > 0xBFFF) return 0;
            quarter = (addr - 0x4000) >> 13;
            bank = value & mask;
            break;

        case MEMMAP_MAPPER_GEN16:
            /* Adaptado de MAP_GEN16. J = (A&0x8000)>>14 da' 0 OU 2 (nao 0
               ou 1!) -- indexa diretamente rom_bank[quarter]/
               chunk[quarter+2], por isso o deslocamento por 14 bits em
               vez de 15. */
            if (addr < 0x4000 || addr > 0xBFFF) return 0;
            quarter = (addr & 0x8000) >> 14;
            bank = (value << 1) & mask;
            wide = 1;
            break;

        case MEMMAP_MAPPER_KONAMI5:
            /* Adaptado de MAP_KONAMI5 -- enderecos EXATOS 5000h/7000h/
               9000h/B000h, nenhum outro (condicao replicada tal qual o
               fMSX: (A&0x1FFF)!=0x1000 rejeita qualquer coisa fora
               desses 4 enderecos). SCC deliberadamente omitida. */
            if (addr < 0x5000 || addr > 0xB000 || (addr & 0x1FFF) != 0x1000) return 0;
            quarter = (addr - 0x5000) >> 13;
            bank = value & mask;
            break;

        case MEMMAP_MAPPER_KONAMI4:
            /* Adaptado de MAP_KONAMI4 -- so 6000h/8000h/A000h (8KB-
               alinhados); 4000h fica de fora de proposito (pagina fixa,
               nao trocavel, igual comentario original do fMSX). */
            if (addr < 0x6000 || addr > 0xA000 || (addr & 0x1FFF) != 0) return 0;
            quarter = (addr - 0x4000) >> 13;
            bank = value & mask;
            break;

        case MEMMAP_MAPPER_ASCII8:
            /* Adaptado de MAP_ASCII8 -- SO a troca de banco de ROM; a
               selecao de SRAM (bit V&(mask+1)) e a escrita em SRAM em
               8000h-BFFFh ficam de fora (ver doc/memory-map-spec.md,
               secao 6) -- uma tentativa de selecionar SRAM e' RECONHECIDA
               (devolve 1, nao cai no descarte generico) mas nao faz
               nada, pra nao travar/corromper nada. */
            if (addr < 0x6000 || addr >= 0x8000) return 0;
            quarter = (addr & 0x1800) >> 11;
            if (value & (uint8_t)(mask + 1)) return 1; /* SRAM nao suportada -- ignorada */
            bank = value & mask;
            break;

        case MEMMAP_MAPPER_ASCII16: {
            /* Adaptado de MAP_ASCII16 -- mesma omissao de SRAM que
               ASCII8. A condicao de aceitacao extra
               ((V<=mask+1)||!(A&0xFFF)) e' fidelidade real de hardware
               (fMSX a mantem por compatibilidade com cartuchos que
               escrevem lixo em enderecos vizinhos) -- portada tal qual,
               nao removida como "complexidade desnecessaria". */
            if (addr < 0x6000 || addr >= 0x8000) return 0;
            const int accepted = (value <= (uint8_t)(mask + 1)) || ((addr & 0x0FFF) == 0);
            if (!accepted) return 0;
            quarter = (addr & 0x1000) >> 11; /* 0 ou 2 */
            if (value & (uint8_t)(mask + 1)) return 1; /* SRAM nao suportada -- ignorada */
            bank = (value << 1) & mask;
            wide = 1;
            break;
        }

        default:
            return 0;
    }

    if (quarter < 0 || bank < 0) return 0;

    // NOTA: ao contrario do MapROM() do fMSX (que pula o trabalho quando
    // `V==ROMMapper[I][J]`, um bookkeeping que so' e' um espelho fiel do
    // que esta' de fato mapeado porque o proprio fMSX inicializa
    // ROMMapper[] com SetMegaROM(0,1,2,3) -- bancos DISTINTOS de verdade),
    // aqui SEMPRE aplicamos a troca. Motivo: o estado inicial deste fwMSX
    // e' mais simples (todos os quartos comecam no banco 0 -- ver
    // memmap_attach_megarom()), o que quebra esse invariante para mappers
    // "wide" (GEN16/ASCII16): o quarto emparelhado (quarter+1) tambem
    // comeca apontando pro banco 0 em vez de banco 1, entao um "sem
    // mudanca" na leitura de rom_bank[quarter] deixaria o par
    // inconsistente (chunk[quarter+2] correto, chunk[quarter+3] preso no
    // banco errado). Sem essa otimizacao (que nao tem valor real de
    // desempenho aqui -- troca de banco e' rarissima comparada a
    // instrucoes de Z80 executadas), o codigo fica mais simples E correto.
    state->rom_bank[primary][secondary][quarter] = (uint8_t)bank;
    RefreshChunk(state, primary, secondary, quarter + 2, rom + ((size_t)bank << 13));
    if (wide) {
        state->rom_bank[primary][secondary][quarter + 1] = (uint8_t)(bank | 1);
        RefreshChunk(state, primary, secondary, quarter + 3, rom + (((size_t)bank + 1) << 13));
    }
    return 1;
}
