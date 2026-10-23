# HELP e MAN -- ajuda e manual dentro do fwMSX (1.38.0, "Road Fighter")

`help` mostra os comandos de forma organizada; `help <comando>` da' o uso e um resumo; `man <topico>` abre o manual
completo (com exemplos) -- dos comandos e da REFERENCIA do MSX (BIOS, variaveis do sistema, ganchos, BASIC, cores,
portas, slots...). Vale no console (`--cli`) e na TUI (`--tui`).

## 1. Uso

```
fwmsx> help                  # a tabela de comandos, agrupada por categoria, com separadores
fwmsx> help peek             # peek <end> [n]  /  le n bytes da memoria  /  man peek = manual completo
fwmsx> man                   # o indice de TODOS os topicos
fwmsx> man type              # como digitar teclas ({enter}, {ctrl}+{c}...)
fwmsx> man bios              # a tabela de entradas da BIOS (CHPUT, CHGET, WRTVDP...)
fwmsx> man sysv              # nome parcial: "nao ha' topico 'sysv' -- parecidos: sysvars"
```

- **Console**: o `man` e' paginado pela altura do terminal -- espaco = proxima pagina, Enter = proxima linha, `q`/Esc sai. A
  largura segue a do terminal (ate' 100 colunas). Se a saida for redirecionada, imprime tudo de uma vez.
- **TUI**: o `man` abre uma janelinha. Setas rolam, PgUp/PgDn (ou espaco/`b`) rolam uma pagina, Home/End (ou `g`/`G`) vao ao
  topo/fim, Esc ou `q` fecham. O titulo mostra a posicao (`1-18/47`). O texto e' quebrado na largura da caixa.
- Os nomes nao diferenciam maiusculas e aceitam apelidos (`?` = help, `quit` = exit, `ganchos` = hooks, `colors` = cores...).

## 1b. O manual do MSX-BASIC (1.38.1)

`man <palavra>` mostra a pagina de qualquer palavra reservada do MSX-BASIC: **TIPO** (instrucao/funcao, uso avancado), **FORMATO**, **EXEMPLO**,
**DESCRICAO**, **PROGRAMA EXEMPLO** e a **FONTE** (livro e pagina). Vale o mesmo para `help <palavra>` (formato + resumo). Os nomes aceitam varias palavras
(`man put sprite`), apelidos dos verbetes compostos (`man for`, `man next`, `man err`, `man cload`) e prefixo unico (`man interval`). `screen`, `color`,
`width`, `circle`, `play`... trazem o verbete do MSX1 e, abaixo, o do MSX2+. `stop` e `wait` sao tambem comandos do fwMSX: a pagina mostra o comando e,
depois, "NO MSX-BASIC". Tambem entram 23 topicos do manual, por exemplo `apendice-c-codigos-de-erro`, `apendice-d-mapa-da-memoria`,
`apendice-a-caracteres-ascii`, `operadores`, `variaveis`, `interrupcoes` e os do FM-Music.

### 1c. BIOS, BDOS, variaveis de RAM e hardware (1.38.2)

`tools/paleobasic/import_bios_help.py` importa `BiosCallsHelpData.pbi`, `BiosDocHelpData.pbi` e `HardwareHelpData.pbi` (o paleobasic as gerou do MSXBIOS.CHM do RuMSX, Lex Lechz).
Categorias: **BIOS-ROM** (MainROM, SubROM, Disk-ROM, Hangul-ROM, MSX-JE, EXTBIO), **BDOS**, **Variaveis RAM**, **Doc BIOS** e **Hardware** (RTC, PSG, SCC, OPLL, MSX-Audio,
MoonSound, V9958, V9990, teclado, portas, Kanji, PCM, MIDI, SCSI, caneta optica, fita). O texto e' em INGLES, como no original. Uma rotina que existe em areas
diferentes (WRTVDP na MainROM e na SubROM) vira UMA pagina com uma secao `AREA:` por area; um nome que ja' existe em outra categoria (BEEP no BASIC e na BIOS) vira uma
secao `==== categoria ====` da pagina existente. `man -l` lista as categorias, `man -l <categoria>` os nomes, `man -k <texto>` procura em nomes, apelidos e resumos.
Os topicos de hardware tem apelidos (`hw-rtc`/`rtc`, `hw-psg`, `hw-opll`/`opll`, `hw-v9990`/`v9990`, `hw-ports`...). Muitas variaveis de RAM trazem so' o endereco e o nome: e'
o que a fonte tem; as tabelas `man bios` e `man sysvars` (escritas a mao e conferidas contra as ROMs) trazem os registradores de cada rotina.

### 1d. Livros: Red Book, MSX2 Technical Handbook e Basic Dignified (1.38.3)

`tools/paleobasic/import_books_help.py` importa `RedBookHelpData.pbi` (*The MSX Red Book*, Avalon Software / Kuma Computers, 1985; versao Markdown de Gustavo Seidler),
`Th2HandbookHelpData.pbi` (*MSX2 Technical Handbook*, ASCII Corporation, 1987; edicao Markdown de Konamiman) e `BasicDignifiedHelpData.pbi` (ajuda do Basic Dignified, portugues,
escrita para o paleobasic). Nomes: `rb-`, `th2-`, `bd-` + o titulo; categorias "Red Book: <capitulo>", "Handbook: <capitulo>", "Basic Dignified: <grupo>". Os links internos viram
"VER TAMBEM"; as figuras nao existem no terminal (`[Ver Figura: CH01F01]`). Como sao milhares de topicos, o indice mostra so' uma linha por livro; navegue com `man -l red`,
`man -l handbook`, `man -k <texto>`. O texto ocupa ~2,3 MB no executavel.

### 1e. MSX-DOS 2, MSX-DOS 1, Z80, R800, FM-PAC e extensoes do BASIC (1.38.4)

`tools/paleobasic/import_manuals_help.py` importa `MsxManualsHelpData.pbi` (MANUALS.CHM do RuMSX) e `MsxSoftwareHelpData.pbi` (SOFTWARE.CHM). Do primeiro, o MSX-DOS 2 vem **partido**: `dos2-<comando>`
(40), `dos2-fn-XXh` (92 chamadas; apelidos `_simbolo` e `dos2-fn-<nome>`), os capitulos e a interface de programa; mais `z80-opcodes`, `z80-mnemonics`, `z80-overview`, `r800-instructions`, `r800-msx`,
`fmpac-manual` e `turbo-basic-compiler`. Do segundo: as extensoes `CALL ...`, Disk-BASIC, MSX2/2+/Turbo-R, `operators`, `variables`, `using`..., as referencias `ref-*` (cores, erros, modos de tela, vozes do
MSX-Music...), `dos-*` (MSX-DOS 1) e `cpm`. Palavras que ja' tinham pagina (livro da Gradiente) ganham a versao em ingles como secao `==== MSX-BASIC ====` da mesma pagina (a fusao considera apelidos da
mesma categoria). **Nao importado**: OpenMSX (o fwMSX nao usa) e os 8 capitulos do MSX2 Technical Handbook do MANUALS.CHM (o livro inteiro ja' vem de `import_books_help.py`).

### 1g. O manual do fwasm na 1.39.0

Com os formatos implementados, `man fwasm-directives` nao tem mais nada "FALTA", e ganhou duas paginas: `fwasm-formats` (`.rom .basic .msxdos .size .search .cas .wav .sinclair`) e `fwasm-megarom`
(`.megarom .subpage .select` e os registradores de cada mapper). Escritas a partir do que o `asmsx.exe` de verdade faz (os achados estao em `doc/asm-spec.md`).

### 1f. NestorBASIC, SEE Tracker, MSXBAS2ROM e o manual do fwasm (1.38.5)

`tools/paleobasic/import_tools_help.py` importa as ajudas do paleobasic de **NestorBASIC** (`nestorbasic`, `nb-<n>`; apelido pelo wrapper `.NB_*`), **SEE Tracker** (`seetracker`, `see-<titulo>`) e **MSXBAS2ROM**
(`msxbas2rom`, `msxbas2rom-<arquivo>`; o "MSX Turbo Basic" do menu do paleobasic). O manual do **fwasm** e' escrito a mao (`man_pages_fwasm.cpp`), em portugues, a partir do manual do asMSX, com o ESTADO real
de cada diretiva; as tabelas de BIOS vem de `src/asm/cpp/bios_tables.inc`. Nao importados: o manual do asMSX do paleobasic (substituido pelo do fwasm), N80/LK80/LIBSTOR (por enquanto) e o OpenMSX.

### 1h. O Mamute (1.40.0; `mamute-edit` na 1.40.1; `mamute-screens` na 1.40.2; `mamute-superx` na 1.40.3; `mamute-fita` na 1.40.4)

`man mamute` (alias `mon`) e as paginas `mamute-modes`, `mamute-memory`, `mamute-dump`, `mamute-exec`, `mamute-edit` (1.40.1), `mamute-screens` (1.40.2), `mamute-superx` (1.40.3: os comandos X, `CL` e as variaveis `@`), `mamute-fita` (1.40.4: `TAPE`, `R`, `TLOAD`, `TSAVE`, fita `.cas/.tsx/.wav`), `mamute-files` e `mamute-config` (categoria "Mamute", escritas a mao em
`src/man/cpp/man_pages_mamute.cpp`). So' `mamute` entra na tabela do `help` (uso `mon [linha]`). Ao mudar um comando do monitor, atualizar a pagina do grupo
(o `mantest` confere que as paginas existem e alguns trechos, nao o texto de cada comando). Projeto: [mamute-spec.md](mamute-spec.md).

### Origem e direitos
O material do MSX-BASIC e' de duas obras de terceiros (a de BIOS/hardware, acima, e' do MSXBIOS.CHM do RuMSX, que reune fontes publicas: MSX Assembly Page, manuais do blueMSX e
do openMSX, livros da Aleph). O responsavel pela Aleph autorizou expressamente a distribuicao de material de MSX e ZX81 (informado pelo autor do projeto). Texto transcrito no
projeto **paleobasic** (`third-party/paleobasic/src/editor/help/MsxBasic*Data.pbi`) de duas obras: *Linguagem BASIC MSX*
(Denise Santoro Cruz, Editora Aleph/Gradiente, 4a edicao, 1986) e *Manual MSX 2+ FM* (Ademir Carchano / Flavio Monaco, ACVS Eletronica). Cada pagina
cita a fonte e a pagina. A importacao e' feita UMA vez por `tools/paleobasic/import_basic_help.py`, que gera `src/man/cpp/man_pages_basic.cpp`
(arquivo gerado: nao editar a mao; corrigir no paleobasic e rodar o script de novo). Para distribuir sem esse texto basta tirar o arquivo de
`MAN_LIB_SOURCES` no CMake: o resto do man continua funcionando.

