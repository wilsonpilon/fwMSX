// fwMSX -- editor de textos: hash que ignora a caixa (Assembly, NASM dual-ABI Win64/SysV). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver ed_hash.asm e doc/editor-spec.md (fase E4).
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// FNV-1a de 32 bits de s[0, len), dobrando A-Z para a-z.
unsigned int ed_hash_ci(const unsigned char *s, int len);

#ifdef __cplusplus
}
#endif
