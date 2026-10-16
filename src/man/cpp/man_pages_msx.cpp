// fwMSX -- MAN/HELP: paginas de REFERENCIA do MSX (BIOS, variaveis, ganchos, BASIC, cores, portas...). Texto sem acento.
// As tabelas (BIOS, variaveis, ganchos) saem dos dados de man_pages_msx_data.cpp; a matriz do teclado sai das tabelas do
// proprio emulador (ppi_state), entao nao pode divergir delas.
#include <cstdio>

#include "../../ppi/core/ppi_state.h"
#include "man_internal.h"

namespace man {

namespace {

std::string Hex4(unsigned v) {
    char b[8];
    std::snprintf(b, sizeof(b), "%04X", v & 0xFFFF);
    return b;
}

// Uma linha de tabela: `prefix` + texto, com o texto quebrado e alinhado depois do prefixo (ate' 78 colunas).
void Row(std::string &out, const std::string &prefix, const std::string &text) {
    const std::vector<std::string> lines = Wrap(text, 78 - static_cast<int>(prefix.size()));
    for (size_t i = 0; i < lines.size(); ++i) out += (i == 0 ? prefix : std::string(prefix.size(), ' ')) + lines[i] + "\n";
}

std::string Pad(const std::string &s, size_t n) { return s.size() >= n ? s + " " : s + std::string(n - s.size(), ' '); }

std::string BiosPage() {
    std::string out =
        "DESCRICAO\n"
        "  A BIOS do MSX fica na ROM do slot 0 e oferece rotinas fixas a partir\n"
        "  do endereco 0000h. Cada entrada abaixo e' um JP para a rotina de\n"
        "  verdade: o ENDERECO nunca muda entre MSX1, MSX2, MSX2+ e turbo R.\n"
        "  Chame com CALL <endereco> com a BIOS selecionada na pagina 0 (no BASIC\n"
        "  e no MSX-DOS ela esta'). De uma pagina onde a BIOS nao esta', use\n"
        "  CALSLT (IY = slot da BIOS no byte alto, IX = endereco).\n"
        "  Os registradores de entrada e saida estao em cada linha (A, HL, BC...);\n"
        "  a maioria destroi AF, BC, DE e HL.\n\n"
        "EXEMPLO (escreve \"OI\" na tela e espera uma tecla)\n"
        "      LD   A,'O'\n"
        "      CALL 00A2h        ; CHPUT\n"
        "      LD   A,'I'\n"
        "      CALL 00A2h\n"
        "      CALL 009Fh        ; CHGET\n\n"
        "ENTRADAS (0000h-0038h: RST e rotinas de slot; depois a tabela de saltos)\n";
    for (const MsxEntry &e : BiosEntries()) {
        if (e.flags & kMsx2) continue;
        Row(out, "  " + Hex4(e.addr) + "  " + Pad(e.name, 7), e.text);
    }
    out += "\nMSX2 EM DIANTE (BIOS principal; as rotinas moram na SUB-ROM;\nRDBTST/WRBTST so' no MSX2+)\n";
    for (const MsxEntry &e : BiosEntries()) {
        if (!(e.flags & kMsx2)) continue;
        Row(out, "  " + Hex4(e.addr) + "  " + Pad(e.name, 7), e.text);
    }
    out += "\nVER TAMBEM\n  sysvars, hooks, slotsmsx, peek, dis";
    return out;
}

std::string SysVarsPage() {
    std::string out =
        "DESCRICAO\n"
        "  Area de trabalho do sistema, no fim da RAM (F380h a FFFFh num MSX com\n"
        "  64 KB). A BIOS e o BASIC guardam aqui o estado do sistema. Os nomes sao\n"
        "  os tradicionais (MSX Red Book / Technical Handbook). Valores de 2 bytes\n"
        "  ficam em ordem little-endian (baixo primeiro). Veja um valor com\n"
        "  \"peek <endereco> [n]\" e mude com \"poke\".\n\n"
        "EXEMPLOS\n"
        "  peek 0xFCAF            SCRMOD: modo de tela atual\n"
        "  peek 0xFC9E 2          JIFFY: contador de interrupcoes (TIME no BASIC)\n"
        "  poke 0xF3E9 7          muda a cor da frente (vale depois de um CLS)\n\n"
        "VARIAVEIS (endereco, nome, tamanho em bytes)\n";
    for (const MsxEntry &e : SysVarEntries()) {
        char size[8];
        std::snprintf(size, sizeof(size), "%3u", e.size);
        Row(out, "  " + Hex4(e.addr) + "  " + Pad(e.name, 7) + size + "  ", e.text);
    }
    out += "\nVER TAMBEM\n  hooks, bios, peek, poke";
    return out;
}

std::string HooksPage() {
    std::string out =
        "DESCRICAO\n"
        "  Ganchos (hooks) sao areas de 5 bytes na RAM, de FD9Ah a FFC9h, que a\n"
        "  BIOS e o BASIC CHAMAM em pontos conhecidos. Normalmente contem RET (C9h)\n"
        "  e 4 bytes de enchimento. Para interceptar, um programa troca os 5 bytes\n"
        "  por \"JP seu_codigo\" (C3h, endereco) -- e antes COPIA os 5 bytes antigos\n"
        "  e chama-os no fim, para nao quebrar quem ja' usava o gancho (encadear).\n"
        "  Com disco, o MSX-DOS e as interfaces tambem usam esses ganchos.\n\n"
        "EXEMPLO (uma rotina chamada 60 vezes por segundo)\n"
        "      DI\n"
        "      LD   HL,0FD9Fh        ; H.TIMI\n"
        "      LD   DE,antigo\n"
        "      LD   BC,5\n"
        "      LDIR                  ; guarda os 5 bytes antigos\n"
        "      LD   A,0C3h           ; JP\n"
        "      LD   (0FD9Fh),A\n"
        "      LD   HL,minha_rotina\n"
        "      LD   (0FDA0h),HL\n"
        "      EI\n"
        "  minha_rotina:\n"
        "      ...                   ; seu codigo (curto!)\n"
        "      JP   antigo           ; os 5 bytes guardados (terminam em RET)\n\n"
        "GANCHOS (endereco, nome, quando e' chamado)\n";
    for (const MsxEntry &e : HookEntries()) Row(out, "  " + Hex4(e.addr) + "  " + Pad(e.name, 8), e.text);
    out += "\nVER TAMBEM\n  sysvars, bios, watch (descobre quem usa um gancho)";
    return out;
}

std::string KeyboardPage() {
    std::string out =
        "DESCRICAO\n"
        "  O teclado do MSX e' uma matriz de 11 linhas (0 a 10) por 8 colunas\n"
        "  (bit 0 a 7). O programa escolhe a linha nos 4 bits baixos da porta AAh\n"
        "  e le a coluna na porta A9h (bit em 0 = tecla apertada). A BIOS faz\n"
        "  isso na interrupcao; a rotina SNSMAT (0141h) le uma linha para quem\n"
        "  precisa. A tabela sai das tabelas do PROPRIO emulador (os nomes sao\n"
        "  os de \"type {tecla}\").\n\n"
        "LINHA  bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0\n";
    for (int row = 0; row <= 10; ++row) {
        std::string names[8];
        for (int id = 0; id < ppi_key_count(); ++id) {
            int r = 0;
            uint8_t mask = 0;
            if (!ppi_key_position(id, &r, &mask) || r != row) continue;
            for (int b = 0; b < 8; ++b)
                if (mask == (1u << b)) names[b] = ppi_key_name(id);
        }
        char head[16];
        std::snprintf(head, sizeof(head), "%4d   ", row);
        out += head;
        for (int b = 7; b >= 0; --b) out += Pad(names[b].empty() ? "-" : names[b], 7);
        while (!out.empty() && out.back() == ' ') out.pop_back();
        out += "\n";
    }
    out += "\nNOTAS\n"
           "  Os nomes seguem o teclado internacional. No japones as mesmas posicoes\n"
           "  trazem outras letras (a BIOS do MSX2+ do fMSX e' japonesa; o \"type\"\n"
           "  cuida disso sozinho). \"-\" = posicao sem tecla.\n"
           "\nVER TAMBEM\n  type, portas, bios";
    return out;
}

const char *const kColorPage = R"MAN(DESCRICAO
  Os 16 codigos de cor do MSX (COLOR a,b,c e os registradores do VDP). No
  MSX1 (TMS9918) as cores sao fixas; no MSX2 em diante cada uma das 16
  posicoes e' uma entrada da paleta programavel (COLOR=(n,r,g,b), com r,g,b
  de 0 a 7), inicializada com valores parecidos. RGB abaixo: aproximacao
  do TMS9918.

