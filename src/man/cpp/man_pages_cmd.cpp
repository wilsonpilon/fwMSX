// fwMSX -- MAN/HELP: paginas dos comandos do console, do emulador e do depurador. Texto sem acento (ASCII).
#include "man_internal.h"

namespace man {

std::vector<Topic> CommandTopics() {
    std::vector<Topic> v;
    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {
        v.push_back(Topic{name, aliases, category, usage, summary, page, true});
    };

    // ------------------------------------------------------------------ Console
    add("help", "?", "Console", "help [comando]", "esta tabela; com um comando, o resumo dele",
        R"MAN(DESCRICAO
  Sem argumento mostra todos os comandos agrupados. Com um comando mostra o
  uso e uma linha de resumo. Para o manual completo, com exemplos, use "man
  <comando>".

EXEMPLOS
  help              a tabela de comandos
  help peek         uso e resumo do peek
  man peek          o manual do peek

VER TAMBEM
  man)MAN");

    add("man", "manual", "Console", "man [topico]", "manual completo de um comando ou assunto do MSX",
        R"MAN(DESCRICAO
  Abre o manual de um topico. Os topicos sao os comandos do fwMSX e a
  REFERENCIA do MSX: BIOS, variaveis do sistema, ganchos, comandos do BASIC,
  cores, portas de E/S, slots... Sem argumento lista todos. Se o nome nao
  existe, sugere os parecidos (aceita so' o comeco ou um pedaco: "man sysv"
  acha sysvars).

  Na TUI o manual abre numa janelinha: setas, PgUp/PgDn, Home/End rolam; Esc
  ou Q fecha. No console (--cli) o texto e' paginado: espaco = proxima
  pagina, Enter = proxima linha, Q = sai.

EXEMPLOS
  man               o indice de todos os topicos
  man type          como digitar teclas no MSX
  man bios          a tabela de entradas da BIOS (CHPUT, CHGET...)
  man cores         os codigos de cor do MSX

VER TAMBEM
  help)MAN");

    add("exit", "quit", "Console", "exit | quit", "sai do console (o emulador continua aberto)",
        R"MAN(DESCRICAO
  Encerra o console (--cli) ou a TUI. O emulador que foi aberto com "emu
  start" CONTINUA rodando; para fecha-lo use "emu stop" antes de sair.

  Na TUI, "clear" limpa o historico da tela.

VER TAMBEM
  emu)MAN");

    add("clear", "cls", "Console", "clear", "limpa o historico da tela (so' na TUI)",
        "DESCRICAO\n  Na TUI apaga o historico mostrado na tela. No console (--cli) use CTRL+L do terminal.\n\nVER TAMBEM\n  exit");

    add("emu", "", "Console", "emu start|attach|detach|stop|status", "abre, conecta ou fecha o emulador (janela)",
        R"MAN(DESCRICAO
  O console e a TUI controlam o emulador por uma ponte local (127.0.0.1).
  "emu" cuida da ligacao.

  emu start [opcoes]   abre a JANELA do emulador e conecta. As opcoes sao as
                       do --msx (ver o MANUAL): --msx2, --msx2p, --cart <arq>
                       [mapper], --disk <arq.dsk>, --fita <arq>, --mute,
                       --debug, --break <end>, --watch <end>, --bios <arq>...
                       O splash NAO aparece, a menos que se peca --splash.
  emu attach <porta>   conecta num emulador ja' aberto com --ctl-port <porta>.
  emu detach           desconecta; o emulador continua aberto.
  emu stop             fecha o emulador conectado.
  emu status           mostra se ha' emulador conectado e em que porta.

  Depois de "emu start" os comandos do emulador (peek, poke, type, cart,
  disk, state, dbg...) passam a funcionar. O prompt mostra a porta:
  fwmsx:60123>.

EXEMPLOS
  emu start                              MSX1 vazio (so' o BASIC)
  emu start --msx2p --cart jogo.rom      MSX2+ com um cartucho
  emu start --msx2 --disk jogo.dsk       MSX2 com um disquete
  emu start --debug --break 0x4010       comeca PARADO, com um breakpoint

VER TAMBEM
  wait, type, status, quit)MAN");

    add("wait", "", "Console", "wait [segundos]", "espera um evento do emulador (breakpoint, etc.)",
        R"MAN(DESCRICAO
  Fica ate' N segundos (padrao 10) esperando um evento assincrono do
  emulador -- por exemplo o breakpoint atingido depois de "go". O evento
  aparece como "[evento] break id=1 pc=4010 hits=1". Sem evento no tempo
  dado, avisa e volta ao prompt. Precisa de emulador conectado.

EXEMPLOS
  break 0x4010
  go
  wait 30

VER TAMBEM
  go, break, watch, events)MAN");

    // ------------------------------------------------------------------ Emulador
    add("version", "", "Emulador", "version", "versao do fwMSX", R"MAN(DESCRICAO
  Mostra o nome e o numero da versao do emulador conectado (X.Y.Z).)MAN");

    add("status", "", "Emulador", "status", "estado: quadro, pausa, tela, PC, cartucho, discos",
        R"MAN(DESCRICAO
  Uma linha com o estado atual:
    frame=N paused=0 stopped=0 screen=M pc=XXXX typing=0 cart=... diskA=...

  frame    quadros emulados desde o reset
  paused   1 = pausado por "pause"
  stopped  1 = parado pelo depurador
  screen   modo de tela do VDP (0 = SCREEN 0...)
  pc       contador de programa do Z80
  typing   1 = ainda digitando um "type"
  cart, diskA, diskB, tape   o que esta' inserido ("-" = nada))MAN");

    add("reset", "", "Emulador", "reset", "reinicia a maquina",
        R"MAN(DESCRICAO
  Reinicia o MSX (como apertar o botao de reset): a memoria e' zerada e a
  BIOS recomeca em 0000h. O cartucho e os discos inseridos continuam.)MAN");

    add("pause", "", "Emulador", "pause", "pausa a emulacao",
        "DESCRICAO\n  Congela a emulacao (a janela continua respondendo). Com a maquina pausada \"step\" avanca quadro a quadro.\n\nVER TAMBEM\n  resume, step");
    add("resume", "", "Emulador", "resume", "retoma a emulacao pausada",
        "DESCRICAO\n  Volta a rodar depois de \"pause\".\n\nVER TAMBEM\n  pause, step");
    add("step", "", "Emulador", "step [n]", "avanca n quadros (maquina pausada)",
        R"MAN(DESCRICAO
  Executa n quadros de video (1 a 3600, padrao 1) e fica pausado de novo.
  So' funciona com a maquina pausada ("pause" antes). Um quadro e' 1/60 s
  (NTSC) ou 1/50 s (PAL). Para passos de INSTRUCAO use "stepi" (depurador).

EXEMPLOS
  pause
  step 60          avanca 1 segundo

VER TAMBEM
  pause, resume, stepi)MAN");

    add("type", "", "Emulador", "type \"texto\" {tecla}...", "digita no teclado do MSX (texto e teclas)",
        R"MAN(DESCRICAO
  Digita no teclado do MSX. A pontuacao segue o teclado da BIOS carregada
  (japones no MSX2+ do fMSX, internacional nas outras). O comando volta na
  hora e a digitacao anda a cada quadro; "status" mostra typing=1 enquanto
  durar.

SINTAXE
  "texto"      texto entre aspas: cada caractere vira uma tecla (com SHIFT
               quando precisa). Dentro das aspas: \n = Enter, \\ = uma barra,
               \" = aspas.
  texto solto  igual, sem aspas; varios espacos viram um so'.
  {tecla}      uma tecla pelo nome, FORA das aspas. Maiusculas tanto faz.
  {a}+{b}      teclas apertadas JUNTAS: {ctrl}+{c} = Control-C (para um
               programa BASIC), {ctrl}+{stop}. {ctrl+c} e' o mesmo.
  {a}{b}       teclas EM SEQUENCIA (aperta e solta uma, depois a outra).
  {enter*3}    repete 3 vezes (1 a 200).

TECLAS
  letras e numeros: {a}..{z} {0}..{9}
  edicao:    {enter} {esc} {tab} {bs} {del} {ins} {home} {space}
  controle:  {stop} {select} {ctrl} {shift} {graph} {caps} {code}
  funcao:    {f1}..{f5}
  setas:     {up} {down} {left} {right}
  teclado numerico: {pad0}..{pad9} {pad*} {pad+} {pad/} {pad-} {pad,} {pad.}
  apelidos:  {return} {escape} {backspace} {delete} {insert} {control}
             {cls} (=home) {break} (=stop)

EXEMPLOS
  type "key off" {enter}
  type "10 print \"ola\"\nrun\n"
  type "list" {enter} {ctrl}+{stop}
  type {f1}
  type "color 15,4,4" {enter}

OBSERVACOES
  --keys (linha de comando do fwMSX) usa a mesma sintaxe, mas aspas e espacos ali sao texto comum.
  Um caractere sem tecla no MSX ou um nome de tecla desconhecido da' erro e
  nada e' digitado.)MAN");

    add("peek", "", "Emulador", "peek <end> [n]", "le n bytes da memoria (1 a 4096)",
        R"MAN(DESCRICAO
  Le n bytes (padrao 1) como o Z80 enxerga AGORA, isto e', com os slots e
  subslots selecionados no momento. Responde "ok 41 42 43 ...". Enderecos:
  0x4010, $4010, 4010h ou decimal.

EXEMPLOS
  peek 0xFCAF            o modo de tela atual (SCRMOD)
  peek $F3E9 3           cores: frente, fundo e borda (FORCLR, BAKCLR, BDRCLR)
  peek 0 16              os 16 primeiros bytes da BIOS

VER TAMBEM
  poke, mem, sysvars, slots)MAN");

    add("poke", "", "Emulador", "poke <end> <v>...", "grava bytes na memoria (a ROM nao muda)",
        R"MAN(DESCRICAO
  Grava um ou mais bytes a partir de <end>. Escrever em ROM nao altera nada
  (e' como no MSX real).

EXEMPLOS
  poke 0xC000 0x41 0x42     grava "AB" na RAM
  poke $F3E9 7              muda a cor da frente (depois um CLS no BASIC)

VER TAMBEM
  peek, mem, sysvars)MAN");

    add("vdp", "", "Emulador", "vdp [pal | n [valor]]", "registradores do VDP, paleta e tabelas da tela (e o comando screen)",
        R"MAN(DESCRICAO
  vdp                 R#0..R#46 em hexa (R#0..R#7 no MSX1), o status S#0..S#9 e uma linha com os bits
                      decodificados: tela ligada, IE0/IE1, sprites 8x8/16x16 e ampliados, 192/212
                      linhas, NTSC/PAL, sprites desligados.
  vdp pal             a paleta (V9938/V9958): 16 entradas em R/G/B de 0 a 7 e em #RRGGBB.
  vdp <n>             le o registrador n.
  vdp <n> <valor>     grava como um OUT do Z80 faria: o modo de tela e as tabelas sao recalculados e
                      R#46 dispara um comando do V9938. O BIOS pode regravar R#0/R#1 depois (ele guarda
                      copias em RG0SAV..RG7SAV); para a mudanca durar use vdpbios.
  vdpbios <n> <valor> o mesmo, e grava tambem o espelho do BIOS: RG0SAV..RG7SAV (F3DF), RG8SAV..RG23SAV
                      (FFE7) e RG25SAV..RG27SAV (FFFA). Os demais registradores nao tem espelho.

  screen              o texto da tela do MSX (SCREEN 0 e 1), lido da VRAM pelas variaveis do BIOS.
  screen info         o modo de tela e onde ficam as tabelas na VRAM: nomes, cores, padroes, atributos de
                      sprite, cores de sprite (modo 2) e padroes de sprite, com o tamanho quando e' fixo.

EXEMPLOS
  vdp                    tudo de uma vez
  vdp 7 0x1F             borda e cor de fundo (R#7)
  vdpbios 1 0x60         desliga IE0 e deixa o BIOS saber
  screen info            SCREEN 1: nomes 1800, cores 2000, padroes 0000, sprites 1B00/3800

NOTAS
  O Lua tem o mesmo: msx.vdp(n [, valor]) e msx.screen() (man lua).

VER TAMBEM
  peek, poke, lua, vdpregs, screens)MAN");

    add("vdpbios", "", "Emulador", "vdpbios <n> <valor>", "grava um registrador do VDP e o espelho dele no BIOS",
        R"MAN(DESCRICAO
  Como "vdp <n> <valor>", e grava tambem a copia que o BIOS guarda (RG0SAV..RG7SAV em F3DF, RG8SAV..RG23SAV
  em FFE7, RG25SAV..RG27SAV em FFFA), para que o BIOS nao desfaca a mudanca na proxima vez que regravar
  o registrador. Nao mexe nas outras variaveis do sistema (TXTNAM, SCRMOD...): serve para experimentar.

EXEMPLOS
  vdpbios 7 0xF4         borda azul clara que o BIOS respeita

VER TAMBEM
  vdp, poke, sysvars)MAN");

    add("regs", "", "Emulador", "regs", "registradores do Z80 numa linha",
        R"MAN(DESCRICAO
  AF BC DE HL IX IY SP PC numa linha. Para todos (inclusive sombra, I, R e
  flags) use "reg" do depurador.

VER TAMBEM
  reg, dis)MAN");

    add("quit", "", "Emulador", "quit", "fecha o emulador conectado",
        "DESCRICAO\n  Pede ao emulador para fechar a janela e encerrar (o mesmo que \"emu stop\" no console).\n\nVER TAMBEM\n  emu");

    // ------------------------------------------------------------------ Midia e estado
    add("cart", "", "Midia e estado", "cart <arq|-> [mapper]", "troca o cartucho (reinicia)",
        R"MAN(DESCRICAO
  Troca o cartucho do slot 1 e reinicia a maquina. "-" retira o cartucho.
  Sem mapper o fwMSX consulta o banco de ROMs pelo SHA-1 (comando romdb) e,
  se nao achar, usa ROM plana ate' 32 KB e deteccao acima.

  Mappers: auto, gen8, gen16, konami5 (SCC), konami4, ascii8, ascii16,
  msxdos2.

EXEMPLOS
  cart jogo.rom
  cart "C:\jogos\nemesis.rom" konami4
  cart -                    retira o cartucho

VER TAMBEM
  romdb, reset)MAN");

    add("dos2", "", "Midia e estado", "dos2 [rom.rom] [disco.dsk] | dos2 off", "liga o MSX-DOS 2: o kernel no slot 1 e o disco em A:",
        R"MAN(DESCRICAO
  O MSX-DOS 2 tem o kernel numa ROM de 64 KB (cartucho generico, mapper msxdos2) e no disco so' os
  arquivos MSXDOS2.SYS e COMMAND2.COM. "dos2" poe essa ROM no slot 1, liga a interface de disquete,
  coloca o disco em A: (se vier) e reinicia. Sem "rom.rom" usa a do banco de ROMs: o msxdos22.rom do
  Full Set System ROMs (janela Downloads; depois "romdb identify" marca como "MSX-DOS 2"). "dos2 off"
  tira o cartucho e reinicia.

  O disco de sistema vem de "Obifile: dos: 2" (F6 no editor, ou "obi build") ou de dist/emulator/
  msxdos2.dsk. Com "dos: 2" no Obifile, o comando "obirun" liga o MSX-DOS 2 sozinho.
  Prefira uma MSX2 (--msx2): ela tem relogio e RAM mapeada, e o MSX-DOS 2 nao pergunta a data.

  Na linha de comando: fwmsx --msx --msx2 --dos2 [rom.rom] --disk sistema.dsk

EXEMPLOS
  dos2 msxdos2.dsk
  dos2 C:\roms\msxdos22.rom jogo.dsk
  dos2 off

VER TAMBEM
  cart, disk, obi, romdb)MAN");

    add("disk", "", "Midia e estado", "disk <A|B> <arq>", "insere um disquete (.dsk) na unidade",
        R"MAN(DESCRICAO
  Insere a imagem .dsk (crua) na unidade A: ou B:. Precisa de interface de
  disco (emu start --disk...). Disco novo: "newdisk". Conteudo do disco:
  "msxdisk".

EXEMPLOS
  disk A jogo.dsk
  disk B dados.dsk

VER TAMBEM
  eject, newdisk, msxdisk)MAN");

    add("eject", "", "Midia e estado", "eject <A|B>", "tira o disquete da unidade",
        "DESCRICAO\n  Ejeta o disquete da unidade A: ou B:.\n\nVER TAMBEM\n  disk");

    add("tape", "", "Midia e estado", "tape [<arq>|eject|rewind|new|list|seek|protect|write|mode|save|put|load]", "fita cassete: inserir, listar, gravar, converter",
        R"MAN(DESCRICAO
  tape                    estado da fita (arquivo, quantos arquivos, protecao, modo de gravacao, ponto)
  tape <arq>              insere uma fita: .cas, .tsx/.tzx ou .wav (pela extensao); entra PROTEGIDA. tape insert <arq> se o nome
                          for igual a um subcomando
  tape eject              tira a fita
  tape rewind             volta ao comeco
  tape new <arq>          cria uma fita vazia e gravavel (.cas, .tsx ou .wav)
  tape list               os arquivos da fita: numero, tipo (bin/bas/asc), nome, inicio-fim, exec, tamanho
  tape seek <n|nome|->    marca o ponto: de onde o CLOAD/BLOAD continuam e onde "write overwrite" corta; - = inicio
  tape protect on|off     protege ou libera a gravacao (CSAVE/BSAVE "CAS:" e tape put)
  tape write append|overwrite|new   o que a PROXIMA gravacao faz: acrescenta, corta no ponto, ou apaga tudo
  tape mode fast|normal   carregamento: gancho de BIOS (rapido) ou pulsos de verdade (com o som do gravador)
  tape save <arq>         grava uma COPIA em outro formato (.cas, .tsx ou .wav, pela extensao)
  tape put <nome> <ini> <fim> [exec]   grava um bloco da memoria da maquina como arquivo binario na fita
  tape load <n|nome> [fim]             carrega um arquivo binario da fita na memoria (no endereco do cabecalho)

DEPURADOR
  As entradas da fita na BIOS tem nome: break tapin, until tapoon, dis tapion... (tapion tapin tapiof tapoon tapout tapoof,
  00E1 00E4 00E7 00EA 00ED 00F0).

EXEMPLOS
  tape jogo.tsx
  type "bload\"cas:\",r" {enter}
  tape new gravado.wav
  tape protect off

VER TAMBEM
  cas, fitadb, mamute-fita, fwasm-directives)MAN");

    add("state", "", "Midia e estado", "state save|load <arq>", "salva ou carrega o estado inteiro da maquina",
        R"MAN(DESCRICAO
  state save <arq>   grava um save-state (.sst): CPU, memoria, VDP, PSG...
  state load <arq>   restaura. Responde "ok aviso: ..." se a BIOS ou o
                     cartucho mudaram.
  Na janela: F6 salva, F7 carrega, F8/F9 trocam o slot.

EXEMPLOS
  state save antes-do-chefe.sst
  state load antes-do-chefe.sst)MAN");

    add("screenshot", "", "Midia e estado", "screenshot <arq>", "grava a tela do MSX (PNG)",
        "DESCRICAO\n  Grava a tela atual numa imagem (PNG na janela do emulador). Na janela, F12 faz o mesmo.\n\nEXEMPLO\n  screenshot tela.png");

    // ------------------------------------------------------------------ Depurador
    add("break", "", "Depurador", "break <end>", "breakpoint num endereco",
        R"MAN(DESCRICAO
  Para a maquina quando o PC chegar em <end>. Cada breakpoint tem um id. Ao
  parar, o evento "! break id=1 pc=4010 hits=1" aparece (no console como
  [evento]; na TUI abre o painel). Tambem pela linha de comando: emu start
  --break 0x4010.

  O endereco pode ser um numero, o nome de uma rotina de fita da BIOS (tapin,
  tapoon...), um rotulo do fonte montado com asmrun (break loop, break loop+4) ou
  uma linha do fonte (break jogo.asm:42). Ver sym e asmrun.

EXEMPLOS
  break 0x4010
  go
  wait 30

VER TAMBEM
  unbreak, breaks, go, watch, asmrun, sym)MAN");
    add("asmrun", "", "Depurador", "asmrun [-z] [-n] [--dos-disk <arq.dsk>] <fonte.asm>", "monta um fonte, carrega na maquina e entrega simbolos e linhas ao depurador",
        R"MAN(DESCRICAO
  Monta o fonte com o fwasm (as saidas .rom, .bin, .com, .sym... saem ao lado dele, como o comando asm), CARREGA o resultado na maquina emulada e
  entrega ao depurador os simbolos (rotulos e constantes) e o mapa endereco -> arquivo:linha. Depois disso valem break loop, dis meu_rotulo,
  break jogo.asm:42, src e as paradas mostram "parou em loop+2  jogo.asm:42". Precisa de um emulador em execucao (emu start).

  O que "carregar" quer dizer depende do formato que o fonte pede:
    .rom / .megarom   insere como cartucho (megarom: mapper konami->konami4, konamiscc->konami5, ascii8, ascii16) e reinicia
    .basic            monta uma fita na hora (mesmo sem a diretiva .cas), insere, reinicia e digita BLOAD"CAS:",R sozinho
    .msxdos           copia o .com para uma COPIA do disco de sistema (msxdos1.dsk na pasta atual, ou --dos-disk), insere em A:, reinicia,
                      pula a pergunta da data e digita o nome do programa. Exige a maquina com interface de disquete (emu start --disk)
    outros            so' monta
  Se o fonte usa .break ou .debug, os marcadores do depurador sao ligados sozinhos (markers on).

OPCOES
  -z                sintaxe Zilog padrao        -n   so' monta: nada e' carregado, mas simbolos e linhas entram no depurador
  --dos-disk <arq>  disco de sistema do MSX-DOS 1 para o .msxdos

EXEMPLOS
  asmrun jogo.asm
  asmrun -n -z jogo.asm
  break loop
  go

NOTA
  O .debug "texto" do asMSX gera LD D,D / JR (tamanho+4) / texto: o salto passa 4 bytes DEPOIS do texto. Ponha algo inofensivo (4 NOPs) depois dele,
  ou confira no seu depurador de referencia se o asMSX gera mesmo esse deslocamento.

VER TAMBEM
  sym, src, markers, break, asm, tape)MAN");
    add("sym", "", "Depurador", "sym [load <arq.sym>|clear|list [texto]|<nome|end>]", "simbolos do fonte (rotulos e constantes)",
        R"MAN(DESCRICAO
  sym                     resumo: quantos simbolos, quantas linhas de fonte, se os marcadores estao ligados
  sym load <arq.sym>      acrescenta os simbolos de um .sym do fwasm/asMSX ("4010h nome"; MegaROM: "01h:8000h nome")
  sym clear               esquece simbolos e linhas
  sym list [texto]        os simbolos por endereco (ate' 200), filtrados por texto
  sym <nome>              o endereco do simbolo (aceita nome+4)
  sym <endereco>          o rotulo mais proximo abaixo do endereco

  Os simbolos entram sozinhos com asmrun. Com eles, qualquer comando que pede um endereco aceita o NOME do rotulo (break loop, until loop+4,
  dis inicio, mem tabela) e arquivo:linha (break jogo.asm:42). O comando dis mostra "rotulo:" antes da instrucao, e os eventos de parada trazem
  sym=loop+2 e src=jogo.asm:42.

VER TAMBEM
  asmrun, src, break, dis)MAN");
    add("src", "", "Depurador", "src [linhas]", "linhas do fonte em volta do PC", "DESCRICAO\n  Mostra o fonte (arquivo:linha) que gerou a instrucao no PC, com algumas linhas em volta (padrao 4 de cada lado) e um > na atual. Precisa do mapa de linhas do asmrun.\n\nVER TAMBEM\n  asmrun, sym, stepi");
    add("markers", "", "Depurador", "markers [on|off]", "liga ou desliga os marcadores .break/.debug do fonte",
        R"MAN(DESCRICAO
  Os marcadores do depurador do blueMSX que o asMSX/fwasm colocam no codigo:
    .break              LD B,B / JR 0               para a maquina neste ponto ("! break marker pc=... sym=... src=...")
    .break <end>        LD B,B / JR 2 / DW <end>    cria um breakpoint em <end> (nao para agora)
    .debug "texto"      LD D,D / JR n / texto       imprime o texto como evento ("! debug pc=... text=...") e continua
  Sao instrucoes de verdade (LD B,B e LD D,D nao fazem nada), entao o programa roda igual com os marcadores desligados. Ligados, a maquina olha o
  byte em PC antes de cada instrucao (um pouco mais lento); por isso o padrao e' desligado. O asmrun os liga sozinho se o fonte os usa.
  markers sem argumento mostra o estado.

VER TAMBEM
  asmrun, break)MAN");
    add("unbreak", "", "Depurador", "unbreak <end|all>", "remove um breakpoint (ou todos)", "DESCRICAO\n  Remove o breakpoint de <end>; \"all\" remove todos.\n\nVER TAMBEM\n  break, breaks");
    add("breaks", "", "Depurador", "breaks", "lista os breakpoints (id, endereco, paradas)", "DESCRICAO\n  Lista os breakpoints: id, endereco e quantas vezes pararam.\n\nVER TAMBEM\n  break, unbreak");
    add("watch", "", "Depurador", "watch <end> [tam] [r|w|rw]", "watchpoint: para ao ler/escrever na memoria",
        R"MAN(DESCRICAO
  Para a maquina quando o Z80 LE (r), ESCREVE (w) ou faz os dois (rw) em
  [end, end+tam). Padrao: escrita, 1 byte. Evento: "! watch id=2 addr=C100
  rw=w value=77 pc=C012 next=C015".

EXEMPLOS
  watch 0xFC9E 2 w       quem mexe no JIFFY (contador de interrupcoes)?
  watch 0xC000 16 rw

VER TAMBEM
  unwatch, watches, break)MAN");
    add("unwatch", "", "Depurador", "unwatch <end|all>", "remove um watchpoint (ou todos)", "DESCRICAO\n  Remove o watchpoint de <end>; \"all\" remove todos.");
    add("watches", "", "Depurador", "watches", "lista os watchpoints", "DESCRICAO\n  Lista os watchpoints: id, faixa, tipo e quantas vezes dispararam.");
    add("stepi", "", "Depurador", "stepi [n]", "executa n instrucoes e fica parado",
        R"MAN(DESCRICAO
  Executa n instrucoes do Z80 (1 a 1000000, padrao 1) e fica parado.
  Responde com o PC e a proxima
  instrucao: "pc=0001  JP 02D7h".

VER TAMBEM
  next, until, go, dis)MAN");
    add("next", "", "Depurador", "next", "passo SOBRE: CALL/RST/blocos/HALT rodam inteiros",
        R"MAN(DESCRICAO
  Como "stepi", mas CALL, CALL cc, RST,
  LDIR/LDDR/CPIR/CPDR/INIR/INDR/OTIR/OTDR e HALT executam por inteiro (a
  subrotina roda toda e para na instrucao seguinte). O resto e' um stepi.

VER TAMBEM
  stepi, until)MAN");
    add("until", "", "Depurador", "until <end>", "roda ate' o endereco",
        R"MAN(DESCRICAO
  Roda ate o PC chegar em <end>. Espera ate 20 quadros; se nao chegar
  responde "running" e o evento "! until pc=..." avisa depois (use "wait").

VER TAMBEM
  go, break, wait)MAN");
    add("go", "", "Depurador", "go [end]", "volta a rodar (opcionalmente de <end>)",
        "DESCRICAO\n  Retoma a execucao parada pelo depurador; com <end> o PC passa a ser <end> antes. Responde \"running\" na hora.\n\nVER TAMBEM\n  stop, break, wait");
    add("stop", "", "Depurador", "stop", "para na proxima instrucao", "DESCRICAO\n  Para a maquina na proxima fronteira de instrucao. Use depois \"dis\", \"reg\", \"mem\" para olhar.\n\nVER TAMBEM\n  go, stepi");
    add("jump", "", "Depurador", "jump <end>", "muda o PC", "DESCRICAO\n  Coloca <end> no PC (nada mais muda). Depois \"go\" ou \"stepi\".\n\nVER TAMBEM\n  go, reg");
    add("dis", "", "Depurador", "dis [end] [n]", "desmonta n linhas (padrao 8) a partir de <end>/PC",
        R"MAN(DESCRICAO
  Desmonta n linhas (padrao 8) a partir de <end> (padrao: o PC). ">" marca o
  PC, "*" um breakpoint.

EXEMPLO
  dis 0 3
   0000  F3           DI
   0001  C3 D7 02     JP 02D7h

VER TAMBEM
  mem, stepi)MAN");
    add("mem", "", "Depurador", "mem <end> [n]", "hexdump com ASCII (16 bytes por linha)", "DESCRICAO\n  Hexdump de n bytes (ate 1024) a partir de <end>, 16 por linha, com a coluna ASCII.\n\nVER TAMBEM\n  peek, poke");
    add("stack", "", "Depurador", "stack [n]", "n palavras a partir de SP", "DESCRICAO\n  Mostra n palavras (1 a 128, padrao 8) da pilha a partir de SP.\n\nVER TAMBEM\n  reg");
    add("reg", "", "Depurador", "reg [nome valor]", "todos os registradores; ou altera um",
        R"MAN(DESCRICAO
  Sem argumento mostra todos os registradores e as flags. Com nome e valor
  altera: af bc de hl ix iy sp pc af' bc' de' hl' a f b c d e h l i r

EXEMPLO
  reg hl 0xC000

VER TAMBEM
  regs, jump)MAN");
    add("slots", "", "Depurador", "slots", "de que slot:subslot vem cada pagina de 16 KB",
        R"MAN(DESCRICAO
  Mostra, para as 4 paginas (0000h, 4000h, 8000h, C000h), o slot e subslot
  selecionados agora e se a regiao e' RAM ou ROM/vazia.

VER TAMBEM
  slotsmsx)MAN");
    add("events", "", "Depurador", "events on|off", "assina/cancela os eventos assincronos",
        "DESCRICAO\n  Liga/desliga o envio de eventos (! break, ! watch, ! until) NESTA conexao. O console e a TUI ligam sozinhos.\n\nVER TAMBEM\n  wait, break");
    add("dbgui", "", "Depurador", "dbgui open|show|key|char", "o painel Z80 Debugger como texto",
        R"MAN(DESCRICAO
  O painel visual do depurador (o mesmo da janela, tecla F10) devolvido como
  texto -- e' o que a TUI usa.
    dbgui open        abre (para a maquina, cursor no PC) e desenha
    dbgui show        so' desenha
    dbgui key <k>     enter up down left right g d m s j r c q b esc bs
    dbgui char <c>    um digito hexa para a pergunta do G
  Na TUI o painel abre sozinho quando um breakpoint dispara (Depurador >
  Painel). Teclas do painel: ENTER passo, setas movem o cursor, B
  breakpoint, M dump de memoria, G vai ao endereco, C continua, Q encerra o
  emulador, Esc fecha.)MAN");

    return v;
}

} // namespace man
