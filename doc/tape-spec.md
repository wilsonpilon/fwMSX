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
  chamadas 26/27, selecao 28) agora sao NAVEGADOS de verdade, nao so'
  pulados -- ver secao 10. **Selecao (#28)** continua com uma limitacao
  por natureza: sem como mostrar um menu de verdade numa ferramenta
  batch, escolhe sempre a 1a opcao da lista.
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

**`OverwriteAtPoint`/`NewTape` cortam/limpam UMA SO' VEZ -- corrigido na
1.20.3**: um CSAVE/BSAVE de verdade chama `TAPOON`/`TAPOUT`/`TAPOOF` DUAS
vezes (um bloco so' para o cabecalho com o nome, outro so' para os dados
de verdade -- cada bloco tem seu proprio piloto/sincronismo). Ate' a
1.20.2, `OnTapoon()` reaplicava o corte/limpeza em CADA chamada, nao so'
na primeira -- na 2a chamada (bloco de dados), "o ponto marcado" ja' nao
era mais o programa antigo, e sim o CABECALHO COM NOME que a 1a chamada
tinha acabado de escrever, apagando-o e deixando so' os dados, sem nome
nenhum (o programa "desaparecia" por completo, nem o antigo nem o novo
ficavam reconheciveis). Corrigido: depois da 1a chamada que corta/limpa,
`write_mode_` volta sozinho para `AppendAtEnd` -- qualquer bloco seguinte
(do mesmo CSAVE, ou de um CSAVE futuro sem marcar outro ponto) so'
acrescenta, nunca corta de novo. Equivale ao comportamento fisico real:
depois de cortar a fita e comecar a gravar, o que vem a seguir so' pode
ir para a frente.

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

## 8. Empacotador .BIN/.BAS -> .TSX (`fwmsx --cas`)

Ferramenta de linha de comando (`src/tape/cli/cas_tool.cpp`, roteada em
`src/cpp/main.cpp`) para empacotar um arquivo solto num `.TSX` (ou
`.CAS`) valido **sem precisar abrir o emulador** -- util para quem ja
tem o binario/programa tokenizado pronto (de um cross-assembler, de uma
extracao, ou de um `BSAVE` feito antes dentro do proprio fwMSX) e so'
quer uma fita pronta para `BLOAD`/`CLOAD "CAS:"`.

```
fwmsx --cas pack --tipo bin|bas --nome NOME [opcoes] <entrada> <saida.tsx|.cas>
fwmsx --cas rip [--tolerancia N] [--anexar <arquivo>] <entrada.wav> <saida.tsx|.cas>
fwmsx --cas list <arquivo.tsx|.tzx|.cas>
```

- `--tipo bin`: para `BLOAD "CAS:"`. Precisa de `--inicio`/`--fim`/
  `--exec` (enderecos em hexadecimal, ex. `0x8000`) -- os mesmos 6 bytes
  (3 enderecos de 16 bits, little-endian) que um `BSAVE` de verdade
  grava ANTES dos bytes crus do arquivo de entrada (ver secao 2.1). O
  arquivo de entrada e' so' o conteudo binario puro, sem cabecalho
  nenhum -- a ferramenta monta o cabecalho a partir das 3 opcoes.
- `--tipo bas`: para `CLOAD "CAS:"`. O arquivo de entrada tem que estar
  **ja tokenizado** -- os bytes exatos que um `CSAVE` gravaria (um
  programa BASIC do MSX embute ponteiros de memoria entre as linhas, e
  so' faz sentido tokenizar sabendo o endereco onde vai ficar carregado
  -- por isso esta ferramenta nao tokeniza texto solto, so' empacota
  bytes que ja estao no formato certo). `BSAVE` dentro do proprio fwMSX
  (do inicio ao fim do programa na memoria) e' uma forma facil de obter
  esse arquivo.
- `--nome`: vai para o cabecalho de 6 bytes do `.CAS` (secao 2.1) --
  maiusculas (convencao do MSX) e truncado com aviso se for maior.
- `--anexar <arquivo>`: carrega uma fita existente (`.cas`/`.tsx`/
  `.tzx`, pela extensao) e acrescenta o novo arquivo no FINAL dela, em
  vez de criar uma fita so' com ele -- a saida pode ser o MESMO caminho
  do `--anexar` (reescreve a fita com um arquivo mais).
- A saida e' sempre um `.tsx` valido (reusa `WriteTsxFromCas()`, o
  MESMO escritor que `TapeEngine` usa para gravar -- ver secao 6) ou um
  `.cas` cru, pela extensao do caminho.
