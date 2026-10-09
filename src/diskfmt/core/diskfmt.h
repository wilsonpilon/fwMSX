// fwMSX -- criacao de disquetes MSX em branco e FORMATADOS (C). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause), reescrito a partir do CreateDisk() de resource/msxDiskUtil/MSXDisk.pbi
// (PureBasic): monta o setor de boot com o BPB, as duas copias da FAT (media descriptor +
// entrada reservada) e o diretorio raiz vazio, e enche a area de dados com E5h (o byte de
// "formatado" do MSX-DOS FORMAT). O preenchimento em massa e' feito em Assembly
// (diskfmt_fill, src/diskfmt/asm). Ver doc/diskfmt-spec.md.
//
// So' os 4 formatos que o MSX realmente tem. NAO existe 3 1/2 dupla face / dupla densidade
// de 1,44 MB (o MSX nao suporta).
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DiskFmtId {
    DISKFMT_SS525_180 = 0, /* 5 1/4, face simples, densidade simples: 40 x 1 x 9 = 180 KB */
    DISKFMT_DS525_360,     /* 5 1/4, face dupla, densidade dupla:    40 x 2 x 9 = 360 KB */
    DISKFMT_SS35_360,      /* 3 1/2, face simples, densidade dupla:  80 x 1 x 9 = 360 KB */
    DISKFMT_DS35_720,      /* 3 1/2, face dupla, densidade dupla:    80 x 2 x 9 = 720 KB */
    DISKFMT_COUNT
} DiskFmtId;

typedef struct DiskFmtSpec {
    DiskFmtId id;
    const char *key;  /* nome curto da linha de comando: ss525, ds525, ss35, ds35 */
    const char *name; /* nome para a interface */
    int tracks;
    int sides;
    int sectors_per_track;
    uint8_t media;              /* media descriptor (FCh, FDh, F8h, F9h) */
    uint8_t sectors_per_cluster;
    uint16_t root_entries;
    uint16_t sectors_per_fat;
} DiskFmtSpec;

/* NULL se `id` e' invalido. */
const DiskFmtSpec *diskfmt_spec(DiskFmtId id);
/* Procura pela chave curta ("ss525"...); NULL se nao existe. */
const DiskFmtSpec *diskfmt_spec_by_key(const char *key);

/* Tamanho da imagem em bytes (trilhas x lados x setores x 512). */
size_t diskfmt_image_size(const DiskFmtSpec *spec);

/* Monta a imagem formatada em `image` (precisa de diskfmt_image_size() bytes). `boot_template`
 * (512 bytes, opcional) fornece o bootstrap Z80 do setor de boot; o BPB e' sempre reescrito
 * para o formato pedido. NULL = boot sem codigo (disco FAT12 valido mas nao inicializavel).
 * Devolve 0 em caso de sucesso, -1 se os argumentos nao servem. */
int diskfmt_build(const DiskFmtSpec *spec, uint8_t *image, size_t size, const uint8_t *boot_template);

/* Assembly (src/diskfmt/asm/diskfmt_fill.asm): preenche `count` bytes de `dst` com `value`. */
void diskfmt_fill(uint8_t *dst, uint8_t value, size_t count);

#ifdef __cplusplus
}
#endif
