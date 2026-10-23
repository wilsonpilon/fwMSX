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
  (so' o "sem joystick" de `InZ80()`), nem OPLL/Drum(). Ver
  `doc/psg-spec.md`.
- `src/scc/core/scc_state.{h,c}` -- as regras de registrador do chip SCC de
  `resource/fMSX/EMULib/SCC.{h,c}` (`WriteSCC()`/`WriteSCCP()`/`ReadSCC()`,
  espelhamento `B0h`-`BFh`, mascara do mixer, ondas compartilhadas dos canais 3 e 4)
  e a ativacao pelo cartucho de `resource/fMSX/fMSX/MSX.c` (escritas em `9000h`/`8000h-9FFFh`).
  A geracao de amostras PCM (fase Q16, reamostragem por ciclos de Z80), a soma de canal em
  Assembly (`src/scc/asm/render_channel.asm`), a tabela de volume linear (Fortran) e o
  `SccDevice` (C++) sao codigo original (BSD-3-Clause). Ver `doc/scc-spec.md`.

- `src/fm/core/ym2413_patches.h` -- os 15 timbres prontos do OPLL, copiados da tabela
  `Synth2413` de `resource/fMSX/EMULib/YM2413.c` (fMSX, Copyright (C) Marat Fayzullin
  1996-2021; uso para estudo autorizado pelo autor). Sao dados do chip, nao codigo. O motor
  do OPLL (`src/fm/core/ym2413_state.c`) e' original do fwMSX; o `YM2413.c` do fMSX (um
  wrapper que toca instrumentos MIDI) nao foi portado. O mapper `MAP_FMPAC`
  (`src/memmap/core/slot_state.c`) segue o protocolo de `MAP_FMPAC` de
  `resource/fMSX/fMSX/MSX.c`, e o FM-PAC usa a ROM `resource/fMSX/FMPAC.ROM`. Ver
  `doc/fm-spec.md`.

- `src/vdp/core/vdp_render.c` (SCREEN 10-12 do V9958) -- base de `RefreshLine10()`/`RefreshLine12()` e
  `YJKColor()` de `resource/fMSX/fMSX/Common.h` (decodificacao de crominancia, regra de YAE).
  O scroll pixel a pixel, o arredondamento do azul, a expansao de 5 bits e a mascara da esquerda
  sao do fwMSX, com o comportamento conferido no openMSX (GPL, so' como referencia: nenhum codigo
  copiado). Ver `doc/msx2p-spec.md`.
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
- `src/vdp/core/vdp_cmd.{h,c}` -- o motor de comandos do V9938 de `resource/fMSX/fMSX/V9938.c`
  (Copyright Marat Fayzullin; reescrito por **Alex Wulms**, ver o cabecalho original:
  execucao "em paralelo" e temporizacao). Adaptado para operar sobre `VdpState`, com um
  desvio (o bit TR comeca limpo a cada comando). Ver `doc/msx2-spec.md`.
- `src/vdp/core/vdp_render.{h,c}` e `vdp_sprites.{h,c}` agora tambem adaptam `RefreshLine3..8`,
  `RefreshLineTx80`, `ColorSprites()` (`resource/fMSX/fMSX/Common.h`, `Wide.h`) e o `CheckSprites()`
  de `MSX.c`. A paleta fixa de SCREEN 8 (`BPal[]`) vem de `resource/fMSX/fMSX/Unix/Unix.c`.
- `src/memmap/cpp/ram_mapper.h` e `src/rtc/rtc_device.h` seguem o comportamento dos casos
  `FCh`-`FFh` e `B4h`/`B5h` de `InZ80()`/`OutZ80()` e de `RTCIn()` (inclusive os valores padrao da
  CMOS, `RTCInit`) de `resource/fMSX/fMSX/MSX.c`; o codigo e' original (BSD-3-Clause).
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

## Bibliotecas baixadas na configuracao do build (FetchContent)

Nenhuma e' copiada para este repositorio; o CMake baixa e compila (todas permissivas):
**SQLite** (dominio publico; o `.msxproject` e o banco de ROMs), **CLI11** (BSD-3-Clause; linha de comando), **replxx** (BSD-3-Clause; o console),
**FTXUI** (MIT; a TUI), **Dear ImGui** (MIT) e **GLFW** (zlib; a janela grafica), **miniaudio** (dominio publico / MIT-0; ver abaixo), **miniz** (MIT; ver abaixo)
e **Lua** (MIT; abaixo).

## Downloads (janela Downloads e `fwmsx --download`, 1.43.17)

O fwMSX **nao distribui** o que baixa por essa janela; o usuario baixa dos sites dos autores, um arquivo por vez (doc/download-spec.md):

- **File Hunter** (`download.file-hunter.com`, Arnaud de Klerk): arquivo de MSX de acesso livre. O proprio site pede que nao se baixe em massa
  (ban de IP) e diz ter feito o possivel para nao incluir titulos comerciais disponiveis; a janela baixa so' o arquivo clicado, com intervalo
  minimo, e usa o `allfiles.txt` do site como indice.
