// Adaptado de fMSX -- ver fdc_state.h para o aviso de licenca completo.
#include "fdc_state.h"

#include <string.h>

/* Localiza o setor (side_id, track_id, sector_id) no drive, com a cabeca em
 * (side, track). Num .dsk cru os IDs gravados sao os fisicos. Devolve o
 * ponteiro para os dados do setor ou NULL se nao existe. Preenche o cabecalho
 * (SeekFDI() do fMSX). */
static uint8_t *seek_sector(FdcDisk *d, int side, int track, int side_id, int track_id, int sector_id) {
    if (!d || !d->data) return NULL;
    if (side >= d->sides || track >= d->tracks) return NULL;
    if (side_id != side || track_id != track) return NULL;
    if (sector_id < 1 || sector_id > d->sectors) return NULL;

    {
        const size_t index = ((size_t)track * (size_t)d->sides + (size_t)side) * (size_t)d->sectors + (size_t)(sector_id - 1);
        const size_t offset = index * (size_t)d->sec_size;
        int code = 0, n = d->sec_size;
        if (offset + (size_t)d->sec_size > d->size) return NULL;
        while (n > 128) {
            n >>= 1;
            ++code;
        }
        d->header[0] = (uint8_t)track_id;
        d->header[1] = (uint8_t)side_id;
        d->header[2] = (uint8_t)sector_id;
        d->header[3] = (uint8_t)code;
        d->header[4] = 0;
        d->header[5] = 0;
        return d->data + offset;
    }
}

void fdc_reset(Fdc *f) {
    int j;
    f->r[0] = 0x00;
    f->r[1] = 0x00;
    f->r[2] = 0x00;
    f->r[3] = 0x00;
    f->r[4] = FDC_S_RESET;
    f->drive = 0;
    f->side = 0;
    f->last_step = 0;
    f->irq = 0;
    f->wr_length = 0;
    f->rd_length = 0;
    f->wait = 0;
    f->cmd = 0xD0;
    f->ptr = NULL;
    for (j = 0; j < FDC_DRIVES; ++j) f->track[j] = 0;
}

void fdc_attach(Fdc *f, int n, FdcDisk *disk) {
    if (n >= 0 && n < FDC_DRIVES) f->disk[n] = disk;
}

uint8_t fdc_read(Fdc *f, uint8_t reg) {
    uint8_t a;
    FdcDisk *d = f->disk[f->drive];

    switch (reg) {
    case FDC_REG_STATUS:
        a = f->r[0];
        /* sem disco: drive nao pronto */
        if (!d || !d->data) a |= FDC_F_NOTREADY;
        if (d && d->data && d->write_protected) a |= FDC_F_READONLY;
        if ((f->cmd < 0x80) || (f->cmd == 0xD0)) {
            /* o bit INDEX fica alternando enquanto o disco gira */
            f->r[0] = (uint8_t)((f->r[0] ^ FDC_F_INDEX) & (FDC_F_INDEX | FDC_F_BUSY | FDC_F_NOTREADY | FDC_F_READONLY | FDC_F_TRACK0));
        } else {
            /* ler o status limpa tudo, menos BUSY/NOTREADY/READONLY/DRQ */
            f->r[0] &= (uint8_t)(FDC_F_BUSY | FDC_F_NOTREADY | FDC_F_READONLY | FDC_F_DRQ);
        }
        return a;
    case FDC_REG_TRACK:
    case FDC_REG_SECTOR:
        return f->r[reg];
    case FDC_REG_DATA:
        if (f->rd_length) {
            f->r[reg] = *f->ptr++;
            if (--f->rd_length) {
                f->wait = 255;
                /* passa para o proximo setor quando termina o atual */
                if (d && !(f->rd_length & (d->sec_size - 1))) ++f->r[2];
            } else {
                f->r[0] &= (uint8_t) ~(FDC_F_DRQ | FDC_F_BUSY);
                f->irq = FDC_IRQ;
            }
        }
        return f->r[reg];
    case FDC_REG_READY:
        /* depois de um tempo ocioso o comando e' abortado (dado perdido) */
        if (f->wait) {
            if (!--f->wait) {
                f->rd_length = f->wr_length = 0;
                f->r[0] = (uint8_t)((f->r[0] & ~(FDC_F_DRQ | FDC_F_BUSY)) | FDC_F_LOSTDATA);
                f->irq = FDC_IRQ;
            }
        }
        return f->irq;
    default:
        return 0xFF;
    }
}

