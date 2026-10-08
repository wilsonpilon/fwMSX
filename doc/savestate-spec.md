# Save-state -- especificacao (documento vivo)

> Mesmo espirito de `doc/memory-map-spec.md` e `doc/sram-spec.md`: o que foi
> feito, as decisoes e o que falta, para retomar do ponto exato onde parou.

Estado: **concluido em 2026-10-08** (item 5 de "fazer o 1, o 4 e o 5 na
sequencia", pedido pelo usuario -- o ultimo da lista). `Machine::SaveState()`/
`Machine::LoadState()` (`src/machine/machine.{h,cpp}`) gravam/restauram um
retrato do estado AO VIVO da maquina num arquivo binario proprio do fwMSX.

## 1. Decisao de design: sobrepor na maquina JA' RODANDO, nao recriar do zero

A alternativa mais "completa" seria serializar o `MachineConfig` inteiro
(caminhos de BIOS/cartucho/disco/fita + layout de slots) e, ao carregar, criar
uma `Machine` NOVA com `Machine::Create()` e so' depois aplicar o estado por
cima. Decisao tomada: **nao fazer isso em v1**. `LoadState()` aplica direto
sobre a `Machine` que ja esta' rodando -- pressupoe que a MESMA midia (BIOS,
cartucho, disco, fita) continua carregada, exatamente como a maioria dos
emuladores reais funciona (carregar o estado errado sobre o jogo errado
produz imagem/som estranhos, nunca um erro reportado). Isso elimina toda a
complexidade de serializar/validar caminhos de arquivo, reconstruir o layout
de slots, e tratar o caso "o cartucho nao esta' mais no mesmo lugar" -- sem
perder nada que o uso real (salvar e recarregar o MESMO jogo, na MESMA
sessao ou numa sessao seguinte com o mesmo cartucho/disco inseridos) precisa.

## 2. Formato do arquivo

Cabecalho fixo, seguido de secoes TLV (tag de 4 bytes + tamanho de 4 bytes +
payload) ate o fim do arquivo -- mesmo raciocinio do TZX (ver
`doc/tape-spec.md`): uma versao futura pode acrescentar secoes novas sem
quebrar leitores antigos (secao desconhecida e' so' pulada, nao um erro).

```
Cabecalho:
  char magic[8]       = "FWMSXSST"
  uint32 format_version = 1
  uint8  model         (0=MSX1, 1=MSX2, 2=MSX2+)
  uint64 frame_count    (informativo)

Secoes (tag + uint32 tamanho + payload):
  "Z80 "  registradores do Z80 (pc,sp,af,bc,de,hl,ix,iy + sombra + i,r,iff)
  "VDP "  VdpState ate' scanline_snapshot (regs/paleta/VRAM/modo/scanline/...)
  "PSG "  PsgState, pulando joy[]/cassette_in (ver secao 3)
  "SCC "  SccState inteiro
  "OPLL"  Ym2413State inteiro
  "PPI "  registradores do PPI (r/rout/rin), sem key_state
  "FDCM"  registradores do WD2793 pela memoria (se ligado)
  "FDCP"  registradores do WD2793 pelas portas (se ligado)
  "TAPE"  modo, protecao, modo de gravacao, rele do motor, ponto marcado
  "MAPR"  registrador de segmento (4 paginas) de cada RamMapperDevice
  "RAM "  (repetida) conteudo de cada combinacao primario:secundario que e'
          RAM comum ou RAM de mapper -- primario, secundario, tamanho, bytes
```