- **Konamiman** (`github.com/Konamiman/MSX`, `SRC/NBASIC`): NestorBASIC (`NBASIC.BIN`, `NBASIC.DAT`...), SEE e NPLAYER. O repositorio **nao declara
  licenca**; por isso o `NBASIC.BIN` deixou de ir no repositorio e nos pacotes e passou a ser baixado. O `nbasic.dmx` (as rotinas `.NB_*` do Basic
  Dignified, vindas do msxIDE) continua nos pacotes.

## Lua (`lua-5.4.7`, baixado na configuracao do build)

O comando `lua` do console (doc/script-spec.md) usa o interpretador Lua
5.4.7, copyright PUC-Rio, sob a licenca MIT. O CMake baixa o tarball
oficial de https://www.lua.org/ftp/ (SHA-256 fixo) e o compila como
biblioteca estatica; o fonte do Lua nao e' copiado para este repositorio.

## third-party/ (2026-10-23)

Desde 2026-10-23 o material de apoio do autor mora em **`third-party/`** (raiz do repositorio), nao mais em `resource/`. Estao versionados:
`Konpass`, `n80` e `msxbas2rom` (software de terceiros mantido por enquanto so' para consulta, com seus proprios `LICENSE`; os binarios compilados
dos exemplos nao sobem) e os projetos do proprio autor abaixo (limpos de midia, binarios e SDKs de terceiros). O resto de `third-party/` fica so' no disco
do autor (nao rastreado). Tudo o que e' de terceiros sai antes de a licenca unica entrar em vigor.

### Assets e referencias em third-party/ (2026-10-23, provisorio)

Por decisao do autor entram no git, **por enquanto**, para sincronizar entre computadores e te-los a mao: `fx80` (projeto proprio, usado em partes no fwMSX;
contem tambem a digitalizacao do manual da Epson FX-80, de 1983, material de terceiros), `fontes` (fontes TrueType de matriz de pontos), os assets
`alfabetos-aquarela`, `alfabetos-graphos`, `layout-graphos`, `padrao-aquarela`, `shapes-*`, `telas-*`, `graphos`, `superx` (alfabetos, telas, shapes e
layouts do Graphos e do Aquarela) e as figuras dos livros (`redbook_images`, `th2handbook_images`). **Os assets irao para um site proprio e passarao a ser
baixados** (janela Downloads) em vez de ficarem no repositorio. A autoria e a licenca de cada grupo (Graphos, Aquarela, as fontes, os livros) ainda nao
estao registradas aqui: preencher quando o autor confirmar. Continuam so' no disco do autor, nao rastreados: `openmsx`, `fMSX`, `megasm`, `nestor`, `NBASIC`.

## fmsxgo, paleobasic, msxide e kizuna (projetos proprios do autor)

`resource/fmsxgo/`, `resource/paleobasic/`, `resource/msxide/` e `resource/kizuna/` sao **projetos do proprio autor do fwMSX**, mantidos so' como
referencia (editores de tela, sprites, som, draw...) para as versoes novas em C/C++/ASM/Fortran, e serao eliminados ao final do projeto, ficando uma
unica licenca (BSD-3-Clause). O que cada um **carrega de terceiros** (SDKs como MSXgl e Fusion-C, discos e ROMs de jogos, BIOS, copias de outros
projetos) nao e' do autor e continua sob as licencas de origem; o autor esta' fazendo uma limpeza manual dessas pastas (doc/media-review.md, 4d).

## Resto de `resource/`

Todo o restante de `resource/` (`fmsxgo/`, `kizuna/`, `msxide/`,
`paleobasic/`, arquivos de sistema do MSX-DOS 1/2, ROMs de BIOS do MSX
etc.) e material de terceiros incluido **apenas para estudo/referencia**,
sob as licencas de seus proprios autores -- ver o aviso em
[resource/README.md](resource/README.md). Nada disso e distribuido como
parte do binario compilado do fwMSX/msxdisk. **Excecao deliberada**: a
pasta `emulator/` (arquivos do MSX-DOS 1 e 2 copiados de `resource/` pela
build, e os discos de sistema gerados a partir deles, usados pelo `obi`)
**vai nos pacotes de distribuicao** (zip e tar.gz) por decisao do autor do
fwMSX (2026-10-22): sao arquivos amplamente disponiveis, de terceiros
(Microsoft / MSX Association / The New Image), e serao retirados se algum
titular dos direitos pedir.

## Banco de ROMs (`src/romdb/`, nao lancado)

- **miniz 3.0.2** (Rich Geldreich; licenca MIT ou dominio publico): biblioteca de ZIP baixada
  pelo CMake (`FetchContent`) e compilada no projeto (`miniz.c`, `miniz_tdef.c`, `miniz_tinfl.c`,
  `miniz_zip.c`). Nao e' codigo do fwMSX.
