# Mapa de memoria MSX (slots/subslots) -- especificacao (documento vivo)

> Mesmo espirito de `doc/z80-core-spec.md`: analise do fMSX, decisoes de
> design e fases de implementacao, para retomar o trabalho do ponto exato
> onde parou. Nao remova secoes de fases concluidas -- marque como feitas
> e adicione as novas por baixo.

Estado desta secao: **Fase 1 concluida em 2026-09-30** (ver secao 6). Fases
2-4 ainda nao iniciadas.

## 1. Objetivo

Dar ao nucleo Z80 (`doc/z80-core-spec.md`, Fases 1-4, concluidas) um
barramento de memoria de verdade -- hoje o `--z80dbg` roda sobre uma RAM
plana de 64KB de teste (`FlatMemoryBus`), sem nenhuma noção de slot. Um
MSX real enderece ate **4 slots primarios × 4 slots secundarios × 8
paginas de 8KB** (64KB de espaço de endereçamento do Z80, mas varias
"camadas" de conteudo por baixo, trocadas por hardware) -- é isso que
permite ter BIOS + RAM + cartucho MegaROM convivendo no mesmo mapa de 64KB
do Z80, trocando de banco em tempo real.

**Requisito explicito do autor, tratado como vital desde a Fase 1 deste
modulo (não deixado para depois, ao contrário do padrão do núcleo Z80,
onde o depurador só chegou na Fase 4)**: o depurador embutido em
`fwMSX.exe` precisa conseguir **ver todos os slots e subslots**, não só o
que está atualmente visível para a CPU. Isso é central para depurar
software MSX real -- um MegaROM troca bancos o tempo todo, e a BIOS
continua existindo no slot 0 mesmo quando um cartucho está mapeado por
cima dela nas páginas visíveis.

## 2. O que a análise de `resource/fMSX/fMSX/MSX.c`/`MSX.h` revelou

Arquivos lidos (parcial, focado no subsistema de slots -- `MSX.c` tem
109KB e cobre PPI/teclado/VDP/FDC/som também, fora do escopo desta
análise): declarações de slot em `MSX.h`, `RdZ80`/`WrZ80` (~linha
1000-1074), `PSlot()`/`SSlot()` (~linha 1749-1794), inicialização de slots
em `ResetMSX` (~linha 898-927).

- **Tabela central**: `byte *MemMap[4][4][8]` -- um ponteiro por
  (slot primário, slot secundário, página de 8KB). Cada ponteiro aponta
  para um bloco de memória do host (ROM carregada, RAM alocada, ou um
  sentinela `EmptyRAM` para slot vazio -- leitura retorna lixo/`0xFF`,
  escrita é descartada). É a "verdade" sobre o que existe em cada
  combinação de slot -- 16 combinações possíveis, a maioria vazia na
  prática.
- **Vista ativa (cache rápido)**: `RAM[8]` -- só 8 ponteiros, o que a CPU
  enxerga *agora*. Recalculado toda vez que o slot primário (porta `A8h`,
  `PSlot()`) ou secundário (endereço `FFFFh`, `SSlot()`) muda:
  `RAM[i] = MemMap[PSL[pagina]][SSL[pagina]][i]`. É a mesma técnica de
  "ponteiro direto pré-calculado" que o `FAST_RDOP` do `Z80.c` usa para
  opcode fetch -- aqui aplicada ao barramento de memória inteiro.
  `RdZ80`/`WrZ80` leem/escrevem **só** nessa vista ativa
  (`RAM[A>>13][A&0x1FFF]`), nunca em `MemMap` diretamente -- por isso um
  depurador que só olha para o que o Z80 vê fica cego para os outros
  15/16 combinações de slot.
- **`EnWrite[4]`**: um flag de permissão de escrita por página (16KB),
  verdadeiro só quando aquela página mostra RAM de verdade (fMSX fixa a
  RAM principal do sistema em **slot primário 3, secundário 2**, por
  convenção própria da inicialização -- não é uma regra universal do
  hardware MSX, é a topologia que o fMSX monta). Fora disso (ROM, slot
  vazio), escrita é ignorada ou tratada como troca de banco MegaROM (ver
  abaixo).
