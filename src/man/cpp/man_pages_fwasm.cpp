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
  conferida por mais de 1500 casos de teste gerados com o asmsx.exe de verdade).
  Aceita todas as instrucoes oficiais e as nao documentadas do Z80, expressoes
  com operadores e funcoes matematicas, macros, repeticao, montagem condicional,
  includes e as tabelas de nomes da BIOS do MSX.

USO
  fwmsx --asm [-z] [-s] [-vv] [-o <caminho>] <arquivo.asm>
  asm [-z] [-s] [-vv] [-o <caminho>] <arquivo.asm>     (no console e na TUI)

  -z   sintaxe Zilog padrao (parenteses para memoria), sem a diretiva .zilog
  -s   silencioso: sem mensagens de progresso
  -vv  mensagens detalhadas
  -o   nome-base das saidas (sem extensao), ou uma pasta terminada em / ou \
  -r   aceito e ignorado (os includes ja' sao relativos a pasta do fonte)

SAIDAS
  arquivo.z80   os bytes montados, do menor ao maior endereco gravado, SEM
                cabecalho (a extensao e' sempre .z80 por enquanto)
  arquivo.sym   tabela de simbolos (constantes, variaveis e rotulos)
  arquivo.txt   as mensagens das diretivas print (so' existe se houver print)
  Codigo de saida: 0 = ok; diferente de 0 = erro no fonte (a mensagem diz a
  linha); 2 = uso incorreto ou arquivo ilegivel.

O QUE JA' FUNCIONA E O QUE FALTA
  Pronto:   org page phase dephase zilog bios biosvars db defb dt deft dw defw
            ds defs byte word incbin include print printdec printhex printfix
            printtext printstring filename if ifdef ifndef else endif macro
            endm rept endr equ, variaveis (X=expr), rotulos locais, todas as
            expressoes e funcoes, a funcao .random(n).
  Parcial:  .start (valida a expressao; ainda nao muda o ponto de entrada).
  FALTA:    os formatos de saida .rom .megarom .basic .msxdos .cas .wav
            .sinclair, e select subpage search callbios calldos size debug
            break breakpoint. Sem eles a saida e' um bloco cru (.z80); veja o
            exemplo "cartucho de 16 KB" abaixo para rodar no emulador mesmo assim.

EXEMPLO: UM CARTUCHO DE 16 KB (testado)
        org 4000h
        db "AB"             ; assinatura de cartucho
        dw init             ; INIT
        ds 12               ; STATEXT, DEVICE, TEXT e reservado
  init: ld hl,msg
  loop: ld a,[hl]
        or a
        jr z,fim
        call 00A2h          ; CHPUT
        inc hl
        jr loop
  fim:  jr fim
  msg:  db "Ola do fwasm!",0

  Monte, renomeie o .z80 para .rom e rode:
    asm hello.asm
    (copie hello.z80 para hello.rom)
    emu start --cart hello.rom

TOPICOS DESTE MANUAL
  fwasm-syntax        formato da linha, comentarios, colchetes x parenteses
  fwasm-labels        rotulos, rotulos locais, equ, variaveis, $
  fwasm-numbers       formatos numericos
  fwasm-operators     operadores, funcoes matematicas, ponto fixo
  fwasm-data          db dw ds dt byte word incbin include
  fwasm-directives    todas as diretivas, com o estado de cada uma
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
  As diretivas podem ser escritas com ou sem o ponto inicial. Coluna ESTADO:
  PRONTA = funciona no fwasm; PARCIAL; FALTA = existe no asMSX, ainda nao no
  fwasm (a montagem acusa erro de sintaxe).

  diretiva        ESTADO   O QUE FAZ
  zilog           PRONTA   parenteses = memoria, colchetes = contas (-z faz o mesmo)
  org X           PRONTA   muda o endereco atual
  page X          PRONTA   idem, em blocos de 16 KB (page 1 = 4000h)
  equ             PRONTA   define constantes:  nome equ expr
  nome = expr     PRONTA   variavel inteira reatribuivel
  bios            PRONTA   libera os nomes das rotinas da BIOS (fwasm-bios)
  biosvars        PRONTA   libera os nomes das variaveis (fwasm-biosvars)
  phase X/dephase PRONTA   monta para um endereco e guarda em outro
  db defb dt deft PRONTA   bytes e textos
  dw defw         PRONTA   palavras de 16 bits
  ds defs         PRONTA   reserva bytes
  byte word       PRONTA   reservam um byte / uma palavra
  include         PRONTA   inclui outro fonte
  incbin          PRONTA   inclui um binario (skip, size)
  print printdec  PRONTA   escreve o valor decimal de uma expressao no .txt
  printhex        PRONTA   idem, em hexadecimal
  printfix        PRONTA   idem, para um ponto fixo
  printtext       PRONTA   escreve um texto no .txt (printstring e' sinonimo)
  filename "x"    PRONTA   nome-base das saidas (o -o tem prioridade)
  if/ifdef/ifndef/else/endif   PRONTA   montagem condicional (fwasm-conditional)
  macro/endm, rept/endr        PRONTA   macros e repeticao (fwasm-macros)
  random(n)       PRONTA   funcao: inteiro aleatorio de 0 a n-1
  start X         PARCIAL  so' valida a expressao; o ponto de entrada ainda nao e'
                           gravado
  rom             FALTA    cabecalho de cartucho ROM (use "AB" + INIT a' mao)
  megarom [tipo]  FALTA    MegaROM: konami, konamiscc, ascii8, ascii16
  subpage n at X  FALTA    define uma subpagina do MegaROM
  select n at X   FALTA    seleciona uma subpagina do MegaROM
  search          FALTA    acha o slot da pagina 2 numa ROM que comeca na 1
  basic           FALTA    binario carregavel por BLOAD "x",R (extensao .bin)
  msxdos          FALTA    executavel .com do MSX-DOS (carregado em 0100h)
  callbios X      FALTA    chama uma rotina da BIOS de um programa do MSX-DOS
  calldos X       FALTA    chama uma funcao do MSX-DOS
  cas ["nome"]    FALTA    fita .cas para BLOAD "CAS:",R
  wav ["nome"]    FALTA    fita .wav para um MSX de verdade
  sinclair        FALTA    (quebrada ate' no asMSX original) arquivo .tap
  size X          FALTA    tamanho do arquivo de saida em KB
  debug "texto"   FALTA    texto visivel no depurador do blueMSX
  break [X]       FALTA    breakpoint do blueMSX (breakpoint e' sinonimo)

  (O depurador do proprio fwMSX tem breakpoints: man break.)

DETALHES
  .org X         todo o codigo seguinte e' montado a partir de X. Voltar para um
                 endereco menor e' permitido; a saida .z80 vai do menor ao maior
                 endereco gravado.
  .page X        page 0 = 0000h, 1 = 4000h, 2 = 8000h, 3 = 0C000h.
  .phase X       as instrucoes sao montadas para executar em X, mas guardadas no
                 endereco atual: util para codigo que fica na ROM e e' copiado
                 para a RAM. Os rotulos valem X em diante. .dephase volta ao
                 normal (org, page e subpage tambem).
  .incbin "f" skip X size Y   injeta um binario; skip e size sao opcionais.
  .print expr    escreve no arquivo .txt (nao na tela).

PREVISTO (quando as diretivas de formato existirem)
  .rom       Depois de .page/.org e antes de .start; escreve o cabecalho "AB".
  .megarom   Konami e KonamiSCC: subpaginas de 8 KB (ate' 256 KB / 512 KB);
             ASCII8: 8 KB (ate' 2 MB); ASCII16: 16 KB (ate' 4 MB). O codigo vive em
             subpaginas (.subpage n at X) e se troca com .select n at X.
  .basic     Cabecalho de arquivo binario do BASIC; precisa de .org; .start diz a
             entrada.
  .msxdos    Executavel .com (sempre em 0100h; dispensa .org).
  .search    Descobre o slot da pagina 2 (RSLREG, EXPTBL, ENASLT).

VER TAMBEM
  fwasm, fwasm-data, fwasm-conditional, fwasm-macros)MAN";

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
