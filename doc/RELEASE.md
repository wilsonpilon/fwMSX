# Releases

Cada versao do fwMSX recebe o nome de um jogo classico de MSX, seguido de
um subtitulo curto indicando em que ponto do projeto estamos -- nao se
usam tags puramente numericas. Versionamento no formato `X.Y.Z`:

- **X** (major): sobe quando um grupo de mudancas fecha uma base estavel.
- **Y** (minor): sobe a cada feature nova incorporada ao projeto.
- **Z** (patch): sobe a cada compilacao/build gerado.

Resumo curto de cada versao tambem em [CHANGELOG.md](CHANGELOG.md);
especificacao completa e historico de fases em [SPEC.md](SPEC.md).

---

## v1.4.1 -- "Salamander: Compilando em Linux" (2026-09-30)

**Fase:** validação de portabilidade, sem features novas -- primeira vez
que o projeto foi de fato compilado, linkado e executado (não só
compilado no papel) numa máquina Linux real. Nome escolhido por
"Salamander" ser um dos jogos mais emblemáticos de portar entre
plataformas diferentes (arcade, MSX e outros sistemas japoneses),
exatamente o que esta versão prova sobre o próprio projeto.

### O que motivou esta versão

A v1.4.0 já tinha `build.sh` (equivalente Linux do `build.ps1`), mas
nunca tinha sido rodado numa máquina Linux de verdade -- a branch
`elf64`/SysV do `.asm` dual-ABI do núcleo Z80 só tinha sido *montada*
(`nasm -f elf64`), nunca linkada/executada, por falta de ambiente Linux
no desenvolvimento original (só Windows). O autor rodou `build.sh` pela
primeira vez numa máquina Linux real (WSL2) e encontramos, juntos, três
problemas reais de portabilidade -- nenhum deles no código novo do
núcleo Z80/mapa de memória (que já nasceu multiplataforma), todos em
código **mais antigo**, nunca antes testado fora do Windows.

### Bugs corrigidos

1. **`src/asm/init_asm.asm`** (módulo Assembly da própria Fase 0 do
   projeto, de 2026-09-28) -- Win64-only. Montava sem erro para `elf64`
   (o NASM não valida convenção de chamada, só gera bytes), mas tinha
   dois problemas reais rodando de verdade: os argumentos chegariam nos
   registradores errados (SysV entrega major/minor/patch em
   `EDI/ESI/EDX`, não `ECX/EDX/R8D` da Win64), e o **link falhava**:
   ```
   relocation R_X86_64_PC32 against symbol `printf@@GLIBC_2.2.5' can
   not be used when making a PIE object; recompile with -fPIE
   ```
   porque uma chamada direta a `printf` é incompatível com executável
   PIE (posição-independente), o padrão em toda distro Linux moderna.
   Corrigido com a mesma técnica `%ifidn __OUTPUT_FORMAT__` já usada em
   `src/z80/asm/block_ops.asm`, mais `call printf wrt ..plt` (chamada
   relativa à PLT, que funciona com ou sem PIE).
2. **`src/msxdisk/asm/name_match.asm`** (msxdisk) -- mesmo problema de
   registrador errado, mas **mais perigoso** porque não chama nenhuma
   função externa: o build **não falhava**, só o resultado da
   comparação de nome de arquivo (`list`/`extract` com coringa, ex.
   `*.COM`) ficaria silenciosamente errado em tempo de execução no
   Linux. Corrigido com a mesma técnica de mapeamento de registrador de
   entrada por ABI.
3. **`src/msxdisk/gui/file_dialog.cpp`** incluía `<windows.h>` sem
   nenhuma guarda de plataforma -- quebrava a compilação inteira
   (`msxdisk` e `fwMSX`, que compilam os mesmos fontes) fora do
   Windows. Guardado atrás de `#ifdef _WIN32`; fora do Windows, os
   diálogos nativos de Novo/Abrir/Salvar Como devolvem "cancelado" por
   enquanto (a GUI continua funcionando normalmente, só sem seletor de
   arquivo nativo fora do Windows ainda). `comdlg32` (a biblioteca de
   diálogo do Windows, linkada sem condição no `CMakeLists.txt`) também
   corrigida para só entrar quando `WIN32` é verdadeiro.
