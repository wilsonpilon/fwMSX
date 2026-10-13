# Comparacao de quadros com o fmsxgo (`tools/fmsxgo_ref/`) -- especificacao

Ferramenta de desenvolvimento (nao entra no build nem no `ctest`) que mede a compatibilidade do fwMSX com o fMSX
comparando a TELA que os dois produzem para a mesma ROM. A referencia e' o **fmsxgo** (`E:\fmsxgo`, port do fMSX em
Go do proprio autor); ele entra por `replace` no `go.mod` e nao e' copiado nem alterado.

## Como usar

    python tools/fmsxgo_ref/compare.py [--fmsxgo E:/fmsxgo] [--fw dist/fwMSXc.exe] [--png pasta] [--cores] [--min 0.98] [caso ...]

Precisa de Go no PATH, do `dist/fwMSXc.exe` e do `dist/roms/bios/{MSX,MSX2,MSX2P}.ROM`; as sub-ROMs MSX2EXT/MSX2PEXT
vem do proprio fmsxgo. `--png` grava, por ponto, `referencia | fwMSX | diferenca` lado a lado; `--cores` lista as
cores da referencia que o fwMSX desenha diferente. Codigo de saida 1 se algum caso falhar.

## Como funciona

- `main.go` (gerador): roda `N` quadros no fmsxgo (`msx.NewMachine`, `LoadCartridge`, `StepFrame`) e grava o quadro
  inteiro em PPM (288x240, x reduzido a metade). Sem DISK.ROM, igual ao fwMSX sem `--disk`.
- `compare.py`: roda o `fwMSXc.exe --msx ... --frames N --shot` (as telas MSX2 saem em dobro, 544x456: reduzidas a 1x),
  **alinha** os dois quadros (as bordas dos emuladores diferem; o deslocamento achado foi sempre (8, 6)) e calcula
  duas metricas na janela comum:
  - `exato`: pixels identicos;
  - `forma`: pixels iguais depois de mapear cada cor da referencia para a cor mais comum do fwMSX (ignora paleta;
    e' o criterio de aprovacao, piso 98%).
- Cada caso tem varios pontos (quadros): os dois emuladores nao ficam sincronizados ao quadro (fades, aberturas),
  entao o caso passa se ALGUM ponto bater; todos sao listados.

## Resultado de referencia (2026-10-13, fwMSX 1.36.0)

| caso | modelo | forma | exato |
|------|--------|-------|-------|
| bios_msx1 / bios_msx2 / bios_msx2p | 1 / 2 / 2+ | 100% | 0% (MSX1: paleta) / 100% / 100% |
| kings_valley | MSX1 | 100% | 96,5% |
| f1_spirit | MSX1 | 100% | 30,7% (paleta) |
| lode_runner_scc | MSX1 | 100% | 97,9% |
| firebird_msx2 | MSX2 | 100% | 100% nos 500 e 800 quadros (aos 600 o fade esta' uma etapa adiante: sincronia) |

- **Paleta do MSX1**: o fmsxgo usa cores a 255 (`#2424FF`, `#FFFFFF`...) e o fwMSX a paleta do fMSX original
  (`#2020E0`, `#E0E0E0`...; `src/vdp/core/vdp_state.c`, `kPalInit`). E' diferenca de paleta, nao de imagem: o fwMSX
  segue o fMSX.
- Nenhuma diferenca de forma (VDP, mapper, Z80, timing grosso) apareceu nos 7 casos.

## O que falta

- Jogos citados no OUTLINE como pendentes (Parodius, Mega Chase, F-1 Spirit 3D) nao estao no acervo local
  (`dist/roms` so' tem BIOS/sistema); entram na lista quando houver a ROM.
- Casos com disco (MSX-DOS), teclas digitadas e SCREEN 5-8 de jogo; o gerador ainda nao tem `-keys`.
- Comparar o AUDIO (PSG/SCC/FM) -- por ora so' a imagem.
- Sincronia por quadro: hoje o ponto de comparacao e' o numero do quadro; um disparo por evento (ex.: "esperar a tela
  parar de mudar") reduziria os falsos negativos.
