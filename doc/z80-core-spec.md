# Nucleo Z80 do fwMSX -- especificacao (documento vivo)

> Segue o mesmo espirito de `doc/msxdisk-spec.md`: registra analise, decisoes
> de design e fases de implementacao do nucleo de CPU Z80 do fwMSX, para que
> o trabalho possa ser retomado do ponto exato onde parou. Nao remova secoes
> de fases concluidas -- marque como feitas e adicione as novas por baixo.

Estado desta secao: **fase de analise e design concluida em 2026-09-29**.
Nenhum codigo do nucleo foi escrito ainda -- este documento e a especificacao
que orienta a implementacao (Fase 1 em diante, secao 6).

## 1. Objetivo

Este e o primeiro pedaco do "core de emulacao propriamente dito" citado em
`doc/SPEC.md`, secao 5. Objetivo: um emulador de Z80 **fielmente compativel**
com o comportamento do fMSX (mesmos opcodes, mesmos ciclos, mesmo tratamento
de flags/interrupcoes), reescrito como nucleo proprio do fwMSX, moderno na
organizacao do codigo, e que **exercite de verdade as quatro linguagens do
projeto** (C++, C, Assembly, Fortran) -- nao so o esqueleto inicial de
`src/`, mas a parte que efetivamente importa em termos de desempenho.

Regra de licenciamento (ver `LICENSE-THIRD-PARTY.md`): o nucleo pode
adaptar/estudar o codigo de `resource/fMSX/Z80/`, com o aval do Fayzullin,
mas isso nao e uma relicenciacao. Qualquer arquivo novo que incorporar
logica adaptada do fMSX (tabelas de ciclos, formulas de flags, mnemonic dos
opcodes) deve:

1. Ter um cabecalho citando a origem (`Adaptado de fMSX, Copyright (C)
   Marat Fayzullin -- ver LICENSE-THIRD-PARTY.md`).
2. Ser adicionado a lista em `LICENSE-THIRD-PARTY.md`.
3. Ser tratado como **nao-comercial** ate autorizacao explicita em
   contrario -- exatamente como ja vale para `msxdos1_boot.cpp`.

Isso vale mesmo quando o arquivo novo e escrito em C, ASM ou Fortran (a
adaptacao de *logica*, nao so de C literal, e o que importa).

## 2. O que a analise de `resource/fMSX/Z80/` revelou

Arquivos lidos: `Z80.h` (188 linhas), `Z80.c` (726 linhas), `Codes.h`
(trecho representativo), `Tables.h` (trecho representativo). Resumo do
design original, que o nosso precisa igualar em comportamento:

- **Estado da CPU**: um `union pair` (`{byte l,h;} B` + `word W`) da acesso
  dual 8/16 bits a cada par de registrador, com endianismo resolvido em
  tempo de compilacao (`LSB_FIRST`/`MSB_FIRST`). O `struct Z80` guarda os
  pares principais e sombra (AF/BC/DE/HL/IX/IY/PC/SP + AF1/BC1/DE1/HL1),
  I/R, flip-flops de interrupcao (`IFF`), `IPeriod`/`ICount` (contagem de
  ciclos), `IRequest`, flags de configuracao (`IAutoReset`, `TrapBadOps`,
  `Trace`/`Trap` para debug) e um campo `User` livre para o host guardar
  contexto (ponteiro de RAM, id da maquina etc.).
- **Barramento por callback**: `RdZ80`/`WrZ80` (memoria), `InZ80`/`OutZ80`
  (portas de I/O) sao funcoes que o *host* (fMSX/MSX) implementa -- o
  nucleo do Z80 nunca sabe como a memoria/mapeamento de banco funciona.
  Existe tambem `OpZ80` (fetch de opcode), com um atalho `FAST_RDOP` que,
  quando a plataforma permite acesso direto a paginas de RAM contiguas
  (caso do fMSX), vira ponteiro direto em vez de callback -- otimizacao
  que so se aplica ao *fetch* de opcode, nao a leitura/escrita geral.
- **`PatchZ80` (opcode `ED FE`)**: um opcode nao-Z80 reservado para o host
  interceptar (usado pelo fMSX para emular chamadas de BIOS de disco/fita
  sem implementar o hardware real). Precisamos manter esse mesmo gancho --
  e' a base para o `msxdisk` eventualmente injetar arquivos MSX-DOS "por
  fora" sem floppy real, se quisermos essa feature no emulador.
- **`LoopZ80`**: chamado periodicamente (a cada `IPeriod` ciclos) para o
  host decidir se ha interrupcao pendente; pode devolver `INT_QUIT` para
  encerrar a emulacao. E' o ponto de integracao com o "resto da maquina"
  (VDP gerando IRQ de vsync, etc.).
- **Despacho**: nao e' tabela de ponteiros de funcao nem computed-goto -- e'
  um `switch(opcode)` gigante, com os `case` de cada opcode **inclusos via
  `#include`** (`Codes.h`, `CodesCB.h`, `CodesED.h`, `CodesXX.h`,
  `CodesXCB.h`), um arquivo por prefixo (sem prefixo / `CB` / `ED` / `DD`
  ou `FD` / `DDCB` ou `FDCB`). Os `case` sao rotulados por um `enum`
  (`NOP, LD_BC_WORD, ...`) em vez do valor hex direto, o que deixa o codigo
  legivel sem perder a eficiencia do `switch` (o compilador gera jump
  table quando os valores sao densos, como aqui).
