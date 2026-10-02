// Adaptado de fMSX (EMULib/I8255.c, MSX.c) -- ver ppi_state.h para a nota
// de atribuicao. A tabela de nomes de tecla e' design proprio do fwMSX; as
// COORDENADAS na matriz (fato de hardware do MSX, conferido contra
// Keys[] de MSX.c) vem de src/ppi/fortran/key_matrix.f90.
#include "ppi_state.h"

#include <string.h>

// Implementada em src/ppi/fortran/key_matrix.f90 (bind(c)).
extern void ppi_build_key_matrix(int8_t *rows, int8_t *masks);

// Ordem == ids de tecla; DEVE bater com key_matrix.f90 (o teste de
// tests/z80/ppi_test.cpp confere uma amostra de cada grupo).
static const char *const kKeyNames[] = {
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "-", "=", "\\", "[", "]", ";", "'", "`", ",", ".", "/",
    "shift", "ctrl", "graph", "caps", "code", "f1", "f2", "f3",
    "f4", "f5", "esc", "tab", "stop", "bs", "select", "enter",
    "space", "home", "ins", "del", "left", "up", "down", "right",
    "pad0", "pad1", "pad2", "pad3", "pad4", "pad5", "pad6", "pad7", "pad8", "pad9",
    "pad*", "pad+", "pad/", "pad-", "pad,", "pad.",
};
#define KEY_COUNT ((int)(sizeof(kKeyNames) / sizeof(kKeyNames[0])))

static int8_t g_key_row[KEY_COUNT];
static int8_t g_key_mask[KEY_COUNT];
static int g_keys_ready = 0;

static void KeysInit(void) {
    if (g_keys_ready) return;
    ppi_build_key_matrix(g_key_row, g_key_mask);
    g_keys_ready = 1;
}

void ppi_reset_chip(PpiState *p) {
    /* Reset8255(): tudo 0x00, controle 0x9B (todas as portas em entrada). */
    p->r[0] = p->rout[0] = p->rin[0] = 0x00;
    p->r[1] = p->rout[1] = p->rin[1] = 0x00;
    p->r[2] = p->rout[2] = p->rin[2] = 0x00;
    p->r[3] = 0x9B;
}

void ppi_reset(PpiState *p) {
    KeysInit();
    ppi_reset_chip(p);
    ppi_key_release_all(p);
}

void ppi_write(PpiState *p, int reg, uint8_t value) {
    switch (reg & 3) {
        case 0:
        case 1:
        case 2:
            p->r[reg & 3] = value;
            break;
        default:
            if (value & 0x80) {
                p->r[3] = value;
            } else {
                /* Set/reset de um bit da porta C: bits 3-1 = qual, bit 0 = valor. */
                const uint8_t bit = (uint8_t)(1u << ((value & 0x0E) >> 1));
                if (value & 0x01) p->r[2] |= bit;
                else p->r[2] &= (uint8_t)~bit;
            }
            break;
    }

    /* Pinos de saida: a porta so' dirige o pino quando esta' em modo saida. */
    {
        const uint8_t c = p->r[3];
        p->rout[0] = (c & 0x10) ? 0x00 : p->r[0];
        p->rout[1] = (c & 0x02) ? 0x00 : p->r[1];
        p->rout[2] = (uint8_t)((((c & 0x01) ? 0x00 : p->r[2]) & 0x0F) | (((c & 0x08) ? 0x00 : p->r[2]) & 0xF0));
    }
}

uint8_t ppi_read(PpiState *p, int reg) {
    p->rin[1] = p->key_state[p->rout[2] & 0x0F];
    switch (reg & 3) {
        case 0: return (p->r[3] & 0x10) ? p->rin[0] : p->r[0];
        case 1: return (p->r[3] & 0x02) ? p->rin[1] : p->r[1];
        case 2:
            return (uint8_t)((((p->r[3] & 0x01) ? p->rin[2] : p->r[2]) & 0x0F) |
                             (((p->r[3] & 0x08) ? p->rin[2] : p->r[2]) & 0xF0));
        default: return p->r[3];
    }
}

int ppi_key_count(void) { return KEY_COUNT; }

const char *ppi_key_name(int id) { return (id >= 0 && id < KEY_COUNT) ? kKeyNames[id] : NULL; }

static char Lower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c; }

int ppi_key_lookup(const char *name) {
    int i;
    if (!name) return PPI_KEY_NONE;
    for (i = 0; i < KEY_COUNT; ++i) {
        const char *a = name;
        const char *b = kKeyNames[i];
        while (*a && Lower(*a) == *b) {
            ++a;
            ++b;
        }
        if (!*a && !*b) return i;
    }
    return PPI_KEY_NONE;
}

int ppi_key_position(int id, int *row, uint8_t *mask) {
    if (id < 0 || id >= KEY_COUNT) return 0;
    KeysInit();
    if (row) *row = g_key_row[id];
    if (mask) *mask = (uint8_t)g_key_mask[id];
    return 1;
}

int ppi_key_set(PpiState *p, int id, int pressed) {
    int row;
    uint8_t mask;
    if (!ppi_key_position(id, &row, &mask)) return 0;
    if (pressed) p->key_state[row] &= (uint8_t)~mask;
    else p->key_state[row] |= mask;
    return 1;
}

void ppi_key_release_all(PpiState *p) { memset(p->key_state, 0xFF, sizeof(p->key_state)); }
