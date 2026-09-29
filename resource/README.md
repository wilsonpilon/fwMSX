# resource/

Codigo-fonte de terceiros usado como material de estudo: o fMSX original
e outros emuladores/ferramentas de MSX cujo codigo e lido, comparado e,
quando faz sentido, traduzido (clean-room) ou usado como referencia
direta de formato.

Nada aqui e compilado como parte do build oficial do fwMSX/msxdisk (ver
`CMakeLists.txt` na raiz); este diretorio e so referencia de leitura.

## Ja estudados/usados (msxdisk, v1.2.0)

- **`DiskUtilities/`** -- `Boot.h`, `DiskUtil.c/.h`, `rddsk.c`, `wrdsk.c`
  (fMSX, autoria mista): `Boot.h` e contribuicao direta de Marat
  Fayzullin (autor do fMSX, que deu aval para esta adaptacao -- ver
  README.md da raiz) e foi incorporado **verbatim** como o setor de boot
  real do MSX-DOS 1 usado pelo `msxdisk`
  (`src/msxdisk/core/msxdos1_boot.cpp`). O restante (`DiskUtil.c/.h`,
  `rddsk.c`, `wrdsk.c`) e de Arnold Metselaar, sob termos proprios
  (restringe distribuicao comercial) -- usado so como referencia de
  formato FAT12, nunca portado linha a linha (ver
  `doc/msxdisk-spec.md`, secao 2).
- **`msxDiskUtil/`** -- reescrita em PureBasic (GPLv3) do mesmo
  utilitario; usado para confirmar, byte a byte, que o setor de boot
  acima e o mesmo usado por uma implementacao independente e testada.
  Nao portado (licenca incompativel com o BSD-3-Clause do fwMSX).
- **`msxdos1/`** (`COMMAND.COM`, `MSXDOS.SYS`) e **`msxdos2/`**
  (`COMMAND2.COM`, `MSXDOS2.SYS`, `UTILS/`, `HELP/`) -- arquivos de
  sistema reais do MSX-DOS 1/2, usados para testar `msxdisk create
  --dos1/--dos2` localmente. O `msxdisk` nunca embute nem redistribui
  esses arquivos; quem usa `--dos1`/`--dos2` aponta o proprio caminho
  local pra eles.

## Ainda so referencia (fases futuras do emulador)

`fMSX/` (Z80/EMULib completos), `fmsxgo/`, `kizuna/`, `msxide/`,
`paleobasic/` -- trazidos para estudo geral do ecossistema MSX, ainda nao
usados diretamente em nenhum codigo do projeto. Relevantes quando o core
de emulacao (CPU Z80, VDP, PSG) comecar, conforme
`doc/SPEC.md`, secao 5.
