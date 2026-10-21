/* fwMSX -- projeto .msxproject: o "tipo" de cada arquivo, pela extensao. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/project-spec.md.
 *
 * O tipo diz ao IDE qual editor abrir e, aqui, se o arquivo e' TEXTO: o hash de um texto ignora o CR (o git no Windows troca LF por CRLF e isso
 * nao pode virar "arquivo alterado"). Em C porque e' uma tabela fixa, sem estado. */
#include <stddef.h>
#include <string.h>

struct prj_kind_row {
    const char *ext; /* sem o ponto, minuscula */
    const char *kind;
    int text;
};

static const struct prj_kind_row k_rows[] = {
    /* fontes e texto */
    {"bas", "basic", 1},   {"amx", "basic", 1},     {"dmx", "dignified", 1}, {"bad", "dignified", 1}, {"asm", "asm", 1},
    {"inc", "asm", 1},     {"z80", "asm", 1},       {"s", "asm", 1},         {"pas", "pascal", 1},    {"c", "c", 1},
    {"h", "c", 1},         {"md", "doc", 1},        {"txt", "doc", 1},       {"mml", "mml", 1},       {"dml", "dml", 1},
    {"ini", "config", 1},  {"cfg", "config", 1},    {"json", "config", 1},   {"obi", "recipe", 1},    {"sym", "symbols", 1},
    {"lmx", "listing", 1}, {"gitignore", "config", 1},
    /* binarios */
    {"bmx", "basic-bin", 0}, {"bin", "binary", 0},  {"rom", "rom", 0},       {"dsk", "disk", 0},      {"tsx", "tape", 0},
    {"tzx", "tape", 0},    {"cas", "tape", 0},      {"wav", "audio", 0},     {"pdf", "pdf", 0},       {"spr", "sprite", 0},
    {"alf", "font", 0},    {"sc1", "screen", 0},    {"sc2", "screen", 0},    {"sc3", "screen", 0},    {"sc5", "screen", 0},
    {"sc7", "screen", 0},  {"sc8", "screen", 0},    {"sc12", "screen", 0},   {"png", "image", 0},     {"com", "program", 0},
};

/* Classifica `path` (barras / ou \): escreve o tipo em `kind` (ate' `cap` bytes, com o NUL) e devolve 1 se e' texto, 0 se binario.
 * Sem extensao conhecida: "file", binario (o hash dos bytes crus nunca erra). "Obifile" (sem extensao) e' a receita de build, texto. */
int prj_classify(const char *path, char *kind, int cap) {
    const char *base = path;
    const char *p;
    const char *dot = NULL;
    const char *k = "file";
    int text = 0;
    size_t i;
    char ext[16];
    size_t n;

    for (p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
            dot = NULL;
        } else if (*p == '.') {
            dot = p;
        }
    }
    if (strcmp(base, "Obifile") == 0) {
        k = "recipe";
        text = 1;
    } else if (dot && dot[1]) {
        n = strlen(dot + 1);
        if (n < sizeof(ext)) {
            for (i = 0; i < n; ++i) {
                const char c = dot[1 + i];
                ext[i] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
            }
            ext[n] = 0;
            for (i = 0; i < sizeof(k_rows) / sizeof(k_rows[0]); ++i) {
                if (strcmp(ext, k_rows[i].ext) == 0) {
                    k = k_rows[i].kind;
                    text = k_rows[i].text;
                    break;
                }
            }
        }
    }
    if (cap > 0) {
        strncpy(kind, k, (size_t)cap - 1);
        kind[cap - 1] = 0;
    }
    return text;
}