   0  transparente      #000000        8  vermelho medio    #FC5554
   1  preto             #000000        9  vermelho claro    #FF7978
   2  verde medio       #21C842       10  amarelo escuro    #D4C154
   3  verde claro       #5EDC78       11  amarelo claro     #E6CE80
   4  azul escuro       #5455ED       12  verde escuro      #21B03B
   5  azul claro        #7D76FC       13  magenta           #C95BBA
   6  vermelho escuro   #D4524D       14  cinza             #CCCCCC
   7  ciano             #42EBF5       15  branco            #FFFFFF

USO
  BASIC:    COLOR 15,4,4   letras brancas, fundo e borda azul escuro (padrao)
  Variaveis do sistema: FORCLR F3E9h (frente), BAKCLR F3EAh (fundo) e
            BDRCLR F3EBh (borda)
  VDP:      o registrador 7 = (frente << 4) OR fundo; o byte alto e' a
            cor do texto
  Cor 0:    transparente (deixa ver o fundo); os sprites usam 0 como
            "sem pixel"

EXEMPLOS
  type "color 10,1,1" {enter}      amarelo sobre preto
  poke 0xF3E9 10                   muda FORCLR (vale depois de um CLS)

VER TAMBEM
  sysvars, vdpregs, screens)MAN";

