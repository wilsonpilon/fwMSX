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
"de sistema", sem MSYS2) -- em particular para validar de verdade a
branch `elf64`/SysV AMD64 do `.asm` dual-ABI do nucleo Z80
(`src/z80/asm/block_ops.asm`), que no desenvolvimento original (Windows)
so pode ser montada (`nasm -f elf64`), nunca linkada/executada. Use
`build.sh` (equivalente ao `build.ps1`):

```bash
sudo apt install build-essential gfortran nasm cmake ninja-build   # Debian/Ubuntu
./build.sh              # configura, compila, roda ctest e empacota
./build.sh --no-gui     # sem Dear ImGui/GLFW/OpenGL3 (evita libs de X11/OpenGL)
```

Executaveis em `dist/fwMSX` e `dist/msxdisk`; pacote em
`dist/fwMSX-X.Y.Z-linux.tar.gz`.

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

### Compilar sem a GUI (sem GLFW/OpenGL)

A GUI (Dear ImGui + GLFW + OpenGL3) vem ligada por padrao. Pra compilar
so a versao console (CLI/shell/TUI), sem essa dependencia:

```powershell
cmake -S . -B build -G Ninja -DFWMSX_MSXDISK_GUI=OFF
cmake --build build
```

`--gui`/`call gui` nessa build so avisam que a GUI nao foi compilada.

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
