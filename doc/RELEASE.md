# Releases

Cada versao do fwMSX recebe o nome de um jogo classico de MSX, seguido de
um subtitulo curto indicando em que ponto do projeto estamos -- nao se
usam tags puramente numericas. Versionamento no formato `X.Y.Z`:

- **X** (major): sobe quando um grupo de mudancas fecha uma base estavel.
- **Y** (minor): sobe a cada feature nova incorporada ao projeto.
- **Z** (patch): sobe a cada compilacao/build gerado.

Resumo curto de cada versao tambem em [CHANGELOG.md](CHANGELOG.md);
especificacao completa e historico de fases em [SPEC.md](SPEC.md).

---

## v1.12.0 -- "Metal Gear: Entrada Direta" (2026-10-05)

**Fase:** item menor adiado desde a v1.2.0 ("5.1 Visao registrada", `doc/SPEC.md`) -- sem
recurso novo, so' uma mudanca de comportamento padrao agora que faz sentido.

### Destaques

- **`fwMSX.exe` sem argumento nenhum abre a maquina completa em janela** (os mesmos padroes de
  `fwmsx --msx`: BIOS MSX1 ao lado do executavel, sem cartucho/disco). Ate aqui, sem argumentos,
  o executavel imprimia so' o esqueleto dos quatro modulos (C++/C/Assembly/Fortran) -- fazia
  sentido enquanto nao havia emulacao de verdade, mas deixou de fazer desde que o core existe
  (Z80 desde a v1.3.0, VDP desde a v1.5.0, maquina completa desde a v1.9.0).
- O esqueleto continua acessivel, so' que agora precisa de argumentos explicitos que nao batem
  com nenhum modo conhecido (ex.: `fwMSX.exe NomeDoProduto 1 2 3`) -- preservado por ser o
  historico do projeto, nao removido.

### Exemplo rapido

```powershell
.\dist\fwMSX.exe                 # abre a maquina MSX1 numa janela (BASIC pronto)
.\dist\fwMSX.exe fwMSX 2 0 5     # esqueleto multi-linguagem (historico)
```

### Build usado para validar esta release

- Toolchain MSYS2 UCRT64: GCC/GNU Fortran 16.2.0, NASM 3.02, CMake 4.4.4, Ninja 1.13.2.
- `.\build.ps1` gerou `dist\fwMSX.exe`, `dist\msxdisk.exe` e `dist\fwMSX-1.12.0.zip`.
- `ctest`: 11 suites, todas passando (`z80_smoke`, `z80_debug_session`, `memmap_slots`,
  `vdp_digital`, `ppi_keyboard`, `psg_sound`, `machine_frames`, `fdc_wd2793`, `vdp_msx2`,
  `msx2_machine`, `audio_live`).
- Verificado a mao: `dist\fwMSX.exe` sem argumentos abre a janela e fica rodando.

### Correcoes no script de build

- `build.ps1` agora para no primeiro erro do CMake (configuracao ou compilacao). Antes ele
  imprimia "Pronto" e empacotava um zip mesmo quando a compilacao falhava.

### Limitacoes conhecidas

- As mesmas da v1.11.1.

---

## v1.11.1 -- "Firebird: Arrumando a Casa" (2026-10-02)

**Fase:** acabamento da v1.11.0 -- sem recurso grande, so' o que sobrou pelo caminho.

### Destaques

- **`--disk-ro`:** discos somente leitura. O MSX-DOS le normalmente, recusa gravar (`Write
  protect error writing drive A`) e o arquivo `.dsk` fica byte a byte identico. Vale para os
  discos inseridos pelo menu **Disco** tambem. Fecha o maior risco de perda de dados
  documentado desde a v1.10.0.
- **Documentacao em dia:** textos que diziam "sem janela" ou listavam como pendentes coisas ja'
  feitas (audio ao vivo, joystick, janela, cartuchos) foram corrigidos, e dois `TODO(FDC)` que
  apontavam para um controlador ja' existente viraram notas.