const char *const kPortsPage = R"MAN(DESCRICAO
  Portas de E/S (instrucoes IN/OUT do Z80) do MSX e dos acessorios que o
  fwMSX emula.

  porta    nome            funcao
  7Ch      OPLL endereco   MSX-MUSIC (YM2413): escolhe o registrador
  7Dh      OPLL dados      MSX-MUSIC: valor do registrador
  90h      impressora      leitura = estado (bit 1 = ocupada); escrita = strobe
  91h      impressora      dados
  98h      VDP dados       le/grava a VRAM no endereco corrente (auto-incr.)
  99h      VDP comando     escrita: 2 bytes = registrador (bit 7 do 2o = 1) ou
                           endereco; leitura = status
  9Ah      VDP paleta      (MSX2) escreve 2 bytes: R<<4|B, depois G
  9Bh      VDP indireto    (MSX2) escreve no registrador apontado por R#17
  A0h      PSG endereco    AY-3-8910: escolhe o registrador (0 a 15)
  A1h      PSG dados       escrita do valor
  A2h      PSG leitura     le o registrador escolhido (R#14 = joystick)
  A8h      PPI A           slots primarios: 2 bits por pagina (pag. 0 nos
                           bits 1-0 ... pag. 3 nos bits 7-6)
  A9h      PPI B           teclado: coluna da linha escolhida (0 = apertada)
  AAh      PPI C           bits 3-0 = linha do teclado; 4 = motor do gravador;
                           5 = saida da fita; 6 = LED CAPS (0 = aceso);
                           7 = clique
  ABh      PPI controle    modo do 8255 (set/reset de bits do PPI C)
  B4h      RTC endereco    (MSX2) relogio RP-5C01: escolhe o registrador
  B5h      RTC dados       (MSX2) valor do registrador
  FCh-FFh  mapper de RAM   (MSX2) segmento de 16 KB de cada pagina
                           (FCh = pag. 0 ... FFh = pag. 3)

  No fwMSX a controladora de disco por portas fica em D0h-D4h (opcao
  --disk-acesso porta; a base muda com --disk-porta).

EXEMPLOS (Z80)
      LD   A,7
      OUT  (0A0h),A        ; PSG: registrador 7 (mixer)
      LD   A,10111000b
      OUT  (0A1h),A        ; so' tons ligados (bits 0-2 = 0)

      IN   A,(0A8h)        ; slots primarios atuais

VER TAMBEM
  slotsmsx, psgregs, vdpregs, teclado, bios)MAN";

