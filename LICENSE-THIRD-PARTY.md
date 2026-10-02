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
- `src/memmap/core/slot_state.{h,c}` -- motor do mapa de memoria MSX
  (topologia de slots/subslots, troca de slot primario/secundario), de
  `resource/fMSX/fMSX/MSX.c` e `resource/fMSX/fMSX/MSX.h` (tabela
  `MemMap[4][4][8]`, `PSL`/`SSL`/`SSLReg`, funcoes `PSlot()`/`SSlot()`,
  trecho de `RdZ80`/`WrZ80` relativo a mapeamento de memoria -- **nao** a
  parte de controlador de disquete dessas funcoes, que fica de fora de
  proposito, ver `doc/memory-map-spec.md`, secao 3.2). Desde a Fase 3,
  tambem adapta a parte de troca de banco (SO ROM, sem SCC/SRAM) de
  `MapROM()` para os mappers `MAP_GEN8`/`MAP_GEN16`/`MAP_KONAMI5`/
  `MAP_KONAMI4`/`MAP_ASCII8`/`MAP_ASCII16` -- `MAP_GMASTER2`/`MAP_FMPAC`/
  `MAP_GUESS` ficam de fora, ver `doc/memory-map-spec.md`, secao 6. Ver
  `doc/memory-map-spec.md`, secao 6 (Fases 1 e 3), para as diferencas
  deliberadas em relacao ao original (permissao de escrita explicita por
  pedaco de 8KB em vez de inferida por regra hardcoded; indexacao direta
  por primario/secundario em vez de um indice de slot de cartucho;
  estado inicial de MegaROM simplificado -- todos os quartos comecam no
  banco 0, em vez da heuristica de assinatura 'AB' do fMSX).
- `src/vdp/core/vdp_state.{h,c}` -- motor "digital" do VDP (registradores,
  protocolo de porta 98h-9Bh, maquina de estados de scanline/
  interrupcao), de `resource/fMSX/fMSX/MSX.c` e `resource/fMSX/fMSX/
  MSX.h` (trechos de `InZ80`/`WrZ80` relativos as portas 98h-9Bh,
  `VDPOut()`, `SetScreen()`, `SetIRQ()`, e a fatia de `LoopZ80()`
  referente a VBlank/HBlank/coincidencia de linha -- **nao** a
  renderizacao de pixel, som, sprites, teclado/joystick/mouse ou o
  motor de comando V9938 dessa mesma funcao, todos fora de escopo ate'
  agora, ver `doc/vdp-spec.md`, secao 4/6). Diferencas deliberadas: uma
  unica pagina de VRAM (16KB, ver `VDP_VRAM_PAGES` em
  `src/vdp/common/vdp_types.h`); deslocamentos de tabela em vez de
  ponteiros crus (o buffer de VRAM e' um array de tamanho fixo dentro do
  struct, nao alocado separadamente como no fMSX). Ver
  `doc/vdp-spec.md`, secao 6 (Fase 0.5/1), para o detalhamento completo.
  Desde a Fase 2, tambem contem a correcao da paleta padrao (`PalInit[16]`
  de `MSX.c`, ~linha 687), portada para dentro de `vdp_reset()`.
- `src/vdp/core/vdp_render.{h,c}` -- decodificacao de pixel para SCREEN
  0/1/2, de `resource/fMSX/fMSX/Common.h` (`RefreshLine0()`,
  `RefreshLine1()`, `RefreshLine2()`) -- **nao** a borda/overscan
  (`RefreshBorder()`), sprites (`Sprites()`), `ScreenON`/blank ou
  `FontBuf`/`MSX_FIXEDFONT`, todos fora de escopo (ver
  `doc/vdp-spec.md`, secao 6, Fase 2, para o detalhamento completo das
  simplificacoes e das dimensoes de framebuffer resultantes).