4. `build.sh` compartilhava `build/` com `build.ps1` -- num checkout
   acessado tanto nativamente pelo Windows quanto via WSL (`/mnt/c/...`
   para o mesmo diretório), o CMake recusava reconfigurar
   (`CMakeCache.txt` "de outro diretório"). `build.sh` agora usa
   `build-linux/`, diretório próprio, resolvendo o conflito de raiz.

### Resultado

```
$ ./build.sh
[...]
==> Rodando testes (ctest)...
Test project /mnt/c/dos/fwMSX/build-linux
    Start 1: z80_smoke
1/3 Test #1: z80_smoke ........................   Passed
    Start 2: z80_debug_session
2/3 Test #2: z80_debug_session ................   Passed
    Start 3: memmap_slots
3/3 Test #3: memmap_slots .....................   Passed
100% tests passed, 0 tests failed out of 3
==> Empacotando fwMSX-1.4.1-linux.tar.gz...
==> Pronto:
    /mnt/c/dos/fwMSX/dist/fwMSX
    /mnt/c/dos/fwMSX/dist/msxdisk
    /mnt/c/dos/fwMSX/dist/fwMSX-1.4.1-linux.tar.gz
```

As mesmas **328 verificações automatizadas** do núcleo Z80 e do mapa de
memória, incluindo o teste diferencial de `LDIR`/`LDDR` que exercita a
aceleração em Assembly de verdade, passaram no Linux.

### Build usado para validar esta release

- **Windows**: `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64), `nasm`
  3.02, `cmake` 4.4.3 + `ninja` 1.13.2 -- `dist/fwMSX.exe` e
  `dist/msxdisk.exe` estáticos.
- **Linux (novo nesta versão)**: WSL2, toolchain de sistema
  (gcc/g++/gfortran/nasm/cmake/ninja via `apt`) -- `dist/fwMSX` e
  `dist/msxdisk`, dinamicamente ligados ao runtime padrão da distro
  (sem o link estático usado no Windows). Pacote:
  `dist/fwMSX-1.4.1-linux.tar.gz`.

### Limitações conhecidas
- Diálogos nativos de arquivo (Novo/Abrir/Salvar Como na GUI do
  msxdisk) só funcionam no Windows por enquanto -- um seletor nativo
  para Linux (ex. GTK) fica para uma tarefa à parte, se fizer sentido.
- Mesmas limitações de escopo já registradas na v1.4.0 (SCC/SRAM/
  `MAP_GMASTER2`/`MAP_FMPAC`/`MAP_GUESS` no mapa de memória,
  `CPIR`/`CPDR` sem aceleração em Assembly) -- ver `SPEC.md`, seção 5.0.
- Ainda não existe VDP, PSG nem uma máquina MSX completa -- trabalho no
  core de emulação continua pausado deliberadamente (ver `SPEC.md`,
  seção 5.0).

---

## v1.4.0 -- "Illusion City: Mapa de Memória" (2026-09-30)

**Fase:** mapa de memória MSX (slots/subslots/MegaROM), sobre o núcleo
Z80 da v1.3.0 -- ver [memory-map-spec.md](memory-map-spec.md) para a
especificação completa e o histórico de todas as 4 fases. **Trabalho no
core de emulação pausado deliberadamente a partir desta versão** (não
abandono) -- ver [SPEC.md, seção 5.0](SPEC.md) para os próximos passos
já registrados, pensados para retomar sem se perder.

Nome escolhido por ser um dos MegaROMs mais emblemáticos do MSX2+ --
uma aventura gigante (16 Mbit) que depende pesadamente de troca de
slot/subslot e bank-switch pra caber no espaço de 64KB do Z80,
exatamente o que este módulo constrói.

### Destaques
- **Motor de slots/subslots em C**, adaptado de
  `resource/fMSX/fMSX/MSX.c`/`MSX.h` -- topologia real de 4 slots
  primários x 4 secundários x 8 páginas de 8KB, incluindo o quirk de
  hardware onde o registrador de slot secundário é indexado pelo slot
  primário que ocupa a página `C000h-FFFFh`, não um registrador global
  único.
- **`MemorySystem`/`SlotMemoryBus`** (C++, design próprio) com uma API
  de depuração (`PeekSlot`/`PokeSlot`/`Describe`/`CurrentView`) que
  enxerga qualquer slot **independente** do que a CPU vê agora --
  requisito vital do autor, atendido desde a primeira fase deste
  módulo, não deixado para uma fase final como aconteceu com o
  depurador do núcleo Z80.
- **Carregamento de ROM real** com checksum **CRC32 em Fortran**
  (verificado contra o vetor de teste padrão de qualquer CRC32).
- **Seis mappers MegaROM** (`Gen8`/`Gen16`/`Konami5`/`Konami4`/
  `ASCII8`/`ASCII16`, só a troca de banco de ROM) -- cobrem a grande
  maioria dos cartuchos MegaROM reais do MSX.
- **`fwmsx --z80dbg --slots [rom]`**: depurador completo com mapa de
  memória real -- `slots`/`pages`/`slotmem`/`slotpeek`/`slotpoke`/
  `loadrom`, com carregamento automático de ROM de boot na abertura.
- **A BIOS MSX1 real do fMSX rodou de verdade** no núcleo Z80 deste
  projeto: 192 endereços de PC distintos visitados em 100 mil ciclos de
  execução real de código de BIOS -- a primeira prova concreta de
  compatibilidade com software MSX real, não só casos de teste
  escritos à mão.
- **`build.sh`**: equivalente Linux do `build.ps1`.
- 328 verificações automatizadas (`ctest -R "z80|memmap"`, até 168 +
  58 + 102), todas passando.

### Exemplo rápido (BIOS real rodando)

```
> .\dist\fwMSX.exe --z80dbg --slots resource\fMSX\ROMs\MSX.ROM
ROM de boot carregada em 0:0: resource/fMSX/ROMs/MSX.ROM
z80dbg> reset
z80dbg> run 5000
parado: orcamento de ciclos esgotado (ciclos consumidos: 5003, PC=0365)
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estáticos, dependências externas
  apenas as DLLs base do Windows/UCRT.