## 2. Os topicos

**Comandos** (categoria no `help`): Console (`help man exit clear emu wait`), Emulador (`version status reset pause resume
step type peek poke regs quit`), Midia e estado (`cart disk eject tape state screenshot`), Depurador (`break unbreak breaks
watch unwatch watches stepi next until go stop jump dis mem stack reg slots events dbgui`), Ferramentas (`newdisk asm romdb
cas fitadb msxdisk`).

**Referencia MSX** (so' no `man`):

| Topico | Conteudo |
|---|---|
| `bios` | tabela de saltos da BIOS (0000h-0189h, e as do MSX2) com os registradores de cada rotina e um exemplo |
| `sysvars` | variaveis do sistema (F3AEh-FD09h): cores, cursor, tabelas de VRAM, JIFFY, SCRMOD, EXPTBL... |
| `hooks` | os ganchos (FD9Ah-FFC9h), como encadear e um exemplo (rotina a 60 Hz no H.TIMI) |
| `basic` | resumo dos comandos do MSX-BASIC, por assunto |
| `cores` | os 16 codigos de cor |
| `screens` | os modos de tela (SCREEN 0 a 12) |
| `vdpregs` | registradores do VDP (TMS9918, V9938, V9958) e o status |
| `psgregs` | registradores do PSG AY-3-8910 |
| `portas` | portas de E/S |
| `slotsmsx` | slots, subslots e o formato do ENASLT |
| `teclado` | a matriz do teclado (gerada das tabelas do proprio emulador) |

## 3. Arquitetura (as 4 linguagens)

| Camada | Arquivo | Faz |
|---|---|---|
| C++ | `src/man/cpp/man.{h,cpp}` | registro de topicos (`Topic`), `Find`, `Search`, `HelpTable`, `HelpFor`, `Index`, `Render`, `Wrap` |
| C++ | `man_pages_cmd.cpp`, `man_pages_tools.cpp` | as paginas dos comandos (texto sem acento) |
| C++ | `man_pages_msx.cpp`, `man_pages_msx_data.cpp` | as paginas de referencia; BIOS, variaveis e ganchos saem de TABELAS (`MsxEntry`) |
| C | `src/man/c/man_wrap.c` | `man_wrap_break`: onde quebrar uma linha longa (ultimo espaco que cabe) |
| Fortran | `src/man/fortran/man_match.f90` | `man_match_score`: casamento de nome (igual 3, comeco 2, trecho 1) -- usado pelo `Search` e pelo "parecidos" |
| Assembly | `src/man/asm/man_fold.asm` | `man_fold`: dobra A-Z em a-z sem desvio (NASM, dual-ABI Win64/SysV); o Fortran e o C++ a usam |

Integracao: `ReplSession` (console e TUI) trata `help [x]` e `man [x]`; `SetViewer()` recebe `(titulo, texto)` das paginas do man --
o console passa o pager (`Page()` em `repl.cpp`) e a TUI abre `Mode::Man` (`TuiModel::OpenMan`, `ManDialog`). `SetWidth()` ajusta a
largura (terminal, ou a da caixa da TUI). O man nao fala com o emulador: tudo vive no processo do console.

A formatacao de uma linha de tabela mais larga que a tela mantem as colunas: a continuacao fica alinhada na ultima coluna
(depois de uma folga de 3+ espacos antes da coluna 40); texto corrido so' mantem a indentacao.

## 4. Testes (`mantest`, `repltest`, `tuitest`)

- Camadas: `man_fold` (so' A-Z), `man_match_score` (3/2/1/0), `man_wrap_break`, `Wrap` (indentacao).
- Todo comando tem uso, resumo e pagina; a tabela do help lista todos; as paginas sao ASCII puro, sem tabulacao, ate' 78 colunas;
  os "VER TAMBEM" apontam para topicos que existem.
- **As tabelas de referencia sao conferidas contra a BIOS do repositorio** (e nao escritas de cabeca): cada entrada documentada da BIOS e'
  um `JP` (C3h) em `MSX.ROM` (as do MSX2 em `MSX2.ROM`, as do MSX2+ em `MSX2P.ROM`) -- 104 entradas; 110 dos 111 ganchos sao
  chamados (CALL/JP) em alguma ROM (BIOS, BASIC ou DISK.ROM; `H.DEVN` nao e' chamado por nenhuma delas); 11 ganchos essenciais
  (H.KEYI, H.TIMI, H.READ, H.MAIN, H.ERRO...) sao exigidos. Depois de bootar a maquina emulada: SCRMOD, FORCLR/BAKCLR/BDRCLR (15/4/4),
  RG7SAV (F4h), CRTCNT (24), TXTTAB (8001h), HIMEM, TXTNAM/TXTCGP, CSRX/CSRY, FNKSTR ("color") e o JIFFY contando 1 por quadro.
- `repltest`: `help`, `help <x>`, `man <x>` com e sem visualizador, nome parcial. `tuitest`: abrir, rolar, paginar, fechar a janelinha.

## 5. O que falta

- **Nao importado do paleobasic** (por escolha): `OpenMsxHelpData.pbi` (o projeto nao usa o openMSX). Tudo o mais da pasta `help/` ja' esta' no man.
- Mais topicos de referencia: comandos da SUB-ROM do MSX2, tabela de caracteres/ASCII do MSX, codigos de erro do BASIC, MSX-DOS 1/2
  (chamadas do BDOS), MSX-MUSIC (OPLL), SCC, os comandos do VDP em detalhe, mappers de cartucho, mapa de memoria da VRAM por modo.
- O modo janela (`--term`, F11) ainda tem a ajuda antiga; o `man` la' e' o proximo passo.
- Completar o TAB do console com os nomes dos topicos (`man <TAB>`); busca por assunto (`man -k cor`).
- Exemplos executaveis ("copiar e colar" para o `type`/`asm`).

- 1.40.5: paginas `asmrun`, `sym`, `src` e `markers` (categoria Depurador) e o `break` com rotulos e arquivo:linha; `mantest` confere.
