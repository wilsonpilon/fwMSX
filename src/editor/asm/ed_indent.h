// fwMSX -- editor de textos: indentacao de uma linha (Assembly, NASM dual-ABI Win64/SysV). Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). Ver ed_indent.asm e doc/editor-spec.md.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Quantos bytes de espaco (32) ou tabulacao (9) ha' no inicio de s[0, len). E' a indentacao que o auto-indent
// copia, que "Unindent" remove e que "Put in Tabs" converte.
int ed_leading_ws(const unsigned char *s, int len);

#ifdef __cplusplus
}
#endif
