# fwMSX -- Manual de compilacao e execucao

> Como compilar e executar o fwMSX (emulador MSX1/MSX2, `msxdisk` embutido e
> o depurador do Z80). O estado de cada parte esta em [SPEC.md](SPEC.md) e nas
> specs de fase (`doc/*-spec.md`).

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

**Desde a v1.12.0**, sem nenhum argumento isso abre a maquina MSX1 completa
numa janela (os mesmos padroes de `--msx`: BIOS `resource/fMSX/ROMs/MSX.ROM`
ao lado do executavel, sem cartucho/disco) -- ver secao "Emulador MSX1 numa
janela" abaixo e `doc/machine-spec.md`.

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

- **Windows**: `fwMSX-X.Y.Z.zip` (executavel `fwMSX.exe`, `msxdisk.exe`, README, licenca, MANUAL e RELEASE).
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

## Emulador MSX (`--msx`) -- v1.30.0

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
| `--keys "texto"` | digita o texto no teclado do MSX; `|` = Enter (`\"` para aspas; `(`, `)`, `*` e `@` ja' saem certos) |
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
| `--vdplog` | mostra os registradores do VDP que mudam, quadro a quadro |
| `--mute` | sem audio |

**Modo sem janela:** a maquina roda sem abrir janela quando ha `--keys` ou `--shot`. Nesse
modo, `--text`, `--fmstat` e `--wav` funcionam no fim da execucao. Sem `--keys` nem `--shot`, a
janela abre.

### A janela

- **Teclado**: mapeamento posicional (layout US). Alt esquerdo = GRAPH, Alt direito = CODE, End =
  SELECT, Pause = STOP, F6/F7 = salvar/carregar estado, F8/F9 = slot de estado, F11 = tela cheia, F12 = captura de tela (PNG). Teclado numerico = o do MSX.
- **Joystick**: setas + Z ou Espaco (botao A) + X (botao B); ou gamepad (menu Joystick).
Os menus foram reagrupados na 1.30.0 (antes eram 13 menus separados no topo da janela; agora sao 9):

- **Arquivo**: Carregar cartucho, **Salvar estado.../Carregar estado...** (`.sst` -- ver
  `doc/savestate-spec.md`), Sair.
- **Maquina**: Reiniciar, **Modelo** (MSX1, MSX2, MSX2+), **Configuracao de disco...**,
  **Configuracao de slots...** (secao abaixo), Pausar.
- **Midia**: agrupa os tres submenus de midia removivel --
  - **Disco**: inserir e ejetar A: e B:.
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
- **Som**: mudo e volume.
- **Joystick**: status do gamepad em cada porta.
- **ROMs**: banco de ROMs (menu completo, ver `doc/romdb-spec.md`).
- **Ferramentas**: **Interface** (tema escuro/claro, tamanho da letra, moldura da tela), mais os
  placeholders existentes ("em breve").
- **Ajuda**: teclado, versao.

Trocar o modelo, o cartucho ou o FM-PAC reinicia a maquina. A SRAM (`.sav`) e' gravada antes.

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

**Funciona (v1.30.0):**

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
- **Disco novo**: **Midia > Disco > Novo disco em branco em A:/B:** cria um disquete formatado (5 1/4 face simples 180 KB, 5 1/4 face dupla 360 KB, 3 1/2 face simples 360 KB, 3 1/2 face dupla 720 KB -- o MSX nao tem 1,44 MB) e ja' o insere. Pela linha de comando: `fwMSX.exe --disknew disco.dsk ds35` (`ss525`, `ds525`, `ss35`, `ds35`). Dentro do MSX, `CALL FORMAT` (Disk BASIC) formata o disco inserido. Ver `doc/diskfmt-spec.md`.
- **Save-state**: menu **Arquivo > Salvar estado.../Carregar estado...** (`.sst`; atalhos **F6** salva / **F7** carrega o slot atual (`fwmsx-estado-N.sst`), **F8/F9** trocam de slot (1 a 9); avisa na tela se a BIOS/cartucho mudou desde o save; **F12** captura a tela em PNG) -- grava/restaura
  Z80, VDP, PSG, SCC, OPLL, PPI, controladora de disco e RAM/RAM de mapper, aplicado sobre a
  maquina ja' rodando (nao recarrega BIOS/cartucho/disco/fita). Ver [savestate-spec.md](savestate-spec.md).

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
- **Sem**: GameMaster2, cartucho MSX-MUSIC com BIOS propria, e `CALL VOICECOPY`. Save-state nao
  valida se o cartucho/disco/fita inserido agora e' o mesmo de quando foi salvo. **Confirmado
  pelo usuario na janela em 2026-10-08**: salvar e carregar estado (**Arquivo > Salvar
  estado.../Carregar estado...**) funcionam normalmente.
- **Outras BIOS** (ex.: Gradiente Expert 1.1): o layout aceita, mas o hardware que a BIOS espera nao foi testado.

Lista completa e atualizada: [RELEASE.md](RELEASE.md) (secao da versao) e [SPEC.md](SPEC.md), secao 5.0.

### Documentacao por assunto

[machine-spec.md](machine-spec.md) (maquina e janela), [slots-spec.md](slots-spec.md) (layout de slots),
[fm-spec.md](fm-spec.md) (FM, MSX-MUSIC e FM-PAC), [sram-spec.md](sram-spec.md) (SRAM e `.sav`),
[audio-spec.md](audio-spec.md) (audio), [fdc-spec.md](fdc-spec.md) (disco), [msx2-spec.md](msx2-spec.md) e
[msx2p-spec.md](msx2p-spec.md) (MSX2 e MSX2+), [scc-spec.md](scc-spec.md) (SCC), [memory-map-spec.md](memory-map-spec.md) (mapa de memoria).

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