const char *const kSlotsPage = R"MAN(DESCRICAO
  O Z80 enxerga 64 KB em 4 paginas de 16 KB (0000h, 4000h, 8000h, C000h).
  Cada pagina pode vir de um de 4 SLOTS PRIMARIOS (0 a 3), escolhido na
  porta A8h (2 bits por pagina). Um slot primario pode ser EXPANDIDO em 4
  SUBSLOTS (0 a 3): o registrador de subslot fica no endereco FFFFh do slot
  expandido (a leitura vem INVERTIDA: le-se o complemento do que se gravou).

  Registrador A8h:    bits 1-0 pag.0 | 3-2 pag.1 | 5-4 pag.2 | 7-6 pag.3
  Registrador FFFFh:  mesma forma, para os subslots (so' no slot expandido
                      selecionado na pagina 3)
  Formato de slot na BIOS (ENASLT, RDSLT, CALSLT...):  F000SSPP
      bit 7 = 1 se indica subslot; bits 3-2 = subslot; bits 1-0 = slot
  EXPTBL (FCC1h): 4 bytes, bit 7 = slot primario expandido.
  SLTTBL (FCC5h): copia do registrador de subslot de cada slot primario.

LAYOUT TIPICO
  slot 0   BIOS + BASIC (0000h-7FFFh); no MSX2 tambem a sub-ROM
  slot 1   cartucho A
  slot 2   cartucho B ou interface de disco
  slot 3   RAM (no MSX2 costuma ser expandido)

  O comando "slots" mostra o que esta' mapeado AGORA em cada pagina, e
  "man sysvars" mostra EXPTBL/SLTTBL.

EXEMPLOS (Z80)
      IN   A,(0A8h)           ; slots primarios
      AND  11001111b          ; limpa a pagina 2
      OR   00010000b          ; pagina 2 <- slot 1 (cartucho)
      OUT  (0A8h),A           ; (NAO faca isto com a pilha na pagina 2!)

  Para trocar de slot com seguranca use ENASLT (0024h) e chame sub-rotinas
  com CALSLT (001Ch).

VER TAMBEM
  slots, bios, sysvars, portas)MAN";

const char *const kVdpPage = R"MAN(DESCRICAO
  Registradores do VDP (TMS9918A no MSX1; V9938 no MSX2; V9958 no MSX2+).
  Escrita: OUT (99h),valor e depois OUT (99h),80h+n (o segundo byte tem o
  bit 7 em 1 e o numero do registrador). Do BASIC: VDP(n). A BIOS guarda
  copias de R#0 a R#7 em RG0SAV..RG7SAV (F3DFh-F3E6h); use WRTVDP (0047h).

REGISTRADORES (MSX1)
  R#0    bit 1 = M3 (Graphic 2); bit 0 = entrada de video externa
  R#1    bit 7 = 16K; 6 = BL (1 = tela ligada); 5 = IE (interrupcao de
         quadro); 4 = M1 (texto); 3 = M2 (multicolor); 1 = SI (sprites
         16x16); 0 = MAG (sprites ampliados)
  R#2    tabela de nomes = valor * 400h
  R#3    tabela de cores = valor * 40h
  R#4    tabela de padroes = valor * 800h
  R#5    atributos de sprites = valor * 80h
  R#6    padroes de sprites = valor * 800h
  R#7    nibble alto = cor do texto; baixo = cor do fundo/borda

