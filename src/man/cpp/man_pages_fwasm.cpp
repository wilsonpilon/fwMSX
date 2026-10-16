// fwMSX -- MAN: manual do montador fwasm (fwmsx --asm / comando "asm"). Texto sem acento (ASCII).
//
// O fwasm e' uma reimplementacao clean-room do asMSX 1.2.0 (BSD-3-Clause; Eduardo A. Robsy Petrus e colaboradores). Estas paginas
// seguem a estrutura do manual do asMSX (resource/asmsx.md) mas descrevem o que o FWASM faz DE FATO: cada diretiva diz se ja'
// foi implementada. As tabelas de rotinas e variaveis da BIOS saem das mesmas tabelas que o montador usa (bios_tables.inc).
#include <algorithm>
#include <cstdio>

#include "man_internal.h"

namespace {
#include "../../asm/cpp/bios_tables.inc"
} // namespace

namespace man {

namespace {

std::string Hex4(int v) {
    char b[8];
    std::snprintf(b, sizeof(b), "%04X", v & 0xFFFF);
    return b;
}

// As tabelas de simbolos em 4 colunas: "NOME    XXXXh   NOME    XXXXh ...", em ordem de endereco.
template <typename T, size_t N>
std::string SymbolColumns(const T (&table)[N]) {
    std::vector<std::pair<int, std::string>> v;
    for (size_t i = 0; i < N; ++i) v.emplace_back(table[i].value, table[i].name);
    std::stable_sort(v.begin(), v.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    std::string out;
    for (size_t i = 0; i < v.size(); ++i) {
        std::string name = v[i].second;
        name.resize(std::max<size_t>(name.size(), 7), ' ');
        out += (i % 4 == 0 ? "  " : "   ") + name + " " + Hex4(v[i].first) + "h";
        if (i % 4 == 3 || i + 1 == v.size()) out += "\n";
    }
    return out;
}

const char *const kFwasm = R"MAN(DESCRICAO
  O fwasm e' o montador Z80 do fwMSX: uma reimplementacao do asMSX 1.2.0
  (clean-room: escrita a partir do manual e do comportamento do asmsx.exe,
  conferida por mais de 6200 casos de teste gerados com o asmsx.exe de verdade,
  inclusive os arquivos .rom, .bin, .com, .cas, .wav e .tap, byte a byte).
  Aceita todas as instrucoes oficiais e as nao documentadas do Z80, expressoes
  com operadores e funcoes matematicas, macros, repeticao, montagem condicional,
  includes, as tabelas de nomes da BIOS do MSX e gera cartuchos ROM e MegaROM
  (Konami, Konami SCC, ASCII8, ASCII16), programas para o BASIC, MSX-DOS, fitas
  e arquivos para o ZX Spectrum.

USO
  fwmsx --asm [-z] [-s] [-vv] [-o <caminho>] <arquivo.asm>
  asm [-z] [-s] [-vv] [-o <caminho>] <arquivo.asm>     (no console e na TUI)

  -z   sintaxe Zilog padrao (parenteses para memoria), sem a diretiva .zilog
  -s   silencioso: sem mensagens de progresso
  -vv  mensagens detalhadas
  -o   nome-base das saidas (sem extensao), ou uma pasta terminada em / ou \
  -r   aceito e ignorado (os includes ja' sao relativos a pasta do fonte)

SAIDAS (a extensao depende do formato; .filename "x" troca o nome-base)
  .z80   os bytes montados SEM cabecalho (quando nao ha' diretiva de formato)
  .rom   cartucho ROM ou MegaROM (.rom, .megarom)
  .bin   arquivo do BASIC, BLOAD "x.bin",R (.basic)
  .com   executavel do MSX-DOS (.msxdos)
  .cas   fita para emuladores (.cas); BLOAD "CAS:",R
  .wav   fita para um MSX de verdade, 44,1 kHz, 8 bits (.wav)
  .tap   fita para o ZX Spectrum (.sinclair)
  .sym   tabela de simbolos (a MegaROM mostra a subpagina: "01h:8000h nome")
  .txt   mensagens de print (so' existe se houver print)
  Codigo de saida: 0 = ok; diferente de 0 = erro no fonte (a mensagem diz a
  linha); 2 = uso incorreto ou arquivo ilegivel.

ESTADO
  Todas as diretivas do asMSX estao prontas (fwasm-directives). Diferencas
  conhecidas: um programa com codigo no endereco 0000h antes de .sinclair gera,
  no asMSX, um bloco de 32 KB (o fwasm gera os bytes certos); .debug/.break
  gravam os marcadores do blueMSX mas o fwMSX ainda nao os usa (o depurador do
  proprio fwMSX tem breakpoints: man break).

EXEMPLO: UM CARTUCHO DE 16 KB (testado)
        .page 1
        .start init
        .rom                ; cabecalho "AB" + INIT, gerado pelo fwasm
  init: ld hl,msg
  loop: ld a,[hl]
        or a
        jr z,fim
        call 00A2h          ; CHPUT
        inc hl
        jr loop
  fim:  jr fim
  msg:  db "Ola do fwasm!",0

  Monte e rode (o .rom sai pronto):
    asm hello.asm
    emu start --cart hello.rom

EXEMPLO: PROGRAMA DO BASIC
        .org 0d000h
        .basic
        .cas "TESTE"        ; tambem gera TESTE.cas
        ld a,[0fcafh]       ; SCRMOD
        ret
  No MSX-BASIC:  BLOAD"teste.bin",R   (ou  BLOAD"CAS:",R  com a fita)

TOPICOS DESTE MANUAL
  fwasm-syntax        formato da linha, comentarios, colchetes x parenteses
  fwasm-labels        rotulos, rotulos locais, equ, variaveis, $
  fwasm-numbers       formatos numericos
  fwasm-operators     operadores, funcoes matematicas, ponto fixo
  fwasm-data          db dw ds dt byte word incbin include
  fwasm-directives    todas as diretivas, com o estado de cada uma
  fwasm-formats       .rom .megarom .basic .msxdos .cas .wav .sinclair .size
  fwasm-megarom       .megarom, .subpage e .select (mappers)
  fwasm-conditional   if / ifdef / ifndef / else / endif
  fwasm-macros        macro / endm e rept / endr
  fwasm-bios          nomes das rotinas da BIOS (.bios)
  fwasm-biosvars      nomes das variaveis do sistema (.biosvars)

VER TAMBEM
  asm, bios, sysvars, hooks)MAN";

const char *const kSyntax = R"MAN(DESCRICAO
  Formato da linha do fonte:

    [rotulo:]   [diretiva [parametros]]   [;comentario]
    [rotulo:]   [instrucao [parametros]]  [;comentario]

  Cada bloco entre colchetes e' opcional. Nao ha' limite para o tamanho da
  linha. Espacos, tabulacoes e linhas em branco sao ignorados. Fins de linha
  CR/LF (Windows) ou so' LF (Linux) funcionam.

  Diferenca em relacao ao Z80 padrao: a indirecao de memoria usa COLCHETES
  [ ], nao parenteses; os parenteses ficam livres para ordenar contas:

    ld a,[hl]        ; A <- (HL)
    ld a,[ix+3]
    ld hl,(2+3)*4    ; parenteses = conta, nao memoria

  A diretiva .zilog (ou a opcao -z) inverte: parenteses = memoria, colchetes =
  agrupamento de contas. Vale do ponto onde aparece em diante.

INSTRUCOES
  Todas as instrucoes oficiais do Z80 e as nao documentadas (SLL, IXH, IXL, IYH,
  IYL, OUT (C),0...), com acumulador implicito: "sub b" e "sub a,b" valem.

COMENTARIOS
  ; texto            ate' o fim da linha (o padrao dos listados)
  // texto           idem, como em C++
  /* texto */        varias linhas, como em C
  { texto }          varias linhas, como em Pascal
  -- texto           ate' o fim da linha, como em Ada

DIRETIVAS SEM PONTO
  Todas as diretivas podem ser escritas com ou sem o ponto inicial (.org = org),
  em maiusculas ou minusculas.

EXEMPLO
  loop:
          ld a,[0ffffh]   ; le o registro mestre de subslots
  points:
          db 20,30,40,50  ; tabela de pontos
          org 08000h      ; codigo na pagina 2
          nop

VER TAMBEM
  fwasm, fwasm-labels, fwasm-directives)MAN";

const char *const kLabels = R"MAN(DESCRICAO
  ROTULOS
    Um rotulo e' o nome simbolico de um endereco. Comeca com letra ou sublinhado
    (_), o resto pode ser letra ou numero, e TERMINA com dois-pontos na
    definicao. Maiusculas e minusculas sao diferentes (LABEL: e Label: sao dois
    rotulos). O dois-pontos define o rotulo com o endereco da proxima instrucao
    ou dado; para usar o endereco, escreva o nome SEM os dois-pontos.

      LABEL:    Label:    _alfa:    loop001:

  ROTULOS LOCAIS
    Com dois arrobas (@@nome) ou um ponto (.nome) na frente, o rotulo so' vale
    ate' o proximo rotulo global. Assim nomes de laco se repetem sem conflito:

      Funcao1:
      @@loop:   ...            Funcao1:
      Funcao2:                 .loop:    ...
      @@loop:   ...            Funcao2:
                               .loop:    ...

  CONSTANTES E VARIAVEIS
    nome equ expressao       constante (nao ha' constantes locais). Tambem vale
    nome: equ expressao      com dois-pontos.
    nome = expressao         variavel inteira: pode ser reatribuida e usada em
                             contas. Precisa de um valor inicial.

      Valor1 = 0
      Valor1 = Valor1 + 1
      ld a,Valor1
      Valor1 = Valor1 * 2
      ld b,Valor1

  O ENDERECO ATUAL: $
    $ e' uma variavel somente de leitura. Na montagem vale o endereco da proxima
    instrucao ou dado; na execucao seria o PC.  Ex.:  jr $+2

  NOMES RESERVADOS
    As diretivas, as funcoes (sin, cos, ...), os nomes dos mappers (konami,
    ascii8...) e os registradores nao servem de nome de rotulo: o fwasm avisa
    "reserved word used as identifier".

  A SAIDA .sym
    O arquivo .sym lista todos os simbolos com o valor em decimal (compativel com
    o depurador do blueMSX). Nao e' gerado se nada foi definido.

VER TAMBEM
  fwasm, fwasm-numbers, fwasm-operators, fwasm-bios)MAN";

const char *const kNumbers = R"MAN(DESCRICAO
  Formatos numericos aceitos nas expressoes:

  DECIMAL INTEIRO     0  10  25  255  2048
  DECIMAL COM PONTO   3.14  1.25  0.0  32768.0  -0.50  15.2
                      (o ponto e' obrigatorio para ser ponto flutuante; veja
                      fix() em fwasm-operators para virar ponto fixo de 16 bits)
  OCTAL               0 na frente: 01 077 010;  ou letra o no fim: 1o 77o 10o
                      (a forma com o e' so' por compatibilidade: O parece 0)
  HEXADECIMAL         0x8a 0xff   |   $8a $ff   |   8ah 0ffh 10h
                      (com h no fim o primeiro digito tem de ser numerico:
                      escreva 0ffh, nao ffh)
  BINARIO             1000000b  11110000b  0010101b  1001b

  Textos entre aspas viram bytes nas diretivas de dados (db "texto").

EXEMPLOS
  ld a,0ffh
  ld hl,$c000
  ld b,00001111b

VER TAMBEM
  fwasm, fwasm-operators)MAN";

const char *const kOperators = R"MAN(DESCRICAO
  OPERADORES (a precedencia e' a do C/C++; parenteses mudam a ordem)
    +  -  *  /  %          soma, subtracao, produto, divisao, resto
    <<  >>                 desloca bits para a esquerda / direita
    |  &  ^                OU, E, OU-exclusivo bit a bit
    NOT                    negacao (unaria)
    ~                      complemento
    ||  &&                 OU e E logicos
    ==  !=  <  <=  >  >=   comparacoes (resultado 0 ou nao zero)

  FUNCOES
    SIN(x) COS(x) TAN(x)   trigonometricas, em radianos
    ACOS(x) ASIN(x) ATAN(x)
    SQR(x)                 quadrado           SQRT(x)   raiz quadrada
    EXP(x)  POW(x,y)       exponencial; x elevado a y
    LOG(x)  LN(x)          logaritmo; logaritmo natural
    ABS(x)                 valor absoluto
    PI                     constante (dupla precisao)
    RANDOM(n)              inteiro aleatorio de 0 a n-1 (.random(n))

  PONTO FIXO
    FIX(x)                 converte um ponto flutuante em ponto fixo de 16 bits
    FIXMUL(x,y)            multiplica dois pontos fixos
    FIXDIV(x,y)            divide x por y
    INT(x)                 converte um valor nao inteiro em inteiro

EXEMPLOS
  ((2*8)/(1+3))<<2
  sin(pi*45.0/180.0)
  dw fix(1.5), fixmul(fix(2.0),fix(3.0))

VER TAMBEM
  fwasm, fwasm-numbers, fwasm-labels)MAN";

const char *const kData = R"MAN(DESCRICAO
  DADOS EM LINHA
    db  dado[,dado...]       bytes (aliases: defb, dt, deft; todos equivalem)
    dt  "texto"
    dw  dado[,dado...]       palavras de 16 bits (alias: defw)
    ds  n                    reserva n bytes a partir do endereco atual (defs)
    .byte                    reserva um byte      (variavel: nome: .byte)
    .word                    reserva uma palavra  (variavel: nome: .word)

    msg:    db "Ola",13,10,0
    tabela: dw inicio, fim, 1234h
    buffer: ds 256
    cont:   .byte
    ptr:    .word

  Valores fora da faixa (db: -128..255; dw: -32768..65535) geram o aviso
  "overflow. Cropping to fit" e o valor e' cortado em 8 ou 16 bits. Textos entre
  aspas em db viram um byte por caractere.

  INCLUIR ARQUIVOS
    include "arquivo"        inclui outro fonte (relativo a pasta do fonte;
                             aninhado vale)
    incbin "arquivo" [skip X] [size Y]
                             injeta os bytes de um arquivo binario; skip salta X
                             bytes do inicio, size limita a Y bytes

  O PONTEIRO DE MONTAGEM
    org X          muda o endereco atual (page 0 = org 0000h, page 1 = 4000h,
    page n         page 2 = 8000h, page 3 = 0C000h)
    phase X        monta o codigo para o endereco X mas o guarda no atual
    dephase        volta ao normal (org e page tambem desfazem o phase)

VER TAMBEM
  fwasm, fwasm-directives, fwasm-labels)MAN";

const char *const kDirectives = R"MAN(DESCRICAO
  As diretivas podem ser escritas com ou sem o ponto inicial (.org = org), em
  maiusculas ou minusculas. Todas estao PRONTAS no fwasm.