uint8_t fdc_write(Fdc *f, uint8_t reg, uint8_t v) {
    FdcDisk *d = f->disk[f->drive];
    int j;

    switch (reg) {
    case FDC_REG_COMMAND:
        f->irq = 0;
        /* FORCE INTERRUPT */
        if ((v & 0xF0) == 0xD0) {
            f->rd_length = f->wr_length = 0;
            f->cmd = 0xD0;
            if (f->r[0] & FDC_F_BUSY) f->r[0] &= (uint8_t)~FDC_F_BUSY;
            else f->r[0] = (uint8_t)((f->track[f->drive] ? 0 : FDC_F_TRACK0) | FDC_F_INDEX);
            if (v & 0x08) f->irq = FDC_IRQ;
            return f->irq;
        }
        if (f->r[0] & FDC_F_BUSY) break;
        f->r[0] = 0x00;
        f->cmd = v;

        switch (v & 0xF0) {
        case 0x00: /* RESTORE */
            f->track[f->drive] = 0;
            f->r[0] = (uint8_t)(FDC_F_INDEX | FDC_F_TRACK0 | ((v & 0x08) ? FDC_F_HEADLOAD : 0));
            f->r[1] = 0;
            f->irq = FDC_IRQ;
            break;

        case 0x10: /* SEEK */
            f->rd_length = f->wr_length = 0;
            f->track[f->drive] = f->r[3];
            f->r[0] = (uint8_t)(FDC_F_INDEX | (f->track[f->drive] ? 0 : FDC_F_TRACK0) | ((v & 0x08) ? FDC_F_HEADLOAD : 0));
            f->r[1] = f->track[f->drive];
            f->irq = FDC_IRQ;
            break;

        case 0x20: case 0x30: /* STEP, STEP-AND-UPDATE */
        case 0x40: case 0x50: /* STEP-IN */
        case 0x60: case 0x70: /* STEP-OUT */
            if (v & 0x40) f->last_step = (uint8_t)(v & 0x20);
            else v = (uint8_t)((v & ~0x20) | f->last_step);
            if (v & 0x20) {
                if (f->track[f->drive]) --f->track[f->drive];
            } else {
                ++f->track[f->drive];
            }
            if (v & 0x10) f->r[1] = f->track[f->drive];
            f->r[0] = (uint8_t)(FDC_F_INDEX | (f->track[f->drive] ? 0 : FDC_F_TRACK0));
            f->irq = FDC_IRQ;
            break;

        case 0x80: case 0x90: /* READ SECTOR(S) */
            f->ptr = seek_sector(d, f->side, f->track[f->drive], (v & 0x02) ? !!(v & 0x08) : f->side, f->r[1], f->r[2]);
            if (!f->ptr) {
                f->r[0] = (uint8_t)((f->r[0] & ~FDC_F_ERRCODE) | FDC_F_NOTFOUND);
                f->irq = FDC_IRQ;
            } else {
                f->rd_length = d->sec_size * ((v & 0x10) ? (d->sectors - f->r[2] + 1) : 1);
                f->r[0] |= (uint8_t)(FDC_F_BUSY | FDC_F_DRQ);
                f->irq = FDC_DRQ;
                f->wait = 255;
            }
            break;

        case 0xA0: case 0xB0: /* WRITE SECTOR(S) */
            if (d && d->data && d->write_protected) {
                f->r[0] |= 0x40; /* protegido contra gravacao */
                f->irq = FDC_IRQ;
                break;
            }
            f->ptr = seek_sector(d, f->side, f->track[f->drive], (v & 0x02) ? !!(v & 0x08) : f->side, f->r[1], f->r[2]);
            if (!f->ptr) {
                f->r[0] = (uint8_t)((f->r[0] & ~FDC_F_ERRCODE) | FDC_F_NOTFOUND);
                f->irq = FDC_IRQ;
            } else {
                f->wr_length = d->sec_size * ((v & 0x10) ? (d->sectors - f->r[2] + 1) : 1);
                f->r[0] |= (uint8_t)(FDC_F_BUSY | FDC_F_DRQ);
                f->irq = FDC_DRQ;
                f->wait = 255;
            }
            break;

        case 0xC0: /* READ ADDRESS */
            f->ptr = NULL;
            if (d && d->data) {
                for (j = 1; j <= d->sectors; ++j) {
                    if (seek_sector(d, f->side, f->track[f->drive], f->side, f->track[f->drive], j)) {
                        f->ptr = d->header;
                        break;
                    }
                }
            }
            if (!f->ptr) {
                f->r[0] |= FDC_F_NOTFOUND;
                f->irq = FDC_IRQ;
            } else {
                f->rd_length = 6;
                f->r[0] |= (uint8_t)(FDC_F_BUSY | FDC_F_DRQ);
                f->irq = FDC_DRQ;
                f->wait = 255;
            }
            break;

        default: /* READ/WRITE TRACK: nao suportados (como no fMSX) */
            break;
        }
        break;

    case FDC_REG_TRACK:
    case FDC_REG_SECTOR:
        if (!(f->r[0] & FDC_F_BUSY)) f->r[reg] = v;
        break;

    case FDC_REG_SYSTEM:
        /* S_RESET subindo reinicia a controladora */
        if ((f->r[4] ^ v) & v & FDC_S_RESET) fdc_reset(f);
        f->drive = (uint8_t)(v & FDC_S_DRIVE);
        f->side = (uint8_t)!(v & FDC_S_SIDE);
        f->r[4] = v;
        break;

    case FDC_REG_DATA:
        if (f->wr_length) {
            *f->ptr++ = v;
            if (--f->wr_length) {
                f->wait = 255;
                if (d && !(f->wr_length & (d->sec_size - 1))) {
                    ++f->r[2];
                    if (d->write_cb) d->write_cb(d->write_user, (size_t)(f->ptr - d->data) - (size_t)d->sec_size, (size_t)d->sec_size);
                }
            } else {
                f->r[0] &= (uint8_t) ~(FDC_F_DRQ | FDC_F_BUSY);
                f->irq = FDC_IRQ;
                if (d && d->write_cb) d->write_cb(d->write_user, (size_t)(f->ptr - d->data) - (size_t)d->sec_size, (size_t)d->sec_size);
            }
        }
        f->r[reg] = v;
        break;

    default:
        break;
    }
    return f->irq;
}