- **`PSLReg`/`PSL[4]`**: o byte bruto da porta `A8h` (2 bits por página,
  4 páginas de 16KB) e sua decodificação por página.
- **`SSLReg[4]`/`SSL[4]`**: um registrador de slot secundário **por slot
  primário** (não por página!) -- quirk real do hardware MSX: o
  registrador de slot secundário mora fisicamente dentro do slot primário
  "expandido" que ocupa a página 3 (`C000h-FFFFh`) no momento da escrita
  (`SSLReg[PSL[3]]`), não em um registrador global único. Leitura em
  `FFFFh` devolve o complemento (`~SSLReg[PSL[3]]`) -- outro detalhe de
  hardware que scripts/jogos de proteção por vezes exploram.
- **Slots sem subslot**: cartuchos (`PSL[3]==1` ou `2`) e, em MSX1, o
  próprio slot 0, não têm subslots -- `SSlot()` força `V=0` nesses casos
  antes de decodificar.
- **`CartMap[4][4]`**: traduz uma coordenada (primário, secundário) para
  qual "slot de cartucho" físico (0..5, `MAXSLOTS`) está ali plugado --
  usado por `SetMegaROM()`/`LoadCart()` para achar onde uma ROM carregada
  deve aparecer no mapa.
- **MegaROM (bank switch)**: `ROMMapper[slot][4]` guarda a página de 8KB
  atualmente visível para cada um dos 4 quartos de 16KB de espaço de
  ROM; `ROMMask[slot]` mascara o número de páginas disponível (wraparound
  de banco); `ROMType[slot]` seleciona o tipo de mapper -- `MAP_GEN8`,
  `MAP_GEN16`, `MAP_KONAMI5`, `MAP_KONAMI4`, `MAP_ASCII8`, `MAP_ASCII16`,
  `MAP_GMASTER2`, `MAP_FMPAC`, `MAP_GUESS` (heurística automática por
  tamanho/conteúdo do arquivo). Escrever num endereço de ROM
  (`WrZ80`, quando `EnWrite` é falso e o endereço cai entre `4000h` e
  `BFFFh`) aciona `MapROM()` (não lida em detalhe nesta análise --
  fica para a Fase 3 de implementação, ver seção 6) que decodifica o
  endereço/valor escrito conforme `ROMType` e ajusta `ROMMapper`/
  `MemMap`.

## 3. Arquitetura proposta

### 3.1 Estrutura de diretórios

```
src/memmap/
├── core/       C -- SlotState (equivalente a MemMap/PSL/SSL/SSLReg/
│               EnWrite/RAM[8]), troca de slot primario/secundario,
│               classificacao de conteudo por slot (vazio/RAM/ROM)
├── cpp/        C++ -- MemorySystem (orquestracao): carregar ROM/RAM num
│               slot, adaptador para IBus do nucleo Z80 (SlotMemoryBus),
│               API de inspecao "fora da vista da CPU" (ver 3.3)
├── fortran/    Fortran -- checksum/CRC de imagens de ROM carregadas
│               (auto-deteccao heuristica de mapper por assinatura, uma
│               vez por carga de arquivo -- fora do caminho quente)
└── common/     cabecalhos compartilhados (constantes de tipo de mapper,
                layout de SlotState)
```

**Sem pasta `asm/` nesta fase, de proposito**: a troca de slot em si é
fundamentalmente busca em tabela de ponteiros (`RAM[i] = MemMap[...]`),
não processamento de dados -- não há um loop quente genuíno aqui para
acelerar em Assembly, ao contrário do `LDIR`/`LDDR` do núcleo Z80. A
sinergia com Assembly já existente **é reaproveitada, não recriada**:
uma vez que `SlotMemoryBus` exista, seu `ram_ptr()` (a extensão opcional
de `Z80Bus`/`IBus` criada na Fase 3 do núcleo Z80) passa a devolver
ponteiros reais dentro da página de 8KB atualmente visível, e o
acelerador `z80_fast_block_move` (já existente, já testado) passa a
valer de verdade para `LDIR`/`LDDR` que cruzem regiões de RAM mapeada --
sem precisar de nenhum código Assembly novo. Documentar essa decisão
aqui evita que uma fase futura invente ASM artificial só para "ter ASM".

