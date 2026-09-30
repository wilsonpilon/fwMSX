# Licenciamento de terceiros

O `LICENSE` na raiz (BSD-3-Clause) cobre o **codigo original do fwMSX e
do msxdisk** -- tudo escrito para este projeto. Ele **nao** cobre codigo
adaptado/incorporado de terceiros, que continua sob os termos do autor
original. Este arquivo documenta essas excecoes.

> **Por que nao "GPL" ou "BSD" para o projeto inteiro?** O fMSX original
> tem uma licenca propria e mais restritiva que proibe distribuicao
> comercial (ver abaixo). O fwMSX evolui a partir do fMSX **com o aval do
> proprio Fayzullin para adaptar/estudar seu codigo** -- mas esse aval
> cobre a adaptacao em si, nao uma autorizacao para relicenciar o codigo
> dele sob uma licenca aberta (BSD/GPL/MIT). Por isso os arquivos
> derivados do fMSX continuam com a licenca e o aviso originais dele,
> mesmo dentro deste repositorio; so o codigo genuinamente novo (escrito
> para o fwMSX/msxdisk) e que fica em BSD-3-Clause.

## fMSX (Marat Fayzullin)

Texto original, presente no cabecalho de praticamente todo arquivo do
fMSX (ver `resource/fMSX/`):

> Copyright (C) Marat Fayzullin 1994-2021
> You are not allowed to distribute this software commercially. Please,
> notify me, if you make any changes to this file.

**Arquivos deste repositorio (fora de `resource/`, que e so referencia)
que incorporam codigo do fMSX diretamente**, e portanto continuam sob os
termos acima:

- `src/msxdisk/core/msxdos1_boot.cpp` -- setor de boot do MSX-DOS 1,
  copiado verbatim de `resource/DiskUtilities/Boot.h` (contribuicao
  direta do proprio Fayzullin dentro do fMSX -- ver
  `doc/msxdisk-spec.md`, secao 2, para o raciocinio completo).
- `src/z80/common/z80_state.h` -- estado da CPU (registradores, flags),
  adaptado de `resource/fMSX/Z80/Z80.h`.
- `src/z80/core/z80_opcodes.h` -- enums de opcode (nomes e ordem), de
  `resource/fMSX/Z80/Z80.c` (enums `Codes`/`CodesCB`/`CodesED`).
- `src/z80/core/z80_tables.{h,c}` -- tabelas de ciclos e de flags
  pre-computadas (Sign/Zero, Parity/Zero/Sign, correcao DAA), de
  `resource/fMSX/Z80/Tables.h`.
- `src/z80/core/opcodes_base.h` -- corpo dos opcodes sem prefixo, de
  `resource/fMSX/Z80/Codes.h`.
- `src/z80/core/opcodes_cb.h` -- corpo dos opcodes `CB`, de
  `resource/fMSX/Z80/CodesCB.h`.
- `src/z80/core/opcodes_ed.h` -- corpo dos opcodes `ED` (inclui o patch
  de BIOS `ED FE`), de `resource/fMSX/Z80/CodesED.h`.
- `src/z80/core/opcodes_xx.h` -- corpo dos opcodes `DD`/`FD` (IX/IY), de
  `resource/fMSX/Z80/CodesXX.h`.
- `src/z80/core/opcodes_xcb.h` -- corpo dos opcodes `DD CB`/`FD CB`, de
  `resource/fMSX/Z80/CodesXCB.h`.
- `src/z80/core/z80_core.{h,c}` -- motor de despacho/execucao (loop
  principal, sub-dispatchers por prefixo, tratamento de interrupcao), de
  `resource/fMSX/Z80/Z80.c`. Ver `doc/z80-core-spec.md`, secao 6 (Fase
  1), para os desvios/simplificacoes registrados nesta adaptacao.
- `src/z80/debug/z80_disasm.{h,cpp}` -- desmontador (tabelas de
  mnemonicos e o algoritmo de gabaritos com caracteres-coringa), de
  `resource/fMSX/Z80/Debug.c` (tabelas `Mnemonics*[]` e a funcao
  `DAsm()`). Ver `doc/z80-core-spec.md`, secao 6 (Fase 4), para as tres
  correcoes cosmeticas feitas em relacao ao original (nenhuma afeta
  execucao/timing da CPU, so o texto mostrado pelo desmontador).

Esta lista **sera atualizada conforme o core de emulacao (Z80, VDP, PSG
etc.) for adaptado do fMSX** nas proximas fases (ver `doc/SPEC.md`,
secao 5) -- qualquer arquivo novo que incorporar/adaptar codigo de
`resource/fMSX/` deve:

1. Manter, no topo do arquivo, um comentario citando a origem
   (`Adaptado de fMSX, Copyright (C) Marat Fayzullin -- ver
   LICENSE-THIRD-PARTY.md`).
2. Ser adicionado a lista acima.
3. Ser tratado como **nao-comercial** para fins de distribuicao, ate que
   haja autorizacao explicita em contrario do autor original.

## DiskUtilities (Arnold Metselaar)

`resource/DiskUtilities/DiskUtil.c/.h`, `rddsk.c`, `wrdsk.c` sao de
autoria de Arnold Metselaar, distribuidos dentro do pacote do fMSX sob
termos proprios (tambem restringe distribuicao comercial, pede
notificacao ao autor em caso de mudanca). **Nenhum codigo deste
repositorio fora de `resource/` foi copiado desses arquivos** -- eles
foram usados so como referencia de estudo do formato FAT12/MSX-DOS,
reimplementado de forma clean-room (ver `doc/msxdisk-spec.md`, secao 2).
Mantidos aqui listados por transparencia, nao por incorporacao de codigo.

## msxDiskUtil (GPLv3)

`resource/msxDiskUtil/` e uma reescrita em PureBasic (GPLv3, ver
`resource/msxDiskUtil/LICENSE`) de outro projeto do mesmo autor do
fwMSX. Usado so como referencia/confirmacao de comportamento (ver
`doc/msxdisk-spec.md`), nunca portado -- misturar codigo GPLv3 com o
BSD-3-Clause do fwMSX exigiria relicenciar o projeto inteiro como GPL,
o que nao foi feito.

## Resto de `resource/`

Todo o restante de `resource/` (`fmsxgo/`, `kizuna/`, `msxide/`,
`paleobasic/`, arquivos de sistema do MSX-DOS 1/2, ROMs de BIOS do MSX
etc.) e material de terceiros incluido **apenas para estudo/referencia**,
sob as licencas de seus proprios autores -- ver o aviso em
[resource/README.md](resource/README.md). Nada disso e distribuido como
parte do binario compilado do fwMSX/msxdisk.
