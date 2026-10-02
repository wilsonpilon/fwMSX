# fwMSX

**fwMSX** e um projeto de aprendizado com o objetivo final de se tornar um
emulador de MSX, evoluido a partir do [fMSX](https://fms.komkon.org/fMSX/)
(Marat Fayzullin), com o aval do autor original para essa adaptacao. A base
do fMSX e C/Unix; a ideia aqui e reestrutura-la, modernizar e, ao longo do
caminho, estudar e reaproveitar codigo de outros emuladores/ferramentas de
MSX (ver [resource/](resource/)).

> **Este projeto NAO substitui o fmsxGO** (o projeto principal). O fwMSX e
> apenas para aprendizado: praticar o ferramental MSYS2 no Windows e me
> atualizar com C/C++/Fortran/Assembly mais atuais.

Por se tratar de um projeto de estudo, ele tem uma regra propria, alem de
"emular um MSX direito": **toda fase relevante do projeto precisa exercitar
codigo real em C, C++, Assembly e Fortran** -- mesmo quando minimo -- para
forcar contato pratico com interoperabilidade entre linguagens (ABI,
name mangling, calling conventions, linkedicao).

## Estado atual

Ainda **nao existe emulacao de MSX completa** (sem PSG, sem janela
grafica de verdade, sem PPI/teclado ainda). O `main()` de `fwMSX.exe`
continua "inicializando" um modulo de cada linguagem (C++, C, Assembly
e Fortran) via `init_cpp()`/`init_c()`/`init_asm()`/`init_fortran()`
quando rodado sem argumentos, exatamente como no esqueleto original --
essa e a base sobre a qual o emulador de fato foi sendo construido nas
fases seguintes.

A v1.2.x entregou o primeiro utilitario "de verdade" construido nesse
processo de aprendizado: o **msxdisk**, um manipulador completo de
imagens de disco MSX (`.dsk`, FAT12, MSX-DOS 1/2 com subdiretorios), num
unico executavel (`dist/msxdisk.exe`) com quatro modos -- CLI, shell
interativo, TUI (Norton Commander/XTree) e GUI (Dear ImGui) -- tambem
acessivel embutido via `fwmsx --msxdisk`. Ver
[doc/msxdisk-spec.md](doc/msxdisk-spec.md) para a especificacao completa.

A v1.3.0 trouxe o **nucleo da CPU Z80**: motor de despacho fiel ao fMSX
(C + wrapper C++), tabelas de flag geradas em Fortran, aceleracao de
`LDIR`/`LDDR` em Assembly dual-ABI Win64/SysV (primeiro `.asm` do
projeto portavel pra Linux -- ver `build.sh`), e um depurador embutido
em `fwMSX.exe` (`fwmsx --z80dbg`: registradores, memoria, breakpoints e
desmontador). Ver [doc/z80-core-spec.md](doc/z80-core-spec.md).

A v1.4.0 trouxe o **mapa de memoria MSX** (slots/subslots): motor fiel
ao fMSX em C, `MemorySystem`/`SlotMemoryBus` em C++ com uma API de
depuracao que enxerga qualquer slot independente do que a CPU ve agora,
carregamento de ROM real com CRC32 em Fortran, e seis mappers MegaROM de
troca de banco (Konami/ASCII/generic). `fwmsx --z80dbg --slots
resource/fMSX/ROMs/MSX.ROM` roda a **BIOS MSX1 real** no nucleo Z80. Ver
[doc/memory-map-spec.md](doc/memory-map-spec.md).

A v1.4.1 **validou o build em Linux de verdade** (WSL2): build completo
+ `ctest` (328 verificacoes) passando, incluindo a branch `elf64`/SysV
do Assembly dual-ABI do nucleo Z80 -- so tinha sido montada antes, nunca
linkada/executada. Duas correcoes reais de portabilidade em modulos
Assembly mais antigos do projeto (`src/asm/init_asm.asm`,
`src/msxdisk/asm/name_match.asm`, Win64-only desde antes do nucleo Z80
existir). Pacote Linux (`.tar.gz`) disponivel, gerado por `build.sh`.

A v1.5.0 trouxe o **VDP** (TMS9918/V9938) -- a primeira vez que o
projeto desenha pixels de verdade. `CompositeBus` (design proprio)
multiplexa I/O de porta entre o mapa de memoria e o VDP; motor
"digital" do VDP em **C** (registradores, protocolo de porta `98h`-
`9Bh`, maquina de estados de scanline/interrupcao) finalmente da uso
real ao `Z80Cpu::interrupt()`, confirmado com um programa sintetico
recebendo a interrupcao de VBlank de verdade; renderizacao real de
SCREEN 0/1/2 (texto mono, texto colorido, bitmap), exportavel como
imagem PPM via `vdpshot`, sem janela ainda; tabela de paleta de 512
cores em **Fortran**, finalmente com consumidor. `fwmsx --z80dbg
--slots resource/fMSX/ROMs/MSX.ROM --vdp` e' o comando mais completo de
depuracao que o projeto tem hoje. Ver [doc/vdp-spec.md](doc/vdp-spec.md).

A v1.6.0 trouxe os **sprites** do VDP (Fase 3): SCREEN 1/2/3 com 8x8/
16x16, ampliacao, prioridade, limite de 4 por linha, flag de quinto
sprite e de colisao, em **C**, adaptados do fMSX. Ver
[doc/vdp-spec.md](doc/vdp-spec.md).

A v1.7.0 trouxe o **PPI i8255 e o teclado** (portas `A8h`-`ABh`): chip
em **C** adaptado do fMSX, `PpiDevice` em **C++** ligado ao mapa de
memoria (o slot primario passa a mudar pelo PPI, como no hardware),
tabela de posicao das 87 teclas na matriz em **Fortran** e contagem de
teclas pressionadas em **Assembly** dual-ABI. `fwmsx --z80dbg --slots
resource/fMSX/ROMs/MSX.ROM --vdp --ppi` ganha `ppiregs`/`keys`/
`keydown`/`keyup`. A BIOS real ja programa e le o PPI, mas ainda nao
passa da varredura de RAM -- ver [doc/ppi-spec.md](doc/ppi-spec.md).

**Trabalho no core de emulacao continua em andamento** -- faltam
descobrir por que a BIOS nao sai da varredura de RAM, modos MSX2 + janela
de verdade (Fase 4 do VDP), PPI/
teclado e PSG para existir uma maquina MSX completa. Ver
[doc/SPEC.md, secao 5.0](doc/SPEC.md) para os proximos passos
registrados, pra retomar sem se perder.

Veja [doc/SPEC.md](doc/SPEC.md) para a especificacao completa e o historico
de fases (documento vivo, atualizado a cada mudanca relevante).

## Estrutura do projeto

```
fwMSX/
├── src/            fontes do projeto, um subdiretorio por linguagem
│   ├── cpp/        ponto de entrada (main) e modulo C++
│   ├── c/          modulo C
│   ├── asm/        modulo Assembly (NASM, ABI Win64)
│   ├── fortran/    modulo Fortran
│   ├── common/     cabecalhos compartilhados (versao, etc.)
│   ├── msxdisk/    utilitario de imagens de disco MSX (core/cli/shell/
│   │               tui/gui/config -- ver doc/msxdisk-spec.md)
│   ├── z80/        nucleo da CPU Z80 (core/cpp/asm/fortran/debug --
│   │               ver doc/z80-core-spec.md)
│   ├── memmap/     mapa de memoria MSX -- slots/subslots/MegaROM
│   │               (core/cpp/fortran -- ver doc/memory-map-spec.md)
│   ├── vdp/        VDP (TMS9918/V9938) -- registradores/portas/
│   │               renderizacao (core/cpp/fortran -- ver doc/vdp-spec.md)
│   └── ppi/        PPI i8255 + teclado (core/cpp/fortran/asm -- ver
│                   doc/ppi-spec.md)
├── tools/msxdisk/  ponto de entrada do executavel msxdisk standalone
├── tests/z80/      testes do nucleo Z80, mapa de memoria e VDP (CTest --
│                   z80test/z80dbgtest/memmaptest/vdptest/ppitest)
├── doc/            documentacao viva do projeto
│   ├── SPEC.md         especificacao completa + fases do projeto
│   ├── msxdisk-spec.md especificacao + fases do utilitario msxdisk
│   ├── z80-core-spec.md especificacao + fases do nucleo Z80
│   ├── memory-map-spec.md especificacao + fases do mapa de memoria
│   ├── vdp-spec.md      especificacao + fases do VDP
│   ├── ppi-spec.md      especificacao + fases do PPI/teclado
│   ├── MANUAL.md       como compilar e executar
│   ├── CHANGELOG.md    resumo das alteracoes entre versoes
│   └── RELEASE.md       detalhes de cada release
├── dist/           pacote pronto para execucao (.exe + .zip)
├── resource/       codigo-fonte de terceiros usado como referencia/estudo
├── CMakeLists.txt  build raiz (CMake + Ninja)
├── build.ps1       script de build (PowerShell/Windows)
└── build.sh        script de build (Bash/Linux)
```

## Compilar e executar

Resumo rapido (toolchain: MSYS2 UCRT64 -- gcc/g++/gfortran/nasm/cmake):

```powershell
.\build.ps1
.\dist\fwMSX.exe
.\dist\msxdisk.exe          # utilitario de disco standalone (CLI/shell/TUI/GUI)
```

Instrucoes completas, pre-requisitos e saida esperada em
[doc/MANUAL.md](doc/MANUAL.md).

## Versionamento

Formato `X.Y.Z`, sem tags numericas isoladas -- cada versao recebe o nome de
um jogo classico de MSX e um subtitulo curto indicando onde o projeto esta
naquele ponto (ex.: `v1.1.2 -- "Nemesis: Renomeacao"`). Regras completas em
[doc/SPEC.md](doc/SPEC.md#versionamento); detalhes de cada release em
[doc/RELEASE.md](doc/RELEASE.md); resumo das mudancas em
[doc/CHANGELOG.md](doc/CHANGELOG.md).

## Creditos

- **[Marat Fayzullin](https://fms.komkon.org/fMSX/)** -- autor original do
  fMSX, sobre o qual o fwMSX e construido (com aval do proprio autor para
  esta adaptacao/evolucao). O setor de boot real do MSX-DOS 1 usado pelo
  `msxdisk` (`src/msxdisk/core/msxdos1_boot.cpp`) e contribuicao direta
  dele (`resource/DiskUtilities/Boot.h`). O motor de despacho, tabelas e
  desmontador da CPU Z80 (`src/z80/core/`, `src/z80/debug/z80_disasm.*`)
  sao adaptados de `resource/fMSX/Z80/` (`Z80.c`, `Tables.h`, `Codes*.h`,
  `Debug.c`) -- ver `doc/z80-core-spec.md` e `LICENSE-THIRD-PARTY.md`. O
  motor de slots/subslots e os mappers MegaROM (`src/memmap/core/`) sao
  adaptados de `resource/fMSX/fMSX/MSX.c`/`MSX.h` -- ver
  `doc/memory-map-spec.md`. O motor "digital" do VDP e a renderizacao de
  SCREEN 0/1/2 (`src/vdp/core/`) sao adaptados de
  `resource/fMSX/fMSX/MSX.c` (registradores/portas/interrupcao) e
  `resource/fMSX/fMSX/Common.h` (`RefreshLine0/1/2`) -- ver
  `doc/vdp-spec.md`.
- **Arnold Metselaar** -- autor do utilitario original de manipulacao de
  discos MSX (`resource/DiskUtilities/DiskUtil.c` e correlatos), usado
  como referencia de estudo do formato FAT12/MSX-DOS para o `msxdisk`
  (nunca portado linha a linha -- ver `doc/msxdisk-spec.md`, secao 2).
- **msxDiskUtil** (`resource/msxDiskUtil/`) -- projeto irmao do mesmo
  autor do fwMSX, reescrita em PureBasic do utilitario acima; usado para
  confirmar, de forma independente, o setor de boot do MSX-DOS 1.
- Bibliotecas de terceiros usadas pelo `msxdisk` (baixadas via CMake
  `FetchContent` no build, nao redistribuidas neste repositorio):
  [CLI11](https://github.com/CLIUtils/CLI11),
  [replxx](https://github.com/AmokHuginnsson/replxx),
  [FTXUI](https://github.com/ArthurSonzogni/FTXUI),
  [Dear ImGui](https://github.com/ocornut/imgui),
  [GLFW](https://www.glfw.org/) e [SQLite](https://www.sqlite.org/).

## Licenca

O **codigo original do fwMSX e do msxdisk** (tudo escrito para este
projeto) e licenciado em **BSD 3-Clause** -- ver [LICENSE](LICENSE).

Isso **nao** cobre codigo adaptado/incorporado de terceiros. O fMSX tem
sua propria licenca, mais restritiva (proibe distribuicao comercial); o
fwMSX evolui a partir dele com o aval do proprio Fayzullin **para
adaptar/estudar seu codigo**, mas esse aval nao e uma autorizacao para
relicenciar o codigo dele sob BSD/GPL/MIT. Por isso, qualquer arquivo
deste repositorio que incorporar codigo do fMSX diretamente (o setor de
boot em `src/msxdisk/core/msxdos1_boot.cpp`; o motor/tabelas/desmontador
da CPU Z80 em `src/z80/core/` e `src/z80/debug/z80_disasm.*`; o motor de
slots/subslots e mappers MegaROM em `src/memmap/core/`; o motor do VDP e
a renderizacao em `src/vdp/core/`; mais arquivos devem se juntar a essa
lista quando PSG for adaptado do fMSX) continua sob a licenca original
dele, nao BSD. Ver
**[LICENSE-THIRD-PARTY.md](LICENSE-THIRD-PARTY.md)** para o
detalhamento completo (fMSX, DiskUtilities, msxDiskUtil) e
[resource/README.md](resource/README.md) para o aviso sobre o material
de terceiros usado so como referencia de estudo.
