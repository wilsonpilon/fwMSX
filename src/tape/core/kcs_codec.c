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
