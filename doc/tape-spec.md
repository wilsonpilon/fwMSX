# Fita (.CAS e .TSX/.TZX) -- fwMSX

Modulo `src/tape/`. Implementa os passos (a), (b), (d) e (g), e parte do
(c), da ordem sugerida em `doc/SPEC.md`, secao 5.2 ("Feature a
desenvolver em breve: fitas"): leitura e GRAVACAO de .CAS e .TSX/.TZX,
carregamento pela BIOS (modo rapido) e carregamento por pulsos de
verdade (modo normal, com som), fita nova em branco, protecao contra
gravacao, marcar o ponto de carga/gravacao e um contagiros simulado. Nao
inclui banco de fitas nem download -- isso continua para depois (itens
(e) e (f)); a gravacao em "modo normal" (pulsos reais capturados do
sinal, nao so' do gancho de BIOS) tambem nao -- ver secao 7.

## 1. Dois modos de carregamento

- **Rapido** (padrao): intercepta as rotinas da BIOS que leem a fita
  (TAPION/TAPIN/TAPIOF) e devolve os bytes direto de um buffer em
  memoria, sem nenhuma temporizacao. Nao ha' som.
- **Normal**: gera os pulsos de verdade (piloto + bits codificados em
  Kansas City Standard) e os entrega pela porta de verdade, na cadencia
  de T-states da CPU -- a BIOS roda a rotina ORIGINAL dela (sem patch
  nenhum), fazendo a mesma temporizacao que faria num MSX real. O sinal
  tambem e' sintetizado em audio, entao o barulho do carregamento e'
  ouvido (como num gravador de fita antigo).

Trocar de modo nao reinicia a maquina: so' troca o patch da BIOS (ver
secao 3) e liga/desliga o cursor de pulsos. Menu **Fita** da janela, ou
`--fita-modo rapido|normal` na linha de comando.

## 2. Formatos lidos

### 2.1 .CAS

Imagem crua de fita do MSX. Convencao (a mesma do fMSX e do openMSX,
conferida em `resource/fMSX/fMSX/Patch.c` e
`resource/openMSX/src/cassette/CasImage.cc`):

- Marcador de sincronismo de 8 bytes entre blocos logicos:
  `1F A6 DE BA CC 13 7D 74`.
- Cabecalho de arquivo: 10 bytes do mesmo valor (tipo) + 6 bytes de nome
  (preenchido com espaco). Tipos: `D0h` binario (BLOAD), `D3h` BASIC
  (CLOAD), `EAh` ASCII.
- Depois de outro marcador de 8 bytes, os dados do arquivo (para
  binario, comecam com 6 bytes de endereco inicio/fim/execucao).

### 2.2 .TSX/.TZX

TZX 1.20 (`resource/makeTSX/docs/TZX_format.md`); TSX e' o mesmo formato
usado para MSX. O leitor (`src/tape/cpp/tzx_reader.cpp`) reconhece TODOS
os blocos da lista do TZX 1.20 (10, 11, 12, 13, 14, 15, 18, 19, 20, 21-28,
2A, 2B, 30-33, 35, 4B, 5A) -- o suficiente para nunca travar num arquivo
valido, mesmo quando um bloco nao e' reproduzido (ver secao 4).

O bloco que importa de verdade para o MSX e' o **#4B (Kansas City
Standard)** -- layout conferido contra `resource/makeTSX/TZX_Blocks.h`
(o proprio gerador de TSX para MSX) e o algoritmo de pulsos contra
`resource/openMSX_TSXadv/Contrib/tsx/TsxParser.cc` (so' leitura, GPL,
nenhum codigo copiado):

| Campo | Tamanho | Descricao |
|---|---|---|
| Tamanho do bloco | DWORD | `12+N`, sem contar estes 4 bytes |
| Pausa | WORD | milissegundos depois do bloco |
| Piloto | WORD | duracao do pulso do piloto (T-states) |
| Pulsos do piloto | WORD | quantos pulsos de piloto |
| Pulso de ZERO | WORD | T-states |
| Pulso de UM | WORD | T-states |
| bitCfg | BYTE | nibble alto = pulsos por bit 0 (2, MSX); nibble baixo = pulsos por bit 1 (4, MSX) |
| byteCfg | BYTE | bits 7-6 = numero de bits de inicio (1, MSX); bit 5 = valor deles (0); bits 4-3 = numero de bits de fim (2, MSX); bit 2 = valor deles (1); bit 0 = ordem (0 = LSb primeiro, MSX) |
| Dados | N bytes | os bytes PUROS do arquivo (ainda nao codificados em pulso) |

Os padroes entre parenteses (bitcfg `24h`, bytecfg `54h`) sao os mesmos
que o makeTSX usa por padrao para MSX -- conferido batendo a conta contra
o comentario do `Block4B` no proprio `TZX_Blocks.h`.