**NAO e' um formato portavel entre sistema operacional**: a maioria das
secoes grava o struct quase cru (so' excluindo os campos explicados na secao
3) -- o layout de padding pode diferir entre MSVC (build Windows) e GCC
(build Linux). Um estado so' e' garantido carregar de volta no MESMO
executavel (ou pelo menos no mesmo compilador/SO) que o salvou. Decisao
deliberada (ver a licao do `doc/vdp-spec.md`/outros documentos deste
projeto: simplificar quando o ganho real e' pequeno) -- fazer serializacao
campo a campo 100% portavel multiplicaria o codigo para um cenario raro
(ningeum costuma copiar um save do Windows pro Linux do MESMO projeto
pessoal).

## 3. O que fica DE FORA, e por que

- **Teclas pressionadas e joystick** (`PpiState::key_state`,
  `PsgState::joy[]`/`cassette_in`): entrada do MUNDO EXTERNO, nao estado da
  maquina -- carregar um save nao deve "replay" uma tecla que estava
  pressionada no instante do save. Preservados do estado AO VIVO ao
  carregar (nunca sobrescritos).
- **CMOS do RTC** (`doc/msx2-spec.md`): preferencias da BIOS (cor de tela,
  largura, ajuste), nao estado de jogo -- e ja' e' um limite aceito do
  projeto que a CMOS nao persiste entre execucoes. Como `LoadState()`
  sobrepoe na MESMA maquina ja rodando (ver secao 1), o RTC nunca e'
  reconstruido do zero de qualquer forma (continua com o que ja tinha).
- **SRAM de cartucho/FM-PAC**: ja persiste sozinha, em arquivo `.sav` proprio
  (ver `doc/sram-spec.md`) -- incluir de novo no save-state duplicaria dado.
- **Conteudo de disco e de fita**: continuam arquivos `.dsk`/`.cas`/`.tsx`
  separados, gravados direto pelo emulador como sempre -- so' os campos
  pequenos que decidem o comportamento do PROXIMO acesso (ver a lista de
  "TAPE" acima) fazem parte do save-state.
- **Transferencia de disco/fita EM ANDAMENTO no instante exato do save**: o
  ponteiro de posicao dentro da imagem (`Fdc::ptr`) nao e' salvo (aponta pra
  dentro de um buffer que so' existe na maquina viva) -- se uma
  transferencia estava no meio, `LoadState()` zera `wr_length`/`rd_length`
  (aborta a transferencia de forma segura) em vez de arriscar um ponteiro
  invalido. Limitacao aceita: na pratica, salvar com MSX-DOS no prompt
  (o caso comum) nunca tem uma transferencia em andamento.
- **Campos de bookkeeping do nucleo Z80** (`iperiod`/`icount`/`ibackup`/
  `irequest`/`iautoreset`/`trapbadops`/`user_data`): internos do
  interpretador entre chamadas de `z80_run()`, nunca "estado de jogo" --
  `user_data` em especial e' um PONTEIRO do host, nunca deve ser
  sobrescrito por um valor de arquivo.
- **O array `VdpState::scanline_snapshot[]`**: derivado, recomputado do zero
  a cada `RunFrame()` (ver `doc/vdp-spec.md`, a feature de efeitos de
  rastreio, 1.27.0) -- nao faz sentido salvar ~46KB de algo que a proxima
  chamada de `RunFrame()` ja' substitui.

## 4. RAM de mapper: por que `MapperRamBase()`, nao `PeekSlot`/`PokeSlot`

`MemorySystem::PeekSlot()`/`PokeSlot()` so' enxergam a VISTA PAGINADA atual
(os 4 segmentos de 16KB que estao visiveis agora em 0000h-FFFFh) -- para uma
RAM de mapper de ate 1024KB (64 segmentos), isso deixaria de fora tudo que
nao esta' paginado NESTE EXATO MOMENTO. `MemorySystem::MapperRamBase()`
(novo, `src/memmap/cpp/memory_system.h`) devolve o ponteiro para o buffer
INTEIRO (todos os segmentos), permitindo salvar/restaurar o mapper completo
de uma vez, independente do que esta' paginado no instante do save. Para RAM
COMUM (nao-mapper, ate 64KB), `PeekSlot`/`PokeSlot` continuam suficientes
(o endereco inteiro ja' cabe na janela de 64KB).

## 5. Interface (menu Arquivo)

`src/machine/gui/emu_window.cpp`: **Arquivo > Salvar estado...**/**Carregar
estado...**, com dialogo de arquivo (`.sst`) -- substitui o item desabilitado
"Salvar estado (em breve)" que ja existia como placeholder desde a janela com
menus (1.16.0). Sem atalho de teclado nem slots numerados em v1 (o dialogo de
arquivo ja permite multiplos saves, cada um com seu proprio nome, do mesmo
jeito que "Nova fita..." ja funciona). Sem opcao de linha de comando (`--cart`/
`--disk`/`--fita` tem porque fazem parte do BOOT da maquina; carregar um
estado so' faz sentido com a maquina ja' rodando).

## 6. Testes

`tests/z80/machine_test.cpp`, secao 5b (CTest `machine_frames`, executavel
`machinetest`): salva o estado depois do boot + `print 1234`; MUTILA a
maquina de proposito (300 quadros a mais, `cls`, PC e R7 do PSG escritos
direto para valores errados) para provar que a restauracao NAO e' coincidencia;
recarrega e confere PC do Z80, R7 do PSG e a VRAM inteira (incluindo o texto
`print 1234` na tela) byte a byte; e os dois caminhos de erro (arquivo
inexistente, arquivo sem a assinatura certa).

**Nao testado automaticamente** (ficaria complexo demais para o retorno, ver
secao 7): RAM de mapper (precisaria montar um layout MSX2 com mapper no
teste), FDC/fita em transferencia, SCC/OPLL/PPI especificamente (usam a
MESMA tecnica ja' provada para Z80/VDP/PSG, risco baixo).

## 7. O que falta

- **Validado na janela de verdade em 2026-10-08**: o usuario confirmou que
  salvar e carregar estado (**Arquivo > Salvar estado.../Carregar
  estado...**) funcionam normalmente pela GUI real.
- **RAM de mapper**: logica implementada (`MapperRamBase()`, secao "MAPR"),
  mas sem teste automatizado dedicado (precisaria de uma maquina MSX2 com
  layout de mapper no `machine_test.cpp`).
- **Slots e atalhos (feitos na 1.30.1)**: 9 slots (`fwmsx-estado-N.sst`), F6 salva / F7 carrega / F8-F9 trocam de slot, com aviso na tela. **Validacao da midia**: secao `MEDA` (CRC32 da BIOS e do cartucho) -> `Machine::state_warning()`; disco e fita nao entram.
- (historico) Slots NOMEADOS pelo usuario continuam fora.
- **Multiplos slots nomeados** (a parte de atalho ja foi feita; antes: F5 salva, F7 carrega,
  como muitos emuladores): nao implementado -- o dialogo de arquivo ja cobre
  "varios saves" (cada um com seu nome), mas nao e' tao rapido quanto um
  atalho.
- **Validar o cartucho/disco/fita contra o save**: `LoadState()` nao avisa
  se o usuario carregar um estado salvo com um JOGO DIFERENTE do que esta'
  inserido agora -- produz imagem/som estranhos, nao um erro (ver secao 1).
  Um aviso (comparando CRC32 do cartucho, por exemplo) e' melhoria futura.
