# Changelog

Resumo das alteracoes entre versoes do fwMSX, ligado ao
[README.md](../README.md). Formato inspirado em
[Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/). Para o
detalhamento completo de cada versao (nome do jogo de MSX + subtitulo,
notas de build) veja [RELEASE.md](RELEASE.md); para a especificacao viva
e o historico de fases, veja [SPEC.md](SPEC.md).

## [1.21.0] - 2026-10-08 - "King's Valley: Empacotador de Fita (BIN/BAS -> TSX)"

### Adicionado
- **`fwmsx --cas pack`**: empacota um `.BIN`/`.BAS` solto (ja no formato binario/tokenizado do
  MSX) num `.TSX` (ou `.CAS`) valido, pronto para `BLOAD`/`CLOAD "CAS:"`, sem precisar abrir o
  emulador. `--tipo bin` monta o cabecalho de 6 bytes de endereco (inicio/fim/execucao,
  `--inicio`/`--fim`/`--exec` em hexadecimal) na frente do binario; `--tipo bas` empacota os bytes
  tokenizados como estao (um programa BASIC do MSX embute ponteiros de memoria entre linhas, por
  isso a ferramenta nao tokeniza texto solto -- so' empacota bytes que ja estao no formato certo,
  por exemplo extraidos com `BSAVE` dentro do proprio emulador). `--anexar <arquivo>` carrega uma
  fita existente e acrescenta o novo arquivo no final, em vez de criar uma fita so' com ele.
- **`fwmsx --cas list`**: lista indice, tipo, nome e tamanho dos dados de cada arquivo de uma
  fita (`.cas`/`.tsx`/`.tzx`) -- util para confirmar o resultado do `pack` sem abrir a janela
  "Fita K7".
- Reaproveita o MESMO escritor (`WriteTsxFromCas()`) que `TapeEngine` usa para gravar pela BIOS --
  os blocos gerados (cabecalho com nome + dados) sao exatamente os dois blocos que um
  `CSAVE`/`BSAVE` de verdade produziria. Primeiro item da lista de pendencias de fita (`doc/SPEC.md`,
  secao 5.2) a ser fechado.
- Testes: `castooltest` (CTest `cas_pack`) -- pack de `.bin`/`.bas`, `--anexar`, `list`, e
  validacao de argumentos, sem precisar de Z80/mapa de memoria (`CAS_PACK_SOURCES`, so' leitura/
  escrita de arquivo). Ver `doc/tape-spec.md`, secao 8.

## [1.20.3] - 2026-10-08 - "King's Valley: Gravacao em K7: Um Corte So'"

### Corrigido
- **"Sobrescrever o ponto marcado" apagava TUDO (o antigo e o novo)**: um CSAVE/BSAVE de verdade
  chama TAPOON/TAPOUT/TAPOOF DUAS vezes -- um bloco so' para o cabecalho com o nome do programa,
  outro so' para os dados -- e `OnTapoon()` reaplicava o corte ("sobrescrever o ponto marcado") em
  CADA chamada, nao so' na primeira. Na 2a chamada (bloco de dados), "o ponto marcado" passava a
  ser o PROPRIO cabecalho com nome que a 1a chamada tinha acabado de escrever -- apagando-o e
  deixando so' os dados, sem nome nenhum. Resultado: nem o programa antigo (de verdade apagado,
  como esperado) nem o novo (com o cabecalho destruido) ficavam reconheciveis -- "como se o
  programa tivesse sumido". "Nova fita" tinha o mesmo problema (limpava tudo de novo na 2a
  chamada). Corrigido: depois de cortar/limpar UMA VEZ, o modo volta sozinho para "incluir no
  final" -- qualquer bloco seguinte (do mesmo CSAVE ou de um CSAVE futuro) so' acrescenta, nunca
  corta de novo. Equivale ao comportamento fisico de uma fita de verdade: depois de cortar e
  comecar a gravar, o que vem a seguir so' pode ir para a frente. Relatado pelo usuario testando
  "sobrescrever o ponto marcado" pela janela de verdade; teste de regressao novo em `tapetest`
  simula os 2 blocos de um CSAVE real e confere que o nome e os dados do novo programa sobrevivem
  os dois.

## [1.20.2] - 2026-10-08 - "King's Valley: Gravacao em K7: Protecao e Preenchimento"

### Corrigido
- **Contagiros parado no CLOAD**: nao era bug -- o contador so' avanca enquanto o motor gira
  (`motor_on()`), e numa fita curta (poucos segundos) o avanco e' sutil. Confirmado via
  diagnostico direto (`Machine::RunFrame()` + `tape().odometer()`): sobe normalmente durante
  todo o CLOAD e congela quando o motor para (fim do carregamento).
- **"Device I/O error" ao trocar para "sobrescrever" ou "nova fita" apos marcar um ponto**:
  `TapeEngine::Insert()` sempre travava a fita de novo (`read_only_ = true`), MESMO ao reinserir
  a fita que o usuario ja' tinha destravado -- bastava reabrir o menu Fita para a protecao voltar
  silenciosamente. Corrigido: reinserir a MESMA fita preserva o estado de protecao atual. O menu
  tambem trocou o unico item ambiguo ("Fita protegida contra gravacao") por dois itens explicitos
  ("Destravar para gravar" / "Travar contra gravacao"), cada um habilitado so' quando faz sentido,
  deixando o estado atual sempre visivel.
- **Preenchimento de alinhamento (padding) gravado como dado**: `TAPOON` insere bytes zero antes
  de cada cabecalho novo so' para alinhar em 8 bytes (como a BIOS de verdade) -- esses bytes NUNCA
  foram conteudo real, mas os sintetizadores de pulso (`SynthesizeCasPulses`, o antigo
  `RebuildFromFastBytes` e `WriteTsxFromCas`) descobriam o tamanho de cada bloco "procurando o
  proximo cabecalho", o que misturava esse preenchimento com o bloco ANTERIOR de verdade. Corrigido
  com uma nova marca exata (`TapeMark{fast_byte_offset, pulse_index, content_length}`) que o motor
  de gravacao preenche com o tamanho REAL de cada bloco, sem adivinhar -- usada tanto na sintese de
  pulsos em memoria quanto na escrita do `.tsx` persistido. Bug relatado pelo usuario ao reinserir
  uma fita gravada e dar CLOAD no modo normal; confirmado tanto em memoria quanto reabrindo o
  arquivo do disco (teste novo em `tapetest`: grava dois blocos, recarrega do zero, confere que o
  segundo bloco tem EXATAMENTE os bytes gravados, sem contaminacao do preenchimento do primeiro).
  Limitacao que CONTINUA valendo para `.cas` carregado direto do disco (sem passar pela gravacao
  deste emulador): ali ainda nao ha' tamanho exato por bloco, so' seguro para fitas geradas por este
  fluxo -- ver doc/tape-spec.md.

## [1.20.1] - 2026-10-08 - "King's Valley: Gravacao em K7: Corrigindo o Piloto"

