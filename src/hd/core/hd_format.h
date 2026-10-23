// fwMSX -- formato de disco rigido (HD/SD) para o Nextor/MSX-DOS 2 (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause), escrito a partir da
// especificacao do MBR do PC e do FAT16 (nada copiado do openMSX): tabela de particoes MBR (a que o Nextor le), setor de boot com o BPB
// e FAT16 vazio. Os tamanhos da FAT saem de uma rotina em Fortran (hd_geometry.f90) e o preenchimento em massa e a troca de bytes das
// strings do IDENTIFY, de rotinas em Assembly (hd_fill.asm). Ver doc/hd-spec.md.
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HD_SECTOR_SIZE 512u
#define HD_MBR_ALIGN_SECTORS 2048u      /* a 1a particao comeca em 1 MiB (como o PC e as ferramentas novas) */
#define HD_MAX_PARTITIONS 4
#define HD_MIN_PARTITION_MB 8u          /* abaixo disso a conta de clusters cairia em FAT12 */
#define HD_MAX_PARTITION_MB 2048u       /* FAT16 com clusters de 32 KB: o teto de uma particao do Nextor */
#define HD_MAX_TOTAL_MB 4096u           /* teto da soma das particoes de uma imagem */
#define HD_HEADS 16u                    /* geometria CHS dita ao BPB e ao IDENTIFY (o Nextor usa LBA) */
#define HD_SECTORS_PER_TRACK 63u

typedef struct HdFat16Plan {
    uint32_t total_sectors;       /* a particao inteira */
    uint8_t sectors_per_cluster;  /* 1, 2, 4... 64 */
    uint16_t reserved_sectors;    /* 1 (o setor de boot) */
    uint16_t root_entries;        /* 512 */
    uint16_t fat_sectors;         /* de cada uma das 2 FATs */
    uint32_t clusters;            /* de dados: entre 4085 e 65524 (FAT16) */
} HdFat16Plan;

/* Escolhe o tamanho do cluster e das FATs para uma particao de `total_sectors`. 0 = ok; -1 = fora de 8 MB a 2048 MB. */
int hd_fat16_plan(uint32_t total_sectors, HdFat16Plan *plan);

/* Monta o setor de boot (512 bytes) da particao: BPB do FAT16, media F8h, sem codigo de boot. `hidden` = setores antes da particao. */
void hd_fat16_boot_sector(uint8_t *sector, const HdFat16Plan *plan, uint32_t hidden);

/* Escreve a FAT (as duas copias) e a raiz vazia de uma particao em `out` (hd_fat16_meta_bytes() bytes, ja' zerados pelo chamador nao e' preciso). */
size_t hd_fat16_meta_bytes(const HdFat16Plan *plan);
void hd_fat16_meta(uint8_t *out, const HdFat16Plan *plan);

typedef struct HdPartition {
    uint32_t start;    /* primeiro setor */
    uint32_t sectors;  /* tamanho */
    uint8_t type;      /* 04h/06h FAT16 */
} HdPartition;

/* Monta o MBR (512 bytes) com ate' 4 particoes e a assinatura 55AAh. */
void hd_mbr(uint8_t *sector, const HdPartition *parts, int count);

/* Os campos CHS (3 bytes) de um LBA, saturando em FE FF FF quando passa de 1023 cilindros. */
void hd_lba_to_chs(uint32_t lba, uint8_t out[3]);

/* Assembly (hd_fill.asm). */
void hd_fill(uint8_t *dst, uint8_t value, size_t count);
/* Copia `count` bytes de `src` para `dst` trocando cada par (a string do IDENTIFY tem o 1o caractere no byte alto). `count` par. */
void hd_swap_pairs(uint8_t *dst, const uint8_t *src, size_t count);

/* Fortran (hd_geometry.f90): setores de cada FAT para `total` setores, cluster de `spc` setores, `reserved` reservados e `root_sectors` de raiz. */
uint32_t hd_fat_sectors(uint32_t total, uint32_t spc, uint32_t reserved, uint32_t root_sectors);

#ifdef __cplusplus
}
#endif
