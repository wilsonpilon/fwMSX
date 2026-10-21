# Basic Dignified no fwMSX (`doc/badig-spec.md`)

Clone nosso, em C++/C/Fortran/NASM, do **Basic Dignified Suite** (Fred Rique, v2.2.1): le um programa em BASIC Dignified (`.dmx`) e gera o BASIC ASCII
classico (`.amx`) e o BASIC **tokenizado** do MSX (`.bmx`), alem da listagem do tokenizador (`.lmx`). Entrou na 1.42.0 ("Road Fighter").

O pedido do usuario: *"nao vamos usar o badig em python, vamos fazer um clone nosso nas nossas linguagens; quero o programa processando o DMX e gerando o
BMX tokenizado e processado"*, com a tela de configuracao do msxIDE (so' o que vale para o MSX e o fwMSX).

## 1. Decisoes (respondidas pelo usuario, 2026-10-20)

1. **Referencia = o `badig.py` original v2.2.1** (`resource/basic-dignified`), nao o `compiler.bas` do msxIDE (que e' uma reescrita por linhas, sem remtags,
   toggles, `true`/`false`, operadores compostos, loop labels completos...). O msxIDE serviu para a tela de configuracao e para o jeito de executar
   (disco com `AUTOEXEC.BAS`).
2. **Configuracao** = `badig.cfg` ao lado do executavel (os nomes do `badig.ini`/`badig_msx.ini`/`tokenizer_interface.ini`), editado pelo dialogo
   *Montar > Dignified setup...*; as remtags `##BB:` do proprio `.dmx` continuam mandando por arquivo. Quando existir o `.msxproject` a configuracao migra
   para ele.
3. **Executar** = disco temporario com o `.bmx` e um `AUTOEXEC.BAS` (`10 RUN "NOME.BMX"`), inserido em A: com a maquina reiniciada: o DISK BASIC roda sozinho.
   Precisa de uma maquina com interface de disquete (`--disk`).

## 2. Estrutura (`src/badig/`)

| Arquivo | Linguagem | O que faz |
| --- | --- | --- |
| `cpp/badig.h` | C++ | API publica: `Options`, `Diag`, `Result`, `Convert`, `ApplyArgs`, `LoadConfig/SaveConfig`, `RunBadigCommand`, `Tokenize` |
| `cpp/badig_core.h`, `badig_base.cpp` | C++ | Token, Position, Logger, utilitarios com a semantica do Python, e `Description::Classify` (o `commands.match` do original) |
| `cpp/badig_lex.cpp` | C++ | o Lexer (prefixo que cresce enquanto algum grupo casa) e o `tok_str` |
| `cpp/badig_parse.cpp` | C++ | o Parser em 5 passadas, o modulo MSX (variaveis, `?`/PRINT, THEN/GOTO, compostos, traducao) e a geracao |
| `cpp/badig_msxtok.cpp` | C++ | o tokenizador (`msxbatoken.py`): linhas, numeros, desvios, DATA/REM/CALL, listagem |
| `cpp/badig_run.cpp` | C++ | o pipeline, os argumentos, as remtags, o INI, os relatorios e `fwmsx --badig` |
| `c/badig_tok.c` | **C** | a tabela de palavras do tokenizador (a ordem do original) e a busca |
| `fortran/badig_num.f90` | **Fortran** | `bd_round_digits`: corte e arredondamento dos digitos das constantes reais (BCD simples/dupla) |
| `asm/badig_str.asm` | **NASM** | `bd_prefix_ci`: prefixo sem diferenciar caixa (o laco quente do tokenizador), ABI dupla Win64/SysV |

Regra do projeto das 4 linguagens: cumprida pelo nucleo, nao so' por enfeite.

## 3. Fidelidade: o clone sai IGUAL ao original

O objetivo e' **byte a byte**, inclusive nos detalhes que parecem acidentais: o identificador `PROGRAM` que sobra no fim vira a variavel `zz` (por isso as
variaveis de nome longo comecam em `zy`); os nomes curtos sao alocados de `zz` para `aa`; `Position.copy()` soma 1 a coluna; o intervalo `[a-zA-z]`
do original aceita `[ \ ] ^ _` ; o `ELSE` vira `:` + token; etc. Os tokens sao **compartilhados** entre as listas (como os objetos do Python).

Como se garante: `tools/badig_compare/compare.py` roda o `badig.py` real e o clone (`badigcli`) sobre o mesmo fonte e compara o `.amx`, o `.bmx`, o `.lmx`
(sem a linha de data) e os relatorios `-lex -par -lnr -var -lbr`; para os erros compara a mensagem (com a linha do codigo e o `^`). Com `--golden` grava
os arquivos de referencia em `tests/badig/golden/`, que o `ctest badig` confere sem precisar do Python.

Cobertura dos testes (`tests/badig/data`): defines (com argumentos, aninhados, o `[?]` embutido), variaveis (declare, `~`, conflitos), funcoes (argumentos
padrao, retornos), linhas (`:` e `_`), comentarios exclusivos e blocos, toggles (`keep`, blocos), `true`/`false` e compostos, labels e loop labels, include
(com namespaces separados), `-tr` (Unicode -> MSX), as opcoes `-ss -ca -ls -lp -rh -cp -tg -tl`, os numeros do tokenizador (inteiros, `&H &O &B`, reais
simples e duplos, expoentes), desvios (`GOTO a,b,,c`, `THEN n ELSE n`), DATA, `CALL`/`_`, **o exemplo real Change Graph Kit (CGK173, 30 KB)** e **33 casos de
erro** (mensagens identicas as do original). 376 verificacoes em `badigtest`.