## 3. Modo rapido: o gancho de BIOS

Enderecos fixos (tabela de saltos da BIOS, pagina 0 -- conferidos em
`resource/fMSX/fMSX/Patch.c`, que faz a mesma coisa):

| Endereco | Rotina | O que o fwMSX faz |
|---|---|---|
| `00E1h` | TAPION | Acha o proximo marcador de 8 bytes no fluxo "rapido" (alinhando a 8, como o fMSX). Falha (`CARRY=1`) se nao achar ou se nao houver fita. |
| `00E4h` | TAPIN | Le o proximo byte do fluxo em `A`. Falha no fim do fluxo. |
| `00E7h` | TAPIOF | Sempre sucesso. |

So' o lado de LEITURA -- TAPOON/TAPOUT/TAPOOF (gravacao) nao sao
tocados; fora de escopo (sem `CSAVE` nesta fase).

O mecanismo e' o mesmo "ED FE" do fMSX (`PatchZ80()`), ja' preparado de
fabrica no nucleo Z80 do fwMSX (`bus->patch`, ver
`src/z80/core/opcodes_ed.h` e `src/z80/cpp/z80_bus.h`), so' que nunca
tinha sido usado ate' agora. `TapeEngine::SetMode(Fast)` escreve `ED FE`
nos 3 enderecos (`MemorySystem::PatchRomBytes()`, que ignora a regra de
"ROM nao e' gravavel" -- e' host-side, nao uma escrita do Z80);
`SetMode(Normal)` devolve os bytes originais (capturados na primeira
vez que a maquina liga). O proprio `SlotMemoryBus::on_bios_patch()` faz o
`RET` que o opcode "ED FE" nao faz (le o endereco de retorno da pilha).

**Para o fluxo "rapido" funcionar igual nos dois formatos**, o leitor de
.TSX reconstroi, so' a partir dos blocos #4B, um buffer EQUIVALENTE a um
.CAS (marcador de 8 bytes + os mesmos dados). Os outros tipos de bloco
(10, 11, 12, 13, 14...) nao contribuem para esse buffer -- so' para os
pulsos (modo normal, secao 4).

## 4. Modo normal: pulsos de verdade

Pulsos (meio-periodos, T-states de Z80) sao gerados para:

- **#4B**: piloto + cada byte (bits de inicio/dados/fim, conforme
  bitcfg/byteCfg) + pausa.