- `build.sh` (Linux) criado nesta versão, mas ainda **não validado em
  execução real** -- só sintaxe (`bash -n`) e a extração de versão via
  `sed`, sem toolchain Linux disponível nesta máquina de
  desenvolvimento. Fica para quando o autor compilar na própria máquina
  Linux.

### Limitações conhecidas
- Ainda não existe VDP, PSG nem uma máquina MSX completa -- ver
  `SPEC.md`, seção 5.0, para os próximos passos.
- SCC, SRAM persistente, `MAP_GMASTER2`/`MAP_FMPAC` e a heurística
  `MAP_GUESS` ficaram deliberadamente de fora do mapa de memória (ver
  `memory-map-spec.md`, seção 6) -- dependem de som/save-state, que
  ainda não existem.
- `CPIR`/`CPDR` continuam sem aceleração em Assembly (só `LDIR`/`LDDR`)
  -- limitação já registrada na v1.3.0.
- A branch `elf64`/Linux do `.asm` dual-ABI do núcleo Z80 continua só
  montada, nunca linkada/executada de verdade (mesma limitação da
  v1.3.0) -- `build.sh` existe agora especificamente para resolver isso
  quando houver uma máquina Linux disponível.

---

## v1.3.0 -- "SD Snatcher: Núcleo do Z80" (2026-09-30)

**Fase:** núcleo de emulação, primeiro pedaço real (CPU Z80) -- ver
[z80-core-spec.md](z80-core-spec.md) para a especificação completa e o
histórico de todas as fases (1 a 4). `fwMSX.exe` em si continua sem
VDP/PSG/mapa de memória de uma máquina MSX real; esta release entrega a
CPU isolada, exercitada sobre uma RAM plana de teste via o depurador
embutido.

### Destaques
- **Motor de despacho do Z80 em C** (`src/z80/core/`), adaptado de
  `resource/fMSX/Z80/` (registradores, tabelas de ciclo/flag, opcodes com
  e sem prefixo `CB`/`ED`/`DD`/`FD`/`DDCB`/`FDCB`), por trás de um
  `Z80Bus` próprio (callbacks + contexto, permitindo múltiplas instâncias
  de CPU no mesmo processo -- o fMSX original usa funções globais).
