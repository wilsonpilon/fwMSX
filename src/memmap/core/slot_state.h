// Adaptado de fMSX (resource/fMSX/fMSX/MSX.c, resource/fMSX/fMSX/MSX.h),
// Copyright (C) Marat Fayzullin 1994-2021. O fwMSX evolui a partir do
// fMSX com o aval do autor original para adaptar/estudar seu codigo (ver
// README.md) -- isso nao e uma relicenciacao: este arquivo continua sob
// os termos originais dele (nao-comercial, aviso ao autor em caso de
// mudanca), nao o BSD-3-Clause do restante do fwMSX. Ver
// LICENSE-THIRD-PARTY.md.
//
// Motor do mapa de memoria MSX (slots primarios/secundarios). Adapta a
// topologia MemMap[4][4][8] + PSL/SSL/SSLReg + PSlot()/SSlot() de
// MSX.c/MSX.h -- ver doc/memory-map-spec.md, secao 2 e 3.2, para o
// raciocinio completo. Desde a Fase 3, tambem adapta a parte de troca de
// banco (ROM apenas, sem SCC/SRAM) de MapROM() -- ver secao 6 do design
// doc para o escopo exato e o raciocinio de cada mapper.
//
// Diferencas deliberadas em relacao ao fMSX (documentadas em detalhe na
// secao 3.2/6 do design doc, nao repetidas aqui):
//   - chunk_writable[][][chunks] e' explicito por pedaco de 8KB, em vez
//     de inferido de uma regra hardcoded ("RAM mora em 3:2", como o
//     EnWrite do fMSX assume).
//   - Sem a logica especifica de controlador de disquete (enderecos
//     7FF8h/BFF8h/etc. do RdZ80/WrZ80 do fMSX) -- esse hardware nao
//     existe ainda no fwMSX. TODO explicito, nao omissao silenciosa: ver
//     doc/memory-map-spec.md, secao 3.2.
//   - MegaROM (Fase 3): so a troca de banco de ROM de MAP_GEN8/GEN16/
//     KONAMI5/KONAMI4/ASCII8/ASCII16 -- sem SCC, sem SRAM (ASCII8/16 sem
//     ela), sem MAP_GMASTER2/MAP_FMPAC/MAP_GUESS. Ver doc/
//     memory-map-spec.md, secao 6, para a justificativa de cada omissao.
//   - Indexado por (primario,secundario) direto, nao por um indice de
//     "slot de cartucho" como o CartMap[PS][SS] do fMSX -- o fwMSX ainda
//     nao tem o conceito de slot fisico de cartucho separado do logico.
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../common/memmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SlotState {
    /* "Verdade" sobre cada uma das 16 combinacoes (primario, secundario):
       um ponteiro por pedaco de 8KB, se e' gravavel, e uma descricao de
       alto nivel (kind/tamanho) para a API de inspecao do depurador. */
    uint8_t *chunk[MEMMAP_PRIMARY_SLOTS][MEMMAP_SECONDARY_SLOTS][MEMMAP_CHUNKS];
    uint8_t chunk_writable[MEMMAP_PRIMARY_SLOTS][MEMMAP_SECONDARY_SLOTS][MEMMAP_CHUNKS];
    MemMapKind slot_kind[MEMMAP_PRIMARY_SLOTS][MEMMAP_SECONDARY_SLOTS];
    size_t slot_size[MEMMAP_PRIMARY_SLOTS][MEMMAP_SECONDARY_SLOTS];

    /* Registradores de troca de slot -- mesmos papeis de
       PSLReg/PSL/SSLReg/SSL no fMSX. ssl_reg e' UM POR SLOT PRIMARIO
       (nao por pagina!) -- ver a nota sobre esse quirk de hardware em
       doc/memory-map-spec.md, secao 2. */
    uint8_t psl_reg;
    uint8_t psl[MEMMAP_PAGES];
    uint8_t ssl_reg[MEMMAP_PRIMARY_SLOTS];
    uint8_t ssl[MEMMAP_PAGES];

    /* Vista ativa (cache rapido): o que a CPU enxerga agora, recomputado
       em memmap_switch_primary()/memmap_switch_secondary(). Equivalente
       a RAM[8] do fMSX, mas com permissao de escrita por pedaco de 8KB
       (active_writable) em vez do EnWrite[4] por pagina de 16KB do
       original -- granularidade mais fina, ver nota de topo do arquivo. */
    uint8_t *active_view[MEMMAP_CHUNKS];
    uint8_t active_writable[MEMMAP_CHUNKS];

    /* MegaROM (bank-switch), Fase 3 -- ver doc/memory-map-spec.md, secao
       6. Chaveado por (primario,secundario) como o resto do struct, ao
       contrario do fMSX (que indexa por um numero de "slot de cartucho"
       via CartMap[PS][SS] -- indirecao que existe la' pra suportar
       hardware fisico de slot de cartucho, conceito que o fwMSX ainda nao
       tem; indexar direto por (primario,secundario) e' a simplificacao
       correta aqui). rom_base pode apontar pra um buffer MAIOR que 64KB
       (varios bancos de 8KB) -- diferente de chunk[][][], que so guarda
       ponteiros pro que esta VISIVEL agora em cada pedaco de 8KB. */
    MemMapMapperType slot_mapper[MEMMAP_PRIMARY_SLOTS][MEMMAP_SECONDARY_SLOTS];
    uint8_t *rom_base[MEMMAP_PRIMARY_SLOTS][MEMMAP_SECONDARY_SLOTS];
    uint8_t rom_bank_mask[MEMMAP_PRIMARY_SLOTS][MEMMAP_SECONDARY_SLOTS];
    uint8_t rom_bank[MEMMAP_PRIMARY_SLOTS][MEMMAP_SECONDARY_SLOTS][4];
} SlotState;