- `rip`: "ripa" uma gravacao real de fita (`.wav`) para um `.TSX` valido
  -- ver secao 9 para o algoritmo e os limites.
- `list`: mostra indice, tipo, nome e tamanho dos dados de cada arquivo
  de uma fita -- util para confirmar o resultado do `pack`/`rip` sem
  abrir a janela "Fita K7".

Os dois blocos escritos por `pack` (cabecalho com nome + dados) sao
exatamente os mesmos dois blocos que um `CSAVE`/`BSAVE` de verdade
grava (ver secao 6 e o bug da 1.20.3) -- a ferramenta so' monta esses
blocos fora do gancho de BIOS, sem motor, sem Z80
(`src/tape/cpp/cas_pack.{h,cpp}`). Testes: `castooltest` (CTest
`cas_pack`).

**Fora de escopo (por enquanto, so' para `pack`):** `--tipo ascii` (o
formato ASCII em blocos de 256 bytes com preenchimento `1Ah` e' mais
complexo, e nao foi pedido); tokenizar um `.BAS` em TEXTO puro
(precisaria de um tokenizador completo do MSX BASIC, incluindo os
ponteiros de linha dependentes do endereco de carga -- um projeto bem
maior que "empacotar um arquivo solto").

## 9. Ripper de .WAV (`fwmsx --cas rip`)

Demodula uma gravacao real de fita (um `.wav` PCM mono, 8 ou 16 bits --
`src/tape/cpp/wav_reader.{h,cpp}`) para o formato `.CAS` interno
(`fast_bytes`/`marks`), detectando o piloto e decodificando os bytes do
bloco #4B (Kansas City Standard) -- o MESMO caminho de escrita do
`pack` (secao 8) depois disso. Algoritmo **portado do conceito do
makeTSX** (`resource/makeTSX/BlockRipper.cpp` e
`rippers/MSX4B_Ripper.cpp`, MIT -- nenhum codigo copiado, so' a ideia
geral, simplificada para o caso fixo do MSX). Codigo em
`src/tape/cpp/wav_ripper.{h,cpp}` e a decodificacao KCS em si (o
inverso de `kcs_emit_byte()`) em `src/tape/core/kcs_codec.{h,c}`
(`kcs_decode_byte()`).

**Algoritmo:**

1. **Deteccao de pulsos**: percorre as amostras do `.wav` com um limiar
   adaptativo (20% do pico absoluto do arquivo, com zona morta no meio
   para nao contar ruido perto de zero como transicao) -- cada
   transicao de nivel (baixo <-> alto) gera a duracao (em AMOSTRAS) do
   pulso anterior. Equivalente ao `BlockRipper::initializeStatesVector()`
   do makeTSX, sem a "fase" dele (nao e' necessaria para decodificar
   KCS -- um pulso e' so' uma duracao, o lado nao importa).