- **#10/#11/#14**: convencao do ZX Spectrum (piloto opcional + 2 pulsos
  de sincronismo opcionais + cada bit = 1 periodo completo -- 2 pulsos --
  da mesma duracao, MSb primeiro, sem bits de inicio/fim). Rara em fitas
  MSX de verdade (o MSX usa o #4B), mas suportada.
- **#12 (tom puro)** e **#13 (sequencia de pulsos)**: direto.
- **#20 (pausa)**: um pulso curto para terminar a borda, depois silencio
  pelo resto do tempo (regra do proprio TZX, secao 2 do format.md).
- **.CAS**: sem pulsos gravados -- sintetizados com os parametros padrao
  do MSX (piloto 855 T-states x 8000 pulsos [~1,9s], zero 1710, um 855,
  2/4 pulsos por bit, 1 bit de inicio=0, 2 de fim=1, LSb primeiro --
  `src/tape/cpp/cas_format.h`). **O pulso de ZERO e' o DOBRO do de UM, e
  o piloto tem a MESMA duracao do UM** -- essa e' a convencao real do
  bloco #4B do MSX (confirmada em `resource/makeTSX/rippers/
  MSX4B_Ripper.h/.cpp`, `bit0len = bit1len*2`), NAO a convencao generica
  de ZX Spectrum dos blocos #10/#11 (que tambem usa 855/1710, mas com
  ZERO e UM do jeito contrario). Um bug corrigido em 2026-10-08 tinha os
  dois valores trocados e o piloto curto demais (2000 pulsos, ~0.48s) --
  uma fita GRAVADA por este emulador carregava certo no modo rapido
  (nao usa pulso) mas nunca no modo normal (so' o "chiado", nunca achava
  o programa -- a BIOS de verdade nunca calibrava a tempo). Ver
  `doc/CHANGELOG.md`, `[1.20.1]`.

O cursor (`src/tape/core/tape_pulse.c`) avanca junto com a CPU
(`TapeEngine::Advance()`, chamado a cada ciclo por `Machine::RunFrame()`,
como o PSG/SCC/FM). O nivel atual alimenta:

- **A porta de entrada de cassete**: bit 7 do R14 do PSG (CASRD) --
  conferido contra o openMSX (`src/sound/MSXPSG.cc`, `readA()`): a
  entrada de cassete NAO fica no PPI, fica no PSG (porta A do AY-3-8910,
  compartilhada com o joystick). `psg_set_cassette_in()`
  (`src/psg/core/psg_state.c`).
- **O audio ao vivo**: uma onda quadrada simples (sem envelope), somada
  ao PSG/SCC/FM em `Machine::TakeLiveAudio()`, baixa o suficiente para
  nao dominar a mistura.

O **motor** (rele' do PPI, porta C bit 4, `AAh`, ativo em ZERO --
conferido contra o openMSX, `src/MSXPPI.cc`, `writeC1()`) decide se o
cursor avanca: `Machine::RunFrame()` olha `rout[2]&0x10` a cada ciclo e
chama `TapeEngine::SetMotor()` quando muda (o PPI em si nao sabe nada de
fita, so' latcheia o bit -- a sincronia fica na Machine, como o
`SyncSlot()` do proprio `PpiDevice` faz para o slot primario).

## 5. Limites (nesta fase)

- **Blocos de controle do TZX** (grupos 21/22, saltos 23, lacos 24/25,
  chamadas 26/27, selecao 28) sao pulados com seguranca (o comprimento e'
  sempre conhecido), mas SEM NAVEGAR -- o arquivo e' lido em sequencia,
  do primeiro ao ultimo bloco, sempre. Bleepload e protecoes parecidas
  (uso pesado desses blocos) nao vao funcionar direito.
- **Gravacao direta (#15), CSW (#18) e bloco generalizado (#19)** sao
  reconhecidos (comprimento correto) mas NAO reproduzidos -- blocos raros
  em fitas de MSX.
- **Fast_bytes so' a partir do #4B**: um .TSX que use #10/#11/#14 para o
  conteudo de verdade (em vez de so' o piloto) carrega certo no modo
  NORMAL, mas nao no modo RAPIDO (o `TAPION`/`TAPIN` nao acham nada no
  fluxo reconstruido). Nao e' o caso normal (o MSX usa #4B).
- **Banco de fitas e download**: nao existem ainda (itens (e)/(f) da
  ordem do SPEC).
- **A janela "Fita K7"** e' so' visual (reels girando quando o motor
  esta' ligado) -- nao mostra a posicao real da fita nos rolos, so' a
  barra de progresso em segundos; o contagiros (secao 6) tambem e' uma
  simulacao, nao fisicamente exata.
- **Sintese de pulsos de uma fita gravada -- RESOLVIDO na 1.20.2 para
  fitas gravadas por este emulador**: ate' a 1.20.1, tanto a sintese de
  pulsos em memoria quanto a escrita do `.tsx` persistido descobriam o
  tamanho de cada bloco "procurando o proximo cabecalho de 8 bytes",
  o que misturava os bytes de preenchimento (zeros que o proprio gancho
  de gravacao insere para alinhar o PROXIMO cabecalho, ver secao 6) com
  o conteudo de verdade do bloco ANTERIOR -- uma fita gravada, ejetada e
  recarregada podia nao dar `CLOAD` no modo normal. Corrigido com
  `TapeMark{fast_byte_offset, pulse_index, content_length}`
  (`tape_image.h`): o proprio motor de gravacao (`TapeEngine::FinalizeWrite()`)
  preenche o tamanho REAL de cada bloco no momento em que ele e' fechado
  (TAPOOF), sem precisar adivinhar depois -- usado tanto por
  `SynthesizeCasPulses()`/`PulseIndexFor()` quanto por `WriteTsxFromCas()`.
  Verificado com um teste que grava dois blocos, recarrega do ZERO a
  partir do arquivo persistido e confere que o segundo bloco tem
  exatamente os bytes gravados (`tests/z80/tape_test.cpp`).
  **Continua valendo** para um `.cas` CRU carregado direto do disco, sem
  ter passado por uma gravacao deste emulador: nesse caso nao ha' marca
  exata nenhuma (o `.cas` nao guarda comprimento por bloco), so' a busca
  pelo proximo cabecalho -- o modo RAPIDO nunca foi afetado em nenhum dos
  dois casos (le' so' os bytes que o programa pediu, exatamente como a
  BIOS real faria).

## 6. Gravacao (CSAVE/BSAVE "CAS:", fita nova, protecao, marcar o ponto)

A gravacao e' SEMPRE pelo gancho de BIOS (TAPOON/TAPOUT/TAPOOF, 00EAh/
00EDh/00F0h, mesmo mecanismo "ED FE" da leitura rapida) -- nao existe um
"modo normal" de gravar: precisaria decodificar os pulsos que o proprio
programa gera de volta em bytes (como um "ripper" de WAV), que e' muito
mais trabalho do que decodificar um arquivo ja' pronto (os bytes ja'
vem certos; so' as duracoes dos pulsos teriam que ser classificadas em
tempo real). Por isso TAPOON/TAPOUT/TAPOOF ficam PATCHEADOS SEMPRE,
independente do modo de leitura escolhido (`TapeEngine::ApplyWritePatch()`,
chamado uma vez na construcao do motor).

**TAPOON** (00EAh): falha (carry ligado, "Device I/O error" na BASIC) se
nao houver fita ou se ela estiver protegida contra gravacao
(`read_only()`). Caso contrario, decide ONDE escrever conforme o
`TapeWriteMode` escolhido (fitas sao lineares -- gravar a partir de um
ponto destroi fisicamente o que vinha depois, exatamente como um
gravador de fita de verdade):

- `AppendAtEnd` (padrao): escreve depois do ultimo arquivo -- "ir
  enchendo a fita com programas pequenos".
- `OverwriteAtPoint`: trunca a fita a partir do arquivo MARCADO
  (`SeekToFile()`, janela "Fita K7") e grava ali.
- `NewTape`: esquece tudo que havia (como se a fita tivesse sido
  apagada) e comeca do zero.

Em qualquer caso, alinha a posicao a um multiplo de 8 bytes antes de
escrever o cabecalho de 8 bytes (a MESMA regra do TAPION/TAPOON de
verdade -- ver secao 3/o comentario no caso #4B de `tzx_reader.cpp`).
**TAPOUT** (00EDh) so' funciona depois de um TAPOON com sucesso; cada
chamada acrescenta um byte (registrador A) ao fluxo. **TAPOOF** (00F0h)
sempre devolve sucesso; se havia uma gravacao em andamento, reconstroi
`files()`/`pulses()` a partir do fluxo atualizado e GRAVA o arquivo no
disco imediatamente (um `.tsx` valido -- um bloco #4B por cabecalho, com
os parametros padrao do MSX -- ou um `.cas` cru, pela extensao do
caminho da fita). Escritas vao direto para o arquivo, mesma filosofia
do disco ("as gravacoes do MSX-DOS vao direto para o arquivo").

**Protecao contra gravacao** (`TapeEngine::read_only()`): uma fita
inserida de um ARQUIVO (`Insert()`) comeca SEMPRE travada -- o usuario
destrava pelo menu Fita antes de gravar. Uma fita NOVA (`NewBlank()`)
comeca DESTRAVADA (e' o motivo de criar uma). **Reinserir a MESMA fita**
(mesmo caminho de arquivo que ja' estava inserido) preserva o estado de
protecao atual em vez de travar de novo -- corrigido na 1.20.2: como o
menu Fita reinsere a imagem atual para atualizar a lista, sem essa
excecao o usuario destravava a fita, marcava um ponto, e a protecao
"voltava" sozinha ao reabrir o menu, dando "Device I/O error" ao tentar
gravar. O menu tambem tem dois itens explicitos ("Destravar para
gravar" / "Travar contra gravacao", cada um habilitado so' quando faz
sentido) no lugar de um unico toggle ambiguo.

**Fita nova** (`TapeEngine::NewBlank()`, menu "Nova fita (.tsx)..."):
cria uma imagem vazia em memoria e grava, na hora, um `.tsx` valido (so'
o cabecalho `ZXTape!`, sem blocos) no caminho escolhido -- para o
arquivo existir no disco desde ja'.

**Marcar o ponto** (`TapeEngine::SeekToFile()`/`marked_file()`, janela
"Fita K7"): cada `TapeFileEntry` guarda `fast_byte_offset` (posicao no
fluxo "rapido") e `pulse_index` (posicao nos pulsos do modo normal) de
onde ele comeca. Marcar um arquivo da lista faz DUAS coisas ao mesmo
tempo: (1) o PROXIMO `TAPION` (carregamento) comeca a busca a partir
dali, em vez do inicio da fita -- como avancar manualmente a fita
rebobinada ate' o programa certo; (2) se o modo de gravacao for
`OverwriteAtPoint`, e' ali que a proxima gravacao trunca e escreve.

## 7. CLI e menu

- `--fita <arquivo>`: insere a fita (`.cas`, `.tsx` ou `.tzx`, pela
  extensao) ao iniciar -- SEMPRE protegida contra gravacao (so' o menu
  cria fita nova ou destrava uma existente).
- `--fita-modo rapido|normal`: escolhe o modo de CARREGAMENTO (padrao:
  rapido). Gravacao nao tem essa opcao (ver secao 6).
- Menu **Fita** da janela: inserir, **nova fita (.tsx)...**, ejetar,
  rebobinar, trocar o modo de carregamento, **destravar/travar contra
  gravacao**, **modo de gravacao** (incluir no final / sobrescrever o
  ponto marcado / nova fita), e mostrar a janela visual "Fita K7" --
  nela, clicar num arquivo da lista marca o ponto (seção 6); clicar de
  novo desmarca. Tudo a qualquer momento, sem reiniciar a maquina.