/* Inicializa todas as 16 combinacoes como vazias (leitura =
   MEMMAP_EMPTY_BYTE, escrita descartada) e a vista ativa em 0:0. */
void memmap_init(SlotState *state);

/* Aponta os MEMMAP_CHUNKS/2 pedacos de 8KB de uma combinacao (primario,
   secundario) para `data` (buffer do host, mantido/possuido pelo
   chamador -- SlotState nunca aloca/libera memoria sozinho). `size` deve
   ser multiplo de MEMMAP_CHUNK_SIZE; regioes de 16KB nao preenchidas por
   `size` continuam vazias. Usado hoje so para RAM (Fase 1); ROM chega na
   Fase 2 com o mesmo mecanismo, so com chunk_writable=0. */
void memmap_attach(SlotState *state, int primary, int secondary, uint8_t *data, size_t size, MemMapKind kind,
                    int writable);

/* Conecta uma ROM com bank-switch (MegaROM) na combinacao (primario,
   secundario) -- Fase 3, ver doc/memory-map-spec.md, secao 6. Ao
   contrario de memmap_attach(), `data` pode ser MAIOR que 64KB (varios
   bancos de 8KB agrupados no mesmo buffer); so os 4 pedacos de 8KB
   enderecaveis por bank-switch (indices 2-5, enderecos 4000h-BFFFh) sao
   afetados -- os pedacos 0,1 (0000h-3FFFh) e 6,7 (C000h-FFFFh) ficam
   vazios, replicando hardware real de cartucho MSX classico (que so
   responde em 4000h-BFFFh). `size` deve ser multiplo de
   MEMMAP_CHUNK_SIZE; `mask = size/MEMMAP_CHUNK_SIZE - 1` precisa caber
   num uint8_t (ate 256 bancos = 2MB), o chamador (MemorySystem::LoadRom)
   valida isso antes de chamar. Estado inicial: todos os 4 quartos
   mostram o banco 0 -- simplificacao deliberada em vez da heuristica de
   assinatura 'AB' do fMSX (ver doc/memory-map-spec.md, secao 6). */
void memmap_attach_megarom(SlotState *state, int primary, int secondary, uint8_t *data, size_t size,
                            MemMapMapperType mapper);

/* Tenta tratar uma escrita em (primario,secundario,endereco) como troca de
   banco MegaROM, conforme o protocolo do mapper dessa combinacao (ver
   MapROM() em resource/fMSX/fMSX/MSX.c, e a nota de atribuicao no topo
   deste arquivo). Devolve 1 se a escrita foi RECONHECIDA pelo protocolo
   (inclusive quando nao muda nada, ex.: mesmo banco de novo, ou uma
   selecao de SRAM ignorada por estar fora do escopo desta fase -- ver
   doc/memory-map-spec.md, secao 6) -- nesse caso o chamador (memmap_write)
   NAO deve tratar como descarte padrao de escrita em pedaco nao-gravavel.
   Devolve 0 se o endereco/mapper nao e' reconhecido (combinacao sem
   mapper, ou escrita fora do protocolo esperado) -- cai no descarte
   padrao. Atualiza SEMPRE chunk[primario][secundario][...] (a tabela de
   apoio); so atualiza active_view/active_writable da pagina afetada
   quando essa pagina atualmente mostra a MESMA combinacao
   (primario,secundario) -- replica um detalhe real do MapROM() do fMSX
   em MAP_ASCII8/MAP_ASCII16, onde o endereco de controle (sempre na
   pagina 1) pode afetar um pedaco que vive na pagina 2; para os outros
   mappers essa checagem e' sempre verdadeira por construcao (o endereco
   de controle sempre cai na mesma pagina que o pedaco afetado), entao
   aplicar a mesma checagem pra todos os mappers e' seguro e uniforme. */
int memmap_try_bank_switch(SlotState *state, int primary, int secondary, uint16_t addr, uint8_t value);

/* Troca de slot primario (porta A8h do PPI) / secundario (endereco
   FFFFh) -- portados de PSlot()/SSlot(). O endereco FFFFh em si (leitura
   devolvendo ~ssl_reg[psl[3]], escrita chamando memmap_switch_secondary)
   fica por conta da camada C++ (SlotMemoryBus) -- ver
   doc/memory-map-spec.md, secao 3.3: este motor em C so conhece troca de
   slot e acesso a memoria, nao "que endereco do Z80 aciona o que". */
void memmap_switch_primary(SlotState *state, uint8_t value);
void memmap_switch_secondary(SlotState *state, uint8_t value);

/* Le/escreve na VISTA ATIVA (o que a CPU enxerga agora) -- equivalente a
   RAM[A>>13][A&0x1FFF] do RdZ80/WrZ80 do fMSX, sem a parte de FDC (ver
   nota de topo do arquivo). */
uint8_t memmap_read(const SlotState *state, uint16_t addr);
void memmap_write(SlotState *state, uint16_t addr, uint8_t value);

/* Le/escreve numa combinacao de slot ESPECIFICA, independente da vista
   ativa da CPU -- a API que existe para o depurador enxergar todos os
   slots/subslots (requisito "vital" do autor, ver doc/memory-map-spec.md,
   secao 1/3.3). Escrita e' silenciosamente descartada se o pedaco nao for
   gravavel (mesma regra de memmap_write, so que mirando uma combinacao
   escolhida em vez da vista ativa). */
uint8_t memmap_peek_slot(const SlotState *state, int primary, int secondary, uint16_t addr);
void memmap_poke_slot(SlotState *state, int primary, int secondary, uint16_t addr, uint8_t value);

#ifdef __cplusplus
}
#endif
