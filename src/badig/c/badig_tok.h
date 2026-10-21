// fwMSX -- Basic Dignified: tabela de palavras do tokenizador do MSX BASIC (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// A ORDEM da tabela e' a do msxbatoken.py: o tokenizador escolhe a primeira palavra que e' prefixo do texto, nao a mais longa.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Numero de palavras da tabela.
int bd_tok_count(void);
// Indice da primeira palavra (na ordem da tabela) que e' prefixo de s[0, len) sem diferenciar maiusculas, ou -1.
int bd_tok_find(const char *s, int len);
// O texto da palavra `i` ("PRINT", "LEFT$", "SPC(") e o seu token em hexadecimal ("91", "ff81", "ffac28").
const char *bd_tok_name(int i);
const char *bd_tok_hex(int i);
// 1 se a palavra e' de desvio (RESTORE, GOTO, THEN, ELSE...): depois dela vem um numero de linha.
int bd_tok_is_jump(int i);

#ifdef __cplusplus
}
#endif
