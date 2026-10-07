// fwMSX -- framing de byte do Kansas City Standard (KCS), usado tanto
// pelo bloco #4B do TZX/TSX (Kansas City Standard do MSX, ver
// resource/makeTSX/TZX_Blocks.h) quanto pela sintese de pulsos a partir
// de um .CAS (sem pulsos gravados -- ver cas_reader.h). Codigo ORIGINAL
// do fwMSX (BSD-3-Clause), algoritmo conferido contra
// resource/openMSX_TSXadv/Contrib/tsx/TsxParser.cc (so' como referencia,
// GPL, nenhum codigo copiado). Ver doc/tape-spec.md.
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Chamado uma vez por PULSO (meio-periodo) gerado, na ordem em que a fita
// os reproduziria.
typedef void (*TapePulseSink)(void *ctx, uint32_t t_states);

// Parametros de enquadramento de um byte (bitCfg/byteCfg do bloco #4B --
// ver resource/makeTSX/TZX_Blocks.h, comentario do Block4B). Padrao MSX
// (bitcfg 0x24, bytecfg 0x54, conferido contra os defaults do makeTSX):
// 1 bit de inicio (valor 0), 8 bits de dados (LSb primeiro), 2 bits de
// fim (valor 1); bit 0 = 2 pulsos de `zero_len`, bit 1 = 4 pulsos de
// `one_len`.
typedef struct KcsByteFraming {
    int start_bits;
    int start_value;
    int stop_bits;
    int stop_value;
    int msb_first;     // 0 = LSb primeiro (MSX), 1 = MSb primeiro (ZX)
    int zero_pulses;   // pulsos por bit 0
    int one_pulses;    // pulsos por bit 1
    uint32_t zero_len; // T-states de cada pulso de um bit 0
    uint32_t one_len;  // T-states de cada pulso de um bit 1
} KcsByteFraming;

// Gera os pulsos de UM byte (bits de inicio + 8 bits de dados + bits de
// fim), entregando cada um a `sink(ctx, t_states)`.
void kcs_emit_byte(const KcsByteFraming *cfg, uint8_t byte, TapePulseSink sink, void *ctx);

// Decodifica UM byte (o inverso de kcs_emit_byte) a partir das duracoes
// MEDIDAS em `pulses[*pos..count)` -- usado pelo "ripper" de .WAV (ver
// src/tape/cpp/wav_ripper.h, doc/tape-spec.md, secao 9). As duracoes
// vem de uma gravacao de verdade (com jitter), por isso toda comparacao
// usa tolerancia (`tolerance_percent`, sobre a soma do grupo de pulsos
// de cada bit) em vez de igualdade exata. O(s) bit(s) de INICIO
// precisam bater de verdade (sem tolerancia extra) -- e' esse
// casamento estrito que impede o piloto do PROXIMO bloco (puro bit 1)
// de ser lido como um fluxo infinito de bytes 0xFF, fazendo a funcao
// falhar exatamente onde os dados de verdade acabam. Os bits de FIM,
// ao contrario, sao aceitos mesmo fora da tolerancia (assumidos no
// valor esperado) -- a essa altura o byte ja foi decidido pelos 8 bits
// de dados, e um bit de fim ruidoso nao deve descartar um byte bom. Os
// 8 bits de DADOS precisam ser decididos sem ambiguidade (bater com um
// valor e nao com o outro); caso contrario (nenhum bate, ou os dois
// batem) a funcao falha sem avancar `*pos` -- indica fim de bloco
// (piloto do proximo bloco, silencio, ou dado corrompido demais para
// decidir). Devolve 1 e preenche `*out_byte`/avanca `*pos` em caso de
// sucesso; 0 senao.
int kcs_decode_byte(const KcsByteFraming *cfg, const uint32_t *pulses, size_t count, size_t *pos,
                     int tolerance_percent, uint8_t *out_byte);

#ifdef __cplusplus
}
#endif