- **Wrapper de orquestração em C++** (`Z80Cpu`/`IBus`): `reset()`,
  `run(ciclos)`, `interrupt()`, acesso a registradores -- sem expor a
  união de par de registrador na API pública.
- **Tabelas de flag geradas em Fortran** (`ZSTable`/`PZSTable`, usando o
  intrínseco `POPCNT` para paridade), calculadas uma única vez no reset
  da CPU, nunca no caminho quente do despachante.
- **Aceleração de `LDIR`/`LDDR` em Assembly**: primeiro `.asm` do projeto
  com Win64 **e** SysV AMD64 (Linux) no mesmo arquivo-fonte (`%ifidn
  __OUTPUT_FORMAT__`), usada só quando o bloco inteiro cai em RAM plana
  do host -- o loop byte-a-byte original do fMSX continua como caminho
  de reserva sempre que isso não vale (correção nunca depende do caminho
  rápido, só o desempenho).
- **Depurador embutido em `fwMSX.exe`** -- `fwmsx --z80dbg`: REPL
  (replxx) com `reset`/`regs`/`step`/`run`/`break`/`clear`/`breaks`/
  `mem`/`peek`/`poke`/`load`/`fill`/`disasm`, rodando sobre uma RAM plana
  de 64KB de teste.
- **Desmontador Z80**, adaptado do desmontador já existente em
  `resource/fMSX/Z80/Debug.c` (tabelas de mnemônicos + algoritmo de
  substituição de gabarito) em vez de reescrito do zero -- com três
  correções cosméticas documentadas em relação ao original (nenhuma
  afeta execução/timing da CPU, só o texto exibido pelo desmontador).
- 203 verificações automatizadas novas (`ctest -R z80`): 168 no motor
  (`z80_smoke`) e 35 no depurador/desmontador (`z80_debug_session`),
  incluindo uma varredura de completude sem crash sobre 2044 combinações
  de opcode do desmontador e um fuzz diferencial de 150+ casos
  comparando o caminho rápido de `LDIR`/`LDDR` contra o loop lento.

### Saida de referencia do esqueleto (`fwMSX.exe` sem argumentos, inalterada)

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.3.0]
----------------------------------------
Loading module... CPP [v 1.3.0]
Loading module...C [v 1.3.0]
Loading module...Assembly [v 1.3.0]
Loading module Fortran [v 1.3.0]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Exemplo rápido do depurador (`fwmsx --z80dbg`)