- **Contagem de ciclos**: tabelas estaticas `Cycles[256]`, `CyclesCB[256]`,
  `CyclesED[256]`, `CyclesXX[256]`, `CyclesXXCB[256]` -- ciclos por opcode,
  subtraidos de `ICount` a cada instrucao. O loop principal roda
  `while(ICount>0) { decodifica; executa; }`.
- **Flags via tabela, nao bit-a-bit**: `ZSTable[256]` (Sign+Zero) e
  `PZSTable[256]` (Parity+Zero+Sign) sao tabelas de 256 bytes pre-calculadas
  e indexadas pelo resultado da operacao -- evita recalcular paridade
  (contagem de bits) a cada instrucao logica/aritmetica. Essa e a tecnica
  central de desempenho do nucleo inteiro; qualquer redesenho precisa
  preservar esse principio (tabela pre-computada > calculo repetido).
- **Interrupcoes**: `IFF_1`/`IFF_2` (flip-flops classicos do Z80),
  `IFF_IM1`/`IFF_IM2` (modo de interrupcao), `IFF_EI` (estado "acabou de
  executar EI", que atrasa a interrupcao em uma instrucao, comportamento
  real do Z80), `IFF_HALT`. `IntZ80()` trata NMI (vetor fixo 0x0066), IM1
  (vetor fixo 0x0038) e IM2 (vetor calculado a partir do registrador `I`).

Essa analise confirma que **o "nucleo" de verdade e' pequeno e conceitualmente
simples** (registradores + switch + tabelas + callbacks); a maior parte do
volume de codigo (Codes*.h) e' so a enumeracao mecânica dos ~1500 opcodes/
variacoes (incluindo prefixados). Isso molda a divisao de linguagens abaixo:
não faz sentido reescrever ~1500 `case` em Assembly (risco altissimo, ganho
baixo); faz sentido acelerar os poucos pontos onde o trabalho por instrucao
e' genuinamente proporcional a um contador (block ops) ou onde o calculo e'
preparatorio e roda uma unica vez (geracao de tabela).

## 3. Arquitetura proposta

### 3.1 Estrutura de diretorios

Segue o padrao ja usado por `src/msxdisk/` (subpastas por linguagem dentro
da feature, nao uma pasta por linguagem na raiz de `src/`):

```
src/z80/
├── core/           C -- estado da CPU + loop de despacho + tabelas de
│                   ciclo/opcode adaptadas do fMSX (Codes*.h equivalentes)
├── cpp/            C++ -- orquestracao: Z80Cpu (wrapper de alto nivel),
│                   interface IBus, integracao com debug/CLI do fwMSX
├── asm/            Assembly (NASM) -- aceleradores de block-ops
│                   (LDIR/LDDR/CPIR/CPDR/...), portavel Win64+SysV64
├── fortran/        Fortran -- geracao das tabelas de flags (ZSTable,
│                   PZSTable, tabela de correcao DAA) na inicializacao
└── common/         cabecalhos compartilhados entre C/C++/ASM (layout de
                    Z80State compativel com as 3 linguagens)
```

### 3.2 Camada C -- `src/z80/core/` (motor de despacho)

**Por que C, nao C++:** e' exatamente o que o proprio fMSX faz e a razao e'
valida hoje como era em 1994 -- o loop de despacho roda potencialmente
milhoes de vezes por segundo, e' o unico lugar do projeto inteiro onde
abstracao de C++ (vtable, exceptions, RAII em cada instrucao) tem custo
real e mensuravel. Vamos adaptar (nao copiar 1:1) a logica de:

- `Z80State` -- equivalente ao `struct Z80`, layout POD simples (sem
  ponteiros de C++, sem `std::` nada) para poder ser tocado por ASM
  (offsets fixos) e por Fortran (via `iso_c_binding`, `bind(c)`) sem
  reescrever a struct em cada linguagem.
- `z80_reset()`, `z80_run(Z80State*, IBus*, int cycles)` -- equivalentes a
  `ResetZ80`/`RunZ80`/`ExecZ80`. Preferimos o modelo `ExecZ80` (roda N
  ciclos e devolve controle) ao modelo `RunZ80` (so devolve via
  `INT_QUIT`), porque um frontend com TUI/CLI (breakpoints, step-by-step,
  inspecao de registradores entre frames de video) precisa de controle
  fino sobre "quantos ciclos rodar agora" -- e' o padrao usado por
  emuladores modernos (ex.: MAME) e evita reinventar um mecanismo de
  pausa via `LoopZ80` sobrecarregado.
- Tabelas de ciclos (`Cycles`, `CyclesCB`, `CyclesED`, `CyclesXX`,
  `CyclesXXCB`) -- dados puros, adaptados do fMSX (numeros de timing do
  Z80 sao dominio publico/factuais, mas o arquivo-fonte que os carrega
  ainda cita a origem por transparencia).
- Despacho via `switch` + arquivos de opcode inclusos (`opcodes_base.h`,
  `opcodes_cb.h`, `opcodes_ed.h`, `opcodes_xx.h`, `opcodes_xcb.h`),
  reescritos com nomes/estilo do fwMSX mas preservando a formula de cada
  instrucao (as formulas de flag de Z80 sao as mesmas em qualquer
  implementacao correta -- e' o comportamento definido pelo hardware real,
  nao uma escolha de projeto do fMSX).
