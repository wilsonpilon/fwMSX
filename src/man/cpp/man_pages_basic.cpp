// fwMSX -- MAN: paginas do MSX-BASIC (dicionario de palavras reservadas e manual). ARQUIVO GERADO por
// tools/paleobasic/import_basic_help.py a partir de resource/paleobasic/src/editor/help/*.pbi -- nao editar a mao.
//
// Fontes dos textos (transcricao feita no projeto paleobasic): livro "Linguagem BASIC MSX" (Denise Santoro Cruz, Editora Aleph/
// Gradiente, 4a edicao, 1986) e "Manual MSX 2+ FM" (Ademir Carchano / Flavio Monaco, ACVS Eletronica). Obras de terceiros,
// citadas como referencia do projeto; ver doc/man-spec.md, secao "Origem e direitos".
#include "man_internal.h"

namespace man {

std::vector<Topic> BasicTopics() {
    std::vector<Topic> v;
    // nome, apelidos, categoria, uso, resumo, pagina
    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {
        v.push_back(Topic{name, aliases, category, usage, summary, page, false});
    };

    add("abs", "", "MSX-BASIC", "ABS (argumento)", "Fornece o valor absoluto de um numero.",
        R"MAN(TIPO: FUNCAO  (absolute)

  Fornece o valor absoluto de um numero.

FORMATO
  ABS (argumento)

EXEMPLO
  B=ABS(-2)

DESCRICAO
  Fornece o valor absoluto do argumento, ou da expressao X.
  ABS (X) = X   para todo X maior ou igual a zero.
  ABS (X) = -X  para todo X menor que zero.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ABS
    20 FOR F=1 TO 20
    30 A=100-INT(RND(-TIME)*200)
    40 PRINT A;"......";ABS(A)
    50 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 26.

VER TAMBEM
  basic, screens, cores)MAN");

    add("asc", "", "MSX-BASIC", "ASC (string)", "Fornece o codigo ASCII do primeiro caractere de uma dada string.",
        R"MAN(TIPO: FUNCAO  (ascii)

  Fornece o codigo ASCII do primeiro caractere de uma dada string.

FORMATO
  ASC (string)

EXEMPLO
  A=ASC("A")

DESCRICAO
  Fornece o codigo ASCII (American Standard Code for Interchange Information) do primeiro caractere de uma string especificada. A string ou variavel string deve ser colocada entre parenteses, nao podendo ser nula. Caso contrario, ocasionara uma mensagem de erro.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ASC
    20 A$=INKEY$
    30 IF A$="" THEN 20
    40 PRINT"ASC(";A$;")=";ASC(A$),
    50 GOTO 20

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 27.

VER TAMBEM
  basic, screens, cores)MAN");

    add("atn", "", "MSX-BASIC", "ATN (argumento)", "Fornece o valor do arco, em radianos, cuja tangente trigonometrica e igual ao argumento.",
        R"MAN(TIPO: FUNCAO  (arc tangent)

  Fornece o valor do arco, em radianos, cuja tangente trigonometrica e igual ao argumento.

FORMATO
  ATN (argumento)

EXEMPLO
  A=ATN(1)

DESCRICAO
  A funcao ATN fornece o arco-tangente (em radianos) do argumento, isto e, o angulo cuja tangente e igual ao valor do argumento. O limite deste resultado esta entre -pi/2 e pi/2. A expressao do argumento pode ser qualquer numero, mas o calculo de ATN sempre sera realizado em modo de precisao dupla.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ATN
    20 CLS
    30 SCREEN 2
    40 FOR F=-128 TO 127
    50 PSET(F+128,80-70*ATN((F)*6.2832/255))
    60 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 28.

VER TAMBEM
  basic, screens, cores)MAN");

    add("auto", "", "MSX-BASIC", "AUTO [numero inicial da linha] [,incremento]", "Gera, automaticamente, numeros de linhas a partir de um valor inicial especificado, com...",
        R"MAN(TIPO: INSTRUCAO  (auto)

  Gera, automaticamente, numeros de linhas a partir de um valor inicial especificado, com incremento tambem especificado.

FORMATO
  AUTO [numero inicial da linha] [,incremento]

EXEMPLO
  AUTO 100,5

DESCRICAO
  Este comando liga uma funcao automatica de numeracao de linha, para entrada de programas. Tudo o que voce tem a fazer e introduzir as instrucoes reais do programa. Voce pode especificar um numero de linha inicial e um incremento para ser usado entre os numeros de linha ou entao, simplesmente digitar AUTO e pressionar RETURN. Neste caso, a numeracao de linha iniciara na 10 e usara incremento 10. Cada vez que voce pressionar RETURN, o computador avancara para o numero seguinte de linha.

  Por exemplo:
  COMANDO         NUMERACAO DAS LINHAS DO PROGRAMA
  AUTO            10, 20, 30 ...
  AUTO 5,5        5, 10, 15...
  AUTO 100        100, 110, 120...
  AUTO 100,25     100, 125, 150...
  AUTO 0          0, 10, 20...
  AUTO 10         10, 20, 30...

  Para desligar a funcao AUTO, pressione CTRL + C ou CTRL + STOP. Nao sera considerada a linha em que for teclado o CTRL + C. Quando o comando AUTO traz com o numero da linha um asterisco (*), significa que a linha ja esta sendo usada. Se voce nao deseja reprogramar a linha, devera pressionar CTRL + STOP para desligar a funcao.

PROGRAMA EXEMPLO
    10 REM PROGRAMA AUTO
    20 PRINT"VOCE QUER ALTERAR O PROGRAMA A PARTIR DA LINHA 100? (S/N)?"
    30 A$=INKEY$
    40 IF A$="S" THEN 90
    50 IF A$="N" THEN 100
    60 GOTO 30
    90 AUTO 100,10
    100 PRINT"VOCE NAO ME ALTEROU."

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 29.

VER TAMBEM
  basic, screens, cores)MAN");

    add("base", "", "MSX-BASIC", "BASE (numero)", "Permite o acesso ao endereco inicial das tabelas utilizadas pelo processador de video.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (base)

  Permite o acesso ao endereco inicial das tabelas utilizadas pelo processador de video.

FORMATO
  BASE (numero)
  BASE (numero) = expressao

EXEMPLO
  BASE(10)

DESCRICAO
  O comando BASE permite que o usuario leia e reescreva na memoria o endereco inicial das tabelas utilizadas pelo VDP (Video Display Processor). Estas tabelas sao utilizadas pelo processador de video conforme o modo em que se esta trabalhando. A expressao deve ser um valor inteiro entre 0 e 65535 e o numero deve estar entre 0 e 19 (nao se utilizando os valores 1, 3, 4 e 16). As tabelas, cujos enderecos iniciais sao especificados por BASE, estao relacionadas a seguir:

  SCREEN 0:
  - 2 - Tabela do gerador de padroes utilizados no modo texto de 40 caracteres x 24 linhas

  SCREEN 1:
  - 5 - Tabela dos nomes utilizados no modo texto de 32 caracteres x 24 linhas
  - 6 - Tabela de cores utilizadas no modo texto de 32 caracteres x 24 linhas
  - 7 - Tabela do gerador de padroes utilizados no modo texto de 32 caracteres x 24 linhas
  - 8 - Tabela dos atributos de sprites utilizados no modo texto de 32 caracteres x 24 linhas
  - 9 - Tabela dos padroes de sprites utilizados no modo texto de 32 caracteres x 24 linhas

  SCREEN 2:
  - 10 - Tabela dos nomes utilizados no modo grafico em alta-resolucao
  - 11 - Tabela de cores utilizadas no modo grafico em alta-resolucao
  - 12 - Tabela do gerador de padroes utilizados no modo grafico em alta-resolucao
  - 13 - Tabela dos atributos de sprites utilizados no modo grafico em alta-resolucao
  - 14 - Tabela dos padroes de sprites utilizados no modo grafico em alta-resolucao

  SCREEN 3:
  - 15 - Tabela dos nomes utilizados no modo policromatico
  - 17 - Tabela do gerador de padroes utilizados no modo policromatico
  - 18 - Tabela dos atributos de sprites utilizados no modo policromatico
  - 19 - Tabela dos padroes de sprites utilizados no modo policromatico

PROGRAMA EXEMPLO
    10 REM PROGRAMA BASE
    20 FOR F=1 TO 15:PRINT BASE(F):NEXT F

NO MSX2+ (Manual MSX 2+ FM)
TIPO: FUNCAO  (base)

  Mostra o endereco do primeiro byte das tabelas do processador de video (VDP) ou escreve um endereco inicial nos registradores das tabelas do VDP - versao MSX2+, com mais registros de tabela (0 a 44) para as SCREENs 0 a 8.

FORMATO
  BASE (X) ou BASE (X) = Y

EXEMPLO
  PRINT BASE(0)

DESCRICAO
  (X) deve ser sempre uma expressao de numeros inteiros entre 0 e 44 que, dependendo da SCREEN utilizada, tem o significado dado pela tabela a seguir (SCREEN 0 a 8):
  - Tabela Nomes: SCREEN 0=0, 1=5, 2=10, 3=15, 4=20, 5=25, 6=30, 7=35, 8=40
  - Tabela Cores: SCREEN 1=6, 2=11, 3=16, 4=21, 5=26, 6=31, 7=36, 8=41
  - Tabela Caracteres: SCREEN 0=2, 1=7, 2=12, 3=17, 4=22, 5=27, 6=32, 7=37, 8=42
  - Tabela Atrib Sprites: SCREEN 1=8, 2=13, 3=18, 4=23, 5=28, 6=33, 7=38, 8=43
  - Tabela Formato Sprites: SCREEN 1=9, 2=14, 3=19, 4=24, 5=29, 6=34, 7=39, 8=44

  Quando se le um endereco com a funcao BASE (X), o valor de X pode variar entre 0 e 44. Para se escrever um valor nos registradores do VDP com a funcao BASE (X) = Y, o valor de X so pode variar entre 0 e 19, ou seja, somente as SCREENs 0, 1, 2 e 3 sao afetadas. Quando se troca um endereco na SCREEN 3, os enderecos das SCREENs 4, 5, 6, 7 e 8 tambem sao alterados. Os dados da SCREEN 8 valem tambem para as SCREENs 10, 11 e 12.

  OBS: Use esta funcao ou instrucao somente quando estiver bem familiarizado com o modo de trabalho do processador de video (VDP).

PROGRAMA EXEMPLO
    100 'Enche duas telas
    110 SCREEN0:WIDTH40:KEYOFF
    140 FORA=0TO959:PRINT"-";:NEXT
    150 BASE(0)=1024:SCREEN0
    160 FORA=0TO959:PRINT"+";:NEXT
    180 'Alterna as paginas de texto
    200 BASE(0)=0
    210 FORT=0TO100:NEXT
    220 BASE(0)=1024
    230 FORT=0TO100:NEXT
    240 GOTO200

    Outro exemplo: PRINT BASE(0)

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 30.

VER TAMBEM
  basic, screens, cores)MAN");

    add("beep", "", "MSX-BASIC", "BEEP", "Provoca um beep (sinal sonoro).",
        R"MAN(TIPO: INSTRUCAO  (beep)

  Provoca um beep (sinal sonoro).

FORMATO
  BEEP

EXEMPLO
  BEEP

DESCRICAO
  Utilizado para gerar um sinal sonoro. Produz o mesmo efeito que o CHR$(7).

PROGRAMA EXEMPLO
    0 REM programa BEEP
    10 FOR I = 0 TO 19
    20 BEEP
    30 NEXT I

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 31.

VER TAMBEM
  basic, screens, cores)MAN");

    add("bin$", "", "MSX-BASIC", "BIN$ (X)", "Transforma um dado numerico em uma expressao binaria na forma de uma string.",
        R"MAN(TIPO: FUNCAO  (binary dollar)

  Transforma um dado numerico em uma expressao binaria na forma de uma string.

FORMATO
  BIN$ (X)

EXEMPLO
  LPRINT BIN$(A)

DESCRICAO
  O comando BIN$ transforma um dado numerico (constante, variavel numerica ou variavel de matriz numerica) em uma expressao binaria na forma de uma string. Caso o numero seja negativo, primeiro calcula-se o valor (em decimal) subtraindo-se de 65536, e a partir do resultado desta operacao, faz-se a conversao para binario e a transforma posteriormente numa string.
  PRINT BIN$(115)
  1110011
  PRINT BIN$(-19)
  1111111111101101   ( 65536-19=65517 )

PROGRAMA EXEMPLO
    10 REM PROGRAMA BIN$
    20 FOR F=0 TO 20
    30 PRINT F;"........";BIN$(F)
    40 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 32.

VER TAMBEM
  basic, screens, cores)MAN");

    add("bload", "", "MSX-BASIC", "BLOAD \"nome do dispositivo [nome do arquivo]\" [R] [,deslocamento]", "Carrega um programa em linguagem de maquina e o executa (caso esta opcao esteja ativada).",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (binary load)

  Carrega um programa em linguagem de maquina e o executa (caso esta opcao esteja ativada).

FORMATO
  BLOAD "nome do dispositivo [nome do arquivo]" [R] [,deslocamento]

EXEMPLO
  BLOAD"CAS:EXEMPL",R

DESCRICAO
  Carrega um programa em linguagem de maquina do dispositivo especificado, em geral "CAS:" (cassete). O nome do arquivo deve ter no maximo 6 caracteres (os excedentes serao desprezados). Se o nome do arquivo for omitido, o primeiro programa, em linguagem de maquina, que for encontrado, sera carregado. Se R for especificado no comando, depois de carregado o programa, a execucao comecara automaticamente no endereco que havia sido indicado em BSAVE por ocasiao da gravacao. O programa sera armazenado no trecho de memoria especificado tambem em BSAVE pelos enderecos inicial e final. Se o deslocamento esta especificado, todos estes enderecos sao deslocados. O deslocamento deve ser um numero inteiro entre -32768 e 65535.

PROGRAMA EXEMPLO
    10 BLOAD "CAS:ALEPH"
    20 CLS:FOR I=0 TO 41
    30 PRINT HEX$(PEEK(60000!+I));" ";
    40 FOR T=1 TO 90:NEXT T
    50 NEXT I
    60 FOR T=1 TO 500:NEXT T
    70 DEFUSR=60006!
    80 X=USR(0):GOTO 80
    90 REM
    100 REM USAR EM CONJUNTO COM O PROGRAMA BSAVE

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 33.

VER TAMBEM
  basic, screens, cores)MAN");

    add("bsave", "", "MSX-BASIC", "BSAVE \"nome do dispositivo [nome do arquivo]\", endereco inicial,", "Grava programas em codigo de maquina extraidos de uma area especifica da memoria.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (binary save)

  Grava programas em codigo de maquina extraidos de uma area especifica da memoria.

FORMATO
  BSAVE "nome do dispositivo [nome do arquivo]", endereco inicial,
  endereco final, [endereco do inicio de execucao]

EXEMPLO
  BSAVE"CAS:DEMO",&HA100,&HA2EF

DESCRICAO
  Este comando e utilizado para gravar o conteudo de uma area da memoria em um dispositivo (em geral, "CAS:", onde CAS = cassete). O nome do arquivo deve ter no maximo 6 caracteres (os excedentes serao ignorados). O nome do arquivo pode ser tambem uma string nula. Os enderecos, inicial e final, indicam a area da memoria que deve ser gravada. Devem ser numeros inteiros entre -32768 e 65535. O endereco do inicio de execucao indica o endereco no qual o programa devera iniciar a execucao quando for carregado de volta ao computador pelo comando BLOAD com a opcao R ativada. Tambem deve ser um numero inteiro entre -32768 e 65535.

PROGRAMA EXEMPLO
    10 DATA 41,4C,45,50,48,FF,11,60,EA,1A
    20 DATA FE,FF,C8,21,00,00,CD,4D,00,23
    30 DATA E5,D5,11,BF,03,ED,52,D1,E1,38
    40 DATA F1,21,FF,FF,2B,7C,B5,28,FB,13
    50 DATA 18,DF
    60 FOR I=0 TO 41
    70 READ A$
    80 POKE 60000!+I,VAL("&H"+A$)
    90 NEXT I
    100 DEFUSR=60006!
    110 SCREEN 0
    120 X=USR(0)
    130 FOR T=1 TO 1000:NEXT T:CLS
    140 BSAVE "CAS:ALEPH",60000!,60042!,60006!

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 34.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call", "", "MSX-BASIC", "CALL nome do comando [, (argumentos)]", "Executa um comando contido num cartucho ROM.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (call)

  Executa um comando contido num cartucho ROM.

FORMATO
  CALL nome do comando [, (argumentos)]

EXEMPLO
  CALL START

DESCRICAO
  Serve para chamar um comando de uma memoria auxiliar em cartucho. Do mesmo modo que o comando PRINT pode ser substituido por ? e o comando REM pode ser substituido pelo ' , o comando CALL pode ser substituido pelo sinal - .

PROGRAMA EXEMPLO
    10 ' CALL
    20 PRINT"Voce quer formatar seu diskette (s/n)?"
    30 Z$=INPUT$(1)
    40 IF Z$="s" THEN CALL FORMAT
    50 IF Z$<>"n" THEN 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 35.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call audreg", "", "MSX-BASIC", "CALL AUDREG (n1,n2)", "Altera o som dos instrumentos da bateria.",
        R"MAN(TIPO: INSTRUCAO  (audio register)

  Altera o som dos instrumentos da bateria.

FORMATO
  CALL AUDREG (n1,n2)

EXEMPLO
  CALL AUDREG (24,0)

DESCRICAO
  <n1> devera ser um numero com os valores abaixo:
  de 00 a 07;
  de 14 a 24;
  de 32 a 40;
  de 48 a 56.

  Esse numero indicara o registrador a ser alterado. Valores de 8 a 13, de 25 a 31, de 41 a 47, e acima de 39, geram mensagem de erro. Note que essa numeracao e diferente da numeracao normal dos instrumentos.

  <n2> pode ser qualquer numero inteiro, entre 0 e 255, e indicara o valor do atributo para o registrador.

PROGRAMA EXEMPLO
    CALL MUSIC (1)
    PLAY #2,"@A15V0B!H8H16B!16S!HBB!HBHBB!HBS!HBC!HB"
    CALL AUDREG (24,00)
    PLAY #2,"@A15V0B!H8H16B!16S!HBB!HBHBB!HBS!HBC!HB"
    'Com este exemplo, voce notara a mudanca no som do HI-HAT e do CHIMBAU.

    10 CALL MUSIC (1)
    20 FOR F=0 TO 255
    30 CALL AUDREG (23,F)
    40 PLAY #2,"V15S16"
    50 NEXT F
    'Neste exemplo, e possivel perceber a variacao no som da CAIXA.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 46.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call bgm", "", "MSX-BASIC", "CALL BGM (n)", "Sincroniza o processador Z80 com os processadores de audio FM e PSG, evitando que os comandos,...",
        R"MAN(TIPO: INSTRUCAO  (background music)

  Sincroniza o processador Z80 com os processadores de audio FM e PSG, evitando que os comandos, apos uma instrucao PLAY, sejam executados antes do termino da musica.

FORMATO
  CALL BGM (n)

EXEMPLO
  CALL BGM (1)

DESCRICAO
  n pode ser 0 ou 1. Se for 1, o recurso estara desativado. Dessa forma o processador Z80 terminara sempre primeiro. Se for 0, o recurso sera ativado e os processadores executarao suas tarefas juntos. O default e 1.

PROGRAMA EXEMPLO
    'EXEMPLO SEM BGM
    10 CLS:LOCATE10,10:PRINT"EXEMPLO SEM BGM"
    20 CALL MUSIC (0,0,1):CALL BGM (1)
    30 PLAY#2,"V15@4104CDEFGAB>CDEFGAB":CLS
    'Neste exemplo, a tela sera limpa antes da musica terminar.

    'EXEMPLO COM BGM
    10 CLS:LOCATE10,10:PRINT"EXEMPLO COM BGM"
    20 CALL MUSIC (0,0,1):CALL BGM (0)
    30 PLAY#2,"V15@4104CDEFGAB>CDEFGAB":CLS
    'Neste outro exemplo, a tela so sera limpa apos todas as notas terem sido executadas.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 45.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call memini", "", "MSX-BASIC", "CALL MEMINI [(X)]", "Prepara uma area da memoria RAM para trabalhar como RAMDISK (disco em memoria).",
        R"MAN(TIPO: INSTRUCAO  (mem init)

  Prepara uma area da memoria RAM para trabalhar como RAMDISK (disco em memoria).

FORMATO
  CALL MEMINI [(X)]

EXEMPLO
  _MEMINI

DESCRICAO
  X e o tamanho do disco em memoria em bytes e tem de ser um numero inteiro entre 1023 e 32767, ou 0 (zero). CALL MEMINI (0) cancela o modo RAMDISK. Quando se executa a instrucao CALL MEMINI, todo o conteudo do disco em memoria sera limpo.

  No lugar da palavra CALL pode-se utilizar o sinal de sublinhado (_). Por exemplo: _MEMINI

  A instrucao CALL MEMINI deve ser executada antes das instrucoes: CALL MFILES, CALL MNAME ou CALL MKILL.

  As seguintes instrucoes podem ser utilizadas apos a instrucao CALL MEMINI para se trabalhar com arquivos em RAMDISK: SAVE, LOAD, RUN, MERGE, OPEN, CLOSE, PRINT#, PRINT#USING, INPUT#, LINEINPUT#, EOF, LOC, LOF.

  A RAMDISK deve ser tratada como se fosse um disquete normal. O unico detalhe e que o nome do dispositivo deve ser MEM:.

PROGRAMA EXEMPLO
    _MEMINI
    10 PRINT "TESTE "; : GOTO 10
    SAVE "MEM:TESTE.BAS"
    NEW
    LIST
    LOAD "MEM:TESTE.BAS"
    LIST
    RUN

FONTE
  Manual MSX 2+ FM (ACVS), pagina 29.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call mfiles", "", "MSX-BASIC", "CALL MFILES", "Lista na tela os nomes dos arquivos armazenados na RAMDISK e quantos bytes ainda estao livres...",
        R"MAN(TIPO: INSTRUCAO  (mem files)

  Lista na tela os nomes dos arquivos armazenados na RAMDISK e quantos bytes ainda estao livres para uso.

FORMATO
  CALL MFILES

EXEMPLO
  _MFILES

DESCRICAO
  No lugar da palavra CALL pode-se utilizar o sinal de sublinhado (_). Por exemplo: _MFILES. Este comando so sera aceito depois da execucao da instrucao CALL MEMINI. Caso a instrucao CALL MEMINI nao tenha sido executada ou a RAMDISK tenha sido eliminada pela instrucao CALL MEMINI (0), sera emitida a mensagem de erro "disk offline".

PROGRAMA EXEMPLO
    _MEMINI
    10 PRINT "TESTE "; : GOTO 10
    SAVE "MEM:TESTE.BAS"
    NEW
    _MFILES
    LOAD "MEM:TESTE.BAS"
    LIST
    RUN

FONTE
  Manual MSX 2+ FM (ACVS), pagina 29.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call mkill", "", "MSX-BASIC", "CALL MKILL (\"nome do arquivo\")", "Apaga um arquivo armazenado na RAMDISK.",
        R"MAN(TIPO: INSTRUCAO  (mem kill)

  Apaga um arquivo armazenado na RAMDISK.

FORMATO
  CALL MKILL ("nome do arquivo")

EXEMPLO
  _MKILL "TESTE.BAS"

DESCRICAO
  No lugar da palavra CALL pode-se utilizar o sinal de sublinhado (_). Este comando so sera aceito depois da execucao da instrucao CALL MEMINI. Caso a instrucao CALL MEMINI nao tenha sido executada ou a RAMDISK tenha sido eliminada pela instrucao CALL MEMINI (0), sera emitida a mensagem de erro "disk offline".

PROGRAMA EXEMPLO
    _MEMINI
    10 PRINT "TESTE "; : GOTO 10
    SAVE "MEM:TESTE.BAS"
    _MFILES
    _MKILL "TESTE.BAS"
    _MFILES

FONTE
  Manual MSX 2+ FM (ACVS), pagina 30.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call mname", "", "MSX-BASIC", "CALL MNAME(\"nome arquivo 1\" as \"nome arq 2\")", "Troca o nome de um arquivo gravado na RAMDISK.",
        R"MAN(TIPO: INSTRUCAO  (mem name)

  Troca o nome de um arquivo gravado na RAMDISK.

FORMATO
  CALL MNAME("nome arquivo 1" as "nome arq 2")

EXEMPLO
  _MNAME ("TESTE.BAS" AS "PROG.BAS")

DESCRICAO
  O "nome do arquivo 1" e o nome ja gravado na RAMDISK e o "nome do arquivo 2" e aquele que substituira o antigo. No lugar da palavra CALL pode-se utilizar o sinal de sublinhado (_). Este comando so sera aceito depois da execucao da instrucao CALL MEMINI. Caso a instrucao CALL MEMINI nao tenha sido executada ou a RAMDISK tenha sido eliminada pela instrucao CALL MEMINI (0), sera emitida a mensagem de erro "disk offline".

PROGRAMA EXEMPLO
    _MEMINI
    10 PRINT "TESTE "; : GOTO 10
    SAVE "MEM:TESTE.BAS"
    _MFILES
    _MNAME ("TESTE.BAS" AS "PROG.BAS")
    _MFILES

FONTE
  Manual MSX 2+ FM (ACVS), pagina 31.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call music", "", "MSX-BASIC", "CALL MUSIC (BT,0,C1,C2,C3,C4,...,[C8],[C9])", "Define os canais de som que serao utilizados, bem como se sera ou nao usada a bateria...",
        R"MAN(TIPO: INSTRUCAO  (music)

  Define os canais de som que serao utilizados, bem como se sera ou nao usada a bateria eletronica.

FORMATO
  CALL MUSIC (BT,0,C1,C2,C3,C4,...,[C8],[C9])

EXEMPLO
  CALL MUSIC (1,0,1,1,1,1,1)

DESCRICAO
  Com este comando e possivel tambem a concatenacao de canais, respeitando-se o limite disponivel (6 canais com bateria ou 9 canais sem bateria).

  O primeiro parametro <BT> pode ser 0 ou 1. Se 0, nao sera utilizada a bateria. Se 1, os 3 canais de bateria serao ativados.

  O segundo parametro deve ser obrigatoriamente 0. Qualquer valor diferente de 0 gerara uma mensagem de erro.

  Os parametros de <C1> a <C9> referem-se ao chaveamento de canais de som. Esses parametros devem conter um numero de 1 a 6 (ou de 1 a 9 dependendo da bateria). Esse numero indicara o maximo de vozes que sera tocado ou mixado nesse canal.

  Ex: CALL MUSIC (1,0,1,1,1,1,1). Com esse comando sera acionada a bateria e cada um dos 6 canais de som disponiveis podera tocar uma voz (instrumento) diferente.

  CALL MUSIC (0,0,2,1,1,1). Neste exemplo, nao sera utilizada a bateria e teremos 3 dos 9 canais disponiveis, sendo que o primeiro (que contem o no 2) podera tocar ate dois instrumentos mixados.

  E importante observar que a concatenacao de canais (como no exemplo acima) so pode ser feita respeitando-se o numero maximo de canais do FM. Sendo assim, a soma dos parametros (apenas dos canais) nao devera ultrapassar o limite de 6 (com bateria) ou 9 (sem bateria). Nesse caso: CALL MUSIC (1,0,2,2,2,2) Gerara uma mensagem de erro.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 38.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call pitch", "", "MSX-BASIC", "CALL PITCH (n)", "Altera a velocidade tonal da musica.",
        R"MAN(TIPO: INSTRUCAO  (pitch)

  Altera a velocidade tonal da musica.

FORMATO
  CALL PITCH (n)

EXEMPLO
  CALL PITCH (459)

DESCRICAO
  E util quando se deseja acertar a afinacao dos instrumentos pois atua sobre todos os canais. n pode ser um numero de 410 a 459 (410 mais lento, 459 mais rapido). O default e 440.

PROGRAMA EXEMPLO
    CALL MUSIC (0,0,1)
    PLAY #2,"V15C"
    CALL PITCH (459)
    PLAY #2,"V15C"
    CALL PITCH (410)
    PLAY #2,"V15C"

FONTE
  Manual MSX 2+ FM (ACVS), pagina 43.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call play", "", "MSX-BASIC", "CALL PLAY (nc,var)", "Indica se os canais de FM estao ou nao ativos (tocando).",
        R"MAN(TIPO: INSTRUCAO  (play)

  Indica se os canais de FM estao ou nao ativos (tocando).

FORMATO
  CALL PLAY (nc,var)

EXEMPLO
  CALL PLAY (3,A):PRINT A

DESCRICAO
  <nc> e o numero do canal que se deseja saber se esta ou nao tocando. Se <nc> for 0 sera possivel saber se existe algum canal ativo. Enquanto um deles estiver tocando o valor devolvido sera -1, caso contrario, sera 0. Com <nc> de 1 a 9, voce sabera apenas o estado (ativo ou nao) do canal escolhido.

  <var> e a variavel numerica onde sera colocado o valor devolvido. Ela contera 0 caso o(s) canal(is) nao esteja(m) tocando e, -1 se estiver(em) tocando.

PROGRAMA EXEMPLO
    CALL PLAY (3,A):PRINT A
    'Le o estado do canal 3 e retorna o resultado para a variavel "A". Como nao tocamos nota nenhuma, o valor devolvido sera 0.

    CALL MUSIC (0,0,1)
    PLAY #2,"V15C":CALL PLAY (0,A):PRINT A
    'Esse exemplo ira tocar uma nota enquanto le o estado do canal 1. Obviamente o valor devolvido sera -1.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 44.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call stopm", "", "MSX-BASIC", "CALL STOPM", "Determina uma parada na musica no ponto em que se encontra.",
        R"MAN(TIPO: INSTRUCAO  (stop music)

  Determina uma parada na musica no ponto em que se encontra.

FORMATO
  CALL STOPM

EXEMPLO
  CALL STOPM

DESCRICAO
  Como sabemos, o processador Z80 trabalha a uma velocidade maior que o processador de audio (PSG) e o FM. Portanto, quando executamos uma ou varias notas, algumas vezes, o controle e devolvido para o BASIC antes mesmo de haver terminado a sequencia de notas, o que faz com que os proximos comandos (no caso de um programa), apos a instrucao PLAY, sejam executados antes do termino da musica.

PROGRAMA EXEMPLO
    'EXEMPLO SEM STOPM
    10 CLS:LOCATE10,10:PRINT"EXEMPLO SEM STOPM"
    20 CALL MUSIC (0,0,1)
    30 PLAY#2,"V15@4104CDEFGAB>CDEFGAB"
    40 CLS
    'Ao executar esse programa voce notara que a tela sera limpa antes de terminar a musica.

    'EXEMPLO COM STOPM
    10 CLS:LOCATE10,10:PRINT"EXEMPLO COM STOPM"
    20 CALL MUSIC (0,0,1)
    30 PLAY#2,"V15@4104CDEFGAB>CDEFGAB"
    40 CALL STOPM
    50 CLS
    'Nesse exemplo a tela sera limpa simultaneamente a interrupcao da musica.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 45.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call temper", "", "MSX-BASIC", "CALL TEMPER (n)", "Serve para afinar os instrumentos.",
        R"MAN(TIPO: INSTRUCAO  (temper)

  Serve para afinar os instrumentos.

FORMATO
  CALL TEMPER (n)

EXEMPLO
  CALL TEMPER (0)

DESCRICAO
  n deve ser um numero entre 0 e 21.

PROGRAMA EXEMPLO
    CALL MUSIC (0,0,1)
    CALL TEMPER (0)
    PLAY #2,"@12L8CDEFGAB"
    CALL TEMPER (21)
    PLAY #2,"@12L8CDEFGAB"

    'Note que o som gerado pelo segundo exemplo esta "desafinado".

FONTE
  Manual MSX 2+ FM (ACVS), pagina 44.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call transpose", "", "MSX-BASIC", "CALL TRANSPOSE (n)", "Semelhante ao PITCH, porem, cobre uma faixa maior de valores.",
        R"MAN(TIPO: INSTRUCAO  (transpose)

  Semelhante ao PITCH, porem, cobre uma faixa maior de valores.

FORMATO
  CALL TRANSPOSE (n)

EXEMPLO
  CALL TRANSPOSE (0)

DESCRICAO
  n pode ser qualquer numero entre -12799 e 12799 sendo a cada 100 valores aumentado (ou diminuido) meio tom. O default e 0.

PROGRAMA EXEMPLO
    CALL MUSIC (0,0,1)
    CALL TRANSPOSE (0)
    PLAY #2,"V15C"
    CALL TRANSPOSE (-500)
    PLAY #2,"V15C"
    CALL TRANSPOSE (500)
    PLAY #2,"V15C"

    'Digitando esses comandos voce notara a mudanca entre diferentes valores do TRANSPOSE.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 43.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call voice", "", "MSX-BASIC", "CALL VOICE (@I1,@I2,@I3,@I4,......,[@I8],[@I9])", "Define os instrumentos que serao utilizados em cada canal.",
        R"MAN(TIPO: INSTRUCAO  (voice)

  Define os instrumentos que serao utilizados em cada canal.

FORMATO
  CALL VOICE (@I1,@I2,@I3,@I4,......,[@I8],[@I9])

EXEMPLO
  CALL VOICE (@00,@03,@43)

DESCRICAO
  Estao disponiveis 64 instrumentos. Os de 0 a 62 sao os instrumentos normais (pre-definidos) e o 63 e o instrumento programavel que nao possui som algum. Os labels de <I1> a <I9> deverao ser substituidos pelos numeros dos instrumentos. Deverao ser sempre precedidos por "@".

  Ex: CALL VOICE (@00,@03,@43). Esse comando definiu a utilizacao de PIANO no canal 1, FLAUTA no canal 2 e VIOLINO no canal 3.

  CALL VOICE (@02,@16,@01,@01,@01). Nesse exemplo ficou definida a utilizacao de CORDAS no canal 1, VIBRAPHONE no canal 2, e PIANO II nos canais 3, 4 e 5.

  LEMBRE-SE que o numero de canais depende da instrucao CALL MUSIC e que, caso seja definido algum canal com concatenacao, o comando CALL VOICE devera conter os instrumentos que serao usados em cada canal concatenado. Assim, ao se executar: CALL MUSIC (0,0,3):CALL VOICE (@00,@36,@02). Voce ira tocar a nota Do (C) com PIANO, FLAUTA e CORDAS mixados, no primeiro (e unico) canal.

  OBS: No APENDICE C, ha uma relacao com todos os instrumentos pre-definidos.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 39.

VER TAMBEM
  basic, screens, cores)MAN");

    add("call voicecopy", "", "MSX-BASIC", "CALL VOICECOPY (@ins,var%)", "Copia os valores dos registradores de som (definicao) de cada instrumento para uma variavel de...",
        R"MAN(TIPO: INSTRUCAO  (voice copy)

  Copia os valores dos registradores de som (definicao) de cada instrumento para uma variavel de precisao dupla. Permite tambem que passemos uma definicao de instrumento (que sao as informacoes necessarias para a composicao do som) contida numa variavel para os registradores do instrumento 63 (programavel).

FORMATO
  CALL VOICECOPY (@ins,var%)

EXEMPLO
  CALL VOICECOPY (@1,A%)

DESCRICAO
  A primeira sintaxe demonstra como colocar a definicao de um instrumento (indicado por <ins>, de 0 a 63) numa variavel de precisao dupla (indicada por <var>, seguida do simbolo de porcentagem).

  Alguns instrumentos, por motivos desconhecidos, nao sao visualizados (geram mensagem de erro).

  CALL VOICECOPY (<var>%,@63)

  Neste outro exemplo, o instrumento 63 recebera os valores contidos na variavel <var>. Os valores dos registradores estao divididos em 16 bytes de 16 bits cada, portanto, deverao ser tratados em blocos de 16 bytes (isso sera melhor explicado no APENDICE A).

PROGRAMA EXEMPLO
    10 CALL MUSIC:Y=0
    20 CLEAR 1000:DIM A%(15)
    30 CALL VOICECOPY (@1,A%)
    40 FOR X=0 TO 3
    50 PRINT RIGHT$("0000"+HEX$(A%(X+Y)),4)" ";
    60 NEXT X:Y=Y+4:PRINT:IF Y=16 THEN END ELSE 40
    'No exemplo acima e possivel ver na tela a definicao hexadecimal do instrumento 1 (PIANO II).

    10 CALL MUSIC (0,0,3)
    20 CLEAR 2000:DIMA%(15)
    30 FOR I=0 TO 15
    40 READ A$:A%(I)=VAL("&H"+A$)
    50 NEXT I
    60 DATA 6847,736F,2074,2020
    70 DATA 0000,00FF,0000,0000
    80 DATA 1572,01AA,0000,0000
    90 DATA 0131,02A0,0000,0000
    100 CALL VOICECOPY (A%,@63):POKE &HFA3C,40:POKE &HFA3C,80
    110 PLAY#2,"V15@6303G1D1F1C1"
    'Neste outro exemplo o instrumento 63 recebeu uma definicao (contida nas linhas DATA), e foi acionado (tocado).

    OBS: Somente o instrumento 63 pode receber uma definicao do usuario. Os outros instrumentos, que sao pre-programados, geram uma mensagem de erro.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 47.

VER TAMBEM
  basic, screens, cores)MAN");

    add("cdbl", "", "MSX-BASIC", "CDBL (argumento)", "Converte um dado numerico no formato \"precisao simples\" em um dado no formato de \"precisao...",
        R"MAN(TIPO: FUNCAO  (change double)

  Converte um dado numerico no formato "precisao simples" em um dado no formato de "precisao dupla".

FORMATO
  CDBL (argumento)

EXEMPLO
  A#=CDBL(B!/2)

DESCRICAO
  Esta funcao converte o argumento para o formato em precisao dupla. O valor resultante apresenta um formato com 17 digitos. CDBL pode ser util, quando se quer forcar uma operacao a ser efetuada em precisao dupla, mesmo que os operandos sejam de precisao simples ou inteiros.

PROGRAMA EXEMPLO
    10 REM PROGRAMA CDBL
    20 A!=20
    30 B=3.141592#
    40 C=CDBL(A!/B)
    50 PRINT A!,B,C

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 36.

VER TAMBEM
  basic, screens, cores)MAN");

    add("chr$", "", "MSX-BASIC", "CHR$ (argumento)", "Fornece o caractere correspondente ao codigo especificado.",
        R"MAN(TIPO: FUNCAO  (character dollar)

  Fornece o caractere correspondente ao codigo especificado.

FORMATO
  CHR$ (argumento)

EXEMPLO
  A$=CHR$(65)

DESCRICAO
  Realiza o inverso da funcao ASC, isto e, fornece o caractere correspondente ao codigo ASCII especificado. O argumento pode ser qualquer numero de 0 a 255, ou qualquer expressao variavel com um valor nesta faixa. O argumento ou expressao deve estar entre parenteses.

PROGRAMA EXEMPLO
    10 ' TABLE DE CARACTERES
    20 DATA 0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
    30 SCREEN 2,,,,1
    40 OPEN"GRP:"FOR OUTPUT AS 1
    50 FOR F=0 TO 255
    60 L=F\16:C=4+F-16*L
    70 PSET (11*C,13+11*L),1
    80 IF F<32 THEN PRINT #1,CHR$(1)+CHR$(64+F)
    90 IF F>31 THEN PRINT #1,CHR$(F)
    100 NEXTF
    110 FOR F=0 TO 187 STEP 11
    120 LINE(31,F)-(218,F)
    130 NEXT F
    140 FOR F=31 TO 218 STEP 11
    150 LINE(F,0)-(F,187)
    160 NEXT F
    170 FOR F=45 TO 217 STEP 11
    180 C=C+1:READ A$
    190 PSET(F+1,2),1:PRINT#1,A$
    200 PSET(F+1,3),1:PRINT#1,A$
    210 PSET(F,2),1:PRINT#1,A$
    220 PSET(F,3),1:PRINT#1,A$
    230 PSET(35,F-32),1:PRINT#1,A$
    240 PSET(35,F-31),1:PRINT#1,A$
    250 PSET(34,F-32),1:PRINT#1,A$
    260 PSET(34,F-31),1:PRINT#1,A$
    270 NEXT F
    280 GOTO 280

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 37.

VER TAMBEM
  basic, screens, cores)MAN");

    add("cint", "", "MSX-BASIC", "CINT (argumento)", "Converte dados numericos em numeros inteiros.",
        R"MAN(TIPO: FUNCAO  (convert to interger)

  Converte dados numericos em numeros inteiros.

FORMATO
  CINT (argumento)

EXEMPLO
  A%=CINT(B#*2)

DESCRICAO
  A funcao CINT (X) fornece o maior numero inteiro possivel menor que o valor do argumento especificado. Por exemplo:
  CINT (1.5) resulta em 1
  CINT (-1.5) resulta em -2.
  Para a funcao CINT, o argumento deve estar entre os limites de -32768 e +32767, caso contrario, ocasionara um erro de overflow. CINT pode ser utilizado para acelerar operacoes envolvendo operandos de precisao simples e dupla.

PROGRAMA EXEMPLO
    10 REM PROGRAMA CINT
    20 CLS
    30 FOR F=1 TO 20
    40 A=500-RND(-TIME)*1000
    50 PRINT A;"............";CINT(A)
    60 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 38.

VER TAMBEM
  basic, screens, cores)MAN");

    add("circle", "", "MSX-BASIC", "CIRCLE [STEP](X,Y), raio [,cor] [,angulo inicial] [,angulo final] [,proporcao]", "Traca, no modo grafico, um circulo, uma elipse, uma parte de um arco circular (ou um setor).",
        R"MAN(TIPO: INSTRUCAO  (circle)

  Traca, no modo grafico, um circulo, uma elipse, uma parte de um arco circular (ou um setor).

FORMATO
  CIRCLE [STEP](X,Y), raio [,cor] [,angulo inicial] [,angulo final] [,proporcao]

  (X,Y): Especifica as coordenadas que determinam o centro do circulo na tela. Podem ser constantes, variaveis ou variaveis indexadas, com valor entre -32768 e 32767. Se o STEP for especificado antes, a origem do sistema de coordenadas da tela (0,0) sera transferida do canto superior esquerdo para a posicao do ultimo ponto plotado.
  RAIO: Pode ser uma constante, variavel ou uma variavel indexada com valor entre -32768 a 32767, indicando o raio da figura a ser tracada.
  COR: Indica qual a cor com que a elipse sera desenhada. Deve ser um inteiro entre 0 e 15.
  Os parametros [angulo inicial] e [angulo final], sao medidos em radianos entre 0 e 2 PI (2pi),
  PROPORCAO: E a proporcao entre o eixo horizontal e vertical da elipse. Os dados podem ser: constantes, variaveis indexadas, numeros positivos ou suas expressoes. Se omitido, sera assumido o valor 1

EXEMPLO
  CIRCLE (128,86),70,10,0,3,4

DESCRICAO
  Este comando e utilizado para desenhar um circulo com a especificacao do raio e com seu centro nas coordenadas tambem especificadas. Quando se especificam os angulos (inicial e final), somente sera tracado uma parte do arco circular. O arco podera ser tracado colocando o sinal de menos ("-") para o valor dos angulos inicial e final. Uma elipse podera ser tracada especificando a "proporcao" (relacao altura/largura), isto e, o numero de vezes que o eixo horizontal esta contido na vertical.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ESFERA
    20 SCREEN2
    30 FOR B=80 TO 1 STEP -10
    40 E1=80/B
    50 E2=B/80
    60 CIRCLE (128,80),80,5,,,E1
    70 CIRCLE (128,80),80,5,,,E2
    80 NEXT B
    90 COLOR 5,1
    100 LINE (128,160)-(128,0)
    110 LINE (48,80)-(208,80)
    120 GOTO 100

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 39.

VER TAMBEM
  basic, screens, cores)MAN");

    add("circle, draw, line, paint, point, preset e pset", "circle,draw,line,paint,point,preset,pset", "MSX-BASIC", "CIRCLE, DRAW, LINE, PAINT, POINT, PRESET e PSET", "Nota geral do manual MSX2+: todas estas instrucoes passam a trabalhar com os novos valores de...",
        R"MAN(TIPO: INSTRUCAO  (circle, draw, line, paint, point, preset, pset)

  Nota geral do manual MSX2+: todas estas instrucoes passam a trabalhar com os novos valores de cores e coordenadas das telas graficas, e o comando PAINT ganha um segundo parametro de cor para a borda da figura.

EXEMPLO
  PAINT C1,C2

DESCRICAO
  Todas estas instrucoes passam a trabalhar com os novos valores das cores e coordenadas das telas graficas. Para obter os valores maximos das coordenadas, ver instrucao SCREEN (MSX2+). Para saber as cores que podem ser utilizadas veja as instrucoes COLOR (MSX2+) e SCREEN (MSX2+).

  Nas SCREENs de 5 em diante, os desenhos podem ter uma cor para o tracado e serem preenchidos com outra diferente. A instrucao PAINT passa a aceitar a seguinte sintaxe:
  PAINT C1,C2
  onde C1 e a cor que ira pintar a figura e C2, a cor do tracado ou borda da figura.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 23.

VER TAMBEM
  basic, screens, cores)MAN");

    add("clear", "", "MSX-BASIC", "CLEAR (tamanho da area de caracteres)[, (endereco da RAMTOP)]", "Inicializa todas as variaveis e estabelece o tamanho da area de caracteres e o ultimo endereco...",
        R"MAN(TIPO: INSTRUCAO  (clear)

  Inicializa todas as variaveis e estabelece o tamanho da area de caracteres e o ultimo endereco de memoria utilizada pelo BASIC. Tambem fecha todos os arquivos abertos.

FORMATO
  CLEAR (tamanho da area de caracteres)[, (endereco da RAMTOP)]

  TAMANHO DA AREA DE CARACTERES: Espaco para variavel string.

  ENDERECO DA RAMTOP: Especifica a area de memoria disponivel para ser usado pelo BASIC. Exemplo:

EXEMPLO
  CLEAR 400,55296

DESCRICAO
  Com esta instrucao, inicializa-se todas as variaveis e estabelece (quando especificado) o ultimo endereco da area de programacao BASIC, ou seja, o valor da RAMTOP.

PROGRAMA EXEMPLO
    10 REM Programa CLEAR
    20 CLEAR 200
    30 ON ERROR GOTO 90
    40 FOR F=1 TO 25
    50 A$=A$+"##########"
    60 PRINT"A$ esta com";10*F;"caracteres."
    70 NEXT F
    80 END
    90 PRINT"Com";10*F;"caracteres em A$, o micro nao"
    100 PRINT"consegue trabalhar e ocorre um erro:"
    110 PRINT CHR$(34);"Out of string space";CHR$(34)
    120 PRINT:PRINT"Agora sera executado um comando CLEAR  e o programa recomecara!"
    130 CLEAR 10000
    140 FOR G=1 TO 3000 : NEXT G
    150 GOTO 40

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 40.

VER TAMBEM
  basic, screens, cores)MAN");

    add("cload/cload?", "cload,cload?", "MSX-BASIC", "CLOAD [\"nome do arquivo\"]", "Carrega um programa em BASIC MSX de um cassete para a memoria do MSX.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (cassete load)

  Carrega um programa em BASIC MSX de um cassete para a memoria do MSX.

FORMATO
  CLOAD ["nome do arquivo"]
  CLOAD? ["nome do arquivo"]

EXEMPLO
  CLOAD"DEMO"
  CLOAD?"DEMO"

DESCRICAO
  O comando CLOAD possibilita a carga de um programa armazenado em fita cassete (colocando o gravador no modo PLAY). Certifique-se de que as ligacoes foram feitas corretamente e de que a fita foi rebobinada ate a posicao correta. O nome de arquivo pode ter no maximo 6 caracteres entre aspas. Se ele possuir mais do que 6 caracteres, os excedentes serao desprezados. No nome de arquivo podem ser utilizados quaisquer caracteres, exceto as proprias aspas. O BASIC permite especificar o nome de arquivo desejado, por exemplo, CLOAD "EXPERT". Isso fara com que o computador ignore qualquer programa, ate que ele encontre o programa rotulado "EXPERT". Enquanto o computador estiver procurando o arquivo "EXPERT", os nomes dos outros programas encontrados aparecerao da seguinte forma:

  Skip:GRADIE
  Skip:ALEPH
  Skip:MSX

  e, quando o arquivo "EXPERT" for encontrado aparecera:

  Found:EXPERT

  isto significa que "EXPERT" foi encontrado e esta sendo carregado. Se o nome do arquivo nao for especificado, o primeiro programa encontrado pelo computador sera carregado na memoria. CLOAD? permite a comparacao de um programa armazenado em fita cassete, com um programa na memoria do computador. Isto e util quando, apos gravar programas na fita (usando CSAVE), se deseja verificar se a transferencia foi bem sucedida. Durante o CLOAD? o programa na fita e o programa na memoria, sao comparados byte por byte. Se houver alguma irregularidade (indicando uma gravacao mal feita) o computador mandara a seguinte mensagem:

  Verify error

  Neste caso, voce devera gravar o programa novamente.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 41.

VER TAMBEM
  basic, screens, cores)MAN");

    add("close", "", "MSX-BASIC", "CLOSE [#] [numero do arquivo]", "Fecha um arquivo que foi aberto por um comando OPEN.",
        R"MAN(TIPO: INSTRUCAO  (close)

  Fecha um arquivo que foi aberto por um comando OPEN.

FORMATO
  CLOSE [#] [numero do arquivo]
  [, numero do arquivo]

EXEMPLO
  CLOSE#1

DESCRICAO
  Tem a funcao de fechar o acesso a um arquivo, atraves de um ou mais buffers especificados, liberando-o(s). Se o numero do arquivo nao for especificado, todos os canais abertos pelo comando OPEN serao fechados.

PROGRAMA EXEMPLO
    10 REM PROGRAMA CLOSE
    20 MAXFILES=1
    30 CLS
    40 OPEN "GRP:" FOR OUTPUT AS #1
    50 SCREEN 2
    55 LINE(0,0)-(255,191),5,BF
    56 CIRCLE(120,120),50
    60 PRINT #1,"EXPERT"
    70 FOR I=1 TO 1000:NEXT
    80 CLOSE 1
    90 SCREEN 0
    100 LIST

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 42.

VER TAMBEM
  basic, screens, cores)MAN");

    add("cls", "", "MSX-BASIC", "CLS", "Apaga tudo o que esta visualizado na tela.",
        R"MAN(TIPO: INSTRUCAO  (clear screen)

  Apaga tudo o que esta visualizado na tela.

FORMATO
  CLS

EXEMPLO
  CLS

DESCRICAO
  Esse comando tem a funcao de "limpar a tela". Desativa todos seus pontos graficos e move o cursor para o canto superior esquerdo. Muito util quando se deseja uma boa apresentacao visual. No modo grafico o comando COLOR sera executado apos um comando CLS.

PROGRAMA EXEMPLO
    10 REM PROGRAMA CLS
    20 INPUT"VOCE QUER LIMPAR A TELA (S/N)";R$
    30 IF R$="N" THEN 50
    40 CLS
    50 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 43.

VER TAMBEM
  basic, screens, cores)MAN");

    add("color", "", "MSX-BASIC", "COLOR [cor do 1o plano], [cor do fundo], [cor da borda]", "Especifica as cores do primeiro plano, fundo e area das bordas.",
        R"MAN(TIPO: INSTRUCAO  (color)

  Especifica as cores do primeiro plano, fundo e area das bordas.

FORMATO
  COLOR [cor do 1o plano], [cor do fundo], [cor da borda]

EXEMPLO
  COLOR 15,7,7

DESCRICAO
  Tem a funcao de definir a cor da tela que esta dividida em: borda, fundo, 1o plano.

  Para definir a cor, o argumento deve estar entre 0 a 15. As cores correspondentes a cada valor estao na tabela abaixo:

  - 0 - transparente
  - 1 - negro
  - 2 - verde
  - 3 - verde claro
  - 4 - azul escuro
  - 5 - azul claro
  - 6 - vermelho escuro
  - 7 - azul celeste
  - 8 - vermelho
  - 9 - vermelho claro
  - 10 - amarelo escuro
  - 11 - amarelo claro
  - 12 - verde escuro
  - 13 - magenta
  - 14 - cinza
  - 15 - branco

  Exemplo:

  COLOR 6           somente colocara a cor do primeiro plano

  COLOR ,2          somente colocara a cor do fundo

  COLOR ,,11        somente colocara a cor da area das bordas

  COLOR 15,1,1      cores de inicializacao

PROGRAMA EXEMPLO
    10 REM PROGRAMA COLOR
    20 CLS
    30 FOR F=1 TO 14
    40 FOR G=0 TO150
    50 NEXT G
    60 PRINT"XXXXXXXX ALEPH  &  GRADIENTE XXXXXXXX"
    70 COLORF,F-1,F+1
    80 NEXTF
    90 COLOR 15,1,1

NO MSX2+ (Manual MSX 2+ FM)
TIPO: INSTRUCAO  (color)

  Define as cores que serao utilizadas na tela nos modos texto e grafico - versao MSX2+, com paleta de 512 combinacoes e suporte aos sistemas de cor YJK (SCREEN 10/11/12).

FORMATO
  COLOR [X] [,Y] [,Z]

EXEMPLO
  COLOR 15,0,0

DESCRICAO
  X define a cor do caractere, Y define a cor do fundo da tela e Z define a cor da borda da tela. X, Y e Z devem ser numeros inteiros.

  Nas SCREENs 0, 1, 2, 3, 4, 5 e 7, os numeros X, Y e Z devem estar compreendidos entre 0 e 15.

  Nas SCREENs 5 e 7 cada byte guarda a cor de dois pontos em sequencia.

  A cor correspondente a cada numero na paleta de cores (numero - nome - intensidade Verm.,Verde,Azul):
  - 0 - transparente - 0,0,0
  - 1 - preta - 0,0,0
  - 2 - verde media - 1,6,1
  - 3 - verde clara - 3,7,3
  - 4 - azul escura - 1,1,7
  - 5 - azul clara - 2,3,7
  - 6 - vermelha escura - 5,1,1
  - 7 - ciano - 2,6,7
  - 8 - vermelha media - 7,1,1
  - 9 - vermelha clara - 7,3,3
  - 10 - amarela escura - 6,6,1
  - 11 - amarela clara - 6,6,4
  - 12 - verde escura - 1,4,1
  - 13 - magenta - 6,2,5
  - 14 - cinza - 5,5,5
  - 15 - branca - 7,7,7

  A intensidade do vermelho, verde e azul na paleta de cores pode ser alterada com a instrucao COLOR=.

  Na SCREEN 6, X, Y e Z devem estar entre 0 e 3. Neste modo grafico, se a cor da borda da tela estiver entre 0 e 3, sera respeitada a tabela da paleta de cores. Porem, se a cor estiver entre 16 e 31, sera feita uma combinacao das cores (a cor da borda passa a combinar duas cores da paleta: 16=cor0+cor0, 17=cor0+cor1, 18=cor0+cor2, 19=cor0+cor3, 20=cor1+cor0, 21=cor1+cor1, 22=cor1+cor2, 23=cor1+cor3, 24=cor2+cor0, 25=cor2+cor1, 26=cor2+cor2, 27=cor2+cor3, 28=cor3+cor0, 29=cor3+cor1, 30=cor3+cor2, 31=cor3+cor3).

  Na SCREEN 8, X, Y e Z devem estar entre 0 e 255. Neste modo grafico cada byte corresponde a cor de um ponto. Para saber o numero da cor de acordo com a intensidade de vermelho, verde e azul, use a equacao: G*32+R*4+B, onde G, R e B sao respectivamente as intensidades de verde, vermelho e azul. O verde e o vermelho podem variar entre 0 e 7. O azul so pode variar entre 0 e 3. A formacao do byte da cor na SCREEN 8 e: bit 7654 3210 = cor GGGRRRBB.

  Nas SCREEN 10, 11 e 12 e usado o sistema YJK para mostrar as cores. Neste modo especial, 4 (quatro) bytes sao usados para compor as cores de quatro pontos na mesma linha horizontal. Os (Y) sao a intensidade da cor (brilho) e podem variar de 0 a 31 (cada valor corresponde a 8 niveis na escala de intensidade: 0=0, 1=8, 2=16, ..., 31=248). Os (J e K) sao os vetores que determinam qual cor sera usada, podendo variar de -32 a 31 (6 bits).

  A conversao entre sistemas e feita com as equacoes:
  De YJK para RGB: R=Y+J, G=Y+K, B=5/4 Y - 1/2 J - 1/4 K
  De RGB para YJK: Y=B/2+R/4+G/8, J=R-Y, K=G-Y

  SCREEN 10 - as cores sao formadas no sistema YJK ou com a paleta normal de 16 cores, dependendo do bit 3 (A). Se for 0, as cores sao mostradas no sistema YJK (0 a 240 de intensidade), obtendo-se ate 12.499 variacoes possiveis. Se o bit 3 (A) for 1, as cores sao as da paleta de cores conforme os bits Y (0 a 15). Neste caso, os bits J e K sao ignorados.

  SCREEN 11 - as cores sao formadas como na SCREEN 10 so que as instrucoes graficas trabalham com valores para a cor entre 0 e 255.

  Quando nao for dado nenhum valor a X, Y ou Z, a instrucao COLOR utilizara os ultimos valores aplicados as cores dos caracteres, fundo e borda da tela.

PROGRAMA EXEMPLO
    Ex1:
    10 SCREEN7
    20 FORX=0TO511
    30 LINE(255,0)-(X,211),C
    40 C=C+1:IFC=16THENC=0
    50 NEXT
    60 GOTO60

    Ex2:
    10 SCREEN8
    20 FORX=0TO255
    30 LINE(0,0)-(X,211),X
    40 LINE(255,211)-(255-X,0),X
    50 NEXT
    60 GOTO60

    Ex3:
    100 SCREEN12
    110 Y=93
    120 FOR JH=0 TO 3
    130 FOR JL=0 TO 7
    140 FOR KH=0 TO 3
    150 FOR KL=0 TO 7
    160 LINE (X,Y)-(X,Y+2),KL,BF
    170 LINE (X+1,Y)-(X+1,Y+2),KH,BF
    180 LINE (X+2,Y)-(X+2,Y+2),JL,BF
    190 LINE (X+3,Y)-(X+3,Y+2),JH,BF
    200 X=X+4 : IFX=128 THEN X=0 : Y=Y-3
    210 NEXT KL,KH,JL,JH
    220 COPY (0,0)-(127,95) TO (128,0)
    230 COPY (0,0)-(255,95) TO (0,96)
    240 FOR X=0 TO 127 STEP 4
    250 LINE (X+1,0)-(X+1,191),4,,OR
    260 NEXT X
    270 FOR X=0 TO 255 STEP 4
    280 LINE (X+3,96)-(X+3,191),4,,OR
    290 NEXT X
    300 FOR N=0 TO 31
    310 LINE (0,0)-(255,191),7,BF,AND
    320 LINE (0,0)-(255,191),CC,BF,OR
    330 FOR T=0 TO 300 : NEXT T
    340 CC=CC+8
    350 NEXT N
    360 IF INKEY$="" THEN 360

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 44.

VER TAMBEM
  basic, screens, cores)MAN");

    add("color=", "", "MSX-BASIC", "COLOR=( X, R, G, B )", "Altera as intensidades do vermelho, verde e azul na paleta de cores.",
        R"MAN(TIPO: INSTRUCAO  (color equal)

  Altera as intensidades do vermelho, verde e azul na paleta de cores.

FORMATO
  COLOR=( X, R, G, B )

EXEMPLO
  COLOR=(1,7,0,0)

DESCRICAO
  Altera as intensidades do vermelho, verde e azul na paleta de cores. X e o numero da cor na paleta de cores e deve estar entre 0 e 15. R e a intensidade do vermelho entre 0 e 7. G e a intensidade do verde entre 0 e 7. B e a intensidade do azul e deve estar entre 0 e 7. A SCREEN 8 nao usa a paleta de cores.

PROGRAMA EXEMPLO
    100 SCREEN5
    110 OPEN"GRP:"AS#1
    120 FORN=0TO7
    130 PRESET(N*32+6,40):PRINT#1,"COR"
    140 PRESET(N*32+6,50)
    150 PRINT#1,N+1
    160 NEXT
    170 FORX=0TO7
    180 LINE(X*32,60)-(X*32+32,140),X+1,BF
    190 LINE(X*32+32,141)-(X*32+64,211),15-X,BF
    200 LINE(X*32+35,211)-(X*32+64,211),10,B
    210 NEXT
    220 PRESET(0,10)
    230 FORN=0TO7
    240 PRINT#1," RGB";
    250 NEXT
    260 FORC=0TO7
    270 COLOR=(C+1,R,G,B)
    280 R$=RIGHT$(STR$(R),1)
    290 G$=RIGHT$(STR$(G),1)
    300 B$=RIGHT$(STR$(B),1)
    310 PRESET(C*32+8,20):PRINT#1,R$;G$;B$
    320 B=B+1:IFB=8THENB=0:G=G+1:IFG=8THENG=0:R=R+1:IFR=8 THENR=0
    330 NEXT
    340 GOTO260

FONTE
  Manual MSX 2+ FM (ACVS), pagina 14.

VER TAMBEM
  basic, screens, cores)MAN");

    add("color=new", "", "MSX-BASIC", "COLOR = NEW", "Volta as intensidades de vermelho, verde e azul da paleta de cores aos seus valores originais.",
        R"MAN(TIPO: INSTRUCAO  (color equal new)

  Volta as intensidades de vermelho, verde e azul da paleta de cores aos seus valores originais.

FORMATO
  COLOR = NEW

EXEMPLO
  COLOR = NEW

DESCRICAO
  Os valores originais de vermelho, verde e azul sao aqueles mostrados na tabela junto a instrucao COLOR (MSX2+).

PROGRAMA EXEMPLO
    10 SCREEN 5 : COLOR 15,0,0
    20 OPEN "GRP:" AS #1
    30 FOR C=2 TO 15
    40 COLOR C
    50 PRINT#1,"ALTERACAO DAS CORES DA PALETA"
    60 NEXT C
    70 FOR N=2 TO 15
    80 R=INT(RND(1)*7)
    90 G=INT(RND(1)*7)
    100 B=INT(RND(1)*7)
    110 COLOR=(N,R,G,B)
    120 NEXT N
    130 FOR T=0 TO 1000 : NEXT T : BEEP
    140 COLOR = NEW
    150 IF INKEY$="" THEN 150

FONTE
  Manual MSX 2+ FM (ACVS), pagina 15.

VER TAMBEM
  basic, screens, cores)MAN");

    add("color=restore", "", "MSX-BASIC", "COLOR=RESTORE", "Transfere da memoria de video para a paleta de cores os valores das intensidades do vermelho,...",
        R"MAN(TIPO: INSTRUCAO  (color equal restore)

  Transfere da memoria de video para a paleta de cores os valores das intensidades do vermelho, verde e azul.

FORMATO
  COLOR=RESTORE

EXEMPLO
  COLOR=RESTORE

DESCRICAO
  Esta instrucao e usada quando os valores das intensidades do vermelho, verde e azul sao carregados para a memoria de video com a instrucao BLOAD"nome do arquivo",S.

  A tabela da paleta de cores pode ser encontrada nos seguintes enderecos na memoria de video:
  - SCREEN 0 (40 colunas): &H0400 - &H0420
  - SCREEN 0 (80 colunas): &H0F00 - &H0F20
  - SCREEN 1: &H2020 - &H2040
  - SCREEN 2: &H2020 - &H2040
  - SCREEN 3: &H2020 - &H2040
  - SCREEN 4: &H2020 - &H2040
  - SCREEN 5: &H76B0 - &H76A0
  - SCREEN 6: &H76B0 - &H76A0
  - SCREEN 7: &HFAB0 - &HFAA0
  - SCREEN 8: &HFAB0 - &HFAA0

  Nas SCREENs 5, 6, 7 e 8 pode-se trabalhar com mais de uma pagina de video (ver instrucao SETPAGE). Para descobrir o endereco da tabela da paleta de cores, use o seguinte calculo:
  SCREEN 5 e 6 --> numero da pagina * &H08000 + &H76B0
  SCREEN 7 e 8 --> numero da pagina * &H10000 + &HFAB0

PROGRAMA EXEMPLO
    10 SCREEN 0 : WIDTH 40 : COLOR 15,0,0
    20 LOCATE 10,10 : PRINT "COR 15 NORMAL"
    30 FOR T=0 TO 500 : NEXT : BEEP
    40 COLOR = (15,3,2,1)
    50 LOCATE 10,10 : PRINT "COR 15 ALTERADA"
    60 BSAVE "TESTECOR.PIC",&H0400,&H0420,S
    70 COLOR=NEW : BLOAD "TESTECOR.PIC",S
    80 FOR T=0 TO 500 : NEXT : BEEP
    90 COLOR=RESTORE

FONTE
  Manual MSX 2+ FM (ACVS), pagina 15.

VER TAMBEM
  basic, screens, cores)MAN");

    add("colorsprite", "", "MSX-BASIC", "COLORSPRITE (X) = Y", "Define a cor e algumas caracteristicas de um SPRITE.",
        R"MAN(TIPO: INSTRUCAO  (color sprite)

  Define a cor e algumas caracteristicas de um SPRITE.

FORMATO
  COLORSPRITE (X) = Y

EXEMPLO
  COLORSPRITE(1)=33

DESCRICAO
  Esta instrucao so funciona nos modos graficos de SCREEN 4 em diante. X e o numero do SPRITE e deve ser igual ao numero na instrucao SPRITE$ que define as caracteristicas do SPRITE. Y deve ser um numero inteiro compreendido entre 0 e 111. Tem o seguinte significado:
  - Entre 0 e 15, sao os numeros das cores da paleta de cores.
  - Somando 32 ao numero da cor escolhida, o SPRITE nao ativa mais o sensor de coincidencia de SPRITES. Ou seja, se este SPRITE colidir com outro na tela a instrucao ON SPRITE GOSUB nao sera executada.
  - Somando 64 ao numero da cor escolhida, o SPRITE nao ativa mais o sensor de coincidencia de SPRITE e faz uma operacao logica OR com as cores dos SPRITES que estao sobrepostos.

  As cores dos SPRITES podem ser determinadas pelas instrucoes: PUTSPRITE, COLORSPRITE ou COLORSPRITE$. A instrucao que for executada por ultimo e que determinara a cor do SPRITE. Quando for usar as instrucoes COLORSPRITE ou COLORSPRITE$, nao defina o numero da paleta de cores na instrucao PUTSPRITE. Para detalhes da instrucao PUT SPRITE, recorra ao manual do MSX1.

PROGRAMA EXEMPLO
    100 SCREEN5,1:COLOR15,0,0
    110 FORX=0TO15:LINE(X*16,40)-STEP(16,120),15-X,BF:NEXT
    120 B$=""
    130 FORI=1TO8:READA$:B$=B$+CHR$(VAL("&b"+A$)):NEXT
    140 SPRITE$(0)=B$
    150 SPRITE$(1)=B$
    160 COLORSPRITE(1)=33 'SOMBRA
    170COLORSPRITE$(0)=CHR$(12)+CHR$(10)+CHR$(5)+CHR$(8)+CHR$(3)+CHR$(3)
    180 FORI=0TO212
    190 PUTSPRITE0,(I,I),,0
    200 PUTSPRITE1,(I+B,I+B),,1
    210 NEXT
    220 GOTO220
    230 DATA 10011001
    240 DATA 00111100
    250 DATA 01111110
    260 DATA 11111111
    270 DATA 11111111
    280 DATA 01111110
    290 DATA 01000010
    300 DATA 10000001

FONTE
  Manual MSX 2+ FM (ACVS), pagina 16.

VER TAMBEM
  basic, screens, cores)MAN");

    add("colorsprite$", "", "MSX-BASIC", "COLORSPRITE$ (X) = Y$", "Define a cor das linhas de um SPRITE.",
        R"MAN(TIPO: INSTRUCAO  (color sprite dollar)

  Define a cor das linhas de um SPRITE.

FORMATO
  COLORSPRITE$ (X) = Y$

EXEMPLO
  COLORSPRITE$(0)=CHR$(12)+CHR$(10)+CHR$(5)+CHR$(8)+CHR$(3)+CHR$(3)

DESCRICAO
  Esta instrucao so funciona nos modos graficos de SCREEN 4 em diante. X e o numero do SPRITE e deve ser igual ao numero na instrucao SPRITE$ que define as caracteristicas do SPRITE. Y$ deve ser uma expressao com 1 ate 16 caracteres, cada um correspondendo a uma linha do SPRITE. O codigo dos caracteres, de acordo com o seu valor, tem o seguinte significado:
  - Entre 0 e 15, sao os numeros das cores da paleta de cores.
  - Somando 32 ao numero da paleta de cores, a linha do SPRITE nao ativa mais o sensor de coincidencia de SPRITES. Ou seja, se a linha do SPRITE colidir com outra na tela, a instrucao ON SPRITE GOSUB nao sera executada.
  - Somando 64 ao numero da paleta de cores, a linha do SPRITE nao ativa mais o sensor de coincidencia de SPRITES e faz uma operacao logica OR com as cores dos SPRITES que estao sobrepostas.
  - Ao se somar 128 ao numero da paleta de cores, a linha do SPRITE se deslocara 32 pontos para a esquerda.

  As cores dos SPRITES podem ser determinadas pelas instrucoes: PUTSPRITE, COLORSPRITE ou COLORSPRITE$. A instrucao que for executada por ultimo e que determinara a cor do SPRITE. Quando for usar as instrucoes COLORSPRITE ou COLORSPRITE$, nao defina o numero da paleta de cores na instrucao PUTSPRITE. Para detalhes da instrucao PUT SPRITE, recorra ao manual do MSX 1.0.

  Exemplo: ver exemplo na instrucao COLORSPRITE.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 18.

VER TAMBEM
  basic, screens, cores)MAN");

    add("cont", "", "MSX-BASIC", "CONT", "Continua a execucao de um programa interrompido.",
        R"MAN(TIPO: INSTRUCAO  (continue)

  Continua a execucao de um programa interrompido.

FORMATO
  CONT

EXEMPLO
  CONT

DESCRICAO
  Continua a execucao de um programa interrompido por uma instrucao END, STOP, ou mediante a digitacao de CONTROL + STOP. O comando CONT reinicia a execucao a partir da instrucao seguinte aquela que foi interrompida, a nao ser que a interrupcao tenha ocorrido durante uma instrucao INPUT. Nesse caso, CONT faz com que o INPUT seja executado desde o comeco.

PROGRAMA EXEMPLO
    10 ' CONT
    20 FOR F=1 TO 20
    30 PRINT TAB(F);F
    40 NEXT F
    50 PRINT"A execucao do programa foi interrompida por STOP."
    60 PRINT"Digite CONT+RETURN para continuar!!!"
    70 STOP
    80 CLS
    90 PRINT:PRINT"A execucao esta continuando!"
    100 FOR R=333 TO 0 STEP-1
    110 LOCATE 13,10
    120 PRINTUSING"###";R
    130 NEXT R
    140 PRINT TAB(13);"FIM!!!"
    150 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 45.

VER TAMBEM
  basic, screens, cores)MAN");

    add("copy", "", "MSX-BASIC", "COPY origem TO destino", "Copia dados de uma tela grafica para outro ponto da tela, para outra pagina de tela grafica,...",
        R"MAN(TIPO: INSTRUCAO  (copy)

  Copia dados de uma tela grafica para outro ponto da tela, para outra pagina de tela grafica, para uma matriz ou para um arquivo em disquete. Traz dados de uma matriz ou que estejam gravados em disquetes para a tela grafica. Copia um arquivo de um disquete para outro, ou faz copia do arquivo no mesmo disquete com outro nome.

FORMATO
  COPY origem TO destino
  COPY(X1,Y1)-[STEP](X2,Y2)[,XX] TO (X3,Y3)[,YY] [,op logico]
  COPY (X1,Y1) - [STEP] (X2,Y2) [,XX] TO matriz
  COPY(X1,Y1)-[STEP](X2,Y2)[,XX] TO "perif: nome arquivo"
  COPY matriz[,sentido] TO (X3,Y3)[,YY] [,operacao logica]
  COPY matriz TO "periferico: nome arquivo"
  COPY"perif:nome arq."[,sentido]TO (X3,Y3)[,YY]
  COPY "perif: nome arquivo1" TO "perif: nome arquivo2"

EXEMPLO
  COPY(0,0)-(127,95) TO (128,0)

DESCRICAO
  Esta instrucao so e valida nas SCREENs 5, 6, 7 e 8.

  X1 e Y1 sao as coordenadas na tela grafica do inicio do bloco que sera copiado.

  X2 e Y2 sao as coordenadas na tela grafica do fim do bloco que sera copiado. Se for usada a palavra STEP antes dos numeros (X2,Y2), estes numeros passam a ser a quantidade de pontos nos eixos X e Y que serao somados as coordenadas X1 e Y1, respectivamente. Os resultados das somas serao as coordenadas finais do bloco.

  X3 e Y3 sao as coordenadas de destino, ou seja, ponto na tela grafica para onde os dados comecarao a ser transferidos.

  X1, X2 e X3 sao numeros inteiros e podem variar entre 0 e 255 nas SCREENs 5 e 8, e entre 0 e 511 nas SCREENs 6 e 7.

  Y1, Y2 e Y3 podem variar entre 0 e 211 nas SCREENs 5,6,7 ou 8.

  Nas SCREENs 5, 6, 7 e 8 voce pode trabalhar com mais de uma pagina de video (ver instrucao SETPAGE). Nestes casos: XX e YY representam o numero da pagina de origem e o numero da pagina de destino respectivamente. Se XX e YY nao forem determinados, sera assumida a pagina que esta sendo usada.

  [Sentido]: determina o sentido em que os dados serao transferidos. Podem variar entre 0 e 3 e tem o seguinte significado:
  - 0 - da esquerda superior para a direita inferior.
  - 1 - da direita superior para a esquerda inferior.
  - 2 - da esquerda inferior para a direita superior.
  - 3 - da direita inferior para a esquerda superior.

  Matriz: e o nome da variavel (matriz numerica) que deve ter tamanho suficiente para receber os dados da tela grafica. O tamanho da matriz pode ser calculado com a seguinte equacao:
  INT ((pixel * (ABS(X2-X1)+1) * (ABS(Y2-Y1)+1) +7)/8)+4
  Pixel nas SCREENs 5, 7 e 8 vale quatro e na SCREEN 6 vale dois.

  [Operacao]: e uma operacao logica que pode ser feita entre as cores de origem e as cores de destino da area copiada. O resultado da operacao e visto na regiao de destino. As operacoes logicas podem ser:
  - OR - C2 = C1 + C2
  - XOR - C2 = NOT(C1) * C2 + C1 * NOT(C2)
  - AND - C2 = C1 * C2
  - PSET - C2 = C1
  - PRESET - C2 = NOT(C1)

  TOR, TXOR, TAND, TPSET e TPRESET, executam as mesmas operacoes logicas descritas acima com a diferenca de nao afetarem a cor transparente (0).

  C1 e o numero da cor de origem. C2 e o numero da cor de destino. NOT(cor) significa a inversao bit a bit do numero da cor.

  Periferico: (Perif) e a letra que indica qual controlador de disquete sera usado, podendo ser: A, B, C, D, E ou F dependendo do numero de interfaces de drives ligadas ao computador.

  Nome arquivo1 e nome arquivo2 sao, respectivamente, os nomes dos arquivos de origem e de destino que sao gravados no disquete.

PROGRAMA EXEMPLO
    100 SCREEN7,0:COLOR15,0,0
    110 X1=0:Y1=0:X2=64:Y2=32
    120 TM=INT((4*(ABS(X2-X1)+1)*(ABS(Y2-Y1)+1)+7)/8)+4
    130 DIM A(TM)
    140 FORX=1TO7
    150 CIRCLE(16,16),X*2-1,X:CIRCLE(16,16),X*2,X
    160 CIRCLE(48,16),X*2-1,8+X:CIRCLE(48,16),X*2,8+X
    170 NEXT
    180 FORC=1TO7
    190 COLOR=(C,0,0,7-C/2):COLOR=(C+8,7,0,0)
    200 NEXT
    210 COPY(0,0)-STEP(64,32)TOA
    220 FORN=1TO100
    230 X=RND(N)*256:Y=RND(N)*200
    240 COPYA,0TO(X,Y),,TPSET
    250 NEXT
    260 COPY(0,0)-(350,211)TO(200,0),,TPSET
    270 GOTO270

FONTE
  Manual MSX 2+ FM (ACVS), pagina 19.

VER TAMBEM
  basic, screens, cores)MAN");

    add("cos", "", "MSX-BASIC", "COS (argumento)", "Fornece o valor do cosseno de um arco em radianos.",
        R"MAN(TIPO: FUNCAO  (cosine)

  Fornece o valor do cosseno de um arco em radianos.

FORMATO
  COS (argumento)

EXEMPLO
  C=COS(3.14159265535898/2)

DESCRICAO
  A funcao COS (X) tem por finalidade fornecer o cosseno do argumento especificado que deve estar em radianos. A funcao COS e calculada em precisao dupla.

PROGRAMA EXEMPLO
    10 REM PROGRAMA COS
    20 CLS
    30 SCREEN 2
    40 FOR F=0 TO 255 STEP .5
    50 PSET(F,80-70*COS(F*6.141592#/255))
    60 NEXT F
    70 GOTO 70

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 46.

VER TAMBEM
  basic, screens, cores)MAN");

    add("csave", "", "MSX-BASIC", "CSAVE \"(nome do arquivo)\" [, velocidade de transmissao em bauds]", "Armazena, num cassete, um programa em BASIC MSX.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (cassete save)

  Armazena, num cassete, um programa em BASIC MSX.

FORMATO
  CSAVE "(nome do arquivo)" [, velocidade de transmissao em bauds]

  NOME DO ARQUIVO: Adota no maximo 6 caracteres, sendo que o primeiro deve obrigatoriamente ser um caractere alfabetico. Se forem especificados 7 ou mais caracteres, o setimo e os demais serao ignorados.

  VELOCIDADE DA TRANSMISSAO EM BAUDS: Especifica a velocidade de gravacao, adotando o seguinte criterio: 1 para 1200 bauds e 2 para 2400 bauds. Se a velocidade for omitida, sera adotada a velocidade 1 (=1200 bauds).

EXEMPLO
  CSAVE"PROG1"

DESCRICAO
  Gravar arquivos em fita cassete. A gravacao e feita no formato binario condensado, de modo que nao se pode usar o comando MERGE (ele so funciona para arquivos gravados em ASCII). A velocidade de transmissao de dados para o cassete tambem pode ser definida numa instrucao SCREEN.

PROGRAMA EXEMPLO
    10 CLS
    20 PRINT"ESTE PROGRAMA SE AUTO-COPIA"
    30 PRINT"APERTE (RETURN) PARA GRAVAR"
    40 INPUT B$
    50 CSAVE"ALEPH"
    60 INPUT"OUTRA GRAVACAO ";A$
    70 IF A$="S" OR A$="s" THEN 10
    80 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 47.

VER TAMBEM
  basic, screens, cores)MAN");

    add("csng", "", "MSX-BASIC", "CSNG (argumento)", "Converte um dado numerico para precisao simples.",
        R"MAN(TIPO: FUNCAO  (convert to single)

  Converte um dado numerico para precisao simples.

FORMATO
  CSNG (argumento)

EXEMPLO
  A!=CSNG(B#)

DESCRICAO
  Fornece uma representacao em precisao simples do argumento. Quando o argumento for um valor de precisao dupla, esta funcao fornecera um valor com seis digitos significativos e um arredondamento de 4/5 no digito menos significativo. Assim, temos:

  CSNG(0.666666666666667)     resulta em .666667
  e
  CSNG(0.333333333333333)     resulta em .333333

PROGRAMA EXEMPLO
    10 REM PROGRAMA CSNG
    20 A=12.15456723#
    30 PRINT A,CSNG(A)

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 48.

VER TAMBEM
  basic, screens, cores)MAN");

    add("csrlin", "", "MSX-BASIC", "CSRLIN", "Fornece a linha em que se encontra o cursor.",
        R"MAN(TIPO: FUNCAO  (cursor line)

  Fornece a linha em que se encontra o cursor.

FORMATO
  CSRLIN

EXEMPLO
  Y=CSRLIN

DESCRICAO
  A funcao CSRLIN fornece o numero da linha em que se encontra o cursor.

PROGRAMA EXEMPLO
    10 REM PROGRAMA CSRLIN
    20 CLS
    30 FOR F=1 TO 20
    40 A$=A$+"#"
    50 PRINT A$;CSRLIN
    60 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 49.

VER TAMBEM
  basic, screens, cores)MAN");

    add("data", "", "MSX-BASIC", "DATA constante [, constantes]", "Fornece os dados que serao lidos pelo comando READ.",
        R"MAN(TIPO: INSTRUCAO  (data)

  Fornece os dados que serao lidos pelo comando READ.

FORMATO
  DATA constante [, constantes]

EXEMPLO
  DATA 123,ABC,456

DESCRICAO
  A instrucao DATA e utilizada para armazenar as constantes numericas e as strings que serao acessadas pelo programa atraves da instrucao READ. O comando DATA nao e executavel, ou seja, quando um programa que esta sendo executado encontra-o, a instrucao e desprezada e passa-se a execucao da proxima instrucao. Assim, um comando DATA pode ser colocado em qualquer parte do programa. Pode conter numeros e strings separados por uma virgula. Um programa pode conter tantas instrucoes DATAs quantas forem necessarias. A instrucao READ acessara os dados contidos nas instrucoes DATA na ordem em que foram definidas (por ordem do numero da linha e pela sequencia dentro de uma instrucao). As strings que possuam uma virgula (,), dois-pontos (:) ou um espaco no comeco ou no final, devem estar entre aspas (" "). As constantes contidas nas instrucoes DATA devem estar de acordo com a variavel que a requisitara. O comando DATA pode ser lido novamente desde o comeco ou a partir da linha especificada com o uso do comando RESTORE.

PROGRAMA EXEMPLO
    10 REM PROGRAMA DATA
    20 DATA 0,1,3,5
    30 DATA 3,4,1,0
    40 DATA 4,6,0,9
    50 FOR F= 1 TO 12
    60 READ A
    70 PRINT A
    80 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 50.

VER TAMBEM
  basic, screens, cores)MAN");

    add("def fn", "", "MSX-BASIC", "DEF FN nome da funcao [(lista de parametros)] = (expressao da funcao definida)", "Define e atribui um nome a uma funcao escrita pelo usuario.",
        R"MAN(TIPO: INSTRUCAO  (define function)

  Define e atribui um nome a uma funcao escrita pelo usuario.

FORMATO
  DEF FN nome da funcao [(lista de parametros)] = (expressao da funcao definida)

EXEMPLO
  DEF FN ARCCOTH(X) = LOG((X+1)/(X-1))/2

DESCRICAO
  A instrucao DEF FN e utilizada para definir uma funcao que sera usada num programa. Nao pode ser utilizada como um comando direto. O nome da funcao a ser definida deve ser precedido pelo FN. A lista de parametros representa as variaveis que serao utilizadas pela funcao. Por exemplo:
  10 DEF FNE(X,Y,Z)=X+7*Y/Z

  Quando a funcao for chamada, cada parametro assumira o valor que lhe for passado. Por exemplo, em:
  20 A=FNE(4,16,8)

  X = 4, Y = 16 e Z = 8 e a funcao FNE e executada e retornara com o valor em A igual a 18. A lista de parametros podera conter no maximo 9 deles. A expressao da funcao a ser definida deve utilizar os parametros definidos. As variaveis do programa que contenham o mesmo nome dos parametros utilizados pela funcao nao sao afetadas. Porem pode-se utilizar na expressao da funcao uma variavel que nao foi definida na lista de parametros. Neste caso, quando a funcao for chamada, ela assumira seu valor atual. Os valores passados para a funcao devem ser do mesmo tipo dos que foram especificadas na funcao. Se eles nao coincidirem, uma mensagem de erro sera enviada, indicando que o valor que foi passado para a funcao nao esta de acordo com o tipo definido na funcao. A expressao da funcao deve ser apenas uma linha. A funcao deve ser definida antes de ser chamada. Caso contrario, sera enviada uma mensagem de erro.

PROGRAMA EXEMPLO
    10 REM PROGRAMA DEF FN
    20 PRINT" FORMULA DO DELTA"
    30 DEF FN D(A)=B*B-4*A*C
    40 INPUT"A=";A
    50 INPUT"B=";B
    60 INPUT"C=";C
    70 PRINT"DELTA=";FN D(A)

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 51.

VER TAMBEM
  basic, screens, cores)MAN");

    add("defint/defsng/defdbl/defstr", "defint,defsng,defdbl,defstr", "MSX-BASIC", "DEFINT caractere [-caractere] [, caractere]", "Define a correspondencia entre o primeiro caractere de uma variavel e o tipo de numero que ela...",
        R"MAN(TIPO: INSTRUCAO  (define integer) (define single) (define double) (define string)

  Define a correspondencia entre o primeiro caractere de uma variavel e o tipo de numero que ela armazena.

FORMATO
  DEFINT caractere [-caractere] [, caractere]
  DEFSNG caractere [-caractere] [, caractere]
  DEFDBL caractere [-caractere] [, caractere]
  DEFSTR caractere [-caractere] [, caractere]

EXEMPLO
  DEFINT A,I-K

DESCRICAO
  Este tipo de comando estabelece que os nomes das variaveis que comecam com as letras especificadas deverao corresponder a um tipo especifico de variavel, ou seja:

  - DEFINT - define que as variaveis armazenarao numeros inteiros

  - DEFSNG - define que as variaveis armazenarao numeros de precisao simples

  - DEFDBL - define que as variaveis armazenarao numeros de precisao dupla

  - DEFSTR - define que as variaveis serao do tipo string.

  O tipo de variavel pode ser alterado durante um programa, desde que a prioridade quanto ao tipo de variavel seja respeitada. A prioridade utilizada pelo MSX, e a seguinte:
  - $ - variavel string
  - # - variavel de precisao dupla
  - ! - variavel de precisao simples
  - % - variavel inteira

  Por exemplo, uma variavel definida como inteira pode ser alterada para uma variavel de precisao simples ou dupla ou para uma variavel string, mas uma variavel string nao pode ser alterada.

PROGRAMA EXEMPLO
    10 REM PROGRAMA DEFINT
    20 DEFINT A,B,C
    30 A=3.141592#
    40 B=2.718281#
    50 C=22.45525#
    60 PRINT A,B,C

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 52.

VER TAMBEM
  basic, screens, cores)MAN");

    add("defusr", "", "MSX-BASIC", "DEFUSR [digito] = endereco para inicio de execucao", "Define um endereco para iniciar a execucao de uma sub-rotina em linguagem de maquina que sera...",
        R"MAN(TIPO: INSTRUCAO  (define user)

  Define um endereco para iniciar a execucao de uma sub-rotina em linguagem de maquina que sera chamada pelo comando USR.

FORMATO
  DEFUSR [digito] = endereco para inicio de execucao

EXEMPLO
  DEFUSR1=&HE00A

DESCRICAO
  Este comando especifica o endereco inicial para a execucao de uma sub-rotina em linguagem de maquina. A sub-rotina iniciara a execucao quando ela for chamada pela instrucao USR. O digito a ser especificado deve ser um numero inteiro entre 0 e 9 e corresponde ao numero da rotina USR a ser executada. Se o valor do digito for omitido, sera assumido o valor 0. O endereco de inicio da rotina deve ser um numero entre 0 a 65535. O endereco de inicio de sub-rotina pode ser redefinido posteriormente, ou seja, uma funcao DEF USR pode assumir valores diferentes durante a execucao de um programa.

PROGRAMA EXEMPLO
    10 DATA 3E,41,21,00,00,CD,4D,00,23,E5,D5,11,BF,03,ED,52,D1,E1,38,F1,C9
    20 FOR I=0 TO 20
    30 READ A$:POKE 60000!+I,VAL("&H"+A$)
    40 NEXT I
    50 SCREEN 0:CLS:PRINT "APERTE <RETURN> PARA EXECUTAR"
    60 INPUT B$
    70 DEF USR0=60000!
    80 X=USR0 (0)
    90 FOR T=1 TO 1000:NEXT T:CLS:END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 53.

VER TAMBEM
  basic, screens, cores)MAN");

    add("delete", "", "MSX-BASIC", "DELETE [numero da linha A] [-numero da linha B]", "Apaga as linhas de um programa.",
        R"MAN(TIPO: INSTRUCAO  (delete)

  Apaga as linhas de um programa.

FORMATO
  DELETE [numero da linha A] [-numero da linha B]

EXEMPLO
  DELETE 10-230
  DELETE 23
  DELETE -120

DESCRICAO
  Este comando tem a funcao de apagar as linhas de um programa que esta na memoria. Pode-se especificar uma linha individual ou uma sequencia de linhas.

  DELETE (linha A) - (linha B): apaga todas as linhas de programa comecando pela linha A e terminando na linha B (inclusive).

  DELETE (linha A): Apaga apenas a linha especificada.

  DELETE -(linha B): Apaga desde a primeira linha do programa ate a linha B (inclusive).

  Voce pode apagar a linha que acabou de introduzir, ou que acaba de ser evidenciada por uma mensagem de erro, digitando:
  DELETE .
  ou seja, ao inves de digitar um numero de linha, digitar ponto (.).

PROGRAMA EXEMPLO
    10 REM PROGRAMA DELETE
    20 FOR F=0 TO 500
    30 PRINT"ESTE PROGRAMA SE AUTO DESTROI"
    40 NEXT F
    50 DELETE -50

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 54.

VER TAMBEM
  basic, screens, cores)MAN");

    add("dim", "", "MSX-BASIC", "DIM nome da variavel (valor maximo de um sub-indice, ...)", "Definicao de uma ou mais variaveis do tipo matriz, especificando o tipo de matriz e a sua...",
        R"MAN(TIPO: INSTRUCAO  (dimension)

  Definicao de uma ou mais variaveis do tipo matriz, especificando o tipo de matriz e a sua dimensao.

FORMATO
  DIM nome da variavel (valor maximo de um sub-indice, ...)
  [,nome da variavel ...]

EXEMPLO
  DIM Y(15,20)
  DIM A$(2,3),C(4,8)

DESCRICAO
  Utilizada para declarar variaveis do tipo matriz, bem como o tipo de matriz que a variavel sera, alem da sua dimensao. Se durante a execucao de um programa for acessado um sub-indice maior do que o assumido, uma mensagem de erro sera apresentada:

  Subscript out of range

  Numa sentenca DIM podem ser declaradas mais de uma matriz, desde que separadas por virgula.

  VARIAVEIS DE MATRIZ MULTIDIMENSIONAIS: As variaveis de matriz multidimensionais sao geradas especificando-se dois ou mais valores maximos para o sub-indice. Por exemplo:
  DIM A(3,4,5)

  OMISSAO DA SENTENCA DIM: Quando se utilizar uma variavel do tipo matriz sem ter declarado uma sentenca DIM, o valor maximo do sub-indice sera assumido como 10.

PROGRAMA EXEMPLO
    10 REM PROGRAMA DIM
    20 DIM I(12)
    30 CLS:IA=1
    40 FOR F= 1 TO 12
    50 PRINT"INFLACAO DO MES";F;":"
    60 INPUT I(F)
    70 IA=IA*(I(F)+100)/100
    80 NEXT F
    90 CLS
    100 PRINT"INFLACAO ANUAL:";IA*100-100

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 55.

VER TAMBEM
  basic, screens, cores)MAN");

    add("draw", "", "MSX-BASIC", "DRAW \"sub-comandos\"", "Desenha figuras na tela grafica (SCREEN 2 ou 3), de acordo com o sub-comando especificado.",
        R"MAN(TIPO: INSTRUCAO  (draw)

  Desenha figuras na tela grafica (SCREEN 2 ou 3), de acordo com o sub-comando especificado.

FORMATO
  DRAW "sub-comandos"

EXEMPLO
  DRAW "U20D10L30R40"

DESCRICAO
  A instrucao DRAW desenha na tela de modo grafico a figura especificada pelo sub-comando. Sao eles:

  - Sn (n entre 0 e 255): Define a escala em que sera tracada uma reta, ou seja, (1/4 pontos para n = 1). O valor inicial e S4.

  - An (n entre 0 e 3): Gira o sistema de coordenadas de 90 em 90 graus, sendo que o valor inicial e A0.

  - Cn (n entre 0 e 15): Define a cor da linha que sera tracada. Se omitido, assumira C15 (15 = branco).
    - 0 - transparente
    - 1 - negro
    - 2 - verde
    - 3 - verde claro
    - 4 - azul escuro
    - 5 - azul claro
    - 6 - vermelho escuro
    - 7 - azul celeste
    - 8 - vermelho
    - 9 - vermelho claro
    - 10 - amarelo escuro
    - 11 - amarelo claro
    - 12 - verde escuro
    - 13 - magenta
    - 14 - cinza
    - 15 - branco

  - M x,y (x entre 0 e 255; y entre 0 e 191): Traca uma linha desde o ultimo ponto plotado (posicao atual) ate a posicao definida por x e y.

  - M +-dx, +-dy (dx entre 0 e 255; dy entre 0 e 191): Traca uma linha desde a posicao atual ate a posicao em que a coordenada x e x atual +-dx e a coordenada y e y atual +-dy.

  - Un: Traca uma linha vertical para cima desde a posicao atual ate uma distancia n, onde n e o numero de pontos definido pelo sub-comando S. Quando omitido assume o valor 1.

  - Dn: Traca uma linha vertical para baixo desde a posicao atual ate uma distancia n, onde n e o numero de pontos definido pelo sub-comando S. Quando omitido assume o valor 1.

  - Rn: Traca uma linha horizontal da esquerda para a direita desde a posicao atual ate uma distancia n, onde n e o numero de pontos definido pelo sub-comando S. Quando omitido assume o valor 1.

  - Ln: Traca uma linha horizontal da direita para a esquerda desde a posicao atual ate uma distancia n, onde n e o numero de pontos definido pelo sub-comando S. Quando omitido assume o valor 1.

  - En: E uma composicao dos sub-comandos U e R, ou seja, traca uma reta desde a posicao atual ate a posicao em que no eixo X a distancia varie da esquerda para a direita de n posicoes e no eixo Y a distancia varie de baixo para cima de n posicoes, onde n e o valor definido pelo sub-comando S. Quando omitido assume o valor 1.

  - Fn: E uma composicao dos sub-comandos D e R, ou seja, traca uma reta desde a posicao atual ate a posicao em que no eixo X a distancia varie da esquerda para a direita de n posicoes e no eixo Y a distancia varie de cima para baixo de n posicoes, onde n e o valor definido pelo sub-comando S. Quando omitido assume o valor 1.

  - Gn: E uma composicao dos sub-comandos D e L, ou seja, traca uma reta desde a posicao atual ate a posicao em que no eixo X a distancia varie da direita para a esquerda de n posicoes e no eixo Y a distancia varie de cima para baixo de n posicoes, onde n e o valor definido pelo sub-comando S. Quando omitido assume o valor 1.

  - Hn: E uma composicao dos sub-comandos U e L, ou seja, traca uma reta desde a posicao atual ate a posicao em que no eixo X a distancia varie da direita para a esquerda de n posicoes e no eixo Y a distancia varie de baixo para cima de n posicoes, onde n e o valor definido pelo sub-comando S. Quando omitido assume o valor 1.

  A posicao atual e armazenada apos um dos sub-comandos exceto S, A e C. Antes dos sub-comandos que alteram o valor da posicao atual pode-se utilizar os comandos:

  - B: Posiciona o cursor na coordenada especificada sem que a reta seja tracada. Por exemplo:
    BM 120,50
    posiciona o cursor na posicao de coordenadas (120,50).

  - N: Traca a reta sem modificar o valor da posicao atual. Por exemplo:
    NU30
    traca a reta, porem a posicao do cursor nao se modifica.

  O comando DRAW traca graficos utilizando os sub-comandos citados anteriormente. Pode-se utilizar varios sub-comandos dentro de um mesmo comando DRAW, sendo que eles serao executados na ordem em que foram definidos. Por exemplo:
  DRAW "BM20,40NU56R12"

  Os sub-comandos podem tambem ser definidos em uma variavel string. Por exemplo:
  S$ = "BM0,50NG34F34"
  e podera ser executado especificando-se, depois do comando DRAW, a variavel string. Por exemplo:
  DRAW S$

  Porem quando a variavel string for utilizada juntamente com outros sub-comandos do comando DRAW, ou seja, entre as aspas (" ") ela devera aparecer entre um X e um ponto e virgula (;). Por exemplo:
  DRAW "S6XS$;"

  Quando uma variavel numerica for utilizada dentro de um comando DRAW para expressar o valor do angulo, a cor ou a distancia, ela devera ser precedida pelo sinal de igual (=) e sucedida por um ponto e virgula (;). Por exemplo:
  N=3
  DRAW "BM45,60U=N;"

  Observacao: Antes do comando DRAW, deve-se executar a instrucao:
  SCREEN 2 ou SCREEN 3
  que correspondem as telas graficas.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 56.

VER TAMBEM
  basic, screens, cores)MAN");

    add("end", "", "MSX-BASIC", "END", "Termina a execucao de um programa, fecha todos os arquivos e retorna ao estado de espera de um...",
        R"MAN(TIPO: INSTRUCAO  (end)

  Termina a execucao de um programa, fecha todos os arquivos e retorna ao estado de espera de um comando direto.

FORMATO
  END

EXEMPLO
  END

DESCRICAO
  Tem a funcao de finalizar a execucao de um programa, fechando os arquivos abertos durante o programa e retornando ao estado de espera de um comando direto. E um comando diferente do comando STOP, pois o comando END nao ocasiona uma mensagem de BREAK na tela. Quando as sub-rotinas sao escritas logo abaixo do programa principal, costuma-se colocar um comando END como sendo a ultima linha do programa principal. Desta forma, evita-se que as sub-rotinas sejam executadas ao final da execucao do programa principal. O comando END pode ser colocado em qualquer parte do programa e quantas vezes forem necessarias. Por exemplo, quando uma condicao causa uma ramificacao, pode-se colocar um comando END ao final de cada ramificacao. O comando END no final de programa e opcional. Para continuar a execucao de um programa que foi parado por um comando END, deve-se digitar RUN ou GOTO pois, com o comando CONT, nao sera possivel o retorno a execucao.

PROGRAMA EXEMPLO
    10 ' END
    20 CLS:PRINT"Agora, o programa principal esta sendo executado."
    30 FOR F=333 TO 0 STEP -1
    40 LOCATE 10,7:PRINT USING"###";F
    50 NEXT F:CLS
    60 GOSUB 90
    70 PRINT"O comando END termina o programa, evitando que a sub-rotina da linha 90 seja executada novamente."
    80 END
    90 PRINT"Agora, a sub-rotina da linha 90 esta sendo executada."
    100 FOR F=0 TO 333
    110 LOCATE 10,7:PRINT USING "###";F
    120 NEXT F
    130 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 59.

VER TAMBEM
  basic, screens, cores)MAN");

    add("eof", "", "MSX-BASIC", "EOF (numero do arquivo)", "Utilizado para testar o fim de um arquivo.",
        R"MAN(TIPO: FUNCAO  (end of file)

  Utilizado para testar o fim de um arquivo.

FORMATO
  EOF (numero do arquivo)

EXEMPLO
  IF EOF(1) THEN CLOSE#1:END

DESCRICAO
  EOF e utilizado como uma variavel para o controle de fim de arquivo. Quando um comando READ e executado e um registro foi lido do arquivo, EOF resulta em 0. Mas se o arquivo nao possuia mais nenhum registro, EOF resulta em -1. EOF tambem e utilizado para testar o final de um arquivo e impedir erros do tipo:
  Input past end
  ou seja, entrada depois de um fim de arquivo.

PROGRAMA EXEMPLO
    10 REM PROGRAMA EOF
    20 OPEN"cas:II" FOR INPUT AS #1
    30 IF EOF(1) THEN 70
    40 INPUT #1,A$
    50 GOTO 30
    60 CLOSE #1

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 60.

VER TAMBEM
  basic, screens, cores)MAN");

    add("erase", "", "MSX-BASIC", "ERASE nome da variavel [, nome da variavel ...]", "Apaga, da memoria, as variaveis do tipo matriz especificadas na instrucao.",
        R"MAN(TIPO: INSTRUCAO  (erase)

  Apaga, da memoria, as variaveis do tipo matriz especificadas na instrucao.

FORMATO
  ERASE nome da variavel [, nome da variavel ...]

EXEMPLO
  ERASE C,D

DESCRICAO
  Libera a memoria apenas na area utilizada pelas variaveis do tipo matriz especificadas na instrucao ERASE. A area liberada pode ser utilizada para outros propositos, ou entao para um novo redimensionamento das variaveis apagadas. Se for feita uma tentativa de redimensionamento de uma variavel que nao foi apagada da memoria, ocorrera um erro do tipo:
  Redimensioned array

PROGRAMA EXEMPLO
    10 DIM I(9)
    20 FOR F=1 TO 9
    30 READ I(F)
    40 PRINT I(F)
    50 NEXT F
    60 ERASE I
    70 FOR F=1 TO 9
    80 PRINT I(F)
    90 NEXT F
    100 DATA 1,2,3,4,5,6,7,8,9

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 61.

VER TAMBEM
  basic, screens, cores)MAN");

    add("erl/err", "erl,err", "MSX-BASIC", "ERL", "Fornece o numero da linha em que ocorreu um erro e o numero do erro respectivamente.",
        R"MAN(TIPO: FUNCAO  (error line/error)

  Fornece o numero da linha em que ocorreu um erro e o numero do erro respectivamente.

FORMATO
  ERL
  ERR

EXEMPLO
  L=ERL
  E=ERR

DESCRICAO
  As variaveis ERL e ERR sao geralmente utilizadas nas rotinas de erros de um programa com a instrucao IF...THEN. Quando um erro e manuseado por um programa, a variavel ERR fornece o codigo referente ao erro ocorrido, enquanto que a variavel ERL fornece o numero da linha em que o erro foi encontrado. Se usado como um comando direto, ERL contera 65535. Para testar se o erro ocorreu por um comando direto, digite:
  IF 65535=ERL THEN ...
  Caso contrario, comande:
  IF ERL=(numero da linha) THEN ...
  IF ERR=(codigo de erro) THEN ...

PROGRAMA EXEMPLO
    10 REM PROGRAMA ERR/ERL
    20 ON ERROR GOTO 80
    30 CLS
    40 INPUT"DIGITE UM NUMERO NEGATIVO";A
    50 PRINT SQR(A)
    60 FOR F=1 TO 500: NEXT F
    70 GOTO 30
    80 PRINT"OCORREU O ERRO";ERR;"NA LINHA";ERL
    90 PRINT"POIS NAO EXISTE RAIZ DE NUMERO NEGATIVO"
    100 FOR F=1 TO 1000:NEXT F
    110 RUN 20

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 62.

VER TAMBEM
  basic, screens, cores)MAN");

    add("error", "", "MSX-BASIC", "ERROR numero do erro", "Simula a ocorrencia de um erro e permite que o usuario defina um codigo de erro correspondente.",
        R"MAN(TIPO: INSTRUCAO  (error)

  Simula a ocorrencia de um erro e permite que o usuario defina um codigo de erro correspondente.

FORMATO
  ERROR numero do erro

EXEMPLO
  ERROR 200

DESCRICAO
  O comando ERROR simulara a ocorrencia de um erro expondo a mensagem correspondente se o numero do erro coincidir com um dos erros utilizados pelo BASIC MSX. Para definir seu proprio codigo de erro, utilize um valor maior do que qualquer valor usado pelo BASIC. Veja no apendice a lista de erros e suas respectivas mensagens. O numero do erro deve estar entre 0 e 255, sendo que o BASIC MSX utiliza os codigos de 0 a 59. Estes codigos podem depois ser convenientemente manuseados em uma rotina de erro. Por exemplo:
  10 ON ERROR GOTO 1000
  120 IF A$="S" THEN ERROR 250
  1000 IF ERR=250 THEN PRINT "DESCULPE-ME"

  Se um termo ERROR especificando um codigo nao possuir uma mensagem definida, o BASIC respondera com uma mensagem do tipo:
  Unprintable error
  que sera exposta na tela e suspendera a execucao do programa.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ERROR
    20 ON ERROR GOTO 80
    30 PRINT"Digite a tecla A ."
    40 A$=INKEY$
    50 IF A$="" THEN 40
    60 IF A$<>"A" AND A$<>"a" THEN ERROR 230
    70 END
    80 PRINT "A tecla A!!! (por favor)"
    90 FOR F=1 TO 500:NEXT F
    100 RUN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 63.

VER TAMBEM
  basic, screens, cores)MAN");

    add("exp", "", "MSX-BASIC", "EXP (argumento)", "Fornece o valor de e elevado a X, ou seja, da funcao exponencial natural de X.",
        R"MAN(TIPO: FUNCAO  (exponential)

  Fornece o valor de e elevado a X, ou seja, da funcao exponencial natural de X.

FORMATO
  EXP (argumento)

EXEMPLO
  E=EXP(1)

DESCRICAO
  Fornece o valor da exponencial natural do argumento especificado (e elevado a X, onde e=2.7182818284). O valor do argumento pode ser menor ou igual a 145,06286085862. Se o valor do argumento for maior, uma mensagem do tipo:
  Overflow
  aparecera na tela.

PROGRAMA EXEMPLO
    10 REM PROGRAMA EXP
    20 CLS
    30 SCREEN 2
    40 FOR F=0 TO 255 STEP .5
    50 PSET(F,191-290*EXP(-F/30))
    60 NEXT F
    70 GOTO 70

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 64.

VER TAMBEM
  basic, screens, cores)MAN");

    add("fix", "", "MSX-BASIC", "FIX (argumento)", "Fornece apenas a parte inteira de um dado numerico.",
        R"MAN(TIPO: FUNCAO  (fix)

  Fornece apenas a parte inteira de um dado numerico.

FORMATO
  FIX (argumento)

EXEMPLO
  F=FIX(B/3)

DESCRICAO
  A funcao FIX fornece apenas a parte inteira do argumento especificado, ou seja, fornece uma representacao truncada de um dado numerico, sendo que todos os digitos a direita do ponto decimal sao desprezados. O argumento pode ser uma constante, uma variavel numerica, uma variavel numerica do tipo matriz ou entao uma expressao matematica. A grande diferenca entre as funcoes INT e FIX e que a funcao INT retorna com o maior inteiro possivel, enquanto que a funcao FIX retorna com a parte inteira do argumento truncado, desprezando as casas decimais. Portanto, equivale a:
  SGN(X)*INT(ABS(X))
  pois esta diferenca existe apenas para os numeros negativos.

PROGRAMA EXEMPLO
    10 REM PROGRAMA FIX
    20 CLS
    30 FOR F=1 TO 20
    40 A=500-RND(-TIME)*1000
    50 PRINT A;".............";FIX(A)
    60 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 65.

VER TAMBEM
  basic, screens, cores)MAN");

    add("for-next", "for,next", "MSX-BASIC", "FOR variavel = valor inicial TO valor final [STEP incremento]", "Repete a execucao de um bloco de instrucoes entre um comando FOR e o seu correspondente NEXT.",
        R"MAN(TIPO: INSTRUCAO  (for-next)

  Repete a execucao de um bloco de instrucoes entre um comando FOR e o seu correspondente NEXT.

FORMATO
  FOR variavel = valor inicial TO valor final [STEP incremento]
  NEXT [variavel]

EXEMPLO
  FOR J=0 TO 100 STEP 2
  NEXT J

DESCRICAO
  O comando FOR-NEXT executa, repetidamente, um bloco de instrucoes contido entre as instrucoes FOR e NEXT correspondentes. A variavel que controla o laco FOR-NEXT varia desde valor inicial ate o valor final sendo acrescida pelo valor do incremento todas as vezes que o comando NEXT correspondente for executado. Quando o incremento nao e definido, e adotado o valor 1. Por exemplo:
  10 REM PROGRAMA FOR...NEXT...STEP...
  20 CLS
  30 FOR F=1 TO 40 STEP 2
  40 G=G+F
  50 PRINT F,G
  60 NEXT F

  apresentando na tela:
  1  1
  3  4
  5  9
  7  16
  9  25
  11  36
  13  49
  15  64
  17  81
  19  100
  21  121
  23  144
  25  169
  27  196
  29  225
  31  256
  33  289
  35  324
  37  361
  39  400

  LACOS MULTIPLOS: Um loop FOR-NEXT pode ser colocado dentro de outro do mesmo tipo. Em tal caso, o laco interior devera estar completamente incluido no laco exterior e para cada laco devera se utilizar variaveis diferentes. Por exemplo:
  10 FOR I=1 TO 5
  20 FOR J=1 TO I
  30 PRINT"*";
  40 NEXT J
  50 PRINT
  60 NEXT I

  apresentando na tela:
  *
  **
  ***
  ****
  *

  Com uma instrucao NEXT pode-se terminar varias instrucoes FOR. Porem, nao se pode omitir o nome das variaveis, que devem estar dispostos sequencialmente, sendo que o laco mais interno deve estar em primeiro lugar e as variaveis devem estar separadas por virgula. Por exemplo:
  10 FOR I=0 TO 10
  20 FOR J=0 TO 5
  .
  .
  .
  10010 NEXT J,I

PROGRAMA EXEMPLO
    10 REM PROGRAMA FOR...NEXT
    20 FOR F=1 TO 20
    30 G=G+F
    40 PRINT F,G
    50 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 66.

VER TAMBEM
  basic, screens, cores)MAN");

    add("fre", "", "MSX-BASIC", "FRE (X)", "Fornece o numero de bytes livres na RAM de uma area de memoria nao utilizada pelo BASIC MSX.",
        R"MAN(TIPO: FUNCAO  (free)

  Fornece o numero de bytes livres na RAM de uma area de memoria nao utilizada pelo BASIC MSX.

FORMATO
  FRE (X)
  FRE ("")

EXEMPLO
  FRE(0)
  FRE("")

DESCRICAO
  FRE (X) fornece o numero de bytes na memoria que podem ser usados por um programa BASIC, arquivos de texto, arquivos de um programa em linguagem de maquina, etc., ou seja, a area livre na memoria. O valor de X pode ser um valor numerico qualquer.

  FRE ("") fornece o numero de bytes livre na area da memoria utilizada pelas variaveis strings.

PROGRAMA EXEMPLO
    10 REM PROGRAMA FRE
    20 CLS
    30 PRINT"Existem";FRE(1);"bytes livres e"
    40 PRINTFRE("");"bytes para strings."

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 68.

VER TAMBEM
  basic, screens, cores)MAN");

    add("getdate", "", "MSX-BASIC", "GETDATE X$", "Obtem a data armazenada no relogio/calendario interno.",
        R"MAN(TIPO: INSTRUCAO  (get date)

  Obtem a data armazenada no relogio/calendario interno.

FORMATO
  GETDATE X$

EXEMPLO
  GETDATE A$ : PRINT A$

DESCRICAO
  A data no relogio interno e transferida para a variavel X$. O formato dos dados na variavel e o seguinte: DD/MM/AA - onde DD e o dia, MM o mes e AA o ano. A data armazenada no relogio/calendario interno e atualizada mesmo com o computador desligado.

PROGRAMA EXEMPLO
    GETDATE A$ : PRINT A$

FONTE
  Manual MSX 2+ FM (ACVS), pagina 24.

VER TAMBEM
  basic, screens, cores)MAN");

    add("gettime", "", "MSX-BASIC", "GETTIME X$ [,A]", "Obtem a hora armazenada no relogio interno.",
        R"MAN(TIPO: INSTRUCAO  (get time)

  Obtem a hora armazenada no relogio interno.

FORMATO
  GETTIME X$ [,A]

EXEMPLO
  GETTIME A$

DESCRICAO
  A hora no relogio interno e transferida para a variavel X$. O formato dos dados na variavel e o seguinte: HH/MM/SS - onde HH e a hora, MM o minuto e SS o segundo. A hora do relogio e atualizada mesmo com o computador desligado.

PROGRAMA EXEMPLO
    10 SCREEN 1
    20 GETTIME A$
    30 LOCATE 10,10
    40 PRINT A$
    50 GOTO 20

FONTE
  Manual MSX 2+ FM (ACVS), pagina 25.

VER TAMBEM
  basic, screens, cores)MAN");

    add("gosub-return", "gosub,return", "MSX-BASIC", "GOSUB numero da linha A", "Transfere a execucao do programa para a sub-rotina especificada e indica o ponto de retorno ao...",
        R"MAN(TIPO: INSTRUCAO  (go to subroutine - return)

  Transfere a execucao do programa para a sub-rotina especificada e indica o ponto de retorno ao programa principal.

FORMATO
  GOSUB numero da linha A
  RETURN [numero da linha B]

EXEMPLO
  GOSUB 1000

DESCRICAO
  O comando GOSUB transfere a execucao do programa para a sub-rotina que comeca na linha A, enquanto que o comando RETURN indica o fim da sub-rotina e o retorno a linha B do programa principal. Quando o numero de linha B e omitido, o retorno ao programa principal se da na linha imediatamente seguinte a que contem o correspondente GOSUB. O numero das linhas A e B devem estar entre 0 e 65535. Uma sub-rotina pode ser chamada quantas vezes forem necessarias. Por exemplo (com os diagramas de fluxo do livro representados aqui apenas pelo codigo, sem as setas graficas):

  10 GOSUB 1000

  20 GOSUB 1000

  1000 (sub rotina)

  2000 RETURN

  Uma sub-rotina pode chamar outra sub-rotina, porem o comportamento da multi-execucao dependera da capacidade da memoria existente.

  10 GOSUB 100

  100 (sub rotina)

  110 GOSUB 1000
  200 RETURN
  1000 (sub rotina)

  2000 RETURN

PROGRAMA EXEMPLO
    10 REM GOSUB ... RETURN
    20 GOSUB 1000
    30 GOSUB 2000
    40 PRINT"ESTA ROTINA FAZ O MINIMO POSSIVEL"
    50 FOR F=1 TO 1000:NEXT F:PRINT
    60 RUN
    1000 PRINT"ESTA SUB-ROTINA SO IMPRIME"
    1010 RETURN
    2000 PRINT"ESTA SUB-ROTINA SO IMPRIME"
    2010 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 69.

VER TAMBEM
  basic, screens, cores)MAN");

    add("goto", "", "MSX-BASIC", "GOTO numero da linha", "Desvia incondicionalmente a sequencia normal do programa para uma linha especificada.",
        R"MAN(TIPO: INSTRUCAO  (goto)

  Desvia incondicionalmente a sequencia normal do programa para uma linha especificada.

FORMATO
  GOTO numero da linha

EXEMPLO
  GOTO 390

DESCRICAO
  A execucao do programa e transferida para a linha especificada pelo comando GOTO. Se esta contiver uma instrucao valida, sera executada e, depois dela, as linhas seguintes. Se a linha indicada, porem, contiver uma instrucao nao executavel (REM ou DATA), o micro a pulara passando para a primeira linha que contiver um comando valido, a partir da indicada pelo comando GOTO. Se a linha indicada pelo comando GOTO, contiver uma instrucao mal-escrita, a tela exibira a mensagem de erro:
  Syntax error in (linha que contem o GOTO)
  Se o numero da linha indicada pelo GOTO nao existir, sera ativada a mensagem de erro:
  Undefined line number in (linha com GOTO)
  GOTO pode ser executado com comando direto.

PROGRAMA EXEMPLO
    10 ' GOTO
    20 PRINT"Linha 20":GOTO 50
    30 PRINT"Linha 30"
    40 PRINT"Linha 40":GOTO 60
    50 PRINT"Linha 50":GOTO 30
    60 PRINT"Linha 60"
    70 PRINT"Linha 70":GOTO 90
    80 PRINT"Linha 80":GOTO 70
    90 PRINT" F I M"

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 70.

VER TAMBEM
  basic, screens, cores)MAN");

    add("hex$", "", "MSX-BASIC", "HEX$ (X)", "Transforma um numero em uma expressao hexadecimal na forma de uma string.",
        R"MAN(TIPO: FUNCAO  (hexadecimal dollar)

  Transforma um numero em uma expressao hexadecimal na forma de uma string.

FORMATO
  HEX$ (X)

EXEMPLO
  PRINT HEX$(13)

DESCRICAO
  A funcao HEX$ transforma um dado numerico em uma expressao hexadecimal na forma de uma string. X pode assumir um valor entre -32768 e 65535. Para X negativo, antes e calculado seu complemento (65536 - X) e a partir do resultado desta operacao e feita a transformacao, ou seja:
  HEX$ (-X) = HEX$ (65536-X)

PROGRAMA EXEMPLO
    10 REM HEX$
    20 CLS
    30 PRINT" HEXADECIMAL ***** ALEPH PUBLICACOES"
    40 PRINT
    50 INPUT" QUANTOS NUMEROS";NS
    60 PRINT :DIM H(NS)
    70 CLS
    80 PRINT" HEXADECIMAL ***** ALEPH PUBLICACOES"
    90 FOR F=1 TO NS
    100 INPUT"NUMERO";H(F)
    110 NEXT F
    120 CLS
    130 PRINT" HEXADECIMAL ***** ALEPH PUBLICACOES"
    140 FOR F=1 TO NS
    150 PRINT"DECIMAL","HEXADECIMAL"
    160 PRINT H(F),HEX$(H(F))
    170 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 71.

VER TAMBEM
  basic, screens, cores)MAN");

    add("if-then-else", "if,then,else", "MSX-BASIC", "IF condicao THEN comando se condicao verdadeira", "Permite a bifurcacao na execucao do programa em funcao do cumprimento de uma condicao.",
        R"MAN(TIPO: INSTRUCAO  (if-then-else)

  Permite a bifurcacao na execucao do programa em funcao do cumprimento de uma condicao.

FORMATO
  IF condicao THEN comando se condicao verdadeira
  [ELSE comando se condicao falsa]

EXEMPLO
  IF A$="MSX" THEN GOTO 200 ELSE 300

DESCRICAO
  Se a condicao for verdadeira, sera executado o comando apos o termo THEN. Caso contrario, ou seja, se a condicao for falsa, o comando apos o termo THEN e ignorado e sera executado o comando apos o termo ELSE. Por exemplo:
  X = 3 e Y = 8   IF X < Y ... (verdadeiro)
  X = 9 e Y = 8   IF X < Y ... (falso)
  O comando apos o termo THEN pode inclusive ser um comando nulo, ou seja, nao possuir nenhum comando. Se a condicao for uma expressao entre parenteses, assumira o valor -1 se verdadeira, e 0 se falsa. Por exemplo, na instrucao:
  IF (A$="MSX")=-1 THEN PRINT "OK"
  sera impresso um "OK", se a condicao for verdadeira. No formato IF-THEN-GOTO pode-se omitir o termo THEN ou o termo GOTO. Por exemplo, se digitarmos:
  IF A$="MSX" THEN 30   ou
  IF A$="MSX" GOTO 30
  obteremos o mesmo resultado, pois as duas expressoes tem o mesmo significado. Porem, no formato IF-THEN-ELSE-GOTO, o termo ELSE pode ser seguido do comando GOTO numero da linha ou simplesmente do numero da linha (GOTO pode ser omitido, e mesmo assim, a execucao sera desviada, conforme o resultado da condicao para o numero da linha indicada). Depois do termo THEN e do termo ELSE, podem ser escritas varias sentencas. Elas serao executadas sequencialmente, comecando pela esquerda. Estas sentencas devem ser separadas por "dois pontos" (:).

  COMANDO IF-THEN-ELSE ENCADEADOS: Depois de um termo THEN ou de um termo ELSE, pode ser tambem comandada uma nova condicao. A este tipo de sentenca da-se o nome de comando encadeado.

PROGRAMA EXEMPLO
    10 REM IF ... THEN ... ELSE
    20 CLS : A=INT(10*RND(-TIME)) : PRINT
    30 INPUT "Adivinhe o numero que pensei (0-9).";N : BEEP
    40 IF N<>A THEN GOTO 80 ELSE 50
    50 PRINT "Muito bem, voce acertou !!!"
    60 FOR F=1 TO 1000 : NEXT F
    70 GOTO 20
    80 PRINT "Voce errou! Tente outra vez!"
    90 GOTO 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 72.

VER TAMBEM
  basic, screens, cores)MAN");

    add("inkey$", "", "MSX-BASIC", "INKEY$", "Obtem o caractere da tecla pressionada.",
        R"MAN(TIPO: FUNCAO  (inkey dollar)

  Obtem o caractere da tecla pressionada.

FORMATO
  INKEY$

EXEMPLO
  C$=INKEY$

DESCRICAO
  A funcao INKEY$ obtem o caractere da tecla (exceto CTRL + STOP, SHIFT e CTRL) que esta sendo pressionada. Se nenhuma tecla estiver pressionada, o resultado sera um caractere nulo.

PROGRAMA EXEMPLO
    10 REM INKEY$
    20 X=128
    30 Y=96
    40 SCREEN 2
    50 PSET(128,96),10
    60 DRAW"E2F2G2H2"
    70 X=X-(INKEY$="D")+(INKEY$="A")
    80 Y=Y-(INKEY$="X")+(INKEY$="W")
    90 PSET(X,Y),10
    100 GOTO 60

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 73.

VER TAMBEM
  basic, screens, cores)MAN");

    add("inp", "", "MSX-BASIC", "INP (numero da via de acesso)", "Le os dados da via de acesso especificada.",
        R"MAN(TIPO: FUNCAO (uso avancado)  (input)

  Le os dados da via de acesso especificada.

FORMATO
  INP (numero da via de acesso)

EXEMPLO
  A=INP(15)

DESCRICAO
  Este comando le dados atraves da via de acesso especificada pelo numero entre parenteses. Este numero pode ser fornecido atraves de uma constante, uma variavel numerica, uma variavel do tipo matriz ou uma expressao matematica e o seu valor deve estar compreendido entre 0 e 255.

PROGRAMA EXEMPLO
    10 CLS:PRINT"EXPERIMENTE APERTAR <CAPS LOCK> !"
    20 LOCATE 10,10:X=INP(170):IF X=26 THEN PRINT"MAIUSCULAS" ELSE PRINT"minusculas"
    30 GOTO 20

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 74.

VER TAMBEM
  basic, screens, cores)MAN");

    add("input", "", "MSX-BASIC", "INPUT [\"mensagem\";] variavel", "Introduz o valor de uma variavel atraves do teclado.",
        R"MAN(TIPO: INSTRUCAO  (input)

  Introduz o valor de uma variavel atraves do teclado.

FORMATO
  INPUT ["mensagem";] variavel
  [, variavel] ...

EXEMPLO
  INPUT"QUAL O SEU NOME";N$

DESCRICAO
  Quando um comando INPUT e encontrado no programa, um ponto de interrogacao e colocado na tela e a execucao do programa e interrompida a espera de que um dado seja fornecido pelo usuario via teclado. Quando a mensagem e especificada, ela e impressa na tela, seguida do ponto de interrogacao. Deve-se digitar o dado requisitado que sera transferido para a variavel ou variaveis na ordem em que foram fornecidos ao MSX. O numero de dados a serem introduzidos deve ser igual ao numero de variaveis, sendo que cada dado deve ser separado por uma virgula. Se for digitado um numero maior do que o necessario, aparecera na tela uma mensagem do tipo:
  Extra ignored
  indicando que os dados excedentes foram ignorados e se passa a execucao do proximo comando. Porem, se o numero de dados introduzidos for menor do que o requisitado, aparecerao na tela dois pontos de interrogacao (??) indicando que o MSX esta a espera de mais dados. O tipo de dado introduzido deve estar de acordo com o tipo de variavel especificado, caso contrario, uma mensagem do tipo:
  Redo from start
  aparecera na tela, e o comando INPUT sera executado novamente. Digitando CTRL + STOP ou CTRL + C, o MSX retorna ao nivel de comando direto mandando uma mensagem do tipo:
  Break in (numero da linha do comando INPUT)
  Para continuar a execucao, basta comandar CONT e o programa retornara a execucao do comando INPUT.

PROGRAMA EXEMPLO
    10 REM PROGRAMA INPUT
    20 PRINT"TUDO O QUE FOR ESCRITO DEPOIS DO PONTO DE INTERROGACAO, SERA ESCRITO NA TELA!"
    30 INPUT"QUAL O SEU NOME";A$
    40 PRINT A$
    50 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 75.

VER TAMBEM
  basic, screens, cores)MAN");

    add("input#", "", "MSX-BASIC", "INPUT # numero do arquivo, lista de variaveis", "Le dados de um arquivo aberto, associando-os as variaveis de um programa.",
        R"MAN(TIPO: INSTRUCAO  (input number)

  Le dados de um arquivo aberto, associando-os as variaveis de um programa.

FORMATO
  INPUT # numero do arquivo, lista de variaveis

EXEMPLO
  INPUT#1,A,B$

DESCRICAO
  A instrucao INPUT# le dados de um arquivo que foi aberto por uma instrucao OPEN e associa os dados as variaveis listadas. O numero do arquivo deve ser um numero maior do que zero e menor ou igual ao numero de arquivos especificado em MAXFILES. A lista de variaveis inclui as variaveis numericas, alfanumericas (strings) e as indexadas.

PROGRAMA EXEMPLO
    10 REM PROGRAMA INPUT#
    20 OPEN "CAS:II" FOR INPUT AS #1
    30 IF EOF(1)=-1 THEN 70
    40 INPUT #1,A$
    50 PRINT A$:BEEP
    60 GOTO 30
    70 CLOSE #1

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 76.

VER TAMBEM
  basic, screens, cores)MAN");

    add("input$", "", "MSX-BASIC", "INPUT$ (numero de caracteres)", "Le um numero especificado de caracteres introduzidos pelo teclado ou atraves de um arquivo.",
        R"MAN(TIPO: FUNCAO  (input dollar)

  Le um numero especificado de caracteres introduzidos pelo teclado ou atraves de um arquivo.

FORMATO
  INPUT$ (numero de caracteres)
  INPUT$ (numero de caracteres, [, # numero do arquivo])

EXEMPLO
  X$=INPUT$(6)
  W$=INPUT$(3,#1)

DESCRICAO
  O formato INPUT$ (numero de caracteres) le, do teclado, o numero especificado de caracteres e os fornece na forma de uma string. O formato INPUT$ (numero de caracteres, # numero do arquivo) le, de um arquivo que esta aberto, o numero de caracteres especificado, e os fornece na forma de uma string. O numero de caracteres pode ser fornecido por uma constante, uma variavel numerica ou entao por uma variavel numerica indexada. O valor do numero de caracteres deve estar entre 1 e 255 (se for maior do que 200, a instrucao CLEAR deve ser executada antes).

PROGRAMA EXEMPLO
    10 REM PROGRAMA INPUT$
    20 A$=INPUT$(8)
    30 PRINT A$

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 77.

VER TAMBEM
  basic, screens, cores)MAN");

    add("instr", "", "MSX-BASIC", "INSTR ([N,] string de pesquisa, string procurada)", "Localiza a posicao de uma string dentro de uma outra string.",
        R"MAN(TIPO: FUNCAO  (in string)

  Localiza a posicao de uma string dentro de uma outra string.

FORMATO
  INSTR ([N,] string de pesquisa, string procurada)

EXEMPLO
  INSTR("SOU O MSX","O")

DESCRICAO
  A funcao INSTR fornece a localizacao do primeiro caractere da string procurada dentro da string de pesquisa, sendo que esta contagem comeca-se a partir do n-esimo caractere da string de pesquisa. N pode ser fornecido atraves de uma constante, uma variavel numerica ou uma variavel numerica indexada, sendo que seu valor deve estar entre 1 e 255. As strings de pesquisa e de procura podem ser fornecidas por uma constante, variavel alfanumerica ou uma variavel alfanumerica indexada. Se o valor N for maior do que o comprimento da string de pesquisa, ou se a string de pesquisa for uma string nula, ou se a string procurada nao for encontrada na string de pesquisa, o resultado sera zero (0).

PROGRAMA EXEMPLO
    10 REM PROGRAMA INSTR
    20 A$="GRADIENTE&ALEPH-HPELA&ETNEIDARG"
    30 INPUT"ENTRE UMA SEQUENCIA DE ATE 10 CARACTERES";B$
    40 C=INSTR(A$,B$)
    50 IF C=0 THEN GOTO 80
    60 PRINT"ESSA SEQUENCIA COMECA NA POSICAO";C
    70 END
    80 PRINT"NAO EXISTE ESSA SEQUENCIA EM A$"
    90 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 78.

VER TAMBEM
  basic, screens, cores)MAN");

    add("int", "", "MSX-BASIC", "INT (argumento)", "Fornece o valor do maior inteiro possivel, menor do que o argumento.",
        R"MAN(TIPO: FUNCAO  (integer)

  Fornece o valor do maior inteiro possivel, menor do que o argumento.

FORMATO
  INT (argumento)

EXEMPLO
  PRINT INT(1.12345)

DESCRICAO
  A funcao INT fornece o valor do maior inteiro possivel que nao seja maior do que o valor do argumento. Por exemplo:
  INT(2.5653)=2
  INT(-2.5653)=-3
  INT(1000101.23)=1000101

  O argumento pode ser fornecido por uma constante, uma variavel numerica ou uma variavel numerica indexada.

PROGRAMA EXEMPLO
    10 REM PROGRAMA INT
    20 CLS
    30 FOR F=1 TO 20
    40 A=500-RND(-TIME)*1000
    50 PRINT A;".............";INT(A)
    60 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 79.

VER TAMBEM
  basic, screens, cores)MAN");

    add("interval on/off/stop", "interval", "MSX-BASIC", "INTERVAL ON", "Cada uma destas tres funcoes respectivamente, habilita, desabilita e adia uma interrupcao feita...",
        R"MAN(TIPO: INSTRUCAO  (interval on/off/stop)

  Cada uma destas tres funcoes respectivamente, habilita, desabilita e adia uma interrupcao feita pelo temporizador.

FORMATO
  INTERVAL ON
  INTERVAL OFF
  INTERVAL STOP

EXEMPLO
  INTERVAL ON
  INTERVAL OFF
  INTERVAL STOP

DESCRICAO
  A instrucao INTERVAL ON habilita uma interrupcao causada pelo temporizador, caso exista uma instrucao ON INTERVAL GOSUB no programa. A instrucao INTERVAL OFF desabilita (ou desliga) a instrucao INTERVAL ON. A instrucao INTERVAL STOP adia a interrupcao ate que seja encontrada a instrucao INTERVAL ON.

PROGRAMA EXEMPLO
    10 REM PROGRAMA INTERVAL ON
    20 INTERVAL ON
    30 ON INTERVAL=50 GOSUB 50
    40 GOTO 40
    50 C=C+1
    60 PRINT C
    70 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 80.

VER TAMBEM
  basic, screens, cores)MAN");

    add("key", "", "MSX-BASIC", "KEY numero da tecla de funcao, string", "Associa uma string (cadeia de caracteres) a uma tecla de funcao (F1 a F10).",
        R"MAN(TIPO: INSTRUCAO  (key)

  Associa uma string (cadeia de caracteres) a uma tecla de funcao (F1 a F10).

FORMATO
  KEY numero da tecla de funcao, string

EXEMPLO
  KEY 1,"LOAD"

DESCRICAO
  A funcao KEY associa uma string a uma tecla de funcao, localizada na parte superior esquerda do teclado. O numero da tecla de funcao deve estar entre 1 e 10 (as teclas F6 a F10 sao obtidas mantendo-se a tecla SHIFT pressionada) e a string deve ter, no maximo, 15 caracteres. Esta associacao sera desfeita se o computador for desligado. Podemos tambem utilizar a funcao CHR$ na formacao da cadeia de caracteres (por exemplo, o caractere de retorno, que nao possui um correspondente alfanumerico, entra como codigo 13). Exemplos:

  KEY 1,"gradiente"+"aleph"

  KEY 2,"LIST"+CHR$(13)

  KEY 7,"FOR I="

  KEY 8,"NEXT I"

PROGRAMA EXEMPLO
    10 ' KEY
    20 KEY 1,"CLS:LIST"+CHR$(13)
    30 INPUT"Voce quer definir alguma outra tecla";R$
    35 IF R$="n" THEN END
    40 INPUT"Qual a tecla (1-10)";T
    50 INPUT"Qual a funcao";F$
    60 KEY T,F$
    70 GOTO 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 81.

VER TAMBEM
  basic, screens, cores)MAN");

    add("key (n) on/off/stop", "key (n)", "MSX-BASIC", "KEY (n) ON", "Cada uma destas tres funcoes respectivamente, habilita, desabilita e adia uma interrupcao...",
        R"MAN(TIPO: INSTRUCAO  (key (n) on/off/stop)

  Cada uma destas tres funcoes respectivamente, habilita, desabilita e adia uma interrupcao atraves das teclas de funcao.

FORMATO
  KEY (n) ON
  KEY (n) OFF
  KEY (n) STOP

EXEMPLO
  KEY(1) ON
  KEY(2) OFF
  KEY(3) STOP

DESCRICAO
  A instrucao KEY (n) ON habilita uma interrupcao causada caso seja pressionada uma das teclas de funcao, quando existir a instrucao ON KEY GOSUB no programa. A instrucao KEY (n) OFF desabilita (ou desliga) a funcao KEY (n) ON. A instrucao KEY (n) STOP adia a interrupcao ate que seja encontrada a instrucao KEY (n) ON.

PROGRAMA EXEMPLO
    10 REM PROGRAMA KEY(n) ON
    20 KEY(1) ON
    30 ON KEY GOSUB 100
    40 GOTO 30
    100 PRINT"VOCE PRESSIONOU A TECLA F1."
    110 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 84.

VER TAMBEM
  basic, screens, cores)MAN");

    add("key list", "", "MSX-BASIC", "KEY LIST", "Visualiza o conteudo das teclas de funcao.",
        R"MAN(TIPO: INSTRUCAO  (key list)

  Visualiza o conteudo das teclas de funcao.

FORMATO
  KEY LIST

EXEMPLO
  KEY LIST

DESCRICAO
  O comando KEY LIST, permite a visualizacao do conjunto de caracteres associado as teclas de funcao. O estado inicial (default) que o computador oferece e o seguinte:

  F1 ................."color" + CHR$ (32)
  F2 ................."auto" + CHR$ (32)
  F3 ................."goto" + CHR$ (32)
  F4 ................."list" + CHR$ (32)
  F5 ................."run" + CHR$ (13)
  F6 (F1 + SHIFT) ......."color" + CHR$ (32) +
                  "15,1,1" + CHR$ (13)
  F7 (F2 + SHIFT) ........"cload" + CHR$ (34)
  F8 (F3 + SHIFT) ........"cont" + CHR$ (13)
  F9 (F4 + SHIFT) ........"list" + CHR$ (13)
  F10 (F5 + SHIFT) ......."run" + CHR$ (13)

  Observacao: CHR$ (13) = RETURN
              CHR$ (32) = ESPACO
              CHR$ (34) = ASPAS (")

PROGRAMA EXEMPLO
    10 REM PROGRAMA KEY LIST
    20 INPUT"VOCE QUER VERIFICAR AS TECLAS FUNCIONAIS (S/N)";R$
    30 IF R$="N" THEN 50
    40 KEY LIST
    50 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 82.

VER TAMBEM
  basic, screens, cores)MAN");

    add("key on/off", "key", "MSX-BASIC", "KEY ON", "Ativa ou desativa a visualizacao, na parte inferior da tela, dos caracteres associados as...",
        R"MAN(TIPO: INSTRUCAO  (key on/off)

  Ativa ou desativa a visualizacao, na parte inferior da tela, dos caracteres associados as teclas de funcao.

FORMATO
  KEY ON
  KEY OFF

EXEMPLO
  KEY ON
  KEY OFF

DESCRICAO
  O comando KEY ON ativa a visualizacao na parte inferior da tela da cadeia de caracteres associada a cada tecla de funcao. O comando KEY OFF desativa esta visualizacao.

PROGRAMA EXEMPLO
    10 REM PROGRAMA KEY ON/OFF
    20 INPUT"VOCE QUER AS TECLAS FUNCIONAIS NA TELA (S/N)";R$
    30 IF R$="N" THEN KEY OFF ELSE KEY ON
    40 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 83.

VER TAMBEM
  basic, screens, cores)MAN");

    add("left$", "", "MSX-BASIC", "LEFT$ (string, quantidade de caracteres)", "Fornece na forma de string os n-primeiros caracteres de uma cadeia.",
        R"MAN(TIPO: FUNCAO  (left dollar)

  Fornece na forma de string os n-primeiros caracteres de uma cadeia.

FORMATO
  LEFT$ (string, quantidade de caracteres)

EXEMPLO
  LEFT$("GRADIENTE-ALEPH",5)

DESCRICAO
  A funcao LEFT$ fornece os n-primeiros caracteres de uma string, sendo o primeiro caractere o mais a esquerda da string.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LEFT$
    20 CLS
    30 INPUT " ESCREVA UMA PALAVRA";W$: PRINT
    40 PRINT "A PRIMEIRA LETRA E'...";LEFT$(W$,1): PRINT

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 85.

VER TAMBEM
  basic, screens, cores)MAN");

    add("len", "", "MSX-BASIC", "LEN (string)", "Fornece a quantidade de caracteres de uma string.",
        R"MAN(TIPO: FUNCAO  (lenght)

  Fornece a quantidade de caracteres de uma string.

FORMATO
  LEN (string)

EXEMPLO
  LEN("COMPUTADOR")

DESCRICAO
  A funcao LEN fornece a quantidade de caracteres de uma string, ou seja, o seu comprimento.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LEN
    20 CLS
    30 INPUT"ESCREVA UMA SENTENCA QUE NAO ULTRAPASSE 10 PALAVRAS.";S$: PRINT
    40 PRINT " O NUMERO DE CARACTERES QUE ESTA FRASE POSSUI E':"
    50 PRINT LEN(S$)

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 86.

VER TAMBEM
  basic, screens, cores)MAN");

    add("let", "", "MSX-BASIC", "[LET] variavel = expressao", "Realiza a associacao de um dado a uma variavel.",
        R"MAN(TIPO: INSTRUCAO  (let)

  Realiza a associacao de um dado a uma variavel.

FORMATO
  [LET] variavel = expressao

EXEMPLO
  LET A=10
  LET A="ABC"+"DEF"

DESCRICAO
  A instrucao LET faz a associacao de um dado (numerico ou alfanumerico) com uma variavel (do mesmo tipo do dado), sendo que as variaveis alfanumericas devem estar entre aspas (" "), caso contrario, uma mensagem do tipo:
  Type mismatch
  aparecera na tela. A palavra LET pode ser omitida neste tipo de instrucao.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LET
    20 CLS
    30 LET A=10:PRINT"A=";A
    40 LET B=20:PRINT"B=";B
    50 LET C=A+B
    60 PRINT "A+B=";C
    70 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 87.

VER TAMBEM
  basic, screens, cores)MAN");

    add("line", "", "MSX-BASIC", "LINE [[STEP] (X1,Y1)] -", "Traca, no modo grafico, uma linha ou um quadrado, dependendo da sintaxe da instrucao.",
        R"MAN(TIPO: INSTRUCAO  (line)

  Traca, no modo grafico, uma linha ou um quadrado, dependendo da sintaxe da instrucao.

FORMATO
  LINE [[STEP] (X1,Y1)] -
  [STEP] (X2, Y2)
  [cor] [,B] [,BF]

EXEMPLO
  LINE(100,100)-(135,145),3,B

DESCRICAO
  O comando LINE traca uma linha reta entre os pontos de coordenadas (X1,Y1) e (X2,Y2). Esta reta podera ser colorida, se o item cor for especificado. Quando a letra B (box) e especificada, sera tracado um retangulo que ligara "diagonalmente" os dois pontos especificados. Se BF (box full) for especificado, sera tracado um retangulo com a parte interna pintada pela cor especificada. Quando a cor nao for especificada, sera usada a cor do primeiro plano definida pela ultima instrucao COLOR.
  Por exemplo:

  LINE(100,130)-(120,150),,B

  Para desenhar um quadrado, a diferenca entre X1 e X2 deve ser 1,25 vezes menor do que a diferenca entre Y1 e Y2. Isto porque a relacao entre os caracteres horizontais e verticais e de 8:10. Por exemplo, em:
  LINE(50,50)-(130,150)

  X1-X2=-80, Y1-Y2=-100 e (Y1-Y2)/(X1-X2)=1.25
  Se esta relacao entre o eixo X e o eixo Y nao for obedecida, sera desenhado um retangulo. Por exemplo:
  LINE(10,70)-(40,80),3,B

  Observacao: Antes de uma instrucao LINE, deve existir um comando SCREEN 2 ou SCREEN 3, que muda para a pagina grafica.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LINE
    20 SCREEN2
    30 FOR F=0 TO 125 STEP 10
    40 G=80-F*80/125
    50 LINE (F,80)-(125,G)
    60 LINE (125,G)-(250-F,80)
    70 LINE (F,80)-(125,160-G)
    80 LINE (125,160-G)-(250-F,80)
    90 NEXT F
    100 GOTO 100

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 88.

VER TAMBEM
  basic, screens, cores)MAN");

    add("line input", "", "MSX-BASIC", "LINE INPUT [\"mensagem\";] variavel", "Associa um dado introduzido pelo teclado, a uma variavel string.",
        R"MAN(TIPO: INSTRUCAO  (line input)

  Associa um dado introduzido pelo teclado, a uma variavel string.

FORMATO
  LINE INPUT ["mensagem";] variavel

EXEMPLO
  LINE INPUT"Entre com um nome:";A$

DESCRICAO
  A instrucao LINE INPUT executa a leitura do teclado e associa os caracteres pressionados a uma variavel string. E uma instrucao semelhante ao INPUT, com a diferenca de que o LINE INPUT nao coloca o ponto de interrogacao na tela e associa os caracteres digitados a apenas uma variavel. A mensagem e opcional e, quando especificada, sera mostrada na tela.

PROGRAMA EXEMPLO
    10 ' LINE INPUT
    20 LINE INPUT"Introduza o seu primeiro nome e digite RETURN.   ";N$
    25 CLS:N$=N$+" "
    30 N$=RIGHT$(N$,LEN(N$)-1)+LEFT$(N$,1):LOCATE 10,10
    40 PRINT N$:FOR F=1 TO 30:NEXT F
    50 GOTO 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 89.

VER TAMBEM
  basic, screens, cores)MAN");

    add("line input#", "", "MSX-BASIC", "LINE INPUT # numero do arquivo,nome da variavel", "Le uma sequencia de ate 254 caracteres de um arquivo e a atribui a uma variavel string.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (line input number)

  Le uma sequencia de ate 254 caracteres de um arquivo e a atribui a uma variavel string.

FORMATO
  LINE INPUT # numero do arquivo,nome da variavel

EXEMPLO
  LINE INPUT#2,A$

DESCRICAO
  O comando LINE INPUT # obtem uma string de um arquivo e a armazena numa variavel. O arquivo e definido por um numero entre 1 e o numero especificado por MAXFILES, e a variavel pode ser simples ou indexada.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LINE INPUT
    20 OPEN "CAS:II" FOR INPUT AS#1
    30 IF EOF=-1 THEN 70
    40 LINE INPUT#1,A$
    50 PRINT A$:BEEP
    60 GOTO 30
    70 CLOSE#1

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 90.

VER TAMBEM
  basic, screens, cores)MAN");

    add("list/llist", "list,llist", "MSX-BASIC", "LIST [numero da linha inicial]", "Lista o programa ou parte dele na tela ou na impressora.",
        R"MAN(TIPO: INSTRUCAO  (list out/line printer list out)

  Lista o programa ou parte dele na tela ou na impressora.

FORMATO
  LIST [numero da linha inicial]
  [-numero da linha final]

  LLIST [numero da linha inicial]
  [-numero da linha final]

EXEMPLO
  LIST-900
  LIST 30-70

DESCRICAO
  Os dois comandos (LIST e LLIST) listam todo ou parte de um programa sendo que com o comando LIST a listagem do programa e visualizada na tela, enquanto que o comando LLIST e executado na impressora. Os numeros das linhas inicial e final sao opcionais e, portanto, podem ser omitidos. Se for feito um comando LIST sem os numeros de linhas, todo o programa sera listado. Se for feito um comando com apenas um numero de linha, sera listada apenas a linha especificada. Por exemplo:
  LIST 50

  Outra opcao seria um comando com um hifen antes do numero da linha. Neste caso, o programa sera listado do inicio ate a linha especificada. Por exemplo:
  LIST-900

PROGRAMA EXEMPLO
    10 REM PROGRAMA LIST/LLIST
    20 INPUT"VOCE QUER LISTAR O PROGRAMA NA TELA (T)OU NA IMPRESSORA (I)";R$
    30 IF R$="T" THEN LIST
    40 IF R$="I" THEN LLIST
    50 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 91.

VER TAMBEM
  basic, screens, cores)MAN");

    add("load", "", "MSX-BASIC", "LOAD\"[nome do dispositivo] [nome do arquivo]\" [,R]", "Carrega um arquivo de um dispositivo especificado para a memoria do micro.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (load)

  Carrega um arquivo de um dispositivo especificado para a memoria do micro.

FORMATO
  LOAD"[nome do dispositivo] [nome do arquivo]" [,R]

EXEMPLO
  LOAD"CAS:MARRE",R

DESCRICAO
  Carregar, para a memoria do computador, um arquivo em BASIC gravado no dispositivo especificado. Se a letra R for colocado apos a ultima aspas o programa sera executado assim que a transferencia para a RAM for completada.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 92.

VER TAMBEM
  basic, screens, cores)MAN");

    add("locate", "", "MSX-BASIC", "LOCATE [coordenada x] [, coordenada y]", "Move o cursor para posicao especificada pelas coordenadas x e y.",
        R"MAN(TIPO: INSTRUCAO  (locate)

  Move o cursor para posicao especificada pelas coordenadas x e y.

FORMATO
  LOCATE [coordenada x] [, coordenada y]
  [, interruptor do cursor]

EXEMPLO
  LOCATE 10,10,1

DESCRICAO
  A instrucao LOCATE move o cursor para uma posicao especificada pelas coordenadas X e Y.

  Coordenada X: constantes, variaveis indexadas ou numericas, podendo assumir valores entre 0 e 39. Se omitida, o computador assumira o valor 0.

  Coordenada Y: constantes, variaveis indexadas ou numericas, podendo assumir valores entre 0 e 23. Se omitida, o computador assumira o valor 0.

  Interruptor do Cursor: se for 0 o cursor nao sera visivel. Se for 1, o cursor aparecera. Se omitido, o computador assumira o valor 1.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LOCATE
    20 INPUT"INTRODUZA UMA PALAVRA";P$
    30 INPUT"EM QUE COLUNA VOCE QUER IMPRIMI-LA";CO
    40 INPUT"E EM QUE LINHA";LI
    50 CLS
    60 LOCATE CO,LI
    70 PRINT P$

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 93.

VER TAMBEM
  basic, screens, cores)MAN");

    add("log", "", "MSX-BASIC", "LOG (X)", "Determina o logaritmo natural de um numero.",
        R"MAN(TIPO: FUNCAO  (logaritmo natural)

  Determina o logaritmo natural de um numero.

FORMATO
  LOG (X)

EXEMPLO
  LOG(10)

DESCRICAO
  A funcao LOG determina o logaritmo natural cuja base e .......... (2.7182818284588...). Para determinar o valor de um logaritmo em outra base (A):
  LOGA B
  sendo B>0 e A (A = base) um numero positivo diferente de zero, utiliza-se a formula:
  LOG (B)/LOG (A)

  Observacao: o valor de X sempre devera ser maior que zero.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LOG
    20 INPUT"INTRODUZA UM NUMERO POSITIVO:";N
    30 PRINT"O LOGARITMO NATURAL DESSE NUMERO E:";LOG(N),"E SEU LOGARITMO DECIMAL E:";LOG(N)/LOG(10)
    40 PRINT
    50 RUN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 94.

VER TAMBEM
  basic, screens, cores)MAN");

    add("lpos", "", "MSX-BASIC", "LPOS (X)", "Fornece a posicao do cabecote da impressora.",
        R"MAN(TIPO: FUNCAO  (line printer position)

  Fornece a posicao do cabecote da impressora.

FORMATO
  LPOS (X)

EXEMPLO
  LPOS(0)

DESCRICAO
  Fornece a posicao no buffer de memoria da impressora, do caractere que esta sendo impresso (posicao inicial = 0). O valor de X pode ser um numero arbitrario qualquer e o valor obtido e um numero inteiro.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LPOS
    20 A$=""
    30 FOR F=1 TO 60
    40 LPRINT A$;LPOS(1)
    50 A$=A$+">"
    60 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 95.

VER TAMBEM
  basic, screens, cores)MAN");

    add("lprint", "", "MSX-BASIC", "LPRINT expressao [expressao...]", "Escreve na impressora o valor de uma expressao.",
        R"MAN(TIPO: INSTRUCAO  (line print)

  Escreve na impressora o valor de uma expressao.

FORMATO
  LPRINT expressao [expressao...]

EXEMPLO
  LPRINT A$,B;"ALEPH",D+2

DESCRICAO
  Escrever dados na impressora, da mesma forma que o comando PRINT os escreve na tela. As expressoes podem ser: constantes, qualquer tipo de variavel ou expressoes algebricas. Entre as expressoes podem ser utilizados os simbolos "," ou ";". O comando LPRINT desacompanhado de qualquer expressao ou simbolo causa o avanco de uma linha. Para maiores detalhes consulte PRINT.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LPRINT
    20 FOR F=1 TO 50
    30 LPRINT
    40 LPRINT
    50 LPRINT
    60 FOR G=1 TO 60
    70 LPRINT"_________________________________________________________________"
    80 NEXT G
    90 LPRINT
    100 LPRINT
    110 LPRINT TAB 70;F
    120 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 96.

VER TAMBEM
  basic, screens, cores)MAN");

    add("lprint using", "", "MSX-BASIC", "LPRINT USING \"simbolo de formato\" [expressao]", "Escreve na impressora, com o formato especificado, o valor de uma expressao.",
        R"MAN(TIPO: INSTRUCAO  (line print using)

  Escreve na impressora, com o formato especificado, o valor de uma expressao.

FORMATO
  LPRINT USING "simbolo de formato" [expressao]

EXEMPLO
  LPRINT USING"####.##";A$;B

DESCRICAO
  Escreve dados na impressora, com formato determinado, da mesma forma que o comando PRINT USING os escreve na tela. As expressoes podem ser: constantes, qualquer tipo de variavel ou expressoes algebricas. Entre as expressoes podem ser utilizados os simbolos "," ou ";". Para maiores detalhes consulte PRINT USING.

PROGRAMA EXEMPLO
    10 REM PROGRAMA LPRINT USING
    20 INPUT A
    30 LPRINT USING"$$######.##-";A
    40 GOTO 20

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 97.

VER TAMBEM
  basic, screens, cores)MAN");

    add("maxfiles", "", "MSX-BASIC", "MAXFILES = numero de arquivos", "Determina o numero de arquivos que podem ser abertos ao mesmo tempo em um programa.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (maxfiles)

  Determina o numero de arquivos que podem ser abertos ao mesmo tempo em um programa.

FORMATO
  MAXFILES = numero de arquivos

EXEMPLO
  MAXFILES=5

DESCRICAO
  Declara o numero de arquivos que podem ser abertos ao mesmo tempo em um programa. O numero de arquivos pode assumir valores entre 0 e 15. Se o numero de arquivos nao for especificado atraves de MAXFILES, somente um pode ser aberto. Nao se deve superdimensionar o valor de MAXFILES (quanto maior for, menor a area de memoria restante para o usuario). O numero de arquivos pode ser constituido de constantes, variaveis numericas, variaveis indexadas, ou suas expressoes.

PROGRAMA EXEMPLO
    10 REM PROGRAMA MAXFILES
    20 MAXFILES=2
    30 OPEN"GRP:"FOR OUTPUT AS #1
    40 OPEN"CRT:"FOR OUTPUT AS #2
    50 SCREEN 2
    60 LINE(20,20)-(235,171),7,BF
    70 LINE(50,50)-(205,141),4,BF
    80 CIRCLE(128,80),30,6
    90 PSET(107,70)
    100 PRINT#1,"EXPERT"
    110 CIRCLE(128,100),20,6
    120 PSET(119,94)
    130 PRINT#1,"MSX"
    140 CLOSE 1
    150 FOR F=1 TO 3000:NEXT F
    160 SCREEN 0
    170 PRINT#2,"EXPERT"
    180 PRINT#2,"MSX"
    190 CLOSE 2

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 98.

VER TAMBEM
  basic, screens, cores)MAN");

    add("merge", "", "MSX-BASIC", "MERGE \"nome do dispositivo, nome do arquivo\"", "Carrega um programa armazenado em codigo ASCII e o une com o programa que esta na memoria do...",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (merge)

  Carrega um programa armazenado em codigo ASCII e o une com o programa que esta na memoria do computador.

FORMATO
  MERGE "nome do dispositivo, nome do arquivo"

EXEMPLO
  MERGE"CAS:TESTE"

DESCRICAO
  Une um programa que esteja armazenado em codigo ASCII (em um dispositivo externo) com um programa que esteja na memoria do computador.

  Importante: as linhas do programa que se encontram no dispositivo externo serao simplesmente inseridas no programa residente, em sequencia. Se os numeros das linhas do programa externo coincidirem com os numeros das linhas do programa residente, as linhas deste serao substituidas pelas linhas do programa externo.

PROGRAMA EXEMPLO
    10 'Programa da RAM
    20 PRINT"RAM .... 20"
    30 PRINT"RAM .... 30"
    40 PRINT"RAM .... 40"
    50 PRINT"RAM .... 50"
    60 PRINT"RAM .... 60"
    70 PRINT"RAM .... 70"

    5 'Programa da FITA
    15 PRINT"fita _ 15"
    25 PRINT"fita _ 25"
    35 PRINT"fita _ 35"
    45 PRINT"fita _ 45"
    55 PRINT"fita _ 55"
    65 PRINT"fita _ 65"
    75 PRINT"fita _ 75"
    80 PRINT"fita _ 80 *"

    MERGE"CAS:FITA"

    Resultado (programa unido):
    5 'Programa da FITA
    10 'Programa da RAM
    15 PRINT"fita _ 15"
    20 PRINT"RAM .... 20"
    25 PRINT"fita _ 25"
    30 PRINT"RAM .... 30"
    35 PRINT"fita _ 35"
    40 PRINT"RAM .... 40"
    45 PRINT"fita _ 45"
    50 PRINT"RAM .... 50"
    55 PRINT"fita _ 55"
    60 PRINT"RAM .... 60"
    65 PRINT"fita _ 65"
    70 PRINT"RAM .... 70"
    75 PRINT"fita _ 75"
    80 PRINT"fita _ 80 *"

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 99.

VER TAMBEM
  basic, screens, cores)MAN");

    add("mid$", "", "MSX-BASIC", "MID$(A$,m,[,n])", "Seleciona parte de strings (cadeias alfanumericas).",
        R"MAN(TIPO: FUNCAO  (middle dollar)

  Seleciona parte de strings (cadeias alfanumericas).

FORMATO
  MID$(A$,m,[,n])

EXEMPLO
  A$=MID$("GOEDELESCHERBACH",7,6)

DESCRICAO
  Seleciona um subconjunto de caracteres de A$ (substring), com um comprimento n e comecando na posicao m. Os valores de m e n devem estar entre 0 e 255. Se n nao for inteiro somente sua parte inteira sera considerada. Se m for maior que o comprimento da string ou se n for zero, a string resultante sera vazia. Se n for omitido, a string resultante sera formada pelos caracteres de A$ a partir da posicao m.

PROGRAMA EXEMPLO
    10 REM PROGRAMA MID$
    20 A$="GOEDELESCHERBACH   ":CLS
    30 FOR F=1 TO LEN(A$)-3
    40 LOCATE 16,10:FOR D=1 TO 100:NEXT D
    50 PRINT MID$(A$,F,3)
    60 NEXT F
    70 GOTO 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 100.

VER TAMBEM
  basic, screens, cores)MAN");

    add("mid$=", "", "MSX-BASIC", "MID$(A$,m,[,n])=B$", "Substitui uma parte de uma string (cadeia alfanumerica) por elementos de outra string.",
        R"MAN(TIPO: INSTRUCAO  (middle dollar)

  Substitui uma parte de uma string (cadeia alfanumerica) por elementos de outra string.

FORMATO
  MID$(A$,m,[,n])=B$

EXEMPLO
  MID$(A$,3,4)=B$

DESCRICAO
  Enxerta em A$, a partir do m-esimo caractere, n caracteres de B$. Os valores de m e n devem estar entre 0 e 255. Se n for omitido o m-esimo caractere e os seguintes serao substituidos.
  Importante: o comprimento de A$ nao e alterado e a substituicao para quando o ultimo caractere de A$ for substituido.

PROGRAMA EXEMPLO
    10 REM PROGRAMA MID$=
    20 A$="GOEDELESCHERBACH GOEDELESCHERBACH":CLS
    30 C=INT(1+LEN(A$)*RND(1))
    40 D=INT(1+LEN(A$)*RND(1))
    50 MID$(A$,C,1)=MID$(A$,D,1)
    60 LOCATE 3,10:FOR D=1 TO 100:NEXT D
    70 PRINT A$
    80 GOTO 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 101.

VER TAMBEM
  basic, screens, cores)MAN");

    add("motor", "", "MSX-BASIC", "MOTOR [ON]", "Liga ou desliga o gravador.",
        R"MAN(TIPO: INSTRUCAO  (motor)

  Liga ou desliga o gravador.

FORMATO
  MOTOR [ON]
  MOTOR [OFF]

EXEMPLO
  MOTOR
  MOTOR ON

DESCRICAO
  A instrucao MOTOR ON liga o gravador e a MOTOR OFF desliga. Se for somente digitado MOTOR o computador desligara o gravador que estiver ligado e ligara se estiver desligado.

PROGRAMA EXEMPLO
    10 REM PROGRAMA MOTOR
    20 INPUT " QUANTOS SEGUNDOS O GRAVADOR DEVE FICAR LIGADO";T
    30 MOTOR ON
    40 TIME=0
    50 IF TIME<T*60 THEN 50
    60 MOTOR OFF

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 102.

VER TAMBEM
  basic, screens, cores)MAN");

    add("new", "", "MSX-BASIC", "NEW", "Apaga o programa da memoria.",
        R"MAN(TIPO: INSTRUCAO  (new)

  Apaga o programa da memoria.

FORMATO
  NEW

EXEMPLO
  NEW

DESCRICAO
  O comando NEW apaga o programa em BASIC e suas variaveis da memoria. Ele e usado quando se quer digitar um novo programa. Se existir um programa em linguagem de maquina na memoria do computador ele nao sera apagado com o comando NEW.

PROGRAMA EXEMPLO
    10 REM PROGRAMA NEW
    20 PRINT"ESTE PROGRAMA SE AUTO-DESTRUIRA EM 10 SEGUNDOS."
    30 TIME=0
    40 IF TIME=600 THEN NEW
    50 GOTO 40

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 103.

VER TAMBEM
  basic, screens, cores)MAN");

    add("oct$", "", "MSX-BASIC", "OCT$ (argumento)", "Transforma um dado numerico em uma string na forma octal.",
        R"MAN(TIPO: FUNCAO  (octonary dollar)

  Transforma um dado numerico em uma string na forma octal.

FORMATO
  OCT$ (argumento)

EXEMPLO
  A$=OCT$(56)

DESCRICAO
  A funcao OCT$ transforma um dado numerico para a forma octal armazenando-o no formato de uma string. O valor do argumento deve estar entre -32768 e 65535 e pode ser expresso atraves de uma constante, uma variavel numerica, ou uma variavel indexada. Se o argumento for um numero negativo, primeiramente calcula-se o valor do argumento subtraido de 65536 e depois e feita a transformacao para a forma octal.

PROGRAMA EXEMPLO
    10 REM PROGRAMA OCT$
    20 FOR F=0 TO 20
    30 PRINT F;".......";OCT$(F)
    40 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 104.

VER TAMBEM
  basic, screens, cores)MAN");

    add("on error goto", "", "MSX-BASIC", "ON ERROR GOTO numero da linha", "Desvia o programa para uma determinada linha quando ocorre um erro na execucao ou na entrada de...",
        R"MAN(TIPO: INSTRUCAO  (on error goto)

  Desvia o programa para uma determinada linha quando ocorre um erro na execucao ou na entrada de dados.

FORMATO
  ON ERROR GOTO numero da linha

EXEMPLO
  ON ERROR GOTO 100,200,300,400,500

DESCRICAO
  A instrucao ON ERROR GOTO desvia o programa para uma certa linha definida pelo usuario, em caso de erro na execucao do programa ou na entrada de dados. Esta instrucao e muito util para evitar paradas inuteis nos programas.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ON ERROR
    20 PRINT
    30 ON ERROR GOTO 90
    40 PRINT"DIGITE UM NUMERO NEGATIVO."
    50 INPUT A
    60 PRINT SQR(A)
    70 FOR F=1 TO 300: NEXT F
    80 RUN
    90 PRINT"NAO EXISTE RAIZ REAL."
    100 RUN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 105.

VER TAMBEM
  basic, screens, cores)MAN");

    add("on interval-gosub", "on interval,gosub", "MSX-BASIC", "ON INTERVAL = intervalo GOSUB numero da linha", "Desvia a execucao do programa para uma sub-rotina em um intervalo de tempo periodico.",
        R"MAN(TIPO: INSTRUCAO  (on interval go to subroutine)

  Desvia a execucao do programa para uma sub-rotina em um intervalo de tempo periodico.

FORMATO
  ON INTERVAL = intervalo GOSUB numero da linha

EXEMPLO
  ON INTERVAL=60 GOSUB 100

DESCRICAO
  A instrucao ON INTERVAL GOSUB desvia o programa para uma sub-rotina. No MSX existe um temporizador que causa uma interrupcao a cada 1/60 de segundo. O intervalo apontado no formato e exatamente o numero de interrupcoes que serao feitas para que o programa seja desviado. No exemplo anterior acontece o seguinte: o computador espera que sejam feitas 60 interrupcoes pelo temporizador para executar a sub-rotina da linha 100.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ON INTERVAL
    20 ON INTERVAL=50 GOSUB 50
    30 INTERVAL ON
    40 GOTO 40
    50 C=C+1
    60 PRINT C
    70 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 108.

VER TAMBEM
  basic, screens, cores)MAN");

    add("on key gosub", "", "MSX-BASIC", "ON KEY GOSUB numero da linha [,numero da linha] [,numero da linha]...", "Desvia o programa para uma certa sub-rotina, dependendo da tecla de funcao que for pressionada.",
        R"MAN(TIPO: INSTRUCAO  (on key go to subroutine)

  Desvia o programa para uma certa sub-rotina, dependendo da tecla de funcao que for pressionada.

FORMATO
  ON KEY GOSUB numero da linha [,numero da linha] [,numero da linha]...

EXEMPLO
  ON KEY GOSUB 10,20,30

DESCRICAO
  A instrucao ON KEY GOSUB e utilizada para o desvio do programa para uma sub-rotina quando se pressiona uma das teclas de funcoes. No exemplo dado acima, caso se pressione a tecla de funcoes 1, o programa vai para a sub-rotina da linha 10, se for pressionada a segunda tecla de funcoes o programa vai para a sub-rotina da linha 20, e finalmente, se for pressionada a tecla de funcoes 3 o programa salta para a sub-rotina da linha 30.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ON KEY
    20 ON KEY GOSUB 60,80,100,120,140
    30 KEY(5) ON:KEY(4) ON:KEY(3) ON:KEY(2) ON:KEY(1) ON
    40 CLS:PRINT"Pressione uma tecla funcional."
    50 GOTO 50
    60 PRINT"Tecla 1 pressionada."
    70 RETURN
    80 PRINT"Tecla 2 pressionada."
    90 RETURN
    100 PRINT"Tecla 3 pressionada."
    110 RETURN
    120 PRINT"Tecla 4 pressionada."
    130 RETURN
    140 PRINT"Tecla 5 pressionada."
    150 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 109.

VER TAMBEM
  basic, screens, cores)MAN");

    add("on sprite gosub", "", "MSX-BASIC", "ON SPRITE GOSUB [numero de linha]", "Desvia o programa para uma sub-rotina caso duas figuras moveis na tela se sobreponham.",
        R"MAN(TIPO: INSTRUCAO  (on sprite go to subroutine)

  Desvia o programa para uma sub-rotina caso duas figuras moveis na tela se sobreponham.

FORMATO
  ON SPRITE GOSUB [numero de linha]

EXEMPLO
  ON SPRITE GOSUB 500

DESCRICAO
  A instrucao ON SPRITE GOSUB indica em que linha se inicia a sub-rotina caso exista a sobreposicao de duas figuras. Esta instrucao e muito util na confeccao de jogos de acao.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ON SPRITE
    20 SCREEN 2,1
    30 SPRITE$(0)=CHR$(&H3C)+CHR$(&H7E)+CHR$(&H81)+CHR$(&H81)+CHR$(&HFF)+CHR$(&H7E)+CHR$(&H24)+CHR$(&H42)
    40 ON SPRITE GOSUB 110
    50 SPRITE ON
    60 FOR X=0 TO 255
    70 PUT SPRITE 0,(X,100),15,0
    80 PUT SPRITE 1,(255-X,100),10,0
    90 NEXT X
    100 RUN
    110 SPRITE OFF
    120 BEEP
    130 SPRITE ON
    140 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 110.

VER TAMBEM
  basic, screens, cores)MAN");

    add("on stop gosub", "", "MSX-BASIC", "ON STOP GOSUB numero de linha", "Instrucao que indica para que sub-rotina o programa deve ser desviado caso se pressione CONTROL...",
        R"MAN(TIPO: INSTRUCAO  (on stop gosub)

  Instrucao que indica para que sub-rotina o programa deve ser desviado caso se pressione CONTROL + STOP.

FORMATO
  ON STOP GOSUB numero de linha

EXEMPLO
  ON STOP GOSUB 100

DESCRICAO
  A instrucao ON STOP GOSUB desvia o programa para a sub-rotina indicada pelo numero da linha apos o GOSUB, quando se pressionam as teclas CONTROL e STOP simultaneamente.

PROGRAMA EXEMPLO
    10 REM Programa ON STOP
    20 ON STOP GOSUB 70
    30 STOP ON
    40 SCREEN 0
    50 PRINT"Pressione: CONTROL + STOP"
    60 GOTO 60
    70 PRINT"As teclas CONTROL e STOP foram"
    80 PRINT"pressionadas!"
    90 PRINT"Para parar o programa, digite"
    100 PRINT"a barra de espacos!"
    110 IF STRIG(0)=0 THEN 110 ELSE END
    120 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 111.

VER TAMBEM
  basic, screens, cores)MAN");

    add("on strig gosub", "", "MSX-BASIC", "ON STRIG GOSUB numero de linha [,numero de linha ...]", "Desvia para uma sub-rotina caso a barra de espacos ou os botoes do joystick sejam pressionados.",
        R"MAN(TIPO: INSTRUCAO  (on stick trigger go to subroutine)

  Desvia para uma sub-rotina caso a barra de espacos ou os botoes do joystick sejam pressionados.

FORMATO
  ON STRIG GOSUB numero de linha [,numero de linha ...]

EXEMPLO
  ON STRIG GOSUB 100,200,300,400,500

DESCRICAO
  A instrucao ON STRIG GOSUB tem como funcao desviar para uma sub-rotina caso a barra de espacos ou algum dos botoes dos joysticks sejam pressionados. Depois do GOSUB devem vir os numeros das linhas onde se iniciam as sub-rotinas, na seguinte ordem:

  1. - numero da linha que indica que a barra foi pressionada.
  2. - numero da linha que indica que o botao 1 do joystick A foi pressionado.
  3. - numero da linha que indica que o botao 1 do joystick B foi pressionado.
  4. - numero da linha que indica que o botao 2 do joystick A foi pressionado.
  5. - numero da linha que indica que o botao 2 do joystick B foi pressionado.

PROGRAMA EXEMPLO
    10 REM PROGRAMA ON STRIG
    20 ON STRIG GOSUB 90,110,130,150,170
    30 FOR F=0 TO 4
    40 STRIG(F) ON
    50 NEXT F
    60 CLS
    70 PRINT"Pressione a barra de espacos ou um dos botoes dos joysticks."
    80 GOTO 80
    90 PRINT"Barra de espacos pressionada."
    100 RETURN
    110 PRINT"Botao 1 pressionado (joystick A)"
    120 RETURN
    130 PRINT"Botao 1 pressionado (joystick B)"
    140 RETURN
    150 PRINT"Botao 2 pressionado (joystick A)"
    160 RETURN
    170 PRINT"Botao 2 pressionado (joystick B)"
    180 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 112.

VER TAMBEM
  basic, screens, cores)MAN");

    add("on-gosub", "on,gosub", "MSX-BASIC", "ON variavel GOSUB numero da linha [,numero da linha, ...]", "Desvia o programa para uma sub-rotina condicionada pelo valor de uma variavel.",
        R"MAN(TIPO: INSTRUCAO  (on-goto subroutine)

  Desvia o programa para uma sub-rotina condicionada pelo valor de uma variavel.

FORMATO
  ON variavel GOSUB numero da linha [,numero da linha, ...]

EXEMPLO
  ON B GOSUB 10,20,30

DESCRICAO
  A instrucao ON-GOSUB funciona analogamente ao ON-GOTO, so que desta vez o programa sera desviado para uma sub-rotina. ON-GOSUB tambem e uma condensacao de comandos condicionados e pode ser substituido por:
  IF B=1 THEN GOSUB 10
  IF B=2 THEN GOSUB 20
  IF B=3 THEN GOSUB 30

PROGRAMA EXEMPLO
    10 REM PROGRAMA ON ... GOSUB
    20 INPUT"ENTRE COM UM NUMERO ENTRE 1 E 3";A
    30 IF A<1 OR A>3 THEN 20
    40 ON A GOSUB 100,200,300
    50 RUN
    100 PRINT"SUB-ROTINA 1"
    110 RETURN
    200 PRINT"SUB-ROTINA 2"
    210 RETURN
    300 PRINT"SUB-ROTINA 3"
    310 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 106.

VER TAMBEM
  basic, screens, cores)MAN");

    add("on-goto", "on,goto", "MSX-BASIC", "ON variavel GOTO numero da linha [, numero da linha, ...]", "Desvio condicionado por uma variavel.",
        R"MAN(TIPO: INSTRUCAO  (on-goto)

  Desvio condicionado por uma variavel.

FORMATO
  ON variavel GOTO numero da linha [, numero da linha, ...]

EXEMPLO
  ON A GOTO 10,20,30

DESCRICAO
  A instrucao ON-GOTO desvia para uma certa linha do programa dependendo do valor de uma variavel. O valor da variavel que condicionara o desvio deve ser um numero inteiro entre 0 e 255. Se o numero nao for inteiro, as casas decimais serao desprezadas. No exemplo acima ocorre o seguinte: se o valor de A for 1, entao o programa desvia para a linha 10, se for 2 para a linha 20, e se 3 para a linha 30. A instrucao ON-GOTO pode ser encarada como a condensacao dos comandos:
  IF A=1 THEN GOTO 10
  IF A=2 THEN GOTO 20
  IF A=3 THEN GOTO 30

PROGRAMA EXEMPLO
    10 REM PROGRAMA ON GOTO
    20 B=INT(RND(1)*3)+1
    30 FOR F=1 TO 400:NEXT F
    40 ON B GOTO 60,80,100
    50 GOTO 20
    60 PRINT"Desvio    1"
    70 GOTO 20
    80 PRINT"Desvio    2"
    90 GOTO 20
    100 PRINT"Desvio    3"
    110 GOTO 20

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 107.

VER TAMBEM
  basic, screens, cores)MAN");

    add("open", "", "MSX-BASIC", "OPEN \"nome do dispositivo [nome do arquivo]\"", "Abre um arquivo e especifica modo leitura/escrita.",
        R"MAN(TIPO: INSTRUCAO  (open)

  Abre um arquivo e especifica modo leitura/escrita.

FORMATO
  OPEN "nome do dispositivo [nome do arquivo]"
  FOR modo AS [#] numero do arquivo

EXEMPLO
  OPEN"CAS:MICRO" FOR INPUT AS # 4

DESCRICAO
  Abrir arquivo com um numero especificado e realizar operacoes de entrada/saida para um dispositivo especificado. Os dispositivos sao:

  Cassete .......................CAS:
  Tela no modo de texto..........CRT:
  Tela no modo grafico...........GRP:
  Impressora......................LPT:

  O nome do arquivo pode ser formado por ate seis caracteres (os excedentes sao ignorados). O modo pode ser OUTPUT (para escrita) e INPUT (para leitura). O numero de arquivo deve estar entre 1 e o numero especificado por MAXFILES.

  NOTA: CTR:, GRP: e LPT: estao destinados a escrita. Neste caso, portanto, o modo so podera ser especificado como OUTPUT.

PROGRAMA EXEMPLO
    10 REM PROGRAMA OPEN
    20 MAXFILES=1
    30 CLS
    40 OPEN "GRP:" FOR OUTPUT AS #1
    50 SCREEN 2
    55 LINE(0,0)-(255,191),5,BF
    56 CIRCLE(120,120),50
    60 PRINT #1,"EXPERT"
    70 FOR I=1 TO 1000:NEXT
    80 CLOSE 1
    90 SCREEN 0
    100 LIST

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 113.

VER TAMBEM
  basic, screens, cores)MAN");

    add("out", "", "MSX-BASIC", "OUT numero da porta, expressao", "Coloca um byte na porta especificada.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (out)

  Coloca um byte na porta especificada.

FORMATO
  OUT numero da porta, expressao

EXEMPLO
  OUT 13,240

DESCRICAO
  Enviar dados diretamente a uma porta especificada.

PROGRAMA EXEMPLO
    10 SCREEN 0:OUT 153,0:OUT 153,0
    20 FOR I=0 TO 959
    30 OUT 152,65
    40 NEXT I

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 114.

VER TAMBEM
  basic, screens, cores)MAN");

    add("pad", "", "MSX-BASIC", "PAD (n)", "Verifica o estado do touch pad.",
        R"MAN(TIPO: FUNCAO  (pad)

  Verifica o estado do touch pad.

FORMATO
  PAD (n)

EXEMPLO
  PAD(2)

DESCRICAO
  Mostrar o estado do touch pad. Se n estiver entre 0 e 3, PAD(n) mostrara o estado do touch pad ligado ao terminal A. Entre 4 e 7, o valor obtido correspondera ao terminal B.

  Valor de n (terminal A / terminal B) e significado de PAD(n):
  - 0 / 4 - 0 tocado; -1 nao tocado
  - 1 / 5 - coordenada x do lugar tocado
  - 2 / 6 - coordenada y do lugar tocado
  - 3 / 7 - 0 interruptor pressionado; -1 interruptor nao pressionado

PROGRAMA EXEMPLO
    10 REM PROGRAMA PAD
    20 CLS
    30 C=0
    40 COLOR,C,C
    50 IF PAD(3)=0 THEN GOTO 40
    60 C=C+1:IF C=15 THEN C=0
    70 GOTO 40

NO MSX2+ (Manual MSX 2+ FM)
TIPO: FUNCAO  (pad)

  Retorna o estado da TOUCHPAD, LIGHTPEN, MOUSE ou TRACKBALL que estejam ligados as entradas de JOYSTICK - versao MSX2+.

FORMATO
  PAD X

EXEMPLO
  PRINT PAD(3)

DESCRICAO
  X deve ser um numero inteiro entre 0 e 19 e tem o seguinte significado:
  - Se X estiver entre 0 e 3 presume-se que ha uma TOUCHPAD ligada na tomada do JOYSTICK 1.
  - Se X estiver entre 4 e 7 presume-se que ha uma TOUCHPAD ligada na tomada do JOYSTICK 2.
  - Se X estiver entre 8 e 11 presume-se que tenha uma LIGHTPEN ligada na tomada do JOYSTICK.
  - Se X estiver entre 12 e 15 presume-se que tenha um MOUSE ou TRACKBALL ligado na tomada do JOYSTICK 1.
  - Se X estiver entre 16 e 19 presume-se que tenha um MOUSE ou TRACKBALL ligado na tomada do JOYSTICK 2.

  Nos casos acima se X for igual a:
  - 0 ou 4 - retorna o estado da mesa da TOUCHPAD: 0 se nao foi tocada ou -1 se foi tocada.
  - 1 ou 5 - retorna a coordenada X do ponto onde a mesa foi tocada.
  - 2 ou 6 - retorna a coordenada Y do ponto onde a mesa foi tocada.
  - 3 ou 7 - retorna o estado do botao da TOUCHPAD. O resultado sera: 0 (botao nao pressionado) ou -1 (botao pressionado).
  - 8 - retorna o estado da LIGHTPEN: 0 se as coordenadas nao estiverem prontas ou -1 se as coordenadas estiverem prontas.
  - 9 - retorna a coordenada X da posicao da LIGHTPEN.
  - 10 - retorna a coordenada Y da posicao da LIGHTPEN.
  - 11 - retorna o estado do botao da LIGHTPEN: 0 (botao nao pressionado) ou -1 (botao pressionado).
  - 12 ou 16 - retorna o estado do MOUSE ou TRACKBALL: e sempre -1.
  - 13 ou 17 - retorna a coordenada X do MOUSE ou TRACKBALL.
  - 14 ou 18 - retorna a coordenada Y do MOUSE ou TRACKBALL.
  - 15 ou 19 - retorna sempre 0.

  A leitura do estado dos botoes do MOUSE ou TRACKBALL e feita com a instrucao STRIG.

  Antes de ler as coordenadas do MOUSE ou TRACKBALL e obrigatoria a execucao da funcao PAD (12) ou PAD (16) dependendo da tomada onde o MOUSE ou TRACKBALL estejam ligados.

  Antes de ler as coordenadas da LIGHTPEN, verifique se a funcao PAD(8) retorna o valor -1.

  A funcao PAD so tera validade se algum dos acessorios (TOUCHPAD, LIGHTPEN, MOUSE ou TRACKBALL) estiver ligado ao computador.

  - TOUCHPAD e uma pequena prancheta usada para desenhos com sensores em toda a sua extensao que, ao serem tocados, transmitem ao computador as coordenadas X,Y. Para acompanhar os tracos do desenho e usada uma caneta com dois botoes: um como nos JOYSTICKS e outro que e acionado quando a caneta toca a superficie da prancheta.
  - LIGHTPEN e uma caneta com um sensor de luz na ponta que, quando encostada na tela do monitor ou televisor, transmite ao computador a sua posicao em coordenadas X,Y. A Lightpen possui dois botoes: um como nos JOYSTICKS e outro que e o sensor quando cruza com um raio luminoso na tela.
  - MOUSE e um controlador manual com uma esfera na sua parte inferior e dois botoes como nos JOYSTICKS. A movimentacao do MOUSE sobre qualquer superficie faz com que a esfera movimente duas rodas perfuradas, uma para o eixo X e outra para o eixo Y. Estas rodas geram uma sequencia de pulsos que sao transformados em coordenadas pelo computador.
  - TRACKBALL e semelhante ao MOUSE, porem, a esfera fica na parte superior do controlador. Ao contrario do Mouse, sua esfera e controlada pela mao do operador que a faz girar para o lado desejado.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 115.

VER TAMBEM
  basic, screens, cores)MAN");

    add("paint", "", "MSX-BASIC", "PAINT [STEP] (X,Y) [, cor da pintura]", "Preenche uma certa area com uma cor especificada pelo usuario.",
        R"MAN(TIPO: INSTRUCAO  (paint)

  Preenche uma certa area com uma cor especificada pelo usuario.

FORMATO
  PAINT [STEP] (X,Y) [, cor da pintura]
  [,cor do limite da area]

EXEMPLO
  PAINT(70,70),10,10

DESCRICAO
  A instrucao PAINT colore com a cor especificada pelo usuario uma area pre-definida. O valor da coordenada X pode variar de 0 ate 255 enquanto a coordenada Y pode variar de 0 ate 191. No modo de alta resolucao (SCREEN 2) a cor da linha que limita a area deve ser igual a cor que preenchera a area. Isto nao acontece no modo de menor resolucao (SCREEN 3). Para maiores detalhes sobre o STEP veja o comando PSET.

PROGRAMA EXEMPLO
    10 REM PROGRAMA PAINT
    20 SCREEN 2
    30 FOR F=1 TO 90
    40 X=RND(1)*256
    50 Y=RND(1)*192
    60 C=RND(1)*14+2
    70 R=RND(1)*30
    80 CIRCLE (X,Y),R,C
    90 PAINT (X,Y),C,C
    100 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 116.

VER TAMBEM
  basic, screens, cores)MAN");

    add("pdl", "", "MSX-BASIC", "PDL (n)", "Apresenta o valor determinado pelo paddle.",
        R"MAN(TIPO: FUNCAO  (paddle)

  Apresenta o valor determinado pelo paddle.

FORMATO
  PDL (n)

EXEMPLO
  PDL(3)

DESCRICAO
  Transforma em dados numericos as posicoes de um paddle. Se n for um numero impar os dados obtidos serao correspondentes ao paddle conectado ao terminal A. Se n for par os dados serao provenientes do terminal B. O valor de n pode estar entre 0 e 12 e os valores obtidos estarao entre 0 e 255.

PROGRAMA EXEMPLO
    10 REM PROGRAMA PDL
    20 CLS
    30 C=0
    40 COLOR,C,C
    50 IF PDL(1)=0 THEN GOTO 40
    60 C=C+1:IF C=15 THEN C=0
    70 GOTO 40

NO MSX2+ (Manual MSX 2+ FM)
TIPO: FUNCAO  (paddle)

  Retorna o estado de PADDLES ligados ao computador - versao MSX2+, com ate 12 paddles.

FORMATO
  PDL (X)

EXEMPLO
  PRINT PDL(1) : GOTO 10

DESCRICAO
  Ate 12 (doze) PADDLES podem ser ligados as entradas de JOYSTICK (6 na entrada A e 6 na B). Se X for 1, 3, 5, 7, 9 ou 11 presume-se que os PADDLES estejam ligados na entrada do JOYSTICK-1 (A). Se for 2, 4, 6, 8, 10 ou 12 presume-se que estejam ligados na entrada do JOYSTICK-2 (B). Esta funcao retorna um valor entre 0 e 255. PADDLEs sao controles rotativos utilizados em jogos ou programas que tem movimentos em duas direcoes (direita e esquerda por exemplo). A funcao PDL(X) so e valida com um ou mais PADDLES ligados ao micro.

PROGRAMA EXEMPLO
    10 PRINT PDL(1) : GOTO10

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 117.

VER TAMBEM
  basic, screens, cores)MAN");

    add("peek", "", "MSX-BASIC", "PEEK (endereco)", "Apresenta o valor armazenado em um endereco da memoria.",
        R"MAN(TIPO: FUNCAO (uso avancado)  (peek)

  Apresenta o valor armazenado em um endereco da memoria.

FORMATO
  PEEK (endereco)

EXEMPLO
  PEEK(1020)

DESCRICAO
  Determina o valor do byte armazenado em um determinado endereco da memoria. Este endereco pode ser uma constante, variavel numerica, variavel indexada ou expressoes.

PROGRAMA EXEMPLO
    10 REM PROGRAMA PEEK
    20 FOR F=0 TO 8191
    30 PRINT F;"......."PEEK(F)
    40 FOR G=1 TO 200:NEXT G
    50 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 118.

VER TAMBEM
  basic, screens, cores)MAN");

    add("play", "", "MSX-BASIC", "PLAY subcomandos", "Toca sequencias de notas e/ou acordes musicais compostos de uma a tres notas simultaneamente,...",
        R"MAN(TIPO: INSTRUCAO  (play)

  Toca sequencias de notas e/ou acordes musicais compostos de uma a tres notas simultaneamente, com tempo, oitava, duracao, tom e volume programavel.

FORMATO
  PLAY subcomandos

EXEMPLO
  PLAY"s0m5000v15cdefgab"

DESCRICAO
  Gerar, utilizando o Gerador Programavel de Som (PSG), sequencias musicais compostas de ate tres notas simultaneas, especificadas por subcomandos. Esses subcomandos podem ser representados por strings dentro de " " ou por variaveis string. Por exemplo:

  10 PLAY"C"
  20 GOTO 10
  -> toca uma nota repetidamente

  10 PLAY"G","E","B"
  20 GOTO 10
  -> toca um acorde com tres notas

  10 M$="GEB"
  20 N$="EGB"
  30 O$="BEG"
  -> atribui a string uma sequencia de notas musicais

  40 PLAY M$,N$,O$
  -> toca as sequencias utilizando simultaneamente os tres canais de som

  50 M$="GEB"
  60 PLAY M$,"02EGB","06BEG"
  -> tres canais de som em diferentes oitavas

  SUBCOMANDOS DA FUNCAO PLAY:

  - Tn (tempo): valores permitidos de 32 a 255. Determina o andamento da musica. O valor inicial e T120.

  - On (oitava): valores permitidos de 1 a 8. Determina uma das 8 oitavas do MSX. O valor inicial e 04.

  - Ln (duracao): valores permitidos de 1 a 64 (L1, L2, L4, L8, L16, L32, L64 - da semibreve ate a semifusa). Determina a duracao da nota. O valor inicial e L4.

  - Nn (nota): valores permitidos de 0 a 96. Especifica uma nota musical (N0 e uma pausa; quando n aumenta de uma unidade a nota sobe meio tom).

  - A-G / An-Gn: valores permitidos de 1 a 64. Especifica a nota musical dentro de uma oitava pre-determinada. O sinal "#" (ou "+") colocado ao lado da nota especifica um sustenido. O sinal "-" determina um bemol. Por exemplo:
    PLAY"C-"    tres notas
    PLAY"C"     separadas
    PLAY"C#"    por meio tom
    A duracao da nota pode ser fixada atraves de n. (D4 e o mesmo que L4D). Quando omitido, a duracao sera a determinada por Ln.

  - Rn (pausa): valores permitidos de 1 a 64 (R1, R2, R4, R8, R16, R32, R64). Determina uma pausa.

  - . (ponto): aumenta a duracao de uma nota ou de uma pausa em 50%.

  - Vn (volume): valores permitidos de 0 a 15. Determina o volume. O volume aumenta com o valor de n. O valor inicial e V8.

  - Mn (periodo do envelope): valores permitidos de 0 a 65535. Determina o periodo da variacao de volume durante a execucao da nota. Veja a funcao SOUND, nota (i) para mais informacoes.

  - Sn (forma do envelope): valores permitidos de 0 a 15. Determina o formato do envelope. Para maiores detalhes veja a funcao SOUND (tabela de envelopes e nota (j)).

PROGRAMA EXEMPLO
    10 REM PROGRAMA PLAY
    20 PLAY"S0M10000V15T180"
    30 PLAY"L403GFL8EDEC02BABG"
    40 PLAY"03L4CEAGB04L3C03"
    50 PLAY"L8AGFEL3EL4DGB04C"
    60 PLAY"L6CEL4DL6DFL4ECFEDE"
    70 PLAY"L6FEDCL4C03B"
    80 PLAY"04L4CC#L3DL6DFL4E"
    90 PLAY"L6EGL4FD03GB04C"

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 119.

VER TAMBEM
  basic, screens, cores)MAN");

    add("play (msx2+/fm-music)", "", "MSX-BASIC", "PLAY#n,C1,C2,....,C8,C9,PSG1,PSG2,PSG3", "Toca a musica de acordo com os parametros nele definidos (nota, volume, duracao, tempo, etc.) -...",
        R"MAN(TIPO: INSTRUCAO  (play)

  Toca a musica de acordo com os parametros nele definidos (nota, volume, duracao, tempo, etc.) - versao MSX2+/FM-Music, com canais adicionais de FM alem dos 3 canais do PSG.

FORMATO
  PLAY#n,C1,C2,....,C8,C9,PSG1,PSG2,PSG3

EXEMPLO
  PLAY#2,C1,C2,C3,C4,C5,C6,C7,C8,C9,PSG1,PSG2,PSG3

DESCRICAO
  O comando PLAY original do MSX sofreu uma pequena alteracao: recebeu um parametro a mais, o #<n>, para indicar que estamos nos reportando ao FM.

  PLAY#<n>,<C1>,<C2>,<C3>,<C4>,<C5>,<C6>,<C7>,<C8>,<C9>,<PSG1>,<PSG2>,<PSG3>

  Esta primeira sintaxe se refere a utilizacao sem bateria. Nesse caso, teremos 9 canais disponiveis, (<C1> a <C9>), mais os 3 canais do PSG, (<PSG1> a <PSG3>).

  ou

  PLAY#<n>,<C1>,<C2>,<C3>,<C4>,<C5>,<C6>,<BT>,<PSG1>,<PSG2>,<PSG3>

  Nesta segunda sintaxe utilizamos a bateria (lembre-se que isso depende do comando CALL MUSIC). Nesse caso, teremos 6 canais (<C1> a <C6>), a bateria (<BT>) e os 3 canais do PSG (<PSG1> a <PSG3>). Note que a bateria nao faz parte dos canais de musica. Ela sera definida sempre depois do ultimo canal musical. Assim, se fizermos: CALL MUSIC (1,0,1,1,1,1,1). Teremos que definir a bateria no quinto canal do comando PLAY: PLAY#2,C1,C2,C3,C4,BT.

  O <n> pode ser substituido por 0, 2 ou 3. Se for 2 ou 3, valem as sintaxes acima explicadas. Se for 0, indica que estaremos trabalhando apenas com o PSG. Assim, o comando PLAY nao podera ultrapassar 3 canais: PLAY0,PSG1,PSG2,PSG3.

  Todos os labels acima mostrados (<C1> a <C9>, <BT>, <PSG1> a <PSG3>) deverao ser substituidos por parametros colocados entre " " (aspas).

  PARAMETROS PARA SOM:
  - @n - Muda o instrumento dentro do PLAY. n pode variar de 0 a 63 (igual ao CALL VOICE). O default e 0.
  - Vn - Altera o volume do som. n pode variar de 0 a 15 e o default e 8.
  - Tn - Muda o tempo da musica (andamento). n pode ser de 32 a 255, default e 120.
  - Ln - Altera a duracao de cada nota. n varia de 1 a 64. O default e 4.
  - On - Define a oitava nota utilizada. n pode variar de 1 a 8 sendo o default 4.
  - Qn - Muda o "decay" (finalizacao) das notas. n pode ser de 1 a 64. default e 8.
  - Nn - Toca uma nota especificada pelo seu numero. n pode ser de 0 a 96.
  - Rn - Pausa por n tempos. n varia de 1 a 64. O default e 4.
  - "." - Quando colocado ao lado da nota, determina que esta sera tocada por mais 1/2 tempo de seu valor original.
  - @Vn - Assim como o V, esse comando altera o volume do som mas cobre uma faixa de valores maior do que o V. n pode ser de 0 a 127, o default e 127.
  - @Wn - Prolonga a execucao da ultima nota por n tempos. n pode ser de 1 a 64.
  - "&" - Faz a "juncao de 2 notas de mesmo valor".
  - ">" - Aumenta 1 (uma) oitava.
  - "<" - Diminui 1 (uma) oitava.
  - Sn e Mn - Comandos para geracao de envelopes. So funcionam com o PSG. Para maiores detalhes, consulte o manual de BASIC MSX.

  NOTAS MUSICAIS: C (do), D (re), E (mi), F (fa), G (sol), A (la), B (si). As notas podem estar acompanhadas de um numero (de 1 a 64) indicando sua duracao. Se nao especificado, e assumido o valor do parametro L.
  - "#" ou "+" - Acompanhado da nota, indica que esta sera SUSTENIDA (1/2 tom acima).
  - "-" - Indica que a nota sera BEMOL (1/2 tom abaixo).

  Yn1,n2 - Altera a definicao de alguns instrumentos pre-definidos.

  PARAMETROS PARA BATERIA:
  - @An - Indica o volume dos instrumentos tocados em primeiro plano (com enfase). n pode ser de 0 a 15, o default e 12.
  - Vn - Indica o volume dos instrumentos tocados em segundo plano. Identico ao V do SOM.
  - Tn - Identico ao T do SOM.
  - @Vn - Define o volume geral (primeiro e segundo planos). Identico ao @V do SOM.
  - "!" - Enfase no instrumento (este e tocado no primeiro plano).
  - Rn - Identico ao R do SOM.
  - Yn1,n2 - Permite alterar o som dos instrumentos da bateria (maiores detalhes, consulte o comando CALL AUDREG).

  INSTRUMENTOS DE BATERIA:
  - B - Bumbo
  - H - Hi-Hat
  - C - Chimbau
  - S - Caixa
  - M - Tom Tom

  Os instrumentos de bateria devem, obrigatoriamente, vir acompanhados do valor de duracao (de 1 a 64). E possivel tocar ate 5 instrumentos ao mesmo tempo, colocando um ao lado do outro e, no final, o valor de duracao.

  Ex: CALL MUSIC (1):PLAY#2,"BC4". Tocara o BUMBO e o CHIMBAU juntos. Quando colocados dessa forma, os sons dos instrumentos aparecem em segundo plano. Para tocar um instrumento em primeiro plano, deve-se colocar o sinal "!" logo apos.

  CALL MUSIC (1):PLAY#2,"B!H4". Tocara o BUMBO em primeiro plano junto com o HI-HAT em segundo plano. E importante ressaltar que os instrumentos tocados no primeiro plano somente terao enfase se o volume sonoro do segundo plano for menor.

PROGRAMA EXEMPLO
    CALL MUSIC (0,0,1,1,1)
    PLAY#2,"V15O4@00C1G1F1","V15O4@00E1B1A1","V15O4@00G1>D1C1"
    'Nesse exemplo, serao tocados 3 acordes: DO maior, SOL maior e FA maior com som de PIANO (@00).

    CALL MUSIC (1,0)
    PLAY #2,"@A15V0B!H8H8S!H8B!H8B!H8H8S!H8C!H8"
    'Aqui e possivel ouvir um ritmo de bateria (ROCK). Note como foi utilizada a juncao de instrumentos de bateria.

    CALL MUSIC (0,0,3)
    POKE &HFA3C,40:POKE &HFA4C,80
    CALL VOICE (@53,@53,@53)
    PLAY #2,"V15O4C1<G1A1F1"
    'Neste ultimo exemplo, serao tocados algumas notas com o instrumento 53 (HARDROCK) apresentando uma leve reverberacao (explicada no APENDICE B).

FONTE
  Manual MSX 2+ FM (ACVS), pagina 40.

VER TAMBEM
  basic, screens, cores)MAN");

    add("point", "", "MSX-BASIC", "POINT(coluna,linha)", "Obtem o codigo de cor de um ponto especificado na tela.",
        R"MAN(TIPO: FUNCAO  (point)

  Obtem o codigo de cor de um ponto especificado na tela.

FORMATO
  POINT(coluna,linha)

EXEMPLO
  POINT(143,77)

DESCRICAO
  A funcao POINT obtem o codigo de cor de um dado ponto da tela. Os valores da coluna e da linha devem ser uma constante, uma variavel ou uma expressao que resulte um numero entre -32767 e 32767. Se esse valor estiver fora da faixa correspondente a tela, o valor obtido por POINT sera -1.

PROGRAMA EXEMPLO
    10 REM PROGRAMA POINT
    20 SCREEN 3
    30 FOR I=1 TO 25
    40 X=INT(RND(1)*255)
    50 Y=INT(RND(1)*191)
    60 PSET(X,Y),8
    70 NEXT I
    80 FOR X=1 TO 250 STEP 4
    90 FOR Y=1 TO 191 STEP 4
    100 C=POINT(X,Y)
    110 IF C=8 THEN PSET(X,Y),14
    120 NEXT Y,X
    130 FOR B=1 TO 500:NEXT B

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 122.

VER TAMBEM
  basic, screens, cores)MAN");

    add("poke", "", "MSX-BASIC", "POKE endereco, expressao", "Escreve um dado numerico em um certo endereco da memoria.",
        R"MAN(TIPO: INSTRUCAO  (poke)

  Escreve um dado numerico em um certo endereco da memoria.

FORMATO
  POKE endereco, expressao

EXEMPLO
  POKE &HE111,201

DESCRICAO
  A instrucao POKE escreve dados numericos em um certo endereco da memoria. Tanto a memoria como o dado podem ser escritos em decimal ou hexadecimal. Caso voce use numeros hexadecimais, lembre-se de precede-los por &H. O valor do endereco pode variar de -32768 (decimal) a 65535 (decimal) e o valor do dado de 0 (decimal) a 255 (decimal).

PROGRAMA EXEMPLO
    10 REM PROGRAMA ESPELHO
    20 CLS
    30 POKE 40001!,ASC("E")
    40 POKE 40002!,ASC("T")
    50 POKE 40003!,ASC("N")
    60 POKE 40004!,ASC("E")
    70 POKE 40005!,ASC("I")
    80 POKE 40006!,ASC("D")
    90 POKE 40007!,ASC("A")
    100 POKE 40008!,ASC("R")
    110 POKE 40009!,ASC("G")
    120 FOR I=40009! TO 40001! STEP-1!
    130 PRINT I, "....."CHR$(PEEK(I))
    140 NEXT I
    150 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 123.

VER TAMBEM
  basic, screens, cores)MAN");

    add("pos", "", "MSX-BASIC", "POS (X)", "Indica a abcissa X que o cursor ocupa.",
        R"MAN(TIPO: FUNCAO  (position)

  Indica a abcissa X que o cursor ocupa.

FORMATO
  POS (X)

EXEMPLO
  PRINT POS(0)

DESCRICAO
  A funcao POS indica a abcissa que o cursor ocupa, sendo que o valor de X entre parenteses nao tem significado algum (argumento ficticio).

PROGRAMA EXEMPLO
    10 REM PROGRAMA POS
    20 CLS
    30 INPUT A$
    40 PRINT A$;" ";
    50 X=POS(5)
    60 PRINT"POS=";X
    70 GOTO 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 124.

VER TAMBEM
  basic, screens, cores)MAN");

    add("preset", "", "MSX-BASIC", "PRESET [STEP] (X,Y) [,cor]", "Acende ou apaga um ponto nas telas graficas.",
        R"MAN(TIPO: INSTRUCAO  (point reset)

  Acende ou apaga um ponto nas telas graficas.

FORMATO
  PRESET [STEP] (X,Y) [,cor]

EXEMPLO
  PRESET(7,10)

DESCRICAO
  Se for escolhida uma cor diferente da cor do fundo a instrucao PRESET funciona exatamente como a PSET. Se nenhuma cor for especificada, porem, sera plotado um ponto de cor igual a do fundo, dando a impressao de que o ponto foi apagado.

PROGRAMA EXEMPLO
    10 REM PROGRAMA PRESET
    20 SCREEN 3
    30 FOR X=0 TO 255
    40 PSET (X,25),11
    50 PRESET (X,25)
    60 NEXT X
    70 GOTO 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 125.

VER TAMBEM
  basic, screens, cores)MAN");

    add("print", "", "MSX-BASIC", "PRINT expressao[separador expressao separador]", "Apresenta dados na tela.",
        R"MAN(TIPO: INSTRUCAO  (print)

  Apresenta dados na tela.

FORMATO
  PRINT expressao[separador expressao separador]

EXEMPLO
  PRINT "Isto aparecera' na tela"

DESCRICAO
  O comando PRINT apresenta na tela os dados definidos pelas expressoes. Uma expressao pode ser uma constante, uma variavel (numericas ou strings). Se a expressao for numerica, basta escreve-la normalmente, mas se ela for uma string deve estar entre aspas. O separador pode ser uma virgula (,) ou um ponto e virgula (;). A virgula faz com que a expressao a sua frente seja apresentada a partir da coluna 0 ou da coluna 14. O ponto e virgula faz com que a apresentacao seja feita logo a seguir o ultimo dado apresentado.

PROGRAMA EXEMPLO
    10 REM PROGRAMA PRINT
    20 PRINT "GRADIENTE & ALEPH"
    30 GOTO 20

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 126.

VER TAMBEM
  basic, screens, cores)MAN");

    add("print using", "", "MSX-BASIC", "PRINT USING formato;expressao separador ...", "Apresenta dados com um formato especifico na tela.",
        R"MAN(TIPO: INSTRUCAO  (print using)

  Apresenta dados com um formato especifico na tela.

FORMATO
  PRINT USING formato;expressao separador ...

EXEMPLO
  PRINT USING"!";"GRADIENTE","ALEPH"

DESCRICAO
  O comando PRINT USING apresenta dados na tela com um formato definido por um dos simbolos da tabela a seguir:

  - "!" : Apresenta apenas o primeiro caractere de uma string. Exemplo: PRINT USING"!";A$;B$;C$

  - "\n-espaco\" : Apresenta n+2 caracteres de uma string. Exemplo: PRINT USING"\  \";A$;B$;C$

  - "&" : Apresenta todos os caracteres de uma string. Exemplo: PRINT USING"&";A$;B$;C$

  - "#" : Especifica o formato e o numero de digitos apresentados de um dado numerico. Exemplo: PRINT USING"### #.##";455.43557

  - "+" : Acrescenta o sinal + ou - antes ou apos dados numericos conforme eles sejam positivos, nulos ou negativos. Exemplo: PRINT USING"+ ###";-233,3434,675,-13435.232

  - "-" : Acrescenta o sinal - apos numeros negativos. Exemplo: PRINT USING"-";-12312;-13;122

  - "" : Preenche com asteriscos os espacos ocupados por um dado numerico. Exemplo: PRINT USING".##";1.2367

  - "$$" : Acrescenta o simbolo $ antes de dados numericos. Exemplo: PRINT USING"$$ ###";233,3434,675,13435.232

  - "$" : Acrescenta o simbolo $ antes de dados numericos e preenche com asteriscos os espacos nao ocupados. Exemplo: PRINT USING" $ ###";233,3434,675,13435,232

  - "," : Acrescenta uma virgula a cada tres digitos a esquerda do ponto decimal. Exemplo: PRINT USING"####,. ##";1235.122,1133.11378

  - "^^^^" : Apresenta dados numericos com ponto flutuante. Exemplo: PRINT USING"### ##^^^^";1235,122,1133,11378

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 127.

VER TAMBEM
  basic, screens, cores)MAN");

    add("print#", "", "MSX-BASIC", "PRINT# numero de arquivo, expressao", "Escreve dados em arquivo aberto por OPEN.",
        R"MAN(TIPO: INSTRUCAO  (print number)

  Escreve dados em arquivo aberto por OPEN.

FORMATO
  PRINT# numero de arquivo, expressao

EXEMPLO
  PRINT #2,"ACROS"

DESCRICAO
  Introduzir dados em arquivo aberto por OPEN. O numero de arquivos deve estar entre 1 e o especificado por MAXFILES. A expressao pode ser constituida de constantes, variaveis numericas, variaveis indexadas, alfanumericas ou suas expressoes.

PROGRAMA EXEMPLO
    10 REM PROGRAMA PRINT #
    20 MAXFILES=2
    30 OPEN "GRP:" FOR OUTPUT AS #1
    40 OPEN "CRT:" FOR OUTPUT AS #2
    50 SCREEN 2
    60 LINE(10,10)-(240,190),9,BF
    70 CIRCLE(128,80),60,8
    71 PSET(110,70),6
    80 PRINT #1,"EXPERT"
    90 CIRCLE(128,100),18,8
    91 PSET(119,100),3
    100 PRINT #1,"MSX"
    110 FOR I=1 TO 3000:NEXT
    120 CLOSE 1
    130 SCREEN 0
    140 LIST

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 128.

VER TAMBEM
  basic, screens, cores)MAN");

    add("print# using", "", "MSX-BASIC", "PRINT # numero de arquivo USING simbolo de formato, expressao", "Escreve dados com o formato desejado em um arquivo aberto pelo comando OPEN.",
        R"MAN(TIPO: INSTRUCAO  (print number using)

  Escreve dados com o formato desejado em um arquivo aberto pelo comando OPEN.

FORMATO
  PRINT # numero de arquivo USING simbolo de formato, expressao

EXEMPLO
  PRINT#2,USING"###.####";A

DESCRICAO
  Esta instrucao e usada para introduzir dados em um arquivo, com formato especificado. O numero do arquivo deve estar entre 1 e o numero de arquivos definido em MAXFILES. A expressao pode ser formada de variaveis numericas, constantes, variaveis indexadas ou suas expressoes.

PROGRAMA EXEMPLO
    10 REM PROGRAMA PRINT # USING
    20 MAXFILES=1
    30 OPEN "GRP:" FOR OUTPUT AS #1
    50 SCREEN 2
    60 LINE(60,14)-(196,154),9,BF
    70 CIRCLE(128,80),60,12
    71 PSET(106,65),9
    80 PRINT #1,"EXPERT"
    90 CIRCLE(128,100),26,13
    91 PSET(110,100),9
    100 PRINT #1,USING"##.##";14.11
    110 FOR I=1 TO 4000:NEXT
    120 CLOSE 1

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 129.

VER TAMBEM
  basic, screens, cores)MAN");

    add("pset", "", "MSX-BASIC", "PSET [STEP] (coordenada x,coordenada y) [,cor]", "Desenha um ponto na tela de modo grafico.",
        R"MAN(TIPO: INSTRUCAO  (point set)

  Desenha um ponto na tela de modo grafico.

FORMATO
  PSET [STEP] (coordenada x,coordenada y) [,cor]

EXEMPLO
  PSET(129,73),3

DESCRICAO
  Desenha um ponto na tela de modo grafico, determinado pelas coordenadas x e y. Se as coordenadas forem precedidas pelo STEP (opcional), a origem do sistema de eixos sera deslocada do canto superior esquerdo para o ultimo ponto plotado antes da execucao. A cor deve ser indicada por um inteiro entre 0 e 15. Se for omitida, permanece a cor de fundo atual. As coordenadas x e y podem ser constantes, variaveis numericas, variaveis indexadas ou suas expressoes.

PROGRAMA EXEMPLO
    10 REM PROGRAMA PSET
    20 SCREEN 3
    30 FOR C=2 TO 8
    40 PSET (70,50),C
    50 PSET (66,54),C+1
    60 PSET (74,54),C+2
    70 PSET (62,58),C+3
    80 PSET (78,58),C+4
    90 PSET (66,62),C+5
    100 PSET (74,62),C+6
    110 PSET (70,66),C+7
    120 FOR F=1 TO 500:NEXT F
    130 NEXT C
    140 GOTO 30
    150 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 130.

VER TAMBEM
  basic, screens, cores)MAN");

    add("put sprite", "", "MSX-BASIC", "PUT SPRITE numero da camada", "Torna visivel o sprite especificado em um endereco determinado do plano de sprites.",
        R"MAN(TIPO: INSTRUCAO  (put sprite)

  Torna visivel o sprite especificado em um endereco determinado do plano de sprites.

FORMATO
  PUT SPRITE numero da camada
  [[STEP] (coordenada x, coordenada y)],
  [cor], [numero do sprite]

EXEMPLO
  PUT SPRITE 9,(111,25),4,2

DESCRICAO
  Coloca na tela um sprite especificado nas coordenadas desejadas de uma camada tambem especificada. O numero da camada pode variar entre 0 e 31. A coordenada x deve estar entre -32 e 255. A coordenada y deve estar entre -32 e 191. Tanto x quanto y podem ser constantes, variaveis indexadas, variaveis numericas ou suas expressoes. A cor deve ser determinada por inteiros de zero a 15. Se omitida permanece a cor de fundo atual. Se STEP (coordenada x,coordenada y) for omitido fica a posicao anteriormente determinada pela ultima instrucao de graficos. O numero do sprite depende do numero de pontos, ou seja, para 8x8 pontos deve ser um numero entre 0 e 255 e para 16x16 pontos, de 0 a 63. Se for omitido sera igual ao numero da camada a que ele pertence.

PROGRAMA EXEMPLO
    10 REM PROGRAMA PUT SPRITE
    20 SCREEN 2,1
    30 FOR T=1 TO 8
    40 READ A$
    50 S$=S$+CHR$(VAL("&B"+A$))
    60 NEXT T
    70 SPRITE$(1) = S$
    80 PUT SPRITE 0,(128,96),8,1: GOTO 80
    90 DATA    11000111
    100 DATA    11100011
    110 DATA    01110011
    120 DATA    00111011
    130 DATA    11011100
    140 DATA    11001110
    150 DATA    11000111
    160 DATA    11000011
    170 GOTO 170

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 131.

VER TAMBEM
  basic, screens, cores)MAN");

    add("read", "", "MSX-BASIC", "READ variavel [,variavel ...]", "Le os dados que foram armazenados em uma instrucao DATA.",
        R"MAN(TIPO: INSTRUCAO  (read)

  Le os dados que foram armazenados em uma instrucao DATA.

FORMATO
  READ variavel [,variavel ...]

EXEMPLO
  READ A,A$,B

DESCRICAO
  A instrucao READ le os dados armazenados na instrucao DATA. Os dados podem ser numericos ou alfanumericos, desde que o tipo da variavel coincida com o tipo de dado. O READ normalmente comeca a ler os dados a partir da linha DATA de numero mais baixo.

PROGRAMA EXEMPLO
    5 REM PROGRAMA READ
    10 CLS
    20 DIM A$(5), B(5)
    30 FOR I=1 TO 5
    40 READ A$(I), B(I)
    50 NEXT I
    60 PRINT "NOME", " NOTA": PRINT
    70 FOR K=1 TO 5
    80 PRINT A$(K), B(K)
    90 NEXT K
    100 DATA ALDO, 2.75, ROSANA, 3.17, NANCY, 3.65, VANIA, 3.96, ANA, 2.98

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 132.

VER TAMBEM
  basic, screens, cores)MAN");

    add("rem", "", "MSX-BASIC", "REM comentario", "Introducao de comentarios ou observacoes na listagem do programa.",
        R"MAN(TIPO: INSTRUCAO  (remark)

  Introducao de comentarios ou observacoes na listagem do programa.

FORMATO
  REM comentario

EXEMPLO
  REM programa exemplo

DESCRICAO
  A instrucao REM tem como funcao facilitar o entendimento de um programa atraves da leitura de sua listagem. A instrucao REM pode ser substituida pelo apostrofo ('), que tem a mesma funcao. A instrucao REM nao interferira na execucao do programa, pois e ignorada pelo computador.

PROGRAMA EXEMPLO
    10 REM PROGRAMA REM
    20 REM ESTE PROGRAMA NAO FAZ
    30 REM ABSOLUTAMENTE NADA!
    40 '   ESTE SIMBOLO PODE SER USADO NO
    50 '   LUGAR DO REM!

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 133.

VER TAMBEM
  basic, screens, cores)MAN");

    add("renum", "", "MSX-BASIC", "RENUM [novo numero da 1a linha] [, a partir de que linha sera renumerado]", "Renumera as linhas do programa.",
        R"MAN(TIPO: INSTRUCAO  (renumber)

  Renumera as linhas do programa.

FORMATO
  RENUM [novo numero da 1a linha] [, a partir de que linha sera renumerado]
  [, incremento]

EXEMPLO
  RENUM

DESCRICAO
  O comando RENUM e muito util para ser usado no final da confeccao de um programa para deixa-lo esteticamente melhor. O RENUM renumera as linhas de 10 em 10 a partir da primeira linha, se for digitado sozinho, ou entao renumera como voce preferir, utilizando os recursos mostrados no formato. O RENUM tambem renumera as instrucoes GOTO e GOSUB.

PROGRAMA EXEMPLO
    10 REM PROGRAMA RENUM
    20 PRINT 1
    30 PRINT 2
    40 PRINT 3
    50 PRINT 4
    60 PRINT 5
    70 RENUM 50,10,20

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 134.

VER TAMBEM
  basic, screens, cores)MAN");

    add("restore", "", "MSX-BASIC", "RESTORE [numero de linha]", "Indica a partir de qual linha a instrucao READ deve ler os dados contidos na instrucao DATA.",
        R"MAN(TIPO: INSTRUCAO  (restore)

  Indica a partir de qual linha a instrucao READ deve ler os dados contidos na instrucao DATA.

FORMATO
  RESTORE [numero de linha]

EXEMPLO
  RESTORE 100

DESCRICAO
  Normalmente os dados lidos pela instrucao READ comecam a partir da linha de numero mais baixo que contenha a instrucao DATA, porem esta ordem pode ser alterada com a instrucao RESTORE que indica a partir de que linha vao ser lidos os dados. A instrucao RESTORE tambem e usada para a releitura dos dados.

PROGRAMA EXEMPLO
    10 REM PROGRAMA RESTORE
    20 CLS
    30 FOR A=1 TO 2
    40 READ A,A$:PRINT A,A$
    50 NEXT A
    60 RESTORE 100
    70 FOR A=1 TO 2
    80 READ A,A$:PRINT A,A$
    90 NEXT A
    100 DATA 123,ABC,456,DEF

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 135.

VER TAMBEM
  basic, screens, cores)MAN");

    add("resume", "", "MSX-BASIC", "RESUME [0]", "Indica a linha de retorno da rotina principal, apos a execucao da rotina de erro.",
        R"MAN(TIPO: INSTRUCAO  (resume)

  Indica a linha de retorno da rotina principal, apos a execucao da rotina de erro.

FORMATO
  RESUME [0]
  RESUME numero de linha
  RESUME NEXT

EXEMPLO
  RESUME 89

DESCRICAO
  A instrucao RESUME indica a linha de retorno a rotina principal apos a execucao de um processo de erros comandado por ON ERROR GOTO. O numero da linha deve ser um inteiro entre 0 e 65529 e pode ser expresso por uma constante, uma variavel numerica ou uma variavel numerica indexada. Se um numero de linha for especificado, a execucao do programa retornara para a linha indicada. Se for especificado 0, o programa retornara para a linha em que ocorreu o erro e se o NEXT for especificado, retornara para a linha seguinte aquela em que ocorreu o erro.

PROGRAMA EXEMPLO
    10 REM PROGRAMA RESUME
    20 ON ERROR GOTO 70
    30 CLS
    40 INPUT"Digite um numero negativo.";A
    50 PRINT SQR(A):FOR B=1 TO 500:NEXT B
    60 GOTO 30
    70 PRINT"Nao existe raiz real de numero negativo"
    80 FOR B=1 TO 1000:NEXT B
    90 RESUME 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 136.

VER TAMBEM
  basic, screens, cores)MAN");

    add("right$", "", "MSX-BASIC", "RIGHT$ (string, numero de caracteres que serao destacados)", "Separa um pedaco de uma string, comecando pela direita.",
        R"MAN(TIPO: FUNCAO  (right dollar)

  Separa um pedaco de uma string, comecando pela direita.

FORMATO
  RIGHT$ (string, numero de caracteres que serao destacados)

EXEMPLO
  PRINT RIGHT$("AGUARDENTE",7)

DESCRICAO
  A funcao RIGHT$ destaca um certo numero de caracteres de uma string. No exemplo acima, o resultado sera a palavra ARDENTE, pois ela e formada pelos sete ultimos caracteres de AGUARDENTE.

PROGRAMA EXEMPLO
    10 REM PROGRAMA RIGHT$
    20 CLS
    30 INPUT " ESCREVA UMA PALAVRA";W$: PRINT
    40 PRINT "A ULTIMA LETRA E'...";RIGHT$(W$,1): PRINT

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 137.

VER TAMBEM
  basic, screens, cores)MAN");

    add("rnd", "", "MSX-BASIC", "RND (X)", "Escolhe um valor aleatorio entre 0 e 1.",
        R"MAN(TIPO: FUNCAO  (random)

  Escolhe um valor aleatorio entre 0 e 1.

FORMATO
  RND (X)

EXEMPLO
  PRINT RND(1)

DESCRICAO
  A funcao RND tem como finalidade gerar um numero pseudo-aleatorio entre 0 e 1. O valor de X varia da seguinte maneira: se X e positivo retorna um valor diferente cada vez que e usado; se X e negativo o valor retornado e sempre o mesmo para cada X negativo; se X e igual a 0 retornara o ultimo numero aleatorio gerado. Para que esta funcao gere um numero realmente aleatorio, deve-se usar (-TIME) como argumento.

PROGRAMA EXEMPLO
    10 REM PROGRAMA RND
    20 SCREEN 2
    30 FOR F=0 TO 100
    40 X=256*RND(1):Y=192*RND(2)
    50 CIRCLE(X,Y),1
    60 NEXT F
    70 GOTO 70

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 138.

VER TAMBEM
  basic, screens, cores)MAN");

    add("run", "", "MSX-BASIC", "RUN [numero de linha]", "Inicia a execucao de um programa a partir de uma linha especificada.",
        R"MAN(TIPO: INSTRUCAO  (run)

  Inicia a execucao de um programa a partir de uma linha especificada.

FORMATO
  RUN [numero de linha]

EXEMPLO
  RUN 500

DESCRICAO
  O comando RUN zera todas as variaveis e da inicio a execucao do programa que esta na memoria. O numero de linha pode ser qualquer inteiro entre 0 e 65529 e, se for omitido, o programa comecara a ser executado a partir da primeira linha existente. Para parar a execucao tecle STOP. Para continuar tecle STOP novamente. Para interromper definitivamente a execucao tecle CTRL + STOP. Por exemplo:

  RUN

  inicia a execucao a partir da primeira linha do programa e

  RUN 30

  inicia a execucao a partir da linha 30 do programa.

PROGRAMA EXEMPLO
    10 ' RUN
    20 A%=7*RND(-TIME):B%=7*RND(-TIME)
    30 CLS:LOCATE6,10
    40 PRINT"Digite a barra de espacos."
    50 IFSTRIG(0)=0THENLOCATE5,5:GOTO20
    60 PRINT:PRINT:PRINT"Dado 1:";A%
    70 PRINT"Dado 2:";B%:PRINT:PRINT
    80 PRINT"Digite RETURN para novo lancamento."
    90 LINE INPUT O$
    100 RUN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 139.

VER TAMBEM
  basic, screens, cores)MAN");

    add("save", "", "MSX-BASIC", "SAVE\"[nome do dispositivo] [nome do arquivo]\" [,A]", "Armazena, em um dispositivo especificado, um programa em BASIC.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (save)

  Armazena, em um dispositivo especificado, um programa em BASIC.

FORMATO
  SAVE"[nome do dispositivo] [nome do arquivo]" [,A]

EXEMPLO
  SAVE"CAS:BROCHE"

DESCRICAO
  Armazena o programa em BASIC que esta na memoria do computador no dispositivo especificado.

  Os dispositivos sao:

  Impressora .....................LPT:
  Cassete ........................CAS:
  Tela modo grafico...............GRP:
  Tela modo texto.................CRT:

  Nota: o nome de arquivo nao pode exceder seis caracteres (os excedentes nao serao considerados). Por exemplo:

  SAVE"CAS:PROGRA"

  armazena no gravador o programa em BASIC e

  SAVE"CRT:"

  coloca o programa em BASIC na tela de texto.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 140.

VER TAMBEM
  basic, screens, cores)MAN");

    add("screen", "", "MSX-BASIC", "SCREEN [modo], [tamanho do sprite],", "Determina: tipo de tela, tamanho do sprite, o \"liga/desliga\" do clic das teclas, velocidade de...",
        R"MAN(TIPO: INSTRUCAO  (screen)

  Determina: tipo de tela, tamanho do sprite, o "liga/desliga" do clic das teclas, velocidade de transferencia de dados para o cassete e seleciona o tipo de impressora.

FORMATO
  SCREEN [modo], [tamanho do sprite],
  [interruptor de clic das teclas],
  [velocidade em bauds], [tipo de impressora]

EXEMPLO
  SCREEN 1,,0,2

DESCRICAO
  Formata a tela, o sprite, o clic das teclas, determina a velocidade de transferencia de dados para o cassete e especifica o tipo de impressora.

  MODO:
  - *0 - texto, 40 caracteres x 24 linhas
  - 1 - texto, 32 caracteres x 24 linhas
  - 2 - grafico alta resolucao
  - 3 - grafico colorido

  TAMANHO DO SPRITE:
  - *0 - 8 x 8 sem ampliar
  - 1 - 8 x 8 ampliado
  - 2 - 16 x 16 sem ampliar
  - 3 - 16 x 16 ampliado

  INTERRUPTOR DO CLIC DAS TECLAS:
  - 0 - desligado
  - *1 ou qualquer outro valor - ligado

  VELOCIDADE DE TRANSFERENCIA:
  - *1 - 1200 Bauds
  - 2 - 2400 Bauds

  IMPRESSORA:
  - *0 - Padrao ABNT
  - 1 ou qualquer outro valor - outra impressora

  Nota: Os asteriscos indicam os valores iniciais. Se qualquer uma das opcoes for omitida, permanecem os valores atuais.

  No exemplo anterior, a instrucao SCREEN determina modo de 32 caracteres por 24 linhas, clic das teclas desligado e velocidade de transferencia de dados para o cassete de 2400 bauds.

PROGRAMA EXEMPLO
    20 A$="01234567890123456789012345678901234567890123456789"
    30 SCREEN 0:PRINT A$
    40 FOR F=1 TO 1000 : NEXT F
    50 SCREEN 1:PRINT A$
    60 FOR F=1 TO 1000 : NEXT F
    70 SCREEN 2:LINE(9,9)-(99,99),3,BF
    80 FOR F=1 TO 1000 : NEXT F
    90 SCREEN 3:LINE(9,9)-(99,99),3,BF
    100 FOR F=1 TO 1000 : NEXT F

NO MSX2+ (Manual MSX 2+ FM)
TIPO: INSTRUCAO  (screen)

  Define o modo de tela que sera usado, o tamanho dos SPRITES, o chaveamento do CLICK do teclado, a velocidade (baud rate) do gravador cassete e o tipo de impressora utilizada - versao MSX2+, com os modos de tela 4 a 12 adicionais e sprites multicoloridos.

FORMATO
  SCREEN [X [,Y [,Z [,XX [,YY [,ZZ]]]]]]

EXEMPLO
  SCREEN 1,,0,2

DESCRICAO
  X e o modo de tela que sera usado. Podendo ser:
  - SCREEN 0 - modo texto 1. Com ate 80 caracteres por linha num maximo de 26.5 linhas. Pode assumir ate duas cores, uma para o caractere e outra para a tela de fundo. O numero de caracteres e definido pela instrucao WIDTH. As cores sao definidas pelo comando COLOR. Cada caractere e montado numa matriz de 6 pontos na horizontal e 8 na vertical.

  - SCREEN 1 - modo texto 2. Com ate 32 caracteres por linha num maximo de 26.5 linhas. Trabalha com ate 16 cores de 512 combinacoes e pode ter uma cor para os caracteres, uma para o fundo e outra para a borda da tela. A matriz de caracteres e formada por um quadrado de 8 por 8 pontos. Usa os SPRITEs no modo 1.

  - SCREEN 2 - modo grafico 1. Com 256 pontos na horizontal por 192 pontos na vertical e 16 cores de 512 combinacoes. Podem ser usadas somente duas cores a cada 8 pontos na horizontal, na vertical nao ha esta limitacao. Usa os SPRITEs no modo 1.

  - SCREEN 3 - modo grafico 2 - 64 pontos na horizontal por 48 na vertical. Cada ponto e formado por um quarto do tamanho normal de um caractere na SCREEN 1. Usa 16 cores de 512 combinacoes. Usa os SPRITEs no modo 1.

  - SCREEN 4 - modo grafico 1. Identica a SCREEN 2 so que usa os SPRITEs no modo 2.

  - SCREEN 5 - modo grafico com bit mapeado - resolucao de 256 pontos na horizontal e 212 na vertical. Trabalha com 16 cores ponto a ponto de uma paleta de 512 combinacoes. Possui 4 paginas graficas e usa os sprites no modo 2.

  - SCREEN 6 - modo grafico com bit mapeado de alta resolucao - 512 pontos na horizontal por 212 na vertical. Trabalha com 4 cores ponto a ponto de uma paleta de 512 combinacoes. Quatro paginas graficas e sprites no modo 2.

  - SCREEN 7 - modo grafico com bit mapeado de alta resolucao - 512 pontos na horizontal por 212 na vertical. Trabalha com 16 cores ponto a ponto de uma paleta de 512 combinacoes. Duas paginas graficas e sprites no modo 2.

  - SCREEN 8 - modo grafico com bit mapeado - resolucao de 256 pontos na horizontal por 212 na vertical e 256 cores simultaneas ponto a ponto. Duas paginas graficas e sprites no modo 2.

  - SCREEN 9 - nao existe nos MSX 2+, mesmo nos importados.

  - SCREEN 10 - modo grafico 1 com cores no sistema YJK - 256 pontos na horizontal e 212 na vertical. Trabalha com ate 12.499 cores do sistema YJK mais as 16 cores basicas (0 a 15) da paleta de 512 combinacoes. Neste tipo de tela os comandos graficos trabalham somente com cores usando valores entre 0 e 15. Portanto, pode-se tracar linhas sobre uma tela feita no sistema YJK sem ocorrer borroes na tela de fundo. Tem duas paginas graficas e usa os sprites no modo 2.

  - SCREEN 11 - modo grafico 1 com cores no sistema YJK - 256 pontos na horizontal por 212 na vertical. Trabalha com ate 12.499 cores. O valor das cores nos comandos graficos pode variar entre 0 e 255. Dependendo do valor da cor e do tipo de comando grafico podem ocorrer borroes entre as cores no limite de quatro em quatro pontos. Tem duas paginas graficas e usa sprites no modo 2.

  - SCREEN 12 - modo grafico 2 com cores no sistema YJK - 256 pontos na horizontal por 212 na vertical. Trabalha com ate 19.268 cores de 131072 combinacoes. O limite para troca abrupta de cores e de quatro em quatro pontos na horizontal. Tem duas paginas graficas e usa os sprites no modo 2.

  Tipos de SPRITEs: nos MSX 1 os SPRITES podem ter somente uma cor e no maximo 4 SPRITES podem estar na mesma linha horizontal. Nos MSX 2+ os SPRITES podem ser multicoloridos (cada linha da figura pode ter uma cor) e ate 8 (oito) SPRITES podem ser colocados na mesma linha horizontal. Tambem podem ser formadas figuras mais complexas sobrepondo-se varios SPRITES, pois existe a possibilidade de se desativar o sensor de coincidencia de SPRITES (bit que e ativado sempre que dois ou mais SPRITES se tocam).

  A resolucao vertical pode ser dobrada para 424 usando-se o modo entrelacado de exibicao (ver item ZZ).

  Y determina o tamanho dos SPRITES:
  - 0 - 8x8 mag. 1 (8x8)
  - 1 - 8x8 mag. 2 (16x16)
  - 2 - 16x16 mag. 1 (16x16)
  - 3 - 16x16 mag. 2 (32x32)

  Z ativa ou desativa o CLICK do teclado:
  - 0 - desativa o CLICK
  - 1 - ativa CLICK

  XX define a velocidade de transmissao dos dados para o gravador cassete (baud rate):
  - 1 - 1200 baud
  - 2 - 2400 baud

  YY define o tipo de impressora usada:
  - 0 - MSX
  - 1 - nao MSX

  ZZ define o modo de exibicao da tela. Podendo ser:
  - 0 - Normal
  - 1 - Entrelacado
  - 2 - Nao entrelacado, alternando paginas impares e pares
  - 3 - Entrelacado, alternando paginas impares e pares

  Nos modos de exibicao 2 e 3, o numero da pagina de exibicao deve ser impar (ver instrucao SETPAGE).

PROGRAMA EXEMPLO
    10 COLOR 15,0,0
    20 FOR I= 2 TO 8 : SCREEN I
    30 FOR N=0 TO 255:LINE (0,0)-(N,211),C
    40 IF I=8 THEN C=N ELSE C=C+1:IFC=16THENC=1
    50 NEXT
    60 FOR T=0 TO 1000:NEXT
    70 NEXT

    10 SCREEN 7,,,,,3
    20 LINE(80,80)-(400,200),4,BF
    30 SETPAGE1,1:CLS
    40 LINE(100,100)-(380,180),4,BF
    50 GOTO50

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 141.

VER TAMBEM
  basic, screens, cores)MAN");

    add("setadjust", "", "MSX-BASIC", "SETADJUST (X,Y)", "Ajusta a posicao da tela para se adequar ao televisor ou monitor.",
        R"MAN(TIPO: INSTRUCAO  (set adjust)

  Ajusta a posicao da tela para se adequar ao televisor ou monitor.

FORMATO
  SETADJUST (X,Y)

EXEMPLO
  SETADJUST (X,Y)

DESCRICAO
  X deve ser um numero inteiro entre -7 e 8. Quando negativo, a tela se deslocara para a esquerda. Quando for positivo, a tela se deslocara para a direita. Y deve ser um numero inteiro entre -7 e 8. Quando negativo, a tela se deslocara para cima e quando positivo, a tela se deslocara para baixo. Os valores de X e Y sao armazenados na memoria do relogio. Assim, quando o computador for ligado, os parametros armazenados serao usados automaticamente.

PROGRAMA EXEMPLO
    10 SCREEN1:COLOR 15,4,7
    20 PRINT "USE AS TECLAS DO CURSOR"
    30 C=STICK(0)
    40 X=X+(C=6 OR C=7 OR C=8)-(C=2 OR C=3 OR C=4)
    50 Y=Y+(C=8 OR C=1 OR C=2)-(C=4 OR C=5 OR C=6)
    60 IF X=-8 THEN X=-7
    70 IF X=9 THEN X=8
    80 IF Y=-8 THEN Y=-7
    90 IF Y=9 THEN Y=8
    100 LOCATE 4,4
    110 PRINT "X = ";X ,"Y = ";Y
    120 SETADJUST (X,Y)
    130 GOTO 30

FONTE
  Manual MSX 2+ FM (ACVS), pagina 25.

VER TAMBEM
  basic, screens, cores)MAN");

    add("setbeep", "", "MSX-BASIC", "SETBEEP X,Y", "Seleciona o tipo e o volume do BEEP.",
        R"MAN(TIPO: INSTRUCAO  (set beep)

  Seleciona o tipo e o volume do BEEP.

FORMATO
  SETBEEP X,Y

EXEMPLO
  SETBEEP A,B

DESCRICAO
  X e o tipo de BEEP e deve ser um numero inteiro entre 1 e 4. Y e o volume do BEEP e deve estar entre 1 e 4. O numero 1 e o volume minimo e o 4 o volume maximo. Os valores de X e Y sao armazenados na memoria do relogio interno. Assim, quando o computador for ligado, os parametros armazenados serao usados automaticamente. O BEEP tambem pode ser gerado pressionando as teclas CONTROL+G.

PROGRAMA EXEMPLO
    10 FOR A=1 TO 4
    20 FOR B=1 TO 4
    30 SETBEEP A,B
    40 FOR N=1 TO 10 : BEEP : NEXT
    50 FOR T=0 TO 500:NEXT
    60 NEXT:NEXT

FONTE
  Manual MSX 2+ FM (ACVS), pagina 26.

VER TAMBEM
  basic, screens, cores)MAN");

    add("setdate", "", "MSX-BASIC", "SETDATE X$", "Acerta a data no relogio interno.",
        R"MAN(TIPO: INSTRUCAO  (set date)

  Acerta a data no relogio interno.

FORMATO
  SETDATE X$

EXEMPLO
  SETDATE "14/04/61"

DESCRICAO
  X$ e uma expressao alfanumerica com o seguinte formato: DD/MM/AA - onde: DD e o dia (1 a 31), MM e o mes (1 a 12) e AA e o ano. A data e armazenada no relogio interno e sera incrementada mesmo com o computador desligado.

PROGRAMA EXEMPLO
    SETDATE "14/04/61"
    GETDATE A$
    PRINT A$

FONTE
  Manual MSX 2+ FM (ACVS), pagina 24.

VER TAMBEM
  basic, screens, cores)MAN");

    add("setpage", "", "MSX-BASIC", "SETPAGE X,Y", "Seleciona a pagina da tela que sera exibida e a pagina que sera ativa (na qual os desenhos...",
        R"MAN(TIPO: INSTRUCAO  (set page)

  Seleciona a pagina da tela que sera exibida e a pagina que sera ativa (na qual os desenhos serao feitos).

FORMATO
  SETPAGE X,Y

EXEMPLO
  SETPAGE 0,1

DESCRICAO
  Esta instrucao so e valida nas SCREENs 5 a 12. X determina a pagina que sera exibida e deve ser um numero inteiro entre 0 e 3, dependendo da SCREEN que esta sendo utilizada. Y determina a pagina que sera ativa e deve ser um numero inteiro entre 0 e 3, dependendo da SCREEN que esta sendo utilizada.

  As paginas de cada SCREEN sao:
  - SCREEN 5 - 4 paginas (0 a 3)
  - SCREEN 6 - 4 paginas (0 a 3)
  - SCREEN 7 - 2 paginas (0 e 1)
  - SCREEN 8 - 2 paginas (0 e 1)
  - SCREEN 10 - 2 paginas (0 e 1)
  - SCREEN 11 - 2 paginas (0 e 1)
  - SCREEN 12 - 2 paginas (0 e 1)

PROGRAMA EXEMPLO
    10 SCREEN 5: COLOR 15,0,0
    20 LINE (20,20)-(220,200),4,BF
    30 SETPAGE 0,1:CLS
    40 LINE (40,40)-(200,180),8,BF
    50 SETPAGE 0,2:CLS
    60 LINE (60,60)-(180,160),12,BF
    70 SETPAGE 0,3:CLS
    80 CIRCLE (127,105),80,4
    90 PAINT (127,105),10,4
    100 FOR P=0 TO 3
    110 SETPAGE P,P
    120 FOR T=0 TO 300 :NEXT T:BEEP
    130 NEXT P
    140 GOTO100

FONTE
  Manual MSX 2+ FM (ACVS), pagina 21.

VER TAMBEM
  basic, screens, cores)MAN");

    add("setpassword", "", "MSX-BASIC", "SETPASSWORD X$", "Estabelece uma senha ou palavra de acesso ao computador.",
        R"MAN(TIPO: INSTRUCAO  (set password)

  Estabelece uma senha ou palavra de acesso ao computador.

FORMATO
  SETPASSWORD X$

EXEMPLO
  SETPASSWORD"TESTE"

DESCRICAO
  X$ e uma sequencia alfanumerica de ate 255 caracteres. A senha e armazenada na memoria do relogio interno que e mantido por baterias. Assim, mesmo com o computador desligado, a senha continua armazenada. Ao se ligar o computador, sera apresentada a palavra "PASSWORD:" esperando que seja digitada a senha. Caso a senha digitada nao coincida com aquela armazenada, o computador continuara esperando a senha correta.

  Para eliminar a senha, use a instrucao SETTITLE "".

  So se pode utilizar uma das instrucoes "SETPASSWORD", "SETPROMPT" ou "SETTITLE" de cada vez. A ultima instrucao executada sera a armazenada pois todas utilizam a mesma area na memoria do relogio interno.

PROGRAMA EXEMPLO
    SETPASSWORD"TESTE"
    'desligue o computador e ligue-o novamente.
    'digite TESTE seguido da tecla RETURN

FONTE
  Manual MSX 2+ FM (ACVS), pagina 27.

VER TAMBEM
  basic, screens, cores)MAN");

    add("setprompt", "", "MSX-BASIC", "SETPROMPT X$", "Estabelece um PROMPT ou palavra de aviso de espera de um comando.",
        R"MAN(TIPO: INSTRUCAO  (set prompt)

  Estabelece um PROMPT ou palavra de aviso de espera de um comando.

FORMATO
  SETPROMPT X$

EXEMPLO
  SETPROMPT "TESTE"

DESCRICAO
  X$ e uma sequencia alfanumerica de ate 6 caracteres. O PROMPT e armazenado na memoria do relogio interno que e mantido por baterias. Assim, mesmo com o computador desligado, ele sera preservado. O PROMPT normal do computador e "Ok". Quando este PROMPT e selecionado, a area na memoria de relogio fica livre para ser usada pelas instrucoes SETPASSWORD ou SETTITLE.

  So se pode utilizar uma das instrucoes "SETPASSWORD", "SETPROMPT" ou "SETTITLE" de cada vez. A ultima instrucao executada sera a armazenada pois elas utilizam a mesma area na memoria do relogio interno.

PROGRAMA EXEMPLO
    SETPROMPT "TESTE"

FONTE
  Manual MSX 2+ FM (ACVS), pagina 27.

VER TAMBEM
  basic, screens, cores)MAN");

    add("setscreen", "", "MSX-BASIC", "SETSCREEN", "Transfere os parametros da tela e do modo de trabalho do computador para a memoria do relogio.",
        R"MAN(TIPO: INSTRUCAO  (set screen)

  Transfere os parametros da tela e do modo de trabalho do computador para a memoria do relogio.

FORMATO
  SETSCREEN

EXEMPLO
  SETSCREEN

DESCRICAO
  Os parametros armazenados sao:
  - Tipo de SCREEN de texto: 0 ou 1.
  - Numero de colunas (WIDTH): 1 a 80.
  - Numero da cor do caractere: 0 a 15.
  - Numero da cor do fundo do caractere: 0 a 15.
  - Numero da cor da borda da tela: 0 a 15.
  - Exibicao das teclas de funcoes: ligada ou desligada.
  - Click do teclado: ligado ou desligado.
  - Tipo de impressora: MSX ou nao MSX.
  - Velocidade dos dados para o gravador cassete: 1200 ou 2400 bauds.
  - Modo de exibicao da tela: 1 a 4.

  Estes dados sao armazenados na memoria do relogio interno, mantido por baterias. Assim, mesmo com o computador desligado, eles serao preservados.

PROGRAMA EXEMPLO
    'Para inicializar o computador em 80 colunas automaticamente:
    SCREEN0 : WIDTH80 : SETSCREEN
    '(desligue o computador e ligue-o novamente)

FONTE
  Manual MSX 2+ FM (ACVS), pagina 28.

VER TAMBEM
  basic, screens, cores)MAN");

    add("setscroll", "", "MSX-BASIC", "SETSCROLL [X] [,Y] [,Z] [,XX]", "Determina uma coordenada que sera usada como ponto inicial, a partir do qual a tela sera...",
        R"MAN(TIPO: INSTRUCAO  (set scroll)

  Determina uma coordenada que sera usada como ponto inicial, a partir do qual a tela sera mostrada no video.

FORMATO
  SETSCROLL [X] [,Y] [,Z] [,XX]

EXEMPLO
  SETSCROLL X,0,1,1

DESCRICAO
  X e um numero entre 0 e 511 e define a coordenada horizontal. Y e um numero entre 0 e 255 e define a coordenada vertical. Z determina se sera feito SCROLL nas 8 colunas da esquerda. Podendo ser:
  - 0 - as 8 colunas da esquerda da tela sao mostradas.
  - 1 - as 8 colunas da esquerda da tela nao aparecem.

  XX determina se havera troca de paginas de video ou nao. Podendo ser:
  - 0 - somente uma pagina. Neste caso, quando a tela desaparece em uma das laterais do video, a mesma tela reaparece no lado oposto.
  - 1 - duas paginas. Neste caso, quando a primeira pagina desaparece em uma das laterais do video, a segunda pagina aparece no lado oposto.

  Para movimentar duas paginas, a pagina ativa deve ser impar. Para isso, use a instrucao SETPAGE.

PROGRAMA EXEMPLO
    10 SCREEN 8,0
    20 COPY (0,0)-(255,80) TO (0,211)
    30 FOR X=0 TO 255
    40 LINE (255,0)-(X,211),X
    50 NEXT
    60 SETPAGE 1,1:CLS
    70 COPY (0,0)-(255,80) TO (0,211)
    80 FOR X=0 TO 255
    90 LINE (0,0)-(X,211),X
    100 NEXT
    110 FOR X=0TO511
    120 SETSCROLL X,0,1,1
    130 NEXT
    140 FOR X=0 TO 511
    150 SETSCROLL X,X/2,1,1
    160 NEXT
    170 GOTO110

FONTE
  Manual MSX 2+ FM (ACVS), pagina 22.

VER TAMBEM
  basic, screens, cores)MAN");

    add("settime", "", "MSX-BASIC", "SETTIME X$", "Acerta a hora do relogio interno.",
        R"MAN(TIPO: INSTRUCAO  (set time)

  Acerta a hora do relogio interno.

FORMATO
  SETTIME X$

EXEMPLO
  SETTIME "13:30:20"

DESCRICAO
  X$ e uma sequencia de caracteres com o seguinte formato: HH:MM:SS - onde HH e a hora (entre 00 e 23), MM o minuto (entre 00 e 59) e SS o segundo (entre 00 e 59). Estes dados sao guardados na memoria do relogio interno que e mantido por baterias. Assim, mesmo com o computador desligado, eles serao preservados e atualizados.

PROGRAMA EXEMPLO
    10 SETTIME "13:30:20"
    20 GETTIME A$
    30 LOCATE 10,10
    40 PRINT A$
    50 GOTO 20

FONTE
  Manual MSX 2+ FM (ACVS), pagina 25.

VER TAMBEM
  basic, screens, cores)MAN");

    add("settitle", "", "MSX-BASIC", "SETTITLE X$ [,Y]", "Armazena um nome que sera mostrado na apresentacao quando o computador for ligado.",
        R"MAN(TIPO: INSTRUCAO  (set title)

  Armazena um nome que sera mostrado na apresentacao quando o computador for ligado.

FORMATO
  SETTITLE X$ [,Y]

EXEMPLO
  SETTITLE "TESTE ",4

DESCRICAO
  X$ e uma sequencia alfanumerica com ate 6 caracteres. Se X$ tiver exatamente 6 caracteres, o nome aparecera na apresentacao e o computador ficara esperando que qualquer tecla seja pressionada, continuando a inicializacao somente depois. Se tiver ate 5 caracteres, o nome permanecera na tela durante alguns segundos e, entao, continuara a inicializacao. Y pode variar de 1 a 4 e seleciona qual combinacao de cores sera usada na apresentacao quando o computador for ligado.

  Estes dados sao armazenados na memoria do relogio interno que e mantido por baterias. Assim, mesmo com o computador desligado, os dados serao preservados.

  So se pode utilizar uma das instrucoes "SETPASSWORD", "SETPROMPT" ou "SETTITLE" de cada vez. A ultima instrucao executada sera a armazenada pois todas utilizam a mesma area na memoria do relogio interno.

  SETTITLE "" e usado para apagar os dados do SET PASSWORD.

PROGRAMA EXEMPLO
    SETTITLE "TESTE ",4
    'desligue o computador e ligue-o novamente

FONTE
  Manual MSX 2+ FM (ACVS), pagina 28.

VER TAMBEM
  basic, screens, cores)MAN");

    add("sgn", "", "MSX-BASIC", "SGN (expressao)", "Verifica o sinal de um numero ou expressao.",
        R"MAN(TIPO: FUNCAO  (sign)

  Verifica o sinal de um numero ou expressao.

FORMATO
  SGN (expressao)

EXEMPLO
  PRINT SGN(-10)

DESCRICAO
  A funcao SGN verifica o sinal de um numero da seguinte maneira: se o numero for positivo o valor fornecido por SGN sera igual a 1; se o numero for negativo, -1 e, se for igual a zero, retornara o proprio valor zero.

PROGRAMA EXEMPLO
    10 REM PROGRAMA SGN
    20 A=34
    30 B=0
    40 C=-345
    50 PRINT A,SGN(A)
    60 PRINT B,SGN(B)
    70 PRINT C,SGN(C)

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 142.

VER TAMBEM
  basic, screens, cores)MAN");

    add("sin", "", "MSX-BASIC", "SIN (angulo)", "Calcula o valor do seno de um angulo.",
        R"MAN(TIPO: FUNCAO  (sinus)

  Calcula o valor do seno de um angulo.

FORMATO
  SIN (angulo)

EXEMPLO
  PRINT SIN(1.234)

DESCRICAO
  A funcao SIN calcula o seno de um angulo fornecido pelo usuario. E importante lembrar que o computador trabalha com o angulo em radianos. Caso voce queira calcular o seno de um angulo em graus digite da seguinte maneira:
  PRINT SIN(angulo*3.1415927/180)

PROGRAMA EXEMPLO
    10 REM PROGRAMA SIN
    20 CLS
    30 SCREEN 2
    40 FOR F=0 TO 255 STEP .5
    50 PSET(F,80-70*SIN(F*6.141592#/255))
    60 NEXT F
    70 GOTO 70

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 143.

VER TAMBEM
  basic, screens, cores)MAN");

    add("sound", "", "MSX-BASIC", "SOUND numero de registro, expressao", "Escreve dados diretamente nos registros do PSG (gerador de som programavel).",
        R"MAN(TIPO: INSTRUCAO  (sound)

  Escreve dados diretamente nos registros do PSG (gerador de som programavel).

FORMATO
  SOUND numero de registro, expressao

EXEMPLO
  SOUND 7,8

DESCRICAO
  Gerar som programando diretamente os registros do PSG. O PSG dispoe de 3 canais que podem gerar tons com a frequencia desejada pelo usuario. E tambem possivel aplicar ruido a todos esses canais, controlar os envelopes (variacao de volume durante a geracao de um tom) e a duracao de cada nota em cada um dos canais. Em outras palavras, podem ser criados acordes triplos e ruido. O PSG possui 16 registros. Veja na tabela as funcoes de cada um (os registros 14 e 15 nao sao utilizados para a composicao musical). No comando:
  SOUND numero de registro, expressao
  o numero de registro deve ser um inteiro entre 0 e 13. As expressoes tambem sao inteiras e devem obedecer aos limites estabelecidos na tabela.

  Um registro e um "lugar" no PSG onde se armazenam temporariamente dados para o processamento. Variando o dado em um registro, modifica-se o som produzido.

  TABELA DE REGISTROS:
  - 0/1 (0 a 255 / 0 a 15): Determinam a frequencia do canal A (a)
  - 2/3 (0 a 255 / 0 a 15): Determinam a frequencia do canal B (b)
  - 4/5 (0 a 255 / 0 a 15): Determinam a frequencia do canal C (c)
  - 6 (0 a 31): Determina a frequencia do ruido (d)
  - 7 (0 a 63): Seleciona um canal para geracao de tons e ruidos (e)
  - 8 (0 a 15): Volume do canal A * (f)
  - 9 (0 a 15): Volume do canal B * (g)
  - 10 (0 a 15): Volume do canal C * (h)
  - 11/12 (0 a 255 / 0 a 255): Frequencia do gerador de envelope (i)
  - 13 (0 a 14): Selecao da forma do envelope (j)

  * Nota: So pode haver geracao de envelope se o registro de volume do canal for programado com o valor 16.

  NOTAS:

  (a),(b),(c) - Para determinar os valores dos registros para uma determinada frequencia, use este programa:
  10 ' Para R0 ate R5
  20 INPUT"Qual a frequencia";F
  30 A=3575611#/8192/F
  40 H=INT(A)
  50 L=INT(.5+256*(A-H))
  60 PRINT"Alto (impar)=";H,"Baixo (par)=";L

  O valor de H deve ser colocado nos registros impares (1 para o canal A, 3 para o canal B e 5 para o canal C) e o valor L deve ser colocado nos registros pares (0 para o canal A, 2 para o canal B e 4 para o canal C). Por exemplo, imagine que voce quer programar o canal A com uma frequencia de 1000 Hz. Rode o programa e introduza o numero 1000.

  RUN
  Qual a frequencia? 1000
  Alto (impar)= 0
  Baixo (par)= 112
  Ok

  Portanto, devemos fazer:
  SOUND 0,112
  SOUND 1,0

  (d) - Para programar a frequencia do ruido use o programa:
  10 INPUT"Qual a frequencia do ruido";F
  20 R6=3575611#/32/F
  30 PRINT"O valor a ser atribuido ao registro","6 e':";INT(R6+.5)

  RUN
  Qual a frequencia do ruido? 9600
  O valor a ser atribuido ao registro 6 e': 12

  Portanto fazendo:
  SOUND 6,12

  Teremos um ruido de aproximadamente 9600 Hz.

  (e) - Apenas com as instrucoes anteriores o PSG ainda nao pode produzir som. E preciso determinar o volume (veremos a seguir) e habilitar a saida do canal que se esta programando. Para especificar o canal utilize a tabela:

  SOM: canal A=1, canal B=2, canal C=4
  RUIDO: canal A=8, canal B=16, canal C=32

  Para programar o registro 7 deve-se somar os valores correspondentes as saidas desejadas e subtrair esse total de 255. Por exemplo: para selecionar so o canal A, o valor e 1. Subtraindo temos:
  255 - 1 = 254

  Portanto, para selecionar o canal A fazemos:
  SOUND 7,254

  Se quisessemos selecionar os canais A e B para produzir som e A, B e C para produzir ruido, teriamos:
  255 - (1+2+8+16+32) = 196

  Portanto:
  SOUND 7,196

  (f),(g),(h) - Ha 16 niveis de volume (0 a 15). Para programar o volume do canal A, fazemos:
  SOUND 8,15    Volume maximo no canal A

  Para os outros canais o procedimento e semelhante, bastando mudar o numero do registro (9 para o canal B, 10 para o canal C). Para utilizar o gerador de envelope (veremos a seguir) os registros escolhidos devem ser carregados com o valor 16.

  (i) - Os registros 11 e 12 determinam a frequencia do envelope (variacao de volume durante a producao de um tom). Para obter o valor destes registros utilize este programa:
  10 ' Para R11 e R12
  20 INPUT"Qual a frequencia";F
  30 A=3575611#/131072!/F
  40 H=INT(A)
  50 L=INT(.5+256*(A-H))
  60 PRINT"Alto (12)=";H
  70 PRINT"Baixo (11)=";L

  Execute-o, digitando:
  RUN

  Na tela, deverao surgir as mensagens:
  RUN
  Qual a frequencia? 16
  Alto (12)= 1
  Baixo (11)= 180
  Ok

  Portanto, para obter uma frequencia de envelope de 16 Hz no canal A devemos fazer:
  SOUND 8,16     deixa o canal A sob o controle do gerador de envelope
  SOUND 11,180
  SOUND 12,1     frequencia do envelope

  (j) - Varios envelopes podem ser selecionados para modular a(s) nota(s) emitida(s). O valor atribuido ao registro 13 seleciona a forma do envelope, entre 8 formatos possiveis representados graficamente no livro (mesma tabela e mesmos valores do subcomando Sn da funcao PLAY):
  - 0, 1, 2, 3 ou 9
  - 4, 5, 6, 7 ou 15
  - 8
  - 10
  - 11
  - 12
  - 13
  - 14

  (As formas exatas de cada envelope sao graficos de linha na pagina 148 do livro, nao reproduzidos aqui.)

PROGRAMA EXEMPLO
    100 CLS
    110 PRINT"HISTORIA (INACABADA) DA AVIACAO"
    120 SOUND 6,12
    130 SOUND 7,246
    140 SOUND 0,93
    150 SOUND 1,4
    160 FOR V = 0 TO 16
    170 FOR T = 0 TO 250:NEXT T
    180 SOUND 8,V
    190 NEXT V
    200 SOUND 11,180
    210 SOUND 12,1
    220 SOUND 13,14
    230 FOR T = 0 TO 1000:NEXT T
    240 SOUND 1,0
    250 SOUND 7,254
    260 SOUND 8,15
    270 FOR X = 255 TO 0 STEP -1
    280 SOUND 0,X
    290 NEXT X
    300 SOUND 0,232
    310 SOUND 1,8
    320 SOUND 7,246
    330 FOR T = 0 TO 50:NEXT T
    340 SOUND 8,0

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 144.

VER TAMBEM
  basic, screens, cores)MAN");

    add("space$", "", "MSX-BASIC", "SPACE$(argumento)", "Obtem uma string com o numero especificado de espacos.",
        R"MAN(TIPO: FUNCAO  (space dollar)

  Obtem uma string com o numero especificado de espacos.

FORMATO
  SPACE$(argumento)

EXEMPLO
  A$=SPACE$(25)

DESCRICAO
  Essa funcao cria uma string com o numero de espacos definidos pelo argumento colocado entre parenteses a sua frente. O argumento pode ser uma constante, uma variavel simples ou indexada, ou uma expressao matematica, desde que o resultado seja um numero entre 0 e 255. Se o resultado for fracionario os digitos a direita da virgula (ponto decimal) sao ignorados.

PROGRAMA EXEMPLO
    10 REM PROG. SPACE$
    20 FOR F=1 TO 22
    30 A$=SPACE$(F)
    40 PRINTA$;"ALEPH"
    50 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 151.

VER TAMBEM
  basic, screens, cores)MAN");

    add("spc", "", "MSX-BASIC", "SPC(numero)", "Obtem um certo numero de espacos.",
        R"MAN(TIPO: FUNCAO  (space)

  Obtem um certo numero de espacos.

FORMATO
  SPC(numero)

EXEMPLO
  SPC(21)

DESCRICAO
  Essa funcao deve ser usada com as instrucoes PRINT ou LPRINT e serve para inserir espacos entre expressoes a serem apresentadas.

PROGRAMA EXEMPLO
    10 REM PROGRAMA SPC
    20 PRINT"GRADIENTE";SPC(25);"ALEPH"

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 152.

VER TAMBEM
  basic, screens, cores)MAN");

    add("sprite on/off/stop", "sprite", "MSX-BASIC", "SPRITE ON", "Estas tres funcoes respectivamente, habilitam, desabilitam e adiam um desvio para uma...",
        R"MAN(TIPO: INSTRUCAO  (sprite on/off/stop)

  Estas tres funcoes respectivamente, habilitam, desabilitam e adiam um desvio para uma interrupcao por sobreposicao.

FORMATO
  SPRITE ON
  SPRITE OFF
  SPRITE STOP

EXEMPLO
  SPRITE ON
  SPRITE OFF
  SPRITE STOP

DESCRICAO
  A instrucao SPRITE ON habilita uma interrupcao por sobreposicao de figuras na tela caso exista uma instrucao ON SPRITE GOSUB. A instrucao SPRITE OFF desabilita (ou desliga) a funcao do SPRITE ON. Finalmente a instrucao SPRITE STOP adia a interrupcao ate que seja encontrada uma instrucao SPRITE ON.

PROGRAMA EXEMPLO
    10 REM PROGRAMA SPRITE ON/OFF/STOP
    20 SCREEN 2,1
    30 SPRITE$(0)=CHR$(&H3C)+CHR$(&H7E)+CHR$(&H81)+CHR$(&H81)+CHR$(&HFF)+CHR$(&H7E)+CHR$(&H24)+CHR$(&H42)
    40 ON SPRITE GOSUB 110
    50 SPRITE ON
    60 FOR X=0 TO 255
    70 PUT SPRITE 0,(X,100),8,0
    80 PUT SPRITE 1,(255-X,100),10,0
    90 NEXT X
    100 GOTO 40
    110 SPRITE OFF
    120 BEEP
    130 SPRITE ON
    140 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 153.

VER TAMBEM
  basic, screens, cores)MAN");

    add("sprite$", "", "MSX-BASIC", "SPRITE$ (numero de sprite) = string", "Define os dados de um \"sprite\".",
        R"MAN(TIPO: INSTRUCAO  (sprite dollar)

  Define os dados de um "sprite".

FORMATO
  SPRITE$ (numero de sprite) = string

EXEMPLO
  SPRITE$(31)=A$

DESCRICAO
  Definir sprites. O numero do sprite pode estar entre 0 e 255 se ele for de 8x8 pontos e de 0 a 63 se ele for de 16 por 16 pontos. O tamanho do sprite e o tamanho de cada pixel devem ser definidos por uma instrucao SCREEN.

PROGRAMA EXEMPLO
    10 ' SPRITE's
    20 FORE=0TO3:D=E+1:KEYOFF
    30 SCREEN 1,E:DIM S$(4)
    40 LOCATE 4,2:PRINT"SCREEN 1 ,";E:LOCATE8,6:PRINT"<-SPRITE 5"
    50 FOR G= 1 TO 4
    60 FOR F=1 TO 8
    70 READ K$
    80 S$(G)=S$(G)+CHR$(VAL("&H"+K$))
    90 NEXT F
    100 NEXT G
    110 SPRITE$(1)=S$(1)
    120 SPRITE$(2)=S$(2)
    130 SPRITE$(3)=S$(3)
    140 SPRITE$(4)=S$(4)
    150 SPRITE$(5)=S$(1)+S$(2)+S$(3)+S$(4)
    160 PUT SPRITE 1,(90,90),3,1
    170 PUT SPRITE 2,(90,90+8*D),4,2
    180 PUT SPRITE 3,(90+8*D,90),7,3
    190 PUT SPRITE 4,(90+8*D,90+8*D),8,4
    200 PUT SPRITE 5,(44,44),15,5
    205 LOCATE 2,20:PRINT"Digite a barra de espacos!"
    210 IFSTRIG(0)=0THEN210ELSERESTORE:ERASES$
    220 NEXTE:RUN
    230 DATA FF,89,99,89,89,89,9D,FF
    240 DATA FF,99,A5,85,99,A1,BD,FF
    250 DATA FF,BD,85,89,85,A5,99,FF
    260 DATA FF,A5,A5,A5,BD,85,85,FF

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 154.

VER TAMBEM
  basic, screens, cores)MAN");

    add("sqr", "", "MSX-BASIC", "SQR (expressao)", "Fornece o valor da raiz quadrada de um numero dado pelo usuario.",
        R"MAN(TIPO: FUNCAO  (square root)

  Fornece o valor da raiz quadrada de um numero dado pelo usuario.

FORMATO
  SQR (expressao)

EXEMPLO
  PRINT SQR(16)

DESCRICAO
  A funcao SQR tem por finalidade dar o valor da raiz quadrada do numero entre parenteses. E importante salientar que o numero deve ser maior ou igual a zero, caso contrario o computador acusara erro.

PROGRAMA EXEMPLO
    10 REM PROGRAMA SQR
    20 FOR A=1 TO 100
    30 PRINT A,SQR(A)
    40 NEXT A

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 155.

VER TAMBEM
  basic, screens, cores)MAN");

    add("stick", "", "MSX-BASIC", "STICK (N)", "Mostra o estado dos joysticks e das teclas que movem o cursor.",
        R"MAN(TIPO: FUNCAO  (stick)

  Mostra o estado dos joysticks e das teclas que movem o cursor.

FORMATO
  STICK (N)
  se N=0 - Teclado
  se N=1 - Joystick A
  se N=2 - Joystick B

EXEMPLO
  A=STICK(0)

DESCRICAO
  A funcao STICK tem por finalidade mostrar a direcao em que estao se movendo os joysticks e as teclas que movem o cursor. A direcao e dada por um esquema numerado de 1 a 8 em torno de um centro (0=parado), no sentido horario a partir de cima: 1=cima, 2=cima-direita, 3=direita, 4=baixo-direita, 5=baixo, 6=baixo-esquerda, 7=esquerda, 8=cima-esquerda.

PROGRAMA EXEMPLO
    10 REM PROGRAMA STICK
    20 ON ERROR GOTO 30
    30 SCREEN 3
    40 X=0:Y=0
    50 PSET(X,Y),6
    60 C=STICK(0)
    70 IF C=1THENY=Y-1
    80 IF C=2THENY=Y-1:X=X+1
    90 IF C=3THENX=X+1
    100 IF C=4THENY=Y+1:X=X+1
    110 IF C=5THENY=Y+1
    120 IF C=6THENY=Y+1:X=X-1
    130 IF C=7THENX=X-1
    140 IF C=8THENY=Y-1:X=X-1
    150 GOTO 50

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 156.

VER TAMBEM
  basic, screens, cores)MAN");

    add("stop", "", "MSX-BASIC", "STOP", "Para a execucao do programa.",
        R"MAN(TIPO: INSTRUCAO  (stop)

  Para a execucao do programa.

FORMATO
  STOP

EXEMPLO
  STOP

DESCRICAO
  Interrompe a execucao do programa na linha onde se encontra. A execucao do programa pode ser continuada digitando-se o comando CONT.

PROGRAMA EXEMPLO
    10 REM PROGRAMA STOP
    20 WIDTH 40
    30 CLS
    40 FOR G=1 TO 200
    50 CO=RND(1)*40
    60 LOCATE CO,23
    70 PRINT "*"
    80 FOR F=1 TO 20:NEXT F
    90 NEXT G
    100 LOCATE 10,23
    110 PRINT"STOP sera executado"
    120 FOR F=1 TO 500:NEXT F
    130 STOP
    140 RUN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 157.

VER TAMBEM
  basic, screens, cores)MAN");

    add("stop on/off/stop", "stop", "MSX-BASIC", "STOP ON", "Permite, proibe ou retem a execucao de uma sub-rotina definida por ON STOP GOSUB mediante a...",
        R"MAN(TIPO: INSTRUCAO  (stop on/off/stop)

  Permite, proibe ou retem a execucao de uma sub-rotina definida por ON STOP GOSUB mediante a digitacao das teclas CONTROL e STOP.

FORMATO
  STOP ON
  STOP OFF
  STOP STOP

EXEMPLO
  STOP ON
  STOP OFF
  STOP STOP

DESCRICAO
  Esses comandos permitem, proibem ou retem a execucao de uma sub-rotina especificada por uma instrucao ON STOP GOSUB mediante a digitacao simultanea das teclas CONTROL e STOP.

PROGRAMA EXEMPLO
    10 REM PROGRAMA STOP
    20 STOP ON
    30 ON STOP GOSUB 60
    40 CLS:PRINT"Pressione CTRL + STOP."
    50 GOTO 50
    60 PRINT"CTRL + STOP foi pressionado."
    70 FOR B=1 TO 500:NEXT B
    80 C=C+1
    90 IF C=5 THEN END
    100 RETURN 40

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 158.

VER TAMBEM
  basic, screens, cores)MAN");

    add("str$", "", "MSX-BASIC", "STR$ (expressao numerica)", "Transforma dados numericos em alfanumericos.",
        R"MAN(TIPO: FUNCAO  (convert to string)

  Transforma dados numericos em alfanumericos.

FORMATO
  STR$ (expressao numerica)

EXEMPLO
  A$=STR$(X)

DESCRICAO
  A funcao STR$ funciona ao contrario da funcao VAL, ou seja, parte de uma constante ou uma variavel numerica e a transforma em alfanumerica (string).

PROGRAMA EXEMPLO
    10 REM PROGRAMA STR$
    20 CLS
    30 A=45
    40 B=60
    50 A$=STR$(A)
    60 B$=STR$(B)
    70 PRINT A+B,A$+B$

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 161.

VER TAMBEM
  basic, screens, cores)MAN");

    add("strig", "", "MSX-BASIC", "STRIG(numero)", "Obtem o estado de toque da barra de espacos e dos disparadores dos joysticks.",
        R"MAN(TIPO: FUNCAO  (stick trigger)

  Obtem o estado de toque da barra de espacos e dos disparadores dos joysticks.

FORMATO
  STRIG(numero)

EXEMPLO
  STRIG(1)

DESCRICAO
  Essa funcao verifica se a barra de espacos do teclado ou os disparadores dos joysticks estao pressionados. O numero que serve como seu argumento pode ser qualquer inteiro entre 0 e 4 e cada um deles corresponde a um caso especifico.

  0 . . . barra de espacos
  1 . . . disparador 1 do joystick 1
  2 . . . disparador 1 do joystick 2
  3 . . . disparador 2 do joystick 1
  4 . . . disparador 2 do joystick 2

  Se nenhum deles esta pressionado, o valor obtido por STICK e 0, caso contrario o valor obtido e -1.

PROGRAMA EXEMPLO
    10 REM PROGRAMA STRIG
    20 CLS
    30 PRINT"PRESSIONE A BARRA DE ESPACOS"
    40 C=0
    50 COLOR,C,C
    60 IF STRIG(0)=0 THEN GOTO 50
    70 C=C+1:IF C=15 THEN C=0
    80 GOTO 50

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 159.

VER TAMBEM
  basic, screens, cores)MAN");

    add("strig(n) on/off/stop", "strig(n)", "MSX-BASIC", "STRIG(n) ON", "Cada uma dessas tres instrucoes respectivamente habilitam, proibem ou retem uma interrupcao...",
        R"MAN(TIPO: INSTRUCAO  (strig (n) on/off/stop)

  Cada uma dessas tres instrucoes respectivamente habilitam, proibem ou retem uma interrupcao mediante o pressionamento da barra de espacos ou de um disparador de um dos joysticks.

FORMATO
  STRIG(n) ON
  STRIG(n) OFF
  STRIG(n) STOP

EXEMPLO
  STRIG(0) ON
  STRIG(3) OFF
  STRIG(2) STOP

DESCRICAO
  A instrucao STRIG(n) ON habilita uma interrupcao quando for pressionada a barra de espacos ou um dos disparadores dos joysticks; STRIG(n) OFF desabilita a interrupcao e STRIG(n) STOP a retem na mesma situacao. A escolha da barra ou de um dos disparadores e feita atraves do numero n colocado entre parenteses logo apos STRIG. Esse numero pode estar entre 0 e 4, correspondendo em cada caso a um disparador ou a barra de espacos.

  0 . . . barra de espacos
  1 . . . disparador 1 do joystick 1
  2 . . . disparador 1 do joystick 2
  3 . . . disparador 2 do joystick 1
  4 . . . disparador 2 do joystick 2

PROGRAMA EXEMPLO
    10 REM PROGRAMA STRIG(n) ON/OFF/STOP
    20 STRIG(0) ON
    30 PRINT"DIGITE A BARRA DE ESPACOS"
    40 ON STRIG GOSUB 80
    50 IF C<>6 THEN 40
    60 SCREEN 0:PRINT SPC(17);"FIM":END
    70 GOTO 40
    80 SCREEN 2
    90 COLOR C+3,C+4,C+5
    100 CIRCLE (128,85),50
    110 COLOR C+6,C+7,C+8
    120 C=C+1:IF C=6 THEN COLOR 15,1,1
    130 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 160.

VER TAMBEM
  basic, screens, cores)MAN");

    add("string$", "", "MSX-BASIC", "STRING$ (numero de vezes que o caractere sera repetido, caractere a ser repetido)", "Imprime um caractere definido pelo usuario varias vezes.",
        R"MAN(TIPO: FUNCAO  (string dollar)

  Imprime um caractere definido pelo usuario varias vezes.

FORMATO
  STRING$ (numero de vezes que o caractere sera repetido, caractere a ser repetido)

EXEMPLO
  PRINT STRING$(30,67)

DESCRICAO
  A funcao STRING$ tem como objetivo imprimir varias vezes um certo caractere. O mesmo pode ser impresso ate 255 vezes, ou seja, o primeiro numero entre parenteses pode variar de 0 ate 255, enquanto o segundo define o codigo do caractere que sera impresso. Para saber o codigo do caractere, consulte a tabela de codigos. Pode-se usar STRING$ tambem com uma string ou uma variavel alfanumerica, como por exemplo:

  PRINT STRING$(10,"A")
  PRINT STRING$(19,A$)

PROGRAMA EXEMPLO
    10 REM PROGRAMA STRING$
    20 A$="###___###"
    30 B$="GRADIENTE"
    40 C$=STRING$(25,A$)
    50 D$="ALEPH"
    60 E$=B$+C$+D$
    70 PRINT E$

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 162.

VER TAMBEM
  basic, screens, cores)MAN");

    add("swap", "", "MSX-BASIC", "SWAP variavel, variavel", "Troca o valor de duas variaveis.",
        R"MAN(TIPO: INSTRUCAO  (swap)

  Troca o valor de duas variaveis.

FORMATO
  SWAP variavel, variavel

EXEMPLO
  SWAP A,B

DESCRICAO
  A instrucao SWAP troca os valores de duas variaveis que podem ser numericas ou alfanumericas. E importante notar que so podem ser trocados os valores de variaveis de mesmo tipo, ou seja, numerica com numerica e alfanumerica com alfanumerica.

PROGRAMA EXEMPLO
    10 REM PROGRAMA SWAP
    20 A=12
    30 B=24
    40 PRINT "A=";A,"B=";B
    50 SWAP A,B
    60 PRINT "A=";A,"B=";B

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 163.

VER TAMBEM
  basic, screens, cores)MAN");

    add("tab", "", "MSX-BASIC", "TAB (numero de espacos)", "Move o cursor para a direita um certo numero de espacos, definido pelo usuario.",
        R"MAN(TIPO: FUNCAO  (tab)

  Move o cursor para a direita um certo numero de espacos, definido pelo usuario.

FORMATO
  TAB (numero de espacos)

EXEMPLO
  PRINT TAB(10);"ALEPH"

DESCRICAO
  A finalidade da funcao TAB e tabular o texto que esta sendo impresso tanto na tela quanto na impressora. O numero de espacos, que e definido entre parenteses, nao deve exceder 255.

PROGRAMA EXEMPLO
    10 REM PROGRAMA TAB
    20 FOR A=1 TO 20
    30 PRINT TAB(A);A
    40 NEXT A

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 164.

VER TAMBEM
  basic, screens, cores)MAN");

    add("tan", "", "MSX-BASIC", "TAN (angulo)", "Calcula o valor da tangente de um angulo.",
        R"MAN(TIPO: FUNCAO  (tangent)

  Calcula o valor da tangente de um angulo.

FORMATO
  TAN (angulo)

EXEMPLO
  PRINT TAN(35)

DESCRICAO
  A funcao TAN calcula a tangente de um angulo fornecido pelo usuario. E importante lembrar que o computador trabalha com o angulo em radianos. Caso voce queira calcular a tangente de um angulo em graus digite da seguinte maneira:
  PRINT TAN(angulo*3.1415927/180)

PROGRAMA EXEMPLO
    10 REM PROGRAMA TAN
    20 CLS
    30 SCREEN 2
    40 FOR F=0 TO 255 STEP .5
    50 PSET(F,80-70*TAN((F+127)*6.1432#/255))
    60 NEXT F
    70 GOTO 70

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 165.

VER TAMBEM
  basic, screens, cores)MAN");

    add("time", "", "MSX-BASIC", "TIME", "Determina o valor do temporizador interno.",
        R"MAN(TIPO: INSTRUCAO  (time)

  Determina o valor do temporizador interno.

FORMATO
  TIME
  TIME = expressao

EXEMPLO
  TIME=0

DESCRICAO
  Enquanto o BASIC estiver acionado, o valor de TIME e incrementado em uma unidade a cada 1/60 de segundo aproximadamente. Ao atingir 65535, TIME voltara a zero e recomecara a contagem. O valor de TIME pode ser modificado a qualquer momento utilizando-se um comando LET. A expressao pode ser uma constante, variavel (indexada ou numerica) ou suas expressoes.

  NOTA: O temporizador para quando o processador realiza algumas operacoes de entrada/saida. Por exemplo, durante a gravacao ou leitura de dados no cassete.

PROGRAMA EXEMPLO
    10 ' Relogio
    20 INPUT "Horas";H
    30 INPUT "Minutos";M
    40 INPUT "Segundos";S
    50 CLS
    60 ON INTERVAL=60 GOSUB 90
    70 INTERVAL ON
    80 GOTO 80
    90 LOCATE 12,10
    100 S=S+1
    110 IF S=60 THEN M=M+1:S=0
    120 IF M=60 THEN H=H+1:M=0
    130 IF H=24 THEN H=0
    140 PRINT USING"##:##:##";H;M;S
    150 RETURN

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 166.

VER TAMBEM
  basic, screens, cores)MAN");

    add("troff", "", "MSX-BASIC", "TROFF", "Desliga o funcionamento do comando TRON.",
        R"MAN(TIPO: INSTRUCAO  (trace off)

  Desliga o funcionamento do comando TRON.

FORMATO
  TROFF

EXEMPLO
  TROFF

DESCRICAO
  Depois de dado o comando TROFF, nao mais aparecerao os numeros das linhas que estao sendo executadas, ou seja, e desativado o comando TRON.

PROGRAMA EXEMPLO
    10 REM PROGRAMA TROFF
    20 TRON
    30 FOR F=1 TO 40
    40 PRINT"............................."
    50 NEXT F
    60 TROFF
    70 FOR F=1 TO 40
    80 PRINT"............................."
    90 NEXT F
    100 GOTO020

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 167.

VER TAMBEM
  basic, screens, cores)MAN");

    add("tron", "", "MSX-BASIC", "TRON", "Permite visualizar na tela o numero da linha que esta sendo executada.",
        R"MAN(TIPO: INSTRUCAO  (trace on)

  Permite visualizar na tela o numero da linha que esta sendo executada.

FORMATO
  TRON

EXEMPLO
  TRON

DESCRICAO
  O comando TRON faz com que o numero da linha de programa que esta sendo executada apareca na tela entre colchetes. O TRON e muito util na correcao de programas. E importante lembrar que os numeros das linhas executadas so aparecem no modo de texto. Nas telas graficas eles nao aparecerao. O comando TRON e desativado pelo comando TROFF.

PROGRAMA EXEMPLO
    10 REM PROGRAMA TRON
    20 TRON
    30 PRINT"LINHA 20";
    40 PRINT"LINHA 30";
    50 FOR F=1 TO 350:NEXT F
    60 GOTO 30

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 168.

VER TAMBEM
  basic, screens, cores)MAN");

    add("usr", "", "MSX-BASIC", "USR [numero da rotina]", "Fornece o resultado obtido pela execucao de uma rotina em linguagem de maquina, que comeca no...",
        R"MAN(TIPO: FUNCAO (uso avancado)  (user)

  Fornece o resultado obtido pela execucao de uma rotina em linguagem de maquina, que comeca no endereco definido em DEFUSR.

FORMATO
  USR [numero da rotina]
  (valor a ser transferido)

EXEMPLO
  M=USR1(34)

DESCRICAO
  A funcao USR fornece o resultado obtido apos a execucao de uma rotina em linguagem de maquina. Os enderecos de inicio das rotinas sao definidos por DEFUSR. O numero da rotina deve ser um inteiro entre 0 e 9 e, quando omitido, assumira o valor 0. O valor a ser transferido para a sub-rotina, pode ser expressado por uma constante, uma variavel numerica ou entao, por uma variavel numerica indexada. Veja o item SUB-ROTINAS EM LINGUAGEM DE MAQUINA na parte 3 deste livro.

PROGRAMA EXEMPLO
    10 DATA 3E,41,21,00,00,CD,4D,00,23,E5,D5,11,BF,03,ED,52,D1,E1,38,F1,C9
    20 FOR I=0 TO 20
    30 READ A$:POKE 60000!+I,VAL("&H"+A$)
    40 NEXT I
    50 SCREEN 0:CLS:PRINT "APERTE <RETURN> PARA EXECUTAR"
    60 INPUT B$
    70 DEF USR0=60000!
    80 X=USR0 (0)
    90 FOR T=1 TO 1000:NEXT T:CLS:END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 169.

VER TAMBEM
  basic, screens, cores)MAN");

    add("val", "", "MSX-BASIC", "VAL (string ou variavel alfanumerica)", "Transforma dados alfanumericos em numericos.",
        R"MAN(TIPO: FUNCAO  (value)

  Transforma dados alfanumericos em numericos.

FORMATO
  VAL (string ou variavel alfanumerica)

EXEMPLO
  PRINT VAL("10")

DESCRICAO
  A funcao VAL tem por finalidade transformar dados alfanumericos em numericos, sendo que os dados alfanumericos podem ser uma string ou mesmo uma variavel alfanumerica. Essa funcao ignora caracteres que nao sejam de algarismos.

PROGRAMA EXEMPLO
    10 REM PROGRAMA VAL
    20 A$="10"
    30 B$="A55"
    40 C$="55A"
    50 PRINT A$,VAL(A$)
    60 PRINT B$,VAL(B$)
    70 PRINT C$,VAL(C$)

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 170.

VER TAMBEM
  basic, screens, cores)MAN");

    add("varptr", "", "MSX-BASIC", "VARPTR(variavel)", "Obtem o endereco do byte a partir do qual esta armazenado o conteudo de uma variavel.",
        R"MAN(TIPO: FUNCAO  (variable pointer)

  Obtem o endereco do byte a partir do qual esta armazenado o conteudo de uma variavel.

FORMATO
  VARPTR(variavel)

EXEMPLO
  VARPTR(A$)

DESCRICAO
  Essa funcao obtem o endereco do primeiro byte a partir do qual esta armazenado o conteudo de uma variavel, que pode ser numerica, string ou indexada. O valor obtido pode estar entre -32768 e 32767. Se o valor e negativo deve-se acrescentar 65536 a ele para obter o endereco.

PROGRAMA EXEMPLO
    10 REM PROGRAMA VARPTR
    20 CLS
    30 PRINT"ESTE PROGRAMA MOSTRA COMO A VARIAVEL YX ESTA ARMAZENADA NA MEMORIA DO EXPERT."
    40 YX=124
    50 C=VARPTR(YX)
    60 FOR F=C-3 TO C+7
    70 PRINT F;"...";PEEK(F);"...";CHR$(PEEK(F));"..."
    80 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 171.

VER TAMBEM
  basic, screens, cores)MAN");

    add("vdp", "", "MSX-BASIC", "VDP(registro)", "Le e insere dados diretamente nos registros do VDP.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (video display processor)

  Le e insere dados diretamente nos registros do VDP.

FORMATO
  VDP(registro)
  VDP(registro)=expressao

EXEMPLO
  VDP(7)=123

DESCRICAO
  Esse comando le ou escreve dados nos registros do VDP, responsavel pelo gerenciamento da tela do MSX. O registro e especificado por um numero inteiro entre 0 e 8 e a expressao pode ser uma constante, uma variavel simples ou indexada que resulte um numero inteiro entre 0 e 255.

PROGRAMA EXEMPLO
    10 SCREEN0:LOCATE7,10:PRINT"Pressione a barra de espaco"
    20 FOR I=0 TO 255
    30 IF STRIG(0)=0 THEN 30
    40 VDP(7)=I
    50 NEXT I
    60 RUN

NO MSX2+ (Manual MSX 2+ FM)
TIPO: FUNCAO  (video display processor)

  Verifica ou altera o conteudo dos registradores do processador de video (VDP) - versao MSX2+, com registros extras (status -9 a -1, e 9 a 47).

FORMATO
  VDP (X) ou VDP (X) = Y

EXEMPLO
  A=VDP(0)

DESCRICAO
  X e um numero inteiro e pode variar entre:
  - -9 e -1 - registradores de STATUS 1 a 9.
  - 0 e 8 - mesmos do MSX1.
  - 9 e 24 - registradores de 8 a 23.
  - 33 e 47 - registradores de 32 a 46.

  Esta instrucao so deve ser utilizada quando se estiver bem familiarizado com o funcionamento do processador de video (VDP).

PROGRAMA EXEMPLO
    10 FOR R=-9 TO 8
    20 A=VDP(R)
    30 PRINT "Registro:";R;" = ";RIGHT$("0000000"+BIN$(A),8)
    40 NEXT

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 172.

VER TAMBEM
  basic, screens, cores)MAN");

    add("vpeek", "", "MSX-BASIC", "VPEEK (endereco)", "Le o byte armazenado no endereco especificado na RAM de video.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (video ram peek)

  Le o byte armazenado no endereco especificado na RAM de video.

FORMATO
  VPEEK (endereco)

EXEMPLO
  PRINT VPEEK(1234)

DESCRICAO
  Obter o valor do byte armazenado em determinado endereco da RAM de video. Esse endereco deve ser indicado por inteiros entre 0 e 16383. O endereco de cada tabela pode ser encontrado com a funcao BASE.

PROGRAMA EXEMPLO
    10 REM PROGRAMA VPEEK
    20 CLS
    30 PRINT"ABBCCCDDDDEEEEEFFFFFFGGGGGGGHHHHHHHHIIIIIIIIIJJJJJJJJJJ"
    40 FOR F=1 TO 55
    50 PRINT VPEEK(F);"...";
    60 NEXT F

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 173.

VER TAMBEM
  basic, screens, cores)MAN");

    add("vpoke", "", "MSX-BASIC", "VPOKE endereco,dado", "Insere dados num dado byte da VRAM (RAM de video).",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (video RAM poke)

  Insere dados num dado byte da VRAM (RAM de video).

FORMATO
  VPOKE endereco,dado

EXEMPLO
  VPOKE 929,255

DESCRICAO
  Esse comando insere um dado num byte da VRAM, controlada pelo VDP. O endereco pode estar entre 0 e 16383 e o dado pode estar entre 0 e 255.

PROGRAMA EXEMPLO
    10 REM PROGRAMA VPOKE
    20 CLS
    30 FOR F=0 TO 255
    40 VPOKE F,F
    50 NEXT F
    60 FOR F=1 TO 1000:NEXT F
    70 CLS
    80 LIST

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 174.

VER TAMBEM
  basic, screens, cores)MAN");

    add("wait", "", "MSX-BASIC", "WAIT numero da porta de E/S,", "Espera ate que um certo valor surja na porta de entrada/saida.",
        R"MAN(TIPO: INSTRUCAO (uso avancado)  (wait)

  Espera ate que um certo valor surja na porta de entrada/saida.

FORMATO
  WAIT numero da porta de E/S,
  expressao1, [expressao 2]

EXEMPLO
  WAIT 21,127

DESCRICAO
  Um XOR (ou exclusivo) e executado entre os dados da porta de E/S especificada e o valor da expressao 2. Um AND (e) e executado entre o resultado desta operacao e o valor da expressao 1. Se o resultado final for 0, os dados procedentes da porta de entrada/saida serao introduzidos continuamente. As expressoes podem assumir qualquer valor entre 0 e 255. Se a expressao 2 for omitida, assumira o valor 0.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 175.

VER TAMBEM
  basic, screens, cores)MAN");

    add("width", "", "MSX-BASIC", "WIDTH (numero de caracteres)", "Determina o numero de caracteres por linha no modo texto (SCREEN 0 ou 1).",
        R"MAN(TIPO: INSTRUCAO  (width)

  Determina o numero de caracteres por linha no modo texto (SCREEN 0 ou 1).

FORMATO
  WIDTH (numero de caracteres)

DESCRICAO
  Pode-se determinar o numero de caracteres por linha na tela do modo texto. No modo texto SCREEN 0, o numero de caracteres deve ser um numero inteiro entre 0 e 40. No modo texto SCREEN 1, o numero de caracteres deve estar entre 1 e 32. Ao ser ligado, o computador coloca 40 colunas no SCREEN 0 (29 no SCREEN 1).

PROGRAMA EXEMPLO
    10 REM PROGRAMA WIDTH
    20 INPUT"ESTE PROGRAMA SERA LISTADO ASSIM QUE FOR EXECUTADO. QUANTAS COLUNAS A TELA DEVERA TER";N
    30 WIDTH(N)
    40 LIST

NO MSX2+ (Manual MSX 2+ FM)
TIPO: INSTRUCAO  (width)

  Determina o numero de caracteres que sera usado por linha - versao MSX2+, com suporte a ate 80 colunas.

FORMATO
  WIDTH X

EXEMPLO
  WIDTH 80

DESCRICAO
  X e um numero inteiro que pode variar de 1 a 80 na SCREEN 0 e de 1 a 32 na SCREEN 1. Na SCREEN 0 se X for menor ou igual a 40, serao utilizados caracteres grandes para texto. Se X estiver entre 41 e 80 os caracteres serao pequenos. Os parametros de fabrica, quando o KIT 2+ e adquirido, sao: SCREEN 0 - 37 colunas, SCREEN 1 - 29 colunas.

PROGRAMA EXEMPLO
    10 SCREEN0
    20 FOR N=1 TO 80
    30 WIDTH N
    50 PRINT "Esta tela tem ";N;" colunas."
    60 FOR T=0 TO 300:NEXT
    70 NEXT

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 176.

VER TAMBEM
  basic, screens, cores)MAN");

    add("modos-de-operacao", "", "Manual MSX-BASIC", "", "Modos de Operacao (Parte I - A Estrutura do BASIC MSX)",
        R"MAN(Parte I - A Estrutura do BASIC MSX

Quando o computador e ligado, gera uma mensagem na tela:

MSX BASIC versao 1.1 Br
Gradiente
28815 bytes livres
Ok

Este "OK" significa que ele esta apto a receber comandos. Eles podem ser introduzidos de duas formas:

MODO DIRETO: O comando e digitado sem ser precedido por um "numero de linha". Ao teclarmos RETURN, ele e imediatamente executado. Seu resultado pode permanecer armazenado na memoria, mas a instrucao em si e perdida.

MODO PROGRAMACAO: Neste caso, cada linha contendo instrucoes e precedida por um numero. O computador, inicialmente, armazena todas as linhas em sua memoria (portanto, as instrucoes nelas contidas nao se perdem apos a execucao) e, quando solicitado, as executa sequencialmente.

As linhas do BASIC, no MSX, tem a seguinte estrutura:
NUMERO DA LINHA   INSTRUCAO   [: INSTRUCAO]   (CR)

onde:
- NUMERO DA LINHA: deve ser um numero inteiro entre 0 e 65529.
- INSTRUCAO: deve ser uma instrucao do BASIC MSX.
- [: INSTRUCAO]: varias instrucoes podem ser digitadas numa mesma linha, desde que o total de caracteres nela contidos nao ultrapasse 255. Os colchetes indicam complemento opcional.
- (CR): carriage return, ou, simplesmente, return. E obtido pressionando-se a tecla RETURN.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 15.)MAN");

    add("conjunto-de-caracteres", "", "Manual MSX-BASIC", "", "Conjunto de Caracteres (Parte I - A Estrutura do BASIC MSX)",
        R"MAN(Parte I - A Estrutura do BASIC MSX

Os caracteres do MSX sao os alfabeticos, os numericos, os especiais, os graficos e os do alfabeto europeu (Portugal tambem esta na Europa).

- Caracteres alfabeticos: sao as letras de A a Z, maiusculas e minusculas.
- Caracteres numericos: sao os algarismos de 0 a 9 (o zero e cortado para evitar confusoes com a letra O).
- Caracteres especiais: alem dos caracteres alfabeticos e numericos, os seguintes caracteres especiais sao reconhecidos pelo BASIC MSX:

CARACTERE -> INTERPRETACAO
- (espaco) -> Branco
- = -> Igual
- + -> Simbolo de adicao
- - -> Simbolo de subtracao
- * -> Simbolo de multiplicacao
- / -> Simbolo de divisao
- \ -> Simbolo de divisao inteira
- ^ -> Exponenciacao
- ( -> Abre parenteses
- ) -> Fecha parenteses
- % -> Porcentagem
- # -> Numero ou escopo
- " -> Aspas
- ! -> Ponto de exclamacao
- ; -> Ponto e virgula
- $ -> Dolar ou cifrao
- , -> Virgula
- . -> Ponto
- ' -> Apostrofo
- : -> Dois pontos
- & -> "E" comercial
- ? -> Ponto de interrogacao
- < -> Menor que
- > -> Maior que
- _ -> Barra inferior
- (BS) -> Back Space (volta o cursor apagando)
- (ESC) -> Escape
- (TAB) -> Tabulador
- (CR) -> Carriage Return
- (LF) -> Line Feed

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 16.)MAN");

    add("constantes", "", "Manual MSX-BASIC", "", "Constantes (Parte I - A Estrutura do BASIC MSX)",
        R"MAN(Parte I - A Estrutura do BASIC MSX

Sao valores usados durante uma execucao e podem ser classificados em dois tipos: alfabeticos (strings) e numericos.

CONSTANTE STRING: Uma constante string e uma sequencia de ate 255 caracteres, digitada entre aspas.

CONSTANTE NUMERICA: As constantes numericas sao de 6 tipos:
- Inteiras: que nao possuem ponto decimal. Seu valor pode estar entre -32768 e 32767.
- Ponto-fixo: sao numeros reais positivos ou negativos, que podem conter o ponto decimal.
- Ponto-flutuante: sao numeros positivos ou negativos representados na forma exponencial, semelhante a notacao cientifica. Uma constante com ponto flutuante consiste em uma constante inteira ou de ponto-fixo (com ou sem sinal) seguida de uma letra E ou D e da quantidade de posicoes que o ponto decimal deve andar (se o deslocamento for para a esquerda, o numero de posicoes e negativo). Por exemplo:
  23.4E3 = 23400
  419E-5 = .00419
  As constantes de ponto flutuante podem assumir valores entre 10^-64 e 10^63.
- Hexadecimal: numeros hexadecimais, precedidos por &H.
- Octal: numeros octais, precedidos por &O.
- Binarios: numeros binarios, precedidos por &B.

As constantes numericas podem ser processadas com precisao simples (6 algarismos significativos) ou dupla (14 algarismos significativos). O computador reconhece uma constante com precisao simples quando, no ponto flutuante, e usada a letra E, ou quando ela e seguida pelo ponto de exclamacao (!). Por exemplo:
  -5.007E3
  417.31!
Se, na notacao de ponto flutuante, for usada a letra D ao inves do E, ou se a constante for seguida de (#), ela e de precisao dupla. Por exemplo:
  2.71838183D-19
  3.141592#
Se nada disso for indicado, o BASIC MSX interpretara a constante como sendo de precisao dupla.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 17.)MAN");

    add("variaveis", "", "Manual MSX-BASIC", "", "Variaveis (Parte I - A Estrutura do BASIC MSX)",
        R"MAN(Parte I - A Estrutura do BASIC MSX

Variaveis sao nomes que podem ser atribuidos ou associados a resultados de calculos feitos nos programas.

O nome correspondente a uma variavel podera ser de qualquer tamanho, sendo que so os dois primeiros caracteres sao significativos. Este nome pode conter letras e numeros, porem o primeiro caractere deve ser sempre alfabetico.

Uma variavel nao pode ter um nome igual ao de um comando BASIC; isto inclui todos os comandos, os termos, os nomes das funcoes e as operacoes.

TIPOS DE VARIAVEIS: Assim como as constantes, as variaveis tambem podem ser alfanumericas (cujo nome deve sempre terminar por $) e numericas.

As variaveis numericas sao de 3 tipos e tambem podem ser distinguidas pelo ultimo caractere do nome:
- Variaveis inteiras (%): sao aquelas que so podem conter valores inteiros. Por exemplo, Z%.
- Variaveis de precisao simples (!): sao aquelas que possuem valores com ate 6 digitos significativos. Exemplo: PRECO!.
- Variaveis de precisao dupla (#): sao variaveis que possuem valores com ate 14 algarismos significativos. Exemplo: PI#.

Quando o nome de variavel nao termina por nenhum destes caracteres especiais ($, %, !, #), o BASIC MSX a interpreta como sendo numerica e de precisao dupla.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 18.)MAN");

    add("matrizes-ou-variaveis-indexadas", "", "Manual MSX-BASIC", "", "Matrizes ou Variaveis Indexadas (Parte I - A Estrutura do BASIC MSX)",
        R"MAN(Parte I - A Estrutura do BASIC MSX

Uma matriz e um conjunto de valores, todos associados a uma variavel de mesmo nome. Cada elemento da matriz e subscrito por indices inteiros (ou representados por expressoes inteiras). A quantidade de indices utilizados determina o numero de dimensoes da matriz e nao pode superar 255. Por exemplo:
- A(20) - Elemento de uma matriz de uma dimensao
- C(20,3,14) - Elemento de uma matriz de tres dimensoes

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 18.)MAN");

    add("ocupacao-de-memoria", "", "Manual MSX-BASIC", "", "Ocupacao de Memoria (Parte I - A Estrutura do BASIC MSX)",
        R"MAN(Parte I - A Estrutura do BASIC MSX

Cada string ocupa um numero de bytes igual ao numero de caracteres que ela contem, mais 3. As variaveis numericas e cada elemento de matriz ocupam um numero de bytes conforme a lista a seguir:
- Inteira: 2 bytes
- Precisao Simples: 4 bytes
- Precisao Dupla: 8 bytes

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 19.)MAN");

    add("conversao-de-precisao", "", "Manual MSX-BASIC", "", "Conversao de Precisao (Parte I - A Estrutura do BASIC MSX)",
        R"MAN(Parte I - A Estrutura do BASIC MSX

Se o valor de uma constante numerica for atribuido a uma variavel string (ou vice-versa) sera explicitada uma mensagem de erro "Type Mismatch".

Se o valor de uma constante numerica de um certo tipo for atribuido a variavel numerica de tipo diferente, ele sera armazenado na memoria do computador na forma determinada pelo tipo de variavel. Por exemplo, o programa:
10 Z%=257.8131#
20 PRINT Z%
dara como resultado:
257
Note que a parte decimal foi truncada sem arredondamento.

Durante calculos, operandos de tipos diferentes serao todos convertidos para o tipo de maior precisao, e nesse tipo sera fornecido o resultado.

Os operadores logicos convertem seus operandos em inteiros. Se eles cairem fora da faixa permitida aos inteiros, ocorrera erro (overflow).

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 19.)MAN");

    add("operadores", "", "Manual MSX-BASIC", "", "Operadores (Parte I - A Estrutura do BASIC MSX)",
        R"MAN(Parte I - A Estrutura do BASIC MSX

Os operadores agem sobre constantes e variaveis contidas numa expressao, de maneira a produzir um valor unico como resultado. Eles podem ser divididos em quatro categorias: ARITMETICOS, RELACIONAIS, LOGICOS, FUNCIONAIS.

OPERADORES ARITMETICOS (na ordem de prioridade):
- ^ : Exponenciacao. Exemplo: X ^ Y
- - : Mudanca de sinal. Exemplo: -X
- * , / : Multiplicacao e Divisao. Exemplo: X * Y  /  X / Y
- \ : Divisao Inteira. Exemplo: X \ Y
- MOD : Resto da Divisao. Exemplo: X MOD Y
- + , - : Adicao e Subtracao. Exemplo: X + Y  /  X - Y

Para mudar a prioridade, ou seja, a ordem de execucao, devemos usar parenteses.
Observacao: Note que \ e MOD transformam seus operandos em inteiros e fornecem resultados inteiros, sendo que \ fornece o quociente e MOD o resto da divisao. Note tambem que o sinal de adicao (+) pode ser usado para concatenar strings. Por exemplo:
"GRADI" + "ENTE" = "GRADIENTE"

OPERADORES RELACIONAIS: Estes operadores permitem comparacoes de valores:
- = : Igual. Exemplo: X = Y
- <> : Diferente. Exemplo: X<>Y
- < : Menor que. Exemplo: X < Y
- > : Maior que. Exemplo: X > Y
- <= : Menor ou igual. Exemplo: X <= Y
- >= : Maior ou igual. Exemplo: X >= Y

Observacao: Os operandos relacionados permitem tambem a comparacao de strings. Esta comparacao e feita entre os codigos ASCII de seus caracteres.

OPERADORES LOGICOS: Estes operadores permitem a manipulacao de bits, testes em relacoes multiplas e operacoes de algebra booleana. Estao listados a seguir em ordem de prioridade, com suas tabelas-verdade (1=verdadeiro, 0=falso):

NOT X:
- X=1 -> NOT X=0
- X=0 -> NOT X=1

X AND Y:
- X=1,Y=1 -> 1
- X=1,Y=0 -> 0
- X=0,Y=1 -> 0
- X=0,Y=0 -> 0

X OR Y:
- X=1,Y=1 -> 1
- X=1,Y=0 -> 1
- X=0,Y=1 -> 1
- X=0,Y=0 -> 0

X XOR Y:
- X=1,Y=1 -> 0
- X=1,Y=0 -> 1
- X=0,Y=1 -> 1
- X=0,Y=0 -> 0

X EQV Y:
- X=1,Y=1 -> 1
- X=1,Y=0 -> 0
- X=0,Y=1 -> 0
- X=0,Y=0 -> 1

X IMP Y:
- X=1,Y=1 -> 1
- X=1,Y=0 -> 0
- X=0,Y=1 -> 1
- X=0,Y=0 -> 1

Por exemplo, o resultado da instrucao:
PRINT 60 XOR 240
sera:
204
pois:
60 = &B 00111100
240 = &B 11110000
60 XOR 240 = &B 11001100 = 204

FUNCOES: O BASIC MSX tem funcoes residentes, ou seja, funcoes cuja tecnica de processamento ja esta gravada no seu sistema operacional, tais como LOG (logaritmo), SIN (seno), CSRLIN (cursor line), etc. Alem destas funcoes, o BASIC MSX admite outras concebidas pelo usuario. Neste caso ele devera defini-las pela instrucao DEF FN.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 19.)MAN");

    add("edicao-de-programas", "", "Manual MSX-BASIC", "", "Edicao de Programas (Parte I - A Estrutura do BASIC MSX)",
        R"MAN(Parte I - A Estrutura do BASIC MSX

O BASIC MSX dispoe de um editor cuja funcao e escrever, alterar e armazenar as linhas numeradas que constituem o programa. Este editor permite mover o cursor por toda a tela, reescrever caracteres, apaga-los, inseri-los e armazenar linhas de BASIC na memoria.

Uma linha de BASIC e aceita e armazenada na memoria se precedida por um numero inteiro entre 0 e 65529 e se contiver pelo menos um caractere diferente do espaco. Para que ela seja mandada para a memoria, apos terminar sua digitacao, devemos sempre teclar o RETURN.

Se digitarmos uma nova linha com o numero de uma ja armazenada, ao teclarmos RETURN, a segunda substitui a primeira, que e apagada. Isto ocorre tambem quando a nova linha tem so o numero. Esta caracteristica pode ser usada para apagar linhas.

Varias instrucoes podem ser colocadas numa mesma linha, desde que separadas por dois pontos (:). O total de caracteres contidos na linha, porem, nao pode exceder 255.

COMBINACOES DE TECLAS PARA EDICAO (codigo decimal / hex / combinacao / funcao / tecla especial correspondente, quando existir):
- 02 / 02 / CONTROL + B: Move o cursor ate o comeco da palavra anterior
- 03 / 03 / CONTROL + C: Da um BREAK na espera do INPUT
- 05 / 05 / CONTROL + E: Apaga do cursor ao fim da linha BASIC
- 06 / 06 / CONTROL + F: Move o cursor ate o comeco da palavra seguinte
- 07 / 07 / CONTROL + G: Da um "bip"
- 08 / 08 / CONTROL + H: Volta o cursor, apagando (tecla especial: BS)
- 09 / 09 / CONTROL + I: Move o cursor ate a proxima tabulacao (tecla especial: TAB)
- 10 / 0A / CONTROL + J: Coloca o cursor na proxima linha (line feed)
- 11 / 0B / CONTROL + K: Move o cursor ate o canto superior esquerdo (tecla especial: HOME)
- 12 / 0C / CONTROL + L: Limpa a tela colocando o cursor em HOME (tecla especial: CLS)
- 13 / 0D / CONTROL + M: Insere a linha BASIC na memoria (tecla especial: RETURN)
- 14 / 0E / CONTROL + N: Leva o cursor ao fim da linha
- 18 / 12 / CONTROL + R: Poe o cursor no modo insercao (tecla especial: INSERT)
- 21 / 15 / CONTROL + U: Apaga a linha interna (tecla especial: SELECT)
- 24 / 18 / CONTROL + X: (tecla especial: ESC)

Pressionando simultaneamente as teclas CONTROL + SHIFT + STOP, obtemos um RESET por software, ou seja, o Expert reinicializa as variaveis do sistema, deixando-as no estado em que se encontravam ao ligar o micro.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 21.)MAN");

    add("interrupcoes", "", "Manual MSX-BASIC", "", "Interrupcoes (Parte III - Aplicacoes Especiais)",
        R"MAN(Parte III - Aplicacoes Especiais

A interrupcao para a execucao de um programa para executar uma sub-rotina chamada normalmente de rotina de interrupcao.

Quando desenvolvemos um jogo, por exemplo, e normal usarmos o comando ON STRIG GOSUB para saber se os botoes do joystick foram pressionados.

OS COMANDOS DE INTERRUPCAO NO MSX: O MSX dispoe de varios comandos de interrupcao. A tabela a seguir mostra um resumo destas instrucoes:
- interrupcao quando se pressiona uma tecla de funcao -> ON KEY GOSUB
- interrupcao quando se pressiona a barra de espacos ou botoes dos joysticks -> ON STRIG GOSUB
- interrupcao quando se pressiona CTRL + STOP -> ON STOP GOSUB

Alem dessas instrucoes, que interrompem a execucao caso estas teclas sejam pressionadas, existem mais dois comandos de interrupcao: ON SPRITE GOSUB, que interrompe o programa quando duas figuras se sobrepoem, e ON INTERVAL GOSUB, que interrompe o programa em espacos periodicos de tempo.

TORNANDO AS INTERRUPCOES HABILITADAS: Para que a interrupcao realmente aconteca e necessario que seja dado um comando que habilite (ligue) esta interrupcao. Estes comandos sao:
KEY (X) ON
STRIG (X) ON
STOP ON
SPRITE ON
INTERVAL ON

EXEMPLIFICANDO ALGUMAS INTERRUPCOES: Digite o programa a seguir:
10 ON KEY GOSUB 100
20 KEY(1)ON
30 SCREEN 2
40 LINE(50,50)-(200,150),,B
50 GOTO 40
100 REM Sub-rotina
110 BEEP:CLS
120 FORI=1 TO 90 STEP 10
130 CIRCLE(120,100),I
140 NEXT I
150 CLS
160 RETURN

Este programa depois de rodado desenhara na tela um retangulo. Caso seja pressionada a tecla F1 o programa se desviara para a linha 100 devido a sentenca das instrucoes de interrupcoes nas linhas 10 e 20. Na sub-rotina que comeca na linha 100 ele desenhara nove circulos concentricos, e depois retornara para a linha 40, onde novamente o retangulo sera desenhado.

Agora pressione duas vezes seguidas a tecla F1 e verifique o que aconteceu. Voce notou que quando foi pressionada a tecla F1 pela segunda vez, a sub-rotina de interrupcao nao parou, e a nova interrupcao so foi feita quando a sub-rotina acabou de ser executada.

Vejamos duas variacoes deste mesmo programa. Acrescente a linha:
105 KEY(1) OFF
Execute novamente o programa e pressione a tecla F1. Agora pressione-a de novo. Voce percebe que a interrupcao so foi executada uma vez. Isto acontece porque quando e causada a primeira interrupcao o programa executa a linha 105 que desabilitara as proximas execucoes.

Como uma segunda variacao acrescente ao programa a linha:
105 KEY(1) ON
Com esta modificacao, toda vez que for pressionada a tecla F1, sera executada a sub-rotina de interrupcao. Mesmo que ela ja esteja sendo executada, sera desviada para seu inicio. Isto ocorre porque a linha 105, neste caso, reabilita uma nova interrupcao.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 178.)MAN");

    add("processo-de-arquivo", "", "Manual MSX-BASIC", "", "Processo de Arquivo (Parte III - Aplicacoes Especiais)",
        R"MAN(Parte III - Aplicacoes Especiais

Um arquivo e uma regiao da memoria onde podem ser armazenados dados, que possam ser constantemente atualizados.

Um bom exemplo seria um DIARIO pessoal. Ele e manuseado constantemente e, alem de guardar dados do dia, pode ser consultado tambem sobre registros anteriores.

No MSX existem comandos que fazem exatamente isto. Antes, porem, vejamos o que e necessario para montar este sistema.

Para se escrever no arquivo, serao necessarios os chamados dispositivos de saida (video, impressora, gravador) e para consultar um arquivo precisamos dos dispositivos de entrada (gravador). O gravador funciona tanto como dispositivo de entrada, como saida, pois nele podemos ler e escrever dados. Todas estas entradas e saidas sao controladas pelo microprocessador que se encontra dentro das UCP.

Cada um desses dispositivos tem uma abreviatura que sera usada quando forem dados os comandos de leitura e gravacao de dados. Estas abreviaturas sao:
- gravador cassete -> CAS:
- tela modo texto -> CRT:
- tela modo grafico -> GRP:
- impressora -> LPT:

Os nomes dos arquivos podem conter, no maximo, 6 caracteres (os excedentes serao desprezados).

Os comandos para abrir, ler, gravar e fechar um arquivo sao:
- OPEN -> abre um arquivo
- PRINT# -> escreve no arquivo
- PRINT# USING -> escreve no arquivo
- INPUT# -> le um arquivo
- LINE INPUT# -> le um arquivo
- CLOSE -> fecha arquivo

GRAVACAO DE DADOS DE UM ARQUIVO: Para gravar dados de um arquivo, deve-se proceder da seguinte maneira:
1) Abre-se o arquivo com a instrucao OPEN.
2) Escrevem-se os dados com a instrucao PRINT#.
3) Fecha-se o arquivo com a instrucao CLOSE.

O formato da instrucao OPEN para a gravacao de dados e:
OPEN "abreviatura do dispositivo nome do arquivo" FOR OUTPUT AS [#] numero do arquivo

Quando se executa isto, estabelece-se que um arquivo sera gravado no dispositivo indicado. Quando o MSX esta lendo ou escrevendo um arquivo, antes de armazena-los, passa os dados para uma memoria intermediaria (buffer). O MSX contem 16 buffers. O numero do arquivo determina em qual buffer serao armazenados provisoriamente os dados.

Depois de aberto o arquivo, com a instrucao OPEN, devem-se escrever os dados com a instrucao PRINT# que tem o seguinte formato:
PRINT# numero do arquivo, expressao [, expressao ...]

O numero do arquivo deve ser o mesmo que consta da instrucao OPEN. Depois de gravados os dados, sera automaticamente gravado um codigo de "carriage return" (&H0D) e um "line feed" (&H0A), para que os dados escritos sejam separados.

Um exemplo de como usar a instrucao PRINT# pode ser o seguinte:
PRINT#1,A$;",";B$

A virgula entre aspas indica que a variavel A$ deve ser separada da B$.

Finalmente, para fechar o arquivo voce deve digitar a instrucao CLOSE da seguinte maneira:
CLOSE [#] numero do arquivo

LEITURA DE DADOS DE UM ARQUIVO: Para se ler um arquivo deve-se proceder da seguinte maneira:
1) Abre-se o arquivo com a instrucao OPEN.
2) Le-se o arquivo com a instrucao INPUT#.
3) Fecha-se o arquivo com a instrucao CLOSE.

O formato da instrucao OPEN para leitura de dados e o seguinte:
OPEN "abreviatura do dispositivo nome do arquivo" FOR INPUT AS [#] numero do arquivo

Depois de aberto o arquivo com a instrucao OPEN, os dados devem ser lidos com a instrucao INPUT#, da seguinte maneira:
INPUT# numero do arquivo, variavel

Finalmente, deve-se fechar o arquivo com a instrucao CLOSE.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 180.)MAN");

    add("sub-rotinas-em-linguagem-de-maquina", "", "Manual MSX-BASIC", "", "Sub-rotinas em Linguagem de Maquina (Parte III - Aplicacoes Especiais)",
        R"MAN(Parte III - Aplicacoes Especiais

O microprocessador utilizado pelos micros da linha MSX e o Z-80A, para o qual podemos escrever programas em linguagem de maquina. Atraves de instrucoes do BASIC pode-se tambem transferir valores para esse programa e iniciar sua execucao. Se for necessario e possivel voltar ao BASIC, atraves da instrucao RET (return) do Z-80.

PREPARANDO UM PROGRAMA EM LINGUAGEM DE MAQUINA:

1. Limpe uma area da memoria para colocar seu programa em LM usando o comando:
CLEAR n, endereco
Por exemplo:
10 CLEAR 200,&HDFFF
No exemplo foram reservados 200 bytes de memoria.

2. Coloque o programa na memoria atraves do comando:
POKE (endereco), byte
Por exemplo:
20 POKE &HDFFF,&HCD
30 POKE &HE000,&HC3
40 POKE &HE001,&H0
50 POKE &HE002,&HC9
No exemplo, o programa e constituido pelas instrucoes CALL (&HCD), pelo endereco &H00C3 (&HC3,&H00 - o byte menos significativo vem antes) e por um RETurn (&HC9).

3. Defina o ponto de entrada (onde o Z-80 deve iniciar a execucao do programa), utilizando o comando:
DEFUSR n = endereco
Por exemplo:
DEFUSR 1 = &HDFFF
O BASIC-MSX permite definir 10 pontos de entrada diferentes, o que significa que podemos chamar 10 sub-rotinas diferentes em linguagem de maquina a partir do programa em BASIC. No exemplo definiu-se como inicio da sub-rotina 1, o endereco &HDFFF.

4. Execute a sub-rotina atraves do comando:
X = USRn (I)
Por exemplo:
40 X = USR1(I)
Este programa em BASIC coloca na memoria do computador um programa em LM que chama uma sub-rotina da ROM que limpa a tela. Esta sub-rotina esta no endereco &H00C3.

PASSANDO VALORES PARA A SUB-ROTINA EM LM: Quando for preciso enviar um valor a uma sub-rotina em LM utiliza-se a forma:
Variavel=USR n(I)

O valor enviado sera I e se houver valor para retornar ao BASIC este sera atribuido a variavel. Se fizermos:
X = USR1(I)

Os dados que indicam o tipo e o valor da variavel utilizada sao armazenados na memoria de acordo com a tabela a seguir, e o endereco do comeco da area de armazenamento e indicada pelo registro HL. O dado no registro A tambem e armazenado no endereco &HF663.

TIPO DE I / REGISTRO A / REGISTROS HL / ENDERECO ONDE E ARMAZENADO O VALOR DE I:
- dupla precisao: A=8, HL=&HF7F6, enderecos &HF7F6-&HF7FD
- precisao simples: A=4, HL=&HF7F6, enderecos &HF7F6-&HF7F9
- inteiro: A=2, HL=&HF7F6, enderecos &HF7F8-&HF7F9

Se a variavel for uma string: REGISTRO A=3, REGISTROS DE=endereco do ponteiro da string, PONTEIRO DA STRING: 1 byte=comprimento da string, 2 e 3 bytes=endereco onde a variavel esta armazenada.

Terminada a sub-rotina, o valor obtido sera atribuido a variavel X e a memoria, e os registros ficarao organizados assim:
TIPO DE RESULTADO / ENDERECO &HF633 / REGISTROS DE / REGISTROS HL / ENDERECO ONDE O RESULTADO E ARMAZENADO:
- precisao dupla: 8, (nenhum), &HF7F6, &HF7F6-&HF7FD
- precisao simples: 4, (nenhum), &HF7F6, &HF7F6-&HF7F9
- inteiro: 2, (nenhum), &HF7F6, &HF7F8-&HF7F9
- string: 3, endereco do ponteiro da string, &HF7F6, endereco indicado pelos 2 e 3 bytes do ponteiro da string

EXEMPLO (ORDENADOR EM LINGUAGEM DE MAQUINA - ordena nomes digitados pelo usuario usando uma sub-rotina em linguagem de maquina; ver nota CONFERIR acima sobre risco de erro de transcricao neste listing):
5 REM ORDENADOR EM LINGUAGEM DE MAQUINA
10 '                DADOS
15 DATA 035,035,094,035,086,213,221,225
20 DATA 221,094,254,221,006,255,027,221
25 DATA 229,221,070,000,078,003,221
30 DATA 110,004,221,102,005,229,253,225
35 DATA 126,221,110,001,221,102,150
40 DATA 056,014,032,058,005,040,055,013
45 DATA 040,000,035,253,035,253,126,000
50 DATA 024,237,221,043,221,043,043
55 DATA 221,126,006,221,070,003,221,043
60 DATA 006,221,119,003,221,126,004
65 DATA 070,004,221,112,007,221,119,004
70 DATA 221,126,221,119,005,221,043
75 DATA 008,221,007,005,024,171,225,027
80 DATA 221,035,221,035,221,027,225,027,122
85 DATA 179,032,156,201
90 '           INICIO DO PROCESSAMENTO
95 CLEAR 6000,&HF000:DEFINT A-Z
100 DEFUSR=&HF000
105 INPUT"Quantos nomes serao inseridos";N:DIM N$(N)
110 '        INSERE OS CODIGOS DAS LINHAS DATA NOS BYTES ESPECIFICADOS
115 FOR F=0 TO &H73
120 READ D:POKE &HF000+F,D
125 NEXT F
130 '        RECEBE OS NOMES A SEREM ORDENADOS ATRAVES DO TECLADO
135 FOR F=1 TO N: F$=STR$(F)
140 PRINT"Qual o ";F$;CHR$(&HF8);") nome";:INPUT N$(F)
145 FOR G=1 TO LEN(B$)
150 C$=MID$(B$,G,1):C=ASC(C$)
155 IF C<97 THEN C=C+32:C$=CHR$(C)
160 MID$(B$,G,1)=C$
165 NEXT G
170 MID$(B$,1,1)=CHR$(ASC(LEFT$(B$,1))-32):N$(F)=B$
175 NEXT F
180 '        INICIA A ORDENACAO
185 N$(0)=" "+CHR$(1):E=VARPTR(N$(0))
190 PRINT "INICIO !":TIME=0
195 E=USR(E):T=TIME
200 FOR F=1 TO N
205 PRINT F;N$(F)
210 NEXT F
215 PRINT:PRINT:PRINT"Tempo de processamento"
220 PRINT INT(1000000#*T/60);"micro-segundos"
225 END

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 182.)MAN");

    add("apendice-a-caracteres-ascii", "", "Manual MSX-BASIC", "", "Apendice A - Caracteres ASCII (Parte IV - Apendices)",
        R"MAN(Parte IV - Apendices

A tabela fornece o codigo hexadecimal dos 256 caracteres armazenados na memoria ROM do Expert. Para obter um determinado codigo, le-se primeiro a linha e depois a coluna. Por exemplo, o caractere L corresponde ao codigo ASCII 4C (em hexadecimal).

FAIXA 00-1F (codigos de controle): no MSX, esta faixa nao mostra caracteres de controle invisiveis como no ASCII padrao, e sim simbolos graficos especiais (carinhas, naipes de baralho, setas, simbolos de genero, notas musicais, etc.) usados sobretudo em jogos e telas decorativas.

FAIXA 20-7E (caracteres imprimiveis, identica ao ASCII padrao):
20=(espaco) 21=! 22=" 23=# 24=$ 25=% 26=& 27=' 28=( 29=) 2A=* 2B=+ 2C=, 2D=- 2E=. 2F=/
30=0 31=1 32=2 33=3 34=4 35=5 36=6 37=7 38=8 39=9 3A=: 3B=; 3C=< 3D== 3E=> 3F=?
40=@ 41=A 42=B 43=C 44=D 45=E 46=F 47=G 48=H 49=I 4A=J 4B=K 4C=L 4D=M 4E=N 4F=O
50=P 51=Q 52=R 53=S 54=T 55=U 56=V 57=W 58=X 59=Y 5A=Z 5B=[ 5C=\ 5D=] 5E=^ 5F=_
60=` 61=a 62=b 63=c 64=d 65=e 66=f 67=g 68=h 69=i 6A=j 6B=k 6C=l 6D=m 6E=n 6F=o
70=p 71=q 72=r 73=s 74=t 75=u 76=v 77=w 78=x 79=y 7A=z 7B={ 7C=| 7D=} 7E=~

FAIXA 7F: caractere grafico especial (nao e DEL/apagar como no ASCII padrao).

FAIXA 80-FF (caracteres internacionais e graficos): contem vogais e consoantes acentuadas (a exemplo de C-cedilha, U-trema e as vogais com til/acento/circunflexo usadas em portugues), fracoes (1/2, 1/4), sinais de pontuacao invertidos (! (invertido), ? (invertido)), caracteres de desenho de caixas e blocos solidos/sombreados, e letras gregas e simbolos matematicos usados em ciencia (alfa, beta, gama, pi, sigma, mi/micro, tau, phi, teta, omega, delta, infinito, raiz quadrada, graus, mais-ou-menos, maior-ou-igual, menor-ou-igual, integral). Ver a grade grafica completa na pagina 186 para o glifo exato de cada codigo desta faixa.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 186.)MAN");

    add("apendice-c-codigos-de-erro", "", "Manual MSX-BASIC", "", "Apendice C - Codigos de Erro (Parte IV - Apendices)",
        R"MAN(Parte IV - Apendices

Lista completa das mensagens de erro do BASIC MSX (nome original em ingles, como aparece na tela, e a explicacao dada pelo livro):

- Bad file name (Nome Incorreto para Arquivo): Uma forma ilegal foi usada para o nome do arquivo com: LOAD, SAVE, KILL, NAME, etc.

- Bad file number (Problemas com Numero de Arquivo): Ha um termo, ou comando, referente a um numero de arquivo, o qual nao esta aberto (OPEN). Ou entao, ele esta fora do alcance dos numeros de arquivo especificados pelo termo MAXFILES.

- Can't continue (Impossibilidade de Continuar): Foi feita uma tentativa de continuar um programa, o qual: 1. Parou devido a um erro; 2. Tenta ser modificado durante uma parada na execucao; 3. Ou, simplesmente, nao existe continuacao.

- Device I/O error (Erro no Dispositivo de I/O): Ocorreu um erro de I/O (Input/Output) no cassete, na impressora ou na operacao de CRT; ou seja, ha um erro na entrada ou saida de dados. Este erro e fatal e isso significa que o BASIC nao pode encontra-lo.

- Direct statement in file (Termo no Modo Direto foi Encontrado no Arquivo): Um termo foi encontrado carregando um arquivo no formato ASCII. O LOAD foi terminado.

- Division by zero (Divisao por Zero): Uma divisao por zero foi encontrada em uma expressao.

- FIELD overflow (Overflow em FIELD): Tentativa de inserir um FIELD num espaco muito pequeno de bytes.

- File already open (Arquivo Ja Aberto): Uma saida sequencial, no modo OPEN, foi o resultado de arquivo que ja havia sido aberto ou, entao, um termo KILL abrindo o mesmo tipo de arquivo.

- File not found (Arquivo Nao Encontrado): Ha um termo LOAD, KILL ou OPEN referente a um arquivo, que nao existe na memoria.

- File not open (Arquivo Nao Aberto): O arquivo especificado em um termo PRINT#, INPUT#, etc, nao havia sido aberto.

- Illegal direct (Modo Direto Usado Ilegalmente): Um comando que e usado no modo indireto foi introduzido no modo direto.

- Illegal function call (Funcao Ilegal): Tentativa de executar uma operacao usando um parametro ilegal.

- INPUT past end (INPUT Depois do Final): Um termo INPUT foi executado depois de todos os dados de um arquivo (ou arquivo nulo) serem lidos. Evita-se este erro usando a funcao EOF para encontrar o final do arquivo.

- Internal error (Erro Interno): Mau funcionamento interno.

- Line buffer overflow (Linha do Buffer em Overflow): Uma linha introduzida possui caracteres demais.

- Missing operand (Falta de Operando): Tentativa de operacao sem o fornecimento de um dos operandos necessarios.

- NEXT without FOR (NEXT sem FOR): Comando NEXT utilizado sem uma instrucao FOR correspondente.

- No RESUME (Falta RESUME): Um erro encontrado na rotina foi introduzido porem, nao contem o termo RESUME.

- Out of DATA (Insuficiencia de Dados): Uma instrucao READ foi executada sem encontrar nenhum dado para ler.

- Out of memory (Insuficiencia de Memoria): Toda memoria disponivel foi utilizada ou reservada.

- Out of string space (Fora do Espaco para String): O espaco reservado para as variaveis strings foi excedido na memoria.

- Overflow (Sobrecarga): A magnitude de um numero e muito grande para o computador.

- Redimensioned array (Redimensionamento): Dois termos DIM foram usados para dimensionar a mesma matriz.

- RESUME without error (RESUME sem Erro): Um termo RESUME foi encontrado antes de um erro colocado na rotina.

- RETURN without GOSUB (RETURN sem GOSUB): Uma instrucao RETURN foi encontrada sem que um GOSUB fosse executado antes.

- Sequential I/O only (Somente Entrada/Saida Sequencial): Um termo de acesso randomico esta distribuindo o arquivo de forma sequencial.

- String formula too complex (Formula de String Muito Complexa): Uma expressao string esta muito longa ou muito complexa. A expressao deve ser dividida em expressoes menores.

- String too long (String Muito Longa): Tentativa de criar uma string com mais de 255 caracteres.

- Subscript out of range (Sub-indice Fora de Faixa): Tentativa de usar um indice nao definido para uma variavel indexada.

- Syntax error (Erro de Sintaxe): Ocorre quando uma palavra do vocabulario BASIC e escrita de forma incorreta ou quando se faz uso indevido da pontuacao.

- Type mismatch (Atribuicao Ilegal): Para um nome de uma variavel string foi atribuido um valor, ou vice-versa. Para uma funcao, que aguarda um argumento numerico foi dado um argumento string, ou vice-versa.

- Undefined line number (Numero de Linha Nao Definido): Uma linha contendo um GOTO, GOSUB, IF...THEN...ELSE, esta se referindo a outra linha que e inexistente.

- Undefined user function (Uso Indefinido da Funcao): A funcao FN foi chamada antes de ser definida pela instrucao DEF FN.

- Unprintable error (23) (Erro Nao Imprimivel): Nao ha mensagem de erro disponivel para a condicao existente. Isto geralmente acontece quando uma mensagem ERROR aparecer, para um codigo de erro invalido ou indefinido.

- Unprintable error (Erros Nao Imprimiveis): Estes codigos nao tem definicao. Devem ser reservados para futuras expansoes do BASIC.

- Unprintable error (60-255) (Erro Nao Imprimivel): Estes codigos nao possuem definicao. Costuma-se usa-los para definir codigos pessoais de erro.

- Verify error (Erro de Verificacao): O programa que esta sendo executado nao esta coerente com o programa gravado no cassete.

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 188.)MAN");

    add("apendice-d-mapa-da-memoria", "", "Manual MSX-BASIC", "", "Apendice D - Mapa da Memoria (Parte IV - Apendices)",
        R"MAN(Parte IV - Apendices

O Expert possui 32 Kbytes de memoria ROM (pre-gravada na fabrica, apenas para leitura) e 80 Kbytes de memoria RAM (para escrita e leitura). O microprocessador Z80-A pode acessar diretamente 64 Kbytes de memoria RAM e e em parte dessa area que o usuario pode trabalhar. Outros 16 Kbytes (VRAM) sao acessados pelo microprocessador TMS9128NL, especifico para controlar o video.

Tanto a ROM como a RAM sao divididas em paginas para agilizar o acesso aos seus enderecos. Cada pagina de memoria tem 16 Kbytes.

A numeracao dos enderecos da ROM e sempre a mesma, independentemente de se estar ou nao usando expansoes, interfaces ou cartuchos. Ela se sobrepoe a parte da numeracao da RAM e e por isso que apenas 28,8 Kbytes desta podem ser acessados diretamente pelo BASIC.

O MSX possui quatro slots, sendo dois internos (0 e 2) e dois externos (1 e 3). Os dois slots internos sao ocupados pela ROM (slot 0) e pela RAM (slot 2). Os slots externos podem ser conectados a cartuchos, interfaces, etc.

O slot 1 tem prioridade sobre o slot 3, isto e, se existir um cartucho ligado em cada um deles, prevalecera a operacao do que esta no slot 1. A conexao do slot 1 e a marcada com CARTRIDGE A. O slot 3 possui duas conexoes: uma dianteira marcada com CARTRIDGE B e outra traseira marcada com BUS EXPANSION. Salvo indicacao em contrario, eles nao devem ser utilizados simultaneamente.

MAPA DA MEMORIA (4 paginas de 16K, slots 0-3):
- pagina 3: slot 0=(vazio), slot 1=(vazio - CARTRIDGE A), slot 2=RAM USUARIO BASIC, slot 3=(vazio - BUS EXPANSION)
- pagina 2: slot 0=(vazio), slot 1=(vazio - CARTRIDGE A), slot 2=RAM USUARIO BASIC, slot 3=(vazio - BUS EXPANSION)
- pagina 1: slot 0=ROM, slot 1=(vazio - CARTRIDGE A), slot 2=RAM USUARIO, slot 3=(vazio - BUS EXPANSION)
- pagina 0: slot 0=ROM, slot 1=(vazio - CARTRIDGE A), slot 2=RAM USUARIO, slot 3=(vazio - BUS EXPANSION)

A RAM acessivel pelo usuario que programa em BASIC e constituida pelas duas paginas superiores do slot 2 (paginas 2 e 3, 32K no total). As duas paginas inferiores podem ser acessadas mudando-se o status do PPI (veja APROFUNDANDO-SE NO MSX, da mesma editora, para maiores detalhes).

DISTRIBUICAO DA RAM (usuario BASIC, slot 2, do endereco alto &HFFFF ao baixo &H8000, total 28815 bytes livres dentro dos 32K):
- &HFFFF: Area usada pelo sistema
- &HF380: Bloco de controle de arquivo (tamanho determinado por MAXFILES)
- Area de string (tamanho definido pela instrucao CLEAR; se nao houver definicao, sera reservada uma area de 200 bytes)
- Area de stack (guarda enderecos de retorno quando executadas as instrucoes FOR-NEXT ou GOSUB-RETURN)
- Area livre (a area nao utilizada)
- Area de matrizes
- Area de variaveis (armazena dados numericos e os "pointers" para os dados alfanumericos)
- &H8000: Area do programa em BASIC (onde serao armazenados programas com seus respectivos numeros de linha)

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 192.)MAN");

    add("apendice-i-funcoes-trigonometricas-e-hiperbolicas", "", "Manual MSX-BASIC", "", "Apendice I - Funcoes Trigonometricas e Hiperbolicas (Parte IV - Apendices)",
        R"MAN(Parte IV - Apendices

O BASIC MSX permite calcular diretamente as funcoes SIN (seno), COS (cosseno), TAN (tangente) e ATN (arco-tangente), onde os angulos sao medidos em radianos.

Para as outras funcoes trigonometricas e hiperbolicas, o calculo deve ser precedido da definicao de funcao. Por exemplo, sabendo-se que a funcao secante e o inverso do cosseno, podemos calcula-la para um angulo X qualquer (em radianos) usando o seguinte programa exemplo:
10 DEF FN SEC(X) = 1/COS(X)
20 INPUT X
30 PRINT FN SEC(X)

A correlacao entre as funcoes a serem definidas e as residentes no BASIC MSX e dada na tabela a seguir:
SEC(X)      = 1/COS(X)
CSC(X)      = 1/SIN(X)
COT(X)      = 1/TAN(X)
ARCSIN(X)   = ATN(X/SQR(-X*X+1))
ARCCOS(X)   = -ATN(X/SQR(-X*X+1))+1.5708
ARCSEC(X)   = ATN(SQR(X*X-1))+(SGN(X)-1)*1.5708
ARCCSC(X)   = ATN(1/SQR(X*X-1))+(SGN(X)-1)*1.5708
ARCCOT(X)   = -ATN(X)+1.5708
SINH(X)     = (EXP(X)-EXP(-X))/2
COSH(X)     = (EXP(X)+EXP(-X))/2
TANH(X)     = -EXP(-X)/(EXP(X)+EXP(-X))*2 + 1
SECH(X)     = 2/(EXP(X)+EXP(-X))
CSCH(X)     = 2/(EXP(X)-EXP(-X))
COTH(X)     = EXP(-X)/(EXP(X)-EXP(-X))*2 + 1
ARCSINH(X)  = LOG(X+SQR(X*X+1))
ARCCOSH(X)  = LOG(X+SQR(X*X-1))
ARCTANH(X)  = LOG((1+X)/(1-X))/2
ARCSECH(X)  = LOG((SQR(-X*X+1)+1)/X)
ARCCSCH(X)  = LOG((SGN(X)*SQR(X*X+1)+1)/X)
ARCCOTH(X)  = LOG((X+1)/(X-1))/2

FONTE
  Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina 207.)MAN");

    add("msx-2-apresentacao", "", "Manual MSX-BASIC", "", "MSX 2+ - Apresentacao (MSX 2+ (manual ACVS))",
        R"MAN(MSX 2+ (manual ACVS)

As informacoes desta secao permitem que seja usada toda a potencialidade do novo MSX BASIC 3.0 incorporado no cartucho MSX 2+ FM fabricado pela ACVS Eletronica.

As principais caracteristicas do computador depois da conexao do cartucho MSX 2+ FM sao:
- Texto de ate 80 colunas (mesmo pela televisao).
- Resolucao grafica maxima de 512 x 424 pontos com 16 cores de 512 combinacoes (screen 7 modo entrelacado).
- Resolucao maxima de cores de 19.268 (screen 12).
- Memoria ROM do BASIC com 64 KBytes (48 KBytes com o MSX BASIC 3.0 e 16 KBytes com Music-Basic).
- Memoria RAM de video com 128 KBytes.
- Trabalha com varias paginas graficas.
- SCREEN 0 a 12.
- SPRITES multicoloridos.
- Pode ter ate 8 SPRITES na mesma linha horizontal.
- Relogio-calendario interno mantido por bateria que atualiza hora e data mesmo com o cartucho desconectado.
- Processador de video que executa a maior parte das funcoes graficas acelerando o processamento.
- RAMDISK interna com ate 32 KBytes.
- Instrucoes SET PASSWORD, SETTITLE, SETPROMPT, etc.
- Sintetizador com mais 9 canais de som FM.
- 5 sons de instrumentos de percussao (baterias).
- 64 sons de instrumentos musicais pre-programados.
- Saida do sintetizador FM em 2 canais.
- Saidas de video colorido (composto) e de RF (canal 3).

FONTE
  Manual MSX 2+ FM (ACVS), pagina 5.)MAN");

    add("msx-2-sintaxe-usada-no-manual", "", "Manual MSX-BASIC", "", "MSX 2+ - Sintaxe usada no manual (MSX 2+ (manual ACVS))",
        R"MAN(MSX 2+ (manual ACVS)

Descricao da sintaxe usada no capitulo MSX 2+ do manual ACVS (mesma ideia da legenda do livro Gradiente, com uma categoria a mais: COMANDO).

NOME [(X)]   (TIPO)

1 - O nome do comando, instrucao ou funcao esta em letras maiusculas e devera ser escrito exatamente como aparece.
2 - Itens entre parenteses ( ) devem ser definidos pelo usuario.
3 - Itens entre colchetes [ ] sao opcionais.
4 - Todas as pontuacoes devem ser escritas como aparecem no formato. Isto se aplica a pontos finais, virgulas, parenteses, dois pontos, ponto e virgula, etc.
5 - X, Y e Z representam expressoes numericas.
6 - X$, Y$ e Z$ representam expressoes alfanumericas.

(TIPO) - Indica se e um COMANDO, uma INSTRUCAO ou uma FUNCAO. A diferenca entre os tipos e a seguinte:
- COMANDO: e a ordem que, depois de executada, coloca o computador no modo de comando direto via teclado. Por exemplo: LIST.
- INSTRUCAO: e a ordem que pode ser dada via teclado ou dentro de um programa. Por exemplo PRINT, GOTO, GOSUB, etc.
- FUNCAO: so e valida acompanhada por uma INSTRUCAO. Por exemplo: PRINT BASE(0).

FONTE
  Manual MSX 2+ FM (ACVS), pagina 6.)MAN");

    add("fm-music-apresentacao", "", "Manual MSX-BASIC", "", "FM-Music - Apresentacao (FM-Music (manual ACVS))",
        R"MAN(FM-Music (manual ACVS)

O FM-Music, residente no cartucho MSX2+ FM (e tambem vendido como cartucho separado) e capaz de sintetizar o som de diversos instrumentos musicais, utilizando-se de 2 osciladores multi-timbrais e mais os sons de 5 instrumentos de percussao. Nele estao disponiveis 9 canais que podem tocar ate 9 instrumentos ao mesmo tempo sem utilizar a bateria.

Contudo, se os 5 canais da bateria eletronica forem acionados, ficam ainda 6 canais simultaneos para outros instrumentos. Os 3 canais do PSG (chip de som original do MSX) continuam ativos, podendo ser tocados junto com os canais do FM, resultando em ate 14 canais simultaneos.

O Music-Basic tem armazenado os sons de 64 instrumentos musicais sendo 63 pre-definidos e 1 programavel. Os pre-definidos (de 0 a 62) sao os mais utilizados, como piano, flauta, clarineta, violino, cordas, trompete, xilofone, etc.

O som desses instrumentos pode ser alterado atraves de rotinas em Assembler ou Poke's em Basic. A programacao do instrumento 63 e feita atraves de parametros transmitidos aos registradores dos osciladores. E necessario, portanto, um certo conhecimento de sonorizacao para sintetizar com perfeicao outros tipos de instrumentos.

O FM pode ser programado tanto em Basic quanto em Assembler. Em Basic atraves de comandos expandidos, tais como CALL MUSIC, CALL VOICE, entre outros (ver dicionario, letra C). Em Assembler atraves de rotinas especificas em linguagem de maquina (um processo que, pelo nivel de complexidade, nao foi detalhado no manual original).

FONTE
  Manual MSX 2+ FM (ACVS), pagina 37.)MAN");

    add("fm-music-apendice-a-programacao-de-instrumentos", "", "Manual MSX-BASIC", "", "FM-Music - Apendice A - Programacao de Instrumentos (FM-Music (manual ACVS))",
        R"MAN(FM-Music (manual ACVS)

O FM possui 64 instrumentos sendo um deles, o 63, programavel. Mas existem algumas dificuldades para se gerar o som de um instrumento. Uma delas e saber o que faz cada um dos 16 registradores do FM.

Cada instrumento e definido por 4 linhas de 4 bytes (16 bytes no total, tratados em blocos de 16 bits cada - ver CALL VOICECOPY):
1) Nome do instrumento (opcional, 8 bytes, em ASCII hexadecimal se usado).
2) Afinacao e envelope: byte 1 = ajuste grosso da afinacao (default/inicial), byte 2 = ajuste fino da afinacao, byte 3 = inutil, byte 4 = valor do gerador de envelope (formato da amplitude do som) para mixagem dos dois osciladores. Os quatro bytes restantes desta categoria (5,6,7,8) tem funcao desconhecida.
3) Dados do 1o oscilador (CARRIER): byte 1 = nivel de graves; byte 2 (subdividido em 4 partes) = vibracao (efeitos vibro e tremolo) e tonalidade do som; byte 3 = prolongamento do som em relacao ao gerador de envelopes; byte 4 = ataque do 1o oscilador. Os ultimos 4 bytes tambem sao inuteis neste caso.
4) Dados do 2o oscilador (MODULATE): parecido com o topico 3, com poucas diferencas - byte 1 tem a mesma funcao do topico anterior (ligeiramente mais fino no ajuste); byte 2 subdividido em 3 partes (vibracao, que dependendo de alguns fatores pode nao funcionar, e tonalidade deste oscilador); byte 3 = volume dos dois osciladores; byte 4 = ataque deste oscilador.

OBS: Os ultimos bytes de cada linha (exceto a primeira), que nao tem funcao, podem assumir qualquer valor. Os valores devem sempre ser colocados em hexadecimal. No APENDICE C ha uma relacao com todos os instrumentos pre-definidos.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 48.)MAN");

    add("fm-music-apendice-b-dicas-e-macetes", "", "Manual MSX-BASIC", "", "FM-Music - Apendice B - Dicas e Macetes (FM-Music (manual ACVS))",
        R"MAN(FM-Music (manual ACVS)

O FM Music se caracteriza principalmente pela facilidade de programacao tanto em BASIC quanto em ASSEMBLER. Alguns truques que produzem bons efeitos musicais:

GERACAO DE ECO OU REVERBERACAO: O som dos instrumentos no FM Music e originalmente composto por apenas 2 osciladores. Sao considerados sons puros (extraidos direto do instrumento que esta sendo simulado), ou seja, nao ha a entonacao timbral como numa musica de estudio. Para resolver este problema e necessario adicionar mais um canal ao que desejamos tocar e colocar um POKE que va alterar gradualmente o timbre da nota que estivermos tocando, dando a impressao de uma pequena reverberacao (ex.: CALL MUSIC(0,0,2):POKE&HFA3C,40 antes de um PLAY em dois canais, comparado com o mesmo PLAY em um so canal, sem o POKE, resulta num som mais polido).

Variaveis de memoria uteis para todos os canais:
&HFA2C - PITCH CANAL 1 ; &HFA2D - TRANSPOSE CANAL 1
&HFA3C - PITCH CANAL 2 ; &HFA3D - TRANSPOSE CANAL 2
&HFA4C - PITCH CANAL 3 ; &HFA5D - TRANSPOSE CANAL 3
&HFA6C - PITCH CANAL 4 ; &HFA6D - TRANSPOSE CANAL 4
&HFA7C - PITCH CANAL 5 ; &HFA7D - TRANSPOSE CANAL 5
&HFA8C - PITCH CANAL 6 ; &HFA8D - TRANSPOSE CANAL 6
&HFA9C - PITCH CANAL 7 ; &HFA9D - TRANSPOSE CANAL 7
&HFAAC - PITCH CANAL 8 ; &HFAAD - TRANSPOSE CANAL 8
&HFAAC - PITCH CANAL 9 ; &HFAAD - TRANSPOSE CANAL 9

Outro recurso para o som sair com eco consiste em utilizar 2 canais tocando uma nota em cada, separando a mesma sequencia de notas em dois canais concatenados (ex.: uma sequencia unica dividida em dois PLAY# com um pequeno atraso relativo entre eles produz um bonito efeito de eco - e necessario que o instrumento utilizado tenha um som prolongado).

INSTRUMENTOS NAO-MIXAVEIS: alguns instrumentos, por motivos desconhecidos, nao podem ser mixados com outros mesmo em diversos canais. Esses instrumentos, geralmente os mais altos, devem ser utilizados com certa precaucao sob risco de atrapalhar os outros.
- MIXAVEIS: 00,02,03,04,05,06,07,09,12,14,16,24.
- NAO-MIXAVEIS: 01,08,10,11,13,15,17,18,19,20,21,22,23,24 ate o 62.

PARAMETRO "Y": quando utilizado dentro de um comando PLAY, serve para alterar o som de alguns instrumentos pre-definidos. Caso esteja dentro de um canal de bateria, ira alterar o som dos instrumentos de bateria (como no comando CALL AUDREG). A sintaxe de sua utilizacao e: Y<r>,<n>, onde <r> e um registrador de som entre 0 e 255 onde nao sao aceitos todos os numeros, apenas alguns. O <n> e um valor qualquer entre 0 e 255 atribuido a esse registrador.

Note que se pode usar o parametro "@W" para prolongar o tempo de execucao da nota, uma vez que o parametro "Y" nao faz isso (maiores detalhes veja o comando CALL AUDREG).

FONTE
  Manual MSX 2+ FM (ACVS), pagina 50.)MAN");

    add("fm-music-apendice-c-relacao-de-instrumentos", "", "Manual MSX-BASIC", "", "FM-Music - Apendice C - Relacao de Instrumentos (FM-Music (manual ACVS))",
        R"MAN(FM-Music (manual ACVS)

Tabela com todos os 64 instrumentos do FM (numero : nome):
00 Piano I, 01 Piano II, 02 Violin, 03 Flute, 04 Clarinet, 05 Oboe, 06 Trumpet, 07 Pipe Organ, 08 Xylophone, 09 Organ, 10 Guitar, 11 Santool, 12 Eletric Piano, 13 Clavicod, 14 Harp Sicked, 15 Harp Sicked II, 16 Vibraphone, 17 Koto, 18 Taiko, 19 Engine, 20 UFO, 21 Synth Bell, 22 Chime, 23 Synth Bass, 24 Synthesizer, 25 Synth Percussion, 26 Synth Rhythm, 27 Harm Drum, 28 Cow Bell, 29 Close Hi-Hat, 30 Snare Drum, 31 Bass Drum, 32 Piano III, 33 Eletric Piano II, 34 Santool II, 35 Brass, 36 Flute II, 37 Clavicod II, 38 Clavicod III, 39 Koto II, 40 Pipe Organ II, 41 PohdsPLA, 42 RohdsPLA, 43 Orch L, 44 Orch R, 45 Synth Violin, 46 Synth Organ, 47 Synth Brass, 48 Tube, 49 Shamisen, 50 Magical, 51 Huwawa, 52 Wonder Flute, 53 Hard Rock, 54 Machine, 55 Machine V, 56 Comic, 57 SE Comic, 58 SE Laser, 59 SE Noise, 60 SE Star, 61 SE Star II, 62 Engine II, 63 Silence (instrumento programavel, sem som proprio ate ser definido pelo usuario - ver CALL VOICECOPY).

FONTE
  Manual MSX 2+ FM (ACVS), pagina 52.)MAN");

    add("fm-music-apendice-d-exemplos-de-musicas", "", "Manual MSX-BASIC", "", "FM-Music - Apendice D - Exemplos de Musicas (FM-Music (manual ACVS))",
        R"MAN(FM-Music (manual ACVS)

O manual traz dois exemplos completos de musicas em BASIC usando o FM-Music (paginas 53 a 62 do manual ACVS). Este topico DESCREVE os dois programas em vez de transcreve-los nota a nota - os listagens originais usam strings de notas extremamente densas e concatenadas (uma pratica normal de otimizacao de espaco em programas MSX de epoca, mas muito arriscada de transcrever a mao: um unico caractere trocado quebra a musica sem gerar nenhum erro de sintaxe detectavel). Quem precisar do listing exato deve consultar docs/manual_msx2fm_acvs.pdf diretamente.

UNCHAINED MELODY (trilha do filme "GHOST", adaptada e mixada por Flavio Monaco, 1991): usa 4 canais de FM, sendo 2 concatenados (para efeito de reverberacao), mais a bateria eletronica acionada. Os instrumentos usados sao FLAUTA, PIANO, VIOLINO e BAIXO. O programa desenha uma moldura decorativa em modo texto com creditos, depois toca a musica usando series de strings (uma por instrumento/trecho) concatenadas em sequencia de PLAY#2,... com parametros de acompanhamento de piano, flauta solo, violino e baixo tocados simultaneamente em canais diferentes.

THEME FROM OVER THE NET (tema do jogo original do AMIGA 500, adaptado por Flavio Monaco, 1991): usa 2 canais concatenados (6 vozes) com um unico instrumento, o CLAVICOD (que tem som semelhante ao de uma guitarra de heavy metal), mais a bateria totalmente reprogramada via CALL AUDREG, usando tambem o PSG (canais do som original do MSX) como reforco. E uma musica retirada de um jogo, com bastante uso de efeitos de eco (2 canais tocando a mesma nota defasada) e transposicao (CALL TRANSPOSE) no trecho final para um efeito de saida (fade/pitch bend) antes do END do programa.

FONTE
  Manual MSX 2+ FM (ACVS), pagina 53.)MAN");

    return v;
}

} // namespace man
