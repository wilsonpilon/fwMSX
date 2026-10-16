# Montador fwasm (reimplementacao do asMSX) -- especificacao

Reimplementacao **clean-room** do asMSX 1.2.0 (BSD-3-Clause, escrita a partir do manual `resource/asmsx.md` e do
comportamento observado do `asmsx.exe` como caixa-preta; nao deriva do fonte GPLv3). Linguagens: C (codificador de
instrucoes Z80, `src/asm/core/z80_enc.*`), Fortran (funcoes matematicas das expressoes, `src/asm/fortran/asm_math.f90`),
C++ (lexer, expressoes, pre-processador, montador, CLI em `src/asm/cpp/`), Assembly (NASM dual-ABI,
`src/asm/asm/wav_synth.asm`: a sintese dos pulsos da fita para o `.wav`).

## Linha de comando e console (`src/asm/cpp/cli.*`)

`fwmsx --asm [-z] [-s] [-vv] [-r] [-o <caminho>] <arquivo.asm>`; no console (`--cli`) o comando `asm` repassa os
mesmos argumentos.

- Codigo de saida: o do montador (`AsmResult::rc`); 2 = uso incorreto ou arquivo ilegivel ou impossivel de gravar.
- Mensagens (erros/avisos) no stderr; sem `-s`, uma linha `gerado: <arquivo> (N bytes)` por saida no stdout.
- Nome-base das saidas: fonte sem extensao; `-o nome` troca (extensao ignorada); `-o pasta/` (terminado em `/` ou `\`)
  usa a pasta com o nome do fonte. Chave de saida sem ponto = extensao anexada ao nome-base.
- `.include`/`.incbin` sao lidos relativos a pasta do fonte (`-r` e' aceito e ignorado, como no asMSX atual).
- Decisao: o provedor de arquivos (`FileProvider`) isola o montador do disco, e o teste usa um provedor em memoria.

## Manual dentro do fwMSX (1.38.5; 1.39.0 acrescentou fwasm-formats e fwasm-megarom)

`man fwasm` (e `fwasm-syntax`, `fwasm-labels`, `fwasm-numbers`, `fwasm-operators`, `fwasm-data`, `fwasm-directives`, `fwasm-conditional`, `fwasm-macros`, `fwasm-bios`, `fwasm-biosvars`) e' o manual deste
montador, escrito em `src/man/cpp/man_pages_fwasm.cpp` a partir de `resource/asmsx.md`. Ao mudar uma diretiva, **atualizar `fwasm-directives`, `fwasm-formats`/`fwasm-megarom` e a lista em `fwasm`**
(o `mantest` confere que as paginas existem, nao o texto de cada diretiva). As tabelas de BIOS saem de `bios_tables.inc`.

## Estado (1.39.0): o fwasm e' completo em relacao ao asMSX

- **Passadas**: ate 4 passadas de simbolos (reaproveitando a tabela da anterior para referencias adiante) mais uma
  passada final que emite bytes e mensagens. Erro de instrucao com valor de simbolo ainda indefinido nao aborta a
  passada de simbolos (mede-se o tamanho com 0; o erro real sai na final).
- **Diretivas** (todas as do asMSX): `org`, `page`, `phase`/`dephase`, `zilog`, `bios`/`biosvars`, `db`/`defb`/`dt`/`deft`,
  `dw`/`defw`, `ds`/`defs`, `byte`, `word`, `incbin` (`skip`/`size`), `print*`, `filename`, `start`, `size`, `if`/`ifdef`/`ifndef`/
  `else`/`endif`, `include`, macros e repeticao, e -- desde a 1.39.0 -- `rom`, `megarom` (`subpage`, `select`), `basic`, `msxdos`,
  `search`, `callbios`, `calldos`, `cas`/`cassette`, `wav`, `sinclair`, `debug`, `break`/`breakpoint`.
- **Saidas**: `.z80` (cru), `.rom`, `.bin`, `.com`, `.cas`, `.wav`, `.tap`, `.sym`, `.txt`. `.filename "x"` troca o nome-base (`x.rom`...); o
  `-o` do CLI tem prioridade. O CLI so' fixa o nome-base quando ha' `-o` (antes fixava sempre e o `.filename` nunca valia).
- **Particularidades do asMSX reproduzidas** (todas conferidas contra o `asmsx.exe`): `org -1` grava um byte antes do 0; arquivo `.rom` com o tamanho
  arredondado a partir do ULTIMO endereco (8 KB + 1 byte perde o ultimo); `.start` depois do `.rom` e' ignorado no arquivo (mas com fita pedida vale
  e o `.rom` sai com ele); aviso da MegaROM "only N out of M ... defined" onde N e' quantas FALTAM; offsets negativos dentro de uma subpagina; o `.wav`
  fecha com numero par de amostras; os checksums do carregador BASIC do `.sinclair` saem errados (o do cabecalho ignora o nome e leva 11h a mais, o dos dados
  leva 11h a mais e o XOR do nome); o nome da fita e' fixado pelo primeiro `.cas`/`.wav`.
- **Nao reproduzido**: codigo no endereco 0000h antes de `.sinclair` (o asMSX gera um bloco de 32 KB); `.org` abaixo do inicio de uma subpagina que
  deixaria o indice do arquivo negativo (o asMSX trava com violacao de acesso; o fwasm da' "code/data without subpage"). O fwasm nao imprime as mensagens
  "Redefining output name..." do `-o`.

### Formatos de saida (detalhes dos achados)

- **ROM**: `.rom` grava 16 bytes (`AB`, INIT, 12 zeros) no endereco atual; INIT = `.start` dado antes, ou o fim do cabecalho (um segundo `.rom` repete o
  INIT do primeiro). Escrever em `>= 0C000h` com `.rom`: erro 29. Tamanho: `roundup8K(len-1)` ou `.size` KB (completa com zeros; `code size overflow`
  rc 18 se nao cabe; `memory overflow` rc 2 se `base + size*1024 > 64 KB`).
- **BASIC**: `FE` + inicio (menor endereco) + fim (maior, o enchimento do `.size` nao conta) + execucao (`.start` em qualquer ordem, senao o inicio).
- **MSX-DOS**: `.msxdos` poe o endereco em 0100h e tem de vir antes de qualquer codigo (rc 24); `.com` = so' os bytes. `.calldos` so' com `.msxdos` (rc 26).
- **MegaROM**: o arquivo e' `(maior subpagina + 1) * tamanho` (8 KB; 16 KB no ASCII16). Offset de um byte = `subpagina*tamanho + (endereco fisico - X do .subpage)`;
  `>= tamanho` = rc 32. `.select` gera `PUSH AF / LD A,n / LD (reg),A / POP AF`; `reg` por mapper: Konami `X & FFF8h` (4000h-4007h = erro 39), SCC `(X+1000h) & FFF8h`,
  ASCII8 `(6000h + ((X-4000h)>>2)) & FFFEh`, ASCII16 `6000h` se `X & FFF8h == 4000h` senao `7000h`. Erros: 33 (`.subpage 0`), 36 (fora de 4000h-0BFFFh ou depois de `.page`),
  37 (numero fora do mapper ou referencia adiante), 38 (subpagina repetida), 40 (`.select` de subpagina inexistente), 41 (usada sem `.megarom`).
- **Fita**: `.cas` = marca de 8 bytes `1F A6 DE BA CC 13 7D 74` + (BLOAD: `D0`x10 + nome de 6 letras + marca) + corpo (inicio, fim, execucao, bytes; ROM a partir de 8000h
  tambem entra assim). `.wav` = 44.100 Hz, 8 bits, mono: tom de 16.000 ciclos de 2400 Hz + bytes a 1200 baud + (BLOAD) tom de 4.000 ciclos entre a identificacao e
  os dados. Cada ciclo = meio-periodo baixo (0) + alto (255) cujas fronteiras caem na amostra ARREDONDADA do tempo acumulado (em 1/16 de amostra: 147 = meio periodo
  curto, 294 = longo): `wav_synth.asm`.
- **Sinclair**: `.tap` = blocos `[len][flag][dados][xor]`; bloco CODE com o nome (10 letras); com `.start` != 0 antecedido de um carregador BASIC de 30 bytes.

## Testes

`asmtest` repete os casos de `tests/asm/golden/*.golden`, gerados pelo `asmsx.exe` de verdade por `tools/asmdiff/` (oraculo): `instr` (4170), `expr` (1590), `misc` (279),
`lang` (185) e `formats` (382: ROM, MegaROM com os 4 mappers e dezenas de janelas, BASIC, MSX-DOS, `.size`, `.search`, `.callbios`/`.calldos`, `.debug`/`.break`, `.cas`, `.wav`,
`.sinclair` e todos os erros). Compara codigo de saida, mensagens e arquivos de saida; arquivos acima de 2048 bytes (os `.wav`) entram no golden so' como tamanho + CRC-32
(`@outcrc`). `tools/asmdiff/gen_formats.py` regenera `formats.golden` (precisa do `resource/asmsx.exe`).
