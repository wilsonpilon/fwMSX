#include "kcs_codec.h"

static void EmitBit(const KcsByteFraming *cfg, int bit, TapePulseSink sink, void *ctx) {
    const int n = bit ? cfg->one_pulses : cfg->zero_pulses;
    const uint32_t len = bit ? cfg->one_len : cfg->zero_len;
    int i;
    for (i = 0; i < n; ++i) sink(ctx, len);
}

void kcs_emit_byte(const KcsByteFraming *cfg, uint8_t byte, TapePulseSink sink, void *ctx) {
    int i;
    for (i = 0; i < cfg->start_bits; ++i) EmitBit(cfg, cfg->start_value, sink, ctx);
    for (i = 0; i < 8; ++i) {
        const int shift = cfg->msb_first ? (7 - i) : i;
        const int bit = (byte >> shift) & 1;
        EmitBit(cfg, bit, sink, ctx);
    }
    for (i = 0; i < cfg->stop_bits; ++i) EmitBit(cfg, cfg->stop_value, sink, ctx);
}

// Confere se os `n` pulsos a partir de `pulses[*pos]` somam perto de
// `n*len` (dentro de `tolerance_percent"%"` sobre a soma do grupo, nao
// pulso a pulso -- mais tolerante a jitter individual, suficiente para
// decidir entre as duas unicas opcoes possiveis: bit 0 ou bit 1). Em
// caso positivo avanca `*pos` e devolve 1; senao devolve 0 sem avancar.
static int MatchPulseGroup(const uint32_t *pulses, size_t count, size_t *pos, int n, uint32_t len,
                            int tolerance_percent) {
    uint32_t total = 0;
    uint32_t expect, tol, diff;
    int i;
    if (n <= 0 || *pos + (size_t)n > count) return 0;
    for (i = 0; i < n; ++i) total += pulses[*pos + (size_t)i];
    expect = len * (uint32_t)n;
    tol = expect * (uint32_t)tolerance_percent / 100;
    diff = total > expect ? total - expect : expect - total;
    if (diff > tol) return 0;
    *pos += (size_t)n;
    return 1;
}

static int MatchBit(const KcsByteFraming *cfg, const uint32_t *pulses, size_t count, size_t *pos,
                     int tolerance_percent, int bit_value) {
    const int n = bit_value ? cfg->one_pulses : cfg->zero_pulses;
    const uint32_t len = bit_value ? cfg->one_len : cfg->zero_len;
    return MatchPulseGroup(pulses, count, pos, n, len, tolerance_percent);
}

int kcs_decode_byte(const KcsByteFraming *cfg, const uint32_t *pulses, size_t count, size_t *pos,
                     int tolerance_percent, uint8_t *out_byte) {
    size_t p = *pos;
    int i;
    uint8_t byte = 0;

    /* O(s) bit(s) de INICIO precisam bater de verdade (sem "assumir") --
       diferente dos de FIM abaixo. Sem essa exigencia, o piloto do
       PROXIMO bloco (uma sequencia de pulsos todos do tamanho do bit 1,
       sem nenhum zero) seria lido como um fluxo infinito de bytes 0xFF
       (cada "bit" sucessivo bateria com o padrao do bit 1, inclusive o
       de inicio assumido) -- e' esse casamento estrito que faz o loop
       de leitura do bloco (RipWavToCas) parar exatamente onde os dados
       de verdade acabam, igual o "Check for a PILOT without previous
       silence" do getByte() original do makeTSX (so' que por um
       caminho mais simples). */
    for (i = 0; i < cfg->start_bits; ++i) {
        if (!MatchBit(cfg, pulses, count, &p, tolerance_percent, cfg->start_value)) return 0;
    }

    for (i = 0; i < 8; ++i) {
        const int shift = cfg->msb_first ? (7 - i) : i;
        size_t try0 = p, try1 = p;
        const int m0 = MatchBit(cfg, pulses, count, &try0, tolerance_percent, 0);
        const int m1 = MatchBit(cfg, pulses, count, &try1, tolerance_percent, 1);
        if (m0 == m1) return 0; /* nenhum bateu, ou os dois bateram (ambiguo) */
        if (m1) byte = (uint8_t)(byte | (1u << shift));
        p = m1 ? try1 : try0;
    }

    for (i = 0; i < cfg->stop_bits; ++i) {
        if (!MatchBit(cfg, pulses, count, &p, tolerance_percent, cfg->stop_value)) {
            const size_t nominal = (size_t)(cfg->stop_value ? cfg->one_pulses : cfg->zero_pulses);
            /* Faltam pulsos para o nominal (ex.: o ULTIMO byte do arquivo,
               sem nenhum pulso depois do bit de fim -- a gravacao so'
               termina ali) -- aceita o byte do mesmo jeito, so' nao sobra
               pulso pra confirmar o resto. Os 8 bits de DADOS ja foram
               decididos sem ambiguidade antes daqui; descartar o byte
               inteiro so' por faltar o fim seria perder um dado bom. */
            if (p + nominal > count) {
                p = count;
                break;
            }
            p += nominal;
        }
    }

    *out_byte = byte;
    *pos = p;
    return 1;
}