MSX2 EM DIANTE
  R#0    bits 3-1 = M5 M4 M3 (modo de tela)
  R#8    bit 7 = MS, 5 = TP (cor 0 opaca), 3 = VR (VRAM 64K), 1 = SPD
  R#9    bit 7 = LN (212 linhas), 3 = IL, 2 = EO, 1 = NT (1 = PAL), 0 = DC
  R#14   bits 2-0 = A16-A14 (VRAM acima de 16 KB)
  R#15   numero do registrador de status lido por IN (99h)
  R#16   indice da paleta
  R#17   registrador indireto (bit 7 = sem auto-incremento)
  R#32-46  comandos do VDP (HMMM, LMMV, LINE...): SX SY DX DY NX NY CLR ARG
           CMR

STATUS S#0 (leitura em 99h)
  bit 7 = F (acabou o quadro; ler limpa)   bit 6 = 5S (5o sprite)
  bit 5 = C (colisao de sprites)           bits 4-0 = numero do 5o sprite

MODOS (R#0 bits M5 M4 M3 / R#1 bits M2 M1)
  Graphic 1 = 000/00      Text 1 = 000/01       Multicolor = 000/10
  Graphic 2 = 001/00      Graphic 3 = 010/00    Graphic 4 = 011/00
  Graphic 5 = 100/00      Graphic 6 = 101/00    Graphic 7 = 111/00

EXEMPLO (Z80: cor da borda = 4)
      LD   B,4
      LD   C,7
      CALL 0047h             ; WRTVDP  (R#7 = 04h)

VER TAMBEM
  screens, cores, portas, sysvars)MAN";

const char *const kPsgPage = R"MAN(DESCRICAO
  PSG AY-3-8910 (3 canais de tom + ruido + envelope). Portas A0h
  (registrador), A1h (escrever) e A2h (ler). Pela BIOS: WRTPSG (0093h:
  A=registrador, E=valor) e RDPSG (0096h). Do BASIC: SOUND r,v.

  R#0,1    periodo do tom do canal A (8 bits + 4 bits)
  R#2,3    periodo do canal B
  R#4,5    periodo do canal C
           frequencia = 111861 / periodo Hz (relogio 1,789773 MHz / 16)
  R#6      periodo do ruido (5 bits)
  R#7      mixer: bits 0-2 = tom A,B,C DESLIGADO; 3-5 = ruido A,B,C
           DESLIGADO; bit 6 = 0 porta A como ENTRADA; bit 7 = 1 porta B
           como SAIDA (valor tipico do MSX: B8h)
  R#8-10   volume de A, B, C: bits 3-0 = 0 a 15; bit 4 = usa o envelope
  R#11,12  periodo do envelope (16 bits)
  R#13     forma do envelope (bits 3-0: continuar, ataque, alternar, segurar)
  R#14     porta A: leitura do joystick (bits 0-3 = cima, baixo, esquerda,
           direita; 4-5 = botoes; 0 = apertado)
  R#15     porta B: bit 6 escolhe a porta de joystick (0 = porta 1,
           1 = porta 2); os outros controlam os pinos 6 e 7

EXEMPLO (BASIC: nota A4, 440 Hz, no canal A)
  SOUND 0,254 : SOUND 1,0 : SOUND 7,&B10111110 : SOUND 8,12
  (111861 / 440 = 254, o periodo)

VER TAMBEM
  portas, bios (WRTPSG, RDPSG, GICINI))MAN";

