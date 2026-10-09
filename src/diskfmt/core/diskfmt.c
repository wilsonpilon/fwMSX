// fwMSX -- criacao de disquetes MSX formatados (C). Ver diskfmt.h.
#include "diskfmt.h"

#include <string.h>

/* BPB e' o mesmo desde o MS-DOS 2: so' 5 formatos de 512 bytes/setor existem no MSX. Valores
 * conferidos contra o BPB do msxdos1.dsk (720 KB) do repositorio e contra a tabela FAT12
 * classica (FCh = 180 KB, FDh = 360 KB 5 1/4, F8h = 360 KB 1 lado 3 1/2, F9h = 720 KB). */
static const DiskFmtSpec kSpecs[DISKFMT_COUNT] = {
    {DISKFMT_SS525_180, "ss525", "5 1/4, face simples, densidade simples (180 KB)", 40, 1, 9, 0xFC, 1, 64, 2},
    {DISKFMT_DS525_360, "ds525", "5 1/4, face dupla, densidade dupla (360 KB)", 40, 2, 9, 0xFD, 2, 112, 2},
    {DISKFMT_SS35_360, "ss35", "3 1/2, face simples, densidade dupla (360 KB)", 80, 1, 9, 0xF8, 2, 112, 2},
    {DISKFMT_DS35_720, "ds35", "3 1/2, face dupla, densidade dupla (720 KB)", 80, 2, 9, 0xF9, 2, 112, 3},
};

const DiskFmtSpec *diskfmt_spec(DiskFmtId id) {
    return ((int)id >= 0 && (int)id < DISKFMT_COUNT) ? &kSpecs[id] : NULL;
}

const DiskFmtSpec *diskfmt_spec_by_key(const char *key) {
    int i;
    if (!key) return NULL;
    for (i = 0; i < DISKFMT_COUNT; ++i) {
        if (strcmp(kSpecs[i].key, key) == 0) return &kSpecs[i];
    }
    return NULL;
}

size_t diskfmt_image_size(const DiskFmtSpec *spec) {
    if (!spec) return 0;
    return (size_t)spec->tracks * (size_t)spec->sides * (size_t)spec->sectors_per_track * 512u;
}

static void put16(uint8_t *p, unsigned v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

/* Entrada `n` da FAT12 (mesmo empacotamento de MSXDisk.pbi: WriteFAT). */
static void fat12_set(uint8_t *fat, unsigned n, unsigned value) {
    uint8_t *p = fat + (n * 3u) / 2u;
    if (n & 1u) {
        p[0] = (uint8_t)((p[0] & 0x0F) | ((value & 0x0F) << 4));
        p[1] = (uint8_t)((value >> 4) & 0xFF);
    } else {
        p[0] = (uint8_t)(value & 0xFF);
        p[1] = (uint8_t)((p[1] & 0xF0) | ((value >> 8) & 0x0F));
    }
}

int diskfmt_build(const DiskFmtSpec *spec, uint8_t *image, size_t size, const uint8_t *boot_template) {
    const size_t total_sectors = size / 512u;
    size_t fat_start, root_start, root_sectors, data_start, copy;
    int f;

    if (!spec || !image || size != diskfmt_image_size(spec)) return -1;

    /* 1) tudo "formatado": E5h (Assembly), depois sobrescreve o que tem estrutura. */
    diskfmt_fill(image, 0xE5, size);

    /* 2) setor de boot: bootstrap (se houver) + BPB do formato. */
    memset(image, 0, 512);
    if (boot_template) {
        memcpy(image, boot_template, 512);
    } else {
        image[0] = 0xEB;
        image[1] = 0xFE;
        image[2] = 0x90;
        memcpy(image + 3, "fwMSX   ", 8);
    }
    put16(image + 0x0B, 512);
    image[0x0D] = spec->sectors_per_cluster;
    put16(image + 0x0E, 1);
    image[0x10] = 2;
    put16(image + 0x11, spec->root_entries);
    put16(image + 0x13, (unsigned)total_sectors);
    image[0x15] = spec->media;
    put16(image + 0x16, spec->sectors_per_fat);
    put16(image + 0x18, (unsigned)spec->sectors_per_track);
    put16(image + 0x1A, (unsigned)spec->sides);

    /* 3) FATs (2 copias) e diretorio raiz vazio. */
    fat_start = 1;
    root_start = fat_start + 2u * spec->sectors_per_fat;
    root_sectors = ((size_t)spec->root_entries * 32u + 511u) / 512u;
    data_start = root_start + root_sectors;
    if (data_start >= total_sectors) return -1;

    memset(image + fat_start * 512u, 0, 2u * spec->sectors_per_fat * 512u);
    fat12_set(image + fat_start * 512u, 0, 0x0F00u | spec->media);
    fat12_set(image + fat_start * 512u, 1, 0x0FFFu);
    for (f = 1; f < 2; ++f) {
        copy = fat_start + (size_t)f * spec->sectors_per_fat;
        memcpy(image + copy * 512u, image + fat_start * 512u, (size_t)spec->sectors_per_fat * 512u);
    }
    memset(image + root_start * 512u, 0, root_sectors * 512u);
    return 0;
}
