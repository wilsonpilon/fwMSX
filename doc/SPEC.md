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

## 5. Proximas fases

- ~~Trazer para `resource/` os fontes do fMSX original e de outras
  referencias de MSX para estudo.~~ Feito -- `resource/` ja tem fMSX,
  fmsxgo, kizuna, msxide, paleobasic, msxDiskUtil, msxdos1/2 etc. (ver
  `resource/README.md`). O `msxdisk` (ver `doc/msxdisk-spec.md`) e o
  primeiro fruto direto disso.
- ~~Iniciar o core de emulacao propriamente dito (CPU Z80, VDP, PSG, etc.),
  decidindo em qual(is) modulo(s)/linguagem(ns) cada parte sera
  implementada, sempre respeitando a regra de ter as quatro linguagens
  representadas em uso real.~~ **Feito**: o core existe desde a v1.3.0 (Z80)
  e a maquina completa desde a v1.9.0 (MSX1) e v1.11.0 (MSX2). Cada fase
  segue a regra das quatro linguagens.

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

  **Estado atual (2026-10-06, v1.17.0):** a maquina MSX1, MSX2 e MSX2+ estao completas no nucleo
  (VDP, PSG, SCC, FM com BASIC, disco, joystick, SRAM e layout de slots). Ver a secao 5.0 para
  o que funciona e o que falta. O mapa de memoria ja tem o FM-PAC e a SRAM; falta o GameMaster2
  (`doc/memory-map-spec.md`, secao 6).

### 5.0 Estado atual e proximos passos (atualizado em 2026-10-06, depois da v1.17.0)

Esta secao e' o "onde paramos" oficial. O historico de cada versao esta em
[CHANGELOG.md](CHANGELOG.md) e [RELEASE.md](RELEASE.md).

**Funcionando (v1.17.0, publicada):**

- [x] MSX1, MSX2 e MSX2+: BIOS real ate o prompt do MSX BASIC (1.0, 2.1 e 3.0); MSX-DOS 1.8 do disco ate `A>`.
- [x] Z80 completo; mapa de slots e subslots; mappers Konami, ASCII, Gen8 e Gen16; SRAM ASCII8/ASCII16 e FM-PAC (`.sav`).
- [x] VDP completo: TMS9918 (SCREEN 0-3), V9938 (SCREEN 4-8, comandos, sprites) e V9958 (SCREEN 10-12, YJK/YAE, scroll).
- [x] PSG, SCC e FM (OPLL): 9 canais melodicos, 15 timbres, modo ritmo; comandos de BASIC do MSX-MUSIC pelo FM-PAC.
- [x] Janela com os menus do fMSX, zoom, proporcao, tela cheia e filtros de video.
- [x] Configuracao de slots pelo menu: 16 celulas, BIOS em 0:0, RAM 16/32/64 KB, mapper ate 1024 KB, disco, sub-ROM, cartucho e FM-PAC.
- [x] Pacotes Windows (zip) e Linux (tar.gz); `ctest` com 14 suites.

**Feito depois da v1.17.0 (nao lancado, sem commit):**

- [x] Banco de ROMs (SQLite, `fwmsx --romdb`): downloads do fMSX 6.0, do System ROMs do file-hunter (com navegacao) e do Vampier; CRUD, busca, identificacao, CARTS.SHA. Menu ROMs na janela. Ver `doc/romdb-spec.md`.
- [x] Codigo do openMSX em `resource/openMSX/` (GPL), provisorio, para estudo. Branch `estudo/openmsx`.
- [x] RAM de 16KB ocupa C000h-FFFFh (a BIOS Expert travava com a RAM em 0000h da celula).
- [x] RAM de 32KB ocupa duas celulas (pagina 2 + pagina 3 da seguinte); mapper de 64KB a 4096KB, varios mappers, segmento k na pagina k.
- [x] Controladora de disco por portas no estilo Microsol: DDX 3.0 e CDX-2 com o MSX-DOS 1.8 subindo pelas portas D0h. Formatos 180/360/720 KB pela configuracao do drive. Ver `doc/fdc-spec.md`, secao 6.
- [x] Opcao `--slot P:S=tipo[:arg]` para montar o layout pela linha de comando; `--disk-acesso`, `--disk-porta`, `--disk-formato`, `--text`, `--fmstat`, `--wav`.
- [x] Correcoes de teclado: `(`, `)`, `*`, `"` e `&` no layout do MSX.

**Politica de midias (2026-10-06):** o repositorio e' pessoal; ROMs, discos e fitas de terceiros podem ser versionadas. Antes da liberacao publica, revisar cada midia e remover as que o detentor contestar (`LICENSE-THIRD-PARTY.md`).

**Projeto futuro:** o fwMSX sera integrado ao msxide num utilitario de desenvolvimento MSX chamado **MSX-PoorManOS**. Ate la, sem preocupacao com redistribuicao.

**Pendencias (ordem sugerida):**