- **curl** (programa externo, nao incluido no repositorio): usado para baixar os arquivos. Nao ha codigo
  dele no fwMSX.
- **Dados baixados pelo usuario, nao redistribuidos**: o pacote do fMSX 6.0 (`fMSX60-Windows-bin.zip`, de
  `fms.komkon.org`), o "Full Set System ROMs for OpenMSX" (`download.file-hunter.com`) e o banco do
  Vampier (`romdb.vampier.net`, `sql-msxromdb.zip`). As ROMs e o `CARTS.SHA` ficam na pasta `roms/`, que
  esta fora do git. Os direitos sobre essas ROMs sao de seus titulares; o fwMSX so' le e organiza os arquivos.

## openMSX (`resource/openMSX/`, provisorio)

- **openMSX**: licenca GPL, versao 2 ou posterior (`resource/openMSX/doc/GPL.txt`). Material de
  estudo, fora do build. Nao ha codigo dele no fwMSX. Sera retirado do repositorio quando o fwMSX
  estiver pronto.

## asMSX (`resource/asMSX/`, `resource/asmsx.md`, provisorio)

- **asMSX** 1.2.0: licenca GPLv3 (`resource/asMSX/LICENSE.md`). O fonte e o manual ficam em `resource/` so' como
  material de estudo, fora do build. O montador do fwMSX (`src/asm/`) foi escrito do zero a partir do manual e do
  comportamento observado do `asmsx.exe` como caixa-preta (`tools/asmdiff/`, usado so' para gerar os casos de
  teste); nao ha codigo do asMSX no fwMSX. O `asmsx.exe` (oraculo) nao e' versionado. Sera retirado do
  repositorio quando o fwMSX estiver pronto.

## O repositorio e' publico (revisao de 2026-10-22)

O repositorio `github.com/wilsonpilon/fwMSX` e' **publico**, e o que o git rastreia fica disponivel a quem o clonar, historico incluido -- mesmo o que
os pacotes de release nao levam. A frase acima ("nada disso e' distribuido como parte do binario") vale para os **pacotes**; o levantamento do que
o **repositorio** publica (ROMs de jogos em `tests/media/`, o banco `dist/roms/`, discos e ROMs dentro de `resource/`, pacotes antigos em `dist/`...)
e as opcoes de tratamento estao em [doc/media-review.md](doc/media-review.md). **Aplicado em 2026-10-22**: `tests/media/`, `dist/roms/`, os pacotes antigos de `dist/`, `*.sst` e as capturas soltas deixaram de ser rastreados e estao no `.gitignore` (continuam no disco do autor); o historico nao foi reescrito, e `resource/` segue como estava, a decidir.

## Pacote OCM-PLD / SDBIOS do KdL (`third-party/ocm/` e `dist/roms/ocm/`, 2026-10-24)

O usuario baixou o pacote **OCM-PLD v3.9.2 e SDBIOS** (KdL; o link do SDBIOS e' enviado por e-mail pelo autor) para `third-party/ocm/` (788 MB; **no `.gitignore`**) e copiou o que o
emulador usa (ROMs, utilitarios, `msxtools`, o fonte do driver do Nextor) para `dist/roms/ocm/` (tambem ignorado: `dist/roms/` nao entra no git). **Nada disso e' redistribuido** com o
fwMSX nem publicado neste repositorio.
- As **ROMs** (BIOS A1WSX, sub-ROM, MSX-Music, Kanji, XBASIC, Nextor + driver MegaSDHC...) sao material de terceiros (a base e' do Panasonic FS-A1WSX, modificada pelo KdL; o Nextor e' do
  Konamiman): uso pessoal/local.
- O **VHDL/Verilog** do OCM-PLD (`emsx_top.vhd`, `megasd.v`, `swioports.vhd`...) tem licenca que proibe uso comercial e exige manter o aviso; foi lido **so' como referencia** para
  o mapa de slots e as portas, e o codigo do fwMSX (`src/hd`, `src/ocm`) e' **original** (BSD-3-Clause).
- A documentacao de como o usuario baixa o pacote e a opcao de download (modo INTERNAL) ainda serao escritas.

## Arte propria

`fwMSX-splash.png` (a imagem de abertura) e o icone do programa sao **arte do proprio projeto** (informacao do autor, 2026-10-22), sob o BSD-3-Clause
do fwMSX.

## Politica de distribuicao de midias de terceiros (2026-10-06)

Hoje o repositorio e' pessoal. ROMs, discos, fitas e outras midias de terceiros (fMSX,
file-hunter, Vampier, Microsol/DDX, TSX e CAS) podem ser versionadas e baixadas livremente
pelo autor, sem restricao.

**Aviso para a liberacao publica (a fazer):** antes de liberar o fwMSX ao publico, cada
arquivo de midia de terceiros precisa de revisao. Se o detentor dos direitos de algum arquivo
estiver em desacordo com a distribuicao, o arquivo sera removido do repositorio e dos pacotes,
mediante pedido. O mesmo aviso deve constar no README da liberacao publica.