2. **Deteccao do piloto**: a partir de um pulso ainda nao consumido,
   conta quantos pulsos seguidos tem duracao parecida (dentro da
   tolerancia) com a MEDIA acumulada dos anteriores do mesmo trecho
   (comeca so' com o 1o como referencia, refinando a cada pulso aceito
   -- mais robusto contra jitter/arredondamento da gravacao que fixar
   so' no 1o pulso). Precisa de pelo menos 400 pulsos assim em sequencia
   para contar como piloto de verdade (o makeTSX usa 500). A duracao
   MEDIA medida vira o `one_len` do bloco; `zero_len = one_len*2`
   (convencao fixa do MSX, independente da velocidade real da fita --
   ver `cas_format.h`).
3. **Decodificacao byte a byte**: `kcs_decode_byte()` confere 1 bit de
   inicio (tem que bater DE VERDADE, sem tolerancia extra -- e' isso
   que impede o piloto do PROXIMO bloco, uma sequencia pura de pulsos
   do tamanho do bit 1, de ser lido como um fluxo infinito de bytes
   `0xFF`), 8 bits de dados (LSb primeiro, cada um decidido por
   qual das duas opcoes -- 2 pulsos de `zero_len` ou 4 de `one_len` --
   bate dentro da tolerancia; ambiguo ou nenhum bate = fim do bloco) e 2
   bits de fim (aceitos mesmo fora da tolerancia, incluindo faltar
   pulso nenhum no fim do arquivo -- a essa altura o byte ja foi
   decidido pelos bits de dados, um bit de fim ruidoso ou ausente nao
   deve descartar um byte bom).
4. Decodifica bytes assim enquanto conseguir; quando um byte falha
   (fim do bloco, piloto do proximo, silencio, ou dado corrompido
   demais), fecha o bloco atual (`AppendCasBlock()`, mesma funcao do
   empacotador -- secao 8) e volta ao passo 2 procurando o PROXIMO
   piloto a partir de onde parou.
5. No fim, `ScanCasFiles()`/`WriteTsxFromCas()` (as MESMAS funcoes de
   sempre) reconhecem os arquivos e escrevem o `.tsx` -- a saida usa os
   parametros de pulso CANONICOS do MSX (`cas_format.h`), nao os
   medidos na gravacao: "ripar" uma fita tambem normaliza a velocidade
   (remove o "embalo" natural de um motor de fita analogico).

**Testado contra uma fita de verdade** (gravacao real de MSX dos anos
80, `resource/openMSX/Contrib/reverse_engineering_tools/kanji/
ktst31 [RUN'CAS-'].wav`, GPL, so' usada aqui como teste manual -- nunca
versionada como parte deste projeto): reconheceu 86 blocos (4 arquivos
ASCII, `KTST31`/`KT31A`/`KT31B`/`KT31C`) sem nenhum erro. O teste
automatizado (`castooltest`/CTest `cas_pack`) usa um `.wav` SINTETICO
(gerado no proprio teste a partir de bytes conhecidos, via
`kcs_emit_byte()` -- sem precisar de uma gravacao real) para o
round-trip completo: bytes -> audio -> `rip` -> bytes, byte a byte.

**Limites desta primeira versao** (comparados ao makeTSX original):

- **So' o bloco #4B/KCS do MSX** (bitcfg/bytecfg fixos -- o unico que o
  MSX usa de verdade). O makeTSX suporta blocos #10/#11/#12/#13/#15
  tambem (generico de ZX Spectrum) -- fora de escopo aqui.
- **Sem os modos interativo/preditivo** do makeTSX original (que pede
  ajuda ao usuario pela linha de comando, ou tenta adivinhar um bit
  ambiguo "olhando para frente" nos proximos bits/bytes antes de
  decidir). Aqui, um bit ambiguo simplesmente termina o bloco corrente
  -- gravacoes muito ruidosas podem perder o resto de um bloco por
  causa disso, em vez de recuperar o que vier depois do trecho
  corrompido.
- **Sem filtros de volume** (`normalize()`/`envelopeCorrection()` do
  original) -- so' deteccao de limiar adaptativo (fracao do pico do
  arquivo todo). Gravacoes com volume muito baixo, ou com um volume que
  varia MUITO ao longo da fita, podem nao ser detectadas bem numa
  passada so'.
- `--tolerancia` (1-90%, padrao 25) e' um unico numero para toda a
  gravacao -- o makeTSX usa duas janelas diferentes (pulso individual
  vs. soma do grupo) com valores fixos (16%/22%). Mais simples, um
  pouco menos preciso em casos extremos.