```
z80dbg> poke 0x0000 0x21
z80dbg> poke 0x0001 0x34
z80dbg> poke 0x0002 0x12
z80dbg> poke 0x0003 0x76
z80dbg> disasm 0x0000 2
0000: 21 34 12     LD HL,1234h
0003: 76           HALT
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estáticos, dependências externas
  apenas as DLLs base do Windows/UCRT.
- A branch `elf64` do `.asm` dual-ABI foi verificada só até a montagem
  (`nasm -f elf64`, sem link/execução -- não há máquina Linux disponível
  nesta sessão de desenvolvimento); a branch `win64` foi validada
  completa (build + link + execução + testes).

### Limitações conhecidas
- Ainda não existe VDP, PSG nem mapa de memória de uma máquina MSX real
  -- o núcleo roda isolado sobre RAM de teste via `--z80dbg`.
- `CPIR`/`CPDR` não têm aceleração em Assembly (só `LDIR`/`LDDR`) --
  decisão deliberada: as flags dessas instruções dependem do byte
  comparado, não só do contador chegar a zero, tornando o corte de lote
  mais arriscado de acertar sob o mesmo padrão de verificação usado para
  `LDIR`/`LDDR`. Ver `z80-core-spec.md`, Fase 3.
- O caminho rápido em Assembly (`elf64`/Linux) não foi testado em
  execução real, só montagem -- ver acima.
- Mesmas limitações do `msxdisk` já registradas nas releases anteriores
  (`create --dos2` não validado num emulador real).

---

## v1.2.1 -- "Metal Gear: Ajustes de Campo" (2026-09-29)

**Fase:** msxdisk (polimento pós-lançamento). Correções encontradas
testando a GUI de verdade em janela gráfica, depois da v1.2.0.

### Destaques
- **Ejetar disco** na GUI (`Arquivo > Ejetar` / `F12`), com confirmação
  só quando há alterações não salvas.
- **Novo/Abrir/Salvar Como** na GUI viraram diálogo nativo de arquivo do
  Windows (navegação de pastas de verdade), em vez de caixa de texto.
- Dois bugs reais de interface corrigidos: diálogos modais que não
  abriam (escopo de ID do ImGui) e teclas de função que agiam no
  painel/item errado depois de marcar por checkbox (painel "ativo"
  desatualizado). Ambos encontrados e confirmados corrigidos em teste
  real pelo autor.

### Saida de referencia do esqueleto (`fwMSX.exe` sem argumentos, inalterada)

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.2.1]
----------------------------------------
Loading module... CPP [v 1.2.1]
Loading module...C [v 1.2.1]
Loading module...Assembly [v 1.2.1]
Loading module Fortran [v 1.2.1]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estaticos, dependencias externas
  apenas as DLLs base do Windows/UCRT.

### Limitacoes conhecidas
- Mesmas da v1.2.0 (`create --dos2` não validado num emulador real;
  nenhuma emulação de MSX ainda).

---

## v1.2.0 -- "Maze of Galious: Gerenciador de Discos" (2026-09-29)

**Fase:** msxdisk (5 fases completas -- ver
[msxdisk-spec.md](msxdisk-spec.md)). `fwMSX.exe` em si continua na Fase 0
do emulador (esqueleto multi-linguagem, sem Z80/VDP); o que essa release
entrega e o utilitario `msxdisk` (standalone e embutido no `fwMSX.exe`).

### Destaques
- `dist/msxdisk.exe`: um unico executavel para CLI one-shot, shell
  interativo (FTP-like), TUI (Norton Commander/XTree) e GUI (Dear ImGui,
  visual moderno proprio) -- manipulacao completa de imagens `.dsk`
  MSX-DOS 1/2 (criar, listar, adicionar, extrair, renomear, apagar,
  subdiretorios, copiar disco, salvar como).
- `fwmsx --msxdisk <argumentos>`: mesmos quatro modos acessiveis direto
  pelo `fwMSX.exe`, sem precisar do binario separado.
- Configuracao/temas/metadados de imagem em SQLite
  (`~/.msxdisk/config.sqlite3`), compartilhado entre TUI e GUI.
- Novas dependencias externas (buscadas via CMake `FetchContent` no
  build, nao redistribuidas em `resource/`): CLI11, replxx, FTXUI,
  Dear ImGui, GLFW, SQLite (amalgamation).

### Saida de referencia do esqueleto (`fwMSX.exe` sem argumentos, inalterada)

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.2.0]
----------------------------------------
Loading module... CPP [v 1.2.0]
Loading module...C [v 1.2.0]
Loading module...Assembly [v 1.2.0]
Loading module Fortran [v 1.2.0]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estaticos (`-static
  -static-libgcc -static-libstdc++ -static-libgfortran`), dependencias
  externas apenas as DLLs base do Windows/UCRT.

### Limitacoes conhecidas
- Nenhuma emulacao de MSX ainda -- `msxdisk` e um utilitario de
  ferramental, nao faz parte do core do emulador em si.
- `create --dos2` (MSX-DOS 2) nao foi validado num emulador/hardware real
  (o `--dos1` foi, no openMSX, MSX1 e MSX2) -- ver
  [msxdisk-spec.md](msxdisk-spec.md), secao 6.
- Varios fluxos interativos da TUI/GUI (dialogos, teclas de funcao,
  duplo-clique) so foram validados parcialmente por automacao (capturas
  de tela pontuais); dependem de confirmacao continua de uso real.

---

## v1.1.2 -- "Nemesis: Renomeacao" (2026-09-28)

**Fase:** 0 -- Esqueleto multi-linguagem (refinamento). Sem mudanca de
comportamento observavel; foco em legibilidade do codigo-fonte.

### Destaques
- Arquivos e funcoes de cada modulo renomeados de `module_<lang>.*` /
  `load_module_<lang>()` para `init_<lang>.*` / `init_<lang>()`, deixando
  o nome do arquivo e da funcao exportada identicos e mais claros sobre o
  papel de cada um (inicializacao do modulo daquela linguagem).
- `CMakeLists.txt`, `src/cpp/main.cpp` e a documentacao (`SPEC.md`,
  `MANUAL.md`, `README.md`) atualizados de acordo.
- Saida do programa **inalterada** em relacao a v1.1.1 (mesmas mensagens
  e assinaturas), apenas com a versao impressa em `[v 1.1.2]`.

### Assinaturas dos modulos (saida de referencia)

| Modulo   | Mensagem                                   | Assinatura |
|----------|---------------------------------------------|:----------:|
| C++      | `Loading module... CPP [v X.Y.Z]`            | `0x0001`   |
| C        | `Loading module...C [v X.Y.Z]`               | `0x0002`   |
| Assembly | `Loading module...Assembly [v X.Y.Z]`        | `0x0003`   |
| Fortran  | `Loading module Fortran [v X.Y.Z]`           | `0x0004`   |

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe`: ~3.1 MB (estatico), dependencias externas apenas
  `ntdll.dll`, `KERNEL32.DLL`, `KERNELBASE.dll` e `ucrtbase.dll`.

