# Mapa de memoria MSX (slots/subslots) -- especificacao (documento vivo)

> Mesmo espirito de `doc/z80-core-spec.md`: analise do fMSX, decisoes de
> design e fases de implementacao, para retomar o trabalho do ponto exato
> onde parou. Nao remova secoes de fases concluidas -- marque como feitas
> e adicione as novas por baixo.

Estado desta secao: **Fases 1-4 concluidas em 2026-09-30** (ver secao 6).
Este modulo fica **pausado aqui por enquanto** -- proximos passos do
projeto ficam registrados em `doc/SPEC.md`, nao aqui (nao ha mais fases
deste modulo planejadas no momento).

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

### 3.4 Fortran -- CRC32 (conveniência do depurador, NÃO detecção de mapper)

**Correção de escopo em relação à versão original deste parágrafo**
(decidida durante a implementação da Fase 2, ver seção 6): checado
`resource/fMSX/fMSX/MSX.c` e `resource/fMSX/ROMs/CARTS.SHA` -- a
detecção automática de cartucho/mapper do fMSX usa **SHA1** contra um
banco de assinaturas (`CARTS.SHA`, via `EMULib/SHA1.*`), não um
checksum simples. Reimplementar SHA1 é um algoritmo real, sensível a
bugs sutis, e nem é necessário ainda: a Fase 2 é ROM plana sem
bank-switch, então não há "qual mapper?" para adivinhar.

`src/memmap/fortran/rom_checksum.f90` calcula, em vez disso, um
**CRC32 padrão** (polinômio `0xEDB88320`, reflected -- o mesmo do
zlib/PKZIP/Ethernet) sobre os bytes de uma ROM recém-carregada, uma
única vez no momento do `LoadRom()` -- trabalho numérico sobre um array
de bytes via tabela de 256 entradas, mesmo padrão de `flag_tables.f90`
(Fase 2 do núcleo Z80): cálculo pontual, fora do caminho quente. Serve
**só** como conveniência do depurador -- "qual imagem exata é essa"
(ex.: distinguir uma BIOS padrão de uma remendada), verificável contra
o vetor de teste padrão de qualquer CRC32 (`CRC32("123456789") ==
0xCBF43926`, checado em `tests/z80/memmap_test.cpp`). Detecção de
mapper via SHA1/`CARTS.SHA` fica para quando a Fase 3 (bank-switch)
precisar de verdade -- este CRC32 **não** cobre esse caso de uso, para
não confundir quem for implementar a Fase 3 depois.

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
uma RAM plana simples, sem ganho real de cobertura.

