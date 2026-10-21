/* fwMSX -- Basic Dignified: tabela de palavras do tokenizador do MSX BASIC (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause). */
#include "badig_tok.h"

#include <string.h>

#include "../asm/badig_str.h"

/* A mesma ordem de TOKENS em msxbatoken.py: a primeira palavra que e' prefixo do texto vence (por isso ERROR vem antes de ERR, SPC( e TAB( levam o
   parentese, LOC( leva o 28 e o apostrofo vira  :'  mais o token de REM). Os tokens de duas posicoes comecam por ff. */
static const struct {
    const char *name;
    const char *hex;
} kTable[] = {
    {">", "ee"}, {"PAINT", "bf"}, {"=", "ef"}, {"ERROR", "a6"}, {"ERR", "e2"}, {"<", "f0"}, {"+", "f1"},
    {"FIELD", "b1"}, {"PLAY", "c1"}, {"-", "f2"}, {"FILES", "b7"}, {"POINT", "ed"}, {"*", "f3"}, {"POKE", "98"},
    {"/", "f4"}, {"FN", "de"}, {"^", "f5"}, {"FOR", "82"}, {"PRESET", "c3"}, {"\\", "fc"}, {"PRINT", "91"}, {"?", "91"},
    {"PSET", "c2"}, {"AND", "f6"}, {"GET", "b2"}, {"PUT", "b3"}, {"GOSUB", "8d"}, {"READ", "87"}, {"GOTO", "89"},
    {"ATTR$", "e9"}, {"RENUM", "aa"}, {"AUTO", "a9"}, {"IF", "8b"}, {"RESTORE", "8c"}, {"BASE", "c9"}, {"IMP", "fa"},
    {"RESUME", "a7"}, {"BEEP", "c0"}, {"INKEY$", "ec"}, {"RETURN", "8e"}, {"BLOAD", "cf"}, {"INPUT", "85"},
    {"BSAVE", "d0"}, {"INSTR", "e5"}, {"RSET", "b9"}, {"CALL", "ca"}, {"_", "5f"}, {"RUN", "8a"}, {"IPL", "d5"}, {"SAVE", "ba"},
    {"KEY", "cc"}, {"SCREEN", "c5"}, {"KILL", "d4"}, {"SET", "d2"}, {"CIRCLE", "bc"}, {"CLEAR", "92"}, {"CLOAD", "9b"},
    {"LET", "88"}, {"SOUND", "c4"}, {"CLOSE", "b4"}, {"LFILES", "bb"}, {"CLS", "9f"}, {"LINE", "af"}, {"SPC(", "df"},
    {"CMD", "d7"}, {"LIST", "93"}, {"SPRITE", "c7"}, {"COLOR", "bd"}, {"LLIST", "9e"}, {"CONT", "99"}, {"LOAD", "b5"},
    {"STEP", "dc"}, {"COPY", "d6"}, {"LOCATE", "d8"}, {"STOP", "90"}, {"CSAVE", "9a"}, {"CSRLIN", "e8"},
    {"STRING$", "e3"}, {"LPRINT", "9d"}, {"SWAP", "a4"}, {"LSET", "b8"}, {"TAB(", "db"}, {"MAX", "cd"}, {"DATA", "84"},
    {"MERGE", "b6"}, {"THEN", "da"}, {"TIME", "cb"}, {"TO", "d9"}, {"DEFDBL", "ae"}, {"DEFINT", "ac"}, {"DEFSTR", "ab"},
    {"TROFF", "a3"}, {"DEFSNG", "ad"}, {"TRON", "a2"}, {"DEF", "97"}, {"MOD", "fb"}, {"USING", "e4"},
    {"DELETE", "a8"}, {"MOTOR", "ce"}, {"USR", "dd"}, {"DIM", "86"}, {"NAME", "d3"}, {"DRAW", "be"}, {"NEW", "94"},
    {"VARPTR", "e7"}, {"NEXT", "83"}, {"VDP", "c8"}, {"DSKI$", "ea"}, {"NOT", "e0"}, {"DSKO$", "d1"}, {"VPOKE", "c6"},
    {"OFF", "eb"}, {"WAIT", "96"}, {"END", "81"}, {"ON", "95"}, {"WIDTH", "a0"}, {"OPEN", "b0"}, {"XOR", "f8"},
    {"EQV", "f9"}, {"OR", "f7"}, {"ERASE", "a5"}, {"OUT", "9c"}, {"ERL", "e1"}, {"REM", "8f"},

    {"PDL", "ffa4"}, {"EXP", "ff8b"}, {"PEEK", "ff97"}, {"FIX", "ffa1"}, {"POS", "ff91"}, {"FPOS", "ffa7"},
    {"ABS", "ff86"}, {"FRE", "ff8f"}, {"ASC", "ff95"}, {"ATN", "ff8e"}, {"HEX$", "ff9b"}, {"BIN$", "ff9d"},
    {"INP", "ff90"}, {"RIGHT$", "ff82"}, {"RND", "ff88"}, {"INT", "ff85"}, {"CDBL", "ffa0"}, {"CHR$", "ff96"},
    {"CINT", "ff9e"}, {"LEFT$", "ff81"}, {"SGN", "ff84"}, {"LEN", "ff92"}, {"SIN", "ff89"}, {"SPACE$", "ff99"},
    {"SQR", "ff87"}, {"LOC(", "ffac28"}, {"STICK", "ffa2"}, {"COS", "ff8c"}, {"LOF", "ffad"}, {"STR$", "ff93"},
    {"CSNG", "ff9f"}, {"LOG", "ff8a"}, {"STRIG", "ffa3"}, {"LPOS", "ff9c"}, {"CVD", "ffaa"}, {"CVI", "ffa8"},
    {"CVS", "ffa9"}, {"TAN", "ff8d"}, {"MID$", "ff83"}, {"MKD$", "ffb0"}, {"MKI$", "ffae"}, {"MKS$", "ffaf"},
    {"VAL", "ff94"}, {"DSKF", "ffa6"}, {"VPEEK", "ff98"}, {"OCT$", "ff9a"}, {"EOF", "ffab"}, {"PAD", "ffa5"},

    {"'", "3a8fe6"}, {"ELSE", "3aa1"}, {"AS", "4153"},
};

#define TABLE_N ((int)(sizeof kTable / sizeof kTable[0]))

static const char *const kJumps[] = {"RESTORE", "AUTO", "RENUM", "DELETE", "RESUME", "ERL", "ELSE",
                                     "RUN", "LIST", "LLIST", "GOTO", "RETURN", "THEN", "GOSUB"};

int bd_tok_count(void) { return TABLE_N; }

int bd_tok_find(const char *s, int len) {
    for (int i = 0; i < TABLE_N; ++i) {
        if (bd_prefix_ci(s, len, kTable[i].name, (int)strlen(kTable[i].name))) return i;
    }
    return -1;
}

const char *bd_tok_name(int i) { return i >= 0 && i < TABLE_N ? kTable[i].name : ""; }
const char *bd_tok_hex(int i) { return i >= 0 && i < TABLE_N ? kTable[i].hex : ""; }

int bd_tok_is_jump(int i) {
    if (i < 0 || i >= TABLE_N) return 0;
    for (size_t k = 0; k < sizeof kJumps / sizeof kJumps[0]; ++k) {
        if (strcmp(kJumps[k], kTable[i].name) == 0) return 1;
    }
    return 0;
}