### 3.2 `SlotState` (C) -- o motor

Adaptado de `MSX.c`/`MSX.h` (registro aqui, não invento um design do
zero -- a topologia 4×4×8 e a técnica de vista-ativa-em-cache são
soluções de hardware/emulação bem estabelecidas, replicar é o caminho
certo):

- `uint8_t *chunk[4][4][8]` -- equivalente a `MemMap`. Cada ponteiro
  aponta para um bloco de 8KB alocado pelo host, ou para um sentinela
  `kEmptySlot` (leitura = `0xFF`, escrita descartada).
- `uint8_t chunk_writable[4][4][8]` -- **diferença deliberada do fMSX**:
  em vez de inferir "é RAM escrevível" via uma regra hardcoded (fMSX
  assume que RAM sempre mora em 3:2), guardamos a permissão de escrita
  por chunk explicitamente. Isso deixa o sistema livre para colocar RAM
  em qualquer combinação de slot (útil para testes e para não herdar a
  suposição "RAM é sempre 3:2" como se fosse regra de hardware, quando
  na verdade é só a topologia que o fMSX monta por convenção própria).
- `uint8_t psl[4]`, `uint8_t ssl_reg[4]`, `uint8_t ssl[4]`,
  `uint8_t en_write[4]` -- mesmo papel que `PSL`/`SSLReg`/`SSL`/
  `EnWrite` do fMSX.
- `uint8_t *active_view[8]` -- equivalente a `RAM[8]`, a vista atual da
  CPU, recomputada em `memmap_switch_primary(state, value)` (porta
  `A8h`) e `memmap_switch_secondary(state, value)` (endereço `FFFFh`),
  portados 1:1 da lógica de `PSlot()`/`SSlot()` (incluindo o quirk do
  registrador secundário ser indexado por `psl[3]`, e slots sem subslot
  --cartucho, ou slot 0 em MSX1 -- forçando `V=0`).
- `uint8_t memmap_read(SlotState*, uint16_t addr)` /
  `void memmap_write(SlotState*, uint16_t addr, uint8_t value)` --
  equivalentes a `RdZ80`/`WrZ80`, mas **sem** a parte específica de
  hardware do fMSX (FDC em `7FF8h`/`BFF8h` etc. -- isso pertence a um
  controlador de disquete que não existe ainda no fwMSX; fica marcado
  como TODO explícito para quando `msxdisk`/FDC entrarem em cena, não
  omitido silenciosamente).

### 3.3 `MemorySystem` (C++) -- orquestração e API de depuração

- Carregamento: `LoadRom(primary, secondary, data, mapper_type)`,
  `AllocateRam(primary, secondary, size)` -- monta `chunk[][][]` a partir
  de um arquivo ou de RAM alocada, decidindo os ponteiros de 8KB.
- `SlotMemoryBus : public z80::IBus` -- adapta `SlotState` para a
  interface que `Z80Cpu` já entende (`read`/`write` delegam para
  `memmap_read`/`memmap_write`; `out` intercepta a porta `A8h` chamando
  `memmap_switch_primary`; `write` intercepta o endereço `FFFFh` chamando
  `memmap_switch_secondary` antes de cair no caminho normal; `ram_ptr`
  devolve um ponteiro real quando o intervalo pedido cai inteiro dentro
  de um único chunk de 8KB da vista ativa -- nunca atravessando fronteira
  de chunk, mesmo que dois chunks vizinhos por acaso sejam contíguos em
  memória do host, para não depender de um detalhe de alocação).