- **`IBus` em C**: em vez de `RdZ80`/`WrZ80`/`InZ80`/`OutZ80` como funcoes
  globais extern (que travam o processo a uma unica instancia de CPU, como
  no fMSX original), o core em C recebe um `struct Z80Bus` com ponteiros de
  funcao + um `void* ctx` (o classico "manual vtable" em C). Isso permite
  varias instancias de Z80 no mesmo processo (util para testes automatizados
  e, no futuro, se o projeto quiser emular mais de uma maquina) sem mudar o
  principio de design do fMSX (callback-based bus), so removendo o
  acoplamento a variaveis globais.

### 3.3 Camada C++ -- `src/z80/cpp/` (orquestracao)

- `Z80Cpu` -- classe fina que possui um `Z80State` e delega a execucao ao
  core em C. Expoe API moderna: `reset()`, `run(int cycles) -> int
  cycles_left`, `request_interrupt(Vector)`, getters/setters de registrador
  nomeados (`af()`, `set_pc(word)`, etc.) para uso pelo debugger/CLI, sem
  expor a `union pair` diretamente na API publica.
- `IBus` (interface C++, `virtual byte read(word); virtual void
  write(word, byte); ...`) -- implementada pela camada de maquina MSX
  (fora do escopo desta fase). O `Z80Cpu` converte essa interface para o
  `Z80Bus` (struct de ponteiros de funcao) que o core em C entende, uma
  unica vez no construtor -- o custo de uma chamada virtual por acesso a
  memoria e aceitavel (e' o mesmo custo que qualquer emulador C++ moderno
  paga; o hot path real, o `switch` de opcode, continua em C puro).
- Gancho de `PatchZ80`/`ED FE` preservado como parte de `IBus`
  (`virtual void on_bios_patch(Z80Cpu&)`), mantendo a porta aberta para o
  `msxdisk` (ja existente) se conectar a chamadas de BIOS de disco no
  futuro, sem reescrever o mapeamento de FAT12/MSX-DOS que ja temos.
- Esta e' tambem a camada que vai integrar com CLI/TUI/GUI do fwMSX
  (visao registrada em `doc/SPEC.md`, secao 5.1): comandos de debug
  (`step`, `regs`, `break <addr>`, `disasm`) chamam metodos de `Z80Cpu`.

### 3.4 Camada Assembly -- `src/z80/asm/` (aceleradores de block-ops)