- Arquivos ASCII de verdade sao gravados em VARIOS blocos de 256 bytes
  (um cabecalho #4B por bloco) -- `ScanCasFiles()` so' sabe juntar DOIS
  blocos por arquivo (nome + UM bloco de dados, ver secao 2.1), entao
  cada sub-bloco de 256 bytes de um arquivo ASCII aparece como um
  "arquivo" separado (e incompleto) na lista, mesmo com a decodificacao
  de pulsos em si (o `rip`) funcionando certo. O teste manual contra a
  fita real (abaixo) mostrou exatamente isso. Juntar os sub-blocos de
  um ASCII multi-bloco e' trabalho futuro do leitor .CAS, nao do
  `rip`.

## 10. Navegacao de blocos de controle do TZX

Os blocos de controle do TZX (IDs 21-28, `TZX_format.md`) mudam a ORDEM
de execucao do arquivo -- antes da 1.23.0, eram so' pulados com
seguranca (o comprimento sempre conhecido, por isso a leitura nunca
travava), mas a ordem real era ignorada: o arquivo sempre era lido do
primeiro ao ultimo bloco, em sequencia. Corrigido com um leitor em DUAS
passadas em `src/tape/cpp/tzx_reader.cpp`:

1. **1a passada** (`SkipOneBlock()`): percorre TODO o arquivo so' para
   descobrir onde cada bloco comeca (sem gerar pulso nem conteudo
   nenhum) -- necessario porque saltos/lacos/chamadas/selecao se
   referem a OUTROS blocos pelo NUMERO DE ORDEM (ex.: "salto 2" = "pule
   1 bloco"), nao pelo deslocamento em bytes no arquivo. Sem indexar
   tudo ANTES, um salto para a FRENTE nao teria como saber quantos
   bytes os blocos no meio do caminho ocupam.
2. **2a passada** (o loop de navegacao em `LoadTzxImage()`, mais
   `ExecuteDataBlock()` para os blocos "passivos" -- #10 a #20, #2A,
   #2B, #30-35, #4B, #5A, que nao mudam a ordem): executa de verdade,
   com um "PC" (indice do bloco atual na lista da 1a passada, NAO um
   deslocamento em bytes).

**Semantica de cada bloco** (ver TZX_format.md para os detalhes completos):

- **Grupo (#21/#22)**: so' um marcador (nome do grupo) -- sem efeito na
  ordem, sempre foi assim.
- **Salto (#23)**: deslocamento relativo ao PROPRIO bloco do salto
  (`novo_pc = pc + valor`; "salto 1" = bloco seguinte = NOP; "salto 0"
  seria um laco infinito -- a propria especificacao do TZX avisa que
  "isso nunca deveria acontecer"). Um salto para fora dos limites do
  arquivo e' um ERRO (nao ha' como continuar, o arquivo esta' malformado).
- **Laco (#24/#25)**: `#24` guarda quantas vezes repetir; o corpo (os
  blocos entre `#24` e o `#25` correspondente) toca essa quantidade de
  vezes antes de continuar depois do `#25`. Zero repeticoes pula o
  corpo inteiro, sem tocar nem uma vez (procura o PROXIMO `#25` -- a
  especificacao nao permite lacos aninhados, entao e' sempre o
  correspondente). Usamos uma pilha de lacos por seguranca contra um
  arquivo malformado, apesar da especificacao proibir o aninhamento.
- **Chamada (#26/#27)**: `#26` guarda uma LISTA de deslocamentos (como
  varias sub-rotinas chamadas em sequencia); cada `#27` encontrado
  avanca para a PROXIMA chamada da MESMA lista, e quando a lista se
  esgota, volta para o bloco logo depois do `#26` original. Uma pilha
  de chamadas permite aninhamento com lacos (permitido pela
  especificacao: "you can use CALL blocks in LOOP sequences and vice
  versa").
- **Selecao (#28)**: a especificacao pede um MENU interativo (varias
  opcoes com descricao, o usuario escolhe uma) -- sem como fazer isso
  numa ferramenta batch sem interface, **escolhe sempre a 1a opcao da
  lista**, por padrao. Documentado como limitacao (ver secao 5).

**Protecao contra laco infinito**: um TZX malformado (ou deliberadamente
malicioso) poderia ter um salto/laco/chamada que nunca termina -- um
contador de passos com um limite bem generoso (bem mais que o numero de
blocos do arquivo) interrompe a leitura com um erro claro nesse caso,
em vez de travar o processo para sempre.

Testes: `tapetest` (CTest `tape_load`) com 6 checagens novas -- monta um
`.tsx` sintetico pequeno para cada caso (salto pula 1 bloco; laco repete
o corpo 3 vezes; laco com 0 repeticoes pula o corpo inteiro; chamada com
lista de 2 execucoes + retorno; selecao escolhe a 1a opcao; salto fora
dos limites e' recusado com erro) e confere a ORDEM REAL de execucao
pela ordem dos arquivos reconhecidos no resultado (`img.files`).

## 11. Banco de fitas (`fwmsx --fitadb`)

Banco de metadados (SQLite) das fitas que o usuario ja' tem no disco --
titulo, empresa, ano e SHA-1 -- fecha o item (e) da lista original de
fitas (`doc/SPEC.md`, secao 5.2). **Sem download nenhum**: o site de
referencia (tsx.eslamejor.com, "TSX MSX Files Repository") nao publica
termos de uso (paginas de termos/politica/FAQ dao 404, verificado em
2026-10-06) e oferece a colecao so' como um `.torrent`, nao downloads
individuais -- sem uma licenca clara, o fwMSX nao implementa download
automatico nenhum (ver `doc/SPEC.md`, secao 5.2, item 7, para a analise
completa). Este banco so' CADASTRA o que o usuario ja' tem localmente,
igual `fwmsx --romdb scan` faz para ROMs.

**Modulo** (`src/tapedb/`): estrutura espelhada em `src/romdb/`
(`store/`, `service.h/.cpp`, `cli.h/.cpp`) POR ANALOGIA -- mesmo padrao
de CRUD/busca/SQLite, mas um banco SEPARADO (`fitas/fitas.db`, nao uma
categoria dentro de `roms.db`, ja' que fita nunca foi uma categoria de
ROM). Reusa `romdb::Sha1Hex()` diretamente (e' uma funcao generica, nao
especifica de ROM) em vez de duplicar o calculo de hash.

```
fwmsx --fitadb scan <pasta> [--origem x]
fwmsx --fitadb add <arquivo> [--titulo t] [--empresa e] [--ano a] [--notas t]
fwmsx --fitadb list
fwmsx --fitadb search <texto>
fwmsx --fitadb show <id|sha1>
fwmsx --fitadb edit <id> [--titulo t] [--empresa e] [--ano a] [--notas t]
fwmsx --fitadb del <id>
fwmsx --fitadb stats
```

- `scan`: varre uma pasta (recursivo), cadastrando toda fita `.cas`/
  `.tsx`/`.tzx` encontrada pelo SHA-1 do ARQUIVO (nao do bloco #4B --
  um `.tsx` e' um arquivo binario como outro qualquer para esse fim).
- **Auto-preenchimento do titulo**: `TapeDb::ScanFile()` abre a fita com
  o leitor existente (`cas_reader`/`tzx_reader`, o MESMO que a janela e
  o `--cas` usam) e usa o nome do 1o arquivo encontrado dentro dela como
  palpite de titulo -- so' quando a fita e' NOVA no banco. Uma fita ja'
  cadastrada MANTEM o titulo que o usuario tiver editado manualmente,
  mesmo re-escaneando a mesma pasta depois (mesma logica de preservar
  nome/notas que o `romdb` ja' usava para ROMs).
- `empresa`/`ano` (convencao TOSEC) nunca vem da fita em si -- nenhum
  desses dois formatos (`.cas`/`.tsx`/`.tzx`) guarda editora ou ano --
  so' podem ser preenchidos manualmente (`--empresa`/`--ano`).
- `--fitas <pasta>`: pasta de fitas e banco, em qualquer comando (padrao:
  `fitas/` ao lado do executavel, mesma convencao de `--roms` no
  `--romdb`).

Testes: `tapedbtest` (CTest `tapedb_store`) -- CRUD, busca,
`ScanFile`/`ScanDirectory` (com o auto-preenchimento de titulo e a
preservacao de edicoes manuais em um re-scan), e a CLI completa (todos
os comandos + validacao de argumentos). Smoke test manual contra uma
fita real (`resource/fmsxgo/media/teste.tsx`): `--fitadb add` detectou
o titulo certo ("teste") sozinho.

**Fora de escopo (por natureza, nao por falta de tempo):** qualquer
download automatico (ver acima); importar um indice/torrent de terceiros
sem antes confirmar os termos de uso por escrito.
