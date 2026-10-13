# Comparacao de quadros com o fMSX (`tools/fmsxgo_ref/`, `tools/fmsx_ref/`) -- especificacao

Ferramenta de desenvolvimento (nao entra no build nem no `ctest`) que mede a compatibilidade do fwMSX com o fMSX
comparando a TELA que os dois produzem para a mesma ROM.

## Referencias

- **fMSX original (padrao, `--ref fmsx`)**: o codigo de `resource/fMSX` (fMSX 6.0, Marat Fayzullin), SEM alteracao,
  compilado com gcc e uma camada de plataforma minima, `tools/fmsx_ref/Headless.c`, no lugar do `Unix.c`: video em
  memoria (32 bpp, 272x228, ou 544x228 nos modos largos), sem som, sem teclado, `-frames N -out arq.ppm`
  (grava o quadro N e sai; `-trace` imprime as mudancas de SCREEN e dos registradores 0-7 do VDP). Roda todos os
  quadros (`-skip 0`). E' o mesmo nucleo do `fMSX.exe` do Windows, mas determinista e sem janela.
  O `fMSX.exe` distribuido (`E:\msxide\paleobasic\...`) nao serve para isso: e' so' janela, sem captura por quadro.
- **fmsxgo (`--ref fmsxgo`)**: port do fMSX em Go do proprio autor, ainda em desenvolvimento -- so' como segunda
  opiniao (`tools/fmsxgo_ref/main.go`; entra por `replace` no `go.mod`, nao e' alterado).

## Como usar

    python tools/fmsxgo_ref/compare.py [--ref fmsx|fmsxgo] [--png pasta] [--cores] [--min 0.98] [caso ...]

Precisa de gcc (MSYS2 UCRT64) no PATH, do `dist/fwMSXc.exe` e do `dist/roms/bios/{MSX,MSX2,MSX2P}.ROM`; as sub-ROMs
MSX2EXT/MSX2PEXT vem de `resource/fMSX`; as ROMs de jogo vem de `E:\fmsxgo\media`. `--png` grava, por ponto,
`referencia | fwMSX | diferenca`; `--cores` lista as cores da referencia que o fwMSX desenha diferente.

## Como funciona

- O fwMSX roda `fwMSXc.exe --msx ... --frames N --shot` (as telas MSX2 saem em dobro, 544x456: reduzidas a 1x).
- Os dois quadros sao **alinhados** (deslocamento achado por busca) e comparados na janela comum:
  `exato` (pixels identicos) e `forma` (igual depois de mapear as cores; criterio de aprovacao, piso 98%).
- Cada caso tem varios pontos (quadros): os emuladores nao ficam sincronizados ao quadro (fades, aberturas), entao o
  caso passa se ALGUM ponto bater; todos sao listados.

## Resultado contra o fMSX original (2026-10-13, fwMSX 1.36.0)

| caso | modelo | exato | forma | observacao |
|------|--------|-------|-------|------------|
| bios_msx1 | MSX1 | 100% | 100% | deslocamento de 1 pixel (ver abaixo) |
| bios_msx2 | MSX2 | 100% | 100% | idem |
| bios_msx2p | MSX2+ | 86% | 87% | **DIFERENTE**: ver abaixo |
| kings_valley | MSX1 | 100% | 100% | |
| f1_spirit | MSX1 | 100% | 100% | (99,88% aos 700 quadros) |
| lode_runner_scc | MSX1 | 100% | 100% | deslocamento de 1 pixel |
| firebird_msx2 | MSX2 | 100% (500 e 800 quadros) | 100% | aos 600 o fade esta' uma etapa adiante (sincronia) |

A **paleta e' identica** (a diferenca de cor que o fmsxgo mostrava era do fmsxgo, nao do fwMSX).

### Diferencas reais achadas (abertas)

1. **Boot do MSX2+ diferente.** No fMSX a BIOS MSX2+ passa pela abertura em SCREEN 1 (quadros 61-111) e cai no
   prompt em **SCREEN 0, 40 colunas, cores 15/4/4** (R#7 = F4) a partir do quadro 112. No fwMSX a primeira escrita
   no VDP so' vem no quadro 140, a abertura nunca termina em SCREEN 0 e o prompt fica em **SCREEN 1** com borda
   ciano (R#7 = 07), mesmo aos 1500 quadros. A CMOS padrao (`RTCInit`) e a leitura do RTC sao identicas as do
   fMSX, e a `MSX2P.ROM` e' a mesma; a causa nao foi achada. Pista: o MSX2 tambem chega mais tarde no fwMSX
   (primeira tela aos 81 quadros contra 66 no fMSX; fim da abertura aos 269 contra 223, ~1,2x), o que sugere
   tempo de CPU por quadro ou espera de BIOS diferente; no MSX2+ o efeito e' muito maior.
2. **Texto de SCREEN 0 deslocado 1 pixel a esquerda** no fwMSX (conteudo de x=28..212 contra 29..213 no fMSX,
   borda e altura iguais). Aparece nos casos de texto (BIOS MSX1/MSX2, Lode Runner na tela de texto).

## O que falta

- Investigar as duas diferencas acima (a 1 e' a que mais importa: muda a tela de partida do MSX2+).
- Jogos citados no OUTLINE como pendentes (Parodius, Mega Chase, F-1 Spirit 3D) nao estao no acervo local
  (`dist/roms` so' tem BIOS/sistema); entram na lista quando houver a ROM.
- Casos com disco (MSX-DOS), teclas digitadas e SCREEN 5-8 de jogo (o `Headless.c` ainda nao injeta teclas).
- Comparar o AUDIO (PSG/SCC/FM) -- por ora so' a imagem.
- Sincronia por quadro: hoje o ponto e' o numero do quadro; um disparo por evento reduziria os falsos negativos.
