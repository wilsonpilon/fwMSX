// Adaptado de fMSX (resource/fMSX/Z80/Debug.c, tabelas Mnemonics*[] e a
// funcao DAsm()), Copyright (C) Marat Fayzullin 1995-2021. O fwMSX
// evolui a partir do fMSX com o aval do autor original para adaptar/
// estudar seu codigo (ver README.md) -- isso nao e uma relicenciacao:
// este arquivo continua sob os termos originais dele (nao-comercial,
// aviso ao autor em caso de mudanca), nao o BSD-3-Clause do restante do
// fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Desmontador Z80 (comando `disasm` do --z80dbg, Fase 4 -- ver
// doc/z80-core-spec.md). Tres correcoes cosmeticas em relacao ao DAsm()
// original estao documentadas no .cpp e na secao 6 do documento de
// design -- nenhuma delas afeta execucao/timing da CPU, so o texto
// mostrado pelo desmontador.
#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace z80::debug {

struct DisasmResult {
    std::string text;
    uint16_t length;  // bytes consumidos (prefixo + opcode + operandos)
};

// `read` fornece bytes crus do barramento, a partir de `addr` --
// desacoplado de qualquer IBus/sessao especifica, para poder testar sem
// montar um Z80Cpu/FlatMemoryBus de verdade.
DisasmResult Disassemble(const std::function<uint8_t(uint16_t)> &read, uint16_t addr);

}  // namespace z80::debug
