// fwMSX -- Basic Dignified: comparacao de prefixo sem diferenciar maiusculas (Assembly NASM, ABI dupla Win64/SysV).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver badig_str.asm.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 1 se s[0, slen) comeca com p[0, plen), dobrando A-Z para a-z nos dois lados; 0 se nao (ou se slen < plen).
int bd_prefix_ci(const char *s, int slen, const char *p, int plen);

#ifdef __cplusplus
}
#endif
