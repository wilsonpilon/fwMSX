/* fwMSX -- Mamute: formatacao de despejos de memoria (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
 * Ver doc/mamute-spec.md. O checksum dos modos 2 e 3 vem do Fortran (mamute_sum.f90). */
#ifndef FWMSX_MAMUTE_DUMP_H
#define FWMSX_MAMUTE_DUMP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Bytes por linha de cada modo do comando C: 0 = 4, 1 = 16, 2 = 8, 3 = 8. Modo invalido = 0. */
int mamute_dump_width(int mode);

/* Escreve UMA linha do despejo em `out` (terminada em '\0') e devolve o tamanho. `n` e' quantos bytes
 * valem (a ultima linha pode ter menos que a largura do modo; as colunas faltantes ficam em branco). */
int mamute_dump_line(int mode, unsigned addr, const unsigned char *bytes, int n, char *out, int out_size);

/* Fortran (mamute_sum.f90): soma dos `n` bytes (mais o byte baixo do endereco, se with_addr), modulo 256. */
int mamute_checksum8(const signed char *buf, int n, int addr_lo, int with_addr);

/* Fortran (mamute_total.f90): soma de TODOS os `n` bytes, modulo 65536 (o checksum agregado do XTS). */
int mamute_sum16(const signed char *buf, int n);

#ifdef __cplusplus
}
#endif

#endif
