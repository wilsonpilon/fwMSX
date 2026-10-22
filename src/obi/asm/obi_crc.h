// fwMSX -- obi: CRC-32 dos arquivos da receita (Assembly NASM, ABI dupla Win64/SysV). Ver obi_crc.asm.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// CRC-32 (o mesmo do zip/PNG: polinomio refletido 0xEDB88320) de data[0, len).
unsigned int obi_crc32(const unsigned char *data, int len);

#ifdef __cplusplus
}
#endif
