// fwMSX -- MAN: manual do Mamute (o monitor Z80 do fwMSX; comando "mon" / "mamute" e a opcao --mon). Texto sem acento (ASCII).
//
// O Mamute vem do Mamute Assembler do Paleobasic (monitor de memoria/disassembler/depurador no estilo dos monitores de linha dos anos 80,
// escrito em PureBasic), reescrito para o fwMSX sem janela separada. Estas paginas descrevem o que o monitor faz DE FATO nesta versao.
// Projeto e decisoes: doc/mamute-spec.md.
#include "man_internal.h"

namespace man {

namespace {

const char *const kMamute = R"MAN(DESCRICAO
  O Mamute e' o monitor Z80 do fwMSX: um prompt de linha onde cada comando mexe na memoria, disassembla,
  executa e salva programas, no estilo dos monitores dos anos 80 (MegaAssembler, SUPER-X). Funciona no console
  (CLI), na TUI e pela ponte de controle, sem janela separada. Todo endereco e' HEXADECIMAL.

COMO ABRIR
  mon                 entra no monitor (o prompt vira MON(off)> ou MON(on)>); BA ou QUIT volta ao console
  mon <linha>         executa UM comando do monitor sem entrar nele (mon d 4000,400f)
  fwmsx --mon         abre o console ja' dentro do monitor (--mon --attach <porta> liga a um emulador)
  !comando            dentro do monitor, roda um comando do CONSOLE (por exemplo !emu start)

DOIS MODOS
  OFFLINE   memoria e CPU PRIVADAS do monitor: 16 blocos de 16 KB (4 slots x 4 paginas). Serve para criar,
            depurar e rodar um programa sem tocar na maquina. E' o modo de quem abre o monitor sem emulador.
  ONLINE    a memoria e a CPU da MAQUINA emulada: o que se escreve aparece na hora no programa em execucao.
            Por exemplo, escreva uma rotina em C000h e, no BASIC da janela, DEFUSR=&HC000:A=USR(0).
  ONLINE e OFFLINE trocam de modo; PUSH e PULL levam um bloco de um lado para o outro. Ver man mamute-modes.

COMANDOS (todos em HEXA; man mamute-<grupo> detalha)
  memoria     PAGE DM M SH MS T F          man mamute-memory
  telas       DM M S ZAP SCR               man mamute-screens   (paineis na TUI, comandos de linha no console)
  despejo     C D P V                      man mamute-dump
  execucao    L LP X G                     man mamute-exec
  arquivos    LOAD SAVE                    man mamute-files
  fita        TAPE R TLOAD TSAVE (A OT)    man mamute-fita   (.cas .tsx .wav; online usa a fita da maquina)
  fonte       EDIT LIST A MAP              man mamute-edit   (editor de linhas numeradas, monta pelo fwasm)
  SUPER-X     XD XA XI XM XH XBT XRT XFL XCM XFD XTS XCS XSD XRG XGO XTR XPP XPI XPO XDK XFS XCI XTP XSV XLD XS# XL# XL% XS%
              XIM XIC XIL XIS XIR XCO  +  CL (calculadora) e @ (variaveis)      man mamute-superx
  modos       ONLINE OFFLINE STATUS PUSH PULL    man mamute-modes
  configurar  CFG                          man mamute-config
  sair        BA QUIT XQT

DIFERENCAS EM RELACAO AO MAMUTE DO PALEOBASIC
  Sem janelas separadas: na TUI, DM, M, S, ZAP e SCR sao PAINEIS (man mamute-screens); no console sao comandos de linha.
  P, LP e A P/H gravam PDF (Courier, A4) quando o nome termina em .pdf, senao texto. O EDIT/A monta pelo fwasm (nao ha' um
  segundo montador). Os comandos do SUPER-X (prefixo X) estao em man mamute-superx. O R (fita), que no original so' validava a sintaxe, agora carrega de verdade, e ha' TAPE, TLOAD e TSAVE (man mamute-fita).

VER TAMBEM
  mamute-modes, mamute-memory, mamute-dump, mamute-exec, mamute-edit, mamute-screens, mamute-superx, mamute-fita, mamute-files, mamute-config, dis, fwasm)MAN";

const char *const kEdit = R"MAN(EDIT / LIST / A / MAP -- o editor de fonte Z80 e a montagem

O Mamute guarda um programa-fonte Z80 de linhas NUMERADAS, no estilo do MegaAssembler e do ZX-81, e monta pelo fwasm (o montador do
fwMSX): cada linha e' traduzida para o asMSX, na mesma ordem, e os erros voltam com o numero da linha do Mamute.

COMO ENTRAR
  EDIT              entra no editor (o prompt vira EDIT>); QUIT volta ao monitor (o programa fica); BA sai de tudo
  EDIT <comando>    executa UM comando do editor sem entrar nele (edit list, edit a o)
  LIST, A, MAP      tambem funcionam direto no monitor

FORMATO DA LINHA
  NN [Label:] instrucao operandos [;comentario]
  NN e' o numero da linha, em DECIMAL (1 a 65535); digitar de novo o mesmo numero substitui a linha; so' o numero apaga.
  Instrucoes: qualquer mnemonico Z80 (com (HL), nao [HL]) e ORG, DEFB, DEFW, DEFM (texto 'entre apostrofos'), DEFS, EQU (exige Label:) e END.
  NUMEROS NO TEXTO: sem sufixo e' HEXADECIMAL (LD A,10 carrega 16). Sufixos: H hexa, B binario (1010B), D decimal (10D).
  Um numero que comeca com A-F precisa de um 0 na frente (0FFH), senao e' um rotulo.
  Rotulos com o nome de uma diretiva do fwasm (PRINT, START, SIZE...) sao aceitos: a traducao os renomeia por dentro.

COMANDOS DO EDITOR
  LIST [a[-[b]]]               lista (tudo, uma linha, um intervalo ou de a ate' o fim)
  NEW                          apaga o programa;   DELETE a[-[b]]   apaga uma linha, um intervalo ou ate' o fim
  RENUM [novo[,antigo[,passo]]]   renumera (sem argumentos: tudo, de 10 em 10); rotulos nao mudam
  CHANGE 'a'[,'b']             troca a por b em todas as linhas (sem b, apaga a)
  SEARCH|FIND 'x'              acha (maiusculas contam);  SEARCH x   acha sem distinguir maiusculas;  LSEARCH grava em mamute.srch
  SAVE arq | LOAD arq | MERGE arq   arquivo .mza (ASCII "NN texto"); MERGE funde (a linha do arquivo vence)

A -- MONTAR
  A [opcoes][/desloc][>nome]    mostra PASSO-1, PASSO-2 e a listagem: numero da linha, endereco (ou o valor do EQU), ate' 4 bytes e a linha;
                                ORG e END nao aparecem. Em caso de erro mostra a mensagem e a linha com o cursor >.
  Opcoes, coladas e em qualquer ordem:
    O   grava o codigo-objeto na memoria do monitor (no endereco do ORG; so' onde for RAM -- ONLINE escreve na maquina)
    N   sem a coluna do numero da linha
    P   tambem grava a listagem: NOME.pdf (PDF A4); com >NOME.lst ou >NOME.txt, texto
    I   grava o codigo-objeto em NOME.bin no formato BSAVE do MSX (FE inicio fim exec); LOAD NOME.bin carrega de volta
    R   anexa a referencia cruzada (nome, valor e os enderecos das linhas que usam o simbolo)
    S   anexa a lista alfabetica de simbolos;  D   a mesma em ordem de aparicao
    H   grava as listas S e/ou D: NOME_labels.pdf; com >NOME.lst ou >NOME.txt, NOME.sym em texto
    T   grava o codigo-objeto na FITA como arquivo binario: >NOME.cas, >NOME.tsx ou >NOME.wav cria essa fita; sem isso acrescenta na
        fita inserida (TAPE); o nome na fita e' o NOME (6 letras). R (TAPE) ou LOAD NOME.cas carregam de volta (man mamute-fita)
  /desloc  soma desloc (hexa) a TODO ORG;  >nome  nome-base de P, I e H (padrao: mamute)
MAP   inicio e fim do codigo da ultima montagem que deu certo (NEW e qualquer edicao a invalidam)

EXEMPLO
  edit
  10 ORG 0C100H
  20 CHPUT: EQU 0A2H
  30 LD HL,TEXTO
  40 LOOP: LD A,(HL)
  50 OR A
  60 RET Z
  70 CALL CHPUT
  80 INC HL
  90 JR LOOP
  100 TEXTO: DEFM 'MEGA ASSEMBLER'
  110 DEFB 0
  120 END
  a o                   monta e grava em C100; no BASIC: DEFUSR=&HC100:A=USR(0)  (com ONLINE; OFFLINE: G C100)

VER TAMBEM
  mamute, mamute-exec, fwasm, fwasm-syntax)MAN";

const char *const kScreens = R"MAN(DM / M / S / ZAP / SCR -- as telas do Mamute

No Mamute do Paleobasic eram janelas separadas. Aqui:
  * na TUI (fwmsx --tui) cada uma abre um PAINEL interativo, com as teclas de verdade;
  * no console (--cli), em scripts e pelo TCP sao comandos de LINHA: mostram a grade/tela uma vez e as mudancas vao por argumentos.

DM end[,desloc]      128 bytes (16 linhas de 8) em hexa e ASCII. DM sozinho continua 128 bytes depois do ultimo (o "proxima pagina" da linha).
M end[,b1,b2..]      sem bytes: a grade; com bytes: grava.            M C000,3E,2A,C9
S end[,teclas]       como o M, mas digitando com o teclado S (abaixo). S D010,1q  grava 15 (as teclas 1 e q sao os digitos 1 e 5).
ZAP arq,setor[,desloc]   a grade de um setor de uma imagem .dsk (512 bytes por setor; o rotulo da linha e' o deslocamento no setor).
ZAPW arq,setor,byte,b1..  grava bytes no setor direto no arquivo (so' linha).   ZAPW disco.dsk,1,10,DE,AD
SCR end,dx,dy[,modo[,zoom]]   a memoria como uma tela de 256x192 pixels (32x24 caracteres de 8 bytes), em braille (U+2800).
  dx,dy (hexa, >= 1) e' o AZULEJO: a tela e' ladrilhada com blocos de dx x dy caracteres; modo 0 le o azulejo por linha, 1 por coluna
  (a ordem real dos sprites do MSX). zoom 1 = 128x48 colunas (um ponto por pixel), 2 = 64x24 (cada ponto e' um bloco 2x2; padrao).

TECLAS DOS PAINEIS (TUI)
  DM e ZAP                          M e S
    setas      movem o cursor          setas, TAB, PgUp/PgDn, + e - iguais ao DM
    TAB        hexa <-> texto          dois digitos hexa gravam direto (o 1o fica como 3_) e o cursor avanca;
    PgUp/PgDn  pulam 128 bytes         o bloco de texto e' so' leitura;  ENTER sempre sai;  ESC cancela o digito
    + e -      deslocamento do texto
    ENTER      abre o campo de edicao; ENTER confirma, ESC cancela
               (hexa: 1 ou 2 digitos, o 1o digito troca o valor mostrado; texto: cada caractere vira (caractere - desloc))
    Ctrl+S     (ZAP) grava so' o setor do cursor; o titulo mostra * enquanto ha' alteracao; ESC com alteracao pede ESC de novo
    ESC        fecha (com um campo aberto, so' cancela o campo)
  O desloc (-7F a +80) nao muda a memoria: so' como o texto e' lido/escrito.
  Escrever em ROM ou celula vazia nao grava e o painel avisa. No ZAP todo byte da imagem e' editavel.

TECLADO S (padrao de mamute.cfg: skeys=1234qwerasdfzxcv)
    1 2 3 4        1 2 3 4
    Q W E R   =>   5 6 7 8        as 16 teclas valem os digitos 1 2 3 4 5 6 7 8 9 A B C D E F 0;
    A S D F        9 A B C        CFG SKEYS <16 caracteres> troca (CFG SAVE grava)
    Z X C V        D E F 0

SCR NA TUI
    esquerda/direita  mexem o endereco base 1 byte           cima/baixo  1 azulejo inteiro (dx x dy x 8 bytes)
    TAB   liga/desliga a moldura de 2x2 caracteres (16x16 pixels) no canto superior esquerdo
    E     mostra/oculta o endereco base            Z   alterna o zoom
    ENTER entra na edicao dos 16x16 pixels da moldura (ampliados abaixo da tela)
      setas movem o cursor de pixel, ESPACO inverte o pixel, I inverte tudo, L apaga tudo,
      ENTER sai (o que foi feito fica), ESC cancela TUDO desde que entrou;  celula que nao e' RAM: nada e' gravado
    ESC   fecha

VER TAMBEM
  mamute, mamute-memory, mamute-config)MAN";

const char *const kModes = R"MAN(ONLINE / OFFLINE / STATUS / PUSH / PULL

ONLINE        liga o modo online (a maquina emulada). Exige um emulador conectado ao console (emu start / emu attach).
OFFLINE       volta para a memoria e a CPU privadas do monitor.
STATUS        modo atual, slot de cada pagina, formato de despejo (C), tamanho da VRAM, PC e SP.

PUSH ini,fim[,dest]   copia um bloco da memoria OFFLINE para a MAQUINA (so' bytes que cabem em RAM)
PULL ini,fim[,dest]   copia um bloco da MAQUINA para a memoria OFFLINE
  O destino e' o proprio ini quando omitido. Os dois comandos so' existem com um emulador conectado.

COMO AS COISAS FUNCIONAM NOS DOIS MODOS
  Leitura (D, DM, L, SH...)   offline: memoria privada; online: o que o Z80 da maquina enxerga agora.
  Escrita (M, MS, F, T)       so' em RAM. Em ROM ou celula vazia a escrita e' ignorada, sem erro, e o log diz
                              quantos bytes foram ignorados -- como num MSX de verdade.
  PAGE                        offline: troca o slot de cada pagina. Online: o mapeamento e' o da maquina; PAGE ? mostra.
  X e G                       offline: a CPU privada (roda de verdade, ate' 5.000.000 instrucoes); online: a CPU da maquina.
  VRAM (V)                    offline: VRAM privada de 16/128/192 KB (CFG VRAM); online: a VRAM do VDP (so' leitura).

FLUXO TIPICO
  1. mon                         (modo offline)
  2. M C000,3E,2A,C9             (LD A,2A / RET)  ou  LOAD programa.bin
  3. L C000,C002                 conferir o disassembly
  4. G C000                      testar offline (X mostra os registradores)
  5. !emu start, ONLINE          liga a maquina
  6. PUSH C000,C002              leva o programa para a maquina
  7. no BASIC da janela: DEFUSR=&HC000:A=USR(0)

VER TAMBEM
  mamute, mamute-exec, mamute-config)MAN";

const char *const kMemory = R"MAN(PAGE / DM / M / SH / MS / T / F

PAGE            offline: poe as 4 paginas no primeiro slot com RAM (o estado de boot)
PAGE ?          mostra o slot ativo de cada pagina, com o tipo da celula (RAM, ROM, BASIC, VAZIO)
PAGE X,Y,Z,W    offline: pagina 0 no slot X, 1 no Y, 2 no Z, 3 no W (0-3, sempre os 4)

DM end[,desloc]   (DM sozinho continua 128 bytes depois; na TUI abre o painel: man mamute-screens)
  128 bytes (16 linhas de 8) em hexa e ASCII lado a lado, mais ENDERECO e DESLOC. O deslocamento (-7F a +80, hexa com
  sinal) nao muda a memoria: so' a leitura do texto, somando-o ao byte (para ler texto "cifrado" por um deslocamento fixo).

M end[,b1,b2,...]
  sem bytes, e' o DM (na TUI, o painel de edicao rapida); com bytes, grava os bytes a partir de end. M C000,3E,2A,C9
S end[,teclas]   como o M, com o teclado S (man mamute-screens);  ZAP / SCR: telas de disco e de pixels (man mamute-screens)

SH [end],b1,b2,...        procura bytes; um campo VAZIO e' curinga:  SH 4000,2A,,0C
SH [end],'texto           procura um texto (o apostrofo abre, nada fecha), em TODOS os deslocamentos de -7F a +80:
                          acha tanto o texto puro (DESLOC +00) quanto texto cifrado por um deslocamento
  Sem end (SH ,2A) continua do ultimo achado + 1. Resposta: ACHADO EM aaaa (DESLOC +dd) ou NAO ENCONTRADO
  depois de varrer os 64 KB dando a volta.

MS end,[desloc],'texto    grava o texto: cada caractere vira (codigo - desloc) & FF. MS 9A15,20,'nome
T ini,fim,dest            copia um bloco (a sobreposicao e' tratada como memmove)
F ini,fim,byte            preenche um bloco com um byte

VER TAMBEM
  mamute, mamute-dump, mamute-modes)MAN";

const char *const kDump = R"MAN(C / D / P / V

C modo   escolhe o formato dos comandos D, P e V (vale ate' CFG DUMP mudar o inicial):
  0   hexa + ASCII, 4 bytes por linha
  1   hexa + ASCII, 16 bytes por linha
  2   so' hexa, 8 bytes por linha, com um CHECKSUM no fim: soma dos 8 bytes + byte baixo do endereco, modulo 256
  3   igual ao 2, mas o checksum e' so' a soma dos bytes

D ini[,fim]           despejo no log. Sem fim, 16 bytes. O fim nao pode ser menor que o inicio nem passar de FFFF.
P ini[,fim][,arq]     igual ao D, mas grava um arquivo: PDF (A4, Courier) quando arq termina em .pdf (padrao mamute.pdf), senao texto
V ini[,fim][,arq]     igual ao D, mas da VRAM (enderecos de ate' 5 digitos; o limite e' o tamanho da VRAM); com arq, grava como o P

EXEMPLO
  C 2
  D C001,C008          C001  2A C9 00 00 00 00 00 00  F4

VER TAMBEM
  mamute, mamute-memory)MAN";

const char *const kExec = R"MAN(L / LP / X / G

L [ini[,fim]]
  Disassembla a memoria no log: endereco, ate' 4 bytes e o mnemonico (JR e DJNZ mostram o destino absoluto).
  Com ini e fim, ate' passar de fim; so' com ini, 10 instrucoes; sem nada, continua de onde o ultimo L/LP parou.
LP [ini[,fim]][,arq]  igual ao L, mas grava um arquivo: PDF quando arq termina em .pdf (padrao mamute-dis.pdf), senao texto.

X                     mostra AF BC DE HL IX IY SP PC e os alternados
X reg                 mostra um registrador (AF BC DE HL IX IY SP PC AF' BC' DE' HL' ou A F B C D E H L)
X reg valor           grava (16 bits com 4 digitos hexa, 8 bits com 2)

G [ini[,bp1[,bp2]]]
  Executa a partir de ini (sem ini, continua do PC) ate' um breakpoint (a execucao PARA ANTES da instrucao nele),
  um HALT ou o teto de 5.000.000 instrucoes. OFFLINE roda na CPU privada e mostra os registradores. ONLINE muda o
  PC da maquina, poe os breakpoints no depurador dela e solta (use stop/regs/dis no console para ver onde parou).

EXEMPLO (offline)
  M C100,06,05,05,20,FD,3E,77,76     ; LD B,5 / DEC B / JR NZ,-3 / LD A,77 / HALT
  G C100                             ; PARADO EM HALT ... A=77 B=00

VER TAMBEM
  mamute, mamute-modes, mamute-superx (XRG, XGO, XTR: a versao com breakpoints nomeados e trace), dis, regs, go)MAN";

const char *const kFiles = R"MAN(LOAD / SAVE

LOAD arq[,slot[,end]]
  Carrega um arquivo na memoria do SLOT escolhido (0-3), direto, sem passar pelo PAGE.
  .rom        vai para 4000 (ate' 32 KB); a celula vira ROM (offline)
  FE...       binario com cabecalho BLOAD: carrega no endereco do cabecalho
  outro       sem cabecalho: informe o endereco (LOAD arq,3,C000)
  .cas .tsx .tzx .wav   fita: carrega o primeiro arquivo BINARIO dela no endereco do cabecalho (man mamute-fita)
  Sem slot: o primeiro slot com RAM (offline) ou o slot ativo da pagina 1 (online). Resposta:
  CARREGADO NO SLOT s EM aaaa - TAMANHO tttt - FIM ffff.

SAVE arq,ini,fim[,exec[,BIN|ROM|RAW]]
  Grava a memoria que o Z80 enxerga (ini..fim, inclusive).
  BIN   cabecalho BSAVE real: FE + ini + fim + exec (2 bytes cada, exec = ini se omitido)  [padrao]
  ROM   igual, com AB no lugar do FE (formato do Mamute original, nao e' o cabecalho de um cartucho)
  RAW   so' os bytes  [padrao quando o arquivo termina em .rom: uma imagem de cartucho de verdade]
  Se o arquivo termina em .cas, .tsx, .tzx ou .wav, cria uma FITA com um arquivo binario (o nome de 6 letras sai do nome do arquivo).

VER TAMBEM
  mamute, mamute-memory, mamute-fita)MAN";

const char *const kConfig = R"MAN(CFG e o arquivo mamute.cfg

O arquivo mamute.cfg (ao lado do executavel, texto chave=valor) e' lido quando o monitor abre. Sem ele valem os padroes
(offline, VRAM de 16 KB, formato 0, slot 3 todo RAM).

CHAVES
  online=0|1                  abre online (so' vale com um emulador conectado)
  vram=16|128|192             VRAM da memoria offline, em KB
  dump=0..3                   formato inicial do comando C
  cell.S.P=vazio|ram|rom|basic   tipo da celula do slot S (0-3), pagina P (0-3)
  file.S.P=caminho            arquivo carregado na celula (rom e basic)
  half.S.P=1                  usa os ULTIMOS 16 KB do arquivo (BIOS+BASIC de 32 KB repartidos em duas celulas)
  skeys=1234qwerasdfzxcv      o teclado S (16 caracteres = os digitos 1 2 3 4 5 6 7 8 9 A B C D E F 0); CFG SKEYS muda

COMANDOS
  CFG                                 mostra a configuracao e a tabela das 16 celulas
  CFG SET slot,pagina,tipo[,arq[,HALF]]   muda uma celula (recarrega a memoria OFFLINE: o conteudo e' perdido)
  CFG VRAM n | CFG ONLINE 0|1 | CFG DUMP n
  CFG SAVE                            grava o mamute.cfg
  CFG LOAD                            le o mamute.cfg de novo

EXEMPLO (um MSX1: BIOS e BASIC num arquivo de 32 KB, RAM no slot 3)
  CFG SET 0,0,ROM,MSX.ROM
  CFG SET 0,1,BASIC,MSX.ROM,HALF
  CFG SET 3,0,RAM
  CFG SET 3,1,RAM
  CFG SET 3,2,RAM
  CFG SET 3,3,RAM
  CFG SAVE

VER TAMBEM
  mamute, mamute-modes)MAN";

const char *const kSuperX = R"MAN(OS COMANDOS DO SUPER-X (prefixo X), CL e @

O SUPER-X era um monitor japones de MSX. O Mamute do Paleobasic portou os comandos dele com o prefixo X (para nao colidir com os do
MegaAssembler). No fwMSX cada um que era uma janela vira um PAINEL na TUI e um comando de LINHA no console. Enderecos em HEXA.

ENDERECAMENTO ESTENDIDO -- depois do endereco, um sufixo escolhe ONDE ler e gravar, sem olhar o PAGE:
  4000#3      o slot primario 3 (mesmo que nao esteja em pagina nenhuma agora); online, o sub-slot que a CPU enxerga naquele slot
  4000#3-1    o sub-slot 1 do slot 3 (slot expandido; offline sao 64 KB de RAM por sub-slot, ate' 1 MB)
  00000#V     a VRAM, plana, com ate' 5 digitos (#4 e' sinonimo);   4000#S   o PAGE corrente de proposito (#5)
  O <fim> de um bloco e' sempre um endereco PURO, no MESMO alvo do inicio. Ninguem da' a volta: passar do teto do alvo e' erro.
  Escrever so' funciona onde e' RAM (sub-slots offline 1-3 e a VRAM sempre aceitam; ROM e celula vazia ignoram).

VARIAVEIS DE DEBUGGER -- @0 a @3 e @B @E @S guardam um endereco COM o alvo e valem pelo endereco inteiro:
  @                 mostra as 7        @0=8000#3-1   define        XD @0   usa
  XD, XA, XI, XM e XH gravam o endereco aberto em @0. XLD, XL# e XL% preenchem @B (inicio), @E (fim) e @S (execucao ou tamanho).

CL <expressao>   calculadora de 16 bits (HEX, BIN, DEC+ e DEC+-). Numeros em HEXA; sufixos D B H O (10D = decimal 10; 10DH = hexa);
                 operadores + - * / % | & ^ ! e parenteses (do mais apertado: unarios, * / %, + -, &, ^, |); @0 vale so' o numero;
                 'A' e "AB" sao literais ASCII.   CL (100H+2ADH)*3-1

TELAS DE MEMORIA (UM endereco = a tela; sem endereco reabre onde ficou)
  XD [end]            128 bytes em hexa+ASCII; editavel nos dois blocos. Teclas: as do M (dois digitos hexa gravam), " liga a digitacao
                      ASCII direta (ESC sai), @ repete o byte anterior, + e - o deslocamento do texto, PgUp/PgDn 128 bytes.
  XA [end]            256 bytes so' em ASCII (16x16): qualquer tecla imprimivel grava o byte cru e avanca.
  XI [end]            o disassembly rolando (so' leitura): cima/baixo 1 byte, PgDn a proxima tela, PgUp volta a largura da tela.
  XH [end]            4 caracteres/sprites 8x8 (32 bytes), 2 em cima e 2 embaixo, pixel a pixel: setas, ESPACO inverte o pixel,
                      I inverte o caractere, L apaga, F preenche, B apaga os 4, PgUp/PgDn 32 bytes; mostra a miniatura em braille.
  XM end[#alvo]       abre uma SESSAO (o prompt vira XM 4000>): cada linha e' uma instrucao Z80 (montada pelo fwasm e gravada), dados
                      (.cd,4dh = bytes  :1234 = palavras  ;expr = palavras numericas  [expr = bytes numericos  "texto cru), um endereco
                      (o alvo continua o mesmo; so' #S volta ao PAGE) ou I [n] (lista n instrucoes). O endereco avanca sozinho.
                      QUIT volta ao monitor; BA e XQT saem de tudo. DAA e CCF sozinhos sao a instrucao, nao o endereco.
  No console XD, XA, XI e XH mostram a tela uma vez. Com DOIS ou TRES campos viram listagens:
  XD ini,fim          8 bytes por linha:  AAAA XX XX .. : YY : ASCII   (YY = checksum de 8 bits; XCS escolhe NORMAL ou +ADDR)
  XA ini,fim          64 caracteres por linha        XI ini,fim   disassembly, uma instrucao por linha
  sem fim             256 bytes (ate' o teto do alvo):  XD 4000,   ou   XD 4000,,arq   ou   XD 4000,arq
  ini,fim,arquivo     XD e XA gravam os bytes crus; XI grava a listagem em texto
  ?XD ini,fim[,arq]   o "?" manda a listagem para a "impressora": um PDF (Courier, A4) quando o nome termina em .pdf;  vale para XD, XA, XI

BLOCOS
  XBT ini,fim,dest    copia entre alvos QUAISQUER (slot, sub-slot, VRAM, PAGE); le tudo antes de gravar, entao pode se sobrepor
  XRT ini,fim,dest    copia relocando: cada JP/CALL/LD com endereco absoluto que aponta para DENTRO do bloco anda junto (CALL 8012H vira
                      CALL C012H ao mover 8000H para C000H; CALL 004DH, fora do bloco, fica); JR/DJNZ para FORA do bloco sao recalculados e
                      se nao couberem em -128..127 nada e' gravado (RELOCACAO INVALIDA). Dados no meio do codigo podem confundir.
  XFL ini,fim,byte    preenche
  XCM ini,fim,comp[,S]   compara com o bloco de mesmo tamanho em comp: lista os bytes DIFERENTES (,S lista os IGUAIS) e a contagem
  XFD ini,fim,instr   acha uma INSTRUCAO Z80 (monta instr e procura nos limites de instrucao, nao em bytes soltos):  XFD 4000,7FFF,CALL 8012H
                      JR e DJNZ so' casam na mesma distancia relativa.
  XTS ini,fim         UM checksum de 16 bits do bloco inteiro, em HEX, BIN, DEC+, DEC+- e OCT
  XCS                 alterna o checksum por linha do XD entre NORMAL (so' os bytes) e +ADDR (soma o byte baixo do endereco)
  XSD arq,ini,fim[,B|D|X]   grava uma listagem assembly (com ORG), ou os bytes crus como DEFB, DATA do BASIC (com o laco FOR/READ/POKE)
                      ou dados do X-BASIC. O modo D nao serve para a VRAM.

REGISTRADORES, EXECUCAO E PORTAS (a CPU do modo atual: privada offline, a da maquina online)
  XRG                 AF BC DE HL / IX IY SP / AF' BC' DE' HL' / PC / os 5 breakpoints BP BP1 BP2 BP3 BPF
  XRG *               limpa tudo, menos a pilha        XRG +   SP=0000       XRG reg,valor   edita (A..L, A'..L', pares, IXH IXL IYH IYL, SP, PC)
  XRG BP,4000         marca um breakpoint nomeado (XRG BP,- limpa)
  XGO end[#slot]      executa a partir de end (reinicia o HALT). Para no BP; XGO sem endereco continua e para no BP1, depois BP2, BP3 e BPF
                      (o que nao existir cai no BPF). Sem breakpoint roda "livre": para no RET que devolve alem da pilha de entrada, no HALT
                      ou em 2000000 instrucoes. #slot troca o slot da pagina (como o PAGE); VRAM e sub-slot nao: a CPU executa a memoria
                      mapeada. Online, solta a CPU da maquina (o depurador cuida do resto).
  XTR [end][,n]       trace: executa n instrucoes (1) e mostra endereco, bytes, mnemonico e os registradores; sem end continua do PC.
                      Na TUI abre o painel: ENTER executa a proxima, ESC interrompe.
  XPP                 o painel de portas de E/S monitoradas: ENTRADA (o ultimo byte de um OUT) e SAIDA (o que um IN le); * marca as que sofreram OUT.
                      Painel: A adiciona, D remove, S define a SAIDA, E a ENTRADA, C limpa as marcas. Nao ha' hardware simulado atras:
                      quem decide o que o IN le e' voce (o padrao e' FF).
  XPI porta           le a SAIDA da porta (e a cria no painel)       XPO porta,byte   grava a ENTRADA e marca a porta

DISCO -- o "disco corrente" e' uma imagem .dsk (FAT12 do MSX-DOS)
  XDK arq.dsk         escolhe o disco corrente (XDK sozinho mostra qual e'). Os outros comandos de disco nao aceitam nome de disco.
  XFS                 lista o diretorio          XCI   clusters livres / total
  XTP arquivo         mostra um arquivo DE DENTRO do disco como texto (para no 1A, expande TAB). TUI: painel com rolagem e busca
                      (/ busca, c liga Case, r liga expressao regular, n proxima)
  XL% ini[,fim],end   le setor(es) (em hexa, 512 bytes cada) DIRETO para a memoria        XS%   o inverso (grava no disco)
  Arquivos do COMPUTADOR (nao do disco):
  XSV arq,ini,fim[,exec[,desloc]]   grava com o cabecalho do BSAVE (FE ini fim exec); desloc soma so' nos enderecos do cabecalho; nao salva VRAM
  XLD arq[,desloc[#alvo]]           carrega um arquivo do BSAVE (no endereco do cabecalho, ou em desloc); NAO executa (use XGO)
  XS# arq,ini,fim                   grava so' os bytes (aceita VRAM)        XL# arq,ini   carrega o arquivo inteiro

NOTAS -- um arquivo de texto "AAAA;slot;tipo;texto" que documenta enderecos (o SUPER-X-PT.notas traz 471 da BIOS, do WORK e das portas)
  XIL [arq]           carrega (sem argumento, o SUPER-X-PT.notas ao lado do executavel); troca TODAS as notas em memoria
  XIC end             mostra as notas daquele endereco (pode haver mais de uma)       XIM end,slot,tipo,texto   adiciona uma nota
  XIS arq             grava as notas em memoria
  XIR [end]           TUI: o painel das notas, uma por vez, com busca (/ c r n). Console: lista as notas (XIR texto filtra)
  slot: 0 GERAL 1 MAIN 2 SUB 3 FDC 4 RAM    tipo: 0 GERAL 1 BIOS 2 WORK 3 DATA 4 PORT 5 MATH 6 KEY 7 HOOK

XCO [f],[b],[r]     cores do Mamute original (indices 0-15, DECIMAL, vazio mantem); fica no mamute.cfg. Aqui nao ha' janela para pintar:
                    a escolha so' e' guardada e mostrada.

DIFERENCAS EM RELACAO AO PALEOBASIC
  Sem janela de arquivo: XFS, XCI, XTP, XL% e XS% pedem o XDK; os nomes de arquivo vao no proprio comando. XFD pede a instrucao no comando.
  Alem do que o original fazia: os alvos valem tambem em XI, XSD, XFD e XBT; online, #3 e o sub-slot que a CPU enxerga e #V escreve na
  VRAM da maquina; XRG BP,- limpa um breakpoint; ?XD aceita um nome de arquivo. Ainda nao existem: a "cruz" de troca entre XD/XA/XI/XM/XH
  (cada um e' aberto pelo seu comando) e o CLS.

VER TAMBEM
  mamute, mamute-memory, mamute-exec, mamute-screens, mamute-files, mamute-config, dis, fwasm)MAN";

const char *const kFita = R"MAN(FITA -- TAPE, R, TLOAD, TSAVE e o LOAD/SAVE de .cas, .tsx e .wav

DESCRICAO
  O Mamute le e grava fitas cassete nos tres formatos do fwMSX, escolhidos pela EXTENSAO: .cas (imagem crua), .tsx/.tzx (blocos #4B do
  Kansas City Standard) e .wav (audio de verdade, 44100 Hz, PCM mono: a leitura passa pelo "ripper", a gravacao sintetiza os pulsos).
  Em ONLINE o toca-fitas e' o da MAQUINA EMULADA: a mesma fita que o CLOAD/CSAVE/BLOAD "CAS:" do MSX usam. O que o Mamute grava, o BASIC
  enxerga na hora, e a posicao e' uma so'. Em OFFLINE o monitor tem um toca-fitas PROPRIO (sem Z80 nem BIOS). O TAPE (sem argumento) diz qual.

TAPE [arquivo | subcomando]
  TAPE                       estado: o arquivo, quantos arquivos logicos, protegida ou gravavel, modo de gravacao e o ponto
  TAPE arq.cas|.tsx|.wav     insere a fita (entra PROTEGIDA contra gravacao); TAPE INSERT arq serve para nomes iguais a comandos
  TAPE NEW arq               cria uma fita vazia e GRAVAVEL (o arquivo ja' nasce no disco)
  TAPE LIST                  um arquivo por linha: numero, tipo (BIN/BAS/ASC), nome, inicio-fim, exec e tamanho; ">" marca o ponto
  TAPE EJECT | TAPE REW      tira a fita / volta ao inicio
  TAPE SEEK n|nome|-         marca o PONTO (de onde o R e o carregamento da maquina continuam); "-" volta ao inicio
  TAPE PROT ON|OFF           protege ou libera a gravacao (uma fita de arquivo entra protegida; uma NOVA, liberada)
  TAPE WRITE APPEND|OVER|NEW o que a PROXIMA gravacao faz: acrescenta no fim, corta no ponto marcado (SEEK), ou apaga tudo. Vale
                             para UMA gravacao; depois volta para APPEND (fita de verdade e' linear)
  TAPE SAVE arq              grava uma COPIA da fita em outro arquivo (a extensao converte: .cas -> .tsx -> .wav)
  TAPE MODE FAST|NORMAL      so' ONLINE: como a maquina le a fita (gancho de BIOS rapido, ou pulsos de verdade, com som)

R [desloc]
  Carrega o PROXIMO arquivo BINARIO da fita (o que o BLOAD "CAS:" carregaria) no endereco do cabecalho dele, mais o deslocamento
  (hexa) se houver, e mostra o EXEC. Cada R avanca para o arquivo seguinte; na maquina o ponto tambem avanca. Fim da fita avisa.
  No original o R so' validava a sintaxe; aqui ele de fato carrega.

TLOAD n|nome[,slot[,end]]
  Carrega o arquivo n (numero do TAPE LIST) ou de nome dado. Binario vai para o endereco do cabecalho; com <end> vai so' o programa,
  nesse endereco. BASIC (BAS) e texto (ASC) so' com <end>, porque nao tem endereco proprio.

TSAVE nome,ini,fim[,exec]
  Acrescenta o bloco ini-fim da memoria na fita inserida como arquivo binario (cabecalho com o nome de ate' 6 letras e os dados,
  os mesmos dois blocos de um BSAVE "CAS:"). Respeita a protecao e o TAPE WRITE.

LOAD e SAVE por extensao
  LOAD arq.cas[,slot[,end]]  abre o arquivo de fita (.cas, .tsx, .tzx ou .wav) SEM inseri-lo e carrega o primeiro arquivo binario dele
  SAVE arq.tsx,ini,fim[,exec] cria um arquivo de fita com UM arquivo binario (o nome de 6 letras sai do nome do arquivo)

A com a opcao T (montar para a fita)
  A OT>jogo.cas     monta, grava na memoria e cria jogo.cas com o programa (tambem .tsx e .wav)
  A T               acrescenta o programa montado na fita inserida (o nome e' o do >nome, padrao MAMUTE)

EXEMPLOS
  TAPE NEW jogo.tsx            fita nova
  M C000,3E,2A,C9
  TSAVE JOGO,C000,C002,C000    grava JOGO na fita
  TAPE REW
  R                            carrega JOGO de volta (e mostra EXEC C000)
  TAPE SAVE jogo.wav           a mesma fita como audio, para um MSX de verdade

LIMITES
  So' o bloco KCS do MSX (pilot + bytes). Do .tsx entram os blocos #4B; ASCII usa varios blocos de dados concatenados. O .wav precisa
  de um sinal razoavel (o ripper usa um limiar adaptativo). Gravar numa fita inserida de um .wav REGRAVA o .wav sintetizado (perde o
  audio original): proteja-a (TAPE PROT ON), que e' o padrao.

VER TAMBEM
  mamute, mamute-files, mamute-edit, tape (o comando do emulador), cas (empacotar e "ripar"), fwasm-formats)MAN";

} // namespace

std::vector<Topic> MamuteTopics() {
    std::vector<Topic> v;
    // categoria "Mamute"; o primeiro e' o comando que entra na tabela do help
    v.push_back(Topic{"mamute", "mon,monitor,mamute-manual", "Mamute", "mon [linha]", "monitor Z80: memoria, disassembler e depurador, online ou offline", kMamute, true});
    auto add = [&](const char *name, const char *summary, const char *page) { v.push_back(Topic{name, "", "Mamute", "", summary, page, false}); };
    add("mamute-modes", "ONLINE, OFFLINE, STATUS, PUSH e PULL: a maquina emulada e a memoria privada", kModes);
    add("mamute-memory", "PAGE, DM, M, SH, MS, T, F: ver e editar a memoria", kMemory);
    add("mamute-dump", "C, D, P, V: despejo formatado (4/16 bytes, checksum)", kDump);
    add("mamute-exec", "L, LP, X, G: disassembler, registradores e execucao", kExec);
    add("mamute-edit", "EDIT, LIST, A, MAP: editor de fonte Z80 (linhas numeradas) e montagem pelo fwasm", kEdit);
    add("mamute-screens", "DM, M, S, ZAP, SCR: paineis interativos na TUI e comandos de linha no console", kScreens);
    add("mamute-superx", "XD, XM, XBT, XRT, XRG, XGO, XDK, XIM, XPP... (SUPER-X), CL e as variaveis @: o prefixo X", kSuperX);
    add("mamute-fita", "TAPE, R, TLOAD, TSAVE: fita cassete (.cas, .tsx, .wav), online na maquina ou offline", kFita);
    add("mamute-files", "LOAD e SAVE: arquivos na memoria", kFiles);
    add("mamute-config", "CFG e o arquivo mamute.cfg: configuracao inicial", kConfig);
    return v;
}

} // namespace man