**Decisão final (Fase 4 deste módulo, ver seção 6)**: `--z80dbg` sem
argumento nenhum continua usando `FlatMemoryBus` -- não existe (nem
existiu) uma flag `--flat` separada, essa ideia inicial foi descartada
em favor de algo mais simples: `--slots` liga o mapa de memória real
(opcionalmente com `--slots <rom>` pra já carregar uma ROM de boot em
0:0), e a ausência de `--slots` é, ela mesma, o "modo flat". Nenhum
caso de uso real apareceu para justificar trocar esse default.

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
- [x] **Fase 2 -- Carregamento de ROM real** (concluída em 2026-09-30):
      `MemorySystem::LoadRom(primary, secondary, data, size, error)` --
      ROM plana (8..64KB, múltiplo de 8KB, sem bank-switch -- isso é
      Fase 3), reaproveitando `memmap_attach()` já existente da Fase 1
      (o parâmetro `kind`/`writable` já estava preparado para isso,
      nenhuma mudança no motor em C foi necessária). CRC32 em Fortran
      (`rom_checksum.f90`, ver seção 3.4 -- e sua correção de escopo em
      relação à ideia original de "ajudar `MAP_GUESS`"). Novo campo
      `SlotDescriptor::crc32`. Comando `loadrom <primario> <secundario>
      <arquivo>` no `--z80dbg --slots`; `slots` passou a mostrar o CRC32
      de entradas ROM.

  **Decisão explícita**: a ROM sempre começa no pedaço 0 da combinação
  de slot (endereço relativo `0x0000`) -- pedaços de 8KB além do
  tamanho da imagem ficam vazios. Mesma convenção que `AllocateRam()`
  já usava desde a Fase 1 (via `memmap_attach()`), escolhida por ser o
  caso normal de uma BIOS/ROM que ocupa o slot inteiro a partir do
  início; carregar uma ROM num deslocamento diferente de 0 dentro do
  slot não é um caso de uso real conhecido, então não foi exposto.

  **Teste de aceitação com BIOS real** (o critério mais importante desta
  fase): carrega `resource/fMSX/ROMs/MSX.ROM` (BIOS MSX1 real, 32KB, já
  presente no repositório como material de estudo/referência -- ver
  seção 5 -- só LIDO pelo teste, nunca copiado/redistribuído) em 0:0,
  reseta o Z80 e roda 100.000 ciclos. Critério concreto (não só "não
  travou"): `PC != 0x0000` ao final, **e** o número de endereços de PC
  distintos visitados durante a execução é maior que 50. Resultado real
  (2026-09-30): 192 endereços distintos visitados, `PC` terminou em
  `0x0365` -- evidência de que o núcleo Z80 está de fato buscando e
  executando instruções reais da BIOS em sequência, não preso num loop
  trivial ou travado num opcode inválido logo de cara. Resiliente à
  ausência do arquivo (`[SKIP]`, não falha o suite) caso alguém clone o
  repositório sem esse arquivo de `resource/`.

  **Build/teste**: `cmake --build build --target memmaptest z80dbgtest
  fwMSX` seguido de `ctest --test-dir build -R "z80|memmap"` -- verde
  (168 + 35 + 56 = 259 verificações; `memmaptest` sozinho foi de 29 para
  56, as 27 novas cobrindo CRC32 isolado, `LoadRom` em 3 tamanhos válidos
  com CRC32 cruzado contra uma segunda implementação independente (sem
  tabela, ver `tests/z80/memmap_test.cpp`), rejeição de tamanho inválido,
  somente-leitura nos dois caminhos -- `PokeSlot` e
  `SlotMemoryBus::write` --, o comando `loadrom` via
  `Z80DebugSession::ProcessCommand`, e o teste de aceitação com BIOS
  real). Verificação manual: `fwMSX.exe --z80dbg --slots` com
  `loadrom 0 0 resource/fMSX/ROMs/MSX.ROM` seguido de `slots`/`pages`/
  `reset`/`run 5000`/`regs` -- PC foi de `0000` para `0365` em 5003
  ciclos, páginas mostraram corretamente "somente leitura" para a ROM.

  **Sem mudanças no motor em C (`slot_state.c`/`.h`) nesta fase** --
  `memmap_attach()` já suportava `MEMMAP_KIND_ROM`/`writable=0` desde a
  Fase 1 (o campo existia "desde já" exatamente para isso, ver seção
  3.2/6); `LoadRom()` só precisou alocar um buffer próprio, copiar os
  bytes, calcular o CRC32 e chamar a função já existente.
- [x] **Fase 3 -- MegaROM (bank switch)** (concluída em 2026-09-30):
      troca de banco de ROM (só a parte de ROM, sem SCC/SRAM) para
      `MAP_GEN8`/`MAP_GEN16`/`MAP_KONAMI5`/`MAP_KONAMI4`/`MAP_ASCII8`/
      `MAP_ASCII16`, adaptado de `MapROM()` em
      `resource/fMSX/fMSX/MSX.c`.

  **Escopo reduzido em relação ao item original desta fase** (decidido
  ao ler `MapROM()` por completo antes de portar, não antes): o texto
  original listava também `MAP_GMASTER2`/`MAP_FMPAC`/`MAP_GUESS`.
  Deixados de fora, com justificativa:
  - **SCC** (chip de som, interação em `9800h-98FFh` quando ligado) --
    aparece em `MAP_GEN8`/`MAP_KONAMI5`. Adiado nesta fase; **implementado na
    v1.13.0**: o barramento entrega essas leituras/escritas ao `SccDevice`
    antes do mapper (`memmap::SlotCartIo`, `src/memmap/cpp/slot_memory_bus.h`).
    Ver `doc/scc-spec.md`.
  - **SRAM** (bateria, selecionada por um bit no valor de troca de
    banco, persistida em arquivo) em `MAP_ASCII8`/`MAP_ASCII16` --
    exige infraestrutura de save-state que não existe ainda. Portada só
    a metade de troca de banco de ROM de cada um; uma tentativa de
    selecionar SRAM é **reconhecida** (a escrita não cai no descarte
    genérico) mas **ignorada** -- sem crash, sem corrupção, só sem
    efeito.
  - **`MAP_GMASTER2` e `MAP_FMPAC`** -- ambos só são interessantes por
    causa de SRAM (GameMaster2) ou SRAM+som FM (FMPAC); sem isso,
    degeneram pra uma troca de banco trivial não muito diferente do
    `MAP_KONAMI4`. Não implementados.
  - **`MAP_GUESS`** (detecção automática de mapper via `GuessROM()` --
    tenta `CARTS.CRC`/`CARTS.SHA` primeiro, depois varre a ROM por
    padrões de bytes característicos) -- feature separada com
    dependências de formato de arquivo próprias; exigir o tipo de
    mapper explícito em `loadrom` é mais simples e suficiente por
    enquanto.

  **Arquivos**: `src/memmap/core/slot_state.{h,c}` (extensão: campos
  `slot_mapper`/`rom_base`/`rom_bank_mask`/`rom_bank` em `SlotState`;
  novas funções `memmap_attach_megarom()` e `memmap_try_bank_switch()`;
  `memmap_write()` agora tenta bank-switch antes de descartar uma
  escrita não-gravável). `src/memmap/cpp/memory_system.{h,cpp}`
  (`LoadRom()` ganhou um parâmetro `mapper` opcional -- default
  `MEMMAP_MAPPER_NONE` preserva o comportamento da Fase 2 exatamente;
  `SlotDescriptor::mapper_name` populado). `src/memmap/common/
  memmap_types.h` (`enum MemMapMapperType`). `src/z80/debug/
  z80_debug_session.cpp` (`loadrom` ganhou um 4º argumento opcional de
  mapper; `slots` mostra o nome do mapper; mensagem de `poke`/`slotpoke`
  ajustada -- ver "achado" abaixo). `tests/z80/memmap_test.cpp` (46
  verificações novas).

  **Decisões explícitas**:
  - **Endereçamento**: chaveado direto por `(primário,secundário)`, não
    por um índice de "slot de cartucho" como o `CartMap[PS][SS]` do
    fMSX -- o fwMSX ainda não tem o conceito de slot físico de cartucho
    separado do lógico (simplificação já registrada na seção 3.2).
  - **Estado inicial de uma MegaROM recém-carregada** (CORRIGIDO na
    v1.10 -- a versão original desta fase era uma suposição errada): os
    4 pedaços de 8KB mostram os bancos **0,1,2,3** em `4000h/6000h/
    8000h/A000h` (mascarados pelo tamanho da ROM), como `SetMegaROM(J,
    0,1,2,3)` do fMSX. A decisão original ("todos no banco 0, o INIT
    sempre troca os bancos antes de depender deles") não vale: o INIT de
    vários jogos chama rotinas em `6000h-7FFFh` esperando o banco 1 ali,
    e executava lixo -- ver `doc/machine-spec.md`, seção 4b (Firebird).
    O fMSX também tem a variante `N-2,N-1,N-2,N-1` (cartuchos cujo
    cabeçalho `AB` fica no fim da ROM) -- ainda não implementada.
  - **Tamanho válido de MegaROM**: múltiplo de 8KB, entre 8KB e 2MB (256
    bancos) -- o teto vem de `rom_bank_mask` ser um `uint8_t` (mesmo
    tipo `byte ROMMask[MAXSLOTS]` do fMSX, que tem a mesma limitação
    real). ROM plana (Fase 2, `mapper` omitido) continua limitada a
    64KB, comportamento inalterado.

  **A sutileza central desta fase, implementada e testada**: a vista
  ativa da CPU (`active_view`/`active_writable`) precisa refletir a
  troca de banco **na hora**, sem esperar por um slot-switch separado --
  `memmap_try_bank_switch()` sempre atualiza `chunk[primário][secundário]`
  (a tabela de apoio) e, quando a página afetada atualmente mostra essa
  mesma combinação, também a vista ativa. Para `GEN8`/`GEN16`/`KONAMI4`/
  `KONAMI5` essa checagem é sempre verdadeira por construção (o endereço
  de controle sempre cai na mesma página que o pedaço afetado); só
  `ASCII8`/`ASCII16` de fato precisam dela (o endereço de controle mora
  sempre na página 1, `6000h-7FFFh`, mas pode afetar um pedaço da
  página 2) -- replicado fielmente do `MapROM()` original, que faz
  exatamente essa distinção (`if((PSL[(J>>1)+1]==PS)&&...)`). Testado
  explicitamente na seção "vista-ativa" de `memmap_test.cpp`: escreve a
  troca de banco através de `SlotMemoryBus::write` (como o Z80 faria de
  verdade) e confirma que `IBus::read` já mostra o novo banco, sem
  nenhuma chamada de troca de slot no meio.

  **Dois bugs reais encontrados e corrigidos durante a implementação**
  (nenhum estava previsto no design original):
  1. **Bug de estado obsoleto (corrigido no motor)**: a primeira versão
     tinha uma otimização "pula se o banco não mudou" (`if
     (rom_bank[quarto]==novo_banco) return;`), copiada do padrão
     `if(V!=ROMMapper[I][J])` do fMSX. Só que o estado inicial
     simplificado desta fase (todos os quartos = banco 0, ver decisão
     acima) quebra o invariante que essa otimização pressupõe pros
     mappers de granularidade 16KB (`GEN16`/`ASCII16`): o quarto
     "parceiro" (`quarto+1`) também começa apontando pro banco 0 em vez
     de banco 1, então uma escrita legítima de "banco 0" no quarto
     principal batia com o bookkeeping obsoleto e pulava a atualização
     do parceiro, deixando-o preso mostrando o banco errado
     indefinidamente. Como essa otimização não tem valor real de
     desempenho aqui (troca de banco é um evento raríssimo comparado a
     instruções de Z80 executadas), a correção foi **remover a
     otimização inteira** em vez de tentar reparar o invariante -- mais
     simples e sem essa classe de bug. Pego pelos próprios testes
     automatizados (não precisou de teste manual).
  2. **Mensagem enganosa do `poke` (corrigido na UX do depurador,
     achado testando `--z80dbg --slots` na mão)**: depois da Fase 3,
     escrever num endereço de MegaROM via `poke` mostrava "(não gravado
     -- página atual não é gravável...)", que é **factualmente errado**
     nesse caso -- a escrita foi reconhecida e interpretada como
     comando de troca de banco (nunca descartada), só que o byte lido
     de volta é o conteúdo do banco recém-selecionado, não o número de
     banco escrito, então batia com a heurística antiga de "byte lido
     ≠ byte escrito = descartado". Como `IBus::write()` não devolve
     informação sobre o que aconteceu (descarte vs. comando de mapper),
     a mensagem foi reescrita pra não afirmar qual dos dois casos
     ocorreu, só descrever o estado observável e sugerir `slots`/`pages`
     pra investigar. `slotpoke` (que usa `PokeSlot`, o backdoor cru do
     depurador que nunca aciona bank-switch por design) não tinha esse
     problema -- mensagem original mantida, continua correta.

  **Build/teste**: `cmake --build build --target memmaptest z80dbgtest
  fwMSX` seguido de `ctest --test-dir build -R "z80|memmap"` -- verde
  (168 + 35 + 102 = 305 verificações; `memmaptest` foi de 56 para 102).
  Verificação manual: `fwMSX.exe --z80dbg --slots` com uma ROM sintética
  de 4 bancos (`gen8`) carregada em 0:0 -- `loadrom`/`slots` mostram o
  mapper corretamente, `poke 4000 2` troca de banco de verdade (`peek
  4000` confirma o novo conteúdo), mensagem de `poke` revisada exibida
  corretamente. Regressão confirmada em `fwMSX.exe` (sem argumentos),
  `fwMSX.exe --z80dbg` (sem `--slots`) e `fwMSX.exe --msxdisk info`.
- [x] **Fase 4 -- Decisão de comportamento default do `--z80dbg`**
      (concluída em 2026-09-30, **última fase deste módulo por enquanto**).

  **Decisão**: o `--z80dbg` **sem argumentos continua exatamente como
  era** -- `FlatMemoryBus`, RAM plana de 64KB, nada de slots/BIOS. O
  caso de uso mais simples (poke de alguns bytes, `step` num programa
  de teste escrito na mão) não ganha uma dependência de achar/carregar
  uma ROM só pra iniciar; quem quer o mapa de memória real continua
  pedindo `--slots` explicitamente. Isso não muda nada da seção 4 --
  a decisão ali registrada (não substituir `FlatMemoryBus` por padrão)
  se manteve.

  **O que a Fase 4 de fato adicionou**: uma conveniência para não
  precisar de um `loadrom` manual toda vez que `--slots` é usado com
  uma ROM de boot. `fwmsx --z80dbg --slots <rom>` carrega `<rom>` em
  0:0 (ROM plana, `mapper=None`, igual uma BIOS carregada na mão)
  **antes** de entrar no REPL, e já deixa essa combinação como vista
  ativa da CPU. `fwmsx --z80dbg --slots` sem caminho nenhum continua
  igual (RAM vazia em 0:0). Se o caminho dado falhar (arquivo
  inexistente, tamanho inválido), a sessão **ainda inicia** -- mensagem
  de aviso na tela, RAM vazia em 0:0 como se nenhum caminho tivesse
  sido passado (mesmo espírito "avisa, não trava" de todo comando de
  arquivo já existente no `--z80dbg`).

  **Regra de sintaxe** (deliberadamente simples, não um parser de
  argumentos genérico): só o token **imediatamente seguinte** a
  `--slots` (se houver) é tratado como caminho de ROM -- `--z80dbg
  --slots caminho.rom`. Nenhuma outra ordem de argumento é suportada.

  **Arquivos**: `src/z80/debug/z80_debug_shell_startup.{h,cpp}` (novo)
  -- `Z80DebugShellStartup`/`BuildZ80DebugShellStartup()`, a lógica de
  montagem em si, **sem** depender de replxx (permite testar sem TTY).
  `src/z80/debug/z80_debug_shell.{h,cpp}` simplificado para só o loop
  replxx, chamando a função acima. `tests/z80/debug_session_test.cpp`
  ganhou 23 verificações novas (58 no total, de 35): `--slots` sem
  caminho (regressão), `--slots <rom válida>` (ROM carregada, virou
  vista ativa -- checado via `IBus::read`, não só `PeekSlot`), `--slots
  <rom inexistente>` (não trava, cai pra RAM vazia, mensagem de erro
  preenchida), e `--z80dbg` puro (sem `--slots`, comportamento
  inalterado). `CMakeLists.txt`: `z80_debug_shell_startup.cpp` entrou em
  `Z80_DEBUG_SESSION_SOURCES` (compilado em `z80dbgtest`/`memmaptest`
  também, não só `fwMSX`) -- é exatamente a separação que evita
  arrastar replxx pros alvos de teste (achada e corrigida durante a
  implementação: a primeira tentativa colocou a função nova dentro do
  próprio `z80_debug_shell.cpp`, que já inclui `<replxx.hxx>`, e o link
  de `z80dbgtest` quebrou por falta do símbolo -- `z80dbgtest` nunca
  linkou replxx, de propósito).

  **Build/teste**: `cmake --build build --target z80dbgtest fwMSX
  memmaptest z80test` seguido de `ctest --test-dir build -R
  "z80|memmap"` -- verde (168 + 58 + 102 = 328 verificações;
  `z80dbgtest` foi de 35 para 58). Verificação manual: `fwMSX.exe
  --z80dbg --slots resource/fMSX/ROMs/MSX.ROM` carrega a BIOS
  automaticamente e roda igual ao teste manual da Fase 2 (PC sai de
  `0000` pra `0365` em 5003 ciclos); `--slots naoexiste.rom` mostra o
  aviso e inicia normalmente com RAM vazia. Regressão confirmada em
  `fwMSX.exe` (sem argumentos), `fwMSX.exe --z80dbg` (sem `--slots`) e
  `fwMSX.exe --msxdisk info`.

*(Este módulo fica pausado aqui por enquanto -- ver `doc/SPEC.md` para
os próximos passos do projeto como um todo.)*

### Nota (2026-10-01, v1.7.0): regras de subslot do MSX1

`SlotState.msx1_subslot_rules` (default 0) liga o `SSlot()` fiel do fMSX:
slots 0/1/2 nunca tem subslot -- escrever em `FFFFh` neles e' forcado a 0.
So' o startup do depurador com BIOS + `--ppi` liga isso (junto com RAM em
`3:2`), porque so' ali ha' uma maquina MSX1 completa; a decisao da Fase 1
("todo slot primario aceita subslot livremente") continua valendo para o
resto. Ver `doc/ppi-spec.md`, secoes 4 e 5.
