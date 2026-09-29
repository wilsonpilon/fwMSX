//
// Setor de boot real do MSX-DOS 1 (disco DS/DD de 720KB).
//
// Bytes identicos a resource/DiskUtilities/Boot.h -- fMSX, Copyright (C)
// Marat Fayzullin 1994,1995. Diferente do restante de
// resource/DiskUtilities/ (DiskUtil.c/h, rddsk.c, wrdsk.c, de autoria de
// Arnold Metselaar sob termos proprios, tratado como clean-room only --
// ver doc/msxdisk-spec.md, secao 2), Boot.h e contribuicao direta do
// proprio autor original do fMSX. O fwMSX evolui a partir do fMSX com o
// aval desse mesmo autor para adaptar/estudar seu codigo (ver README.md
// do projeto), entao incorporar este setor de boot verbatim esta dentro
// do escopo autorizado -- ao contrario do bootstrap generico que o
// msxdisk gravava antes, que nao continha o bootloader real e por isso
// os discos criados nao davam boot corretamente.
//
// IMPORTANTE (licenciamento): esse aval e para adaptar/usar o codigo, nao
// uma autorizacao pra relicencia-lo. Este arquivo (so este, nao o resto
// do msxdisk) continua sob a licenca original do Fayzullin -- nao-
// comercial, aviso ao autor em caso de mudanca -- e NAO faz parte do BSD
// 3-Clause do restante do projeto. Ver LICENSE-THIRD-PARTY.md na raiz.
//
// Confirmacao independente: resource/msxDiskUtil/MSXDisk.pbi (reescrita
// em PureBasic, testada e aprovada pelo autor deste projeto) usa
// exatamente este mesmo array de 512 bytes como seu boot sector default.
//
#pragma once

#include <cstdint>

namespace msxdisk {

extern const uint8_t kMsxDos1BootSector720KB[512];

} // namespace msxdisk
