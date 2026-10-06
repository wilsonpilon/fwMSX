# SRAM de cartucho (ASCII8/ASCII16) e arquivo .sav -- especificacao (documento vivo)

> Mesmo espirito de `doc/memory-map-spec.md` e `doc/scc-spec.md`: o que foi
> feito, as decisoes e o que falta, para retomar do ponto exato onde parou.

Estado: **concluido em 2026-10-06** (item 6 do SPEC, primeira parte: SRAM). Os
cartuchos ASCII8 e ASCII16 agora tem a memoria de bateria do fMSX: a SRAM e
selecionada pelos registradores de banco, grava, e e' salva num arquivo `.sav`
ao lado da ROM. Save-state completo e o GameMaster2 continua
pendentes (secao 5).

## 1. O que o cartucho faz

- **ASCII8**: a SRAM tem 8KB e ocupa um quarto da janela (uma pagina de 8KB). O
  registrador de banco do quarto escolhido recebe o bit `mask+1` (0x10 em uma
  ROM de 128KB): a pagina vira SRAM, gravavel.
- **ASCII16**: a SRAM tem 2KB, espelhada dentro da janela de 16KB (a mesma
  posicao aparece a cada 2KB). Selecionada pelo bit `mask+1` no registrador de
  banco da metade (6000h para a metade baixa, 7000h para a alta).
- Com a SRAM selecionada, os registradores de banco (6000h-7FFFh) continuam
  sendo registradores: escrever ali troca de banco e nao grava na SRAM, como no
  fMSX.

## 2. Como fica no motor (C, `src/memmap/core/slot_state.c`)

- Cada pedaco de 8KB tem um **tipo de escrita** (`chunk_writable`):
  - `MEMMAP_WRITE_NONE` (0): so' leitura (ROM);
  - `MEMMAP_WRITE_RAM` (1): RAM comum;
  - `MEMMAP_WRITE_SRAM` (2): SRAM de 8KB;
  - `MEMMAP_WRITE_SRAM_MIRROR` (3): SRAM de 2KB, repetida nos 8KB do pedaco.
- Uma escrita em SRAM marca `sram_dirty` da combinacao (slot). `ram_ptr()` do
  barramento (LDIR/LDDR) so' devolve ponteiro para RAM comum, para a escrita
  sempre passar por esse controle.
- O buffer da SRAM pertence ao `MemorySystem` (C++), sempre de 8KB.

## 3. Arquivo .sav (C++, `src/machine/machine.cpp`)

- O arquivo fica ao lado da ROM, com a extensao trocada por `.sav`
  (`jogo.rom` -> `jogo.sav`). Tamanho: 8192 bytes (ASCII8) ou 2048 bytes (ASCII16).
- Ao carregar o cartucho, se o `.sav` existir e tiver o tamanho certo, ele e' lido.
  Um arquivo de tamanho errado e' ignorado (a SRAM comeca zerada).
- A gravacao so' acontece quando houve escrita desde a ultima vez (`sram_dirty`).
  Quando acontece: a cada 300 quadros na janela; antes de trocar modelo ou
  cartucho; ao fechar a janela; e ao final de uma execucao sem janela (`--frames`).

## 4. Testes

`tests/z80/memmap_test.cpp`, secoes 19 e 20 (CTest `memmap_slots`, executavel `memmaptest`):

- ASCII8: a selecao de SRAM aparece zerada; escrever grava e marca como alterada;
  voltar a ROM mostra o banco certo; o conteudo sobrevive a trocar de banco;
  `SramImage` gera 8KB; uma segunda instancia recarrega a imagem (simulando o
  `.sav`) e mostra o mesmo conteudo; `LoadSram` recusa tamanho errado.
- ASCII16: a selecao de SRAM aparece zerada; a escrita em 4123h aparece espelhada
  em 4923h; `SramImage` gera 2KB; ao voltar a ROM, as verificacoes seguintes
  continuam valendo.

## 5. O que falta

- **GameMaster2 (`MAP_GMASTER2`)**: o mapper e' do cartucho utilitario GM2, usa um
  arquivo de ROM proprio e um esquema de bancos diferente. Nao foi portado.
- **FM-PAC (`MAP_FMPAC`)**: implementado, com a SRAM de 8KB pela chave 694Dh e o
  arquivo `FMPAC.sav` (ver `doc/fm-spec.md`, secao 4).
- **Save-state completo** (salvar a maquina inteira): fase separada.
- **Deteccao automatica de SRAM**: o cartucho e' SRAM quando o mapper e' ASCII8 ou
  ASCII16; nao ha' verificacao de cabecalho.