const char *const kScreensPage = R"MAN(DESCRICAO
  Modos de tela do MSX-BASIC (SCREEN n) e o que existe em cada geracao.

  n    modo           resolucao         cores
  0    Text 1         40x24 caract.     2 (frente/fundo)            MSX1 
  0    Text 2         80x24 caract.     2 + piscante (WIDTH 80)     MSX2 
  1    Graphic 1      32x24 caract.     16, 8 caracteres por cor    MSX1 
  2    Graphic 2      256x192           16, 2 por faixa de 8 pontos MSX1 
  3    Multicolor     64x48 blocos      16                          MSX1 
  4    Graphic 3      256x192           como o 2, sprites tipo 2    MSX2 
  5    Graphic 4      256x212           16 (4 bits/pixel)           MSX2 
  6    Graphic 5      512x212           4 (2 bits/pixel)            MSX2 
  7    Graphic 6      512x212           16                          MSX2 
  8    Graphic 7      256x212           256 (3 bits R, 3 G, 2 B)    MSX2 
  10   Graphic 7 YJK  256x212           YJK/RGB (12499 cores)       MSX2+
  11   Graphic 7 YJK  256x212           YJK/RGB (12499 cores)       MSX2+
  12   Graphic 7 YJK  256x212           YJK (19268 cores)           MSX2+

  A ultima coluna e' o primeiro modelo que tem o modo (os seguintes tambem).

  Troca: SCREEN n (limpa a tela); pela BIOS, CHGMOD (005Fh, A = n) ou
  INITXT/INIT32/INIGRP/INIMLT. O modo atual esta' em SCRMOD (FCAFh).
  Sprites: ate' 32, 8x8 ou 16x16 (SCREEN ,n), 4 por linha (8 no tipo 2).

EXEMPLOS
  type "screen 2" {enter}
  type "screen 5:color 15,4,4:cls" {enter}
  peek 0xFCAF

VER TAMBEM
  vdpregs, cores, bios (CHGMOD), sysvars)MAN";

const char *const kBasicPage = R"MAN(DESCRICAO
  Resumo dos comandos do MSX-BASIC (1.0 no MSX1; 2.0/2.1 no MSX2; 3.0 no
  MSX2+). Marcado (2): MSX2 em diante.

  CADA palavra tem um manual proprio, com formato, descricao e programa de
  exemplo (livro "Linguagem BASIC MSX", Aleph/Gradiente, e Manual MSX 2+ FM):
  man print, man screen, man put sprite, man for... A lista completa das
  palavras esta' no indice (man). Topicos do manual (variaveis, operadores,
  codigos de erro, mapa da memoria, ASCII...) tambem estao la'.

CONTROLE DE FLUXO
  FOR v=a TO b [STEP s] ... NEXT [v]      GOTO n
  GOSUB n ... RETURN [n]                  ON x GOTO/GOSUB n,n,...
  IF cond THEN a [ELSE b]                 END   STOP   CONT   NEW
  RUN [n|"arq"]                           CLEAR [str,topo]
  ON ERROR GOTO n   RESUME [n|NEXT]       ERR   ERL
  ON KEY|STOP|STRIG|SPRITE|INTERVAL=t|PLAY GOSUB n
  KEY(n)|STOP|STRIG(n)|SPRITE|INTERVAL|PLAY ON|OFF|STOP
  DEF FN f(x)=...   DEF USR[n]=end   DEFINT|SNG|DBL|STR letras
  DIM v(n)   ERASE v   LET   SWAP a,b   DATA ...   READ v   RESTORE [n]
  REM ou '   TRON   TROFF   AUTO   DELETE   LIST   RENUM

TELA E TECLADO
  PRINT [USING f;]x   LPRINT   INPUT ["txt";]v   LINE INPUT   INKEY$
  INPUT$(n)   LOCATE x,y,c   CLS   WIDTH n   KEY n,"txt"   KEY ON|OFF|LIST
  SCREEN modo,tam_sprite,clique,baud,imp,interlace(2)
  COLOR f,b,borda   COLOR=(n,r,g,b)(2)   COLOR=RESTORE(2)
  CSRLIN   POS(0)   TAB()   SPC()   BEEP

