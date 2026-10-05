# VDP (V9938/TMS9918) -- especificacao (documento vivo)

> Mesmo espirito de `doc/z80-core-spec.md` e `doc/memory-map-spec.md`:
> analise do fMSX, decisoes de design e fases de implementacao, para
> retomar o trabalho do ponto exato onde parou. Nao remova secoes de
> fases concluidas -- marque como feitas e adicione as novas por baixo.

Estado desta secao: **fase de analise e design, em 2026-09-30**. Nenhum
codigo deste modulo foi escrito ainda. Este e' o "Passo 1" registrado em
`doc/SPEC.md`, secao 5.0 (proximos passos apos o mapa de memoria).

## 1. Objetivo

Dar ao core (Z80 + mapa de memoria, ambos completos) o componente que
**finalmente desenha algo na tela** -- o VDP (Video Display Processor).
O MSX1 usa o TMS9918(A); o MSX2/2+ usa o V9938/V9958, que e'
retrocompativel com o TMS9918 e acrescenta mais modos de tela, paleta de
512 cores e um motor de comando de desenho acelerado por hardware. O
fMSX emula os dois com um unico modulo (nao ha' um "TMS9918.c" separado
-- o V9938 en compatibilidade cobre o caso MSX1 tambem).

**Marco importante para o proprio nucleo Z80**: o VDP e' o que finalmente
da' um uso real para o gancho `LoopZ80`/interrupcao periodica que foi
**deliberadamente deixado de fora** da Fase 1 do nucleo Z80 (ver
`doc/z80-core-spec.md`, secao 3.2 -- "fica para a Fase 4, quando houver
de fato uma 'maquina' [VDP/memoria] para orientar esse desenho"). O VDP
e' exatamente essa maquina: ele gera as interrupcoes de VBlank/HBlank/
coincidencia de linha que o Z80 precisa receber para que qualquer
software MSX real funcione (a rotina de VBlank da BIOS e' o coracao de
como o MSX le teclado, atualiza o relogio, troca de banco de RAM, etc.).

## 2. O que a analise de `resource/fMSX/fMSX/MSX.c`/`V9938.c`/`V9938.h` revelou

Arquivos lidos: `V9938.h` (completo, 38 linhas -- so' declara a API do
motor de comando), lista de funcoes documentadas de `V9938.c` (31KB,
so' os cabecalhos `/** Nome() **/`, sem ler os corpos), e trechos
direcionados de `MSX.c`: `InZ80`/`WrZ80` para as portas `98h`-`9Bh`
(~linha 1121-1360), `VDPOut()` (escrita em registrador, ~linha
1888-1943), e o loop de scanline/interrupcao (~linha 2028-2160, a
implementacao de `LoopZ80` do fMSX -- nao lida em detalhe alem do que
faz parte do "esqueleto digital" abaixo). **Nao lidas ainda**: as ~12
funcoes `RefreshLineN()` (uma por modo de tela, decodificacao de pixel
de verdade -- deliberadamente adiado para quando a Fase 2 comecar, ver
secao 6) e os corpos das funcoes do motor de comando V9938 em `V9938.c`
(SRCH/LINE/LMMV/LMMM/LMCM/LMMC/HMMV/HMMM/YMMM/HMMC -- adiado para uma
fase bem mais a frente, ver secao 6).

- **Registradores**: `byte VDP[64]` (registradores de controle, so' uns
  ~30 realmente usados: R#0-R#8 compativeis com TMS9918, R#9-R#46
  extensoes do V9938) e `byte VDPStatus[16]` (registradores de status,
  lidos pela CPU via porta `99h` -- S#0 tem os flags de sprite/colisao,
  S#1 tem HBlank/coincidencia de linha, S#2 tem VBlank/HBlank raw etc.).
- **Protocolo das 4 portas de I/O** (`98h`-`9Bh`, todo baseado no
  protocolo classico do TMS9918 de "latch de 2 escritas"):
  - `98h` (dados): leitura/escrita direta de um byte de VRAM no
    endereco atual (`VAddr`, 14 bits -- 16KB de VRAM por "pagina",
    `VDP[14]` seleciona qual pagina de 16KB quando ha' mais de uma),
    com auto-incremento e rollover de pagina.
  - `99h` (latch de endereco/registrador): duas escritas em sequencia
    (`VKey` alterna entre elas) -- a segunda escrita decide, pelos 2
    bits mais altos do byte, se e' "define endereco de VRAM para
    leitura", "define endereco de VRAM para escrita" ou "escreve num
    registrador de controle" (delega para `VDPOut()`).
  - `9Ah` (latch de paleta, so' V9938+): duas escritas definindo uma
    entrada de paleta (3 bits de R/G/B cada, convertidos para RGB888
    por uma formula de escala -- `R=(PLatch&0x70)*255/112` etc.),
    avancando automaticamente pelo indice de paleta (`VDP[16]`).
  - `9Bh` (acesso indireto a registrador, so' V9938+): conveniencia que
    usa `VDP[17]` como "registrador atualmente selecionado" com
    auto-incremento opcional (bit 7 de R#17).
- **Cache de ponteiro por tabela, mesmo padrao ja' usado no Z80 (`RAM[]`)
  e no mapa de memoria (`active_view[]`)**: `ChrTab`/`ColTab`/`ChrGen`/
  `SprTab`/`SprGen` sao ponteiros pre-calculados para dentro da VRAM,
  recalculados so' quando o registrador relevante muda (`VDPOut()`) ou o
  modo de tela muda (`SetScreen()`) -- evita recalcular o endereco base
  de cada tabela a cada pixel desenhado. Vamos preservar exatamente essa
  tecnica, ja' validada duas vezes no projeto.
- **Maquina de estados por scanline** (a implementacao de `LoopZ80` do
  fMSX): alterna duas vezes por linha de varredura (HRefresh liga/
  desliga, ajustando `R->IPeriod` para a duracao certa de cada metade),
  contando `ScanLine` (0-261 NTSC / 0-311 PAL), e gerando tres tipos de
  interrupcao conforme os bits de habilitacao em `VDP[0]`/`VDP[1]`:
  - **VBlank (IE0)**: no inicio da area de retraco vertical.
  - **HBlank/coincidencia de linha (IE1)**: quando `ScanLine` bate com o
    registrador `VDP[19]` (ajustado por `VScroll`) -- e' o que MSX2
    usa para efeitos de "raster split" (trocar paleta/scroll no meio da
    tela).
  - Atualiza tambem o flag de piscar (`BFlag`) para os modos de texto
    com "blink" (cor alternante por tempo, VDP[12]/[13]).
  - Ao final, chama `LoopVDP()` (o motor de comando V9938 avancando um
    passo, se houver uma operacao de desenho em andamento) e devolve a
    interrupcao pendente pro Z80 -- **exatamente o papel que `LoopZ80`
    tem na nossa API** (`z80_run()` ainda nao usa esse gancho porque
    nao existia "maquina" nenhuma até agora).
- **Renderizacao por modo, tabela de function pointers**: um array de 12
  funcoes `RefreshLineN` (uma por modo de tela SCREEN 0/1/2/3/4/5/6/7/8/
  10/11/12), cada uma decodificando os pixels de UMA linha de varredura
  a partir da VRAM pro formato de exibicao. Corpos nao lidos ainda
  (proposital, ver secao 6) -- mas a existencia dessa tabela ja' informa
  a arquitetura: o dispatch por modo e' feito uma vez por linha, nao uma
  vez por pixel, o que e' uma boa unidade de trabalho para uma futura
  aceleracao em Assembly (ver secao 3.4).
- **Motor de comando V9938** (`V9938.c` inteiro, so' MSX2+): um
  "blitter" por hardware com ~10 operacoes logicas (busca de ponto,
  linha, movimentacao logica CPU<->VRAM/VRAM<->VRAM com varias
  operacoes booleanas). Acionado pelos registradores 36-46 (`VDPWrite`/
  `VDPDraw`) e avancado incrementalmente por `LoopVDP()`. Recurso
  avancado de MSX2, fora do escopo das primeiras fases (ver secao 6).

## 3. Arquitetura proposta

### 3.1 Estrutura de diretorios

```
src/vdp/
├── core/       C -- VdpState (registradores/status/VRAM/cache de
│               ponteiro de tabela), protocolo de porta 98h-9Bh, maquina
│               de estados de scanline/interrupcao (LoopZ80-equivalente)
├── cpp/        C++ -- VdpDevice (orquestracao), integracao com IBus do
│               Z80 (porta de I/O), janela real (GLFW/OpenGL,
│               reaproveitando a infraestrutura ja' existente da GUI do
│               msxdisk) quando a Fase de renderizacao chegar
├── fortran/    Fortran -- tabela de conversao paleta V9938 (3+3+3 bits
│               RGB) -> RGB888, calculada uma vez na inicializacao
└── debug/      comandos novos no --z80dbg para inspecionar
                registradores/status/VRAM do VDP (ver secao 3.5)
```

### 3.2 `VdpState` (C) -- Fase 1, sem renderizacao ainda

- `uint8_t regs[64]`, `uint8_t status[16]` -- espelham `VDP[]`/
  `VDPStatus[]`.
- VRAM propria (128KB max, 8 paginas de 16KB -- aloca conforme o modelo
  MSX1/MSX2/MSX2+ escolhido, decisao a confirmar na Fase 1).
- `vdp_out(VdpState*, addr_ou_porta, valor)` -- portas `98h`-`9Bh`,
  portado fielmente do protocolo de latch descrito na secao 2.
  `vdp_write_register(VdpState*, r, v)` -- equivalente a `VDPOut()`,
  incluindo o recalculo dos ponteiros de tabela cacheados.
- `vdp_step_scanline(VdpState*) -> proximo_IPeriod, interrupcao_pendente`
  -- adaptado da maquina de estados de `LoopZ80`, faithful a' logica de
  VBlank/HBlank/coincidencia de linha descrita na secao 2. **Sem
  chamada a `LoopVDP()`/motor de comando nesta fase** (isso e' MSX2+,
  fases futuras).
- Atribuicao (ver `LICENSE-THIRD-PARTY.md`): este arquivo adapta
  `MSX.c` (nao `V9938.c`, que so' entra nas fases do motor de comando).

### 3.3 `VdpDevice` (C++) -- orquestracao e integracao com o Z80

- Implementa a fatia de `IBus::in`/`IBus::out` para as portas `98h`-
  `9Bh` (delegando para `vdp_out`), do mesmo jeito que `SlotMemoryBus`
  ja' intercepta a porta `A8h` para o mapa de memoria -- os dois
  dispositivos (mapa de memoria e VDP) vao precisar coexistir no mesmo
  `IBus` que o `Z80Cpu` usa; a Fase 1 do VDP e' tambem o primeiro lugar
  onde vamos precisar de um "barramento composto" que despache pra mais
  de um dispositivo por endereco/porta -- decisao de design registrada
  aqui, a refinar quando a implementacao comecar (ver secao 6, "Fase
  0.5" abaixo).
- Chama `vdp_step_scanline()` a cada vez que o `Z80Cpu::run()` retorna
  controle (o host decide quando, ja' que nosso modelo de execucao e'
  "rode N ciclos e devolva controle" desde a Fase 1 do nucleo Z80 --
  isso substitui o papel do `LoopZ80` do fMSX sem precisar adicionar
  esse gancho ao core em C do Z80).

### 3.4 Fortran -- tabela de paleta

`src/vdp/fortran/palette_table.f90`: converte as 512 combinacoes
possiveis de RGB de 3 bits cada (3+3+3 = 9 bits = 512 entradas) para
RGB888, uma unica vez na inicializacao do VDP -- mesmo principio ja'
validado duas vezes (tabelas de flag do Z80, CRC32 do mapa de memoria):
trabalho numerico simples, uma vez so', fora do caminho quente.

### 3.5 Depurador (`--z80dbg`) -- visibilidade do VDP desde o inicio

Seguindo o padrao que ja' funcionou bem para o mapa de memoria (a
visibilidade foi tratada como requisito de Fase 1, nao deixada para o
final): `--z80dbg` ganha comandos para inspecionar o VDP mesmo antes de
existir renderizacao de verdade --
- `vdpregs` -- despeja os 64 registradores + 16 status decodificados
  (modo de tela atual, flags de interrupcao habilitados, etc.).
- `vdpmem <endereco> [tamanho]` / `vdppeek`/`vdppoke` -- inspeciona/edita
  a VRAM diretamente (equivalente a `slotmem`/`slotpeek`/`slotpoke` do
  mapa de memoria, mesma filosofia).
- `vdpstep [scanlines]` -- avanca a maquina de estados manualmente, util
  para testar a geracao de interrupcao sem precisar rodar o Z80 de
  verdade.

### 3.6 Candidato a Assembly (registrado, nao decidido ainda)

As funcoes `RefreshLineN()` (uma por modo de tela, decodificando uma
linha de varredura inteira da VRAM pro framebuffer) rodam uma vez por
linha por frame -- ~192-212 vezes por frame, ~60 frames/segundo, e' o
tipo de loop quente e regular (desempacotar bits/nibbles de um array de
bytes num array de pixels) que justificou Assembly no nucleo Z80
(`LDIR`/`LDDR`). Diferente do Z80, porem, a corretude aqui e' MUITO mais
facil de errar visualmente sem perceber (um pixel errado nao trava
nada) -- por isso a recomendacao e' **implementar a decodificacao de
pixel primeiro em C/C++ (Fase 2), estabelecer testes de saida
byte-a-byte confiaveis, e so' entao considerar acelerar um modo
especifico em Assembly numa fase posterior**, nunca pular direto pro
Assembly sem uma referencia C ja' validada pra comparar (mesmo principio
usado no core do Z80: LDIR/LDDR so' ganharam ASM depois do loop lento em
C already existir e ser a fonte de verdade pros testes diferenciais).

## 4. Por que nao renderizar pixels de verdade na Fase 1

Mesmo padrao ja' usado nas outras duas fases-zero do projeto (o nucleo
Z80 comecou sem barramento de memoria real; o mapa de memoria comecou
sem carregar ROM): a parte "digital" do VDP (registradores, portas,
interrupcoes) e' testavel e verificavel isoladamente, sem precisar
resolver ainda a questao maior de "como uma janela de verdade vai
mostrar isso" (GLFW/OpenGL, reaproveitando a infra da GUI do msxdisk, ou
algo novo). Separar as duas preocupacoes reduz o risco de a Fase 1
travar numa decisao de UI/janela que nao tem nada a ver com "o VDP gera
a interrupcao de VBlank corretamente?".

## 5. Licenciamento

Mesma regra ja aplicada ao nucleo Z80 e ao mapa de memoria: qualquer
arquivo que adaptar a logica de `MSX.c` (protocolo de porta, VDPOut,
maquina de estados de scanline) ou, em fases futuras, de `V9938.c` (o
motor de comando) deve (1) citar a origem em cabecalho, (2) entrar na
lista de `LICENSE-THIRD-PARTY.md`, (3) ser tratado como nao-comercial
ate' autorizacao explicita em contrario. `VdpDevice`/a integracao com
`IBus`/os comandos de depuracao sao design proprio do fwMSX
(BSD-3-Clause).

**`V9938.c` tem uma nota de atribuicao extra**: o cabecalho do arquivo
credita uma reescrita completa por Alex Wulms (nao so' Fayzullin) --
quando a fase do motor de comando chegar, isso precisa ser documentado
com precisao em `LICENSE-THIRD-PARTY.md` (dois autores, nao um).

## 6. Fases de implementacao propostas

- [x] **Fase 0.5 -- Barramento composto** (concluida em 2026-09-30):
      `z80::CompositeBus` (`src/z80/cpp/composite_bus.h`, header-only,
      design proprio) -- memoria sempre vai para um unico dispositivo
      designado (nao existe "porta de memoria" no MSX real); porta de
      I/O e' despachada por uma tabela de 256 entradas
      (`std::array<IBus*, 256>`), populada via `RegisterPort()`/
      `RegisterPortRange()`. Cada dispositivo registrado recebe o numero
      de porta COMPLETO (nao mascarado) -- `SlotMemoryBus` continuou
      funcionando sem nenhuma mudanca, confirmado por teste (ver Fase
      1 abaixo, teste 7 do `vdptest`).
- [x] **Fase 1 -- VDP "digital"** (concluida em 2026-09-30): `VdpState`
      (C, `src/vdp/core/`) com registradores, protocolo de porta
      `98h`-`9Bh` (incluindo os dois latches de 2 escritas e o
      reconhecimento de interrupcao ao ler o status), VRAM, maquina de
      estados de scanline/interrupcao -- **sem renderizar nenhum pixel**.
      `VdpDevice` (C++, `src/vdp/cpp/`) adapta isso para `z80::IBus`.
      Comandos `vdpregs`/`vdpmem`/`vdppeek`/`vdppoke`/`vdpstep` no
      `--z80dbg --slots --vdp` desde esta fase. Tabela de conversao de
      paleta em Fortran (`src/vdp/fortran/palette_table.f90`, secao
      3.4) -- ainda sem consumidor (Fase 2+).

  **Build/teste** (`PATH="/c/msys64/ucrt64/bin:$PATH"`):
  ```
  cmake -S . -B build -G Ninja
  cmake --build build
  ctest --test-dir build
  ```
  Resultado em 2026-09-30: build limpo (89 alvos), `ctest` verde -- 4
  suites (`z80_smoke` 168, `z80_debug_session` 58, `memmap_slots` 102,
  `vdp_digital` 38 -- 366 verificacoes no total). Regressao confirmada:
  `fwMSX.exe` (sem args), `--z80dbg` (sem `--slots`), `--z80dbg --slots`
  (sem `--vdp`) e `--msxdisk info` -- todos inalterados.

  **Criterio de aceite -- revisado durante a implementacao (leia com
  atencao, e' o achado mais importante desta fase)**: o plano original
  desta secao dizia "rodar a BIOS real e confirmar que ela recebe e
  processa VBlank". Na pratica, isso **nao aconteceu dentro de nenhum
  orcamento de ciclos testado** (ate' 20 milhoes) -- investigando, o PC
  fica preso perto de `0x0C3C`-`0x0C44` cedo no boot da BIOS,
  quase certamente esperando uma resposta de PPI/teclado (portas
  `A9h`/`AAh`/`ABh`) que este projeto ainda nao emula (**correcao de
  2026-10-01: o PPI agora existe (v1.7.0) e o diagnostico estava errado --
  a causa real era o `Z80Cpu` nao inicializar as tabelas de flag (v1.8.0).
  A BIOS real agora habilita o VBlank e chega ao prompt do BASIC; ver
  `doc/ppi-spec.md`, secao 5**) (so' o mapa de
  memoria e o VDP existem ate' agora -- PPI e' trabalho futuro nao
  coberto por este design doc). **Isso e' uma limitacao real e
  esperada, separada da correcao do VDP em si** -- confirmado
  construindo um **programa Z80 sintetico** (12 bytes: liga IE0 via
  R#1, `IM 1`, `EI`, `HALT`) que exercita exatamente o mecanismo VDP ->
  `Z80Cpu::interrupt()` sem depender de nada alem do que esta fase
  entrega -- esse programa RECEBE a interrupcao de verdade (PC chega em
  `0x0038`, o vetor fixo de IM1), provando que o mecanismo funciona de
  ponta a ponta. O teste com a BIOS real foi mantido como
  **informativo, nao-bloqueante** (`tests/z80/vdp_test.cpp`, teste 9) --
  imprime o resultado mas nunca falha o suite, documentando o achado
  para quando PPI/teclado forem implementados e esse teste puder virar
  o criterio de aceite completo de verdade.

  **Outros achados/decisoes durante a implementacao**:
  - **Deteccao de "interrupcao genuina" em teste precisou de duas
    correcoes**: a primeira versao do teste de aceitacao so' checava
    "PC chegou em 0x0038 alguma vez", que da' falso-positivo (a BIOS
    pode ter um `CALL 0038h` legitimo em algum lugar, sem nenhuma
    interrupcao de hardware envolvida). A versao corrigida exige que
    IFF1 estivesse ligado ANTES e desligado DEPOIS da chamada de
    `cpu.interrupt()` -- so' uma interrupcao de verdade faz isso (ver
    `z80_interrupt()`), uma `CALL` normal nunca mexe em IFF1.
  - **Bug real no proprio harness de teste**: o loop de execucao do
    teste de aceitacao copiava o padrao "para em HALT" do teste de
    aceitacao do mapa de memoria (onde fazia sentido, sem VDP nao ha' o
    que esperar) -- mas aqui HALT e' exatamente o estado de ESPERA que
    o programa usa entre `EI` e a interrupcao chegar; parar o loop ali
    matava o teste antes do VDP ter qualquer chance de gerar a
    interrupcao. Corrigido removendo esse `break` no
    `RunTrackingInterrupts()` do `vdp_test.cpp`.
  - **Bit `0x40` do segundo byte da porta `99h`**: 0=leitura (dispara
    pre-busca imediata, `VAddr` ja' avanca so' de SETAR o endereco --
    comportamento real de hardware), 1=escrita (sem pre-busca). Os
    testes iniciais tinham isso invertido -- corrigido apos falharem
    (ver `tests/z80/vdp_test.cpp`, testes 1-2).
  - **VRAM de uma pagina so'** (`VDP_VRAM_PAGES=1`, ver
    `src/vdp/common/vdp_types.h`): simplificacao deliberada. O efeito
    colateral de "muda de pagina" em `regs[14]` continua no codigo
    (fielmente portado), so' vira um no-op observavel com 1 pagina --
    nao removido, so' inofensivo por enquanto.
  - **`--vdp` sem `--slots`**: ignorado com aviso (`vdp_error` em
    `Z80DebugShellStartup`), sessao continua em RAM plana -- mesmo
    espirito "avisa, nao trava" ja usado para ROM de boot invalida.
  - **`step` tambem avanca o VDP** (nao so' `run`) -- decisao tomada por
    consistencia (os dois usam a mesma logica de `DriveVdp()`).
- [x] **Fase 2 -- Renderizacao MSX1** (concluida em 2026-09-30):
      decodificacao de pixel de verdade para SCREEN 0 (TEXT 40x24),
      SCREEN 1 (TEXT 32x24 com cor) e SCREEN 2 (256x192 bitmap),
      adaptada de `RefreshLine0/1/2` em `resource/fMSX/fMSX/Common.h`.
      Exportacao PPM (P6 binario, sem biblioteca externa) para
      inspecao/testes -- comando `vdpshot <arquivo> [linha_ini]
      [linha_fim]` no `--z80dbg --slots --vdp`.

  **Arquivos**: `src/vdp/core/vdp_render.{h,c}` (renderizacao, adaptada
  de `Common.h`), `src/vdp/core/vdp_tables.{h,c}` (tabela global de
  paleta, ate' agora sem consumidor -- ver abaixo), `src/vdp/cpp/
  ppm_writer.{h,cpp}` (exportacao PPM, design proprio); `vdp_state.c`
  ganhou o comando `vdpshot` em `z80_debug_session.cpp`.

  **Dimensoes de framebuffer** (verificadas contra `Common.h`, nao
  adivinhadas): SCREEN 0 = **240x192** (40 colunas * 6px/caractere --
  `RefreshLine0` so' escreve 6 dos 8 bits do glifo por caractere, bits
  7..2; os outros 16px que o original preenche com a cor de fundo sao
  preenchimento de BORDA do "slot" de video de 256px, fora de escopo
  aqui). SCREEN 1/2 = **256x192** (32 colunas * 8px/caractere, os 8
  bits completos do glifo).

  **Simplificacoes deliberadas** (ver secao 4 e a nota de topo de
  `vdp_render.h`): SEM borda/overscan (`RefreshBorder()` inteiro fica
  de fora -- so' a area ativa e' desenhada); SEM sprites
  (`Sprites()`, Fase 3); SEM tratamento de `ScreenON=0` alem de mostrar
  a cor de fundo solida; SEM `FontBuf`/`MSX_FIXEDFONT` (recurso de
  conveniencia do fMSX pra substituir a fonte por uma do host, nao
  existe no hardware real); `VScroll` = `regs[23]`, ja' portado desde a
  Fase 1 (usado tal qual). Modos fora de {0,1,2}: preenchem a linha
  inteira com a cor de fundo em vez de decodificar pixels errados ou
  deixar memoria nao-inicializada.

  **Bug real encontrado e corrigido nesta fase**: `vdp_reset()` zerava
  `palette_r/g/b[]` (Fase 1 nunca tinha um consumidor pra notar isso).
  Como software SCREEN 0/1/2 real quase nunca escreve os registradores
  de paleta (recurso do V9938+, nao existe no TMS9918/MSX1), sem essa
  correcao TODO pixel renderizado sairia preto. Corrigido portando
  `PalInit[16]` de `MSX.c` (~linha 687) para o reset -- testado contra
  as 16 entradas completas, nao so' uma amostra.

  **Tabela de paleta em Fortran, finalmente consumida**: a Fase 1
  construiu `palette_table.f90` (512 entradas) sem nenhum consumidor;
  a escrita de paleta via porta `9Ah` agora usa essa tabela (busca por
  indice) em vez de recalcular a formula de escala inline a cada
  escrita -- mesmos valores, calculados uma unica vez. Verificado que
  os testes de paleta da Fase 1 continuam passando byte-a-byte
  identicos apos essa troca.

  **Testes**: 16 novas verificacoes (38 -> 54 no total de `vdptest`) --
  as 16 entradas de paleta padrao; um glifo conhecido em SCREEN 0
  (pixels exatos, nao so' "nao esta tudo preto"); o quirk de cor
  compartilhada por GRUPO DE 8 CODIGOS de caractere em SCREEN 1 (dois
  codigos diferentes no mesmo grupo devem renderizar com a MESMA cor,
  contrastado com enderecos de tabela de cor "por posicao" que dariam
  cores diferentes se a formula estivesse errada); a mascara `Y&0xC0`
  de SCREEN 2 (dois "tercos" da tabela de cor/padrao devem dar
  enderecos DIFERENTES quando a config de registrador permite tabela
  unica por terco -- achado depurando: a config inicial escolhida por
  engano selecionava o modo "tabela compartilhada/espelhada" do
  hardware real, onde os dois tercos leem o MESMO endereco de
  proposito, o que fazia o teste falhar por causa da configuracao de
  teste errada, nao por bug no renderizador -- corrigido ajustando os
  registradores do teste, nao a formula); fallback de modo nao
  suportado; round-trip completo de exportacao PPM (escreve, le de
  volta, compara byte a byte).

  **Build/teste**: `cmake --build build --target vdptest z80dbgtest
  memmaptest z80test fwMSX && ctest --test-dir build` -- 4 suites, 382
  verificacoes no total (168+58+102+54), todas passando. Verificacao
  manual via `fwMSX.exe --z80dbg --slots --vdp` (`vdpshot` gerando um
  PPM real, cabecalho conferido byte a byte).
- [x] **Fase 3 -- Sprites de modo 1** (concluida em 2026-10-01): sprites
      do TMS9918 em SCREEN 1/2/3 -- 8x8/16x16, ampliacao 2x, prioridade
      por indice, cor 0 transparente, early clock (bit 7 do atributo),
      terminador Y=208, limite de 4 por linha, flag/numero do "quinto
      sprite" e flag de colisao. Adaptado de `Sprites()`/`CheckSprites()`
      do fMSX. **Sprites de modo 2** (`ColorSprites()`, SCREEN 4-8, cor
      por linha, 8 por linha) ficam para a Fase 4, junto com os modos
      MSX2 -- nao fazem sentido sem eles.

  **Arquivos**: `src/vdp/core/vdp_sprites.{h,c}` (C, adaptado do fMSX);
  `vdp_render_line()` chama `vdp_sprites_draw_line()` por cima do fundo
  em SCREEN 1/2; `vdp_step_scanline()` ganhou o efeito de status de
  `Sprites()` por linha visivel e a checagem de colisao na linha 192
  (S#0 bit 5; o bit 6/bits 4-0 do quinto sprite sao limpos/reiniciados
  nessa linha, como no original).

  **Decisoes/simplificacoes**: recorte pixel a pixel em vez das mascaras
  de bits do original (mesmo resultado, verificado nos casos de borda);
  sem `MSX_ALLSPRITE` (5o sprite nunca e' desenhado, como o hardware);
  `ScreenON=0` nao esconde sprites (coerente com a Fase 2); a colisao
  ignora a ampliacao, como o fMSX; quirk preservado do fMSX: VScroll
  (R#23) e' somado duas vezes ao Y dos sprites em SCREEN 1 (irrelevante
  com R#23=0). A renderizacao e' `const` -- o status de sprite e'
  atualizado so' por `vdp_step_scanline()`, nunca por `vdpshot`.

  **Testes**: +20 verificacoes em `vdptest` (54 -> 74; 402 no total das 4
  suites): janela vertical exata Y+1..Y+8, prioridade, cor 0, early
  clock, recorte na borda direita, Y negativo, terminador 208, R#8
  SPD, 16x16 (indice & 0xFC, quadrantes), ampliado, quinto sprite
  (flag + numero + nao desenhado + limpeza), colisao (adjacentes,
  sobrepostos, distantes, dv=8 vs dv=7) e ponta a ponta via
  `vdp_step_scanline()`/leitura da porta 99h. Dois erros de teste (nao
  de codigo) achados no processo: a linha 0 de um sprite com Y=250 mostra
  a linha 5 do padrao (nao a 6), e padroes de colisao precisam ter todas
  as 8 linhas preenchidas.
- [x] **Fase 4 -- Modos MSX2 + janela real** -- **concluida em 2026-10-02**
      (a janela veio na v1.9.0; os modos agora): VRAM de 128KB com paginas
      (R#14), SCREEN 3 (multicolor), 4, 5, 6, 7, 8 e TEXT80, 212 linhas, cor 0
      transparente, tela desligada, **sprites de modo 2** (cor por linha, CC/EC,
      9o sprite, colisao). Adaptado de `Common.h`/`Wide.h` do fMSX; SCREEN 6/7 e
      TEXT80 em 512 pixels de verdade. Ver [msx2-spec.md](msx2-spec.md).
- [x] **Fase 5 -- Motor de comando V9938** -- **concluida em 2026-10-02**:
      POINT/PSET/SRCH/LINE/LMMV/LMMM/LMCM/LMMC/HMMV/HMMM/YMMM/HMMC, adaptado de
      `V9938.c` (atribuicao dupla Fayzullin/Wulms), com a temporizacao por scanline
      do fMSX e o handshake TR. Desvio: o TR comeca limpo a cada comando.
      **Feito na v1.15.0 (V9958, ver `doc/msx2p-spec.md`)**: SCREEN 10-12 (YJK/YAE),
      rolagem horizontal (R#26/R#27) em 5-8 e YJK/YAE, mascara da esquerda (R#25 bit 1).

*(Cada fase sera detalhada em sub-fases, como aconteceu em
`doc/z80-core-spec.md` e `doc/memory-map-spec.md`, no momento em que a
implementacao comecar.)*