- **API de inspeção fora da vista da CPU (o requisito "vital" do autor,
  Fase 1 deste módulo, não adiado)**:
  - `uint8_t PeekSlot(int primary, int secondary, uint16_t addr) const`
    / `void PokeSlot(int primary, int secondary, uint16_t addr, uint8_t value)`
    -- lê/escreve em qualquer combinação de slot, **independente** do que
    está mapeado agora para a CPU.
  - `SlotDescriptor Describe(int primary, int secondary) const` --
    `{ kind: Empty|Ram|Rom, size, mapper_type_name (se Rom) }`, para o
    depurador listar o que existe em cada uma das 16 combinações.
  - `CurrentView() const` -- devolve, para cada uma das 4 páginas de
    16KB do Z80, `{ primary, secondary, writable }` -- "o que a CPU
    enxerga agora", equivalente a `PSL`/`SSL`/`EnWrite` legíveis de fora.

### 3.4 Fortran -- checksum/detecção de mapper

`src/memmap/fortran/rom_checksum.f90`: calcula um checksum/CRC simples
sobre os bytes de uma ROM recém-carregada, uma única vez no momento do
`LoadRom()` -- trabalho numérico sobre um array de bytes, exatamente o
tipo de tarefa que já justificou Fortran na Fase 2 do núcleo Z80
(cálculo pontual, fora do caminho quente, sem regressão de desempenho
possível já que roda uma vez por carga de arquivo, não por instrução
Z80 executada). Usado para ajudar a heurística `MAP_GUESS` (detecção
automática de tipo de mapper) a decidir com mais confiança do que só
tamanho de arquivo.

### 3.5 Depurador (`--z80dbg`) -- comandos novos, Fase 1

Novos comandos em `Z80DebugSession` (não substituem os existentes --
`FlatMemoryBus` continua disponível, ver seção 4):

- `slots` -- tabela das 16 combinações (primário × secundário): vazio,
  RAM, ou ROM + tipo de mapper.
- `pages` -- o que está visível *agora* para a CPU em cada uma das 4
  páginas de 16KB (primário, secundário, gravável ou não) -- equivalente
  a inspecionar `PSL`/`SSL`/`EnWrite` do fMSX.
- `slotmem <primario> <secundario> <endereco> [tamanho]` -- dump
  hexadecimal de uma combinação de slot específica, **sem trocar** a
  vista ativa da CPU -- o ponto central do requisito do autor.
- `slotpeek <primario> <secundario> <endereco>` /
  `slotpoke <primario> <secundario> <endereco> <byte>` -- leitura/escrita
  pontual na mesma lógica.

## 4. Por que não substituir `FlatMemoryBus` agora

`FlatMemoryBus` (Fase 4 do núcleo Z80) continua existindo e sendo usada
pelos testes já escritos (`z80test`, `debug_session_test`) -- trocar a
base desses testes para o novo `SlotMemoryBus` só porque ele existe
adicionaria uma dependência de slot/ROM a testes que hoje só precisam de
uma RAM plana simples, sem ganho real de cobertura. `--z80dbg` passa a
aceitar **as duas** (`--flat`, comportamento atual, e o novo padrão
slot-aware quando um mapa de memória for carregado), decisão a refinar na
Fase 4 deste módulo (ver seção 6) quando houver um caso de uso real (ex.:
carregar uma BIOS) para decidir o comportammento default sem argumentos.

## 5. Licenciamento

Mesma regra já aplicada ao núcleo Z80 (`LICENSE-THIRD-PARTY.md`):
qualquer arquivo que adaptar a lógica de `MSX.c`/`MSX.h` (a topologia de
`MemMap`, a lógica de `PSlot()`/`SSlot()`, as constantes `MAP_*`) deve (1)
citar a origem em um comentário de cabeçalho, (2) ser adicionado à lista
de `LICENSE-THIRD-PARTY.md`, (3) ser tratado como não-comercial até
autorização explícita em contrário. `MemorySystem`/`SlotMemoryBus`/a API
de depuração são design próprio do fwMSX (BSD-3-Clause) -- não existem
no fMSX (que não tem um depurador com essa granularidade de inspeção).

