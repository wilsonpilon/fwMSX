// fwMSX -- MAN/HELP: quebra de linhas longas em colunas (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver man_wrap.c e doc/man-spec.md.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Quantos caracteres de s[0..len) formam a primeira linha de no maximo `width` colunas: para no primeiro '\n'
// (esse nao entra na conta do chamador, que o pula), senao no ULTIMO espaco que cabe, senao corta em `width`.
// Devolve sempre >= 1 quando len > 0 (a quebra sempre avanca).
int man_wrap_break(const char *s, int len, int width);

#ifdef __cplusplus
}
#endif
