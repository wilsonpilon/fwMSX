// fwMSX -- formato de disco rigido (C). Ver hd_format.h e doc/hd-spec.md.
#include "hd_format.h"

#include <string.h>

static void put16(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

static void put32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

int hd_fat16_plan(uint32_t total_sectors, HdFat16Plan *plan) {
    const uint32_t min_sectors = HD_MIN_PARTITION_MB * 2048u;
    const uint32_t max_sectors = HD_MAX_PARTITION_MB * 2048u;
    if (!plan || total_sectors < min_sectors || total_sectors > max_sectors) return -1;
    memset(plan, 0, sizeof *plan);
    plan->total_sectors = total_sectors;
    plan->reserved_sectors = 1;
    plan->root_entries = 512;
    const uint32_t root_sectors = (uint32_t)plan->root_entries * 32u / HD_SECTOR_SIZE;
    // o menor cluster que deixa o numero de clusters dentro do FAT16 (ate' 65524)
    for (uint32_t spc = 1; spc <= 64; spc <<= 1) {
        const uint32_t fat = hd_fat_sectors(total_sectors, spc, plan->reserved_sectors, root_sectors);
        uint32_t clusters = (total_sectors - plan->reserved_sectors - root_sectors - 2u * fat) / spc;
        // com cluster de 32 KB, 2048 MB inteiros dariam 65527 clusters (> 65524 = FAT32 para quem le): o sistema de arquivos fica com 65524
        // clusters e o BPB declara so' o que usa; os setores que sobram no fim da particao ficam sem uso
        if (clusters <= 65524u || spc == 64) {
            plan->sectors_per_cluster = (uint8_t)spc;
            plan->fat_sectors = (uint16_t)fat;
            if (clusters > 65524u) {
                clusters = 65524u;
                plan->total_sectors = plan->reserved_sectors + root_sectors + 2u * fat + clusters * spc;
            }
            plan->clusters = clusters;
            return clusters >= 4085u ? 0 : -1;
        }
    }
    return -1;
}

void hd_fat16_boot_sector(uint8_t *s, const HdFat16Plan *plan, uint32_t hidden) {
    memset(s, 0, HD_SECTOR_SIZE);
    s[0] = 0xEB; // JMP curto sobre o BPB + NOP (sem codigo de boot: a particao nao inicializa por si)
    s[1] = 0xFE;
    s[2] = 0x90;
    memcpy(s + 3, "fwMSX   ", 8);
    put16(s + 11, HD_SECTOR_SIZE);
    s[13] = plan->sectors_per_cluster;
    put16(s + 14, plan->reserved_sectors);
    s[16] = 2; // FATs
    put16(s + 17, plan->root_entries);
    put16(s + 19, plan->total_sectors < 65536u ? plan->total_sectors : 0u);
    s[21] = 0xF8; // disco fixo
    put16(s + 22, plan->fat_sectors);
    put16(s + 24, HD_SECTORS_PER_TRACK);
    put16(s + 26, HD_HEADS);
    put32(s + 28, hidden);
    put32(s + 32, plan->total_sectors >= 65536u ? plan->total_sectors : 0u);
    s[36] = 0x80; // drive fisico
    s[38] = 0x29; // assinatura estendida
    put32(s + 39, 0x4D535832u);
    memcpy(s + 43, "NO NAME    ", 11);
    memcpy(s + 54, "FAT16   ", 8);
    s[510] = 0x55;
    s[511] = 0xAA;
}

size_t hd_fat16_meta_bytes(const HdFat16Plan *plan) {
    const size_t root = (size_t)plan->root_entries * 32u;
    return (size_t)plan->reserved_sectors * HD_SECTOR_SIZE + 2u * plan->fat_sectors * HD_SECTOR_SIZE + root;
}

void hd_fat16_meta(uint8_t *out, const HdFat16Plan *plan) {
    memset(out, 0, hd_fat16_meta_bytes(plan));
    for (int copy = 0; copy < 2; ++copy) {
        uint8_t *fat = out + ((size_t)plan->reserved_sectors + (size_t)copy * plan->fat_sectors) * HD_SECTOR_SIZE;
        fat[0] = 0xF8; // media descriptor
        fat[1] = 0xFF;
        fat[2] = 0xFF; // a entrada 1: fim de cadeia, disco limpo
        fat[3] = 0xFF;
    }
}

void hd_lba_to_chs(uint32_t lba, uint8_t out[3]) {
    const uint32_t per_cyl = HD_HEADS * HD_SECTORS_PER_TRACK;
    uint32_t cyl = lba / per_cyl;
    if (cyl > 1023u) {
        out[0] = 0xFE;
        out[1] = 0xFF;
        out[2] = 0xFF;
        return;
    }
    const uint32_t rest = lba % per_cyl;
    const uint32_t head = rest / HD_SECTORS_PER_TRACK;
    const uint32_t sec = rest % HD_SECTORS_PER_TRACK + 1u;
    out[0] = (uint8_t)head;
    out[1] = (uint8_t)((sec & 0x3F) | ((cyl >> 2) & 0xC0));
    out[2] = (uint8_t)(cyl & 0xFF);
}

void hd_mbr(uint8_t *sector, const HdPartition *parts, int count) {
    memset(sector, 0, HD_SECTOR_SIZE);
    if (count > HD_MAX_PARTITIONS) count = HD_MAX_PARTITIONS;
    for (int i = 0; i < count; ++i) {
        uint8_t *e = sector + 446 + i * 16;
        e[0] = i == 0 ? 0x80 : 0x00; // ativa
        hd_lba_to_chs(parts[i].start, e + 1);
        e[4] = parts[i].type;
        hd_lba_to_chs(parts[i].start + parts[i].sectors - 1u, e + 5);
        put32(e + 8, parts[i].start);
        put32(e + 12, parts[i].sectors);
    }
    sector[510] = 0x55;
    sector[511] = 0xAA;
}
