// fwMSX -- editor de textos: nucleo de varredura do destaque de sintaxe (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Medir onde acaba um identificador, um numero ou uma string; o resto (que palavra e' que classe) e' do syntax.cpp.
// Ver doc/editor-spec.md (fase E4).
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Bits de `flags` de ed_hl_ident_end.
enum {
    ED_ID_SUFFIX = 1,   // sufixo de tipo do BASIC no fim do nome: $ % ! #
    ED_ID_DOT = 2,      // '.' dentro do nome (rotulos locais, nomes qualificados)
    ED_ID_AT = 4,       // '@' dentro do nome
    ED_ID_QUOTE = 8     // um apostrofo no fim (af')
};

// 1 se `c` (0-255) pode comecar um identificador: letra, '_' ou byte alto (acentos do MSX).
int ed_hl_ident_start(int c);
// 1 se `c` pode continuar um identificador: letra, digito, '_' ou byte alto.
int ed_hl_ident_char(int c);

// Fim (indice depois do ultimo caractere) do identificador que comeca em `pos`; `pos` se nao ha' identificador.
int ed_hl_ident_end(const char *s, int len, int pos, int flags);

// Fim do token numerico que comeca em `pos`: digitos, &H &B &O, 0x, $ # % seguidos de digitos, fracao e expoente,
// sufixos h/b de assembler. `pos` se nao ha' numero. O token ainda precisa ser validado por ed_number_kind (Fortran).
int ed_hl_number_end(const char *s, int len, int pos);

// Fim da string que comeca em `pos` (onde esta' a aspa `quote`): indice depois da aspa que fecha, ou `len` se a linha acaba antes.
// Aspa dobrada ("" ou '') continua a string.
int ed_hl_string_end(const char *s, int len, int pos, char quote);

// Classifica um token numerico (Fortran, ed_stats-like): 0 = nao e' numero, 1 = decimal inteiro, 2 = hexadecimal,
// 3 = binario, 4 = octal, 5 = real.
int ed_number_kind(const char *s, int n);

// Inicio da palavra que termina em `col` (letras, digitos, '_' e bytes altos para tras): o prefixo que o completar usa.
int ed_word_start(const char *s, int len, int col);

// Nota de um candidato do completar para o prefixo (Fortran): 0 = nao serve, 2 = prefixo em outra caixa, 3 = prefixo com a mesma caixa.
int ed_match_score(const char *pre, int np, const char *cand, int nc);

#ifdef __cplusplus
}
#endif
