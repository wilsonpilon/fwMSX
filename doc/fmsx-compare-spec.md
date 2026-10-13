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

## Resultado contra o fMSX original (2026-10-13, fwMSX 1.36.1)

| caso | modelo | exato | forma | observacao |
|------|--------|-------|-------|------------|
| bios_msx1 | MSX1 | 100% | 100% | (1.36.0: deslocado 1 pixel) |
| bios_msx2 | MSX2 | 100% | 100% | idem |
| bios_msx2p | MSX2+ | 100% | 100% | |
| kings_valley | MSX1 | 100% | 100% | |
| f1_spirit | MSX1 | 100% | 100% | (99,88% aos 700 quadros) |
| lode_runner_scc | MSX1 | 100% | 100% | idem (tela de texto, SCREEN 0) |
| firebird_msx2 | MSX2 | 100% (500 e 800 quadros) | 100% | aos 600 o fade esta' uma etapa adiante (sincronia) |

A **paleta e' identica** (a diferenca de cor que o fmsxgo mostrava era do fmsxgo, nao do fwMSX), e a BIOS dos tres
modelos sobe ate o prompt exatamente igual ao fMSX.

### Diferenca real achada e corrigida (1.36.1)

**Texto de SCREEN 0 (40 colunas) 1 pixel a esquerda** no fwMSX 1.36.0: conteudo de x=28 a 212 contra 29 a 213 no
fMSX. O fMSX desenha o texto a partir do pixel 9 da area de 256 (`RefreshLine0`: `P+=9`; TEXT80: `P+=18` na tela
larga), e o fwMSX centralizava (8/8). Corrigido em `Machine::RenderFrame` (`+1` no MSX1, `+2` na tela de 512 do
MSX2/MSX2+; texto 240 e TEXT80). O TEXT80 segue sem caso de comparacao.

### Licao do harness (armadilha)

O fMSX **grava `CMOS.ROM` ao sair e le na proxima partida**. Sem apagar entre os casos, o MSX2+ herdou a
configuracao de tela deixada pela BIOS MSX2 do caso anterior e parecia ter um "boot diferente" no fwMSX (86% de
igualdade). Era artefato: o `compare.py` agora apaga o `CMOS.ROM` antes de cada execucao do fMSX, e o resultado
do MSX2+ e' 100%.

## Jogos provisorios do file-hunter (`tests/media/`, 2026-10-13)

O usuario liberou o uso do acervo `D:\msx\archive\download.file-hunter.com - 24-06-2026\Games` para testes. 19 ROMs
(2,7 MB; Konami, Compile, IREM) foram copiadas para `tests/media/` (`MSX1_<nome>.rom` roda como MSX1, `MSX2_<nome>.rom`
como MSX2; cada uma vira um caso com pontos em 600/900/1200 quadros). **Provisorio**: na versao final nenhuma midia de
terceiros fica no repositorio nem nos pacotes (nem as ROMs do fMSX): o fwMSX tera uma opcao para baixar as ROMs do
site do fMSX e navegar/baixar arquivos de https://download.file-hunter.com. O `compare.py` da o mesmo `CARTS.SHA` aos
dois emuladores (sem ele o fMSX adivinha o mapper e erra, como em R-Type e Aleste).

Resultado (26 casos): 25 passam (>= 98% em min(forma, detalhe)); so' o R-Type falha (91,7%). A maioria bate a 100%
pixel a pixel nos pontos em que as animacoes de abertura coincidem. Diferencas abaixo de 100% sao quase todas de **sincronia** (modo de atracao/fade em quadros
diferentes: Pippols, Vampire Killer, Space Manbow, Metal Gear, Firebird, Yie Ar Kung-Fu, Gofer no Yabou).
A metrica `forma` agora casa as cores UM-PARA-UM (antes uma tela quase uniforme, como o prompt do BASIC contra um jogo
parado, passava a 98%: o R-Type enganou a primeira versao) e ha' a metrica `detalhe` (so' os pixels que nao sao o fundo).

### Diferenca real achada (aberta): R-Type nao inicia

R-Type (IREM, 384 KB, ASCII16 pelo `CARTS.SHA`) mostra o titulo no fMSX e fica no prompt do BASIC no fwMSX. O
cabecalho `AB` dessa ROM esta' no FIM (banco 23, offset 0x5C000), nao no comeco. O fMSX trata isso em `ResetMSX`
(`MSX.c`, "Some MegaROMs default to last pages on reset": se nao ha `AB` no inicio mas ha' nas duas ultimas paginas
de 8 KB, mapeia `SetMegaROM(J, I-2, I-1, I-2, I-1)`); o fwMSX so' parte das primeiras paginas. Conserto previsto em
`src/memmap` (estado inicial do mapper de MegaROM). Mesmo criterio vale para outras MegaROMs com `AB` no fim.

## O que falta

- Caso de TEXT80 (precisa injetar teclas no `Headless.c`).
- Jogos citados no OUTLINE como pendentes (Parodius, Mega Chase, F-1 Spirit 3D) nao estao no acervo local
  (`dist/roms` so' tem BIOS/sistema); entram na lista quando houver a ROM.
- Casos com disco (MSX-DOS), teclas digitadas e SCREEN 5-8 de jogo (o `Headless.c` ainda nao injeta teclas).
- Comparar o AUDIO (PSG/SCC/FM) -- por ora so' a imagem.
- Sincronia por quadro: hoje o ponto e' o numero do quadro; um disparo por evento reduziria os falsos negativos.
