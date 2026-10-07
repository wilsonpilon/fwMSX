// fwMSX -- leitor de .TSX/.TZX (TZX 1.20, do qual o TSX e' um
// superconjunto -- ver resource/makeTSX/docs/TZX_format.md). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/tape-spec.md.
#pragma once

#include <string>

#include "tape_image.h"

namespace tape {

// Le um arquivo .TSX/.TZX e preenche `out` para os dois modos: `fast_bytes`
// so' a partir dos blocos #4B (Kansas City Standard -- o unico que o MSX
// usa de verdade, ver doc/SPEC.md secao 5.2); `pulses` a partir de todo
// bloco de dados reconhecido (10, 11, 12, 13, 14, 20, 4B). Blocos de
// controle (grupos, lacos, saltos, chamadas) e metadados sao PULADOS com
// seguranca (o comprimento de cada um e' conhecido, ver TZX_format.md),
// mas sem efeito algum -- ficam em `out.skipped_blocks`. Devolve false com
// `error` so' se o arquivo nao abrir, a assinatura nao bater, ou um bloco
// desconhecido (fora da lista do TZX 1.20) aparecer -- nesse caso nao ha'
// como saber o tamanho dele para continuar com seguranca.
bool LoadTzxImage(const std::string &path, TapeImage &out, std::string &error);

} // namespace tape