int fdc_disk_detect_geometry(FdcDisk *disk, const uint8_t *data, size_t size) {
    int sec_size = 0, spt = 0, heads = 0;
    size_t total = 0;

    /* 1) BPB do setor de boot (DOS 2+ / MSX-DOS): 0Bh bytes/setor, 13h total de
     * setores, 18h setores/trilha, 1Ah lados. So' vale se for coerente com o
     * tamanho do arquivo. */
    if (size >= 512) {
        sec_size = data[0x0B] | (data[0x0C] << 8);
        total = (size_t)(data[0x13] | (data[0x14] << 8));
        spt = data[0x18] | (data[0x19] << 8);
        heads = data[0x1A] | (data[0x1B] << 8);
        if (sec_size == 512 && spt >= 8 && spt <= 18 && (heads == 1 || heads == 2) && total * 512 == size &&
            total % ((size_t)spt * (size_t)heads) == 0) {
            disk->sec_size = 512;
            disk->sectors = spt;
            disk->sides = heads;
            disk->tracks = (int)(total / ((size_t)spt * (size_t)heads));
            return 1;
        }
    }

    /* 2) so' pelo tamanho */
    switch (size) {
    case 163840: disk->sides = 1; disk->tracks = 40; disk->sectors = 8; break;
    case 184320: disk->sides = 1; disk->tracks = 40; disk->sectors = 9; break;
    case 327680: disk->sides = 2; disk->tracks = 40; disk->sectors = 8; break;
    case 368640: disk->sides = 2; disk->tracks = 40; disk->sectors = 9; break;
    case 737280: disk->sides = 2; disk->tracks = 80; disk->sectors = 9; break;
    case 1474560: disk->sides = 2; disk->tracks = 80; disk->sectors = 18; break;
    default: return 0;
    }
    disk->sec_size = 512;
    return 1;
}
