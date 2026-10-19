# fwMSX -- Manual

> Como compilar, executar e operar o fwMSX: emulador MSX1/MSX2/MSX2+, utilitario de discos
> (`msxdisk`), console interativo, ponte de controle para outros programas e depurador do Z80. O
> estado de cada parte esta em [SPEC.md](SPEC.md) e nas specs de fase (`doc/*-spec.md`).

## Indice

1. [Visao geral e inicio rapido](#visao-geral-e-inicio-rapido)
2. [Atalhos de teclado e como operar sem o mouse](#atalhos-de-teclado-e-como-operar-sem-o-mouse)
3. [Pre-requisitos](#pre-requisitos) e [Compilar](#compilar)
4. [Emulador MSX](#emulador-msx---msx) (opcoes, janela, slots, FM, SRAM)
5. [Disco novo e FORMAT](#disco-novo-em-branco-e-format)
6. [Save-state e captura de tela](#save-state-e-captura-de-tela)
7. [Console (`--cli`)](#console-interativo---cli) e [Ponte de controle](#ponte-de-controle-externa---ctl-port)
8. [Banco de ROMs](#banco-de-roms---romdb), [Fita](#fita-por-linha-de-comando-fwmsx---cas) e [Banco de fitas](#banco-de-fitas-fwmsx---fitadb)
9. [msxdisk](#msxdisk----utilitario-de-imagens-de-disco-msx) e [depurador do Z80](#nucleo-z80----depurador-embutido---z80dbg)

## Visao geral e inicio rapido

### Os executaveis

| Arquivo | Para que serve |
|---|---|
| `fwMSX.exe` | O emulador em **janela** (nao abre console no duplo clique). Aceita todas as opcoes (`--msx`, `--romdb`, `--cas`...), mas e' um programa "de janela": o terminal **nao espera** o fim dele. |
| `fwMSXc.exe` | O **mesmo programa em modo console** (Windows). Use no terminal para o console interativo (`--cli`) e para as ferramentas de linha de comando: o terminal espera o fim e o codigo de saida e' o certo. |
| `msxdisk.exe` | Utilitario de imagens de disco (CLI, shell, TUI e GUI). Tambem embutido: `fwMSX.exe --msxdisk ...`. |
| `fwMSX` (Linux) | Um unico executavel de console, que faz tudo. |

### As formas de usar

| Forma | Como abrir | Para que |
|---|---|---|
| **Janela** | `fwMSX.exe` (ou duplo clique) | jogar e usar o MSX, com menus e atalhos |
| **Opcoes de linha de comando** | `fwMSX.exe --msx --msx2p --cart jogo.rom --disk d.dsk` | escolher a maquina e a midia ao iniciar |
| **Ferramentas de linha de comando** | `fwMSXc.exe --romdb ...`, `--cas ...`, `--fitadb ...`, `--disknew ...`, `--msxdisk ...` | uma tarefa e sai |
| **Console interativo** | `fwMSXc.exe --cli` | iniciar o emulador e mandar comandos a ele sem sair do terminal |
| **Ponte de controle** | `fwMSX.exe --msx --ctl-port 7777` | outros programas (editor, depurador, montador) mandam comandos por TCP |
| **Depurador do Z80** | `fwMSXc.exe --z80dbg` | depurar o nucleo do Z80 |
| **Modo terminal** | `fwMSXc.exe --term [opcoes do --msx]` | a maquina dentro do terminal, com menu e atalhos (bom para testes rapidos) |
| **TUI de menus** | `fwMSXc.exe --tui` | menus com os comandos da CLI que controlam o emulador em **janela** |

### Inicio rapido

```powershell
.\dist\fwMSX.exe                                           # MSX1 numa janela (MSX BASIC)
.\dist\fwMSX.exe --msx --msx2p --cart jogo.rom             # MSX2+ com um cartucho
.\dist\fwMSX.exe --msx --disk msxdos1.dsk                  # MSX-DOS 1.8
.\dist\fwMSXc.exe --disknew meu.dsk ds35                   # cria um disquete novo de 720 KB, formatado
.\dist\fwMSXc.exe --cli                                    # console: 'emu start', 'peek', 'type', 'help'...
```

Dentro da janela: **F6** salva o estado, **F7** carrega, **F11** tela cheia, **F12** captura a tela.
A lista completa de teclas esta na proxima secao.

## Atalhos de teclado e como operar sem o mouse

### Atalhos do emulador (na janela; **nao** vao para o MSX)

| Tecla | Faz |
|---|---|
| **F6** | salva o estado da maquina no slot atual (`fwmsx-estado-N.sst`, na pasta de trabalho) |
| **F7** | carrega o estado do slot atual |
| **F8** / **F9** | slot de estado anterior / proximo (1 a 9); o aviso na tela mostra o slot |
| **F11** | tela cheia (o menu some e reaparece quando o mouse chega ao topo) |
| **F12** | captura a tela em PNG (`fwmsx-AAAAMMDD-HHMMSS.png`, na pasta de trabalho) |

F1-F5 sao teclas do MSX, por isso os atalhos do emulador comecam em F6. A janela mostra um aviso
temporario no canto inferior esquerdo ao salvar, carregar ou capturar (laranja quando ha' erro ou
quando o estado carregado e' de outra BIOS/cartucho).

### O teclado do MSX (mapeamento posicional, layout US)

Letras, numeros e simbolos vao para a **mesma tecla** do MSX.

| Tecla do PC | Tecla do MSX |
|---|---|
| `A`-`Z`, `0`-`9`, `- = \ [ ] ; ' ` , . /` | a mesma |
| Espaco, Enter, Backspace, Tab, Esc | SPACE, RETURN, BS, TAB, ESC |
| Shift, Ctrl | SHIFT, CTRL |
| Alt esquerdo / Alt direito | **GRAPH** / **CODE** |
| Caps Lock | CAPS |
| F1 a F5 | F1 a F5 |
| Home / Ins / Del | HOME / INS / DEL |
| **End** | **SELECT** |
| **Pause** | **STOP** |
| Setas | cursor |
| Teclado numerico (`0`-`9`, `* + / - .`, Enter) | teclado numerico do MSX |

Para um comando digitado sem teclado fisico, ou para automatizar: `type` (console e ponte) e `--keys`
(linha de comando), onde `|` e `\n` sao Enter.

### Joystick

Porta A: **setas** + **Z** ou **Espaco** (botao A) + **X** (botao B). Gamepads (GLFW): o 1o vai para
a porta A e o 2o para a porta B (menu **Joystick** mostra o estado). As setas e o Espaco continuam
valendo tambem como cursor e barra de espaco.

### Na TUI de menus (`fwMSXc.exe --tui`)

**F10** abre a barra de menus (setas, Enter, Esc; o mouse tambem funciona) - **F1** ajuda - **Enter**
executa a linha de comando - **setas cima/baixo** historico - **Tab** completa - **PgUp/PgDn** rola o
historico. Nos assistentes: Enter avanca, Esc cancela; no navegador de arquivos: Enter entra/escolhe,
Backspace sobe, `t` digita o caminho. `exit`/`quit` sai da TUI. Ver [tui-spec.md](tui-spec.md).

### No modo terminal (`fwMSXc.exe --term`)

**F10** menu (setas + Enter) - **F11** linha de comando - **F6/F7** salva/carrega estado - **F8/F9** slot
- **F12** captura PNG - **Ctrl+C** = CTRL+STOP do MSX - todas as outras teclas vao para o MSX (F1-F5,
End = SELECT, setas = cursor e joystick). Para sair: menu > Sair, ou o comando `quit`. Ver
[term-spec.md](term-spec.md).

### Operar tudo sem o mouse

Os **menus da janela sao de mouse**. Tudo o que eles fazem tem um equivalente por teclado, de tres jeitos:

| Menu da janela | Por teclado |
|---|---|
| Arquivo > Salvar/Carregar estado | **F6** / **F7** / **F8** / **F9**; `state save` / `state load` |
| Arquivo > Capturar tela | **F12**; `screenshot` |
| Tela > Exibir > Tela cheia | **F11** |
| Maquina > Reiniciar / Pausar | `reset`, `pause`, `resume`, `step` (console/ponte) |
| Maquina > Modelo | `--msx2` / `--msx2p` ao iniciar |
| Midia > Cartucho | `--cart arq [mapper]` ao iniciar; `cart arq [mapper]` (console/ponte) |
| Midia > Disco | `--disk` / `--diskb`; `disk A arq`, `eject A`; `--disknew` |
| Midia > Fita | `--fita`; `tape arq`, `tape eject`, `tape rewind` |
| Sair | `quit` (ponte) ou fechar a janela |

Ou seja: **tudo** pode ser feito pelo console (`fwMSXc.exe --cli`), que e' 100% teclado, com
historico (seta para cima/baixo) e **TAB** para completar.

### No console (`--cli`)

Edicao de linha (replxx): **TAB** completa o comando; **Seta para cima/baixo** percorre o historico
(guardado em `~/.fwmsx_history`); **Home/End** ou **Ctrl+A/Ctrl+E** vao ao inicio/fim da linha;
**Ctrl+U/Ctrl+K** apagam ate' o inicio/fim; **Ctrl+W** apaga a palavra; **Ctrl+R** busca no
historico; **Ctrl+L** limpa a tela; **Ctrl+D** (linha vazia) ou `exit` sai.

### No msxdisk (TUI de discos)

`F2` Novo, `F3` Abrir, `F4` Renomear, `F5` Salvar, `F6` Salvar como, `F7` Enviar (local -> imagem),
`F8` Receber (imagem -> local), `F9` Tema, `F11` Nova pasta, `Del` Excluir, `Espaco` marca,
`F10`/`Esc` sai. Detalhes em [msxdisk-spec.md](msxdisk-spec.md).

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
3. Compila e gera `dist\fwMSX.exe` (janela) e `dist\fwMSXc.exe` (o mesmo programa em
   modo console, gerado por `tools/pe_subsystem`) e `dist\msxdisk.exe`;
4. Le a versao corrente em `src\common\version.h` e empacota
   `dist\fwMSX-X.Y.Z.zip` com os executaveis, `README.md`, `LICENSE`,
   `doc\MANUAL.md` e `doc\RELEASE.md`.

### Opcao 2 -- manual (MSYS2 UCRT64 shell)

Abra o **"MSYS2 UCRT64"** (nao o MSYS2 normal, nem o MINGW64) a partir do
menu iniciar, va ate a pasta do projeto e rode:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

O executavel fica em `dist/fwMSX.exe` (o `fwMSXc.exe` e' gerado no fim do link; no MSYS2 UCRT64
o utilitario `pe_subsystem` entra no build normalmente).

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

Sem nenhum argumento isso abre a maquina MSX1 completa numa janela (os mesmos
padroes de `--msx`: BIOS `resource/fMSX/ROMs/MSX.ROM` ao lado do executavel,
sem cartucho/disco) -- ver a secao "Emulador MSX" abaixo e `doc/machine-spec.md`.
No terminal, prefira o `fwMSXc.exe` (o terminal espera o fim dele); o `fwMSX.exe`
e' pensado para o duplo clique e para atalhos.

### Esqueleto multi-linguagem (historico)

O esqueleto original do projeto (um modulo "carregado" em cada uma das quatro
linguagens -- C++, C, Assembly e Fortran) continua existindo, mas so e'
alcancado com argumentos explicitos que nao batem com nenhum modo conhecido
(nome do produto + versao, sobrescrevendo os defaults de `version.h`):

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

Cada versao gera dois pacotes em `dist/`:

- **Windows**: `fwMSX-X.Y.Z.zip` (executaveis `fwMSX.exe` e `fwMSXc.exe`, `msxdisk.exe`, README, licenca, MANUAL e RELEASE).
  `fwMSX.exe` e' linkado estaticamente e depende apenas do **UCRT** (`ucrtbase.dll`, nativo do Windows 10
  versao 1607 ou mais recente): roda em outra maquina sem instalar o MSYS2.
- **Linux**: `fwMSX-X.Y.Z-linux.tar.gz` (mesmos arquivos, executaveis `fwMSX` e `msxdisk`), gerado por
  `build.sh` no WSL/Ubuntu.

As BIOS, o BASIC, o FM-PAC e as ROMs de `resource/fMSX/` **nao** vao no pacote: ficam no repositorio
(ver [LICENSE-THIRD-PARTY.md](../LICENSE-THIRD-PARTY.md)).

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
`konami4`, `ascii8`, `ascii16` (a troca de banco de ROM; o SCC de
Konami5/Gen8 esta ligado desde a v1.13.0, ver `scc-spec.md`; SRAM continua
fora, ver `memory-map-spec.md` secao 6 pro motivo). Sem o parametro,
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

## Emulador MSX (`--msx`)

Sem argumentos, `fwMSX.exe` abre a maquina MSX1 numa janela (a BIOS real, com o MSX BASIC).
`--msx` escolhe a maquina e as opcoes abaixo.

### Exemplos

```powershell
.\dist\fwMSX.exe                                   # MSX1 numa janela (BIOS resource/fMSX/ROMs/MSX.ROM)
.\dist\fwMSX.exe --msx --msx2                      # MSX2: MSX BASIC 2.1, SCREEN 0-8
.\dist\fwMSX.exe --msx --msx2p                     # MSX2+: MSX BASIC 3.0, V9958, SCREEN 10-12
.\dist\fwMSX.exe --msx --cart jogo.rom             # cartucho no slot 1:0 (ROM plana ou MegaROM detectada)
.\dist\fwMSX.exe --msx --cart megarom.rom ascii8   # mapper escolhido: auto, gen8, gen16, konami5, konami4, ascii8, ascii16, msxdos2
.\dist\fwMSX.exe --msx --msx2 --cart MSXDOS2.ROM msxdos2 --disk disco720.dsk   # MSX-DOS 2 (cartucho generico)
.\dist\fwMSX.exe --msx --disk msxdos1.dsk          # MSX-DOS 1.8 (interface de disco + disquete em A:)
.\dist\fwMSX.exe --msx --disk msxdos1.dsk --disk-ro   # disco somente leitura (o MSX-DOS nao grava)
.\dist\fwMSX.exe --msx --no-fmpac                  # sem FM-PAC (o padrao liga o FM-PAC)
.\dist\fwMSX.exe --msx --mute                      # sem audio
.\dist\fwMSX.exe --msx --keys "print 1234|" --wait 120 --shot tela.ppm   # sem janela: digita, espera e salva a tela
.\dist\fwMSX.exe --msx --keys "call music(0,0,1)|play#2,\"v15c\"|" --wait 300 --wav musica.wav --mute
```

### Opcoes da linha de comando

| Opcao | Efeito |
|---|---|
| `--msx` | liga a maquina (MSX1 por padrao) |
| `--msx2` / `--msx2p` | modelo MSX2 (BASIC 2.1) ou MSX2+ (BASIC 3.0) |
| `--bios <arquivo>` | BIOS principal (padrao: `resource/fMSX/ROMs/MSX.ROM`, ou `MSX2.ROM` / `MSX2P.ROM`) |
| `--ext <arquivo>` | sub-ROM do MSX2 (16KB; padrao: `MSX2EXT.ROM` / `MSX2PEXT.ROM` ao lado da BIOS) |
| `--cart <arquivo> [mapper]` | cartucho no slot 1:0; sem mapper, consulta o banco de ROMs pelo SHA-1 (ver `--romdb`, abaixo) e, se nao encontrar, ROM de ate 32KB e' plana e acima disso e' detectada |
| `--disk` / `--diska <arquivo>` | disquete (`.dsk` cru) na unidade A: (liga a interface de disco) |
| `--diskb <arquivo>` | disquete na unidade B: |
| `--disk-ro` | discos entram protegidos contra gravacao (o arquivo nunca e' alterado) |
| `--disk-interface` | liga a interface de disco mesmo sem disco montado |
| `--diskrom <arquivo>` | `DISK.ROM` (padrao: ao lado da BIOS) |
| `--fmpac [arquivo]` | FM-PAC no slot 2:0 (padrao: `resource/fMSX/FMPAC.ROM`) |
| `--no-fmpac` | sem FM-PAC (o padrao o liga quando o `FMPAC.ROM` existe) |
| `--keys "texto"` | digita o texto no teclado do MSX; `|` = Enter (`\"` para aspas; `(`, `)`, `*` e `@` ja' saem certos). Aceita a sintaxe de teclas do `type`: `{enter}`, `{esc}`, `{f1}`, `{stop}`, `{ctrl}+{c}` (juntas), `{ctrl}{c}` (em sequencia), `{enter*3}`. Aspas e espacos aqui sao texto comum |
| `--wait N` | quadros de espera depois de `--keys` (padrao 60) |
| `--shot <arquivo.ppm>` | salva a tela em PPM |
| `--text` | imprime a tela em texto no fim (SCREEN 0 e 1) |
| `--fmstat` | imprime o estado dos 9 canais do OPLL (nota, timbre, frequencia, volume) e o modo ritmo |
| `--wav <arquivo.wav>` | grava a mistura de audio (PSG + SCC + FM) |
| `--disk-acesso mem\|porta` | controladora de disco pela memoria (DISK.ROM, padrao) ou pelas portas (sem ROM) |
| `--disk-porta D0h` | base das 5 portas da controladora por portas (padrao D0h) |
| `--disk-formato auto\|ss525\|ds525\|ss35\|ds35` | formato dos drives: auto (180/360/720 KB), 5 1/4 face simples (180), 5 1/4 face dupla (360), 3 1/2 face simples (360), 3 1/2 face dupla (720) |
| `--fita <arquivo>` | insere uma fita (`.cas`, `.tsx` ou `.tzx`, pela extensao) na unidade |
| `--fita-modo rapido\|normal` | carregamento rapido (gancho de BIOS, sem som, padrao) ou normal (pulsos de verdade, com o barulho do gravador) -- ver `doc/tape-spec.md` |
| `--frames N` | quadros de boot antes dos comandos seguintes (sem janela) |
| `--ctl-port N` | liga a ponte de controle em 127.0.0.1:N (0 = porta livre; vai para `fwmsx.port`) -- ver a secao da ponte |
| `--vdplog` | mostra os registradores do VDP que mudam, quadro a quadro |
| `--mute` | sem audio |

**Modo sem janela:** a maquina roda sem abrir janela quando ha `--keys` ou `--shot`. Nesse
modo, `--text`, `--fmstat` e `--wav` funcionam no fim da execucao. Sem `--keys` nem `--shot`, a
janela abre.

### A janela

- **Teclado e joystick**: ver [Atalhos de teclado](#atalhos-de-teclado-e-como-operar-sem-o-mouse)
  (F6/F7/F8/F9 estado, F11 tela cheia, F12 captura; Alt esq. = GRAPH, Alt dir. = CODE, End = SELECT,
  Pause = STOP; joystick = setas + Z/Espaco + X).
- **Titulo da janela**: `fwMSX - SCREEN n - fps` (mais `(pausado)`, `(mudo)` e, com a ponte ligada,
  `controle: porta N`).

Os menus do topo da janela (9):

- **Arquivo**: Carregar cartucho; **Salvar no slot atual (F6)**, **Carregar do slot atual (F7)**,
  **Slot de estado (F8/F9)** (1 a 9), **Capturar tela (PNG) (F12)**; **Salvar estado.../Carregar
  estado...** (escolhe o arquivo `.sst` na mao); Sair.
- **Maquina**: Reiniciar, **Modelo** (MSX1, MSX2, MSX2+), **Configuracao de disco...**,
  **Configuracao de slots...** (secao abaixo), Video (NTSC; PAL e' "em breve"), **Pausar**,
  **Soltar todas as teclas** (util se uma tecla "ficou presa").
- **Midia**: agrupa os tres submenus de midia removivel --
  - **Disco**: inserir e ejetar A: e B:; **Novo disco em branco em A:/B:** (cria um disquete
    formatado nos 4 formatos do MSX e ja' o insere -- ver "Disco novo e FORMAT").
  - **Fita**: inserir ou criar uma **fita nova (.tsx)**, ejetar e rebobinar; trocar entre
    carregamento **rapido** (sem som) e **normal** (pulsos de verdade, com o barulho do gravador);
    **destravar/travar contra gravacao** (uma fita de arquivo entra sempre travada; uma fita nova
    entra destravada) e escolher o **modo de gravacao** (incluir no final da fita, sobrescrever o
    ponto marcado, ou nova fita); mostrar a janela visual "Fita K7" (rolos girando enquanto o
    motor esta' ligado, contagiros, barra de progresso, lista de arquivos -- clicar num arquivo
    marca o ponto de carga/gravacao, clicar de novo desmarca). `CSAVE`/`BSAVE "CAS:"` gravam na
    fita destravada, pelo modo escolhido; a gravacao e' salva no arquivo imediatamente. Trocar de
    fita ou de modo NAO reinicia a maquina. Ver `doc/tape-spec.md`.
  - **Cartucho**: inserir ou retirar o cartucho do slot 1:0; escolher o **Mapper** (submenu --
    Automatico/deteccao, ou explicito: Gen8, Gen16, Konami5, Konami4, ASCII8, ASCII16, MSX-DOS 2;
    com um cartucho ja' inserido, trocar o mapper reinicia na hora com o MESMO arquivo); ligar ou
    desligar o **FM-PAC** (slot 2:0).
- **Tela**: agrupa exibicao e filtros de video --
  - **Exibir**: zoom 2x, 3x, 4x, 6x; proporcao original, 4:3 corrigido ou 16:9 esticado; tela
    cheia (o menu some e volta quando o mouse chega ao topo).
  - **Video**: interpolacao, scanlines e filtros de cor (Monochrome, Sepia, Green CRT, Amber CRT,
    CMY e RGB Raster).
- **Som**: mudo e volume (gravar trilha de som e' "em breve").
- **Joystick**: status do gamepad em cada porta.
- **ROMs**: banco de ROMs (menu completo, ver `doc/romdb-spec.md`).
- **Ferramentas**: **Interface** (tema escuro/claro, tamanho da letra, moldura da tela), mais os
  placeholders existentes ("em breve").
- **Ajuda**: **Teclado** (janela com o mapeamento e os atalhos) e a versao
  (`fwMSX vX.Y.Z - Empresa: Jogo: Subtitulo`).

Os itens marcados "(em breve)" (PAL, trapacas, dispositivos de entrada, bateria MIDI...) ainda
nao fazem nada. Trocar o modelo, o cartucho ou o FM-PAC reinicia a maquina. A SRAM (`.sav`) e' gravada antes.

### Configuracao de slots (Maquina > Configuracao de slots...)

A maquina e' montada a partir de uma tabela de 16 celulas (slot:subslot). Cada celula recebe um
conteudo:

- **Vazio**
- **ROM (BIOS, BASIC ou cartucho)**: arquivo, pagina (0 = 0000h, 1 = 4000h) e, na BIOS, um segundo arquivo (BASIC)
- **Sub-ROM MSX2 (16KB)**: pagina 0 (MSX2EXT)
- **RAM**: 16, 32 ou 64 KB
- **RAM mapeada (mapper)**: 64, 128, 256, 512 ou 1024 KB
- **Disco (DISK.ROM)**: a controladora WD2793 no enderecos 7FF8h-7FFFh da celula; a sub-ROM do MSX2 pode ir na pagina 0
- **FM-PAC**: ROM de 16KB com SRAM de 8KB

Regras:

- **A BIOS fica em 0:0** (o Z80 comeca la'). Sem ela, o layout e' recusado.
- **BIOS de 32KB**: um arquivo ocupa a pagina 0 (BIOS) e a pagina 1 (BASIC) do slot escolhido.
- **BIOS e BASIC em arquivos separados**: cada um com 16KB; um na pagina 0 e outro na pagina 1 da celula 0:0.
- **Quatro bancos de RAM de 64KB no slot 2**: quatro celulas (2:0 a 2:3), cada uma com RAM de 64KB.
- **Mapper de 1024KB em 3:1**: a celula 3:1 vira RAM mapeada. So' uma RAM mapeada por maquina.
- **Discos**: precisam de uma celula de Disco no layout.
- **Padrao**: o botao *Padrao* volta ao layout de sempre (MSX1: BIOS em 0:0, RAM de 64KB em 3:2;
  MSX2: BIOS em 0:0, mapper de 128KB em 3:2, sub-ROM em 3:1). **Aplicar e reiniciar** valida e recria a
  maquina; se o layout nao montar, o motivo aparece em vermelho.

**Limite:** o layout so' se edita pela janela. Ainda nao ha opcao de linha de comando nem arquivo
para salvar um layout ([doc/slots-spec.md](slots-spec.md)).

### MSX-MUSIC e FM-PAC (chip FM)

- As portas `7Ch`/`7Dh` do OPLL respondem sempre, como no fMSX.
- Os **comandos de BASIC** do MSX-MUSIC vem na ROM do FM-PAC, que liga por padrao. Verificados nesta
  versao: `CALL MUSIC`, `PLAY #n`, `CALL VOICE`, `CALL PITCH`, `CALL AUDREG` e `CALL PLAY`.

```basic
CALL MUSIC (0,0,1)              ' 1 canal de FM, sem bateria
PLAY #2,"T120 V15 C4 D4 E4 F4 G4 A4 B4 >C4"
CALL MUSIC (1,0,1,1,1,1,1)      ' liga a bateria (BD, HH, SD, TOM, TC) e 6 canais
```

- **Limite:** `CALL VOICECOPY` nao e' aceito pela ROM do fMSX. Status e timers do OPLL nao sao emulados.
- **Limite:** o som do FM ainda nao foi comparado com um MSX-MUSIC real. Use `--wav` para gravar e ouvir.

### SRAM (`.sav`)

Cartuchos ASCII8 e ASCII16 e o FM-PAC tem memoria de bateria. O arquivo `.sav` fica ao lado da ROM
(`jogo.rom` -> `jogo.sav`; `FMPAC.sav` em `resource/fMSX/`). Ele e' lido ao carregar e gravado quando
muda: a cada 300 quadros, ao trocar cartucho ou modelo, e ao fechar. **Faca copia dos saves
importantes.** O formato e' o mesmo do fMSX (8 KB, ou 2 KB para ASCII16).

### O que funciona e o que nao funciona

**Funciona:**

- MSX1, MSX2 e MSX2+ ate o prompt do BASIC (1.0, 2.1 e 3.0).
- MSX-DOS 1.8 a partir de `msxdos1.dsk` (leitura e gravacao; use `--disk-ro` para proteger), pela
  memoria (DISK.ROM) ou pelas portas (DDX 3.0/CDX-2, estilo Microsol).
- Cartuchos ROM plana, MegaROM (Konami, ASCII, Gen8, Gen16), SCC (F1 Spirit toca a trilha de 5 canais)
  e MSX-DOS 2 (cartucho generico, testado com um kernel 2.30 real e um disco de 720KB -- `dir`/`cd`
  em subdiretorios funcionam). Mapper escolhivel pela CLI (`--cart <rom> msxdos2`) ou pela janela
  (menu Midia > Cartucho > Mapper, ou a Configuracao de slots).
- VDP completo (SCREEN 0 a 8 no V9938; V9958 com SCREEN 10-12); efeitos de rastreio no meio do
  quadro (paleta/scroll trocados por interrupcao de linha).
- PSG, SCC e FM (MSX-MUSIC e FM-PAC) com os comandos de BASIC, modo ritmo e saida ao vivo.
- **Fita** (`.cas`, `.tsx`/`.tzx`): leitura (rapida e normal, com som), gravacao (`CSAVE`/`BSAVE "CAS:"`,
  fita nova, protecao, 3 modos, marcar o ponto, contagiros), janela "Fita K7", e a ferramenta de linha
  de comando `fwmsx --cas` (empacotar, "ripar" `.wav`, listar). Navegacao completa dos blocos de
  controle do TZX. Banco de fitas com metadados (`fwmsx --fitadb`, sem download). Ver as secoes
  acima e [tape-spec.md](tape-spec.md).
- **Banco de ROMs** (`fwmsx --romdb` e menu **ROMs**): downloads, busca, edicao, identificacao,
  `verify` (SHA-1) e auto-mapper ao carregar `--cart` sem escolher um a dedo.
- Configuracao de slots pela janela (secao acima).
- **Console** e **ponte de controle** (para outros programas): ver as secoes proprias, abaixo.
- **Disco novo e FORMAT**: ver a secao propria, abaixo.
- **Save-state** e **captura de tela**: ver a secao propria, abaixo.

**Nao funciona ou nao foi verificado:**

- **Lode Runner + SCC** nao sobe. **Parodius (Smooth Scroll)** mostra tela fragmentada, causa nao
  diagnosticada. **Mega Chase** foi validado so' ate o titulo. **F-1 Spirit 3D**: a troca de disco pela
  janela nao foi testada.
- **Som do FM, do SCC e da fita (modo normal)**: nao comparados com hardware real.
- **Cores YJK** do V9958: nao conferidas com hardware real.
- **Fita**: download pelo site continua fora de escopo (sem termos de uso publicados -- so' o banco
  de metadados, sem download, existe); `.cas` cru sem gravacao previa deste emulador ainda adivinha
  o preenchimento de alinhamento; um ASCII multi-bloco de verdade (256 bytes por bloco) aparece
  fragmentado na lista; "Selecao" (#28 do TZX) escolhe sempre a 1a opcao (sem como mostrar um menu de
  verdade numa ferramenta batch). Ver [tape-spec.md](tape-spec.md), secoes 5, 9, 10 e 11.
- **Banco de ROMs**: montar a maquina pelo banco (layout por nome) ainda nao existe; o JSON do
  Vampier nao e' usado (so' o SQL, que ja' cobre jogo/empresa/ano/SHA-1).
- **Sem**: GameMaster2, cartucho MSX-MUSIC com BIOS propria, e `CALL VOICECOPY`. O save-state avisa
  se a BIOS ou o cartucho mudaram desde o save, mas nao confere disco nem fita.
- **Outras BIOS** (ex.: Gradiente Expert 1.1): sobem ate o BASIC. O texto da Expert com espacos entre
  as letras ("G r a d i e n t e") esta' gravado na propria ROM -- nao e' defeito do emulador.

Lista completa e atualizada: [RELEASE.md](RELEASE.md) (secao da versao) e [SPEC.md](SPEC.md), secao 5.0.

### Documentacao por assunto

[machine-spec.md](machine-spec.md) (maquina e janela), [slots-spec.md](slots-spec.md) (layout de slots),
[tui-spec.md](tui-spec.md) (TUI de menus), [term-spec.md](term-spec.md) (modo terminal), [control-spec.md](control-spec.md) (ponte de controle), [repl-spec.md](repl-spec.md) (console),
[diskfmt-spec.md](diskfmt-spec.md) (disco novo e FORMAT), [savestate-spec.md](savestate-spec.md) (save-state),
[tape-spec.md](tape-spec.md) (fita), [romdb-spec.md](romdb-spec.md) (banco de ROMs),
[fm-spec.md](fm-spec.md) (FM, MSX-MUSIC e FM-PAC), [sram-spec.md](sram-spec.md) (SRAM e `.sav`),
[audio-spec.md](audio-spec.md) (audio), [fdc-spec.md](fdc-spec.md) (disco), [msx2-spec.md](msx2-spec.md) e
[msx2p-spec.md](msx2p-spec.md) (MSX2 e MSX2+), [scc-spec.md](scc-spec.md) (SCC), [memory-map-spec.md](memory-map-spec.md) (mapa de memoria).

## TUI de menus (`--tui`)

`fwMSXc.exe --tui` mostra os comandos do console em **menus** e controla o emulador em **janela** pela ponte
de controle (nao embute a maquina). Comece por **Emulador > Iniciar MSX1/MSX2/MSX2+**, que abre a janela do
emulador e conecta; depois use Arquivo (estado, captura), Maquina (reiniciar, pausar, registradores,
memoria, digitar texto), Midia (cartucho, disco, fita, disco novo) e Ferramentas (banco de ROMs, fitas,
banco de fitas, msxdisk). Itens com `...` perguntam os dados e, para arquivos, abrem um navegador. Embaixo
ha' a linha de comando (os mesmos comandos do console) e um status do emulador. `--attach <porta>` conecta
num emulador ja' aberto. Ver [tui-spec.md](tui-spec.md).

## Modo terminal do emulador (`--term`)

`fwMSXc.exe --term` roda a maquina dentro do terminal (use o `fwMSXc.exe` no Windows). Aceita as mesmas
opcoes do `--msx` (`--msx2p --cart jogo.rom msxdos2 --disk d.dsk ...`) e `--ctl-port`. Texto de
verdade em SCREEN 0/1 (prompt do BASIC, MSX-DOS); graficos em meios blocos reduzidos para caber no
terminal, na proporcao 4:3 (o menu alterna para pixels originais). **F10** abre o menu (as mesmas acoes da janela), **F11** a linha de comando (todos os comandos
da ponte de controle mais `newdisk`), e F6/F7/F8/F9/F12 como na janela. Um terminal nao avisa quando uma
tecla e' solta: cada tecla vale como segura por uma fracao de segundo, e segurar repete. Ver [term-spec.md](term-spec.md).

## Disco novo em branco e FORMAT

Cria um disquete **formatado** (boot sector com o bootstrap do MSX-DOS 1, duas FATs, diretorio raiz
vazio e area de dados com E5h). So' os 4 formatos que o MSX tem -- **nao existe 3 1/2 de 1,44 MB**:

| Chave | Disco | Lados | Capacidade |
|---|---|---|---|
| `ss525` | 5 1/4, densidade simples | 1 | 180 KB |
| `ds525` | 5 1/4, densidade dupla | 2 | 360 KB |
| `ss35` | 3 1/2, densidade dupla | 1 | 360 KB |
| `ds35` | 3 1/2, densidade dupla | 2 | 720 KB |

- **Janela**: **Midia > Disco > Novo disco em branco em A: / B:** -> escolhe o formato -> escolhe o
  nome do arquivo; o disco ja' entra no drive.
- **Linha de comando**: `fwMSXc.exe --disknew meu.dsk ds35`. No console: `newdisk meu.dsk ds35`.
- **Dentro do MSX**: `CALL FORMAT` (Disk BASIC) formata o disco inserido -- "Drive name? (A,B)",
  "Strike a key when ready", "Format complete". O Disk BASIC 1.0 do fMSX formata sempre em face simples
  (360 KB). A controladora aceita o comando WRITE TRACK; a imagem precisa ter o tamanho do formato
  (use um disco novo, ou um `.dsk` do tamanho desejado).

Ver [diskfmt-spec.md](diskfmt-spec.md).

## Save-state e captura de tela

- **Estado**: **F6** salva, **F7** carrega, **F8/F9** trocam o slot (1 a 9; arquivos
  `fwmsx-estado-N.sst` na pasta de trabalho). O menu **Arquivo** tambem salva/carrega um `.sst` em
  qualquer lugar. Guarda Z80, VDP (inclusive a VRAM), PSG, SCC, OPLL, PPI, controladora de disco e
  RAM/RAM de mapper, e e' aplicado **sobre a maquina que ja' esta' rodando** (nao recarrega
  BIOS/cartucho/disco/fita: deixe a mesma midia inserida). O `.sst` guarda o CRC32 da BIOS e do
  cartucho, e a janela **avisa** (em laranja) se voce carregar sobre outra midia. Disco e fita nao sao
  conferidos. Ver [savestate-spec.md](savestate-spec.md).
- **Captura de tela**: **F12** (ou **Arquivo > Capturar tela**) grava um PNG com a proporcao certa; no
  console/ponte, `screenshot arquivo.png`; sem janela, `--shot arquivo.ppm`.

## Console interativo (`--cli`)

No Windows use o **`fwMSXc.exe`** (a versao de console); o `fwMSX.exe` e' de janela e o terminal nao o
espera, o que quebra a digitacao. O console **nao embute o emulador**: ele inicia o emulador em outro
processo e manda comandos a ele pela ponte de controle.

```
fwMSXc.exe --cli                  # abre o console
fwMSXc.exe --cli --attach 7777    # ja' conecta num emulador aberto com --ctl-port 7777

fwmsx> emu start --msx2p --cart jogo.rom msxdos2     # abre a janela do emulador e conecta
fwmsx:51607> peek 0xC000 4
fwmsx:51607> type "print 6*7\n"
fwmsx:51607> state save meu.sst
fwmsx:51607> emu stop
fwmsx> newdisk novo.dsk ds35
```

| Comando | Faz |
|---|---|
| `emu start [opcoes do --msx]` | inicia o emulador (janela) e conecta |
| `emu attach <porta>` / `emu detach` | conecta / desconecta de um emulador ja' aberto |
| `emu stop` / `emu status` | encerra o emulador / mostra o estado |
| `newdisk <arq> <ss525\|ds525\|ss35\|ds35>` | disquete novo e formatado (`--disknew`) |
| `romdb`, `cas`, `fitadb`, `msxdisk` | as ferramentas de mesmo nome (`--romdb`, `--cas`, `--fitadb`, `--msxdisk`) |
| `help` / `help <comando>` | a tabela de comandos / uso e resumo de um comando |
| `man` / `man <topico>` | o manual completo (ver abaixo) |
| `exit` / `quit` | sai (o emulador continua aberto) |
| *qualquer outro* | vai para o emulador conectado (comandos da tabela abaixo) |

### Ajuda e manual (`help`, `man`)

`help` lista os comandos agrupados (Console, Emulador, Midia e estado, Depurador, Ferramentas); `help peek` mostra o uso e um resumo; `man peek`
abre o manual completo, com exemplos. O `man` tambem traz a **referencia do MSX**: `man bios` (rotinas da BIOS: CHPUT, CHGET, WRTVDP...),
`man sysvars` (variaveis do sistema: FORCLR, JIFFY, SCRMOD...), `man hooks` (ganchos), `man basic`, `man cores`, `man screens`, `man vdpregs`,
`man psgregs`, `man portas`, `man slotsmsx` e `man teclado`. `man` sozinho lista tudo; `man sysv` (nome parcial) sugere `sysvars`.
**Manual do MSX-BASIC**: cada palavra reservada tem sua pagina, em portugues, com formato, exemplo, descricao e programa exemplo (`man print`,
`man screen`, `man put sprite`, `man for`, `man call music`...; `help print` mostra so' o formato e o resumo), alem de topicos do manual como
`man apendice-c-codigos-de-erro` e `man apendice-d-mapa-da-memoria`. Os textos vem do livro *Linguagem BASIC MSX* (Aleph/Gradiente) e do *Manual MSX 2+ FM*.
**BIOS e hardware**: `man chput`, `man wrtvdp`, `man jiffy`... (rotinas da MainROM/SubROM/Disk-ROM, BDOS e variaveis de RAM, em ingles) e os chips: `man hw-rtc`,
`man hw-psg`, `man opll`, `man v9990`, `man hw-ports`... **`man -l`** lista as categorias; `man -l bios-rom` lista os nomes de uma; **`man -k sprite`** procura o texto
nos nomes e resumos.
**Livros**: `man rb-<titulo>` (The MSX Red Book, 973 topicos), `man th2-<titulo>` (MSX2 Technical Handbook, 1356) e `man dignified` / `man bd-<titulo>` (ajuda do
Basic Dignified, em portugues). Os links dos livros aparecem como "VER TAMBEM" no fim de cada pagina. Para achar: `man -l red` (capitulos e nomes do Red Book), `man -l handbook`,
`man -k expander` (procura em nomes e resumos). O texto dos livros e' em ingles.
**MSX-DOS e processadores**: `man dos2-copy`, `man dos2-xcopy`... (um topico por comando do MSX-DOS 2; `man dos2-commands` e' a introducao), `man dos2-fn-09h` ou `man _strout` (uma por chamada de
funcao do sistema), `man dos2-program-interface`, `man dos-dir` (MSX-DOS 1) e `man cpm`; `man z80-opcodes` / `man z80-mnemonics` (tabelas de instrucoes), `man r800-instructions`, `man fmpac-manual`,
`man turbo-basic-compiler`. Extensoes de firmware do BASIC: `man call kanji`, `man call music`...; referencias: `man ref-color-table`, `man ref-error-codes`, `man ref-screen-modes`. Em ingles, como nos originais.
**Montador e ferramentas**: `man fwasm` e' o manual do nosso montador Z80 (11 paginas: `fwasm-syntax`, `fwasm-labels`, `fwasm-numbers`, `fwasm-operators`, `fwasm-data`, `fwasm-directives` com o estado de cada diretiva,
`fwasm-conditional`, `fwasm-macros`, `fwasm-bios`, `fwasm-biosvars`); `man nestorbasic` e `man nb-<n>` (NestorBASIC, 87 funcoes), `man seetracker` (SEE), `man msxbas2rom` (compilador de BASIC para ROM).
No console o texto e' paginado (espaco = pagina, Enter = linha, `q` sai); na TUI abre uma janelinha (setas, PgUp/PgDn, Home/End, Esc ou `q`
fecham). Detalhes e como acrescentar topicos: [man-spec.md](man-spec.md).

As respostas aparecem sem o `ok`; os erros como `erro: ...`. O prompt mostra a porta
(`fwmsx:51607> `). Se o emulador for fechado, o proximo comando avisa e o console volta ao prompt
sem emulador. Teclas de edicao: ver [No console](#no-console---cli). Ver [repl-spec.md](repl-spec.md).

## Ponte de controle externa (`--ctl-port`)

`fwMSX.exe --msx --ctl-port 0` (ou um numero de porta) abre um servidor **so' em 127.0.0.1**. A porta
vai para `fwmsx.port` (na pasta do executavel e na de trabalho; apagado ao sair) e aparece no titulo
da janela. Qualquer programa manda **uma linha de texto por comando** e recebe uma linha: `ok ...` ou
`err ...`. Teste com `telnet 127.0.0.1 <porta>`. Argumentos com espaco vao entre aspas duplas (a
barra invertida dos caminhos do Windows e' literal). Numeros: `123`, `0x7B`, `$7B`, `7Bh`.

| Comando | Faz |
|---|---|
| `help`, `version`, `status` | ajuda, versao, `frame=N paused=0 screen=M pc=XXXX typing=0 cart=... diskA=... tape=...` |
| `reset` | reinicia a maquina |
| `pause`, `resume`, `step [n]` | pausa, retoma, avanca n quadros (so' pausado) |
| `type <texto>` | digita no teclado do MSX. Texto entre aspas ou solto (`\n` = Enter); teclas entre chaves FORA das aspas: `type "key off" {enter}`, `{esc}`, `{f1}`, `{stop}`; `{ctrl}+{c}` aperta juntas (Control-C), `{ctrl}{c}` aperta uma e solta, depois a outra; `{enter*3}` repete; `{ctrl+c}` e' o mesmo que `{ctrl}+{c}`. A pontuacao segue o teclado da BIOS carregada (japones no MSX2+ do fMSX, internacional nas outras) |
| `peek <end> [n]`, `poke <end> <v>...` | le/grava memoria como o Z80 enxerga (ROM nao e' alterada) |
| `regs` | registradores do Z80 (o depurador tem o `reg`, completo) |
| `cart <arq\|-> [mapper]` | troca o cartucho (reinicia); `-` retira; mapper: auto, gen8, gen16, konami5, konami4, ascii8, ascii16, msxdos2 |
| `disk <A\|B> <arq>`, `eject <A\|B>` | insere / ejeta disco |
| `tape <arq>`, `tape eject`, `tape rewind` | fita |
| `state save <arq>`, `state load <arq>` | save-state (`ok aviso: ...` se a BIOS/cartucho mudou) |
| `screenshot <arq>` | grava a tela (PNG) |
| `quit` | fecha o emulador |

Ver [control-spec.md](control-spec.md).

## Depurador (`break`, `stepi`, `go`...)

O fwMSX tem um depurador do EMULADOR (a maquina inteira: slots, VDP, interrupcoes), no estilo do depurador do fMSX:
pontos de parada, passo a passo, desmontador, memoria e registradores. Ele funciona por **quatro caminhos**, todos com os
MESMOS comandos (e a janela tem ainda o painel visual, abaixo):

| Caminho | Como usar |
|---|---|
| **TUI** (`fwMSXc.exe --tui`) | menu Depurador > Painel Z80 Debugger (ver [O painel Z80 Debugger na TUI](#o-painel-z80-debugger-na-tui)); abre sozinho ao parar |
| **Janela** (`fwMSX.exe`) | **F10** abre o painel Z80 Debugger (ver [O Z80 Debugger na janela](#o-z80-debugger-na-janela-f10)) |
| **Console** (`fwMSXc.exe --cli`) | digite os comandos no prompt; os avisos aparecem como `[evento] ...` |
| **TUI de menus** (`fwMSXc.exe --tui`) | menu **Depurador** (inclusive o painel visual) ou a linha de comando do rodape; avisos no historico |
| **Modo terminal** (`fwMSXc.exe --term`) | F11 abre a linha de comando; respostas longas vao para o log |
| **Ponte de controle** (`--ctl-port`) | qualquer programa manda os comandos por TCP (ver [Ponte de controle](#ponte-de-controle-externa---ctl-port)) |

### O Z80 Debugger na janela (F10)

Na janela do emulador existe um depurador visual, no estilo do depurador do fMSX: um painel que mostra o que o Z80 esta'
fazendo e deixa andar instrucao por instrucao. Ele usa a **fonte do proprio MSX** (a tabela de caracteres da BIOS), como a tela
de texto do MSX, entao os bytes de um dump aparecem com os graficos e simbolos especiais do MSX.

**Como entrar:** tecla **F10** (ou o menu **Depurador > Entrar no Z80 Debugger**). A maquina para e o painel abre sobre a
imagem. Ele tambem abre SOZINHO quando a execucao para num breakpoint, num watchpoint, num "rodar ate'" ou quando o emulador
foi aberto com `--debug`. **F10** de novo (ou **Esc**) fecha o painel deixando a maquina parada; o menu **Depurador >
Continuar a execucao** (ou a tecla **C**) a retoma. Com o painel aberto o teclado e' do depurador: nada chega ao MSX.

O que o painel mostra (nesta ordem, de cima para baixo):

```
Z80 Debugger - Disassembly
AF=0042 BC=00FF DE=00A3 HL=0105         <- registradores nas primeiras linhas
IX=0000 IY=0000 SP=F092 PC=C100
AF'=0000 BC'=0000 DE'=0000 HL'=0000
I=00 R=12 IM1 EI     F=-Z----N-           <- I, R, modo de interrupcao, EI/DI e as flags
                                          <- uma linha em branco separa os registradores
*>C100: 3E 05       LD A,05h              <- ENDERECO: bytes em hexa  instrucao com os parametros
  C102: C3 00 40    JP 4000h
  C105: 00          NOP
  ...
```

`>` marca o PC (a proxima instrucao a executar) e `*` um breakpoint; a linha com a barra clara e' o **cursor**.

| Tecla | Faz |
|---|---|
| **ENTER** | executa a proxima instrucao (o cursor acompanha o PC) |
| **Cima** / **Baixo** | instrucao anterior / instrucao abaixo (move o cursor) |
| **Esquerda** / **Direita** | uma pagina para cima / para baixo |
| **G** | vai para um endereco: digite ate' 4 digitos hexa e ENTER (Esc cancela, Backspace apaga) |
| **D** | vista de desmontagem (a padrao, descrita acima) |
| **M** | dump de memoria: `ENDERECO: 8 bytes em hexa` e, ao lado, os mesmos 8 bytes como caracteres do MSX; os registradores continuam em cima |
| **S** | pilha: palavras a partir de SP (`SP+00`, `SP+02`...); `>` marca SP |
| **J** | pula: o PC passa a ser a posicao do cursor |
| **R** | roda ate' o cursor: a maquina volta a rodar e o painel reabre sozinho quando chegar la' |
| **C** | continua a execucao (fecha o painel) |
| **B** | liga/desliga um breakpoint no cursor (o mesmo da linha de comando) |
| **Q** | encerra o emulador |
| **Esc** | fecha o painel (a maquina continua parada); na pergunta do G, cancela |

Dicas: use **G** e **M** para ver uma variavel (`G`, `C200`, `ENTER`); **B** numa instrucao + **C** deixa a maquina rodar ate' ali;
no painel, **ENTER** repetido (a tecla repete) anda varias instrucoes. Os comandos de console da secao anterior (`watch`,
`until`, `reg`...) continuam valendo ao mesmo tempo. Se a BIOS carregada nao tiver a fonte esperada, o painel aparece com a
fonte comum da interface.

### O painel Z80 Debugger na TUI

A TUI (`fwMSXc.exe --tui`) tem o MESMO painel da janela, desenhado como caixa de texto no terminal. Menu **Depurador > Painel Z80
Debugger**. Ele tambem abre SOZINHO quando o emulador conectado para num breakpoint, watchpoint ou "rodar ate'" (o aviso
`[evento] ...` aparece no historico e o painel abre por cima). O painel e' calculado no proprio emulador (o mesmo codigo da janela;
comando `dbgui`), entao o comportamento e' identico: registradores nas primeiras linhas, `ENDERECO: bytes instrucao`, `>` no PC, `*` num
breakpoint, a linha do cursor em destaque.

As teclas sao as da janela: **ENTER** passo, **Cima/Baixo**, **Esquerda/Direita** (ou PageUp/PageDown) pagina, **G** vai ao endereco (digite os
digitos hexa e ENTER; Backspace apaga), **D** desmontagem, **M** dump (8 bytes), **S** pilha, **J** pula para o cursor, **R** roda ate' o cursor,
**C** continua, **B** breakpoint no cursor, **Q** encerra o emulador, **Esc** ou **F10** fecham o painel (a maquina continua parada).
Num terminal baixo o painel mostra o cabecalho, o rodape (se couber) e a parte do corpo que contem o cursor; num terminal de 30 linhas ou
mais aparece inteiro. As letras estranhas do dump (os caracteres especiais do MSX) saem como `.` no terminal -- a janela mostra os
graficos do MSX de verdade.

Quem escreve outro programa pode usar o comando `dbgui` pela ponte de controle: `dbgui open`, `dbgui key <enter|up|down|left|right|g|d|m|s|j|r|c|q|b|esc|bs>`,
`dbgui char <hexa>` e `dbgui show`; a resposta e' `ok +33`, a linha `action=none|close|quit prompt=0|1 mode=disasm|memory|stack` e as 32 linhas
do painel, cada uma com uma letra de estilo na frente (T titulo, R registradores, C cursor, P pergunta, D ajuda, N normal).

### Passo a passo (um primeiro uso)

1. **Abra o emulador com a ponte e conecte o console.** No console:

   ```
   fwMSXc.exe --cli
   fwmsx> emu start --msx --cart jogo.rom --break 0x4010
   emulador iniciado e conectado (porta 51607)
   ```

   `--break 0x4010` ja' deixa um breakpoint armado (ver os parametros abaixo). O console assina os eventos sozinho.

2. **Espere a parada.** Quando o MSX executar a instrucao em `4010`, a maquina para ANTES dela e o console avisa:

   ```
   fwmsx:51607> wait
   [evento] break id=1 pc=4010 hits=1
   ```

   (Sem `wait`, o aviso aparece sozinho no proximo prompt.) Na janela, o titulo passa a dizer `(PARADO: depurador)`;
   na TUI, a barra mostra `PARADO (depurador)`; o comando `status` mostra `stopped=1`.

3. **Olhe o que esta' acontecendo.**

   ```
   fwmsx:51607> reg                      # todos os registradores, os sombra, I/R, IM/EI e as flags (SZ-H-PNC)
   fwmsx:51607> dis                      # desmonta a partir do PC; '>' marca o PC e '*' um breakpoint
   fwmsx:51607> dis 0x4000 12            # 12 linhas a partir de 4000
   fwmsx:51607> mem 0xC000 64            # hexdump (16 bytes por linha, com ASCII)
   fwmsx:51607> stack 8                  # 8 palavras a partir de SP
   fwmsx:51607> slots                    # de qual slot:subslot vem cada pagina de 16 KB agora
   ```

4. **Ande.**

   ```
   fwmsx:51607> stepi                    # UMA instrucao; responde "pc=4012  LD B,A"
   fwmsx:51607> stepi 20                 # vinte instrucoes
   fwmsx:51607> next                     # passo SOBRE: um CALL (ou RST, LDIR...) roda inteiro
   fwmsx:51607> until 0x4100             # roda ate' o endereco (ate' 20 quadros; se nao chegar, segue rodando)
   fwmsx:51607> go                       # continua; para no proximo breakpoint/watchpoint
   fwmsx:51607> stop                     # para na proxima fronteira de instrucao
   ```

5. **Pare quando a memoria for mexida (watchpoint).** Descubra quem escreve no placar, por exemplo:

   ```
   fwmsx:51607> watch 0xE000 2 w         # escrita em E000-E001 (r = leitura, rw = as duas)
   fwmsx:51607> go
   fwmsx:51607> wait
   [evento] watch id=2 addr=E000 rw=w value=05 pc=4A21 next=4A24
   ```

   O evento diz QUEM escreveu (`pc=4A21`, o endereco da instrucao), o valor e onde a execucao vai continuar. A maquina para
   DEPOIS da instrucao (a escrita ja' aconteceu); o breakpoint, ao contrario, para ANTES.

6. **Mude coisas.** `reg hl 0x1234` e `reg a 0x80` mudam registradores; `poke 0xC000 0x41` grava memoria (a ROM nao e'
   alterada); `jump 0x4000` muda o PC; `go 0x4000` muda o PC e continua.

7. **Limpe.** `breaks` e `watches` listam; `unbreak 0x4010` / `unwatch 0xE000` removem um; `unbreak all` / `unwatch all`
   removem todos.

### Referencia dos comandos

Enderecos e numeros aceitam `0x4010`, `$4010`, `4010h` ou decimal. O que o comando faz com a maquina parada ou rodando:
todos funcionam nos dois estados (inspecionar rodando le a memoria "ao vivo").

| Comando | Faz |
|---|---|
| `break <end>` | breakpoint de execucao: para ANTES da instrucao em `end`; responde o `id` |
| `unbreak <end>` / `unbreak all` / `breaks` | remove / remove todos / lista (`#id  endereco  hits=N`) |
| `watch <end> [tam] [r\|w\|rw]` | watchpoint de memoria; padrao: 1 byte, escrita; `tam` de 1 a FFFF |
| `unwatch <end>` / `unwatch all` / `watches` | remove / remove todos / lista |
| `stepi [n]` | executa `n` instrucoes (1 a 1000000) e fica parado; responde `pc=XXXX  <proxima instrucao>` |
| `next` | passo sobre: `CALL`, `CALL cc`, `RST`, `LDIR`/`LDDR`/`CPIR`/`CPDR`/`INIR`/`INDR`/`OTIR`/`OTDR` e `HALT` rodam por inteiro; o resto e' um `stepi` |
| `until <end>` | roda ate' `end` e para la' (espera ate' 20 quadros; se nao chegar responde `running` e o evento `! until` avisa) |
| `go [end]` | volta a rodar, opcionalmente a partir de `end`; responde `running` na hora |
| `stop` | para a maquina |
| `jump <end>` | muda o PC (sem rodar) |
| `dis [end] [n]` | desmonta `n` linhas (padrao 8) a partir de `end` (padrao: PC) |
| `mem <end> [n]` | hexdump de `n` bytes (padrao 64, ate' 1024) |
| `stack [n]` | `n` palavras (padrao 8) a partir de SP |
| `reg` / `reg <nome> <valor>` | mostra tudo / muda um registrador: `af bc de hl ix iy sp pc af' bc' de' hl'` e `a f b c d e h l i r` |
| `slots` | mapa de paginas: slot e subslot de cada pagina de 16 KB e se e' RAM |
| `events on` / `events off` | assina / cancela os avisos nesta conexao (o console e a TUI assinam sozinhos) |
| `status` | `... stopped=0\|1 ...` mostra se o depurador parou a maquina |

### Parametros do `--msx` (e de `emu start ...`)

| Opcao | Efeito |
|---|---|
| `--break <end>` | breakpoint inicial; pode repetir (`--break 0x0038 --break 0x4010`) |
| `--watch <end>[:tam[:r\|w\|rw]]` | watchpoint inicial; pode repetir (`--watch 0xC000:16:rw`) |
| `--debug` | comeca PARADA no PC 0000, antes da primeira instrucao: use `stepi`/`go` para andar |

Valem tambem depois de trocar o cartucho pela janela. Sem janela (`--frames N`), se a maquina parou o programa imprime
`depurador: parado em PC=XXXX (motivo)` e os avisos pendentes.

### Os avisos (eventos)

Linhas que o emulador manda SEM ninguem pedir, para quem assinou (`events on`):

```
! break id=1 pc=0038 hits=1                       <- um breakpoint foi atingido
! watch id=2 addr=C100 rw=w value=77 pc=C012 next=C015
! until pc=C003                                   <- o `until`/`next` chegou (so' quando nao deu para esperar na hora)
```

`stepi`, `next` e `until` ja' respondem na hora e nao geram aviso. No console eles aparecem como `[evento] ...`, na TUI no
historico, no modo terminal no log e na janela no `stderr`. Quem escreve outro programa (editor, IDE) le essas linhas pelo
socket da ponte de controle.

### Dicas

- **Parar no vetor de interrupcao** (`break 0x0038`) e' um bom jeito de "congelar" um jogo no inicio de um quadro.
- **Quem mexe nesta variavel?** use `watch` com `w`; **quem LE?** `r`; os dois: `rw` (cuidado: `rw` numa regiao que a
  BIOS toca o tempo todo para a cada instante).
- Os breakpoints criados por comando se perdem se a janela recriar a maquina (trocar o cartucho pelo menu); os de
  `--break`/`--watch` ficam.
- Um watchpoint desliga o caminho rapido de `LDIR`/`LDDR`: a emulacao fica um pouco mais lenta enquanto ele existir.
- O que ainda NAO existe (condicoes, simbolos, trace, parada em E/S e interrupcoes, painel na TUI) esta' no roteiro de
  [debug-spec.md](debug-spec.md), secao 7.

Detalhes tecnicos: [debug-spec.md](debug-spec.md).

## Abertura (splash), icone e configuracoes salvas

### A imagem de abertura (splash)

Ao abrir a janela do emulador, o fwMSX mostra por **3 segundos** a imagem de abertura (`fwMSX-splash.png`, ao lado do
executavel) antes de ligar o MSX; ela some em fade. Qualquer tecla ou clique pula; fechar a janela encerra o programa. A
maquina so' comeca depois, entao o boot nao e' atrasado nem perde teclas.

| Onde / como | O splash aparece? |
|---|---|
| Janela (`fwMSX.exe`, duplo clique, `--msx`) | **sim**, se a opcao estiver ligada (padrao: ligada) |
| `--no-splash` | **nao**, nesta execucao (vence `--splash`) |
| `--splash` | **sim**, nesta execucao, mesmo com a opcao desligada |
| `--cli`, `--tui` e `--term` | **nao** por padrao; **sim** so' com `--splash` (a imagem e' desenhada no terminal em meios blocos com cor de 24 bits, por 2,5 s, e so' se a saida for um terminal de verdade) |
| `emu start` (console e TUI) | **nao**: a janela aberta por eles nao mostra o splash, a menos que voce passe `--splash` (`emu start --splash`) |
| `--cmd` (um comando e sai; ainda por vir) | **nunca**, de modo algum, nem com `--splash` |

Para desligar de vez: menu **Ferramentas > Interface > Mostrar splash ao iniciar** (desmarcar). A escolha fica gravada.
Se o arquivo `fwMSX-splash.png` nao for achado, o programa simplesmente abre sem splash.

### Icone do aplicativo

O fwMSX tem icone proprio: no **Windows** ele esta' dentro do `fwMSX.exe` (e do `fwMSXc.exe`), entao aparece no Explorer,
na barra de tarefas, no Alt+Tab e no canto da janela. No **Linux**, a janela e a barra de tarefas usam o mesmo icone; para o
gerenciador de arquivos e o menu de aplicativos, instale o atalho que vai no pacote:

```
cp fwmsx.png ~/.local/share/icons/            # o icone
cp fwmsx.desktop ~/.local/share/applications/ # o atalho (ajuste Exec= para o caminho do fwMSX, se precisar)
```

### Configuracoes salvas (`fwmsx.cfg`)

O tema, o tamanho da letra, a moldura da tela e o splash ficam gravados em `fwmsx.cfg`, um arquivo de texto ao lado do
executavel (criado so' quando algo muda no menu **Ferramentas > Interface**). Pode editar a mao:

```
splash=1     # 1 = mostra o splash ao iniciar, 0 = nao
theme=0      # 0 = escuro, 1 = claro
font=1       # 0 = normal, 1 = grande, 2 = muito grande
frame=1      # 1 = borda e sombra em volta da tela do MSX
```

Detalhes: [splash-spec.md](splash-spec.md).

## Montador Z80 (`fwmsx --asm`)

O fwMSX inclui um montador compativel com o asMSX 1.2.0 (reescrito do zero a partir do manual, em C, C++, Fortran e
Assembly). E' um compilador completo para o MSX: gera cartuchos **ROM e MegaROM** (Konami, Konami SCC, ASCII8, ASCII16),
programas para o **BASIC** (`.bin`) e o **MSX-DOS** (`.com`), **fitas** (`.cas` para emuladores, `.wav` para um MSX de verdade) e
`.tap` do ZX Spectrum. Uso:

    fwmsx --asm [-z] [-s] [-vv] [-o <caminho>] <arquivo.asm>

| Opcao | Efeito |
|-------|--------|
| `-z` | sintaxe Zilog padrao, sem precisar da diretiva `.zilog` |
| `-s` | modo silencioso (sem mensagens de progresso) |
| `-vv` | mensagens detalhadas |
| `-o base` | nome-base (sem extensao) das saidas; `-o pasta/` grava na pasta com o nome do fonte |
| `-r` | aceito e ignorado (obsoleto: includes sao relativos ao fonte) |

Exemplo -- um cartucho de 16 KB (monte com `asm hello.asm` no console e rode com `emu start --cart hello.rom`):

```
        .page 1
        .start init
        .rom
  init: ld hl,msg
  loop: ld a,[hl]
        or a
        jr z,fim
        call 00A2h          ; CHPUT
        inc hl
        jr loop
  fim:  jr fim
  msg:  db "Ola do fwasm!",0
```

As diretivas de formato: `.rom`, `.megarom konami|konamiscc|ascii8|ascii16` (com `.subpage n at X` e `.select n at X`), `.basic`, `.msxdos`, `.sinclair`,
mais `.cas`/`.wav "NOME"` para a fita, `.size KB`, `.start`, `.search`, `.callbios`, `.calldos`. O manual completo, com todas as diretivas, mappers e exemplos,
esta' no proprio fwMSX: `man fwasm`, `man fwasm-formats`, `man fwasm-megarom`, `man fwasm-directives`.

Os arquivos de saida (`.z80`, `.rom`, `.bin`, `.com`, `.cas`, `.wav`, `.tap`, `.sym`, `.txt`, conforme as diretivas do fonte) ficam ao lado do
fonte, ou onde o `-o` indicar. O codigo de saida e' o do asMSX (0 = ok; erros e avisos vao para o stderr no
formato `arquivo.asm, line N: mensagem`). No console (`fwMSXc.exe --cli`) o mesmo montador e' o comando `asm`.
Detalhes e estado do suporte as diretivas: `doc/asm-spec.md`.

## Mamute -- monitor Z80 (`mon`, `fwmsx --mon`)

O **Mamute** e' o monitor Z80 do fwMSX: um prompt de linha onde cada comando mexe na memoria, disassembla, executa e salva programas
(no estilo dos monitores dos anos 80). Funciona no console (`--cli`), na TUI (`--tui`) e pela ponte de controle, sem janela separada.
**Todo endereco e' hexadecimal.** O manual completo esta' no proprio programa: `man mamute`.

### Como abrir

| Forma | O que faz |
|---|---|
| `mon` (ou `mamute`) | entra no monitor; o prompt vira `MON(off)> ` (offline) ou `MON(on)> ` (online); `BA` ou `QUIT` volta ao console |
| `mon <linha>` | executa UM comando do monitor, sem entrar nele (`mon d 4000,400f`) |
| `fwmsx --mon` (alias `--mamute`) | abre o console ja' dentro do monitor; com `--attach <porta>` liga a um emulador |
| `!comando` (dentro do monitor) | roda um comando do CONSOLE (`!emu start`) e continua no monitor |

### Online e offline

- **Offline**: memoria e CPU PRIVADAS do monitor (16 blocos de 16 KB: 4 slots x 4 paginas). Serve para criar, depurar e rodar um programa sem tocar na maquina.
  E' o modo de quem abre o monitor sem emulador.
- **Online**: a memoria e a CPU da MAQUINA emulada. O que o monitor escreve aparece na hora no programa em execucao: escreva uma rotina em `C000` e, no BASIC da janela,
  `DEFUSR=&HC000:A=USR(0)`. A escrita em ROM e' ignorada (como num MSX de verdade).
- `ONLINE` e `OFFLINE` trocam de modo; `PUSH ini,fim[,dest]` leva um bloco do offline para a maquina e `PULL` traz de volta.

### Comandos

| Grupo | Comandos |
|---|---|
| Memoria | `PAGE [?\|a,b,c,d]` `DM end[,desloc]` `M end[,b1,b2..]` `SH [end],b1,,b3` / `SH [end],'texto` `MS end,[desloc],'texto` `T ini,fim,dest` `F ini,fim,byte` |
| Despejo | `C modo` (0 = 4 B/linha, 1 = 16 B, 2 = 8 B + checksum, 3 = idem sem o endereco) `D ini[,fim]` `P ini[,fim][,arq]` (texto) `V ini[,fim]` (VRAM) |
| Execucao | `L [ini[,fim]]` `LP` (disassembler) `X [reg [valor]]` `G [ini[,bp1[,bp2]]]` |
| Telas | `DM` `M` `S` `ZAP arq.dsk,setor` `SCR end,dx,dy` - paineis na TUI, comandos de linha no console - ver "Telas" abaixo |
| Fonte Z80 | `EDIT` (editor de linhas numeradas) `LIST` `A [opcoes][/desloc][>nome]` (monta pelo fwasm) `MAP` - ver "Editor de fonte e montagem" abaixo |
| Arquivos | `LOAD arq[,slot[,end]]` `SAVE arq,ini,fim[,exec[,BIN\|ROM\|RAW]]` |
| Modos | `ONLINE` `OFFLINE` `STATUS` `PUSH` `PULL` |
| Configuracao | `CFG` `CFG SET slot,pagina,tipo[,arq[,HALF]]` `CFG VRAM n` `CFG ONLINE 0\|1` `CFG DUMP n` `CFG SAVE` `CFG LOAD` |
| Outros | `HELP` `BA` `QUIT` `XQT` |

Exemplo (offline, depois online):

```
fwmsx> mon
MON(off)> m c100,06,05,05,20,fd,3e,77,76       ; LD B,5 / DEC B / JR NZ / LD A,77 / HALT
MON(off)> g c100
PARADO EM HALT NO ENDERECO C107 (...)   A=77 B=00 ...
MON(off)> !emu start
MON(off)> online
MODO ONLINE: o monitor agora le e escreve na MAQUINA EMULADA
MON(on)> push c100,c107            (so' vale do offline para a maquina: ver man mamute-modes)
```

### Telas (`DM`, `M`, `S`, `ZAP`, `SCR`)

Na **TUI** (`fwMSXc.exe --tui`, e depois `mon`) estes cinco comandos abrem **paineis interativos**; no console (`--cli`), em scripts e pela ponte de controle sao **comandos de linha** (a grade uma vez, as mudancas por argumentos).

| Comando | O que mostra |
|---|---|
| `DM end[,desloc]` | 128 bytes (16 linhas de 8) em hexa e texto; `DM` sozinho continua 128 bytes depois (so' linha) |
| `M end` / `M end,b1,b2..` | a grade de edicao rapida / grava bytes |
| `S end[,teclas]` | como o `M`, digitando com o **teclado S** (`1234 qwer asdf zxcv` = `1234 5678 9ABC DEF0`; `skeys=` no `mamute.cfg`, `CFG SKEYS`) |
| `ZAP arq.dsk,setor[,desloc]` / `ZAPW arq,setor,byte,b1..` | um setor de uma imagem de disco (512 bytes; o rotulo e' o deslocamento no setor) / grava direto |
| `SCR end,dx,dy[,modo[,zoom]]` | a memoria como uma tela de 256x192 pixels (32x24 caracteres de 8 bytes) em braille; `dx,dy` e' o azulejo; modo 0 le por linha, 1 por coluna; zoom 1 = 128x48 colunas, 2 = 64x24 |

Teclas dos paineis: **setas** movem; **TAB** alterna hexa/texto; **PgUp/PgDn** pulam 128 bytes; **+ / -** ajustam o deslocamento do texto (nao mudam a memoria: so' como o texto e' lido e escrito); no `DM`/`ZAP` o **ENTER**
abre o campo de edicao (confirma com ENTER, cancela com ESC); no `M`/`S` dois digitos hexa gravam direto; no `ZAP` **Ctrl+S** grava so' o setor do cursor (o titulo mostra `*` ate' salvar). No `SCR`: setas mexem o endereco base
(1 byte / 1 azulejo), **TAB** a moldura 2x2, **E** o endereco, **Z** o zoom, **ENTER** a edicao dos 16x16 pixels (espaco inverte, I inverte tudo, L apaga, ENTER sai, ESC desfaz tudo). **ESC** fecha o painel.
Escrever em ROM ou celula vazia nao grava e o painel avisa. Manual completo: `man mamute-screens`.

**PDF**: `P`, `LP`, `V ...,arq`, `A P` e `A H` gravam um **PDF A4 em Courier** quando o nome termina em `.pdf` (padroes `mamute.pdf`, `mamute-dis.pdf`, `NOME.pdf`, `NOME_labels.pdf`); com outro nome (ou `>nome.lst` no `A`) saem em texto.

### Editor de fonte e montagem (`EDIT`, `A`)

`EDIT` entra num editor de programa-fonte Z80 de linhas **numeradas** (estilo MegaAssembler/ZX-81; prompt `EDIT> `). A linha e' `NN [Label:] instrucao operandos [;comentario]`; o numero da linha e' decimal,
**os numeros no texto sao HEXADECIMAIS por padrao** (`LD A,10` carrega 16; sufixos `H`, `B` binario `1010B`, `D` decimal `10D`; um numero que comeca com A-F leva um `0` na frente). Digitar o mesmo numero
substitui a linha; so' o numero apaga. `QUIT` volta ao monitor (o programa fica); `BA` sai de tudo.

| Comando | O que faz |
|---|---|
| `LIST [a[-[b]]]` `NEW` `DELETE a[-[b]]` `RENUM [novo[,antigo[,passo]]]` | ver e reorganizar as linhas |
| `CHANGE 'a'[,'b']` `SEARCH 'x'` / `SEARCH x` `LSEARCH` | trocar / procurar (com aspas as maiusculas contam) |
| `SAVE arq` `LOAD arq` `MERGE arq` | arquivo `.mza` em ASCII |
| `A [opcoes][/desloc][>nome]` | monta pelo **fwasm** e lista (linha, endereco, bytes, texto); erro mostra a linha com o cursor `>` |
| `MAP` | inicio e fim do ultimo codigo montado |

Opcoes do `A` (coladas): `O` grava o codigo na memoria do monitor (offline, ou **na maquina** no modo online), `N` sem a coluna do numero, `P` listagem em `NOME.lst`, `I` objeto em `NOME.bin` (BSAVE),
`R` referencia cruzada, `S`/`D` lista de simbolos (alfabetica / em ordem de aparicao), `H` grava as listas em `NOME.sym`, `/desloc` soma a todo `ORG`, `>nome` escolhe o nome-base.

```
MON(off)> edit
EDIT> 10 ORG 0C100H
EDIT> 20 LD A,2A
EDIT> 30 RET
EDIT> a o
...  GRAVADOS 3 BYTES NA MEMORIA (OFFLINE) EM C100-C102
EDIT> quit
MON(off)> g c100          (offline)   ou   online + a o   e, no BASIC, DEFUSR=&HC100:A=USR(0)
```

### Configuracao inicial (`mamute.cfg`)

Arquivo de texto `chave=valor` ao lado do executavel, lido quando o monitor abre: `online=0|1`, `vram=16|128|192`, `dump=0..3`,
`cell.S.P=vazio|ram|rom|basic`, `file.S.P=caminho`, `half.S.P=1`. Sem ele: offline, VRAM de 16 KB, formato 0 e RAM no slot 3. `CFG` mostra e muda
(`CFG SAVE` grava). Detalhes: `man mamute-config`.

### O que ainda nao existe

`R` (referencia de fita) e os comandos do SUPER-X (prefixo `X`) do Mamute original: ver as fases em [mamute-spec.md](mamute-spec.md).

## Banco de ROMs (`--romdb`)

O banco guarda as ROMs que voce tem no disco (pelo SHA-1), com nome, tipo de hardware, mapper e
notas. As ROMs **nao** vem com o emulador: voce baixa de fontes publicas, pelo menu **ROMs** da
janela ou pela CLI. Elas ficam em `roms/` (ao lado do executavel, ou `--roms <pasta>`), separadas por
tipo (`bios`, `interfaces`, `cartuchos`, `discos`, `tabelas`, `outros`). O banco e' `roms/roms.db`.

```powershell
.\dist\fwMSX.exe --romdb fmsx                          # fMSX 6.0 para Windows (ROMs e CARTS.SHA)
.\dist\fwMSX.exe --romdb filehunter                    # lista a pasta System ROMs do file-hunter
.\dist\fwMSX.exe --romdb filehunter-full               # baixa o Full Set System ROMs mais recente
.\dist\fwMSX.exe --romdb filehunter-full --data 26-08-2025   # ou o de outra data
.\dist\fwMSX.exe --romdb vampier                       # banco do Vampier (referencia de nomes)
.\dist\fwMSX.exe --romdb cartsha roms\tabelas\CARTS.SHA   # mappers do fMSX
.\dist\fwMSX.exe --romdb scan roms                     # cadastra as ROMs de uma pasta
.\dist\fwMSX.exe --romdb identify                      # da nome as ROMs que o Vampier conhece
.\dist\fwMSX.exe --romdb search gradius --cat cartucho
.\dist\fwMSX.exe --romdb show 655
.\dist\fwMSX.exe --romdb edit 655 --hw "Konami SCC" --notas "minha copia"
.\dist\fwMSX.exe --romdb del 655                       # tira do banco (o arquivo nao e' apagado)
.\dist\fwMSX.exe --romdb verify                        # recalcula o SHA-1 de cada ROM e confere com o banco
```

Na janela, o menu **ROMs** tem os downloads, a navegacao pelo file-hunter e a janela **Banco de ROMs**
(busca, edicao, exclusao e busca no Vampier). Downloads rodam em segundo plano.

Desde a 1.25.0, `--cart <arquivo>` (ver a secao do emulador, acima) SEM mapper explicito tambem
consulta este banco pelo SHA-1 do cartucho (`CARTS.SHA` ja' importado) antes de cair na heuristica
por tamanho/conteudo de sempre -- nenhuma opcao nova e' necessaria, e um mapper escolhido a dedo
continua tendo prioridade.

**Requisitos**: o programa `curl` (Windows 10 1803+ ja tem; Linux, instale o pacote `curl`).

**Limites**: o JSON do Vampier nao e' usado (so' o SQL). O parser do file-hunter depende do layout atual
do site. Montar a maquina pelo banco (layout por nome) ainda nao existe. Ver [romdb-spec.md](romdb-spec.md).

## Fita por linha de comando (`fwmsx --cas`)

Empacota um `.BIN`/`.BAS` solto num `.TSX` valido, "ripa" uma gravacao real (`.wav`) de volta para
`.TSX`, ou lista o conteudo de uma fita -- tudo sem abrir o emulador. Ver [tape-spec.md](tape-spec.md),
secoes 8 e 9, para o detalhamento completo.

```powershell
# Empacotar um binario (BLOAD "CAS:") -- precisa dos enderecos de inicio/fim/execucao
.\dist\fwMSX.exe --cas pack --tipo bin --nome JOGO --inicio 0x8000 --fim 0x81FF --exec 0x8000 jogo.bin saida.tsx

# Empacotar um programa BASIC ja tokenizado (CLOAD "CAS:") -- ex.: extraido com BSAVE dentro do emulador
.\dist\fwMSX.exe --cas pack --tipo bas --nome TESTE programa.bas saida.tsx

# Acrescentar mais um arquivo numa fita existente, em vez de criar uma nova
.\dist\fwMSX.exe --cas pack --tipo bin --nome N2 --inicio 0x9000 --fim 0x9100 --exec 0x9000 --anexar saida.tsx entrada2.bin saida.tsx

# "Ripar" uma gravacao real de fita (.wav PCM mono, 8 ou 16 bits) para .TSX
.\dist\fwMSX.exe --cas rip --tolerancia 25 gravacao.wav saida.tsx

# Listar o conteudo de uma fita (indice, tipo, nome, tamanho dos dados)
.\dist\fwMSX.exe --cas list saida.tsx
```

**Limites**: `--tipo bas` espera o arquivo JA tokenizado (nao tokeniza texto solto -- um programa BASIC
do MSX embute ponteiros de memoria entre linhas, dependentes do endereco de carga); so' `.BIN`/`.BAS`
(sem `--tipo ascii`); o `rip` nao tem os modos interativo/preditivo do makeTSX original (um bit ambiguo
termina o bloco corrente, em vez de tentar adivinhar); um `.wav` estereo e' rejeitado (so' mono).

## Banco de fitas (`fwmsx --fitadb`)

Cadastra metadados (titulo, empresa, ano, SHA-1) das fitas (`.cas`/`.tsx`/`.tzx`) que voce ja' tem no
disco -- pela extensao e pelo conteudo, igual o `--romdb` faz para ROMs. **Sem download nenhum**: o
site de referencia nao publica termos de uso (ver [tape-spec.md](tape-spec.md), secao 11). Elas ficam
em `fitas/` (ao lado do executavel, ou `--fitas <pasta>`); o banco e' `fitas/fitas.db`.

```powershell
.\dist\fwMSX.exe --fitadb scan fitas                         # cadastra toda fita da pasta (recursivo)
.\dist\fwMSX.exe --fitadb add jogo.tsx --empresa Konami --ano 1987
.\dist\fwMSX.exe --fitadb list
.\dist\fwMSX.exe --fitadb search nemesis
.\dist\fwMSX.exe --fitadb show 1
.\dist\fwMSX.exe --fitadb edit 1 --titulo "Nemesis" --notas "minha copia"
.\dist\fwMSX.exe --fitadb del 1                               # tira do banco (o arquivo nao e' apagado)
.\dist\fwMSX.exe --fitadb stats
```

O `titulo` e' auto-preenchido com o nome do 1o arquivo encontrado dentro da fita (pelo mesmo leitor
que a janela e o `--cas` usam) quando ela e' cadastrada por `scan`/`add` -- um titulo editado
manualmente nunca e' sobrescrito depois. `empresa`/`ano` so' vem de voce (nenhum formato de fita
guarda essa informacao).

**Limites**: so' cadastra fitas que voce ja' tem -- sem busca nem download de nenhuma fonte externa.

## Compilar sem a GUI (sem GLFW/OpenGL)

A GUI (Dear ImGui + GLFW + OpenGL3) vem ligada por padrao. Pra compilar
so a versao console (CLI/shell/TUI), sem essa dependencia:

```powershell
cmake -S . -B build -G Ninja -DFWMSX_MSXDISK_GUI=OFF
cmake --build build
```

`--gui`/`call gui` nessa build so avisam que a GUI nao foi compilada. Para
compilar sem audio: `-DFWMSX_AUDIO=OFF` (o emulador roda mudo).

## Problemas comuns

- **O console (`--cli`) troca letras, fica lento ou fecha sozinho (Windows)**: voce abriu o `fwMSX.exe`
  (programa de janela): o terminal nao o espera e disputa o teclado com ele. Use o **`fwMSXc.exe --cli`**.
- **Nao acho a porta da ponte de controle**: ela aparece no titulo da janela (`controle: porta N`) e
  em `fwmsx.port` (pasta do executavel e pasta de trabalho). Use `--ctl-port 7777` para uma porta fixa.
- **O build do Windows falha com "Permission denied" em `fwMSX.exe`**: ainda ha' uma janela do
  emulador (ou do console) aberta com esse executavel. Feche-a e compile de novo.
- **Uma tecla "ficou presa" no MSX**: **Maquina > Soltar todas as teclas**.
- **`CALL FORMAT` diz erro de disco**: a imagem precisa existir e ter o tamanho do formato (crie com
  `--disknew`); discos protegidos contra gravacao (`--disk-ro`) respondem "Write protect".

- **`cmake` ou `gcc` errado sendo usado / erros estranhos de link**: quase
  sempre e outro toolchain (ex.: FPC, Visual Studio) na frente do UCRT64
  no `PATH`. Use `.\build.ps1` (que forca o UCRT64 na frente) ou abra o
  shell **MSYS2 UCRT64** dedicado.
- **Saida do modulo Fortran aparece fora de ordem**: se voce mexer em
  `src/fortran/init_fortran.f90`, mantenha o `FLUSH(output_unit)` apos
  o `PRINT` -- o runtime do Fortran usa buffer de E/S proprio, separado
  do `std::cout`/`printf` do C/C++ (detalhes em
  [SPEC.md](SPEC.md#32-notas-de-implementacao-para-quem-retomar-o-projeto)).