- [ ] Validar na tela o que so' foi compilado: menu ROMs, janelas Banco de ROMs e Navegar file-hunter, Configuracao de disco e de slots, tela cheia, 4:3, 16:9 e filtros.
- [ ] Banco de ROMs: verificar as ROMs baixadas contra o SHA-1 conhecido; usar o banco para escolher o mapper ao carregar cartucho; importar o JSON do Vampier se for util.
- [ ] Ouvir o FM, o SCC e o disco contra referencia; ajustar as constantes do OPLL (`doc/fm-spec.md`, secao 2) e os ganhos da mistura.
- [ ] Jogos: Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada, causa nao diagnosticada; Mega Chase validado so' ate o titulo; F-1 Spirit 3D: troca de disco pela janela nao testada.
- [ ] FM: `CALL VOICECOPY` (a ROM do fMSX nao aceita); status e timers do OPLL.
- [ ] Disco: formatar disquetes (hoje so' le e grava); FM/MFM modelados; drives A e B com formatos diferentes; mais de um driver de porta (Sony, Philips, Spectravideo do openMSX) se quiser estudar.
- [ ] Layout de slots: salvar e carregar o layout em arquivo; perfis de maquina salvos no banco.
- [ ] Controle externo do emulador, no estilo openMSX (ideia de hoje): canal de controle opt-in em localhost (TCP ou pipe), comandos `status`, `reset`, `pause`/`resume`, `type`, `cart`, `disk`, `screenshot`, `peek`/`poke`, `quit`; eventos de troca de modelo e de midia. A thread de controle so' enfileira comandos; quem toca na maquina e' o laco de quadros. Ainda nao comecou.
- [ ] Cartuchos: MSX-DOS 2, GameMaster2 e o MSX-MUSIC com BIOS propria.
- [ ] BIOS de outras maquinas (ex.: Gradiente Expert 1.1): a BIOS sobe, mas a tela sai com espacos entre as letras; investigar.
- [ ] Save-state completo, incluindo o estado de PSG, SCC, OPLL, disco e VDP.
- [ ] Efeitos de rastreio no meio do quadro (troca de palheta e de scroll por linha).
- [ ] Cores YJK do V9958 conferidas com hardware real.
- [ ] Desempenho: medir o custo de CPU no pior caso (FM ativo, SCC, mapa de slots, disco) em tempo real.
- [ ] Licenca: confirmar por escrito a autorizacao de uso do fMSX antes de mudar o texto de README e LICENSE-THIRD-PARTY. O openMSX (GPL) segue so' como referencia; decidir se sai do repositorio quando o fwMSX estiver pronto.
- [ ] Frontend para jogar (biblioteca de jogos e imagens) sobre o banco de ROMs: depois.
- [x] Commit e push do trabalho de 2026-10-06 (branch `estudo/openmsx`, que contem o openMSX em `resource/`).
- [ ] Decidir: merge de `estudo/openmsx` no `main` (fast-forward), e tag/release da proxima versao (1.18.0) quando o banco de ROMs e o disco por portas estiverem validados na tela.

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
  **Implementado na v1.12.0** (2026-10-05): o core de emulacao (Z80, mapa
  de memoria, VDP, PSG, PPI/teclado, disco) ja existe de sobra, entao
  `fwMSX.exe` sem argumento nenhum agora abre a maquina completa em
  janela (`main()` chama `machine::RunMachineCommand({}, argv[0])` antes
  de qualquer outra checagem de modo) -- mudanca de uma linha de
  comportamento, nao de codigo novo, ja que `--msx` ja cobria tudo.
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
### 5.2 Feature a desenvolver em breve: fitas (TSX, TZX e CAS) e banco de fitas

Estado: **planejado, nao iniciado** (2026-10-06). Pedido do usuario: ler e criar TSX, ler e criar CAS,
um banco de fitas com download pelo site oficial, e uma ferramenta de linha de comando e pelo menu para
manipular esses arquivos. Ver tambem `doc/SPEC.md`, secao 5.0, e a viabilidade abaixo.

**Formatos.**

- **TSX** e' um superconjunto do **TZX 1.20** (fita do ZX Spectrum), com o bloco **ID 0x4B** (Kansas City
  Standard, usado pelo MSX). Especificacao em `resource/makeTSX/docs/TZX_format.md` e no wiki do makeTSX.
  Blocos do TZX que aparecem em fitas MSX: 10, 11, 12, 13, 14, 15, 19, 20, 21-28 (grupos, saltos, laços),
  2A, 2B, 30, 31, 32, 33, 35, 4B e 5A. Os blocos 16, 17, 34 e 40 (C64, emulacao, snapshot) estao
  descontinuados pela propria especificacao.
- **Bloco 0x4B (KCS)**: 4 bytes de tamanho, pausa (ms), duracao do pulso de pilot, numero de pulsos do pilot,
  duracao do pulso de ZERO e de UM (T-states), a configuracao de bits (pulsos por bit) e de bytes (bits de
  inicio/fim, valor, ordem), e os dados. Os valores padrao usados pelo makeTSX sao ZERO = 855 e UM = 1710
  T-states (base de 3,5 MHz); conferir no codigo antes de usar.
