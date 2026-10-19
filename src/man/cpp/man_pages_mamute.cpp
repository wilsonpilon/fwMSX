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
  fonte       EDIT LIST A MAP              man mamute-edit   (editor de linhas numeradas, monta pelo fwasm)
  modos       ONLINE OFFLINE STATUS PUSH PULL    man mamute-modes
  configurar  CFG                          man mamute-config
  sair        BA QUIT XQT

DIFERENCAS EM RELACAO AO MAMUTE DO PALEOBASIC
  Sem janelas separadas: na TUI, DM, M, S, ZAP e SCR sao PAINEIS (man mamute-screens); no console sao comandos de linha.
  P, LP e A P/H gravam PDF (Courier, A4) quando o nome termina em .pdf, senao texto. O EDIT/A monta pelo fwasm (nao ha' um
  segundo montador). Ainda nao existem: R (fita) e os comandos do SUPER-X (prefixo X). Entram nas proximas versoes (doc/mamute-spec.md).

VER TAMBEM
  mamute-modes, mamute-memory, mamute-dump, mamute-exec, mamute-edit, mamute-screens, mamute-files, mamute-config, dis, fwasm)MAN";

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
  mamute, mamute-modes, dis, regs, go)MAN";

const char *const kFiles = R"MAN(LOAD / SAVE

LOAD arq[,slot[,end]]
  Carrega um arquivo na memoria do SLOT escolhido (0-3), direto, sem passar pelo PAGE.
  .rom        vai para 4000 (ate' 32 KB); a celula vira ROM (offline)
  FE...       binario com cabecalho BLOAD: carrega no endereco do cabecalho
  outro       sem cabecalho: informe o endereco (LOAD arq,3,C000)
  .cas        ainda nao e' suportado
  Sem slot: o primeiro slot com RAM (offline) ou o slot ativo da pagina 1 (online). Resposta:
  CARREGADO NO SLOT s EM aaaa - TAMANHO tttt - FIM ffff.

SAVE arq,ini,fim[,exec[,BIN|ROM|RAW]]
  Grava a memoria que o Z80 enxerga (ini..fim, inclusive).
  BIN   cabecalho BSAVE real: FE + ini + fim + exec (2 bytes cada, exec = ini se omitido)  [padrao]
  ROM   igual, com AB no lugar do FE (formato do Mamute original, nao e' o cabecalho de um cartucho)
  RAW   so' os bytes  [padrao quando o arquivo termina em .rom: uma imagem de cartucho de verdade]

VER TAMBEM
  mamute, mamute-memory)MAN";

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
    add("mamute-files", "LOAD e SAVE: arquivos na memoria", kFiles);
    add("mamute-config", "CFG e o arquivo mamute.cfg: configuracao inicial", kConfig);
    return v;
}

} // namespace man
