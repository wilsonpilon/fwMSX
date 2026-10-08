# MSX2+ (V9958, YJK/YAE, scroll, MSK) -- especificacao (documento vivo)

> Mesmo espirito de `doc/msx2-spec.md` e `doc/scc-spec.md`: o que foi feito,
> as decisoes e o que falta, para retomar do ponto exato onde parou.

Estado: **concluido em 2026-10-05** (v1.15.0). `fwmsx --msx --msx2p` roda a
BIOS MSX2+ real (`MSX2P.ROM` + `MSX2PEXT.ROM`, MSX BASIC 3.0) com o VDP
**V9958**: modos **SCREEN 10, 11 e 12** (YJK/YAE), **scroll horizontal** de 9
bits (R#26/R#27) e **mascara da esquerda** (R#25 bit 1) nos modos graficos de
5 a 8 e nos modos YJK/YAE.

Nao existe mais pendencia do V9958 em relacao ao MSX2+ padrao. O SCREEN 9 que
aparece nas tabelas do fMSX **nao e' do MSX2+**: segundo o proprio material de
ajuda do msxide (`resource/fmsxgo/third-party/msxide/ajuda`), ele existe so' no
MSX2 coreano. O TEXT 2 do V9958 (80 colunas) e' o SCREEN 0 de 80 colunas, que o
projeto ja tinha desde a v1.11.0.

## 1. Uso

```
fwmsx --msx --msx2p                          # MSX2+: BIOS MSX2P.ROM, sub-ROM MSX2PEXT.ROM
fwmsx --msx --msx2p --frames N --shot tela.ppm --keys "10 screen 12|20 line (10,10)-(200,100),5,bf|30 goto 30|run|" --wait 600
```

`--msx2p` escolhe o modelo MSX2+: a mesma maquina do `--msx2` (RAM de 128KB
com mapper, RTC, sub-ROM no slot 3:1), com o VDP trocado pelo V9958 e a BIOS
`MSX2P.ROM`. `--bios` e `--ext` continuam valendo para escolher outros arquivos.

## 2. O que o V9958 acrescenta ao V9938

| Recurso | V9938 (MSX2) | V9958 (MSX2+) |
|---|---|---|
| S#1 bit 2 (ID do VDP) | 0 | 1 (a BIOS MSX2+ detecta o chip por ele) |
| R#25 bit 3 (YJK) e bit 4 (YAE) | ignorados | SCREEN 10/11/12 em scr 7/8 |
| R#25 bit 0 (HScroll512) | ignorado | linha de 256 pixels da a volta em 512 |
| R#25 bit 1 (MSK) | ignorado | 8 primeiros pixels (16 em modo de 512) na cor de fundo |
| R#26/R#27 (scroll de 9 bits) | ignorados | scroll dos modos 5-8 e de YJK/YAE |

Nada do motor de comandos, dos sprites ou da VRAM muda: o V9958 aqui e' o
V9938 mais os registradores e os modos acima. Modos de caractere (0-4) nao
tem scroll, como no hardware.

## 3. Decisoes

- **O modo YJK nao ganha numero proprio.** Como no fMSX, `scr_mode` continua
  7 ou 8, e o renderizador troca quando R#25 bit 3 esta ligado. As tabelas de
  endereco e as mascaras sao as de SCREEN 7/8 (o `SetScreen()` do fMSX).
- **YAE (R#25 bit 4) escolhe a variante:** com ele, SCREEN 10 (e 11, que o fMSX
  tambem trata como YAE); sem ele, SCREEN 12. A diferenca de atributo entre 10
  e 11 nao e' modelada, como no fMSX.
- **Pixel YAE:** se o bit 3 do byte esta ligado, o pixel usa a paleta de
  16 cores (`byte >> 4`); senao, YJK. Igual ao fMSX e ao openMSX.
- **Scroll pixel a pixel.** Cada pixel da tela le a VRAM em `x + scroll`. Os
  grupos de crominancia (4 bytes) ficam alinhados na VRAM, como no openMSX.
  Isso difere do fMSX, que bloqueia os quatro primeiros pixels com a cor de
  fundo e alinha o scroll a 4 pixels: o fMSX aproximava o scroll fino.
- **YJK de 5 bits por canal.** A cor e' `(Y+J, Y+K, (5Y-2J-K+2)/4)` com cada
  canal limitado a 0..31 e expandido para 8 bits por replicacao
  (`x<<3 | x>>2`). O `+2` no azul e' a regra confirmada num turbo R real
  (nota do openMSX). O fMSX usa o indice 3-3-2 de SCREEN 8 e nao tem esse
  arredondamento, entao suas cores de YJK sao mais grosseiras.
- **Scroll de 5-8 com scroll zero** reproduz o renderizador do V9938 pixel a
  pixel: isso e' um teste automatico (secao 4).
- **MSK** pinta a faixa da esquerda com a cor de fundo (R#7), igual ao
  openMSX e ao `fmsxgo`. Nos modos de 512 pixels, a faixa tem 16 pixels.
- **Modelo separado (`VDP_MODEL_MSX2P`)**, com `VDP_MODEL_IS_V9938()` para o
  que e' comum aos dois chips (comandos, VRAM, status, altura de 212 linhas).

## 4. Testes

`tests/z80/vdp2_test.cpp`, secao 9 (CTest `msx2_machine`, executavel `vdp2test`):

- bit 2 de S#1 e VRAM de 128KB;
- YJK (16,0,0) -> (132,132,165) em todos os pixels;
- scroll de 8 e de 1 pixel, lendo a VRAM em `x + scroll` (Y=20 -> (165,165,206));
- MSK: 8 primeiros pixels na cor de fundo, o 9o normal;
- HScroll512: a linha continua lendo a VRAM depois de cruzar 256 pixels;
- YAE: Y impar usa a paleta `Y>>1`;
- SCREEN 6 com 512 pixels: scroll de 1 pixel anda a linha;
- **diferencial**: SCREEN 5/6/7/8 no V9958 sem scroll == V9938, em VRAM aleatoria;
- controle: a V9938 ignora R#25 (continua SCREEN 8 normal).

**Verificacao com BASIC real** (fora do CTest): a BIOS MSX2+ sobe com
`MSX BASIC version 3.0`. `10 screen 12 / 20 line (...),5,bf / 30 goto 30 /
run` escreve R#25 = `08h` e desenha em YJK; `screen 10` escreve R#25 = `18h`.
Na MSX2 comum o mesmo programa nao escreve R#25 nem R#0 para o SCREEN 12: o
BASIC nao aceita o modo.

## 5. Fontes e licencas

- `RefreshLine10()`/`RefreshLine12()`/`YJKColor()` (`resource/fMSX/fMSX/Common.h`):
  base do renderizador YJK/YAE. As mudancas acima (scroll pixel a pixel, +2 no
  azul, 5 bits por canal, MSK) sao do fwMSX.
- Comportamento conferido no **openMSX** (`resource/fmsxgo/third-party/openmsx`,
  GPL): usado so' como referencia de comportamento. Nenhum codigo dele foi
  copiado; a decodificacao de crominancia e' a mesma do fMSX, e a regra do azul
  e' a descrita no comentario dele.
- O `fmsxgo` (pacote Go do autor) serviu de referencia para o scroll dos modos
  5-8 e para a mascara da esquerda.

## 6. O que nao esta coberto

- **Efeitos de rastreio no meio do quadro** (paleta e scroll por linha): ja'
  cobertos, como nos outros modos (ver `doc/vdp-spec.md`, secao 6, e
  `doc/msx2-spec.md`, secao 6) -- inclusive os modos YJK/YAE/scroll do V9958,
  ja' que o snapshot por linha guarda `regs[]`/paleta/cache de tabela inteiros.
- **Cores YJK nao foram conferidas com um V9958 real**: a regra do azul vem de
  um turbo R segundo o openMSX, mas nao houve medicao propria.
- **Jogos MSX2+ reais (validacao em 2026-10-05)**, em `resource/fmsxgo/media`:
  - **Sonyc (Analogy) (1995)**, ROM ASCII8 de 1 MB: sobe no `--msx2p`, pergunta
    "Easy or Normal" (E/N); com `E` passa a SCREEN 8 com R#25 = `08h` (YJK). A tela
    de titulo ("ANALOGY presents") sai com cores naturais, o que indica que a
    decodificacao YJK esta correta com dados reais. O jogo avisa "FM not found" e
    "R800 not found" e segue na versao Z80: esta maquina nao tem FM.
  - **Mega Chase - Shadowfax (2026)**, disco de 720KB montado com `--disk-ro`: inicia
    a partir do disco, liga YJK com HScroll512 (R#25 = `09h`) e faz rolagem horizontal
    real (R#26/R#27 mudam a cada quadro). O titulo "PUSH FIRE" aparece nitido. A
    arte do titulo ocupa pouca area da tela e a tecla de espaco nao saiu do titulo,
    entao o jogo em si nao foi validado. A rolagem com HScroll512 ainda precisa de
    conferencia visual contra um V9958 real.
- **Atributo de 10 e 11** nao e' distinguido, como no fMSX.