GRAFICOS E SPRITES
  PSET (x,y),c   PRESET   LINE (x1,y1)-(x2,y2),c,[B|BF]
  CIRCLE (x,y),r,c,ini,fim,aspecto   PAINT (x,y),c,borda
  DRAW "cmds"   POINT(x,y)   SPRITE$(n)="..."   PUT SPRITE p,(x,y),c,padrao
  COPY (x1,y1)-(x2,y2),pag1 TO (x,y),pag2(2)   SET PAGE d,a(2)
  SET ADJUST(x,y)(2)   VDP(n)   BASE(n)

SOM
  PLAY "mml"[,"mml2","mml3"]   SOUND reg,valor   BEEP
  PLAY: T tempo, L duracao, O oitava, V volume, S/M envelope, R pausa,
  N nota, < > oitava.   Com MSX-MUSIC: CALL MUSIC / CALL VOICE / CALL AUDIO

MEMORIA E E/S
  PEEK(a)   POKE a,v   VPEEK(a)   VPOKE a,v   INP(p)   OUT p,v
  FRE(0)   FRE("")   USR[n](x)   VARPTR   CALL SYSTEM   MAXFILES=n
  BLOAD "arq"[,R|,S]   BSAVE "arq",ini,fim[,exec]

ARQUIVOS E FITA
  OPEN "dev:arq" FOR INPUT|OUTPUT|APPEND AS #n   CLOSE   EOF(n)
  PRINT #n,...   INPUT #n,v   LINE INPUT #n,v$   LOC(n)   LOF(n)
  LOAD   SAVE   MERGE   CLOAD   CSAVE   CLOAD?   MOTOR [ON|OFF]
  FILES   KILL   NAME a AS b   CALL FORMAT   (Disk BASIC: FIELD LSET
  RSET GET PUT MKI$ CVI...)

FUNCOES
  ABS ATN COS SIN TAN EXP LOG SQR INT FIX RND SGN CINT CSNG CDBL
  ASC CHR$ LEFT$ RIGHT$ MID$ LEN STR$ VAL STRING$ SPACE$ INSTR
  HEX$ OCT$ BIN$ STICK STRIG PAD PDL TIME DATE$(2) TIME$(2)
  (TIME e' o JIFFY: ver sysvars)

EXEMPLOS
  type "10 for i=1 to 8:color i:print i:next" {enter}
  type "run" {enter}
  type "?peek(&hfcaf)" {enter}

VER TAMBEM
  screens, cores, bios, sysvars, type)MAN";

} // namespace

std::vector<Topic> ReferenceTopics() {
    std::vector<Topic> v;
    auto add = [&](const char *name, const char *aliases, const char *summary, std::string page) {
        v.push_back(Topic{name, aliases, "Referencia MSX", "", summary, std::move(page), false});
    };
    add("bios", "rotinas", "tabela de entradas da BIOS (CHPUT, CHGET, WRTVDP...)", BiosPage());
    add("sysvars", "workarea", "variaveis do sistema (FORCLR, JIFFY, SCRMOD...)", SysVarsPage());
    add("hooks", "ganchos,hook", "ganchos da RAM (H.KEYI, H.TIMI...) e como encadear", HooksPage());
    add("basic", "msxbasic", "resumo dos comandos do MSX-BASIC", kBasicPage);
    add("cores", "colors,cor", "os 16 codigos de cor do MSX", kColorPage);
    add("screens", "telas,screen", "modos de tela (SCREEN 0 a 12)", kScreensPage);
    add("vdpregs", "vdp", "registradores do VDP (TMS9918, V9938, V9958)", kVdpPage);
    add("psgregs", "psg,ay8910", "registradores do PSG AY-3-8910 (som)", kPsgPage);
    add("portas", "ports,io", "portas de E/S do MSX (VDP, PSG, PPI, RTC, mapper...)", kPortsPage);
    add("slotsmsx", "slotmsx,slotsfisicos", "slots, subslots e a pagina de 16 KB", kSlotsPage);
    add("teclado", "matriz,keyboard", "a matriz do teclado (linhas, colunas, nomes de tecla)", KeyboardPage());
    return v;
}

} // namespace man
