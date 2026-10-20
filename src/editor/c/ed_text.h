// fwMSX -- editor de textos: nucleo de varredura de linha (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Palavras, busca numa linha e inversao de maiuscula/minuscula: o que o editor faz a cada tecla, sem alocar.
// Ver doc/editor-spec.md.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 1 se `c` (0-255) forma palavra: letras, digitos, '_' e os caracteres acima de 127 (acentos do MSX).
int ed_is_word_char(int c);

// Coluna do inicio da palavra anterior a `col`: pula o que nao e' palavra a esquerda e depois a palavra.
int ed_word_left(const char *s, int len, int col);

// Coluna do inicio da proxima palavra depois de `col`: pula o resto da palavra e depois o que nao e' palavra
// (ou `len` se a linha acabar).
int ed_word_right(const char *s, int len, int col);

// Busca `nd` (nlen bytes) em `hay` (hlen bytes). Adiante: o menor indice >= start; para tras: o maior indice
// <= start. match_case == 0 ignora maiuscula/minuscula (so' ASCII). Devolve -1 se nao achar.
int ed_find(const char *hay, int hlen, const char *nd, int nlen, int start, int fwd, int match_case);

// Inverte maiuscula/minuscula (so' ASCII) em s[from, to).
void ed_flip_case(char *s, int from, int to);

#ifdef __cplusplus
}
#endif
