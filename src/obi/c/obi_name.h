// fwMSX -- obi: nomes 8.3 do MSX-DOS e o tipo de um fonte pela extensao (C). Ver obi_name.c.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum {
    OBI_NAME_OK = 0,
    OBI_NAME_EMPTY = 1,    // sem nome
    OBI_NAME_BASE_LONG = 2, // mais de 8 caracteres antes do ponto
    OBI_NAME_EXT_LONG = 3,  // mais de 3 depois do ponto
    OBI_NAME_BAD_CHAR = 4   // caractere que o MSX-DOS nao aceita
};

// Escreve em `out` (cap >= 13) o nome `in` em maiusculas e valida como 8.3. Devolve OBI_NAME_*; em erro `out` fica vazio.
int obi_msx_name(const char *in, char *out, int cap);

enum {
    OBI_KIND_RAW = 0, // copiado como esta' (.dat, .bin, .spr, .scr, .bas, ...)
    OBI_KIND_DMX = 1, // Basic Dignified: vira .BMX pelo badig
    OBI_KIND_ASM = 2  // Assembly Z80: vira .BIN/.COM/.ROM pelo fwasm
};

// O que o obi faz com um arquivo de nome `path`, pela extensao (sem diferenciar maiusculas).
int obi_kind(const char *path);

#ifdef __cplusplus
}
#endif
