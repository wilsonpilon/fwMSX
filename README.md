# fwMSX

**fwMSX** e um projeto de aprendizado com o objetivo final de se tornar um
emulador de MSX, evoluido a partir do [fMSX](https://fms.komkon.org/fMSX/)
(Marat Fayzullin), com o aval do autor original para essa adaptacao. A base
do fMSX e C/Unix; a ideia aqui e reestrutura-la, modernizar e, ao longo do
caminho, estudar e reaproveitar codigo de outros emuladores/ferramentas de
MSX (ver [resource/](resource/)).

> **Este projeto NAO substitui o fmsxGO** (o projeto principal). O fwMSX e
> apenas para aprendizado: praticar o ferramental MSYS2 no Windows e me
> atualizar com C/C++/Fortran/Assembly mais atuais.

Por se tratar de um projeto de estudo, ele tem uma regra propria, alem de
"emular um MSX direito": **toda fase relevante do projeto precisa exercitar
codigo real em C, C++, Assembly e Fortran** -- mesmo quando minimo -- para
forcar contato pratico com interoperabilidade entre linguagens (ABI,
name mangling, calling conventions, linkedicao).

## Estado atual (v1.43.14 "Yie Ar Kung-Fu: Outline e preview", Konami)

![fwMSX em janela](images/fwMSX-01.png)

O **fwMSX e um emulador de MSX** (MSX1, MSX2 e MSX2+) escrito em C, C++, Assembly e Fortran. Sem
argumentos, `fwMSX.exe` abre a maquina MSX1 numa janela, com a BIOS real e o MSX BASIC.

<p align="center">
  <img src="images/fwMSX-05.png" alt="Gradius (MSX1) rodando no fwMSX" width="49%">
  <img src="images/fwMSX-06.png" alt="Vampire Killer (MSX2) rodando no fwMSX" width="49%"><br>
  <em>Um jogo de MSX1 (Gradius / Nemesis) e um de MSX2 (Vampire Killer), como o emulador os desenha.</em>
</p>

**Funciona hoje:**

- **Maquinas**: MSX1 (BASIC 1.0), MSX2 (BASIC 2.1) e MSX2+ (BASIC 3.0, V9958), ate o prompt do BASIC.

  <p align="center"><img src="images/fwMSX-04.png" alt="MSX BASIC 3.0 (MSX2+) no fwMSX" width="60%"><br>
  <em>MSX BASIC 3.0 (MSX2+) com a BIOS real, comandos digitados pela ponte de controle (<code>--keys</code>).</em></p>

- **Disco**: MSX-DOS 1.8 a partir de `.dsk`, com leitura e gravacao (e protecao contra gravacao); tambem pela
  controladora por portas (estilo Microsol, driver DDX 3.0/CDX-2), formatos 180/360/720 KB.
- **Fita** (`.cas`, `.tsx`/`.tzx`, `.wav`): leitura (modo rapido e normal, com som), gravacao (`CSAVE`/`BSAVE "CAS:"`,
  fita nova, protecao, 3 modos, marcar o ponto, contagiros), janela visual "Fita K7", o comando `tape` (listar, ponto,
  protecao, converter entre formatos, gravar um bloco da memoria, carregar um arquivo da fita), `break tapin` no depurador,
  `TAPE`/`R`/`TLOAD`/`TSAVE` no Mamute (`man mamute-fita`), a diretiva `.TSX` do fwasm, e a ferramenta de linha
  de comando `fwmsx --cas` (empacotar `.BIN`/`.BAS` num `.TSX`, "ripar" uma gravacao `.wav` real, listar o
  conteudo de uma fita). Navegacao completa dos blocos de controle do TZX (grupos/lacos/saltos/chamadas/
  selecao). Banco de fitas com metadados (`fwmsx --fitadb`, titulo/empresa/ano/SHA-1, SEM download). Ver
  [tape-spec.md](doc/tape-spec.md).
- **Cartuchos**: ROM plana, MegaROM (Konami, ASCII, Gen8, Gen16), SRAM com `.sav`, o SCC (F1 Spirit toca
  a trilha) e MSX-DOS 2 (cartucho generico, testado com um kernel 2.30 real) -- mapper escolhivel
  pela CLI (`--cart <rom> msxdos2`) ou pela janela (menu Midia > Cartucho > Mapper).
- **Video**: VDP completo: SCREEN 0 a 8 no V9938 e 10 a 12 no V9958 (YJK, YAE e scroll); efeitos
  de rastreio no meio do quadro (paleta/scroll trocados por interrupcao de linha).
- **Som**: PSG, SCC e **FM (MSX-MUSIC e FM-PAC)** com os comandos de BASIC (`CALL MUSIC`, `PLAY #n`, `CALL VOICE`), modo ritmo e saida ao vivo.
- **Layout de slots mais amplo**: RAM de 16 KB no fim da celula, RAM de 32 KB em duas celulas, mapper de 64 KB a 4 MB (varios mappers), BIOS Expert 1.1 subindo.
- **Banco de ROMs** (`fwmsx --romdb` e menu **ROMs**): baixa o fMSX 6.0, o System ROMs do file-hunter e o banco
  do Vampier; busca, edicao e identificacao; `verify` confere o SHA-1 de cada ROM contra o banco (arquivo
  faltando/alterado); `--cart` sem mapper explicito consulta o banco (SHA-1) antes da heuristica por tamanho.
  As ROMs nao vem com o emulador (ver [romdb-spec.md](doc/romdb-spec.md)).
- **Configuracao**: menu **Maquina > Configuracao de slots...**: 16 celulas (BIOS, BASIC, RAM de 16 a 64 KB, mapper de 64 a 1024 KB, cartucho, disco, sub-ROM e FM-PAC).
- **Save-state**: menu **Arquivo > Salvar estado.../Carregar estado...** (`.sst`; atalhos **F6** salva / **F7** carrega / **F8-F9** trocam de slot, 9 slots; avisa se a BIOS/cartucho mudou; **F12** captura a tela em PNG) -- grava/restaura
  Z80, VDP, PSG, SCC, OPLL, PPI, controladora de disco e RAM/RAM de mapper. Aplica sobre a maquina
  ja' rodando (nao recarrega BIOS/cartucho/disco/fita). Ver [savestate-spec.md](doc/savestate-spec.md).
- **Janela**: menus do fMSX, zoom, proporcao, tela cheia e filtros de video -- confirmados na tela; **splash** de abertura e icone proprio
  (desligaveis; ver [splash-spec.md](doc/splash-spec.md)); tema, letra e moldura ficam gravados em `fwmsx.cfg`.
- **Depurador**: breakpoints, watchpoints, passo/passo sobre/ate', desmontagem, memoria, pilha e registradores, com avisos assincronos
  (`! break ...`); **Z80 Debugger visual** na janela (F10, com a fonte do proprio MSX) e na TUI. Ver [debug-spec.md](doc/debug-spec.md).

  <p align="center">
    <img src="images/fwMSX-07.png" alt="Z80 Debugger: desmontagem" width="48%">
    <img src="images/fwMSX-08.png" alt="Z80 Debugger: dump de memoria com os caracteres do MSX" width="48%"><br>
    <em>O Z80 Debugger parado num breakpoint do jogo (desmontagem: endereco, bytes e instrucao, registradores no topo) e o dump de memoria,
    onde os bytes aparecem tambem como os caracteres do MSX (fonte tirada da BIOS).</em>
  </p>
- **Controle externo e interfaces**: ponte de controle por TCP (`--ctl-port`), console (`fwMSXc.exe --cli`), TUI de menus (`--tui`) e modo
  terminal (`--term`). Ver [control-spec.md](doc/control-spec.md).
- **Mamute** (`mon` no console/TUI, `fwmsx --mon`): monitor Z80 -- memoria, disassembler, execucao, LOAD/SAVE, editor de fonte e montagem pelo fwasm, telas (paineis na TUI) e os comandos do **SUPER-X**
  (alvos `#slot-sub`/`#V`, relocar blocos com `XRT`, sessao de montagem `XM`, registradores/breakpoints/trace, portas, disco e notas) -- online (a maquina emulada) ou offline (memoria propria). `man mamute`. Ver [mamute-spec.md](doc/mamute-spec.md).
- **Montador Z80** (`fwmsx --asm`, compativel com o asMSX 1.2.0; saidas `.z80`/`.sym`/`.txt` -- formatos de ROM ainda por vir). Ver [asm-spec.md](doc/asm-spec.md).
- **Compatibilidade com o fMSX**: comparacao quadro a quadro com o fMSX original (`tools/fmsxgo_ref/compare.py`): 26 casos, todos aprovados
  (BIOS dos 3 modelos e jogos). Ver [fmsx-compare-spec.md](doc/fmsx-compare-spec.md).

**Limites (detalhes em [RELEASE.md](doc/RELEASE.md) e [SPEC.md](doc/SPEC.md), secao 5.0):**

- O **som do FM, do SCC e da fita (modo normal)** ainda nao foram comparados com hardware real. Use `--wav` para gravar e ouvir.
- **Fita**: download de fitas pelo site continua fora de escopo (sem termos de uso publicados -- so' o
  banco de metadados, sem download, existe); `.cas` cru sem gravacao previa deste emulador ainda
  adivinha o preenchimento de alinhamento; um arquivo ASCII multi-bloco de verdade (256 bytes por bloco)
  aparece fragmentado na lista; "Selecao" (#28 do TZX) nao tem como mostrar um menu de verdade numa
  ferramenta batch -- escolhe sempre a 1a opcao. Ver [tape-spec.md](doc/tape-spec.md), secoes 5, 9, 10 e 11.
- **Jogos**: Lode Runner + SCC, Parodius, R-Type e outros 20 jogos batem com o fMSX na comparacao de quadros; Mega Chase so' foi validado
  ate o titulo e F-1 Spirit 3D (troca de disco pela janela) nao foi testado.
- **Ausentes**: GameMaster2, `CALL VOICECOPY` e opcao de linha de comando para o layout de slots.
  Save-state nao valida se o cartucho/disco/fita inserido agora e' o mesmo de quando foi salvo.
- **BIOS**: so' em 0:0. Outras BIOS (ex.: Gradiente Expert 1.1) montam no layout, mas nao foram testadas.

**Configurar a maquina** (menus da janela; a lista completa de opcoes de linha de comando esta no
[MANUAL](doc/MANUAL.md)):

- **Maquina > Modelo**: MSX1, MSX2 ou MSX2+ (volta o layout ao padrao).
- **Maquina > Configuracao de slots...**: monta o layout celula a celula. Exemplos que funcionam:
  - uma BIOS de 32KB em 0:0 (a pagina 0 recebe a BIOS, a pagina 1 o BASIC);
  - BIOS e BASIC em dois arquivos de 16KB, na mesma celula 0:0;
  - quatro bancos de RAM de 64KB no slot 2 (celulas 2:0 a 2:3);
  - um mapper de 1024KB no slot 3:1.
- **Midia > Cartucho**: insere ou retira o cartucho do slot 1:0; escolhe o mapper (auto ou
  explicito -- ex.: MSX-DOS 2) pelo submenu **Mapper**; liga/desliga o FM-PAC no slot 2:0.
- **Midia > Disco**: insere e ejeta os discos A: e B:.
- **Midia > Fita**: inserir, criar fita nova, ejetar, rebobinar, trocar o modo de carregamento,
  destravar/travar contra gravacao, escolher o modo de gravacao, e a janela visual "Fita K7".
- **Tela > Exibir/Video**: zoom, tela cheia, proporcao, interpolacao, scanlines, filtro de cor.
- **Som, Ferramentas > Interface**: aparencia e audio.

Historico das versoes, em uma linha cada: 1.12.0 abre a maquina sem argumentos; 1.13.0 SCC;
1.14.0 CPIR/CPDR em Assembly; 1.15.0 MSX2+ (V9958); 1.16.0 janela com menus e filtros; 1.17.0 FM com
BASIC, layout de slots e SRAM do FM-PAC; 1.18.0 banco de ROMs e disco por portas; 1.19.x fita (leitura);
1.20.x fita (gravacao); 1.21.0 empacotador `.BIN`/`.BAS` -> `.TSX` (`--cas pack`); 1.22.0 ripper de
`.WAV` (`--cas rip`); 1.23.0 navegacao de blocos de controle do TZX; 1.24.0 banco de fitas com
metadados (`--fitadb`, sem download); 1.25.0 banco de ROMs com `verify` e auto-mapper pelo SHA-1;
1.26.0 Vampier com Platform/CRC32/tamanho; 1.27.0 efeitos de rastreio no meio do quadro;
1.28.0 MSX-DOS 2 (cartucho generico); 1.29.0 save-state;
1.30.0 menus reorganizados + mapper do cartucho na janela;
1.30.1 atalhos de save-state (9 slots), captura de tela (F12) e janela sem console;
1.31.0 disco novo em branco/formatado (180/360/720 KB) e `CALL FORMAT`;
1.32.0 ponte de controle externa (comandos de texto por TCP em localhost);
1.33.0 console `fwMSXc.exe --cli` (inicia o emulador e manda comandos a ele);
1.34.0 modo terminal `fwMSXc.exe --term` (a maquina rodando dentro do terminal);
1.35.0 TUI de menus `fwMSXc.exe --tui` (menus que controlam o emulador em janela);
1.36.0 montador Z80 `fwmsx --asm` (compativel com o asMSX 1.2.0);
1.36.1 texto de SCREEN 0 como no fMSX (comparacao pixel a pixel com o fMSX original);
1.36.2 MegaROM com AB no fim (R-Type);
1.37.0 depurador basico + eventos assincronos (console, TUI, ponte, parametros);
1.37.1 splash de abertura, icone do aplicativo e configuracoes salvas;
1.37.2 Z80 Debugger na janela (F10, fonte do MSX);
1.37.3 Z80 Debugger na TUI (comando `dbgui`);
1.37.4 `type`/`--keys` seguem o teclado da BIOS (japones x internacional);
1.37.5 TUI mais leve (redesenha so' quando muda);
1.37.6 TUI sem sujeira (saida dos comandos sem codigos ANSI);
1.37.7 `type` com teclas: `type "key off" {enter}`, `{ctrl}+{c}`;
1.37.8 `--keys` aceita a mesma sintaxe;
1.38.0 `help` organizado, `help <comando>` e `man <topico>` (manual + referencia do MSX: BIOS, variaveis, ganchos, cores...);
1.38.1 manual completo do MSX-BASIC no `man` (172 palavras com exemplos + apendices);
1.38.2 rotinas/variaveis da BIOS e BDOS (554) e hardware (33) no `man`; `man -l` e `man -k`;
1.38.3 The MSX Red Book, MSX2 Technical Handbook e ajuda do Basic Dignified no `man`;
1.38.4 MSX-DOS 2 (por comando e por chamada), MSX-DOS 1, CP/M, Z80, R800, FM-PAC e extensoes CALL no `man`;
1.38.5 manual do montador fwasm (`man fwasm`), NestorBASIC, SEE Tracker e MSXBAS2ROM no `man`;
**1.43.14 o editor tem Outline (titulos, REMs, rotulos) e preview de Markdown (`F7`)**
**1.43.13 o editor formata paragrafos e o documento (Edit > Paragraph format / Document format); o pacote leva a pasta `emulator/`**
**1.43.12 pasta `emulator/` com os discos de sistema do MSX-DOS 1 e 2; `dos: 1|2` no `Obifile`**
**1.43.11 o `obi`: um `Obifile` junta o `.dmx`, o `.asm` e os dados num `.dsk` pronto para rodar (`obi build`, `obirun`, Run > Build disk, F6)**
**1.43.10 o editor imprime em PDF (File > Print / Print Block, com as margens do Print setup)**
**1.43.9 o completar do editor oferece os rotulos e constantes dos outros arquivos do projeto; `man editor`**
**1.43.8 os menus do editor foram concentrados em oito (File Edit Search View Project Run Tools + Help), como nas IDEs atuais; as teclas do Norton continuam**
**1.43.7 com um projeto aberto, o Build (montador e Basic Dignified) le os arquivos incluidos do proprio projeto, nao da copia da pasta**
**1.43.5 o projeto guarda as versoes anteriores de cada arquivo (restaurar) e a remocao de um arquivo viaja para os outros PCs (pergunta antes de apagar a copia)**
**1.43.4 a arvore do projeto tem menu de contexto (abrir, atualizar, restaurar, renomear, adicionar, tirar)**
**1.43.3 Project > Files... virou arvore (pastas recolhiveis) no TUI**
**1.43.2 o editor lembra o ultimo projeto, mostra o nome na barra de status e (na janela) a arvore de arquivos do projeto**
**1.43.1 o editor ganhou o menu Project: salvar leva o arquivo ao .msxproject, Sync mostra as diferencas e conflito pergunta**
**1.43.0 o projeto .msxproject: UM arquivo SQLite com tudo + copias soltas sincronizadas (comando `project`, GitHub pela pasta)**
**1.42.2 erro de execucao do BASIC (Syntax error in 130) volta a' linha do .dmx: Montar > BASIC line...**
**1.42.1 o Basic Dignified roda programas reais: biblioteca do NestorBASIC (include "nbasic.dmx") e arquivos de apoio no disco de execucao (##BB:disk_files=)**
**1.42.0 o Basic Dignified Suite e' nosso: `fwmsx --badig prog.dmx` gera o `.amx` e o BASIC tokenizado `.bmx` (igual ao badig.py original), F9/F5 no editor convertem e rodam no emulador, e ha' a tela de configuracao**
**1.41.9 o editor tem o menu Build: F9 monta o fonte Z80 do buffer e leva o cursor ao erro, F5 executa no MSX, Ctrl-F1 abre o manual da palavra**
**1.41.8 o editor completa palavras (Alt-/) e expande snippets (Tab), e mostra numeros de linha na TUI e na janela**
**1.41.7 o destaque conhece o MSX-BASIC KUN de verdade: blocos `_TURBO ON/OFF`, remtags `'#I` `'#C` `'#N` e os comandos que o compilador recusa em vermelho**
**1.41.6 destaque de sintaxe no editor (TUI e janela): MSX-BASIC, Basic Dignified, MSXBAS2ROM, NestorBASIC, Turbo Basic, Assembly Z80 (com a BIOS), Pascal, Batch e Markdown**
**1.41.5 autosave do editor (a cada N minutos; so' a sessao de recuperacao ou tambem os arquivos com nome) e abas sem nome noname01, noname02...**
**1.41.4 o editor do emulador so' esconde ao fechar e guarda as anotacoes (inclusive entre execucoes); corrigido o travamento ao reabrir**
**1.41.3 o editor de textos GRAFICO (fase E3 refeita): menu "Editor" no fwMSX (janela flutuante) e `fwMSX.exe --edit`, com menus, abas, divisao em 2 paineis, mouse e dialogos de verdade, no visual do fwMSX e com os atalhos do Norton Editor 2**
**1.41.2 o editor de textos em janela (fase E3): `fwMSX.exe --edit arquivo`, o mesmo editor da TUI com o teclado completo do Norton Editor 2 (Ctrl-Enter, Ctrl-PgUp, Alt sozinho abre o menu)**
**1.41.1 o editor de textos na TUI (fase E2): `fwMSXc --edit arquivo` / `--view arquivo` com a tela, os menus, os dialogos, as duas janelas e o mouse do Norton Editor 2**
**1.41.0 o editor de textos, fase E1: o nucleo do editor estilo Norton Editor 2 (teclas e menus do NE 2, blocos, busca, desfazer, modo somente leitura `view`), ainda sem tela (TUI na proxima fase)**
**1.40.5 o montador na TUI: `asmrun` monta e CARREGA (ROM, fita ou disco), simbolos e linhas do fonte no depurador (`break loop`, `break jogo.asm:42`, `sym`, `src`), marcadores `.break`/`.debug` funcionando no Z80, BOM/UTF-8 no fwasm**
**1.40.4 Mamute: fita cassete em todos os pontos -- `.cas`, `.tsx` e `.wav` (ler e gravar) no emulador (`tape new/list/seek/save/put/load`), no depurador (`break tapin`), no Mamute (`TAPE`, `R`, `TLOAD`, `TSAVE`, `A OT>jogo.cas`) e no fwasm (`.TSX`)**
**1.40.3 Mamute: os comandos do SUPER-X (prefixo X) -- `XD XA XI XH XM`, `XBT XRT XFL XCM XFD`, `XRG XGO XTR`, portas, disco (`XDK XFS XTP XL% XS%`...) e notas, com alvos `#slot-sub`/`#V`; `man mamute-superx`**
**1.40.2 Mamute: as telas -- `DM`, `M`, `S`, `ZAP` (setores de disco) e `SCR` (memoria como pixels) como paineis interativos na TUI, e listagens em PDF**
**1.40.1 Mamute: `EDIT` (editor de fonte de linhas numeradas) e `A` (montar pelo fwasm, com listagem, referencia cruzada e gravacao na memoria/maquina)**
**1.40.0 Mamute: o monitor Z80 do fwMSX (`mon`, `fwmsx --mon`), online na maquina emulada ou offline com memoria propria; `man mamute`**
1.39.2 fwasm monta o jogo Amazonia (cartucho de 48 KB) igual ao asMSX;
1.39.1 fwasm testado com um replayer de musica PT3 real (monta igual ao asMSX e toca no emulador);
**1.39.0 fwasm completo: gera ROM, MegaROM, BASIC (.bin), MSX-DOS (.com), fita (.cas/.wav) e .tap (ZX Spectrum)**. Detalhes em
[CHANGELOG.md](doc/CHANGELOG.md).

Veja [doc/SPEC.md](doc/SPEC.md) para a especificacao completa e o historico de fases (documento vivo).

**Onde paramos e o que falta:** [OUTLINE.md](OUTLINE.md), feito para retomar o trabalho em outro computador ou com outra IA.

**Politica de midias:** este repositorio e' pessoal. ROMs, discos e fitas de terceiros podem ser versionadas por enquanto. Antes de liberar o projeto ao publico, cada midia de terceiros sera revisada, e os arquivos cujos detentores de direitos estiverem em desacordo serao removidos (ver LICENSE-THIRD-PARTY.md).

## Estrutura do projeto

```
fwMSX/
├── src/            fontes do projeto, um subdiretorio por linguagem
│   ├── cpp/        ponto de entrada (main) e modulo C++
│   ├── c/          modulo C
│   ├── asm/        modulo Assembly (NASM, ABI Win64)
│   ├── fortran/    modulo Fortran
│   ├── common/     cabecalhos compartilhados (versao, etc.)
│   ├── msxdisk/    utilitario de imagens de disco MSX (core/cli/shell/
│   │               tui/gui/config -- ver doc/msxdisk-spec.md)
│   ├── z80/        nucleo da CPU Z80 (core/cpp/asm/fortran/debug --
│   │               ver doc/z80-core-spec.md)
│   ├── memmap/     mapa de memoria MSX -- slots/subslots/MegaROM
│   │               (core/cpp/fortran -- ver doc/memory-map-spec.md)
│   ├── vdp/        VDP (TMS9918/V9938) -- registradores/portas/
│   │               renderizacao (core/cpp/fortran -- ver doc/vdp-spec.md)
│   ├── ppi/        PPI i8255 + teclado (core/cpp/fortran/asm -- ver
│   │               doc/ppi-spec.md)
│   ├── psg/        PSG AY-3-8910 (core/cpp/fortran -- ver doc/psg-spec.md)
│   ├── scc/        chip de som SCC (core/asm/fortran/cpp -- ver doc/scc-spec.md)
│   ├── fm/         chip FM OPLL (MSX-MUSIC/FM-PAC; core/asm/fortran/cpp -- ver doc/fm-spec.md)
│   ├── rtc/        relogio RTC do MSX2 (header-only -- ver doc/msx2-spec.md)
│   ├── fdc/        controladora de disquete WD2793 (core/cpp -- ver
│   │               doc/fdc-spec.md)
│   ├── tape/       fita (.CAS, .TSX/.TZX), gancho de BIOS, pulsos KCS e
│   │               a CLI `fwmsx --cas` (core/cpp/fortran/asm/cli -- ver
│   │               doc/tape-spec.md)
│   ├── romdb/      banco de ROMs (SQLite, downloads, CLI `--romdb` --
│   │               ver doc/romdb-spec.md)
│   ├── tapedb/     banco de fitas (SQLite, SEM download, CLI `--fitadb`
│   │               -- ver doc/tape-spec.md, secao 11)
│   ├── audio/      saida de audio ao vivo (miniaudio -- ver
│   │               doc/audio-spec.md)
│   └── machine/    maquina MSX1 completa + janela (`--msx` -- ver
│                   doc/machine-spec.md)
├── tools/msxdisk/  ponto de entrada do executavel msxdisk standalone
├── tests/z80/      testes do nucleo Z80, memoria, VDP, PPI, PSG, maquina,
│                   audio, disco e fita (CTest -- z80test/z80dbgtest/
│                   memmaptest/vdptest/vdp2test/ppitest/psgtest/
│                   machinetest/msx2test/audiotest/fdctest/tapetest/
│                   castooltest), mais tests/romdb/ (romdbtest) e tests/tapedb/ (tapedbtest)
├── doc/            documentacao viva do projeto
│   ├── SPEC.md         especificacao completa + fases do projeto
│   ├── msxdisk-spec.md especificacao + fases do utilitario msxdisk
│   ├── z80-core-spec.md especificacao + fases do nucleo Z80
│   ├── memory-map-spec.md especificacao + fases do mapa de memoria
│   ├── vdp-spec.md      especificacao + fases do VDP
│   ├── ppi-spec.md      especificacao + fases do PPI/teclado
│   ├── psg-spec.md      especificacao + fases do PSG
│   ├── scc-spec.md      especificacao + fases do SCC
│   ├── fm-spec.md       chip FM (MSX-MUSIC e FM-PAC) e comandos de BASIC
│   ├── tape-spec.md     fita: .CAS/.TSX/.TZX, gravacao, `--cas` (pack/rip/list), navegacao do TZX
│   ├── romdb-spec.md    banco de ROMs (SQLite, downloads, CLI)
│   ├── slots-spec.md    layout de slots e configuracao da maquina
│   ├── sram-spec.md     SRAM de cartucho e FM-PAC (arquivos .sav)
│   ├── machine-spec.md  maquina completa + janela com teclado do host
│   ├── audio-spec.md    audio ao vivo
│   ├── fdc-spec.md      disco: controladora WD2793 + MSX-DOS
│   ├── msx2-spec.md     MSX2: VDP V9938, mapper de RAM, RTC
│   ├── MANUAL.md       como compilar e executar
│   ├── CHANGELOG.md    resumo das alteracoes entre versoes
│   └── RELEASE.md       detalhes de cada release
├── images/         imagens usadas nesta documentacao
├── dist/           pacote pronto para execucao (.exe + .zip)
├── resource/       codigo-fonte de terceiros usado como referencia/estudo
├── CMakeLists.txt  build raiz (CMake + Ninja)
├── build.ps1       script de build (PowerShell/Windows)
└── build.sh        script de build (Bash/Linux)
```

## Compilar e executar

Resumo rapido (toolchain: MSYS2 UCRT64 -- gcc/g++/gfortran/nasm/cmake):

```powershell
.\build.ps1
.\dist\fwMSX.exe
.\dist\msxdisk.exe          # utilitario de disco standalone (CLI/shell/TUI/GUI)
```

Instrucoes completas, pre-requisitos e saida esperada em
[doc/MANUAL.md](doc/MANUAL.md).

## Versionamento

Formato `X.Y.Z`, sem tags numericas isoladas -- cada versao recebe o nome de
um jogo classico de MSX e um subtitulo curto indicando onde o projeto esta
naquele ponto (ex.: `v1.1.2 -- "Nemesis: Renomeacao"`). Regras completas em
[doc/SPEC.md](doc/SPEC.md#versionamento); detalhes de cada release em
[doc/RELEASE.md](doc/RELEASE.md); resumo das mudancas em
[doc/CHANGELOG.md](doc/CHANGELOG.md).

## Creditos

- **[Marat Fayzullin](https://fms.komkon.org/fMSX/)** -- autor original do
  fMSX, sobre o qual o fwMSX e construido (com aval do proprio autor para
  esta adaptacao/evolucao). O setor de boot real do MSX-DOS 1 usado pelo
  `msxdisk` (`src/msxdisk/core/msxdos1_boot.cpp`) e contribuicao direta
  dele (`resource/DiskUtilities/Boot.h`). O motor de despacho, tabelas e
  desmontador da CPU Z80 (`src/z80/core/`, `src/z80/debug/z80_disasm.*`)
  sao adaptados de `resource/fMSX/Z80/` (`Z80.c`, `Tables.h`, `Codes*.h`,
  `Debug.c`) -- ver `doc/z80-core-spec.md` e `LICENSE-THIRD-PARTY.md`. O
  motor de slots/subslots e os mappers MegaROM (`src/memmap/core/`) sao
  adaptados de `resource/fMSX/fMSX/MSX.c`/`MSX.h` -- ver
  `doc/memory-map-spec.md`. O motor "digital" do VDP e a renderizacao de
  SCREEN 0/1/2 (`src/vdp/core/`) sao adaptados de
  `resource/fMSX/fMSX/MSX.c` (registradores/portas/interrupcao) e
  `resource/fMSX/fMSX/Common.h` (`RefreshLine0/1/2`) -- ver
  `doc/vdp-spec.md`.
- **Arnold Metselaar** -- autor do utilitario original de manipulacao de
  discos MSX (`resource/DiskUtilities/DiskUtil.c` e correlatos), usado
  como referencia de estudo do formato FAT12/MSX-DOS para o `msxdisk`
  (nunca portado linha a linha -- ver `doc/msxdisk-spec.md`, secao 2).
- **msxDiskUtil** (`resource/msxDiskUtil/`) -- projeto irmao do mesmo
  autor do fwMSX, reescrita em PureBasic do utilitario acima; usado para
  confirmar, de forma independente, o setor de boot do MSX-DOS 1.
- Bibliotecas de terceiros usadas pelo `msxdisk` (baixadas via CMake
  `FetchContent` no build, nao redistribuidas neste repositorio):
  [CLI11](https://github.com/CLIUtils/CLI11),
  [replxx](https://github.com/AmokHuginnsson/replxx),
  [FTXUI](https://github.com/ArthurSonzogni/FTXUI),
  [Dear ImGui](https://github.com/ocornut/imgui),
  [GLFW](https://www.glfw.org/), [SQLite](https://www.sqlite.org/) e
  [miniaudio](https://github.com/mackron/miniaudio) (audio do emulador).

## Licenca

O **codigo original do fwMSX e do msxdisk** (tudo escrito para este
projeto) e licenciado em **BSD 3-Clause** -- ver [LICENSE](LICENSE).

Isso **nao** cobre codigo adaptado/incorporado de terceiros. O fMSX tem
sua propria licenca, mais restritiva (proibe distribuicao comercial); o
fwMSX evolui a partir dele com o aval do proprio Fayzullin **para
adaptar/estudar seu codigo**, mas esse aval nao e uma autorizacao para
relicenciar o codigo dele sob BSD/GPL/MIT. Por isso, qualquer arquivo
deste repositorio que incorporar codigo do fMSX diretamente (o setor de
boot em `src/msxdisk/core/msxdos1_boot.cpp`; o motor/tabelas/desmontador
da CPU Z80 em `src/z80/core/` e `src/z80/debug/z80_disasm.*`; o motor de
slots/subslots e mappers MegaROM em `src/memmap/core/`; o motor do VDP e
a renderizacao em `src/vdp/core/`; o PSG em `src/psg/core/`; o SCC em
`src/scc/core/`; mais arquivos devem se juntar a essa lista) continua sob a licenca original
dele, nao BSD. Ver
**[LICENSE-THIRD-PARTY.md](LICENSE-THIRD-PARTY.md)** para o
detalhamento completo (fMSX, DiskUtilities, msxDiskUtil) e
[resource/README.md](resource/README.md) para o aviso sobre o material
de terceiros usado so como referencia de estudo.
