# fwMSX -- Manual de compilacao e execucao

> Documento ainda simples: por enquanto so cobre como compilar e executar
> o esqueleto multi-linguagem da fase atual. Vai crescer conforme o
> emulador for tomando forma (ver [SPEC.md](SPEC.md)).

## Pre-requisitos

- Windows 10/11.
- [MSYS2](https://www.msys2.org/) instalado em `C:\msys64`.
- Toolchain **UCRT64** do MSYS2, com os seguintes pacotes:

  ```bash
  pacman -S \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-gcc-fortran \
    mingw-w64-ucrt-x86_64-nasm \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja
  ```

  Todo o toolchain (gcc, g++, gfortran, nasm, cmake, ninja) deve vir do
  **mesmo** grupo UCRT64 -- evita misturar versoes/ABIs de compiladores
  diferentes na mesma build.

- **Acesso à internet na primeira compilação**: o `msxdisk` (ver seção
  dedicada abaixo) busca CLI11, replxx, FTXUI, Dear ImGui, GLFW e SQLite
  via CMake `FetchContent` -- baixados uma vez e cacheados em `build/`.
  Builds seguintes (sem apagar `build/`) não precisam de rede.

## Compilar

### Opcao 1 -- script pronto (recomendado)

No PowerShell, a partir da raiz do projeto:

```powershell
.\build.ps1
```

O script:
1. Coloca `C:\msys64\ucrt64\bin` no inicio do `PATH` (so para o processo
   do script, sem alterar o `PATH` do sistema);
2. Configura o build com CMake + Ninja em `build\`;
3. Compila e gera `dist\fwMSX.exe`;
4. Le a versao corrente em `src\common\version.h` e empacota
   `dist\fwMSX-X.Y.Z.zip` com o executavel, `README.md`, `LICENSE`,
   `doc\MANUAL.md` e `doc\RELEASE.md`.

### Opcao 2 -- manual (MSYS2 UCRT64 shell)

Abra o **"MSYS2 UCRT64"** (nao o MSYS2 normal, nem o MINGW64) a partir do
menu iniciar, va ate a pasta do projeto e rode:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

O executavel fica em `dist/fwMSX.exe`.

### Linux

O projeto tambem compila em Linux (gcc/g++/gfortran/nasm/cmake/ninja
"de sistema", sem MSYS2) -- **validado de verdade numa maquina Linux
(WSL2) na v1.4.1**: build completo + `ctest` com as 328 verificacoes
passando, incluindo a branch `elf64`/SysV AMD64 do `.asm` dual-ABI do
nucleo Z80 (`src/z80/asm/block_ops.asm`), que no desenvolvimento
original (Windows) so podia ser montada (`nasm -f elf64`), nunca
linkada/executada de verdade. Use `build.sh` (equivalente ao
`build.ps1`):

```bash
sudo apt install build-essential gfortran nasm cmake ninja-build   # Debian/Ubuntu
./build.sh              # configura, compila, roda ctest e empacota
./build.sh --no-gui     # sem Dear ImGui/GLFW/OpenGL3 (evita libs de X11/OpenGL)
```

Executaveis em `dist/fwMSX` e `dist/msxdisk`; pacote em
`dist/fwMSX-X.Y.Z-linux.tar.gz`.

**Se `build.sh` estiver rodando num checkout compartilhado com Windows**
(ex.: mesmo repositorio acessado como `/mnt/c/...` de dentro do WSL) e
o CMake reclamar de `CMakeCache.txt` de outro diretorio: isso e'
esperado e ja' tratado -- `build.sh` usa `build-linux/` (nao `build/`,
que e' onde o `build.ps1` do Windows grava o cache dele), exatamente
para evitar esse conflito. Se ver esse erro mesmo assim, confira se
esta' numa copia antiga do `build.sh` (versoes anteriores usavam
`build/` e colidiam de verdade).

## Executar

```powershell
.\dist\fwMSX.exe
```

Saida esperada (com os valores default de `src\common\version.h`):

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.1.1]
----------------------------------------
Loading module... CPP [v 1.1.1]
Loading module...C [v 1.1.1]
Loading module...Assembly [v 1.1.1]
Loading module Fortran [v 1.1.1]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Parametros opcionais

`fwMSX.exe` aceita, na linha de comando, o nome do produto e a versao a
serem exibidos (sobrescrevendo os defaults de `version.h`):

```powershell
.\dist\fwMSX.exe fwMSX 2 0 5
```

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 2.0.5]
----------------------------------------
Loading module... CPP [v 2.0.5]
Loading module...C [v 2.0.5]
Loading module...Assembly [v 2.0.5]
Loading module Fortran [v 2.0.5]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

## Distribuicao

`dist\fwMSX.exe` e linkado estaticamente (`-static -static-libgcc
-static-libstdc++ -static-libgfortran`) e depende apenas do **UCRT**
(`ucrtbase.dll`), nativo do Windows 10 versao 1607 ou mais recente -- ou
seja, roda em outra maquina Windows sem precisar instalar o MSYS2 nem
copiar DLLs. O ZIP gerado em `dist\fwMSX-X.Y.Z.zip` contem tudo o que e
necessario para rodar (executavel + documentacao + licenca).

## msxdisk -- utilitario de imagens de disco MSX

Documentacao completa (arquitetura, fases, decisoes) em
[msxdisk-spec.md](msxdisk-spec.md). Aqui so o essencial pra usar.

`.\build.ps1` tambem gera `dist\msxdisk.exe` -- um unico executavel com
quatro modos:

```powershell
# CLI one-shot
.\dist\msxdisk.exe create disco.dsk
.\dist\msxdisk.exe list disco.dsk
.\dist\msxdisk.exe add disco.dsk arquivo.bas

# Shell interativo (estilo FTP) -- sem argumentos, ou --cli
.\dist\msxdisk.exe

# TUI (estilo Norton Commander/XTree)
.\dist\msxdisk.exe --tui disco.dsk

# GUI (Dear ImGui, visual moderno)
.\dist\msxdisk.exe --gui disco.dsk
```

De dentro do shell interativo, os comandos `tui`/`call tui` e `gui`/
`call gui` abrem a TUI/GUI sem sair do processo.

O mesmo utilitario tambem esta embutido no `fwMSX.exe`:

```powershell
.\dist\fwMSX.exe --msxdisk create disco.dsk
.\dist\fwMSX.exe --msxdisk --tui disco.dsk
```

Configuracao, temas e metadados de imagem ficam em
`%USERPROFILE%\.msxdisk\config.sqlite3` (compartilhado entre TUI e GUI).

## Nucleo Z80 -- depurador embutido (`--z80dbg`)

Documentacao completa (arquitetura, fases, decisoes) em
[z80-core-spec.md](z80-core-spec.md). Aqui so o essencial pra usar.

Ainda **nao existe uma maquina MSX de verdade** (sem VDP/PSG/mapa de
memoria) -- `--z80dbg` abre um REPL de depuracao do `Z80Cpu` sobre uma
RAM plana de 64KB, so pra inspecionar/exercitar o core isoladamente:

```powershell
.\dist\fwMSX.exe --z80dbg
```

Dentro do REPL (`help` lista tudo):

```
reset                 reseta a CPU
regs                  mostra registradores/flags/IFF
step [n]              executa n instrucoes (default 1), uma por vez
run [ciclos]          executa ate esgotar o orcamento (default 1000) ou breakpoint
break/clear/breaks    gerencia breakpoints
mem/peek/poke         inspeciona/edita a RAM
load <arquivo> <end>  carrega um binario cru na RAM
fill <end> <tam> <b>  preenche memoria
disasm [end] [n]      desmonta n instrucoes (default: PC atual, 10)
```

Enderecos/numeros aceitam decimal, `0x`-hex ou `$`-hex. Exemplo rapido
(carrega `LD HL,1234h` / `HALT` a mao e desmonta):

```
fwMSX - depurador do nucleo Z80 (RAM plana de teste, sem maquina MSX ainda).
z80dbg> poke 0x0000 0x21
z80dbg> poke 0x0001 0x34
z80dbg> poke 0x0002 0x12
z80dbg> poke 0x0003 0x76
z80dbg> disasm 0x0000 2
0000: 21 34 12     LD HL,1234h
0003: 76           HALT
```

### Mapa de memoria real (`--z80dbg --slots`)

Documentacao completa em [memory-map-spec.md](memory-map-spec.md). Por
cima da RAM plana acima, `--slots` liga o mapa de memoria MSX de
verdade -- 4 slots primarios x 4 secundarios x 8 paginas de 8KB, fiel ao
fMSX, com comandos que enxergam qualquer slot **independente** do que a
CPU ve agora:

```
slots                             lista as 16 combinacoes (vazio/RAM/ROM)
pages                              o que esta visivel para a CPU agora
slotmem/slotpeek/slotpoke <p> <s> ...  inspeciona um slot especifico
loadrom <p> <s> <arquivo> [mapper]     carrega ROM (plana ou MegaROM)
```

Mappers MegaROM suportados no `loadrom`: `gen8`, `gen16`, `konami5`,
`konami4`, `ascii8`, `ascii16` (so a troca de banco de ROM -- sem SCC/
SRAM, ver `memory-map-spec.md` secao 6 pro motivo). Sem o parametro,
carrega como ROM plana (sem bank-switch), o mesmo usado para a BIOS.

`--z80dbg --slots [rom]` aceita um caminho de ROM opcional logo depois
de `--slots`, carregado automaticamente no slot 0:0 na abertura -- util
pra nao ter que digitar `loadrom` toda vez. Exemplo rodando a **BIOS
MSX1 real** (ja incluida em `resource/` para estudo -- ver
`resource/README.md`):

```powershell
.\dist\fwMSX.exe --z80dbg --slots resource\fMSX\ROMs\MSX.ROM
```

```
ROM de boot carregada em 0:0: resource/fMSX/ROMs/MSX.ROM
z80dbg> reset
z80dbg> run 5000
parado: orcamento de ciclos esgotado (ciclos consumidos: 5003, PC=0365)
```

Isso e' codigo real de BIOS executando no nucleo Z80 do fwMSX -- a CPU
esta rodando software MSX de verdade (ver a secao seguinte pro VDP; a
BIOS real ainda trava mais na frente esperando teclado/PPI, que ainda
nao existe).

### PPI e teclado (`--z80dbg --slots --ppi`)

Documentacao completa em [ppi-spec.md](ppi-spec.md). `--ppi` liga o i8255
(portas `A8h`-`ABh`): o slot primario passa a mudar pelo PPI (como no
hardware) e ha' uma matriz de teclado de 11 linhas. **Requer `--slots`**
(avisa e ignora sem ele). Com uma ROM de BIOS, monta o layout MSX1 padrao:
RAM de 64KB no slot `3:2` e regras de subslot do MSX1.

```
ppiregs                  modo das portas, slot primario, linha do teclado, LED/motor
keys                     matriz de teclado + teclas pressionadas
keydown <tecla>...       pressiona (a-z, 0-9, shift, ctrl, enter, space, f1-f5, pad0-pad9...)
keyup <tecla>...|all     solta
```

```powershell
.\dist\fwMSX.exe --z80dbg --slots resource\fMSX\ROMs\MSX.ROM --vdp --ppi
```

### VDP real (`--z80dbg --slots --vdp`)

Documentacao completa em [vdp-spec.md](vdp-spec.md). Por cima do mapa
de memoria, `--vdp` liga o VDP (TMS9918/V9938) de verdade -- registradores,
VRAM, protocolo de porta `98h`-`9Bh`, e a maquina de estados que gera as
interrupcoes de VBlank/HBlank que software MSX real depende para
funcionar. **Requer `--slots`** (avisa e ignora `--vdp` se usado sem
`--slots`). Ainda sem sprites, modos MSX2 ou janela grafica -- so os
modos SCREEN 0/1/2, exportaveis como imagem (ver `vdpshot` abaixo).

```
vdpregs                  registradores (R#0-R#46) + status + modo de tela
vdpmem/vdppeek/vdppoke <end> ...   inspeciona/edita a VRAM
vdpstep [n]               avanca a maquina de estados de scanline manualmente
vdpshot <arq.ppm> [ini] [fim]      exporta o frame atual como imagem PPM
```

Exemplo (poke manual de um caractere em SCREEN 0 e exportacao):

```powershell
.\dist\fwMSX.exe --z80dbg --slots --vdp
```

```
z80dbg> vdppoke 0 0x41
z80dbg> vdpshot tela.ppm 0 0
escrito tela.ppm (240x1, modo de tela 0)
```

O arquivo PPM (`P6`, binario) pode ser aberto em qualquer visualizador
de imagem que suporte o formato, ou inspecionado byte a byte -- ainda
para ver a maquina rodando numa janela, use `fwmsx --msx` (secao abaixo).

## Emulador MSX1 numa janela (`--msx`)

```powershell
.\dist\fwMSX.exe --msx                      # BIOS padrao (resource/fMSX/ROMs/MSX.ROM)
.\dist\fwMSX.exe --msx --bios MSX.ROM --cart jogo.rom
.\dist\fwMSX.exe --msx --cart megarom.rom ascii8   # MegaROM (gen8 gen16 konami5 konami4 ascii8 ascii16)
.\dist\fwMSX.exe --msx --disk msxdos1.dsk    # MSX-DOS 1.8 (interface de disco + disquete em A:)
.\dist\fwMSX.exe --msx --disk msxdos1.dsk --disk-ro   # disco somente leitura (o MSX-DOS nao grava)
.\dist\fwMSX.exe --msx --msx2                # MSX2 (MSX2.ROM + MSX2EXT.ROM): BASIC 2.1, SCREEN 0-8
.\dist\fwMSX.exe --msx --msx2 --cart jogo2.rom   # cartucho MSX2 (ex.: Firebird)
.\dist\fwMSX.exe --msx --mute                # sem audio
.\dist\fwMSX.exe --msx --frames 400 --keys "print 1234|" --shot tela.ppm   # sem janela
```

Abre uma janela com o MSX BASIC rodando em tempo real e som ao vivo. Com `--disk`
a maquina ganha a interface de disquete e boota o MSX-DOS; **as gravacoes vao
direto para o arquivo `.dsk`** (faca backup) -- ver [fdc-spec.md](fdc-spec.md).
Joystick: setas + Z/Espaco (A) + X (B), ou gamepad. Teclado
posicional (layout US): Alt esquerdo = GRAPH, Alt direito = CODE, End =
SELECT, Pause = STOP, F11 = tela cheia; menu **Maquina** (Reset, Pausar),
**Exibir** e **Som** (mudo/volume). Cartuchos de ROM plana ate' 32KB ou
MegaROM com mapper, no slot 1. Limites: so' SCREEN 0/1/2, sem joystick nem
disco -- ver [machine-spec.md](machine-spec.md) e [audio-spec.md](audio-spec.md).

### Compilar sem a GUI (sem GLFW/OpenGL)

A GUI (Dear ImGui + GLFW + OpenGL3) vem ligada por padrao. Pra compilar
so a versao console (CLI/shell/TUI), sem essa dependencia:

```powershell
cmake -S . -B build -G Ninja -DFWMSX_MSXDISK_GUI=OFF
cmake --build build
```

`--gui`/`call gui` nessa build so avisam que a GUI nao foi compilada. Para
compilar sem audio: `-DFWMSX_AUDIO=OFF` (o emulador roda mudo).

## Problemas comuns

- **`cmake` ou `gcc` errado sendo usado / erros estranhos de link**: quase
  sempre e outro toolchain (ex.: FPC, Visual Studio) na frente do UCRT64
  no `PATH`. Use `.\build.ps1` (que forca o UCRT64 na frente) ou abra o
  shell **MSYS2 UCRT64** dedicado.
- **Saida do modulo Fortran aparece fora de ordem**: se voce mexer em
  `src/fortran/init_fortran.f90`, mantenha o `FLUSH(output_unit)` apos
  o `PRINT` -- o runtime do Fortran usa buffer de E/S proprio, separado
  do `std::cout`/`printf` do C/C++ (detalhes em
  [SPEC.md](SPEC.md#32-notas-de-implementacao-para-quem-retomar-o-projeto)).