**Nota separada, importante**: a BIOS real do MSX (a ROM em si, não o
código que a carrega) é propriedade da Microsoft/ASCII/MSX Licensing
Corporation, não do Fayzullin -- o aval do fMSX cobre adaptar o *código*
dele, não nos dá direito sobre a *BIOS*. Carregar/testar contra uma BIOS
real (Fase 2, seção 6) exige que o próprio autor tenha uma cópia
legalmente obtida; o fwMSX não deve redistribuir nenhuma BIOS.
`resource/fMSX/ROMs/` já existe no repositório como material de
estudo/referência (ver `resource/README.md`) -- usar essas ROMs para
testar localmente é diferente de redistribuir binários do emulador com
elas embutidas, o que nunca deve acontecer.

## 6. Fases de implementação propostas

- [x] **Fase 1 -- Motor de slots em C + `MemorySystem`/`SlotMemoryBus`
      em C++, com os comandos de depuração desde o início** (concluída em
      2026-09-30): `SlotState` (C), troca de slot primário/secundário
      fiel ao fMSX, só RAM (sem carregamento de ROM ainda) para validar a
      mecânica isoladamente; `slots`/`pages`/`slotmem`/`slotpeek`/
      `slotpoke` no `--z80dbg --slots`.

  **Arquivos**: `src/memmap/common/memmap_types.h`;
  `src/memmap/core/{slot_state.h,slot_state.c}`;
  `src/memmap/cpp/{memory_system.h,memory_system.cpp,slot_memory_bus.h}`.
  Refatoração em `src/z80/debug/{z80_debug_session.h,.cpp}` (a sessão
  deixou de possuir sua própria `FlatMemoryBus`, agora recebe qualquer
  `z80::IBus` por referência + um `memmap::MemorySystem*` opcional) e em
  `src/z80/debug/{z80_debug_shell.h,.cpp}`/`src/cpp/main.cpp` (`--z80dbg
  --slots` liga o mapa de memória real). Testes novos:
  `tests/z80/memmap_test.cpp` (alvo `memmaptest`, CTest `memmap_slots`,
  27 verificações) + `tests/z80/debug_session_test.cpp` atualizado (as 35
  verificações da Fase 4 do núcleo Z80 continuam passando sem mudança de
  comportamento, só a linha de construção da sessão mudou).

  **Build/teste**: `cmake --build build --target z80test z80dbgtest
  memmaptest fwMSX` seguido de `ctest --test-dir build -R "z80|memmap"`
  -- verde (168 + 35 + 27 = 230 verificações). Testado também um build
  limpo do zero (diretório novo) para garantir que nenhuma dependência
  incremental estava mascarando um arquivo faltando. Verificação manual:
  `fwMSX.exe --z80dbg --slots` via stdin (comandos `slots`/`pages`/
  `poke`/`slotpoke`/`slotmem`); regressão confirmada em `fwMSX.exe` (sem
  argumentos), `fwMSX.exe --z80dbg` (sem `--slots`) e
  `fwMSX.exe --msxdisk info` -- todos com saída idêntica à de antes desta
  fase.

  **Decisões explícitas tomadas nesta fase** (a spec deixou em aberto,
  decidido aqui):
  - **Restrição de subslot em MSX1/cartucho**: o fMSX força `V=0` em
    `SSlot()` quando o slot primário é um cartucho ou (em MSX1) o slot 0.
    Decisão: **não portar essa restrição ainda** -- não existe conceito
    de cartucho nem de modo MSX1/MSX2 no fwMSX nesta fase (chegam nas
    Fases 2/3), então adiantar a regra seria complexidade especulativa
    para algo que não tem onde se aplicar. Documentado em
    `memmap_switch_secondary()` (`src/memmap/core/slot_state.c`) para ser
    revisitado quando esses conceitos existirem de verdade.
  - **RAM default de `--z80dbg --slots`**: 64KB inteiros alocados na
    combinação 0:0, que já fica visível por padrão logo após
    `memmap_init()` (todo `psl`/`ssl` começa em 0) -- o usuário pode
    `poke`/`run` direto sem precisar trocar de slot primeiro. Tamanho
    máximo (64KB, não um valor menor) escolhido para não impor um limite
    arbitrário de RAM de teste.

  **Achado importante durante a implementação, não previsto no design
  original**: o core do Z80 (Fase 3, `src/z80/core/opcodes_ed.h`) chama
  `IBus::ram_ptr()` tanto para a ORIGEM quanto para o DESTINO de um
  `LDIR`/`LDDR`, e **escreve diretamente** no ponteiro de destino via
  `z80_fast_block_move()`, sem passar por `memmap_write()`. Se
  `SlotMemoryBus::ram_ptr()` devolvesse um ponteiro para um pedaço não
  gravável (ex.: uma futura ROM), o caminho rápido corromperia memória
  que `memmap_write()` teria corretamente recusado a escrever. Por isso
  `ram_ptr()` só devolve ponteiro para pedaços com `active_writable!=0`
  -- mais conservador que o estritamente necessário quando o intervalo é
  só origem de leitura (nesse caso o `LDIR` cai no loop lento em vez de
  usar o caminho rápido), mas é a única forma segura de manter a garantia
  "corretude nunca depende do caminho rápido" com uma única função
  servindo os dois papéis. Ver o comentário em
  `src/memmap/cpp/slot_memory_bus.h`.

  **Segundo achado, via teste manual (não pego pelos testes automatizados
  porque usavam slots já alocados)**: `slotpoke`/`poke` numa página/slot
  não-gravável (vazio, ou uma futura ROM) reportavam sucesso mesmo quando
  a escrita era descartada em silêncio por `memmap_write`/
  `memmap_poke_slot`. Corrigido em `Z80DebugSession::CmdPoke`/
  `CmdSlotPoke`: agora leem de volta o valor após escrever e reportam o
  que realmente ficou gravado, revelando o descarte em vez de mascará-lo.

  **Sem Fortran/ASM nesta fase, de propósito** (não é lacuna esquecida):
  o motor de troca de slot é busca em tabela de ponteiros, sem loop quente
  genuíno para acelerar em Assembly (ver seção 3.1); o checksum em
  Fortran só faz sentido quando existir alguma ROM pra calcular checksum
  (Fase 2). Fase 1 deste módulo é C + C++ apenas -- registrado aqui com a
  mesma transparência que o núcleo Z80 já usa para justificar decisões de
  linguagem por fase.
