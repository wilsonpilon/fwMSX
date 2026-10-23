/* fwMSX -- o endereco da ROM de Kanji do MSX (portas D8h/D9h = JIS nivel 1, DAh/DBh = nivel 2) como maquina de estados em C. Codigo ORIGINAL do
 * fwMSX (BSD-3-Clause), escrito a partir da interface padrao do MSX (Kanji ROM): ver doc/ocm-spec.md.
 *
 * A ROM de 128 KB e' enderecada por 17 bits: bits 16-11 = os 6 bits altos (porta D9h/DBh de escrita), bits 10-5 = os 6 bits baixos (porta D8h/DAh de escrita)
 * e bits 4-0 = o contador de linha do caractere (32 bytes por caractere). Escrever qualquer das portas zera o contador; ler a porta D9h/DBh devolve o
 * byte e incrementa o contador (so' os 5 bits baixos, em volta). */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KanjiAddr {
    uint32_t addr; /* 17 bits */
} KanjiAddr;

void kanji_reset(KanjiAddr *k);
/* Escrita na porta "baixa" (D8h/DAh): os 6 bits baixos do codigo; zera o contador. */
void kanji_write_low(KanjiAddr *k, uint8_t value);
/* Escrita na porta "alta" (D9h/DBh): os 6 bits altos do codigo; zera o contador. */
void kanji_write_high(KanjiAddr *k, uint8_t value);
/* Leitura: devolve o endereco do byte (dentro de uma ROM de 128 KB) e avanca o contador de linha. */
uint32_t kanji_next(KanjiAddr *k);

#ifdef __cplusplus
}
#endif