- **CAS**: imagem binaria de fita do MSX (dados em blocos com cabecalho de arquivo: tipo BASIC, ASCII ou binario,
  nome, endereco, tamanho). Nao tem pulsos; e' uma forma compacta da mesma informacao.

**Viabilidade (por parte).**

1. **Leitor de TZX/TSX (blocos listados acima): viavel, baixo a medio.** Referencias: `makeTSX` (MIT) e o
   `TsxParser` do openMSX-TSX (GPL, so' estudo) e `CLK/Storage/Tape/Formats/TZX.cpp` (MIT). O CLK ja' le
   o bloco 0x4B. Estimativa: 800 a 1200 linhas em C++ mais testes.
2. **Escritor de TSX a partir de arquivos (.BIN, .BAS, ASCII): viavel, medio.** Gera blocos 0x4B com os
   cabecalhos de arquivo do MSX (`BLOAD`, `CLOAD`, `RUN"CAS:"`). Primeiro passo util, sem analisar WAV.
3. **Criar TSX a partir de WAV (o que o makeTSX faz): viavel, alto.** E' um port do makeTSX (MIT), com os
   "rippers" (blocos 10 a 20, 4B). Fase posterior.
4. **Emulacao de sinal de fita (pulsos no PPI): viavel, alto; e' o que torna TSX com protecao e loaders
   proprios funcionais.** O PPI do fwMSX ainda nao modela a porta de cassete (porta C, `AAh`: bit 4 = motor,
   bit 5 = saida; a entrada do cassete ainda sem definicao neste projeto -- validar no fMSX e no openMSX antes).
   A reproducao gera a sequencia de pulsos a partir dos blocos e entrega o nivel de entrada na cadencia
   de T-states da CPU, como o PSG faz com as amostras. Risco: temporizacao exata; os loaders de protecao
   dependem de ciclos.
5. **CAS por hooks da BIOS (como o fMSX faz): viavel, baixo.** O carregamento de `BLOAD "CAS:"` e `CLOAD`
   pode ser interceptado no nivel da BIOS, sem sinal. Nao serve para fitas com protecao, mas resolve o caso
   comum de programas em CAS. Dar prioridade a esta parte antes do sinal.
6. **Banco de fitas com metadados (SQLite, nome TOSEC, titulo, empresa, ano, hash, arquivo local): viavel,
   baixo.** Reaproveita o `romdb` (mesma estrutura de CRUD e busca).
7. **Download do site (tsx.eslamejor.com, "TSX MSX Files Repository"): viavel tecnicamente, mas sem termos
   de uso publicados.** Verificado em 2026-10-06: as paginas de termos, politica e FAQ nao existem (404). A
   pagina principal diz que o objetivo e' "preservar fitas antigas de MSX" e que os responsaveis nao pretendem
   "atentar contra qualquer direito autoral ainda em vigor". Isso nao e' uma licenca de uso. A colecao e' oferecida
   como um arquivo `.torrent` (`files/tsx-files_20260402.torrent`), nao como downloads individuais. Nao foi
   encontrada nenhuma loja ou venda no site (a unica referencia a app e' um link de terceiros na Google Play).
   Consequencias: (a) o fwMSX nao redistribui fitas; (b) a busca e o download so' acontecem por acao do
   usuario; (c) antes de implementar um download automatico, pedir confirmacao por escrito ao responsavel pelo
   site sobre o uso programatico do arquivo e a permissao de baixar; (d) o banco guarda metadados e hash, nao o
   arquivo da fita.
8. **Ferramenta de linha de comando e menu: viavel, baixo.** `fwmsx --fita <comando>` (ler, criar, converter,
   listar, buscar no banco) e um menu "Fita" na janela (inserir CAS/TSX, ejetar, rebobinar, banco de fitas).

**Licencas.** makeTSX (MIT) e CLK (MIT): podem ser portados com aviso de copyright. openMSX-TSX (GPL): so' como
referencia, sem copia de codigo. Os arquivos TSX/CAS de jogos sao de terceiros: nao entram no repositorio.

**Ordem sugerida de implementacao.**

- [ ] (a) Leitor de TZX/TSX e de CAS, sem janela; testes com arquivos de exemplo.
- [ ] (b) CAS por hooks da BIOS (BLOAD e CLOAD), para programas que nao dependem de protecao.
- [ ] (c) Escritor de TSX a partir de .BIN e .BAS.
- [ ] (d) Porta de cassete no PPI e reproducao de pulsos (TSX e TZX completos).
- [ ] (e) Banco de fitas (SQLite) e download pelo site, depois de conferir os termos.
- [ ] (f) Port do makeTSX (WAV para TSX).
- [ ] (g) CLI `fwmsx --fita` e menu "Fita" na janela.

