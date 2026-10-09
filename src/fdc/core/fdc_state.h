// Adaptado de fMSX (resource/fMSX/EMULib/WD1793.{h,c}), Copyright (C) Marat
// Fayzullin 2005-2021. O fwMSX evolui a partir do fMSX com o aval do autor
// original para adaptar/estudar seu codigo (ver README.md) -- isso nao e'
// uma relicenciacao: este arquivo continua sob os termos originais dele
// (nao-comercial, aviso ao autor em caso de mudanca), nao o BSD-3-Clause do
// restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Controladora de disquete WD1793/WD2793 do MSX. DIFERENTE do fMSX, que acessa
// as imagens por FDIDisk (formatos DSK/FDI/IMG/TRD... com varredura de
// cabecalhos de setor), aqui a imagem e' um .dsk "cru" (setores em sequencia:
// trilha, lado, setor), descrito por FdcDisk -- o que o MSX-DOS usa. Sem
// temporizacao fisica: como no fMSX, um comando termina na hora e so' o
// protocolo DRQ/IRQ e o "watchdog" de leitura do registrador READY (7FFFh)
// sao reproduzidos. Ver doc/fdc-spec.md.
//
// No MSX a controladora fica mapeada em memoria dentro do slot do DiskROM:
// 7FF8h status/comando, 7FF9h trilha, 7FFAh setor, 7FFBh dados, 7FFCh lado,
// 7FFDh drive, 7FFFh DRQ/IRQ (a ligacao com o barramento e' do FdcDevice, em
// C++; este motor so' conhece os registradores 0-4).
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FDC_DRIVES 4

#define FDC_REG_COMMAND 0
#define FDC_REG_STATUS 0
#define FDC_REG_TRACK 1
#define FDC_REG_SECTOR 2
#define FDC_REG_DATA 3
#define FDC_REG_SYSTEM 4
#define FDC_REG_READY 4

#define FDC_IRQ 0x80
#define FDC_DRQ 0x40

/* Bits de status (nomes do WD1793.h do fMSX) */
#define FDC_F_BUSY 0x01
#define FDC_F_READONLY 0x40
#define FDC_F_NOTREADY 0x80
#define FDC_F_INDEX 0x02    /* tipo 1 */
#define FDC_F_TRACK0 0x04   /* tipo 1 */
#define FDC_F_SEEKERR 0x10  /* tipo 1 */
#define FDC_F_HEADLOAD 0x20 /* tipo 1 */
#define FDC_F_DRQ 0x02      /* tipo 2/3 */
#define FDC_F_LOSTDATA 0x04
#define FDC_F_ERRCODE 0x18
#define FDC_F_NOTFOUND 0x10

/* Bits do registrador de sistema (escrita em FDC_REG_SYSTEM) */
#define FDC_S_DRIVE 0x03
#define FDC_S_RESET 0x04
#define FDC_S_SIDE 0x10
#define FDC_S_DENSITY 0x20

/* Imagem de disco "crua": setores 1..sectors, em sequencia por trilha/lado. */
typedef struct FdcDisk {
    uint8_t *data;        /* NULL = sem disco no drive */
    size_t size;
    int sides;            /* 1 ou 2 */
    int tracks;
    int sectors;          /* setores por trilha */
    int sec_size;         /* bytes por setor (512) */
    int write_protected;
    uint8_t header[6];    /* ultimo cabecalho de setor (READ ADDRESS): trilha, lado, setor, codigo de tamanho, 0, 0 */
    /* Chamado ao fim de cada setor gravado (offset/tamanho dentro de `data`),
     * para o dono da imagem persistir a escrita. Pode ser NULL. */
    void (*write_cb)(void *user, size_t offset, size_t length);
    void *write_user;
} FdcDisk;

typedef struct Fdc {
    uint8_t r[5];            /* 0 status/comando, 1 trilha, 2 setor, 3 dados, 4 sistema */
    uint8_t drive;
    uint8_t side;
    uint8_t track[FDC_DRIVES]; /* trilha fisica de cada drive */
    uint8_t last_step;       /* ultima direcao de STEP */
    uint8_t irq;             /* FDC_IRQ ou FDC_DRQ pendente */
    uint8_t wait;            /* contador de expiracao (watchdog) */
    uint8_t cmd;
    int wr_length;           /* bytes ainda a gravar */
    int rd_length;           /* bytes ainda a ler */
    uint8_t *ptr;            /* posicao atual dentro da imagem */
    FdcDisk *disk[FDC_DRIVES];
    /* WRITE TRACK (formatacao): ficam DEPOIS de `disk` de proposito -- o save-state grava o
     * Fdc so' ate' `ptr`, e uma formatacao em andamento e' abortada ao carregar (como as
     * transferencias de setor). */
    int trk_left;            /* bytes do fluxo de formatacao ainda por receber (0 = ocioso) */
    uint8_t trk_mode;        /* FDC_TRK_GAP, FDC_TRK_ID ou FDC_TRK_DATA */
    uint8_t trk_id[4];       /* trilha, lado, setor, codigo de tamanho do ultimo campo ID */
    uint8_t trk_id_ok;       /* ha' um campo ID valido esperando o campo de dados */
    int trk_cnt;             /* bytes ja' recebidos do campo atual (ID ou dados) */
    int trk_len;             /* tamanho do campo de dados atual */
    uint8_t *trk_dst;        /* onde gravar o campo de dados (NULL = setor inexistente/diferente) */
} Fdc;

/* Comprimento de uma trilha MFM de 250 kbit/s a 300 rpm (bytes do fluxo do WRITE TRACK). */
#define FDC_TRACK_BYTES 6250
#define FDC_TRK_GAP 0
#define FDC_TRK_ID 1
#define FDC_TRK_DATA 2

/* Zera a controladora (os ponteiros para as imagens sao mantidos). */
void fdc_reset(Fdc *f);

/* Liga `disk` ao drive `n` (NULL desliga). */
void fdc_attach(Fdc *f, int n, FdcDisk *disk);

uint8_t fdc_read(Fdc *f, uint8_t reg);
uint8_t fdc_write(Fdc *f, uint8_t reg, uint8_t value);

/* Descobre a geometria de uma imagem crua: primeiro pelo BPB do setor de boot
 * (bytes/setor, setores/trilha, lados, total de setores), depois pelo tamanho
 * do arquivo (160K/180K/320K/360K/720K/1.44M). Devolve 1 e preenche
 * sides/tracks/sectors/sec_size de `disk`; 0 se o tamanho nao e' de disquete. */
int fdc_disk_detect_geometry(FdcDisk *disk, const uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif
