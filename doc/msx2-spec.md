# MSX2 (V9938, mapper de RAM, RTC) -- especificacao (documento vivo)

> Mesmo espirito de `doc/vdp-spec.md`, `doc/machine-spec.md` e
> `doc/fdc-spec.md`: o que foi feito, as decisoes e o que falta, para
> retomar do ponto exato onde parou.

Estado: **concluido em 2026-10-02**. `fwmsx --msx --msx2` roda a BIOS MSX2
(MSX BASIC 2.1) com o VDP V9938 completo (SCREEN 0-8 e TEXT80, sprites de modo
2, motor de comandos), RAM de 128KB com mapper e relogio RTC. **O Firebird
(Hi no Tori Hououhen, MSX2) roda e e' jogavel.** O V9958 (MSX2+) saiu depois,
na v1.15.0 -- ver [msx2p-spec.md](msx2p-spec.md). Faltam alguns detalhes do
V9938 -- ver a secao 6.

## 1. Uso

```
fwmsx --msx --msx2                                  # BASIC 2.1 (MSX2.ROM + MSX2EXT.ROM)
fwmsx --msx --msx2 --cart firebird.rom              # cartucho MSX2 no slot 1
fwmsx --msx --msx2 --disk msxdos1.dsk               # MSX-DOS 1.8 no MSX2
fwmsx --msx --msx2 --frames 900 --keys '10 screen 5|20 line (20,20)-(120,80),9,bf|30 goto 30|run|' --wait 600 --shot tela.ppm
fwmsx --msx --msx2 --frames 500 --vdplog --keys 'screen 2|'   # imprime as mudancas dos registradores do VDP
```

- `--msx2` troca a BIOS padrao para `MSX2.ROM` e a sub-ROM para `MSX2EXT.ROM`
  (`--bios`/`--ext` escolhem outras; as ROMs estao em `resource/fMSX/ROMs/`).
- A imagem do MSX2 tem **512 pixels de largura** (modos de 256 saem dobrados) e
  192 ou 212 linhas; cada linha deve ser repetida 2 vezes ao exibir (`FrameSize::
  y_scale`) para a proporcao ficar certa. A janela e o `--shot` ja' fazem isso.
- `--keys` agora digita pontuacao (parenteses, virgula, aspas, `+`, `:`...) com
  SHIFT quando preciso. **Um `SCREEN 5` digitado direto no prompt volta ao modo
  texto ao devolver o "Ok"** (comportamento normal do MSX BASIC): para ficar em
  graficos use um programa em loop (`10 screen 5 ... 60 goto 60` + `run`).

## 2. Arquitetura

```
src/vdp/core/
├── vdp_state.{h,c}   estado do VDP: modelo MSX1/MSX2, VRAM 128KB e paginas (R#14),
│                     status S#0-S#9, R#44/R#46 -> motor de comandos, piscar do TEXT80
├── vdp_render.{h,c}  modos 0-8 + TEXT80, cor 0 transparente, tela desligada, 212 linhas
├── vdp_sprites.{h,c} sprites de modo 1 (SCREEN 1-3) e de modo 2 (SCREEN 4-8)
└── vdp_cmd.{h,c}     motor de comandos do V9938 (adaptado de V9938.c do fMSX)
src/memmap/cpp/ram_mapper.h   mapper de RAM (portas FCh-FFh) + MemorySystem::AllocateMapperRam
src/rtc/rtc_device.h          RTC RP5C01 + CMOS (portas B4h/B5h)
src/machine/machine.{h,cpp}   Machine::Create(model = MSX2): monta tudo
```