- [ ] **Fase 2 -- Carregamento de ROM real**: `LoadRom()`, suporte a ROM
      plana de 16/32KB sem bank-switch, checksum em Fortran
      (`rom_checksum.f90`). Testar contra a BIOS do fMSX em
      `resource/fMSX/ROMs/` (se o autor confirmar posse legal -- ver
      seção 5) para validar que o Z80 executa código real de BIOS por
      alguns milhares de ciclos sem travar.
- [ ] **Fase 3 -- MegaROM (bank switch)**: `MAP_KONAMI5`/`MAP_KONAMI4`/
      `MAP_ASCII8`/`MAP_ASCII16`/`MAP_GMASTER2`/`MAP_FMPAC`, decodificação
      de escrita em endereço de ROM (`MapROM()` equivalente), heurística
      `MAP_GUESS`.
- [ ] **Fase 4 -- Decisão de comportamento default do `--z80dbg`**: uma
      vez que carregar uma BIOS seja possível, decidir se `--z80dbg` sem
      argumentos passa a usar `SlotMemoryBus` com a BIOS pré-carregada em
      vez de `FlatMemoryBus` vazio -- ver seção 4.

*(Cada fase será detalhada em sub-fases, como aconteceu em
`doc/z80-core-spec.md`, no momento em que a implementação começar.)*
