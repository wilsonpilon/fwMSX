// fwMSX -- Mamute: a calculadora do comando CL (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md, "Fase 4".
//
// Expressoes em 16 bits sem sinal, com a volta (um resultado "grande demais" so' da' a volta, nunca da' erro). Numeros em HEXA por padrao;
// sufixos opcionais D (decimal), B (binario), H (hexa) e O (octal) -- o sufixo so' vale se os digitos antes dele forem validos naquela base
// ("10D" e' decimal 10; para hexa 10D use "10DH"). Operadores, do mais apertado ao mais frouxo: unarios (+ - !), * / %, + -, &, ^, |.
// Parenteses agrupam. "@0".."@3", "@B", "@E", "@S" valem o NUMERO gravado na variavel de debugger (sem o slot). 'A' e "AB" sao literais
// ASCII de ate' 2 caracteres (1 caractere = o codigo; 2 = byte alto + byte baixo).
#pragma once

#include <string>

#include "mamute_target.h"

namespace mamute {

// true e *out (0-65535) em caso de sucesso; senao `error` explica ("DIVISAO POR ZERO", "NUMERO INVALIDO: 12G"...).
bool CalcEval(const std::string &expr, const Vars *vars, int *out, std::string &error);

} // namespace mamute