  diretiva          O QUE FAZ
  zilog             parenteses = memoria, colchetes = contas (-z faz o mesmo)
  org X             muda o endereco atual
  page X            idem, em blocos de 16 KB (page 1 = 4000h)
  equ               define constantes:  nome equ expr
  nome = expr       variavel inteira reatribuivel
  bios              libera os nomes das rotinas da BIOS (fwasm-bios)
  biosvars          libera os nomes das variaveis (fwasm-biosvars)
  phase X/dephase   monta para um endereco e guarda em outro
  db defb dt deft   bytes e textos
  dw defw           palavras de 16 bits
  ds defs           reserva bytes
  byte word         reservam um byte / uma palavra
  include           inclui outro fonte
  incbin            inclui um binario (skip, size)
  print printdec    escreve o valor decimal de uma expressao no .txt
  printhex          idem, em hexadecimal
  printfix          idem, para um ponto fixo
  printtext         escreve um texto no .txt (printstring e' sinonimo)
  filename "x"      nome-base das saidas (o -o tem prioridade)
  if/ifdef/ifndef/else/endif    montagem condicional (fwasm-conditional)
  macro/endm, rept/endr         macros e repeticao (fwasm-macros)
  random(n)         funcao: inteiro aleatorio de 0 a n-1
  start X           ponto de entrada (cabecalho da ROM/MegaROM/BASIC)
  rom               cartucho ROM: cabecalho "AB" de 16 bytes
  megarom [mapper]  MegaROM: konami, konamiscc, ascii8, ascii16
  subpage n at X    define uma subpagina da MegaROM
  select n at X     troca a subpagina visivel (gera o codigo)
  search            acha o slot da pagina 2 numa ROM que comeca na 1
  basic             binario carregavel por BLOAD "x",R (extensao .bin)
  msxdos            executavel .com do MSX-DOS (carregado em 0100h)
  callbios X        chama uma rotina da BIOS (LD IY,..; LD IX,X; CALL CALSLT)
  calldos X         chama uma funcao do MSX-DOS (so' com .msxdos)
  cas ["nome"]      fita .cas para BLOAD "CAS:",R (cassette e' sinonimo)
  wav ["nome"]      fita .wav para um MSX de verdade
  sinclair          arquivo .tap para o ZX Spectrum
  size X            tamanho do arquivo de saida em KB
  debug "texto"     marcador de texto para o depurador do blueMSX
  break [X]         marcador de breakpoint do blueMSX (breakpoint = sinonimo)

  Os formatos (rom, megarom, basic, msxdos, sinclair) sao exclusivos: misturar
  da' o erro "type previously defined". Detalhes: man fwasm-formats e
  man fwasm-megarom.

DETALHES
  .org X         todo o codigo seguinte e' montado a partir de X. Voltar para um
                 endereco menor e' permitido; a saida vai do menor ao maior
                 endereco gravado.
  .page X        page 0 = 0000h, 1 = 4000h, 2 = 8000h, 3 = 0C000h.
  .phase X       as instrucoes sao montadas para executar em X, mas guardadas no
                 endereco atual: util para codigo que fica na ROM e e' copiado
                 para a RAM. Os rotulos valem X em diante. .dephase volta ao
                 normal (org, page e subpage tambem).
  .start X       na ROM o .start tem de vir ANTES do .rom (depois e' ignorado, a
                 nao ser que haja fita); na MegaROM e no BASIC vale em qualquer
                 ordem. Sem .start o INIT/execucao e' o fim do cabecalho / o
                 primeiro endereco.
  .size X        enche com zeros ate' X KB. Se o programa nao cabe: erro "code
                 size overflow"; se passa de 64 KB a partir do primeiro endereco:
                 "memory overflow". Na MegaROM so' corta ou completa.
  .incbin "f" skip X size Y   injeta um binario; skip e size sao opcionais.
  .print expr    escreve no arquivo .txt (nao na tela).
  .callbios X    gera  LD IY,[EXPTBL-1] / LD IX,X / CALL CALSLT  (11 bytes).
  .calldos X     gera  LD C,X / CALL 0005h  (5 bytes).
  .debug "t"     gera  LD D,D / JR (tamanho+4) / texto  (sem terminador).
  .break [X]     sem X: LD B,B / JR 0; com X: LD B,B / JR 2 / DW X.

VER TAMBEM
  fwasm, fwasm-formats, fwasm-megarom, fwasm-data, fwasm-conditional)MAN";

const char *const kFormats = R"MAN(DESCRICAO
  Sem diretiva de formato a saida e' o bloco cru (.z80). Os formatos abaixo
  trocam o arquivo gerado; so' um por programa (repetir o mesmo e' permitido).

.ROM  (arquivo .rom)
  Escreve um cabecalho de 16 bytes no endereco atual -- "AB", o INIT, 12 zeros --
  e o codigo vem a seguir. Use depois de .page/.org. O INIT e' o .start dado
  ANTES do .rom, ou o fim do cabecalho. O arquivo comeca no MENOR endereco gravado e
  seu tamanho e' o do bloco arredondado para multiplo de 8 KB (ou o .size).
  Escrever na pagina 3 (>= 0C000h) e' o erro "cannot write ROM code/data to page 3".
  Particularidade do asMSX, reproduzida: o arredondamento conta a partir do ULTIMO
  endereco, entao um bloco de exatamente 8 KB + 1 byte perde esse ultimo byte.

.BASIC  (arquivo .bin)
  Arquivo do BASIC: byte 0FEh, endereco inicial, final e de execucao (2 bytes cada)
  e os bytes. A execucao e' o .start (em qualquer ordem) ou o endereco inicial. Com
  .size o arquivo e' completado com zeros, mas o endereco final nao muda.

.MSXDOS  (arquivo .com)
  Executavel do MSX-DOS: so' os bytes, montados a partir de 0100h (a diretiva poe o
  endereco em 0100h; .org depois dela vale). Tem de vir antes de qualquer codigo.
  Com .msxdos existe .calldos.

.SIZE n
  Tamanho do arquivo em KB (zeros no fim). Uma vez so'. .size 0 = tamanho natural.

.SEARCH
  Para ROM e MegaROM: gera 31 bytes que descobrem o slot da pagina 2 e a ativam
  (RSLREG, EXPTBL, ENASLT). Use num cartucho que comeca na pagina 1.

.CAS ["nome"]  /  .WAV ["nome"]
  Gera tambem a fita. O nome (ate' 6 letras, exatamente como escrito) e' o que o MSX
  mostra no BLOAD; sem nome vale o nome-base da saida. O PRIMEIRO .cas ou .wav fixa
  o nome dos dois. Mensagens: "cas file x.cas" e "Audio file x.wav saved [8.55 sec]".
    .basic        fita de arquivo BLOAD: BLOAD"CAS:",R
    .rom na pagina 2 (a partir de 8000h)   idem; o INIT final entra no cabecalho
    sem formato, .msxdos, .sinclair   so' os bytes (precedidos da marca do .cas)
    .rom abaixo de 8000h   aviso "cas file generation is not supported for ROM that
                           start below address 0x8000" e nenhum arquivo de fita
    .megarom   aviso "cas file generation is not supported for MEGAROM"
  O .wav tem 16000 ciclos de 2400 Hz de tom de cabecalho, os bytes a 1200 baud
  (1 start, 8 bits, 2 stop; o 1 e' dois ciclos de 2400 Hz, o 0 um de 1200 Hz) e um
  segundo tom de 4000 ciclos entre a identificacao e os dados; termina com um numero
  par de amostras. Carregue num MSX real pela entrada de fita.

.SINCLAIR  (arquivo .tap)
  Fita do ZX Spectrum: bloco CODE com o nome-base (10 letras) e os bytes. Com
  .start (diferente de 0) vem antes um carregador BASIC de uma linha:
      10 CLEAR VAL "<inicio-1>": LOAD ""CODE: RANDOMIZE USR VAL "<start>"
  Nao pode ser misturado com os outros formatos ("Sinclair directive should
  preceed any code"). Os checksums do carregador seguem o asMSX original.

EXEMPLOS
  ROM de 32 KB:    .page 1 / .rom / .size 32 / ...
  MegaROM:         .megarom konami / .start init / ...   (man fwasm-megarom)
  BASIC + fita:    .org 0d000h / .basic / .cas "JOGO" / ...
  MSX-DOS:         .msxdos / ld c,9 / ld de,msg / call 5 / ret / msg: db "Oi$"
                   (ou  .calldos 9  no lugar de  ld c,9 / call 5 )

VER TAMBEM
  fwasm, fwasm-directives, fwasm-megarom, cart, tape)MAN";

const char *const kMegaRom = R"MAN(DESCRICAO
  Uma MegaROM e' um cartucho maior que 64 KB cujo codigo vive em SUBPAGINAS que
  um registrador do mapper coloca, uma de cada vez, nas janelas de 8 KB (ou 16 KB)
  do Z80. O fwasm monta o arquivo .rom inteiro; o programa troca de subpagina com
  .select.

.MEGAROM [mapper]
  Escolhe o mapper e escreve o cabecalho "AB" em 4000h (a subpagina 0 vive la'; .page
  e .org antes dela nao importam). Mappers:
    konami     subpaginas de 8 KB, ate' 32 (256 KB). A janela 4000h e' fixa.
    konamiscc  8 KB, ate' 64 (512 KB); acessa o SCC.
    ascii8     8 KB, ate' 256 (2 MB).
    ascii16    16 KB, ate' 256 (4 MB).
  Sem argumento vale konami. Um segundo .megarom nao faz nada. Depois do .start
  (em qualquer ordem) o INIT do cabecalho e' o final.

.SUBPAGE n AT X
  Abre a subpagina n (1 em diante; a 0 e' a do .megarom) cujo codigo sera' executado
  com a janela em X (4000h a 0BFFFh). O codigo seguinte vai para o arquivo em
  n*tamanho + (endereco - X); passar do tamanho da subpagina e' "megaROM subpage
  overflow". Definir a mesma subpagina duas vezes: "megaROM subpage already defined".

.SELECT n AT X   |   .SELECT registrador AT X
  Gera o codigo que escolhe a subpagina n na janela X: PUSH AF / LD A,n / LD (reg),A
  / POP AF (7 bytes); com A como registrador so' o LD (reg),A (3 bytes); com B, C, D,
  E, H ou L: PUSH AF / LD A,r / LD (reg),A / POP AF. "reg" e' o registrador do mapper:
    konami     janela 6000h -> 6000h, 8000h -> 8000h, 0A000h -> 0A000h
               (janela 4000h: erro "Konami megaROM forces page 0 at 4000h")
    konamiscc  4000h -> 5000h, 6000h -> 7000h, 8000h -> 9000h, 0A000h -> 0B000h
    ascii8     4000h -> 6000h, 6000h -> 6800h, 8000h -> 7000h, 0A000h -> 7800h
    ascii16    4000h -> 6000h, 8000h -> 7000h
  A subpagina n tem de existir em algum ponto do programa (pode vir DEPOIS do select):
  senao "megaROM subpage not defined".
  A BIOS so' liga a pagina 1 (4000h) ao cartucho: para usar janelas em 8000h ou 0A000h
  chame .search antes do primeiro .select (testado no emulador com os quatro mappers).

ARQUIVO .rom
  Tem (maior subpagina definida + 1) paginas. Se alguma entre 0 e a maior nao foi
  definida o aviso diz "only N out of M megaROM pages are defined" (N e' quantas
  FALTAM -- assim e' no asMSX). .size corta ou completa o arquivo. O .sym mostra
  cada rotulo com sua subpagina ("01h:8000h nome"). Na MegaROM nao ha' fita
  (.cas/.wav dao um aviso) e .page nao serve (depois dele o codigo da' "code/data
  without subpage" ate' o proximo .subpage).

EXEMPLO
        .bios
        .megarom konami
        .start init
  init: .search                  ; liga a pagina 2 (8000h) ao slot do cartucho
        .select 1 at 8000h       ; mostra a subpagina 1 na janela de 8000h
        jp sub1
        .subpage 1 at 8000h
  sub1: ld a,'A'
        call CHPUT
        jr $

VER TAMBEM
  fwasm, fwasm-formats, fwasm-directives, cart)MAN";

const char *const kConditional = R"MAN(DESCRICAO
  Montagem condicional:

    IF condicao
      instrucoes
    ELSE
      instrucoes
    ENDIF

  A condicao pode ser qualquer expressao; e' verdadeira se o resultado nao e'
  zero. O ELSE e' opcional; o ENDIF e' obrigatorio. IF, ELSE e ENDIF ficam cada um
  na SUA linha (nada mais na mesma linha). Os IFs podem se aninhar. Nos blocos
  falsos nada e' montado: nem codigo, nem diretivas, nem macros (erros dentro
  deles nao aparecem).

  IFDEF simbolo / IFNDEF simbolo   montam se o simbolo ja' foi definido / nao
  foi. Atencao: o simbolo precisa estar definido ANTES do IFDEF.

EXEMPLOS
  IF (computer==MSX)
    call MSX1_Init
  ELSE
    IF (computer==MSX2)
      call MSX2_Init
    ELSE
      call TurboR_Init
    ENDIF
  ENDIF

  CARTUCHO=1
  BINARIO=2
  formato=CARTUCHO
  IF (formato==CARTUCHO)
    org 4000h
    db "AB"
  ENDIF

  IFDEF depurar
    print 1
  ENDIF

ERRADO (tudo numa linha):
  IF (variavel) nop ELSE inc a ENDIF

VER TAMBEM
  fwasm, fwasm-labels, fwasm-macros)MAN";

const char *const kMacros = R"MAN(DESCRICAO
  MACROS
    Uma macro e' um pedaco de codigo copiado onde for chamada (nao e' uma funcao:
    o codigo cresce a cada uso). Os parametros sao trocados por busca e
    substituicao do TEXTO que se passa na chamada, entao valem constantes,
    variaveis, registradores ou nomes.

      nome: MACRO #param1, #param2
        ...
      ENDM

    Chame escrevendo o nome e os argumentos separados por virgula.

  REPETICAO
    REPT n ... ENDR   repete o bloco n vezes. O n tem de ser um numero inteiro
    escrito no fonte (nao pode ser calculado por uma expressao). Aninha-se.

EXEMPLOS
  ; macro com um parametro
  m_INC16: MACRO #VARIAVEL
      push hl
      ld   hl,#VARIAVEL
      inc  [hl]
      pop  hl
  ENDM
      m_INC16 CONTADOR

  ; macro com um registrador e uma constante como argumentos
  m_PONTO: MACRO #X,#Y
      ld  b,#X
      ld  c,#Y
  ENDM
      m_PONTO 0,a

  ; repeticao
  REPT 16
      outi
  ENDR

  X=0
  Y=0
  REPT 10
    REPT 10
      db X*Y
      X=X+1
    ENDR
    Y=Y+1
  ENDR

OBSERVACOES
  Macros e repeticoes podem se chamar e se conter. Uma macro nao pode ser definida
  duas vezes com o mesmo nome. Os rotulos locais dentro de macros seguem as regras
  de fwasm-labels.

VER TAMBEM
  fwasm, fwasm-conditional, fwasm-data)MAN";

} // namespace

std::vector<Topic> FwasmTopics() {
    std::vector<Topic> v;
    auto add = [&](const char *name, const char *aliases, const char *summary, std::string page) {
        v.push_back(Topic{name, aliases, "Montador fwasm", "", summary, std::move(page), false});
    };
    add("fwasm", "fwasm-manual,asmsx", "manual do montador Z80 do fwMSX (baseado no asMSX): uso, saidas e estado", kFwasm);
    add("fwasm-syntax", "", "formato da linha, comentarios, colchetes x parenteses, instrucoes", kSyntax);
    add("fwasm-labels", "", "rotulos, rotulos locais, equ, variaveis e o endereco atual ($)", kLabels);
    add("fwasm-numbers", "", "formatos numericos: decimal, hexadecimal, octal, binario, ponto flutuante", kNumbers);
    add("fwasm-operators", "fwasm-functions", "operadores, funcoes matematicas e ponto fixo das expressoes", kOperators);
    add("fwasm-data", "", "db, dw, ds, dt, byte, word, include, incbin, org, page, phase", kData);
    add("fwasm-directives", "", "todas as diretivas do asMSX e o estado de cada uma no fwasm", kDirectives);
    add("fwasm-formats", "", ".rom .basic .msxdos .cas .wav .sinclair .size .search: os arquivos de saida", kFormats);
    add("fwasm-megarom", "", "MegaROM: .megarom, .subpage, .select e os mappers", kMegaRom);
    add("fwasm-conditional", "", "if / ifdef / ifndef / else / endif", kConditional);
    add("fwasm-macros", "fwasm-rept", "macro / endm e rept / endr", kMacros);
    add("fwasm-bios", "", "nomes das rotinas da BIOS liberados por .bios (com os enderecos)",
        "DESCRICAO\n  A diretiva .bios libera estes nomes como constantes (maiusculas). O endereco de cada\n"
        "  rotina e' fixo em todas as geracoes do MSX. Explicacao de cada uma: man bios\n  (e man <nome>, ex.: man chput).\n\n"
        "NOMES (" + std::to_string(sizeof(kBiosLabels) / sizeof(kBiosLabels[0])) + ")\n" + SymbolColumns(kBiosLabels) +
        "\nEXEMPLO\n      .bios\n      org 4000h\n      call CHPUT      ; escreve o caractere de A\n\nVER TAMBEM\n  fwasm, fwasm-biosvars, bios");
    add("fwasm-biosvars", "", "nomes das variaveis do sistema liberados por .biosvars (com os enderecos)",
        "DESCRICAO\n  A diretiva .biosvars libera estes nomes como constantes. Valor e significado:\n  man sysvars (e man <nome>, ex.: man jiffy).\n\n"
        "NOMES (" + std::to_string(sizeof(kBiosVars) / sizeof(kBiosVars[0])) + ")\n" + SymbolColumns(kBiosVars) +
        "\nEXEMPLO\n      .biosvars\n      ld a,[SCRMOD]   ; modo de tela atual\n\nVER TAMBEM\n  fwasm, fwasm-bios, sysvars");
    return v;
}

} // namespace man
