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

  **Estado atual (2026-10-13, v1.39.0 "Yie Ar Kung-Fu", Konami):** a maquina MSX1, MSX2 e MSX2+ estao completas no nucleo
  (VDP com efeitos de rastreio por linha, PSG, SCC, FM com BASIC, disco, joystick, SRAM e layout
  de slots), com o banco de ROMs (`--romdb`, agora com `verify` e auto-mapper pelo SHA-1 ao
  carregar um cartucho), o subsistema de fita completo (leitura, gravacao, empacotador
  `.BIN`/`.BAS`, ripper de `.WAV`, navegacao de blocos do TZX e banco de metadados `--fitadb`),
  MSX-DOS 2 (cartucho generico, agora tambem escolhivel pela janela) e save-state tambem
  publicados. Menus da janela reagrupados (Midia, Tela) na 1.30.0. Ver a secao 5.0 para o que
  funciona e o que falta. O mapa de memoria ja tem o FM-PAC, a SRAM e o MSX-DOS 2; falta o
  GameMaster2 (`doc/memory-map-spec.md`, secao 6).

### 5.0 Estado atual e proximos passos (atualizado em 2026-10-19, depois da v1.40.5)

Esta secao e' o "onde paramos" oficial. O historico de cada versao esta em
[CHANGELOG.md](CHANGELOG.md) e [RELEASE.md](RELEASE.md).

