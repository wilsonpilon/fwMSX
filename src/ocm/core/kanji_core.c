/* fwMSX -- endereco da ROM de Kanji. Ver kanji_core.h e doc/ocm-spec.md. */
#include "kanji_core.h"

void kanji_reset(KanjiAddr *k) { k->addr = 0; }

void kanji_write_low(KanjiAddr *k, uint8_t value) { k->addr = (k->addr & 0x1F800u) | ((uint32_t)(value & 0x3F) << 5); }

void kanji_write_high(KanjiAddr *k, uint8_t value) { k->addr = (k->addr & 0x007E0u) | ((uint32_t)(value & 0x3F) << 11); }

uint32_t kanji_next(KanjiAddr *k) {
    const uint32_t at = k->addr;
    k->addr = (k->addr & ~0x1Fu) | ((k->addr + 1u) & 0x1Fu);
    return at;
}