**Decisao central: um unico VdpState para MSX1 e MSX2**, escolhido por `vdp_set_model()`.
Os 128KB de VRAM sao sempre alocados; o modelo define quanto vale (`vram_mask`:
`3FFFh` no MSX1, `1FFFFh` no MSX2). Tudo que ja' funcionava no MSX1 continua
identico (os testes antigos so' mudaram onde estavam errados -- ver 5).

### Layout de slots do MSX2 (o do fMSX)

| Slot | Conteudo |
|------|----------|
| 0:0 | `MSX2.ROM` (32KB, BIOS + BASIC 2.1) |
| 1 | cartucho A (como no MSX1) |
| 3:1 | `MSX2EXT.ROM` (sub-ROM, 16KB) em 0000h-3FFFh e, com disco, `DISK.ROM` em 4000h-7FFFh |
| 3:2 | RAM de 128KB com mapper (8 segmentos de 16KB) |

Regras de subslot do MSX2 (`msx1_subslot_rules = 2`): so' os slots de cartucho (1 e
2) ficam sem subslot; o slot 0 pode ser expandido (o `SSlot()` do fMSX so' o
proibe em MSX1).

## 3. VDP V9938

- **VRAM de 128KB:** endereco de 17 bits = pagina (R#14, 3 bits) x 16KB + endereco
  de 14 bits. Ao dar a volta nos 16KB a pagina **avanca** -- so' em SCREEN 4+
  (como o fMSX); a pagina 7 volta para a 0. R#10 (3 bits) e R#11 (2 bits) levam os
  bits altos da tabela de cor e da de atributos de sprite.
- **Modos:** SCREEN 0 (40 colunas) e TEXT80 (480 px, com o piscar de R#12/R#13),
  1, 2, 3 (multicolor), 4 (graphics 3), 5 (256x4bpp), 6 (512x2bpp), 7 (512x4bpp),
  8 (256x8bpp, paleta GRB 3-3-2 fixa). 192 ou 212 linhas (R#9 bit 7); rolagem
  vertical (R#23); paginas de tela por R#2.
- **Cor 0 transparente:** mostra a cor de fundo (R#7), a menos que a de fundo
  seja 0 ou R#8 bit 5 (TP) force a cor 0 a ser solida -- `XPal[0]` do fMSX. Vale
  tambem no MSX1.
- **Tela desligada** (R#1 bit 6): so' a cor de fundo. **Isto corrige o MSX1:** o
  renderizador antigo ignorava o bit; os testes de VDP que renderizavam sem ligar
  a tela foram ajustados.
- **Sprites de modo 2** (SCREEN 4-8, `ColorSprites()` do fMSX): cor por linha de
  sprite numa tabela de 16 bytes antes da de atributos (`spr_tab - 200h`), ate' 8
  por linha, bits CC (OR de cores), EC (desloca 32 px a esquerda), fim da lista em
  Y=216, flag do 9o sprite e colisao tambem nesses modos.
- **Status** iniciais do MSX2 (`VDPSInit` do fMSX): S#0=9Fh, S#2=6Ch.

## 4. Motor de comandos (`vdp_cmd.c`)

POINT, PSET, SRCH, LINE, LMMV, LMMM, LMCM, LMMC, HMMV, HMMM, YMMM, HMMC, ABORT, com
as 10 operacoes logicas (SET/AND/OR/XOR/NOT e as versoes transparentes TSET...) e
os 4 modos de pixel. Adaptado do `V9938.c` do fMSX (reescrito por Alex Wulms).

- **Temporizacao como o fMSX:** um orcamento de 12500 "ciclos" por scanline
  (`vdp_cmd_loop()`, chamado em `vdp_step_scanline()`), com um custo por passo
  medido em hardware para cada comando. Um `HMMV` de 256x100 leva ~450 scanlines
  (~1,7 quadro); um pequeno termina em 1-2. `S#2 bit 0` (CE) fica ligado enquanto
  executa -- quem faz polling de CE funciona.
- **Handshake de transferencia:** LMMC/HMMC (CPU->VRAM) e LMCM (VRAM->CPU) usam R#44
  e S#7 com o bit TR (S#2 bit 7). O 1o dado de LMMC/HMMC vai em R#44 *antes* do
  comando.
- **Desvio deliberado do fMSX:** o TR comeca **limpo** a cada comando. No fMSX ele
  fica ligado depois do ultimo LMMC/HMMC/LMCM e o comando de transferencia
  seguinte nunca consome o 1o dado (o LMCM devolveria um pixel velho). Achado pelo
  teste do LMCM.
- Verificado de duas formas: `vdp2test` (cada comando, com valores calculados a
  mao) e **pelo BASIC**: `LINE (...),BF` (LMMV), `CIRCLE`/`PAINT` (PSET/POINT/SRCH)
  e `LINE` desenham em SCREEN 5 exatamente onde deveriam (`msx2test`).

## 5. Mapper de RAM, RTC e a sub-ROM

- **Mapper (FCh-FFh):** `MemorySystem::AllocateMapperRam(3, 2, 8)`; cada porta
  escolhe o segmento de 16KB de uma pagina da CPU (FCh = 0000h, FDh = 4000h, FEh =
  8000h, FFh = C000h). Estado inicial = o do fMSX (segmentos 3,2,1,0); a leitura
  traz os bits altos em 1 (`valor | ~mascara`). O remapeamento vale na hora, mesmo
  com o slot fora da vista (`memmap_remap_ram_chunk()`).
- **RTC (B4h/B5h):** banco 0 = hora do host (digitos de segundo, minuto, hora, dia da
  semana, dia, mes, ano desde 1980); bancos 1-3 = CMOS de 13 nibbles. **Os padroes
  da CMOS importam**: o banco 2 guarda as preferencias da BIOS (largura 40, cores
  15/4/4...); sem eles o BASIC do MSX2 sobe sem cores. Com o RTC o MSX-DOS nem
  pergunta a data. A CMOS **nao persiste** entre execucoes (`CMOS.ROM` do fMSX nao e'
  carregada/salva).
- **Sub-ROM:** `MSX2EXT.ROM` fica no slot 3:1, pagina 0. Disco e sub-ROM dividem o
  mesmo slot (uma ROM plana de 32KB montada por `Machine::Create`).

## 6. Limites e o que falta

- **V9958 (MSX2+)**: nao faz parte desta fase; ver `doc/msx2p-spec.md` (SCREEN
  10-12, scroll e mascara da esquerda ja' prontos).
- **Efeitos de rastreio (trocar paleta/scroll por linha, tipico de alguns jogos)
  ja' aparecem:** cada linha e' desenhada com o estado que ela tinha DE VERDADE
  durante a execucao do quadro (snapshot por scanline, ver `VdpScanlineSnapshot`
  em `src/vdp/core/vdp_state.h` e `Machine::RenderFrame()`), nao retroativo ao
  quadro inteiro.
- **Interlace, PAL e ajuste de posicao (R#18)** ignorados. Sem borda desenhada
  (so' a cor de fundo nas laterais).
- **Colisao de sprites** de modo 2 usa so' os padroes (como o fMSX), nao as cores;
  `S#3-S#6` (coordenadas da colisao) valem 0.
- **Teclado/CMOS:** sem Kanji ROM (`KANJI.ROM`), sem impressora, sem RS232.
- **O depurador (`--z80dbg`) so' conhece o MSX1** (`vdppeek`/`vdppoke` ja' respeitam
  o tamanho da VRAM do modelo, mas nao ha' flag `--msx2` la'): use `--msx --msx2
  --vdplog` e `--shot`.
- **MSX-DOS 2 testado (1.28.0, 2026-10-08):** `--cart MSXDOS2.ROM msxdos2 --disk
  <disco de 720KB>` sobe o kernel de verdade a partir do disco, com arvore de
  diretorios (`cd`, `dir` dentro de subdiretorios) funcionando. Ver
  `doc/memory-map-spec.md`, secao 6, para o mapper (`MEMMAP_MAPPER_MSXDOS2`).
- **Verificado contra uma referencia:** o prompt do BASIC 2.1 bate em 98% dos pixels
  com o fMSXgo em modo MSX2 (o resto e' o cursor piscando); o Firebird mostra o
  mesmo logo MSX, logo Konami e titulo que a referencia. Comparacoes exatas de
  quadro so' valem em telas estaticas: a fase das animacoes de boot difere.