- `src/vdp/core/vdp_sprites.{h,c}` -- sprites de SCREEN 1/2/3, de
  `resource/fMSX/fMSX/Common.h` (`Sprites()`) e `resource/fMSX/fMSX/MSX.c`
  (`CheckSprites()` e o trecho de status de sprite de `LoopZ80()`) --
  **nao** `ColorSprites()` (sprites de modo 2, SCREEN 4-8, Fase 4). Ver
  `doc/vdp-spec.md`, secao 6, Fase 3.
- `src/ppi/core/ppi_state.{h,c}` -- chip i8255 (`resource/fMSX/EMULib/
  I8255.{h,c}`: `Reset8255()`/`Write8255()`/`Read8255()`) e o tratamento
  das portas `A8h`-`ABh` e da matriz de teclado de `resource/fMSX/fMSX/
  MSX.c` (`InZ80()`/`WrZ80()`, `KeyState[]`). A tabela de nomes de tecla,
  `PpiDevice` (C++), `key_matrix.f90` (Fortran) e `key_count.asm`
  (Assembly) sao codigo original do fwMSX (BSD-3-Clause); as coordenadas
  da matriz sao fato de hardware, conferidas contra `Keys[]` de `MSX.c`.
  **Nao** inclui `PPIOut()` (som de click/rele). Ver `doc/ppi-spec.md`.
- `src/psg/core/psg_state.{h,c}` -- as mascaras de registrador de
  `Write8910()` e o protocolo de portas (`WrCtrl8910()`/`WrData8910()`/
  `RdData8910()`, `RegInit[]` de `Reset8910()`) de `resource/fMSX/EMULib/
  AY8910.{h,c}`, e os casos `A0h`-`A2h` de `InZ80()`/`OutZ80()` de
  `resource/fMSX/fMSX/MSX.c`. Os geradores (tom, ruido, envelope) e a
  geracao de amostras PCM sao codigo original do fwMSX -- o fMSX nao gera
  amostras, repassa freq/volume para `Sound()`. A tabela de volume
  (`volume_table.f90`, Fortran), `PsgDevice` e `wav_writer` (C++) sao
  codigo original (BSD-3-Clause). **Nao** inclui joystick/mouse em R14/R15
  (so' o "sem joystick" de `InZ80()`), nem SCC/OPLL/Drum(). Ver
  `doc/psg-spec.md`.

- `src/memmap/cpp/rom_guess.{h,cpp}` -- a heuristica de `GuessROM()` de
  `resource/fMSX/fMSX/MSX.c` (contagem de `LD (nnnn),A` nos enderecos de
  registrador de banco de cada mapper). **Nao** inclui a consulta a
  `CARTS.CRC`/`CARTS.SHA` do fMSX. Ver `doc/memory-map-spec.md`.
- `src/fdc/core/fdc_state.{h,c}` -- a controladora WD1793/WD2793 de
  `resource/fMSX/EMULib/WD1793.{h,c}` (comandos tipo 1-4, protocolo DRQ/IRQ e o
  "watchdog" de leitura do registrador READY) e o mapeamento `7FF8h`-`7FFFh`
  de `resource/fMSX/fMSX/MSX.c`. A imagem e' um `.dsk` cru (`FdcDisk`) em vez
  do `FDIDisk` do fMSX; `DiskImage` e `FdcDevice` (C++) e o `SlotMmio` sao
  codigo original (BSD-3-Clause). **Nao** inclui READ/WRITE TRACK (tambem nao
  suportados no fMSX) nem formatos de imagem alem de `.dsk`. Ver `doc/fdc-spec.md`.
- **miniaudio** (<https://github.com/mackron/miniaudio>, David Reid) --
  baixado em tempo de configuracao (FetchContent) e compilado em
  `src/audio/audio_output.cpp` para a saida de audio ao vivo. Dominio
  publico / MIT-0 (a escolha do licenciado); so' o dispositivo de saida e'
  usado. Nao e' codigo do fMSX. Ver `doc/audio-spec.md`.

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