**Candidato genuino de ASM, nao decorativo**: as instrucoes de bloco do Z80
(`LDIR`/`LDDR`/`CPIR`/`CPDR`/`INIR`/`INDR`/`OTIR`/`OTDR`) sao, no hardware
real e no fMSX, um `case` que se repete via `R->PC.W-=2` enquanto `BC!=0` e
`ICount>0` -- ou seja, o unico ponto do nucleo onde o trabalho por instrucao
Z80 e' proporcional a um contador em vez de O(1). Sao tambem, na pratica de
software MSX-DOS, as instrucoes mais usadas em copias de buffer/limpeza de
memoria/VRAM -- exatamente o tipo de rotina pequena, auto-contida, chamada
com alta frequencia, que justifica Assembly de verdade (nao "ASM so para
ter ASM").

Escopo da aceleracao: **so quando origem e destino sao paginas de RAM do
host acessiveis por ponteiro direto** (mesmo principio do `FAST_RDOP` do
fMSX) -- ou seja, um `z80_fast_block_move(dst_ptr, src_ptr, len)`/
`z80_fast_block_cmp(...)` chamado pelo core em C **apenas** quando o `IBus`
confirma que o intervalo de enderecos inteiro do bloco cai dentro de uma
pagina de RAM simples (sem I/O mapeado, sem cruzar fronteira de banco). Fora
desse caso (comum, mas nao universal), o core em C cai para o loop
callback-a-callback, byte a byte, exatamente como o fMSX faz hoje -- entao
a corretude nunca depende do caminho rapido, so o desempenho.

**Portabilidade Windows+Linux em um so `.asm`**: NASM permite gerar tanto
`win64` (COFF, convencao de chamada RCX/RDX/R8/R9 + 32 bytes de shadow
space) quanto `elf64` (SysV AMD64: RDI/RSI/RDX/RCX/R8/R9, sem shadow space)
a partir do **mesmo arquivo-fonte**, selecionando a convencao via
`%ifidn __OUTPUT_FORMAT__, win64` / `%else` no proprio `.asm` -- e' a tecnica
padrao para NASM multi-ABI, e evita manter dois arquivos `.asm` divergentes
(o que seria a fonte mais provavel de bug de portabilidade). O CMake escolhe
o `-f win64` ou `-f elf64` de acordo com a plataforma de build, do mesmo
jeito que ja escolhe hoje para `src/asm/init_asm.asm` (que so tem a branch
Win64 -- vamos generalizar esse trecho do `CMakeLists.txt` quando
chegarmos na Fase 3, secao 6, em vez de mudar `init_asm.asm`, que continua
fora do escopo desta feature).

### 3.5 Camada Fortran -- `src/z80/fortran/` (geracao de tabelas de flags)

**Por que aqui, e por que e seguro para desempenho**: `ZSTable`,
`PZSTable` (e, se adaptarmos a instrucao `DAA`, a tabela de correcao BCD)
sao arrays de 256 bytes calculados **uma unica vez**, na inicializacao do
`Z80Cpu` -- nunca no hot path do `switch`. E' precisamente o tipo de tarefa
que o proprio usuario descreveu: "onde o impacto seja o mesmo tanto em C
quanto em Fortran" -- preencher 256 posicoes de array com uma formula
(contagem de bits pares para paridade, teste de sinal/zero) e' um trabalho
numerico simples, idiomatico em Fortran (loops `DO` sobre arrays, `IAND`/
`IEOR`/`POPCNT` como intrinsecos de bit a partir do Fortran 2008), e cujo
custo total (256 iteracoes, uma vez, no boot do emulador) e' irrelevante
comparado a rodar bilhoes de instrucoes Z80 por sessao de uso.

- `flag_tables.f90`: `subroutine z80_build_flag_tables(zs_table, pzs_table)
  bind(c)`, recebendo os dois arrays de 256 bytes (`integer(c_int8_t),
  intent(out) :: zs_table(0:255)`) e preenchendo-os. Chamado uma vez por
  `Z80Cpu` (ou por processo, com `static` cache) a partir do C++/C.
  Precisa do mesmo cuidado com buffer de I/O ja documentado em
  `doc/SPEC.md` secao 3.2 (`FLUSH` explicito) **so se** a rotina imprimir
  algo (nao deveria -- e' calculo puro); se nao houver `PRINT`/`WRITE`
  para stdout, esse problema nem se aplica aqui.
- Continua valendo a regra ja registrada no projeto: usar
  `Fortran_MODULE_DIRECTORY` por alvo no CMake, caso mais de um target
  (por ex., `fwMSX` e um futuro executavel de testes do Z80) compile este
  `.f90` -- mesma armadilha ja documentada em `doc/msxdisk-spec.md`.

### 3.6 `common/` -- layout compartilhado

`src/z80/common/z80_state.h` declara o `Z80State` (a struct POD) e as
constantes de flag/interrupcao (`S_FLAG`, `Z_FLAG`, ..., `INT_NMI`, ...) em
um unico lugar que C, C++ e ASM (via `%include`/offsets calculados, ou
constantes espelhadas manualmente com um teste `static_assert` de tamanho)
enxergam igual. Evita a divergencia de "a struct do C e a do ASM
descreveram coisas diferentes" -- risco real ja que o ASM vai receber
ponteiros para dentro dessa struct (endereco de `PC`, `SP` etc., se algum
acelerador precisar deles no futuro; os aceleradores da Fase 3 atuais so
recebem ponteiros de memoria/tamanho, entao esse risco fica baixo por
enquanto, mas o layout compartilhado e' a base correta para o que vier
depois).

## 4. Por que nao X

- **Nao reescrever o `switch` de despacho em Assembly nem computed-goto**:
  o proprio fMSX, otimizado por Fayzullin ao longo de 25+ anos para
  plataformas muito mais fracas que qualquer host atual, usa `switch` em C
  puro. Reescrever ~1500 casos em ASM seria o tipo de "otimizacao prematura
  de alto risco, baixo ganho" que as diretrizes do projeto pedem para
  evitar -- e um Z80 emulado por software em C num host x86-64 moderno ja
  roda ordens de magnitude mais rapido que um Z80 real (3.58 MHz) mesmo sem
  nenhum truque de ASM.
- **Nao usar C++ no hot path do opcode**: nada contra C++ em si, mas
  `virtual`/exceptions/RTTI por instrucao emulada e' custo que o fMSX nunca
  pagou e nos nao precisamos pagar -- por isso o `switch` fica em C, e o
  C++ entra so na camada que roda "uma vez por chamada de `run()`", nao
  "uma vez por instrucao Z80".
- **Nao usar Fortran em nada tocado pelo `switch`**: cumprindo a instrucao
  explicita do usuario ("nao use Fortran no core que precisa de muita
  performance"). A geracao de tabela e' o unico ponto onde Fortran e' ao
  mesmo tempo genuino (nao "Fortran so pra ter Fortran") e inofensivo para
  desempenho.

## 5. Perguntas em aberto (para confirmar antes de comecar a Fase 1)

- [ ] Modelo de execucao: `run(cycles)` (estilo `ExecZ80`, recomendado
      acima) vs. `RunZ80`-like (loop ate `LoopZ80` pedir saida). A
      recomendacao e' `run(cycles)` por dar controle fino ao frontend
      (TUI/CLI de debug), mas isso muda como o "resto da maquina" (VDP,
      timers) vai se encaixar quando essas pecas existirem.
- [ ] Precisao do `R` (refresh register) e de bugs "conhecidos" do Z80 real
      (ex.: comportamento de `SCF`/`CCF` com bit 3/5 indefinido, `MEMPTR`/
      `WZ` interno usado por alguns testes de compatibilidade de ciclo).
      fMSX nao implementa `MEMPTR`; decisao proposta: **tambem nao
      implementar nesta fase** (manter fidelidade ao fMSX, nao ao Z80
      "perfeito" de testes como ZEXALL/FUSE), documentar como limitacao
      conhecida, revisitar so se algum software MSX real depender disso.
- [ ] Testes automatizados do core: vale a pena, ja na Fase 1, portar um
      subconjunto de um test-suite publico de Z80 (ex.: ZEXDOC, que testa
      flags/timing) como alvo de CTest, ou isso fica para depois de existir
      VDP/memoria suficiente para rodar algo executavel de verdade?

## 6. Fases de implementacao propostas

- [x] **Fase 1 -- Core em C + wrapper C++ minimo** (concluida em
      2026-09-29): `Z80State`, tabelas de ciclo, despacho via
      `switch`+includes (opcodes sem prefixo primeiro, depois
      CB/ED/DD/FD/DDCB/FDCB), `Z80Cpu` com `reset()`/`run()`/acesso a
      registradores, `IBus` de teste simples (RAM plana em memoria, sem
      mapeamento de banco) para validar o core isoladamente.

  **Arquivos**: `src/z80/common/z80_state.h`; `src/z80/core/{z80_bus.h,
  z80_opcodes.h, z80_tables.{h,c}, opcodes_{base,cb,ed,xx,xcb}.h,
  z80_core.{h,c}}`; `src/z80/cpp/{z80_bus.h, z80_cpu.{h,cpp}}`;
  `tests/z80/smoke_test.cpp`. Alvo `z80test` no `CMakeLists.txt`
  (`Z80_LIB_SOURCES`), com `enable_testing()`/`add_test(z80_smoke)` --
  **ainda nao ligado a `fwMSX`/`msxdisk`** (isso e' a Fase 4).

  **Build/teste**: `cmake -S . -B build -G Ninja && cmake --build build
  --target z80test` seguido de `ctest --test-dir build` (ou rodar
  `dist/z80test.exe` diretamente). Os 8 asserts do smoke test (aritmetica
  de 8 bits + escrita em memoria, flags Z/S, flip-flop de HALT, `JR NZ`
  tomado e nao-tomado, wraparound de 16 bits em `INC HL`) passaram de
  primeira depois da porta -- nenhum bug de logica encontrado durante a
  verificacao.

  **Notas de implementacao / desvios registrados**:
  - **Gotcha de ambiente, nao do codigo**: nesta maquina, a shell usada
    para compilar precisou de `PATH="/c/msys64/ucrt64/bin:$PATH"`
    explicito na frente do `cmake --build`/`ctest` -- sem isso, `cc.exe`/
    `c++.exe` rodam (sao encontrados via `which`), mas o processo-filho
    real do compilador (`cc1.exe`/`cc1plus.exe`) falha ao carregar (erro
    de DLL, sem nenhuma mensagem de diagnostico, so `exit 1`/`127`) porque
    o diretorio do proprio compilador nao esta cedo o suficiente no PATH
    para a resolucao de DLL do Windows dar certo. Sintoma enganoso: o
    `ninja`/`cmake --build` reporta "FAILED" para cada arquivo **sem
    nenhuma linha de erro do compilador**, parecendo um problema no
    codigo-fonte quando na verdade e' esse PATH. Documentado aqui para
    quem retomar o projeto (ou rodar em outra maquina) nao perder tempo
    igual.
  - **`ExecZ80`, nao `RunZ80`**: confirmado durante a porta -- o modelo de
    execucao adotado (`z80_run(cycles)`) e' fiel ao branch `#ifdef
    EXECZ80` de `Z80.c`, incluindo o tratamento exato do atraso de `EI`
    (flag `IFF_EI`, restauracao via `IBackup`) e o efeito de `HALT`
    zerando `ICount` (a CPU "quase retorna na hora" depois de um HALT,
    comportamento real do fMSX preservado de proposito, nao seria
    "corrigido").
  - **Duas leituras do byte de deslocamento em `DEC (IX+d)`/`INC (IX+d)`**:
    o fMSX le o byte de deslocamento duas vezes (uma sem avancar `PC`,
    outra avancando) em vez de cachear o valor -- preservado exatamente
    assim em `opcodes_xx.h` (comentado no arquivo) em vez de "otimizar"
    para uma leitura so, para manter o mesmo numero de acessos ao barramento
    que o original.
  - **`LD H,(IX+d)`/`LD L,(IX+d)`/`LD (IX+d),H`/`LD (IX+d),L` usam H/L
    reais**: confirmado no fMSX e preservado -- esses opcodes tocam o par
    HL de verdade, nunca a metade alta/baixa de IX/IY, um comportamento
    real do hardware Z80 (comentado em `opcodes_xx.h`).
  - **RES/SET indocumentados em `(IX+d)`/`(IY+d)` nao escrevem no
    registrador nomeado**: o hardware real as vezes tambem escreve no
    registrador (alem da memoria) para essas variantes indocumentadas; o
    fMSX nunca implementou isso (so a variante "xHL" tem corpo, as
    variantes "nomeadas" caem no `default`), e a porta manteve esse mesmo
    comportamento -- limitacao conhecida, herdada de proposito para bater
    com o fMSX, nao um bug de transcricao (comentado em `opcodes_xcb.h`).
  - **`Z80Pair` simplificado**: fixo em little-endian (sem o
    `LSB_FIRST`/`MSB_FIRST` do original), ja que o fwMSX so roda em
    x86-64. Continua dependendo de type punning via uniao, extensao que
    GCC/Clang/MSVC suportam -- mesmo tradeoff que o proprio fMSX assume.
  - **Sem `FAST_RDOP`, `LoopZ80`, `Trap`/`Trace`/`DebugZ80`,
    `INT_QUIT`**: omitidos de proposito nesta fase (ver secao 3.2) -- o
    fetch de opcode usa o mesmo `bus->read` da leitura geral (sem o atalho
    de ponteiro direto do fMSX), e o rastreador de debug/loop de
    interrupcao periodica ficam para a Fase 4, quando houver de fato uma
    "maquina" (VDP/memoria) para orientar esse desenho.
  - **Diagnostico de opcode desconhecido vai para `stderr`** (via
    `fprintf`), nao `stdout` (`printf`) como no fMSX original -- pequena
    modernizacao, sem efeito no comportamento de execucao.
  - **Macros `Z80_M_*` envolvidas em `do { ... } while (0)`**: o fMSX
    original nao faz isso (confia em como cada macro e' usada em cada
    `case`); a porta adicionou esse invólucro por seguranca de macro em
    C, sem mudar a logica de nenhuma delas -- risco baixo, ganho de
    robustez contra uso futuro em contextos com `if`/`else` sem chaves.
- [x] **Fase 2 -- Fortran para tabelas de flag** (concluida em
      2026-09-30): geracao de `g_z80_zs_table`/`g_z80_pzs_table` movida
      para `src/z80/fortran/flag_tables.f90` (`z80_build_flag_tables`,
      `bind(c)`), chamada uma vez (idempotente, guardada por flag
      estatica) de dentro de `z80_tables_init()`
      (`src/z80/core/z80_tables.c`), por sua vez chamada no inicio de
      `z80_reset()` -- garante que as tabelas existem antes de qualquer
      opcode rodar, sem exigir que o host chame nada na mao.
      `g_z80_daa_table` **continua literal em C**, de proposito (ver
      "Notas de implementacao" abaixo).

  **Arquivos**: `src/z80/fortran/flag_tables.f90` (novo); `z80_tables.h`/
  `.c` (as duas tabelas deixaram de ser `const`/literais, ganharam
  `z80_tables_init()`); `z80_core.c` (`z80_reset()` chama
  `z80_tables_init()`); `CMakeLists.txt` (`flag_tables.f90` em
  `Z80_LIB_SOURCES`, `Fortran_MODULE_DIRECTORY` proprio pro alvo
  `z80test`, `-static-libgfortran` adicionado ao link).

  **Notas de implementacao**:
  - **Escopo deliberadamente menor que "toda tabela de flag"**: so
    `zs_table`/`pzs_table` foram para Fortran. `g_z80_daa_table` (2048
    entradas, correcao BCD do `DAA`) ficou de fora -- regenera-la por
    formula tem risco real de erro (a logica de correcao decimal do Z80
    tem varios casos especiais) para ganho de desempenho zero (nao e'
    hot path de qualquer jeito, e' uma tabela de 2048 entradas lida uma
    vez por `DAA` executado). Nao e' uma lacuna esquecida, e' uma troca
    risco/beneficio que nao compensava.
  - **Flags passados como parametro, nao hard-coded no `.f90`**: a
    sub-rotina Fortran recebe `Z80_S_FLAG`/`Z80_Z_FLAG`/`Z80_P_FLAG`
    como argumentos vindos do lado C -- `z80_state.h` continua sendo a
    unica fonte de verdade sobre qual bit e' qual; o Fortran so faz a
    aritmetica de preenchimento do array.
  - **Cuidado com `INT()` fora de faixa**: `IOR` de ate tres flags de 1
    byte pode passar de 127 (ex.: `0x80|0x04=0x84=132`), que nao cabe no
    intervalo do kind `c_int8_t` (-128..127). Converter um valor fora de
    faixa com `INT(x, kind)` e' processor-dependent pelo padrao Fortran
    -- normalizamos explicitamente pra faixa -128..127 (`if (v>127) v =
    v-256`) antes de converter, em vez de confiar no comportamento do
    compilador.
  - **`POPCNT`** (intrinseco Fortran 2008) foi usado para o calculo de
    paridade, em vez de um loop manual de contagem de bits -- forma
    idiomatica em Fortran, testada e funcionando com gfortran 16.2.0.
  - **Verificacao**: o smoke test ganhou uma checagem que recalcula as
    256 posicoes esperadas de `zs`/`pzs` com uma TERCEIRA implementacao
    (C++, bit a bit, sem nenhum intrinseco de contagem de bits) e
    compara contra o que `z80_build_flag_tables()` realmente gerou --
    nao e' so comparar duas copias da mesma formula. As 256/256
    posicoes bateram de primeira.

- [x] **Fase 3 -- Assembly para block-ops (LDIR/LDDR)** (concluida em
      2026-09-30, escopo reduzido -- ver abaixo): `z80_fast_block_move`
      em NASM (`src/z80/asm/block_ops.asm`), **primeiro `.asm` do
      projeto com duas ABIs no mesmo arquivo-fonte** (Win64 e SysV
      AMD64/Linux, via `%ifidn __OUTPUT_FORMAT__, win64` / `%else`).
      Acionado pelo despachante em C (`opcodes_ed.h`, casos `Z80_LDIR`/
      `Z80_LDDR`) apenas quando `Z80Bus::ram_ptr` (campo novo, opcional)
      devolve ponteiro direto de RAM plana para a fatia de bytes que
      sera' movida NAQUELA chamada -- fora isso, cai no loop
      byte-a-byte original (adaptado do fMSX), que continua intacto como
      fallback. Corretude nunca depende do caminho rapido ser tomado.

  **Escopo reduzido, decisao deliberada**: **so `LDIR`/`LDDR`** ganharam
  aceleracao. `CPIR`/`CPDR` (comparacao/busca) ficaram de fora desta
  passada -- as flags resultantes de `CPIR`/`CPDR` dependem do VALOR do
  ultimo byte comparado (nao so de BC chegar a zero, como em
  `LDIR`/`LDDR`), o que tornaria o "corte" entre o que foi
  processado-em-lote e o que ainda falta processar bem mais delicado de
  acertar com seguranca; preferimos nao arriscar uma tabela de flags
  errada num caminho "rapido" silencioso a entregar isso sem o mesmo
  nivel de verificacao dado a `LDIR`/`LDDR`. Fica registrado como
  trabalho futuro, se algum dia houver motivo de desempenho real pra
  isso (compras/buscas por string nao costumam ser hot path em software
  MSX). `INIR`/`INDR`/`OTIR`/`OTDR` nunca entraram no escopo -- tocam
  porta de I/O a cada iteracao (`bus->in`/`bus->out`), que por definicao
  nunca pode ter ponteiro direto de RAM.

  **Arquivos**: `src/z80/asm/block_ops.asm` + `block_ops.h` (novos);
  `z80_bus.h` (C) e `cpp/z80_bus.h` (`IBus`) ganharam o campo/metodo
  opcional `ram_ptr` (contrato: ponteiro direto pra `len` bytes de RAM
  plana do host cobrindo `[addr, addr+len)`, ou `NULL`; pode nao existir
  -- correcao nunca depende dele); `z80_cpu.h`/`.cpp` (trampolim
  `ram_ptr`); `opcodes_ed.h` (fast path prepended aos casos `Z80_LDIR`/
  `Z80_LDDR`, loop lento original preservado como fallback logo abaixo);
  `tests/z80/smoke_test.cpp` (`FlatRamBus::ram_ptr` real + teste
  diferencial); `CMakeLists.txt` (`block_ops.asm` em
  `Z80_LIB_SOURCES`).

  **Notas de implementacao**:
  - **Convencao de ponteiros no caso `reverse!=0` (LDDR)**: documentada
    em detalhe no cabecalho de `block_ops.asm` -- `dst`/`src` tem que
    apontar para o **ultimo** byte da regiao (ordem decrescente), nao o
    primeiro, espelhando como `REP MOVSB` com `DF=1` endereca memoria.
    Foi o ponto mais delicado desta fase (o lugar mais facil de
    introduzir um off-by-one); o lado C (`opcodes_ed.h`) pede a
    `bus->ram_ptr()` exatamente a fatia de `n` bytes que sera movida
    nesta chamada e desloca o ponteiro devolvido para o topo dessa
    fatia antes de chamar `z80_fast_block_move`.
  - **`REP MOVSB` faz a coisa certa "por acidente feliz"**: LDIR sobre
    regioes sobrepostas (com destino > origem) e' um truque real usado
    por software MSX para preencher memoria ("smear") -- depende da
    copia ser literalmente byte-a-byte em ordem crescente, lendo bytes
    ja sobrescritos. `REP MOVSB`/`STD;REP MOVSB` fazem exatamente isso
    (nao sao um `memmove` "seguro" que detecta sobreposicao), entao a
    aceleracao preserva esse comportamento sem esforco extra -- foi
    verificado explicitamente no teste diferencial (casos "sobreposto",
    ver abaixo).
  - **`BC==0` na entrada nao tenta o caminho rapido**: e' um quirk
    conhecido do Z80 real (`--BC` vira `0xFFFF` e o loop continua om
    mais 65536 iteracoes) que nao cabe num contador `uint16_t` do jeito
    como o caminho rapido foi desenhado; deixado para o loop lento
    resolver o primeiro byte, e a partir dai' (BC ja' nao-zero) o
    caminho rapido volta a valer no proximo redespacho.
  - **Simplificacao proposital na divisao de orcamento de ciclos**: o
    numero de bytes movidos por chamada e' `min(remaining, icount/21)`
    -- nao existe um branch separado pra "aproveitar o ultimo byte mais
    barato (16 ciclos) quando da' pra terminar exatamente no limite do
    orcamento". Isso significa que, numa janela estreita de 5 ciclos
    perto do fim de um bloco, o caminho rapido pode fazer 1 byte a
    menos nesta chamada do que teoricamente caberia, terminando na
    chamada seguinte -- **nunca afeta o estado final** (flags/
    registradores/memoria), so quantas chamadas de `run()` o bloco leva
    pra terminar, o que nenhum teste de estado-final consegue observar
    como erro. Trocamos uma otimizacao de ultimo grau por uma formula
    muito mais simples de verificar.
  - **Verificacao no Linux foi so parcial**: esta maquina e' Windows, so
    da' pra testar o link/execucao real do lado Win64. O lado `elf64`
    foi verificado com `nasm -f elf64 src/z80/asm/block_ops.asm -o
    <tmp>.o` (monta sem erro), mas **nao foi linkado nem executado** --
    isso so podera' ser confirmado numa maquina Linux de verdade.
  - **Teste diferencial obrigatorio**: `tests/z80/smoke_test.cpp` ganhou
    `run_block_case()`, que roda o MESMO programa Z80 (LD HL,nn/LD
    DE,nn/LD BC,nn/LDIR-ou-LDDR/HALT) a partir do MESMO estado inicial
    de memoria em duas instancias de `Z80Cpu` -- uma com
    `bus->ram_ptr` habilitado, outra forcada a `nullptr` (so loop
    lento) -- via `FlatRamBus::allow_fast_path`. Cobre: conclusao numa
    unica chamada de `run()`, conclusao so apos varias chamadas com
    orcamento minusculo (forcando o rebobinamento de PC no meio do
    bloco), `len==1`, LDIR e LDDR, e casos com sobreposicao proposital
    de origem/destino (o cenario de "smear" citado acima) -- mais uma
    varredura aleatoria de 150 combinacoes (semente fixa, reprodutivel)
    de tamanho/orcamento/enderecos por cima dos casos fixos. Todos os
    casos bateram (registradores E memoria inteira identicos entre
    caminho rapido e lento).
  - **`CMAKE_ASM_NASM_OBJECT_FORMAT` nao precisou ser setado na mao**:
    o suporte `ASM_NASM` do CMake ja' escolhe o formato de objeto certo
    pra plataforma de build sozinho (confirmado: os alvos existentes
    `init_asm.asm`/`name_match.asm` ja' compilavam certo em Win64 sem
    nenhuma flag explicita) -- nao foi preciso adicionar logica nova de
    deteccao de plataforma no `CMakeLists.txt`.

- [x] **Fase 4 -- Integracao com fwMSX** (concluida em 2026-09-30): REPL
      de depuracao do `Z80Cpu` embutido em `fwMSX.exe` (`fwmsx --z80dbg`),
      por tras de uma RAM plana de 64KB sem nenhum mapeamento de maquina
      (sem VDP/PSG/bank-switch -- isso ainda nao existe) -- exatamente o
      "so uma RAM de teste" previsto para esta fase.

  **Arquivos**: `src/z80/debug/{flat_memory_bus.h, z80_debug_session.h,
  z80_debug_session.cpp, z80_debug_shell.h, z80_debug_shell.cpp}`;
  `src/cpp/main.cpp` ganhou a interceptacao de `--z80dbg` (mesmo padrao de
  `--msxdisk`); `CMakeLists.txt` ganhou `Z80_DEBUG_SESSION_SOURCES`
  (`z80_debug_session.cpp`, sem dependencia de replxx -- testavel
  isolado), o alvo `z80dbgtest`, e a inclusao de `Z80_LIB_SOURCES` +
  `Z80_DEBUG_SESSION_SOURCES` + `z80_debug_shell.cpp` no proprio `fwMSX`
  (primeira vez que o nucleo Z80 entra no executavel principal).
  `tests/z80/debug_session_test.cpp` (novo, 11 verificacoes).

  **Design**: `Z80DebugSession::ProcessCommand(tokens) -> string` e' o
  nucleo testavel (sem replxx/stdin); `z80_debug_shell.cpp` e' so o REPL
  fino por cima (mesmo estilo de `src/msxdisk/shell/shell.cpp`, historico
  em `~/.fwmsx_z80dbg_history`). Comandos: `reset`, `regs` (registradores
  + flags decodificadas letra a letra + IFF/IM/HALT + I/R), `step [n]`
  (`n` chamadas de `cpu.run(1)` -- cada uma e' genuinamente UM passo de
  instrucao, ver a nota da API em `z80_cpu.h`), `run [ciclos]` (repete
  `cpu.run(1)` ate esgotar o orcamento OU acertar um breakpoint --
  granularidade de 1 instrucao por chamada e' o que permite breakpoint
  sem tocar no core C), `break`/`clear`/`breaks`, `mem`/`peek`/`poke`,
  `load <arquivo> <endereco>` (binario cru do host), `fill`, `help`.
  Enderecos/numeros aceitam decimal, `0x`-hex ou `$`-hex.

  **Adiado de proposito, nao esquecido**: `disasm` (desmontador) --
  mencionado como aspiracao na secao 3.3, mas uma tabela de mnemonicos
  para as ~1500 variantes de opcode e' um trabalho a parte que nao
  bloqueia o valor do resto desta fase (depurar registradores/memoria/
  breakpoints ja' e' util sem isso). Fica para uma fase futura, junto com
  a "maquina" de verdade (VDP/PSG/mapa de memoria) que vai dar mais
  contexto pra decidir o formato de saida do desmontador.

  **Build/teste**: `z80dbgtest` roda isolado (`cmake --build build
  --target z80dbgtest && ./dist/z80dbgtest.exe`) -- 11/11 passando.
  `fwMSX.exe` completo compilado limpo (40 alvos, sem warning novo) e
  verificado de ponta a ponta via stdin nao-interativo (replxx aceita
  entrada via pipe sem TTY de verdade):
  `printf 'reset\nregs\npoke 0x1000 0xAB\npeek 0x1000\nexit\n' |
  ./dist/fwMSX.exe --z80dbg` -- respondeu corretamente a cada comando.
  Regressao conferida: `fwMSX.exe` sem argumentos continua com a saida
  do esqueleto inalterada; `fwMSX.exe --msxdisk info` (sem imagem)
  continua devolvendo o erro de uso esperado do msxdisk, nao quebrou com
  a integracao do Z80.

  **Arquivos desta fase sao codigo ORIGINAL do fwMSX** (BSD-3-Clause) --
  nenhum adapta `resource/fMSX/`, entao nada foi adicionado a
  `LICENSE-THIRD-PARTY.md`.

## 7. Build e teste (Fases 1-4)

```
export PATH="/c/msys64/ucrt64/bin:$PATH"   # ver nota na Fase 1 acima
cmake -S . -B build -G Ninja
cmake --build build --target z80test
cmake --build build --target z80dbgtest
cmake --build build --target fwMSX
ctest --test-dir build -R z80 --output-on-failure
```

Resultado em 2026-09-30: build limpo, `ctest` verde -- `z80_smoke` (168
verificacoes: 8 da Fase 1 + 2 da Fase 2 + 158 da Fase 3) e
`z80_debug_session` (11 verificacoes, Fase 4), mais a verificacao manual
de `fwMSX.exe --z80dbg` via stdin descrita acima.

*(Cada fase sera detalhada em sub-fases, como aconteceu em
`doc/msxdisk-spec.md`, no momento em que a implementacao comecar.)*
