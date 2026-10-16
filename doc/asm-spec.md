# Montador fwasm (reimplementacao do asMSX) -- especificacao

Reimplementacao **clean-room** do asMSX 1.2.0 (BSD-3-Clause, escrita a partir do manual `resource/asmsx.md` e do
comportamento observado do `asmsx.exe` como caixa-preta; nao deriva do fonte GPLv3). Linguagens: C (codificador de
instrucoes Z80, `src/asm/core/z80_enc.*`), Fortran (funcoes matematicas das expressoes, `src/asm/fortran/asm_math.f90`),
C++ (lexer, expressoes, pre-processador, montador, CLI em `src/asm/cpp/`), Assembly (previsto: sintese de
pulsos/WAV/CAS).

## Linha de comando e console (`src/asm/cpp/cli.*`)

`fwmsx --asm [-z] [-s] [-vv] [-r] [-o <caminho>] <arquivo.asm>`; no console (`--cli`) o comando `asm` repassa os
mesmos argumentos.

- Codigo de saida: o do montador (`AsmResult::rc`); 2 = uso incorreto ou arquivo ilegivel ou impossivel de gravar.
- Mensagens (erros/avisos) no stderr; sem `-s`, uma linha `gerado: <arquivo> (N bytes)` por saida no stdout.
- Nome-base das saidas: fonte sem extensao; `-o nome` troca (extensao ignorada); `-o pasta/` (terminado em `/` ou `\`)
  usa a pasta com o nome do fonte. Chave de saida sem ponto = extensao anexada ao nome-base.
- `.include`/`.incbin` sao lidos relativos a pasta do fonte (`-r` e' aceito e ignorado, como no asMSX atual).
- Decisao: o provedor de arquivos (`FileProvider`) isola o montador do disco, e o teste usa um provedor em memoria.

## Manual dentro do fwMSX (1.38.5)

`man fwasm` (e `fwasm-syntax`, `fwasm-labels`, `fwasm-numbers`, `fwasm-operators`, `fwasm-data`, `fwasm-directives`, `fwasm-conditional`, `fwasm-macros`, `fwasm-bios`, `fwasm-biosvars`) e' o manual deste
montador, escrito em `src/man/cpp/man_pages_fwasm.cpp` a partir de `resource/asmsx.md`. Ao implementar uma diretiva que falta, **atualizar a tabela de estado em `fwasm-directives` e a lista em `fwasm`**
(o `mantest` confere que as paginas existem, nao o estado de cada diretiva). As tabelas de BIOS saem de `bios_tables.inc`.

## Testes

`asmtest` repete os casos de `tests/asm/golden/*.golden`, gerados pelo `asmsx.exe` de verdade por
`tools/asmdiff/` (oraculo). Compara codigo de saida, mensagens e arquivos de saida.

## Estado (1.36.0)

- **Passadas**: ate 4 passadas de simbolos (reaproveitando a tabela da anterior para referencias adiante) mais uma
  passada final que emite bytes e mensagens. Erro de instrucao com valor de simbolo ainda indefinido nao aborta a
  passada de simbolos (mede-se o tamanho com 0; o erro real sai na final).
- **Diretivas prontas**: `org`, `page`, `phase`/`dephase`, `zilog`, `bios`/`biosvars`, `db`/`defb`/`dt`, `dw`/`defw`,
  `ds`/`defs`, `byte`, `word`, `incbin` (`skip`/`size`), `print`/`printdec`/`printhex`/`printfix`/`printtext`/
  `printstring`, `filename`, `start`, `if`/`ifdef`/`ifndef`/`else`/`endif`, `include`, macros e repeticao.
- **Saidas**: `.z80` (do menor ao maior endereco gravado; `ds` antes do primeiro byte conta), `.sym`, `.txt`.
  `.filename "x"` troca o nome-base (`x.z80`, `x.sym`...); o `-o` do CLI tem prioridade.
- **Particularidades do asMSX reproduzidas**: `org -1` grava um byte antes do 0 e a saida o inclui (abaixo de -1 o
  original le/grava lixo de heap; aqui e' "memory overflow"); `.start` so' valida a expressao por enquanto.
- **Falta**: formatos `rom`/`megarom`/`basic`/`msxdos`/`cas`/`wav`/`sinclair`, `select`/`subpage`/`search`,
  `callbios`/`calldos`, `debug`/`break`/`breakpoint`, e o modulo em Assembly (NASM) -- por isso a regra "4 linguagens
  por fase" ainda nao esta' cumprida neste subsistema (so' C, Fortran e C++).
