// fwMSX -- tipos/constantes compartilhados do mapa de memoria MSX
// (slots/subslots). Codigo ORIGINAL do fwMSX (BSD-3-Clause) -- a
// topologia numerica (4 slots primarios x 4 secundarios x 8 paginas de
// 8KB) e' um fato de hardware do MSX, nao "expressao" do fMSX, mas os
// nomes/organizacao aqui sao design proprio. Ver doc/memory-map-spec.md,
// secao 3.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEMMAP_PRIMARY_SLOTS 4
#define MEMMAP_SECONDARY_SLOTS 4
#define MEMMAP_PAGES 4        /* paginas de 16KB do espaco de enderecos do Z80 */
#define MEMMAP_CHUNKS 8       /* pedacos de 8KB (2 por pagina de 16KB) */
#define MEMMAP_CHUNK_SIZE 0x2000
#define MEMMAP_PAGE_SIZE 0x4000

/* Byte devolvido por leitura de slot vazio -- mesma convencao do fMSX
   (NORAM em MSX.h, resource/fMSX/fMSX/MSX.h). */
#define MEMMAP_EMPTY_BYTE 0xFF

/* O que existe numa combinacao (primario, secundario): usado pela API de
   inspecao do depurador (MemorySystem::Describe). MEMMAP_KIND_ROM ainda
   nao e' produzido por nenhum codigo nesta Fase 1 (carregamento de ROM e'
   Fase 2/3 -- ver doc/memory-map-spec.md, secao 6) -- o valor existe
   desde ja' pra nao mudar a forma do enum/struct depois. */
typedef enum MemMapKind {
    MEMMAP_KIND_EMPTY = 0,
    MEMMAP_KIND_RAM = 1,
    MEMMAP_KIND_ROM = 2
} MemMapKind;

/* Tipos de mapper MegaROM (bank-switch) suportados -- Fase 3, ver
   doc/memory-map-spec.md, secao 6. Escopo deliberadamente menor que o
   fMSX (MAP_GEN8/MAP_GEN16/MAP_KONAMI5/MAP_KONAMI4/MAP_ASCII8/
   MAP_ASCII16 -- so a parte de troca de banco de ROM de cada um, sem
   SCC/SRAM): MAP_GMASTER2, MAP_FMPAC e MAP_GUESS ficam de fora, ver a
   justificativa detalhada no design doc. Nomes espelham as constantes
   MAP_* de resource/fMSX/fMSX/MSX.h. */
typedef enum MemMapMapperType {
    MEMMAP_MAPPER_NONE = 0,
    MEMMAP_MAPPER_GEN8,
    MEMMAP_MAPPER_GEN16,
    MEMMAP_MAPPER_KONAMI5,
    MEMMAP_MAPPER_KONAMI4,
    MEMMAP_MAPPER_ASCII8,
    MEMMAP_MAPPER_ASCII16
} MemMapMapperType;

#ifdef __cplusplus
}
#endif