### Limitacoes conhecidas
- Nenhuma emulacao de MSX ainda -- apenas o esqueleto de build
  multi-linguagem que servira de base para as proximas fases.
- `resource/` ainda vazio (fontes de referencia a incluir em fases
  futuras).

---

## v1.1.1 -- "Knightmare: Alicerce" (2026-09-28)

**Fase:** 0 -- Esqueleto multi-linguagem. Primeira versao da nova
estrutura de diretorios; ainda nao ha emulacao de MSX.

### Destaques
- Projeto reorganizado em `src/` (um subdiretorio por linguagem), `doc/`,
  `dist/` e `resource/`.
- `main()` em C++ (`src/cpp/main.cpp`) recebendo nome do produto e versao
  via linha de comando, "carregando" um modulo por linguagem (C++, C,
  Assembly, Fortran), cada um retornando sua propria assinatura
  hexadecimal, com resumo final impresso pelo `main`.
- Build via CMake + Ninja sobre o toolchain MSYS2 UCRT64 (GCC/G++/
  GFortran 16.2.0 + NASM 3.02), linkado estaticamente -- `dist/fwMSX.exe`
  depende apenas do UCRT (nativo do Windows 10+).

### Assinaturas dos modulos (saida de referencia)

| Modulo   | Mensagem                                   | Assinatura |
|----------|---------------------------------------------|:----------:|
| C++      | `Loading module... CPP [v X.Y.Z]`            | `0x0001`   |
| C        | `Loading module...C [v X.Y.Z]`               | `0x0002`   |
| Assembly | `Loading module...Assembly [v X.Y.Z]`        | `0x0003`   |
| Fortran  | `Loading module Fortran [v X.Y.Z]`           | `0x0004`   |

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe`: ~3.1 MB (estatico), dependencias externas apenas
  `ntdll.dll`, `KERNEL32.DLL`, `KERNELBASE.dll` e `ucrtbase.dll`.

### Problemas encontrados e corrigidos nesta release
- Saida do modulo Fortran fora de ordem por buffer de E/S proprio do
  `libgfortran` -- ver [CHANGELOG.md](CHANGELOG.md) e
  [SPEC.md](SPEC.md#32-notas-de-implementacao-para-quem-retomar-o-projeto).
- Assinaturas hexadecimais mal formatadas por vazamento de
  `std::left`/`std::right` entre colunas do `std::cout`.

### Limitacoes conhecidas
- Nenhuma emulacao de MSX ainda -- apenas o esqueleto de build
  multi-linguagem que servira de base para as proximas fases.
- `resource/` ainda vazio (fontes de referencia a incluir em fases
  futuras).
