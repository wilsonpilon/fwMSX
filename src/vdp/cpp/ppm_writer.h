// fwMSX -- exportacao de um quadro renderizado do VDP como PPM binario
// (P6). Codigo ORIGINAL do fwMSX (BSD-3-Clause). PPM foi escolhido de
// proposito por nao precisar de nenhuma biblioteca de imagem externa
// (cabecalho texto trivial + bytes RGB crus) -- ver doc/vdp-spec.md,
// secao 6 (Fase 2).
#pragma once

#include <cstdint>
#include <string>

#include "../core/vdp_render.h"

namespace vdp {

// Escreve `pixels` (width*height entradas, linha-a-linha) como PPM P6
// binario em `path`. Devolve true em caso de sucesso; em caso de falha
// (arquivo nao pode ser aberto para escrita), devolve false e preenche
// `error` (se nao-nulo) com uma mensagem.
bool WritePpm(const std::string &path, const VdpRgb888 *pixels, int width, int height, std::string *error = nullptr);

} // namespace vdp