### Corrigido
- **Pulsos de ZERO e UM do bloco #4B estavam TROCADOS** (`src/tape/cpp/cas_format.h`): a convencao
  real do MSX e' zero = 2x o pulso de um, com o piloto na MESMA duracao do um -- confirmado contra o
  proprio gerador do makeTSX (`resource/makeTSX/rippers/MSX4B_Ripper.h/.cpp`, `bit0len = bit1len*2`).
  A nota antiga (ZERO=855, UM=1710) vinha dos defaults GENERICOS de ZX Spectrum (blocos #10/#11), nao
  do #4B do MSX, e nunca tinha sido conferida contra o codigo (como a propria nota avisava).
- **Piloto curto demais** para a BIOS de verdade calibrar (2000 pulsos, ~0.48s) -- aumentado para 8000
  pulsos (~1,9s), confirmado empiricamente contra o arquivo real do usuario e contra um .TSX comercial.
- Resultado pratico: uma fita GRAVADA por este emulador (1.20.0) carregava certo no modo rapido (nao
  usa pulso nenhum), mas o modo normal nunca achava o programa -- so' o barulho do piloto, sem
  calibrar. Relatado pelo usuario ao testar `CSAVE"TESTE"` + `CLOAD`. Testes de regressao em
  `tapetest` (3 novas checagens sobre a relacao zero/um/piloto).

## [1.20.0] - 2026-10-07 - "King's Valley: Gravacao em K7"

### Adicionado
- **Gravacao de fita** (`CSAVE`/`BSAVE "CAS:"`): TAPOON/TAPOUT/TAPOOF (00EAh/00EDh/00F0h) sempre pelo
  gancho de BIOS "ED FE" -- independente do modo de carregamento escolhido (nao ha' "modo normal" de
  gravar; precisaria de um "ripper" de pulsos em tempo real, fora de escopo).
- **Fita nova em branco** (`.tsx`, menu Fita > "Nova fita..."): cria uma imagem vazia e grava um .tsx
  valido no disco na hora, para o usuario salvar programas do zero.
- **Protecao contra gravacao**: uma fita inserida de um arquivo entra sempre travada (so' leitura); uma
  fita nova entra destravada. Travar/destravar pelo menu Fita, a qualquer momento.
- **Tres modos de gravacao** (fitas sao lineares, como as de verdade -- gravar a partir de um ponto
  destroi o que vinha depois): **incluir no final** (padrao, "ir enchendo a fita com programas
  pequenos"), **sobrescrever o ponto marcado**, ou **nova fita** (apaga tudo e comeca do zero).
- **Marcar um arquivo da fita**: a janela "Fita K7" agora lista os arquivos de forma clicavel -- marcar
  um deles decide de onde o proximo carregamento comeca a procurar, e onde a proxima gravacao (no modo
  "sobrescrever o ponto") trunca e escreve.
- **Contagiros**: um odometro simulado na janela "Fita K7", avancando conforme a fita "roda" -- a
  mesma sensacao dos gravadores de fita antigos.
- Escritor generico de .CAS para .TSX (`src/tape/cpp/tsx_writer.cpp`), usado pela gravacao e pela fita
  nova; diálogo "Salvar Como" generico na GUI (`ShowSaveFileDialog`, antes so' existia para `.dsk`).
- Testes: `tapetest` ganhou 22 checagens novas (gravacao round-trip, protecao, os 3 modos, marcar/
  desmarcar, contagiros) -- 68 no total.

### Limitacoes conhecidas
- A sintese de pulsos do modo normal (`SynthesizeCasPulses`) trata o preenchimento de alinhamento
  (zeros inseridos pelo proprio TAPOON antes de um cabecalho) como se fosse dado de verdade -- so'
  afeta a reproducao por PULSOS (modo normal) de uma fita GRAVADA por este emulador; o modo rapido nao
  e' afetado. Ver `doc/tape-spec.md`, secao 5.
- A gravacao/protecao/marcacao so' foi validada por teste automatizado; ninguem clicou nos novos itens
  de menu ainda.

## [1.19.1] - 2026-10-07 - "Yie Ar Kung-Fu: Fita K7: Corrigindo o Carregamento"

### Corrigido
- **Leitor de .TSX**: o bloco #35 (Custom info) usava 10 bytes para a string de identificacao em vez
  de 16 -- o `TZX_format.md` escreve o deslocamento do campo seguinte em HEXADECIMAL (`0x10` = 16, nao
  10). Qualquer .TSX real com esse bloco (makeTSX grava "TSX.RIPPER" nele) tinha a leitura inteira
  corrompida a partir dali. Confirmado com um arquivo `.tsx` real do usuario.
- **Reconstrucao do fluxo "rapido" a partir do .TSX**: faltava o preenchimento até o proximo multiplo
  de 8 bytes ANTES de cada cabecalho (igual ao TAPOON de verdade, que alinha ao gravar um .CAS) --
  sem isso, o SEGUNDO `BLOAD"CAS:"`/`CLOAD` de uma fita com mais de um arquivo desalinhava o TAPION e
  nunca mais achava cabecalho nenhum ("Device I/O error"; em modo normal, a BIOS ficava esperando um
  sinal que nunca chegava, travando a maquina). Testes de regressao em `tapetest` para os dois casos.

## [1.19.0] - 2026-10-07 - "Yie Ar Kung-Fu: Fita K7"

### Adicionado
- **Fita** (`.cas`, `.tsx` e `.tzx`): leitura completa do TZX 1.20 (bloco #4B, o unico que o MSX usa de
  verdade, com pulsos e dados completos; os demais blocos pulados com seguranca, sem travar o arquivo).
  Ver [doc/tape-spec.md](tape-spec.md).
- **Dois modos de carregamento**, trocaveis sem reiniciar a maquina: **rapido** (gancho de BIOS "ED FE"
  em TAPION/TAPIN/TAPIOF, sem som) e **normal** (pulsos de verdade na cadencia de T-states da CPU, com o
  barulho do gravador -- a BIOS roda a rotina ORIGINAL dela, sem patch nenhum).
- **Entrada de cassete no PSG** (R14, bit 7, conferido contra o openMSX) e **motor no PPI** (porta C,
  bit 4); audio sintetizado (onda quadrada) somado ao PSG/SCC/FM.
- `--fita <arquivo>` e `--fita-modo rapido|normal`; menu **Fita** da janela (inserir, ejetar, rebobinar,
  trocar de modo) e a janela visual **"Fita K7"** (dois rolos que giram enquanto o motor esta' ligado,
  barra de progresso e os arquivos encontrados na fita).
- Testes: `tapetest` (CTest `tape_load`, 39 checagens: cursor de pulsos, framing KCS, leitores de .CAS e
  .TSX, o gancho de BIOS de ponta a ponta com um Z80 de verdade, e a entrada de cassete no PSG).

### Limites conhecidos
- Sem escrita (`CSAVE`/`BSAVE "CAS:"`); sem banco de fitas nem download; blocos de controle do TZX
  (grupos, lacos, saltos) sao pulados mas nao navegados. Ver [doc/tape-spec.md](tape-spec.md), secao 5.

## [1.18.0] - 2026-10-06 - "Ys: Banco de ROMs e Disco por Portas"

### Adicionado
- **Banco de ROMs** (SQLite, `fwmsx --romdb`): ROMs do disco pelo SHA-1, com nome, categoria, hardware, mapper,
  notas e caminho. CRUD, busca, escanear pastas, adicionar arquivos. Ver [doc/romdb-spec.md](romdb-spec.md).
- **Downloads** pelo menu **ROMs** da janela e pela CLI: fMSX 6.0 para Windows (separado por tipo), System ROMs
  do file-hunter (o Full Set mais recente, ou navegar pelas pastas e baixar arquivos avulsos) e o banco do
  Vampier (dump SQL importado no SQLite). Hoje as ROMs baixadas ficam versionadas em `dist/roms/`, decisao
  provisoria do projeto pessoal (ver `LICENSE-THIRD-PARTY.md`).
- **Tabela de mappers** do `CARTS.SHA` do fMSX no banco.
- **Identificacao**: ROMs cadastradas recebem o nome do jogo pelo banco do Vampier (SHA-1).
- **Controladora de disco por portas** no estilo Microsol (DDX 3.0 e CDX-2): WD2793 em `D0h`..`D4h`, com a
  ROM do driver no slot. O MSX-DOS sobe com os dois drivers. Escolha no menu Maquina > Configuracao de disco
  ou com `--disk-acesso porta --disk-porta D0h --diskrom <driver>`.
- **Formatos de disco 180, 360 e 720 KB** pela configuracao do drive: 5 1/4 ou 3 1/2, face simples ou dupla,
  densidade simples ou dupla (ou automatico). `--disk-formato auto|ss525|ds525|ss35|ds35`.
- Opcao `--slot P:S=tipo[:arg]` para montar o layout pela linha de comando.
- Testes: `romdbtest` (CTest `romdb_store`); regressao de RAM em C000h (`memmaptest`, secao 25).
- **Estudo**: codigo do openMSX (GPL-2.0+), CLK e makeTSX (MIT) em `resource/`, provisorio, so' para
  consulta -- nenhum codigo copiado. Viabilidade de suporte a fitas TSX/TZX e CAS registrada em
  `doc/SPEC.md`, secao 5.2 (feature a desenvolver em breve, nao iniciada).

### Alterado
- **RAM 32KB** ocupa a celula escolhida (pagina 2) e a seguinte (pagina 3). **RAM 64KB** ocupa a celula inteira.
- **Mapper** de 64 KB a 4096 KB (potencias de 2); os segmentos 0 a 3 vao para as paginas 0 a 3 no reset. Pode haver mais de um mapper no layout.

### Corrigido
- **RAM de 16KB no slot**: ficava em 0000h da celula, e a BIOS nao subia (ex.: Gradiente Expert 1.1 com RAM em 3:x). Agora a RAM ocupa as ultimas paginas (C000h-FFFFh).
- **Tecla `&`**: era digitada como `^` (SHIFT+6); no MSX e' SHIFT+7.
- Nomes de arquivo com entidades HTML (`ROM&#39;s`) no indice do file-hunter.

## [1.17.0] - 2026-10-06 - "Xak: Musica FM e Slots"

### Adicionado
- **Chip FM (OPLL, YM2413)**: 9 canais melodicos com 2 operadores, envelope ADSR, feedback, tremolo,
  vibrato e os 15 timbres prontos. Portas `7Ch`/`7Dh` (MSX-MUSIC), sempre presentes como no fMSX.
  Entra no audio ao vivo junto com o PSG e o SCC. Ver [doc/fm-spec.md](fm-spec.md).
- **FM-PAC** (`--fmpac [arquivo]` ou menu Cartucho > FM-PAC): ROM de 16KB no slot 2:0, com a SRAM
  de 8KB liberada pela chave 694Dh e o arquivo `FMPAC.sav`.
- **Modo ritmo do OPLL** (`CALL MUSIC (1,...)`): BD, HH, SD, TOM e TC nos canais 7-9, com os tres
  timbres de bateria.
- **Comandos de BASIC do MSX-MUSIC** (`CALL MUSIC`, `PLAY #n`, `CALL VOICE`, `CALL PITCH`, `CALL AUDREG`,
  `CALL PLAY`) funcionam com `--fmpac`: a ROM do FM-PAC faz o trabalho, e a musica continua em segundo plano.
- `--text` (tela em texto no fim da execucao), `--fmstat` (estado do FM) e `--wav arquivo` (grava a mistura).
- **Layout de slots e configuracao da maquina** (menu Maquina > Configuracao de slots...): 16 celulas
  com BIOS, BASIC, RAM (16/32/64 KB), mapper (64 a 1024 KB), cartucho, sub-ROM do MSX2, disco e FM-PAC.
  Uma BIOS de 32KB ocupa as paginas 0 e 1 do slot escolhido; BIOS e BASIC podem ser dois arquivos de 16KB.
  Padrao igual ao de antes. Ver [doc/slots-spec.md](slots-spec.md).
- **FM-PAC ligado por padrao** quando o `FMPAC.ROM` existe; `--no-fmpac` e o menu Cartucho desligam.
- Testes: `fmtest` (CTest `fm_sound`, 24 checagens) e secao 24 da `memmaptest`; cinco checagens novas na `machinetest`.
- **SRAM de cartucho** nos mappers ASCII8 (8KB) e ASCII16 (2KB espelhada): o bit de SRAM do
  registrador de banco seleciona a memoria de bateria, que grava como no fMSX. Os registradores de
  banco continuam valendo com a SRAM selecionada.
- **Arquivo .sav** ao lado da ROM: lido ao carregar o cartucho e gravado quando ha alteracao (a cada
  300 quadros na janela, ao trocar cartucho ou modelo, ao fechar a janela e ao fim de `--frames`).
- Testes na `memmaptest` (secoes 19 e 20) para selecao, escrita, espelhamento, persistencia e tamanho errado.

- **SRAM de cartucho** tambem para o FM-PAC (`FMPAC.sav`), ver [doc/sram-spec.md](sram-spec.md).

### Alterado
- **FM-PAC ligado por padrao** quando o `FMPAC.ROM` existe (antes era preciso `--fmpac`). `--no-fmpac`
  e o menu Cartucho desligam.
- **Portas 7Ch/7Dh do FM sempre presentes**, como no fMSX.
- **A maquina monta um layout de 16 celulas** (slot:subslot) em vez de posicoes fixas. O padrao e' o
  mesmo de antes; trocar de modelo volta o layout ao padrao (cartucho e FM-PAC sao mantidos).
- **Mistura de audio**: PSG, SCC e FM somados, com o FM em metade do ganho.
- **Versao 1.17.0**: pacote Windows (`fwMSX-1.17.0.zip`) e Linux (`fwMSX-1.17.0-linux.tar.gz`), com o
  manual e as notas desta versao.

### Corrigido
- **`--keys` e teclado do MSX**: `(`, `)`, `*` e `"` eram digitados com outras teclas. Agora seguem o
  layout do MSX: SHIFT+9 = `(`, SHIFT+0 = `)`, SHIFT+8 = `*`, SHIFT+' = `"`.
- **Erro do FM-PAC no menu**: a mensagem aparece no proprio menu Cartucho.

### Limites conhecidos
- **Som do FM nao validado por ouvido**: o WAV gerado (`--wav`) tem sinal, mas nao foi comparado com um
  MSX-MUSIC real. Timbres e niveis sao estimativas.
- **`CALL VOICECOPY` nao e' aceito** pela ROM do FM-PAC do fMSX ("Syntax error").
- **Status e timers do OPLL** nao sao emulados (`7Ch` le 0).
- **Jogos**: Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada, causa nao
  diagnosticada; Mega Chase so' validado ate o titulo; F-1 Spirit 3D: troca de disco pela janela nao testada.
- **Janela**: menus, tela cheia, 4:3, 16:9 e filtros de video nao foram conferidos na tela (a automacao
  nao opera a janela).
- **Cores YJK** do V9958 nao conferidas com hardware real.
- **Layout de slots** so' pelo menu (sem opcao de linha de comando); BIOS so' em 0:0; GameMaster2 e
  save-state ausentes; efeitos de rastreio no meio do quadro ausentes.

## [1.16.0] - 2026-10-05 - "Aleste: Janela e Video"

Janela com menus do fMSX, zoom, proporcao, tela cheia e filtros de video.

### Adicionado
- **Quadro com borda, como o fMSX**: 272x228 no MSX1 e 544x228 no MSX2; borda de 18 linhas (8 em 212 linhas) e cor de fundo. As teclas de funcao e o texto do BASIC deixam de encostar na borda.
- **Menus do fMSX na janela**: Arquivo, Maquina (Modelo MSX1/MSX2/MSX2+, Reiniciar, Pausar), Exibir, Video, Som, Disco, Cartucho, Joystick, Ferramentas, Configuracoes e Ajuda. Os itens sem implementacao aparecem desabilitados com "(em breve)".
- **Trocar modelo ou cartucho** recria a maquina (os discos montados entram de novo).
- **Configuracoes -> Interface**: tema escuro ou claro, tamanho da letra (Normal, Grande, Muito grande; padrao Grande) e borda e sombra da tela.
- **Exibir -> Janela**: zoom 2x, 3x, 4x e 6x (padrao 3x), com a janela redimensionada. Janela inicial limitada a area util do monitor.
- **Exibir -> Proporcao**: Original (pixels), 4:3 corrigido e 16:9 esticado.
- **Tela cheia com menu oculto**: a barra aparece com o mouse no topo ou com um submenu aberto.
- **Video -> Interpolate Video**: Nearest Neighbor, Linear Scaling, EPX Scale 2x, Eagle Algorithm, Scale 2x Algorithm e 2xSal Algorithm (2xSal e uma aproximacao).
- **Video -> Scanlines**: TV, LCD e LCD Raster.
- **Video -> Color Filter**: Monochrome, Sepia, Green CRT, Amber CRT, CMY Raster e RGB Raster.
- Dialogo de abertura de arquivo generico (Windows), usado para cartuchos.
- Testes: `video_filters` (filtros de video) e a geometria do quadro em `machine_test` e `msx2_test`.

### Corrigido
- Janela inicial centralizada no topo da area util, para a barra de titulo nao ficar fora da tela.

### Pendente (proximas versoes)
- Itens "(em breve)" do menu: salvar estado, memoria, PAL, trapacas, POKE, DiskROM, fita, MIDI, gravacao de som, novo disco, slot 2, dispositivos de entrada e mostrar sprites.
- Validacao visual dos menus, tela cheia, 4:3, 16:9 e dos filtros de video (a automacao nao consegue operar a janela).

## [1.15.0] - 2026-10-05 - "Metal Gear 2: Cores YJK"

**Agora e' um MSX2+**: `fwmsx --msx --msx2p` roda a BIOS MSX2+ real (MSX BASIC 3.0) com o VDP
V9958. Ver [doc/msx2p-spec.md](msx2p-spec.md).

### Adicionado
- **Modelo MSX2+** (`--msx2p`): BIOS `MSX2P.ROM`, sub-ROM `MSX2PEXT.ROM`, RAM de 128KB com mapper e
  RTC, como o `--msx2`, com o VDP V9958. Bit 2 de S#1 ligado, como o fMSX.
- **SCREEN 10, 11 e 12** (YJK/YAE) em `vdp_render.c`, a partir do `RefreshLine10()`/`RefreshLine12()`
  e do `YJKColor()` do fMSX, com as correcoes descritas em `doc/msx2p-spec.md`. O modo e' ligado por
  R#25 (bit 3 YJK, bit 4 YAE) em scr 7/8.
- **Scroll horizontal** (R#26/R#27, 9 bits) em SCREEN 5-8, YJK e YAE, com HScroll512 (R#25 bit 0).
- **Mascara da esquerda** (R#25 bit 1): 8 pixels (16 em modo de 512) na cor de fundo.
- Teste novo no `vdp2test` (secao 9): verificacoes de YJK, scroll, mascara, YAE e um diferencial
  SCREEN 5-8 contra o renderizador da V9938.

### Corrigido
- `--keys`: os simbolos com SHIFT do layout MSX estavam trocados. `(` e `)` saiam como `)` e `-`,
  e `&`, `"` e `*` tambem estavam errados. Agora seguem o layout MSX: `SHIFT+8` = `(`, `SHIFT+9` =
  `)`, `SHIFT+6` = `&`, `SHIFT+2` = `"`, `SHIFT+'` = `*`.

### Verificado
- BIOS MSX2+ sobe com `MSX BASIC version 3.0`.
- Programa BASIC `screen 12` / `line (10,10)-(200,100),5,bf` escreve R#25 = `08h` e desenha em YJK;
  `screen 10` escreve R#25 = `18h`. Na MSX2 comum o mesmo programa nao chega a trocar para o SCREEN 12.
- `vdp2test` com as verificacoes novas do V9958 (secao 9) e suite completa 12/12.

### Limitacoes conhecidas
- Cores YJK conferidas contra o openMSX (regra do azul de um turbo R), e nao contra um V9958 real.
- Atributo de SCREEN 10 e 11 nao e' distinguido (como no fMSX).
- SCREEN 9 nao existe no MSX2+ (so' no MSX2 coreano); nao foi implementado.

---

## [1.14.0] - 2026-10-05 - "Hydlide: Busca Rapida"

**CPIR e CPDR ganham caminho rapido em Assembly**, no mesmo molde do LDIR/LDDR. Ver
[doc/z80-core-spec.md](z80-core-spec.md), secao 6.

### Adicionado
- `z80_fast_block_search()` (`src/z80/asm/block_ops.asm`, dual-ABI Win64/SysV): a busca do byte
  em `REPNE SCASB`, para frente (CPIR) ou para tras (CPDR).
- Caminho rapido de CPIR/CPDR em `opcodes_ed.h`: so' a busca vai para o Assembly. Contagem, HL/BC,
  PC, ciclos e flags saem das mesmas expressoes do loop lento, a partir do ultimo byte examinado.
- Teste diferencial CPIR/CPDR em `tests/z80/smoke_test.cpp` (teste 6): 12 casos fixos e 120 aleatorios,
  com casamento e sem casamento, orcamento grande e pequeno, nos dois sentidos. Comparam registradores,
  flags e a RAM inteira contra o loop lento.

### Verificado
- Teste de sensibilidade: quebrar o tratamento de casamento no caminho rapido derruba 26 checks.
- `z80test`: 300 checks passam no Windows e no Linux (SysV).
- Suite completa: 12/12 suites nas duas plataformas.

## [1.13.0] - 2026-10-05 - "F1 Spirit: Som do SCC"

**O SCC toca**: cartuchos Konami5 e Gen8 que ligam o chip de som SCC agora tocam a trilha.
O F1 Spirit programa os cinco canais de verdade. Ver [doc/scc-spec.md](scc-spec.md).

### Adicionado
- **Chip de som SCC** (`src/scc/`): motor em **C** adaptado do `EMULib/SCC.c` do fMSX, gerando
  amostras PCM (como o PSG); soma de canal em **Assembly** dual-ABI (`render_channel.asm`); tabela
  de volume linear em **Fortran**; `SccDevice` em **C++** ligado ao slot de cartucho.
- Protocolo do cartucho: `3Fh` em `9000h` (Konami5) ou em `8000h-9FFFh` (Gen8) liga o chip, e a
  faixa `9800h-98FFh` vai para ele enquanto estiver ligado. A escrita nessa faixa nao chega ao mapper.
- `memmap::SlotCartIo` / `SlotMemoryBus::AttachCart()`: o barramento entrega leituras e escritas do
  slot de cartucho ao dispositivo antes do mapper (o FDC usa o `SlotMmio`, de outra natureza).
- Saida de audio ao vivo soma PSG e SCC, com saturacao em 16 bits (`Machine::TakeLiveAudio`).
- Teste novo `scctest` (CTest `scc_sound`): 18 verificacoes, inclusive o teste diferencial do
  kernel de Assembly contra a referencia em C (2000 casos aleatorios).

### Verificado
- F1 Spirit (Konami5, 128KB): apos 4000 quadros, o jogo liga os cinco canais (mixer `1Fh`) e o
  estado renderiza 44085 amostras nao-nulas por segundo, pico 21874.
- Lode Runner + Konami SCC continua sem subir o jogo (ver `doc/scc-spec.md`, secao 5): nenhum mapper
  chega a ligar o SCC nesse cartucho. O problema parece ser o boot do cartucho, nao o chip; nao
  investigado a fundo nesta versao.

## [1.12.0] - 2026-10-05 - "Metal Gear: Entrada Direta"

Versao pequena: fecha uma decisao de comportamento registrada desde a v1.2.0.

### Alterado
- **`fwMSX.exe` sem argumento nenhum agora abre a maquina completa em janela**, com os mesmos
  padroes de `--msx` (BIOS MSX1 ao lado do executavel, sem cartucho/disco). Antes, sem
  argumentos, imprimia o esqueleto dos quatro modulos (C++/C/Assembly/Fortran) -- decisao
  registrada em `doc/SPEC.md`, secao 5.1 ("sem argumentos deve abrir em GUI por padrao, uma
  vez que exista emulacao de verdade"), adiada ate agora por nao fazer sentido antes do core
  existir. O esqueleto continua alcancavel com argumentos explicitos que nao batem com nenhum
  modo conhecido (ex.: `fwMSX.exe NomeDoProduto 1 2 3`), preservado por ser o historico do
  projeto.

### Corrigido
- `build.ps1` para no primeiro erro do CMake (configuracao ou compilacao). Antes imprimia
  "Pronto" e empacotava o zip mesmo com a compilacao falhando.

## [1.11.1] - 2026-10-02 - "Firebird: Arrumando a Casa"

Versao pequena de acabamento sobre a 1.11.0.

### Adicionado
- **`--disk-ro`**: discos somente leitura. O MSX-DOS le normalmente e responde `Write protect
  error writing drive A` ao tentar gravar; o arquivo `.dsk` nunca e' alterado. Vale tambem
  para discos inseridos depois, pelo menu **Disco**. Era o maior risco de perda de dados da
  v1.10.0 (as gravacoes vao direto para o arquivo). Teste novo em `machinetest`.

### Corrigido (documentacao e comentarios)
- Textos que ficaram para tras: o `MANUAL.md` ainda dizia "nao ha' janela grafica", o `README`
  "sem janela ainda", o `psg-spec` listava audio ao vivo e joystick como pendentes, o
  `ppi-spec` listava PSG/janela/cartuchos como proximos passos, e dois `TODO(FDC)` em
  `slot_state.c/.h` apontavam para um controlador que ja' existe (mora em `SlotMmio`).

- 800 verificacoes automatizadas (11 suites, +5 da `--disk-ro`).

### Verificado
- Jogos MSX1 rodando na maquina MSX2: o King's Valley mostra o mesmo logo "Konami Software"
  sobre azul que o fMSXgo de referencia.

## [1.11.0] - 2026-10-02 - "Firebird: MSX2 em Cena"

**O fwMSX agora e' tambem um MSX2**: `fwmsx --msx --msx2` roda o MSX BASIC 2.1 e jogos
MSX2 de verdade, como o Firebird. Ver [doc/msx2-spec.md](msx2-spec.md).

### Adicionado
- **MSX2** (`fwmsx --msx --msx2`) -- ver [doc/msx2-spec.md](msx2-spec.md). A BIOS MSX2 real
  roda ate' o **MSX BASIC 2.1** (identico ao fMSXgo de referencia), e o **Firebird**
  (jogo MSX2) joga: logos, titulo, floresta rolando e sprites coloridos.
  - **VDP V9938:** VRAM de 128KB com paginas (R#14); **SCREEN 3, 4, 5, 6, 7, 8 e TEXT80**;
    212 linhas; cor 0 transparente; tela desligada (R#1 bit 6); **sprites de modo 2**
    (cor por linha, bits CC/EC, 8 por linha, 9o sprite, colisao).
  - **Motor de comandos** (`vdp_cmd.c`, adaptado de `V9938.c`): POINT, PSET, SRCH, LINE,
    LMMV, LMMM, LMCM, LMMC, HMMV, HMMM, YMMM, HMMC, com operacoes logicas, temporizacao
    por scanline e handshake TR. `LINE`/`CIRCLE`/`PAINT` do BASIC desenham de verdade.
  - **Maquina:** RAM de 128KB com **mapper** (portas `FCh`-`FFh`), **RTC** RP5C01 com CMOS
    (portas `B4h`/`B5h`), sub-ROM `MSX2EXT.ROM` em 3:1 dividindo o slot com a DISK.ROM
    (o MSX-DOS 1.8 boota no MSX2 e nem pergunta a data). A imagem do MSX2 e' sempre de 512
    pixels de largura (modos de 256 saem dobrados) e a janela recria a textura ao mudar de modo.
  - `--keys` aceita pontuacao com SHIFT (`( ) : $ " ...`); `--vdplog` mostra as mudancas
    dos registradores do VDP por quadro.
- Novos alvos de teste `vdp2test` (VDP isolado, cada modo e cada comando) e `msx2test`
  (mapper, RTC e a maquina com a BIOS MSX2 real, inclusive o BASIC desenhando).
- 795 verificacoes automatizadas (11 suites).

### Corrigido
- **SCREEN 3 (multicolor)** do MSX1 agora e' desenhado (caia no fundo liso).
- A **cor 0 transparente** (mostra a cor de fundo) e a **tela desligada** (R#1 bit 6) agora
  valem em todos os modos; antes o renderizador as ignorava.
- `R#10`/`R#11`/`R#6` guardam so' os bits que existem (como o `VDPOut()` do fMSX).
- Desvio deliberado do fMSX: o bit TR do motor de comandos comeca limpo a cada comando (no
  fMSX o LMCM seguinte a um HMMC devolvia um pixel velho).

## [1.10.0] - 2026-10-02 - "Golvellius: MSX-DOS e Joystick"

**O MSX-DOS 1.8 boota, e os jogos de cartucho ganham joystick e deteccao de
mapper.** Verificado com jogos e discos reais -- ver
[doc/fdc-spec.md](fdc-spec.md) e [doc/machine-spec.md](machine-spec.md), secao 4b.

### Adicionado
- **Disco: controladora WD2793 + DiskROM -- o MSX-DOS 1.8 boota.** `fwmsx --msx
  --disk msxdos1.dsk` roda o **MSX-DOS** de verdade: a `DISK.ROM` no slot 3:1
  fala com um WD2793 mapeado em `7FF8h-7FFFh` (motor em C adaptado do
  `WD1793.c` do fMSX, `.dsk` cru, gravacao direta no arquivo). `dir` e `copy`
  funcionam, e o arquivo copiado sai identico ao original. Novo mecanismo
  `SlotMmio` no barramento de slots; menu **Disco** na janela (inserir/ejetar
  A:/B:); opcoes `--disk`, `--diskb`, `--disk-interface`, `--diskrom`, `--wait`.
  Ver [doc/fdc-spec.md](fdc-spec.md). Novo alvo `fdctest`.
- **Joystick** (R14/R15 do PSG): `psg_set_joystick()`, leitura de R14 com a
  selecao de porta (bit 6 de R15) e o corte das linhas (bits 4/5), como o
  `InZ80()` 0xA2 do fMSX. Na janela: **setas + Z/Espaco (fogo A) + X (fogo B)**
  na porta A e **gamepads do GLFW** (1o -> porta A, 2o -> porta B), com menu
  **Joystick**. Um toque mais curto que um quadro ainda vale 1 quadro. Reset
  de maquina nao solta o joystick.
- **Deteccao automatica de mapper** (`memmap::GuessMapper`, o `MAP_GUESS` do
  fMSX, so' a heuristica de `LD (nnnn),A`): `--cart <arq>` com mais de 32KB
  detecta Konami4/Konami5/ASCII8/ASCII16/Gen8 sozinho (`--cart <arq> auto` ou
  um nome de mapper para forcar). O CLI imprime o que detectou.
- Cartucho de ROM plana de ate' 16KB cujo cabecalho `AB` aponta para
  8000h+ vai para a pagina 2.

### Corrigido
- **Estado inicial de MegaROM**: os bancos 0,1,2,3 aparecem em 4000h/6000h/
  8000h/A000h (como `SetMegaROM(J,0,1,2,3)` do fMSX), nao todos no banco 0.
  Varios jogos chamam rotinas em 6000h-7FFFh antes de trocar qualquer banco.

- 659 verificacoes automatizadas (9 suites).

### Verificado com jogos reais (fora do repositorio)
- **King's Valley** (16KB): roda, tela de titulo e entrada pelo teclado.
- **F1 Spirit** (128KB, Konami5 detectado): roda ate' o menu do jogo.
- **Firebird / Hi no Tori** (128KB, Konami4 detectado): **e' um jogo MSX2**
  (titulo em SCREEN 5) -- na maquina MSX1 ele nao chega a lugar nenhum, e e'
  o que se esperava: confirmado rodando o mesmo jogo num emulador de
  referencia (fMSXgo, em Go) em modo MSX1 (tambem trava) e em modo MSX2 (roda).
  Nao era bug do nucleo; ele passa a funcionar com o VDP MSX2 (Fase 4).
- **Lode Runner + Konami SCC**: cai no BASIC ("Illegal function call in 10") --
  e' uma ROM que espera disco (FDC), que ainda nao existe.

## [1.9.0] - 2026-10-02 - "Gradius 2: Janela e Som"

**O fwMSX agora e' uma maquina que da' para usar**: `fwmsx --msx` abre uma
janela com a BIOS real rodando em tempo real, teclado do host e som. Ver
[doc/machine-spec.md](machine-spec.md), [doc/psg-spec.md](psg-spec.md) e
[doc/audio-spec.md](audio-spec.md).

### Adicionado
- **`fwmsx --msx`: a maquina MSX1 completa numa janela, com teclado do
  host** -- ver [doc/machine-spec.md](machine-spec.md). `machine::Machine`
  (`src/machine/`) junta BIOS + slots + VDP + PPI + PSG + Z80 e avanca por
  quadros (59736 ciclos, 59.92 Hz) sem depender de janela; a janela
  (Dear ImGui + GLFW + OpenGL) roda em tempo real, com menu (Reset, Pausar,
  tela cheia com F11) e mapeamento posicional do teclado do host para a
  matriz do MSX. `--cart <arq> [mapper]` carrega um cartucho no slot 1
  (ROM plana ou MegaROM); `--frames N --shot arq.ppm [--keys "texto"]` roda
  sem janela e salva a tela.
- **Audio ao vivo**: `fwmsx --msx` toca o PSG pelo dispositivo de audio
  padrao (miniaudio, um header de dominio publico baixado por FetchContent),
  via buffer circular sem trava e `PsgDevice::EnableLive()/TakeLive()`.
  Menu **Som** (mudo + volume), `--mute`, `-DFWMSX_AUDIO=OFF` para um stub.
  Ver [doc/audio-spec.md](audio-spec.md). Novo alvo `audiotest` (inclui o
  dispositivo real: ~44100 amostras/s, 0 underruns em 1 s).
- 570 verificacoes automatizadas (8 suites).
- Novo alvo de teste `machinetest` (boot em quadros, RenderFrame, digitar
  `print 1234`, Reset, cartucho com INIT).
- **PSG AY-3-8910 (portas `A0h`-`A2h`)** -- ver [doc/psg-spec.md](psg-spec.md).
  Motor em C (`src/psg/core/psg_state.{h,c}`) que **gera amostras PCM de
  verdade** (3 tons, ruido LFSR de 17 bits, 16 formas de envelope) avancadas
  por ciclos de Z80; tabela de volume logaritmica em Fortran
  (`volume_table.f90`); `psg::PsgDevice` e gravador de WAV em C++.
- **Depurador:** flag `--psg` (exige `--slots`) e comandos `psgregs`,
  `psgpoke`, `psgrec start|stop|clear|save <arq.wav>`; `reset` tambem
  reseta o PSG.
- Novo alvo de teste `psgtest`. Com a BIOS real: `GICINI` programa o mixer
  (R7=`B8h`) e `BEEP` no BASIC toca o canal A em 1316 Hz, volume 7.

### Notas
- Sem joystick em R14/R15 (R14 le `7Fh`). A duracao do BEEP gravado nao foi conferida
  contra um MSX real.

## [1.8.0] - 2026-10-01 - "Zanac: Prompt do BASIC"

**A BIOS MSX1 real sobe ate o prompt do MSX BASIC** e o teclado do PPI
chega ate' ela -- primeira vez que o fwMSX roda software MSX de verdade.
Ver [doc/ppi-spec.md](ppi-spec.md), seção 5.

### Corrigido
- **`Z80Cpu` agora chama `z80_reset()` na construção.** Antes, quem não
  digitasse `reset` rodava com as tabelas de flag Sinal/Zero/Paridade
  **zeradas** (elas são preenchidas por `z80_tables_init()`, dentro de
  `z80_reset()`): `AND`/`OR`/`XOR`/`CP`/`INC`/`DEC` davam flags erradas
  (`AND A` com A=0 devolvia `F=10h` em vez de `54h`). Era a causa real de
  a BIOS ficar "presa" desde a v1.4.0 -- o diagnóstico anterior ("falta
  PPI", depois "falta RAM/subslot") estava errado/incompleto. Os 168
  testes do `z80test` não pegaram porque todos chamam `cpu.reset()` antes.
  Teste de regressão novo em `debug_session_test.cpp`.

### Adicionado
- Teste de aceite com a BIOS real (`ppitest`, antes informativo): 100M de
  ciclos, VBlank habilitado, a abertura "MSX BASIC version 1.0 ... Bytes
  free" na VRAM e `keydown z` aparecendo na tela.
- 470 verificações automatizadas (5 suítes).

### Notas
- Com BIOS + `--ppi`: RAM de 64KB em `3:2` (necessária). As regras de
  subslot do MSX1 foram testadas -- a BIOS sobe com e sem elas; mantidas
  por fidelidade ao fMSX.

## [1.7.0] - 2026-10-01 - "Vampire Killer: Teclado e PPI"

PPI i8255 + matriz de teclado (portas `A8h`-`ABh`) -- ver
[doc/ppi-spec.md](ppi-spec.md).

### Adicionado
- **Chip i8255 em C** (`src/ppi/core/ppi_state.{h,c}`), adaptado de
  `EMULib/I8255.c` e das portas `A8h`-`ABh` de `MSX.c`: modos das portas,
  set/reset de bit da porta C, pinos de saída só dirigidos em modo saída,
  e a matriz de teclado de 11 linhas lida por `A9h`.
- **`ppi::PpiDevice` (C++)**: o slot primário passa a mudar quando o pino
  de saída da porta A muda (como `PSlot(PPI.Rout[0])` no fMSX). Antes de a
  BIOS escrever `82h` em `ABh`, `OUT (A8h)` não troca slot -- fiel ao
  hardware.
- **Tabela de posição das 87 teclas em Fortran**
  (`src/ppi/fortran/key_matrix.f90`) e **contagem de teclas pressionadas
  em Assembly** dual-ABI Win64/SysV (`src/ppi/asm/key_count.asm`).
- **Depurador:** flag `--ppi` (exige `--slots`) e comandos `ppiregs`,
  `keys`, `keydown <tecla>...`, `keyup <tecla>...|all`; `reset` também
  reseta o PPI (teclas pressionadas continuam).
- **Layout MSX1 com BIOS + `--ppi`**: RAM de 64KB no slot `3:2` e regras
  de subslot do MSX1 (`SlotState.msx1_subslot_rules`, `SSlot()` do fMSX:
  slots 0/1/2 sem subslot). Sem `--ppi` nada muda.
- Novo alvo de teste `ppitest` (62 verificações; 464 no total, 5 suítes).

### Achado
- A BIOS real agora programa e lê o PPI e testa a expansão de slots, mas
  **ainda não sai da varredura de RAM** (`0x0305`-`0x0331`). O
  diagnóstico anterior ("só falta o PPI") estava incompleto -- também
  faltavam RAM e as regras de subslot. Próximo passo registrado em
  `doc/ppi-spec.md`, seção 7.

## [1.6.0] - 2026-10-01 - "Penguin Adventure: Sprites em Cena"

Fase 3 do VDP -- sprites de "modo 1" do TMS9918 (SCREEN 1/2/3). Ver
[doc/vdp-spec.md](vdp-spec.md), secao 6, Fase 3.

### Adicionado
- **Sprites em C** (`src/vdp/core/vdp_sprites.{h,c}`), adaptados de
  `Sprites()`/`CheckSprites()` do fMSX: 8x8 e 16x16, ampliação 2x,
  prioridade por índice, cor 0 transparente, early clock, Y negativo,
  terminador Y=208, desligamento por R#8 bit 1.
- **Limite de 4 sprites por linha** com flag/número do "quinto sprite"
  em S#0, e **flag de colisão** (S#0 bit 5) sinalizada na linha 192 por
  `vdp_step_scanline()`; ler S#0 pela porta `99h` devolve e limpa.
- `vdp_render_line()` desenha os sprites por cima do fundo em SCREEN 1/2
  (o `vdpshot` já os mostra).
- 20 novas verificações em `vdptest` (402 no total, 4 suítes).

### Corrigido
- Build Linux: sem `wayland-scanner` o configure do GLFW 3.4 falhava;
  agora o `CMakeLists.txt` desliga o backend Wayland nesse caso e usa só
  X11 (via XWayland em sessões Wayland). Pré-requisitos documentados no
  cabeçalho do `build.sh`.
- Pacote Linux `dist/fwMSX-1.6.0-linux.tar.gz` incluído.

### Notas
- Sem sprites de modo 2 (SCREEN 4-8) -- Fase 4, com os modos MSX2.
- Quirk do fMSX preservado: VScroll (R#23) somado duas vezes ao Y dos
  sprites em SCREEN 1 (irrelevante com R#23=0).

## [1.5.0] - 2026-09-30 - "Antarctic Adventure: Primeiros Pixels"

Primeira vez que o projeto desenha pixels de verdade -- o VDP
(TMS9918/V9938), Fases 0.5, 1 e 2 -- ver
[doc/vdp-spec.md](vdp-spec.md) para o histórico completo de fases.

### Adicionado
- **`CompositeBus`** (`src/z80/cpp/composite_bus.h`, design próprio):
  multiplexa I/O de porta entre vários dispositivos (mapa de memória em
  `A8h`, VDP em `98h`-`9Bh`), enquanto leitura/escrita de memória vai
  sempre para um único dispositivo designado.
- **Motor "digital" do VDP em C** (`src/vdp/core/`), adaptado de
  `resource/fMSX/fMSX/MSX.c`: registradores (64), status (16), VRAM,
  protocolo das 4 portas (`98h` dados, `99h` latch de endereço/
  registrador, `9Ah` latch de paleta, `9Bh` acesso indireto), cache de
  ponteiro de tabela (`ChrTab`/`ColTab`/`ChrGen`/etc., mesmo padrão já
  validado no Z80 e no mapa de memória), e a máquina de estados de
  scanline que gera as interrupções de VBlank (`IE0`)/HBlank-
  coincidência (`IE1`). Preservado um quirk real do fMSX: o latch de
  endereço (`VKey`) não é resetado numa leitura de status, só numa
  leitura/escrita de dados -- o comentário original diz que isso
  "quebra Sir Lancelot no ColecoVision" se mudado.
- **Finalmente dá uso real ao `Z80Cpu::interrupt()`**, que existe desde
  a Fase 1 do núcleo Z80 mas nunca tinha sido exercitado contra nada --
  confirmado com um programa sintético de 12 bytes recebendo a
  interrupção de VBlank em `0x0038` de verdade (`IFF1` desligando,
  não só coincidência de PC/SP).
- **Renderização real de pixel** (`src/vdp/core/vdp_render.*`),
  adaptada de `resource/fMSX/fMSX/Common.h` (`RefreshLine0/1/2`): SCREEN
  0 (texto 40×24 mono, 240×192), SCREEN 1 (texto 32×24 com cor
  compartilhada por grupo de 8 caracteres -- quirk real do "Graphics 1"
  da TMS9918), SCREEN 2 (bitmap 256×192 com tabela de cor/padrão
  dividida em terços). Sem borda, sprites ou tela ligada/desligada
  ainda.
- **Tabela de paleta de 512 cores em Fortran**, construída na Fase 1
  sem consumidor, finalmente usada pela escrita de paleta via porta
  `9Ah` na Fase 2.
- **`vdpshot <arquivo.ppm> [linha_inicial] [linha_final]`**: exporta o
  frame atual como imagem PPM binária (`P6`), sem depender de biblioteca
  de imagem externa -- testável byte a byte, visualizável em qualquer
  leitor de PPM.
- Comandos de depuração `vdpregs`/`vdpmem`/`vdppeek`/`vdppoke`/
  `vdpstep` em `--z80dbg --slots --vdp` (requer `--slots`; avisa e
  ignora `--vdp` sem ele).
- 54 verificações automatizadas novas (382 no total, `ctest`, 4 suítes):
  protocolo de porta, cache de tabela por modo de tela, paleta padrão
  (as 16 entradas), os três modos de renderização com posições de pixel
  específicas conferidas à mão, PPM round-trip byte a byte, e o teste
  de aceitação com o programa sintético de interrupção.

### Corrigido
- `vdp_reset()` zerava a paleta em vez de carregar a paleta padrão do
  TMS9918 (`PalInit[16]` do fMSX) -- sem essa correção, toda cor FG/BG
  cairia em preto, já que software MSX1 normal nunca escreve os 16
  registradores de paleta na mão (espera a paleta padrão desde o ligar).

### Achado, não corrigido nesta versão
- A BIOS MSX1 real ainda não chega a habilitar a interrupção de VBlank
  dentro de nenhum orçamento de ciclos testado -- ela poliniza hardware
  de teclado/PPI (portas `A9h`-`ABh`) que ainda não existe no projeto.
  Não é um bug do VDP; é a próxima peça que desbloquearia testes bem
  mais realistas contra a BIOS (ver `doc/SPEC.md`, seção 5.0).

## [1.4.1] - 2026-09-30 - "Salamander: Compilando em Linux"

Primeira validação real (build + link + execução, não só montagem) do
projeto numa máquina Linux (WSL2) -- ver [doc/SPEC.md](SPEC.md), seção
5.0. Sem features novas; dois bugs reais de portabilidade corrigidos em
código Assembly que nunca tinha sido testado fora do Windows.

### Corrigido
- **`src/asm/init_asm.asm`** (módulo Assembly da Fase 0 do projeto,
  desde 2026-09-28) era Win64-only -- montava sem erro para `elf64`
  (NASM não valida convenção de chamada), mas rodando de verdade no
  Linux os argumentos chegariam nos registradores errados (SysV entrega
  major/minor/patch em EDI/ESI/EDX, não ECX/EDX/R8D da Win64), e o
  *link* falhava (`relocation ... can not be used when making a PIE
  object`) por causa de uma chamada direta a `printf` incompatível com
  executável PIE (padrão em distros Linux modernas). Corrigido com a
  mesma técnica `%ifidn __OUTPUT_FORMAT__` do núcleo Z80
  (`src/z80/asm/block_ops.asm`) e `call printf wrt ..plt` (chamada
  PLT-relativa, funciona com ou sem PIE).
- **`src/msxdisk/asm/name_match.asm`** (msxdisk, Win64-only desde a
  Fase 2 dele) tinha o mesmo problema de registradores errados no
  Linux -- mais sutil que o de cima, porque não chama nenhuma função
  externa: o build **não falhava**, só o resultado ficaria errado em
  tempo de execução (`list`/`extract` com padrão de coringa). Corrigido
  com a mesma técnica de troca de registrador de entrada por ABI.
- `src/msxdisk/gui/file_dialog.cpp` incluía `<windows.h>` sem nenhuma
  guarda de plataforma, quebrando a compilação inteira fora do Windows.
  Guardado atrás de `#ifdef _WIN32`; noutras plataformas os diálogos
  nativos de arquivo devolvem "cancelado" por enquanto (um seletor
  nativo para Linux fica para uma tarefa à parte).
- `comdlg32` (biblioteca de diálogo nativo do Windows) era linkada sem
  condição no `CMakeLists.txt`, quebrando o link fora do Windows.
  Agora só entra quando `WIN32` é verdadeiro.

### Adicionado
- `build.sh` agora usa `build-linux/` como diretório de build (não
  `build/`, que é onde `build.ps1` grava o cache do CMake) -- evita o
  erro "`CMakeCache.txt` directory is different" ao alternar entre
  compilar pelo Windows nativo e pelo WSL no mesmo checkout.
- Pacote de distribuição Linux (`dist/fwMSX-X.Y.Z-linux.tar.gz`) agora
  versionado no repositório, igual ao `.zip` do Windows.

### Validado
- `./build.sh` rodado numa máquina Linux real (WSL2): build completo +
  `ctest -R "z80|memmap"` com as **328 verificações passando** --
  primeira confirmação de que o núcleo Z80 (incluindo a aceleração
  `LDIR`/`LDDR` em Assembly, branch `elf64`) e o mapa de memória
  funcionam de verdade fora do Windows, não só compilam.

## [1.4.0] - 2026-09-30 - "Illusion City: Mapa de Memória"

Mapa de memória MSX (slots/subslots/MegaROM) sobre o núcleo Z80 da
v1.3.0 -- ver [doc/memory-map-spec.md](memory-map-spec.md) para o
histórico completo de fases. **Trabalho no core de emulação pausado
deliberadamente a partir daqui** -- ver [SPEC.md, seção 5.0](SPEC.md)
para os próximos passos registrados.

### Adicionado
- **Motor de slots/subslots em C** (`src/memmap/core/`), adaptado de
  `resource/fMSX/fMSX/MSX.c`/`MSX.h` (`MemMap[4][4][8]`, `PSL`/`SSL`/
  `SSLReg`, incluindo o quirk real de hardware onde o registrador de
  slot secundário é indexado pelo slot primário que ocupa a página
  `C000h-FFFFh`, não um registrador global único).
- **`MemorySystem`/`SlotMemoryBus` em C++** (design próprio), implementa
  o `IBus` do núcleo Z80 e expõe uma API de inspeção
  (`PeekSlot`/`PokeSlot`/`Describe`/`CurrentView`) que enxerga qualquer
  uma das 16 combinações de slot **independente** do que está
  atualmente visível para a CPU -- tratado como requisito vital desde a
  primeira fase deste módulo, por pedido explícito do autor.
- **Carregamento de ROM real** (`LoadRom`) com **CRC32 em Fortran**
  (tabela de 256 entradas, verificado contra o vetor de teste padrão
  `CRC32("123456789") == 0xCBF43926`) como conveniência do depurador.
- **Seis mappers MegaROM de troca de banco** (`Gen8`/`Gen16`/`Konami5`/
  `Konami4`/`ASCII8`/`ASCII16`, só a parte de ROM -- sem SCC/SRAM),
  adaptados de `MapROM()` do fMSX com as condições exatas de
  endereço/máscara de cada protocolo, incluindo o quirk de
  compatibilidade do ASCII16 para escrita "lixo" em endereço alinhado.
- **`fwmsx --z80dbg --slots [rom]`**: comandos `slots`/`pages`/
  `slotmem`/`slotpeek`/`slotpoke`/`loadrom` (com mapper opcional), e
  carregamento automático de uma ROM de boot no slot 0:0 na abertura.
  `--z80dbg` sem `--slots` continua exatamente como antes (RAM plana),
  decisão deliberada para não exigir localizar uma BIOS no caso de uso
  mais simples.
- **Validado contra a BIOS MSX1 real do fMSX**
  (`resource/fMSX/ROMs/MSX.ROM`, já presente no repositório para
  estudo): 192 endereços de PC distintos visitados em 100 mil ciclos de
  execução real de código de BIOS.
- **`build.sh`**: equivalente Linux do `build.ps1`, para compilar/testar
  também fora do Windows -- em particular para validar em execução real
  (não só montagem) a branch `elf64`/SysV AMD64 do `.asm` dual-ABI do
  núcleo Z80.
- 328 verificações automatizadas novas desde a v1.3.0 (`ctest -R
  "z80|memmap"`): 168 no motor Z80 (inalterado), 58 no depurador
  (+23 desde a v1.3.0), 102 no mapa de memória.

### Adiado, com motivo registrado (ver `doc/memory-map-spec.md`, seção 6)
- Chip de som SCC e SRAM persistente (`ASCII8`/`ASCII16`) -- dependem de
  subsistemas (áudio, save-state) que ainda não existem.
- `MAP_GMASTER2`/`MAP_FMPAC` inteiros -- só interessantes por causa de
  SRAM/som, sem isso viram um `Konami4` trivial.
- Heurística `MAP_GUESS` de auto-detecção de mapper -- precisaria de
  bancos de assinatura SHA1/CRC externos (`CARTS.SHA`/`CARTS.CRC`);
  `loadrom` exige o tipo de mapper explícito por enquanto.

## [1.3.0] - 2026-09-30 - "SD Snatcher: Núcleo do Z80"

Início do core de emulação propriamente dito: a CPU Z80, fiel ao fMSX,
usando as quatro linguagens do projeto de verdade (não só simbolicamente)
-- ver [doc/z80-core-spec.md](z80-core-spec.md) para o histórico completo
de fases.

### Adicionado
- **Motor de despacho da CPU Z80 em C** (`src/z80/core/`), adaptado de
  `resource/fMSX/Z80/` (registradores, tabelas de ciclo/flag, opcodes com
  e sem prefixo `CB`/`ED`/`DD`/`FD`/`DDCB`/`FDCB`), por trás de um
  `Z80Bus` próprio (callbacks + contexto, no lugar das funções globais do
  fMSX -- permite múltiplas instâncias de CPU no mesmo processo).
- **Wrapper de orquestração em C++** (`Z80Cpu`/`IBus`, `src/z80/cpp/`):
  `reset()`/`run(ciclos)`/`interrupt()` e acesso a registradores, sem
  expor a união de par de registrador na API pública.
- **Tabelas de flag geradas em Fortran** (`src/z80/fortran/flag_tables.f90`):
  `ZSTable`/`PZSTable` calculadas com `POPCNT`, chamadas uma única vez no
  reset da CPU -- nunca no caminho quente do despachante.
- **Aceleração de `LDIR`/`LDDR` em Assembly** (`src/z80/asm/block_ops.asm`):
  primeiro `.asm` do projeto com Win64 **e** SysV AMD64 (Linux) no mesmo
  arquivo-fonte (`%ifidn __OUTPUT_FORMAT__`), usada só quando o bloco
  inteiro cai em RAM plana do host -- o loop byte-a-byte original do
  fMSX continua como caminho de reserva sempre que isso não vale.
- **Depurador embutido em `fwMSX.exe`** (`fwmsx --z80dbg`): REPL
  (replxx) com `reset`/`regs`/`step`/`run`/`break`/`clear`/`breaks`/
  `mem`/`peek`/`poke`/`load`/`fill`/`disasm`, rodando sobre uma RAM
  plana de 64KB de teste -- ainda sem VDP/PSG/mapa de memória real.
- **Desmontador Z80** (`src/z80/debug/z80_disasm.*`), adaptado do
  desmontador já existente em `resource/fMSX/Z80/Debug.c`, com três
  correções cosméticas documentadas (nenhuma afeta execução/timing da
  CPU) em relação ao original.
- 203 verificações automatizadas novas (`ctest -R z80`): 168 no motor
  (`z80_smoke`) e 35 no depurador/desmontador (`z80_debug_session`),
  incluindo uma varredura de completude sobre 2044 combinações de opcode
  do desmontador.

### Notas
- Ainda não existe VDP, PSG nem mapa de memória de uma máquina MSX real
  -- o núcleo roda isolado sobre RAM de teste. Ver `doc/z80-core-spec.md`
  para o que falta.

## [1.2.1] - 2026-09-29 - "Metal Gear: Ajustes de Campo"

Correções e polimento na GUI do `msxdisk` (v1.2.0), encontrados testando
de verdade em janela gráfica.

### Adicionado
- **Ejetar disco** (`Arquivo > Ejetar` ou `F12`): descarrega a imagem da
  memória sem apagar o arquivo; confirma antes só se houver alterações
  não salvas.
- **Novo/Abrir/Salvar Como** na GUI agora usam o diálogo nativo de
  arquivo do Windows (`GetOpenFileNameW`/`GetSaveFileNameW`), com
  navegação de pastas de verdade, em vez de uma caixa de texto simples.

### Corrigido
- **Diálogos de Novo/Abrir/Salvar/Renomear/Nova Pasta/Excluir não
  abriam** na GUI: `ImGui::OpenPopup`/`BeginPopupModal` calculam o ID do
  popup a partir da janela "atual" no momento da chamada — como esses
  diálogos eram disparados de dentro do menu (uma sub-janela) ou antes da
  janela principal existir no frame, o ID nunca batia com o que o
  `BeginPopupModal` esperava. Corrigido centralizando o `OpenPopup` real
  num único ponto, sempre na mesma janela.
- **Renomear/Nova Pasta/Excluir agiam no painel ou item errado**: marcar
  um arquivo só pela caixinha de seleção (sem clicar no nome da linha)
  não atualizava qual painel estava "ativo" — as teclas de função
  continuavam operando no painel local por padrão. Agora marcar a
  caixinha também ativa o painel e seleciona a linha.

## [1.2.0] - 2026-09-29 - "Maze of Galious: Gerenciador de Discos"

### Adicionado
- **msxdisk**: utilitário completo de manipulação de imagens de disco MSX
  (`.dsk`, FAT12, MSX-DOS 1 e 2 com subdiretórios), num único executável
  (`dist/msxdisk.exe`) com quatro modos de uso -- ver
  [doc/msxdisk-spec.md](msxdisk-spec.md) para o histórico completo de
  fases:
  - **CLI one-shot**: `create`, `list`, `add`, `extract`, `delete`,
    `rename`, `mkdir`, `rmdir`, `copy`, `saveas`, `info`.
  - **Shell interativo** (`msxdisk` sem argumentos, ou `--cli`): sessão
    estilo FTP (`load`/`save`/`saveas`), `put`/`get`/`mput`/`mget` com
    confirmação por arquivo, comandos locais de filesystem (`ls`, `cd`,
    `md`, `rm`, `pwd`), histórico persistente (replxx).
  - **TUI** (`--tui`, estilo Norton Commander/XTree, FTXUI): duas colunas
    (local ↔ imagem), marcar e enviar/receber arquivos, criar/abrir/
    salvar, renomear/criar pasta/excluir, troca de tema.
  - **GUI** (`--gui`, Dear ImGui + GLFW + OpenGL3): mesma interface de
    duas colunas em janela gráfica, com identidade visual moderna própria
    (paleta escura/clara com acento azul, cantos arredondados, fonte
    Segoe UI) -- diferente de propósito do visual retrô da TUI.
  - De dentro do shell, `tui`/`call tui` e `gui`/`call gui` trocam de
    modo sem sair do processo.
- Configuração, temas e metadados de imagem do msxdisk guardados num
  banco SQLite (`~/.msxdisk/config.sqlite3`), compartilhado entre TUI e
  GUI (tema ativo, último diretório/imagem, notas por disco).
- `fwmsx --msxdisk <argumentos>`: o `fwMSX.exe` agora também dá acesso a
  todos os modos do msxdisk embutido, sem precisar do executável
  separado -- `msxdisk.exe` standalone continua existindo pra quem só
  quer o utilitário de disco.
- Nova opção de build `FWMSX_MSXDISK_GUI` (default `ON`) para compilar
  sem a GUI (sem depender de GLFW/OpenGL) quando não for necessária.

### Corrigido
- `DiskImage::RenameFile` aceitava renomear um arquivo para um nome
  diferente do atual normalmente, mas recusava com "já existe" ao
  renomear para o **próprio nome atual** (a busca por conflito achava a
  própria entrada). Agora é tratado como no-op bem-sucedido.

## [1.1.2] - 2026-09-28 - "Nemesis: Renomeacao"

### Alterado
- Renomeados os arquivos e funcoes de cada modulo, de `module_<lang>.*` /
  `load_module_<lang>()` para `init_<lang>.*` / `init_<lang>()`, deixando
  claro que cada funcao e o ponto de "inicializacao" daquele modulo:
  - `src/cpp/module_cpp.{h,cpp}` -> `src/cpp/init_cpp.{h,cpp}`
    (`load_module_cpp` -> `init_cpp`)
  - `src/c/module_c.{h,c}` -> `src/c/init_c.{h,c}` (`load_module_c` ->
    `init_c`)
  - `src/asm/module_asm.{h,asm}` -> `src/asm/init_asm.{h,asm}`
    (`load_module_asm` -> `init_asm`; label interno `fmt_asm` ->
    `fmt_init_asm`)
  - `src/fortran/module_fortran.{h,f90}` -> `src/fortran/init_fortran.{h,f90}`
    (`load_module_fortran` -> `init_fortran`; modulo Fortran interno
    `module_fortran` -> `mod_init_fortran`, para nao colidir com o nome
    da funcao)
- `CMakeLists.txt` e `src/cpp/main.cpp` atualizados para os novos nomes de
  arquivo/funcao.
- Documentacao (`SPEC.md`, `MANUAL.md`, `README.md`) atualizada para
  refletir a nova convencao de nomes.

## [1.1.1] - 2026-09-28 - "Knightmare: Alicerce"

### Adicionado
- Reestruturacao completa do projeto em `src/` (por linguagem: `cpp/`,
  `c/`, `asm/`, `fortran/`, `common/`), `doc/`, `dist/` e `resource/`.
- `main.cpp` (C++) como ponto de entrada do projeto, recebendo o nome do
  produto e a versao (major.minor.patch) como parametros de linha de
  comando, com defaults em `src/common/version.h`.
- Quatro modulos de "carregamento", um por linguagem, cada um imprimindo
  sua propria mensagem com a versao entre colchetes e devolvendo uma
  assinatura hexadecimal: C++ (`load_module_cpp`, `0x0001`), C
  (`load_module_c`, `0x0002`), Assembly (`load_module_asm`, `0x0003`,
  NASM/ABI Win64) e Fortran (`load_module_fortran`, `0x0004`).
- Aviso de copyright `Copyright (c) 1972-2026 Cybernostra, Inc.` impresso
  no inicio da execucao, e resumo das quatro assinaturas impresso ao
  final.
- Documentacao viva do projeto: `SPEC.md`, `MANUAL.md`, `CHANGELOG.md`
  (este arquivo) e `RELEASE.md`.
- `CMakeLists.txt` raiz (C/C++/ASM_NASM/Fortran) e `build.ps1`
  (compilacao via MSYS2 UCRT64 + empacotamento automatico do ZIP em
  `dist/`).

### Corrigido
- Saida do modulo Fortran aparecendo fora de ordem (buffer de E/S proprio
  do `libgfortran`, sem sincronizacao automatica com `std::cout`/`printf`)
  -- corrigido com `FLUSH(output_unit)` explicito apos o `PRINT`.
- Assinaturas hexadecimais impressas erradas (`0x1000` em vez de
  `0x0001` etc.) -- o alinhamento `std::left` usado para a coluna do
  rotulo "vazava" para a coluna do valor hexadecimal; corrigido com
  `std::right` explicito antes do `std::setw` do valor.

### Removido
- Prototipo original solto na raiz do repositorio (`main.cpp`, `hello.c`,
  `hello.h`, `module_c.*`, `module_cpp.*`, `module_fortran.f90`,
  `rotinas.asm`), migrado e reescrito dentro de `src/`.
