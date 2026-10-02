# fwMSX -- Especificacao (documento vivo)

> Este documento e atualizado a cada fase concluida ou funcionalidade
> incorporada ao projeto. Serve para posicionar o trabalho -- "onde
> paramos" -- ao retomar o projeto em outra maquina ou apos um tempo
> parado. Nao remova secoes de fases antigas: marque-as como concluidas e
> siga adicionando as novas por baixo.

## 1. Objetivo do projeto

Construir, a partir do [fMSX](https://fms.komkon.org/fMSX/) de Marat
Fayzullin (com aval do autor para a adaptacao), um emulador de MSX
reestruturado e modernizado, mantido como projeto de aprendizado.

**Licenciamento**: o aval do Fayzullin cobre adaptar/estudar o codigo
dele, nao relicencia-lo. Codigo novo do projeto e BSD-3-Clause
([LICENSE](../LICENSE)); qualquer arquivo que incorporar/adaptar codigo
do fMSX (Z80, VDP, PSG etc., conforme essas fases avancarem) continua sob
a licenca original dele -- ver [LICENSE-THIRD-PARTY.md](../LICENSE-THIRD-PARTY.md)
para o detalhamento e a lista de arquivos afetados (atualizar essa lista
sempre que um novo arquivo for adaptado do fMSX).

Regra obrigatoria de aprendizado (vale para todas as fases futuras, nao so
para o esqueleto inicial): **o projeto precisa ter partes reais e uteis em
C, C++, Assembly e Fortran**, ainda que minimas, para forcar contato
pratico com interoperabilidade entre linguagens (ABI, calling convention,
name mangling, linkedicao estatica/dinamica).

## 2. Estrutura de diretorios

Todos os diretorios de primeiro nivel em minusculas:

| Diretorio   | Conteudo |
|-------------|----------|
| `src/`      | Codigo-fonte do projeto: `src/cpp/`, `src/c/`, `src/asm/`, `src/fortran/` e `src/common/` (esqueleto multi-linguagem da Fase 0); `src/msxdisk/` (utilitario de disco, ver `doc/msxdisk-spec.md`); `src/z80/` (nucleo da CPU Z80 -- `core/cpp/asm/fortran/common/debug`, ver `doc/z80-core-spec.md`). |
| `tools/`    | Pontos de entrada de executaveis standalone (hoje: `tools/msxdisk/main.cpp`). |
| `tests/`    | Testes automatizados (CTest) fora do escopo de `src/` -- hoje `tests/z80/` (`z80test`, `z80dbgtest`). |
| `doc/`      | Documentacao: `SPEC.md` (este arquivo), `msxdisk-spec.md`, `z80-core-spec.md`, `MANUAL.md`, `CHANGELOG.md`, `RELEASE.md`. |
| `dist/`     | Pacote pronto para execucao: `fwMSX.exe`, `msxdisk.exe` (gerados pelo build) e `fwMSX-X.Y.Z.zip` (tudo o que e necessario para rodar em outra maquina). |
| `resource/` | Codigos-fonte de terceiros (fMSX original, outros emuladores/ferramentas de MSX) usados como referencia de estudo/adaptacao -- ver `resource/README.md` (aviso: nao faz parte do projeto compilado, so estudo/referencia). |

Arquivos na raiz: `README.md`, `LICENSE`, `LICENSE-THIRD-PARTY.md`,
`.gitignore`, `CMakeLists.txt` (build raiz) e `build.ps1` (script de
build para PowerShell).

## 3. Fase 0 -- Esqueleto multi-linguagem (CONCLUIDA em v1.1.1, refinada em v1.1.2)

Objetivo desta fase: reestruturar o prototipo original ("learnmix", um
hello-world hibrido solto na raiz do repositorio) na arvore de diretorios
acima, **sem ainda comecar a emulacao do MSX**, e trocar o exemplo trivial
por algo que exercite de forma mais deliberada as quatro linguagens e
sirva de base de build para as proximas fases.

### 3.1 Especificacao funcional

- Ponto de entrada do projeto: `main()` em **C++**
  (`src/cpp/main.cpp`).
- `main()` recebe via linha de comando (parametros de `main`):
  - uma **string**: o nome do produto (`argv[1]`; default `"fwMSX"`
    vindo de `src/common/version.h` se omitido);
  - **tres inteiros**: major, minor e patch da versao corrente
    (`argv[2..4]`; default os valores de `version.h` se omitidos).
- `main()` imprime, nesta ordem:
  1. `Copyright (c) 1972-2026 Cybernostra, Inc.`
  2. `<nome> [v X.Y.Z]`
  3. Chama, em sequencia, o "carregamento" de um modulo por linguagem.
     Cada modulo recebe major/minor/patch e imprime sua propria linha,
     com a versao impressa entre colchetes (`[v X.Y.Z]`):
     - C++: `Loading module... CPP [v X.Y.Z]`
     - C: `Loading module...C [v X.Y.Z]`
     - Assembly: `Loading module...Assembly [v X.Y.Z]`
     - Fortran: `Loading module Fortran [v X.Y.Z]`
  4. Cada modulo devolve uma **assinatura hexadecimal** propria:
     - C++ -> `0x0001`
     - C -> `0x0002`
     - Assembly -> `0x0003`
     - Fortran -> `0x0004`
  5. `main()` imprime, como finalizacao, o resumo com as quatro
     assinaturas recebidas de volta dos modulos.

### 3.2 Notas de implementacao (para quem retomar o projeto)

- **Convencao de nomes** (desde v1.1.2): cada modulo mora em
  `src/<linguagem>/init_<linguagem>.*` (`init_cpp`, `init_c`, `init_asm`,
  `init_fortran`) e a funcao exportada por cada um tem o mesmo nome do
  arquivo (`init_cpp()`, `init_c()`, `init_asm()`, `init_fortran()`).
  Version inicial (v1.1.1) usava `module_<linguagem>.*` /
  `load_module_<linguagem>()`; renomeado para deixar claro que cada
  funcao e o ponto de entrada ("inicializacao") daquele modulo,
  preparando o terreno para quando cada uma passar a fazer trabalho real
  de emulacao em vez de so imprimir uma linha.
- **Assembly** (`src/asm/init_asm.asm`, NASM, ABI Win64): monta na mao
  uma chamada a `printf()` da C runtime, reordenando os argumentos para a
  convencao de retorno variadica (`RCX`=formato, `RDX/R8/R9`=valores),
  reservando os 32 bytes de shadow space e mantendo a pilha alinhada em
  16 bytes no `call`. Devolve a assinatura em `EAX`.
- **Fortran** (`src/fortran/init_fortran.f90`): funcao exposta ao C/C++
  via `iso_c_binding`/`bind(c)`. Formata a mensagem com `WRITE` numa
  string interna (E/S genuina do Fortran) e imprime com `PRINT`.
  **Importante**: o runtime do Fortran (`libgfortran`) usa um buffer de
  E/S proprio, independente do buffer do C/C++. Foi necessario um
  `FLUSH(output_unit)` explicito logo apos o `PRINT`, senao a linha so
  saia no encerramento do processo, fora de ordem em relacao ao que o
  C++ ja tinha impresso. Fique atento a esse tipo de problema sempre que
  novas linguagens/runtimes forem combinados no mesmo binario.
- **C++**: cuidado com estado de formatacao "vazando" entre chamadas ao
  `std::cout` (ex.: um `std::left` de uma coluna anterior afetando o
  `std::setw` de uma coluna seguinte, que precisa de `std::right`). Foi
  o primeiro bug encontrado no build desta fase.
- O executavel e linkado com `-static -static-libgcc -static-libstdc++
  -static-libgfortran` para depender apenas do UCRT (nativo do Windows
  10+), sem exigir DLLs do MSYS2/MinGW na maquina de destino.

### 3.3 Toolchain usado nesta fase

MSYS2, grupo de pacotes **UCRT64** (todos na mesma arvore, mesma versao
16.2.0 do GCC/GFortran, evitando misturar toolchains diferentes que
convivem na mesma maquina de desenvolvimento):

- `gcc`, `g++`, `gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2

Ver [doc/MANUAL.md](MANUAL.md) para instrucoes completas.

## 4. Versionamento

Formato `X.Y.Z`, comecando em `1.1.1`. **Nao se usam tags numericas
isoladas**: cada versao recebe o nome de um jogo classico de MSX seguido
de um subtitulo curto indicando o estado do projeto naquele ponto (ex.:
`v1.1.1 -- "Knightmare: Alicerce"`). Detalhes de cada release em
[RELEASE.md](RELEASE.md).

Regra de incremento (mantida pelo assistente/mantenedor, sem exigir
acompanhamento manual do autor a cada build):

- **Z** (patch) sobe a cada compilacao/build gerado.
- **Y** (minor) sobe a cada feature nova inserida/incorporada ao projeto.
- **X** (major) sobe a cada grupo de mudancas que fecha uma base estavel
  (ex.: "esqueleto multi-linguagem estavel", "core do Z80 estavel",
  "video/VDP estavel" etc.).

`src/common/version.h` e a fonte de verdade dos valores default
(usados quando o executavel roda sem argumentos).

## 5. Proximas fases (ainda nao iniciadas)

- ~~Trazer para `resource/` os fontes do fMSX original e de outras
  referencias de MSX para estudo.~~ Feito -- `resource/` ja tem fMSX,
  fmsxgo, kizuna, msxide, paleobasic, msxDiskUtil, msxdos1/2 etc. (ver
  `resource/README.md`). O `msxdisk` (ver `doc/msxdisk-spec.md`) e o
  primeiro fruto direto disso.
- Iniciar o core de emulacao propriamente dito (CPU Z80, VDP, PSG, etc.),
  decidindo em qual(is) modulo(s)/linguagem(ns) cada parte sera
  implementada, sempre respeitando a regra de ter as quatro linguagens
  representadas em uso real.

  **CPU Z80: Fases 1-4 concluidas em 2026-09-30** -- ver
  [doc/z80-core-spec.md](z80-core-spec.md) para o detalhamento completo.
  Resumo: motor de despacho em **C** (adaptado do fMSX) + wrapper de
  orquestracao em **C++**, tabelas de flag geradas em **Fortran**,
  aceleracao de `LDIR`/`LDDR` em **Assembly** dual-ABI (Win64/SysV,
  primeiro `.asm` do projeto portavel pra Linux), e um depurador
  embutido em `fwMSX.exe` (`fwmsx --z80dbg`: registradores, memoria,
  breakpoints e desmontador).

  **Mapa de memoria MSX (slots/subslots): Fases 1-4 concluidas em
  2026-09-30, modulo pausado aqui por ora** -- ver
  [doc/memory-map-spec.md](memory-map-spec.md) para o detalhamento
  completo. Resumo: motor de slots/subslots em **C** fiel ao fMSX
  (`MemMap`/`PSL`/`SSL`/`SSLReg`, incluindo o quirk real de hardware do
  registrador de slot secundario), `MemorySystem`/`SlotMemoryBus` em
  **C++** com API de inspecao que enxerga qualquer slot independente do
  que a CPU ve agora (requisito vital do autor, atendido desde a Fase 1
  deste modulo), carregamento de ROM real com checksum **CRC32** em
  **Fortran**, seis mappers MegaROM de troca de banco (Gen8/Gen16/
  Konami5/Konami4/ASCII8/ASCII16, so' a parte de ROM) adaptados de
  `MapROM()` do fMSX, e `fwmsx --z80dbg --slots [rom]` como comando
  completo de depuracao com mapa de memoria real -- **validado contra a
  BIOS MSX1 real do fMSX** (192 enderecos de PC distintos visitados em
  100 mil ciclos de execucao real de codigo de BIOS).

  **Ainda faltam VDP e PSG para existir uma maquina MSX completa** --
  ver a nota de "proximos passos" logo abaixo. SCC, SRAM persistente,
  `MAP_GMASTER2`/`MAP_FMPAC` e a heuristica `MAP_GUESS` do mapa de
  memoria ficaram deliberadamente de fora por dependerem de subsistemas
  que ainda nao existem (som, save-state) -- ver
  `doc/memory-map-spec.md`, secao 6, para o raciocinio completo de cada
  item adiado.

### 5.0 Proximos passos (para retomar sem se perder -- v1.9.0, 2026-10-02)

O trabalho no core de emulacao segue em andamento; esta secao continua
sendo o "onde paramos" oficial.

1. **VDP** (TMS9918/V9938) -- o que da tela de verdade pela primeira vez.
   **Fases 0.5, 1 e 2 concluidas em 2026-09-30** -- ver
   [doc/vdp-spec.md](vdp-spec.md). Resumo: `CompositeBus` (multiplexa
   I/O de porta entre mapa de memoria e VDP), motor "digital" do VDP em
   **C** (registradores, protocolo de porta `98h`-`9Bh`, maquina de
   estados de scanline/interrupcao -- **finalmente da uso real** ao
   `Z80Cpu::interrupt()`, confirmado com um programa sintetico recebendo
   a interrupcao de VBlank em `0x0038` de verdade), renderizacao real de
   pixel para SCREEN 0/1/2 (texto mono, texto colorido, bitmap), tabela
   de paleta de 512 cores em **Fortran** (finalmente com consumidor),
   exportacao de frame como PPM (`vdpshot`) para inspecao/teste sem
   janela. **Fase 3 (sprites de modo 1, SCREEN 1/2/3) concluida em 2026-10-01**. **Falta**: modos MSX2 5-8 (com sprites de modo 2) + janela
   de verdade (Fase 4), motor de comando V9938 (Fase 5, avancado/
   opcional). Achado que vale lembrar: a BIOS real ainda nao chega a
   habilitar a interrupcao de VBlank dentro de nenhum orcamento de
   ciclos testado -- ela poliniza hardware de teclado/PPI (portas
   `A9h`-`ABh`) que ainda nao existe no projeto, uma limitacao real e
   separada (nao um bug do VDP), documentada em `doc/vdp-spec.md`.
2. **PSG** (AY-3-8910) -- som. **Fase 1 concluida em 2026-10-02**
   (v1.9.0) -- ver [doc/psg-spec.md](psg-spec.md). Motor em
   **C** que gera amostras PCM de verdade (tom, ruido, envelope, avancado
   por ciclos de Z80), tabela de volume em **Fortran**, `PsgDevice` +
   gravador de WAV em **C++**; flag `--psg` e comandos `psgregs`/`psgpoke`/
   `psgrec` no depurador. A BIOS real programa o PSG e `BEEP` no BASIC sai
   como onda de 1316 Hz (teste `psgtest`). **Audio ao vivo concluido em
   2026-10-02** ([doc/audio-spec.md](audio-spec.md)). **Falta**: joystick
   em R14/R15. O SCC do
   mapa de memoria (adiado na Fase 3, ver `doc/memory-map-spec.md`) agora
   tem um destino de audio possivel.
   **Janela com teclado do host: concluida em 2026-10-02** (item 1 abaixo,
   "janela de verdade") -- `fwmsx --msx`, ver
   [doc/machine-spec.md](machine-spec.md). Roda a BIOS real em tempo real,
   com teclado do host e cartucho no slot 1. Audio ao vivo tambem pronto.
   Continua faltando joystick, disco e os modos MSX2.
3. Itens menores registrados e conscientemente adiados, sem bloquear
   nada do acima:
   - ~~Validacao em execucao real (nao so montagem) da branch `elf64`/
     Linux do `.asm` dual-ABI.~~ **Feito na v1.4.1** (2026-09-30) --
     `build.sh` rodado de verdade numa maquina Linux (WSL2) pelo autor:
     build completo + `ctest` com as 328 verificacoes passando. Duas
     correcoes de portabilidade reais foram necessarias em modulos
     Assembly antigos (`src/asm/init_asm.asm`, `src/msxdisk/asm/
     name_match.asm`, ambos Win64-only desde antes do nucleo Z80
     existir) -- ver `doc/RELEASE.md`, v1.4.1, e `doc/z80-core-spec.md`,
     secao 6 (Fase 3), para o relato completo.
   - `CPIR`/`CPDR` em Assembly (nucleo Z80, Fase 3) -- so `LDIR`/`LDDR`
     foram acelerados; as flags de `CPIR`/`CPDR` dependem do byte
     comparado, tornando o corte de lote mais arriscado.
   - `MAP_GMASTER2`/`MAP_FMPAC`/SRAM persistente/`MAP_GUESS` no mapa de
     memoria (ver `doc/memory-map-spec.md`, secao 6) -- dependem de
     som/save-state, que ainda nao existem.
   - Decidir, quando VDP/PSG existirem, se `fwMSX.exe` sem argumentos
     passa a abrir em modo GUI/maquina completa por padrao (visao
     registrada na secao 5.1 abaixo, ainda nao implementada).
   - ~~PPI/teclado (portas `A8h`-`ABh`)~~ **Feito na v1.7.0**
     (2026-10-01) -- ver `doc/ppi-spec.md`. O achado da Fase 1 do VDP
     ("a BIOS fica presa esperando o PPI") estava *incompleto*: faltavam
     tambem RAM no slot `3:2` e as regras de subslot do MSX1 (ambas
     agora ligadas por `--ppi` quando ha' BIOS). **Causa real (achada na
     v1.8.0): `Z80Cpu` nao chamava `z80_reset()` na construcao e as
     tabelas de flag do Z80 ficavam zeradas** -- corrigido. **A BIOS MSX1
     real agora sobe ate o prompt do MSX BASIC** (`MSX BASIC version 1.0
     ... Ok`), renderizado pelo VDP, e `keydown`/`keyup` digitam nele
     (`doc/ppi-spec.md`, secao 5).

**Para retomar rapido**: leia esta secao, depois `doc/z80-core-spec.md`
e `doc/memory-map-spec.md` (ambos documentos vivos, com todas as fases
e decisoes registradas). O `--z80dbg --slots resource/fMSX/ROMs/MSX.ROM`
e' o jeito mais rapido de ver o que ja funciona de verdade hoje --
adicione `--ppi` pra teclado/PPI (`ppiregs`/`keys`/`keydown`) e `--vdp` pra ver tambem os comandos de VDP (`vdpregs`/
`vdpshot`/etc.).
- Definir empacotamento final (alem do ZIP de `dist/`) quando houver uma
  versao executavel do emulador.

### 5.1 Visao registrada: `fwMSX.exe` como ponto de entrada unico do projeto

**Decidido com o autor em 2026-09-29** (ver tambem `doc/msxdisk-spec.md`,
Fase 5c):

- `fwMSX.exe` deve, no futuro, funcionar como `msxdisk.exe` funciona hoje
  para o `msxdisk`: um unico binario com varios modos de entrada --
  linha de comando/REPL proprio, TUI e GUI -- para escolher
  maquina/extensao/memoria e outras configuracoes do emulador antes de
  rodar. O emulador propriamente dito (a tela do MSX rodando) e sempre
  grafico; REPL/TUI servem para configurar/pilotar em volta disso.
- Sem argumentos, uma vez que exista emulacao de verdade, `fwMSX.exe`
  deve abrir em modo **GUI** por padrao. Ate la (Fase 0 atual, sem Z80/VDP
  rodando), continua imprimindo o resumo dos quatro modulos como hoje --
  **nao faz sentido abrir uma tela de emulador que ainda nao emula nada**,
  entao essa mudanca de comportamento padrao só acontece quando o core de
  emulacao existir.
- **Implementado em 2026-09-29 (v1.2.0)**: `fwMSX.exe` chama o `msxdisk`
  embutido via `fwmsx --msxdisk <resto dos argumentos>`, repassando para
  `msxdisk::RunEntryPoint()` (`src/msxdisk/entry.{h,cpp}`) -- a mesma
  funcao de roteamento usada pelo `msxdisk.exe` standalone
  (`tools/msxdisk/main.cpp`), cobrindo CLI one-shot, shell, TUI e GUI.
  Foi uma integracao barata como previsto: os fontes do msxdisk
  (`MSXDISK_LIB_SOURCES` no `CMakeLists.txt`) sao compilados tanto no
  alvo `msxdisk` quanto no alvo `fwMSX`, sem lib intermediaria (mesma
  logica ja usada entre `src`/`asm`/`fortran`). Testado: `fwMSX.exe` sem
  argumentos continua com a saida do esqueleto **inalterada**;
  `fwmsx --msxdisk list/info/--tui/--gui` funcionam identico ao
  `msxdisk.exe` direto.
- **`msxdisk.exe` continua existindo como binario standalone**, para quem
  quer soh o utilitario de disco sem instalar/rodar o emulador --
  requisito original do proprio autor, nao se perde com essa integracao.
- **Implementado em 2026-09-30**: `fwmsx --z80dbg` abre um REPL de
  depuracao do `Z80Cpu` (registradores, memoria, breakpoints,
  desmontador) sobre uma RAM plana de 64KB -- ver `doc/z80-core-spec.md`,
  Fase 4, para o detalhamento completo. E' o primeiro modo de `fwMSX.exe`
  que expoe o proprio core de emulacao (nao o `msxdisk` embutido), e o
  primeiro passo concreto na direcao desta visao: quando VDP/PSG/mapa de
  memoria existirem, o mesmo padrao de "REPL/TUI/GUI escolhendo um unico
  executavel" se estende para pilotar a maquina completa, nao so a CPU
  isolada.
- Esta visao **nao muda nada da Fase 0 atual** nem bloqueia o trabalho em
  andamento no `msxdisk` (fases 1-5 dele seguem seu proprio ritmo,
  documentadas em `doc/msxdisk-spec.md`). So entra em jogo quando o core
  de emulacao (bullet acima) comecar a existir de verdade.

*(Esta secao sera detalhada/movida para itens concluidos conforme o
projeto avancar.)*