**Visoes registradas (so' estudo, nada implementado; 2026-10-19):** a secao 5.3 (o IDE do fwMSX: editor de textos em TUI/GUI, executar BASIC/Dignified/Assembly no MSX embutido, Markdown/ASCII/PDF, projetos com `Obifile` do Kizuna e as linguagens do Kizuna)
e a secao 5.4 (os editores e utilitarios do msxIDE e do PaleoBasic sob o mesmo teto: fontes, sprites, telas, som, tracker, MML, hexadecimal...). Cada uma tem subtopicos "a detalhar" para as proximas conversas.

**Funcionando (v1.40.5, publicada em `main`):**
- [x] **Montador na TUI e base do depurador avancado** (1.40.5): `asmrun` (monta e CARREGA: ROM/MegaROM como cartucho, `.basic` por fita montada na hora, `.msxdos` num disco de sistema), menu Montador na TUI e item na janela; simbolos e arquivo:linha no depurador (`break loop`, `break jogo.asm:42`,
  `sym`, `src`, eventos com `sym=`/`src=`, `dis` com rotulos, painel Z80 Debugger); marcadores `.break`/`.debug` do fonte funcionando no Z80 (`markers`); BOM/UTF-8 no fwasm. Ver `debug-spec.md`, secao 8.
- [x] **Fita em todos os pontos** (1.40.4): `.cas`, `.tsx`/`.tzx` e `.wav` (ler e gravar) por uma camada so' (`tape_io`) e o `TapeDeck` (a fita sem hardware; o `TapeEngine` herda dele); comando `tape` com `new list seek protect write mode save put load`;
  `break tapin`/`until tapoon`/`dis tapion` (as 6 entradas da fita na BIOS tem nome); Mamute `TAPE R TLOAD TSAVE`, `LOAD`/`SAVE` de fita e `A OT>arq.cas` (online usa a fita da MAQUINA, offline um deck proprio; `man mamute-fita`); fwasm `.TSX`;
  menus da TUI e da janela; banco de fitas varre `.wav`. O `.cas`/`.tsx`/`.wav` montado pelo fwasm carrega por `BLOAD"CAS:",R` e RODA no emulador (`asme2etest`; o `.tsx` tambem pelos pulsos). Ver `tape-spec.md`, secao 12.
- [x] **Mamute SUPER-X** (1.40.3, Fase 4): o enderecamento estendido (`#slot-sub`, `#V`, `#S`) e as variaveis `@`; `CL`; `XD XA XI XH XM` (paineis na TUI, telas e listagens no console, `?XD` em PDF); `XBT XRT XFL XCM XFD XTS XCS XSD`; `XRG XGO XTR` com
  5 breakpoints nomeados; `XPP XPI XPO` (a CPU offline passou a ter portas); `XDK XFS XCI XTP XL% XS% XSV XLD XS# XL#` (disco corrente e arquivos); notas `XIM XIC XIL XIS XIR` (471 notas da BIOS em `SUPER-X-PT.notas`); `XCO`; `man mamute-superx`.
  Kernels novos em C (`mamute_reloc.c`), Fortran (`mamute_total.f90`) e NASM (`mamute_cmp.asm`). Falta no Mamute: o SCR em pixels na janela ImGui, o TAB do console, a "cruz" de troca entre XD/XA/XI/XM/XH, o `CLS`, as cores do `XCO`.
- [x] **Compatibilidade fwasm x asMSX v1.2.0**: medida 100% (6606 casos + 1 programa real), estimada ~95%; pendencias em `asm-spec.md`.
- [x] **Mamute telas e PDF** (1.40.2, Fase 3): paineis `DM M S ZAP SCR` na TUI (e versoes de linha no console), `ZAPW`, teclado S configuravel, `P/LP/V/A P/A H` em PDF, `man mamute-screens`.
- [x] **Mamute EDIT e A** (1.40.1, Fase 2): editor de fonte de linhas numeradas + montagem pelo fwasm (listagem, `O N P I R S D H /desloc`, `MAP`), `man mamute-edit`.
- [x] **Mamute, o monitor Z80** (1.40.0, Fase 1): memoria offline (16 blocos de 16 KB) e online (a maquina emulada), 31 comandos, `mon`/`--mon`, `mamute.cfg`, `man mamute`; fases 2-4 em `mamute-spec.md`.
- [x] **fwasm com o jogo Amazonia** (1.39.2): `.rom` 48 KB e `.sym` iguais ao asMSX v1.2.0; corrigidos fonte em subpasta, include/incbin relativos ao diretorio atual e `#hex`.
- [x] **fwasm com programa real** (1.39.1): replayer PT3 de SapphiRe monta igual ao asMSX v1.2.0 e toca no emulador (`asme2etest`); corrigidos `ROTULO: .include` e `nome .equ valor`.

- [x] **fwasm completo** (1.39.0): `.rom`, `.megarom` (+`.subpage`/`.select`), `.basic`, `.msxdos`, `.cas`, `.wav` (NASM), `.sinclair`, `.size`, `.search`, `.callbios`, `.calldos`, `.debug`, `.break`, `.start`; 382 casos novos conferidos byte a byte contra o asmsx.exe. Ver `doc/asm-spec.md`.
- [x] **Manual do fwasm, NestorBASIC, SEE Tracker e MSXBAS2ROM no `man`** (1.38.5): `man fwasm` (11 paginas, estado de cada diretiva), `man nestorbasic`/`nb-23`, `man seetracker`, `man msxbas2rom`.
- [x] **MSX-DOS 2, MSX-DOS 1, CP/M, Z80, R800, FM-PAC, Turbo-Basic e extensoes `CALL` no `man`** (1.38.4): `man dos2-copy`, `man _strout`, `man z80-opcodes`, `man call kanji`, `man ref-error-codes`... (MSX-DOS 2 partido por comando e por chamada de funcao).
- [x] **Livros no `man`** (1.38.3): The MSX Red Book (973 topicos), MSX2 Technical Handbook (1356) e a ajuda do Basic Dignified (21), com os links internos como VER TAMBEM; `man -l red`, `man -l handbook`, `man dignified`.
- [x] **BIOS/BDOS/RAM e hardware no `man`** (1.38.2): 554 rotinas/variaveis (`man chput`, `man jiffy`) e 33 topicos de hardware (`man hw-rtc`, `man opll`...), em ingles, do paleobasic; `man -l` lista por categoria e `man -k` procura.
- [x] **Manual do MSX-BASIC no `man`** (1.38.1): 172 palavras reservadas + 23 topicos do manual (erros, mapa da memoria, ASCII, FM-Music...), em portugues, importados do paleobasic. `man print`, `man screen`, `help print`.
- [x] **`help` organizado, `help <comando>` e `man <topico>`** (1.38.0): tabela agrupada; manual com exemplos (pager no console, janelinha na TUI); referencia do MSX (`bios`, `sysvars`, `hooks`, `basic`, `cores`, `screens`, `vdpregs`, `psgregs`, `portas`, `slotsmsx`, `teclado`) conferida contra as BIOS. Ver `doc/man-spec.md`.
- [x] **`--keys` com a sintaxe de teclas do `type`** (1.37.8): `{enter}`, `{ctrl}+{c}`, `{ctrl}{c}`, `{enter*3}`; aspas e espacos continuam texto comum.
- [x] **`type` com teclas** (1.37.7): `type "key off" {enter}`, `{esc}`, `{f1}`, `{ctrl}+{c}` (juntas), `{ctrl}{c}` (em sequencia), `{enter*3}`. Ver control-spec secao 2b.
- [x] **TUI sem sujeira** (1.37.6): a saida dos comandos passa a ser capturada por thread (`ThreadCaptureBuf`), nao trocando o `rdbuf` global; antes, redesenhos caiam no historico como codigos ANSI.
- [x] **TUI mais leve** (1.37.5): redesenha so' quando algo muda (antes, 12x/s sempre). Lentidao de digitacao do console relatada pelo usuario: NAO reproduzida (medida em `tools/conperf`); aguardando detalhes do terminal.
- [x] **Teclado japones x internacional no `type`/`--keys`/modo terminal** (1.37.4): a pontuacao digitada usa o layout da BIOS (byte 002Ch); as tabelas sao conferidas contra as das BIOS. F10 (Z80 Debugger) e splash conferidos em tela.
- [x] **Painel Z80 Debugger na TUI + comando `dbgui`** (1.37.3): o mesmo painel da janela como texto no terminal, calculado no emulador; abre sozinho em breakpoint/watchpoint/until. Ver `doc/debug-spec.md`, secao 6c.
- [x] **Z80 Debugger na janela** (1.37.2): F10 abre o painel (registradores no topo, `ENDERECO: bytes instrucao`, ENTER/setas/G/D/M/S/J/R/C/B/Q) com a fonte do proprio MSX; abre sozinho em breakpoint/watchpoint/`--debug`. Ver `doc/debug-spec.md`, secao 6b.
- [x] **Splash, icone do aplicativo e `fwmsx.cfg`** (1.37.1): imagem de abertura de 3 s na janela (`--splash`/`--no-splash`, menu Interface; terminal so' com `--splash`; `--cmd` nunca), icone embutido no .exe e na janela, tema/letra/moldura/splash gravados. Ver `doc/splash-spec.md`.
- [x] **Depurador basico + eventos assincronos** (1.37.0): breakpoints, watchpoints, `stepi`/`next`/`until`/`go`/`stop`/`jump`, `dis`/`mem`/`stack`/`reg`/`slots`; pela ponte, console, TUI (menu Depurador) e modo terminal; eventos `! break ...` por conexao (`events on`); `--break`/`--watch`/`--debug`. Ver `doc/debug-spec.md`.
- [x] **Comparacao de quadros com o fMSX original** (ferramenta, `tools/fmsxgo_ref/` + `tools/fmsx_ref/`): 26 casos (BIOS MSX1/MSX2/MSX2+, 4 jogos do fmsxgo e 19 do file-hunter em `tests/media/`, provisorios), todos aprovados; achou o texto de SCREEN 0 (1.36.1) e o R-Type (1.36.2). Ver `doc/fmsx-compare-spec.md`.- [x] **Montador Z80 `fwasm`** (1.36.0): `fwmsx --asm` / comando `asm` do console -- reescrita clean-room do asMSX 1.2.0 (C + Fortran + C++), saidas `.z80`/`.sym`/`.txt`, testado contra ~6.200 casos gerados pelo `asmsx.exe` real (`ctest` `asm`). Falta: ROM/MegaROM/BASIC/MSX-DOS/CAS/WAV, `select`/`subpage`/`search`, e o modulo Assembly. Ver `doc/asm-spec.md`.
- [x] MSX1, MSX2 e MSX2+: BIOS real ate o prompt do MSX BASIC (1.0, 2.1 e 3.0); MSX-DOS 1.8 do disco (pela memoria ou pelas portas, DDX 3.0/CDX-2) ate `A>`; MSX-DOS 2.30 de verdade via `--cart MSXDOS2.ROM msxdos2` (`dir`/`cd` em subdiretorios).
- [x] Z80 completo; mapa de slots e subslots; mappers Konami, ASCII, Gen8, Gen16 e MSX-DOS 2; SRAM ASCII8/ASCII16 e FM-PAC (`.sav`); save-state (menu Arquivo, `.sst`).
- [x] VDP completo: TMS9918 (SCREEN 0-3), V9938 (SCREEN 4-8, comandos, sprites) e V9958 (SCREEN 10-12, YJK/YAE, scroll); efeitos de rastreio no meio do quadro (paleta/scroll por linha, 1.27.0).
- [x] PSG, SCC e FM (OPLL): 9 canais melodicos, 15 timbres, modo ritmo; comandos de BASIC do MSX-MUSIC pelo FM-PAC.
- [x] Janela com os menus do fMSX, zoom, proporcao, tela cheia e filtros de video -- **confirmados na tela pelo usuario em 2026-10-07**, junto com o menu ROMs, Banco de ROMs, Navegar file-hunter, Configuracao de disco e de slots.
- [x] Configuracao de slots pelo menu ou `--slot`: 16 celulas, BIOS em 0:0, RAM 16/32/64 KB (16KB no fim da celula), mapper ate 4096 KB (varios mappers), disco, sub-ROM, cartucho e FM-PAC.
- [x] Banco de ROMs (SQLite, `fwmsx --romdb`): downloads do fMSX 6.0, do System ROMs do file-hunter e do Vampier; CRUD, busca, identificacao, CARTS.SHA. Ver `doc/romdb-spec.md`.
- [x] **Fita** (.CAS e .TSX/.TZX): leitura completa, carregamento rapido (sem som) e normal (pulsos de verdade, com som), menu "Fita" e janela visual "Fita K7". **Confirmado pelo usuario em 2026-10-07**: carregou um jogo completo (modo normal) e jogou um pouco, som nitido. Ver `doc/tape-spec.md`.
- [x] Pacotes Windows (zip) e Linux (tar.gz); `ctest` com 16 suites.

**Fita -- gravacao, empacotador, ripper de WAV, navegacao de TZX e banco de metadados (v1.20.0 a v1.24.0, 2026-10-07/08, todas publicadas em `main`):**

- [x] **Gravacao** (`CSAVE`/`BSAVE "CAS:"`, sempre pelo gancho de BIOS TAPOON/TAPOUT/TAPOOF, independente do modo de carregamento escolhido). **Confirmado pelo usuario em 2026-10-08**, testando pela janela: criou uma fita nova, gravou um programa, rebobinou e recarregou com sucesso.
- [x] **Corrigido** (1.20.1, achado pelo usuario testando pela janela): pulsos de ZERO e UM do #4B estavam trocados (zero tem que ser 2x o pulso de um, nao o contrario) e o piloto era curto demais para a BIOS de verdade calibrar -- sem isso, uma fita GRAVADA pelo emulador nunca carregava no modo normal (so' no rapido, que nao usa pulso). Ver `doc/tape-spec.md`, secao 4, e `doc/CHANGELOG.md`, `[1.20.1]`.
- [x] **Fita nova em branco** (`.tsx`, menu "Nova fita...") para o usuario salvar programas do zero.
- [x] **Protecao contra gravacao**: fita de arquivo entra sempre travada (so' leitura); fita nova entra destravada. Travar/destravar pelo menu.
- [x] **Tres modos de gravacao** (fitas sao lineares, como as de verdade): incluir no final (padrao, "ir enchendo a fita"), sobrescrever o ponto marcado, ou nova fita (apaga tudo).
- [x] **Marcar um arquivo da lista** (janela "Fita K7", clicavel) como o ponto onde o proximo carregamento comeca a buscar, ou onde a proxima gravacao (no modo "sobrescrever") trunca e escreve.
- [x] **Contagiros** (odometro simulado) na janela "Fita K7", avancando conforme a fita "roda".
- [x] Escritor generico de .CAS -> .TSX (`src/tape/cpp/tsx_writer.cpp`), usado pela gravacao e pela criacao de fita nova.
- [x] Testes: `tapetest` (CTest `tape_load`, 68 checagens -- 22 novas para gravacao/protecao/modos/marcacao/contagiros).
- [x] Ver `doc/tape-spec.md`, secoes 5-7, para o detalhamento completo (inclusive os limites conhecidos).
- [x] **Corrigido** (1.20.2, achado pelo usuario testando pela janela os 3 modos de gravacao): reinserir a MESMA fita ja' destravada voltava a trava-la sem aviso (`Device I/O error` ao tentar sobrescrever ou criar nova fita); e o preenchimento de alinhamento (padding) do `TAPOON` era gravado como se fosse dado de verdade ao reler uma fita GRAVADA por este emulador (bug da secao 5 do `doc/tape-spec.md`, agora resolvido para esse caso via marca exata `TapeMark` por bloco). Contagiros parado no CLOAD investigado e confirmado como NAO sendo bug. Ver `doc/CHANGELOG.md`, `[1.20.2]`.
- [x] **Corrigido** (1.20.3, achado pelo usuario testando "sobrescrever o ponto marcado" pela janela): um CSAVE de verdade chama `TAPOON`/`TAPOOF` duas vezes (cabecalho com nome + dados), e a logica de corte reaplicava em CADA chamada, apagando o cabecalho que a 1a chamada tinha acabado de escrever -- o programa "desaparecia" por completo. Corrigido fazendo o corte rodar uma so' vez (o modo volta sozinho para "incluir no final" depois). **Revalidado pelo usuario na janela em 2026-10-08**: gravou num ponto no meio da fita, sobrescrevendo um programa existente, com sucesso. Ver `doc/CHANGELOG.md`, `[1.20.3]`.
- [x] **`fwmsx --cas pack`/`list`** (1.21.0): ferramenta de linha de comando para empacotar um `.BIN`/`.BAS` solto (ja no formato binario/tokenizado do MSX) num `.TSX`/`.CAS` valido sem passar pelo emulador -- fecha o resto do item (c) da lista de fitas. Reusa o escritor `WriteTsxFromCas()` existente. Ver `doc/tape-spec.md`, secao 8, e `doc/CHANGELOG.md`, `[1.21.0]`.
- [x] **`fwmsx --cas rip`** (1.22.0): port do CONCEITO do makeTSX (WAV -> TSX, item f) -- demodula uma gravacao real de fita (`.wav` PCM mono) detectando o piloto e decodificando os bytes do bloco #4B, simplificado para o caso fixo do MSX (sem os modos interativo/preditivo do original). Testado contra uma fita real dos anos 80 (86 blocos reconhecidos sem erro). Ver `doc/tape-spec.md`, secao 9, e `doc/CHANGELOG.md`, `[1.22.0]`.
- [x] **Navegacao de blocos de controle do TZX** (1.23.0): Grupo/Salto/Laco/Chamada/Selecao (IDs 21-28) agora sao executados de verdade (leitor em 2 passadas -- indexa os blocos, depois navega por indice), nao so' pulados com seguranca como antes. Fecha o ultimo item do "passo 1" pedido pelo usuario. Ver `doc/tape-spec.md`, secao 5, e `doc/CHANGELOG.md`, `[1.23.0]`.
- [x] **`fwmsx --fitadb`** (1.24.0): banco de fitas (SQLite) com metadados (titulo/empresa/ano/SHA-1) -- fecha o item (e), SEM download (o site de referencia nao publica termos de uso). Estrutura espelhada no `romdb` por analogia (banco SEPARADO); auto-preenche o titulo a partir do leitor de fita existente. Ver `doc/tape-spec.md`, secao 11, e `doc/CHANGELOG.md`, `[1.24.0]`.
- [x] **Banco de ROMs: mapper e verificacao** (1.25.0): `--cart <arquivo>` sem mapper explicito consulta o banco pelo SHA-1 antes da heuristica por tamanho/conteudo de sempre; `fwmsx --romdb verify` recalcula o SHA-1 de cada ROM cadastrada e aponta arquivo faltando/alterado. Fecha os dois itens pendentes de `doc/romdb-spec.md`, secao 8. Ver `doc/CHANGELOG.md`, `[1.25.0]`.
- [x] **Vampier: Platform, CRC32 e tamanho** (1.26.0): o JSON do Vampier foi avaliado e descartado (e' um subconjunto do SQL ja' importado); em vez disso, `VampierSearch()` passou a expor `Platform`/`CRC32`/`FileSize`, que o dump SQL ja' trazia mas a consulta ignorava. Fecha o ultimo item da lista original de ROMs. Ver `doc/romdb-spec.md`, secao 8, e `doc/CHANGELOG.md`, `[1.26.0]`.
- [x] **Efeitos de rastreio no meio do quadro** (1.27.0): a interrupcao de linha (IE1) e a intercalacao de Z80/VDP ja' eram corretas (confirmado por investigacao dedicada); a lacuna real era `Machine::RenderFrame()` ler so' o estado FINAL do quadro. Corrigido com um snapshot por scanline (`VdpScanlineSnapshot`, capturado em `vdp_step_scanline()` e aplicado linha a linha em `RenderFrame()`) -- paleta e scroll trocados por uma ISR de IE1 no meio do quadro agora aparecem a partir da linha certa, nao retroativos a tela inteira. Ver `doc/vdp-spec.md`, secao 2, `doc/msx2-spec.md`/`doc/msx2p-spec.md`, secao 6, e `doc/CHANGELOG.md`, `[1.27.0]`.
- [x] **MSX-DOS 2 (cartucho generico)** (1.28.0): `MEMMAP_MAPPER_MSXDOS2`, adaptado de `RomMSXDOS2` do openMSX (GPL, so' estudo) -- ROM de 64KB, granularidade de 16KB, so' a pagina 4000h-7FFFh e' trocavel, endereco de disparo lido do byte 94h da propria ROM. Testado contra um `MSXDOS2.ROM` e um disco de 720KB REAIS: o kernel sobe, `dir`/`cd` em subdiretorios funcionam, sem precisar de mapper de RAM (a suposicao antiga estava errada). Ver `doc/memory-map-spec.md`, secao 6, e `doc/CHANGELOG.md`, `[1.28.0]`.
- [x] **Save-state** (1.29.0): `Machine::SaveState()`/`LoadState()` gravam/restauram o estado AO VIVO da maquina (Z80, VDP incl. VRAM, PSG, SCC, OPLL, PPI, controladora de disco, RAM/RAM de mapper) num arquivo `.sst` proprio, aplicado por cima da maquina JA' RODANDO (nao recarrega BIOS/cartucho/disco/fita -- pressupoe a mesma midia inserida). Menu **Arquivo > Salvar estado.../Carregar estado...**. Fecha o ultimo item de "1, 4, 5". **Confirmado pelo usuario na janela em 2026-10-08.** Ver `doc/savestate-spec.md` e `doc/CHANGELOG.md`, `[1.29.0]`.
- [x] **TUI de menus** (1.35.0): `fwMSXc.exe --tui` -- os comandos do console em menus (Emulador, Arquivo, Maquina, Midia, Ferramentas, Ajuda), com assistentes, navegador de arquivos e linha de comando; controla o emulador em janela pela ponte de controle. Ver `doc/tui-spec.md`.
- [x] **Modo terminal do emulador** (1.34.0): `fwMSXc.exe --term` -- a maquina dentro do terminal (FTXUI), menu, linha de comando e atalhos da janela, tudo via `control::Commander`. Ver `doc/term-spec.md`.
- [x] **Console (REPL)** (1.33.0): `fwmsx --cli` -- `emu start` abre o emulador em outro processo e o console segue livre, mandando comandos pela ponte de controle; `newdisk`, `romdb`, `cas`, `fitadb`, `msxdisk` como comandos. Ver `doc/repl-spec.md`.
- [x] **Ponte de controle externa** (1.32.0): `src/control/` -- protocolo de texto (uma linha por comando) sobre TCP em 127.0.0.1; `fwMSX.exe --msx --ctl-port N`; comandos peek/poke/type/cart/disk/state/screenshot/pause/step/quit... num `Commander` unico que a TUI tambem usara. Ver `doc/control-spec.md`.
- [x] **Disco novo em branco e FORMAT** (1.31.0): `src/diskfmt/` (C + Assembly + C++) cria discos formatados de 180/360/720 KB (5 1/4 e 3 1/2, sem 1,44 MB) pela janela (Midia > Disco) e por `fwmsx --disknew`; o WD2793 aceita WRITE TRACK, entao `CALL FORMAT` funciona (testado com o DISK.ROM real). Ver `doc/diskfmt-spec.md`.
- [x] **Atalhos de save-state, captura de tela e janela sem console** (1.30.1): 9 slots (F6 salva, F7 carrega, F8/F9 trocam), aviso na tela, CRC32 de BIOS/cartucho no `.sst` (secao `MEDA`), F12 = PNG, `fwMSX.exe` sem janela de console no duplo clique. Ver `doc/CHANGELOG.md`, `[1.30.1]`.
- [x] **Menus reagrupados + mapper do cartucho na janela** (1.30.0): feedback do usuario depois de testar save-state e MSX-DOS 2 -- nao achou onde escolher o mapper `msxdos2` pela janela (so' existia via `--cart <rom> msxdos2`), e os 13 menus do topo estavam "muito separados". `SetCartridge()`/`CartridgeMapper()` ganharam um parametro/accessor de mapper; a janela ganhou um submenu **Mapper** (Cartucho) e um combo na Configuracao de slots. Menus reagrupados: **Midia** (Disco/Fita/Cartucho como submenus), **Tela** (Exibir/Video), **Ferramentas** (incorporou Configuracoes > Interface) -- de 13 para 9 menus no topo. Ver `doc/CHANGELOG.md`, `[1.30.0]`.

**Politica de midias (2026-10-06):** o repositorio e' pessoal; ROMs, discos e fitas de terceiros podem ser versionadas. Antes da liberacao publica, revisar cada midia e remover as que o detentor contestar (`LICENSE-THIRD-PARTY.md`).

**Projeto futuro:** o fwMSX sera integrado ao msxide num utilitario de desenvolvimento MSX chamado **MSX-PoorManOS**. Ate la, sem preocupacao com redistribuicao.

**Pendencias (ordem sugerida):**

- [ ] Fitas: download pelo site continua fora de escopo (sem termos de uso publicados -- so' o banco de metadados, sem download, foi feito na 1.24.0); preenchimento de alinhamento ainda e' adivinhado (busca pelo proximo cabecalho) para `.cas` cru carregado direto do disco, sem passar por uma gravacao deste emulador (limitacao conhecida, ver `doc/tape-spec.md`, secao 5); `ScanCasFiles()` so' junta DOIS blocos por arquivo (nome + dados) -- um ASCII multi-bloco de verdade (256 bytes por bloco) aparece fragmentado (ver `doc/tape-spec.md`, secao 9). **Revalidado pelo usuario em 2026-10-08**: o modo "nova fita" de gravacao ja' foi confirmado na janela de verdade (junto com os outros modos).
- [ ] Banco de ROMs: montar a maquina pelo banco (layout por nome) continua sem comecar. Lista de ROMs esta' fechada por ora (ver `doc/romdb-spec.md`, secao 8).
- [ ] Ouvir o FM, o SCC, o disco e a fita (modo normal) contra referencia; ajustar as constantes do OPLL (`doc/fm-spec.md`, secao 2) e os ganhos da mistura.
- [ ] Jogos: Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada, causa nao diagnosticada; Mega Chase validado so' ate o titulo; F-1 Spirit 3D: troca de disco pela janela nao testada.
- [ ] FM: `CALL VOICECOPY` (a ROM do fMSX nao aceita); status e timers do OPLL.
- [ ] Disco: formatar disquetes (hoje so' le e grava); FM/MFM modelados; drives A e B com formatos diferentes; mais de um driver de porta (Sony, Philips, Spectravideo do openMSX) se quiser estudar.
- [ ] Layout de slots: salvar e carregar o layout em arquivo; perfis de maquina salvos no banco.
- [ ] Controle externo do emulador, no estilo openMSX: canal de controle opt-in em localhost (TCP ou pipe), comandos `status`, `reset`, `pause`/`resume`, `type`, `cart`, `disk`, `fita`, `screenshot`, `peek`/`poke`, `quit`; eventos de troca de modelo e de midia. A thread de controle so' enfileira comandos; quem toca na maquina e' o laco de quadros. Ainda nao comecou.
- [ ] Cartuchos: GameMaster2 e o MSX-MUSIC com BIOS propria (MSX-DOS 2 feito na 1.28.0).
- [x] BIOS de outras maquinas (ex.: Gradiente Expert 1.1): a BIOS sobe; os espacos entre as letras ("G r a d i e n t e") estao gravados na PROPRIA ROM (offset 32513), nao sao bug do emulador (1.30.1).
- [ ] Cores YJK do V9958 conferidas com hardware real.
- [ ] Desempenho: medir o custo de CPU no pior caso (FM ativo, SCC, fita, mapa de slots, disco) em tempo real.
- [ ] Licenca: confirmar por escrito a autorizacao de uso do fMSX antes de mudar o texto de README e LICENSE-THIRD-PARTY. O openMSX (GPL) segue so' como referencia; decidir se sai do repositorio quando o fwMSX estiver pronto.
- [ ] Frontend para jogar (biblioteca de jogos e imagens) sobre o banco de ROMs: depois.

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

Estado: **leitura, carregamento, gravacao, empacotador, ripper de WAV, navegacao de TZX e banco de
metadados prontos (2026-10-08)**; so' o download pelo site continua fora de escopo (sem termos de uso
publicados, ver item 7 da viabilidade abaixo). Pedido original do usuario: ler e criar TSX, ler e criar
CAS, um banco de fitas com download pelo site oficial, e uma ferramenta de linha de comando e pelo menu
para manipular esses arquivos, com duas opcoes de carregamento (rapido, sem som; normal, com o barulho
do gravador), uma fita K7 nova e limpa em .TSX para salvar programas, uma janela com o conteudo da fita
(marcar o ponto de carga), tres modos de gravacao (sobrescrever o ponto/nova fita/incluir no final),
protecao contra gravacao por padrao em fitas existentes, e um contagiros simulado. Todos os itens (a)
a (g) estao feitos, exceto a parte de DOWNLOAD do item (e) -- ver `doc/tape-spec.md` para o
detalhamento completo (enderecos da BIOS, layout do bloco #4B, gravacao, CLI, TZX, banco, limites). Ver
tambem `doc/SPEC.md`, secao 5.0, e a viabilidade abaixo (mantida como registro da analise
original).

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
4. **[FEITO, 2026-10-07] Emulacao de sinal de fita (modo normal): pulsos de verdade, cadencia de T-states.**
   A entrada de cassete (CASRD) NAO fica no PPI -- fica no bit 7 do R14 do PSG (confirmado no openMSX,
   `MSXPSG.cc::readA()`); o motor continua na porta C do PPI (bit 4, `AAh`, ativo em zero, confirmado em
   `MSXPPI.cc::writeC1()`). A reproducao gera a sequencia de pulsos a partir dos blocos (#4B, #10-#14, #20)
   e entrega o nivel de entrada na cadencia de T-states da CPU, como o PSG faz com as amostras -- inclusive
   o som, sintetizado e somado ao audio ao vivo. Loaders de protecao com blocos de controle (grupos, lacos,
   saltos) nao navegam ainda (lidos em sequencia, ver `doc/tape-spec.md`, secao 5) -- risco que se confirmou
   limitado, nao bloqueou o caso comum.
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

- [x] (a) Leitor de TZX/TSX e de CAS, sem janela; testes com arquivos sinteticos e um .TSX real
      (`tapetest`, CTest `tape_load`, 68 checagens).
- [x] (b) CAS/TSX por gancho de BIOS (TAPION/TAPIN/TAPIOF -- carregamento "rapido", sem som).
- [x] (c, parcial -- 2026-10-07) Escritor de TSX: a parte GENERICA existe (`src/tape/cpp/tsx_writer.cpp`,
      um .CAS -> um .TSX valido com blocos #4B), usada pela gravacao em uso real (TAPOON/TAPOUT/TAPOOF,
      ver item abaixo). Falta so' uma FERRAMENTA DE LINHA DE COMANDO que empacote um .BIN/.BAS solto
      (sem passar pelo emulador) -- a necessidade principal (`CSAVE`/`BSAVE "CAS:"` de dentro do MSX)
      esta' resolvida.
- [x] (d) Entrada de cassete no PSG (R14) e motor no PPI; reproducao de pulsos (TSX #4B/#10-#14/#20 e
      CAS sintetizado) com som real -- carregamento "normal". **Gravacao** (2026-10-07): TAPOON/TAPOUT/
      TAPOOF sempre pelo gancho de BIOS (nao ha' "modo normal" de gravar -- precisaria de um "ripper" de
      pulsos em tempo real); fita nova em branco; protecao contra gravacao (fita de arquivo trava por
      padrao); 3 modos de gravacao (incluir no final, sobrescrever o ponto marcado, nova fita); marcar
      um arquivo da lista como ponto de carga/gravacao; persistencia imediata no arquivo. Falta:
      navegacao pelos blocos de controle do TZX (grupos/lacos/saltos). Ver `doc/tape-spec.md`, secoes 5-6.
- [x] (e, parcial -- 1.24.0) Banco de fitas (SQLite) com metadados, via `fwmsx --fitadb`. O download
      pelo site continua fora de escopo (sem termos de uso publicados, ver item 7 da viabilidade
      acima) -- o banco so' cadastra fitas que o usuario ja' tem no disco. Ver `doc/tape-spec.md`,
      secao 11.
- [ ] (f) Port do makeTSX (WAV para TSX).
- [x] (g) CLI `fwmsx --fita`/`--fita-modo` e menu "Fita" na janela (inserir, nova fita, ejetar, rebobinar,
      trocar de modo de carregamento, travar/destravar gravacao, modo de gravacao, janela visual "Fita K7"
      com os rolos girando, contagiros e a lista de arquivos clicavel para marcar o ponto).

---

### 5.3 Visao registrada (estudo de viabilidade, 2026-10-19) -- TOPICO 1: o IDE do fwMSX (editor, projetos e build sob o mesmo teto)

> **Status: so' estudo.** Ideia do usuario (Wilson Pilon), registrada aqui para ir sendo detalhada aos poucos. NADA disto esta' implementado nem sera' comecado agora. Os subtopicos marcados "a detalhar" sao
> espacos reservados para as conversas seguintes. Levantamento feito em `resource/msxide/*`, `resource/paleobasic/*` e `resource/kizuna/*` (so' leitura; nenhum codigo copiado).

#### 5.3.1 A ideia, nas palavras do usuario (resumo)

Migrar parte do **msxIDE** (e do **PaleoBasic**) para dentro do fwMSX, para ter "algo como o msxIDE e o PaleoBasic em um unico teto usando o emulador embutido":
- um **editor de textos** que serve a **GUI** (janela ImGui) e a **TUI** (FTXUI);
- digitar um programa **MSX-BASIC** e ja' **executar no MSX** embutido;
- digitar um programa **Basic Dignified Suite** e ja' **compilar e executar**;
- editar **Assembly (asMSX/fwasm)** e ja' **compilar e executar**;
- criar arquivos **`.MD`** e **visualizar** arquivos ASCII e MD na TUI/GUI, e **PDF na GUI**;
- **projetos** que reunem todos os fontes, com uma especie de **Makefile** (mesma sintaxe/ideia do **Kizuna**: o `Obifile` do orquestrador OBI);
- com o tempo, as linguagens do Kizuna (**BASIC, PASCAL e ASSEMBLY**) e outros tipos de editores, alguns so' para a TUI, outros so' para a GUI, outros para as duas.
O usuario viu projetos parecidos para ZX81 e ZX Spectrum (a pesquisar como referencia de interface: nao analisados).

#### 5.3.2 O que o fwMSX JA' tem para sustentar isto (mapa de reuso)

| Peca | Onde | Serve para |
|---|---|---|
| Montador Z80 compativel com o asMSX (fwasm) | `src/asm` | compilar o Assembly do editor; `AsmResult` ja' devolve simbolos, formato, mapper, listagem e se ha' marcadores (1.40.5) |
| `asmrun` | `src/control/command.cpp` | "montar e executar": ROM/MegaROM = cartucho, `.basic` = fita montada na hora + `BLOAD"CAS:",R`, `.msxdos` = disco de sistema + o programa. E' o motor do botao "Executar" |
| Depurador com simbolos e linhas | `src/debug`, `sym`, `src`, `markers` | parar no `.break` do fonte, `break jogo.asm:42`, mostrar a linha do fonte na parada |
| Fita `.cas/.tsx/.wav` e `TapeDeck` | `src/tape` | entregar programas BASIC ao MSX sem disco; gravar o resultado |
| Discos (`msxdisk`, `diskfmt`, `DiskImage`) | `src/msxdisk`, `src/diskfmt` | disco de projeto, MSX-DOS 1/2, criar/ler `.dsk` |
| Console / TUI / terminal / janela | `src/repl`, `src/tui`, `src/term`, `src/machine/gui` | os dois front-ends (FTXUI e ImGui) e a ponte de controle (`Commander`) que todos usam |
| `man` / `help` com BIOS, Red Book, Handbook, MSX-DOS, fwasm, Basic Dignified (21 topicos) | `src/man` | a "Ajuda/Referencia" do IDE ja' existe e e' pesquisavel (`man`) |
| Mamute (monitor Z80) e gerador de PDF | `src/mamute` | `mamute_pdf` ja' escreve PDF (Courier, A4) de listagens |
| Camada de plataforma | CMake, 4 linguagens, Windows + Linux | o IDE nasce multiplataforma |

#### 5.3.3 O que existe nos projetos de origem (levantamento)

**msxIDE** (`resource/msxide`, FreeBASIC, ~29 mil linhas; `editor.bas` sozinho tem ~22,5 mil): IDE em TUI (Windows Console API ou newt), multi-documento com janelas MDI (arrastar, redimensionar, maximizar), rolagem e mouse;
**editor de Markdown** `.md` (F7 alterna edicao / dividido com preview ao vivo / so' leitura; o mesmo motor renderiza toda a ajuda); editor de fontes (`.alf/.fnt/.chr`) e de sprites (`.spr`); compilacao de MSX BASIC / Basic Dignified
(gera `.amx/.bmx`, monta `.dsk` com boot MSX-DOS e lanca o openMSX); Assembly via asMSX (`.asm` com "Hello ASM World", compilar+executar, oferece inserir o binario num BASIC aberto: `BLOAD`, loader `DATA/POKE+DEFUSR` ou `.inc`);
**projetos `.msxproj`** (um SQLite que empacota fontes, binarios e config; abrir extrai para uma pasta de trabalho, salvar reimporta); ajuda integrada e menu Referencia (10 documentos tecnicos); Mamute Assembler (monitor/assembler Z80 nativo,
SUPER-X, impressora virtual em PDF).

**PaleoBasic** (`resource/paleobasic`, PureBasic, v8.8.0): editor com Scintilla (lexers para Dignified e Z80/N80), abas, temas, auto-completar e auto-indentacao; **pre-processador Dignified nativo** (`DignifiedPreprocessor.pbi`, 2,9 mil linhas: labels, `DEFINE`,
`FUNC/RET`, `INCLUDE` com namespaces, remtags) e **tokenizador MSX-BASIC** (`MsxTokenizer.pbi`, 1,2 mil linhas, conferido byte a byte contra o original em Python); gerenciador de disco (`MSXDisk.pbi`); leitor/editor de Markdown (`MdViewerGui.pbi`);
sistema de projeto em SQLite (`ProjectDB.pbi`, 2,2 mil linhas, `.msxproject`, recursos numerados e indice do projeto); ponte com o openMSX; montador Z80 proprio + linker + biblioteca `.REL` + sub-projetos de Assembly (`Z80Asm/Z80Link/Z80Lib/Z80SubProject`);
NestorBASIC e MSXBAS2ROM como ferramentas externas; e os **editores visuais** (ver o TOPICO 2).

**Kizuna** (`resource/kizuna`, Go, v4.11.0): toolchain de 3 linguagens para MSX2+ / MSX-DOS 2 / mapper: `KAJI80` (assembler Z80 modular, estilo asMSX), `WIRTH80` (Pascal tipo Turbo Pascal 4, units), `DIGNAC` (MSX-BASIC Dignified compilado para Z80),
`MUSUBI` (linker com smart-linking e trampolins de troca de banco), `HAKO` (bibliotecario `.hlib`), `MOBDUMP`, `MSXLIB` (biblioteca padrao) e `OBI` (o "make": le o `Obifile`). Objeto relocavel proprio `.MOB`.

#### 5.3.4 Arquitetura proposta (para discutir)

- **Nucleo do editor sem interface** (`src/ide/` ou `src/editor/`): buffer de texto (gap buffer ou piece table), desfazer/refazer, selecao, busca/substituicao, multiplos cursores no futuro, e **realce de sintaxe** como uma funcao
  `Tokenize(linha, estado) -> spans` (MSX-BASIC, Dignified, Z80/asMSX, Markdown, Pascal, Obifile). Nada de FTXUI/ImGui dentro: testavel como o resto do projeto (`tests/`).
- **Dois front-ends finos**: TUI (componente FTXUI proprio: o `Input` do FTXUI nao serve para arquivos grandes) e GUI (widget ImGui proprio, ou uma biblioteca de editor de codigo para ImGui -- a avaliar, inclusive a licenca).
  Mesmo modelo que o painel do Mamute e o Z80 Debugger ja' usam: o modelo calcula, a interface so' desenha e repassa teclas.
- **Ponte de controle como espinha dorsal**: todo comando do IDE tambem e' um comando do `Commander` (`asmrun` ja' e'), entao console, TUI, terminal e janela compartilham o mesmo fluxo de "construir e executar".
- **Regra das 4 linguagens** (CLAUDE.md): C para o tokenizador/lexer de linha, Fortran/NASM onde houver laco numerico quente (busca, medicao de colunas), C++ para o resto. A cada fase pelo menos um modulo em cada.
- **Emulador embutido**: o painel do MSX ao lado do editor (divisao de tela) ou o emulador em janela propria, como hoje; "Executar" carrega e comeca a rodar, "Parar no .break" usa o depurador.
- a detalhar: modelo de janelas/abas na TUI (o msxIDE usa MDI), atalhos de teclado (Scintilla/Windows x estilo EDIT/QuickBasic), temas, configuracao por projeto.

#### 5.3.5 Os fluxos "digitar e executar"

| Linguagem | O que falta alem do editor | Como executar no MSX embutido | Dificuldade |
|---|---|---|---|
| **Assembly (asMSX/fwasm)** | nada de compilador: o fwasm e' o compilador | `asmrun` (ja' feito na 1.40.5) | baixa |
| **MSX-BASIC** (numerado) | tokenizador MSX-BASIC (texto -> binario tokenizado); `MsxTokenizer.pbi` e o original em Python sao a referencia | tokenizado em `.bas` numa fita virtual (`CLOAD`/`RUN`) ou num disco; alternativa sem tokenizador: digitar o texto no BASIC do MSX pelo `type` (lento, mas valido para programas pequenos) | media |
| **Basic Dignified Suite** | pre-processador Dignified (`DignifiedPreprocessor.pbi`, 100% do escopo do `badig.py`) + o mesmo tokenizador | igual ao MSX-BASIC, depois de `.dmx -> .amx -> .bmx` | media/alta (porte de ~4 mil linhas de PureBasic, com a suite de regressao `teste.dmx` ja' existente) |
| **Basic Dignified via Kizuna (`DIGNAC`)** | chamar `dignac` (Go) ou portar | gera `.COM` para MSX-DOS 2: disco + `asmrun`-like | media |
| **Pascal (`WIRTH80`)** e **Assembly Kizuna (`KAJI80`)** | chamar os executaveis Go do Kizuna, ou portar para C++ | `.COM` no disco de sistema (MSX2+ e MSX-DOS 2 obrigatorios; mapper) | alta |
| **Markdown / texto** | nada | so' visualizar | baixa |

a detalhar: dialeto de BASIC "puro" (sem Dignified) como linguagem de primeira classe: numeracao automatica, renumeracao, `LIST` do que o MSX tem na memoria (ler de volta o programa do emulador).

#### 5.3.6 Visualizadores: ASCII, Markdown (TUI e GUI) e PDF (GUI)

- **ASCII** (e texto em geral): trivial nas duas interfaces (modo somente leitura do mesmo editor).
- **Markdown**: renderizar cabecalhos, listas, codigo, links e tabelas. TUI: estilos do FTXUI. GUI: render proprio sobre o ImGui ou uma biblioteca de Markdown para ImGui (a avaliar licenca; md4c e' MIT). O msxIDE tem tres modos (edicao / dividido com preview /
  leitura) e o PaleoBasic um leitor; ambos servem de referencia de comportamento. O `man` do fwMSX ja' e' um corpus grande de texto para testar o visualizador.
- **PDF (so' na GUI)**: o ponto mais caro. Licencas importam num projeto BSD-3: **MuPDF e' AGPL-3.0** e **poppler e' GPL** (nao servem); **PDFium** (BSD/Apache) serve mas e' grande de compilar e embarcar. Alternativa barata e honesta:
  um **visualizador minimo so' dos PDFs que o proprio fwMSX gera** (`mamute_pdf`: Courier, A4, texto puro), sem tentar abrir PDF qualquer; e abrir os demais no visualizador do sistema. a decidir com o usuario.
- a detalhar: imagens (PNG ja' lido pelo splash), `.CHM`/ajuda externa, busca no documento, indice clicavel gerado dos cabecalhos.

#### 5.3.7 Projetos e o "Makefile"

- **Formato do projeto** (a decidir): (a) **pasta + manifesto de texto** (diffavel no git, facil de editar a mao; recomendado para o fwMSX); (b) **arquivo unico SQLite**, como `.msxproj` (msxIDE) e `.msxproject` (PaleoBasic: portatil, mas opaco ao git).
  Em qualquer caso, **importar/exportar** os dois formatos existentes (tabelas `project_meta`, `project_config`, `project_files` no msxIDE; recursos numerados no PaleoBasic).
- **O arquivo de receita** (o "Makefile"): a ideia e' reaproveitar o `Obifile` do Kizuna. Campos que o exemplo real do Kizuna usa: `target` (saida), `entry` (rotulo de entrada), `base` (endereco de carga), `resources` (arquivo, banco, simbolo, tamanho),
  `modules` (nome, fonte, `compiler` = `kaji80|dignac|wirth80`, `bank`) e `link` (mapa). Para o fwMSX ele cresceria com compiladores `fwasm`, `dignified` e `basic`, com alvos de **execucao** (`run: cartridge|tape|disk`), dependencias entre alvos e variaveis.
  Manter a MESMA sintaxe permite usar os projetos do Kizuna sem mudanca; o fwMSX seria um orquestrador a mais (nativo, sem Go).
- **Compatibilidade**: ler `Obifile` e, se o usuario quiser, tambem produzir um (para o `obi build` do Kizuna continuar valendo).
- a detalhar: o que e' "o projeto aberto" na TUI/GUI (arvore de arquivos, aba de saida do build, lista de erros clicavel que pula para a linha do fonte -- o fwasm ja' devolve arquivo e linha), configuracao do emulador por projeto (modelo MSX1/2/2+, slots, discos),
  versionamento do manifesto, projetos de exemplo.

#### 5.3.8 As linguagens do Kizuna dentro do IDE (para depois)

BASIC (Dignified/`DIGNAC`), PASCAL (`WIRTH80`) e ASSEMBLY (`KAJI80`) + linker `MUSUBI` (smart-linking, bancos) + `HAKO`. Duas rotas, a escolher quando chegar a hora: **(a) chamar os executaveis** (Go; o IDE so' orquestra e mostra erros: a mais barata, depende de o
usuario ter os binarios); **(b) portar** o que valer a pena para C++ (a mais cara; so' faz sentido para o que o fwMSX precisa embutir, como o assembler, que o fwasm ja' cobre quase todo). O alvo do Kizuna e' **MSX2+ / MSX-DOS 2 / mapper de memoria**,
que o fwMSX ja' emula (MSX2+, mapper de RAM, MSX-DOS 2 como cartucho): da' para testar de ponta a ponta no mesmo emulador. a detalhar: formato `.MOB`/`.hlib`, mapa de bancos, depuracao com o mapa do `MUSUBI` (simbolos e linhas, na linha do `sym`/`src`).

#### 5.3.9 Riscos e perguntas em aberto

- **Escopo**: o `editor.bas` do msxIDE tem 22,5 mil linhas; nao se porta, reescreve-se por partes, sempre a partir do COMPORTAMENTO (os manuais e o MANUAL.md de cada projeto sao a especificacao).
- **Editor na TUI**: terminais diferentes tratam teclas (Ctrl+setas, F-keys, Alt) de formas diferentes -- o modo terminal do fwMSX ja' tem essa dor; mouse so' onde o terminal deixar.
- **Duas bases de usuarios**: quem usa o msxIDE/PaleoBasic hoje (o proprio usuario) precisa de migracao dos projetos `.msxproj/.msxproject` sem perda.
- **Licenca de tudo que for embutido** (editor de codigo para ImGui, Markdown, PDF): conferir antes de adotar; o projeto e' BSD-3 e nao copia codigo GPL/AGPL.
- **Perguntas para as proximas conversas**: o IDE e' um **modo** do `fwMSX.exe` (como a TUI) ou um executavel a parte? O projeto e' pasta+manifesto ou SQLite? O `Obifile` e' lido como esta' ou ganha extensoes? Quais atalhos/teclado? O que vem primeiro: o editor em si, o fluxo BASIC,
  ou o Dignified?
- **Fases sugeridas (sem compromisso)**: (1) nucleo do editor + TUI com Assembly/`asmrun` (o fluxo que ja' existe) e visualizador de texto/MD; (2) GUI do mesmo editor; (3) projetos + receita; (4) MSX-BASIC e tokenizador; (5) Dignified; (6) Kizuna; (7) PDF na GUI.

---

### 5.4 Visao registrada (estudo de viabilidade, 2026-10-19) -- TOPICO 2: editores e utilitarios do msxIDE e do PaleoBasic sob o mesmo teto

> **Status: so' estudo**, como o topico anterior. Aqui o foco nao e' o editor de texto, e sim a **galeria de ferramentas** que os dois projetos ja' tem e que podem ser reincorporadas ao fwMSX, cada uma na TUI, na GUI ou nas duas.
> Vantagem de estar dentro do fwMSX: elas deixam de ser programas isolados que "geram um arquivo" e passam a ter **preview e teste no emulador embutido** (o VDP, o PSG e o Z80 ja' estao aqui).

#### 5.4.1 Inventario do que existe

| Ferramenta | Origem | Tamanho (linhas) | Formato | Equivalente hoje no fwMSX | Interface sugerida |
|---|---|---|---|---|---|
| Editor de fontes MSX | msxIDE; PaleoBasic (`CharsetEditorGui`, Graphos III `.ALF`; Aquarela `.FNT`) | 2,4 mil (PaleoBasic) | `.alf/.fnt/.chr` (256 x 8 bytes, BSAVE) | nenhum (so' o `XH` do Mamute edita 4 caracteres/sprites 8x8) | **TUI e GUI** (grade 8x8 funciona bem em texto) |
| Editor de sprites | msxIDE (`.spr`); PaleoBasic (`SpriteEditorGui`) | 2,0 mil | bancos 8x8/16x16, cor MSX1 (1/sprite) ou MSX2 (1/linha) | nenhum | **TUI e GUI** (preview com o VDP real do fwMSX) |
| Editores de tela pixel a pixel | PaleoBasic ("Pixelossauro": Screen 0, 1, 2, 1+2; `Screen2EditorGui` 1,5 mil, `Screen12EditorGui` 1,1 mil) | ~5 mil | VRAM de cada modo, `BSAVE` | `SCR` do Mamute so' VE a memoria como pixels | **GUI** (pixels reais); versao simplificada na TUI |
| Graphos III (tela / alfabeto) | PaleoBasic (`GraphosScreenGui` 3,0 mil, `GraphosNativeIO`) | ~3 mil | arquivos do Graphos III | nenhum | **GUI** |
| Editor de som PSG | PaleoBasic (`PsgEditorGui`/`PsgSynth`) | 0,7 mil + sintese | passos por canal, gera BASIC | o nucleo do PSG ja' sintetiza (`src/psg`) | **TUI e GUI** |
| SEE Tracker (efeitos) | PaleoBasic (`SeeTrackerEditorGui` 1,3 mil + driver Z80 + sintese) | ~2 mil | formato do SEE Tracker; gera um driver de replay Z80 | nenhum | **GUI** (e uma tela de tracker na TUI) |
| Editor de musica MML | PaleoBasic (`MmlEditorGui`/`MmlSynth`) | 0,7 mil + sintese | MML -> `PLAY` | nenhum | **TUI e GUI** |
| Editor hexadecimal | PaleoBasic (`HexEditorGui` 1,6 mil) | 1,6 mil | qualquer arquivo | `DM`/`M`/`XD` do Mamute (memoria, nao arquivo) | **TUI e GUI** |
| Gerenciador de disco | PaleoBasic (`DiskManagerGui` + `MSXDisk`); msxIDE | 1,3 mil | `.dsk` FAT12 | **`msxdisk`** (CLI, TUI e GUI, ja' feito) | ja' tem |
| Leitor/editor de Markdown | msxIDE (3 modos, F7); PaleoBasic (`MdViewerGui`) | ~0,2 mil + motor da ajuda | `.md` | `man` (texto) | **TUI e GUI** (ver 5.3.6) |
| Ajuda e Referencia (Red Book, Handbook, BIOS, MSX-DOS, Basic Dignified, asMSX, NestorBASIC, SEE Tracker...) | ambos | enorme (dados `.pbi`) | dicionarios de referencia | **`man`** (importado do paleobasic nas versoes 1.38.x) | ja' tem |
| Mamute (monitor Z80, SUPER-X) | ambos | 5 mil+ (so' a GUI do PaleoBasic) | - | **Mamute** (fases 1-4 e fita, versoes 1.40.x) | ja' tem |
| Ponte com o openMSX / console remoto | ambos | 1,2 mil + console | Tcl/XML | **desnecessaria**: o emulador e' o proprio fwMSX | nao se porta |
| Montador/linker Z80 proprio (`Z80Asm` 4,3 mil, `Z80Link`, `Z80Lib`, `Z80SubProject`, formato `.REL`) | PaleoBasic | ~7 mil | `.REL`, bibliotecas, sub-projetos | **fwasm** (compativel com o asMSX); sem linker de `.REL` | avaliar se o `.REL`/linker entra (o Kizuna tem `MUSUBI`/`.MOB`) |
| NestorBASIC / MSXBAS2ROM | PaleoBasic | 1,2 mil + 0,9 mil | ferramentas externas | so' a ajuda (`man`) | integrar a chamada externa, no projeto |
| Indice de recursos do projeto; associacao de arquivos; temas; botoes tematizados | PaleoBasic | - | - | - | acompanham o IDE (5.3) |

#### 5.4.2 Como uma ferramenta visual se encaixa no fwMSX (padrao proposto)

O mesmo padrao dos paineis do Mamute e do Z80 Debugger: um **modelo sem interface** (`Render(grade)` + `Key(nome)`; testavel sem terminal) e dois desenhistas finos. Para as ferramentas de **pixels** (telas, sprites) o modelo trabalha em
memoria de VRAM e usa o **proprio renderizador do VDP** para o preview; as de **som** usam o nucleo do PSG; o resultado pode ser **enviado ao emulador embutido** ("testar agora": escrever nos registradores/VRAM, ou gerar o `.bin` e rodar com o `asmrun`).
Cada ferramenta entra com um **comando da ponte** (como `asmrun`), para console/TUI/terminal/janela, mais um item de menu.

#### 5.4.3 Classificacao preliminar (a rever com o usuario)

- **TUI e GUI**: fontes, sprites, PSG, MML, hexadecimal, Markdown (ver acima).
- **So' GUI**: telas pixel a pixel, Graphos III, SEE Tracker, PDF.
- **So' TUI**: assistentes de texto (ex.: criar projeto, gerar `Obifile`), navegador de projeto.
- a detalhar: o que o terminal consegue desenhar de verdade (blocos Unicode meia-altura para pixels, cores de 24 bits quando o terminal deixa) para decidir quais ferramentas de pixel valem na TUI.

#### 5.4.4 Formatos e compatibilidade

Cada ferramenta precisa ler e gravar o formato que o msxIDE/PaleoBasic ja' usam, para nao perder o acervo do usuario: `.ALF` (Graphos III) e `.FNT` (Aquarela) de alfabetos (o `docs/reference/aquarela.md` do PaleoBasic tem a engenharia reversa), `.spr`, dumps de tela por modo, MML, o formato do SEE Tracker
(`man seetracker` ja' documenta o programa), e os recursos numerados dos projetos (`.msxproj/.msxproject`). a detalhar: tabela de formatos com tamanho e cabecalho, geradores de codigo (BASIC `DATA/POKE`, `.inc` de Assembly, `BSAVE`).

#### 5.4.5 Ordem de incorporacao sugerida (sem compromisso)

1. As que **reaproveitam o que o fwMSX ja' tem**: editor hexadecimal de arquivo (como o `DM` do Mamute), visualizador Markdown/ASCII (5.3.6).
2. **Fontes e sprites** (TUI e GUI; preview no VDP).
3. **PSG e MML** (preview no nucleo de som; gerar `PLAY`/BASIC).
4. **Telas pixel a pixel** e Graphos III (GUI).
5. **SEE Tracker** (GUI; precisa do driver Z80, que o fwasm monta).
6. O resto (linker `.REL`, NestorBASIC/MSXBAS2ROM no projeto) conforme a necessidade do usuario.

#### 5.4.6 Riscos e perguntas em aberto

- Tamanho: os editores visuais do PaleoBasic somam ~20 mil linhas de PureBasic (diretorio `visual_editors`, com os dados de ajuda); porta-se o comportamento, nao o codigo, e os GUIs em Windows API/Scintilla nao tem equivalente direto no ImGui/FTXUI.
- Ferramentas de pixel na TUI sao limitadas pelo terminal; decidir quais valem o esforco.
- O que o usuario ainda USA de verdade dos dois projetos (para nao portar o que ninguem usa) e a ordem de prioridade dele.
- Se o PaleoBasic/msxIDE seguem em desenvolvimento em paralelo (risco de dois lados divergirem) ou se o fwMSX passa a ser o destino unico.