- Verificado: jogos MSX1 sobem na maquina MSX2 (King's Valley igual ao fMSXgo).
- 800 verificacoes automatizadas (11 suites).

### Limitacoes conhecidas

- As mesmas da v1.11.0. O "Lode Runner + Konami SCC" continua caindo no BASIC ("Illegal
  function call in 10"): e' uma ROM que espera um disco proprio, e nao foi investigada.

---

## v1.11.0 -- "Firebird: MSX2 em Cena" (2026-10-02)

**Fase:** MSX2 -- VDP V9938, mapper de RAM, RTC. Nome escolhido por "Firebird"
(Hi no Tori Hououhen, Konami) ser o jogo MSX2 que motivou tudo: na v1.10.0 ele nao
rodava (e' um jogo MSX2, titulo em SCREEN 5) e agora joga.

### Destaques

- **`fwmsx --msx --msx2`:** a BIOS MSX2 real sobe ate' o **MSX BASIC 2.1**, identico ao
  fMSXgo usado como referencia. `--ext <arq>` escolhe a sub-ROM (padrao: `MSX2EXT.ROM`
  ao lado da BIOS). O MSX-DOS 1.8 tambem boota no MSX2 (a sub-ROM e a DISK.ROM dividem
  o slot 3:1) e nem pergunta a data, porque o RTC ja' a fornece.
- **VDP V9938 completo:** VRAM de 128KB com paginas (R#14); **SCREEN 3, 4, 5, 6, 7, 8 e
  TEXT80**; 212 linhas; cor 0 transparente; **sprites de modo 2** (cor por linha, bits
  CC/EC, 8 por linha, 9o sprite, colisao). SCREEN 6/7 e TEXT80 saem com 512 pixels de verdade.
- **Motor de comandos** (adaptado do `V9938.c`): POINT, PSET, SRCH, LINE, LMMV, LMMM, LMCM,
  LMMC, HMMV, HMMM, YMMM, HMMC, com operacoes logicas, **temporizacao por scanline** e o
  handshake TR. `LINE ,bf`, `CIRCLE` e `PAINT` do BASIC desenham de verdade.
- **RAM de 128KB com mapper** (portas `FCh`-`FFh`) e **relogio RTC** RP5C01 com CMOS
  (`B4h`/`B5h`), com os valores padrao da BIOS.
- **Janela:** a imagem do MSX2 e' sempre de 512 pixels de largura (modos de 256 saem
  dobrados) e a janela recria a textura ao mudar de modo.
- **Firebird joga:** logo MSX, logo Konami, titulo com kanji, floresta rolando e sprites
  coloridos de modo 2.
- `--keys` aceita pontuacao com SHIFT (`( ) : $ " ...`); `--vdplog` (sem janela) mostra, a
  cada quadro, os registradores do VDP que mudaram.
- 795 verificacoes automatizadas (11 suites): `vdp2test` (cada modo e cada comando, pixel a
  pixel) e `msx2test` (mapper, RTC, a BIOS MSX2 real e o BASIC desenhando).

### Correcoes que valem tambem para o MSX1

- **SCREEN 3 (multicolor)** agora e' desenhado (caia no fundo liso).
- A **cor 0 transparente** e a **tela desligada** (R#1 bit 6) valem em todos os modos.
- `R#6`/`R#10`/`R#11` guardam so' os bits que existem (como o `VDPOut()` do fMSX).
- Desvio deliberado do fMSX: o bit TR do motor de comandos comeca limpo a cada comando.

### Exemplo rapido

```
> .\dist\fwMSX.exe --msx --msx2
> .\dist\fwMSX.exe --msx --msx2 --cart "Firebird.rom"
> .\dist\fwMSX.exe --msx --msx2 --frames 600 --keys "screen 5:line (20,20)-(120,80),9,bf:a$=input$(1)|" --wait 900 --shot tela.ppm
```

### Limitacoes conhecidas

- **Sem V9958 (MSX2+):** SCREEN 10-12 (YJK/YAE), rolagem horizontal (R#26/R#27) e R#25.
- A imagem e' montada **no fim do quadro**, nao por scanline: efeitos de rastreio no meio
  do quadro (paleta ou rolagem por linha) nao aparecem. Sem borda/overscan nem entrelacamento.
- Sem SCC/FM (F1 Spirit e jogos da Konami rodam sem esse som); sem MSX-DOS 2, Kanji ROM e RS-232.
- Verificado visualmente contra o fMSXgo (boot do BASIC 2.1 e Firebird), **sem diff automatico
  de pixels**. Testado com um unico jogo MSX2 (o Firebird).
- A CMOS do RTC nao persiste entre execucoes.
- Demais limitacoes das versoes anteriores continuam valendo.

---

## v1.10.0 -- "Golvellius: MSX-DOS e Joystick" (2026-10-02)

**Fase:** disco, joystick e cartuchos de verdade. Nome escolhido por "Golvellius"
(Compile) ser um classico de MSX lembrado como um dos jogos de disco/cartucho
mais ambiciosos da plataforma -- e o marco aqui e' o emulador finalmente rodar
software de terceiros: MSX-DOS e jogos reais.

### Destaques

- **O MSX-DOS 1.8 boota.** `fwmsx --msx --disk msxdos1.dsk` carrega
  `MSXDOS.SYS` + `COMMAND.COM` pela **controladora WD2793** (motor em C adaptado do
  `WD1793.c` do fMSX) mapeada em `7FF8h-7FFFh` do slot da `DISK.ROM` (3:1). `dir` e
  `copy` funcionam; o arquivo copiado sai identico ao original (conferido byte a
  byte e por hash). As escritas vao direto para o `.dsk`. Menu **Disco** na
  janela (inserir/ejetar A:/B:); opcoes `--disk`, `--diskb`, `--disk-interface`,
  `--diskrom`. Novo mecanismo `SlotMmio` no barramento de slots.
- **Joystick** (R14/R15 do PSG): setas + Z/Espaco (fogo A) + X (fogo B) na porta A
  e **gamepads do GLFW** (1o -> A, 2o -> B), com menu **Joystick**.
- **Deteccao automatica de mapper** (`MAP_GUESS` do fMSX): `--cart` com mais de
  32KB acerta Konami4/Konami5/ASCII8/ASCII16/Gen8 sozinho.
- **Bug corrigido:** o estado inicial de uma MegaROM mostra os bancos 0,1,2,3 (como
  o fMSX), nao todos no banco 0 -- varios jogos chamam 6000h-7FFFh antes de trocar banco.
- **Jogos reais verificados:** King's Valley (roda; o espaco inicia o jogo) e F1 Spirit
  (roda ate' o menu). **Firebird e' um jogo MSX2** -- confirmado contra um emulador
  de referencia (fMSXgo) -- e espera pelo VDP MSX2.
- 659 verificacoes automatizadas (9 suites).

### Exemplo rapido

```
> .\dist\fwMSX.exe --msx --disk msxdos1.dsk
> .\dist\fwMSX.exe --msx --cart "King's Valley.rom"
> .\dist\fwMSX.exe --msx --disk msxdos1.dsk --frames 600 --keys "|dir|" --wait 120 --shot tela.ppm
```

### Limitacoes conhecidas

- **Disco:** sem FORMAT (READ/WRITE TRACK nao suportados, como no fMSX), so' imagens
  `.dsk` cruas, sem DiskROM do MSX-DOS 2 (exige MSX2). A interface so' liga com
  `--disk`/`--disk-interface`. O `copy` de 7KB leva ~5 s emulados.
- **As escritas do MSX-DOS vao direto para o arquivo da imagem** -- faca backup.
- Sem modos MSX2 (SCREEN 5-8), logo sem Firebird e demais jogos MSX2; SCREEN 3 ainda
  cai no fallback de cor de fundo. Sem SCC/FM (F1 Spirit roda sem o som do SCC).
- Joystick e gamepad foram testados pelo caminho do programa lendo R14 (cartucho
  sintetico); nao ha' teste com gamepad fisico nem das teclas na janela. A janela
  interativa foi validada em abertura/fechamento; digitar pelo teclado do host e o
  som audivel nao tem teste automatico.
- Demais limitacoes das versoes anteriores continuam valendo.

---

## v1.9.0 -- "Gradius 2: Janela e Som" (2026-10-02)

**Fase:** PSG + janela com teclado do host + audio ao vivo. Nome escolhido
por "Gradius 2" (Konami) ser um classico de MSX1 lembrado justamente pelo
som -- e o marco aqui e' o emulador finalmente *soar*, alem de aparecer.

### Destaques

- **`fwmsx --msx`: a maquina MSX1 completa numa janela.** A BIOS real roda
  em tempo real (Dear ImGui + GLFW + OpenGL) com o teclado do host mapeado
  para a matriz do MSX (posicional, layout US; Alt esq. = GRAPH, Alt dir. =
  CODE, End = SELECT, Pause = STOP, F11 = tela cheia). Menu com Reset,
  Pausar, Soltar teclas, Escala inteira e Som. `--cart <arq> [mapper]`
  carrega um cartucho no slot 1 (ROM plana ate' 32KB ou MegaROM).
- **PSG AY-3-8910** (portas `A0h`-`A2h`): motor em C que gera amostras PCM
  de verdade (3 tons, ruido LFSR de 17 bits, 16 formas de envelope), tabela
  de volume logaritmica em Fortran, `PsgDevice` e gravador de WAV em C++.
  Com a BIOS real, `BEEP` no BASIC toca o canal A em 1316 Hz.
- **Audio ao vivo** pelo dispositivo padrao do sistema (miniaudio, baixado
  por FetchContent), via buffer circular sem trava, com pre-enchimento
  contra estalos e menu **Som** (mudo + volume). `--mute` desliga.
- **Modo sem janela:** `fwmsx --msx --frames N --shot tela.ppm --keys
  "print 1234|"` roda, digita e salva a tela -- util para CI.
- Depurador: `--psg`, `psgregs`, `psgpoke`, `psgrec start|stop|clear|save`.
- 570 verificacoes automatizadas (8 suites).

### Exemplo rapido

```
> .\dist\fwMSX.exe --msx
> .\dist\fwMSX.exe --msx --cart jogo.rom
> .\dist\fwMSX.exe --msx --frames 400 --keys "print 1234|" --shot tela.ppm
```

### Limitacoes conhecidas

- So' SCREEN 0/1/2 (+ sprites de modo 1); SCREEN 3 e os modos MSX2 mostram
  so' a cor de fundo (Fase 4 do VDP).
- Sem joystick (R14/R15 do PSG) e sem disco (FDC): so' teclado.
- Sem click de tecla/cassete, SCC nem FM.
- Teclado posicional: acentos/cedilha do ABNT2 nao tem tecla.
- A janela interativa foi validada em abertura/fechamento e no fluxo de
  audio (`--frames N`); a digitacao pelo teclado do host nao tem teste
  automatico (so' `Machine::KeyDown/KeyUp` por baixo dela). O som foi
  validado pelo consumo do dispositivo no ritmo certo, nao ouvido.
- Audio no Linux: compilado e testado no WSL2, sem dispositivo de audio la'
  (o teste do dispositivo real vira `[SKIP]`).
- Demais limitacoes das versoes anteriores continuam valendo.

---

## v1.8.0 -- "Zanac: Prompt do BASIC" (2026-10-01)

**Fase:** a BIOS MSX1 real sobe. Nome escolhido por "Zanac" (Compile) ser
um clássico de MSX1 -- e o marco aqui é exatamente o "Ok" que o
fMSX/MSX BASIC mostra ao ligar.

### Destaques

- **A BIOS real boota até o prompt do MSX BASIC.** Com `fwmsx --z80dbg
  --slots resource/fMSX/ROMs/MSX.ROM --vdp --ppi`, `run 100000000` e
  `vdpshot tela.ppm`, a tela mostra `MSX BASIC version 1.0 / Copyright
  1983 by Microsoft / 28815 Bytes free / Ok` renderizada pelo VDP do
  projeto. `keydown`/`keyup` digitam no BASIC pelo PPI.
- **Bug do núcleo Z80 corrigido:** `Z80Cpu` não chamava `z80_reset()` na
  construção, então as tabelas de flag ficavam zeradas se ninguém desse
  `reset`. Era a causa real de a BIOS ficar presa desde a v1.4.0.
- 470 verificações automatizadas, teste de aceite com a BIOS real.

### Exemplo rápido

```
> .\dist\fwMSX.exe --z80dbg --slots resource\fMSX\ROMs\MSX.ROM --vdp --ppi
z80dbg> run 100000000
z80dbg> keydown p
z80dbg> run 1000000
z80dbg> keyup p
z80dbg> run 2000000
z80dbg> vdpshot tela.ppm
```

### Limitações conhecidas

- Sem janela (só `vdpshot`), sem teclado do host, sem PSG/som/joystick,
  sem cartuchos carregados automaticamente.
- Demais limitações das versões anteriores continuam valendo.

---

## v1.7.0 -- "Vampire Killer: Teclado e PPI" (2026-10-01)

**Fase:** PPI i8255 + teclado -- ver [ppi-spec.md](ppi-spec.md). Nome
escolhido por "Vampire Killer" (Konami) ser um dos jogos de MSX1 mais
jogados no teclado.

### Destaques

- **As quatro linguagens num único módulo:** chip i8255 em **C**
  (adaptado do fMSX), `PpiDevice` em **C++** ligado ao mapa de memória,
  tabela de posição das 87 teclas em **Fortran**, contagem de teclas
  pressionadas em **Assembly** (`POPCNT`, dual-ABI Win64/SysV).
- **O slot primário agora é do PPI:** muda quando o pino de saída da porta
  A muda, e só depois de a BIOS programar o chip (`82h` em `ABh`).
- **Depurador:** `--ppi`, `ppiregs`, `keys`, `keydown`, `keyup`.
- **Layout MSX1 com BIOS + `--ppi`:** RAM de 64KB em `3:2` e regras de
  subslot do MSX1 (slots 0/1/2 sem subslot).
- 464 verificações automatizadas (`ctest`, 5 suítes), 62 novas.

### Exemplo rápido

```
> .\dist\fwMSX.exe --z80dbg --slots resource\fMSX\ROMs\MSX.ROM --vdp --ppi
z80dbg> keydown shift a
z80dbg> keys
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` (MSYS2 UCRT64), `nasm`, `cmake` + `ninja`;
  `dist/fwMSX.exe`/`dist/msxdisk.exe` estáticos. O pacote Linux
  (`.tar.gz`) desta versão precisa ser gerado via `build.sh` numa máquina
  Linux/WSL2 (o Assembly novo já tem a branch SysV).

### Limitações conhecidas

- **A BIOS real ainda não sobe**: programa e lê o PPI, mas fica presa na
  varredura de RAM (`0x0305`-`0x0331`) e não chega a habilitar o VBlank.
  O teste com a BIOS continua informativo (`doc/ppi-spec.md`, seção 5).
- Sem teclado do host (só `keydown`/`keyup`), sem som de click/relé do
  PPI, sem PSG/joystick.
- Demais limitações das versões anteriores continuam valendo.

---

## v1.6.0 -- "Penguin Adventure: Sprites em Cena" (2026-10-01)

**Fase:** VDP, Fase 3 (sprites de modo 1) -- ver [vdp-spec.md](vdp-spec.md).
Nome escolhido por "Penguin Adventure" (Konami) ser a sequência de
"Antarctic Adventure", o nome da versão anterior: o pinguim agora tem
sprites.

### Destaques

- **Sprites de SCREEN 1/2/3 em C**, adaptados de `Sprites()` e
  `CheckSprites()` do fMSX (`src/vdp/core/vdp_sprites.{h,c}`): 8x8/16x16,
  ampliação 2x, prioridade por índice, cor 0 transparente, early clock,
  Y negativo, terminador Y=208 e R#8 bit 1.
- **Quinto sprite** (limite de 4 por linha, flag + número em S#0) e
  **colisão** (S#0 bit 5, linha 192), tudo dirigido por
  `vdp_step_scanline()`; ler S#0 pela porta `99h` limpa os flags.
- Recorte pixel a pixel no lugar das máscaras de bits do original --
  mesmo resultado, verificado nos casos de borda.
- 402 verificações automatizadas (`ctest`, 4 suítes), 20 novas.

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` (MSYS2 UCRT64), `nasm`, `cmake` + `ninja`;
  `dist/fwMSX.exe`/`dist/msxdisk.exe` estáticos.
- Linux (WSL2): pacote `dist/fwMSX-1.6.0-linux.tar.gz` gerado pelo autor
  via `build.sh`. No caminho apareceu um problema de build da GUI: o
  GLFW 3.4 exige `wayland-scanner` por padrão no Linux; o `CMakeLists.txt`
  agora cai para X11 só quando a ferramenta não existe (ver
  `build.sh`, pré-requisitos).

### Limitações conhecidas

- Sem sprites de modo 2 (SCREEN 4-8) nem modos MSX2 -- Fase 4.
- Sem borda/overscan, sem `ScreenON`, sem janela gráfica em tempo real
  (`vdpshot` exporta PPM).
- A BIOS real ainda depende de PPI/teclado para habilitar o VBlank (ver
  `doc/SPEC.md`, seção 5.0).
- Demais limitações das versões anteriores continuam valendo.

---

## v1.5.0 -- "Antarctic Adventure: Primeiros Pixels" (2026-09-30)

**Fase:** VDP (TMS9918/V9938), Fases 0.5, 1 e 2 -- ver
[vdp-spec.md](vdp-spec.md) para a especificação completa. Nome
escolhido por "Antarctic Adventure" (Konami) ser um dos MSX1 mais
lembrados justamente pelos gráficos -- um pinguim correndo sobre um
SCREEN 2 bem aproveitado -- exatamente o tipo de imagem que este
módulo agora sabe decodificar de verdade.

### Destaques

- **`CompositeBus`** (design próprio): o Z80 agora fala com mais de um
  dispositivo de I/O ao mesmo tempo (mapa de memória + VDP), sem
  acoplar os dois -- porta de I/O despachada por dispositivo registrado,
  memória sempre para um único dispositivo designado.
- **Motor "digital" do VDP em C**, adaptado de `resource/fMSX/fMSX/
  MSX.c`: registradores, VRAM, protocolo das 4 portas (`98h`-`9Bh`), e a
  máquina de estados de scanline que gera VBlank/HBlank -- **finalmente
  dá uso real ao `Z80Cpu::interrupt()`**, que existia desde a Fase 1 do
  núcleo Z80 mas nunca tinha sido exercitado. Confirmado com um
  programa sintético de 12 bytes: a interrupção chega em `0x0038` de
  verdade (`IFF1` desligando, não só PC/SP coincidindo por acaso).
- **Renderização real de SCREEN 0/1/2**, adaptada de `resource/fMSX/
  fMSX/Common.h` -- texto mono (240×192), texto colorido com o quirk
  real de "cor por grupo de 8 caracteres" da TMS9918, e bitmap 256×192
  com tabela de cor/padrão em terços. Exportável como imagem PPM
  (`vdpshot`) para inspeção/teste sem precisar de janela gráfica ainda.
- **Bug real corrigido**: a paleta padrão nunca era carregada no reset
  (ficava zerada) -- sem a correção, tudo renderizaria em preto, já que
  software MSX1 normal conta com a paleta fixa do TMS9918 estar
  presente desde o ligar, sem escrevê-la manualmente.
- **Tabela de paleta de 512 cores em Fortran**, construída na Fase 1
  sem consumidor, finalmente usada na Fase 2.
- 382 verificações automatizadas (`ctest`, 4 suítes), até 54 novas desta
  versão.

### Exemplo rápido (poke manual + captura de tela)

```
> .\dist\fwMSX.exe --z80dbg --slots --vdp
z80dbg> vdppoke 0 0x41
z80dbg> vdpshot tela.ppm 0 0
escrito tela.ppm (240x1, modo de tela 0)
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64), `nasm` 3.02, `cmake`
  4.4.3 + `ninja` 1.13.2 -- `dist/fwMSX.exe`/`dist/msxdisk.exe`
  estáticos.
- Linux (WSL2, mesma validação da v1.4.1): pacote gerado via
  `build.sh` pelo autor, com esta versão já incluída.

### Limitações conhecidas

- Sem borda/overscan, sem sprites, sem tela ligada/desligada (`ScreenON`)
  -- só a área ativa de exibição dos três modos suportados.
- Sem janela gráfica em tempo real ainda -- `vdpshot` é a única forma
  de ver o resultado, exportando para um arquivo PPM.
- A BIOS MSX1 real ainda não chega a habilitar a interrupção de VBlank
  dentro de nenhum orçamento de ciclos testado -- precisa de PPI/
  teclado (portas `A9h`-`ABh`), que ainda não existe (ver `doc/SPEC.md`,
  seção 5.0).
- Mesmas limitações já registradas nas versões anteriores (SCC/SRAM/
  `MAP_GMASTER2`/`MAP_FMPAC`/`MAP_GUESS` no mapa de memória, `CPIR`/
  `CPDR` sem aceleração em Assembly).

---

## v1.4.1 -- "Salamander: Compilando em Linux" (2026-09-30)

**Fase:** validação de portabilidade, sem features novas -- primeira vez
que o projeto foi de fato compilado, linkado e executado (não só
compilado no papel) numa máquina Linux real. Nome escolhido por
"Salamander" ser um dos jogos mais emblemáticos de portar entre
plataformas diferentes (arcade, MSX e outros sistemas japoneses),
exatamente o que esta versão prova sobre o próprio projeto.

### O que motivou esta versão

A v1.4.0 já tinha `build.sh` (equivalente Linux do `build.ps1`), mas
nunca tinha sido rodado numa máquina Linux de verdade -- a branch
`elf64`/SysV do `.asm` dual-ABI do núcleo Z80 só tinha sido *montada*
(`nasm -f elf64`), nunca linkada/executada, por falta de ambiente Linux
no desenvolvimento original (só Windows). O autor rodou `build.sh` pela
primeira vez numa máquina Linux real (WSL2) e encontramos, juntos, três
problemas reais de portabilidade -- nenhum deles no código novo do
núcleo Z80/mapa de memória (que já nasceu multiplataforma), todos em
código **mais antigo**, nunca antes testado fora do Windows.

### Bugs corrigidos

1. **`src/asm/init_asm.asm`** (módulo Assembly da própria Fase 0 do
   projeto, de 2026-09-28) -- Win64-only. Montava sem erro para `elf64`
   (o NASM não valida convenção de chamada, só gera bytes), mas tinha
   dois problemas reais rodando de verdade: os argumentos chegariam nos
   registradores errados (SysV entrega major/minor/patch em
   `EDI/ESI/EDX`, não `ECX/EDX/R8D` da Win64), e o **link falhava**:
   ```
   relocation R_X86_64_PC32 against symbol `printf@@GLIBC_2.2.5' can
   not be used when making a PIE object; recompile with -fPIE
   ```
   porque uma chamada direta a `printf` é incompatível com executável
   PIE (posição-independente), o padrão em toda distro Linux moderna.
   Corrigido com a mesma técnica `%ifidn __OUTPUT_FORMAT__` já usada em
   `src/z80/asm/block_ops.asm`, mais `call printf wrt ..plt` (chamada
   relativa à PLT, que funciona com ou sem PIE).
2. **`src/msxdisk/asm/name_match.asm`** (msxdisk) -- mesmo problema de
   registrador errado, mas **mais perigoso** porque não chama nenhuma
   função externa: o build **não falhava**, só o resultado da
   comparação de nome de arquivo (`list`/`extract` com coringa, ex.
   `*.COM`) ficaria silenciosamente errado em tempo de execução no
   Linux. Corrigido com a mesma técnica de mapeamento de registrador de
   entrada por ABI.
3. **`src/msxdisk/gui/file_dialog.cpp`** incluía `<windows.h>` sem
   nenhuma guarda de plataforma -- quebrava a compilação inteira
   (`msxdisk` e `fwMSX`, que compilam os mesmos fontes) fora do
   Windows. Guardado atrás de `#ifdef _WIN32`; fora do Windows, os
   diálogos nativos de Novo/Abrir/Salvar Como devolvem "cancelado" por
   enquanto (a GUI continua funcionando normalmente, só sem seletor de
   arquivo nativo fora do Windows ainda). `comdlg32` (a biblioteca de
   diálogo do Windows, linkada sem condição no `CMakeLists.txt`) também
   corrigida para só entrar quando `WIN32` é verdadeiro.
4. `build.sh` compartilhava `build/` com `build.ps1` -- num checkout
   acessado tanto nativamente pelo Windows quanto via WSL (`/mnt/c/...`
   para o mesmo diretório), o CMake recusava reconfigurar
   (`CMakeCache.txt` "de outro diretório"). `build.sh` agora usa
   `build-linux/`, diretório próprio, resolvendo o conflito de raiz.

### Resultado

```
$ ./build.sh
[...]
==> Rodando testes (ctest)...
Test project /mnt/c/dos/fwMSX/build-linux
    Start 1: z80_smoke
1/3 Test #1: z80_smoke ........................   Passed
    Start 2: z80_debug_session
2/3 Test #2: z80_debug_session ................   Passed
    Start 3: memmap_slots
3/3 Test #3: memmap_slots .....................   Passed
100% tests passed, 0 tests failed out of 3
==> Empacotando fwMSX-1.4.1-linux.tar.gz...
==> Pronto:
    /mnt/c/dos/fwMSX/dist/fwMSX
    /mnt/c/dos/fwMSX/dist/msxdisk
    /mnt/c/dos/fwMSX/dist/fwMSX-1.4.1-linux.tar.gz
```

As mesmas **328 verificações automatizadas** do núcleo Z80 e do mapa de
memória, incluindo o teste diferencial de `LDIR`/`LDDR` que exercita a
aceleração em Assembly de verdade, passaram no Linux.

### Build usado para validar esta release

- **Windows**: `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64), `nasm`
  3.02, `cmake` 4.4.3 + `ninja` 1.13.2 -- `dist/fwMSX.exe` e
  `dist/msxdisk.exe` estáticos.
- **Linux (novo nesta versão)**: WSL2, toolchain de sistema
  (gcc/g++/gfortran/nasm/cmake/ninja via `apt`) -- `dist/fwMSX` e
  `dist/msxdisk`, dinamicamente ligados ao runtime padrão da distro
  (sem o link estático usado no Windows). Pacote:
  `dist/fwMSX-1.4.1-linux.tar.gz`.

### Limitações conhecidas
- Diálogos nativos de arquivo (Novo/Abrir/Salvar Como na GUI do
  msxdisk) só funcionam no Windows por enquanto -- um seletor nativo
  para Linux (ex. GTK) fica para uma tarefa à parte, se fizer sentido.
- Mesmas limitações de escopo já registradas na v1.4.0 (SCC/SRAM/
  `MAP_GMASTER2`/`MAP_FMPAC`/`MAP_GUESS` no mapa de memória,
  `CPIR`/`CPDR` sem aceleração em Assembly) -- ver `SPEC.md`, seção 5.0.
- Ainda não existe VDP, PSG nem uma máquina MSX completa -- trabalho no
  core de emulação continua pausado deliberadamente (ver `SPEC.md`,
  seção 5.0).

---

## v1.4.0 -- "Illusion City: Mapa de Memória" (2026-09-30)

**Fase:** mapa de memória MSX (slots/subslots/MegaROM), sobre o núcleo
Z80 da v1.3.0 -- ver [memory-map-spec.md](memory-map-spec.md) para a
especificação completa e o histórico de todas as 4 fases. **Trabalho no
core de emulação pausado deliberadamente a partir desta versão** (não
abandono) -- ver [SPEC.md, seção 5.0](SPEC.md) para os próximos passos
já registrados, pensados para retomar sem se perder.

Nome escolhido por ser um dos MegaROMs mais emblemáticos do MSX2+ --
uma aventura gigante (16 Mbit) que depende pesadamente de troca de
slot/subslot e bank-switch pra caber no espaço de 64KB do Z80,
exatamente o que este módulo constrói.

### Destaques
- **Motor de slots/subslots em C**, adaptado de
  `resource/fMSX/fMSX/MSX.c`/`MSX.h` -- topologia real de 4 slots
  primários x 4 secundários x 8 páginas de 8KB, incluindo o quirk de
  hardware onde o registrador de slot secundário é indexado pelo slot
  primário que ocupa a página `C000h-FFFFh`, não um registrador global
  único.
- **`MemorySystem`/`SlotMemoryBus`** (C++, design próprio) com uma API
  de depuração (`PeekSlot`/`PokeSlot`/`Describe`/`CurrentView`) que
  enxerga qualquer slot **independente** do que a CPU vê agora --
  requisito vital do autor, atendido desde a primeira fase deste
  módulo, não deixado para uma fase final como aconteceu com o
  depurador do núcleo Z80.
- **Carregamento de ROM real** com checksum **CRC32 em Fortran**
  (verificado contra o vetor de teste padrão de qualquer CRC32).
- **Seis mappers MegaROM** (`Gen8`/`Gen16`/`Konami5`/`Konami4`/
  `ASCII8`/`ASCII16`, só a troca de banco de ROM) -- cobrem a grande
  maioria dos cartuchos MegaROM reais do MSX.
- **`fwmsx --z80dbg --slots [rom]`**: depurador completo com mapa de
  memória real -- `slots`/`pages`/`slotmem`/`slotpeek`/`slotpoke`/
  `loadrom`, com carregamento automático de ROM de boot na abertura.
- **A BIOS MSX1 real do fMSX rodou de verdade** no núcleo Z80 deste
  projeto: 192 endereços de PC distintos visitados em 100 mil ciclos de
  execução real de código de BIOS -- a primeira prova concreta de
  compatibilidade com software MSX real, não só casos de teste
  escritos à mão.
- **`build.sh`**: equivalente Linux do `build.ps1`.
- 328 verificações automatizadas (`ctest -R "z80|memmap"`, até 168 +
  58 + 102), todas passando.

### Exemplo rápido (BIOS real rodando)

```
> .\dist\fwMSX.exe --z80dbg --slots resource\fMSX\ROMs\MSX.ROM
ROM de boot carregada em 0:0: resource/fMSX/ROMs/MSX.ROM
z80dbg> reset
z80dbg> run 5000
parado: orcamento de ciclos esgotado (ciclos consumidos: 5003, PC=0365)
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estáticos, dependências externas
  apenas as DLLs base do Windows/UCRT.
- `build.sh` (Linux) criado nesta versão, mas ainda **não validado em
  execução real** -- só sintaxe (`bash -n`) e a extração de versão via
  `sed`, sem toolchain Linux disponível nesta máquina de
  desenvolvimento. Fica para quando o autor compilar na própria máquina
  Linux.

### Limitações conhecidas
- Ainda não existe VDP, PSG nem uma máquina MSX completa -- ver
  `SPEC.md`, seção 5.0, para os próximos passos.
- SCC, SRAM persistente, `MAP_GMASTER2`/`MAP_FMPAC` e a heurística
  `MAP_GUESS` ficaram deliberadamente de fora do mapa de memória (ver
  `memory-map-spec.md`, seção 6) -- dependem de som/save-state, que
  ainda não existem.
- `CPIR`/`CPDR` continuam sem aceleração em Assembly (só `LDIR`/`LDDR`)
  -- limitação já registrada na v1.3.0.
- A branch `elf64`/Linux do `.asm` dual-ABI do núcleo Z80 continua só
  montada, nunca linkada/executada de verdade (mesma limitação da
  v1.3.0) -- `build.sh` existe agora especificamente para resolver isso
  quando houver uma máquina Linux disponível.

---

## v1.3.0 -- "SD Snatcher: Núcleo do Z80" (2026-09-30)

**Fase:** núcleo de emulação, primeiro pedaço real (CPU Z80) -- ver
[z80-core-spec.md](z80-core-spec.md) para a especificação completa e o
histórico de todas as fases (1 a 4). `fwMSX.exe` em si continua sem
VDP/PSG/mapa de memória de uma máquina MSX real; esta release entrega a
CPU isolada, exercitada sobre uma RAM plana de teste via o depurador
embutido.

### Destaques
- **Motor de despacho do Z80 em C** (`src/z80/core/`), adaptado de
  `resource/fMSX/Z80/` (registradores, tabelas de ciclo/flag, opcodes com
  e sem prefixo `CB`/`ED`/`DD`/`FD`/`DDCB`/`FDCB`), por trás de um
  `Z80Bus` próprio (callbacks + contexto, permitindo múltiplas instâncias
  de CPU no mesmo processo -- o fMSX original usa funções globais).
- **Wrapper de orquestração em C++** (`Z80Cpu`/`IBus`): `reset()`,
  `run(ciclos)`, `interrupt()`, acesso a registradores -- sem expor a
  união de par de registrador na API pública.
- **Tabelas de flag geradas em Fortran** (`ZSTable`/`PZSTable`, usando o
  intrínseco `POPCNT` para paridade), calculadas uma única vez no reset
  da CPU, nunca no caminho quente do despachante.
- **Aceleração de `LDIR`/`LDDR` em Assembly**: primeiro `.asm` do projeto
  com Win64 **e** SysV AMD64 (Linux) no mesmo arquivo-fonte (`%ifidn
  __OUTPUT_FORMAT__`), usada só quando o bloco inteiro cai em RAM plana
  do host -- o loop byte-a-byte original do fMSX continua como caminho
  de reserva sempre que isso não vale (correção nunca depende do caminho
  rápido, só o desempenho).
- **Depurador embutido em `fwMSX.exe`** -- `fwmsx --z80dbg`: REPL
  (replxx) com `reset`/`regs`/`step`/`run`/`break`/`clear`/`breaks`/
  `mem`/`peek`/`poke`/`load`/`fill`/`disasm`, rodando sobre uma RAM plana
  de 64KB de teste.
- **Desmontador Z80**, adaptado do desmontador já existente em
  `resource/fMSX/Z80/Debug.c` (tabelas de mnemônicos + algoritmo de
  substituição de gabarito) em vez de reescrito do zero -- com três
  correções cosméticas documentadas em relação ao original (nenhuma
  afeta execução/timing da CPU, só o texto exibido pelo desmontador).
- 203 verificações automatizadas novas (`ctest -R z80`): 168 no motor
  (`z80_smoke`) e 35 no depurador/desmontador (`z80_debug_session`),
  incluindo uma varredura de completude sem crash sobre 2044 combinações
  de opcode do desmontador e um fuzz diferencial de 150+ casos
  comparando o caminho rápido de `LDIR`/`LDDR` contra o loop lento.

### Saida de referencia do esqueleto (`fwMSX.exe` sem argumentos, inalterada)

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.3.0]
----------------------------------------
Loading module... CPP [v 1.3.0]
Loading module...C [v 1.3.0]
Loading module...Assembly [v 1.3.0]
Loading module Fortran [v 1.3.0]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Exemplo rápido do depurador (`fwmsx --z80dbg`)

```
z80dbg> poke 0x0000 0x21
z80dbg> poke 0x0001 0x34
z80dbg> poke 0x0002 0x12
z80dbg> poke 0x0003 0x76
z80dbg> disasm 0x0000 2
0000: 21 34 12     LD HL,1234h
0003: 76           HALT
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estáticos, dependências externas
  apenas as DLLs base do Windows/UCRT.
- A branch `elf64` do `.asm` dual-ABI foi verificada só até a montagem
  (`nasm -f elf64`, sem link/execução -- não há máquina Linux disponível
  nesta sessão de desenvolvimento); a branch `win64` foi validada
  completa (build + link + execução + testes).

### Limitações conhecidas
- Ainda não existe VDP, PSG nem mapa de memória de uma máquina MSX real
  -- o núcleo roda isolado sobre RAM de teste via `--z80dbg`.
- `CPIR`/`CPDR` não têm aceleração em Assembly (só `LDIR`/`LDDR`) --
  decisão deliberada: as flags dessas instruções dependem do byte
  comparado, não só do contador chegar a zero, tornando o corte de lote
  mais arriscado de acertar sob o mesmo padrão de verificação usado para
  `LDIR`/`LDDR`. Ver `z80-core-spec.md`, Fase 3.
- O caminho rápido em Assembly (`elf64`/Linux) não foi testado em
  execução real, só montagem -- ver acima.
- Mesmas limitações do `msxdisk` já registradas nas releases anteriores
  (`create --dos2` não validado num emulador real).

---

## v1.2.1 -- "Metal Gear: Ajustes de Campo" (2026-09-29)

**Fase:** msxdisk (polimento pós-lançamento). Correções encontradas
testando a GUI de verdade em janela gráfica, depois da v1.2.0.

### Destaques
- **Ejetar disco** na GUI (`Arquivo > Ejetar` / `F12`), com confirmação
  só quando há alterações não salvas.
- **Novo/Abrir/Salvar Como** na GUI viraram diálogo nativo de arquivo do
  Windows (navegação de pastas de verdade), em vez de caixa de texto.
- Dois bugs reais de interface corrigidos: diálogos modais que não
  abriam (escopo de ID do ImGui) e teclas de função que agiam no
  painel/item errado depois de marcar por checkbox (painel "ativo"
  desatualizado). Ambos encontrados e confirmados corrigidos em teste
  real pelo autor.

### Saida de referencia do esqueleto (`fwMSX.exe` sem argumentos, inalterada)

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.2.1]
----------------------------------------
Loading module... CPP [v 1.2.1]
Loading module...C [v 1.2.1]
Loading module...Assembly [v 1.2.1]
Loading module Fortran [v 1.2.1]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estaticos, dependencias externas
  apenas as DLLs base do Windows/UCRT.

### Limitacoes conhecidas
- Mesmas da v1.2.0 (`create --dos2` não validado num emulador real;
  nenhuma emulação de MSX ainda).

---

## v1.2.0 -- "Maze of Galious: Gerenciador de Discos" (2026-09-29)

**Fase:** msxdisk (5 fases completas -- ver
[msxdisk-spec.md](msxdisk-spec.md)). `fwMSX.exe` em si continua na Fase 0
do emulador (esqueleto multi-linguagem, sem Z80/VDP); o que essa release
entrega e o utilitario `msxdisk` (standalone e embutido no `fwMSX.exe`).

### Destaques
- `dist/msxdisk.exe`: um unico executavel para CLI one-shot, shell
  interativo (FTP-like), TUI (Norton Commander/XTree) e GUI (Dear ImGui,
  visual moderno proprio) -- manipulacao completa de imagens `.dsk`
  MSX-DOS 1/2 (criar, listar, adicionar, extrair, renomear, apagar,
  subdiretorios, copiar disco, salvar como).
- `fwmsx --msxdisk <argumentos>`: mesmos quatro modos acessiveis direto
  pelo `fwMSX.exe`, sem precisar do binario separado.
- Configuracao/temas/metadados de imagem em SQLite
  (`~/.msxdisk/config.sqlite3`), compartilhado entre TUI e GUI.
- Novas dependencias externas (buscadas via CMake `FetchContent` no
  build, nao redistribuidas em `resource/`): CLI11, replxx, FTXUI,
  Dear ImGui, GLFW, SQLite (amalgamation).

### Saida de referencia do esqueleto (`fwMSX.exe` sem argumentos, inalterada)

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.2.0]
----------------------------------------
Loading module... CPP [v 1.2.0]
Loading module...C [v 1.2.0]
Loading module...Assembly [v 1.2.0]
Loading module Fortran [v 1.2.0]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estaticos (`-static
  -static-libgcc -static-libstdc++ -static-libgfortran`), dependencias
  externas apenas as DLLs base do Windows/UCRT.

### Limitacoes conhecidas
- Nenhuma emulacao de MSX ainda -- `msxdisk` e um utilitario de
  ferramental, nao faz parte do core do emulador em si.
- `create --dos2` (MSX-DOS 2) nao foi validado num emulador/hardware real
  (o `--dos1` foi, no openMSX, MSX1 e MSX2) -- ver
  [msxdisk-spec.md](msxdisk-spec.md), secao 6.
- Varios fluxos interativos da TUI/GUI (dialogos, teclas de funcao,
  duplo-clique) so foram validados parcialmente por automacao (capturas
  de tela pontuais); dependem de confirmacao continua de uso real.

---

## v1.1.2 -- "Nemesis: Renomeacao" (2026-09-28)

**Fase:** 0 -- Esqueleto multi-linguagem (refinamento). Sem mudanca de
comportamento observavel; foco em legibilidade do codigo-fonte.

### Destaques
- Arquivos e funcoes de cada modulo renomeados de `module_<lang>.*` /
  `load_module_<lang>()` para `init_<lang>.*` / `init_<lang>()`, deixando
  o nome do arquivo e da funcao exportada identicos e mais claros sobre o
  papel de cada um (inicializacao do modulo daquela linguagem).
- `CMakeLists.txt`, `src/cpp/main.cpp` e a documentacao (`SPEC.md`,
  `MANUAL.md`, `README.md`) atualizados de acordo.
- Saida do programa **inalterada** em relacao a v1.1.1 (mesmas mensagens
  e assinaturas), apenas com a versao impressa em `[v 1.1.2]`.

### Assinaturas dos modulos (saida de referencia)

| Modulo   | Mensagem                                   | Assinatura |
|----------|---------------------------------------------|:----------:|
| C++      | `Loading module... CPP [v X.Y.Z]`            | `0x0001`   |
| C        | `Loading module...C [v X.Y.Z]`               | `0x0002`   |
| Assembly | `Loading module...Assembly [v X.Y.Z]`        | `0x0003`   |
| Fortran  | `Loading module Fortran [v X.Y.Z]`           | `0x0004`   |

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe`: ~3.1 MB (estatico), dependencias externas apenas
  `ntdll.dll`, `KERNEL32.DLL`, `KERNELBASE.dll` e `ucrtbase.dll`.

### Limitacoes conhecidas
- Nenhuma emulacao de MSX ainda -- apenas o esqueleto de build
  multi-linguagem que servira de base para as proximas fases.
- `resource/` ainda vazio (fontes de referencia a incluir em fases
  futuras).

---

## v1.1.1 -- "Knightmare: Alicerce" (2026-09-28)

**Fase:** 0 -- Esqueleto multi-linguagem. Primeira versao da nova
estrutura de diretorios; ainda nao ha emulacao de MSX.

### Destaques
- Projeto reorganizado em `src/` (um subdiretorio por linguagem), `doc/`,
  `dist/` e `resource/`.
- `main()` em C++ (`src/cpp/main.cpp`) recebendo nome do produto e versao
  via linha de comando, "carregando" um modulo por linguagem (C++, C,
  Assembly, Fortran), cada um retornando sua propria assinatura
  hexadecimal, com resumo final impresso pelo `main`.
- Build via CMake + Ninja sobre o toolchain MSYS2 UCRT64 (GCC/G++/
  GFortran 16.2.0 + NASM 3.02), linkado estaticamente -- `dist/fwMSX.exe`
  depende apenas do UCRT (nativo do Windows 10+).

### Assinaturas dos modulos (saida de referencia)

| Modulo   | Mensagem                                   | Assinatura |
|----------|---------------------------------------------|:----------:|
| C++      | `Loading module... CPP [v X.Y.Z]`            | `0x0001`   |
| C        | `Loading module...C [v X.Y.Z]`               | `0x0002`   |
| Assembly | `Loading module...Assembly [v X.Y.Z]`        | `0x0003`   |
| Fortran  | `Loading module Fortran [v X.Y.Z]`           | `0x0004`   |

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe`: ~3.1 MB (estatico), dependencias externas apenas
  `ntdll.dll`, `KERNEL32.DLL`, `KERNELBASE.dll` e `ucrtbase.dll`.

### Problemas encontrados e corrigidos nesta release
- Saida do modulo Fortran fora de ordem por buffer de E/S proprio do
  `libgfortran` -- ver [CHANGELOG.md](CHANGELOG.md) e
  [SPEC.md](SPEC.md#32-notas-de-implementacao-para-quem-retomar-o-projeto).
- Assinaturas hexadecimais mal formatadas por vazamento de
  `std::left`/`std::right` entre colunas do `std::cout`.

### Limitacoes conhecidas
- Nenhuma emulacao de MSX ainda -- apenas o esqueleto de build
  multi-linguagem que servira de base para as proximas fases.
- `resource/` ainda vazio (fontes de referencia a incluir em fases
  futuras).