## 4. Diferencas em relacao ao original (todas deliberadas)

- `tokenize` e' **ligado** por padrao (no original e' `--tk_tokenize`); `--no_tokenize` desliga.
- A saida do `.amx` e' sempre **CRLF** (o original usa o fim de linha do sistema).
- So' o sistema **MSX** (`-id msx`); CoCo, `-id coco` e as opcoes do openMSX (`--em_*`) sao aceitas e ignoradas ou recusadas com mensagem.
- `exit` fora de um loop label: o original quebra com `IndexError`; o clone da' o erro `Exit without a loop label.`.
- Colunas das mensagens com `-tr` (fonte UTF-8): contam **caracteres**, como o original.
- Vem em memoria: `Convert()` aceita o **texto do buffer** do editor (sem gravar) e devolve os arquivos, as mensagens com arquivo/linha/coluna e o log.

## 5. Uso

- `fwmsx --badig [opcoes] <fonte.dmx> [saida.amx]` (e o comando `badig` do console); `man badig`.
- Editor (TUI e janela): `F9` converte o texto da aba, `F8`/`Shift-F8` andam pelas mensagens (o erro de um `.include` abre a aba dele), `F5` converte e **executa**
  (comando `badigrun`), *Montar > Dignified setup...* abre a configuracao (2 paginas). O Build escolhe o que fazer pela linguagem da aba: `.dmx` -> Basic
  Dignified, `.asm` -> fwasm.
- `badigrun <fonte.dmx>` no console/ponte de controle: converte, monta o disco, insere em A: e reinicia. Testado de ponta a ponta no `controltest`
  (o `.bmx` roda no DISK BASIC da maquina emulada e imprime o resultado).

## 6. Em aberto

- **Basic DignifieR** (classico -> Dignified, `msxbader.py`) e o **tokenizador inverso** (`.bmx` -> texto): nao portados.
- Opcoes do msxIDE que nao vieram: NBasic (garantir `include "nbasic.dmx"` e a rotina de carga do `nbasic.bin`), autocompletar do msxIDE, e a
  copia de **arquivos de apoio** (`.bin`, dados) da pasta do fonte para o disco de execucao.
- Salto automatico do erro de execucao: ler a mensagem da tela do MSX depois do F5 (hoje o usuario digita a mensagem em *Montar > BASIC line...*, ver secao 8).
- Integracao com o projeto `.msxproject` (E6, parte 2): fontes, configuracao e saidas dentro do projeto.

## 7. NestorBASIC e arquivos de apoio (1.42.1)

- **Biblioteca** (`src/badig/lib/`, copiada para `badig/` ao lado do executavel; `badig::SetLibraryDir`): `nbasic.dmx` (as rotinas do NestorBASIC como `func .NB_*`, do msxIDE) e `NBASIC.BIN`.
- **Include da biblioteca**: no original as funcoes de um `include` nao valem no principal (namespaces separados). Um `include "x"` que NAO existe na pasta do fonte mas existe na biblioteca e' emendado como TOKENS antes do parser, no namespace do programa (`SpliceLibraryIncludes`); depois a **poda** (`PruneLibrary`) tira os comentarios da biblioteca e as funcoes nao chamadas (fecho transitivo). Includes de arquivos do usuario ficam como o original.
- **Fluxo**: as funcoes sao alvos de `GOSUB` e o codigo cai onde o include esta': ele vai depois do `END` (o snippet `nbload` do editor faz assim).
- **Disco de execucao**: `badig::CollectDiskFiles` junta os `disk_files` (nomes/mascaras da pasta do fonte, 8.3 estrito, em maiusculas) e o `NBASIC.BIN` (com `nbasic`, ou se o fonte inclui `nbasic.dmx`; procura na pasta do fonte e em `badig/`). Opcoes: `##BB:disk_files=`, `##BB:nbasic=true`, `--disk_files`, `--nbasic`, `badig.cfg` secao `[FWMSX]`, dialogo pagina 2.
- Testes: `badigtest` (biblioteca, poda, mascaras, nome invalido, arquivo inexistente), `controltest` (um programa BASIC le o arquivo de apoio no disco de execucao, no emulador).

## 8. Erros de execucao -> linha do fonte (1.42.2)

- `Result::line_map` (`LineMapEntry{basic, file, line}`) vem de qualquer conversao, sem `-lnr`: e' a tabela de linhas do gerador (a primeira entrada, "antes do inicio", fica de fora, como no relatorio). `line = 0` = linha gerada pelo Badig.
- `ParseBasicErrorLine("Syntax error in 130")` = 130: o numero depois de " in " (ou o ultimo numero do texto); 0 se nao ha' ou passa de 65529. `MapBasicLine(result, 130, entry)` acha o arquivo e a linha.
- Editor: *Montar > BASIC line...* (`Cmd::BasicLine`, `Dialog::BasicLine`) usa o mapa da ultima conversao Dignified do `EditorApp` e passa pelo mesmo `BuildGoto` das mensagens (abre a aba do `.include` na janela grafica).
