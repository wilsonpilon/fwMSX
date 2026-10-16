// fwMSX -- MAN: MSX-DOS 2, MSX-DOS 1/CP/M, Z80, R800, FM-PAC, Turbo-Basic e referencias do MSX-BASIC em ingles. ARQUIVO GERADO por
// tools/paleobasic/import_manuals_help.py a partir de resource/paleobasic/src/editor/help/Msx{Software,Manuals}HelpData.pbi -- nao editar a mao.
//
// Origem: SOFTWARE.CHM e MANUALS.CHM do RuMSX (Lex Lechz, lexlechz.at), que reunem material publico (boa parte digitada por Konamiman a partir dos
// manuais oficiais: MSX-DOS 2, MSX2 Technical Handbook, Turbo-Basic Compiler). Texto em ingles, como no original. Ver doc/man-spec.md.
#include "man_internal.h"

namespace man {

std::vector<Topic> ManualTopics() {
    std::vector<Topic> v;
    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {
        v.push_back(Topic{name, aliases, category, usage, summary, page, false});
    };

    add("abs", "", "MSX-BASIC", "ABS(<number>)", "Generates the absolute-value (unsigned) of the specified number.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ABS(<number>)

FUNCAO
Generates the absolute-value (unsigned) of the specified number.

EXEMPLO
PRINT ABS(-10)
 10
Ok

VEJA TAMBEM
SGN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("and", "", "MSX-BASIC", "<number1> AND <number2>", "Performs a bitwise AND-operation:",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
<number1> AND <number2>

FUNCAO
Performs a bitwise AND-operation:
0 AND 0 | = | 0
0 AND 1 | = | 0
1 AND 0 | = | 0
1 AND 1 | = | 1

The AND-operation is often used to "mask" one or more bits.

EXEMPLO
PRINT 15 AND 11       1111B
 11                   1011B AND
Ok                    -----
                      1011B

PRINT 6 AND 13        0110B
 4                    1101B AND
Ok                    -----
                      0100B

VEJA TAMBEM
EQV,
IMP,
MOD,
NOT,
OR,
XOR,
MSX-Basic operators

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("asc", "", "MSX-BASIC", "ASC(<string>)", "Returns the ASCII-code of the 1st character in string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ASC(<string>)

FUNCAO
Returns the ASCII-code of the 1st character in string.

EXEMPLO
PRINT ASC("EMIL")
 69
Ok

VEJA TAMBEM
CHR$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("atn", "", "MSX-BASIC", "ATN(<number>)", "Returns the arc-tangent of the specified number in radians.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ATN(<number>)

FUNCAO
Returns the arc-tangent of the specified number in radians.

EXEMPLO
PRINT ATN(100)
 1.5607966601082
Ok

VEJA TAMBEM
COS,
SIN,
TAN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("auto", "", "MSX-BASIC", "AUTO [<linenumber>[,<increment>]]", "Generates and increments line-numbers automatically.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
AUTO [<linenumber>[,<increment>]]

FUNCAO
Generates and increments line-numbers automatically.

EXEMPLO
AUTO 100,20

The line-numbering begins at 100 and is subsequently incremented by 20.
You can stop this operation mode by pressing Ctr+C.

A '*' (asterisk) character indicates that the linenumber is already in use. I.e. if

  120*

is displayed, this line already exists in the program.

VEJA TAMBEM
RENUM

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("base", "", "MSX-BASIC", "BASE(<number>)", "Returns the base-address of a video-processor register. Where number is a number between...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
BASE(<number>)

FUNCAO
Returns the base-address of a video-processor register. Where number is a number between 0 and 12 and has following meaning:

(*) = Added for MSX2

Mode-name: | Screen-mode: | Table: | Name:
Text 1 | Screen 0, width 40 | pt | pattern-table
*Text 2 | Screen 0, width 80 | spt | sprite-pattern-table
Mutli-color | Screen 3 | spat | sprite-attribute-table
Graphics 1 | Screen 1 | ct | color-table
Graphics 2 | Screen 2 | nt | name-table
*Graphics 3 | Screen 4
*Graphics 4 | Screen 5
*Graphics 5 | Screen 6
*Graphics 6 | Screen 7
*Graphics 7 | Screen 8

Base: | Table: | Mode-name: | Init:
0 | nt | Text 1 | $0000
2 | pt | Text 1 | $0800
5 | nt | Text 2 | $1800
6 | ct | Text 2 | $2000
7 | pt | Text 2 | $0000
10 | nt | Graphics 1 | $1800
11 | ct | Graphics 1 | $2000
12 | pt | Graphics 1 | $0000
13 | spat | Graphics 1 | $1B00
14 | spt | Graphics 1 | $3800
15 | nt | Graphics 2 | $0800
16 | pt | Graphics 2 | $0000
18 | spat | Graphics 2 | $1B00
19 | spt | Graphics 2 | $3800
20 | nt | Graphics 3 | $1800
21 | ct | Graphics 3 | $2000
22 | pt | Graphics 3 | $0000
23 | spat | Graphics 3 | $1E00
24 | spt | Graphics 3 | $3800
25 | nt | Graphics 4 | $0000
28 | spat | Graphics 4 | $7600
29 | spt | Graphics 4 | $7800
30 | nt | Graphics 5 | $0000
33 | spat | Graphics 5 | $7600
34 | spt | Graphics 5 | $7800
35 | nt | Graphics 6 | $0000
38 | spat | Graphics 6 | $FA00
39 | spt | Graphics 6 | $F000
40 | nt | Graphics 7 | $0000
43 | spat | Graphics 7 | $FA00
44 | spt | Graphics 7 | $F000

EXEMPLO
PRINT BASE(2)
 2048
Ok

VEJA TAMBEM
VDP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("beep", "", "MSX-BASIC", "BEEP", "Generates a short beep. On MSX1-computers this is 800Hz, 0.25s. PRINT CHR$(7) also...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
BEEP

FUNCAO
Generates a short beep. On MSX1-computers this is 800Hz, 0.25s. PRINT CHR$(7) also generates a BEEP.

EXEMPLO
BEEP

VEJA TAMBEM
PLAY,
SET BEEP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("bin$", "", "MSX-BASIC", "BIN$(<number>)", "Returns a string which contains the binary representation of number.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
BIN$(<number>)

FUNCAO
Returns a string which contains the binary representation of number.

EXEMPLO
PRINT BIN$(100)
 1100100
Ok

VEJA TAMBEM
HEX$,
OCT$,
STR$,
VAL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("bload", "", "MSX-BASIC", "BLOAD \"<name>\"[,R[,<offset>]]", "Binary LOAD:",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
BLOAD "<name>"[,R[,<offset>]]

BLOAD "<name>"[,S]

FUNCAO
Binary LOAD:

The binary file (often a assembly language program) name will be loaded at the beginaddress that was specified during BSAVE. If the option ',R' is added, the program will be started automatically. If there was no startadress  specified for BSAVE, the program begins it's execution on beginadress.

If an offset is specified the program will be loaded at beginaddres+offset.

If 'S' is specified this contents are loaded to VRAM from disk files. This can done in any screen mode, note, however, that
only the active pages are valid when the screen mode is from 5 to 8. No
cassette tapes can be used.

Turbo-R: the "CAS:" device is no longer supported!

EXEMPLO
BLOAD "TEST.BIN",R

VEJA TAMBEM
LOAD,
CLOAD,
DEFUSR,
SET PAGE,
BSAVE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("bsave", "", "MSX-BASIC", "BSAVE \"<name>\"[,<beginadress>,<endadress>[,<startadress>][,2]]", "Saves the memory-contents from beginadress to endadress with name on the device. If a...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
BSAVE "<name>"[,<beginadress>,<endadress>[,<startadress>][,2]]

BSAVE "<name>"[,<beginadress>,<endadress>][,S]

FUNCAO
Saves the memory-contents from beginadress to endadress with name on the device. If a startadress is specified the machine-code program will be started on this address if it is loaded using BLOAD with the ',R' option. When no startaddress is specified the beginaddress will be used as startaddess.

If the tape-device is used the ',2' option causes the recording speed to be set to 2400 baud in stead of the normal 1200 baud.

If 'S' is specified VRAM contents are saved to disk. This can be done for any screen mode, note, however, that
only the active pages are valid when the screen mode is from 5 to 8. No
cassette tapes can be used. Valid value range of address is from -32768 to
-2, or from 0 to 65534 (&HFFFE).

Turbo-R: the "CAS:" device is no longer supported!

EXEMPLO
BSAVE "TEST.BIN",&H9000,&H92FF

VEJA TAMBEM
SAVE,
CSAVE,
SET PAGE,
BLOAD

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call", "", "MSX-BASIC", "CALL <extended statement name>[(<arguments>)]", "Executes a extended BASIC-command stored in an extension ROM.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CALL <extended statement name>[(<arguments>)]

_ <extended statement name>[(<arguments>)]

FUNCAO
Executes a extended BASIC-command stored in an extension ROM.
Instead of the keyword CALL you can also use a _ (underscore).

EXEMPLOS
CALL SYSTEM
CALL KANJI
CALL ANK
CALL TURBO ON
CALL TURBO OFF
CALL PAUSE(100)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cdbl", "", "MSX-BASIC", "CDBL(<number>)", "Converts number into a double-precision value.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CDBL(<number>)

FUNCAO
Converts number into a double-precision value.

VEJA TAMBEM
CINT,
CSNG

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("chr$", "", "MSX-BASIC", "CHR$(<number>)", "Returns the character with the ASCII-code number. number must be an integer between 0 and...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CHR$(<number>)

FUNCAO
Returns the character with the ASCII-code number. number must be an integer between 0 and 255.

EXEMPLO
10 FOR I=0 TO 255
20 PRINT CHR$(I);
30 NEXT I

VEJA TAMBEM
ASC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cint", "", "MSX-BASIC", "CINT(<number>)", "Converts number into an integer value between -32768 and 32767.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CINT(<number>)

FUNCAO
Converts number into an integer value between -32768 and 32767.
Digits behind the decimal-point are ignored.
In other BASIC-dialects the value is rounded up or down.

EXEMPLO
PRINT CINT(12.34)
 12
Ok
PRINT CINT(123.54)
 123
Ok

VEJA TAMBEM
CDBL,
CSNG

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("circle", "", "MSX-BASIC", "CIRCLE [STEP](<x,y>),<radius> [,<color> [,<start>,<end> [,<proportion>]]]", "This command draws an ellipse on the screen.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CIRCLE [STEP](<x,y>),<radius> [,<color> [,<start>,<end> [,<proportion>]]]

FUNCAO
This command draws an ellipse on the screen.

x, y are coordinates of the center,

radius is the radius of the biggest main-axis

These values must be specified. Optional arguments are the

color of the line (see color-table)

start and end of the line (multiple of PI) and

proportion the aspect-ratio* of the x- and y-axis

EXEMPLOS
10 SCREEN 2
20 CIRCLE(100,100),40,1
30 GOTO 30

Draws an ellipse in the center of the screen. With

20 CIRCLE(100,100),40,,,1.4

you'll draw a circle

*note: On a real msx the results of "aspect-ratio" will be different depending on
your screens refresh-frequency, this example assumes 50 Hz (PAL) but when in 60 Hz (NTSC)
specifing no aspect-ratio will result in a circle.

VEJA TAMBEM
DRAW,
LINE,
PSET,
PAINT,
SCREEN,
CLS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("clear", "", "MSX-BASIC", "CLEAR [<stringsize> [,<adress>]]", "This command clears the contents of all variables and all dimensions of arrays.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CLEAR [<stringsize> [,<adress>]]

FUNCAO
This command clears the contents of all variables and all dimensions of arrays.
The optional stringsize reserves additional memory on top of memory.
Initially there are 200 Bytes reserved - for use in character-strings.

If adress is specified, this is the maximum address of BASIC-memory.
This command can also be used if arrays must be dimensioned dynamically.

EXEMPLO
10 CLEAR
20 INPUT "N=";N
30 DIMA(N)
40 GOTO 10

VEJA TAMBEM
ERASE,
DIM,
FRE,
MAXFILES,
DEF FN

Note: do not confuse with CLS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cload", "", "MSX-BASIC", "CLOAD \"<name>\"", "Cassette LOAD:",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CLOAD "<name>"

FUNCAO
Cassette LOAD:

Loads a basic program from tape. If no name is specified the first available basic-program will be loaded.

Turbo-R: CLOAD is no longer supported and will generate a "syntax error"

EXEMPLO
CLOAD
Found: TEST.BAS
Ok

VEJA TAMBEM
CSAVE,
BLOAD,
RUN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cload?", "", "MSX-BASIC", "CLOAD? \"<name>\"", "Compares a program on tape with the one in memory.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CLOAD? "<name>"

FUNCAO
Compares a program on tape with the one in memory.

If the program in memory differs from the program on tape a "Verify error" will occur.

Turbo-R: CLOAD? is no longer supported and will generate a "syntax error"

EXEMPLO
CLOAD?

VEJA TAMBEM
CSAVE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("close", "", "MSX-BASIC", "CLOSE [#][<filenumber>]", "Closes a file, if no filenumber is specified all open files will be closed.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic, RS232-Basic)

SINTAXE
CLOSE [#][<filenumber>]

FUNCAO
Closes a file, if no filenumber is specified all open files will be closed.

RS232: If the channel was opened in output mode, the EOF character is sent.
Note that the file buffer allocated by BASIC following the FCB is not actually used.

EXEMPLO
10 MAXFILES=1
20 OPEN "TEST" FOR OUTPUT AS#1
30 A$="MSX"
40 PRINT #1,A$
50 CLOSE #1
60 END

VEJA TAMBEM
OPEN, EOF, LOC, LOF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cls", "", "MSX-BASIC", "CLS", "Clears the screen (as well in text as in graphics modes).",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CLS

FUNCAO
Clears the screen (as well in text as in graphics modes).

EXEMPLO
-

VEJA TAMBEM
CALL CLS,
PRINT,
CIRCLE,
DRAW,
LINE,
PSET,
PAINT,
SCREEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("color", "", "MSX-BASIC", "COLOR [<foreground>][,<background>][,<border>]", "COLOR modifies the color of the foreground (active drawing), background and border.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
COLOR [<foreground>][,<background>][,<border>]

FUNCAO
COLOR modifies the color of the foreground (active drawing), background and border.
Possible values and differences in various screen-modes can be found in the color-table.

EXEMPLO
1000 COLOR 15,1,1:SCREEN 2
1010 OPEN "GRP:" FOR OUTPUT AS #1
1020 PRESET (10,5)
1030 PRINT #1, "A WELL TUNED TV-SET"
1040 PRESET (10,15)
1050 PRINT #1, "SHOWS 15 COLORS AND BLACK"
1060 CLOSE #1
1070 FOR G=2 TO 15
1080 COLOR G
1090 LINE (G*16-15,30)-(G*16,184),,BF
1100 NEXT G
1110 GOTO 1110

VEJA TAMBEM
COLOR SPRITE
COLOR SPRITE$
COLOR=,
COLOR=RESTORE,
COLOR=NEW,
DRAW,
LINE,
CIRCLE,
PSET,
PAINT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cont", "", "MSX-BASIC", "CONT", "CONT continues a program, which was halted with CTRL/STOP or the STOP or END instruction.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CONT

FUNCAO
CONT continues a program, which was halted with CTRL/STOP or the STOP or END instruction.

EXEMPLO
10 PRINT "Test line 1"
20 END
30 PRINT "Test line 2"

RUN
Test line 1
Ok
CONT
Test line 2
Ok

VEJA TAMBEM
RUN,
END,
STOP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cos", "", "MSX-BASIC", "COS(<number>)", "Calculates the cosine of the specified number.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
COS(<number>)

FUNCAO
Calculates the cosine of the specified number.

EXEMPLO
PRINT COS(100)
 .86231887228917

VEJA TAMBEM
ATN,
SIN,
TAN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("csave", "", "MSX-BASIC", "CSAVE \"<name>\" [,2]", "Cassette Save, stores a basic program on tape.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CSAVE "<name>" [,2]

FUNCAO
Cassette Save, stores a basic program on tape.

If the ',2' option is added the recording speed is set to 2400 baud in stead of the normal 1200 baud.

Turbo-R: CSAVE is no longer supported and will generate a "syntax error"

EXEMPLO
CSAVE "TEST",2

VEJA TAMBEM
CLOAD?, CLOAD, SAVE, BSAVE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("csng", "", "MSX-BASIC", "CSNG(<number>)", "Converts number into a single-precision value.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CSNG(<number>)

FUNCAO
Converts number into a single-precision value.

EXEMPLO
-

VEJA TAMBEM
CDBL,
CINT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("csrlin", "", "MSX-BASIC", "CSRLIN", "Returns the current Y-position of the cursor.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
CSRLIN

FUNCAO
Returns the current Y-position of the cursor.

EXEMPLO
LOCATE 0,5:PRINT CSRLIN
 5
Ok

VEJA TAMBEM
LOCATE,
POS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("data", "", "MSX-BASIC", "DATA <value>[,<value2> [,...]]", "With this instruction a set of elements can be added a program.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DATA <value>[,<value2> [,...]]

FUNCAO
With this instruction a set of elements can be added a program.

These elements can be either numbers or strings, they can be accessed
one by one using the READ instruction and are always separated by
comma's. Strings are also placed between quotation marks.

EXEMPLO
10 READ A$
20 PRINT A$
30 READ A,B
40 PRINT A
50 PRINT B
60 DATA "TEST",10,20

VEJA TAMBEM
READ,
RESTORE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("def fn", "", "MSX-BASIC", "DEF FN <functionname>[(<arguments>)]=<function definition>", "With this instruction one can create his own functions.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DEF FN <functionname>[(<arguments>)]=<function definition>

FUNCAO
With this instruction one can create his own functions.

EXEMPLO
10 DEF FN TEST$(E)="And the character is: "+CHR$(E)
20 PRINT FN TEST$(65)
RUN
"And the character is: A"

VEJA TAMBEM
CLEAR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("defusr", "", "MSX-BASIC", "DEFUSR[<number>] = <address>", "Specifies the starting address of a machine-language program,",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DEFUSR[<number>] = <address>

FUNCAO
Specifies the starting address of a machine-language program,
the program can be executed by using the USR function.
number can be 0-9, if no number is specified 0 is assumed.

EXEMPLO
DEF USR1=&H156:A=USR1(0)

VEJA TAMBEM
USR,
BLOAD,
POKE,
VARPTR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("defdbl", "", "MSX-BASIC", "DEFDBL <letter>[-<lastletter>] [<letter2> [,-<lastletter2>]]", "Defines all variables starting with the specified letter(s) as double-precision variables.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DEFDBL <letter>[-<lastletter>] [<letter2> [,-<lastletter2>]]

FUNCAO
Defines all variables starting with the specified letter(s) as double-precision variables.

EXEMPLO
DEF DBL A-E

VEJA TAMBEM
DEFINT,
DEFSNG,
DEFSTR,
CLEAR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("defint", "", "MSX-BASIC", "DEFINT <letter>[-<lastletter>] [<letter2> [,-<lastletter2>]]", "Defines all variables starting with the specified letter(s) as integer variables.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DEFINT <letter>[-<lastletter>] [<letter2> [,-<lastletter2>]]

FUNCAO
Defines all variables starting with the specified letter(s) as integer variables.

EXEMPLO
DEF INT G

VEJA TAMBEM
DEFDBL,
DEFSNG,
DEFSTR,
CLEAR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("defsng", "", "MSX-BASIC", "DEFSNG <letter>[-<lastletter>] [<letter2> [,-<lastletter2>]]", "Defines all variables starting with the specified letter(s) as single-precision variables.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DEFSNG <letter>[-<lastletter>] [<letter2> [,-<lastletter2>]]

FUNCAO
Defines all variables starting with the specified letter(s) as single-precision variables.

EXEMPLO
DEF SGN A

VEJA TAMBEM
DEFDBL,
DEFINT,
DEFSTR,
CLEAR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("defstr", "", "MSX-BASIC", "DEFSTR <letter>[-<lastletter>] [<letter2> [,-<lastletter2>]]", "Defines all variables starting with the specified letter(s) as string variables.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DEFSTR <letter>[-<lastletter>] [<letter2> [,-<lastletter2>]]

FUNCAO
Defines all variables starting with the specified letter(s) as string variables.

EXEMPLO
DEF STR A

VEJA TAMBEM
DEFDBL,
DEFINT,
DEFSNG,
CLEAR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("delete", "", "MSX-BASIC", "DELETE <line> [-<line2>]", "Deletes program-lines, if this command is executed",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DELETE <line> [-<line2>]

FUNCAO
Deletes program-lines, if this command is executed
within a program the program will terminate after this command.

EXEMPLO
DELETE 10-40

VEJA TAMBEM
MERGE,
RENUM

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dim", "", "MSX-BASIC", "DIM <array-name>(<maximun index>)[,<array-name>...]", "Reserves memory for the specified variables and intitializes the array.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DIM <array-name>(<maximun index>)[,<array-name>...]

FUNCAO
Reserves memory for the specified variables and intitializes the array.

If if an array is referred to when not first created with a DIM instruction,
a maximum index of 10 is assumed. The first index is always 0.

EXEMPLO
10 DIM M(25)
20 FOR A=0 TO 25:M(A)=3*A:NEXT
30 FOR A=0 TO 25
40 PRINT "3 times ";A;" is ";M(A)
50 NEXT A

VEJA TAMBEM
CLEAR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("draw", "", "MSX-BASIC", "DRAW \"<string>\"", "With this function strait lines can easily be drawn by using simple codes.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
DRAW "<string>"

FUNCAO
With this function strait lines can easily be drawn by using simple codes.

The following codes can be used:

Code | Function
S | Scale
Ax | Rotates the coordinate system in steps of 90 degrees. x can be 0-3.
Cxx | Active drawing color, xx can be 0-15.
Mqq,zz | Move, draws a line from the last known coordinates (x,y) to (x+qq,y+zz) qq en zz can also be negative.
BMqq,zz | Blank move, moves the (x,y) coordinates without ectually drawing a line.
Uxx | Up, draws a line xx pixels up.
Dxx | Down, draws a line xx pixels down
Rxx | Right, draws a line xx pixels to the right.
Lxx | Left, draws a line xx pixels to the left.
Exx | Draws a line xx pixels up-right.
Fxx | Draws a line xx pixels down-right.
Gxx | Draws a line xx pixels up-left.
Hxx | Draws a line xx pixels down-left.

Variables can also be used within draw-instructions, the '=' symbol can be used to allow interger-variables. If a string-variable is used a X should be prefixed and all variables should be closed by a ';'.

EXEMPLO
10 SCREEN 5
20 A$="C15BM50,50"
30 S=50
40 DRAW "XA$;D=B;R=B;U=B;L=B;"
50 A$=INPUT$(1)

VEJA TAMBEM
CIRCLE,
LINE,
PSET,
PAINT,
SCREEN,
CLS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("end", "", "MSX-BASIC", "END", "Terminates a programs execution. When the interpreter reaches the last line of the...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
END

FUNCAO
Terminates a programs execution. When the interpreter reaches the last line of the program,
it is automaticly terminated, END is not necessary there.
After END a CONT instruction can continue the program execution on the next line.

EXEMPLO
10 FOR A=1 TO 10
20 PRINT A
20 IF A=5 THEN END
30 NEXT

VEJA TAMBEM
STOP,
RUN,
CONT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("eof", "", "MSX-BASIC", "EOF(<filenumber>)", "EOF is zero while the end of the open file in not reached.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, RS232-Basic)

SINTAXE
EOF(<filenumber>)

FUNCAO
EOF is zero while the end of the open file in not reached.

EXEMPLO
10 IF EOF(2) THEN CLOSE#2

VEJA TAMBEM
OPEN,
CLOSE,
LOC,
LOF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("eqv", "", "MSX-BASIC", "number1 EQV number2", "Logical equivalence.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
number1 EQV number2

FUNCAO
Logical equivalence.
0 EQV 0 | = | 1
0 EQV 1 | = | 0
1 EQV 0 | = | 0
1 EQV 1 | = | 1

VEJA TAMBEM
AND,
IMP,
MOD,
NOT,
OR,
XOR,
MSX-Basic operators

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("erase", "", "MSX-BASIC", "ERASE <array-name>[,<array-name>]", "ERASE frees up the memory used by array's previously allocated by a DIM instruction.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ERASE <array-name>[,<array-name>]

FUNCAO
ERASE frees up the memory used by array's previously allocated by a DIM instruction.

EXEMPLO
10 DIM A(100),X$(60)
...
500 ERASE A,X$

VEJA TAMBEM
CLEAR,
DIM

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("erl", "", "MSX-BASIC", "ERL", "Returns the line-number where the last error occured.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ERL

FUNCAO
Returns the line-number where the last error occured.

EXEMPLO
See ERR

VEJA TAMBEM
ERR,
ERROR,
ON ERROR GOTO,
RESUME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("err", "", "MSX-BASIC", "ERR", "Returns the number of the last error that occurred. The error-number can be found in the...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
ERR

FUNCAO
Returns the number of the last error that occurred. The error-number can be found in the error-table.

EXEMPLO
10 ON ERROR GOTO 50
20 A=25: PRINT A
30 B=A/0
40 END
50 PRINT "Error number ";ERR;" occured in line ";ERL
60 RESUME NEXT

VEJA TAMBEM
ERL,
ERROR,
ON ERROR GOTO,
RESUME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("error", "", "MSX-BASIC", "ERROR <number>", "Displays the error-message that belongs to the error-number.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
ERROR <number>

FUNCAO
Displays the error-message that belongs to the error-number.
The error-number can be found in the error-table.

EXEMPLO
ERROR 64
File still open
Ok

VEJA TAMBEM
ERR,
ERL,
ON ERROR GOTO,
RESUME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("exp", "", "MSX-BASIC", "EXP(<number>)", "Calculates the power of e for number (base is e=2.718281884588).",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
EXP(<number>)

FUNCAO
Calculates the power of e for number (base is e=2.718281884588).

number must be less or equal 145.06286058562.

EXEMPLO
PRINT EXP(10)
 22026.4657948
Ok

VEJA TAMBEM
LOG

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("fix", "", "MSX-BASIC", "FIX(<number>)", "Return the integer part of number. This function is identical to",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
FIX(<number>)

FUNCAO
Return the integer part of number. This function is identical to

SGN(<number>)*INT(ABS(<number>)).

The difference between "FIX" and "INT" is that negative values are not rounded to the
next lower value.

EXEMPLO
PRINT FIX(1.7)
 1
Ok

VEJA TAMBEM
INT,
SGN,
ABS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("for", "", "MSX-BASIC", "FOR <variable>=<number> TO <number2> [STEP <number3>]", "This instruction forms a program loop that looks like:",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
FOR <variable>=<number> TO <number2> [STEP <number3>]

NEXT [<variable> [,<variable2>][, ...]]]

FUNCAO
This instruction forms a program loop that looks like:

FOR <variable>=X TO Y [STEP Z]

.

.   Instructions

.

NEXT <variable>

The instructions between FOR and NEXT are executed with values from
X to Y. If STEP Z is not specified a step-width
of 1 is used. Within a loop another loop can be implemented,
so loops can be nested.

Every loop will be executed at least once because the termination-condition
is checked in the NEXT statement.

EXEMPLO
10 FOR I=1 TO 4
20 PRINT I;
30 NEXT:PRINT
40 FOR I=0 TO 5 STEP 5
50 FOR J=0 TO I+2
60 PRINT J
70 NEXT J,I:PRINT
80 END

VEJA TAMBEM
CALL PAUSE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("fre", "", "MSX-BASIC", "FRE(number)", "FRE(number) returns the free memory available to Basic programs, where",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
FRE(number)

FRE(string)

FUNCAO
FRE(number) returns the free memory available to Basic programs, where
FRE(string) returns the free memory for strings.

EXEMPLO
NEW
PRINT FRE(0)
28815
Ok
PRINT FRE("")
200
Ok

VEJA TAMBEM
CLEAR,
ERASE,
DIM,
MAXFILES

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("gosub", "", "MSX-BASIC", "GOSUB line", "This statement jumps to a subroutine.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
GOSUB line

FUNCAO
This statement jumps to a subroutine.
The program can return from this routine by using the RETURN statement.

EXEMPLO
100 PRINT "Hello"
110 GOSUB 200
120 GOSUB 300
130 END
200 PRINT "Here am I"
210 RETURN
300 PRINT "End"
310 RETURN

Outputs the following text:

Hello
Here am I
End

VEJA TAMBEM
RETURN,
ON GOSUB,
GOTO

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("goto", "", "MSX-BASIC", "GOTO line", "Unconditional jump-statement. The program continues at the specified line.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
GOTO line

GO TO line

FUNCAO
Unconditional jump-statement. The program continues at the specified line.

EXEMPLO
-

VEJA TAMBEM
ON GOTO,
GOSUB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("hex$", "", "MSX-BASIC", "HEX$(number)", "Converts the specified number into a hexadecimal value.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
HEX$(number)

FUNCAO
Converts the specified number into a hexadecimal value.
A hexadecimal-number consists of digits 0 to 9 and the characters A to F.

EXEMPLO
PRINT HEX$(1023)
3FF
Ok

VEJA TAMBEM
BIN$,
OCT$,
STR$,
VAL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("if", "", "MSX-BASIC", "IF expression THEN <StatementsA> [ELSE <StatementsB>]", "If the condition is met StatementsA are executed.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
IF expression THEN <StatementsA> [ELSE <StatementsB>]

FUNCAO
If the condition is met StatementsA are executed.
After this the program continues with the next program line.

If the condition is not met then StatementsB after the ELSE
are executed. After this the program continues with the next program line too.

EXEMPLO
100 INPUT "A=";A
110 IF A>=0 THEN PRINT "positive" ELSE PRINT "negative"
120 INPUT "Again ( /N)";A$
130 IF A$<>"N" GOTO 100 ELSE END

Is the condition A>=0 in line 110 met then "positive" is printed.
As line 130 indicates the THEN can be replaced by GOTO.
On the other hand the GOTO after THEN is not necessary.
Line 130 can also look like:

130 IF A$<>"N" THEN 100 ELSE END

VEJA TAMBEM
GOTO

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("imp", "", "MSX-BASIC", "number1 IMP number2", "Logical implication.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
number1 IMP number2

FUNCAO
Logical implication.
0 IMP 0 | = | 1
0 IMP 1 | = | 1
1 IMP 0 | = | 0
1 IMP 1 | = | 1

VEJA TAMBEM
AND,
EQV,
MOD,
NOT,
OR,
XOR,
MSX-Basic operators

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("inkey$", "", "MSX-BASIC", "INKEY$", "The function INKEY$ scans the keyboard for a pushed key.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
INKEY$

FUNCAO
The function INKEY$ scans the keyboard for a pushed key.
The returned string is empty if no key is pressed or
the ASCII character of the pressed key.

EXEMPLO
10 PRINT "Please press any key"
20 IF INKEY$="" THEN 20
30 PRINT "Continuing..."

If the key-input should result in a more detailed evaluation
it should be stored in a string-variable.

10 PRINT "Please press any key"
20 A$=INKEY$:IF A$="" THEN 20
30 IF ASC(A$)=13 THEN 10

If ENTER/RETURN is pressed in line 20 the program restarts in line 10.

VEJA TAMBEM
INPUT$,
STICK,
STRIG

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("inp", "", "MSX-BASIC", "INP(port)", "Reads the content of the specified input-port (0-255).",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
INP(port)

FUNCAO
Reads the content of the specified input-port (0-255).

The statement "OUT" can be used to write a byte to an output-port.

EXEMPLO
10 A=INP(&HA8)
20 A$="00000000"+BIN$(A)
30 PRINT RIGHT$(A$,8)
40 END
RUN

VEJA TAMBEM
OUT,
WAIT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("input", "", "MSX-BASIC", "INPUT [\"prompt\";] variable [,variable2 [,...]]", "Retrieves a number or string from the keyboard and assignes it to a variable.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
INPUT ["prompt";] variable [,variable2 [,...]]

INPUT#channel, variable [,variable2 [,...]]

FUNCAO
Retrieves a number or string from the keyboard and assignes it to a variable.
The input is finished with the Return/Enter key.
A prompt can be printed before the input starts.
A "?" sign is also printed to indicate the input.

The INPUT# statement reads data from a sequential file opened as #channel
and assigns it to the listed variables.

Special: If a string already has a value and the new input is empty
(only Return/Enter pushed) then the old value remains unchanged.
In other Basic dialects the value will be cleared. This is also
valid for the input of numbers (not only strings).

EXEMPLO
10 INPUT "Length of the page";L

VEJA TAMBEM
LINE INPUT,
GET,
INPUT$,
PRINT,
OPEN,
PRINT[#]

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("input$", "", "MSX-BASIC", "INPUT$(number [#channel])", "This function reads number characters from the keyboard",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
INPUT$(number [#channel])

FUNCAO
This function reads number characters from the keyboard
or a file opened as #channel.

The INPUT$ function can also recognize control-sequences,
which cannot be entered using the INPUT statement. Therefore
INPUT$ is useful for data-transfer from other machines.
In such transmissions all characters might be important.

EXEMPLO
10 PRINT "C continues, E ends the program"
20 A$=INPUT$(1)
30 IF A$="E" THEN END
40 IF A$="C" THEN GOTO 60
50 GOTO 10
60 REM code continues here

VEJA TAMBEM
INKEY$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("instr", "", "MSX-BASIC", "INSTR([position,] string, substring)", "This function looks for the 1st occurence of the substring in the specified string",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
INSTR([position,] string, substring)

FUNCAO
This function looks for the 1st occurence of the substring in the specified string
and returns the start-position. In addition a position (0-255) can be specified,
which indicates where the search should start.

If the substring is not found, or position is larger as the length of the string
or the string is empty then INSTR returns 0. If substring is empty INSTR returns 1.

EXEMPLO
10 A$="Merry Christmas"
20 PRINT INSTR(A$, "ist")
30 PRINT INSTR(A$, "IST")
40 PRINT INSTR(5, A$, "r")
RUN
10
0
9

VEJA TAMBEM
LEN,
MID$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("int", "", "MSX-BASIC", "INT(<number>)", "Returns the integer-part of <number>.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
INT(<number>)

FUNCAO
Returns the integer-part of <number>.

EXEMPLO
10 A=-2.7
20 PRINT INT(A)
30 A=2.7
40 PRINT INT(A)
50 END
RUN
-2
2
Ok

VEJA TAMBEM
FIX,
SGN,
ABS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("interval", "", "MSX-BASIC", "INTERVAL ON", "After \"INTERVAL ON\" the program checks if a specified time-interval",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
INTERVAL ON

INTERVAL OFF

INTERVAL STOP

FUNCAO
After "INTERVAL ON" the program checks if a specified time-interval
has been elapsed. If so MSX-Basic calls the subroutine that was
specified with the "ON INTERVAL GOSUB" statement.
After the "INTERVAL OFF" statement no checking is performed. After
"INTERVAL STOP" MSX-Basic still checks if the interval has been
elapsed, but calls the subroutine only after the next call of
"INTERVAL ON".

EXEMPLO
See: ON INTERVAL GOSUB

VEJA TAMBEM
ON INTERVAL GOSUB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("key", "", "MSX-BASIC", "KEY ON", "\"KEY ON\" enables and \"KEY OFF\" disables the display of the function-key assignment in...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
KEY ON

KEY OFF

FUNCAO
"KEY ON" enables and "KEY OFF" disables the display of the function-key assignment in line 25
(only in textmodes 1 and 2).

EXEMPLO
KEY OFF

VEJA TAMBEM
KEY (assignment),
KEY LIST

REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
KEY number, string

FUNCAO
Associates the function key identified by number (1-10) with a string.
The string can have up to a maximum of 15 characters.

EXEMPLO
10 A$="CLOAD"
20 KEY 1,A$+CHR$(13)
30 END

VEJA TAMBEM
KEY LIST,
KEY ON/OFF

REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
KEY(number) ON

KEY(number) OFF

KEY(number) STOP

FUNCAO
number specifies a function-key number (1-10).

After the "KEY(number) ON" statement MSX-Basic checks if the specified
function-key was pressed. If so MSX-Basic calls the subroutine, that was
defined by the "ON KEY GOSUB" statement.

After the statement "KEY(number) STOP" MSX-Basic checks if the specified
function-key was pressed, but does not call the defined subroutine until the
next invocation "KEY(number) ON".

EXEMPLO
See: ON KEY GOSUB

VEJA TAMBEM
ON KEY GOSUB

REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
KEY LIST

FUNCAO
Lists the assignment of the function-keys (F1-F10).

EXEMPLO
KEY LIST
color
auto
goto
list
run
color 15,4,4
cload"
cont
list.

VEJA TAMBEM
KEY (assignment),
KEY ON/OFF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("left$", "", "MSX-BASIC", "LEFT$(string, number)", "Returns the first number (0-255) characters from the specified string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LEFT$(string, number)

FUNCAO
Returns the first number (0-255) characters from the specified string.
If number is larger than the string the whole string is returned.
If number is 0 an empty string is returned.

EXEMPLO
10 A$="MSX-Computer"
20 FOR I=1 TO LEN(A$)
30 PRINT LEFT$(A$,I)
40 NEXT I
RUN
M
MS
MSX
MSX-
MSX-C
MSX-Co
MSX-Com
MSX-Comp
MSX-Compu
MSX-Comput
MSX-Compute
MSX-Computer

VEJA TAMBEM
MID$,
RIGHT$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("len", "", "MSX-BASIC", "LEN(string)", "Returns the number of characters in the string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LEN(string)

FUNCAO
Returns the number of characters in the string.

EXEMPLO
PRINT LEN("BASIC")
5

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("let", "", "MSX-BASIC", "[LET] variable=value", "Assigns a value to the specified variable.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
[LET] variable=value

FUNCAO
Assigns a value to the specified variable.
The LET statement itself is optional and can be skipped.

EXEMPLO
LET A=10
LET B$="Hello"
C=20
D$="World"

VEJA TAMBEM
SWAP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("line", "", "MSX-BASIC", "LINE[[STEP](x1,y1)]-[STEP](x2,y2)[,color [,B[F]]]", "Draws a line from x1,y1 to x2,y2, which can be absolute or relative",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LINE[[STEP](x1,y1)]-[STEP](x2,y2)[,color [,B[F]]]

FUNCAO
Draws a line from x1,y1 to x2,y2, which can be absolute or relative
("STEP" keyword) coordinates.

If the word "STEP" is used x1 and y1 are interpreted relative to the
current cursor-position. In this case the values can also be negative.

x2 and y2 are the coordinates of the line-endpoint. If the word "STEP" is
specified these are also relative to the cursor-position.

color is the line-color to be used for drawing
(see color-table).
If color is not specified the current foreground-color is used.
If "B" is specified a rectangle is drawn, where the current line is used as diagonal.

If "F" after "B" is specified the rectangle is filled with the given color.

EXEMPLO
10 SCREEN 2
20 FOR I=0 TO 95 STEP 2
30 LINE (128-I,95-I)-(128+I,95+I),1,B
40 NEXT
50 FOR K=0 TO 2000:NEXT
60 END

VEJA TAMBEM
CIRCLE,
DRAW,
PSET,
PAINT,
SCREEN,
CLS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("line input", "", "MSX-BASIC", "LINE INPUT [\"prompt\";] stringvariable", "Retrieves a whole line, up to 254 characters, from the keyboard",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LINE INPUT ["prompt";] stringvariable

LINE INPUT #channel, stringvariable

FUNCAO
Retrieves a whole line, up to 254 characters, from the keyboard
into the specified stringvariable.
A given prompt (if any) is printed first.

The LINE INPUT# statement is used to read a complete record
from a sequential file (opened as #channel), which
must have been opened for "INPUT". The "LINE INPUT" statement
is often used to read a MSX-Basic program as datafile, if it
was saved (by using the "SAVE" statement) in ASCII format.

EXEMPLO
10 LINE INPUT "TEXT";A$
20 PRINT A$
30 END
RUN
TEXT
Ok

VEJA TAMBEM
INPUT,
GET,
PRINT,
OPEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("list", "", "MSX-BASIC", "LIST [[startline] [-[endline]]]", "Lists a whole or part of the Basic-program from memory.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LIST [[startline] [-[endline]]]

FUNCAO
Lists a whole or part of the Basic-program from memory.

If only startline is given this line is listed only.
If startline and endline are specified all lines within this range are listed.
If only endline is given all lines from the beginning to this line are listed.
If neither startline nor endline are given the whole program is listed.
If startline and "-" are given then this line and all followings are listed.

EXEMPLO
LIST 10

Program line 10 is listed only.

LIST 50-100

Program lines 50 to 100 are listed.

LIST -100

All program lines from 0 to 100 are listed.

LIST

All program lines are listed.

LIST .

The last maintained line is listed.

VEJA TAMBEM
LLIST

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("llist", "", "MSX-BASIC", "LLIST [[startline] [-[endline]]]", "A complete program or part of it is printed.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LLIST [[startline] [-[endline]]]

FUNCAO
A complete program or part of it is printed.
The statement works in the same way as "LIST".

EXEMPLO
See LIST

VEJA TAMBEM
LIST

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("load", "", "MSX-BASIC", "LOAD \"devicename:filename\" [,R]", "Loads a program from a device into the computer-memory.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
LOAD "devicename:filename" [,R]

FUNCAO
Loads a program from a device into the computer-memory.

Devicenames can be, e.g.: CAS:, A: or B:

The "SAVE" statement must have been used to store the program in ASCII format (on tape or disk).

The filename is an alphanumerical constant, that has been specified while the program was
stored using the "SAVE" statement.

Disk-Basic: if ",R" is specified the programm is started immediately,
after the load-operation is finished.

Turbo-R: the "CAS:" device is no longer supported!

EXEMPLO
LOAD "CAS:DEMO"

"LOAD CAS:" can only be used if a tape-recorder is connected to the MSX-Computer.

LOAD "A:DEMO"

"LOAD A:" or "LOAD B:" can only be used if a disk-drive is connected to the MSX-Computer.

VEJA TAMBEM
BLOAD,
CALL MFILES,
CLOAD,
FILES,
MERGE,
RUN,
SAVE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("locate", "", "MSX-BASIC", "LOCATE line, column [,cursoronoff]", "Places the cursor to the position specified by line and column,",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LOCATE line, column [,cursoronoff]

FUNCAO
Places the cursor to the position specified by line and column,
where 0,0 is the upper left corner.
Option cursoronoff toggles the cursor on/off. Usually the cursor is
only visible during input, but

  LOCATE,,1

can enable it at any time.

EXEMPLO
10 KEY OFF
20 LOCATE 1,23
30 PRINT "A"
40 LOCATE,,1
50 GOTO 50

VEJA TAMBEM
CRSLIN,
POS,
PRINT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("log", "", "MSX-BASIC", "LOG(number)", "Calculates the natural logarithm (base e=2.718281884588).",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LOG(number)

FUNCAO
Calculates the natural logarithm (base e=2.718281884588).

EXEMPLO
PRINT LOG(10)
 2.302585092994
PRINT LOG(100)/LOG(10)
 2

VEJA TAMBEM
EXP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("lpos", "", "MSX-BASIC", "LPOS(number)", "Returns the position in the printer-buffer. The result is not necessarily",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LPOS(number)

FUNCAO
Returns the position in the printer-buffer. The result is not necessarily
the physical position of the printhead.

EXEMPLO
10 FOR I=1 TO 1000
20 LPRINT "Hello";
30 IF LPOS(0)>38 THEN LPRINT CHR$(13)
40 NEXT
50 END

VEJA TAMBEM
LPRINT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("lprint", "", "MSX-BASIC", "LPRINT [USING \"format\";] [value [,value2 [,...]]] [;]", "Prints information on the printer. The statement works in the same way as \"PRINT\".",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
LPRINT [USING "format";] [value [,value2 [,...]]] [;]

FUNCAO
Prints information on the printer. The statement works in the same way as "PRINT".

EXEMPLO
See PRINT

VEJA TAMBEM
PRINT,
TAB,
SPC,
LPOS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("maxfiles", "", "MSX-BASIC", "MAXFILES=number", "Specifies the maximum number of files, that can be simultaneously open.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
MAXFILES=number

FUNCAO
Specifies the maximum number of files, that can be simultaneously open.
number can be in range from 0-15.

Technote: the MAXFILES statement is built by 2 basic tokens (MAX FILES)

EXEMPLO
10 MAXFILES=2
20 OPEN "CAS:DEMO" FOR INPUT AS #1
30 OPEN "LPT:" FOR OUTPUT AS #2
40 INPUT #1,A$
50 PRINT #2,A$
60 CLOSE
70 END

VEJA TAMBEM
CLEAR,
OPEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("merge", "", "MSX-BASIC", "MERGE \"[device:]filename\"", "Merges a program in memory with another from tape or disk.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
MERGE "[device:]filename"

FUNCAO
Merges a program in memory with another from tape or disk.
The lines to be merged must be saved in ASCII by using the "SAVE" statement.
If some new lines have the same numbers as in memory the
already existing lines will be replaced.

Turbo-R: the "CAS:" device is no longer supported!

EXEMPLO
MERGE "CAS:DEMO"

VEJA TAMBEM
LOAD,
SAVE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("mid$", "", "MSX-BASIC", "MID$( string, position [,characters])", "(1) An alphanumerical expression consisting of characters number of characters",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
MID$( string, position [,characters])

MID$( string, position [,characters])=string2

FUNCAO
(1) An alphanumerical expression consisting of characters number of characters
(or up to the string-end) beginning by position will be returned.

If characters is not specified or there are no more characters to the right
of position then all remaining characters is returned. If position
if larger than LEN(string) then an empty string is returned.

(2) Replaces parts of string by string2.
Characters in string, beginning on position are replaced by string2.
If characters is not specified all characters of string2 are used,
as long as enabled by the length of string.

Disk-Basic: in addition to LSET and RSET also MID$ can be used
to format strings, declared in the FIELD statement.

EXEMPLO
10 A$="Basic MSX Computer"
20 PRINT MID$(A$,7,3)
30 END
RUN
MSX
Ok

10 A$="ABCDEFG"
20 PRINT A$
30 MID$(A$,4)="XYZ"
40 PRINT A$
50 END
RUN
ABCDEFG
ABCXYZG
Ok

VEJA TAMBEM
LEFT$,
RIGHT$,
INSTR,
LSET,
RSET

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("mod", "", "MSX-BASIC", "number MOD number2", "Calculates the dividend of an integer division.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
number MOD number2

FUNCAO
Calculates the dividend of an integer division.

EXEMPLO
PRINT 5 MOD 3
 2
Ok

PRINT 5 MOD 5
 0
Ok

VEJA TAMBEM
AND,
EQV,
IMP,
NOT,
OR,
XOR,
MSX-Basic operators

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("motor", "", "MSX-BASIC", "MOTOR ON", "Turns the motor of the tape-recorder on or off.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
MOTOR ON

MOTOR OFF

FUNCAO
Turns the motor of the tape-recorder on or off.
If "ON" or "OFF" is specified the motor is toggled. This means that a
switched on motor is turned off and a switched off motor is turned on.

Setting "ON" turns the motor on, setting "OFF" turns the motor off.

Turbo-R: MOTOR is no longer supported and will generate a "syntax error"

EXEMPLO
10 FOR I=1 TO 10
20 MOTOR
30 FOR J=1 TO 500:NEXT J
40 NEXT I

VEJA TAMBEM
CLOAD,
CLOAD?,
CSAVE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("new", "", "MSX-BASIC", "NEW", "Erases the Basic program in memory.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
NEW

FUNCAO
Erases the Basic program in memory.

"NEW" is usually used to delete the computer memory
before starting with a new Basic program.

EXEMPLO
NEW

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("not", "", "MSX-BASIC", "NOT expression", "Logical complement.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
NOT expression

FUNCAO
Logical complement.
NOT 0 | = | 1
NOT 1 | = | 0

EXEMPLO
PRINT HEX$(NOT &H7FFF)
8000
Ok

100 INPUT "Y/N";A$
120 IF NOT A$="N" THEN PRINT "Yes" ELSE PRINT "No"
RUN
(Y/N)?N
No

VEJA TAMBEM
AND,
EQV,
IMP,
MOD,
OR,
XOR,
MSX-Basic operators

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("oct$", "", "MSX-BASIC", "OCT$(number)", "Converts the specified number into an octal value. An octal number consists of",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
OCT$(number)

FUNCAO
Converts the specified number into an octal value. An octal number consists of
the digits 0 to 7.

EXEMPLO
PRINT OCT$(1023)
1777
Ok

VEJA TAMBEM
BIN$,
HEX$,
STR$,
VAL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("on error", "", "MSX-BASIC", "ON ERROR GOTO line", "line is the 1st line of an error-handling routine.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ON ERROR GOTO line

FUNCAO
line is the 1st line of an error-handling routine.

If "ON ERROR GOTO" statement is set MSX-Basic jumps to the specified line
of the error-handler routine, whenever an error occurs.

"ERR" returns the corresponding errorcode and "ERL" returns the line-number
where the error occured.

If the statement "ON ERROR GOTO 0" is set MSX-Basic does not jump to a
specific line. In this case the normal error-handler takes place,
stops the program and prints the error-message. Its recommended to use
"ON ERROR GOTO 0" for errors, which are not handled by another error-handling
routine.

If MSX-Basic recognizes an error while the error-handling routine is
executed the common error is displayed.

The "RESUME" statement returns from the error-handling routine to the
program location where the error occured.

EXEMPLO
See ERR

VEJA TAMBEM
RESUME,
ERL,
ERROR,
ERR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("on gosub", "", "MSX-BASIC", "ON value GOSUB line [,line2 [,...]]", "line1 respectively line2 are the 1st program-line of a subroutine.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ON value GOSUB line [,line2 [,...]]

FUNCAO
line1 respectively line2 are the 1st program-line of a subroutine.

The value determines, which subroutine MSX-Basic executes.

If, for example value is 3 a jump to the 3rd subroutine is performed.

If value is no integer it will be rounded down.

The value cannot be greater than 255 or negative.

EXEMPLO
10 FOR I=1 TO 3
20 ON I GOSUB 50,70,90
30 NEXT I
40 END
50 PRINT "I=";I;"subroutine line 50"
60 RETURN
70 PRINT "I=";I;"subroutine line 70"
80 RETURN
90 PRINT "I=";I;"subroutine line 90"
100 RETURN
RUN
I= 1 subroutine line 50
I= 2 subroutine line 70
I= 3 subroutine line 90
Ok

VEJA TAMBEM
GOSUB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("on goto", "", "MSX-BASIC", "ON value GOTO line [,line2 [,...]]", "line1 and line2 are program line-numbers.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ON value GOTO line [,line2 [,...]]

FUNCAO
line1 and line2 are program line-numbers.

The value determines to which line number MSX-Basic jumps
and where the program continues.

If, for example value is 3 a jump to the 3rd routine is performed.

If value is no integer it will be rounded down.

The value cannot be greater than 255 or negative.

EXEMPLO
10 FOR I=1 TO 3
20 ON I GOTO 50,70,90
30 NEXT I
40 END
50 PRINT "I=";I;"line 50"
60 GOTO 30
70 PRINT "I=";I;"line 70"
80 GOTO 30
90 PRINT "I=";I;"line 90"
100 GOTO 30
RUN
I= 1 line 50
I= 2 line 70
I= 3 line 90
Ok

VEJA TAMBEM
GOTO

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("on interval", "", "MSX-BASIC", "ON INTERVAL=number GOSUB line", "Calls a subroutine after the specified timeout has been elapsed.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ON INTERVAL=number GOSUB line

FUNCAO
Calls a subroutine after the specified timeout has been elapsed.
The timeout is number*1/60 (or number*1/50) seconds.
After this statement MSX-Basic automatically calls the subroutine
in line in the number of intervals.

During the processing of an errorhandler MSX-Basic automatically
performs the "INTERVAL OFF" statement.

EXEMPLO
10 ON INTERVAL=300 GOSUB 60
20 INTERVAL ON
30 FOR I=1 TO 10000:NEXT I
40 INTERVAL OFF
50 END
60 K=K+6:PRINT K;"seconds"
70 RETURN
RUN
6 seconds
12 seconds
Ok

VEJA TAMBEM
INTERVAL ON/OFF/STOP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("on key", "", "MSX-BASIC", "ON KEY GOSUB line [,line2 [,...]]", "line, line2, ... are the first line number of the corresponding subroutine.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ON KEY GOSUB line [,line2 [,...]]

FUNCAO
line, line2, ... are the first line number of the corresponding subroutine.
line substitutes the linenumber of the subroutine, which is called after the
function-key 1 (F1) has been pressed, line2 references F2, and so on...

Before a subroutine can be called the "KEY(number) ON" statement must be executed.
After doing so MSX-Basic automatically checks if the corresponding function-key was
pressed and calls the associated subroutine.

During the processing of an errorhandler MSX-Basic automatically performs the "KEY(number) OFF" statement.

EXEMPLO
10 KEY OFF
20 ON KEY GOSUB 70, 80, 90
30 KEY(1) ON:KEY(2) ON:KEY(3) ON
40 GOTO 60
50 PRINT " pressed"
60 GOTO 60
70 PRINT "F1";:RETURN 50
80 PRINT "F2";:RETURN 50
90 PRINT "F3";:RETURN 50

VEJA TAMBEM
KEY ON/OFF/STOP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("on sprite", "", "MSX-BASIC", "ON SPRITE GOSUB line", "Calls a subroutine in line if two sprites are colliding.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ON SPRITE GOSUB line

FUNCAO
Calls a subroutine in line if two sprites are colliding.
line is the first line of the subroutine.

The subroutine can only be called after the "SPRITE ON" statement has been executed.
After this statement MSX-Basic automatically calls this subroutine when two sprites
are colliding.

During the processing of an errorhandler MSX-Basic automatically performs the
"SPRITE OFF" statement.

EXEMPLO
10 DATA 60,66,165,129,165,153,66,60
20 DATA 60,126,219,255,255,219,102,60
30 A$=""
40 FOR I=1 TO 8
50 READ A:A$=A$+CHR$(A)
60 NEXT I
70 B$=""
80 FOR I=1 TO 8
90 READ A:B$=B$+CHR$(A)
100 NEXT I
110 SCREEN 2,1:COLOR 15,4,1
120 ON SPRITE GOSUB 210
130 SPRITE$(0)=A$:SPRITE$(1)=B$
140 SPRITE ON
150 A=INT(RND(1)*256)
160 B=INT(RND(1)*256)
170 FOR I=0 TO 191
180 PUT SPRITE 0,(A,I),1
190 PUT SPRITE 1,(B,191-I),15
200 NEXT I:GOTO 140
210 SPRITE OFF
220 PLAY "L4CDEFEDCREFGAGFER"
230 PUT SPRITE 0,(0,208)
240 PUT SPRITE 1,(0,208)
250 I=191:RETURN
RUN

VEJA TAMBEM
SPRITE ON/OFF/STOP,
GOSUB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("on stop", "", "MSX-BASIC", "ON STOP GOSUB line", "Calls a subroutine in CTRL+STOP keys are pressed simultaneously.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ON STOP GOSUB line

FUNCAO
Calls a subroutine in CTRL+STOP keys are pressed simultaneously.
line is the first line of the subroutine.

The subroutine cann only be called after the "STOP ON" statement.
After this instruction MSX-Basic automically calls the subroutine
when CTRL- and STOP keys are pressed simultaneously.

During the processing of an errorhandler MSX-Basic automatically
performs the "STOP OFF" statement.

EXEMPLO
10 ON STOP GOSUB 50
20 STOP ON
30 INPUT A$
40 IF A$="END" THEN STOP OFF:END ELSE GOTO 30
50 PRINT "Enter END and press Return":RETURN
RUN
END
Ok

VEJA TAMBEM
STOP ON/OFF/STOP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("on strig", "", "MSX-BASIC", "ON STRIG(stick) GOSUB line, line2, ...", "Pressing a \"fire-button\" calls a subroutine.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
ON STRIG(stick) GOSUB line, line2, ...

FUNCAO
Pressing a "fire-button" calls a subroutine.
line respectively line2 is the first line of the
corresponding subroutine.

line corresponds to the first line number of the subroutine
which is called if the Space key is pressed, where line2 is
the line-number to be called if the key on Joystick 1 is pressed,
and so on.
The subroutine can only be called after the "STRIG(stick) ON"
statement. After this instruction MSX-Basic automically calls the
subroutine when a fire-button is pressed.

During the processing of an errorhandler MSX-Basic automatically performs
the "STRIG(stick) OFF" statement.

EXEMPLO
10 CLS
20 ON STRIG(X) GOSUB 50
30 STRIG(0) ON
40 GOTO 40
50 LOCATE 5,5
60 PRINT "SPACE key pressed"
70 FOR I=1 TO 300:NEXT I
80 LOCATE 5,5:PRINT SPC$(17)
90 RETURN
RUN

VEJA TAMBEM
STRIG ON/OFF/STOP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("open", "", "MSX-BASIC", "OPEN \"device:filename\" [FOR direction] AS #channel [LEN=recordlength]", "Opens an input- or output-channel from or to the specified device.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic, RS232-Basic)

SINTAXE
OPEN "device:filename" [FOR direction] AS #channel [LEN=recordlength]

FUNCAO
Opens an input- or output-channel from or to the specified device.

device-name for MSX-Basic can be (for example):

Device name | device type
CRT: | screen (text-mode)
GRP: | screen (graphic-mode)
LPT: | printer
CAS: | tape (not supported on MSX Turbo-R)
A: | Disk drive
B: | Disk drive
COM: | RS232 (serial interface)
MEM: | MSX2 RAM-Disk

filename is the name of the file, which is about to be opened. The filename
must not specified for screen-modes and printer.

direction indicates if the file is to be read into the buffer,
or the buffer should be written to the file. The following keywords can be specified:

  INPUT = sequential read

  OUTPUT = sequential write

  APPEND = sequential write to the end of an existing file

If the FOR direction is not specified MSX-Basic assumes that
the file is a random file. In this case a file is created, if not
found on the disk of a disk-drive.

Not every direction can be used for all device names.
Allowed combinations are as following:
Device name | OUTPUT | INPUT | APPEND
CRT: | X | - | -
GRP: | X | - | -
LPT: | X | - | -
CAS: | X | X | -
A: | X | X | X
B: | X | X | X
COM: | X | X | -
MEM: | X | X | X

RS232: Note that random access to a channel is illegal. Only sequential accesses
are logically possible.

channel is the file-number, as long as the file is opened.
This number is used to operate with Input/Output instructions
(such as "INPUT#", "PRINT#", "CLOSE#") on the file.

recordlength is an integer. If set it specifies the record-length
of a random-file. The default-value is 256.

Before an I/O operation is performed the file has to be opened first.
The maximum number of simultaneously opene files can be specified by
the "MAXFILES" statement.

EXEMPLO
OPEN "CAS:TEST" FOR OUTPUT AS #1
PRINT #1,X

The value of X is saved in a file named "TEST" (on tape). I.e. it's possible to read the stored value using:

OPEN "CAS:TEST" FOR INPUT AS #2
INPUT #2,X

10 SCREEN 2:COLOR 15,4,7
20 OPEN "GRP:" FOR OUTPUT AS #1
30 LINE (32,32)-(120,120),6,B
40 CIRCLE (120,120),56,1
50 PRESET (40,8)
60 PRINT #1,"Text in graphic screen"
70 FOR I=1 TO 2000: NEXT I
80 END
RUN

VEJA TAMBEM
CLOSE,
INPUT#,
PRINT#,
EOF,
LOC,
LOF,
MAXFILES,
RAM-Disk in MSX-Basic 2

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("or", "", "MSX-BASIC", "number1 OR number2", "Logical inclusive or.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
number1 OR number2

FUNCAO
Logical inclusive or.
0 OR 0 | = | 0
0 OR 1 | = | 1
1 OR 0 | = | 1
1 OR 1 | = | 1

VEJA TAMBEM
AND,
EQV,
IMP,
MOD,
NOT,
XOR,
MSX-Basic operators

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("out", "", "MSX-BASIC", "OUT port,value", "Writes a value to an output-port.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
OUT port,value

FUNCAO
Writes a value to an output-port.
value and port must be in range from 0 to 255.

To read a byte from an input-port the function "INP" must be used.

VEJA TAMBEM
INP,
WAIT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("pad", "", "MSX-BASIC", "PAD( <number> )", "Returns the position on the touchpad. One or more touchpads can be connected on the...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
PAD( <number> )

FUNCAO
Returns the position on the touchpad. One or more touchpads can be connected on the joystick-port(s).

<number> is an integer between 0 and 7. If <number> has a value between 0 and 3 it's assumed that the touchpad is connected to port 1. If <number> has a value between 4 and 7 the touchpad is connected to port 2.

If <number> is 0 or 4 the state of the touchpad is returned. The function returns -1, if the pad is touched and 0 if not.

If <number> is 1 or 5 the X-coordinate is returned, if <number> is 2 or 6 the Y-coordinate is returned.

If <number> is 3 or 7 the state of the button on the touchpad is returned. This function returns -1 if the button is pressed and 0 is not.

MSX 2 and MSX 2+:

If <number> is in range from 8 to 11 PAD is used to retrieve the state of a lightpen.

Note: this support is removed at the MSX Turbo-R, therefore such computers will return 0 in this case.

MSX 2 and higher:

If <number> is 12 the X/Y-offset of the mouse in port 1 is retrieved and can be queried by using PAD(13) and PAD(14). The return-value of PAD(12) is always -1.

If <number> is 13 the X-offset of the mouse in port 1 is returned (-128 to +127)

If <number> is 14 the Y-offset of the mouse in port 1 is returned (-128 to +127)

If <number> is 15 the return-value is 0 (reserved for mouse in port 1).

If <number> is 16 the X/Y-offset of the mouse in port 2 is retrieved and can be queried by using PAD(17) and PAD(18). The return-value of PAD(16) is always -1.

If <number> is 17 the X-offset of the mouse in port 2 is returned (-128 to +127)

If <number> is 18 the Y-offset of the mouse in port 2 is returned (-128 to +127)

If <number> is 19 the return-value is 0 (reserved for mouse in port 2).

Please note that the button-state of the mouse must be retrieved with the STRIG-function.

EXEMPLO
10 SCREEN 2
20 A=0
30 IF PAD(0)=0 THEN 20
40 X=PAD(1):Y=PAD(2)
50 IF A=0 THEN PSET(X,Y) ELSE LINE-(X,Y)
60 A=1
70 GOTO 30

VEJA TAMBEM
PDL,
STICK,
STRIG

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("paint", "", "MSX-BASIC", "PAINT [STEP](x,y)[,color][,bordercolor]", "Fills a drawing in graphics mode with the specified color.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
PAINT [STEP](x,y)[,color][,bordercolor]

FUNCAO
Fills a drawing in graphics mode with the specified color.

x is the X-coordinate of the starting point and must be an integer between 0 and 255 (or 511).

y is the Y-coordinate of the starting point and must be an integer between 0 and 191 (or 211).

If the word "STEP" is used the values of x and y are depending on the
current cursor-position. In this case x and y can be negative too.

color is the color-number to be used for filling.

bordercolor is the color-number of the border-line.

In graphic-mode 1 the fill-color must be equal to the border-color.
In this case bordercolor does not need to be specified.
In graphic-mode 2 (and higher) fill-color and border-color can be different.

color and bordercolor must be integers between 0 and 15.
See: color-table

EXEMPLO
10 SCREEN 2:COLOR 15,4,7
20 CIRCLE (80,80),20,8
30 PAINT (80,80),8
40 FOR I=1 TO 2000:NEXT I
50 SCREEN 3:COLOR 15,4,7
60 LINE (10,10)-(100,100),8,B
70 PAINT (45,45),2,8
80 FOR I=1 TO 2000:NEXT I
90 END
RUN

VEJA TAMBEM
CIRCLE,
DRAW,
LINE,
PSET,
SCREEN,
CLS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("pdl", "", "MSX-BASIC", "PDL( <number> )", "Returns the position of a paddle. One or two paddles can be connected to the...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
PDL( <number> )

FUNCAO
Returns the position of a paddle. One or two paddles can be connected to the joystick-port(s).

This function returns a value between 0 and 255.

<number> is a number from 1 to 12.
If <number> is 1, 3, 5, 7, 9 or 11 so the paddle is connected to port 1.
If <number> is 2, 4, 6, 8, 10 or 12 the paddle is connected to port 2.

EXEMPLO
10 CLS
20 FOR I=1 TO 500
30 LOCATE 10,5:PRINT PDL(1),PDL(2)
40 NEXT I
50 END

VEJA TAMBEM
PAD,
STICK,
STRIG

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("peek", "", "MSX-BASIC", "PEEK(address)", "Returns a memory-byte.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
PEEK(address)

FUNCAO
Returns a memory-byte.

This function returns the decimal-value of a memory-address
and therefore results in a value between 0 and 255.

address is the memory-address and must be in range
between -32768 and 65535.

If address is negative the binary complement is used.
This means PEEK(-1)=PEEK(65536-1)=PEEK(65535).

The "POKE"statement is used to write a byte to a specific
memory-address.

EXEMPLO
10 A=PEEK(65535)
20 A$="00"+HEX$(A)
30 PRINT RIGHT$(A$,2)
40 END
RUN
F0
Ok

VEJA TAMBEM
POKE,
VPEEK,
BLOAD,
BSAVE,
VARPTR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("play", "", "MSX-BASIC", "PLAY \"string\" [,\"string2\"[,\"string3\"]]", "This functions plays the notes specified in string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
PLAY "string" [,"string2"[,"string3"]]

FUNCAO
This functions plays the notes specified in string.
In this string you can also specify the pitch, duration, tempo and pause.

string, string2 and string3 are sub-commands built by
alphanumeric constants. string corresponds to voice 1, string2 to
voice 2 and string3 to voice 3.

To play a note from the scale: the following sub-commands are creating a
note from the scale: A, B, C, D, E, F and G. A "+" or "#" after a note are indicating
an -is (increased by a halftone) where A "-" indicates an -es (decreased by a halftone.

To set an octave: the sub-command "Onumber" sets the octave-number,
which should be played. number is an integer from 1 to 8.

If no octave is specified the last used octave is used. The default value is 4.

To play a note-number: instead of playing an octave-number and a note-number
music-note-numbers can be specified also. The sub-command "Nnumber" is used
to specify a note-number. number is an integer between 0 and 95. If number
is 0 no sound is generated. In this case a short pause is played before the remaining part.

To specify a note-length: the note-length is specified by the sub-command
"Lnumber". number is an integer between 1 and 64 and has the following meaning:

1 = full note

2 = half note

3 = third note

4 = quarter note

and so on.

If no length is specifed the last used note is used. The default value is 4.

To change a note-length: by adding a dot (.) direct after specifying a note the length
is multiplied by 1/2. You can also add multiple dots; e.g. "..." means the note is played in
15/8 times the speed of its original length.

To set a pause: the sub-command "Rnumber" is used to set a pause.
number is an integer between 1 and 64 and has the following meaning:

1 = full pause

2 = half pause

3 = third pause

4 = quarter pause

and so on.

To set a tempo: the tempo is set with the sub-command "Tnumber".
number is an integer from 32 to 255 and indicates the number of quarter
notes per minute. If no tempo is set the last tempo will be used. The default value is 120.

To set an envelope: the envelope is set with the sub-command "Snumber".
number is an integer from 0 to 15.

If no envelope is specified the last set value will be used. The default value is 1.

Sound modulation: the sound-period is specified with the sub-command "Mnumber".
number is an integer from 1 to 65536 and sets the sound-period.

If no modulation is specified the last set modulation will be used.
The default value is 255.

To start execution: The music instructions in an alphanumeric variable can be
executed with the sub-command "XA$;" (don't forget the semicolon!).

A$ is the name of the variable that contains the musical sub-commands.
Numeric variables can be used instead of numeric values in sub-commands.
In this case an equal sign (=) has to be specified before these numeric variables
and they have to be terminated with a semicolon (;), e.g.: AB=10:PLAY "N=AB;".

If MSX-Basic generates a beep-sound all values of the sub-commands will be reset
to their default values!

EXEMPLO
PLAY "O5CDEFGABO6C"

10 A$="T128"
20 B$="L4CCFGL2AL8FEL4D.L8B-L4B"
30 PLAY "XA$;"
40 PLAY "XB$;"
50 END
RUN

VEJA TAMBEM
BEEP,
PLAY(),
PLAY (MSX-Music)

REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
PLAY(voice)

FUNCAO
Returns the state of playing music.

voice has the following meaning:
0 = voice1, voice 2 and voice 3

1 = voice 1

2 = voice 2

3 = voice 3

The function returns -1 if the selected voice is playing.
If playing has ended the function returns 0 again.

If the function is called immediately after the "PLAY"
statement the value -1 is persistent.

EXEMPLO
10 A$="":BEEP
20 FOR I=1 TO 6
30 READ AA$:A$=A$+AA$
40 NEXT I
50 PLAY "XA$;"
60 SCREEN 0:WIDTH 35:CLS
70 IF PLAY(0)=-1 THEN PRINT "The music is playing":GOTO 70
80 PRINT "The music has ended"
90 END
100 DATA CCGGAAGR
110 DATA FFEEDDCR
120 DATA GGFFEEDR
130 DATA GGFFEEDR
140 DATA CCGGAAGR
150 DATA FFEEDDCR
RUN

VEJA TAMBEM
PLAY

REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

(MSX-Music)

SINTAXE
PLAY [#MODE ,] MML1 [,... [, MMLn [, MMLm [, MMLp1... [,MMLp3]]]]]]]]]]]]

MODE can be 0, 2, 3

MML1 through MMLn are FM sound MML's

MML numbers are the same as the values for CALL MUSIC

MML m is for the rhythm MML

MML p1 through MML p3 are for the PSG MML

FUNCAO
Music will be played according to the MML settings.

Notes:  If MODE is set to 0 or not set, PSG will be played
only MML p1-3 can be used
if MODE is set to either 2 or 3 both FM and PSG can be used
MML 1-2 are set according to the MUSIC command as notes from
the  starting  order.  However if a mode that does not  use  the
rhythm  channels  is  specified,  the rhythm  channels  must  be
omitted.

Music MML

This  is a table of the MML (music macro language) that can be used with
 the PLAY command.
Statement | Meaning | Range | Default Value *1
Mn | Envelope cycle setting *2 | 1>=n<=65535 | m255
Sn | Envelope shape *2 | 0<=n<=15 | S0
Vn | Volume | 0<=n<=15 | V8
Ln | Length | 1<=n<=64 | L4
Qn | Division of sound length | 1<=n<=8 | Q8
On | Octave | 1<=n<=8 | O4
> | Raise 1 Octave | - | -
< | Lower 1 Octave | - | -
Tn | Tempo setting | 32<=n<=255 | T120
Nn | Plays Note raised to n | 0<=n<=96 | -
Rn | Rest setting | 1<=n<=64 | R4
A-G | Note | - | -
+, # | Sharp (raise half tone) | - | -
- | Flat (Lower half tone) | - | -
. (period) | Lengthen note or rest by 1.5 | - | -
XA$ | Plays MML stored in string variable A$ *3 | - | -
=x | sets parameter to x | *4 | -
& | Tie | - | -
{}n | will generate even notes of n for the number within the {} | 1<=n<=64 | value set with Ln
@n | changes the voice set to n | 0<=n<=63 | R4
@Vn | sets detail change in volume | 0<=n<=127 | R4
@Nn | maintain length set by n | 1<=n<=64 | value set with Ln
*1 Initial value set when CALL MUSIC is used

*2 For PSG only

*3 Adding MML after [XA$;] will result in error

*4 The   value range is dertimined by the preceeding MML but it cannot exceed the value 32767

Rhythm MML

There  are  5  different  voice  that  are  available  for  the   rhythm
(percussion)  MML and up to 3 voices may be played  simultaneously.  For
this reason the rhythm MML first lines up the instruments that are to be
played  simultaneously.

Statement | Meaning | Range | Default Value
B | Bass drum | - | -
S | Snare drum | - | -
W | Tom tom | - | -
C | Cymbals | - | -
H | Hi Hat | - | -
n (number) | previous notes are played and n th note are rested | 1<=n<=64 | -
Vn | Volume | 0<=n<=15 | 8
! | Accents preceeding note | - | -
@An | Sets volume for voices that are accented. | 0<=n<=15 | -
[Tn], [@Vn], [Rn], [XA$;], [=x;], [.(period)] is the same as music MML.

EXEMPLO
PLAY #2, "CD", "EF", "GA"

PLAY#2,"","","","BSH8H8H8S!H!8H8"

Bass, snare,hihat and wait an 8th note.
Hi hat and wait an 8th note.
snare, hi hat plays accented and waits an 8th note.
Hi hat and wait an 8Th note.

VEJA TAMBEM
PLAY (MSX-Basic),
CALL PLAY,
CALL VOICE,
CALL VOICE COPY,
CALL BGM,
MSX-Music Voices

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("point", "", "MSX-BASIC", "POINT(x,y)", "Returns the color-number of the specified point in graphics-mode.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
POINT(x,y)

FUNCAO
Returns the color-number of the specified point in graphics-mode.

x is the X-coordinate and must be an integer between 0 and 255 (or 0 and 511).

y is the Y-coordinate and must be an integer between 0 and 191 (or 0 and 211).

EXEMPLO
10 SCREEN 2:COLOR 15,1,7
20 OPEN "GRP:" FOR OUTPUT AS #1
30 FOR I=1 TO 10
40 PRESET(I*16,8)
50 C=INT(RND(1)*13)+2
60 COLOR C
70 PRINT #1,CHR$(1)+CHR$(66)
80 NEXT I
90 COLOR 15
100 PRESET (40,24)
110 PRINT #1,"Color numbers"
120 FOR I=1 TO 20
130 C=POINT(I*16+4,12)
140 PRESET(I*16,I*8+56),4
150 PRINT#1,USING "##";C
160 NEXT I
170 FOR I=1 TO 2000:NEXT I
180 COLOR 15,4,7
190 END
RUN

VEJA TAMBEM
PSET

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("poke", "", "MSX-BASIC", "POKE adress,value", "Fills a byte in memory adress with a byte of value.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
POKE adress,value

FUNCAO
Fills a byte in memory adress with a byte of value.

address is the memory-address and must be in range
between -32768 and 65535.

If address is negative the binary complement is used.
This means POKE-1,0=POKE(65536-1),0.

This function is used to change the value of a specific byte in memory.

EXEMPLO
10 POKE 65534,PEEK(65534)
20 END
RUN
Ok

VEJA TAMBEM
PEEK,
VPOKE,
BLOAD,
BSAVE,
DEFUSR,
VARPTR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("pos", "", "MSX-BASIC", "POS(number)", "Gives the X-coordinate (column-number) of the cursor.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
POS(number)

FUNCAO
Gives the X-coordinate (column-number) of the cursor.

This function returns a number between 0 and 39 (or 0 and 79).
The most left column is 0. This function can only be used in text mode.

EXEMPLO
10 SCREEN 0
20 LOCATE 10,20
30 PRINT POS(0)
30 END
RUN
10
Ok

VEJA TAMBEM
CSRLIN,
LOCATE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("preset", "", "MSX-BASIC", "PRESET [STEP](x,y)[,color]", "Sets a color into the specified point at x,y in graphics mode.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
PRESET [STEP](x,y)[,color]

FUNCAO
Sets a color into the specified point at x,y in graphics mode.

x is the X-coordinate of the point and must be an integer between 0 and 255 (or 0 and 511).

y is the Y-coordinate of the point and must be an integer between 0 and 191 (or 0 and 211).

If the word "STEP" is used values for x and y are relative to the current
cursor-position. In this case x and y can be negative too.

color is the color-number and must be an integer between 0 and 15
(see: color-table).

If color is not set the last used background-color will be assumed.
The default-value of color is 4.

EXEMPLO
10 SCREEN 2:COLOR 15,4,7
20 LINE (40,40)-(215,151),15,BF
30 FOR I=0 TO 1000
40 A=INT(RND(1)*173)+41
50 B=INT(RND(1)*109)+41
60 PRESET(A,B)
70 FOR K=0 TO 300:NEXT K
80 NEXT I
90 END
RUN

VEJA TAMBEM
PSET,
CIRCLE,
DRAW,
LINE,
PAINT,
SCREEN,
CLS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("print", "", "MSX-BASIC", "PRINT [USING \"format\";] [value [,value2 [,...]]] [;]", "The first format is used to display data on the screen. If the word PRINT is used without",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
PRINT [USING "format";] [value [,value2 [,...]]] [;]

PRINT #channel, [USING "format";] variable

FUNCAO
The first format is used to display data on the screen. If the word PRINT is used without
suffix MSX-Basic generates an empty line. If an expression is added it's value is displayed
on the screen. The expression can be of numeric or alphanumeric type. In case of alphanumeric
constants these have to be quoted (e.g. 10 PRINT "Hello").

Print position: The position where the printout appears on the screen
depends on the punctation between different values (expressions). MSX-Basic separates each
screen-line into zones of 14 positions. If a value is followed by a colon (,) the next
output appears in the next zone. If the value is followed by a semicolon (;) the next
output appears immediately behind the last value. If a "PRINT" statement ends with a colon (,)
or a semicolon (;) the output of the next "PRINT" statement appears on the same screen-line.
If a "PRINT" statement does not end with a colon (,) or a semicolon (;) the output of the
next "PRINT" statement appears in the next line. If the number of positions in the printout
is larger than the number of positions per screen-line (defined by the "WIDTH" statement)
MSX-Basic automatically continues in the next line.

Numbers: Behind each number, that is displayed on the screen, is always
a space. In front of positive numbers is a space, in front of negative numbers a minus (-).

The 2nd format of the "PRINT" statement is used to write data into a sequential file (opened
as #channel) opened for OUTPUT. After every "PRINT" statement a 'CR' and 'LF' character
is written into the file. MSX-Basic automatically places a colon "," between numerical
expressions in a simple "PRINT" statement as well as a hyphen if printing goes to a file.
Alphanumerical expressions have to be separated by a colon and a hyphen. To read data from
a file with uniform records the statement "INPUT#" can be used.

Printing formats: printout can be formatted, if preceded by "USING".
Possible syntax of the specified format is documented in the chapter "USING".

Instead of the keyword "PRINT" also a question mark (?) can be used.

EXEMPLO
? "Hello"
Hello

PRINT "World"
World

10 CLS
20 FOR I=1 TO 5
30 ? USING "Number is #.#-";I
40 NEXT

VEJA TAMBEM
LPRINT,
TAB,
SPC,
PUT,
INPUT,
CLS,
WIDTH,
LOCATE,
OPEN,
INPUT[#]

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("pset", "", "MSX-BASIC", "PSET [STEP](x,y)[,color]", "Sets a color into the specified point at x,y in graphics mode.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
PSET [STEP](x,y)[,color]

FUNCAO
Sets a color into the specified point at x,y in graphics mode.

x is the X-coordinate of the point and must be an integer between 0 and 255 (or 0 and 511).

y is the Y-coordinate of the point and must be an integer between 0 and 191 (or 0 and 211).

If the word "STEP" is used values for x and y are relative to the current
cursor-position. In this case x and y can be negative too.

color is the color-number and must be an integer between 0 and 15
(see: color-table).

If color is not set the last used foreground-color will be assumed.
The default-value of color is 15.

EXEMPLO
10 SCREEN 2:COLOR 15,4,7
20 LINE (40,40)-(215,151),15,B
30 FOR I=0 TO 1000
40 A=INT(RND(1)*173)+41
50 B=INT(RND(1)*109)+41
60 PSET(A,B)
70 FOR K=0 TO 300:NEXT K
80 NEXT I
90 END
RUN

VEJA TAMBEM
PRESET,
CIRCLE,
DRAW,
LINE,
PAINT,
POINT,
SCREEN,
CLS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("put sprite", "", "MSX-BASIC", "PUT SPRITE priority, [STEP](x,y), color, number", "Put a defined sprite in graphics mode on the screen.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
PUT SPRITE priority, [STEP](x,y), color, number

FUNCAO
Put a defined sprite in graphics mode on the screen.

priority specifies the priority of the sprite and must be an integer between 0 and 31.

x is the X-coordinate of the sprite and must be an integer between -32 and 255.

y is the Y-coordinate of the sprite and must be an integer between -32 and 191 or
208 or 209.
If y is 208 all sprites with a lower priority will disappear from the screen.
If y is 209 only this sprite will disappear from the screen.

If the word "STEP" is specified the values x and y are relative to the current
screen-position. In this case x and y can be negative too. If x and y
are not specified the actual coordinate remains unchanged.

color is the color-number of the sprite and must be an integer between 0 and 15
(see: color-table).

If color is not set the last foreground-color will be used. The default-value is 15.

number is the number of the sprite, as used in SPRITE$(number).
If number is not specified its equal to the priority.

EXEMPLO
See: ON SPRITE GOSUB

VEJA TAMBEM
SPRITE$,
ON SPRITE GOSUB,
SCREEN,
COLOR SPRITE,
COLOR SPRITE$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("read", "", "MSX-BASIC", "READ variable [,variable2 [,...]]", "Reads a constant from the \"DATA\" statement and assigns it to a variable.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
READ variable [,variable2 [,...]]

FUNCAO
Reads a constant from the "DATA" statement and assigns it to a variable.

The "READ" statement can only be used with a "DATA" statement.
The "READ" statement gives access to the data, listed after the "DATA" statement and
in order of the line-numbers.

The variable in the "READ" statement can be numeric or alphanumeric.
The type of the variable must match with the corresponding constant
in the "DATA" statement.

A single "READ" statement can give access to one or more "DATA" statements.
Its also possible to access the data of a single "DATA" statement with multiple
"READ" statements.

If the number of variables in the "READ" statement is larger than the number
of constants in the "DATA" statement MSX-Basic generates an "Out of data" error.

If the number of variables in the "READ" statement is smaller than the number
of constants in the "DATA" statement  the next "READ" statement continues
with the remaining constants. If no more "READ" statements are executed
the remaining constants are ignored.

"DATA" statements can be specified since the program-start, or explicitly
selected with a line-number in the "RESTORE" statement.

EXEMPLO
See: DATA

VEJA TAMBEM
DATA,
RESTORE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("rem", "", "MSX-BASIC", "REM", "Inserts a remark into the program text.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
REM

'

FUNCAO
Inserts a remark into the program text.

"REM" statements are not executed when the program runs, but printed if
the program is listed.

The program use "GOTO" and "GOSUB" to link to a "REM" statement.
In this case the program execution continues at the statement behind
the "REM".

Instead of the keyword "REM" also a single apostroph (') can be used.
Do not use this keyword within a "DATA" statement.

EXEMPLO
10 REM Calculation
20 FOR I=1 TO 10
30 SUM=SUM+I
40 NEXT I
50 SUM=SUM/10

or

10 FOR I=1 TO 10 'Calculation
20 SUM=SUM+I
30 NEXT I
40 SUM=SUM/10

VEJA TAMBEM
DATA

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("renum", "", "MSX-BASIC", "RENUM [line] [,line2 [,increment]", "Renumbers program lines.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
RENUM [line] [,line2 [,increment]

FUNCAO
Renumbers program lines.

line is the first program-line, which is used in the new order.
If line is not specified the order starts in line 10.

line2 is the existing program-line where the re-ordering should start.
If line2 is not specified the order starts in the 1st program-line.

increment is the step-width of the new order.
If increment is not specified MSX-Basic uses 10.

The statement "RENUM" renumbers also the line-numbers after the statements
"GOTO", "GOSUB", "IF THEN ELSE", "ON GOTO" and "ON GOSUB". The renumbered
value is based on the new line-number which has been changed due to the re-ordering.
If a non-existing line-number is recognized the error "Undefined line XX in YY"
is raised.

The statement "RENUM" can not be used to change the order of line-numbers.

EXEMPLO
RENUM

This statement renumbers all program lines, beginning at the first line.
The first line gots the number 10. Subsequent line-numbers are increased by 10.

RENUM 1000,900,50

(new value=1000, previously 900, increment by 50)

This statement renumbers all program lines, beginning on line 900.
Line 900 becomes line 1000 ands ubsequent line-numbers are increased by 50.

900 FOR I=1 TO 10
910 PRINT I;
920 IF I<10 THEN 930 ELSE 940
930 NEXT
940 END

1000 FOR I=1 TO 10
1050 PRINT I;
1100 IF I<10 THEN 1150 ELSE 1200
1150 NEXT
1200 END

VEJA TAMBEM
AUTO

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("restore", "", "MSX-BASIC", "RESTORE [line]", "The \"RESTORE\" statement sets the data-pointer to the first element",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
RESTORE [line]

FUNCAO
The "RESTORE" statement sets the data-pointer to the first element
of the "DATA" statement to line of the program.

line is the line-number of a "DATA" statement.
After a "RESTORE" is performed the first constant of a "DATA" statement
is returned by the "READ" statement.

If line is not specified the "READ" statement returns the
1st constant of the 1st "DATA" statement.

EXEMPLO
10 DATA 123,ABC,48.5
20 READ A%,A$,A!
30 PRINT A%;A$;A!
40 RESTORE
50 READ A%:PRINT A%
60 END
RUN

VEJA TAMBEM
DATA,
READ

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("resume", "", "MSX-BASIC", "RESUME [line]", "line is the line-number of the program, in which",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
RESUME [line]

RESUME 0

RESUME NEXT

FUNCAO
line is the line-number of the program, in which
- after execution of the error-handling routine specified by
"ON ERROR GOTO" - the program will be resumed. If line
is not specified or 0 the program will be continued on the
location where the error was detected.

If the keyword "NEXT" is used (instead of line) the
program execution is resumed on the statement immediately
following the error-location.

EXEMPLO
See ERR

VEJA TAMBEM
ON ERROR GOTO,
ERL,
ERROR,
ERR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("return", "", "MSX-BASIC", "RETURN [linenumber]", "Returns from a subroutine that was invoked by GOSUB.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
RETURN [linenumber]

FUNCAO
Returns from a subroutine that was invoked by GOSUB.

In contrast to other Basic dialects the RETURN statement can specify
an explicit line-number where the program can return to.

EXEMPLO
100 PRINT "Hello"
110 GOSUB 200
120 END
200 PRINT "Here am I"
210 RETURN 110

Generates an infinite loop because the subroutine always returns
to it's invocation point:

Hello
Here am I
Here am I
...

VEJA TAMBEM
GOSUB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("right$", "", "MSX-BASIC", "RIGHT$(string, number)", "Returns the last number (0-255) characters from the right end of the specified string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
RIGHT$(string, number)

FUNCAO
Returns the last number (0-255) characters from the right end of the specified string.
If number is larger than the string the whole string is returned.
If number is 0 an empty string is returned.

EXEMPLO
10 A$="Basic"
20 FOR I=1 TO LEN(A$)
30 PRINT RIGHT$(A$,I)
40 NEXT
50 END
RUN
c
ic
sic
asic
Basic

VEJA TAMBEM
LEFT$,
MID$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("rnd", "", "MSX-BASIC", "RND(number)", "Generates a random-number between 0 and 1.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
RND(number)

FUNCAO
Generates a random-number between 0 and 1.

Whenever a program is started with "RUN" the same sequence of random-numbers is generated.

If number is greater than 0 the next random-value of the sequence is returned.
If number is equal to 0 the last random-value of the sequence is returned.

If number is less than 0 the sequence of random-numbers is rebuilt based on the
negative number.

To generate a real random-number the generator must always select a new sequence
after the program has been started. This can be ensured by using the variable "TIME".

EXEMPLO
10 R=RND(TIME)
20 FOR I=1 TO 10
30 SCREEN 2:COLOR 15,4,7
40 C=INT(RND(1)*15+1)
50 IF C=4 GOTO 40
60 CIRCLE (80,80),20,C
70 PAINT (80,80),C
90 NEXT I
100 END
RUN

VEJA TAMBEM
TIME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("run", "", "MSX-BASIC", "RUN [line]", "Starts the execution of a program, which is stored in the computer-memory",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
RUN [line]

RUN "devicename:filename" [,line]

FUNCAO
Starts the execution of a program, which is stored in the computer-memory
or a device.

Devicenames can be, e.g.: CAS:, A: or B:

The "SAVE" statement must have been used to store the program in ASCII format (on tape or disk).

The filename is an alphanumerical constant, that has been specified while the program was
stored using the "SAVE" statement.

If devicename and filename are not specified the program in memory will be executed.

line corresponds to the program line-number.
If line is specified the program starts in this line.
If line is not specified the program starts in the first line.

Turbo-R: the "CAS:" device is no longer supported!

EXEMPLO
RUN 100

(in this case the program execution starts in line 100)

VEJA TAMBEM
LOAD,
CALL RUN,
END,
STOP,
CONT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("save", "", "MSX-BASIC", "SAVE \"devicename:filename\" [,A]", "Stores a program of the computer-memory onto a device (tape or disk).",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, Disk-Basic)

SINTAXE
SAVE "devicename:filename" [,A]

FUNCAO
Stores a program of the computer-memory onto a device (tape or disk).

If a program is saved to a tape ("CAS:") it can be restored with the "LOAD" and "MERGE" statements.

Reminder:

CSAVE saves in binary format

SAVE "CAS:" saves in ASCII format onto tape

SAVE saves binary and SAVE...,A in ASCII code onto disk

Devicenames can be, e.g.: CAS:, A: or B:

Turbo-R: the "CAS:" device is no longer supported!

EXEMPLO
SAVE "CAS:DEMO"

VEJA TAMBEM
BSAVE,
CSAVE,
LOAD,
MERGE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("screen", "", "MSX-BASIC", "SCREEN screenmode [,spritesize, keyclick, baudrate, printertype, interlacemode]", "Specifies the screenmode, spritesize, sound on keypress, baudrate for the data-recorder",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SCREEN screenmode [,spritesize, keyclick, baudrate, printertype, interlacemode]

FUNCAO
Specifies the screenmode, spritesize, sound on keypress, baudrate for the data-recorder
and the used printer-type.

screenmode may be specified from 0 to 3. When specifying a screen mode,
in BASIC it is called "SCREEN MODE", which is somewhat different from "screen mode" which is used by VDP internally.
The Screen-modes table shows these correspondences and meanings.
The difference between screen modes 2 and 4 is only in the sprite display functions.

If screenmode is not specified the last used screenmode remains unchanged.
The default-value is 0.

The graphic-operations "PUT SPRITE", "CIRCLE", "DRAW", "LINE", "PAINT", "PSET", "PRESET",
"ON SPRITE GOSUB", "SPRITE ON/OFF/STOP" and "POINT" can only be used in graphics-mode.

MSX 2: adds screenmodes 4-8.

Korean MSX 2: supports screenmode 9 too.

MSX 2+: adds screenmodes 10-12.

spritesize specifies the sprite-size.

0 | small sprites (8x8 dots)
1 | small sprites (enlarged to 16x16 dots)
2 | large sprites (16x16 dots)
3 | large sprites (enlarged to 32x32 dots)
If spritesize is not specified the last value remains unchanged.
The default-value is 0.

keyclick specifies if a sound is generated when a key is pressed.
The value should be 0 or 1 and has the following meaning:
0 | no sound
1 | sound (key click)
If keyclick is not specified the last value remains unchanged.
The default value is 0.

baudrate specifies the baud-rate of the data-recorder.
The value must be 1 or 2 and has the following meaning:
1 | 1200 baud
2 | 2400 baud
If baudrate is not specified the last value remains unchanged.
The default value is 1.

printertype indicates if an MSX-Printer is used.
The value should be 0 or 1 and has the following meaning:
0 | MSX-Printer
1 | no MSX-Printer (e.g. Epson compatible)
If printertype is not specified the last value remains unchanged.
The default value is 1.

interlacemode enables to set the interlace functions of VDP (see Interlace mode).
In the alternate screen display mode, the display page specified in "SET PAGE" must be odd.
In this case the display page and the page of which the number is smaller by one is displayed alternately.

If interlacemode is not specified the last value remains unchanged.
The default value is 0.

MSX 2: adds screenmodes 4-8, interlacemode is added too.
The values of the last used text screenmode (0 or 1), keyclick,
baudrate and printertype can be saved by using "SET SCREEN" statement
and will be restored from the CMOS on power-up.

Korean MSX 2: adds screenmode 9.

MSX 2+: adds screenmodes 10-12 (mode 9 is supported on Korean computers only).

Turbo-R: the baudrate setting is ignored.

EXEMPLO
10 FOR I=2 TO 3
20 SCREEN I:COLOR 15,4,7
30 LINE (32,32)-(200,132),6
40 FOR K=0 TO 3000:NEXT
50 NEXT I
60 END
RUN

VEJA TAMBEM
PRINT,
INPUT,
CIRCLE,
DRAW,
LINE,
PAINT,
PSET,
PRESET,
WIDTH,
CLS,
CALL CLS,
CALL KANJI,
CALL ANK,
SET PAGE,
SET SCREEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("sgn", "", "MSX-BASIC", "SGN(number)", "Returns the value 1 if number is > 0,",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SGN(number)

FUNCAO
Returns the value 1 if number is > 0,
returns the value 0 if number is = 0 and
returns the value -1 if number is < 0.

EXEMPLO
10 INPUT "Number";N
20 IF N1 THEN GOTO 10
30 H=SGN(N):H=H+2
40 ON H GOSUB 70,80,90
50 PRINT B$
60 END
70 B$="Number is negative":RETURN
80 B$="Number is zero":RETURN
90 B$="Number is positive":RETURN
RUN

VEJA TAMBEM
ABS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("sin", "", "MSX-BASIC", "SIN(number)", "Returns the sinus of the specified number in radians.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SIN(number)

FUNCAO
Returns the sinus of the specified number in radians.

EXEMPLO
10 X=5
20 PRINT SIN(X)
30 END
RUN

VEJA TAMBEM
ATN,
COS,
TAN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("sound", "", "MSX-BASIC", "SOUND register, value", "Writes a value into the PSG register.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SOUND register, value

FUNCAO
Writes a value into the PSG register.

register is the register-number and must be an integer betwenn 0 and 15.

value is the value to be written into the register.
This must be an integer between 0 and 255.

USE THIS STATEMENT ONLY IF YOU ARE FAMILIAR WITH THE PSG
(PROGRAMMABLE SOUND GENERATOR).

EXEMPLO
10 FOR I=0 TO 13
20 SOUND I,0
30 NEXT I
40 SOUND 7,62:SOUND 8,15
50 FOR I=0 TO 255
60 SOUND 0,I
70 NEXT I
80 SOUND 8,0
90 END
RUN

VEJA TAMBEM
PLAY,
BEEP,
CALL AUDREG (MSX-Music)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("space$", "", "MSX-BASIC", "SPACE$(number)", "Generates an alphanumeric string with number of spaces.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SPACE$(number)

FUNCAO
Generates an alphanumeric string with number of spaces.

number must be an integer between 0 and 255.

EXEMPLO
10 FOR I=0 TO 5
20 A$=SPACE$(I)
30 PRINT A$;I
40 NEXT I
50 END
RUN
 1
  2
   3
    4
     5
Ok

VEJA TAMBEM
STRING$,
TAB,
SPC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("spc", "", "MSX-BASIC", "SPC(number)", "Prints number of characters.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SPC(number)

FUNCAO
Prints number of characters.

This function can only be used with "PRINT" and "LPRINT" statements.

number must be an integer between 0 and 255.

VEJA TAMBEM
PRINT,
LPRINT,
TAB,
SPACE$,
STRING$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("sprite", "", "MSX-BASIC", "SPRITE ON", "Checks for sprite-collisions.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SPRITE ON

SPRITE OFF

SPRITE STOP

FUNCAO
Checks for sprite-collisions.

After the "SPRITE ON" statement MSX-Basic checks with every statement
if two sprites are overlapping. If so MSX-Basic calls the subroutine,
that was specified in the "ON SPRITE GOSUB" statement.

If the "SPRITE OFF" statement is set, MSX-Basic does no longer check
for collisions.

After the "SPRITE STOP" statement MSX-Basic checks for collisions,
but does not call the subroutine that was specified in the "ON SPRITE GOSUB"
statement. But collisions are remembered. In this case the subroutine
is immediately called after the "SPRITE ON" statement is executed.

EXEMPLO
10 DATA 60,66,165,129,165,153,66,60
20 DATA 60,126,219,255,255,219,102,60
30 A$=""
40 FOR I=1 TO 8
50 READ A:A$=A$+CHR$(A)
60 NEXT I
70 B$=""
80 FOR I=1 TO 8
90 READ A:B$=B$+CHR$(A)
100 NEXT I
110 SCREEN 2,1:COLOR 15,4,1
120 ON SPRITE GOSUB 210
130 SPRITE$(0)=A$:SPRITE$(1)=B$
140 SPRITE ON
150 A=INT(RND(1)*256)
160 B=INT(RND(1)*256)
170 FOR I=0 TO 191
180 PUT SPRITE 0,(A,I),1
190 PUT SPRITE 1,(B,191-I),15
200 NEXT I:GOTO 140
210 SPRITE OFF
220 PLAY "L4CDEFEDCREFGAGFER"
230 PUT SPRITE 0,(0,208)
240 PUT SPRITE 1,(0,208)
250 I=191:RETURN
RUN

VEJA TAMBEM
ON SPRITE GOSUB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("sprite$", "", "MSX-BASIC", "SPRITE$(number)=string", "Contains the definition of sprites in graphic mode.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SPRITE$(number)=string

FUNCAO
Contains the definition of sprites in graphic mode.

number stays for the number of sprites. If the sprite-size has been specified
with 0 or 1 (by using the "SCREEN" statement), then number must be an integer
between 0 and 255.
If the sprite-size has been specified with 2 or 3, then number must be an integer
between 0 and 63.

The contents of the variable "SPRITE$" can be a binary, a hexadecimal or decimal value
- as down in the following example:
Binary | Hexadecimal | Decimal
 |  | &B00011000 | &H18 | 24
 |  |  |  | &B00111100 | &H3C | 60
 |  |  |  |  |  | &B01111110 | &H7E | 126
 |  |  |  |  |  |  |  | &B11111111 | &HFF | 255
 |  | &B00100100 | &H24 | 36
 |  | &B00100100 | &H24 | 36
 |  | &B01000010 | &H42 | 66
 |  | &B10000001 | &H81 | 129
Binary definition:

10 SCREEN 2,0
20 Y=1
30 B$=""
40 FOR I=1 TO 8
50 READ A$:B$=B$+CHR$(VAL("&B"+A$))
60 NEXT I
70 SPRITE$(0)=B$
80 FOR X=1 TO 200
90 Y=Y+1
100 PUT SPRITE 0,(X,Y)
110 NEXT X
120 END
130 DATA 00011000,00111100,01111110,11111111,00100100,00100100,010000010,10000001

Hexadecimal definition:

10 SCREEN 2,0
20 Y=1
30 B$=""
40 FOR I=1 TO 8
50 READ A$:B$=B$+CHR$(VAL("&H"+A$))
60 NEXT I
70 SPRITE$(0)=B$
80 FOR X=1 TO 200
90 Y=Y+1
100 PUT SPRITE 0,(X,Y)
110 NEXT X
120 END
130 DATA 18,3C,7E,FF,24,24,42,81

Decimal definition:

10 SCREEN 2,0
20 Y=1
30 B$=""
40 FOR I=1 TO 8
50 READ A$:B$=B$+CHR$(VAL(A$))
60 NEXT I
70 SPRITE$(0)=B$
80 FOR X=1 TO 200
90 Y=Y+1
100 PUT SPRITE 0,(X,Y)
110 NEXT X
120 END
130 DATA 24,60,126,255,36,36,66,129

EXEMPLO
See: ON SPRITE GOSUB

VEJA TAMBEM
PUT SPRITE,
ON SPRITE GOSUB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("sqr", "", "MSX-BASIC", "SQR(number)", "Returns the square-root of the specified number.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SQR(number)

FUNCAO
Returns the square-root of the specified number.

EXEMPLO
10 X=10:Y!=SQR(X)
20 PRINT Y!
30 END
RUN
 3.16228
Ok

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("stick", "", "MSX-BASIC", "STICK(sticknumber)", "This function returns the direction of the joystick. The meaning of the returnvalue",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
STICK(sticknumber)

FUNCAO
This function returns the direction of the joystick. The meaning of the returnvalue
is as following:
8 | 1 | 2
7 | 0 | 3
6 | 5 | 4

The sticknumber can have the following values:

0 = Cursor-keys (space is trigger)

1 = Joystick on port 1

2 = Joystick on port 2

EXEMPLO
10 PRINT "Press the cursor-keys"
20 PRINT STICK(0)
30 FOR I=0 TO 300:NEXT I
40 GOTO 20
RUN

VEJA TAMBEM
PAD,
PDL,
STRIG,
INKEY$,
ON GOSUB,
ON GOTO

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("stop", "", "MSX-BASIC", "STOP", "Terminates a programs execution. When the interpreter reaches the last line of the...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
STOP

FUNCAO
Terminates a programs execution. When the interpreter reaches the last line of the program,
it is automaticly terminated, STOP is not necessary there. After STOP a CONT instruction
can continue the program execution on the next line.

EXEMPLO
10 PRINT "Test"
20 STOP
30 PRINT "CONT was used"
40 END
RUN

VEJA TAMBEM
END,
RUN,
CONT

REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
STOP ON

STOP OFF

STOP STOP

FUNCAO
Checks if the CTRL and STOP keys are pressed simultaneously.

After the "STOP" stamement MSX_Basic checks with every statement if the CTRL+STOP
keys are pressed simultaneously. If so the MSX-Basic program calls the subroutine
that was specified in the "ON STOP GOSUB" statement.

After the "STOP OFF" statement MSX-Basic does no no longer check if CTRL+STOP
keys are pressed simultaneously.

After the "STOP STOP" statement MSX-Basic checks if CTRL+STOP are pressed
simultaneously, but does not call the subroutine that was specified in the
"ON STOP GOSUB" statement. But the state is remembered. In this case the
subroutine is immediately called after the "STOP ON" statement is executed.

EXEMPLO
10 ON STOP GOSUB 50
20 STOP ON
30 INPUT A$
40 IF A$="END" THEN STOP OFF:END ELSE GOTO 30
50 PRINT "Enter END and press Return":RETURN
RUN
END
Ok

VEJA TAMBEM
ON STOP GOSUB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("str$", "", "MSX-BASIC", "STR$(number)", "Creates the alpanumeric representation of the numeric expression in number.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
STR$(number)

FUNCAO
Creates the alpanumeric representation of the numeric expression in number.

Use the "VAL" function to get the numeric representation of the alphanumeric expression.

EXEMPLO
10 AMOUNT=12.34216:PRINT AMOUNT
20 A$=STR$(AMOUNT)
30 H%=INSTR(A$,".")
40 IF H%=<1 GOTO 80
50 MID$(A$,H%,1)=","
60 H$=LEFT$(A$,H%+2)
70 PRINT H$
80 END
RUN
12.34216
12,34
Ok

VEJA TAMBEM
BIN$,
HEX$,
OCT$,
VAL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("strig", "", "MSX-BASIC", "STRIG(sticknumber)", "Returns the state of the Joysticks or space key.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
STRIG(sticknumber)

FUNCAO
Returns the state of the Joysticks or space key.

number is a number between 0 and 4 and has the following meaning:
0 | Space key
1 | Joystick in connector 1
2 | Joystick in connector 2
3 | Joystick in connector 1
4 | Joystick in connector 2
If the joystick or space key are activated the function returns -1.
If not 0 is returned.

EXEMPLO
10 PRINT "Hit the space key"
20 P=STRIG(0)
30 IF P=-1 THEN BEEP
40 GOTO 20
RUN

VEJA TAMBEM
PAD,
PDL,
STICK,
INKEY$

REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
STRIG(number) ON

STRIG(number) OFF

STRIG(number) STOP

FUNCAO
Checks the status if the joystick or space key.

number is a number between 0 and 4 and has the following meaning:
0 | Space key
1 | Joystick in connector 1
2 | Joystick in connector 2
3 | Joystick in connector 1
4 | Joystick in connector 2
After the "STRIG(number) ON" statement MSX-Basic checks with every statement
if the specified joystick button is pressed. If so MSX-Basic calls the subroutine,
that was specified in the "ON STRIG GOSUB" statement.

After the "STRIG(number) OFF" statement MSX-Basic no longer checks, if the
specified joystick button is pressed.

After the "STRIG(number) STOP" statement MSX-Basic checks if the specified
joystick button is pressed, but does not call the subroutine that was specified
in the "ON STRIG GOSUB" statement. But the state is remembered and the subroutine
is automatically called after the "STRIG(number) ON" statement.

EXEMPLO
See: ON STRIG GOSUB

VEJA TAMBEM
ON STRIG GOSUB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("string$", "", "MSX-BASIC", "STRING$(length, character)", "Creates a string with length characters, which consists only",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
STRING$(length, character)

FUNCAO
Creates a string with length characters, which consists only
of characters that was specified in character.

length is the length of the alpanumeric value and must be
an integer in range between 0 and 255.

character is the code of the character and must be an integer
between 32 and 255, or the corresponding character (string) directly.

EXEMPLO
10 SCREEN 1:COLOR 15,4,7
20 A$=CHR$(42)
30 FOR K=1 TO 22
40 C=INT(RND(1)*20)
50 LOCATE 3,K
60 PRINT USING "##";C
70 LOCATE 6,K
80 PRINT STRING$(C,A$)
90 NEXT K
100 END
RUN

VEJA TAMBEM
SPACE$,
SPC,
TAB

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("swap", "", "MSX-BASIC", "SWAP variable, variable2", "Exchanges the contents of two variables.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
SWAP variable, variable2

FUNCAO
Exchanges the contents of two variables.

Every type of variable (integer, single precision, double precision, alphanumeric)
can be used. The only restriction is that both variables are of the same type.

EXEMPLO
10 X=3:Y=7
20 GOSUB 60
30 SWAP X,Y
40 GOSUB 60
50 END
60 PRINT "X=";X;"Y=";Y
70 RETURN
RUN

VEJA TAMBEM
LET

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("tab", "", "MSX-BASIC", "TAB(position)", "Sets a tabulator. The cursor jumps to a pre-defined position in the same line.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
TAB(position)

FUNCAO
Sets a tabulator. The cursor jumps to a pre-defined position in the same line.

This function can only be used with "PRINT" and "LPRINT" statements.

number must be an integer between 0 and 255.

If the cursor has already reached the specified position the function is ignored.

EXEMPLO
10 PRINT "0";TAB(9);"9"
20 END
RUN

VEJA TAMBEM
PRINT,
LPRINT,
SPC,
SPACE$,
STRING$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("tan", "", "MSX-BASIC", "TAN(number)", "Returns the tangent of the specified number in radians.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
TAN(number)

FUNCAO
Returns the tangent of the specified number in radians.

EXEMPLO
10 X=5
20 PRINT TAN(X)
30 END
RUN
 -3.3805150062446
Ok

VEJA TAMBEM
ATN,
COS,
SIN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("time", "", "MSX-BASIC", "TIME", "Contains the internal system-time.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
TIME

FUNCAO
Contains the internal system-time.

Every time the video display processor creates an interrupt (every 1/60s or 1/50s),
the variable "TIME" is incremented by 1.

If no interrupt is generated (e.g. while saving a program to tape or loading from tape)
the "TIME" variable remains unchanged.

EXEMPLO
10 CLS
20 LOCATE 10,6
30 PRINT "HH:MM:SS START"
40 TIME=0
50 T=TIME
60 H=INT(T/180000)
70 T=T-(H*180000)
80 M=INT(T/3000)
90 T=T-(M*3000)
100 S=INT(T/50)
110 LOCATE 10,8
120 PRINT USING "##:##:##";H;M;S
130 GOTO 50
RUN

VEJA TAMBEM
RND,
GET TIME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("troff", "", "MSX-BASIC", "TROFF", "Disable tracing of the program execution.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
TROFF

FUNCAO
Disable tracing of the program execution.

The trace function is enabled with the "TRON" statement.
The trace function can also be disabled with the "NEW" statement.

EXEMPLO
See: TRON

VEJA TAMBEM
TRON,
NEW

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("tron", "", "MSX-BASIC", "TRON", "Enables the trace function.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
TRON

FUNCAO
Enables the trace function.

After the "TRON" statement MSX-Basic prints all program-lines
(that are executed during the program flow) in brackets.

EXEMPLO
10 FOR I=1 TO 3
20 PRINT I
30 NEXT
40 END
TRON
Ok
RUN
[10][20] 1
[30][20] 2
[30][20] 3
[30][40]
Ok
TROFF
Ok

VEJA TAMBEM
TROFF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("usr", "", "MSX-BASIC", "USR[<number>] (argument)", "Starts a machine-language program.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
USR[<number>] (argument)

FUNCAO
Starts a machine-language program.

number is an integer between 0 and 9. If number is not specified 0 will be used.

The start-address of the machine-language program must be specified by using the DEF USR statement.

argument is the value, that is passed to the machine-language subroutine.
It can be passed in the following ways:

For alphanumeric values

Memory address &HF663 contains the value 3.
Memory address &HF7F8 and &HF7F9 contains the address of the type of the alphanumeric value.
The type contains 3 bytes. The first byte is the length of the alphanumeric value, the second and
third contains the memory-address of the alphanumeric value.
For integer values

The memory address &HF663 contains the value 2.
Memory address &HF7F8 and &HF7F9 contains the value of the integer.
For single precision values

The memory address &HF663 contains the value 4.
Memory address &HF7F6 to &HF7F9 contains the single precision value.
For double precision values

The memory address &HF663 contains the value 8.
Memory address &HF7F6 to &HF7FD contains the double precision value.

Values that are returned from the machine-language to MSX-Basic have to be provided in
the same way from the machine-language program.

Use the "CLEAR" statement to reserve the necessary memory for the machine-language program.

EXEMPLO
DEF USR1=&H156:A=USR1(0)

10 CLEAR 200,&HEFFF
20 AB=&HF000
30 FOR I=AB TO AB+9
40 READ A$:A=VAL("&H"+A$)
50 POKE I,A
60 NEXT I
70 DEFUSR=&HF000
80 INPUT "Enter an integer";A%
90 PRINT "The integer is";A%
100 R=USR(A%)
110 PRINT "Result is the integer plus 1";R
120 END
130 DATA 23,23,4E,23,46,03,70,2B,71,C9
RUN

VEJA TAMBEM
DEFUSR,
BLOAD,
POKE,
VARPTR,
CLEAR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("val", "", "MSX-BASIC", "VAL(string)", "Returns the value of the alphanumeric expression in string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
VAL(string)

FUNCAO
Returns the value of the alphanumeric expression in string.

The function ignores spaces and control-characters in the expression.

The "STR$" function can be used to get an alphanumeric representation of a numeric expression.

EXEMPLO
10 AMOUNT=12.34216:PRINT AMOUNT
20 A$=STR$(AMOUNT)
30 H%=INSTR(A$,".")
40 IF H%=<1 GOTO 80
50 H%=H%+2
60 B$=LEFT$(A$,H%)
70 AMOUNT=VAL(B$):PRINT AMOUNT
80 END
RUN
12.34216
12.34
Ok

VEJA TAMBEM
BIN$,
HEX$,
OCT$,
STR$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("varptr", "", "MSX-BASIC", "VARPTR(variable)", "Returns the memory-address of the first byte of a variable",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic, RS232-Basic)

SINTAXE
VARPTR(variable)

VARPTR(#channel)

FUNCAO
Returns the memory-address of the first byte of a variable
or the first byte of a file-control-block (of an open file).

The first format returns the memory-address of a variable. This variable
must be already initialized with a value, otherwise an "Illegal function call" will be generated.

Every type of variable (integer, single precision, double precision, alphanumeric)
and dimensioned variables (defined by the "DIM" statement) can be used with the
"VARPTR" function.

Before the address of a dimensioned variable can be accessed its necessary to assign
a value to all non-dimensioned variables. This is required because adresses of
dimensioned variables will change if new not-dimensioned variables are used.

The function "VARPTR" is most recently used to get the memory-address of a variable,
which should be passed to an assembly-language subroutine.

The second format returns the first address of the file-control-block of a file.

channel is the file-number of the file, which was opened with the "OPEN" statement.

The "VARPTR" function returns a value between -32768 and 32767. If a negative value is
given 65536 is added first to get the actual address.

EXEMPLO
10 A=10
20 B=VARPTR(A)
30 IF B<0 THEN B=B+65536
40 C$="0000"+HEX$(B)
50 PRINT RIGHT$(C$,4)
60 END
RUN
C059
Ok

VEJA TAMBEM
PEEK,
POKE,
DEFUSR,
USR,
DIM,
OPEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("vdp", "", "MSX-BASIC", "VDP(register)", "Contains the value of the specified VDP register (video display processor).",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
VDP(register)

FUNCAO
Contains the value of the specified VDP register (video display processor).

register is the register number and must be an integer between 0 and 8.
The content of register 8 is read-only and cannot be changed.

MSX2: registers 9 to 24 and 33 to 47 are added.

MSX2+ registers 26-28 are added.

USE THIS VARIABLE ONLY IF YOU ARE FAMILIAR WITH THE VDP (VIDEO DISPLAY PROCESSOR).

EXEMPLO
10 FOR I=0 TO 8
20 A=VDP(I)
30 B$="00000000"+BIN$(A)
40 PRINT RIGHT$(B$,8)
50 NEXT I
60 END
RUN

VEJA TAMBEM
BASE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("vpeek", "", "MSX-BASIC", "VPEEK(adress)", "Returns the value of a byte within the video-memory.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
VPEEK(adress)

FUNCAO
Returns the value of a byte within the video-memory.

The function returns a byte-value for the specified address and is
therefore in range between 0 and 255.

address is the address within the video-memory and must be in range between 0 and 16383
(or -32768 and 32767).

The statement "VPOKE" can be used to write a byte into the video memory.

MSX2: the size of video-memory has been increased from 16KB to 64KB and more,
therefore address can be in range from -32768 to 32767.

USE THIS FUNCTION ONLY IF YOU ARE FAMILIAR WITH THE VDP (VIDEO DISPLAY PROCESSOR)

EXEMPLO
10 A=VPEEK(0)
20 A$="00"+HEX$(A)
30 PRINT RIGHT$(A$,2)
40 END
RUN

1000 REM
1010 REM Hardcopy of screen 0
1020 REM
1030 LPRINT CHR$(13):FOR I=0 TO 959
1040 LPRINT CHR$(VPEEK(I));
1050 IF I>0 AND (I+1)/40=INT((I+1)/40) THEN LPRINT CHR$(13);
1060 NEXT I

VEJA TAMBEM
VPOKE,
PEEK,
SCREEN,
SET PAGE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("vpoke", "", "MSX-BASIC", "VPOKE adress,value", "Writes a byte into the video-memory.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
VPOKE adress,value

FUNCAO
Writes a byte into the video-memory.

address is the address within the video-memory and must be
in range between 0 and 16383 (or -32768 and 32767).

value is the byte value and must be between 0 and 255.

The function "VPEEK" can be used to read a byte from video-memory.

MSX2: the size of video-memory has been increased from 16KB to 64KB and more,
therefore address can be in range from -32768 to 32767.

USE THIS FUNCTION ONLY IF YOU ARE FAMILIAR WITH THE VDP (VIDEO DISPLAY PROCESSOR)

EXEMPLO
10 VPOKE 0,(VPEEK(0))
20 END
RUN

VEJA TAMBEM
VPEEK,
POKE,
SCREEN,
SET PAGE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("wait", "", "MSX-BASIC", "WAIT port, xormask[, andmask]", "Monitors the status of a machine input-port port.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
WAIT port, xormask[, andmask]

FUNCAO
Monitors the status of a machine input-port port.

While this statement is executed data from the port is read
and compared with xormask (on XOR base) and andmask
 (on AND base)
.
If the result of this comparison is 0, then MSX-Basic reads the port
again and compares the data. In all other cases the program will be continued.

If andmask is not specified the value 0 will be assumed.

EXEMPLO
See: INP and OUT

VEJA TAMBEM
INP,
OUT,
XOR,
AND

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("width", "", "MSX-BASIC", "WIDTH textwidth", "Specifies the number of positions in a single screenline for textmode 1 and 2.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
WIDTH textwidth

FUNCAO
Specifies the number of positions in a single screenline for textmode 1 and 2.

textwidth is the number of positions per line and must be an integer between 1 and 40
(or 1 and 80) in textmode 1 and an integer between 1 and 32 in textmode 2.

If MSX-Basic starts Textmode 1 uses "WIDTH 37" and textmode 2 uses "WIDTH 19".

MSX2: the textmode 1 can now have up to 80 characters, therefore the textwidth
can be in range between 1 and 80. If textwidth is > 40 the 80 character mode is
activated, if textwidth is <= 40 the 40 character mode is activated.

The width can also be saved by using the "SET SCREEN" statement, so the system will restore
the changed defaults on power-up.

EXEMPLO
5 SCREEN 0
10 FOR I=37 TO 10 STEP -1
15 WIDTH I
20 PRINT "This is a line of text on the screen"
25 FOR X=1 TO 300:NEXT
30 REM line 25 is a time-delay
35 NEXT I
40 WIDTH 40
45 REM line 40 restores the old format again
50 END

VEJA TAMBEM
SCREEN,
SET SCREEN,
CALL KANJI

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("xor", "", "MSX-BASIC", "number1 XOR number2", "Logical exclusive or.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 1 (MSX))

(MSX-Basic)

SINTAXE
number1 XOR number2

FUNCAO
Logical exclusive or.
0 XOR 0 | = | 0
0 XOR 1 | = | 1
1 XOR 0 | = | 1
1 XOR 1 | = | 0

VEJA TAMBEM
AND,
EQV,
IMP,
MOD,
NOT,
OR,
MSX-Basic operators

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call mname", "", "MSX-BASIC", "CALL MNAME (\"old filename\" AS \"new filename\")", "This statement renames the specified file.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
CALL MNAME ("old filename" AS "new filename")

FUNCAO
This statement renames the specified file.

VEJA TAMBEM
NAME,
CALL MKILL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call memini", "", "MSX-BASIC", "CALL MEMINI [(size)]", "This statement specifies the amount of memory to be used as a RAM disk, initialises the...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
CALL MEMINI [(size)]

FUNCAO
This statement specifies the amount of memory to be used as a RAM disk, initialises the RAM disk, and deletes all files. When the RAM disk is to be used, this statement should always be executed.

Size is "the amount of memory to be used as RAM disk minus 1". By default, the maximum size is allocated for RAM disk. "CALL MEMINI(0)" causes the RAM disk feature to be disabled.

EXEMPLO
CALL MEMINI(&H3FFF)

See also:

CALL MFILES,
CALL RAMDISK
RAM-Disk in MSX-Basic 2,
RAM-Disk in Disk-Basic 2,
RAM-Disk in MSX-DOS 2

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call mfiles", "", "MSX-BASIC", "CALL MFILES", "This statement displays file names on the RAM disk.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
CALL MFILES

FUNCAO
This statement displays file names on the RAM disk.

VEJA TAMBEM
FILES,
LOAD,
CALL MEMINI

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call mkill", "", "MSX-BASIC", "CALL MKILL(\"filename\")", "This statement deletes the specified file.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
CALL MKILL("filename")

FUNCAO
This statement deletes the specified file.

VEJA TAMBEM
KILL,
CALL MNAME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("color=", "", "MSX-BASIC", "COLOR=(palette no., red luminance, green lumincane, blue luminance)", "You can express 512 kind of colors by combining red, green and blue luminance. If you do...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
COLOR=(palette no., red luminance, green lumincane, blue luminance)

Palette no. Condition: when screen mode is 0 to 7 or 10: 0 to 15. Numerical constant variable, array variable or expression.

Red/green/blue luminance: 0 to 7. Integer constant

FUNCAO
You can express 512 kind of colors by combining red, green and blue luminance. If you do not execute this command to a palette no, it's color stays default.

Note: use CALL PALETTE statement to specify palette color under Japanese input mode. If you try to use this COLOR statement during Japanese input mode, you will get "Illegal function call". (Same is true with COLOR=NEW and COLOR=RESTORE statements).

EXEMPLO
100 SCREEN 5
110 FOR I=7 to 0 STEP -1
120 CIRCLE(120,100),I*10+5,I
130 PAINT(120,100),I,1
140 NEXT I
150 J=(J+1) MOD 8
160 FOR I=0 to 7
170 COLOR=(I,0,J,0)
180 J=(J+1) MOD 8
190 NEXT I
200 GOTO 150

VEJA TAMBEM
CALL PALETTE,
COLOR=RESTORE,
COLOR=NEW,
DRAW,
LINE,
CIRCLE,
PSET,
PAINT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("color=new", "", "MSX-BASIC", "COLOR=NEW", "This statement initialises the colour palette to the same state",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
COLOR=NEW

FUNCAO
This statement initialises the colour palette to the same state
as when the power of the computer is turned on (see Color-table).
It is a good idea to place this statement at the beginning and the end of the program.

EXEMPLO
COLOR=NEW

VEJA TAMBEM
COLOR=,
COLOR,
COLOR=RESTORE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("color=restore", "", "MSX-BASIC", "COLOR=RESTORE", "This commands substitues contents of \"Color Lookup Table\" inside VRAM, into VDP color...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
COLOR=RESTORE

FUNCAO
This commands substitues contents of "Color Lookup Table" inside VRAM, into VDP color palette registers.

Usage:

COLOR=RESTORE (no options whatsoever; just this form)

How it works, and how you use this:

If you BSAVE with ,S option, you can save the contents of VIDEO RAM to the disk along with color palette infos. If you BLOAD with ,S option, you can load such palette data to VRAM. However, such color palette info is only set into lookup table, and do not make any changes to the actually displayed colors. If you execute COLOR=RESTORE, then you can set the info inside lookup table as color palette, and you can display the picture in the same color at the time you saved with BSAVE,S.

EXEMPLO
10 SCREEN 2
20 BLOAD "BOX",S
30 COLOR=RESTORE
40 GOTO 40

VEJA TAMBEM
COLOR=,
COLOR,
COLOR=NEW

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("color sprite", "", "MSX-BASIC", "COLOR SPRITE(<plane>)=color", "Sets the color to the sprite of plane to the specified color.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
COLOR SPRITE(<plane>)=color

FUNCAO
Sets the color to the sprite of plane to the specified color.

This statement sets the whole sprite of the specified plane to the
color, this uses color for color specification. The format of
the color specification is valid for screen mode 4 through 8 (respectively 12).

Bitwise meanings of color:
Bit 7 | Not used (disabled).
Bit 6 | If 1, move the sprites of the successive planes together.
                  The priority and conflict of sprites are ignored, and when
                  sprites are piled up, they are displayed in the color
                  which is OR-ed with their color numbers.
                  For example, assuming that bit 6 of sprite plane 1 is "0" and bit 6 of
                  sprite plane 2 is "1", only by moving sprite plane 1, will sprite plane 2 be
                  displayed displayed to be piled at the same location.
Bit 5 | If 1, the conflict of sprites are ignored.
Bit 4 | Unused.
Bits 0-3 | Palette number (see color-table).

EXEMPLO
COLOR SPRITE(0)=15

VEJA TAMBEM
COLOR,
COLOR SPRITE$,
PUT SPRITE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("color sprite$", "", "MSX-BASIC", "COLOR SPRITE$(<plane>)=string", "Sets the colour of each horizontal line of the sprite using string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
COLOR SPRITE$(<plane>)=string

FUNCAO
Sets the colour of each horizontal line of the sprite using string.

string consists of one to sixteen characters. Bits 0 throgh 3 of
the character's ASCII code are used for the colour specification, and bits 4
through 7 are used to specify each function of the sprite.

These specifications are valid only for screen modes 4 through 8 (respectively 12).

Bitwise meanings of string expression:
Bit 7 | If 1, move the sprite to left by 32 dots.
Bit 6 | If 1, move the sprites of the successive planes together.
                  The priority and conflict of sprites are ignored, and when
                  sprites are piled up, they are displayed in the color
                  which is OR-ed with their color numbers.
                  For example, assuming that bit 6 of sprite plane 1 is "0" and bit 6 of
                  sprite plane 2 is "1", only by moving sprite plane 1, will sprite plane 2 be
                  displayed displayed to be piled at the same location.
Bit 5 | If 1, the conflict of sprites are ignored.
Bit 4 | Unused.
Bits 0-3 | Palette number (see color-table).

Relation of the sprite and string:

COLOR SPRITE$ = CHR$ (color of the first line) + CHR$ (color of the second
line) + ...... + CHR$ (color of the eight line)

       ---------------------------------
Line 1 --> | * |   |   | * | * |   |   | * |
       |---+---+---+---+---+---+---+---|
Line 2 --> | * | * |   |   |   |   | * | * |
       |---+---+---+---+---+---+---+---|
       |   |   |   |   |   |   |   |   |
       |---+---+---+---+---+---+---+---|
       | * | * |   |   |   |   | * | * |
       |---+---+---+---+---+---+---+---|
       |   |   | * |   |   | * |   |   |
       |---+---+---+---+---+---+---+---|
       | * |   |   | * | * |   |   | * |
       |---+---+---+---+---+---+---+---|
       | * | * |   |   |   |   | * | * |
       |---+---+---+---+---+---+---+---|
Line 8 --> |   |   | * | * | * | * |   |   | The color for each line
       --------------------------------- can be set.

EXEMPLO
COLOR SPRITE$(0)=STRING$(16,&HFF)

VEJA TAMBEM
COLOR,
COLOR SPRITE,
PUT SPRITE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("get date", "", "MSX-BASIC", "GET DATE string-variable [,A]", "This statement is for reading the date from the timer and assigning it to the string...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
GET DATE string-variable [,A]

FUNCAO
This statement is for reading the date from the timer and assigning it to the string variable.
The format of date to be read is as follows:

Japanese: | YY/MM/DD | (YY = lower two digits of year, MM = month, DD = day)
      e.g. 85/03/23 (March 23, 1985)
International: | MM/DD/YY | (YY = lower two digits of year, MM = month, DD = day)
      e.g. 03/23/85 (March 23, 1985)
Korean: | DD/MM/YY | (YY = lower two digits of year, MM = month, DD = day)
      e.g. 23/03/85 (March 23, 1985)

When option A is specified, the alarm date is read.

EXEMPLO
GET DATE D$
PRINT D$

VEJA TAMBEM
GET TIME,
SET DATE,
CALL GREG

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("get time", "", "MSX-BASIC", "GET TIME string-variable [,A]", "This statement is for reading time from the timer and assigning it to a string variable.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
GET TIME string-variable [,A]

FUNCAO
This statement is for reading time from the timer and assigning it to a string variable.
The form of time to be read is as follows:

HH:MM:SS(HH = hour, MM = minute, SS = second)

e.g. 22:15:00(22 hours 15 minutes 0 seconds)

When A is specified, the time for the alarm time is read.

EXEMPLO
GET TIME T$
PRINT T$

VEJA TAMBEM
GET DATE,
SET TIME,
TIME,
CALL TIME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set adjust", "", "MSX-BASIC", "SET ADJUST(x,y)", "This statement sets the location to display on the screen.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET ADJUST(x,y)

FUNCAO
This statement sets the location to display on the screen.
The coordinate offset is from -7 to 8.

EXEMPLO
SET ADJUST(0,0)

VEJA TAMBEM
SET SCROLL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set beep", "", "MSX-BASIC", "SET BEEP timbre, volume", "This statement sets BEEP sound. Timbre and volume are from 1 to 4.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET BEEP timbre, volume

FUNCAO
This statement sets BEEP sound. Timbre and volume are from 1 to 4.
The table below shows the correspondance of timbre and to the actual sound.

Timbre | Sound
1 | High tone beep (same as MSX1)
2 | Low tone beep
3 | 2 - tone beep
4 | 3 - tone beep

VEJA TAMBEM
BEEP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set date", "", "MSX-BASIC", "SET DATE \"date\" [,A]", "This statement sets date to timer. The form of parameter and option is the same as",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET DATE "date" [,A]

FUNCAO
This statement sets date to timer. The form of parameter and option is the same as
"GET DATE". When option A is specified, the alarm date is set.

EXEMPLO
SET DATE "85/03/23"

VEJA TAMBEM
GET DATE,
SET TIME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set page", "", "MSX-BASIC", "SET PAGE [visible][,work]", "This statement allows users to set the page to display and the page to read and write...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET PAGE [visible][,work]

FUNCAO
This statement allows users to set the page to display and the page to read and write data to.
This is valid when the screen mode is greater than 4, and the value specified depends on the
VRAM capacity and the screen mode:

Screen mode | VRAM 64K | VRAM 128K
SCREEN 5 | 0 to 1 | 0 to 3
SCREEN 6 | 0 to 1 | 0 to 3
SCREEN 7 | Unusable | 0 to 1
SCREEN 8 | Unusable | 0 to 1
SCREEN 10 | Unusable | 0 to 1
SCREEN 11 | Unusable | 0 to 1
SCREEN 12 | Unusable | 0 to 1

VEJA TAMBEM
BSAVE,
BLOAD,
COPY,
SCREEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set password", "", "MSX-BASIC", "SET PASSWORD \"password\"", "This statement sets a system password.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET PASSWORD "password"

FUNCAO
This statement sets a system password.
Password is a string expression up to 255 characters.
Once this statement is done, input of the password is requested for invoking the system.
When the correct password is given, the system is normally invoked;
otherwise, correct password input is requested. When the system is invoked by
pressing both graphic key and stop key, no password input is requested
(in this case, the password setting has been done by the key cartridge;
however, password input is always required for system startup).
The password is disabled by specifying a null character in SET TITLE.

VEJA TAMBEM
SET TITLE,
SET PROMPT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set prompt", "", "MSX-BASIC", "SET PROMPT \"prompt\"", "This statement sets the prompt. Prompt can have up to 6 characters.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET PROMPT "prompt"

FUNCAO
This statement sets the prompt. Prompt can have up to 6 characters.

EXEMPLO
SET PROMPT "Ready"

VEJA TAMBEM
SET PASSWORD,
SET TITLE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set screen", "", "MSX-BASIC", "SET SCREEN", "This statement records the current parameters of the \"SCREEN\" statement.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET SCREEN

FUNCAO
This statement records the current parameters of the "SCREEN" statement.
At the system startup, they are automatically set.
Items to be recorded are the following:

Screen number of text mode
Key click switch
Screen width of text mode
Printer option
Foreground, background, and border colours
Cassette baud rate
Function key switch
Display mode

VEJA TAMBEM
SCREEN,
WIDTH

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set time", "", "MSX-BASIC", "SET TIME \"time\" [,A]", "This statement sets the time to the timer.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET TIME "time" [,A]

FUNCAO
This statement sets the time to the timer.
The form of parameter and option is the same as "GET TIME".

e.g.) SET TIME "22:15:00"

When option A is specified, the alarm time is set.

VEJA TAMBEM
SET DATE,
GET TIME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set title", "", "MSX-BASIC", "SET TITLE \"greeting\" [, color]", "This statement specifies the title and the colour of the initial screen at system startup.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET TITLE "greeting" [, color]

FUNCAO
This statement specifies the title and the colour of the initial screen at system startup.
The title is set by a string of up to 6 characters and color is one of the values
listed below. When title is 6 characters, keyboard input is awaited just after the
title screen is displayed.

Color | Screen color
1 | Blue
2 | Green
3 | Red
4 | Orange

EXEMPLO
SET TITLE "Hello"

VEJA TAMBEM
SET PROMPT,
SET PASSWORD

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set video", "", "MSX-BASIC", "SET VIDEO [mode[, Ym[, CB[, sync[, voice[, video input[, AV control]]]]]]]", "With this instruction is possible to define the so called 'superimpose mode'.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2 (MSX2))

(MSX-Basic 2)

SINTAXE
SET VIDEO [mode[, Ym[, CB[, sync[, voice[, video input[, AV control]]]]]]]

FUNCAO
With this instruction is possible to define the so called 'superimpose mode'.
Superimpose means that images could mix together (one on another).

Attention: This statement is for the superimposer or the digitiser which are optional,
so it can be used only for machines which are equipped with this the 'superimpose' capability.

Mode sets the superimposing mode and can be set to the value listed in
the following table. This indicates where the image comes from:
Mode | S1 | S2 | TP | Display screen
0 | 0 | 0 | 0 | Computer
1 | 0 | 1 | 1 | Computer
2 | 0 | 1 | 0 | Superimpose (together with another image, one on the other)
3 | 1 | 0 | 0 | Television (from the input of the video)
Note: In the case of mode 0, external sync cannot be used. In other modes the
compoalte output of VDP is not available. S1, S0, and TP are the names of
flags in the VDP register.

Ym specified the intensity (brightness) of the television.
If not specified the value defaults to 0.
Ym | brightness
0 | half
1 | full

When CB is 1, the colour bus of VDP is prepared for input, and, when 0, it
is prepared for output. If not specified the value defaults to 0.
CB | color-control
0 | only for output
1 | only for input

When sync is 1, "external sync" is selected, and, when 0, "internal sync"
is selected. If not specified the value defaults to 0.
sync | synchronisation
0 | internal
1 | external

Voice specifies whether to mix external signal for output, and values are
listed in the following table. If not specified the value defaults to 0.
Voice | Function for external voice signal
0 | No mixing
1 | Right channel mixed
2 | Left channel mixed
3 | Both channels mixed

Video input is used to alternate the input of external video signals. When
it is 0, the RGB multiconnector is selected; when it is 1, external video
signal connector is selected. If not specified the value defaults to 0.
Video input | external input
0 | Euroconnector RGB (SCART)
1 | Connector TV

AV control specifies AV control terminal output of the RGB multiconnector.
When it is 0, the output is OFF; when it is 1, the output is ON.
If not specified the value defaults to 0.
AV control | audio/video output of the euroconnector RGB (SCART)
0 | not selected
1 | selected

EXEMPLO
SET VIDEO 2

VEJA TAMBEM
COPY SCREEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("set scroll", "", "MSX-BASIC", "SET SCROLL [x ] [,y]", "Performs horizontal and vertical scrolling of the screen-contents in graphic-mode.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC 2+ (MSX2+))

(MSX-Basic 3)

SINTAXE
SET SCROLL [x ] [,y]

FUNCAO
Performs horizontal and vertical scrolling of the screen-contents in graphic-mode.

This is a MSX2+ only command and supports the hard scroll of the V9958. <X coordinate> is between 0 and 511, <Y coordinate> is between 0 and 255.
Leftmost 8 dots part of the screen is not displayed if the <left mask> is 1, displayed if this value is 0. If the <2 page sequential> is 1,
two sequential pages (for instance 0 and 1) are treated as a horizontally connected page or if this value is 0, the scroll takes place only in a single page.
To execute 2 page sequential display, the display page must be a odd numbered page. The hard scroll can be done only with SCREEN 1 and later, and 2 page
sequential display can be done only with SCREEN 5 and later.

Notes:

Under screen 0 mode 0-7 is specifyable as X-distance. If 0 (or omitted), horizontal scroll does not happen. If a value between 1 and 7 is specified, 1 causes 7 pixels, 2 causes 6 pixels, ... 7 causes 1 pixel of scroll to right.

0 to 7 is specifiyable as Y-distance. If 0 (or omitted), vertical scroll does not happen. If a value between  and 7 is specified, 1 causes 1 pixel, 2 causes pixels, ... 7 causes 7 pixels of scroll to up. "Vertical scroll" does not move displayed characters themselves, but instead causes scrolling inside each of the horizontal character rows.

After scrolling, (if horizontal scroll was specified) entire displayed screen is displaced to X-axial direction and (if vertical scroll was) character patterns are scrolled Y-axially inside each of the rows. Scrolled positions return to initial conditions if you re-set the screen mode by using SCREEN statement.

EXEMPLO
SET SCROLL ,1

VEJA TAMBEM
SET ADJUST

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call pause", "", "MSX-BASIC", "CALL PAUSE(time)", "Stops execution of BASIC program for the specified time. The unit of the pause time is...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC Turbo-R)

(MSX-Basic 4)

SINTAXE
CALL PAUSE(time)

FUNCAO
Stops execution of BASIC program for the specified time. The unit of the pause time is millisecond.
When the CALL PAUSE is being executed, interrupt is enabled. Even when the CALL PAUSE is being executed,
you can quit the program by the CTRL + STOP keys. This command is used when you want to take a timing
without being affected by the CPU execution speed.

EXEMPLO
CALL PAUSE(1000)

NOTA
the Chakkari Copy cartridge provides also a CALL PAUSE statement, which performs a different function.

REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Chakkari Copy)

SINTAXE
CALL PAUSE

FUNCAO
Puts the Chakkari Copy cartridge into PAUSE mode.
In this mode it's only possible to print a hardcopy by pressing the COPY button
or to resume execution by pressing the PAUSE button.

It's the same operation as pressing the PAUSE button on the Chakkari Copy cartridge.

Note: the paused program can only be resumed by pressing the PAUSE button. It cannot be resumed by software.

EXEMPLO
CALL PAUSE

IMPORTANT NOTE
the MSX-Turbo R provides also a CALL PAUSE statement,
which usually supersedes this functionality. It performs another function and has a different syntax.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call pcmplay", "", "MSX-BASIC", "CALL PCMPLAY(@startaddr, endaddr, samplerate, ramtype [,S])", "Play a PCM sound.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC Turbo-R)

(MSX-Basic 4)

SINTAXE
CALL PCMPLAY(@startaddr, endaddr, samplerate, ramtype [,S])

CALL PCMPLAY(arrayname, [length], samplerate)

CALL PCMPLAY(@start address, end address, sampling rate)
for playing from the main RAM
CALL PCMPLAY(@start address, end address, sampling rate,S)
for playing from the VRAM
CALL PCMPLAY(array variable name, [length], sampling rate)
for playing from array variables

FUNCAO
Play a PCM sound.

Treats the contents of either the main RAM or the VRAM as a PCM data and plays it by the specified sampling frequency.

- Sampling rate

Specifies the sampling rate.

Sampling rate | Meaning (KHz)
0 | 15.75
1 | 7.875
2 | 5.25
3 | 3.9375

- Length

You can omit this. If you omit, all of the contents of the array variable is played.

The format of the data is absolute binary and 1 to 255 is a normal data. 0 is a special data and causes 0 level (127) to be outputted by a number of the cycles specified by 1 byte proceeding the 0.

Caution

If the mode is Z80 mode, the mode is automatically switched to the R800 mode before the execution, and after finishing the Z80 mode is restored. If the STOP key is pushed during the playing, the program execution is quit and returns to BASIC.

Be careful that in the MSX-BASIC, the hexadecimal number you can express with &H is a four digit number between &H000 and &HFFFF and that &H8000 to &HFFFF are the negative numbers. This does not cause any problem when main RAM is specified but requires caution when VRAM is specified. For instance, to play 64KB part in the latter half of the VRAM, you must specify like,

CALL PCMPLAY(@65536,131071,2,S)

(131071=65536x2-1). If you specify like,

CALL PCMPLAY(@&H10000,&H1FFFF,2,S)

this results in "Overflow" error. On the other hand,

CALL PCMPLAY(&HFFFF+1,&1FFFFx2+1,2,S)

(Translator's note: This is obviously typo of CALL PCMPLAY(&HFFFF+1,&1FFFF*2+1,2,S) )

results same with CALL PCMPLAY(@0,&HFFFF,2,S) since &HFFFF+1=-1+1=0 and &HFFFx2+1=-1x2+1=&HFFFF .

EXEMPLO
CALL PCMPLAY(@&HB000,&HDFFF,1)

VEJA TAMBEM
CALL PCMREC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call pcmrec", "", "MSX-BASIC", "CALL PCMREC(@start address, end address, sampling rate,[,[trigger level][,compre", "Records sounds as PCM.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC Turbo-R)

(MSX-Basic 4)

SINTAXE
CALL PCMREC(@start address, end address, sampling rate,[,[trigger level][,compression switch]])

for recording to the main RAM

CALL PCMREC(@start address, end address, sampling rate, [trigger level], [compression switch],S)

for recording to the VRAM

CALL PCMREC(array variable name, [length],sampling rate[,[trigger level][, compression switch]])

for recording to array variables

FUNCAO
Records sounds as PCM.

Samples sound by the PCM method and by the specified frequency, and records in either main RAM, VRAM or array variable.

- Sampling rate

Specifies the sampling rate.

Sampling rate | Meaning (KHz)
0 | 15.75
1 | 7.875
2 | 5.25
3 | 3.9375

- Trigger level

Specifies the input level when starting the recording. The range of this value is 0 to 127. If this is omitted or is 0, the recording is started immediately.

- Compression switch

Specifies whether or not to execute the silent data compression.

Value | Meaning
Omitted | Does not compress
0 | Does not compress
1 | Compress

The format of the data is absolutely binary and 1 to 255 is normal data. When two or more data near the 0 level (126 to 128) are consecutive, the data can be compressed by recording a 0 and count of the consecutive data. This compression is called silent data compression.

Caution

If the mode is Z80 mode, the mode is automatically switched to the R800 mode before the execution, and after finishing the Z80 mode is restored.
If you specify the sampling rate to 15.75 KHz under Z80 mode or R800 ROM mode, you get "Illegal function call" error. Note if the STOP key is pushed during the playing, the program execution is quit and returns to BASIC.
When recording to the VRAM, be careful with the address you specify, in the same way with CALL PCMPLAY.

EXEMPLO
CLEAR 300,&HB000
CALL PCMREC(@HB000,&HDFFF,1,32,1)

VEJA TAMBEM
CALL PCMPLAY

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call chdir", "", "MSX-BASIC", "CALL CHDIR(\"[drive+]path\")", "Set current directory.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic 2)

SINTAXE
CALL CHDIR("[drive+]path")

FUNCAO
Set current directory.

Each drive has it's own current directory. This remains at the directory
specified  by the last CHDIR command for that drive (or at the root directory
initially) until  another CHDIR command is given or it cannot be found on the
disk when it is accessed (because the disk has been changed, for example). It
is then returned to the root directory.

EXEMPLO
CALL CHDIR("WORK")

VEJA TAMBEM
CALL MKDIR,
CALL RMDIR,
CALL CHDRV

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call chdrv", "", "MSX-BASIC", "CALL CHDRV(\"driveletter\")", "Switch default drive.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic 2)

SINTAXE
CALL CHDRV("driveletter")

FUNCAO
Switch default drive.

EXEMPLO
CALL CHDRV("H:")

VEJA TAMBEM
CALL CHDIR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call format", "", "MSX-BASIC", "CALL FORMAT", "Formats a disk. The CALL FORMAT statement prompts for the drive,",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
CALL FORMAT

FUNCAO
Formats a disk. The CALL FORMAT statement prompts for the drive,
that contains the disk to be formatted.

If the disk-drive supports multiple disk formats the CALL FORMAT
statement pompts for the disk-format to be choosen (can be e.g.
single sided or double-sided disk).

Notes:

A disk must be formatted before it can be used
Formatting irreversibly destroys prior contents of the disk

Harddisk: CALL FORMAT write-protects and write-enables the selected
partition of a harddisk. To create/delete and format partitions the
corresponding FDISK utility (on MSX-DOS disk) has to be used.

EXEMPLO
CALL FORMAT
Drive name? (A,B) a

1 - Single sided, 9 sectors
2 - Double sided, 9 sectors

? 1
Strike a key when ready
Format complete
Ok

Note: the printed prompts might be different, depending on the used
disk-controller and connected drives.

CALL FORMAT
Drive name? (A,B,C) A

1 - Write protect partition
2 - Write enable partition

?

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call mkdir", "", "MSX-BASIC", "CALL MKDIR(\"[drive+]path\")", "Create new subdirectory.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic 2)

SINTAXE
CALL MKDIR("[drive+]path")

FUNCAO
Create new subdirectory.

The  last item in the path is the name of the new sub-directory which is
to be  created on  the current  or specified  drive. Thus if this is the only
item  in the  path, the new directory is created in the current directory.

     When a  new directory  is created,  it is  empty except  for two special
sub-directories  called '.'  and '..'. These are automatically created in the
directory and  it is  these that allow '.' and '..' to be given in path names
to  signify the current and parent directories respectively (see chapter 3 on
Notation for a description of paths).

EXEMPLO
CALL MKDIR("WORK")

VEJA TAMBEM
CALL CHDIR,
CALL RMDIR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call ramdisk", "", "MSX-BASIC", "CALL RAMDISK[([number][, variable name])]", "Set size of RAM disk or assign it into a variable.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic 2)

SINTAXE
CALL RAMDISK[([number][, variable name])]

FUNCAO
Set size of RAM disk or assign it into a variable.

The  number, if  given, specifies the maximum size for the new RAM disk,
and is  specified in kilobytes. The range is 0 to 4064. If the number is 0 the
RAM disk  will be  deleted. This  number will be
rounded up  to the  nearest multiple  of 16K  since the  RAM disk is always a
multiple  of 16K.  A RAM  disk smaller then the specified maximum size may be
created if there is not enough free memory for the full size, although a 'not
enough memory' error will be given if there is no memory at all available for
the RAM  disk. Note that the number specified is the maximum amount of RAM to
use  for the  RAM disk,  which is  not the same as the maximum amount of free
space available  on the  newly-created RAM disk since the system needs to use
some for FAT or directories.

Having created a RAM disk, it can be referred to as drive H:.

EXEMPLO
CALL RAMDISK(32)
CALL RAMDISK(1000, A)

VEJA TAMBEM
CALL MEMINI

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call rmdir", "", "MSX-BASIC", "CALL RMDIR(\"[drive+]path\")", "Remove one or more subdirectories.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic 2)

SINTAXE
CALL RMDIR("[drive+]path")

FUNCAO
Remove one or more subdirectories.

 In  order to delete a directory, it must contain no other files or other
directories except  for the special '.' and '..' directories which are always
contained in a directory. These are put in a new directory when it is created
and cannot be removed. It is these that allow '.' and '..' to be used in path
names to specify the current and parent directories respectively.

EXEMPLO
CALL RMDIR("WORK")

VEJA TAMBEM
CALL CHDIR,
CALL MKDIR,
CALL MKILL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call system", "", "MSX-BASIC", "CALL SYSTEM", "CALL SYSTEM passes the control back to MSX-DOS. The command",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
CALL SYSTEM

CALL SYSTEM[("DOS command name")]

FUNCAO
CALL SYSTEM  passes the  control back to MSX-DOS. The command
name  may be  given to  specify the  operation to be executed
after  the  control returns  to DOS.
MSX-DOS 2: If no  command name  is specified,
REBOOT.BAT  in  the  root directory  on the  boot drive,
if any, will be executed.

Note: this command is only available if the computer was
started in MSX-DOS mode and MSX-Basic has been entered with the
BASIC command.

EXEMPLO
CALL SYSTEM

VEJA TAMBEM
MSX-DOS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("copy", "", "MSX-BASIC", "COPY <sourcefile> [TO <targetfile>]", "COPY duplicates a file, which can also be created on a different disk.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
COPY <sourcefile> [TO <targetfile>]

FUNCAO
COPY duplicates a file, which can also be created on a different disk.

If targetfile is not specified the sourcefile is copied to
the default-drive, where the filename is not changed.

Users of a single floppy-disk drive cannot take full advantage
because the COPY operation copies files sector by sector. This means that
a sector on the source-disk is read, the user is prompted for a disk-change,
the sector on the target-disk is written, the user is prompted again, ...

Also interesting for single users is the possibility to use pseudo-files
such as "CON" and "PRN".

EXEMPLO
COPY "TEST.BAS" TO "TEST.$$$"

COPY "*.*" TO "B:"

COPY "B:*.*"

COPY "TEST.TXT" TO "PRN"

VEJA TAMBEM
COPY (MSX2-Basic), COPY SCREEN, KILL, NAME

REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

(MSX-Basic 2)

SINTAXE
COPY (X1,Y1) - (X2,Y2)[, <source page>] TO (X3,Y3)[, <destination page>
     [, <logical operation>]]
COPY (X1,Y1) - (X2,Y2)[, <source page>] TO {<array variable name> |
     <filename>}
COPY {<array variable name> | <filename>}[, <direction>] TO (X3,Y3)
     [, <destination page>[, <logical operation>]]
COPY <filename> TO <array variable name>
COPY <array variable name> TO <filename>

VEJA TAMBEM
COPY (Disk-Basic), COPY SCREEN, SET PAGE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cvd", "", "MSX-BASIC", "CVD( <string> )", "Converts a 8 Byte string into a double-precision value.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
CVD( <string> )

FUNCAO
Converts a 8 Byte string into a double-precision value.

Type | Length | -> String | -> Number
0...255 | 1 | CHR$() | ASC
INT | 2 | MKI$() | CVI
SNG | 4 | MKS$() | CVS
DBL | 8 | MKD$() | CVD

EXEMPLO
D=CVD(D$)

VEJA TAMBEM
ASC,
CVI,
CVS,
CHR$,
MKI$,
MKS$,
MKD$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cvi", "", "MSX-BASIC", "CVI( <string> )", "Converts a 2 Byte string into an integer value.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
CVI( <string> )

FUNCAO
Converts a 2 Byte string into an integer value.

Type | Length | -> String | -> Number
0...255 | 1 | CHR$() | ASC
INT | 2 | MKI$() | CVI
SNG | 4 | MKS$() | CVS
DBL | 8 | MKD$() | CVD

EXEMPLO
I=CVI(I$)

VEJA TAMBEM
ASC,
CVS,
CVD,
CHR$,
MKI$,
MKS$,
MKD$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cvs", "", "MSX-BASIC", "CVS( <string> )", "Converts a 4 Byte string into a single-precision value.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
CVS( <string> )

FUNCAO
Converts a 4 Byte string into a single-precision value.

Type | Length | -> String | -> Number
0...255 | 1 | CHR$() | ASC
INT | 2 | MKI$() | CVI
SNG | 4 | MKS$() | CVS
DBL | 8 | MKD$() | CVD

EXEMPLO
S=CVS(S$)

VEJA TAMBEM
ASC,
CVI,
CVD,
CHR$,
MKI$,
MKS$,
MKD$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dskf", "", "MSX-BASIC", "DSKF( drivenumber )", "DSKF returns the number of free clusters on the specified drivenumber,",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
DSKF( drivenumber )

FUNCAO
DSKF returns the number of free clusters on the specified drivenumber,
where 1=A:, 2=B:, 3=C: and so on.
If 0 is specified the free space on the default-drive is returned.

EXEMPLO
? DSKF(0)

IF DSKF(1)<100 THEN PRINT "Please insert a new disk!"

VEJA TAMBEM
FILES,
LFILES

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dski$", "", "MSX-BASIC", "<stringvar> = DSKI$( <drivenumber>, <sectornumber> )", "DSKI$ reads the sector sectornumber from the disk-drive specified in",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
<stringvar> = DSKI$( <drivenumber>, <sectornumber> )

FUNCAO
DSKI$ reads the sector sectornumber from the disk-drive specified in
drivenumber (0=default, 1=A:, 2=B:, ...).

DSKI$ returns an empty string (which is historical, because sector-sizes were
smaller in the past and really returned the read sector). In MSX-Basic the
bytes of the read sector can be found on address:

  PEEK(&HF351) + PEEK(&HF352)*256

Note: the contents of this buffer are lost as soon as another command
accesses the disk again.

EXEMPLO
10 X$ = DSKI$(0,0)
20 AD = PEEK(&HF351)+PEEK(&HF352)*256
30 FOR I=0 TO 511:PRINT PEEK(AD+I);:NEXT

VEJA TAMBEM
DSKO$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dsko$", "", "MSX-BASIC", "DSKO$( <drivenumber>, <sectornumber> )", "DSKO$ writes the sector sectornumber on the disk-drive specified in",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
DSKO$( <drivenumber>, <sectornumber> )

FUNCAO
DSKO$ writes the sector sectornumber on the disk-drive specified in
drivenumber (0=default, 1=A:, 2=B:, ...).
The sector data to be written is located on:

  PEEK(&HF351) + PEEK(&HF352)*256

Note: the contents of this buffer are lost as soon as another command
accesses the disk again.

Important: because DSKO$ modifies sectors on the physical disk
this command should be used very careful!

VEJA TAMBEM
DSKI$

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("field", "", "MSX-BASIC", "FIELD #channel, length AS stringvariable,", "After a file is opened and the recordlength has been specified FIELD can be used",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
FIELD #channel, length AS stringvariable,
length2 AS stringvariable2, ...

FUNCAO
After a file is opened and the recordlength has been specified FIELD can be used
how a record is separated into single fields. Every datafield requires one entry
specifying length and the name of a stringvariable.
The number of fields is not limited but must not exceed the recordlength as
specified in the OPEN statement.

EXEMPLO
FIELD #1, 15 AS NA$, 15 AS ST$, 20 AS OT$, 16 AS TE$

VEJA TAMBEM
LSET,
RSET,
OPEN,
GET,
PUT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("files", "", "MSX-BASIC", "FILES [\"pattern\"][,L]", "Lists the contents of (the current directory on) the diskette.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
FILES ["pattern"][,L]

FUNCAO
Lists the contents of (the current directory on) the diskette.
If the disk is empty or the pattern does not match any existing file
a "File not found error" is generated.

All files in the file-listing are using 8 characters for the filename,
a dot (.) and 3 characters for the extension.

Disk-Basic 2: the Parameter "L" can be added
to display the list of files in long format.

EXEMPLO
FILES

Shows all files on the standard-drive (usually A:)

FILES "B:"

Shows all files on drive B:

FILES "*.BAS"

Shows all files with the extension ".BAS"

FILES "*.*"

Works as FILES without pattern.

VEJA TAMBEM
LFILES,
LOAD,
BLOAD,
CALL MFILES,
DSKF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("get", "", "MSX-BASIC", "GET #channel [, recordnumber]", "Reads a record from a file. If recordnumber is not specified GET",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
GET #channel [, recordnumber]

FUNCAO
Reads a record from a file. If recordnumber is not specified GET
reads the next one from the file.

EXEMPLO
100 OPEN "ADRESS.DAT" AS #1 LEN=66
110 FIELD #1, 15 AS NA$, 15 AS ST$, 20 AS OT$, 16 AS TE$
120 SN=LOF(1)/66
130 FOR I=1 TO SN
140 GET #1
150 PRINT
160 PRINT TAB(5);NA$
170 PRINT TAB(5);ST$
180 PRINT TAB(5);OT$
190 NEXT
200 PRINT "Finished"
210 CLOSE

VEJA TAMBEM
FIELD,
PUT,
LSET,
RSET,
OPEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("kill", "", "MSX-BASIC", "KILL <pattern>", "Deletes the file(s) specified in pattern.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
KILL <pattern>

FUNCAO
Deletes the file(s) specified in pattern.

EXEMPLO
KILL "TEST.$$$

KILL "*.$$$"

KILL "B:*.BAK"

VEJA TAMBEM
CALL RMDIR,
CALL MKILL,
COPY,
NAME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("lfiles", "", "MSX-BASIC", "LFILES [\"pattern\"][,L]", "Prints the contents of (the current directory on) the diskette.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
LFILES ["pattern"][,L]

FUNCAO
Prints the contents of (the current directory on) the diskette.
If the disk is empty or the pattern does not match any existing file
a "File not found error" is generated.

All files in the file-listing are using 8 characters for the filename,
a dot (.) and 3 characters for the extension.

Disk-Basic 2: the Parameter "L" can be added
to print the list of files in long format.

EXEMPLO
LFILES

Prints all files on the standard-drive (usually A:)

LFILES "B:"

Prints all files on drive B:

LFILES "*.BAS"

Prints all files with the extension ".BAS"

LFILES "*.*"

Works as LFILES without pattern.

VEJA TAMBEM
FILES,
LOAD,
BLOAD,
CALL MFILES,
DSKF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("loc", "", "MSX-BASIC", "LOC( channel )", "Returns the current recordnumber in the opened file as #channel.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic, RS232-Basic)

SINTAXE
LOC( channel )

FUNCAO
Returns the current recordnumber in the opened file as #channel.

EXEMPLO
100 OPEN "ADRESS.DAT" AS #1 LEN=66
110 FIELD #1, 15 AS NA$, 15 AS ST$, 20 AS OT$, 16 AS TE$
120 SN=LOF(1)/66
130 FOR I=1 TO SN
140 GET #1
150 PRINT "Record";LOC(1)
160 PRINT TAB(5);NA$
170 PRINT TAB(5);ST$
180 PRINT TAB(5);OT$
190 NEXT
200 PRINT "Finished"
210 CLOSE

VEJA TAMBEM
FIELD,
GET,
PUT,
OPEN,
CLOSE,
EOF,
LOF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("lof", "", "MSX-BASIC", "LOF( <filenumber> )", "LOF returns the filesize (LengthOfFile) in bytes.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic, RS232-Basic)

SINTAXE
LOF( <filenumber> )

FUNCAO
LOF returns the filesize (LengthOfFile) in bytes.

EXEMPLO
10 INPUT "Filename: "; FI$
20 OPEN FI$ FOR INPUT AS #1
30 PRINT LOF(1) "Bytes"
40 CLOSE

VEJA TAMBEM
OPEN, CLOSE, EOF, LOC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("lset", "", "MSX-BASIC", "LSET <stringvariable> = <string-expression>", "Assigns a value to a variable, which is defined by the FIELD statement.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
LSET <stringvariable> = <string-expression>

FUNCAO
Assigns a value to a variable, which is defined by the FIELD statement.
Is the result of the string-expression shorter as specified in the FIELD statement
the string will be stored left-aligned, the rest is filled with spaces. If the result is
larger only the number of fitting characters is saved.

EXEMPLO
LSET NA$ = "Peter Pan"             ->  "Peter Pan      "
RSET NA$ = "Peter Pan"             ->  "      Peter Pan"

LSET NA$ = "Carlo Kautschuk"       ->  "Carlo Kautschuk"
RSET NA$ = "Carlo Kautschuk"       ->  "Carlo Kautschuk"

LSET NA$ = "Ludwig van Beethoven"  ->  "Ludwig van Beet"
RSET NA$ = "Ludwig van Beethoven"  ->  "Ludwig van Beet"

VEJA TAMBEM
RSET,
MID$,
FIELD,
GET,
PUT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("mkd$", "", "MSX-BASIC", "MKD$( <number> )", "Converts a double-precision value into a 8 Byte string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
MKD$( <number> )

FUNCAO
Converts a double-precision value into a 8 Byte string.

Type | Length | -> String | -> Number
0...255 | 1 | CHR$() | ASC
INT | 2 | MKI$() | CVI
SNG | 4 | MKS$() | CVS
DBL | 8 | MKD$() | CVD

EXEMPLO
LSET MKD$(D)

VEJA TAMBEM
CHR$,
MKI$,
MKS$,
ASC,
CVI,
CVS,
CVD

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("mki$", "", "MSX-BASIC", "MKI$( <number> )", "Converts an integer value into a 2 Byte string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
MKI$( <number> )

FUNCAO
Converts an integer value into a 2 Byte string.

Type | Length | -> String | -> Number
0...255 | 1 | CHR$() | ASC
INT | 2 | MKI$() | CVI
SNG | 4 | MKS$() | CVS
DBL | 8 | MKD$() | CVD

EXEMPLO
LSET MKI$(I)

VEJA TAMBEM
CHR$,
MKS$,
MKD$,
ASC,
CVI,
CVS,
CVD

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("mks$", "", "MSX-BASIC", "MKS$( <number> )", "Converts a single-precision value into a 4 Byte string.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
MKS$( <number> )

FUNCAO
Converts a single-precision value into a 4 Byte string.

Type | Length | -> String | -> Number
0...255 | 1 | CHR$() | ASC
INT | 2 | MKI$() | CVI
SNG | 4 | MKS$() | CVS
DBL | 8 | MKD$() | CVD

EXEMPLO
LSET MKS$(S)

VEJA TAMBEM
CHR$,
MKI$,
MKD$,
ASC,
CVI,
CVS,
CVD

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("name", "", "MSX-BASIC", "NAME \"oldname\" AS \"newname\"", "Renames the file named oldname to newname.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
NAME "oldname" AS "newname"

FUNCAO
Renames the file named oldname to newname.
If a file named newname already exists a "File already exists" error occurs.

oldname and newname can be fully qualified filenames,
including drive-letter. Wildcards can be included too.

EXEMPLO
NAME "TEST.BAS" AS "TEST.BAK"

VEJA TAMBEM
CALL MNAME, COPY, KILL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("put", "", "MSX-BASIC", "PUT #channel [, recordnumber]", "Writes a record into a file. If recordnumber is not specified PUT writes the next one...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
PUT #channel [, recordnumber]

FUNCAO
Writes a record into a file. If recordnumber is not specified PUT writes the next one into the file.

EXEMPLO
10 OPEN SUPERDAT.DAT" as #1 LEN=66
20 PUT#1, 2000
30 PRINT "The file has now a size of";LOF(1);"Bytes"
40 CLOSE

These records can be read with GET, even if no data was inserted.
Which bytes are really read depends on the history of the disk.
Unused records on new formatted disks are filled with CHR$(229).
They may also contain parts of already deleted files.

To initialize all records the following code is recommended:

10 OPEN "SUPERDAT.DAT" AS #1 LEN=66
20 FIELD #1, 66 AS X$
30 LSET X$=STRING$(66,229)
50 FOR I=1 TO 2000
60  PUT #1, I
70 NEXT
80 CLOSE

VEJA TAMBEM
FIELD,
GET,
LSET,
RSET,
OPEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("rset", "", "MSX-BASIC", "RSET <stringvariable> = <string-expression>", "Assigns a value to a variable, which is defined by the FIELD statement.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - Disk-BASIC)

(Disk-Basic)

SINTAXE
RSET <stringvariable> = <string-expression>

FUNCAO
Assigns a value to a variable, which is defined by the FIELD statement.
Is the result of the string-expression shorter as specified in the FIELD statement
the string will be stored right-aligned, the rest is filled with spaces. If the result is
larger only the number of fitting characters is saved.

EXEMPLO
LSET NA$ = "Peter Pan"             ->  "Peter Pan      "
RSET NA$ = "Peter Pan"             ->  "      Peter Pan"

LSET NA$ = "Carlo Kautschuk"       ->  "Carlo Kautschuk"
RSET NA$ = "Carlo Kautschuk"       ->  "Carlo Kautschuk"

LSET NA$ = "Ludwig van Beethoven"  ->  "Ludwig van Beet"
RSET NA$ = "Ludwig van Beethoven"  ->  "Ludwig van Beet"

VEJA TAMBEM
LSET,
MID$,
FIELD,
GET,
PUT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call adjust", "", "MSX-BASIC", "", "call adjust",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Hangul-Basic 2)

EXEMPLO
CALL ADJUST

VEJA TAMBEM
CALL HANON, CALL HANOFF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call akcnv", "", "MSX-BASIC", "CALL AKCNV(character variable,X$)", "If any characters which are not zenkaku are containted in X$, this command converts them...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Convert all character data to zenkaku characters, and substitue them into a character string.

SINTAXE
CALL AKCNV(character variable,X$)

_ AKCNV(character variable,X$)

Character variable: Variable of character type.

X$: Character constant, variable, array variable or expression.

FUNCAO
If any characters which are not zenkaku are containted in X$, this command converts them into zenkaku characters, and substitute the whole string into a character variable.

EXEMPLO
CALL AKCNV(A$,"ABCS? (invertido)Zs23 "):PRINT A$
abS? (invertido)ZsACEGI

Among the character string given to this CALL AKCNV statement, "ABC" and "23 " were converted, since they were hankaku.

VEJA TAMBEM
Zenkaku and Hankaku

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call ank", "", "MSX-BASIC", "CALL ANK", "Deactivates the Kanji text-mode activated by CALL KANJI.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Quit Japanese input mode.

SINTAXE
CALL ANK

_ ANK

FUNCAO
Deactivates the Kanji text-mode activated by CALL KANJI.
However, execution of this command does not release memory which was allocated for kanji driver through CALL KANJI statement.

VEJA TAMBEM
MSX-JE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call arabic", "", "MSX-BASIC", "", "call arabic",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call araboff", "", "MSX-BASIC", "", "call araboff",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call arabon", "", "MSX-BASIC", "", "call arabon",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call arb", "", "MSX-BASIC", "CALL ARB0", "(?) Changes the system-font to arabic layout. Already in textmode displayed characters...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

SINTAXE
CALL ARB0

CALL ARB1

CALL ARB2

FUNCAO
(?) Changes the system-font to arabic layout. Already in textmode displayed characters will be redefined accordingly.

EXEMPLO
CALL ARB0
CALL ARB1
CALL ARB2

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call arbnum", "", "MSX-BASIC", "", "call arbnum",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call arbval", "", "MSX-BASIC", "CALL ARBVAL", "(?) Changes the digits in the font to international layout.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

SINTAXE
CALL ARBVAL

FUNCAO
(?) Changes the digits in the font to international layout.

EXEMPLO
CALL ARBVAL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call astr", "", "MSX-BASIC", "", "call astr",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call audreg", "", "MSX-BASIC", "CALL AUDREG( register, data )", "This command writes data directly into the registers of the FM-chip. register is the",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL AUDREG( register, data )

FUNCAO
This command writes data directly into the registers of the FM-chip. register is the
register-number. Valid values are 0 to 7, 14 to 24, 32 to 40 and 48 to 56. data is the
data of 8 bit size and has a valid range from 0 to 255.

EXEMPLO
CALL AUDREG(3, 15)

VEJA TAMBEM
SOUND (MSX-Basic),
CALL MUSIC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call bprint", "", "MSX-BASIC", "CALL BPRINT", "Prints a hardcopy of the screen contents in all MSX1 screen modes (0-3).",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Chakkari Copy)

SINTAXE
CALL BPRINT

FUNCAO
Prints a hardcopy of the screen contents in all MSX1 screen modes (0-3).

It's the same operation as pressing the COPY button on the Chakkari Copy cartridge.

Note: if the screen mode cannot be auto-detected Chakkari Copy must be started
with pressed COPY button. In this case all screen-parameters for later capture can be entered
manually.

EXEMPLO
CALL BPRINT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call bc", "", "MSX-BASIC", "CALL BC", "This command turn on the BASIC COMPILER options.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Firmware)

This function is available in the Sanyo MSX 2+ only!

SINTAXE
CALL BC

FUNCAO
This command turn on the BASIC COMPILER options.
X-BASIC must first initialized with CALL BC before using CALL RUN.

VEJA TAMBEM
Turbo-Basic compiler

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call bgm", "", "MSX-BASIC", "CALL BGM( number )", "Specifies, if music should be played in foreground- or background-mode.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL BGM( number )

FUNCAO
Specifies, if music should be played in foreground- or background-mode.

The Extended MSX-Basic makes it possible to play music, while other Basic-commands are about to be processed. If n is 0, then it's not possible to execute other Basic-commands while music is played. But using 1 (default) this is enabled.

EXEMPLOS
10 CALL MUSIC(0, 0, 1)
20 CALL BGM(1)
30 PLAY #2,"CDEFGAB"
40 PRINT "BASIC is ready"
50 END

10 CALL MUSIC(0, 0, 1)
20 CALL BGM(0)
30 PLAY #2,"CDEFGAB"
40 PRINT "BASIC is ready now"
50 END

VEJA TAMBEM
CALL MUSIC, PLAY (MSX-Music)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call calendar", "", "MSX-BASIC", "", "call calendar",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call chcopy", "", "MSX-BASIC", "CALL CHCOPY", "Prints a hardcopy",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

SINTAXE
CALL CHCOPY

FUNCAO
Prints a hardcopy

EXEMPLO
CALL CHCOPY

VEJA TAMBEM
CALL OPTIONS, CALL HCOPY

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call cls", "", "MSX-BASIC", "CALL CLS", "You must use this command to clear away the screen once you enter Japanese input mode by...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Delete all the contents in the screen during Japanese input mode.

SINTAXE
CALL CLS

_CLS

FUNCAO
You must use this command to clear away the screen once you enter Japanese input mode by using CALL KANJI command.

CLS statement may not wok properly in Japanese input mode.

VEJA TAMBEM
CLS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call com", "", "MSX-BASIC", "CALL COM([<n>:],GOSUB <line number>)", "Set up a line number for BASIC to trap when a character is received on the communication...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COM([<n>:],GOSUB <line number>)

FUNCAO
Set up a line number for BASIC to trap when a character is received on the communication channel.

When a trap occurs an automatic CALL COMSTOP is executed so further reception traps can not take place. The RETURN from the trap routine will automatically do a CALL COMON, unless an explicit CALL COMOFF has been performed inside the trap routine.

Even trapping only takes place when BASIC is executing a program. When an error trap (resulting from an ON ERROR statement) occurs, this automatically disables all trapping (including ERROR, STRIG, STOP, SPRITE, INTERVAL and KEY).

VEJA TAMBEM
RETURN, CALL COMON, CALL COMOFF, CALL COMSTOP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call combreak", "", "MSX-BASIC", "CALL COMBREAK([\"<n>:\"],<expression>)", "Send break characters specified by <expression> to the channel specified by <n>. The...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COMBREAK(["<n>:"],<expression>)

FUNCAO
Send break characters specified by <expression> to the channel specified by <n>. The range of the <expression> should be between 0 and 32767. If 0 is specified, a long break is sent (forever).

An OPEN must be executed before this statement. The default channel number is 0.

VEJA TAMBEM
OPEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call comdtr", "", "MSX-BASIC", "CALL COMDTR([\"<n>:\"],<expression>)", "Turn off DTR signal when the <expression> is zero, otherwise turn on the DTR signal.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COMDTR(["<n>:"],<expression>)

FUNCAO
Turn off DTR signal when the <expression> is zero, otherwise turn on the DTR signal.

An OPEN must be executed before this statement. The default channel number is 0.

Note: DTR signal is affected by CALL COMDTR and CALL COMINI statements.

VEJA TAMBEM
OPEN, CALL COMINI

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call comhelp", "", "MSX-BASIC", "CALL COMHELP", "Prints out a brief description of the parameters set by the COMINI statement on the...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COMHELP

FUNCAO
Prints out a brief description of the parameters set by the COMINI statement on the screen.

EXEMPLO
CALL COMHELP
Initialize statement options:

CALL COMINI ("
<device# {0,1,2...9}>:
<character length {5,6,7,8}>
<parity {E,O,I,N}>
<stop bits {1,2,3}>
<XON/XOFF {X,N}>
<CTS hand-shake {H,N}>
<auto LF on receive {A,N}>
<auto LF on transmit {A,N}>
<SI/SO {S,N}>"
,<receiver baud rate>
,<transmitter baud rate>
,<time out count>
                          )
Default:
 CALL COMINI("0:8N1XHNNN",1200,1200,0)

VEJA TAMBEM
CALL COMINI

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call comini", "", "MSX-BASIC", "CALL COMINI [([<string exp.>][,[<Rx baud rate>][,[<Tx baud rate>][,[<timeout>]]]", "Setup communication parameters.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COMINI [([<string exp.>][,[<Rx baud rate>][,[<Tx baud rate>][,[<timeout>]]]])]

<string exp.>

A string specifies the channel control parameters as described below:

"[0:][8[N[1[X[H[N[N[N]]]]]]]]"
  |   | | | | | | | |
  +----------------------- Channel no.
      | | | | | | | |        When there is more than one
      | | | | | | | |        channel, this parameter specifies
      | | | | | | | |        the channel number. May be omitted
      | | | | | | | |        if the system has only one channel.
      | | | | | | | |        The default value is "0".
      +------------------- Character length
        | | | | | | |        "5" - 5 bits
        | | | | | | |        "6" - 6 bits
        | | | | | | |        "7" - 7 bits
        | | | | | | |        "8" - 8 bits
        +----------------- Parity flag
          | | | | | |        "E" - Even parity
          | | | | | |        "O" - Odd parity
          | | | | | |        "I" - Ignore
          | | | | | |        "N" - No parity
          +--------------- Stop bits length
            | | | | |        "1" - 1 bit
            | | | | |        "2" - 1.5 bits
            | | | | |        "3" - 2 bits
            +------------- XON/XOFF control
              | | | |        "X" - do control
              | | | |        "N" - do not control
              +----------- XTS/RTS handshake
                | | |        "H" - do handshake
                | | |        "N" - do not handshake
                +--------- Automatic Line Feed (Rx)
                  | |        Insert LF after CR received
                  | |        "A" - insert Line Feed in buffer
                  | |        "N" - do not insert Line Feed
                  +------- Automatic Line Feed (Tx)
                    |        Insert LF after CR transmitted
                    |        "A" - insert Line Feed
                    |        "N" - do not insert Line Feed
                    +----- Shift-in/Shift-out control
                             Illegal when data length is other
                             than 7 bits.
                             "S" - do control
                             "N" - do not control

<baud rate>

It is possible to set different baud rates for transmitter and receiver. The eighteen values for the baud rate are as follows:

50 75 110 134 150 300 600 1050 1200 1800 2000 2400 3600 4800 7200 9600 19200 38400

Baud rate 134 is actually 134.5. When only the receiver baud rate is specified, the baud rate for the transmitter takes the same speed as the receiver. When only the transmitter baud rate is specified, the baud rate for the receiver takes the default value.

<time out>

If CTS-RTS handshake is selected, the device driver will wait for CTS (Clear To Send) and DCD signals before sending the character. If XON/XOFF protocol is selected and an XOFF has been received, the device driver will wait for XON before sending the character (unless the character is XON/XOFF - hex 11 or hex 13). The waiting time is specified in increments of 10ms. If the time elapses, an error is returned. If the time is specified as 0, the driver waits until the correct signals are received, or <CTRL+STOP> keys are pressed.

FUNCAO
Setup communication parameters.

EXEMPLO
CALL COMINI("0:7E1XHNNN", 600,1200,30)
CALL COMINI("8N1",9600)

The fixed default values for initialisation are as follows:

CALL COMINI("0:8E3XHNNN",1200,1200,0)

VEJA TAMBEM
OPEN, CLOSE, INPUT, LINE INPUT, SAVE, LOAD, MERGE, RUN, EOF, LOC, LOF, VARPTR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call comoff", "", "MSX-BASIC", "CALL COMOFF(\"[<n>:]\")", "Disable event trapping caused by an incoming character on the communication channel. The...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COMOFF("[<n>:]")

FUNCAO
Disable event trapping caused by an incoming character on the communication channel. The communication buffer is flushed.

VEJA TAMBEM
CALL COM, CALL COMON, CALL COMSTOP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call comon", "", "MSX-BASIC", "CALL COMON(\"[<n>:]\")", "Enable event trapping caused by an incoming character on the communication channel.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COMON("[<n>:]")

FUNCAO
Enable event trapping caused by an incoming character on the communication channel.

VEJA TAMBEM
CALL COM, CALL COMOFF, CALL COMSTOP

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call compress", "", "MSX-BASIC", "", "call compress",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call comstat", "", "MSX-BASIC", "CALL COMSTAT([\"<n>:\"],<name of variable>)", "Read the status of the communication channel. The status returned is assigned to the...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COMSTAT(["<n>:"],<name of variable>)

FUNCAO
Read the status of the communication channel. The status returned is assigned to the varaible. Bits 0 to 7 represent the current channel status and bits 8 to 15 represent the status of the last character read from the buffer. The assignment of each of the bits is as follows:
Bit no. | Description
15 | Receive buffer overflow error 0 - no buffer overflow 1 - buffer overflow ... lost character
14 | Time out error 0 - no time out error 1 - time out error ... no character
13 | Framing error 0 - no framing error 1 - framing error ... bad character
12 | Overrun error 0 - no overrun error 1 - overrung error ... lost character
11 | Parity error 0 - no parity error 1 - parity error ... bad character
10 | <CTRL+STOP> keys 0 - keys were not pressed 1 - keys were pressed ... no character
9 | Not used, reserved
8 | Not used, reserved
7 | Clear To Send 0 - can send 1 - cannot send
6 | Rx XON/XOFF control 0 - XON 1 - XOFF
5 | Tx XON/XOFF control 0 - XON 1 - XOFF
4 | Not used, reverved
3 | Data Set Ready 0 - ready 1 - not ready
2 | Break detect 0 - not detected 1 - detected
1 | Ring indicator 0 - ringing 1 - not ringing
0 | Carrier detect 0 - detected 1 - not detected
An OPEN must be executed before this statement. The default channel number is 0.

VEJA TAMBEM
OPEN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call comstop", "", "MSX-BASIC", "CALL COMSTOP(\"[<n>:]\")", "Suspend event trapping caused by an incoming character on the communication channel.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COMSTOP("[<n>:]")

FUNCAO
Suspend event trapping caused by an incoming character on the communication channel.

VEJA TAMBEM
CALL COM, CALL COMON, CALL COMOFF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call comterm", "", "MSX-BASIC", "CALL COMTERM[(\"<n>:\")]", "Enter terminal emulator mode. The channel should be closed when this statement is invoked.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(RS232-Basic)

SINTAXE
CALL COMTERM[("<n>:")]

FUNCAO
Enter terminal emulator mode. The channel should be closed when this statement is invoked.
The following function keys have a special function in the terminal mode:

F-6: Turn on/off literal mode. In the literal mode, control characters are displayed
with upper arrow and the character offset by #40H. For example character whose code is 01H is
displayed as "A".

Initial mode: Literal mode off.

F-7: Toggle Half/Full duplex mode. In the Half duplex mode, characters keyed in are
echoed to the screen as well as sent to the communication channel.

Initial mode: Full duplex.

F-8: Turn on/off printer echo. When the printer echo is on, all the characters sent

to the screen are also sent to the printer.

Initial mode: Printer echo off.

The terminal emulator supports the MSX VT52 protocol subset.

Intel hexfiles can be downloaded with the terminal emulator. This mode is entered by two
successive <ESCAPE> characters. The file format and handshaking follow the rules for
these files. The machine code will be placed in memory at the addresses specified in the
hexfile. In general, only download hexfiles that will be placed at addresses between hex 8000
and BASIC's top of memory.

To exit terminal emulator mode, press <CTRL+STOP> keys.

EXEMPLO
CALL COMTERM

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call dcolor", "", "MSX-BASIC", "", "call dcolor",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call engl", "", "MSX-BASIC", "", "call engl",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call estr", "", "MSX-BASIC", "", "call estr",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call fmpac", "", "MSX-BASIC", "", "FM-PAC is produced by Yamaha.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

FM-PAC is produced by Yamaha.

Overview

Internal file management software of FMPAC is called Pac Commander. To boot the software, insert FMPAC into MSX, turn on the power, then type in CALL FMPAC from BASIC.

Messages in Pac Commander assumes that MSX is a wizard who is managing FM-PAC. Sections names in the initial menu screen are called "spells". Takamichi's comments are enclosed by [].

Decision; SPACE = yes, ESC = no.

Also, a dialogue window on upper-right shows "Yes" and "no".

Use cursor key to select, and then SPACE.

"PAC" might be either a FMPAC, or PAC without FM module. Slot spell is invalid if anything other than one single FMPAC is inserted to MSX, though.

Slot no. that Pac Commander indicates may be different with actual slot no. of your MSX. This is why Slot spell is necessary. Pac Commander always uses slot no. shown with slot spell.

Change spell swaps entire contents of two PACs.

---Spell menu
  Clear
  Copy
  Change
  Delete File
  Slot
  Background Music

---Spell 1: Clear
  MSX said, "Password and data at Slot n disappear. OK?"
  [n stands for slot no. which FM-PAC is inserted in]
  --Press SPACE
  MSX used Clear magic.
  Poooof.....

---Spell 2: Copy
  PAC -> PAC
  PAC -> Disk
  Disk -> PAC

  --PAC -> PAC

  What is PAC you copy from?

  --Choose a PAC you want to copy from.

  [If 3 or more PAC are present]

  What is PAC you copy to?

  [choose a PAC you want to copy to.]

  MSX said, "Password and data at destination (n) disappear. OK?"

  [n stands for slot no. where destination PAC is inserted in]

  --Press SPACE

  MSX used Copy magic.

  Pipipipi, copy

  --PAC -> Disk

  [Prompt for file name appears]

  Destination file:

  --Input file name; use BS key to delete a character

  --Press SPACE

  MSX used Copy magic.

  Pipipipi, copy

  --If there are already existing data, their list appears.

  --Overwrite; move cursor to the file you want to overwrite and press SPACE.

  MSX said, "Overwrite xxxxxxxx. OK?"

  [xxxxxxxx = file name.]

  --Disk -> PAC

  [PAC can use only drive a:]

  [A list of data files appear]

  [Move cursor to file name you want to copy from]

  Original file:

  --Press SPACE

  MSX said, "Password and data at destination (n) disappear. OK?"

  [n stands for slot no. where destination PAC is inserted in]

  --Press SPACE

  MSX used Copy magic.

  Pipipipi, copy

---Spell 3: Change
  --Press SPACE
  [Valid only if there are two PACs]
  MSX used Change magic.
  Chachacha, change!!

---Spell 4: Delete file
  [List of files appears]
  [Move cursor to the  file you want to delete]
  MSX said, "Delete xxxxxxxx. OK?"
  --Press SPACE
  MSX used Delete File magic.
  Dee, lee, tee!!

---Spell 5: Slot
  [Invalid if there are more than one FMPAC]
  MSX used Slot magic.
  MSX said, "PAC is in slot (n). Did you write it down?"
  [n stands for slot no. where your FMPAC is inserted in]

---Spell 6: Background Music
  Sample 1
  Sample 2
  Sample 3
  Sample 4
  Sample 5
  Stop
  [Choose a music and press SPACE to change music.]

Error messages

Error messages are always shown in yellow box.

(1) | Hmm, write protected!!
  Try again?
  [Reason: Disk is write protected.]
(2) | Disk is not set!!
  Try again?
  [Reason: Disk is not set.]
  [Reason: Disk is in a drive other than a:]
  [Reason: Disk is not formatted.]
(3) | MSX said, "file not found."
  [Reason: You chose wrong file name.]
  MSX cannot use this disk!!
  Try again?
  [Reason: Disk is not of a format usable with MSX.]
  [Reason: Disk is broken.]
(4) | This disk seems broken!!
  Try again?
  [same as (3)]
(5) | MSX said, "this spell requires a disk drive
  or one more PAC."
  [Reason: No disk drive nor second PAC is connected.]
(6) | MSX said, "this spell requires a disk drive."
  [Reason: Disk drive is not connected.]
(7) | MSX said, "this spell requires one more PAC."
  [Reason: Second PAC is not connected.]
(8) | 2 or more PACs!
  Shutdown the power, and boot again with one PAC.
  [Reason: Two or more PACs/FMPACs are inserted.]
(9) | MSX said, "insufficient empty disk space."
  [Reason: Disk is full.]
(10) | MSX said, "format of the file is wrong."
  [Reason: you specified a file that is not a PAC data.]
(11) | Not enough memory.
  [Reason: MSX does not have 32 KB or more RAM.]
  [Reason: Insufficient memory due to connected external devices.]

Translations of Messages shown by FMPAC

July 26, 1998 by Takamichi

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call fmpak", "", "MSX-BASIC", "", "The FM STEREO PAK firmware can be invoked with CALL FMPAK.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

The FM STEREO PAK firmware can be invoked with CALL FMPAK.

Even if the FM-Basic can be used with any MSX Computer the CALL FMPAK
statement requires at least an MSX 2 system.
Due to some restrictions Disk Basic must be enabled, otherwise the CALL FMPAK statement would
cause unpredictable results.

The firmware program can be controlled with the cursor keys, joystick in port 1 or mouse in port 2.
As you know the MSX video supports a refresh rate of 50Hz and 60Hz. The program uses the mode
that is active at start-time. If the SHIFT key is pressed while confirming CALL FMPAC command
with Enter the mode is toggled. After startup the FM STEREO PAK logo appears.
Pushing the space key or fire button leaves this screen. An amplifier appears in the above half of the
screen. It can be controlled with the arrow. Firstly the amplifier must be powered on. To do so push
the POWER button. Then you can choose between:

MUSIC mode
Keyboard mode

After powering on the amplifier is in MUSIC mode. This is also visible in the display. In this mode
4 pre-programmed songs can be played, which are selectable on the screen. The 2 TRACK buttons can be
used to select a song. The song starts playing immediately after the selection. While the music is
playing every voice-beat is displayed on the screen. The selection of TRACK 00 stops playing.
You can use the 2 VOL buttons so change the volume, which is also displayed on the screen. The KEYBOARD button
switches to the second mode. Here the keyboard is changed to a polyphonic keyboard which is displayed
in the lower half of the screen. It is connected to the amplifier to enable volume control in this
mode too. The two INSTR buttons enable the selection of an instrument.

You can select from a set of 15 different instruments. The selected instrument is displayed on the
screen. The keyboard has approximately 3 octaves. You cannot play more than 6 sounds simultaneously.
You can play the music with the computer keyboard; which key is assigned to a specific note is
shown in the INFO-screen, which can be displayed by pressing the INFO button.
Beside the picture of a computer, PAK and a hifi-system you can also see 5 different drums, which
can be controlled with the cursor keys (return by pressing the fire button). The program supports
5 different drums with the shortcuts:

 F1 | HH | Hi Hat
F2 | CIM | Cimbal
F3 | TOM | Tom Tom
F4 | SD | Snare drum
F5 | BD | Bass drum
You can not return from PAK to BASIC. The only way to do this is to reset the MSX.

Technical information:
The FM STEREO PAK contains a 64KB ROM in range from &H0000-&HFFFF (no memory mapper).

VEJA TAMBEM
CALL FMPAC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call fnkey", "", "MSX-BASIC", "", "call fnkey",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call gprint", "", "MSX-BASIC", "CALL GPRINT( <arg1> [, <arg2> [, ...]] )", "Prints specified argument(s) in graphic mode.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

SINTAXE
CALL GPRINT( <arg1> [, <arg2> [, ...]] )

FUNCAO
Prints specified argument(s) in graphic mode.

EXEMPLO
10 SCREEN 5
20 CALL GPRINT( "Sqrt of 27 is", SQR(27) )

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call greg", "", "MSX-BASIC", "CALL GREG( <string-variable> )", "CALL GREG( <string-variable> )",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

SINTAXE
CALL GREG( <string-variable> )

EXEMPLO
GET DATE D$
PRINT D$
12/31/99

CALL DATE(D$)
PRINT D$
31/12/1999.5

VEJA TAMBEM
GET DATE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call hanoff", "", "MSX-BASIC", "", "call hanoff",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Hangul-Basic)

EXEMPLO
CALL HANOFF

VEJA TAMBEM
CALL HANON

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call hanon", "", "MSX-BASIC", "", "call hanon",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Hangul-Basic)

EXEMPLO
CALL HANON

VEJA TAMBEM
CALL HANOFF

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call hcopy", "", "MSX-BASIC", "CALL HCOPY", "Prints a hardcopy",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

SINTAXE
CALL HCOPY

FUNCAO
Prints a hardcopy

EXEMPLO
CALL HCOPY

VEJA TAMBEM
CALL OPTIONS, CALL CHCOPY

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call hijri", "", "MSX-BASIC", "", "call hijri",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call hiro", "", "MSX-BASIC", "", "CALL HIRO boots up the Word Processor in the MSX A1-ST (Turbo-R).",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Firmware)

CALL HIRO boots up the Word Processor in the MSX A1-ST (Turbo-R).

This is similar to CALL MWP in the A1-GT. In case of A1-WX/WSX machines
the corresponding command is CALL MENU.

Takamichi Suzukawa: These CALL commands are not written in any manual, but were notified in a
magazine (MSX-FAN) as late as in 1994.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call indnum", "", "MSX-BASIC", "", "call indnum",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call indval", "", "MSX-BASIC", "CALL INDVAL", "(?) Changes the digits in the font to arabic layout.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

SINTAXE
CALL INDVAL

FUNCAO
(?) Changes the digits in the font to arabic layout.

EXEMPLO
CALL INDVAL

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call jis", "", "MSX-BASIC", "CALL JIS(character variable,X$)", "If a first character in X$ is a 2-byte character, this command substitutes the JIS code...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Substitue a JIS code which corresponds to first character in a character data to a numeric variable.

SINTAXE
CALL JIS(character variable,X$)

_ JIS(character variable,X$)

Character variable: Variable of character type.

X$: Character constant, variable, array variable or expression.

FUNCAO
If a first character in X$ is a 2-byte character, this command substitutes the JIS code (hex, 4 digits) of that character into a numeric variable.

EXEMPLO
CALL JIS(A$,"S? (invertido)Zs"):PRINT A$
3441

VEJA TAMBEM
About how to access Kanji-ROM

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call kacnv", "", "MSX-BASIC", "CALL KACNV(character variable,X$)", "Characters contained in X$ will be converted as follows.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Convert any character that can be converted into hankaku characters as hankaku characters, and substitue them into a character string.

SINTAXE
CALL KACNV(character variable,X$)

_ KACNV(character variable,X$)

Character variable: Variable of character type.

X$: Character constant, variable, array variable or expression.

FUNCAO
Characters contained in X$ will be converted as follows.

Before Conversion | After Conversion
Zenkaku alphabets and numerics | Hankaku alphabets and numerics
Zenkaku hiraganas | Hankaku katakanas
Zenkaku katakanas | Hankaku katakanas
Kanji | Kanji (no conversion)

EXEMPLO
CALL KACNV(A$,"{uI Ej...["):PRINT A$
{uEEOO

VEJA TAMBEM
Zenkaku and Hankaku

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call kanji", "", "MSX-BASIC", "", "Activates the Kanji text-mode.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Activates the Kanji text-mode.

This mode supports high-resolution Kanji-characters and is provided by MSX-DOS 2.20 and the Kanji-Basic.

Once the Kanji-mode is activated you can use also graphic-modes to output/input text as in all other text-modes. The SCREEN-command switches between all modes as normal, but does not deactivate the Kanji-mode.

Due to this some Basic-statements/functions are not working properly and will return an error. These statements/functions have new equivalents in Kanji-Basic which must be used instead. Use CALL ANK to deactivate the Kanji-mode.

By executing this command, certain memory domain will be allocated for Japanese processing. At this point, himem address setting specified by CLEAR statement will be negated, and all variables and stacks (for FOR-NEXT and GOSUB/RETURN statements) will return to default.

You can use single-kanji conversion option if MSX-JE is not installed, renbunsetsu (sequential segments) conversion option if installed. See MSX-JE section for details.

Press Ctrl+Space simultaneously to activate input of extended characters. In this mode  the lower bar in bottom of screen is displayed in reverse colors, entered characters are displayed in double width. This mode can be exited by pressing Ctrl+Space again.

CALL KANJI or _KANJICALL KANJI0 or _KANJI0 | 32x13
CALL KANJI1 or _KANJI1 | 40x13
CALL KANJI2 or _KANJI2 | 32x24
CALL KANJI3 or _KANJI3 | 34x24

In case of CALL KANJI (or CALL KANJI0), the font is  taken from the standard kanji ROM of the MSX and is displayed.

In case of CALL KANJI1, the font is taken from the standard kanji ROM of the MSX then the horizontal 16 dots are compressed to 12 dots then are displayed. Note if the Matsushita specification 12 dot font ROM exists in the system, this font is displayed.

In the case of CALL KANJI2, it is same with CALL KANJI (or CALL KANJI0) but the interlace mode is used and the number of characters displayed vertically increases.

If the MSX standard Japanese front end processor (hereafter MSX-JE) exits, it is either installed when the CALL KANJI statement is executed or when the system is started. The direct entry mode (ANK) and the indirect entry mode (kanji) can be toggled by CTRL + SPCAE or GRAPH + SELECT. If the MSX-JE does not exist, you can use single kanji conversion function.

You must be careful when the CALL KANJI is executed for the first time after starting the MSX. The HIMEM setting (set by a CLEAR command) at that time becomes canceled and all variables and software stacks (for FOX/NEXT, GOSUB/RETURN) become cleared. This occurs before a process that allocates th work area of the kanji driver (and of the MSX-JE if existent) is executed. No problem occurs when CALL KANJI is executed second time and latr.

The mode where the CALL KANJI statement or similar has been is executed and kanjis may be displayed is called the kanji mode. To be more precise, the condition when the kanji mode is set and also SCREEN command has set 0 or 1 is called the kanji text mode, while the condition when the kanji mode is set and also SCREEN command has set a 2 or higher screen mode is called the kanji graphic mode.

The BASIC command prompt condition is in the kanji text mode and kanjis can be entered and outputted. Under the kanji graphic mode, kanjis can be only outputted.

WIDTH under kanji mode

- Kanji text mode

The WIDTH after executing CALL KANJI is decided as follows based on the WIDTH under the ANK mode. You can directly specify using SCREEN command and WIDTH command after executing CALL KANJI.

- KANJI0 or KANJI2

4/5 of the WIDTH value under the ANK mode becomes the WIDTH under the kanji text mode.

- KANJI1 or KANJI3

The WIDTH value under the ANK mode becomes the WIDTH under the kanji text mode as is.

2. If SCREEN 1 ws used under the ANK mode

- KANJI0 or KANJI2

The WIDTH value under the ANK mode becomes the WIDTH under the kanji text mode as is.

- KANJI1 or KANJI3

5/4 of the WIDTH value under the ANK mode becomes the WIDTH under the kanji text mode.

- Kanji graphic mode

Maximum displayable number is always set.

SCREEN mode selection by WIDTH

- KANJI0 or KANJI2

The 256 dot mode (GRAPHIC 4 mode of the VDP) is selected if WIDTH is between 26 and 32, while the 512 dot mode (GRAPHIC 6 mode of the VDP) is selected if between 33 and 64.

- KANJI1 or KANJI3

The 256 dot mode (GRAPHIC 4 mode of the VDP) is selected if WIDTH is between 26 and 40, while the 512 dot mode (GRAPHIC 6 mode of the VDP) is selected if between 41 and 80.

VEJA TAMBEM
MSX-JE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call kext", "", "MSX-BASIC", "CALL KEXT(character variable,X$,option)", "This command extracts only hankaku characters out of a character data if value of option...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Extract zenkaku or hankaku characters exclusively out of a character data, and substitue them into a character string.

SINTAXE
CALL KEXT(character variable,X$,option)

_ KEXT(character variable,X$,option)

Character variable: Variable of character type.

X$: Character constant, variable, array variable or expression.

Option: 0 or 1.

FUNCAO
This command extracts only hankaku characters out of a character data if value of option is 0; only zenkaku characters if value option is 1, and substitute them into a character string.

EXEMPLO
CALL KEXT(A$,"AI ",0):PRINT A$

CALL KEXT(A$,"AI ",1):PRINT A$
AI

VEJA TAMBEM
Zenkaku and Hankaku

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call kinstr", "", "MSX-BASIC", "CALL KINSTR(numeric variable,[,N],X$,Y$)", "This command checks from which position the Y$ starts in relation to beginning of a...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Search for a certain character data among a character data, and substitute found position into a numeric variable.

SINTAXE
CALL KINSTR(numeric variable,[,N],X$,Y$)

_ KINSTR(numeric variable,[,N],X$,Y$)

Numeric variable: Variable of numeric type.

N: 1 to 256. Numeric constant, variable, array variable or expression. Default=1.

X$, Y$: Character constant, variable, array variable or expression.

FUNCAO
This command checks from which position the Y$ starts in relation to beginning of a character string X$, and substitutes the found value into a numeric variable. If you state a value for N, the search takes place from Nth character in X$.

You must use this CALL KINSTR command instead of INSTR function if the character in question is of Japanese input type.

EXEMPLO
CALL KINSTR(A,"TOKYOzA33","A"):PRINT A
 8

* If value of N is greater than length of X$, if X$ is a null string, or if Y$ is not found, then this command substitutes 0 to the numeric variable. And if Y$ is a null string, then this command substitutes 1 to the numeric variable. (However, it substitutes 0 if X$ is also a null string)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call klen", "", "MSX-BASIC", "CALL KLEN(numeric variable,X$[,option])", "Substitute number of all characters in (length of) a character data if a value of the...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Substitute number of all characters (length)/hankaku characters only/zenkaku characters only in a character data to a numeric variable.

SINTAXE
CALL KLEN(numeric variable,X$[,option])

_ KLEN(numeric variable,X$[,option])

Numeric variable: Variable of numeric type.

X$: Character constant, variable, array variable or expression.

Option: 0 to 2. Default=0.

FUNCAO
Substitute number of all characters in (length of) a character data if a value of the option is 0 or omitted, number of hankaku characters if 1, and number of zenkaku characters if 2.

You must use this CALL KLEN command instead of LEN function if the character in question is of Japanese input type.

EXEMPLO
CALL KLEN(A,"JOI112"):PRINT A
 7

CALL KLEN(A,"JOI112",1):PRINT A
 6

CALL KLEN(A,"JOI112",2):PRINT A
 1

* A hankaku accent will be counted as an independent character too.

VEJA TAMBEM
Zenkaku and Hankaku

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call kmid", "", "MSX-BASIC", "CALL KMID(character variable,X$,M[,N])", "Choose N characters from Mth character in a character data, and substitute them into a...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Choose a part of a character data and substitute it to a character variable.

SINTAXE
CALL KMID(character variable,X$,M[,N])

_ KMID(character variable,X$,M[,N])

Character variable: Variable of character type.

X$: Character constant, variable, array variable or expression.

M: 1 to 256. Numeric constant, variable, array variable or expression.

N: 0 to 256. Numeric constant, variable, array variable or expression. Default= all characters from M to last.

FUNCAO
Choose N characters from Mth character in a character data, and substitute them into a chracter variable.

You must use this CALL KMID command instead of MID function if the character in question is of Japanese input type.

EXEMPLO
CALL KMID(A$,"! (invertido)uIA",4,2):PRINT A$

* A hankaku accent will be counted as an independent character too.

VEJA TAMBEM
Zenkaku and Hankaku

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call knj", "", "MSX-BASIC", "CALL KNJ(character variable,X$)", "Specify a hexadecimal 4-digits kanji code to X$. The kanji code will be treated as a JIS...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Substitute a character that stands for a specified kanji code to a character variable.

SINTAXE
CALL KNJ(character variable,X$)

_ KNJ(character variable,X$)

Character variable: Variable of character type.

X$: Character variable, variable, array variable or expression which stands for a 4-digits kanji code.

FUNCAO
Specify a hexadecimal 4-digits kanji code to X$. The kanji code will be treated as a JIS code is lower than 8000, or as a Shift-JIS code if it is 8000 or more.

EXEMPLO
CALL KNJ(A$,"3441"):PRINT A$
S? (invertido)

VEJA TAMBEM
About how to access Kanji-ROM

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call ktype", "", "MSX-BASIC", "CALL KTYPE(numeric variable,X$,N)", "This command substitutes 0 to a variable if a Nth character in X$ is hankaku, 1 if...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Check whether a character in a specified position in a character data is hankaku or zenkaku.

SINTAXE
CALL KTYPE(numeric variable,X$,N)

_ KTYPE(numeric variable,X$,N)

Numeric variable: Variable of numeric type.

X$: Character constant, variable, array variable or expression.

N: 1 to 255. Numeric constant, variable, array variable or expression.

FUNCAO
This command substitutes 0 to a variable if a Nth character in X$ is hankaku, 1 if zenkaku.

If you specify N larger than number of characters in X$, you will get "Illegal function call" error.

EXEMPLO
CALL KTYPE(A,"EY SSp",1):PRINT A
 0

CALL KTYPE(A,"EY SSp",5):PRINT A
 1

* A hankaku accent will be counted as an independent character too.

VEJA TAMBEM
Zenkaku and Hankaku

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call latin", "", "MSX-BASIC", "", "call latin",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call menu", "", "MSX-BASIC", "CALL MENU", "Boots up the MSX-computers main-menu (firmware). Any unsaved data will be lost.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Firmware)

This function is available in japanese A1-WX/WSX and arabic machines.

SINTAXE
CALL MENU

FUNCAO
Boots up the MSX-computers main-menu (firmware). Any unsaved data will be lost.

This is similar to CALL MWP in the MSX A1-GT (Turbo-R)
and CALL HIRO in the A1-ST.

Takamichi Suzukawa: These CALL commands are not written in any manual, but were notified in a
magazine (MSX-FAN) as late as in 1994.

EXEMPLO
CALL MENU

VEJA TAMBEM
CALL MWP,
CALL HIRO,
Arabic-Basic

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call music", "", "MSX-BASIC", "CALL MUSIC[( <drum> [,0 [,n1 [,n2 [... [,n9]]]]]]]]])]", "Initializes the FM-Basic. If drum is 0 then no drums will be used and 9 channels are...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL MUSIC[( <drum> [,0 [,n1 [,n2 [... [,n9]]]]]]]]])]

FUNCAO
Initializes the FM-Basic. If drum is 0 then no drums will be used and 9 channels are available. If drum is 1 then drums will be used and 6 channels are available. n1 to n9 are the voices available to MSX-Music. If drums are used then only voices n1 to n6 can be used.

EXEMPLOS
CALL MUSIC( 1, 0, 2, 1, 1, 1, 1, )

Initializes the FM-Basic and requests the use of drums. The 6 channels will be assigned to the 1st 5 voices. The 1st voice gets two channels, all other voices will be assigned to one channel. This example makes use of drum (1) and therefore the maximum number of channels is 6.

If you want to use no drums or 9 voices, the used command is:

CALL MUSIC( 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 )

Now it's possible to use different channels per voice. The maximum number of channels is 9.

VEJA TAMBEM
CALL BGM, PLAY (MSX-Music)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call musicbox", "", "MSX-BASIC", "", "CALL MUSICBOX activates the firmware of the Philips Music Module",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Firmware)

CALL MUSICBOX activates the firmware of the Philips Music Module
if bypassed by pressing ESC during startup.

Lex Lechz: the CALL MUSICBOX command is not documented in any manual
and can therefore be used as "hidden" feature.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call mwp", "", "MSX-BASIC", "", "CALL MWP boots up the Word Processor in the MSX A1-GT (Turbo-R).",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Firmware)

CALL MWP boots up the Word Processor in the MSX A1-GT (Turbo-R).

This is similar to CALL HIRO in the A1-ST. In case of A1-WX/WSX machines
the corresponding command is CALL MENU.

Takamichi Suzukawa: These CALL commands are not written in any manual, but were notified in a
magazine (MSX-FAN) as late as in 1994.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call nocon", "", "MSX-BASIC", "", "call nocon",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call options", "", "MSX-BASIC", "CALL OPTIONS [ ( [<number1> [, <number2>]] ) ]", "(?) Select printer-options. This command displays a selection-screen to choose...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

SINTAXE
CALL OPTIONS [ ( [<number1> [, <number2>]] ) ]

FUNCAO
(?) Select printer-options. This command displays a selection-screen to choose printer-options. Confirmation returns to the calling (Basic-)program.

EXEMPLO
CALL OPTIONS

VEJA TAMBEM
CALL HCOPY, CALL CHCOPY

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call paint", "", "MSX-BASIC", "", "\"The Painter\" is arabic MSX firmware produced by sakhr co. It is located in PAINT.ROM and...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

"The Painter" is arabic MSX firmware produced by sakhr co. It is located in PAINT.ROM and can be activated by entering CALL PAINT on the basic prompt.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call painter", "", "MSX-BASIC", "", "\"The Painter\" is MSX firmware produced by Yamaha. It is located in PAINTER.ROM and can be...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

"The Painter" is MSX firmware produced by Yamaha. It is located in PAINTER.ROM and can be activated by entering CALL PAINTER on the basic prompt.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call palette", "", "MSX-BASIC", "CALL PALETTE(palette no.,red lumincance,green luminance,blue luminance)", "You must use this command to specify colors in Japansese input mode. COLOR= statement is...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Specify display color to a palette during Japanese input mode.

SINTAXE
CALL PALETTE(palette no.,red lumincance,green luminance,blue luminance)

_PALETTE(palette no.,red lumincance,green lumincance,blue luminance)

Palette no.: 0 to 15. Numerical constant, variable, array variable or expression.

Red/green/blue luminance: 0 to 7. Integer constant.

FUNCAO
You must use this command to specify colors in Japansese input mode. COLOR= statement is disabled in Japanese input mode.

If you omit all parameters (execute simply CALL PALETTE), all palettes return to default.

EXEMPLO
CALL PALETTE(15,4,4,4)

VEJA TAMBEM
COLOR=

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call pitch", "", "MSX-BASIC", "CALL PITCH( number ) - where number is between 410 and 459", "Is used to adjust the base-sound \"A\" to the frequency of 440Hz. Normally the frequency of...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL PITCH( number ) - where number is between 410 and 459

FUNCAO
Is used to adjust the base-sound "A" to the frequency of 440Hz. Normally the frequency of "A" is 440Hz, but can be changed within a range of 410 and 459Hz by specifying n. All other notes will be adjusted accordingly.

EXEMPLO
10 CALL MUSIC(0,0,1)
20 CALL VOICE(@2)
30 PLAY #2,"A1B1"
40 CALL PITCH(459)
50 PLAY #2,"A1B1"
60 END

VEJA TAMBEM
CALL MUSIC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call play", "", "MSX-BASIC", "CALL PLAY( number, var )", "This detimes if music is being played and returns the",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL PLAY( number, var )

number is the channel to check

var is the name of the variable to store the value in

FUNCAO
This  detimes  if  music is being played and  returns  the
result as a value.

EXEMPLO
CALL PLAY(0,A):PRINT A

When number=0 if any channels are playing, the value -1
will be stored in A and 0 if no channels are playing.

VEJA TAMBEM
PLAY (MSX-Music), CALL MUSIC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call presend", "", "MSX-BASIC", "", "call presend",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call presort", "", "MSX-BASIC", "", "call presort",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call quran", "", "MSX-BASIC", "", "CALL QURAN is implemented in the arabic \"Holy Quran\" cartridge.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

CALL QURAN is implemented in the arabic "Holy Quran" cartridge.
This command can be executed after the ROM startup was bypassed by
pressing the "Graph" key.

CALL QURAN reboots the MSX to restart the "Holy Quran" again.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call recon", "", "MSX-BASIC", "", "call recon",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call run", "", "MSX-BASIC", "CALL RUN", "This command compiles and executes the entire program on memory. If",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Turbo-Basic compiler)

SINTAXE
CALL RUN

FUNCAO
This command compiles and executes the entire program on memory. If
it finds an error it will stop and yield the message.

EXEMPLO
CALL RUN

VEJA TAMBEM
CALL TURBO ON, CALL TURBO OFF, RUN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call schange", "", "MSX-BASIC", "CALL SCHANGE", "In case the PAUSE and COPY buttons of the Chakkari Copy cartridge don't work",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Chakkari Copy)

SINTAXE
CALL SCHANGE

FUNCAO
In case the PAUSE and COPY buttons of the Chakkari Copy cartridge don't work
the CALL SCHANGE statement restores this functionality. Most common use is for
MSX-Basic programs (when MSX-Basic is started the buttons on the Chakkari Copy
cartridge will not work).

The CALL SCHANGE command is also available on function-key "Shift+F5" and will
be removed after executing it once.

CALL SCHANGE works only if the cartridge is in COPY mode!!! This statement has no effect while in RAM mode.

Details: if, for some reason, the COPY and PAUSE buttons do not work the CALL SCHANGE command restores this functionality.

It switches the MSX BIOS (page 0) from slot 0 to a patched copy within the slot, where the Chakkari Copy cartridge is inserted.
The patched copy is read-only and initialized while booting the system with Chakkari Copy inserted into any slot.

NOTE: Because of a programming issue in this cartridge the ROMSLT (&HFFF7) is not updated.
Therefore, depending on executed MSX programs, the BIOS patch might loose it's effect.

Another bug prevents correct function-key reassignment for "Shift+F5" on MSX2 and later machines (works on MSX1 only).
So the CLS+"run" command is not available by pressing Shift+F5 on such computers.

EXEMPLO
CALL SCHANGE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call sjis", "", "MSX-BASIC", "CALL SJIS(character variable,X$)", "If a first character in X$ is a 2-byte character, this command substitutes the SJIS code...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Kanji-Basic)

Substitue a shift-JIS code which corresponds to first character in a character data to a numeric variable.

SINTAXE
CALL SJIS(character variable,X$)

_ SJIS(character variable,X$)

Character variable: Variable of character type.

X$: Character constant, variable, array variable or expression.

FUNCAO
If a first character in X$ is a 2-byte character, this command substitutes the SJIS code (hex, 4 digits) of that character into a numeric variable.

EXEMPLO
CALL SJIS(A$,"S? (invertido)Zs"):PRINT A$
8ABF

VEJA TAMBEM
About how to access Kanji-ROM

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call spaint", "", "MSX-BASIC", "", "call spaint",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call stopm", "", "MSX-BASIC", "CALL STOPM", "Stop play of BGM.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL STOPM

FUNCAO
Stop play of BGM.

EXEMPLO
CALL STOPM

VEJA TAMBEM
PLAY (MSX-Music), CALL BGM, CALL MUSIC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call swp", "", "MSX-BASIC", "", "\"Sahkr Word Processor\" is arabic MSX firmware produced by sakhr co. It is located in...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

"Sahkr Word Processor" is arabic MSX firmware produced by sakhr co. It is located in SWP.ROM and can be activated by entering CALL SWP on the basic prompt.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call temper", "", "MSX-BASIC", "CALL TEMPER( number )", "Adjusts the temperament of the scale used by the FM-Basic. Using CALL TEMPER you can...",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL TEMPER( number )

FUNCAO
Adjusts the temperament of the scale used by the FM-Basic. Using CALL TEMPER you can specify 22 different atmospheres (listed below). n can have values from 0 to 21. The default-value is 9.

ATMOSPHERES
No. | Rhythm
0 | Pythograph
1 | Mintone
2 | Welkmeyster
3 | Welkmeyster (adjusted)
4 | Welkmeyster (seperate)
5 | Kilanbuger
6 | Kilanbuger  (adjusted)
7 | Velotte Young
8 | Lamour
9 | Perfect Rhythm (default)
10 | c | Pure Rhythm  c major | (a minor)
11 | cis | Pure Rhythm  cis major | (b minor)
12 | d | Pure Rhythm  d major | (h minor)
13 | es | Pure Rhythm  es major | (c minor)
14 | e | Pure Rhythm  e major | (cis minor)
15 | f | Pure Rhythm  f major | (d minor)
16 | fis | Pure Rhythm  fis major | (es minor)
17 | g | Pure Rhythm  g major | (e minor)
18 | gis | Pure Rhythm  gis major | (f minor)
19 | a | Pure Rhythm  a major | (fis minor)
20 | b | Pure Rhythm  b minor | (g minor)
21 | h | Pure Rhythm  h minor | (gis minor)

EXEMPLO
10 CALL MUSIC(0,0,1)
20 CALL BGM(0)
30 CALL VOICE(@1)
40 FOR N=0 TO 21 : PRINT"Atmospere";N
50 CALL TEMPER(N)
60 PLAY #2,"CDEFGAB>C<"
70 NEXT N
80 END

VEJA TAMBEM
CALL MUSIC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call time", "", "MSX-BASIC", "CALL TIME( <string-variable> )", "CALL TIME( <string-variable> )",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

SINTAXE
CALL TIME( <string-variable> )

EXEMPLO
CALL TIME(T$)
PRINT T$

VEJA TAMBEM
GET TIME

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call togreg", "", "MSX-BASIC", "", "call togreg",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call tohijri", "", "MSX-BASIC", "", "call tohijri",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call transpose", "", "MSX-BASIC", "CALL TRANSPOSE( number )", "Will tranpose in units of half tones 1/100 units.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL TRANSPOSE( number )

FUNCAO
Will tranpose in units of half tones 1/100 units.

number can have values from -12799 to 12799 or a variable.
However, some voices have a limited setting even through within the +-12799 range.

Default setting is 0

EXEMPLOS
CALL TRANSPOSE (100)

tranposes a half tone.

VEJA TAMBEM
CALL MUSIC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call turbo off", "", "MSX-BASIC", "CALL TURBO OFF", "The end of a turbo-block started with CALL TURBO ON.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Turbo-Basic compiler)

SINTAXE
CALL TURBO OFF

FUNCAO
The end of a turbo-block started with CALL TURBO ON.

EXEMPLO
100 SCREEN 8:DEFINT A-Z
110 BLOAD"PICTURE",S
120 _TURBO ON
130 FOR X=0 TO 255
140 LINE(X,0)-(X,211),0
150 NEXT X
160 _TURBO OFF
170 GOTO 170

VEJA TAMBEM
CALL TURBO ON, CALL RUN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call turbo on", "", "MSX-BASIC", "CALL TURBO ON", "Defines the beginning of the turbo block.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Turbo-Basic compiler)

SINTAXE
CALL TURBO ON

FUNCAO
Defines the beginning of the turbo block.
The turbo block is the part of the program you want to execute fast. When the
entire program contains some uncompilable statements, you can define the
block to be compiled using this set.

The following statements, commands and functions that can not be compiled:

AUTO, BASE, BLOAD, BSAVE, CALL, CDBL, CINT, CLEAR, CLOAD, CLOAD?, CLOSE,
CONT, CSAVE, CSNG, CVD, CVI, CVS, DEFFN, DELETE, DRAW, DSKF, EOF, ERASE, ERL,
ERR, ERROR, EQV, FIELD, FILES, FPOS, FRE, GET, IMP, INPUT#, KEY LIST, LFILES,
LINEINPUT#, LIST, LLIST, LOAD, LOC, LOF, LPRINT USING, LSET, MAXFILES, MERGE,
MOTOR, MKD$, MKI$, MKS$, NAME, NEW, ON ERROR GOTO, ON INTERVAL GOSUB (due to
a bug), OPEN, PLAY, PRINT#, PRINT USING, PUT KANJI, RENUM, RESUME, RSET,
SAVE, SPC, TAB, TRON, TROFF, WIDTH.

List of those with limits.

CIRCLE: Start, end angles and aspect ratio can't be specified.

COPY: Only graphic COPY is available.

DEFDBL: Same as DEFSNG.

DIM: Must come first in the program or in the turbo block.

INPUT: Can handle only one variable at the time.

KEY: ON KEY GOSUB, KEY(n) ON/OFF only.

LOCATE: x,y must be given in as a set. No cursor switch parameter.

NEXT: Variable names after the NEXT can not be omitted.

ON: ON STOP GOSUB, ON INTERVAL GOSUB not available.

PRINT: Commas work in a different way. No wrapping for digits.

PUT: PUT SPRITE only.

RUN: Variables won't be initialized.

SCREEN: Screen mode and sprite size only.

SET: SET PAGE only.

STOP: Same as END.

USR: Parameter type must be integer only.

VARPTR: File number can not be given as the parameter.

Otherwise there is no significant difference.

In general, I/O commands & functions, and editing commands can not be
compiled. Of course they are available in the direct mode, and outside of the
turbo block. You can edit, debug and save a program in MSX-BASIC and execute
it by _RUN.

If you want to use PRINT# to write characters on GRP:, use it outside of
turbo block. Otherwise study the sample, "PRINT.TRB".

If you want to use PLAY, use BGM compiler, and get the sound by USR(n).

EXEMPLO
100 SCREEN 8:DEFINT A-Z
110 BLOAD"PICTURE",S
120 _TURBO ON
130 FOR X=0 TO 255
140 LINE(X,0)-(X,211),0
150 NEXT X
160 _TURBO OFF
170 GOTO 170

VEJA TAMBEM
CALL TURBO OFF, CALL RUN

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call voice", "", "MSX-BASIC", "CALL VOICE( [@n1], [@n2], ..., [@n9] )", "Stes what instrument is to be played by the FM",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL VOICE( [@n1], [@n2], ..., [@n9] )

n1-n9 can be values from 0-63 (voice nr) or variables.

Default setting is @0

FUNCAO
Stes  what  instrument  is to be played by  the  FM
sound channels.  However,  values without * mark and array
values (refer to MSX-Music Voices) are limited to 2
when used together.

Note: The voice number can be selected from the table MSX-Music Voices table of voices numbers.

EXEMPLO
CALL VOICE(@0,@31,@32)

CALL VOICE(@0,@6,@25,@24)

CALL VOICE(,,@48)

VEJA TAMBEM
CALL VOICE COPY,
CALL MUSIC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call voice copy", "", "MSX-BASIC", "CALL VOICE COPY( @n1, @n2 )", "This command copies the voice data to number 63 or an",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(MSX-Music)

SINTAXE
CALL VOICE COPY( @n1, @n2 )

n1 can be voices from 0-63 that have no * mark or array variables.

n2 can be 63 or an array variable name.

FUNCAO
This   command  copies the voice data to number 63  or  an
array variable.

Note: choose the voice from the table MSX-Music Voices.

EXEMPLO
DIM A%(16)
CALL VOICE COPY (@7,A%)

Pipe organ 1 is copied to array variable A.

10 CALL MUSIC
20 DIM K(15)
30 CALL VOICE COPY(@31, K)
40 CALL VOICE COPY(K, @63)

Copies the software-instrument BASSDRUM from the ROM into free space.
On first view this does not have any useful effect. It is also possible to use CALL VOICE
to copy instrument 31 from the computer directly.

More interesting is the possibility to manipulate data in the array K,
which enables creation of new instruments.

VEJA TAMBEM
CALL VOICE,
CALL MUSIC

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("call vprint", "", "MSX-BASIC", "", "call vprint",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC - CALL (extensoes de firmware))

(Arabic-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("attr$", "", "MSX-BASIC", "", "This keyword is reserved for future use.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

(MSX-Basic)

This keyword is reserved for future use.

VEJA TAMBEM
Reserved keywords

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cmd", "", "MSX-BASIC", "", "This keyword is reserved for future use.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

(MSX-Basic)

This keyword is reserved for future use.

VEJA TAMBEM
Reserved keywords

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("copy screen", "", "MSX-BASIC", "COPY SCREEN [<mode>]", "This instruction is used to specify that a general signal must be digitized.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

(MSX-Basic 2)

SINTAXE
COPY SCREEN [<mode>]

FUNCAO
This instruction is used to specify that a general signal must be digitized.
mode indicates the mode of digitization.

This statement is used for writing data from the colour bus to VRAM, for
example, after digitising. This is valid for screen modes 5 to 8.

Attention: This statement is for the superimposer or the digitiser which are optional,
so it can be used only for machines which are equipped with this the 'superimpose' capability.

In mode 0, one field of signals [=1 frame] is digitised and written to the display page
(that is the page reproduced on screen).

In mode 1, two successive fields (that is, one frame) of signals [=2 frames] are written
to (display page - 1)th page and the display page, so the display page should
be an odd page when the mode is 1.

The default mode is 0.

EXEMPLO
COPY SCREEN

COPY SCREEN 1

110 ON STOP GOSUB 210
120 STOP ON
130 SCREEN 8
140 COLOR,,255
150 SET VIDEO 1
160 COPY SCREEN
170 TIME=0
180 IF INKEY$="" GOTO160
190 IF TIME<4 GOTO 190
200 IF INKEY$="" GOTO 200 ELSE 140
210 'STOP
220 SET VIDEO 0,0
230 END

This program requires to push 'audio/video' button on the MSX-console.

VEJA TAMBEM
SET VIDEO,
SET PAGE,
COPY (MSX2-Basic),
COPY (Disk-Basic)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("fpos", "", "MSX-BASIC", "FPOS( channel )", "This keyword is reserved for future use.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

(MSX-Basic)

This keyword is reserved for future use.

SINTAXE
FPOS( channel )

VEJA TAMBEM
Reserved keywords,
OPEN,
CLOSE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ipl", "", "MSX-BASIC", "", "This keyword is reserved for future use.",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

(MSX-Basic)

This keyword is reserved for future use.

VEJA TAMBEM
Reserved keywords

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("operators", "", "MSX-BASIC", "", "Syntax: <X|X$> <Operator> <Y|Y$> or <Operator> <X|X$>",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

Syntax: <X|X$> <Operator> <Y|Y$> or <Operator> <X|X$>

Operators are used to make expressions (calculations). Underneath is a table with all operators known to MSX Basic, divided in the class sections arithmetical, bitwise, relational and adjudicational:

Class | Operator | Function | Notes
Arithmetical | + | unary plus | has no effect
- | unary minus | yields the negation of its numeric argument
+ | addition | can be used on strings too
- | subtraction
* | multiplication
/ | division
\ | integer division
MOD | modulo | calculates the dividend of an integer division
^ | to the power of
Bitwise | NOT | logical complement | NOT 1 = 0NOT 0 = 1 | one's complements (all bits are inverted)
AND | logical conjunction | 1 AND 1 = 11 AND 0 = 00 AND 1 = 00 AND 0 = 0 | the bit is set if both bits are set
OR | logical inclusive or | 1 OR 1 = 11 OR 0 = 10 OR 1 = 10 OR 0 = 0 | the bit is set if either one of the bits is set
XOR | logical exclusive or | 1 XOR 1 = 01 XOR 0 = 10 XOR 1 = 10 XOR 0 = 0 | the bit is set if one bit is set and the other one isn't
EQV | logical equivalence | 1 EQV 1 = 11 EQV 0 = 00 EQV 1 = 00 EQV 0 = 1 | the bit is set if both bits are equal
IMP | logical implication | 1 IMP 1 = 11 IMP 0 = 00 IMP 1 = 10 IMP 0 = 1 | the bit is not set when the first bit being true implicates that the second bit is false, it is otherwise set
Relational (comparators) | = | equals to
< | smaller than
> | larger than
<> or >< | not equals to (smaller or larger than)
<= or =< | equals to or smaller than
>= or => | equals to or larger than
Adjudicational | = | assignment | only used to assign values to variables

The associativity (reading distance) is always from left to right, with the exception of the unary operators. Priorities are as followed:

Operator(s) priority level
( )
^
*, /, and \
unary operators
MOD
+ and -
relational operators
bitwise operators

Parentheses: ( ) can be used to override any priority; they can be nested, eg. A=((1+2)*3)^2.

VEJA TAMBEM
Variables,

AND,
EQV,
IMP,
MOD,
NOT,
OR,
XOR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("variables", "", "MSX-BASIC", "", "Syntax to assign a value to a variable: [LET] <variable name> = <X|X$>",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

Syntax to assign a value to a variable: [LET] <variable name> = <X|X$>

The LET command is optional, and it's commonly never used.

Variables can have names up to only 2 characters, excluding the variable type character, example: X11% and X12% will both be X1%. A variable name can contain letters and numbers, and must begin with a letter. Continue this chapter for examples.

Variable types
Variable conversion functions
Definitions (DEF <type>)
Arrays (DIM, ERASE)

Variable types:

MSX Basic knows 4 types of variables which can be seen in the table underneath. The variable type character is part of the variable name, Example: A$, A%, and A are 3 different variables. If the variable isn't defined manually, MSX Basic defines it to Double Precision.

The variable types below are ordered from simple to complex. With each more complex variable type, the size gets bigger and the calculation speed gets slower.

Type | Notation | Notes
Integer | decimal: <n>%binary: &B<n>octal: &O<n>hexadecimal: &H<n> | (examples) - DEF INT can also be used to define this type. Value can range between -32768 and 32767, thus, this is a signed 16-bit value where the msb (most significant bit) determines if the value is negative (1), or positive (0). There is no -0; '-0'=-32768.
Single Precision | normal: <n>!exponential: <mantissa>E <exponent> | (examples) - DEF SNG can also be used to define this type. This type can hold up to 6 numbers, if it exceeds that, zeroes are appended.
Double Precision | normal: <n>#exponential: <mantissa>D <exponent> | (examples) - DEF DBL can also be used to define this type. This type can hold up to 14 numbers.
String | <n>$ | (examples) - DEF STR can also be used to define this type. A String can be up to 255 characters long, dependent on the free string memory assigned with CLEAR.

<n> is a value (excluding the String type), or a variable name.

def int
def sng
def dbl
def str
f87f  99
dim
let

cint
cdbl
bin$
csng
hex$
oct$
str$
val

time

Variable conversion functions:

CINT,
CSNG,
CDBL,
ASC

MKI$,
MKS$,
MKD$,
CHR$

DEF <type: INT|DBL|SNG|STR> <letter>[,-<last letter>] [<letter 2> [,-<last letter 2>]]

DEFINT,
DEFSNG,
DEFDBL,
DEFSTR

DIM|ERASE <array name>(<maximum index>)[,<array name>...]

DIM,
ERASE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("using", "", "MSX-BASIC", "", "The syntax of the USING clause in the MSX-Basic PRINT/LPRINT statement",
        R"MAN(REFERENCIA EM INGLES (RuMSX, MSX-BASIC (outros))

(MSX-Basic)

The syntax of the USING clause in the MSX-Basic PRINT/LPRINT statement
may differ between various implementations:

Generic / all versions

"!" | Indicates that only the 1st character of the expression will be printed
10 A$="Europe"
            20 PRINT USING "!";A$
            30 END
            RUN
            E
            Ok
"#" | Numerical expressions can be printed as following:
      USING "#" indicates how many characters of the expression should be printed.
      If the expression has less numbers than the number of positions MSX-Basic
      inserts preceding spaces.
      If the expression has more characters as the number of positions the expression
      is preceded by a percent (%) character.
      If necessary MSX-Basic rounds the value.
10 A=109:B=7:C=1198
            20 PRINT USING "###";A,B,C
            30 END
            RUN
            109  7%1198
            Ok
"." | Indicates where the decimal point should be printed.
      Only one decimal point is allowed.
10 A=10.21:B=5.5:C=.245:D=3
            20 PRINT USING "##.##";A,B,C,D
            30 END
            RUN
            10.21 5.50 0.25 3.00
            Ok
"," | If a comma is placed on the left side of the decimal point
      then every 3rd character will be preceded by a comma (e.g. thousands place).
      If a comma is placed on the end of the format then the command is printed
      immediately after the output.
10 B=1234.5
            20 PRINT USING "####,.##";
            30 PRINT USING "####.##,";
            40 END
            RUN
            1,234.50
            1234.50,
            Ok
"+" | Indicates that a positive expression should be indicated by a plus (+) character
      and negative expression should be indicated by a minus (-) character.
      If the plus character is the first element in the print format,
      then the character is printed first. If the plus is placed on the end of the
      format then it's printed immediately after the value.
10 A=1.25:B=-1.25
            20 PRINT USING "+#.##";A,B
            30 PRINT USING "#.##+";A,B
            40 END
            RUN
            +1.25-1.25
            1.25+1.25-
            Ok
"-" | This symbol can only be used at the end of the print-format.
      It results in a space after a positive expression and a minus (-)
      after a negative expression.
10 A=1.25:B=-1.25
            20 PRINT USING "#.##-";A,B
            30 END
            RUN
            1.25 1.25-
            Ok
"" | The both asteriks can only be used on the start of the print-format.
      It replaces leading zeros by asterisks.
10 A=10.25:B=1.25:C=-1.25
            20 PRINT USING "#.##";A,B
            30 END
            RUN
            *10.251.25*-1.25
            Ok
"^^^^" | This symbol can only be used on the end of a print-format.
      It results in an exponential display of the expression.
      The decimal point can be placed somewhere. The first position
      is the space, except a plus or minus is specified in the format.
10 A=12.35
            20 PRINT USING "##.##^^^^";A
            30 PRINT USING "#.##^^^^-";C
            40 PRINT USING "+#.##^^^^";B,C
            50 END
            RUN
             2.35E+02
            1.23E+01-
            +1.23E+01-1.23E+01
            Ok

Japanese

"& &" | Indicates that the number of characters to be printed is 2 plus the number
      of spaces between & and &.
      Note: International machines are supporting "\ \" instead.
10 A$="Japanese"
            20 PRINT USING "&   &";A$
            30 END
            RUN
            Japan
            Ok
"@" | Indicates that instead of the "@" symbol the value of
      the variable should be printed.
      Note: International machines are supporting "&" instead.
10 A$="I":B$="MSX"
            20 PRINT USING "@ love @";A$,B$
            30 END
            RUN
            I love MSX
            Ok
"" | The both yen () signs can only be used on the start of the print-format.
      It results in a leading yen-character of the output.
      Note: Interational machines are supporting "$$" to display the dollar ($) sign instead.
10 A=12.35:B=-12.35
            20 PRINT USING "###.##";A,B
            30 PRINT USING "###.##-";A,B
            40 END
            RUN
              12.35 -12.35
              12.35   12.35-
            Ok
"" | This symbol can only be used on the start of the print-format.
      It is a combination of the prior formats.
      Note: Interational machines are supporting "$" to use the dollar ($) sign instead.
10 A=12.35
            20 PRINT USING "#.##";A
            30 END
            RUN
            *12.35
            Ok

International

"\ \" | Indicates that the number of characters to be printed is 2 plus the number
      of spaces between \ and \.
      Note: Japanese machines are supporting "& &" instead.
10 A$="Europe"
            20 PRINT USING "\  \";A$
            30 END
            RUN
            Euro
            Ok
"&" | Indicates that instead of the "&" symbol the value of
      the variable should be printed.
      Note: Japanese machines are supporting "@" instead.
10 A$="I":B$="MSX"
            20 PRINT USING "& love &";A$,B$
            30 END
            RUN
            I love MSX
            Ok
"$$" | The both dollar ($) signs can only be used on the start of the print-format.
      It results in a leading dollar-character of the output.
      Note: Japanese machines are supporting"" to display the yen () sign instead.
10 A=12.35:B=-12.35
            20 PRINT USING "$$###.##";A,B
            30 PRINT USING "$$###.##-";A,B
            40 END
            RUN
              $12.35 -$12.35
              $12.35   $12.35-
            Ok
"$" | This symbol can only be used on the start of the print-format.
      It is a combination of the prior formats.
      Note: Japanese machines are supporting "" to use the yen () sign instead.
10 A=12.35
            20 PRINT USING "$#.##";A
            30 END
            RUN
            *$12.35
            Ok

VEJA TAMBEM
PRINT,
LPRINT

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-alarm", "", "Referencia BASIC", "", "Since the alarm feature is optional, the action taken at the specified time depends on...",
        R"MAN(REFERENCIA (RuMSX)

Since the alarm feature is optional, the action taken at the specified time depends on the machine (ordinarily nothing happens).

When the alarm is to be set in both "SET DATE" and "SET TIME", "SET TIME " should be done first (when "SET TIME" is done, date of the alarm set by "SET DATE" will be erased).

The minimum setting for alarm is in minutes (setting in seconds is ignored).

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-color-table", "", "Referencia BASIC", "", "The color-table contains the 16 color palette, which can be modified on MSX2 (and higher)...",
        R"MAN(REFERENCIA (RuMSX)

The color-table contains the 16 color palette, which can be modified on MSX2 (and higher) machines. Default values are as following:

Colornumber | Name | IntensityRed, Green, Blue | MSX2, ... | True MSX1
0 | transparent | 0, 0, 0
1 | black | 0, 0, 0
2 | green | 1, 6, 1
3 | light green | 3, 7, 3
4 | dark blue | 1, 1, 7
5 | light blue | 2, 3, 7
6 | dark red | 5, 1, 1
7 | cyan | 2, 6, 7
8 | red | 7, 1, 1
9 | light red | 7, 3, 3
10 | dark yellow | 6, 6, 1
11 | light yellow | 6, 6, 4
12 | dark green | 1, 4, 1
13 | magenta | 6, 2, 5
14 | gray | 5, 5, 5
15 | white | 7, 7, 7

Note: original MSX1 computers did use a slightly different palette,
which is shown in the right column.

Exceptions:

All screen modes

In all screen-modes palette-color 0 is normally defined as transparent but, it's can also be used as a normal non-transparent color by using:

VDP(9)=VDP(9) OR &H20

to use it as a transparent color again use:

VDP(9)=VDP(9) AND &HDF

SCREEN 6

In this screen-mode (which has only four colors) spripped patterns can be used to fill the border. For the values 0-3 the border acts normal but the values 16-31 have the following meaning:

Border-color | Stripped patternusing colors | Border-color | Stripped patternusing colors
16 | 0 and 0 | 24 | 2 and 0
17 | 0 and 1 | 25 | 2 and 1
18 | 0 and 2 | 26 | 2 and 2
19 | 0 and 3 | 27 | 2 and 3
20 | 1 and 0 | 28 | 3 and 0
21 | 1 and 1 | 29 | 3 and 1
22 | 1 and 2 | 30 | 3 and 2
23 | 1 and 3 | 31 | 3 and 3

SCREEN 8

The color-value in screen 8 can be 0-255, the color is defined by the following formula:

color-number = 4 x R + 32 x G + B. The values for Red and Green can be 0-7, the values for Blue can be 0-3 (they correspond to 0, 2, 4 and 7).

This means that the bits are representing colors as following: G G G R R R B B.

Colornumber | +0 | +1 | +2 | +3 | +4 | +5 | +6 | +7 | +8 | +9 | +10 | +11 | +12 | +13 | +14 | +15
0
16
32
48
64
80
96
112
128
144
160
176
192
208
224
240

EXEMPLO
10 SCREEN 8
20 FOR B=0 TO 3
30 FOR R=0 TO 7
40 FOR G=0 TO 7
50 C=4*R+32*G+B
60 LINE (X,0)-(X,211),C,BF
70 X=X+1:NEXT G,R,B
80 A$=INPUT$(1)

Sprite-colors in screen 8 are hardcoded too and mapped to the following values:

Colornumber | IntensityRed, Green, Blue | Sprite color
0 | 0, 0, 0
1 | 0, 0, 2
2 | 3, 0, 0
3 | 3, 0, 2
4 | 0, 3, 0
5 | 0, 3, 2
6 | 3, 3, 0
7 | 3, 3, 2
8 | 7, 4, 2
9 | 0, 0, 7
10 | 7, 0, 0
11 | 7, 0, 7
12 | 0, 7, 0
13 | 0, 7, 7
14 | 7, 7, 0
15 | 7, 7, 7
Please remember the ranges for R (0-7), G (0-7) and B (0-3).

SCREEN 10+11

Screen mode 10+11 do support 5120 (5*32*32) colors and are essentially equal.
They only differ in the behavior, how pixels can be handled by BIOS-routines:

SCREEN 10: 4 bits per pixel to use palette colors.

SCREEN 11: 8 bits per pixel to access the whole pixel.

Similar to screen mode 12, 4 pixels are grouped together and have the same
color but different brightness.

+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Byte 0 | brightn. | brightn. | brightn. | brightn. | Palette  |  green   |  green   |  green   |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Byte 1 | brightn. | brightn. | brightn. | brightn. | Palette  |  blue    |  green   |  green   |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Byte 2 | brightn. | brightn. | brightn. | brightn. | Palette  |  red     |  red     |  red     |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Byte 3 | brightn. | brightn. | brightn. | brightn. | Palette  |  blue    |  red     |  red     |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+

If the Palette bit is set (=1) the 4 brightness bits are interpreted as
palette color. If the Palette bit is reset (=0) the brightness bits are
interpreted as brightness-value for the 4 pixel-groups color.

In screen mode 10 MSX-Basic graphic operations are always setting the Palette
bit and do use the selected color (in range 0-15).

In screen mode 11 MSX-Basic graphic operations are using the selected color (in range 0-255)
.

SCREEN 12

Screen mode 12 supports 19268 different colors. In this mode 4 pixels are
grouped together and do have the same color, but different brightness:

+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Byte 0 | brightn. | brightn. | brightn. | brightn. | brightn. |  green   |  green   |  green   |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Byte 1 | brightn. | brightn. | brightn. | brightn. | brightn. |  blue    |  green   |  green   |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Byte 2 | brightn. | brightn. | brightn. | brightn. | brightn. |  red     |  red     |  red     |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+
| Byte 3 | brightn. | brightn. | brightn. | brightn. | brightn. |  blue    |  red     |  red     |
+--------+----------+----------+----------+----------+----------+----------+----------+----------+

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-error-codes", "", "Referencia BASIC", "", "The following error-codes are used by the ERROR statement and ERR function.",
        R"MAN(REFERENCIA (RuMSX)

(MSX-Basic, Disk-Basic)

The following error-codes are used by the ERROR statement and ERR function.

Error | Message
1 | NEXT without FOR
2 | Syntax error
3 | RETURN without GOSUB
4 | Out of DATA
5 | Illegal function call
6 | Overflow
7 | Out of memory
8 | Undefined line number
9 | Subscript out of range
10 | Redimensioned array
11 | Division by zero
12 | Illegal direct
13 | Type mismatch
14 | Out of string space
15 | String too long
16 | String formular too complex
17 | Can t CONTINUE
18 | Undefined user function
19 | Device I/O error
20 | Verify error
21 | No RESUME
22 | RESUME without error
23 | Unprintable error
24 | Missing operand
25 | Line buffer overflow

Disk-Basic: following additional ERRORs are defined:

Error | Message
50 | FIELD overvlow
51 | Internal error
52 | Bad file number
53 | File not found
54 | File already open
55 | Input past end
56 | Bad file name
57 | Direct statement in file
58 | Sequential I/O only
59 | File not OPEN
60 | Bad FAT
61 | Bad file mode
62 | Bad drive name
63 | Bad sector number
64 | File still open
65 | File already exists
66 | Disk full
67 | Too many files
68 | Disk write protected
69 | Disk I/O error
70 | Disk offline
71 | Rename across disk

VEJA TAMBEM
ERR,
ERROR

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-firmware", "", "Referencia BASIC", "", "Firmware is particular to the specific model of a MSX-computer.",
        R"MAN(REFERENCIA (RuMSX)

Firmware is particular to the specific model of a MSX-computer.

Availability of implemented commands, parameters and behavior are
changing with the computer-model or extension-ROM.
Therefore MSX-programs should not assume Firmware extensions to
be present on (almost) every MSX computer!

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-interlace-mode", "", "Referencia BASIC", "", "Differences of display function in the interlace mode",
        R"MAN(REFERENCIA (RuMSX)

Differences of display function in the interlace mode

Interlace mode | Display function
0 | Normal non-interlaced display (default)
1 | Interlaced display
2 | Non interlaced, Even/Odd alternate display
3 | Interlaced, Even/Odd alternate display

VEJA TAMBEM
SCREEN,
SET PAGE

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-arabic-basic", "", "Referencia BASIC", "", "Arabic MSX-computers were found to have following extensions:",
        R"MAN(REFERENCIA (RuMSX)

Arabic MSX-computers were found to have following extensions:

CALL OPTIONS
CALL CALENDAR
CALL GPRINT
CALL ARB0
CALL ARB1
CALL ARB2
CALL ENGL
CALL LATIN
CALL RECON
CALL NOCON
CALL INDVAL
CALL ARBVAL
CALL DCOLOR
CALL PRESORT
CALL PRESEND
CALL COMPRESS
CALL ASTR
CALL ESTR
CALL FNKEY
CALL HCOPY
CALL CHCOPY
CALL SPAINT
CALL VPRINT
CALL TIME
CALL GREG
CALL HIJRI
CALL TOHIJRI
CALL TOGREG
CALL ARABIC
CALL ARABON
CALL ARABOFF

Firmware extension:

CALL MENU

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-msx-audio-basic", "", "Referencia BASIC", "", "The MSX-Audio is a rarely sold cartrige, which was replaced by MSX-Music (lower quality)....",
        R"MAN(REFERENCIA (RuMSX)

The MSX-Audio is a rarely sold cartrige, which was replaced by MSX-Music (lower quality). The Philips Music Module is hardware-compatible to the MSX-Audio, but has no MSX-Audio basic (the Music Module uses a separate firmware ROM).

The firmware of both cartridges (MSX-Audio and Music-Module) can be bypassed by pressing ESC during startup.

CALL AUDIO

VEJA TAMBEM
MSX-Music,
CALL MUSICBOX

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-hangul-basic", "", "Referencia BASIC", "", "Hangul ROM's are managing Korean Kanji.",
        R"MAN(REFERENCIA (RuMSX)

Hangul ROM's are managing Korean Kanji.

CALL HANON
CALL HANOFF

New since Version 2:

CALL ADJUST
SCREEN 9 mode

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-kanji-basic", "", "Referencia BASIC", "", "Kanji-Basic is available since MSX-DOS 2.20 and MSX Turbo-R's",
        R"MAN(REFERENCIA (RuMSX)

Kanji-Basic is available since MSX-DOS 2.20 and MSX Turbo-R's

CALL PALETTE
CALL CLS
CALL AKCNV
CALL JIS
CALL SJIS
CALL KACNV
CALL KEXT
CALL KINSTR
CALL KLEN
CALL KMID
CALL KNJ
CALL KTYPE
CALL KANJI
CALL KANJI0
CALL KANJI1
CALL KANJI2
CALL KANJI3
CALL ANK

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-msx-music-basic", "", "Referencia BASIC", "", "The MSX-Music is available in the FM-PAC, the FM STEREO PAK and since MSX 2+ (optional)...",
        R"MAN(REFERENCIA (RuMSX)

The MSX-Music is available in the FM-PAC, the FM STEREO PAK and since MSX 2+ (optional) and MSX-Turbo R. The following MSX-Basic commands are supported by FM-Basic:

CALL MUSIC
CALL BGM
CALL PITCH
CALL TEMPER
CALL TRANSPOSE
PLAY
CALL PLAY
CALL STOPM
CALL VOICE
CALL VOICE COPY
CALL AUDREG

VEJA TAMBEM
MSX-Audio

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-reserved-keywords-basic", "", "Referencia BASIC", "", "Following keywords are reserved on all MSX machines. These keywords can be use to extend...",
        R"MAN(REFERENCIA (RuMSX)

Following keywords are reserved on all MSX machines. These keywords can be use to extend the MSX-Basic funtionality:

ATTR$
CMD
IPL
FPOS

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-turbo-basic-compiler", "", "Referencia BASIC", "", "The Turbo-Basic compiler is a separately sold cartridge.",
        R"MAN(REFERENCIA (RuMSX)

The Turbo-Basic compiler is a separately sold cartridge.
It can be found in MSX2+ Sanyo machines too.
It is also known as KUN-Basic or X-Basic.
The following additional commands are provided:

CALL TURBO ON
CALL TURBO OFF
CALL RUN
CALL BC (in MSX-2+ Sanyo only)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-msx-music-voices", "", "Referencia BASIC", "", "This is a list of the voices available for use with Voice command or the",
        R"MAN(REFERENCIA (RuMSX)

This is a list of the voices available for use with Voice command or the
 MML @ statement.

Voice No. | Name | Voice No. | Name
0 * | Piano 1 | 32 | Piano 3
1 | Piano 2 | 33 * | Wood Bass
2 * | Violin | 34 | Santule 2
3 * | Flute 1 | 35 | Brass
4 * | Clarinet | 36 | Flute 2
5 * | Oboe | 37 | Clavicord 2
6 * | Trumpet | 38 | Clavicord 3
7 | Pipe Organ | 39 | Koto 2
8 | Xylophone | 40 | Pipe Organ 2
9 * | Organ | 41 | P0hdsPLA
10 * | Guitar | 42 | P0hdsPRA
11 | Santule 1 | 43 | Church Organ L
12 * | Electric bass | 44 | Church Organ R
13 | Clavicord 1 | 45 | Synth Violin
14 * | Hapsicord | 46 | Synth Organ
15 | Hapsicord | 47 | Synth Brass
16 * | Vibraphone | 48 * | Horn
17 | Koto 1 | 49 | Shamisen
18 | Taiko | 50 | Magical
19 | Engine 1 | 51 | Fuwawa
20 | UFO | 52 | Wonderflat
21 | Synth Bell | 53 | Hard Rock
22 | Chime | 54 | Machine
23 * | Synth Bass | 55 | Machine V
24 * | Synthesizer | 56 | Comic
25 | Synth drum | 57 | SE comic
26 | Synth rhythm | 58 | SE laser
27 | Harmo drum | 59 | SE noise
28 | Cow Bell | 60 | SE star 1
29 | Hi Hat | 61 | SE star 2
30 | Snare drum | 62 | Engine 2
31 | Bass drum | 63 | no sound
* are sounds that are build in the FM sound chip.

VEJA TAMBEM
CALL VOICE,
CALL VOICE COPY,
PLAY (MSX-Music)

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-screen-modes", "", "Referencia BASIC", "", "Correspondances of BASIC screen (SCREEN) modes and VDP screen modes",
        R"MAN(REFERENCIA (RuMSX)

Correspondances of BASIC screen (SCREEN) modes and VDP screen modes

BASICmode | VDPmode | Meaning
Dots or characters | Display colours at a time | Screen format
SCREEN 0 (1) | TEXT 1 | 40 x 24 chars | 2 from 512 | Text
SCREEN 0 (2) | TEXT 2 | 80 x 24 chars | 2 from 512 | Text
SCREEN 1 | GRAPHIC 1 | 32 x 24 chars | 16 from 512 | Text
SCREEN 2 | GRAPHIC 2 | 256 x 192 dots | 16 from 512 | High res. graphics
SCREEN 3 | MULTICOLOUR | 64 x 48 dots | 16 from 512 | Low res. graphics
SCREEN 4 | GRAPHIC 3 | 256 x 192 dots | 16 from 512 | High res. graphics
SCREEN 5 | GRAPHIC 4 | 256 x 212 dots | 16 from 512 | Bit map graphics
SCREEN 6 | GRAPHIC 5 | 512 x 212 dots | 4 from 512 | Bit map graphics
SCREEN 7 | GRAPHIC 6 | 512 x 212 dots | 16 from 512 | Bit map graphics
SCREEN 8 | GRAPHIC 7 | 256 x 212 dots | 256 | Bit map graphics
SCREEN 9* | GRAPHIC 5 | 80 x 24 chars | 4 from 512 | Text
SCREEN 10 | Graphic 8 | 256 x 212 dots | A lot + 16 from 512 | Bit map graphics
SCREEN 11 | Graphic 8 | 256 x 212 dots | A lot + 16 from 512 | Bit map graphics
SCREEN 12 | Graphic 9 | 256 x 212 dots | 19268 | Bit map graphics

*SCREEN 9: only implemented in Korean MSX2 computers to display Korean KANJI characters.
This mode is not implemented in the normal MSX-BIOS but part of the Hangul-ROM.

This screen-mode is quite curious, but available in the normal MSX2 Video-processor (V9938) too.
The effective VDP-mode is GRAPHIC 5 (SCREEN 6) with some specific settings (see table below).
The only difference to other MSX-machines is the support of this mode by BIOS routines.
In fact SCREEN 9 has the following properties:

Property | Value | Notes
Resolution | 80 x 24 characters | can be adjusted by the WIDTH statement
512 x 384 dots | interlaced (2*192 lines)
Characters | 6*16 pixels | are read from the Kanji-ROM, which (may?) need to be present
Text color | green (3) | Text color is effectively set to 15 (white), but GRAPHIC 5 mode uses only the
        lower 2 bits therefore the value of the displayed color is 3.
Background color | black (1)
Border color | black (1)
Sprites | disabled
Note: when activating SCREEN 9 the system automatically changes the used colors. When leaving SCREEN 9 and
returning to another screen-mode the changed colors remain valid.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-ram-disk", "", "Referencia BASIC", "", "Note: the MSX2 RAM-Disk can not store binary files.",
        R"MAN(REFERENCIA (RuMSX)

(MSX-Basic 2)

CALL MEMINI
CALL MFILES
CALL MKILL
CALL MNAME

Note: the MSX2 RAM-Disk can not store binary files.
All data is stored in ASCII-mode.

VEJA TAMBEM
RAM-Disk in Disk-Basic 2, RAM-Disk in MSX-DOS 2

REFERENCIA (RuMSX)

(Disk-Basic 2)

CALL RAMDISK

VEJA TAMBEM
RAM-Disk in MSX-Basic 2, RAM-Disk in MSX-DOS 2

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ref-zenkakuandhankaku-htm", "", "Referencia BASIC", "", "Zenkaku and Hankaku",
        R"MAN(REFERENCIA (RuMSX)

Zenkaku and Hankaku

"Zen" means whole, "han" means "half", "kaku" stands for "square" in Japanese. In MSX or in any word processing related with computers, hankaku is an adjective to describe 1-byte characters (English alphabets, numbers, western punctuation, unaccented katakanas, Japanese punctuation, Japanese accents; MSX supports unaccented hiraganas, unlike PC) while zenkaku, "2-byte" characters, include all hiraganas and katakanas, both accented and unaccented, English alphabets, special pictograms like large circles and measurement terms, Greek and Russian fonts, and thousands of kanjis.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-dir", "", "MSX-DOS 1", "DIR [/H] [/W] [/P] [compound-filespec]", "Displays the names of files on disk.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
DIR [/H] [/W] [/P] [compound-filespec]

FINALIDADE
Displays the names of files on disk.

USO
The  compound-filespec specifies which files are to  be
    listed.  If  the /H option is given,  then hidden files will
    also be listed.

         In  the DIR command,  unlike all other commands,  it is
    permissible  to not give the main filename or  the  filename
    extension,  and both will default to '*'. Thus a filename of
    'FRED' is equivalent to 'FRED.*' and a filename of '.COM' is
    equivalent to '*.COM'.  Note that if the '.' at the end of a
    main  filename is given,  then the extension is also assumed
    to  have  been given,  so that the filename 'FRED.'  is  not
    equivalent to 'FRED.*', unlike the example above.

         There are two formats of the listing.  If the /W option
    is  given,  then a 'wide' listing is printed,  with  several
    filenames  output  per  line.   Sub-directory  names,   file
    attributes,  and the date and time each file was created are
    not displayed.

         If  the /W option is not given,  then the filenames are
    printed  with  one  filename per  line,  together  with  the
    attributes, the file size and the date and time at which the
    file was last modified. The attributes are printed as an 'r'
    if  the file is read-only and an 'h' if the file  is  hidden
    (and  /H is given).  If the time of a file is zero (i.e. the
    file  does not have an associated time) then the time  field
    will  not be printed.  If the date of a file is  zero,  then
    neither  the date nor the time fields will be  printed.  The
    formats  in which the dates and times are printed in can  be
    changed (see HELP DATE and HELP TIME).

         The  non-/W  display  is designed to fit  within  a  40
    column screen,  but if fewer columns are available then some
    fields  of the listing will not be shown so that the display
    will  always fit on one line.  The number of files per  line
    that  are  printed  when /W is specified  is  also  adjusted
    according  to the screen width.  If the width of the display
    is less than 13 characters however,  then in both cases  the
    filenames will wrap to the next line.

         At the top of the list of files, the volume name of the
    disk  and  the  name  of  the  directory  being  listed   is
    displayed.  At the bottom,  the number of files listed,  the
    total  number  of  bytes  in the files  and  the  amount  of
    remaining disk space is printed.

         When  the directory of a sub-directory is printed,  the
    first   two  items  listed  will  always  be   two   special
    sub-directories called '.' and '..'. These are automatically
    created  when  a new directory is created,  and it is  these
    that allow '.' and '..' to be given in path names to signify
    the  current and parent directories respectively  (see  HELP
    SYNTAX for a description of paths).

         When  printing  a  number  of  bytes,   the  number  is
    truncated  and  printed as the number of kilobytes if 1K  or
    greater.

         If the /P option is given,  then the output will  pause
    at the bottom of the screen until a key is pressed.

EXEMPLOS
    DIR

         All  filenames  and  directory  names  in  the  current
    directory of the current drive will be printed.  This  might
    be as follows:

          Volume in drive A: is MSX-DOS 2
          Directory of A:\

         MSXDOS2  SYS r    4096 86-06-19  2:45p
         COMMAND2 COM r   10496 86-06-19  2:46p
         UTILS           <dir>  86-06-19  2:50p
         HELP            <dir>  86-06-19  2:50p
          14K in 2 files   222K free

    The   disk  thus  contains  the  two  MSX-DOS  system  files
    MSXDOS2.SYS and COMMAND2.COM,  which are read only,  and two
    directories called UTILS and HELP.

    DIR B:\HELP/W

         A  'wide'  directory format has been requested  of  the
    HELP directory on drive B:. This might be as follows:

          Volume in drive B: is MSX-DOS 2
          Directory of B:\HELP

         BUFFERS .HLP    ATTRIB  .HLP    ASSIGN  .HLP
         ATDIR   .HLP    CHDIR   .HLP    CD      .HLP
         SYNTAX  .HLP    ENV     .HLP    BATCH   .HLP
         EDITING .HLP
          25K in 10 files   222K free

    DIR  UTILS + HELP/P

         This will list all the files in the UTILS directory and
    all  the files in the HELP directory,  and will pause at the
    end of every screen full.

    DIR .COM

         No main filename was given,  and so defaults to *. Thus
    this command is equivalent to the command DIR *.COM.

    DIR COMMAND2

         No extension was given,  so this defaults to  .*.  Thus
    this command is equivalent to the command DIR COMMAND2.*.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-copy", "", "MSX-DOS 1", "COPY [/H] [/P] [/A] [/B] [/V] [/T] source dest", "Copies data from files/devices to other files/devices.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
COPY [/H] [/P] [/A] [/B] [/V] [/T] source dest

FINALIDADE
Copies data from files/devices to other files/devices.

USO
The definition of the source is:

          compound-filespec

    The  compound-filespec specifies the files or  devices  that
    are  to be copied.  If /H is given then hidden files may  be
    copied.

         The definition of the dest is:

          [d:][path][filename] | device

    where d: and path default to the current drive and directory
    respectively.  If any part of the filename is ambiguous then
    the  appropriate  character  from  the  source  filename  is
    substituted,  thus  allowing the files to be renamed in  the
    process.  If  the  filename is not given,  then  the  entire
    source  filename  is used.  If the dest  is  an  unambiguous
    directory,  then  the  files are copied into that  directory
    with a filename of *.*.

         COPY  will  read as many source files as possible  into
    memory before writing any out. When it can read no more into
    memory (eg.  when it has used all available memory) it  will
    write out each file in the order that it read them.  When it
    creates  each destination file,  it prints the source  file-
    name.  Then  if it is unable to create the destination file,
    an error message is printed and the copy operation continues
    with the next file. /P can be given to make the output pause
    at the end of the screen.

         Many  reasons exist for COPY to be unable to create the
    destination,  such as a read-only file already existing with
    the  same  name.  Sometimes COPY will refuse to  create  the
    destination  because the user may have made a  mistake.  For
    example,  a  file cannot be copied onto itself,  or  several
    files  cannot  be copied onto one  file.  A  'Cannot  create
    destination'  error  may be given if the destination of  one
    file  would delete a previous source file or a file  already
    being used for something else (eg.  the currently  executing
    batch file).  A 'Cannot overwrite previous destination file'
    error  results  if an attempt is made to copy many files  to
    one file.  This usually means that the intended  destination
    was a directory, but that the name has been misspelled.

         If  /A is specified,  then an ASCII copy is  performed.
    This means that source files will only be read as far as the
    first   end-of-file   character  (CTRL-Z)  and   then   each
    destination   will  have  a  single  end-of-file   character
    appended to it.

         It is also possible to give a /A to the destination  or
    to any of the filespecs in the compound-filespec separately,
    in  which  case  it  applies only to  that  source  or  dest
    specification. Also /B may be given to reverse the effect of
    /A.

         The  /V option can be given to turn write  verification
    on  for  the  duration of the COPY command (see  the  VERIFY
    command). This will ensure that data is written correctly to
    disks but will slow the operation down.

         Normally, the destination files are given the same date
    and time as the source files.  However, the /T option can be
    given  to  cause the destination files to have  the  current
    date  and time.  The destination files will not be hidden or
    read-only, regardless of the attributes of the source files.
    The ATTRIB command can be used to change these.

         If COPY gives a 'Not enough memory' error then probably
    reducing the number of buffers (see the BUFFERS command)  or
    removing some environment items (see HELP ENV on Environment
    Items) will free up sufficient memory.

         Note  that the COPY command is simpler than that in MS-
    DOS  and  MSX-DOS  1 because  it  cannot  concatenate  (join
    together) files.  To do this,  a CONCAT command is available
    (see the CONCAT command).

EXEMPLOS
    COPY FILE1 B:

         The  file FILE1 is copied from the current directory of
    the current drive to the current directory of drive B:.

    COPY /H  MSXDOS2.SYS + COMMAND2.COM  B:

         The  two hidden files MSXDOS2.SYS and COMMAND2.COM  are
    copied to drive B:, thus making it a booting disk.

    COPY A:\DIR1 B:\DIR1 /V

         All files in the directory DIR1 from the root of  drive
    A: are copied to a similar directory on drive B: with verify
    on to ensure that the files were written correctly.

    COPY B:

         All  files  in the current directory of  drive  B:  are
    copied to the current directory of the current drive.

    COPY /A AUX CON

         Characters  are read from the device AUX (which may  be
    used for RS232 serial for example) to the device CON,  which
    is the screen.  The copy is done as far as the first end-of-
    file  character.  If /A was not given,  then there may  have
    been  no way of stopping the COPY operation without pressing
    the CTRL-STOP key.

    COPY A:*.DOC B:/T

         All  files  matching  *.DOC  (for  example   FILE1.DOC,
    FILE2.DOC and FILE3.DOC) are copied to the current directory
    of drive B:  and are given the current date and time instead
    of the dates and times of the *.DOC files.

    COPY *.BAT
    AUTOEXEC.BAT -- File cannot be copied onto itself
    REBOOT.BAT -- File cannot be copied onto itself

         This command told COPY to copy all files matching *.BAT
    (in  this case AUTOEXEC.BAT and REBOOT.BAT) from the current
    directory of the current drive to the same place,  and  COPY
    printed  the messages to warn of this.  No data in this case
    was actually copied.

    COPY *.BAT DIR2
    AUTOEXEC.BAT
    REBOOT.BAT -- Cannot overwrite previous destination file

         This command told COPY to copy all files matching *.BAT
    (in  this case AUTOEXEC.BAT and REBOOT.BAT) to  a  directory
    called DIR2.  DIR2,  however,  did not exist so AUTOEXEC.BAT
    was  copied to a file called DIR2,  and then an attempt  was
    made  to  copy REBOOT.BAT also to a file  called  DIR2.  The
    message was printed as a warning that a mistake was probably
    made  (in this case DIR2 not existing).  REBOOT.BAT was  not
    actually copied anywhere.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-type", "", "MSX-DOS 1", "TYPE [/H] [/P] [/B] compound-filespec | device", "Displays data from a file or device.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
TYPE [/H] [/P] [/B] compound-filespec | device

FINALIDADE
Displays data from a file or device.

USO
    The  compound-filespec specifies the files that are  to
    be displayed, and /H allows hidden files to be typed. If the
    compound-filespec is ambiguous, then the filename is printed
    before each one is typed.

         If  /B is specified,  then data is read from each  file
    and displayed without modification on the screen,  until the
    end of file is reached.  This may have strange effect on the
    screen if the file contains control characters.

         If /B is not given, then TYPE will look for the end-of-
    file  character  (CTRL-Z) and stop when it  finds  it.  Also
    control characters except carriage return, line feed and tab
    will be converted into a '^' symbol followed by that control
    letter so that they will be visible on the screen.
         If /P is given,  then the output will pause at the  end
    of the screen until a key is pressed.

EXEMPLOS
    TYPE FILE1

         Data  is read from the file and printed on the  screen,
    up to the first end-of-file character.

    TYPE *.BAT/H/P

         All batch files, including hidden ones, are read in and
    displayed.  A  prompt is printed at the end of every  screen
    full.

    TYPE AUTOEXEC.BAT + REBOOT.BAT

         The files AUTOEXEC.BAT and REBOOT.BAT are displayed.

    TYPE /B DIR1

         All  files  in the directory DIR1 are  printed  on  the
    screen  and  no  interpretation is put on the  data  in  the
    files.

    TYPE AUX

         Data  is  read from the AUX device (which may  be,  for
    example, an RS232 serial device) up to the first end-of-file
    character and printed on the screen.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-ren", "", "MSX-DOS 1", "RENAME [/H] [/P] compound-filespec filename", "Renames one or more files.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
RENAME [/H] [/P] compound-filespec filename

REN [/H] [/P] compound-filespec filename

FINALIDADE
Renames one or more files.

USO
    The  compound-filespec specifies the files that are  to
    be renamed, and /H allows hidden files to be included in the
    rename operation.

         The  second  filename specifies the new  name  for  the
    files.   A   '?'   in  the  new  name  indicates  that   the
    corresponding character from the filename being renamed will
    be used,  thus allowing an ambiguous rename. Thus '*' in the
    second  filename,  which is just equivalent to a  series  of
    '?'s,  indicates that the whole of the filename or extension
    will remain unchanged.

         If  for some reason a particular file cannot be renamed
    (eg.  if  a  file  or directory with the  new  name  already
    exists)  then the offending filename will be  printed  along
    with an error message and the rename operation will continue
    with the next file. If many errors occur, then /P will cause
    the output to pause at the end of the screen.

EXEMPLOS
    RENAME FILE1 FILE2

         The  file FILE1 in the current directory of the current
    drive is renamed to FILE2.

    REN B:\DIR1\*.DOC/H/P *.OLD
    FILE2.DOC -- File exists

         All  files matching *.DOC in the directory called  DIR1
    in the root directory of drive B:,  including hidden  files,
    are renamed with the same main name but with an extension of
    .OLD.  The file FILE2.DOC could not be renamed because there
    was already a file called FILE2.OLD in the directory, so the
    error  was  printed.  Neither  FILE2.DOC nor  FILE2.OLD  was
    altered at all. If many such errors had been printed, then a
    prompt would have been printed at the bottom of every screen
    full.

    REN DOC + FILE1 *.OLD

         All  files  in the directory called DOC  and  the  file
    FILE1,  both  in the current directory of the current drive,
    and renamed with an extension of .OLD.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-del", "", "MSX-DOS 1", "DEL [/H] [/P] compound-filespec", "Deletes one or more files.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
DEL [/H] [/P] compound-filespec

ERA [/H] [/P] compound-filespec

ERASE [/H] [/P] compound-filespec

FINALIDADE
Deletes one or more files.

USO
    The  compound-filespec specifies which files are to  be
    deleted.  The  /H  option  allows hidden files  to  also  be
    deleted.

         During  the  delete  operation,  if a  file  cannot  be
    deleted for some reason (eg.  it is set to 'read only') then
    the  offending  filename  is printed  along  with  an  error
    message,  and  the delete operation continues with the  next
    file.  If  many such errors occur,  then the /P option  will
    cause the output to pause at the end of the screen.

         If the filename is *.*, then the prompt:

          Erase all files (Y/N) ?

    is  printed,  and  a reply is waited for.  If the  reply  is
    anything other than 'Y' or 'y',  then the file deletion does
    not take place. This is a safety feature designed to prevent
    accidental loss of all files in a directory.

         If files are deleted unintentionally on a disk that was
    formatted  under  MSX-DOS 2,  then the UNDEL command may  be
    used immediately afterwards to restore them again.

EXEMPLOS
    ERASE FILE1.BAK

         The   file  FILE1.BAK  is  deleted  from  the   current
    directory of the current drive.

    DEL *.COM/H

         All  files matching *.COM,  both hidden and not hidden,
    are deleted.

    DEL B:\UTIL\*.COM + B:\UTIL\*.BAT

         All files matching *.COM or *.BAT are deleted from  the
    directory called UTIL on drive B:.

    DEL B:\UTIL
    Erase all files (Y/N)?

         All files in the directory called UTIL on drive B:  are
    deleted.  Since so many files are being deleted, a prompt is
    printed first to prevent a catastrophe.

    DEL *.BAT
    AUTOEXEC.BAT -- Read only file
    REBOOT.BAT -- Read only file

         All  files  matching  *.BAT  are  deleted  except   for
    AUTOEXEC.BAT  and REBOOT.BAT which have been marked as  read
    only.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-date", "", "MSX-DOS 1", "DATE [date]", "Displays or sets the current date.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
DATE [date]

FINALIDADE
Displays or sets the current date.

USO
    If the date is given after the command,  then the  date
    is set to this value (for the format see below). If the date
    is  not  given after the command,  then the current day  and
    date is printed and the new date is prompted for and  input.
    If  no  input  is given (i.e. if the 'enter'  key  alone  is
    pressed) then the current date is not altered. Otherwise the
    input  is  assumed to be a new date,  and is interpreted  as
    described  below.  If  the  date is invalid  then  an  error
    message is displayed and the new date again prompted for and
    input.

         The date is expected to consist of up to three numbers,
    separated by one of the following characters:

         space tab , - . / :

    with  spaces  allowed  either side  of  the  character.  Any
    missing  numbers  will default to the current  setting.  The
    year may either be a full century and year,  or may be  just
    the  year  in which case the century defaults to 19  if  the
    year is greater than 80 or 20 otherwise.

         The  format  in which the date is printed and input  is
    flexible and can be changed. An environment item called DATE
    is set up by default to a format that is appropriate for the
    country  of  origin  of the MSX machine  (see  HELP  ENV  on
    Environment  Items).  For example,  on Japanese machines the
    default setting is YY-MM-DD.  The command SET DATE  DD-MM-YY
    will  change  the date format to the UK format.  The  format
    also affects the dates printed by the DIR command.

         If  the DATE environment item is defined,  then it will
    be  printed  by the DATE command to indicate the  format  in
    which the date is required to be input.

EXEMPLOS
    DATE 86-6-18

         The current date is set to the 18th June 1986.

    DATE
    Current date is Wed 1986-06-18
    Enter new date (yy-mm-dd): - -19

         No  parameter  was given,  so the current date of  18th
    June  1986 was printed and a new date prompted for.  In  the
    reply to the prompt, the date was updated to the next day by
    only specifying the 19th.  Since the year and month were not
    given, they remained the same.

    SET DATE = DD/MM/YY

         The date format has been changed to the UK format.

    DATE
    Current date is Thu 19-06-1986
    Enter new date (DD/MM/YY):

         No  parameter was given,  so the current date  of  19th
    June  1986  was  printed in the UK format,  and  the  prompt
    printed. The reply is expected in the UK format.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-time", "", "MSX-DOS 1", "TIME [time]", "Displays or sets the current time.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
TIME [time]

FINALIDADE
Displays or sets the current time.

USO
    If  the time is given after the command,  then the time
    is set to this value (for the format see below). If the time
    is  not given after the command,  then the current  time  is
    printed  and the new time is prompted for and input.  If  no
    input  is  given (i.e. if the 'enter' key alone is  pressed)
    then the current time is not altered. Otherwise the input is
    assumed  to be a new time,  and is interpreted as  described
    below.  If  the  time is invalid then an  error  message  is
    displayed and the new time again prompted for and input.

         The  time is expected to consist of up to four numbers,
    separated by one of the following characters:

         space tab , - . / :

    with  spaces  allowed  either side  of  the  character.  Any
    missing  numbers will default to the  current  setting.  The
    first  number is the hour,  the second is the  minutes,  the
    third is the seconds and the forth is the centi-seconds. The
    centi-seconds  are not printed however since it is not  very
    useful to know the current value,  or indeed to enter a  new
    one.

         The format in which the time is printed is flexible and
    can   be  changed.   An  environment item (see  HELP ENV  on
    Environment  Items) called TIME is set up by default to  the
    value '12', which indicates that the time will be printed in
    12  hour format with a following 'a' or 'p' for am  and  pm.
    The command SET TIME 24 will cause the time to be printed in
    24 hour mode.  The time can be input unambiguously in either
    format.  The  time format also affects the times printed  by
    the DIR command.

EXEMPLOS
    TIME 16:45

         The current time is set to 4:45 pm.

    TIME
    Current time is 10:45:58a
    Enter new time:

         No  parameters  were  given,  so the  current  time  is
    printed   (in this case in 12 hour mode) and  the  new  time
    prompted for.

    TIME 10-50-30-23

         The time is set to 30.23 seconds after 10:50 am.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-rename", "", "MSX-DOS 1", "RENAME [/H] [/P] compound-filespec filename", "Renames one or more files.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
RENAME [/H] [/P] compound-filespec filename

REN [/H] [/P] compound-filespec filename

FINALIDADE
Renames one or more files.

USO
    The  compound-filespec specifies the files that are  to
    be renamed, and /H allows hidden files to be included in the
    rename operation.

         The  second  filename specifies the new  name  for  the
    files.   A   '?'   in  the  new  name  indicates  that   the
    corresponding character from the filename being renamed will
    be used,  thus allowing an ambiguous rename. Thus '*' in the
    second  filename,  which is just equivalent to a  series  of
    '?'s,  indicates that the whole of the filename or extension
    will remain unchanged.

         If  for some reason a particular file cannot be renamed
    (eg.  if  a  file  or directory with the  new  name  already
    exists)  then the offending filename will be  printed  along
    with an error message and the rename operation will continue
    with the next file. If many errors occur, then /P will cause
    the output to pause at the end of the screen.

EXEMPLOS
    RENAME FILE1 FILE2

         The  file FILE1 in the current directory of the current
    drive is renamed to FILE2.

    REN B:\DIR1\*.DOC/H/P *.OLD
    FILE2.DOC -- File exists

         All  files matching *.DOC in the directory called  DIR1
    in the root directory of drive B:,  including hidden  files,
    are renamed with the same main name but with an extension of
    .OLD.  The file FILE2.DOC could not be renamed because there
    was already a file called FILE2.OLD in the directory, so the
    error  was  printed.  Neither  FILE2.DOC nor  FILE2.OLD  was
    altered at all. If many such errors had been printed, then a
    prompt would have been printed at the bottom of every screen
    full.

    REN DOC + FILE1 *.OLD

         All  files  in the directory called DOC  and  the  file
    FILE1,  both  in the current directory of the current drive,
    and renamed with an extension of .OLD.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-era", "", "MSX-DOS 1", "DEL [/H] [/P] compound-filespec", "Deletes one or more files.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
DEL [/H] [/P] compound-filespec

ERA [/H] [/P] compound-filespec

ERASE [/H] [/P] compound-filespec

FINALIDADE
Deletes one or more files.

USO
    The  compound-filespec specifies which files are to  be
    deleted.  The  /H  option  allows hidden files  to  also  be
    deleted.

         During  the  delete  operation,  if a  file  cannot  be
    deleted for some reason (eg.  it is set to 'read only') then
    the  offending  filename  is printed  along  with  an  error
    message,  and  the delete operation continues with the  next
    file.  If  many such errors occur,  then the /P option  will
    cause the output to pause at the end of the screen.

         If the filename is *.*, then the prompt:

          Erase all files (Y/N) ?

    is  printed,  and  a reply is waited for.  If the  reply  is
    anything other than 'Y' or 'y',  then the file deletion does
    not take place. This is a safety feature designed to prevent
    accidental loss of all files in a directory.

         If files are deleted unintentionally on a disk that was
    formatted  under  MSX-DOS 2,  then the UNDEL command may  be
    used immediately afterwards to restore them again.

EXEMPLOS
    ERASE FILE1.BAK

         The   file  FILE1.BAK  is  deleted  from  the   current
    directory of the current drive.

    DEL *.COM/H

         All  files matching *.COM,  both hidden and not hidden,
    are deleted.

    DEL B:\UTIL\*.COM + B:\UTIL\*.BAT

         All files matching *.COM or *.BAT are deleted from  the
    directory called UTIL on drive B:.

    DEL B:\UTIL
    Erase all files (Y/N)?

         All files in the directory called UTIL on drive B:  are
    deleted.  Since so many files are being deleted, a prompt is
    printed first to prevent a catastrophe.

    DEL *.BAT
    AUTOEXEC.BAT -- Read only file
    REBOOT.BAT -- Read only file

         All  files  matching  *.BAT  are  deleted  except   for
    AUTOEXEC.BAT  and REBOOT.BAT which have been marked as  read
    only.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-erase", "", "MSX-DOS 1", "DEL [/H] [/P] compound-filespec", "Deletes one or more files.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
DEL [/H] [/P] compound-filespec

ERA [/H] [/P] compound-filespec

ERASE [/H] [/P] compound-filespec

FINALIDADE
Deletes one or more files.

USO
    The  compound-filespec specifies which files are to  be
    deleted.  The  /H  option  allows hidden files  to  also  be
    deleted.

         During  the  delete  operation,  if a  file  cannot  be
    deleted for some reason (eg.  it is set to 'read only') then
    the  offending  filename  is printed  along  with  an  error
    message,  and  the delete operation continues with the  next
    file.  If  many such errors occur,  then the /P option  will
    cause the output to pause at the end of the screen.

         If the filename is *.*, then the prompt:

          Erase all files (Y/N) ?

    is  printed,  and  a reply is waited for.  If the  reply  is
    anything other than 'Y' or 'y',  then the file deletion does
    not take place. This is a safety feature designed to prevent
    accidental loss of all files in a directory.

         If files are deleted unintentionally on a disk that was
    formatted  under  MSX-DOS 2,  then the UNDEL command may  be
    used immediately afterwards to restore them again.

EXEMPLOS
    ERASE FILE1.BAK

         The   file  FILE1.BAK  is  deleted  from  the   current
    directory of the current drive.

    DEL *.COM/H

         All  files matching *.COM,  both hidden and not hidden,
    are deleted.

    DEL B:\UTIL\*.COM + B:\UTIL\*.BAT

         All files matching *.COM or *.BAT are deleted from  the
    directory called UTIL on drive B:.

    DEL B:\UTIL
    Erase all files (Y/N)?

         All files in the directory called UTIL on drive B:  are
    deleted.  Since so many files are being deleted, a prompt is
    printed first to prevent a catastrophe.

    DEL *.BAT
    AUTOEXEC.BAT -- Read only file
    REBOOT.BAT -- Read only file

         All  files  matching  *.BAT  are  deleted  except   for
    AUTOEXEC.BAT  and REBOOT.BAT which have been marked as  read
    only.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-pause", "", "MSX-DOS 1", "PAUSE [comment]", "Prompts and waits for a key press in a batch file.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
PAUSE [comment]

FINALIDADE
Prompts and waits for a key press in a batch file.

USO
    The  comments  consists  of an  arbitrary  sequence  of
    characters.

         The  comment,  if  given,  is printed followed  by  the
    prompt 'Press any key to continue... ' on the next line. The
    system  will then wait for a key to be pressed and will echo
    the  key  pressed  if it is a  printable  character.  If  no
    comment is given as a parameter,  then just the prompt  will
    be printed.

         The  main use of this command is to issue prompts  from
    within a batch file.

EXEMPLOS
    PAUSE
    Press any key to continue...

         No comment was given, so just the prompt was printed.

    PAUSE Insert document disk in drive B:
    Insert document disk in drive B:
    Press any key to continue

         The  comment  given was 'Insert document disk in  drive
    B:' so this was printed followed by the prompt.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-rem", "", "MSX-DOS 1", "REM [comment]", "Introduces a comment in a batch file.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
REM [comment]

FINALIDADE
Introduces a comment in a batch file.

USO
    The  comment  is simply ignored,  and the next  command
    executed.   The  comment  consists  of  a  sequence  of  any
    characters  up to the maximum length of a command line  (127
    characters).

EXEMPLOS
    REM This is my AUTOEXEC batch file

         This command,  either in a batch file or typed in, does
    nothing at all with it's parameters.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-verify", "", "MSX-DOS 1", "VERIFY [ON | OFF]", "Displays/sets the current disk write verify state.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
VERIFY [ON | OFF]

FINALIDADE
Displays/sets the current disk write verify state.

USO
    If  no parameters are given,  then the  current  verify
    state is displayed on the screen.

         If ON or OFF is given, then the verify state is changed
    appropriately.

         The  verify state affects all writes to disk.  If  OFF,
    the default state,  then data is simply written. If ON, then
    after writing the data it is read back and compared with the
    original to ensure that it was written correctly.  The extra
    overhead of this means that writing is slower when verify is
    on.

EXEMPLOS
    VERIFY
    VERIFY=OFF

         No   parameters  were  given,   so  the  current verify
    state is printed, which in this case is off.

    VERIFY ON

         Disk write verification is turned on.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-mode", "", "MSX-DOS 1", "MODE number", "Changes the number of characters/line on the screen.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
MODE number

FINALIDADE
Changes the number of characters/line on the screen.

USO
The number must be in the range 1 to 80 inclusive,  and
    the  number of characters per line on the screen will be set
    to this.  The screen will be cleared and the cursor moved to
    the top left corner in the process.

EXEMPLOS
    MODE 80

         The  screen is set to 80 column mode and is cleared  in
    the process.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-basic", "", "MSX-DOS 1", "BASIC [program]", "Transfers control to MSX disk BASIC.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
BASIC [program]

FINALIDADE
Transfers control to MSX disk BASIC.

USO
    Program is the name of a BASIC program on disk.

         Control is passed to the built-in MSX BASIC, which will
    load  and execute the BASIC program if specified.  If a  RAM
    disk has been set up, then it can still be used from BASIC.

         The  BASIC  command _SYSTEM("command") can be  used  to
    return to MSX-DOS,  and the optional command,  which can  be
    any command mentioned in this manual,  will be executed.  If
    the  command  is  not  given,   then  a  batch  file  called
    REBOOT.BAT  will be  searched  for  and  executed  (see HELP
    BATCH on Batch Files).

EXEMPLOS
    BASIC

         MSX disk BASIC is entered.

    BASIC MYPROG.BAS

         MSX  disk  BASIC  is entered,  and  the  BASIC  program
    MYPROG.BAS loaded and RUN.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-format", "", "MSX-DOS 1", "FORMAT [d:]", "Formats (initializes) a disk.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS)

FORMATO
FORMAT [d:]

FINALIDADE
Formats (initializes) a disk.

USO
    The  specified or default drive is formatted,  and  all
    data on the disk will be destroyed.

         After  giving  a  FORMAT  command,  an  option  may  be
    prompted for, allowing the required format of the disk to be
    selected.  The exact nature of these prompts depends on  the
    manufacturer of the MSX machine.

         After formatting, there will be no files or directories
    on  the disk,  and the maximum amount of disk space will  be
    free. The disk will not have a volume name, but can be given
    one with the VOL command.  To turn the disk into a boot disk
    so  that  MSX-DOS  can  be started up  from  it,  the  files
    MSXDOS2.SYS and COMMAND2.COM must be copied onto it with the
    COPY command.

EXEMPLOS
    FORMAT B:
    1 - Single sided
    2 - Double sided
    ? 2
    All data on drive B: will be destroyed
    Press any key to continue...

         The  command was given to format the disk in drive  B:.
    In  this case,  the options available were to select  either
    double sided or single sided, and double sided was selected.
    The standard warning prompt was then printed.

    FORMAT

         This  will  format the current drive after the  prompts
    given above.

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos-ramdisk", "", "MSX-DOS 1", "RAMDISK [number[K]] [/D]", "Displays or sets the RAM disk size.",
        R"MAN(COMANDO DO MSX-DOS (RuMSX)

(MSX-DOS 2)

FORMATO
RAMDISK [number[K]] [/D]

FINALIDADE
Displays or sets the RAM disk size.

USO
   If  no parameters  are given, then the current RAMDISK size is displayed
as the number of kilobytes.

     The  number, if  given, specifies the maximum size for the new RAM disk,
and is  specified in kilobytes. The range is 0 to 4064. If the number is 0 or
only  /D is  specified, the  RAM disk  will be  deleted. This  number will be
rounded up  to the  nearest multiple  of 16K  since the  RAM disk is always a
multiple  of 16K.  A RAM  disk smaller then the specified maximum size may be
created if there is not enough free memory for the full size, although a 'not
enough memory' error will be given if there is no memory at all available for
the RAM  disk. Note that the number specified is the maximum amount of RAM to
use  for the  RAM disk,  which is  not the same as the maximum amount of free
space available  on the  newly-created RAM disk since the system needs to use
some for FAT or directories.

     On MSX machines with 128K RAM, the maximum amount of RAM disk is 32K.

     If  a  RAM  disk already  exists before  a new  one is  created, then  a
'Destroy  all data  on RAM disk (Y/N)?' prompt is printed to avoid accidental
loss of  data. /D  can be  given which will automatically delete any existing
RAM disk first, thus suppressing the prompt.

     Having created a RAM disk, it can be referred to as drive H:.

     The RAMDISK command is normally only used in an AUTOEXEC.BAT batch file,
with a  large number  specified so  that as  large a  RAMDISK as  possible is
created.  It is  not advisable  to keep  any data  on a RAM disk except for a
short length of time that is not also kept on a floppy disk, since it will be
lost if, for example, the power to the computer fails.

EXEMPLOS
RAMDISK
RAMDISK=160K

No parameters  were given,  so the current size is printed, in this case  160K.

RAMDISK
* RAM disk does not exist

No  parameters were given but no RAM disk has been created, so the error is given.

RAMDISK = 300
Destroy all data on RAM disk (Y/N)? y

A RAM disk already existed, so the prompt was printed. In this case, the
reply was 'y' so the current RAM disk was deleted and the new one set up with
a maximum size of 300K.

VEJA TAMBEM
CALL RAMDISK,
RAM-Disk in MSX-Basic 2,
RAM-Disk in Disk-Basic 2,
RAM-Disk in MSX-DOS 2

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cpm", "", "MSX-DOS 1", "", "The CP/M operating system is optional (in most cases extra sold).",
        R"MAN(CP/M (RuMSX)

The CP/M operating system is optional (in most cases extra sold).

1976: Gary Kildall developed CP/M, an operating system for personal computers.
Widely adopted, CP/M made it possible for one version of a program to run on a
variety of computers built around eight-bit microprocessors.

Internal commands:

DIR filespec

displays filenames from disk-device

ERA filespec

deletes file(s)

PIP newname=oldname

renames file(s)

SAVE size filename

Copies memory-contents starting from 0100H (256*size bytes) into filename

TYPE filename

displays file-contents on the screen

USER number

changes the user-area on disk

External commands:

ASM filename

8080 assembler

DDT filename

debugger/editor for machine-code programs

ED filename

text-editor

FORMAT

formats a disk

MOVCPM

configures CP/M for a different memory-size. Is not used by normal users and therefore
not delivered in many cases.

PIP targetfile=sourcefile

copies file(s)

STAT filespec

STAT options

display directory-contents including filesize and attributes,
sets and resets file-attributes,
changes and displays Iobytes

SUBMIT filename parameter

starts a batch-file

SYSGEN

copies the bootstrap-loader and operating-system onto another disk

XSUB

help-program for batch-processing

FONTE
  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-contents", "", "MSX-DOS 2: manual", "", "MSX-DOS 2: indice do manual de comandos",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

                  M S X -- D O S   v e r s i o n  2
                  =================================

                  The advanced disk operating system

                         for MSX 2 computers

            CONTENTS                                   Page

            1.   Introduction .........................   3
            2.   Editing command lines ................   3
            3.   Notation .............................   6
            4.   Description of Commands ..............  10
            5.   Redirection and Piping ...............  70
            6.   Batch files, AUTOEXEC and REBOOT .....  72
            7.   Environment Items ....................  75
            8.   Errors and Messages ..................  79
            9.   Command Summary ......................  94
           10.   DISK-BASIC 2.0 ....................... 100

     This  manual  describes  the  user  interface  and commands  provided by
MSX-DOS 2 version 2.20.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-introduction", "", "MSX-DOS 2: manual", "", "INTRODUCTION (MSX-DOS 2)",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

1.   INTRODUCTION
=================

     MSX-DOS 2,  like its  predecessor MSX-DOS  1, is provided in a cartridge
and  in some  disk files.  The disk files are MSXDOS2.SYS, COMMAND2.COM, help
files and transient commands.

     MSXDOS2.SYS  has the ability to load and execute programs in an enhanced
CP/M-compatible environment.  COMMAND2.COM is  a special  program which, when
loaded  and executed,  provides the user with many sophisticated commands and
features generally  compatible with and in many cases better than those found
in MS-DOS and MSX-DOS 1, such as extended memory management.

     It  also has the ability to load and execute specially written MSX-DOS 1
programs and  most standard  CP/M programs,  and can execute batch files with
parameter substitution and other features similar to those found in MS-DOS.

     An  APPEND  facility  is  provided  to  increase  the  ease  of  use  of
directories with CP/M programs which were not written to handle them.

     Throughout  the rest  of this  manual the  term MSX-DOS  is used to mean
MSX-DOS version 2.xx unless otherwise stated.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-editing-command-lines", "", "MSX-DOS 2: manual", "", "EDITING COMMAND LINES (MSX-DOS 2)",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

2. EDITING COMMAND LINES
========================

     When typing  in a  command line to MSX-DOS, a simple editing facility is
available  for  correction  of  mistakes  or the  re-entering and  editing of
previous commands.

     Typing  ordinary  characters  at  the keyboard  cause the  characters to
appear on  the screen  as would  be expected.  Typing most control characters
cause  them to be represented by a '^' symbol followed by the control letter.
Exceptions are  carriage return  (RET or  CTRL-M), back space (BS or CTRL-H),
tab  (TAB or  CTRL-I), insert  (INS or  CTRL-R), escape (ESC or CTRL-[), home
(HOME or  CTRL-K), CTRL-C,  CTRL-J, CTRL-N, CTRL-P, CTRL-S, CTRL-U and CTRL-X
(SELECT). These perform the following functions:

          CTRL-C  - this  acts as a 'break' key. A more drastic and preferred
                    'break' key is CTRL-STOP.

          CTRL-J  - line  feed;  nothing  happens  if this  was given  in the
                    command line.

          CTRL-K  - home cursor (HOME).

          CTRL-N  - this  turns  the  printer  off after  being turned  on by
                    CTRL-P.

          CTRL-P  - this  turns  the  printer  on.  When  on, all  characters
                    printed on the screen are also printed on the printer.

          CTRL-S  - this suspends  all character  output until another key is
                    pressed.

          CTRL-U  - this erases the line currently being entered.

          CTRL-X  - this erases the line currently being entered (SELECT).

     The line is entered when the 'enter' key is pressed.

     At any  point whilst typing in a command line, the backspace key (marked
BS  or BACK  SPACE on  most MSX machines) can be used to delete the character
immediately to the left of the cursor in the normal way.

     The cursor left and right keys will move the cursor left and right along
the  line.  Typing a  character at  this point  will overwrite  the character
currently underneath the cursor.

     Pressing the insert key (marked INS on most MSX machines) will toggle to
'insert mode',  and the cursor will change to an underline cursor to indicate
this.  Instead of the characters being typed overwriting the characters under
the cursor,  they will  instead be  inserted before the cursor character, the
remaining  characters to  the end of the line being moved one position to the
right.

     The  delete  key  (marked  DEL  on  most MSX  machines) will  delete the
character under  the cursor  and move  the remaining characters to the end of
the line one position to the left.

     The  home key  (marked 'HOME' on most MSX machines) will move the cursor
to the start of the line.

     Pressing ESC, CTRL-U or CTRL-X will clear the line to allow a new one to
be entered.

     The command editor also keeps a list of previous commands entered, up to
a  limit of  256 characters. Pressing the cursor up key will move up the list
and display the previous command line entered, allowing this old command line
to be edited and re-entered. Pressing the cursor down key will similarly move
to the next old command line that was entered.

     If a  previous command  line is changed, then it will be used as the new
command line and added to the bottom of the list. If it was not changed, then
it  will not  be added  to the  list and the current command line will be the
next  one  which  was  originally entered.  This allows  a whole  sequence of
previous commands to be entered easily.

     The list of previous commands is in fact circular and moving off the top
or  bottom will  move to  the last or first command in the list respectively.
The previous  command can  be called  to be  re-entered or  edited from  this
command history list.

     The features  described here are in fact available to many programs that
MSX-DOS can execute. In any program that does 'line inputs', each line can be
edited as described above. Previous lines can be recalled for re-entering and
editing,  although the list of previous lines will of course include previous
commands.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-notation", "", "MSX-DOS 2: manual", "", "NOTATION (MSX-DOS 2)",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

3. NOTATION
===========

     The  syntax  of the  commands available  from MSX-DOS  are described  in
chapter 4 using the following notation:

Words in upper case

     These are  keywords and must be entered as shown in any mixture of upper
or lower case.

Items in lower case

     These are parameters which must be supplied to the command at this point
in the command line.

Items in square brackets ('[' and ']')

     These are optional items. The brackets themselves should not be included
in the command line.

Items separated by a vertical bar ('|')

     This  indicates that only one of the items is required. The vertical bar
itself should not be included in the command line.

     The following is a list of items which can appear on a command line:

d:

     This indicates that a drive name is required (A:, B: etc.).

     If d:  is shown  as optional  and is  not specified,  then the currently
logged-on drive, as indicated by the command prompt, is assumed.

path

     This indicates that a directory path is required, the syntax of which is
similar to  MS-DOS. Each  directory in  the path  is separated by a backslash
'\'.  A backslash  at the start of the path indicates that the path starts at
the root  directory, otherwise  the path  starts at  the current directory as
indicated  by the  CHDIR command.  Frequently a  filename follows  a path, in
which case the two must be separated by a backslash.

     Two consecutive  dots '..' signify the immediate parent directory in the
path.  A  single  dot '.'  signifies the  current directory  in the  path and
therefore usually has no value in a path specification.

     On non-English MSX machines, the backslash character '\' may be replaced
by  some other  character. In  particular, on  Japanese MSX  machines the Yen
character is used.

     If a  path is  shown as  optional and is not specified, then the current
directory as indicated by the CHDIR command is assumed.

     The  syntax of the directory names that make up a path name follows that
for filenames given below.

filename

     This  indicates that the name of a file is required, the syntax of which
is  similar  to  MS-DOS  and MSX-DOS  1. An  ambiguous filename  is one  that
contains '*'  and '?'  characters and  may match  more that one file on disk,
whilst one that does not contain these is an unambiguous filename.

     A filename has the following syntax:-

          mainname[.suf]

     where mainname is a sequence of up to 8 characters and suf is a sequence
of  up to  3 characters  inclusive. Any  characters beyond  these fields  are
ignored. A  '*' in the main name or suffix is equivalent to filling from that
character  position to  the end of the field with '?'. If the suffix is given
then it  must be separated from the main part of the filename by a single dot
'.'.

     The following characters cannot be used in filenames:-

     Control codes and SPACE (in the range 0 to 20h, and 7FH and FFH)

     : ; . , = + \ < > ( ) | / " [ ]

     All  characters  are  converted  to  upper  case  where appropriate  and
therefore  lower and  upper case  characters have the same meaning. Note that
extended two-character Japanese characters(SHIFT-JIS code) are allowed.

     If a filename is shown as optional and is not specified, then a filename
of *.* is assumed.

filespec

     This is  used to  identify a file or several files in the same directory
on a disk. It's syntax is:-

          [d:][path][filename]

where at  least one  of the three optional items must be given. Where this is
used  to specify  existing files, /H may be given to allow hidden files to be
found.

     Generally, d:  if not  given defaults  to the currently logged on drive,
path  if  not  given  defaults  to the  current directory  of that  drive and
filename if not given defaults to a filename of *.*.

compound-filespec

     This  is used  in many  commands to  specify the files or directories to
which the command is applied. It's syntax is:-

          filespec [+ filespec [+ filespec ...]]

     Thus  several  filespecs  (see  above) can  be given,  separated by  '+'
symbols, with spaces etc. allowed either side of the +. The effect of this in
commands is  exactly the  same as  if all  the matched  files could have been
matched by a single filespec.

     Where a  compound-filespec is  used to specify existing files, /H may be
given after each filespec (see above), in which case it will take effect only
for  the files  matched by  that single filespec. If a /H is given before the
compound-filespec, then it will apply over all the filespecs.

volname

     This  indicates  that a  volume name  is required.  A volume  name is  a
sequence of  up to  11 characters, which can include the characters not valid
for  filenames with  the exception of control codes and '/', although leading
spaces will be deleted.

device

     This indicates  one of  the five  standard MSX-DOS  devices is required.
These and their meaning are:

     CON  - screen/keyboard I/O
     NUL  - 'null' device, does nothing
     AUX  - auxiliary I/O (eg. RS232 serial)
     LST  - printer output
     PRN  - printer output

     Unlike  on some  other systems, a colon is not required after the device
name.

     Device names  can generally  be used wherever filenames can be used. For
example,  the command  COPY MYFILE PRN will read the file MYFILE and write it
to the printer.

     When  using the  CON device  as an input filename, lines can be typed in
and edited in the same way as command lines (see chapter 2 on Editing Command
Lines). To  end the operation, control-Z (^Z) must be typed at the start of a
line.  For example,  a small  text file called MYFILE can be created with the
command COPY CON MYFILE:

        A>COPY CON MYFILE
        All work and no play makes Jack a dull boy.
        Can you hear me?
        ^Z
        A>

Lines of  text can  then be  typed in,  and they  will be written to the file
MYFILE.  The  command  will then  complete when  a line  containing a  single
control-Z is entered.

     If the  NUL device  is written  to by the command COPY CON NUL, then the
characters  written are  simply ignored.  If read  from, then  an end-of-file
condition  is  returned  straight  away (which  is equivalent  to typing  the
control-Z in the example above).

     For  most commands,  it is  not sensible  to specify  a device  (the CON
device cannot  be deleted using the ERASE command, for example). The commands
that  devices are  likely to  be used with are those that read and write data
from and to files, such as CONCAT, COPY and TYPE.

number

     This  indicates that a number is required. This may be in the range 0 to
255 or 0 to 65535 depending on the command.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-commands", "", "MSX-DOS 2: manual", "", "Comandos do MSX-DOS 2: introducao ao capitulo",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

4. COMMANDS
===========

     This chapter  describes in  detail all  the commands  available from the
MSX-DOS  CLI.  Each  command  is  described using  the notation  described in
chapter  3. Where  two or  more parameters are described using this notation,
they must  be separated  by separators.  Separators consist  of zero  or more
leading  spaces, a  separator character,  and zero  or more  trailing spaces.
Valid separator characters are:

          space tab ; , =

     Option letters introduced by '/' characters are an exception to this and
need not be preceded by a separator.

     A transient  MSX-DOS or  CP/M-80 program  can be  loaded and executed by
typing  the main  name of  the filename  plus an  optional extension of .COM.
Batch files  can similarly  be executed  except that  the extension  is .BAT.
Where  COM and  BAT files exist in the same directory and with the same name,
the COM  file is  found and executed in preference to the BAT file. The exact
location  on disk  of the  command can  be specified  by including  its drive
and/or path with its name.

     When looking  for a  COM or  BAT file,  the specified  directory of  the
specified  drive is searched. If not found and a drive or path was given with
the command, then an 'unrecognized command' error results.

     If  just the  filename and  optional extension  were given,  the current
directory is  searched first.  If not  found, then  a list  of directories is
searched.  This list  can be specified and changed using the PATH command. If
still not found then an 'unrecognized command' error again results.

     No CP/M  program will be able to specify directories or path names since
these  do not  exist in  CP/M, only  the current directory of the appropriate
drive being  accessible from  these programs.  An APPEND  environment item is
available  which increases  the usability  of these  programs by  allowing an
alternative directory  to be  searched by  the program as well as the current
one (see chapter 7 on Environment Items).

     Many commands  and programs  perform input or output using the 'standard
input'  and  'standard  output'. The  standard input  normally refers  to the
keyboard, and the standard output normally refers to the screen. These can be
changed, however, to refer to other devices or to disk files for the duration
of  the command  by including  the redirection  symbols <,  > and  >> on  the
command line,  followed by  a device or file name. The standard output of one
command  can  also  be sent  to the  standard input  of the  next command  by
including  the piping  symbol | on the command line between the two commands.
See chapter 5 on Redirection and Piping for more details of these facilities.

     When a transient command is executed, that command may overwrite some of
the  memory that  COMMAND2.COM was  using. Thus  when the command terminates,
COMMAND2.COM  may  need  to  re-load itself  from disk  into memory  from the
COMMAND2.COM file that it was originally loaded from. This file is located by
looking at  the SHELL  environment item (see chapter 7 on Environment Items),
or  the root  directory of  the boot drive if it is not found there. If it is
still not  found, then a prompt is issued. For example, if MSX-DOS was booted
from drive A:, then the prompt will be:

          Insert COMMAND2.COM disk in drive A:
          Press any key to continue...

After  inserting into  drive A:  a disk  containing COMMAND2.COM  in the root
directory and  pressing a  key, COMMAND2.COM will be re-loaded and the system
will continue as normal.

     Although  not a  command as  such, the  currently logged on drive can be
changed by giving the command:

     d:

which  causes the  drive d: to become the current drive. This should be shown
by the prompt letter.

     In  the  command examples  that follow,  underlined text  is an  example
response to  a command,  and the  other text  consists of the example command
given  by the user. In most examples a single space is shown as the parameter
separator,  although  other  separator  characters can  be used  as specified
above.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-assign", "", "MSX-DOS 2: comandos", "ASSIGN [d: [d:]]", "ASSIGN: Sets up the logical to physical translation of drives.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

ASSIGN

Format

          ASSIGN [d: [d:]]

Purpose

          Sets up the logical to physical translation of drives.

Use

     If no drives are given, then all current drive assignments are cancelled.

     If only one drive is given, then the physical drive to which this refers
is printed.

     If both  drives are given, then the subsequent access to the first drive
(logical drive) will be done to the second drive (physical drive) by MSX-DOS.

Examples

ASSIGN

     Un-assigns all previous drive assignments.

ASSIGN A: B:

     Assigns drive A: to drive B:, so that all accesses that previously would
go to drive A: go instead to drive B:

ASSIGN A:
A:=B:

     Displays the drive to which A: is currently assigned, which in this case
is B:.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-atdir", "", "MSX-DOS 2: comandos", "ATDIR +|-H  [/H] [/P] compound-filespec", "ATDIR: Changes the attributes of directories to make them hidden/not hidden.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

ATDIR

Format

     ATDIR +|-H  [/H] [/P] compound-filespec

Purpose

     Changes the attributes of directories to make them hidden/not hidden.

Use

     The  compound-filespec specifies the directories whose attributes are to
be changed.

     If  +H is given, then the selected directories are marked as hidden, and
will not  be affected  by other  directory commands or shown by a DIR command
unless  a /H  option is  given with  those commands.  The -H option marks the
selected directories  as not  hidden, and will not have any effect unless the
/H option is given.

     Unlike files, directories cannot be made read only.

     When  an error  occurs, the erroneous directory name is printed followed
by an  error message,  and the command will continue with the next directory.
If  many errors  occur, then the /P option can be used to cause the output to
pause at the end of the screen.

     The DIR  /H command  can be  used to  indicate the current attributes of
directories.

  Examples

  ATDIR +H DIR1

     Marks the directory called DIR1 as hidden.

     ATDIR -H DIR1/H
     Marks the hidden directory DIR1 as not hidden.

     ATDIR +H DIR?

     Marks all directories matching DIR? as hidden (for example DIR1, DIR2 and DIR3).

ATDIR +H \DIR1+\DIR2

     Marks the DIR1 directory and the DIR2 directory as hidden.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-attrib", "", "MSX-DOS 2: comandos", "ATTRIB +|-R|H [/H] [/P] compound-filespec", "ATTRIB: Changes the attributes of files to make them hidden/not hidden and read",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

ATTRIB

Format

     ATTRIB +|-R|H [/H] [/P] compound-filespec

Purpose

     Changes  the attributes of files to make them hidden/not hidden and read
only/not read only.

Use

     The compound-filespec  specifies the  files whose  attributes are  to be
changed, and /H allows hidden files to also have their attributes changed.

     If  +H is  given, then the selected files are marked as hidden, and will
not be  affected by  most commands or be shown by the DIR command unless a /H
option  is given  with those  commands. -H  marks the  selected files  as not
hidden, and will not have any effect unless the files are hidden.

     If +R  is given,  then the  selected files  are marked  as read only. -R
marks  the selected  files as  not read  only (read/write).  Read only  files
cannot be written to or changed.

     When an  error occurs,  the erroneous filename is printed followed by an
error  message, and  the command  will continue  with the  next file. If many
errors occur,  then the /P option can be used to cause the output to pause at
the end of the screen.

     The DIR command can be used to indicate the current attributes of files.

Examples

     ATTRIB +R FILE1

     The  file FILE1  is marked  as read  only, and  will not subsequently be
modifiable or deletable.

     ATTRIB +HB:\DIR1\*.COM

     Marks all *.COM files in the directory B:DIR1 as hidden, and will not be
displayed by the DIR command.

     ATTRIB -R -H \DIR1/H/P

     All the  files in  DIR1 are  marked as not read only and not hidden. The
output, if any, will pause at the bottom of the screen.

ATTRIB +R \DIR1 + \DIR2 + FILE1

     All files in the directories DIR1 and DIR2 and the file FILE1 are marked
as read only.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-basic", "", "MSX-DOS 2: comandos", "BASIC [program]", "BASIC: Transfers control to MSX disk BASIC.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

BASIC

Format

          BASIC [program]

Purpose

          Transfers control to MSX disk BASIC.

Use

     [Program] is the name of a BASIC program on disk.

     Control is passed to the built-in MSX BASIC, which will load and execute
the  BASIC program  if specified.  If a RAM disk has been set up, then it can
still be used from BASIC.

     The  BASIC  command  CALL  SYSTEM("command")  can be  used to  return to
MSX-DOS,  and the  optional command,  which can  be any command executable on
MSX-DOS, will  be executed.  If the  command is  not given, then a batch file
called  REBOOT.BAT will  be searched for and executed if found (see chapter 6
on Batch Files).

Examples

BASIC

     MSX disk BASIC is entered.

BASIC MYPROG.BAS

     MSX disk  BASIC is  entered, and the BASIC program MYPROG.BAS loaded and
RUN.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-buffers", "", "MSX-DOS 2: comandos", "BUFFERS [number]", "BUFFERS: Displays or changes the number of disk buffers in the system.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

BUFFERS

Format

     BUFFERS [number]

Purpose

     Displays or changes the number of disk buffers in the system.

Use

     If the number is not given, then the number of disk buffers currently in
the system will be displayed, otherwise the number of buffers will be changed
to the specified number, the now unused memory being freed for other purposes
if the  new number  is less  than the previous. If there is not enough memory
for the specified number of buffers, then as many as possible are created and
no error is given.

     Increasing  the number  of disk  buffers may speed up some applications,
particularly those  that perform random accesses to files. Setting the number
above  10 is  unlikely to improve performance much, and unnecessarily uses up
memory.

     The memory area used for disk buffers is also used for environment items
and for  opening files.  Thus keeping buffers set to the maximum possible may
prevent some commands from working, particularly SET, COPY and CONCAT. If any
of these commands give a 'not enough memory' error then it may help to reduce
the  number of  buffers. Reducing  them below about three however will impair
performance considerably.

     The default number of buffers in the system is 5, which will be adequate
for most purposes.

Examples

BUFFERS
     BUFFERS=5

     The current number of disk buffers is printed, which in this case is 5.

BUFFERS 10
(or BUFFERS=10)

     The number  of buffers is increased to as many as possible up to a limit
of 10.

BUFFERS = 5
(or BUFFERS 5)

     The number of buffers is reduced again to 5.

CD

     See CHDIR

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-chdir", "", "MSX-DOS 2: comandos", "CHDIR [d:] [path]", "CHDIR: Displays or changes the current directory.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

CHDIR

Format

     CHDIR [d:] [path]
          or
     CD [d:] [path]

Purpose

     Displays or changes the current directory.

Use

     If no path is specified, then the current directory path for the current
or  specified drive  is printed.  This is  the directory  path from  the root
directory to the current directory.

     If a  path is  specified, then  the current directory for the current or
specified drive is changed to the directory specified by the path.

     Each drive has it's own current directory. This remains at the directory
specified  by the last CHDIR command for that drive (or at the root directory
initially) until  another CHDIR command is given or it cannot be found on the
disk when it is accessed (because the disk has been changed, for example). It
is then returned to the root directory.

     The  CD command is an abbreviated form of the CHDIR command provided for
convenience and MS-DOS compatibility.

     Note  that  the command  prompt can  be changed  to display  the current
directory  with  the  command  SET PROMPT  ON (see  chapter 7  on Environment
Items).

Examples

CHDIR \DIR1

     The current directory of the current drive is changed to DIR1.

CHDIR A:DIR2

     The current directory of drive A: is changed to DIR2.

CD
     E:\DIR1

     The current  directory of  the current drive is displayed, which in this
case is DIR1.

CHDIR A:
     A:\DIR2

     The current directory for drive A: is displayed, which is also DIR2.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-chkdsk", "", "MSX-DOS 2: comandos", "CHKDSK [d:] [/F]", "CHKDSK: Checks the integrity of the files on the disk.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

CHKDSK

Format

          CHKDSK [d:] [/F]

Purpose

          Checks the integrity of the files on the disk.

Use

     The integrity of the data structures on disk in the specified or current
drive is checked and lost disk space is checked for. When errors are found on
the disk, corrective action is taken. If lost clusters are found, a prompt is
issued allowing  the lost  disk space to be either converted into usable disk
space  or  into  files.  If  the latter  is chosen,  then files  of the  form
FILE0000.CHK, FILE0001.CHK etc. will be created.

     If  the /F  option is not given, then CHKDSK will not actually write any
corrections it  makes to  disk, but will behave as though it has. This allows
CHKDSK to be executed to see what would be done to the disk if /F was given.

     Disk  space  can  become  lost  (ie.  lost  clusters created)  when some
programs  are  aborted.  This applies  particularly to  CP/M programs.  It is
recommended that CHKDSK is used occasionally on all disks.

     Note  that this is a transient command, and must therefore be loaded off
disk.

Examples

CHKDSK

     The disk  in the  current drive  is checked.  A 'status  report' will be
printed. Any errors found will not be written to disk.

     CHKDSK B:

     The disk in drive B: is checked. Any errors found will not be written to
disk.

CHKDSK/F
    20 lost clusters found in 1 chain
    Convert lost chains to files (Y/N) ?

     The  disk in  the current  drive was  checked, and  some lost disk space
found. Since  /F was  given, the  corrections will be written to disk and the
lost space recovered.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-cls", "", "MSX-DOS 2: comandos", "CLS", "CLS: Clears the screen.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

CLS

Format

          CLS

Purpose

          Clears the screen.

Use

          Simply clears the screen and homes the cursor.

Examples

CLS

     The screen is cleared, and another command can be typed.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-command2", "", "MSX-DOS 2: comandos", "COMMAND2 [command]", "COMMAND2: Invokes the command interpreter.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

COMMAND2

Format

     COMMAND2 [command]

Purpose

     Invokes the command interpreter.

Use

     command is any command that can normally be typed at the prompt (such as
the commands in this manual).

     COMMAND2 is  simply the name of the command interpreter on disk, and can
be  executed as  a normal transient program. In normal use it gets loaded and
executed once  by MSXDOS2.SYS at the boot time, and this provides the ability
to perform all the commands in this manual.

     Advanced users  may, however, wish to invoke another command interpreter
for  a variety  of reasons.  The second  COMMAND2.COM may,  for example, be a
later version  and provide  more facilities.  If a  transient program has the
ability to load and execute programs, as some sophisticated programs do, then
they  can load  the COMMAND2.COM  program and any MSX-DOS command can then be
given. When  COMMAND2.COM exits by EXIT command, the original program will be
returned to.

     If no command is given as a parameter, then the second COMMAND2.COM will
simply issue the normal prompt (without executing AUTOEXEC.BAT or REBOOT.BAT)
and  wait for  commands in the normal way. It will terminate and exit back to
the original  command interpreter  or program  when the EXIT command is given
(see  the EXIT command). If an error code is given to this EXIT command, then
the original  command interpreter or program will receive it and, in the case
of  COMMAND2.COM and  MSXDOS2.SYS, print  an appropriate  error message  (see
chapter 8 on Errors).

     If a  command is  given as  a parameter to COMMAND2.COM however, then it
will  be executed  as though it had been typed in the normal way. The command
may be an internal command or an external COM or BAT command. After executing
the command,  COMMAND2.COM will immediately exit back to the original command
interpreter or program.

     In  this way,  invoking a  second COMMAND2.COM  from the  normal command
interpreter with  a batch  file name as a command can be used to 'nest' batch
files (see chapter 6 on Batch Files), instead of 'chaining'.

     When COMMAND2.COM  is executed, it saves the whole environment, and then
restores  it again  when it  exits. It  only sets  up the default environment
items however  if they  are not already defined. Thus the second COMMAND2.COM
inherits  the environment  of the  first. Any  changes made whilst the second
COMMAND2.COM is  executing will  only last  as long it does, and will be lost
when it exits.

     Each  incarnation of  COMMAND2.COM uses  up some  memory which  is freed
again when  it exits. This depends partly on the number of environment items,
and is typically about 1.5K.

     When COMMAND2.COM  executes a  transient program, it may have to re-load
itself  off disk  since the  program is allowed to use the memory occupied by
COMMAND2.COM. In  this case, it uses the SHELL environment item to locate the
file  that it  must use  to load itself (see chapter 7 on Environment Items).
When first  loaded from  the COMMAND2.COM file on disk, SHELL is set to refer
to that file.

Examples

COMMAND2
A>

     Another  copy of COMMAND2 is loaded, and prints it's normal prompt. EXIT
will exit back to the original prompt.

COMMAND2 FILE.BAT

     Normally in  a batch  file. The  file FILE.BAT  is executed, and when it
ends the current batch file will be resumed with the command after this one.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-concat", "", "MSX-DOS 2: comandos", "CONCAT [/H] [/P] [/B] [/V] compound-filespec filespec", "CONCAT: Concatenates (joins together) files.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

CONCAT

Format

     CONCAT [/H] [/P] [/B] [/V] compound-filespec filespec

Purpose

     Concatenates (joins together) files.

Use

     The  compound-filespec  specifies  the  files  that  are  to  be  joined
together, and /H allows hidden files to be joined.

     The  second parameter  is a  filespec that  must be  unambiguous and  is
created before  the source  files are  read. Each  source file  is then read,
joined onto the end of the previous one and written out to the destination.

     As each  source file  is read,  it's filename  is printed.  If for  some
reason  the file  cannot be read (eg. it is the file that has been created as
the destination)  then the  filename is  followed by an error message and the
CONCAT operation continues with the next source file. If many files are being
concatenated, then /P will cause the output to pause at the end of the screen
until a key is pressed.

     Normally, the  concatenation is  performed on  ASCII files. Source files
are  read  up  to  the  first  end-of-file  character (CTRL-Z)  and a  single
end-of-file  character is appended to the destination after all data has been
written out.  If, however,  /B (binary mode) is given, then no interpretation
is given to the data read and no additional data is added.

     It  is also  possible to give the /B to the destination or to any of the
filespecs in  the compound-filespec,  and it  will then  refer only  to those
files. /A may also be given to reverse the effect of /B.

     The  /V  option  can  be given  to turn  write verification  on for  the
duration of  the CONCAT  command (see  the VERIFY  command). This will ensure
that  data is  written correctly to disks if the device driver being used has
the feature, but will slow the operation down for the verification.

     If CONCAT  gives a  'Not enough memory' error then probably reducing the
number  of buffers  (see the  BUFFERS command)  or removing  some environment
items (see chapter 7 on Environment Items) will free up sufficient memory.

Examples

CONCAT *.DOC ALL.PRN

     A  new file called ALL.PRN is created, and all files matching *.DOC (for
example FILE1.DOC,  FILE2.DOC and  FILE3.DOC) are joined together and written
to  the new  file in the order that they are found on disk. Any existing file
called ALL.PRN will be overwritten.

CONCAT /H /P *.DOC ALL.DOC
     FILE1.DOC
     FILE2.DOC
     FILE3.DOC
     ALL.DOC -- Destination file cannot be concatenated

     A  new file  called ALL.DOC is created, and all files matching *.DOC are
joined together  and written to the new file in the order that they are found
on  disk. Since the destination file ALL.DOC also matched the source filename
*.DOC, the  message is  printed and  it is not included in the concatenation.
Since  /H was  given, hidden  files are  also concatenated  and, since /P was
given, the  key input  is waited after each screenful output if the number of
the lines of the file list is larger than that of the screen.

CONCAT /B  FILE2.DOC + FILE3.DOC + FILE1.DOC  ALL.DOC

     A  new file  ALL.DOC is  created, and the files FILE2.DOC, FILE3.DOC and
FILE1.DOC and joined together in that order and written to the new file. They
are joined together in binary mode.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-copy", "", "MSX-DOS 2: comandos", "COPY [/A] [/H] [/T] [/V] [/P] [/B] source dest", "COPY: Copies data from files or devices to other files or devices.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

COPY

Format

     COPY [/A] [/H] [/T] [/V] [/P] [/B] source dest

Purpose

     Copies data from files or devices to other files or devices.

Use

     The definition of the source is:

     compound-filespec | device

The compound-filespec  specifies the  files that  are to be copied. It may be
the device specification. If /H is given then hidden files may be copied.

     The definition of the dest is:

     [d:] [path] [filename] | device

Where d: and path default to the current drive and directory respectively. If
any part of the filename is ambiguous then the appropriate character from the
source  filename is substituted, thus allowing the files to be renamed in the
process. If  the filename  is not  given, then  the entire source filename is
used. If the dest is an unambiguous directory, then the files are copied into
that directory with a filename of *.*.

     COPY  will  read as  many source  files as  possible into  memory before
writing any  out. When  it can read no more into memory (eg. when it has used
all  available memory)  it will write out each file in the order that it read
them. When  it creates  each destination file, it prints the source filename.
Then  if it  is unable  to create  the destination  file, an error message is
printed and  the copy operation continues with the next file. /P can be given
to make the output pause at the end of the screen.

     Many reasons exist for COPY to be unable to create the destination, such
as  a read-only file already existing with the same name. Sometimes COPY will
refuse to  create the  destination because  the user may have made a mistake.
For  example, a file cannot be copied onto itself, or several files cannot be
copied onto one file. A 'Cannot create destination' error may be given if the
destination of one file would delete a previous source file or a file already
being used  for something  else (eg.  the currently  executing batch file). A
'Cannot  overwrite previous  destination file' error results if an attempt is
made to  copy many  files to  one file.  This usually means that the intended
destination was a directory, but that the name has been misspelled.

     If  /A is  specified, then  an ASCII  copy is performed. This means that
source  files  will  only  be  read  as far  as the  first end-of-file  (EOF)
character (CTRL-Z)  and then  each destination will have a single end-of-file
character appended to it.

     It  is also  possible to  give a  /A to the destination or to any of the
filespecs in  the compound-filespec separately, in which case it applies only
to that source or dest specification.

     The  /B option  can be  given to  copy in binary mode, that is, the file
being read will be copied as it is and no data will be added.

     The  /V  option  can  be  given to  turn write  verification on  for the
duration  of the COPY command (see the VERIFY command). This will ensure that
data is  written correctly  to disks  if the device driver being used has the
feature, but will slow the operation down.

     Normally,  the destination files are given the same date and time as the
source files.  However, the  /T option  can be given to cause the destination
files  to have  the current  date and time. The destination files will not be
hidden or  read-only, regardless  of the  attributes of the source files. The
ATTRIB command can be used to change these.

     If  COPY gives  a 'Not  enough memory'  error then probably reducing the
number of  buffers (see  the BUFFERS  command) or  removing some  environment
items (see chapter 7 on Environment Items) will free up sufficient work area.

     Note  that the COPY command is simpler than that in MS-DOS and MSX-DOS 1
because it  cannot concatenate  (join together)  files. To  do this, a CONCAT
command is available (see the CONCAT command).

Examples

COPY FILE1 B:

     The file FILE1 is copied from the current directory of the current drive
to the current directory of drive B:.

COPY /H  MSXDOS2.SYS + COMMAND2.COM  B:

     The  two hidden  files MSXDOS2.SYS  and COMMAND2.COM are copied to drive
B:, thus making it a booting disk.

COPY A:\DIR1 B:\DIR1 /V

     All files  in the directory DIR1 from the root of drive A: are copied to
a  similar directory on drive B: with verify on to ensure that the files were
written correctly.

COPY B:

     All files in the current directory of drive B: are copied to the current
directory of the current drive.

COPY /A AUX CON

     Characters  are read  from the  device AUX  (which may be used for RS232
serial for  example) to the device CON, which is the screen. The copy is done
as  far as  the first  end-of-file character. If /A was not given, then there
may have  been no  way of  stopping the  COPY operation  without pressing the
CTRL-STOP key.

COPY A:*.DOC B:/T

     All   files  matching   *.DOC  (for  example  FILE1.DOC,  FILE2.DOC  and
FILE3.DOC) are  copied to the current directory of drive B: and are given the
current date and time instead of the dates and times of the *.DOC files.

COPY *.BAT
 AUTOEXEC.BAT -- File cannot be copied onto itself
 REBOOT.BAT -- File cannot be copied onto itself

     This  command told  COPY to  copy all files matching *.BAT (in this case
AUTOEXEC.BAT and  REBOOT.BAT) from the current directory of the current drive
to  the same place, and COPY printed the messages to warn of this. No data in
this case was actually copied.

COPY *.BAT DIR2
AUTOEXEC.BAT
REBOOT.BAT -- Cannot overwrite previous destination file

This  command  told  COPY  to copy  all files  matching *.BAT  (in this  case
AUTOEXEC.BAT and  REBOOT.BAT) to  a directory called DIR2. DIR2, however, did
not  exist so  AUTOEXEC.BAT was  copied to  a file  called DIR2,  and then an
attempt was made to copy REBOOT.BAT also to a file called DIR2. The message was printed as a warning that a
mistake was  probably made  (in this  case DIR2 not existing). REBOOT.BAT was
not actually copied anywhere.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-date", "", "MSX-DOS 2: comandos", "DATE [date]", "DATE: Displays or sets the current date.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

DATE

Format

     DATE [date]

Purpose

     Displays or sets the current date.

Use

     If the  date is  given after  the command,  then the date is set to this
value (for the format see below). If the date is not given after the command,
then the current day and date is printed and the new date is prompted for and
input.  If no  input is  given (ie. if the 'enter' key alone is pressed) then
the current  date is  not altered. Otherwise the input is assumed to be a new
date,  and is  interpreted as described below. If the date is invalid then an
error message is displayed and the new date again prompted for and input.

     The date is expected to consist of up to three numbers, separated by one
of the following characters:

     space tab , - . / :

with  spaces allowed  either side  of the character. Any missing numbers will
default to  the current  setting. The  year may  either be a full century and
year, or may be just the year in which case the century defaults to 19 if the
year is greater than 80 or 20 otherwise. The date and the year specifications
may be substituted by '-' to be omitted.

     The format in which the date is printed and input is flexible and can be
changed. An  environment item  called DATE  is set  up by default to a format
that is appropriate for the country of origin of the MSX machine (see chapter
7  on  Environment  Items).  For  example, on  Japanese machines  the default
setting  is YY-MM-DD.  The command  SET DATE  DD-MM-YY will  change the  date
format to  the European  format. The format also affects the dates printed by
the DIR command.

     If  the DATE environment item is defined, then it will be printed by the
DATE command  to indicate  the format  in which  the date  is required  to be
input.

Examples

DATE 86-6-18

     The current date is set to the 18th June 1986.

DATE
     Current date is Wed 1986-06-18
     Enter new date (yy-mm-dd): - -19

     No  parameter  was  given, so  the current  date of  18th June  1986 was
printed and a new date prompted for. In the reply to the prompt, the date was
updated to the next day by only specifying the 19th. Since the year and month
were not given, they remained the same.

SET DATE = DD/MM/YY

     The date format has been changed to the European format.

DATE
Current date is Thu 19-06-1986
Enter new date (DD/MM/YY):

     No  parameter  was  given, so  the current  date of  19th June  1986 was
printed in the European format, and the prompt printed. The reply is expected
in the European format.

     Formats are:
        ISO             YY/MM/DD
        American        MM/DD/YY
        European        DD/MM/YY

DEL

     See ERASE.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-dir", "", "MSX-DOS 2: comandos", "DIR [/H] [/W] [/P] [compound-filespec]", "DIR: Displays the names of files on disk.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

DIR

Format

     DIR [/H] [/W] [/P] [compound-filespec]

Purpose

     Displays the names of files on disk.

Use

     The  compound-filespec specifies which files are to be listed. If the /H
option is given, then hidden files will also be listed.

     In the  DIR command, unlike all other commands, it is permissible to not
give  the main  filename or  the filename extension, and both will default to
'*'. Thus  a filename  of 'FRED'  is equivalent to 'FRED.*' and a filename of
'.COM'  is equivalent  to '*.COM'.  Note that if the '.' at the end of a main
filename is  given, then the extension is also assumed to have been given, so
that  the filename  'FRED.' is not equivalent to 'FRED.*', unlike the example
above.

     There are  two formats of the listing. If the /W option is given, then a
'wide'   listing  is   printed,  with  several  filenames  output  per  line.
Sub-directory names,  file attributes,  and the  date and  time each file was
created are not displayed.

     If the  /W option  is not given, then the filenames are printed with one
filename  per line,  together with the attributes, the file size and the date
and time  at which  the file was last modified. The attributes are printed as
an  'r' if  the file is read-only and an 'h' if the file is hidden (and /H is
given).  If  the  time  of a  file is  zero (ie.  the file  does not  have an
associated time)  then the  time field  will not be printed. If the date of a
file  is zero, then neither the date nor the time fields will be printed. The
formats in which the dates and times are printed can be changed (see the DATE
and TIME commands).

     The non-/W  display is designed to fit within a 40 column screen, but if
fewer columns are available then some fields of the listing will not be shown
so that the display will always fit on one line. The number of files per line
that  are printed  when /W  is specified  is also  adjusted according  to the
screen width. If the width of the display is less than 13 characters however,
then in both cases the filenames will wrap to the next line.

     At  the top  of the  list of  files, the volume name of the disk and the
name of the directory being listed is displayed. At the bottom, the number of
files  listed,  the total  number of  bytes in  the files  and the  amount of
remaining disk space is printed.

     When  the directory  of a  sub-directory is printed, the first two items
listed will  always be two special sub-directories called '.' and '..'. These
are  automatically created  when a  new directory is created, and it is these
that allow  '.' and '..' to be given in path names to signify the current and
parent  directories respectively (see chapter 3 on Notation for a description
of paths).

     When printing  a number of bytes, the number is truncated and printed as
the number of kilobytes if 1K or greater.

     If  the /P  option is given, then the output will pause at the bottom of
the screen until a key is pressed.

Examples

DIR

     All  filenames  and  directory  names in  the current  directory of  the
current drive will be printed. This might be as follows:

      Volume in drive A: is MSX-DOS 2
      Directory of A:\

     MSXDOS2  SYS r    4096 86-06-19  2:45p
     COMMAND2 COM r   10496 86-06-19  2:46p
     UTILS           <dir>  86-06-19  2:50p
     HELP            <dir>  86-06-19  2:50p
      14K in 2 files   222K free

     The disk  thus contains  the two  MSX-DOS system  files MSXDOS2.SYS  and
COMMAND2.COM, which are read only, and two directories called UTILS and HELP.

DIR B:\HELP/W

     A  'wide' directory  format has  been requested of the HELP directory on
drive B:. This might be as follows:

      Volume in drive B: is MSX-DOS 2
      Directory of B:\HELP

     BUFFERS .HLP    ATTRIB  .HLP    ASSIGN  .HLP
     ATDIR   .HLP    CHDIR   .HLP    CD      .HLP
     SYNTAX  .HLP    ENV     .HLP    BATCH   .HLP
     EDITING .HLP
      25K in 10 files   222K free

     DIR UTILS + HELP/P

     This will list all the files in the UTILS directory and all the files in
the HELP directory, and will pause at the end of every screen full.

DIR .COM

     No main  filename was  given, and so defaults to *. Thus this command is
equivalent to the command DIR *.COM.

DIR COMMAND2

     No  extension was  given, so  this defaults  to .*. Thus this command is
equivalent to the command DIR COMMAND2.*.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-diskcopy", "", "MSX-DOS 2: comandos", "DISKCOPY [d: [d:]] [/X]", "DISKCOPY: Copies one disk to another.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

DISKCOPY

Format

     DISKCOPY [d: [d:]] [/X]

Purpose

     Copies one disk to another.

Use

     The first  drive is  the source  drive and  the second  the destination,
which  defaults to  the current  drive. If no drives are given, then DISKCOPY
will prompt for both the source and the destination.

     Before DISKCOPY is used, the destination disk must be formatted with the
same format as the source disk, and an error will be given if this is not the
case.

     If  /X  is given,  then various  messages printed  during the  disk copy
operation will be suppressed.

     Note that this is a transient command, and must therefore be loaded from
disk.

Examples

     DISKCOPY A: B:

Insert source disk in drive A:

Insert target disk in drive B:

Press any key to continue...

     The command  was given to copy the disk in drive A: to the disk in drive
B:,  thus destroying  all existing data on the disk in drive B: The prompt is
printed first.

DISKCOPY B:

     The disk in drive B: is copied to the disk in the current drive.

DISKCOPY
Enter source drive:
Enter target drive:

     The DISKCOPY  command was  given with  no parameters,  so the source and
destination  disks were  prompted for.  The reply  to the prompts consists of
just a single drive letter.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-echo", "", "MSX-DOS 2: comandos", "ECHO [text]", "ECHO: Prints text.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

ECHO

Format

     ECHO [text]

Purpose

     Prints text.

Use

     The text  is simply  displayed on  the screen. If no text is given, then
just a blank line is output.

     This  command should  not be  confused with  the 'echo'  state in  batch
files, which  is controlled by an environment item called ECHO (see chapter 7
on Environment Items).

Examples

ECHO AUTOEXEC batch file executed
AUTOEXEC batch file executed

     The  specified text  ('AUTOEXEC batch file executed') was printed on the
screen.

ECHO

     No parameters were given, so just a blank line was printed.

ERA

     See ERASE.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-erase", "", "MSX-DOS 2: comandos", "ERASE [/H] [/P] compound-filespec", "ERASE: Deletes one or more files.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

ERASE

Format

     ERASE [/H] [/P] compound-filespec
              or
     DEL [/H] [/P] compound-filespec
              or
     ERA [/H] [/P] compound-filespec

Purpose

     Deletes one or more files.

Use

     The compound-filespec  specifies which  files are  to be deleted. The /H
option allows hidden files to also be deleted.

     During the delete operation, if a file cannot be deleted for some reason
(eg. it  is set  to 'read only') then the offending filename is printed along
with an error message, and the delete operation continues with the next file.
If  many such errors occur, then the /P option will cause the output to pause
at the end of the screen.

     If the filename is *.*, then the prompt:

          Erase all files (Y/N) ?

is printed,  and a  reply is  waited for. If the reply is anything other than
'Y'  or 'y',  then the  file deletion  does not  take place. This is a safety
feature designed to prevent accidental loss of all files in a directory.

     If files  are deleted unintentionally on a disk that was formatted under
MSX-DOS  2, then  the UNDEL  command may  be used  immediately afterwards  to
restore them again.

Examples

ERASE FILE1.BAK

     The file  FILE1.BAK is deleted from the current directory of the current
drive.

DEL *.COM/H

     All files matching *.COM, both hidden and not hidden, are deleted.

DEL B:\UTIL\*.COM + B:\UTIL\*.BAT

     All  files matching *.COM or *.BAT are deleted from the directory called
UTIL on drive B:.

DEL B:\UTIL
     Erase all files (Y/N) ?

     All files in the directory called UTIL on drive B: are deleted. Since so
many  files  are  being  deleted,  a  prompt is  printed first  to prevent  a
catastrophe.

DEL *.BAT
AUTOEXEC.BAT -- Read only file
REBOOT.BAT -- Read only file

     All  files  matching  *.BAT  are  deleted  except  for  AUTOEXEC.BAT and
REBOOT.BAT which have been marked as read only.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-exit", "", "MSX-DOS 2: comandos", "EXIT [number]", "EXIT: Exits COMMAND2.COM to the invoking program.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

EXIT

Format

          EXIT [number]

Purpose

          Exits COMMAND2.COM to the invoking program.

Use

     The  number  is  an  error  code  and defaults  to 0,  which in  MSX-DOS
indicates no error (see chapter 8 on Errors).

     EXIT exits  the command interpreter (COMMAND2.COM) and returns the error
code  to the program that originally loaded and executed it (see the COMMAND2
command). This  may be  another COMMAND2.COM,  another program  or, normally,
MSXDOS2.SYS. In the latter case, an appropriate error message will be printed
and COMMAND2.COM simply reloaded and executed.

     COMMAND2.COM when loaded saves the current environment (see chapter 7 on
Environment  Items),  and EXIT  restores it.  Thus, when  EXIT exits  back to
MSXDOS2.SYS (that is, EXIT is executed at the primary level), the environment
will be  cleared. COMMAND2.COM  will then  be reloaded  and will  set up  the
default environment again, providing a method of resetting the environment to
it's default values.

Examples

EXIT

     The  command interpreter  is exited.  What happens  next depends on what
loaded it.

EXIT 40
* User error 40

The command  interpreter is  exited with an error code of 40. Since this does
not  correspond to an error that is known to the system, the error message is
printed by  whatever loaded  the command  interpreter in the first place (see
chapter 8 on Errors).

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fixdisk", "", "MSX-DOS 2: comandos", "FIXDISK [d:] [/S]", "FIXDISK: Updates a disk to the full MSX-DOS 2 format.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

FIXDISK

Format

     FIXDISK [d:] [/S]

Purpose

     Updates a disk to the full MSX-DOS 2 format.

Use

     d:  specifies the  drive on  which FIXDISK  is to  operate. If d: is not
specified, the current drive will be assumed. /S option causes the disk to be
updated to full MSX-DOS 2 disk.

     This command  is mainly used to update MSX-DOS 1 disks to full MSX-DOS 2
compatibility,  but may  also be useful for updating other disks of a similar
format or reparing incorrect boot sector.

     Although   the  disk   format  used  by  MSX-DOS  1  and  MSX-DOS  2  is
standardized, MSX-DOS  1 does not use the information stored on certain parts
of  the disk  (the boot  sector) and  so this  information is not necessarily
correct on  MSX-DOS 1  disks. This  can cause problems when MSX-DOS 2 is used
with  these disks.  Additionally, the  MSX-DOS 2 UNDEL command will only work
with disks that were formatted using MSX-DOS 2 (ie. disks that have a 'volume
id' in  the boot  sector) and  so will not work with MSX-DOS 1 disks or disks
formatted on other systems.

     The FIXDISK  command will  update a  disk so  that it is fully MSX-DOS 2
compatible,  and it's  use will allow full use of MSX-DOS 2 disk features. If
/S option  is specified,  the boot  program will  be updated for MSX-DOS 2 so
that  the features  of MSX-DOS  2 disk  can fully  work. When a disk has been
updated in  this way,  however, it may no longer be fully compatible with the
original  system. For  example, if /S option is specified against the disk of
the application which uses non-standard boot program, such as some games, the
application will no longer be booted, although it will still be able to start
MSX-DOS 1 or MSX-DOS 2.

     To help  prevent accidental  updates of boot disks from other systems, a
prompt is issued before updating a disk.

Examples

FIXDISK B: /S
Disk in drive B: will only be able to boot MSX-DOS 2
Press any key to continue...

     Drive  B: will  be updated  to be  fully MSX-DOS 2 compatible. Since the
disk may have been a boot disk from another system, a prompt is issued before
the disk is actually updated.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-format", "", "MSX-DOS 2: comandos", "FORMAT [d:]", "FORMAT: Formats (initializes) a disk.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

FORMAT

Format

     FORMAT [d:]

Purpose

     Formats (initializes) a disk.

Use

     The specified  or current  drive is  formatted, and all data on the disk
will be destroyed.

     After giving  a FORMAT  command, an option may be prompted for, allowing
the  required format  of the  disk (such  as 1DD  or 2DD) to be selected. The
exact nature of these prompts depends on the manufacturer of the MSX machine,
so obey the descriptions of the manual of the machine when you want to format
the disk.

     After formatting, there will be no files or directories on the disk, and
the  maximum amount  of disk  space will  be free.  The disk  will not have a
volume name, but can be given one with the VOL command. To turn the disk into
a boot  disk so that MSX-DOS can be started up from it, the files MSXDOS2.SYS
and COMMAND2.COM must be copied onto it with the COPY command.

Examples

FORMAT B:
1 - Single sided
2 - Double sided
? 2
All data on drive B: will be destroyed
Press any key to continue...

     The  command was given to format the disk in drive B:. In this case, the
options available  were to  select either  double sided  or single sided, and
double sided was selected. The standard warning prompt was then printed.

FORMAT

     This will format the current drive after the prompts given above.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-help", "", "MSX-DOS 2: comandos", "HELP [subject]", "HELP: Provides on-line help for an MSX-DOS feature.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

HELP

Format

     HELP [subject]

Purpose

     Provides on-line help for an MSX-DOS feature.

Use

     If no parameter is given, then a list of standard subjects on which help
is  available is printed. This includes all the commands and the major system
features.

     If a  subject is specified, then help text on this subject is printed on
the  screen from  a 'help  file'. This  will pause at the end of every screen
until a key is pressed.

     Help files have a filename of:

          subject.HLP

and are  located by  default on the standard MSX-DOS boot disk in a directory
called HELP.

     An environment item called HELP is set up initially to refer to the HELP
directory  (see chapter 7 on Environment Items). This can be changed with the
SET command to refer to any other directory or disk if required.

     Any HELP  subject can  be added  by the  user for  his own use simply by
adding the appropriate .HLP file in the HELP directory. The help file will be
displayed very much like the TYPE command would display it.

Examples

HELP

     A  general help  screen is printed. This lists all the subjects on which
help  is  available, including  the standard  commands and  main features  of
MSX-DOS. The user-supplied subjects are not available here.

HELP XCOPY

     Help  information  on  the  XCOPY command  is printed.  This includes  a
description of how to use the command and what options are available.

HELP ME
* File for HELP not found

     This command  caused HELP to look for a file called ME.HLP from which to
get  the help  text, but  did not  find it  so printed the error message. The
files containing the help text are normally found in a directory called \HELP
on the  drive from  which MSX-DOS was booted, and any other help files may be
added  if required.  If ME.HLP  was added  then HELP ME would print it on the
screen.

MD

     See MKDIR

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-mkdir", "", "MSX-DOS 2: comandos", "MKDIR [d:] path", "MKDIR: Creates a new sub-directory.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

MKDIR

Format

     MKDIR [d:] path
           or
     MD [d:] path

Purpose

     Creates a new sub-directory.

Use

     The  last item in the path is the name of the new sub-directory which is
to be  created on  the current  or specified  drive. Thus if this is the only
item  in the  path, the new directory is created in the current directory. If
the new  directory is  to be  hidden, then  it must be separately made hidden
with the ATDIR command.

     When a  new directory  is created,  it is  empty except  for two special
sub-directories  called '.'  and '..'. These are automatically created in the
directory and  it is  these that allow '.' and '..' to be given in path names
to  signify the current and parent directories respectively (see chapter 3 on
Notation for a description of paths).

     The MD  command is an abbreviated form of the MKDIR command provided for
convenience and MS-DOS compatibility.

Examples

MKDIR UTIL

     A  directory called  UTIL is  created in  the current  directory of  the
current drive.

MKDIR A:\UTIL\RAM

     A directory  called RAM  is created  in the  UTIL directory  in the root
directory of drive A:.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-mode", "", "MSX-DOS 2: comandos", "MODE number", "MODE: Changes the number of characters/line on the screen.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

MODE

Format

     MODE number

Purpose

     Changes the number of characters/line on the screen.

Use

     The  number must  be in  the range  1 to 80 inclusive, and the number of
characters per  line on  the screen  will be  set to this. The screen will be
cleared and the cursor moved to the top left corner in the process.

Examples

MODE 80
(or MODE=80)

     The screen is set to 80 column mode and is cleared in the process.

MODE 25
(or MODE=25)
     The screen is set to 25 columns.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-move", "", "MSX-DOS 2: comandos", "MOVE [/H] [/P] compound-filespec [path]", "MOVE: Moves files from one place to another on a disk.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

MOVE

Format

     MOVE [/H] [/P] compound-filespec [path]

Purpose

     Moves files from one place to another on a disk.

Use

     The  compound-filespec specifies  which files  are to  be moved,  and /H
allows hidden files to be included in the move.

     The path specifies the directory to which the files are to be moved, the
current directory  being used  if this  is not  given. The path must exist on
each drive referenced by the filespecs in the compound-filespec.

     If  a  particular file  cannot be  moved into  the specified  or current
directory (eg.  if a file of the same name already exists) then the offending
filename  is printed  along with  an error  message, and  the move  operation
continues with  the next  file. If many errors occur, then the /P option will
cause the output to pause at the bottom of the screen.

Examples

MOVE FILE1 \

     The  file FILE1 is moved from the current directory of the current drive
to the root directory of the current drive.

MOVE /H /P E:*.COM \
COMMAND2.COM -- File exists

     All  files matching  *.COM, both  hidden and  not hidden, in the current
directory of drive E: are moved to the root directory of that drive. The file
COMMAND2.COM already existed in the root directory, so the error was printed.
Neither of  the COMMAND2.COM files were moved or altered. If many such errors
had occurred then a prompt would have been printed after a screen full.

MOVE \UTIL\*.COM + \UTIL\*.BAT

     All  files matching  *.COM or  *.BAT in  a directory  called UTIL on the
current drive are moved to the current directory of that drive.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-mvdir", "", "MSX-DOS 2: comandos", "MVDIR [/H] [/P] compound-filespec [path]", "MVDIR: Moves directories from one place to another on a disk.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

MVDIR

Format

     MVDIR [/H] [/P] compound-filespec [path]

Purpose

     Moves directories from one place to another on a disk.

Use

     The compound-filespec  specifies which  directories are to be moved, and
/H allows hidden directories to be included in the move.

     The  second parameter specifies the directory into which the directories
are to  be moved,  the current directory being used if this is not given. The
path  must exist  on each  drive referenced  by the filespecs in the compound
filepecs.

     If a  particular directory cannot be moved into the specified or current
directory  (eg. if  a directory  of the  same name  already exists)  then the
offending directory name is printed along with an error message, and the move
operation continues  with the  next directory. If many errors occur, then the
/P option will cause the output to pause at the bottom of the screen.

     Note  that it  is not  possible to move a directory into one of it's own
descendant directories,  as this would produce an invalid sub-directory tree.
An error is given if this is attempted.

Examples

MVDIR COM UTIL

     A  directory called  COM and  all descendant  directories and  files are
moved into  a directory  called UTIL,  both directories  being in the current
directory of the current drive.

MVDIR \COM + \BAT \UTIL

     A  directory  called  COM and  a directory  called BAT,  and both  their
contents, are moved into a directory called UTIL.

MVDIR E:DIR?/H/P ALL
DIR2 -- Duplicate filename

     All  directories in  drive E  matching DIR?  (eg. DIR1,  DIR2 and DIR3),
which may  be hidden,  and the  contents of the directories, are moved into a
directory  called ALL.  A directory called DIR2 already existed in ALL so the
error was printed. Neither of the DIR2 directories were affected at all.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-path", "", "MSX-DOS 2: comandos", "PATH [ [+|-] [d:]path [ [d:]path [ [d:]path ...]] ]", "PATH: Displays or sets the COM and BAT command search path.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

PATH

Format

     PATH [ [+|-] [d:]path [ [d:]path [ [d:]path ...]] ]

Purpose

     Displays or sets the COM and BAT command search path.

Use

     If no  parameters are specified, then the search path currently set will
be displayed, separated by semi-colons (';').

     If  + or - is not given, then the search path will be set to the list of
path names given and any existing search path will be deleted.

     If -  is given before the list of paths, then each path in the list will
be  deleted from the currently set search path, and an error will be given if
any of the given paths do not already exist.

     If +  is given  before the  list of paths, then each path specified will
first  be deleted  from the  currently set search path if it exists, and will
then be  added onto the end. This allows the order of the paths in the search
path  to be  changed and  allows new  paths to  be appended to the end of the
current search  path. The  + syntax  can also  be used  to set  a search path
longer  than can  be given  in one  command, the maximum length of the search
path being 255 characters and the maximum length of a command 127 characters.

     When  searching for  a COM  or BAT file, the paths in the current search
path will  be used  in order  from left  to right. It is recommended that the
paths  in the  search path  are specified  as full paths starting at the root
directory and  with the  drive specified.  If this  is not the case, then the
meaning  of the  search path could change when the current drive or directory
is changed.

     The search  path is  stored as  an environment  item (see  chapter 7  on
Environment Items), and so can also be accessed with the SET command.

Examples

PATH E:\COM E:\BAT

     When a COM or BAT command is next searched for, the directories searched
will  be the current directory of the current drive, the COM directory in the
root directory  of drive  E: and  the BAT  directory in the root directory of
drive E:, in that order.

PATH
; E:\COM; E:\BAT

     No parameters were given so the current search path was printed.

PATH +A:\COM; A:\BAT

     The  directories A:\COM  and A:\BAT  are added  to the end of the search
path.

PATH
; E:\COM; E:\BAT; A:\COM; A:\BAT

     The new search path is printed.

PATH -E:\COM, E:\BAT

     The directories  E:\COM and  E:\BAT are  deleted from the current search
path.

PATH
 ; A:\COM; A:\BAT

     The new search path is again printed.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-pause", "", "MSX-DOS 2: comandos", "PAUSE [comment]", "PAUSE: Prompts and waits for a key press in a batch file.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

PAUSE

Format

     PAUSE [comment]

Purpose

     Prompts and waits for a key press in a batch file.

Use

     The comment consists of an arbitrary sequence of characters.

     The comment,  if given, is printed followed by the prompt 'Press any key
to  continue... ' on the next line. The system will then wait for a key to be
pressed and  will echo  the key pressed if it is a printable character. If no
comment is given as a parameter, then just the prompt will be printed.

     The  main use  of this  command is  to issue prompts from within a batch
file.

Examples

PAUSE
Press any key to continue...

     No comment was given, so just the prompt was printed.

PAUSE Insert document disk in drive B:
Insert document disk in drive B:
Press any key to continue...

     The comment  given was  'Insert document  disk in  drive B:' so this was
printed followed by the prompt.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-ramdisk", "", "MSX-DOS 2: comandos", "RAMDISK [number[K]] [/D]", "RAMDISK: Displays or sets the RAM disk size.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

RAMDISK

Format

     RAMDISK [number[K]] [/D]

Purpose

     Displays or sets the RAM disk size.

Use

     If  no parameters  are given, then the current RAMDISK size is displayed
as the number of kilobytes.

     The  number, if  given, specifies the maximum size for the new RAM disk,
and is  specified in kilobytes. The range is 0 to 4064. If the number is 0 or
only  /D is  specified, the  RAM disk  will be  deleted. This  number will be
rounded up  to the  nearest multiple  of 16K  since the  RAM disk is always a
multiple  of 16K.  A RAM  disk smaller then the specified maximum size may be
created if there is not enough free memory for the full size, although a 'not
enough memory' error will be given if there is no memory at all available for
the RAM  disk. Note that the number specified is the maximum amount of RAM to
use  for the  RAM disk,  which is  not the same as the maximum amount of free
space available  on the  newly-created RAM disk since the system needs to use
some for FAT or directories.

     On MSX machines with 128K RAM, the maximum amount of RAM disk is 32K.

     If  a  RAM  disk already  exists before  a new  one is  created, then  a
'Destroy  all data  on RAM disk (Y/N)?' prompt is printed to avoid accidental
loss of  data. /D  can be  given which will automatically delete any existing
RAM disk first, thus suppressing the prompt.

     Having created a RAM disk, it can be referred to as drive H:.

     The RAMDISK command is normally only used in an AUTOEXEC.BAT batch file,
with a  large number  specified so  that as  large a  RAMDISK as  possible is
created.  It is  not advisable  to keep  any data  on a RAM disk except for a
short length of time that is not also kept on a floppy disk, since it will be
lost if, for example, the power to the computer fails.

Examples

RAMDISK
RAMDISK=160K

     No parameters  were given,  so the current size is printed, in this case
160K.

     RAMDISK
* RAM disk does not exist

     No  parameters were given but no RAM disk has been created, so the error
is given.

RAMDISK = 300
Destroy all data on RAM disk (Y/N)? y

     A RAM disk already existed, so the prompt was printed. In this case, the
reply was 'y' so the current RAM disk was deleted and the new one set up with
a maximum size of 300K.

RD

     See RMDIR

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-rem", "", "MSX-DOS 2: comandos", "REM [comment]", "REM: Introduces a comment in a batch file.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

REM

Format

     REM [comment]

Purpose

     Introduces a comment in a batch file.

Use

     The  comment  is  simply  ignored, and  the next  command executed.  The
comment consists  of a sequence of any characters up to the maximum length of
a command line (127 characters).

Examples

REM This is my AUTOEXEC batch file

     This  command, either  in a  batch file or typed in, does nothing at all
with it's parameters.

REN

     See RENAME

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-rename", "", "MSX-DOS 2: comandos", "RENAME [/H] [/P] compound-filespec filename", "RENAME: Renames one or more files.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

RENAME

Format

     RENAME [/H] [/P] compound-filespec filename
                    or
     REN [/H] [/P] compound-filespec filename

Purpose

     Renames one or more files.

Use

     The compound-filespec specifies the files that are to be renamed, and /H
allows hidden files to be included in the rename operation.

     The  second filename  specifies the new name for the files. A '?' in the
new name  indicates that  the corresponding character from the filename being
renamed  will be  used, thus  allowing an  ambiguous rename.  Thus '*' in the
second filename, which is just equivalent to a series of '?'s, indicates that
the whole of the filename or extension will remain unchanged.

     If for some reason a particular file cannot be renamed (eg. if a file or
directory  with the  new name  already exists or they are read-only) then the
offending filename will be printed along with an error message and the rename
operation will  continue with  the next  file. If  many errors occur, then /P
will cause the output to pause at the end of the screen.

Examples

RENAME FILE1 FILE2

     The  file FILE1 in the current directory of the current drive is renamed
to FILE2.

REN B:\DIR1\*.DOC/H/P *.OLD
FILE2.DOC -- Duplicate filename

     All  files  matching  *.DOC  in the  directory called  DIR1 in  the root
directory of drive B:, including hidden files, are renamed with the same main
name but  with an  extension of .OLD. The file FILE2.DOC could not be renamed
because  there was  already a  file called FILE2.OLD in the directory, so the
error was  printed. Neither  FILE2.DOC nor  FILE2.OLD was  altered at all. If
many  such errors  had been printed, then a prompt would have been printed at
the bottom of every screen full, since /P was given.

     REN DOC + FILE1 *.OLD

     All files  in the  directory called  DOC and the file FILE1, both in the
current  directory of  the current  drive, and  renamed with  an extension of
.OLD.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-rmdir", "", "MSX-DOS 2: comandos", "RMDIR [/H] [/P] compound-filespec", "RMDIR: Removes one or more sub-directories.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

RMDIR

Format

     RMDIR [/H] [/P] compound-filespec
                    or
     RD [/H] [/P] compound-filespec

Purpose

     Removes one or more sub-directories.

Use

     The compound-filespec specifies which directories are to be deleted, and
/H allows hidden directories to be included in the delete operation.

     In  order to delete a directory, it must contain no other files or other
directories except  for the special '.' and '..' directories which are always
contained in a directory. These are put in a new directory when it is created
and cannot be removed. It is these that allow '.' and '..' to be used in path
names to specify the current and parent directories respectively (see chapter
3 on Notation for a description of paths).

     If a  directory cannot  be deleted for some reason (eg. it is not empty)
then  the name  of the  offending directory  is printed  along with  an error
message, and  the delete operation continues with the next directory. If many
errors occur then /P will cause the output to pause at the end of the screen.

Examples

RMDIR DIR1

     The  directory called DIR1 in the current directory of the current drive
is removed.

RD B:\COM + B:\BAT

     The directories COM and BAT are removed from the root directory of drive
B:.

RD \*.*
UTIL -- Directory not empty

     An attempt was made to remove all directories from the root directory of
the current drive, but a directory called UTIL was not empty and so the error
was printed. UTIL and it's contents are not affected at all.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-rndir", "", "MSX-DOS 2: comandos", "RNDIR [/H] [/P] compound-filespec filename", "RNDIR: Renames one or more sub-directories.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

RNDIR

Format

     RNDIR [/H] [/P] compound-filespec filename

Purpose

     Renames one or more sub-directories.

Use

     The compound-filespec  specifies the directories that are to be renamed,
and  /H allows hidden directories to be included in the rename operation. The
contents of the directories remain unchanged.

     The second filename specifies the new name for the directories. A '?' in
the  new name indicates that the corresponding character from the name of the
directory being renamed will be used, thus allowing an ambiguous rename. Thus
'*' in  the second  filename, which  is just  equivalent to a series of '?'s,
indicates  that the  whole of the filename or extension of the directory name
will remain unchanged.

     If for  some reason  a particular  directory cannot be renamed (eg. if a
file  or  directory  of  the  new  name  already exists)  then the  offending
directory  name will  be printed  along with  an error message and the rename
operation will  continue with  the next directory. If many errors occur, then
/P will cause the output to pause at the end of the screen.

Examples

RNDIR UTIL COM

     The  directory called UTIL in the current directory of the current drive
is renamed COM.

RNDIR A:\*.*/H/P *.OLD
UTIL -- Duplicate filename

     All directories,  hidden and  not hidden, in the root directory of drive
A:  are renamed  with an  extension of  .OLD. The directory UTIL could not be
renamed because a directory called UTIL.OLD already existed, so the error was
printed. If  many such errors were printed then /P would cause a prompt to be
printed at the end of every screen full.

RNDIR COM + BAT *.OLD

     The  directories  COM  and  BAT  are  renamed  to  COM.OLD  and  BAT.OLD
respectively.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-set", "", "MSX-DOS 2: comandos", "SET [name] [separator] [value]", "SET: Displays/sets environment items.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

SET

Format

     SET [name] [separator] [value]

Purpose

     Displays/sets environment items.

Use

     If no parameters are given, then all currently defined environment items
and their current values are displayed. Initially there are several items set
up to default values (see chapter 7 on Environment Items).

     If  just a name is given as the parameter, then the current value of the
specified environment item is printed.

     If the  name is  followed by  a separator, then the separator is ignored
and  the name  is set  to the following value. If the value is blank (ie. not
given) then the environment item is deleted from the environment space.

     The area  of memory  used for  environment items  is also  used for disk
buffers.  Thus  if  a 'not  enough memory'  error occurs  when using  the SET
command,  then it  may help  to reduce  the number  of disk  buffers (see the
BUFFERS command).

     Chapter  7  contains more  information about  environment items  and the
items and values that are set up by default.

Examples

     SET
     ECHO=OFF
     PROMPT=OFF
     PATH=;
     TIME=12
     DATE=yy-mm-dd
     HELP=A:\HELP
     SHELL=A:\COMMAND2.COM

     No parameters  were given,  so all  the currently  set environment items
were printed, in this case typical default values.

SET HELP=A:\HELP

     An item called HELP is set to the value A:\HELP.

SET HELP
A:\HELP

     The current value of HELP is printed.

SET HELP=

     The  item  HELP  is  set  to a  null value,  thus removing  it from  the
environment item list.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-time", "", "MSX-DOS 2: comandos", "TIME [time]", "TIME: Displays or sets the current time.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

TIME

Format

     TIME [time]

Purpose

     Displays or sets the current time.

Use

     If  the time  is given  after the  command, then the time is set to this
value (for the format see below). If the time is not given after the command,
then the  current time is printed and the new time is prompted for and input.
If  no input  is given  (ie. if  the 'enter'  key alone  is pressed) then the
current time is not altered. Otherwise the input is assumed to be a new time,
and is  interpreted as  described below. If the time is invalid then an error
message is displayed and the new time again prompted for and input.

     The  time is expected to consist of up to four numbers, separated by one
of the following characters:

     space tab , - . / :

with  spaces allowed  either side  of the character. Any missing numbers will
default to  the current  setting. The first number is the hour, the second is
the minutes, the third is the seconds and the forth is the centi-seconds. The
centi-seconds are not printed however since it is not very useful to know the
current value, or indeed to enter a new one.

     The  format in which the time is printed is flexible and can be changed.
An environment  item (see  chapter 7 on Environment Items) called TIME is set
up  by default  to the  value '12',  which indicates  that the  time will  be
printed in  12 hour  format with  a following  'a' or  'p' for am and pm. The
command  SET TIME  24 will  cause the time to be printed in 24 hour mode. The
time  can  be  input  unambiguously in  either format.  The time  format also
affects the times printed by the DIR command.

Examples

TIME 16:45

     The current time is set to 4:45 pm.

TIME
     Current time is 10:45:00a
     Enter new time:

     No parameters  were given,  so the current time is printed (in this case
in 12 hour mode) and the new time prompted for.

TIME 10-50-30-23

     The time is set to 30.23 seconds after 10:50 am.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-type", "", "MSX-DOS 2: comandos", "TYPE [/H] [/P] [/B] compound-filespec | device", "TYPE: Displays data from a file or device.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

TYPE

Format

     TYPE [/H] [/P] [/B] compound-filespec | device

Purpose

     Displays data from a file or device.

Use

     The  compound-filespec specifies the files that are to be displayed, and
/H allows  hidden files  to be  typed. If the compound-filespec is ambiguous,
then the filename is printed before each one is typed.

     If  /B is  specified, then  data is  read from  each file  and displayed
without modification  on the  screen, until  the end of file is reached. This
may  have  strange  effect  on  the  screen  if  the  file  contains  control
characters.

     If  /B is  not given,  then TYPE will look for the end-of-file character
(CTRL-Z) and  stop when  it finds it. Also control characters except carriage
return,  line feed  and tab  will be  converted into  characters that  can be
printed, A for ^A, W for ^W, etc.

     If /P  is given,  then the  output will  pause at  the end of the screen
until a key is pressed.

Examples

     TYPE FILE1

     Data  is read  from the  file and printed on the screen, up to the first
end-of-file character.

TYPE *.BAT/H/P

     All batch  files, including  hidden ones,  are read  in and displayed. A
prompt is printed at the end of every screen full.

TYPE AUTOEXEC.BAT + REBOOT.BAT

     The files AUTOEXEC.BAT and REBOOT.BAT are displayed.

TYPE /B DIR1

     All  files  in  the  directory  DIR1 are  printed on  the screen  and no
interpretation is put on the data in the files.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-undel", "", "MSX-DOS 2: comandos", "UNDEL [filespec]", "UNDEL: Recovers a previously deleted file.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

UNDEL

Format

          UNDEL [filespec]

Purpose

          Recovers a previously deleted file.

Use

     The  filespec specifies which files are to be undeleted if possible, and
defaults to *.*.

     Files can only be undeleted if they have been deleted using MSX-DOS 2 on
an MSX-DOS  2 formatted  disk and if no disk allocation has taken place since
the  file was  originally deleted,  which usually  means that they have to be
undeleted immediately after they have been deleted.

     Each deleted file and directory reference that is found in the directory
specified  by the  filespec will  be undeleted if it's name is matched by the
filename in  the filespec, and if undeletion is possible. UNDEL can therefore
be  used to restore a directory removed with the 'RD' or 'RMDIR' commands; to
restore the  contents of  the directory  a further  UNDEL command is required
specifying the now undeleted directory.

     Note  that UNDEL  is a  transient command,  and therefore must be loaded
from disk.

Examples

UNDEL B:HELP.MAC

     Attempts to  undelete the  file HELP.MAC  from the  current directory of
drive B:.

UNDEL A:\DIR1

     All undeletable files and directories in DIR1 are undeleted.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-ver", "", "MSX-DOS 2: comandos", "VER", "VER: Displays the system's version numbers.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

VER

Format

     VER

Purpose

     Displays the system's version numbers.

Use

     The version  numbers of  the three  main components  of the MSX-DOS disk
system are displayed. Each version number consists of three digits. The first
digit  is the main MSX-DOS version number and for MSX-DOS 2 will always be 2.
The second  digit is  the version  number and will change for future versions
that have, for example, had extra major features added. The last digit is the
release number and will change with different releases of the same version of
the system which have had minor changes, improvements and corrections made.

Examples

VER
MSX-DOS kernel version 2.20
MSXDOS2.SYS version 2.20
COMMAND2.COM version 2.20
Copyright 1988 ASCII Corporation

     The version numbers of all the components of the MSX-DOS disk system are
printed out.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-verify", "", "MSX-DOS 2: comandos", "VERIFY [ON | OFF]", "VERIFY: Displays/sets the current disk write verify state.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

VERIFY

Format

     VERIFY [ON | OFF]

Purpose

     Displays/sets the current disk write verify state.

Use

     If  no parameters  are given, then the current verify state is displayed
on the screen.

     If ON or OFF is given, then the verify state is changed appropriately.

     The verify  state affects all writes to disk. If OFF, the default state,
then  data is  simply written.  If ON, then after writing the data it is read
back and  compared with the original to ensure that it was written correctly.
The extra overhead of this means that writing is slower when verify is on.

     This  feature depends  on the device driver, so this will have no effect
if the driver does not have the feature.

Examples

VERIFY
     VERIFY=OFF

     No parameters  were given, so the current verify state is printed, which
in this case is off.

VERIFY ON

     Disk write verification is turned on.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-vol", "", "MSX-DOS 2: comandos", "VOL [d:] [volname]", "VOL: Displays or changes the volume name on a disk.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

VOL

Format

     VOL [d:] [volname]

Purpose

     Displays or changes the volume name on a disk.

Use

     If  no parameters  are given, or if only a drive name is given, then the
volume name of the current or specified drive is printed.

     If a  volname is given, then the volume name of the specified or current
drive is changed to the specified volume name.

Examples

VOL B:
     Volume in drive B: has no name

     Just a drive was given, so the volume name for the disk in that drive is
printed. In this case there was no volume name defined.

VOL B:BACKUP

     The volume name of the disk in drive B: is changed to BACKUP.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-xcopy", "", "MSX-DOS 2: comandos", "XCOPY [filespec [filespec]] [options]", "XCOPY: Copies files and directories from one disk to another.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

XCOPY

Format

     XCOPY [filespec [filespec]] [options]

Purpose

     Copies files and directories from one disk to another.

Use

     The options available are:

     [/A] [/E] [/H] [/M] [/P] [/S] [/T] [/W] [/V]

     XCOPY  is  an  extended  file  copying  command  (compare with  the COPY
command)  that can  selectively copy  both files  and directories.  The first
filespec specifies the source filenames, and if /H is given then hidden files
will also  be copied.  The second  filespec is the destination filename. Thus
files can be renamed during the copy (as in the standard COPY command).

     /T  (Time) will cause the copied files to have the current date and time
rather than the source file's date and time.

     If  /A  (Archive)  is  specified,  then  only files  with the  'archive'
attribute set  are copied. A file has an archive attribute in the same way as
a  'hidden' attribute  and a 'read only' attribute. It is set whenever a file
is updated (written to).

     /M is  similar to /A, but resets the archive bit after copying the file.
Thus, using this option, files can be regularly copied onto another disk only
if they have been updated, providing a file backup facility.

     /S  causes  XCOPY  to copy  directories as  well as  files. Within  each
directory,  all files  are copied  and then  any matching  files within  each
directory are  copied, with the directory being created on the destination if
it does not already exist. Normally, these directories will not be created if
no files are to be copied into them.

     /E  can be  given to cause the /S option to create all directories, even
if they are empty.

     The  /P  (Pause)  option  will cause  XCOPY to  pause and  prompt before
copying each file, which allows files to be selectively copied.

     /W (Wait)  causes XCOPY to pause and prompt before copying any files, so
that disks can be changed.

     /V option can be given to turn write verification on for the duration of
the  XCOPY command  (see the  VERIFY command).  This will ensure that data is
written correctly  to disks  if the device driver being used has the feature,
but will slow the operation down.

     Note that XCOPY is a transient command, and so must be loaded off disk.

Examples

XCOPY B:\

     All  files in  the root  directory of drive B: are copied to the current
directory of the current drive. There is no advantage in this case over using
the standard built-in COPY command.

XCOPY *.* B: /H/S/M

     All  files, including  hidden files, are copied to drive B: only if they
have  been  modified  since  a similar  command was  last given.  The archive
attributes are  then reset  so that  the files  are marked as unmodified. Not
only  are  all  the  files  in  the  current  directory  copied,  but  so are
directories and all their descendant directories and files.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-xdir", "", "MSX-DOS 2: comandos", "XDIR [filespec] [/H]", "XDIR: Lists all files within directories.",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

XDIR

Format

     XDIR [filespec] [/H]

Purpose

     Lists all files within directories.

Use

     The  filespec specifies  which files  are to  be listed,  and /H  allows
hidden files to be included.

     XDIR is  similar to the DIR command, but does not print the files' dates
and times.

     After  all files in the specified directory have been listed, then files
within descendant  directories are  also listed, and are shown indented. This
allows a DIR of a complete directory tree or disk to be obtained.

     Note that XDIR is a transient command, and so must be loaded off disk.

Examples

XDIR

     The  directory of  the entire  disk in  or descending  from the  current
directory of the current drive is printed.

Volume in drive A: is MSX-DOS 2
X-Directory of A:\

MSXDOS2.SYS     r     4480
COMMAND2.COM    r    14976
AUTOEXEC.BAT            57
REBOOT.BAT              57

\UTILS
        CHKDSK.COM       7680
        DISKCOPY.COM     7168
        FIXDISK.COM       768
        UNDEL.COM        3968
        XCOPY.COM       10112
        XDIR.COM         7168
        MKSYS.BAT         569
        AUTOEXEC.BAT       47
        REBOOT.BAT         90
\HELP
        ASSIGN.HLP        819
        ATDIR.HLP        1527
        ATTRIB.HLP       1828
        .
        .
        .
292K in 117 files   530K free

XDIR B:\DIR1

     All the  files and  directories and  their contents are printed from the
directory DIR1.

XDIR \*.COM/H

     The  names  of  all files,  including hidden  files, matching  *.COM are
displayed.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-redirection-and-piping", "", "MSX-DOS 2: manual", "", "Redirection and Piping (MSX-DOS 2)",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

5. Redirection and Piping
=========================

     COMMAND2.COM  offers the  redirection and  piping features  as described
below. They  may be bypassed by setting the environment item "REDIR" to "OFF"
("SET REDIR=OFF"), so the compatibility to MSX-DOS 1 or CP/M can be achieved.

Redirection

     Most  commands, CP/M  programs and  MSX-DOS programs  output text to the
screen by  writing to  the 'standard  output', and  read from the keyboard by
reading from the 'standard input'. COMMAND2.COM, however, provides facilities
for  changing the  standard input and standard output for the duration of the
command to  refer to  other MSX-DOS  devices or to files on disk by including
one  or more  of the  redirection symbols  <, >  and >>  on the command line,
followed by a filename.

     For example,  the ECHO  command normally just outputs it's parameters to
the  screen by  writing the characters to the standard output. It can be made
to output to the printer instead by redirecting it's output, as follows:

          ECHO text >PRN

which changes the standard output to refer to the device PRN for the duration
of the ECHO command. Similarly, the command:

          ECHO text >file1

will  cause the specified file ("file1") to be created, and the output of the
ECHO command  written to  the file.  To append the output of a command to the
end  of an  existing file, the >> symbol can be used instead of the > symbol,
and the file will only be created if it does not already exist.

     To  change the  standard input, the < symbol is used in a similar manner
to the  > symbol. In this case, the file must already exist, and must contain
adequate  input for the command. If the command attempts to read input beyond
the end of the file, then it will be aborted since it cannot continue.

     When redirection information is given on the command line, it is used by
COMMAND2.COM to  set up  the redirection  and then  removed from  the command
line.  Thus  in  the  examples  above,  the  ECHO command  will not  echo the
redirection symbols or the filename.

     If  the  input  or  output  of  a  batch file  is redirected,  then that
redirection  is applied  to all  commands within  the batch  file. Individual
commands within  the batch  file may still be redirected, however, which will
override  the batch  file redirection.  See chapter 6 on Batch Files for more
information on commands in batch files.

Piping

     As  well as  redirecting the input and output of a command or program to
another device  or a  disk file,  it is  possible to  redirect or  'pipe' the
standard  output of one command into the standard input of another. Typically
the second  command will  be a  program which reads from it's standard input,
modifies  the data,  and writes it to it's standard output. Such a program is
called a  'filter'. For  example, a  filter could be produced which read data
from  it's standard input, sorted it into alphabetical order, and wrote it to
it's standard output. Thus the output of the DIR command could be sorted.

     Piping  is indicated  on the command line by separating the two commands
by the  | symbol.  The command  to the left of the | symbol will be performed
first,  and it's  output will  be redirected  to a  temporary file created by
COMMAND2.COM. Then  the second  command will be performed, with it's standard
input  redirected from the same temporary file. When the second command ends,
the temporary file will be deleted. The standard output of the second command
may of course have been piped into the standard input of a third command, and
so on.

     If any input redirection occurs on a command line involving a pipe, then
the redirection is applied to the first command in the pipe, as all the other
commands  receive  their  standard  input  from  the standard  output of  the
previous command  in the pipe. Similarly, if any output redirection occurs on
a  command line involving a pipe, then the redirection will apply to the last
command on the command line.

     It  is not  possible to  use pipes  on either the input or the output of
batch files directly. It is, however, possible to use piping with batch files
if they  are executed  with the  COMMAND2 command (see chapter 4) since it is
then the COMMAND2 command that is being redirected and not the batch file.

     As mentioned  above, in order to pipe the output of one command into the
input   of  another,   temporary  files   will  be  created  and  deleted  by
COMMAND2.COM. The  location of these temporary files is specified by the TEMP
environment  item  (see  chapter 7  on Environment  Items), and  this may  be
changed  to refer  to any  drive and  directory (for  example piping  will be
speeded up  considerably if  TEMP refers  to a  directory on  a RAM disk). By
default,  TEMP refers  to the  root directory  of the boot disk. The filename
used  for  the  temporary  file is  created by  COMMAND2.COM, so  TEMP should
specify just the drive and directory. The filename is of the form:

          %PIPExxx.$$$

where xxx  is a  three digit  number chosen  by COMMAND2.COM to avoid clashes
with any other files in the TEMP directory.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-batch-files", "", "MSX-DOS 2: manual", "", "Batch Files (MSX-DOS 2)",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

6. Batch Files
==============

     When  a command  is given  to MSX-DOS  and it is not one of the internal
commands, a  file of  that name  is searched  for with an extension of COM or
BAT.  If not  found in  the current directory then the current search path is
searched (see  the PATH  command). If  a COM file is found, then it is loaded
and  executed. If  a BAT  file is found, then MSX-DOS starts execution of the
batch file.

     A  batch file is a text file that contains a list of commands, and these
commands are  read from the file a line at a time and executed as though they
were  typed at  the keyboard.  Several of the commands described in chapter 4
are in fact provided mainly for use in batch files, such as ECHO and PAUSE.

     As  each  command  is  read,  normally  it is  executed immediately.  An
environment  item  ECHO exists,  however, that  can be  set to  ON (with  the
command SET  ECHO ON)  to cause each command line to be printed on the screen
before it is executed (see chapter 7 for Environment Items). The command line
is  echoed in  this way  after % parameter substitution (see below) has taken
place. The command SET ECHO OFF will restore the normal state.

     In the  command line  that invoked the batch file, parameters may follow
the  name of  the batch file just like any other command or transient program
name.  These  parameters  may  be  accessed  anywhere  in  the batch  file by
specifying %0 to %9. %1 is the first parameter specified in the command line,
%2 is  the second  and so  on. %0 is the name of the batch file itself. The %
number  will be  replaced by  the parameter on the original command line, and
may appear  anywhere on a batch file command line. To actually use a % symbol
on a command line a double % must be given ('%%') which will then be replaced
by a single one.

     If  the  execution  of  any  command  in  a  batch  file  is  terminated
prematurely  for some  reason (typically  the control-STOP  or control-C  key
being pressed) then the following prompt is issued:

          Terminate batch file (Y/N)?

     If the  response to  this is 'Y', then execution of the whole batch file
is  stopped. If the response is 'N', then batch file execution continues with
the next command in the batch file.

     After  MSX-DOS has executed a command in batch file, it may need to read
the next  command in  the batch  file off disk. If the correct disk is not in
the drive when it comes to do this, then a prompt is issued. For example, the
following  prompt will  be issued  if the  batch file was originally executed
from drive A::

          Insert disk for batch file in drive A:
          Press any key to continue...

When  the  correct  disk  has  been inserted  and a  key pressed,  batch file
execution will continue normally.

     Below  is a  very simple  batch file,  which just  prints the  first few
parameters.

          ECHO Parameter 0 = %0
          ECHO Parameter 1 = %1
          ECHO Parameter 2 = %2
          ECHO Parameter 3 = %3

     If this is called MYBAT.BAT then the command MYBAT a b c will output

          Parameter 0 = MYBAT
          Parameter 1 = a
          Parameter 2 = b
          Parameter 3 = c

     When  MSX-DOS starts  up for the first time, a special batch file called
AUTOEXEC.BAT is  looked for  and is  executed if  found. This may contain any
MSX-DOS command, and usually contains once-only initialization commands, such
as a RAMDISK command to set up a RAM disk.

     One  % parameter  is passed  to AUTOEXEC.BAT  as %1. This the drive that
MSX-DOS booted from and is in the form of a normal drive letter followed by a
colon.

     Another special  batch file is REBOOT.BAT. This is executed when MSX-DOS
is  re-booted after  using DISK-BASIC. As with AUTOEXEC.BAT files, the single
%1 parameter passed is the drive from which MSX-DOS was re-booted.

     Usually  some commands  need to be performed whenever MSX-DOS is booted,
whether for the first time or sometime later, and these are put in the REBOOT
batch file.  They can then be executed from the AUTOEXEC batch file by ending
it  with the  command REBOOT %1. An example of a command that might be put in
the REBOOT  batch file  is the  PATH command  to set up the transient command
search  path. When  setting up  the search  path using the command, %1 can be
used to set up the path on whatever drive was booted from.

     When  a command  in a batch file is the name of another batch file, then
that second  batch file  is executed in the normal way. When it ends, control
passes  back to  the interactive  command interpreter,  and not  to the first
batch file. Batch file commands thus 'chain' together.

     If it  is desired  to 'nest' batch files ie. to pass control back to the
first  batch file above, then this can be done with the COMMAND2 command (see
chapter 4),  passing the name of the second batch file as the parameter. Then
when  the second  batch file  ends, the  first one will be continued with the
command after the COMMAND2 command.

     A typical AUTOEXEC batch file is as follows:

          ECHO AUTOEXEC executing
          RAMDISK 100
          RAMDISK
               COPY COMMAND2.COM H:\
          REBOOT %1

     A typical REBOOT batch file is as follows:

          ECHO REBOOT executing
          PATH H:\, %1\UTILS, %1\BATCH
               SET SHELL=H:\COMMAND2.COM
               SET TEMP=H:\
          SET PROMPT ON
          H:

     When the  AUTOEXEC batch file executes, the message "AUTOEXEC executing"
is  printed, and  then a  RAM disk  is set  up with  a maximum  size of 100K.
Another RAMDISK command is then given which will print out the actual size of
RAM disk created. The COPY command then copies COMMAND2.COM onto the RAM disk
so that  it can  load and  re-load quickly.  Finally the REBOOT batch file is
executed, with the %1 parameter (the boot drive) passed to it.

     The  REBOOT batch  file prints a message and then sets a PATH. The first
item in  the path  refers to  the RAM  disk that  was created by the AUTOEXEC
batch  file, and  the other items refer to directories on the disk from which
MSX-DOS was  booted (ie.  %1). The  SHELL environment  item is set up so that
COMMAND2.COM  can re-load  quickly off the RAM disk, and the TEMP environment
item is  set up  so that  temporary piping files are created on the RAM disk.
The  prompt is  set ON so that the current directory is printed as the prompt
and, finally, the RAM disk is made the default drive.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-environment-items", "", "MSX-DOS 2: manual", "", "Environment Items (MSX-DOS 2)",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

7. Environment Items
====================

     MSX-DOS maintains  a list  of 'environment  items' in  its work area. An
environment item is a named item that has a value associated with it.

     An  environment  item  can have  any name  chosen by  the user,  and can
consist  of the  same characters  that are  valid in  a filename. The maximum
length  of  an  environment  item name  is 255  characters. MSX-DOS  provides
several environment items set up by default.

     The  value  of  an  environment  item is  simply a  string of  arbitrary
characters  up to  a maximum length of 255. No processing is performed on the
characters and so the casing of characters is preserved. Any environment item
that does not exist has a null value (ie. no characters).

     An environment  item can  be changed or set up by the SET command, which
can also display currently set environment items.

     The  environment items  set up  by default and the manner in which their
value is interpreted are as follows:

ECHO

     This controls the echoing of lines read from a batch file (see chapter 6
on  Batch   Files).  Any  value  except  'ON'  (lowercase  also  allowed)  is
interpreted as 'OFF'.

PROMPT

     This  controls the  displaying of the prompt at command level. Any value
except 'ON' is interpreted as 'OFF'.

     When PROMPT is OFF, as it is by default, then the prompt consists of the
current drive followed by '>' eg. A>.

     When PROMPT is ON, then the prompt consists of the current drive and the
current directory  of that  drive followed by '>' eg. A:\COM>. In order to do
this, the current drive must be accessed to read the current directory and so
may take a little longer to appear.

PATH

     The current search path by which COMMAND2.COM searches the command given
is  maintained as  an environment  item PATH,  and it  is this  that the PATH
command manipulates.

SHELL

     The  SHELL  environment  item  indicates where  the command  interpreter
(COMMAND2.COM) exists,  and is set up by default to where it was loaded from.
When the command interpreter needs to re-load itself from disk (after running
a  transient command)  it looks at the SHELL environment item and attempts to
load itself  from the  file that it specifies. If this gives an error then it
attempts  to load  itself from  the root  directory of  the drive that it was
originally loaded from.

     To cause the command interpreter to re-load itself from another drive or
directory, COMMAND2.COM can be copied there and SHELL set to refer to it. For
example, it might be copied to the RAMDISK with the command COPY COMMAND2.COM
H:\ and then SHELL set with the command SET SHELL=H:\COMMAND2.COM.

TIME

     TIME specifies the format the time is displayed by MSX-DOS. If not '24',
which indicates that it is displayed as a 24-hour time, then '12' is assumed,
which  indicates that  it is  displayed as  a 12-hour time with an am. or pm.
indication. The  TIME environment  item does not apply when the time is input
because it can be input in either format unambiguously.

DATE

     DATE specifies the format the date is displayed and input by MSX-DOS. It
defaults  to  a  format  appropriate for  the country  of origin  of the  MSX
machine. It  takes the  form of three letters or three letter pairs separated
by  date/time separators  (see the DATE command). To set the American format,
for example, the command SET DATE=MM/DD/YY could be given.

HELP

     When the  HELP command  is given  the name  of command for which help is
required,  then it  reads the information displayed from a file on disk. This
file is  in the directory specified by the HELP environment item. It defaults
to  a directory  called HELP  in the root directory of the drive that MSX-DOS
was booted from.

APPEND

     APPEND  is  not  actually  defined by  default, but  when set  up is  an
environment item  that has  a special  meaning to the system. It is used only
with standard CP/M programs.

     CP/M programs  do not  know how to use sub-directories because CP/M does
not  have sub-directories, but instead just has the equivalent of the current
directory. When  such a  program opens a file, it searches for it only within
this single directory and thus only has drives and filenames, not paths.

     When a CP/M program is run under MSX-DOS and attempts to open a file, it
only  searches for  the filename  in the current directory of the appropriate
drive. Similarly,  when the user types in a filename to a CP/M program it may
only  contain a drive and filename, and thus also refers only to files in the
current directory.

     When  this search is performed through MSX-DOS, if the file is not found
in the  current directory,  then the APPEND environment item is looked at. If
it  is not  set up  then the  file has  not been found. If set up, then it is
assumed to  be a  path name,  and specifies a single alternative directory in
which the search for the file continues.

     This will only be of use if the CP/M program opens a file and then reads
or writes  to it.  If it  attempts to, for example, delete a file or create a
file,  then APPEND  will not be used. Indeed, it may have undesirable effects
and for  this reason it is recommended that APPEND is used normally only in a
batch  file which  sets it  up, executes the CP/M program, and then unsets it
again.

     Typical uses  for APPEND include specifying the directory in which large
programs  (such as  word processors  and database programs) find 'overlay' or
messages files,  and specifying  the directory in which compilers, assemblers
and  linkers find  their source  and working  files. Typical  cases for which
APPEND is  not suitable  and may  have undesirable  effects include editing a
file  with a wordprocessor when the file to be edited probably cannot be in a
directory other than the current one, even if APPEND is set up.

PROGRAM and PARAMETERS

     These  special  environment  items  are set  up by  COMMAND2.COM when  a
transient command  is executed  and removed when it finishes, and should thus
be avoided for general use.

TEMP

     When piping is performed (see chapter 5 on Redirection and Piping) it is
necessary  for COMMAND2.COM  to create  one or  more temporary files, and the
TEMP  environment  item  indicates  the drive  and directory  in which  these
temporary  files  are  to  be  created.  By default,  it refers  to the  root
directory of  the boot  drive, and typically may be changed to refer to a RAM
disk since this increases the speed of piping.

     Although  the standard  MSX-DOS system  only uses  TEMP for  piping, any
other programs and utilities that need to create temporary files may also use
the TEMP environment item.

UPPER

     UPPER controls  whether the  command line  from 80h  to be passed to the
transient  program  be  converted  to  uppercase.  Any  value except  'ON' is
interpreted as 'OFF'.

     When UPPER is 'OFF' (default), no conversion will be done and the values
will be passed to the transient program as they are typed.

     When UPPER is 'ON', each character in the command line will be converted
to  its  associated  uppercase  character  and then  passed to  the transient
program. This is compatible to CP/M environment.

REDIR

     REDIR  controls  whether  the redirection  or piping  characters in  the
command  line  be  processed  by  COMMAND2.COM.  Any  value  except  'ON'  is
interpreted as 'OFF'.

     When REDIR is 'OFF', the redirection or piping characters will be passed
to  the transient  program as  they are  typed, and the transient program may
process them.

     When REDIR  is 'ON' (default), the redirection or piping characters will
be  interpreted and  executed by  COMMAND2.COM, so they will not be passed to
the transient program.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-errors-and-messages", "", "MSX-DOS 2: manual", "", "ERRORS AND MESSAGES (MSX-DOS 2)",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

8. ERRORS AND MESSAGES
======================

Disk errors

     Disk errors  occur when  a command  or program is attempting to access a
disk  and fails  for some reason, such as a disk not being in the drive. When
this happens,  message and  prompt appears  which allows  the user the either
retry  the operation which may now work (eg. if a disk has been inserted into
the drive), to ignore the operation or to abort the entire command.

     An example disk error message and prompt is as follows, and may be given
if the disk was taken out whilst drive A: was being accessed:

          Not ready reading drive A:
          Abort, Retry or Ignore (A/R/I)?

The  'not ready' part of the message indicates why the disk operation failed,
and  other  possibilities  exist  (see below).  'reading' indicates  that the
command was trying to read the disk, and may be replaced by 'writing' if that
is what it was doing. 'drive A:' is the drive in which the disk was attempted
to be accessed.

     The 'Abort,  Retry or  Ignore' part  indicates the possible actions that
can  be taken by the user, and these are selected by pressing the 'A', 'R' or
'I' key.

     If Abort is selected, then the entire command is aborted and the message
'Disk operation aborted' is printed before another command can be typed.

     If  Retry  is selected  then the  failed disk  operation will  simply be
retried  and  may fail  again or  may work,  particularly if  some corrective
action has been taken such as inserting a missing disk.

     Ignore causes the failed disk operation to be ignored by the command. In
many cases,  ignoring an  error may not be recommended and in these cases the
Ignore  option will not even be displayed, although it may still be selected.
Doing so  may however cause serious system malfunction and could destroy data
on  the disk.  Even if the Ignore option is displayed, it should be used with
extreme caution,  and only  when all else fails. Normally Ignore is only used
when  the data on a disk has got corrupted and ignoring the disk error offers
the only possibility of recovering all or part of the data.

     A few  serious errors  which generally  mean the disk has been corrupted
beyond possible use are automatically aborted, and just the appropriate error
message is printed (eg. 'Bad file allocation table').

     The  possible errors  that can  occur as  disk errors and their meanings
are, in alphabetical order, as follows:

Bad file allocation table

     The  disk contains an invalid file allocation table (FAT). The FAT is an
area on  disk in  which the  system keeps information to tell it where on the
disk  the data  in each file is stored. Thus if the FAT is invalid, it cannot
read any  data at  all. This  message usually  means that  the disk  has been
corrupted beyond possible use.

Cannot format this drive

     An  attempt was  made to  format a disk in a drive that does not support
disk  formatting.  This  probably  means  that  a  FORMAT  command was  given
specifying the RAM disk.

Data error

     The data  was read  or written  without error, but the CRC check failed.
This usually means the disk has been corrupted.

Disk error

     The data could not be read or written to the disk.

Incompatible disk

     An  attempt was made to access 2D or 1D disk or a double sided disk in a
single sided drive.

Not a DOS disk

     The disk  is not  a format  that MSX-DOS can read. For example, although
MSX-DOS can execute CP/M programs it cannot access CP/M disks.

Not ready

     The disk is not in the drive being accessed. The disk should be inserted
into the drive and 'Retry' selected.

Sector not found

     MSX-DOS  tried to  read or  write to a non-existent sector. May indicate
that the disk has been corrupted.

Seek error

     The  requested  track  on  the disk  could not  be found.  Could mean  a
corrupted disk or faulty disk drive.

Unformatted disk

     The disk  has not  been formatted.  Use the  FORMAT command  on the disk
before accessing it.

Verify error

     Only  occurs  when  verify is  on, and  means that  data appeared  to be
written  to disk successfully but when read back was found to be different to
that written.

Write error

     Data was not written correctly.

Write protected disk

     The disk is write protected and an attempt was made to write data to it.
The disk should be made unprotected and 'Retry' selected.

Wrong disk and Wrong disk for file

     MSX-DOS accessed a disk once and subsequently needed to access it again,
but found  that the  drive contained a different one. The correct disk should
be inserted and 'Retry' selected.

Command Errors

     Command  errors  occur  when  a  command  cannot  perform it's  intended
function for some reason.

     If  an  error  occurs in  a command  and it  is unlikely  to be  able to
continue,  then an appropriate error message is printed, and the next command
is read at the prompt.

     An example error message is:

          * File not found

     The three  asterisks * are printed first to indicate that an error has
occurred.  The message is then printed, followed by the normal command prompt
on the next line. The possible errors that can occur are listed below.

     When  a command  error occurs  in a  specific situation, an 'error type'
message may  also be  printed. For  example, generally  when a  required file
cannot  be found  on disk,  the 'File not found' message is printed as in the
above example.  However, if  the file  required is  a file  specified by  the
redirection  symbol <  (see chapter  5 on  Redirection and  Piping) then  the
message printed will be:

          * Redirection error: File not found

     The possible error types are:

Batch file error:

     An  error  occurred whilst  attempting to  read from  a batch  file, for
example a disk error occurred and 'abort' was selected.

Piping error:

     The error  occurred during  a piping  operation, probably  in connection
with  the  temporary  files  that  COMMAND2.COM  creates  (see  chapter  5 on
Redirection and Piping). For example, the TEMP environment item did not refer
to a valid drive or directory (see chapter 7 on Environment Items).

Redirection error:

     The  error  occurred  during a  redirection operation.  For example,  an
invalid  filename was specified after a redirection symbol <, > or >>, or the
specified input file was not found (see chapter 5 on Redirection and Piping).

Standard input error:

     An error  occurred on  the standard  input to a command or program after
redirection  or piping  has been  set up,  for example the standard input has
been redirected from a file and the end of the file has been reached.

Standard output error:

     An error  occurred on  the standard output of a command or program after
redirection  or piping  has been  set up, for example the standard output has
been redirected to a file and the disk is full.

     Many  commands  operate  on files  or directories,  and if  an ambiguous
filename  is given  then the command operates on several files or directories
(for example  the RENAME  command or the COPY command). Often an error occurs
when  it is  trying to perform the command on one of the files, but which may
be  successful  on one  of the  other files.  In this  case, the  filename is
printed followed  by the  error message  and the  command then continues. For
example:

          COMMAND2.COM -- File cannot be copied onto itself

     The  possible  command  errors that  can be  given are,  in alphabetical
order, as follows:

Cannot concatenate destination file

     This  error  is  given by  CONCAT and  means that  one of  the filenames
matched by the source file specification is the destination file. This is not
necessarily wrong but may indicate a mistake in the command.

Cannot create destination file

This  is given  by COPY,  and usually means that the destination file for the
file  it  is copying  would, if  it was  created, overwrite  a file  that was
already in  use. This is likely to be a previously copied source file but may
be some other file such as the currently executing batch file.

Cannot overwrite previous destination file

     This  is given by COPY, and means that the destination file for the file
it is copying would, if it was created, overwrite the destination file of the
file  that  was  previously  copied.  This  usually means  that the  intended
destination was a directory but that it's name was misspelled.

Cannot transfer above 64K

     This should not normally occur from commands.

Command too long

     A command  that was  given is  too long. This will not occur when typing
commands  from the  keyboard, but  may occur  from a  batch file. The maximum
length of a command is 127 characters after % parameter substitution.

Ctrl-C pressed

     The command was interrupted by pressing CTRL-C.

Ctrl-STOP pressed

     The command was interrupted by pressing CTRL-STOP.

Directory exists

     A command  attempted to  create a new file or directory on disk with the
same name as an existing directory.

Directory not empty

     The  RMDIR (RD)  command tried to remove a directory that contains files
or other  directories. These  must be  deleted first with the ERASE and RMDIR
commands since directories must be empty before they can be removed.

Directory not found

     A directory command (eg. RNDIR) could not find the specified directory.

Disk full

     There  is no more room on the disk and files will have to be deleted and
the command given again.

Disk operation aborted

     A disk  error occurred  and the 'Abort' option was chosen, thus aborting
the whole command.

Duplicate filename

RENAME  (REN) or  RNDIR cannot  perform the  specified rename because the new
filename is the same as a filename that already exists. Also occurs from MOVE
or MVDIR  because a filename already exists in the destination directory with
the same name as the file or directory being moved.

End of file

     This should not normally occur from commands.

Environment string too long

     This should not normally occur from commands.

Error on standard input

     This  should not  normally occur  from commands, and means that an error
occurred while a command was attempting to read from the keyboard.

Error on standard output

     This should  not normally  occur from  commands, and means that an error
occurred while a command was attempting to write to the screen.

File access violation

     This should not normally occur from commands.

File allocation error

     This should not normally occur from commands.

File cannot be copied onto itself

     The  destination file  when trying  to do a COPY is the same file as the
source.

File exists

     MKDIR (MD) tried to create a new directory but a file with the same name
already exists in the specified directory.

File for HELP not found

     The HELP  command looked  for a file to get the help text from but could
not  find it.  Help files are usually kept in a directory called \HELP on the
boot disk.

File handle not open

     This should not normally occur from commands.

File is already in use

     A command  tried to  modify a  file that  is already being used for some
other purpose, such as the currently executing batch file.

File not found

     A command could not find the specified file or files.

Internal error

     This should not normally occur from commands.

Invalid MSX-DOS call

     This should not normally occur from commands.

Invalid attributes

     Usually means an invalid +/- attribute was specified in ATTRIB or ATDIR.

Invalid date

     The date typed into the DATE command is not a valid date or was typed in
in an invalid format.

Invalid device operation

     A  command cannot  perform it's  function on  one of the built-in system
devices eg. a file cannot be renamed CON.

Invalid directory move

     MVDIR attempted  to move  a directory  into one  of it's  own descendant
directories, which cannot be done.

Invalid drive

     A drive that does not exist was specified.

Invalid environment string

     The  name of an environment item contains invalid characters. Only those
characters valid in filenames are valid in environment item names.

Invalid file handle

     This should not normally occur from commands.

Invalid filename

     A  filename   contains  invalid  characters.  This  may  be  a  filename
explicitly given, or may be the result of attempting to rename a file with an
ambiguous new name.

Invalid number

     A number given in a command contained non-digit characters.

Invalid option

     An invalid letter was given after a / on a command line.

Invalid . or .. operation

     A  command cannot  perform it's  function on  the special  '.' and  '..'
directories that are present at the start of all sub-directories.

Invalid parameter

     The parameter  to a  command is  generally not valid for that command in
some way.

Invalid pathname

     A  path specified  on a  command line does not exist or is syntactically
incorrect.

Invalid process id

     This should not normally occur from commands.

Invalid time

     The time typed into the TIME command is not a valid time or was typed in
in an invalid format.

Missing parameter

     A command expected a parameter but did not find one.

No spare file handles

     Should not normally occur from commands.

Not enough memory

     Not enough  memory is  available for  the given  command. For example, a
large  program to  large to  fit into  memory or  not enough memory for a new
environment string.

Not enough memory, system halted

     This special  error message is printed when MSX-DOS attempts to start up
and  finds  that  there  is  not enough  memory to  continue. As  the message
suggests, the computer must then be reset. This should normally occur.

Pathname too long

     A  path is too long. Either the length of the pathname given exceeds 100
characters, or  the total  length of a path from the root directory to a file
is more than 63 characters.

RAM disk already exists

     Should not normally occur from commands.

RAM disk does not exist

     The  RAMDISK command  was used  to display  the current  size of the RAM
disk, but no RAM disk exists.

Read only file

     An attempt  was made  to modify or overwrite a file marked as read only.
The DIR command shows this, and the ATTRIB command can make it not read only.

Root directory full

     The  fixed maximum  number of  files in  the root directory (often 64 or
112) has been reached. Directories do not have this limitation.

System file exists

     An  attempt was  made to  create a  file which would, if it was created,
overwrite a  file that  is marked as a system file. System files are not used
in MSX-DOS, and are not shown by the DIR command or accessible from any other
commands, and so this error should not normally occur with commands.

Too many parameters

     All  the parameters a command expected were found on a command line, but
there were still more parameters left on the end of the line.

Unrecognized command

     A given  command was  not an  internal command or an external COM or BAT
command found along the current search path as set by the PATH command.

Wrong version of command

     After executing a program, COMMAND2.COM tried to re-load itself from the
COMMAND2.COM file on disk, and found it was not the same version. A prompt is
then printed and COMMAND2.COM will attempt to re-load itself again.

Wrong version of MSX-DOS, system halted

     This special error message is printed when MSX-DOS attempts to starts up
and finds  that some  other part  of the  MSX-DOS system has a version number
earlier  than required.  As the  message suggests,  the computer must then be
reset. This should not normally occur.

     Internally,  errors  are  represented  as  error  numbers.  The  numbers
corresponding  to the  errors above  start at  255 and  decrease. If an error
number is received for which there is no message, then it is printed. Numbers
above 64 are reserved for future version of MSX-DOS and so are called 'system
errors' and  numbers below  63 can be used by external commercially available
programs  and are  called 'user  errors'. User  errors below 32 never print a
message. The  two default  error messages (which will not normally occur from
commands) are thus:

System error 64

     and

User error 63

where  the 64  and 63  are example error numbers. The only command which uses
error numbers is the EXIT command. A list of the actual numbers for the above
messages is available in the Program Interface specification.

Prompt Messages

     There  are  several  situations in  which user  interaction is  required
before  the system can continue with what it was doing, typically inserting a
disk. Also  many potentially  dangerous commands require confirmation prompts
to  be answered  before they  perform their  operation. These  various system
prompts are given below.

All data on drive A: will be destroyed
Press any key to continue...

     This prompt  is given by the FORMAT command, and is issued to reduce the
risk  of accidentally formatting the wrong disk. To abort the FORMAT command,
control-STOP or control-C can be pressed.

Destroy all data on RAM disk (Y/N)?

     A RAMDISK command was given to set up a RAM disk, but a RAM disk already
existed.  If  the  response  to the  prompt is  'Y', then  any files  on this
existing RAM  disk will  be destroyed.  A response  of 'N' or control-STOP or
control-C will abort the command.

Disk in drive A: will only be able to boot MSX-DOS
Press any key to continue...

     This prompt is given by the FIXDISK command, and is issued to reduce the
risk  of accidentally  updating a  non-MSX-DOS 2  disk. To  abort the FIXDISK
command, control-STOP or control-C can be pressed.

Erase all files (Y/N)?

     This  prompt  occurs  when  a DEL  (or ERA  or ERASE)  command is  given
specifying all  the files in a directory, and is issued to reduce the risk of
accidentally deleting a lot of files.

Insert COMMAND2.COM disk in drive A:
Press any key to continue...

     This  may occur  after running a program, and requires a disk containing
COMMAND2.COM in  the root  directory to  be present  in the  specified drive.
After  inserting the disk in the drive (which is the drive from which MSX-DOS
was  originally  booted)  and  pressing a  key, the  system will  continue as
normal. If  COMMAND2.COM has been copied somewhere else (eg. a RAM disk) then
the  SHELL environment  item can  be set up to make COMMAND2.COM re-load from
there instead (see chapter 7 on Environment Items).

Insert batch file disk in drive A:
Press any key to continue...

     This  may occur during the execution of a batch file, and means that the
system needed to read the next command from the batch file but found that the
wrong disk  was in the drive. After inserting the disk in the specified drive
(which  will be  the drive  from which the batch file was originally started)
and pressing a key, execution of the batch file will continue as normal.

Press any key to continue...

This prompt  is generally  issued when some user interaction is required, and
is  normally  printed  after some  other message  which describes  the action
required.  It is also printed by the PAUSE command. To abort the command that
issued the prompt, control-STOP or control-C can be pressed.

Terminate batch file (Y/N)?

When MSX-DOS  aborts a  command prematurely (such as when the control-STOP or
control-C key is pressed) and the command was executing in a batch file, this
prompt  is issued.  If the  response is  'Y' then the batch file will also be
aborted. If 'N' is the response  then the  batch file will continue with the command that follows
the aborted command.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-command-summary", "", "MSX-DOS 2: manual", "", "COMMAND SUMMARY (MSX-DOS 2)",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

9. COMMAND SUMMARY
==================

     The  following  is  a list  of all  the standard  commands available  in
MSX-DOS, together with their syntax and purpose.

ASSIGN [d: [d:]]

          Sets up the logical to physical translation of drives.

ATDIR +|-H  [/H] [/P] compound-filespec

          Changes  the  attributes  of  directories  to make  them
     hidden/not hidden.

ATTRIB +|- R|H [/H] [/P] compound-filespec

          Changes  the attributes of files to make them hidden/not
     hidden and read only/not read only.

BASIC [program]

          Transfers control to MSX disk BASIC.

BUFFERS [number]

          Displays or  changes the  number of  disk buffers in the
     system.

CD [d:] [path]

          Displays or changes the current directory.

CHDIR [d:] [path]

          Displays or changes the current directory.

CHKDSK [d:] [/F]

          Checks the integrity of the files on the disk.

CLS

          Clears the screen.

COMMAND2 [command]

          Invokes the command interpreter.

CONCAT [/H] [/P] [/B] [/V] compound-filespec  filespec

          Concatenates (joins together) files.

COPY [/A] [/H] [/T] [/V] [/P] compound-filespec [filespec]

          Copies  data from  files or  devices to  other files  or
     devices.

DATE [date]

          Displays or sets the current date.

DEL  [/H] [/P] compound-filespec

          Deletes one or more files.

DIR  [/H] [/W] [/P] [compound-filespec]

          Displays the names of files on disk.

DISKCOPY [d: [d:]] [/X]

          Copies one disk to another.

ECHO [text]

          Prints text in a batch file.

ERA  [/H] [/P] compound-filespec

          Deletes one or more files.

ERASE [/H] [/P] compound-filespec

          Deletes one or more files.

EXIT [number]

          Exits COMMAND2.COM to the invoking program.

FIXDISK [d:] [/S]

          Updates a disk to the full MSX-DOS 2 format.

FORMAT [d:]

          Formats (initializes) a disk.

HELP [subject]

          Provides on-line help for an MSX-DOS feature.

MD [d:] path

          Creates a new sub-directory.

MKDIR [d:] path

          Creates a new sub-directory.

MODE number

          Changes the number of characters/line on the screen.

MOVE [/H] [/P] compound-filespec [path]

          Moves files from one place to another on a disk.

MVDIR [/H] [/P] compound-filespec [path]

          Moves directories from one place to another on a disk.

PATH [ [+|-] [d:]path [ [d:]path [ [d:]path ...]] ]

          Displays or sets the COM and BAT command search path.

PAUSE [comment]

          Prompts and waits for a key press in a batch file.

RAMDISK [number[K]] [/D]

          Displays or sets the RAM disk size.

RD [/H] [/P] compound-filespec

          Removes one or more sub-directories.

REM [comment]

          Introduces a comment in a batch file.

REN [/H] [/P] compound-filespec filename

          Renames one or more files.

RENAME [/H] [/P] compound-filespec filename

          Renames one or more files.

RMDIR [/H] [/P] compound-filespec

          Removes one or more sub-directories.

RNDIR [/H] [/P] compound-filespec filename

          Renames one or more sub-directories.

SET [name] [separator] [value]

          Displays or sets environment items.

TIME [time]

          Displays or sets the current time.

TYPE [/H] [/P] [/B] compound-filespec | device

          Displays data from a file or device.

UNDEL filespec

          Recovers a previously deleted file.

VER

          Displays the system's version numbers.

VERIFY [ON | OFF]

          Displays/sets the current disk write verify state.

VOL [d:] [volname]

          Displays or changes the volume name on a disk.

XCOPY [filespec [filespec]] [/A][/E][/H][/M][/P][/S][/T][/V][/W]

          Copies files and directories from one disk to another.

XDIR [filespec] [/H]

          Lists all files within directories.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-disk-basic-2-0", "", "MSX-DOS 2: manual", "", "DISK-BASIC 2.0 (MSX-DOS 2)",
        R"MAN(MSX-DOS 2 (versao 2.20) - Command Specification

10. DISK-BASIC 2.0
==================

Overview

     When  the   system  disk   (the  one   which  contains  MSXDOS2.SYS  and
COMMAND2.COM)  does not  exist at  the system  startup, or when MSX-DOS BASIC
command is executed, DISK-BASIC 2.0 will be started.

     DISK-BASIC  2.0  is  the  extended version  of previous  DISK-BASIC 1.0.
Instructions to  operate with the RAM disk or the directory has been added or
extended.

Description of Commands

CALL CHDIR

Format          CALL CHDIR("[d:][path]")

Purpose         Set or display current directory

Example         CALL CHDIR("WORK")

Description     The function is the same as MSX-DOS CHDIR. See CHDIR.

CALL CHDRV

Format          CALL CHDRV("[d:]")

Purpose         Switch default drive

Example         CALL CHDRV("H:")

Description     The  same action  is taken as when the drive name is given at
                MSX-DOS prompt.

CALL MKDIR

Format          CALL MKDIR("[d:][path]")

Purpose         Create new subdirectory

Example         CALL MKDIR("WORK")

Description     The function is the same as MSX-DOS MKDIR. See MKDIR.

CALL RMDIR

Format          CALL RMDIR("[d:][path]")

Purpose         Remove one or more subdirectories

Example         CALL RMDIR("WORK")

Description     The function is the same as MSX-DOS RMDIR. See RMDIR.

CALL RAMDISK

Format          CALL RAMDISK[([number][, variable name])]

Purpose         Set size of RAM disk or assign it into a variable

Example         CALL RAMDISK(32)
                CALL RAMDISK(1000, A)

Description     The function is the same as MSX-DOS RAMDISK. See RAMDISK.

CALL SYSTEM

Format          CALL SYSTEM
                or
                CALL SYSTEM[("DOS command name")]

Purpose         Pass control back to MSX-DOS

Example         CALL SYSTEM("WORK")

Description     CALL SYSTEM  passes the  control back to MSX-DOS. The command
                name  may be  given to  specify the  operation to be executed
                after  the  control returns  to DOS.  If no  command name  is
                specified,  REBOOT.BAT  in  the  root directory  on the  boot
                drive, if any, will be executed.

FILES

Format          FILES["filename"][, L]

Purpose         Display names of files and directories on disk

Example         FILES"W*.*"

Description     The function  is the same as MSX-DOS DIR. See DIR. "FILES, L"
                displays the names in the long format.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-program-interface", "", "MSX-DOS 2: manual", "", "MSX-DOS 2: especificacao da interface de programa (handles, fileinfo, ambiente)",
        R"MAN(MSX-DOS 2 - Program Interface Specification

                  M S X -- D O S   v e r s i o n  2
                  =================================

                  The advanced disk operating system

                         for MSX 2 computers

This document is not complete yet:

1.   INTRODUCTION
This  manual describes the environment which MSX-DOS 2 provides  for
transient programs on MSX 2 computers. It is intended as a guide for
writing  new  programs to run under MSX-DOS 2 and also to assist  in
converting existing CP/M and MSX-DOS 1 programs to take advantage of
the advanced features.

MSX-DOS  2 provides many enhancements to the standard CP/M and  MSX-
DOS 1 environment, but is largely compatible with existing programs.
The main features include:

        MS-DOS style tree structured directories
        File handles
        Environment strings
        Proper error handling

Many extra DOS calls are implemented, and these are accessed via the
DOS entry jump at address 5 (the 'BDOS' jump in CP/M).  The descrip-
tions  of the  individual functions  can be found  in the  "Function
Codes  specification".

----

5.   MAPPER SUPPORT ROUTINES

MSX-DOS  2  contains  routines to provide  support  for  the  memory
mapper.   This  allows MSX application programs or MSX-DOS transient
programs  to  utilize more than the basic  64k  of  memory,  without
conflicting with the RAM disk or any other system software.

  5.1   MAPPER INITIALISATION

   When  the  DOS kernel is initialized it checks that there is  the
   memory mapper in the system,  and that there is at least 128k  of
   RAM available.    If the kernel has found at least one slot which
   contains  128k  of  the mapper RAM,  it selects  the  slot  which
   contains the largest amount of RAM (or the slot with the smallest
   slot number, if there are two or more mapper slots which have the
   same amount of RAM) and makes that slot usable as the system RAM.
   When there is not enough memory on the memory mapper,  MSX-DOS  2
   won't start.

   Next  the  kernel builds up a table of all the 16k  RAM  segments
   available  to  this slot (primary mapper slot).   The first  four
   segments (64k) for the user and the two highest numbered segments
   are allocated to the system,  one for the DOS kernel code and one
   for the DOS kernel workspace.   All other segments (at least two)
   are  marked  as free initially.   Then the kernel builds  up  the
   similar  tables  for  other RAM slots,  if any.    All  of  these
   segments are marked as free initially.

  5.2   MAPPER VARIABLES AND ROUTINES

   The mapper support routines use some variables in the DOS  system
   area.  These tables may be referred and used by the user programs
   for the various purposes,  but must not be altered.  The contents
   of the tables are as follows:

     address                  function

     +0               Slot address of the mapper slot
     +1               Total  number  of 16k  RAM  segments.  1...255
                      (8...255 for the primary)
     +2               Number of free 16k RAM segments
     +3               Number  of  16k RAM segments allocated to  the
                      system (at least 6 of the primary)
     +4               Number of 16k RAM segments allocated to the user.
     +5...+7          Unused. Always zero.
     +8...            Entries  for other mapper slots.  If there  is
                      none, +8 will be zero.

   A  program  uses  the  mapper support  code  by  calling  various
   subroutines.   These  are accessed through a jump table which  is
   located in the MSX-DOS system area.    The consists of  the  jump
   table are as follows:

     address   entry name    function

     +0H       ALL_SEG     Allocate a 16k segment.
     +3H       FRE_SEG     Free a 16k segment.
     +6H       RD_SEG      Read byte from address A:HL to A.
     +9H       WR_SEG      Write byte from E to address A:HL.
     +CH       CAL_SEG     Inter-segment call.  Address in Iyh:IX
     +FH       CALLS       Inter-segment  call.   Address  in  line
                           after the call instruction.
     +12H      PUT_PH      Put segment into page (HL).
     +15H      GET_PH      Get current segment for page (HL)
     +18H      PUT_P0      Put segment into page 0.
     +1BH      GET_P0      Get current segment for page 0.
     +1EH      PUT_P1      Put segment into page 1.
     +21H      GET_P1      Get current segment for page 1.
     +24H      PUT_P2      Put segment into page 2.
     +27H      GET_P2      Get current segment for page 2.
     +2AH      PUT_P3      Not supported since page-3 must never be
                           changed.  Acts like a "NOP" if called.
     +2DH      GET_P3      Get current segment for page 3.

   A  program can use the extended BIOS calls for the mapper support
   to obtain these addresses.   The calls are provided because these
   addresses may be changed in the future version,  or to use mapper
   routines other than MSX-DOS mapper support routines.

   To use the extended BIOS,  the program should test "HOKVLD"  flag
   at FB20h in page-3.   If bit-0 (LSB) is 0,   there is no extended
   BIOS  nor  the mapper support.   Otherwise,  "EXTBIO" entry  (see
   below)  has  been  set  up and it  can  be  called  with  various
   parameters.    Note   that  this  test  is  unnecessary  for  the
   applications  which  are based on MSX-DOS (such  as  the  program
   which  is loaded from the disk),  and the program may proceed  to
   the next step.

   Next,  the program sets the device number of the selected BIOS in
   register  D,  the  function number in register  E,  and  required
   parameters  in other registers,  and then calls "EXTBIO" at FFCAh
   in page-3.   In this case,  the stack pointer must be in  page-3.
   If  there  is the extended BIOS for the specified device  number,
   the  contents  of  the  registers AF,  BC,  and  HL  are modified
   according  to  the  function;   otherwise,  they  are  preserved.
   Register  DE  is always preserved.   Note that in any  cases  the
   contents of the alternative registers (AF', BC', DE' and HL') and
   the index registers (IX and IY) are corrupted.

   The functions available in the mapper support extended BIOS are:

   * Get mapper variable table

      Parameter:      A = 0
                      D = 4 (device number of mapper support)
                      E = 1
      Result:         A = slot address of primary mapper
                      DE = reserved
                      HL = start address of mapper variable table

   * Get mapper support routine address

      Parameter:      A = 0
                      D = 4
                      E = 2
      Result:         A = total number of memory mapper segments
                      B = slot number of primary mapper
                      C = number of free segments in primary mapper
                     DE = reserved
                     HL = start address of jump table

   In these mapper support extended BIOS, register A is not required
   to be zero.   Note that,  however,  if there is no mapper support
   routine,  the contents of registers will not be modified, and the
   value which is not zero will be returned in A  otherwise.   Thus,
   the  existence of the mapper support routine can be determined by
   setting zero in A at the calling and examining the returned value
   of A.

   The  slot address of the primary mapper returned by the  extended
   BIOS is the same as the current RAM slot address in page-3,  and,
   in  the ordinary environment (DISK-BASIC and MSX-DOS),  the  same
   RAM slot is also selected in page-2.   In MSX-DOS,  this is  also
   true in page-0 and page-1.

  5.3   USING MAPPER ROUTINES

   A  program  can request a 16k RAM segment at any time by  calling
   the "ALL_SEG" routine.  This either returns an error if there are
   no  free segments,  or the segment number of a new segment  which
   the program can use.  A program must not use any segment which it
   has not explicitly allocated,  except for the four segments which
   make up the basic 64k of RAM.

   A  segment  can  be allocated either as a user segment  or  as  a
   system segment.   User segments will be automatically freed, when
   the program terminates,   whereas system segments are never freed
   unless  the program frees them  explicitly.   Normally,  programs
   should allocate user segments.

   RAM  segments  can  be  accessed by  the  "RD_SEG"  and  "WR_SEG"
   routines which read and write bytes to specified  segments.   The
   routines  "CAL_SEG"  and "CALLS" allow inter-segment calls to  be
   done in much the same way as inter-slot calls in the current  MSX
   system.

   Routines are provided to explicitly page a segment in, or to find
   out which segment is in a particular page.  There are routines in
   which  the  page (0...3) is specified by the top two bits  of  an
   address  in  HL  ("PUT_PH" and "GET_PH").   And  there  are  also
   specific   routines   for  accessing  each  page  ("GET_Pn"   and
   "PUT_Pn").   These routines are very fast so a program should not
   suffer in performance by using them.

   Note that page-3 should never be altered since this contains  the
   mapper support routines and all the other system variables.  Also
   great care must be taken if page-0 is altered since this contains
   the interrupt and the slot-switching entry-points.  Pages 1 and 2
   can be altered in any way.

   None  of  the  mapper  support routines  will  disturb  the  slot
   selection mechanism at all.  For example when "PUT_P1" is called,
   the   specified   RAM  segment  will  only  appear   at   address
   4000h...7FFFh  if  the mapper slot is selected  in  page-1.   The
   "RD_SEG" and "WR_SEG" routines will always access the RAM segment
   regardless  of the current slot selection in the specified  page,
   but the mapper RAM slot must be selected in page-2.

  5.4   ALLOCATING and FREEING SEGMENTS

   The  following  two  routines can be called to allocate  or  free
   segments.   All registers apart from AF and BC are preserved.  An
   error  is indicated by the carry flag being set on  return.   The
   slot  selection  and RAM paging may be in any  state  when  these
   routines  are called and both will be preserved.   The stack must
   not  be  in page-0 or page-2 when either of  these  routines  are
   called.

   A  program  must not use any segment (apart from the  four  which
   make  up the basic 64k) unless it has specifically allocated  it,
   and must not continue to use a segment after it has been freed.

   A  segment may be allocated either as a user or a system segment.
   The  only difference is that user segments will be  automatically
   freed  when the program terminates whereas system  segments  will
   not  be.   In  general a program should allocate a  user  segment
   unless  it  needs the data in the segment to outlast the  program
   itself.   User  segments  are always allocated  from  the  lowest
   numbered  free  segment  and  system segments  from  the  highest
   numbered one.

   An error from "allocate segment" usually indicates that there are
   no  free  segments,  although it can also mean  that  an  invalid
   parameter  was passed in register A and B.    An error from "free
   segment"  indicates  that the specified segment number  does  not
   exist or is already free.

   ALL_SEG - Parameters:   A=0 => allocate user segment
                           A=1 => allocate system segment
                           B=0 => allocate primary mapper
                          B!=0 => allocate
                      FxxxSSPP slot address (primary mapper, if 0)
                      xxx=000 allocate specified slot only
                      xxx=001 try to allocate specified slot and, if
                              it failed, try another slot (if any)
                      xxx=011 try   to  allocate  other  slots  than
                              specified  and,   if  it  failed,  try
                              specified slot
        Results:      Carry set => no free segments
                      Carry clear => segment allocated
                                     A=new segment number
                                     B=slot  address of mapper  slot
                                       (0 if called as B=0)

   FRE_SEG - Parameters:   A=segment number to free
                           B=0 primary mapper
                          B!=0 mapper other than primary
             Returns:      Carry set => error
                           Carry clear => segment freed OK

  5.5   INTER-SEGMENT READ AND WRITE

   The  following  two  routines can be called to read  or  write  a
   single byte from any mapper RAM segment.  The calling sequence is
   very  similar to the inter-slot read and write routines  provided
   by the MSX system ROM.  All registers apart from AF are preserved
   and  no  checking  is done to ensure that the segment  number  is
   valid.

   The top two bits of the address are ignored and the data will  be
   always  read  or  written via page-2,  since the  segment  number
   specifies  a  16k segment which could appear in any of  the  four
   pages.  The data will be read or written from the correct segment
   regardless  of the current paging or slot selection in page-0  or
   page-1,  but  note that the mapper RAM slot must be  selected  in
   page-2 when either of these routines are called.  This is so that
   the  routines do not have to do any slot-switching and so can  be
   fast.  Also the stack must not be in page-2.  These routines will
   return disabling interrupts.

   RD_SEG -  Parameters:   A = segment number to read from
                          HL = address within this segment
             Results:      A = value of byte at address
                           All other registers preserved

   WR_SEG -  Parameters:   A = segment number to write to
                          HL = address within this segment
             Returns:      A = corrupted
                           All other registers preserved

  5.6   INTER-SEGMENT CALLS

   Two  routines are provided for doing inter-segment calls.   These
   are  modelled  very closely on the two inter-slot  call  routines
   provided  by the MSX system ROM,  and the specification of  their
   usage is very similar.

   No check is done that the called segment actually exists so it is
   the  user's responsibility to ensure this.   The  called  segment
   will  be  paged into the specified address page,  but it  is  the
   user's  responsibility to ensure that the mapper slot is  enabled
   in this page, since neither of these routines will alter the slot
   selection at all.   This is to ensure that they can be fast.

   The routine cannot be used to do an inter-segment call into page-
   3.   If  this  is attempted then the specified address in  page-3
   will be simply called without any paging, since page-3 must never
   be  altered.   Calling  into page-0 must be done with  some  care
   because of the interrupt and other entry point.   Also care  must
   be taken that the stack is not paged out by these calls.

   These   routines,   unlike  inter-slot  calls,   do  not  disable
   interrupts before passing control to the called routine.  So they
   return  to  the caller in the same state as  before,  unless  the
   interrupt flag was modified by the called routine.

   Interrupts.

   Parameters cannot be passed in registers IX, IY, AF', BC', DE' or
   HL'  since  these  are used internally  in  the  routine.   These
   registers  will  be corrupted by the inter-segment call  and  may
   also  be corrupted by the called routine.   All  other  registers
   (AF,  BC,  DE and HL) will be passed intact to the called routine
   and returned from it to the caller.

   CAL_SEG - Parameters: IY = segment number to be called
                         IX = address to call
                         AF, BC, DE, HL passed to called routine
                         Other registers corrupted
             Results:    AF,  BC,  DE, HL, IX and IY returned from
                         called routine.  All others corrupted.

   CALLS -   Parameters:  AF, BC, DE, HL passed to called routine
                          Other registers corrupted
             Calling sequence:   CALL  CALLS
                                 DB    SEGMENT
                                 DW    ADDRESS
             Results:     AF,  BC,  DE, HL, IX and IY returned from
                          called routine.  All others corrupted.

  5.7   DIRECT PAGING ROUTINES

   The following routines are provided to allow programs to directly
   manipulate  the current paging state without having to access the
   hardware.   Using  these routines ensures compatibility with  any
   changes  to the details of the hardware.   The routines are  very
   fast  and  so using them will not compromise the  performance  of
   programs.

   Routines  are  provided to directly read or write to any  of  the
   four page registers.   No checking of the validity of the segment
   number is done so this is the user's responsibility.   Note  that
   the  value written in the register is also written in memory and,
   if  the register value is requested,  the value stored in  memory
   will  be returned and the one in the register will never be  read
   directly.   This is done to avoid errors from hardware  conflicts
   when  there are two or more mapper registers in the system.   The
   user  should  always manipulate the memory mapper  through  these
   routines.

   The  "GET"  routines return values from internal  images  of  the
   registers  without  actually  reading the  registers  themselves.
   This  ensures  that  if a segment is  enabled  by,  for  example,
   "PUT_P1"  then a subsequent "GET_P1" call will return the  actual
   value.  Reading the mapper register may produce a different value
   because  the  top bits of the segment numbers are  generally  not
   recorded.

   Although  a "PUT_P3" routine is provided,  it is in fact a  dummy
   routine and will not alter the page-3 register.   This is because
   the contents of the page-3 register should never be altered.  The
   "GET_P3"  routine  does behave as expected to allow the  user  to
   determin what segment is in page-3.

   Another  pair  of routines ("GET_PH" and  "PUT_PH")  is  provided
   which are identical in function except that the page is specified
   by the top two bits of register H.   This is useful when register
   HL  contains  an  address,  and  these routines  do  not  corrupt
   register HL.  "PUT_PH" will never alter the page-3 register.

   PUT_Pn -  Parameters:   n = 0,1,2 or 3 to select page
                           A = segment number
             Results:      None
                           All registers preserved

   GET_Pn -  Parameters:   n = 0,1,2 or 3 to select page
             Results:      A = segment number
                           All other registers preserved

   PUT_PH -  Parameters:   H = high byte of address
                           A = segment number
             Results:      None
                           All registers preserved

   GET_PH -  Parameters:   H = high byte of address
             Results:      A = segment number
                           All other registers preserved

   Before  using  these direct paging routines to alter  the  paging
   state,  a  program  should  first use the  "GET_Pn"  routines  to
   determine the initial four segments for when it needs to  restore
   these.   No  program should assume fixed values for these initial
   segments  since they are likely to change in future  versions  of
   the systems.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-functions", "", "MSX-DOS 2: manual", "", "Chamadas de funcao do MSX-DOS 2: introducao e lista",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

                  M S X -- D O S   v e r s i o n  2
                  =================================

                  The advanced disk operating system

                         for MSX 2 computers

CONTENTS                                       Page

1.    Introduction ............................  3
2.    List of functions .......................  4
3.    Function By Function Definitions ........  7
   3.1   \
   . .    \  Definitions of each function
   . .    /
   3.90  /

   This  manual describes  in detail  the MSX-DOS  function calls provided by
MSX-DOS version 2.20

1.   INTRODUCTION

   This document describes in detail each of the MSX-DOS 2 function calls. It
should  be read  in conjunction  with the  "Program Interface  specification"
which describes  system features  such as  file handles,  fileinfo blocks and
environment strings in general terms.

   There are  two ways  of doing  MSX-DOS function  calls, reflecting the two
different  environments (MSX-DOS and disk BASIC) in which the system can run.
Transient  programs  running  in  the  MSX-DOS  environment  must  access the
functions with a "CALL 00005h" instruction. Disk BASIC and other MSX programs
running  in  the disk  BASIC environment  (usually executing  from ROM)  must
access the system via a "CALL 0F37Dh" instruction.

   There   are  some   limitations  when   calling  the  system  via  0F37Dh,
particularly to do with error handling and abort routines. Also no parameters
may be passed in page-1, unless they are in the master disk ROM (as they will
be for  disk BASIC)  since the master disk ROM will be paged into page-1 when
such  a function  call is  made. The individual function descriptions mention
the differences for particular functions.

2.   LIST OF FUNCTIONS

   Below there  is a  complete list  of the  functions calls. "CPM" indicates
that the function is compatible with the equivalent CP/M 2.2 function, "MSX1"
indicates  compatibility  with  MSX-DOS  version  1,  and  "NEW" indicates  a
function  which is  new to  this system. An asterisk ("*") indicates that the
function may  be safely  called from  a user disk error routine (see function
64h and function 70h).

List of MSX-DOS 2 Function Calls

CPM MSX1  00h -  Program terminate
     CPM MSX1* 01h -  Console input
     CPM MSX1* 02h -  Console output
     CPM MSX1* 03h -  Auxiliary input
     CPM MSX1* 04h -  Auxiliary output
     CPM MSX1* 05h -  Printer output
     CPM MSX1* 06h -  Direct console I/O
         MSX1* 07h -  Direct console input
         MSX1* 08h -  Console input without echo
     CPM MSX1* 09h -  String output
     CPM MSX1* 0Ah -  Buffered line input
     CPM MSX1* 0Bh -  Console status

     CPM MSX1* 0Ch -  Return version number
     CPM MSX1  0Dh -  Disk reset
     CPM MSX1  0Eh -  Select disk

     CPM MSX1  0Fh -  Open file (FCB)
     CPM MSX1  10h -  Close file (FCB)
     CPM MSX1  11h -  Search for first entry (FCB)
     CPM MSX1  12h -  Search for next entry (FCB)
     CPM MSX1  13h -  Delete file (FCB)
     CPM MSX1  14h -  Sequential read (FCB)
     CPM MSX1  15h -  Sequential write FCB)
     CPM MSX1  16h -  Create file (FCB)
     CPM MSX1  17h -  Rename file (FCB)

     CPM MSX1* 18h -  Get login vector
     CPM MSX1* 19h -  Get current drive
     CPM MSX1  1Ah -  Set disk transfer address
         MSX1  1Bh -  Get allocation information
               1Ch -  Unused
               1Dh -  Unused
               1Eh -  Unused
               1Fh -  Unused
               20h -  Unused
     CPM MSX1  21h -  Random read (FCB)
     CPM MSX1  22h -  Random write(FCB)
     CPM MSX1  23h -  Get file size (FCB)
     CPM MSX1  24h -  Set random record (FCB)
               25h -  Unused
         MSX1  26h -  Random block write (FCB)
         MSX1  27h -  Random block read (FCB)
     CPM MSX1  28h -  Random write with zero fill (FCB)
               29h -  Unused
         MSX1* 2Ah -  Get date
         MSX1* 2Bh -  Set date
         MSX1* 2Ch -  Get time
         MSX1* 2Dh -  Set time
         MSX1* 2Eh -  Set/reset verify flag
         MSX1* 2Fh -  Absolute sector read
         MSX1* 30h -  Absolute sector write
         NEW*  31h -  Get disk parameters

            32h - \
            . .    \ Unused
            . .    /
            3Fh - /

    NEW     40h -  Find first entry
    NEW     41h -  Find next entry
    NEW     42h -  Find new entry

    NEW     43h -  Open file handle
    NEW     44h -  Create file handle
    NEW     45h -  Close file handle
    NEW     46h -  Ensure file handle
    NEW     47h -  Duplicate file handle
    NEW     48h -  Read from file handle
    NEW     49h -  Write to file handle
    NEW     4Ah -  Move file handle pointer
    NEW     4Bh -  I/O control for devices
    NEW     4Ch -  Test file handle

    NEW     4Dh -  Delete file or subdirectory
    NEW     4Eh -  Rename file or subdirectory
    NEW     4Fh -  Move file or subdirectory
    NEW     50h -  Get/set file attributes
    NEW     51h -  Get/set file date and time

    NEW     52h -  Delete file handle
    NEW     53h -  Rename file handle
    NEW     54h -  Move file handle
    NEW     55h -  Get/set file handle attributes
    NEW     56h -  Get/set file handle date and time

    NEW   * 57h -  Get disk transfer address
    NEW   * 58h -  Get verify flag setting
    NEW     59h -  Get current directory
    NEW     5Ah -  Change current directory
    NEW     5Bh -  Parse pathname
    NEW     5Ch -  Parse filename
    NEW   * 5Dh -  Check character
    NEW     5Eh -  Get whole path string
    NEW     5Fh -  Flush disk buffers

    NEW     60h -  Fork a child process
    NEW     61h -  Rejoin parent process
    NEW     62h -  Terminate with error code
    NEW     63h -  Define abort exit routine
    NEW     64h -  Define disk error handler routine
    NEW   * 65h -  Get previous error code
    NEW   * 66h -  Explain error code

    NEW     67h -  Format a disk
    NEW     68h -  Create or destroy RAM disk
    NEW     69h -  Allocate sector buffers
    NEW   * 6Ah -  Logical drive assignment

    NEW   * 6Bh -  Get environment item
    NEW   * 6Ch -  Set environment item
    NEW   * 6Dh -  Find environment item

    NEW   * 6Eh -  Get/set disk check status
    NEW   * 6Fh -  Get MSX-DOS version number
    NEW   * 70h -  Get/set redirection status

3.   FUNCTION BY FUNCTION DEFINITIONS

   Below are detailed descriptions of each of the MSX-DOS functions including
both the  old and  new ones. The names in brackets after the function numbers
are   the  public  labels  for  the  function  codes  which  are  defined  in
"CODES.MAC". Programs should use these names whenever possible.

   Many of  the functions below 40h return an error flag rather than an error
code.  If the  error flag  is set  then the  actual error code indicating the
cause of  the error can be obtained by the "get previous error code" function
(function  65h).  All  of the  functions above  40h return  an error  code in
register  A.  The  "Program Interface  Specification" document  describes the
general  errors  which  can  be  returned  from  many of  the functions.  The
individual  function specifications  here describe  the main error conditions
which are specific to particular functions.
   Note that  many of  the function  calls which  modify the information on a
disk  do  not  automatically  flush  disk  buffers  and  so  the disk  is not
necessarily  correctly updated  immediately after  the function call is made.
Such  calls  include  all  types  of  "create", "write",  "delete", "rename",
"change file  attributes" and "change file date and time" function calls. The
only  functions which  always flush disk buffers are "flush buffers", "close"
and  "ensure".  After  these  operations the  disk will  always be  correctly
updated.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-00h", "_term0,dos2-fn-program-terminate", "MSX-DOS 2: funcoes", "C = 00H  (_TERM0)", "Funcao 00H (_TERM0): Program Terminate",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.1   PROGRAM TERMINATE (00H)

     Parameters:    C = 00H (_TERM0)
     Results:       Does not return

   This function  terminates program  with a zero return code. It is provided
for  compatibility with MSX-DOS 1 and CP/M, the preferred method of exiting a
program is  to use  the "terminate  with error  code" function call (function
62h),  passing  a  zero  error  code  if  that is  what is  desired. See  the
description   of  that   function  call,  and  also  the  "Program  Interface
Specification", for  details of  what happens when a program terminates. This
function call never returns to the caller.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-01h", "_conin,dos2-fn-console-input", "MSX-DOS 2: funcoes", "C = 01H  (_CONIN)", "Funcao 01H (_CONIN): Console Input",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.2   CONSOLE INPUT (01H)

    Parameters:    C = 01H (_CONIN)
    Results:     L=A = Character from keyboard

   A  character will be read from the standard input (file handle 0 - usually
the keyboard)  and echoed to the standard output (file handle 1 - usually the
screen).  If no  character is  ready then  this function  will wait  for one.
Various control  characters, as  specified for  the "console status" function
(function  0Bh),  will  be  trapped  by  this  function  for  various control
purposes.  If one  of these  characters is detected then it will be processed
and this function will wait for another character. Thus these characters will
never be returned to the user by this function.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-02h", "_conout,dos2-fn-console-output", "MSX-DOS 2: funcoes", "C = 02H  (_CONOUT)", "Funcao 02H (_CONOUT): Console Output",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.3   CONSOLE OUTPUT (02H)

     Parameters:    C = 02H (_CONOUT)
                    E = Character to be output
     Results:       None

   The character passed in register E is written to the standard output (file
handle 1 - usually the screen). If printer echo is enabled then the character
is also  written to  the printer.  Various control codes and escape sequences
are  interpreted as  screen control codes. A list of these is included in the
"Program Interface  Specification", they  are a sub-set of the standard VT-52
control codes. TABs will be expanded to every eighth column.

   A  console input  status check  is done, and if any of the special control
characters  described  for the  "console status"  function (function  0Bh) is
found then  it will  be processed  as for  that function. Any other character
will be saved internally for a later "console input" function call.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-03h", "_auxin,dos2-fn-auxiliary-input", "MSX-DOS 2: funcoes", "C = 03H  (_AUXIN)", "Funcao 03H (_AUXIN): Auxiliary Input",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.4   AUXILIARY INPUT (03H)

     Parameters:    C = 03H (_AUXIN)
     Results:     L=A = Input character

   A character is read from the auxiliary input device (file handle 3) and if
no character  is ready  then it will wait for one. The auxiliary input device
must  have been installed before this function may be used. If no such device
has been  installed then  this function  will always  return the  end of file
character ("Ctrl-Z").

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-04h", "_auxout,dos2-fn-auxiliary-output", "MSX-DOS 2: funcoes", "C = 04H  (_AUXOUT)", "Funcao 04H (_AUXOUT): Auxiliary Output",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.5   AUXILIARY OUTPUT (04H)

     Parameters:    C = 04H (_AUXOUT)
                   E = Character to be output
     Results:       None

   The character passed in register E will be written to the auxiliary output
device (file  handle 3). The auxiliary output device must have been installed
before  this function  may be used. If no such device has been installed then
this function will simply throw the character away.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-05h", "_lstout,dos2-fn-printer-output", "MSX-DOS 2: funcoes", "C = 05H  (_LSTOUT)", "Funcao 05H (_LSTOUT): Printer Output",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.6   PRINTER OUTPUT (05H)

     Parameters:    C = 05H (_LSTOUT)
                   E = Character to be output
      Results:       None

   The character  passed in  register E  will be sent to the standard printer
device  (file handle  4 -  usually the parallel printer). The same channel is
used for console output which is echoed to the printer. TABs are not expanded
by this  function, although they are expanded when screen output is echoed to
the printer with "Ctrl-P".

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-06h", "_dirio,dos2-fn-direct-console-i-o", "MSX-DOS 2: funcoes", "C = 06H  (_DIRIO)", "Funcao 06H (_DIRIO): Direct Console I/O",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.7   DIRECT CONSOLE I/O (06H)

     Parameters:    C = 06H (_DIRIO)
                    E = 00H...FEH - character for output
                      = FFH  - requests input
     Results:     A=L = input - 00H - no character ready
                                else input character
                       undefined for output

   If  E=FFh on  entry then the keyboard will be examined for a characterfrom
the standard input (file handle 0) and 00h returned if no character is ready.
If a  character is  ready then  it will be read from the standard input (file
handle  0) and  returned in register A without being echoed and with no check
for control characters.

   If  E<>FFh  on entry  then the  character in  register E  will be  printed
directly to  the standard  output (file  handle 1)  with no  TAB expansion or
printer  echo. Also  no console  status check  is done by this function. Note
that although  this function  does not  expand TABs,  the VT-52 control codes
include TAB expansion so the effect on the screen is the same.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-07h", "_dirin,dos2-fn-direct-console-input", "MSX-DOS 2: funcoes", "C = 07H  (_DIRIN)", "Funcao 07H (_DIRIN): Direct Console Input",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.8   DIRECT CONSOLE INPUT (07H)

     Parameters:    C = 07H (_DIRIN)
     Results:     L=A = Input character

   This function  is identical  to the  input option  of function 06h, except
that  if no  character is  ready it  will wait for one. Like function 06h, no
echo  or  control  characters  checks  will  be  done.  This function  is not
compatible with CP/M which uses this function number for "get I/O byte".

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-08h", "_innoe,dos2-fn-console-input-without-echo", "MSX-DOS 2: funcoes", "C = 08H  (_INNOE)", "Funcao 08H (_INNOE): Console Input Without Echo",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.9   CONSOLE INPUT WITHOUT ECHO (08H)

     Parameters:    C = 08H (_INNOE)
     Results:     L=A = Input character

   This function is identical to function 01h except that the input character
will  not be echoed to the standard output. The same control character checks
will be  done. This  function is  not compatible  with CP/M  which uses  this
function number for "set I/O byte".

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-09h", "_strout,dos2-fn-string-output", "MSX-DOS 2: funcoes", "C = 09H  (_STROUT)", "Funcao 09H (_STROUT): String Output",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.10   STRING OUTPUT (09H)

     Parameters:    C = 09H (_STROUT)
                  DE = Address of string
     Results:       None

   The characters  of the  string pointed  to by  register DE  will be output
using  the normal  console output  routine (function call 02h). The string is
terminated by "$" (ASCII 24h).

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-0ah", "_bufin,dos2-fn-buffered-line-input", "MSX-DOS 2: funcoes", "C = 0AH  (_BUFIN)", "Funcao 0AH (_BUFIN): Buffered Line Input",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.11   BUFFERED LINE INPUT (0AH)

     Parameters:    C = 0AH (_BUFIN)
                   DE = Address of an input buffer
     Results:       None

   DE  must point  to a  buffer to  be used for input. The first byte of this
buffer  must  contain  the  number of  characters which  the buffer  can hold
(0...255). A  line of input will be read from the standard input device (file
handle  0 - usually the keyboard) and stored in the buffer. The input will be
terminated  when  a  CR  is  read  from  the  standard input.  The number  of
characters entered,  which does  not include the CR itself, will be stored at
(DE+1).  If there  is room in the buffer then the CR will be stored after the
last character.

   When inputting  from the  keyboard (which  will normally  be the  case), a
simple  line  editor  is provided,  and also  a 256  byte circular  buffer of
previous  lines which  can be  edited and  re-entered. The  details of  these
editing  facilities  are described  in the  separate "Command  Specification"
document, so  they are not included here. When the input buffer becomes full,
the console bell will be rung for each character typed which cannot be put in
the  buffer. Each character entered will be echoed to the standard output and
also to the printer if printer echo is enabled.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-0bh", "_const,dos2-fn-console-status", "MSX-DOS 2: funcoes", "C = 0BH  (_CONST)", "Funcao 0BH (_CONST): Console Status",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.12   CONSOLE STATUS (0BH)

     Parameters:    C = 0BH (_CONST)
     Results:     L=A = 00H if no key was pressed
                      = FFH if a key was pressed

   A  flag is returned in register A to indicate whether a character is ready
(that is,  a key  was pressed) for input from the keyboard. If a character is
ready then it will be read and tested for certain special control characters.
If it is not one of these then it is stored in an internal single byte buffer
and   subsequent  call   to  this  function  will  return  "character  ready"
immediately  without  checking the  keyboard. If  this function  says that  a
character is ready then the character may be read by function 02h or 08h.

   If the  character is  "Ctrl-C" then  the program will be terminated with a
".CTRLC"  error  via  the  user's  abort routine  if one  is defined.  If the
character  is  "Ctrl-P"  then printer  echo will  be enabled  and it  will be
disabled  if it  is "Ctrl-N".  If the  character is "Ctrl-S" then the routine
will hang  up waiting for another character to be pressed and then return "no
character  ready", thus  providing a  "wait" facility. The character typed to
continue operation  will be  ignored, except  that of it is "Ctrl-C" then the
program  will  be  terminated.  These  same input  checks are  also done  for
functions 01h, 02h, 08h, 09h and 0Ah.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-0ch", "_cpmver,dos2-fn-return-version-number", "MSX-DOS 2: funcoes", "C = 0CH  (_CPMVER)", "Funcao 0CH (_CPMVER): Return Version Number",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.13   RETURN VERSION NUMBER (0CH)

     Parameters:    C = 0CH (_CPMVER)
     Results:     L=A = 22H
                 H=B = 00H

   This  function  simply  returns  the  CP/M version  number which  is being
emulated. This is always version 2.2 in current systems.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-0dh", "_dskrst,dos2-fn-disk-reset", "MSX-DOS 2: funcoes", "C = 0DH  (_DSKRST)", "Funcao 0DH (_DSKRST): Disk Reset",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.14   DISK RESET (0DH)

     Parameters:    C = 0DH (_DSKRST)
     Results:       None

   Any  data which  is waiting in internal buffers is written out to disk. It
is not  necessary to call this function in order to allow a disk change as is
the case with CP/M. The disk transfer address is also set back to its default
value of 80h by this function.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-0eh", "_seldsk,dos2-fn-select-disk", "MSX-DOS 2: funcoes", "C = 0EH  (_SELDSK)", "Funcao 0EH (_SELDSK): Select Disk",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.15   SELECT DISK (0EH)

     Parameters:    C = 0EH (_SELDSK)
                    E = Drive number.  0=A:  1=B:   etc.
     Results:     L=A = Number of drives (1...8)

   This function simply selects the specified drive as the default drive. The
current  drive is  also stored  at address  0004h for CP/M compatibility. The
number  of  drives available  is returned  in register  A but  this will  not
include the RAM disk.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-0fh", "_fopen,dos2-fn-open-file-fcb", "MSX-DOS 2: funcoes", "C = 0FH  (_FOPEN)", "Funcao 0FH (_FOPEN): Open File [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.16   OPEN FILE [FCB] (0FH)

     Parameters:    C = 0FH (_FOPEN)
                   DE = Pointer to unopened FCB
     Results:     L=A = 0FFH if file not found
                           =   0  if file is found

   The unopened  FCB must  contain a  drive which may be zero to indicate the
current  drive  and  a filename  and extension  which may  be ambiguous.  The
current directory of the specified drive will be searched for a matching file
and if found it will be opened. Matching entries which are sub-directories or
system files will be ignored, and if the filename is ambiguous then the first
suitable matching entry will be opened.

   Device  names may  be put in the FCB (without a colon) to allow devices to
be accessed  as if  they were  actually disk files. The standard device names
are defined in the "Program Interface Specification".

   The  low byte  of the extent number is not altered by this function, and a
file will only be opened if it is big enough to contain the specified extent.
Normally the  transient program  will set  the extent  number to  zero before
calling this function. The high byte of the extent number will be set to zero
to ensure compatibility with CP/M.

   The filename  and extension in the FCB will be replaced by the actual name
of  the file  opened from the directory entry. This will normally be the same
as what was there before but may be different if an ambiguous filename or one
with lower case letters in was used.

   The  record count  will be  set to  the number  of 128 byte records in the
specified extent, which is calculated from the file size. The file size field
itself, the  volume-id and  the 8  reserved bytes  will also  be set  up. The
current record and random record fields will not be altered by this function,
it  is the  application program's  responsibility to  initialize them  before
using the read or write functions.

   If  the file  cannot be  found, then the "APPEND" environment item will be
examined. If  this is set then it is interpreted as a drive/path string which
specifies  a second  directory in  which to  look for the file. The specified
directory will  be searched  for the  file and  if found it will be opened as
above.  In this  case the  drive byte  of the FCB will be set to the drive on
which the  file was  found to  ensure correct  accessing of  the file  if the
original drive byte was zero (default).

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-10h", "_fclose,dos2-fn-close-file-fcb", "MSX-DOS 2: funcoes", "C = 10H  (_FCLOSE)", "Funcao 10H (_FCLOSE): Close File [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.17   CLOSE FILE [FCB] (10H)

     Parameters:    C = 10H (_FCLOSE)
                   DE = Pointer to opened FCB
     Results:     L=A = 0FFH if not successful
                     =   0  if successful

   The  FCB must  have previously been opened with either an OPEN or a CREATE
function  call.  If  the  file has  only been  read then  this function  does
nothing. If  the file  has been  written to  then any  buffered data  will be
written  to disk  and the directory entry updated appropriately. The file may
still be  accessed after a close, so the function can be regarded as doing an
"ensure" function.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-11h", "_sfirst,dos2-fn-search-for-first-fcb", "MSX-DOS 2: funcoes", "C = 11H  (_SFIRST)", "Funcao 11H (_SFIRST): Search For First [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.18   SEARCH FOR FIRST [FCB] (11H)

     Parameters:    C = 11H (_SFIRST)
                   DE = Pointer to unopened FCB
     Results:     L=A = 0FFH if file not found
                     =   0  if file found.

   This function searches the current directory of the drive specified in the
FCB  (default  drive  if  FCB contains  zero) for  a file  which matches  the
filename and  extension in the FCB. The filename may be ambiguous (containing
"?"  characters) in which case the first match will be found. The low byte of
the extent  field will  be used,  and a  file will only be found if it is big
enough  to contain  this extent number. Normally the extent field will be set
to  zero  by  the  program  before  calling  this  function. System  file and
sub-directory entries will not be found.

   If a suitable match is found (A=0) then the directory entry will be copied
to the  DTA address,  preceded by the drive number. This can be used directly
as an FCB for an OPEN function call if desired. The extent number will be set
to  the low byte of the extent from the search FCB, and the record count will
be initialized  appropriately (as  for OPEN).  The attributes  byte from  the
directory  entry will  be stored  in the  S1 byte  position, since its normal
position (immediately  after the  filename extension  field) is  used for the
extent byte.

   If no match is found (A=0FFh) then the DTA will not be altered. In no case
will  the FCB  pointed to  by DE  be modified at all. This function remembers
sufficient information  internally to  allow it to continue the search with a
SEARCH  FOR NEXT function, so it is not necessary for the FCB to be preserved
if doing a SEARCH FOR NEXT function.

   In CP/M,  if the  drive number  is set  to "?"  in this  function then all
directory  entries, allocated  or free  will be  matched. Also  if the extent
field is  set to "?" then any extent of a file will be matched. Both of these
features  are normally  only used  by special purpose CP/M programs which are
generally  specific  to  the  CP/M filing  system (such  as "STAT").  Neither
feature is present in MSX-DOS 1/2.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-12h", "_snext,dos2-fn-search-for-next-fcb", "MSX-DOS 2: funcoes", "C = 12H  (_SNEXT)", "Funcao 12H (_SNEXT): Search For Next [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.19   SEARCH FOR NEXT [FCB] (12H)

     Parameters:    C = 12H (_SNEXT)
     Results:     L=A = 0FFH if file not found
                     =   0  if file found.

   It continues  the search to look for the next match with the filename. The
results returned from this function are identical to SEARCH FOR FIRST and all
the  same comments apply. The information used to continue the search is held
internally within  MSX-DOS and  so the  original FCB  used in  the SEARCH FOR
FIRST need not still exist.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-13h", "_fdel,dos2-fn-delete-file-fcb", "MSX-DOS 2: funcoes", "C = 13H  (_FDEL)", "Funcao 13H (_FDEL): Delete File [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.20   DELETE FILE [FCB] (13H)

     Parameters:    C = 13H (_FDEL)
                  DE = Pointer to unopened FCB
     Results:     L=A = 0FFH if no files deleted
                      =   0  if files deleted OK

   All  files in  the current directory of the disk specified by the FCB, and
which match  the ambiguous filename in the FCB, are deleted. Sub-directories,
system  files, hidden files and read only files are not deleted. If any files
at all are successfully deleted then this function returns with A=0. A return
with A=FFh indicates that no files were deleted.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-14h", "_rdseq,dos2-fn-sequential-read-fcb", "MSX-DOS 2: funcoes", "C = 14H  (_RDSEQ)", "Funcao 14H (_RDSEQ): Sequential Read [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.21   SEQUENTIAL READ [FCB] (14H)

     Parameters:    C = 14H (_RDSEQ)
                   DE = Pointer to opened FCB
     Results:     L=A = 01H if error (end of file)
                      =  0  if read was successful

   This function reads the next sequential 128 byte record from the file into
the  current  disk transfer  address. The  record is  defined by  the current
extent  (high  and  low  bytes) and  the current  record. After  successfully
reading the  record, this  function increments  the current  record and if it
reaches  080h, sets  it back  to zero  and increments  the extent number. The
record count field is also kept updated when necessary.

   Unlike  CP/M it  is possible  to have  partially filled records, since the
file size is not necessarily a multiple of 128 bytes. If this occurs then the
partial record  is padded  out with zeroes when it is copied to the transient
program's DTA address.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-15h", "_wrseq,dos2-fn-sequential-write-fcb", "MSX-DOS 2: funcoes", "C = 15H  (_WRSEQ)", "Funcao 15H (_WRSEQ): Sequential Write [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.22   SEQUENTIAL WRITE [FCB] (15H)

     Parameters:    C = 15H (_WRSEQ)
                   DE = Pointer to opened FCB
     Results:     L=A = 01H if error (disk full)
                     =  0  if write was successful

   This function  writes the 128 bytes from the current disk transfer address
to  the file  at the position defined by the current record and extent, which
are then  incremented appropriately.  The record  count byte  is kept updated
correctly  if the  file is  extended or if the write moves into a new extent.
The file size in the FCB is also updated if the file is extended.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-16h", "_fmake,dos2-fn-create-file-fcb", "MSX-DOS 2: funcoes", "C = 16H  (_FMAKE)", "Funcao 16H (_FMAKE): Create File [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.23   CREATE FILE [FCB] (16H)

     Parameters:    C = 16H (_FMAKE)
                   DE = Pointer to unopened FCB
     Results:     L=A = 0FFH if unsuccessful
                      =   0  if successful

   This function creates a new file in the current directory of the specified
drive and opens it ready for reading and writing. The drive, filename and low
byte of the extent number must be set up in the FCB and the filename must not
be  ambiguous. Checks  will be  done to ensure that invalid filenames are not
created.

   If there is already a file of the required name then the action depends on
the value  of the  extent number byte. Normally this will be zero and in this
case  the old  file will  be deleted  and a  new one  created. However if the
extent  number  is non-zero  then the  existing file  will be  opened without
creating a  new file.  This ensures compatibility with early versions of CP/M
where each extent had to be explicitly created.

   In all  cases the  resulting file  will be opened with the required extent
number exactly as if an OPEN function call had been done.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-17h", "_fren,dos2-fn-rename-file-fcb", "MSX-DOS 2: funcoes", "C = 17H  (_FREN)", "Funcao 17H (_FREN): Rename File [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.24   RENAME FILE [FCB] (17H)

     Parameters:    C = 17H (_FREN)
                   DE = Pointer to unopened FCB
     Results:     L=A = 0FFH not if successful
                     =   0  if successful

   The  unopened FCB  has the  normal drive  and filename,  and also a second
filename starting  at (DE+17).  Every file  in the  current directory  of the
specified  drive which  matches the  first filename, is changed to the second
filename with  "?" characters  in the second filename leaving the appropriate
character  unchanged.  Checks  are  done  to  prevent  duplicate  or  illegal
filenames  from being  created. Entries for sub-directories, hidden files and
system files will not be renamed.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-18h", "_login,dos2-fn-get-login-vector", "MSX-DOS 2: funcoes", "C = 18H  (_LOGIN)", "Funcao 18H (_LOGIN): Get Login Vector",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.25   GET LOGIN VECTOR (18H)

     Parameters:    C = 18H (_LOGIN)
     Results:      HL = Login vector

   This function  returns a  bit set in HL for each drive which is available,
bit-0 of L corresponding to drive "A:". Up to eight drives ("A:" to "H:") are
supported  by the  system currently,  so register  H will  usually be zero on
return.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-19h", "_curdrv,dos2-fn-get-current-drive", "MSX-DOS 2: funcoes", "C = 19H  (_CURDRV)", "Funcao 19H (_CURDRV): Get Current Drive",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.26   GET CURRENT DRIVE (19H)

     Parameters:    C = 19H (_CURDRV)
     Results:     L=A = Current drive (0=A: etc)

   This function just returns the current drive number.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-1ah", "_setdta,dos2-fn-set-disk-transfer-address", "MSX-DOS 2: funcoes", "C = 1AH  (_SETDTA)", "Funcao 1AH (_SETDTA): Set Disk Transfer Address",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.27   SET DISK TRANSFER ADDRESS (1AH)

     Parameters:    C = 1AH (_SETDTA)
                   DE = Required Disk Transfer Address
     Results:       None

   This function simply records the address passed in DE as the disk transfer
address.  This address  will be  used for  all subsequent  FCB read and write
calls,  for  "search for  first" and  "search for  next" calls  to store  the
directory entry, and for absolute read and write calls. It is not used by the
new MSX-DOS  read and  write functions.  The address  is set back to 80h by a
DISK RESET call.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-1bh", "_alloc,dos2-fn-get-allocation-information", "MSX-DOS 2: funcoes", "C = 1BH  (_ALLOC)", "Funcao 1BH (_ALLOC): Get Allocation Information",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.28   GET ALLOCATION INFORMATION (1BH)

     Parameters:    C = 1BH (_ALLOC)
                    E = Drive number (0=current, 1=A: etc)
     Results:       A = Sectors per cluster
                   BC = Sector size (always 512)
                   DE = Total clusters on disk
                   HL = Free clusters on disk
                   IX = Pointer to DPB
                   IY = Pointer to first FAT sector

   This function  returns various information about the disk in the specified
drive.  It is  not compatible  with CP/M  which uses  this function number to
return the  address of an allocation vector. Note that unlike MSX-DOS 1, only
the  first sector  of the FAT may be accessed from the address in IY, and the
data there will only remain valid until the next MSX-DOS call.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-21h", "_rdrnd,dos2-fn-random-read-fcb", "MSX-DOS 2: funcoes", "C = 21H  (_RDRND)", "Funcao 21H (_RDRND): Random Read [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.29   RANDOM READ [FCB] (21H)

     Parameters:    C = 21H (_RDRND)
                   DE = Pointer to opened FCB
     Results:     L=A = 01H if error (end of file)
                           0  if read was successful

   This function  reads a  128 byte  record from the file to the current disk
transfer  address. The  file position  is defined  by the  three byte  random
record number  in the  FCB (bytes  21h...23h). Unlike CP/M all three bytes of
the  random record  number are  used. A partial record at the end of the file
will be padded with zeroes before being copied to the user's DTA.

   The  random  record  number is  not altered  so successive  calls to  this
function  will read  the same  record unless the transient program alters the
random record number. A side effect is that the current record and extent are
set up  to refer  to the  same record as the random record number. This means
that  sequential reads  (or writes)  can follow  a random read and will start
from the  same record. The record count byte is also set up correctly for the
extent.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-22h", "_wrrnd,dos2-fn-random-write-fcb", "MSX-DOS 2: funcoes", "C = 22H  (_WRRND)", "Funcao 22H (_WRRND): Random Write [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.30   RANDOM WRITE [FCB] (22H)

     Parameters:    C = 22H (_WRRND)
                   DE = Pointer to opened FCB
     Results:     L=A = 01H if error (disk full)
                        =  0  if no error

   This  function writes  a 128  byte record  from the  current disk transfer
address to  the file,  at the  record position  specified by  the three  byte
random  record number (bytes 21h...23h). All three bytes of the random record
number are  used. If  the record  position is  beyond the current end of file
then un-initialized disk space will be allocated to fill the gap.

   The random record number field will not be changed, but the current record
and extent  fields will  be set  up to  refer to  the same record. The record
count  byte will be adjusted as necessary if the file is being extended or if
the write goes into a new extent.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-23h", "_fsize,dos2-fn-get-file-size-fcb", "MSX-DOS 2: funcoes", "C = 23H  (_FSIZE)", "Funcao 23H (_FSIZE): Get File Size [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.31   GET FILE SIZE [FCB] (23H)

     Parameters:    C = 23H (_FSIZE)
                   DE = Pointer to unopened FCB
     Results:     L=A = 0FFH if file not found
                        =   0  if file found OK

   This function  searches for  the first match with the filename in the FCB,
exactly the same as OPEN FILE (function 0FH). The size of the located file is
rounded up to the nearest 128 bytes and the number of records determined. The
three byte random record field of the FCB is set to the number of records, so
it is the number of the first record which does not exist. The fourth byte of
the random record number is not altered.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-24h", "_setrnd,dos2-fn-set-random-record-fcb", "MSX-DOS 2: funcoes", "C = 24H  (_SETRND)", "Funcao 24H (_SETRND): Set Random Record [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.32   SET RANDOM RECORD [FCB] (24H)

     Parameters:    C = 24H (_SETRND)
                   DE = Pointer to opened FCB
     Results:       None

   This function simply sets the three byte random record field in the FCB to
the record  determined by  the current  record and  extent number. The fourth
byte  of the  random record  number is  not altered.  No check  is done as to
whether the record actually exists in the file.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-26h", "_wrblk,dos2-fn-random-block-write-fcb", "MSX-DOS 2: funcoes", "C = 26H  (_WRBLK)", "Funcao 26H (_WRBLK): Random Block Write [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.33   RANDOM BLOCK WRITE [FCB] (26H)

     Parameters:    C = 26H (_WRBLK)
                   DE = Pointer to opened FCB
                   HL = Number of records to write
     Results:       A = 01H if error
                      =  0  if no error

   Data  is written from the current disk transfer address to the position in
the file  defined by  the random record number. The record size is determined
by  the record size field in the FCB (bytes 0Eh and 0Fh) which must be set by
the user  after opening  the file  and before  calling this  function. If the
record  size is  less than  64 bytes then all four bytes of the random record
number are used, otherwise only the first three are used.

   The number  of records to be written is specified by HL, and together with
the  record size  this determines  the amount of data to be written. An error
will be returned if the size exceeds 64k, thus limiting the maximum size of a
transfer.

   After  writing the  data, the  random record field is adjusted to the next
record number  in the file (ie. HL is added on to it). The current record and
extent  fields are not used or altered. The file size field is updated if the
file has been extended.

   The  record size  can be any value from 1...0FFFFh. Small record sizes are
no less  efficient that  large record sizes so if desired the record size can
be set to one and the record count then becomes a byte count. It is desirable
to  write as much as possible with one function call since one large transfer
will be quicker than several small ones.

   If  the number  of records  to write  (HL) is  zero then  no data  will be
written, but  the size  of the file will be altered to the value specified by
the random record field. This may be either longer or shorter than the file's
current  size  and  disk  space  will  be  allocated  or  freed as  required.
Additional  disk space  allocated in  this way will not be initialized to any
particular value.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-27h", "_rdblk,dos2-fn-random-block-read-fcb", "MSX-DOS 2: funcoes", "C = 27H  (_RDBLK)", "Funcao 27H (_RDBLK): Random Block Read [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.34   RANDOM BLOCK READ [FCB] (27H)

     Parameters:    C = 27H (_RDBLK)
                   DE = Pointer to opened FCB
                   HL = Number of records to read
     Results:       A = 01H if error (usually caused by end-of-file)
                      =  0  if no error
                   HL = Number of records actually read

This  function is  the complement of the BLOCK WRITE function described above
and most of the same comments apply as regards its use. Again if large blocks
are read then it will be much faster than the normal CP/M operation.
   For example if it is desired to read 20k from a file, it is better to read
the 20k  with one  function call rather than 20 separate function calls of 1k
each.  However it  makes no  difference whether  the 20k  read is done with a
record size  of 1  and a record count of 20k, with a record size of 20k and a
record count of 1, or any intermediate combination.

   The number of records actually read is returned in HL. This may be smaller
than the  number of records requested if the end of the file was encountered.
In  this case  any partial record will be padded out with zeroes before being
copied to  the users  DTA. The  random record  field is adjusted to the first
record not read, ie. the value returned in HL is added on to it.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-28h", "_wrzer,dos2-fn-random-write-with-zero-fill-fcb", "MSX-DOS 2: funcoes", "C = 28H  (_WRZER)", "Funcao 28H (_WRZER): Random Write With Zero Fill [Fcb]",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.35   RANDOM WRITE WITH ZERO FILL [FCB] (28H)

     Parameters:    C = 28H (_WRZER)
                   DE = Pointer to opened FCB
     Results:     L=A = 01H if error
                     = 00H if no error

   This  function is  identical to RANDOM WRITE (function 22h) except that if
the file has to be extended, any extra allocated disk clusters will be filled
with zeroes before writing the data.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-2ah", "_gdate,dos2-fn-get-date", "MSX-DOS 2: funcoes", "C = 2AH  (_GDATE)", "Funcao 2AH (_GDATE): Get Date",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.36   GET DATE (2AH)

     Parameters:    C = 2AH (_GDATE)
     Results:      HL = Year 1980...2079
                   D = Month (1=Jan...12=Dec)
                   E = Date (1...31)
                   A = Day of week (0=Sun...6=Sat)

   This function simply returns the current value of the internal calender in
the format shown.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-2bh", "_sdate,dos2-fn-set-date", "MSX-DOS 2: funcoes", "C = 2BH  (_SDATE)", "Funcao 2BH (_SDATE): Set Date",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.37   SET DATE (2BH)

     Parameters:    C = 2BH (_SDATE)
                   HL = Year 1980...2079
                   D = Month (1=Jan...12=Dec)
                   E = Date (1...31)
     Results:      A = 00H if date was valid
                       FFH if date was invalid

   The supplied  date is  checked for  validity and if it is valid then it is
stored  as the  new date.  The validity  checks include full checking for the
number of  days in each month and leap years. If the date is invalid then the
current  date will  be left  unaltered. The  date is  stored in the real time
clock chip so it will be remembered when the machine is turned off.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-2ch", "_gtime,dos2-fn-get-time", "MSX-DOS 2: funcoes", "C = 2CH  (_GTIME)", "Funcao 2CH (_GTIME): Get Time",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.38   GET TIME (2CH)

     Parameters:    C = 2CH (_GTIME)
     Results:       H = Hours (0...23)
                    L = Minutes (0...59)
                    D = Seconds (0...59)
                    E = Centiseconds (always zero)

   This function  returns the current value of the system clock in the format
shown.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-2dh", "_stime,dos2-fn-set-time", "MSX-DOS 2: funcoes", "C = 2DH  (_STIME)", "Funcao 2DH (_STIME): Set Time",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.39   SET TIME (2DH)

     Parameters:    C = 2DH (_STIME)
                    H = Hours (0...23)
                    L = Minutes (0...59)
                    D = Seconds (0...59)
                    E = Centiseconds (ignored)
     Results:       A = 00H if time was valid
                        FFH if time was invalid

   This function  sets the internal system clock to the specified time value.
If  the time  is invalid then register A will be returned as 0FFh to indicate
an error  and the  current time will be left unaltered. The time is stored in
the  real time  clock chip and so it will be remembered and kept correct when
the machine is turned off.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-2eh", "_verify,dos2-fn-set-reset-verify-flag", "MSX-DOS 2: funcoes", "C = 2EH  (_VERIFY)", "Funcao 2EH (_VERIFY): Set/Reset Verify Flag",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.40   SET/RESET VERIFY FLAG (2EH)

     Parameters:    C = 2EH (_VERIFY)
                    E =  0 to disable verify
                      <> 0 to enable verify
     Results:       None

   This function  simply enables  or disables  automatic verification  of all
writes.  It  defaults  to off  when MSX-DOS  is started  up. Enabling  verify
improves  system reliability  but also slows down write operations. Note that
this function  depends on  the disk  driver and  the verification will not be
done if the driver does not support it.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-2fh", "_rdabs,dos2-fn-absolute-sector-read", "MSX-DOS 2: funcoes", "C = 2FH  (_RDABS)", "Funcao 2FH (_RDABS): Absolute Sector Read",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.41   ABSOLUTE SECTOR READ (2FH)

     Parameters:    C = 2FH (_RDABS)
                   DE = Sector number
                    L = Drive number (0=A: etc.)
                    H = Number of sectors to read
     Results:       A = Error code (0=> no error)

   This function  reads sectors  directly from  the disk without interpreting
them  as files.  The disk  must be  a valid  DOS disk in order for the sector
number to  be translated  into a  physical position  on the disk. The sectors
will  be read  to the  current disk  transfer address. Any disk error will be
reported by the system in the usual way.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-30h", "_wrabs,dos2-fn-absolute-sector-write", "MSX-DOS 2: funcoes", "C = 30H  (_WRABS)", "Funcao 30H (_WRABS): Absolute Sector Write",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.42   ABSOLUTE SECTOR WRITE (30H)

     Parameters:    C = 30H (_WRABS)
                   DE = Sector number
                    L = Drive number (0=A: etc.)
                    H = Number of sectors to write
     Results:       A = Error code

   This  function writes  sectors directly  to the  disk without interpreting
them as  files. The  disk must  be a  valid DOS  disk in order for the sector
number  to be  translated into  a physical  position on the disk. The sectors
will be  written from  the current disk transfer address. Any disk errors are
reported by the system in the usual way.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-31h", "_dparm,dos2-fn-get-disk-parameters", "MSX-DOS 2: funcoes", "C = 31H  (_DPARM)", "Funcao 31H (_DPARM): Get Disk Parameters",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.43   GET DISK PARAMETERS (31H)

     Parameters:    C = 31H (_DPARM)
                   DE = Pointer to 32 byte buffer
                    L = Drive number (0=default, 1=A: etc.)
     Results:       A = Error code
                   DE = Preserved

   This functions  returns a  series of  parameters relating to the format of
the  disk in  the specified  drive, to the buffer allocated within the user's
program. It  is useful  for programs  which are  going to  do absolute sector
reads  and writes,  in order  for them  to be  able to interpret the absolute
sector numbers. The parameters returned contain some redundant information in
order to  provide parameters which are most useful to transient programs. The
format of the returned parameter block is:

     DE+0      - Physical drive number (1=A: etc)
     DE+1,2    - Sector size (always 512 currently)
     DE+3      - Sectors per cluster (non-zero power of 2)
     DE+4,5    - Number of reserved sectors (usually 1)
     DE+6      - Number of copies of the FAT (usually 2)
     DE+7,8    - Number of root directory entries
     DE+9,10   - Total number of logical sectors
     DE+11     - Media descriptor byte
     DE+12     - Number of sectors per FAT
     DE+13..14 - First root directory sector number
     DE+15..16 - First data sector number
     DE+17..18 - Maximum cluster number
     DE+19     - Dirty disk flag
     DE+20..23 - Volume id. (-1 => no volume id.)
     DE+24..31 - Reserved (currently always zero)

   The dirty  disk flag  indicates whether  in the disk there is a file which
can  be recovered  by UNDEL  command. It is reset when the file allocation is
done.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-40h", "_ffirst,dos2-fn-find-first-entry", "MSX-DOS 2: funcoes", "C = 40H  (_FFIRST)", "Funcao 40H (_FFIRST): Find First Entry",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.44   FIND FIRST ENTRY (40H)

     Parameters:    C = 40H (_FFIRST)
                   DE = Drive/path/file ASCIIZ string
                                or fileinfo block pointer
                        HL = filename ASCIIZ string (only if
                                DE = fileinfo pointer)
                         B = Search attributes
                   IX = Pointer to new fileinfo block
     Results:       A = Error
                 (IX) = Filled in with matching entry

   The "drive/path" portion of the string, or the fileinfo block, specifies a
directory which  is to  be searched.  A ".IATTR"  error will be returned if a
fileinfo  block which  specifies a  file is passed. The "file" portion of the
string, or  the filename  ASCIIZ string in HL, determines what filenames will
be matched. If no match is found then a ".NOFIL" error is returned, otherwise
the  fileinfo block  pointed to  by IX  is filled  in with the details of the
matching entry.

   The filename  may contain  ambiguous filename  characters ("?"  or "*") in
which case the first matching entry will be returned. If the filename is null
(either  the ASCIIZ  string pointed  to by DE is null or ends in a "\" or the
filename string  pointed to  by HL  is null),  then this function will behave
exactly as if the filename was "*.*" so any name will match.

   The  attributes byte  in register  B specifies  what type of entry will be
matched. If  it is zero then only non-hidden, non-system files will be found.
If  the directory,  hidden or  system bits in register B are set then entries
with these  attributes will  be matched  as well  as ordinary files. The read
only and archive bits of register B are ignored.

   If the  volume name bit of register B is set then the search is exclusive,
only  the volume  label entry  will be  found. In this case also the fileinfo
block  and  filename or  the drive/path/file  string are  ignored apart  from
specifying the drive. This means that the volume name will always be found in
the root directory if it exists whether or not it matches the filename given.

   If DE  points to  a fileinfo  block, then  if desired, IX can point to the
same  fileinfo block.  In this  case when  a match  is found the new fileinfo
block will overwrite the old one.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-41h", "_fnext,dos2-fn-find-next-entry", "MSX-DOS 2: funcoes", "C = 41H  (_FNEXT)", "Funcao 41H (_FNEXT): Find Next Entry",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.45   FIND NEXT ENTRY (41H)

     Parameters:    C = 41H (_FNEXT)
                        IX = Pointer to fileinfo block from
                             previous find first function.
     Results:       A = Error
                 (IX) = Filled in with next matching entry

   This function  should only  be used  after a  "find first  entry" function
call.  It  searches  the  directory  for the  next match  to the  (presumably
ambiguous)  filename which was given to the "find first entry" function call.
If there  are no  more matching  entries then  a ".NOFIL"  error is returned,
otherwise  the fileinfo block is filled in with the information about the new
matching entry.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-42h", "_fnew,dos2-fn-find-new-entry", "MSX-DOS 2: funcoes", "C = 42H  (_FNEW)", "Funcao 42H (_FNEW): Find New Entry",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.46   FIND NEW ENTRY (42H)

     Parameters:    C = 42H (_FNEW)
                        DE = Drive/path/file ASCIIZ string
                                or fileinfo block pointer
                        HL = filename ASCIIZ string (only if
                                DE = fileinfo pointer)
                         B = b0..b6 = Required attributes
                             b7 = Create new flag
                   IX = Pointer to new fileinfo block
                         containing template filename
     Results:       A = Error
                 (IX) = Filled in with new entry

   This function is very similar to the "find first entry" function described
above. The  parameters in  HL and  DE are  used in  exactly the  same way  to
specify  a  directory  entry.  However  instead  of  searching  the  selected
directory  for an entry which matches the specified name, a new entry will be
created with this name. The fileinfo block pointed to by IX will be filled in
with information  about the  new entry  just as  if it  had been found with a
"find first entry" call.

   If there  are any  ambiguous characters ("?" or "*") in the filename, then
they will be replaced by the appropriate character from a "template filename"
in  the filename  position of the new fileinfo block pointed to by IX. If the
result is  still ambiguous,  or otherwise  illegal, then  a ".IFNM"  error is
returned. This is useful for copy operations which do an automatic rename.

   Like  "find first entry", if the filename is null, then it will be treated
exactly as  if it  was "*.*".  For this function that means that the template
filename will be used as the new filename to create.

   A  ".DRFUL"  error  will  be  returned if  there is  no room  in the  root
directory, and a ".DKFUL" if a sub-directory must be extended and the disk is
already full.

   The  attribute byte  passed in  register B  is the attribute which the new
entry will be given. If the volume name bit is set then a volume name will be
created in  the root  directory. If  the directory  bit is set then the entry
created  will be  for a  sub-directory, otherwise  it will be for a file. The
system, hidden  and read  only bits may be set for a file, and the hidden bit
for a sub-directory. A file will always be created with the archive attribute
bit set.

   A file  will be  created as  zero length with the current date and time. A
sub-directory will have a single cluster allocated to it and the "." and ".."
entries will be initialized appropriately.

   If there is already an entry with the specified name in the directory then
the action depends on the "create new" flag (bit-7 of register B) and also on
the type  of the entry. If the "create new" flag is set then a ".FILEX" error
will  always be returned. Setting this flag thereforeensures that an existing
file will not be deleted.

   If  an entry  already exists and the "create new" flag is not set then the
type of  the existing  entry is  examined to see whether it can be deleted to
make  room for the new file. An error will be returned if the entry is a read
only file  (".FILRO" error), a system file (".SYSX" error) or a sub-directory
(".DIRX" error) or there is a file handle already open to this file (".FOPEN"
error). If we are trying to create a sub-directory then even an ordinary file
will not be deleted (".FILEX" error).

   For  all  of  these  error  codes (".FILEX",  ".FILRO", ".SYSX",  ".DIRX",
".FOPEN"),  the  fileinfo  block will  be filed  in with  the details  of the
already  existing entry  and this fileinfo block may be used exactly as if it
had been returned from a "find first" function.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-43h", "_open,dos2-fn-open-file-handle", "MSX-DOS 2: funcoes", "C = 43H  (_OPEN)", "Funcao 43H (_OPEN): Open File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.47   OPEN FILE HANDLE (43H)

     Parameters:    C = 43H (_OPEN)
                   DE = Drive/path/file ASCIIZ string
                            or fileinfo block pointer
                    A = Open mode. b0 set => no write
                                   b1 set => no read
                                   b2 set => inheritable
                                   b3..b7   -  must be clear
     Results:       A = Error
                    B = New file handle

   The  drive/path/file string or the fileinfo block should normally refer to
a file  rather than  a sub-directory  or volume  name. If it is a volume name
then a ".IATTR" error will be returned. If it is a sub-directory then ".DIRX"
error will be returned.

   Assuming that a file is specified then it will be opened ready for reading
and/or writing (depending on the open mode in A) and a new file handle for it
will  be returned in register B. The lowest available file handle number will
be used and an error will result if there are no spare file handles (".NHAND"
error), or insufficient memory (".NORAM" error).

   If the  "no read" bit of register A is set then reads from the file handle
will  be  rejected  and if  the "no  write" bit  is set  then writes  will be
rejected,  in both  cases with an ".ACCV" error. Writes will also be rejected
if  the  file is  read only  (".FILRO" error).  If the  "inheritable" bit  of
register A  is set  then the  file handle  will be inherited by a new process
created by the "fork" function call (see function 60h).

   If a  device file  handle is opened by giving a filename which matches one
of  the built in devices (for example "CON" or "NUL"), then it will always be
opened initially in ASCII mode. The IOCTL function (function 4Bh) can be used
to change  this to  binary mode  but great care must be taken in reading from
devices in binary mode because there is no end of file condition.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-44h", "_create,dos2-fn-create-file-handle", "MSX-DOS 2: funcoes", "C = 44H  (_CREATE)", "Funcao 44H (_CREATE): Create File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.48   CREATE FILE HANDLE (44H)

     Parameters:    C = 44H (_CREATE)
                   DE = Drive/path/file ASCIIZ string
                    A = Open mode. b0 set => no write
                                   b1 set => no read
                                   b2 set => inheritable
                                 b3..b7   -  must be clear
                    B = b0..b6 = Required attributes
                            b7 = Create new flag
     Results:       A = Error
                    B = New file handle

   A file  or sub-directory,  as specified  by the  attributes in register B,
will  be  created  with  the  name  and  in  the  directory specified  by the
drive/path/file  string. A ".IATTR" error is returned if register B specifies
a volume name.

   An error  will be returned if the file or sub-directory cannot be created.
The  error conditions  in this  case are the same as for the "find new entry"
function (function  42h) with  the main  error codes being ".FILEX", ".DIRX",
".SYSX",  ".FILRO",  ".FOPEN",  ".DRFUL" and  ".DKFUL". Like  the "find  new"
function,  if the  "create new"  flag (bit-7  of register  B) is  set then an
existing file will not be deleted and will always return a ".FILEX" error.

   If the  attributes byte  specifies a sub-directory then the hidden bit may
also  be set to create a hidden sub-directory. For a file, the hidden, system
or  read  only  bits  may  be  set  to  create  a  file with  the appropriate
attributes. An  invalid attributes  bits will  simply be ignored. A file will
always be created with the archive attribute bit set.

   A  file  will  automatically  be opened  just as  for the  "open" function
described above,  and a  file handle  returned in register B. The "open mode"
parameter  is  interpreted  in the  same way  as for  the "open"  function. A
sub-directory  will not be opened (because this is meaningless) so register B
will be returned as 0FFh which can never be a valid file handle.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-45h", "_close,dos2-fn-close-file-handle", "MSX-DOS 2: funcoes", "C = 45H  (_CLOSE)", "Funcao 45H (_CLOSE): Close File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.49   CLOSE FILE HANDLE (45H)

     Parameters:    C = 45H (_CLOSE)
                    B = File handle
     Results:       A = Error

   This  function  releases  the  specified file  handle for  re-use. If  the
associated file  has been written to then its directory entry will be updated
with  a new  date and  time, the  archive attributes bit will be set, and any
buffered data  will be  flushed to  disk. Any  subsequent attempt to use this
file  handle will return an error. If there are any other copies of this file
handle, created by "duplicate file handle" or "fork", then these other copies
may still be used.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-46h", "_ensure,dos2-fn-ensure-file-handle", "MSX-DOS 2: funcoes", "C = 46H  (_ENSURE)", "Funcao 46H (_ENSURE): Ensure File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.50   ENSURE FILE HANDLE (46H)

     Parameters:    C = 46H (_ENSURE)
                    B = File handle
     Results:       A = Error

   If  the file  associated with the file handle has been written to then its
directory  entry  will  be  updated with  a new  date and  time, the  archive
attributes bit  will be  set, and  any buffered data will be flushed to disk.
The file handle is not released and so it can still be used for accessing the
file, and the current file pointer setting will not be altered.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-47h", "_dup,dos2-fn-duplicate-file-handle", "MSX-DOS 2: funcoes", "C = 47H  (_DUP)", "Funcao 47H (_DUP): Duplicate File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.51   DUPLICATE FILE HANDLE (47H)

     Parameters:    C = 47H (_DUP)
                    B = File handle
     Results:       A = Error
                    B = New file handle

   This  function creates  a copy  of the  specified file  handle. The lowest
available  file  handle  number  will  always  be used  and a  ".NHAND" error
returned if  there are  none available. The new file handle will refer to the
same  file as the original and either one may be used. If the file pointer of
one handle  is moved,  the other  one will also be moved. If either handle is
closed the other one may still be used.

   Note that  because duplicate file handles created by this function are not
"separately  opened", they  do not  count as  separate file  handles for  the
purposes of  generating ".FOPEN" errors. So for example a "DUP"ed file handle
may  be renamed  (function 53h) or have its attributes changed (function 55h)
and the  effect will  apply to  both file handles. Note in particular that if
one  copy of  a "DUP"ed  file handle  is deleted (function 54h) then the file
really will be deleted and the other file handle, although still open, can no
longer be  used safely.  If it  is used  (other than being closed, ensured or
deleted) then an ".FDEL" error will be returned.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-48h", "_read,dos2-fn-read-from-file-handle", "MSX-DOS 2: funcoes", "C = 48H  (_READ)", "Funcao 48H (_READ): Read From File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.52   READ FROM FILE HANDLE (48H)

     Parameters:    C = 48H (_READ)
                    B = File handle
                   DE = Buffer address
                   HL = Number of bytes to read
     Results:       A = Error
                   HL = Number of bytes actually read

   The  specified number  of bytes are read from the file at the current file
pointer position  and copied  to the buffer address specified in register DE.
The file pointer is then updated to the next sequential byte. A ".ACCV" error
will  be returned if the file handle was opened with the "no read" access bit
set.

   The number of bytes read may be less than the number requested for various
reasons, and  the number  read will be returned in register HL if there is no
error.  In general  if less  is read  than requested  then this should not be
treated as  an error  condition but  another read  should be done to read the
next portion, until a ".EOF" error is returned. An ".EOF" error will never be
returned  for a partial read, only for a read which reads zero bytes. Reading
files in  this way  ensures that device file handles will work correctly (see
below).

   For disk  files the number of bytes read will only be less than the number
requested  if the  end of  the file is reached and in this case the next read
operation will  read zero bytes and will return an ".EOF" error. When reading
from a device file handle (for example the standard file handles 0 to 4), the
behaviour  depends on  the particular device, and on whether it is being read
in ASCII  or binary  mode (see  function 4Bh below). The "CON" device will be
described  as an  example because  it is  the most  commonly used device, but
other devices behave similarly.

   When reading from the "CON" device in binary mode, characters will be read
from the keyboard, without any interpretation and without being echoed to the
screen or  printer. The  exact number  of characters requested will always be
read and there is no end of file condition. Because of the lack of any end of
file indication, great care must be taken when reading from devices in binary
mode.

   A  read function  call to the "CON" device in ASCII mode (the default mode
and that  which normally  applies to  the standard  input channel), will only
read  one line  of input.  The input line will be read from the keyboard with
the normal  line editing  facilities available to the user, and the character
typed  will be  echoed to the screen and to the printer if Ctrl-P is enabled.
Special control  characters "Ctrl-P", "Ctrl-N", "Ctrl-S" and "Ctrl-C" will be
tested  for and  will be  treated exactly  as for the console status function
0Bh.

   When  the user types a carriage return the line will be copied to the read
buffer, terminated  with a  CR-LF sequence  and the read function will return
with  an appropriate  byte count.  The next  read will start another buffered
line input  operation. If  the number of bytes requested in the read was less
than the length of the line input then as many character as requested will be
returned,  and the  next read  function call will return immediately with the
next portion of the line until it has all been read.

   If the  user types a line which starts with a "Ctrl-Z" character then this
will be interpreted as indicating end of file. The line will be discarded and
the  read function  call will  read zero  bytes and return an ".EOF" error. A
subsequent read after this will be back to normal and will start another line
input. The end of file condition is thus not permanent.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-49h", "_write,dos2-fn-write-to-file-handle", "MSX-DOS 2: funcoes", "C = 49H  (_WRITE)", "Funcao 49H (_WRITE): Write To File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.53   WRITE TO FILE HANDLE (49H)

     Parameters:    C = 49H (_WRITE)
                    B = File handle
                   DE = Buffer address
                   HL = Number of bytes to write
     Results:       A = Error
                   HL = Number of bytes actually written

   This function is very similar to the "read" function above (function 48h).
The number  of bytes  specified will  be written  to the current file pointer
position  in the file, and the file pointer will be adjusted to point to just
after the  last byte  written. If  the file  was opened  with the  "no write"
access bit set then a ".ACCV" error will be returned, and if the file is read
only then a ".FILRO" error will be returned.

   If the  write goes  beyond the  current end  of file then the file will be
extended  as necessary.  If the file pointer is already beyond the end of the
file then  disk space  will be  allocated to  fill the  gap and  will not  be
initialized.  If there  is insufficient disk space then a ".DKFUL" error will
be returned  and no  data will be written, even if there was room for some of
the data.

   The number of bytes written can usually be ignored since it will either be
zero  if an  error is returned or it will be equal to the number requested if
the write  was successful. It is very much more efficient to write files in a
few  large blocks  rather than many small ones, so programs should always try
to write in as large blocks as possible.

   This function sets a "modified" bit for the file handle which ensures that
when the  file handle  is closed or ensured, either explicitly or implicitly,
the  directory entry  will be  updated with the new date, time and allocation
information. Also  the archive bit will be set to indicate that this file has
been modified since it was last archived.

   Writing  to device  file handles is not a complicated as reading from them
because there  are no  end of  file conditions  or line input to worry about.
There  are some differences between ASCII and binary mode when writing to the
"CON" device, in that a console status check is done in ASCII mode only. Also
printer echo if enabled will only be done in ASCII mode.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-4ah", "_seek,dos2-fn-move-file-handle-pointer", "MSX-DOS 2: funcoes", "C = 4AH  (_SEEK)", "Funcao 4AH (_SEEK): Move File Handle Pointer",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.54   MOVE FILE HANDLE POINTER (4AH)

     Parameters:    C = 4AH (_SEEK)
                    B = File handle
                    A = Method code
                DE:HL = Signed offset
     Results:       A = Error
                DE:HL = New file pointer

   The file pointer associated with the specified file handle will be altered
according to  the method  code and offset, and the new pointer value returned
in DE:HL. The method code specifies where the signed offset is relative to as
follows:

     A=0  Relative to the beginning of the file
     A=1  Relative to the current position
     A=2  Relative to the end of the file.

   Note  that an  offset of  zero with an method code of 1 will simply return
the current  pointer value,  and with a method code of 2 will return the size
of  the file.  No end  of file  check is  done so  it is  quite possible (and
sometimes useful)  to set  the file  pointer beyond  the end  of the file. If
there  are any  copies of  this file  handle created  by the  "duplicate file
handle" function  (function 47h)  or the  "fork" function (function 60h) then
their file pointer will also be changed.

   The file  pointer only  has any  real meaning  on disk  files since random
access is possible. On device files the file pointer is updated appropriately
when  any read  or write  is done,  and can  be examined  or altered  by this
function.  However  changing will  have no  effect and  examining it  is very
unlikely to be useful.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-4bh", "_ioctl,dos2-fn-i-o-control-for-devices", "MSX-DOS 2: funcoes", "C = 4BH  (_IOCTL)", "Funcao 4BH (_IOCTL): I/O Control For Devices",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.55   I/O CONTROL FOR DEVICES (4BH)

     Parameters:    C = 4BH (_IOCTL)
                    B = File handle
                    A = Sub-function code
                        00H => get file handle status
                        01H => set ASCII/binary mode
                        02H => test input ready
                        03H => test output ready
                        04H => find screen size
                   DE = Other parameters
     Results:       A = Error
                   DE = Other results

   This function  allows various  aspects of  file handles to be examined and
altered.  In particular  it can  be used  to determine  whether a file handle
refers to  a disk file or a device. This is useful for programs which want to
behave differently for disk files and device I/O.

   This function  is passed  the file handle in register B and a sub-function
code  in register  A which specifies one of various different operations. Any
other  parameters  required  by  the  particular  sub-function are  passed in
register DE and results are returned in register DE. If the sub-function code
is invalid then a ".ISBFN" error will be returned.

   If A=0 then the operation is "get file handle status". This returns a word
of flags  which give various information about the file handle. The format of
this  word is  different for  device file  handles and disk file handles, and
bit-7 specifies which it is. The format of the word is as follows:

For devices:    DE -     b0  set  => console input device
                         b1  set  => console output device
                     b2..b4  reserved
                         b5  set  => ASCII mode
                             clear=> binary mode
                         b6  set  => end of file
                         b7  always set (=> device)
                    b8..b15  reserved

For disk files: DE - b0..b5  drive number (0=A: etc)
                         b6  set  => end of file
                         b7  always clear (=> disk file)
                    b8..b15  reserved

   Note  that the end of file flag is the same for devices as for disk files.
For devices  it will  be set  if the previous attempt to read from the device
produced  a ".EOF" error and will be cleared by the next read. For disk files
it is worked out by comparing the file pointer with the file size.

   If  A=1 then the operation is a "set ASCII/binary mode". This operation is
only allowed  for device file handles. An ASCII/binary flag must be passed in
bit-5  of  register  E  (exactly  where it  is returned  by "get  file handle
status").  This is  set for  ASCII mode  and clear for binary mode. All other
bits of register DE are ignored.

   If A=2  or 3  then the  operation is  "test input  ready" or  "test output
ready"  respectively. In both cases a flag is returned in register E which is
FFh if  the file  handle is  ready for  a character and 00h if not. The exact
meaning  of "ready  for a character" depends on the device. Disk file handles
are always  ready for  output, and are always ready for input unless the file
pointer is at the end of file. The "CON" device checks the keyboard status to
determine whether it is ready for input or not.

   If A=4  the the  operation is  "get screen size". This returns the logical
screen size for the file handle with the number of rows in register D and the
number  of columns  in register  E. For  devices with no screen size (such as
disk  files)  both  D  and  E  will be  zero. Zero  for either  result should
therefore be interpreted as "unlimited". For example this function is used by
the "DIR  /W" command to decide how many files to print per line, and a value
of zero for register E is defaulted to 80.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-4ch", "_htest,dos2-fn-test-file-handle", "MSX-DOS 2: funcoes", "C = 4CH  (_HTEST)", "Funcao 4CH (_HTEST): Test File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.56   TEST FILE HANDLE (4CH)

     Parameters:    C = 4CH (_HTEST)
                    B = File handle
                   DE = Drive/path/file ASCIIZ string
                                 or fileinfo block pointer
     Results:       A = Error
                    B = 00H => not the same file
                        FFH => same file

   This  rather  specialist function  is passed  a file  handle and  either a
drive/path/file  string  or  a  fileinfo block  which identifies  a file.  It
determines if  the two  files are  actually the  same file and returns a flag
indicating  the result.  Note that  if the file handle is for a device rather
than a disk file then it will always return "B=00h" to indicate "not the same
file".

   This function allows the "COPY" command to detect certain error conditions
such as  copying file  onto themselves  and give  the user  informative error
messages.  It may  also be useful for other programs which need to do similar
tests.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-4dh", "_delete,dos2-fn-delete-file-or-subdirectory", "MSX-DOS 2: funcoes", "C = 4DH  (_DELETE)", "Funcao 4DH (_DELETE): Delete File Or Subdirectory",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.57   DELETE FILE OR SUBDIRECTORY (4DH)

     Parameters:    C = 4DH (_DELETE)
                   DE = Drive/path/file ASCIIZ string
                                or fileinfo block pointer
     Results:       A = Error

   This function  deletes the object (file or sub-directory) specified by the
drive/path/file  string or the fileinfo block. Global filename characters are
not  allowed  so only  one file  or sub-directory  can be  deleted with  this
function. A  sub-directory can  only be  deleted if  it is  empty or an error
(".DIRNE") occurs if not). The "." and ".." entries in a sub-directory cannot
be  deleted (".DOT"  error) and neither can the root directory. A file cannot
be deleted  if there  is a  file handle open to it (.FOPEN error) or if it is
read only (.FILRO error).

   If  it is  a file  then any  disk space  which was allocated to it will be
freed. If  the disk  is an MSX-DOS 2 disk then enough information is retained
on  the disk  to allow the "UNDEL" utility program do undelete the file. This
information is only retain ed until the next disk space allocation (usually a
write to  a file) is done on this disk. After making this function call, if a
fileinfo  block was passed then it must not be used again (other than passing
it to  a "find  next entry"  function) since  the file  to which it refers no
longer exists.

   If a device name such as "CON" is specified then no error will be returned
but the device will not actually be deleted.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-4eh", "_rename,dos2-fn-rename-file-or-subdirectory", "MSX-DOS 2: funcoes", "C = 4EH  (_RENAME)", "Funcao 4EH (_RENAME): Rename File Or Subdirectory",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.58   RENAME FILE OR SUBDIRECTORY (4EH)

     Parameters:    C = 4EH (_RENAME)
                   DE = Drive/path/file ASCIIZ string
                                or fileinfo block pointer
                   HL = New filename ASCIIZ string
     Results:       A = Error

   This function  renames the object (file or sub-directory) specified by the
drive/path/file string or the fileinfo block, with the new name in the string
pointed  to by HL. The new filename string must not contain a drive letter or
directory path  (".IFNM" error if it does). If a device name such as "CON" is
specified  then no error will be returned but the device will not actually be
renamed.

   Global filename  characters are not allowed in the drive/path/file string,
so  only one  object can be renamed by this function. However global filename
characters are  allowed in the new filename passed in HL and where they occur
the  existing filename  character will  be left unaltered. Checks are done to
avoid creating an illegal filename, for example a file called "XYZ" cannot be
renamed with  a new filename string of "????A" because the new filename would
be "XYZ A" which is illegal. In this case a ".IFNM" error will be returned.

   If there is already an entry with the new filename then an error (".DUPF")
is returned  to avoid  creating duplicate filenames. The "." and ".." entries
in a sub-directory cannot be renamed (".IDOT" error) and neither can the root
directory (it has noname). A file cannot be renamed if there is a file handle
open to it (".FOPEN" error) although a read only file can be renamed.

   Note that  if DE pointed to a fileinfo block, this is not updated with the
new  name of  the file.  Therefore care  must be  taken in using the fileinfo
block after making this function call.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-4fh", "_move,dos2-fn-move-file-or-subdirectory", "MSX-DOS 2: funcoes", "C = 4FH  (_MOVE)", "Funcao 4FH (_MOVE): Move File Or Subdirectory",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.59   MOVE FILE OR SUBDIRECTORY (4FH)

     Parameters:    C = 4FH (_MOVE)
                   DE = Drive/path/file ASCIIZ string
                                or fileinfo block pointer
                   HL = New path ASCIIZ string
     Results:       A = Error

   This  function moves  the object  (file or sub-directory) specified by the
drive/path/file string  or the  fileinfo block, to the directory specified by
the  new path  string pointed to by HL. There must not be a drive name in the
new path  string. If  a device  name such as "CON" is specified then no error
will be returned but the device will not actually be moved.

   Global filename  characters are  not allowed in any of the strings so only
one object (file or sub-directory) can be moved by this function, although if
a sub-directory is moved, all its descendants will be moved with it. If there
is  already an  entry of  the required  name in  the target  directory then a
".DUPF" error  is returned  to prevent  creating duplicate filenames. The "."
and ".." entries in a sub-directory cannot be moved (".DOT" error) and also a
directory  cannot be  moved into  one of  its own descendants (".DIRE" error)
since this  would create an isolated loop in the filing system. A file cannot
be moved if there is a file handle open to it (".FOPEN" error).

   Note  that if  a fileinfo  block is  passed to this function, the internal
information in  the fileinfo block is not updated to reflect the new location
of the file. This is necessary because otherwise the fileinfo block could not
be used for a subsequent "find next" function call. However it does mean that
the fileinfo block no longer refers to the moved file and so must not be used
for any operations on it such as "rename" or "open".

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-50h", "_attr,dos2-fn-get-set-file-attributes", "MSX-DOS 2: funcoes", "C = 50H  (_ATTR)", "Funcao 50H (_ATTR): Get/Set File Attributes",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.60   GET/SET FILE ATTRIBUTES (50H)

     Parameters:    C = 50H (_ATTR)
                   DE = Drive/path/file ASCIIZ string
                                or fileinfo block pointer
                    A = 0 => get attributes
                        1 => set attributes
                    L = New attributes byte (only if A=1)
     Results:       A = Error
                    L = Current attributes byte

   This function  is normally  used to  change the  attributes of  a file  or
sub-directory.  It can  also be  used to  find out the current attributes but
this is  more usually  done with  the "find  first entry"  function (function
40h).  If A=0  then the current attributes byte for the file or sub-directory
will just be returned in register L.

   If A=1  then the attributes byte will be set to the new value specified in
register  L, and this new value will also be returned in register L. Only the
system, hidden,  read only  and archive  bits may  be altered for a file, and
only  the hidden  bit for a sub-directory. An ".IATTR" error will be returned
if an  attempt is made to alter any other attribute bits. If a fileinfo block
is  passed then  the attributes  byte in  it will not be updated with the new
setting.

   Global filename  characters are  not allowed  so only  one object (file or
sub-directory)  can have  its attributes set by this function. The attributes
of the  root directory  cannot be  changed because  it does not have any. The
attributes  of a  file cannot be changed if there is a file handle open to it
(".FOPEN"  error).  The  attributes  of the  "." and  ".." directory  entries
however can  be changed.  If a device name such as "CON" is specified then no
error  will be  returned but  the device's  attributes will  not actually  be
changed (since it does not have any).

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-51h", "_ftime,dos2-fn-get-set-file-date-and-time", "MSX-DOS 2: funcoes", "C = 51H  (_FTIME)", "Funcao 51H (_FTIME): Get/Set File Date And Time",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.61   GET/SET FILE DATE AND TIME (51H)

     Parameters:    C = 51H (_FTIME)
                   DE = Drive/path/file ASCIIZ string
                                or fileinfo block pointer
                    A = 0 => get date and time
                        1 => set date and time
                   IX = New time value (only if A=1)
                   HL = New date value (only if A=1)
     Results:       A = Error
                   DE = Current file time value
                   HL = Current file date value

   If  A=1 then  this function sets the date and time of last modification of
the file or sub-directory specified by the drive/path/file string or fileinfo
block. Global  filename characters  are not allowed in any part of the string
so  only one  file can have its date and time modified by this function. If a
device name such as "CON" is specified then no error will be returned but the
device's date and time will not actually be changed.

   The date  and time  format are exactly as contained in the directory entry
and  fileinfo blocks  (see the  "Program Interface Specification"). No checks
are done for sensible dates or times, the values are simply stored. Note that
if a fileinfo block is passed then the date and time stored in it will not be
updated by this function.

   If  A=0 then  the current values are just returned. Note that although the
time value is passed in IX, it is returned in DE. The date and time of a file
cannot be altered (although it can be read) if there is a file handle open to
the file (".FOPEN" error).

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-52h", "_hdelete,dos2-fn-delete-file-handle", "MSX-DOS 2: funcoes", "C = 52H  (_HDELETE)", "Funcao 52H (_HDELETE): Delete File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.62   DELETE FILE HANDLE (52H)

     Parameters:    C = 52H (_HDELETE)
                    B = File handle
     Results:       A = Error

   This function  deletes the  file handle associated with the specified file
and  closes the file handle. A file handle cannot be deleted if there are any
other separately  opened file handles open to the same file (".FOPEN" error).
If  there are any duplicates of the file handle (created by a "duplicate file
handle" or  "fork" function), then these duplicates will be marked as invalid
and any attempt to use them will produce an ".HDEAD" error.

   The  error conditions  for this  function are  the same as for the "delete
file or  sub-directory" function  (function 4Dh). The file handle will always
be closed, even if there is an error condition such as ".FILRO" or ".FOPEN".

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-53h", "_hrename,dos2-fn-rename-file-handle", "MSX-DOS 2: funcoes", "C = 53H  (_HRENAME)", "Funcao 53H (_HRENAME): Rename File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.63   RENAME FILE HANDLE (53H)

     Parameters:    C = 53H (_HRENAME)
                    B = File handle
                   HL = New filename ASCIIZ string
     Results:       A = Error

   This  function renames  the file associated with the specified file handle
with the  new name  in the  string pointed to by HL. Apart from the fact that
the  file is  specified by  a file  handle rather  than an ASCIIZ string or a
fileinfo  block,   this  function   is  identical  to  the  "rename  file  or
subdirectory" function (function 4Eh), and has the same error conditions.

   A file  handle cannot  be renamed if there are any other separately opened
file  handles for  this file  (".FOPEN" error), although it can be renamed if
there are  copies of  this file  handle, and  in this case the copies will be
renamed.  Renaming a  file handle will not alter the file pointer but it will
do an implicit "ensure" operation.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-54h", "_hmove,dos2-fn-move-file-handle", "MSX-DOS 2: funcoes", "C = 54H  (_HMOVE)", "Funcao 54H (_HMOVE): Move File Handle",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.64   MOVE FILE HANDLE (54H)

     Parameters:    C = 54H (_HMOVE)
                    B = File handle
                   HL = New path ASCIIZ string
     Results:       A = Error

   This function  moves the file associated with the specified file handle to
the  directory specified  by the new path string pointed to by HL. Apart from
the fact  that the  file is  specified by a file handle rather than an ASCIIZ
string  or a  fileinfo block, this function is identical to the "move file or
subdirectory" function (function 4Fh), and has the same error conditions.

   A file  handle cannot  be moved  if there  are any other separately opened
file  handles for  this file  (".FOPEN" error),  although it  can be moved if
there are  copies of  this file handle, and in this case the copies will also
be moved. Moving a file handle will not alter the file pointer but it will do
an implicit "ensure" operation.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-55h", "_hattr,dos2-fn-get-set-file-handle-attributes", "MSX-DOS 2: funcoes", "C = 55H  (_HATTR)", "Funcao 55H (_HATTR): Get/Set File Handle Attributes",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.65   GET/SET FILE HANDLE ATTRIBUTES (55H)

     Parameters:    C = 55H (_HATTR)
                    B = File handle
                    A = 0 => get attributes
                        1 => set attributes
                    L = New attributes byte (only if A=1)
     Results:       A = Error
                    L = Current attributes byte

   This function gets or sets the attributes byte of the file associated with
the specified  file handle. Apart from the fact that the file is specified by
a file handle rather than an ASCIIZ string or a fileinfo block, this function
is  identical to  the "get/set  file attributes" function (function 50h), and
has the same error conditions.

   A  file handle  cannot have  its attributes  changed (although they can be
read) if  there are  any other  separately opened  file handles for this file
(".FOPEN"  error).  The  file pointer  will not  be altered  but an  implicit
"ensure" operation will be done.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-56h", "_hftime,dos2-fn-get-set-file-handle-date-and-time", "MSX-DOS 2: funcoes", "C = 56H  (_HFTIME)", "Funcao 56H (_HFTIME): Get/Set File Handle Date And Time",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.66   GET/SET FILE HANDLE DATE AND TIME (56H)

     Parameters:    C = 56H (_HFTIME)
                    B = File handle
                    A = 0 => get date and time
                        1 => set date and time
                   IX = New time value (only if A=1)
                   HL = New date value (only if A=1)
     Results:       A = Error
                   DE = Current file time value
                   HL = Current file date value

   This  function gets  or sets the date and time of the file associated with
the specified  file handle. Apart from the fact that the file is specified by
a file handle rather than an ASCIIZ string or a fileinfo block, this function
is identical to the "get/set file date and time" function (function 51h), and
has the same error conditions.

   A  file handle cannot have its date and time changed (although they can be
read) if  there are  any other  separately opened  file handles for this file
(".FOPEN"  error).  The  file pointer  will not  be altered  but an  implicit
"ensure" operation will be done.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-57h", "_getdta,dos2-fn-get-disk-transfer-address", "MSX-DOS 2: funcoes", "C = 57H  (_GETDTA)", "Funcao 57H (_GETDTA): Get Disk Transfer Address",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.67   GET DISK TRANSFER ADDRESS (57H)

     Parameters:    C = 57H (_GETDTA)
     Results:      DE = Current disk transfer address

   This  function returns  the current disk transfer address. This address is
only used  for the  "traditional" CP/M  style FCB  functions and the absolute
sector read and write functions.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-58h", "_getvfy,dos2-fn-get-verify-flag-setting", "MSX-DOS 2: funcoes", "C = 58H  (_GETVFY)", "Funcao 58H (_GETVFY): Get Verify Flag Setting",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.68   GET VERIFY FLAG SETTING (58H)

     Parameters:    C = 58H (_GETVFY)
     Results:       B = 00H => verify disabled
                        FFH => verify enabled

   This  function simply  returns the  current state of the verify flag which
can be set with MSX-DOS function 2Eh.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-59h", "_getcd,dos2-fn-get-current-directory", "MSX-DOS 2: funcoes", "C = 59H  (_GETCD)", "Funcao 59H (_GETCD): Get Current Directory",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.69   GET CURRENT DIRECTORY (59H)

     Parameters:    C = 59H (_GETCD)
                    B = Drive number (0=current, 1=A: etc)
                   DE = Pointer to 64 byte buffer
     Results:       A = Error
                   DE = Filled in with current path

   This  function  simply  gets  an  ASCIIZ  string representing  the current
directory of the specified drive into the buffer pointed to by DE. The string
will not  include a drive name or a leading or trailing "\" character, so the
root directory is represented by a null string. The drive will be accessed to
make sure that the current directory actually exists on the current disk, and
if  not then  the current  directory will  be set back to the root and a null
string returned.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-5ah", "_chdir,dos2-fn-change-current-directory", "MSX-DOS 2: funcoes", "C = 5AH  (_CHDIR)", "Funcao 5AH (_CHDIR): Change Current Directory",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.70   CHANGE CURRENT DIRECTORY (5AH)

     Parameters:    C = 5AH (_CHDIR)
                   DE = Drive/path/file ASCIIZ string
     Results:       A = Error

   The drive/path/file  string must  specify a  directory rather than a file.
The  current directory  of the drive will be changed to be this directory. If
the  specified  directory does  not exist  then the  current setting  will be
unaltered and a ".NODIR" error returned.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-5bh", "_parse,dos2-fn-parse-pathname", "MSX-DOS 2: funcoes", "C = 5BH  (_PARSE)", "Funcao 5BH (_PARSE): Parse Pathname",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.71   PARSE PATHNAME (5BH)

     Parameters:    C = 5BH (_PARSE)
                    B = Volume name flag (bit 4)
                        DE = ASCIIZ string for parsing
     Results:       A = Error
                   DE = Pointer to termination character
                   HL = Pointer to start of last item
                    B = Parse flags
                    C = Logical drive number (1=A: etc)

   This function is purely a string manipulation function, it will not access
the disks  at all  and it  will not  modify the  user's string  at all. It is
intended to help transient programs in parsing command lines.

   The volume  name flag (bit 4 of register B; it is in the same bit position
as  the volume  name bit in an attributes byte) determines whether the string
will be  parsed as  a "drive/path/file"  string (if  the bit is cleared) or a
"drive/volume" string (if the bit is set).

   The pointer  returned in DE will point to the first character which is not
valid in a pathname string, and may be the null at the end of the string. See
the "Command Specification" for details of the syntax of pathname strings and
also for a list of valid characters.

   The  pointer returned  in HL will point to the first character of the last
item of  a string  (filename portion). For example, when a string "A:\XYZ\P.Q
/F" was passed, DE will point to the white space character before "/F" and HL
will  point to "P". If the parsed string ends with a character "\" or is null
(apart from  drive name),  then there  will be no "last item", thus HL and DE
will  point to the same character. In this case, some special procedures will
be needed to all the programs which use this function.

   The  drive number returned in register C is the logical drive specified in
the string.  If the  string did not start with a drive letter then register C
will  contain the  default drive  number, since  the default  drive has  been
implicitly specified. Register C will never be zero.

   The  parse flags  returned in  register B  indicate various  useful things
about the  string. For  a volume  name bits  1, 4,  5, 6 and 7 will always be
clear. For a filename, bits 3 to 7 relate to the last item on the string (the
"filename" component). The bit assignments are as follows:

b0 - set if any characters parsed other than drive name
b1 - set if any directory path specified
b2 - set if drive name specified
b3 - set if main filename specified in last item
b4 - set if filename extension specified in last item
b5 - set if last item is ambiguous
b6 - set if last item is "." or ".."
b7 - set if last item is ".."

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-5ch", "_pfile,dos2-fn-parse-filename", "MSX-DOS 2: funcoes", "C = 5CH  (_PFILE)", "Funcao 5CH (_PFILE): Parse Filename",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.72   PARSE FILENAME (5CH)

     Parameters:    C = 5CH (_PFILE)
                   DE = ASCIIZ string for parsing
                   HL = Pointer to 11 byte buffer
     Results:       A = Error (always zero)
                   DE = Pointer to termination character
                   HL = Preserved, buffer filled in
                    B = Parse flags

   This function is purely a string manipulation function, it will not access
disks at  all and will not modify the string at all. It is intended mainly to
help  transient programs  in printing  out filenames  in a formatted way. The
ASCIIZ string will be parsed as a single filename item, and the filename will
be  stored  in the  user's 11  byte buffer  in expanded  form, with  both the
filename and the extension padded out with spaces.

   The parse  flags returned  in register  B are  identical to  those for the
"parse  pathname" function  above (function 5Bh), except that bits 0, 1 and 2
will always  be clear.  The user's  buffer will  always be filled in, even if
there  is no  valid filename  in the string, in which case the buffer will be
filled with spaces. "*" characters will be expanded to the appropriate number
of "?"s.  If either  the filename  or the filename extension is too long then
the excess characters will be ignored.

   The pointer  returned in  register DE will point to the first character in
the  string which  was not part of the filename, which may be the null at the
end of  the string.  This character  will never be a valid filename character
(see the "Command Specification" for details of valid filename characters).

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-5dh", "_chkchr,dos2-fn-check-character", "MSX-DOS 2: funcoes", "C = 5DH  (_CHKCHR)", "Funcao 5DH (_CHKCHR): Check Character",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.73   CHECK CHARACTER (5DH)

     Parameters:    C = 5DH (_CHKCHR)
                    D = Character flags
                    E = Character to be checked
     Results:       A = 0 (never returns an error)
                    D = Updated character flags
                    E = Checked (upper cased) character

   This  function allow  language independent  upper casing of characters and
also helps with handling 16-bit characters and manipulation of filenames. The
bit assignments in the character flags are as follows:

   b0   - set to suppress upper casing
   b1   - set if first byte of 16-bit character
   b2   - set if second byte of 16-bit character
   b3   - set => volume name (rather than filename)
   b4   - set => not a valid file/volume name character
b5...b7 - reserved (always clear)

   Bit 0  is used  to control upper casing. If it is clear then the character
will be upper cased according to the language setting of the machine. If this
bit  is  set  then the  returned character  will always  be the  same as  the
character passed.

   The two  16-bit character  flags (bits 1 and 2) can both be clear when the
first  character of  a string  is checked  and the  settings returned  can be
passed straight  back to  this function  for each  subsequent character. Care
must  be taken  with these  flags when moving backwards through strings which
may contain 16-bit characters.

   Bit  4 is  set on return if the character is one of the set of filename or
volume name  terminator characters. Bit 3 is simply used to determine whether
to  test for filename or volume name characters since the sets are different.
16-bit characters  (either byte)  are never  considered as volume or filename
terminators.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-5eh", "_wpath,dos2-fn-get-whole-path-string", "MSX-DOS 2: funcoes", "C = 5EH  (_WPATH)", "Funcao 5EH (_WPATH): Get Whole Path String",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.74   GET WHOLE PATH STRING (5EH)

     Parameters:    C = 5EH (_WPATH)
                        DE = Pointer to 64 byte buffer
     Results:       A = Error
                   DE = Filled in with whole path string
                   HL = Pointer to start of last item

   This  function simply copies an ASCIIZ path string from an internal buffer
into the  user's buffer.  The string  represents the whole path and filename,
from  the root  directory, of  a file  or sub-directory located by a previous
"find first entry" or "find new entry" function. The returned string will not
include a  drive, or  an initial "\" character. Register HL will point at the
first  character of  the last  item on  the string, exactly as for the "parse
path" function (function 5Bh).

   If a  "find first entry" or "find new entry" function call is done with DE
pointing to an ASCIIZ string then a subsequent "get whole path" function call
will  return a string representing the sub-directory or file corresponding to
the  fileinfo   block  returned   by  the  "find"  function.  If  this  is  a
sub-directory  then the  fileinfo block  may be passed back in register DE to
another "find  first entry"  function call,  which will  locate a file within
this  sub-directory. In  this case  the newly located file will be added onto
the already  existing whole  path string internally, and so a subsequent "get
whole  path string" function call will return a correct whole path string for
the located file.

   Great care must be taken in using this function because the internal whole
path  string is modified by many of the function calls, and in many cases can
be invalid.  The "get  whole path"  function call  should be done immediately
after  the  "find  first  entry"  or "find  new entry"  function to  which it
relates.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-5fh", "_flush,dos2-fn-flush-disk-buffers", "MSX-DOS 2: funcoes", "C = 5FH  (_FLUSH)", "Funcao 5FH (_FLUSH): Flush Disk Buffers",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.75   FLUSH DISK BUFFERS (5FH)

     Parameters:    C = 5FH (_FLUSH)
                    B = Drive number (0=current, FFH=all)
                    D = 00H => Flush only
                      = FFH => Flush and invalidate
     Results:       A = Error

   This  function flushes  any dirty disk buffers for the specified drive, or
for all drives if B=FFh. If register D is FFh then all buffers for that drive
will also be invalidated.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-60h", "_fork,dos2-fn-fork-to-child-process", "MSX-DOS 2: funcoes", "C = 60H  (_FORK)", "Funcao 60H (_FORK): Fork To Child Process",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.76   FORK TO CHILD PROCESS (60H)

     Parameters:    C = 60H (_FORK)
     Results:       A = Error
                    B = Process id of parent process

   This function  informs the  system that  a child  process is  about to  be
started.  Typically this  is a new program or sub-command being executed. For
example COMMAND2.COM does a "fork" function call before executing any command
or transient program.

A new set of file handles is created, and any current file handles which were
opened with the "inheritable" access mode bit set (see the "open file handle"
function -  function 43h),  are copied  into the new set of file handles. Any
file handles which were  opened with  the "inheritable" bit clear will not be
copied and so will not be  available to the child process.
The standard file handles (00h...05h) are inheritable and so these will be
copied.

   A new process id is allocated for the child process and the process id. of
the parent  process is  returned so  that a  later "join"  function call  can
switch  back to  the parent process. A ".NORAM" error can be produced by this
function if there is insufficient memory to duplicate the file handles.

   Because  the child  process now  has a  copy of  the previous file handles
rather than  the originals,  if one  of them is closed then the original will
remain  open. So  for example if the child process closes the standard output
file handle  (file handle number 1) an re-opens it to a new file, then when a
"join" function is done to return to the parent process the original standard
output channel will still be there.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-61h", "_join,dos2-fn-rejoin-parent-process", "MSX-DOS 2: funcoes", "C = 61H  (_JOIN)", "Funcao 61H (_JOIN): Rejoin Parent Process",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.77   REJOIN PARENT PROCESS (61H)

     Parameters:    C = 61H (_JOIN)
                    B = Process id of parent, or zero
     Results:       A = Error
                    B = Primary error code from child
                    C = Secondary error code from child

   This function  switches back  to the  specified parent process and returns
the  error code  which the child process terminated with in register B, and a
secondary error  code from the child in register C. Although the relationship
between  parent and  childprocesses is strictly one-to-one, this function can
jump back several levels by giving it a suitable process id. A ".IPROC" error
will be returned if the process id is invalid.

   The child  process's set  of file handles are automatically closed and the
parent  process's set of file handles becomes active again. Also any user RAM
segments which the child process had allocated will be freed.

   If the  process id  passed to  this function is zero then a partial system
re-initialisatin  is done. All file handles are closed and the standard input
and output handles re-opened and all user segments are freed. This should not
normally be  done by  a user  program if  it intends to return to the command
interpreter  since the  command interpreter will not be in a consistent state
after this.

   This function takes great care that the freeing of memory and adjusting of
process id  is done  before actually closing any file handles and thus before
accessing  the disk. This ensures that if a disk error occurs and is aborted,
the join  operation will  have been  done successfully. However if a "join 0"
produces a disk error which is aborted, then the re-initialization of default
file  handles will not have been done. In this case another "join 0" function
call should  be done  and this  will not attempt access disk (because all the
files have been closed) and so will be successful.

   Note that  if this function call is made via 0F37Dh then registers B and C
will  not return  the error  codes. This  is because  program termination and
abort handling  must be  done by the application program. The error code will
have  been passed  to the abort vector and code there must remember the error
code if  it needs  to. See the "terminate with error code" function (function
62h) for the meaning of the primary and secondary error code.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-62h", "_term,dos2-fn-terminate-with-error-code", "MSX-DOS 2: funcoes", "C = 62H  (_TERM)", "Funcao 62H (_TERM): Terminate With Error Code",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.78   TERMINATE WITH ERROR CODE (62H)

     Parameters:    C = 62H (_TERM)
                    B = Error code for termination
     Results:       Does not return

   This  function terminates the program with the specified error code, which
may be  zero indicating no error. This function call will never return to the
caller  (unless a  user abort  routine executes  forces it  to - see function
63h). The operation of this function is different depending on whether it was
called  from  the  MSX-DOS  environment  via  00005h or  from the  disk BASIC
environment via 0F37Dh.

   If called  via 00005h  then if  a user  abort routine  has been defined by
function  63h it  will be  called with  the specified  error code (and a zero
secondary error code). Assuming that this routine returns, or if there was no
user abort  routine defined,  then control  will be  passed back  to whatever
loaded  the transient program via a jump at location 00000h. This will almost
always  be  the command  interpreter, but  in some  cases it  may be  another
transient program.  The error  code will  be remembered by the system and the
next  "join" function  (function 61h)  which is  done will  return this error
code. The command interpreter will print an error message for any code in the
range 20h...FFh, but will not print a message for errors below this.

   If this function is called from the disk BASIC environment via 0F37Dh then
control  will be  passed to the abort vector at location "BREAKVECT". In this
environment there  is no  separately defined user abort routine and the error
code  must be  remembered by  the code at "BREAKVECT" because "join" will not
return the error code.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-63h", "_defab,dos2-fn-define-abort-exit-routine", "MSX-DOS 2: funcoes", "C = 63H  (_DEFAB)", "Funcao 63H (_DEFAB): Define Abort Exit Routine",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.79   DEFINE ABORT EXIT ROUTINE (63H)

     Parameters:    C = 63H (_DEFAB)
                   DE = Address of abort exit routine
                        0000H to un-define routine
     Results:       A = 0 (never generates errors)

   This function  is only  available when  called via  location 00005h in the
MSX-DOS  environment. It  cannot be  called at  location 0F37Dh from the disk
BASIC environment.

   If  register DE  is zero  then a  previously defined abort routine will be
undefined, otherwise  a new  one will  be defined.  The abort routine will be
called by the system whenever the transient program is about to terminate for
any  reason other  than a direct jump to location 0000h. Programs written for
MSX-DOS  2  should  exit  with a  "terminate with  error code"  function call
(function 061h) rather than a jump to location 0000h.

   The user  abort routine  will be  entered with the user stack active, with
IX,  IY and  the alternate  register set as it was when the function call was
made and  with the  whole TPA  paged in.  The termination  error code will be
passed to the routine in register A with a secondary error code in register B
and  if the  routine executes a "RET" then the values returned in registers A
and  B  will  be  stored as  the error  codes to  be returned  by the  "join"
function, and  normally printed out by the command interpreter. Alternatively
the  routine may jump to some warm start code in the transient program rather
than returning. The system will be in a perfectly stable state able to accept
any function calls.

   The primary  error code  passed to  the routine  in register A will be the
code  which the  program itself  passed to  the "terminate  with error  code"
function (which  may be  zero) if this is the reason for the termination. The
routine will also be called if a Ctrl-C or Ctrl-STOP is detected (".CTRLC" or
".STOP"  error), if  a disk error is aborted (".ABORT" error), or if an error
occurred  on  one of  the standard  input or  output channels  being accessed
through MSX-DOS function calls 01h...0Bh (".INERR" or ".OUTERR").

   The errors ".ABORT", ".INERR" and ".OUTERR" are generated by the system as
a  result of  some other  error. For  example a  ".ABORT" can  result from  a
".NRDY" error,  or a  ".INERR" can result from a ".EOF" error. In these cases
the original error code (".NRDY" or ".EOF") is passed to the abort routine in
register  B as  the secondary  error code.  For all  other errors there is no
secondary error code and register B will be zero.

   If the abort routine executes "POP HL : RET" (or equivalent) rather than a
simple  return,  then  control  will  pass  to  the  instruction  immediately
following the MSX-DOS call or BIOS call in which the error occurred. This may
be useful in conjunction with a disk error handler routine (see function 64h)
to  allow  an option  to abort  the current  MSX-DOS call  when a  disk error
occurs.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-64h", "_defer,dos2-fn-define-disk-error-handler-routine", "MSX-DOS 2: funcoes", "C = 64H  (_DEFER)", "Funcao 64H (_DEFER): Define Disk Error Handler Routine",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.80   DEFINE DISK ERROR HANDLER ROUTINE (64H)

     Parameters:    C = 64H (_DEFER)
                   DE = Address of disk error routine
                        0000H to un-define routine
     Results:       A = 0 (never generates errors)

   This function specifies the address of a user routine which will be called
if a  disk error  occurs. The routine will be entered with the full TPA paged
in, but with the system stack in page-3 active and none of the registers will
be preserved from when the MSX-DOS function call was made.

   The error routine can make MSX-DOS calls but must be very careful to avoid
recursion. The list of function calls in section 2 of this document indicates
which function  calls can  be safely  made from  a user  error routine.  This
routine  is called  with the redirection status being temporarily invalidated
in case  the standard  I/O channels  have been  redirected. See  the "get/set
redirection state" function (function 70h) for details of this.

   The  specification of  parameters and results for the routine itself is as
below. All  registers including  IX, IY and the alternate register set may be
destroyed but the paging and stack must be preserved. The routine must return
to the system, it must not jump away to continue the transient program. If it
wants to do this then it should return A=1 ("abort") and a user abort routine
will then get control and this may do whatever it wants to.

     Parameters:    A = Error code which caused error
                    B = Physical drive
                    C = b0 - set if writing
                        b1 - set if ignore not recommended
                        b2 - set if auto-abort suggested
                        b3 - set if sector number is valid
                   DE = Sector number (if b3 of C is set)
     Results:       A = 0 => Call system error routine
                        1 => Abort
                        2 => Retry
                        3 => Ignore

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-65h", "_error,dos2-fn-get-previous-error-code", "MSX-DOS 2: funcoes", "C = 65H  (_ERROR)", "Funcao 65H (_ERROR): Get Previous Error Code",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.81   GET PREVIOUS ERROR CODE (65H)

     Parameters:    C = 65H (_ERROR)
     Results:       A = 0
                    B = Error code from previous function

   This  function allows  a user  program to  find out  the error  code which
caused the  previous MSX-DOS  function call  to fail.  It is intended for use
with the old CP/M compatible functions which do not return an error code. For
example  if a  "create file  FCB" function  returns A=0FFh thee could be many
reasons  for  the  failure  and  doing  this  function  call will  return the
appropriate on, for example ".DRFUL" or ".SYSX".

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-66h", "_explain,dos2-fn-explain-error-code", "MSX-DOS 2: funcoes", "C = 66H  (_EXPLAIN)", "Funcao 66H (_EXPLAIN): Explain Error Code",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.82   EXPLAIN ERROR CODE (66H)

     Parameters:    C = 66H (_EXPLAIN)
                    B = Error code to be explained
                   DE = Pointer to 64 byte string buffer
     Results:       A = 0
                    B = 0 or unchanged
                   DE = Filled in with error message

   This  function allows  a user  program to get an ASCIIZ explanation string
for a  particular error  code returned by any of the MSX-DOS functions. If an
error comes from one of the old functions then "get previous error code" must
be  called first  to get  the real  error code  and then this function can be
called to get an explanation string.

   The "Program Interface Specification" contains a list of all the currently
defined  error codes  and the messages for them. Foreign language versions of
the system  will of  course have  different messages.  If the error code does
have  a built  in explanation  string then  this string  will be returned and
register B  will be  set to  zero. If  there is  no explanation string then a
string  of the  form: "System error 194" or "User error 45" will be returned,
and register  B will  be unchanged.  (System errors  are those  in the  range
40h...FFh and user errors are 00h...3Fh.)

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-67h", "_format,dos2-fn-format-a-disk", "MSX-DOS 2: funcoes", "C = 67H  (_FORMAT)", "Funcao 67H (_FORMAT): Format A Disk",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.83   FORMAT A DISK (67H)

     Parameters:    C = 67H (_FORMAT)
                    B = Drive number (0=>current, 1=>A:)
                    A =    00H    => return choice string
                        01H...09H => format this choice
                        0AH...FDH => illegal
                        FEH, FFH  => new boot sector
                   HL = Pointer to buffer (if A=1...9)
                   DE = Size of buffer (if A=1...9)
     Results:       A = Error
                    B = Slot of choice string (only if A=0 on entry)
                   HL = Address of choice string (only if A=0 on entry)

This function  is used  to format  disks and  is really only provided for the
"FORMAT"  command although other programs may use it (with care) if they find
it useful.  It has  three different  options which  are selected  by the code
passed in register A.

   If  A=0  then  registers  B  and  HL  return the  slot number  and address
respectively  of an ASCIIZ string which specifies the choice of formats which
is  available.  A ".IFORM"  error will  be returned  if this  disk cannot  be
formatted (for  example the RAM disk). Normally the string will be read using
the  "RDSLT" routine  and displayed  on the screen followed by a "? " prompt.
The user  then specifies a choice "1"..."9" and this choice is passed back to
the  "format" function,  after a  suitable warning prompt, to actually format
the disk. If A=0, in some cases zero is returned in HL. This means that there
is only  one kind of the format and no prompt is required. There is no way of
knowing  what  disk  format  a  particular  choice  refers to  since this  is
dependant on the particular disk driver.

   If A=01h...09h then this is interpreted as a format choice and a disk will
be formatted  in the  specified drive  with no further prompting. Register HL
and  DE must specify a buffer area to be used by the disk driver. There is no
way of  knowing how big this buffer should be so it is best to make it as big
as  possible. If  the buffer  crosses page boundaries then this function will
select the largest portion of it which is in one page for passing to the disk
driver. Many disk drivers do not use this buffer at all.

   If  A=FFh then  the disk  will not  actually be  formatted, but it will be
given a  new boot  sector to  make the  disk a  true MSX-DOS  2 disk. This is
designed  to update  old MSX-DOS 1.0 disks to have a volume id and thus allow
the full  disk checking and undeletion which MSX-DOS 2 allows. The case A=FEh
is  the  same  as  A=FFh  except that  only the  disk parameters  are updated
correctly  and the  volume id does not overwrite the boot program. Also there
are some  MSX-DOS 1.0  implementations which  put an incorrect boot sector on
the  disk and  these disks  cannot be  used by MSX-DOS 2 until they have been
corrected by this function.

   The  "new  boot  sector"  function  is mainly  intended for  the "FIXDISK"
utility program, but may be used by other programs if they find it useful. If
it  is used  then a  "get format  choice" function  call (A=0) should be done
first and  if this  returns an  error (typically ".IFORM") then the operation
should be aborted because this is a drive which does not like to be formatted
and the disk could be damaged by this function.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-68h", "_ramd,dos2-fn-create-or-destroy-ramdisk", "MSX-DOS 2: funcoes", "C = 68H  (_RAMD)", "Funcao 68H (_RAMD): Create Or Destroy Ramdisk",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.84   CREATE OR DESTROY RAMDISK (68H)

     Parameters:    C = 68H (_RAMD)
                    B =     00H => destroy RAM disk
                        1...FEH => create new RAM disk
                            FFH => return RAM disk size
     Results:       A = Error
                    B = RAM disk size

   If  register B=0FFh  then this  routine just returns the number of 16k RAM
segments which  are allocated  to the  RAM disk  currently. A  value of  zero
indicates  that  there  is no  RAM disk  currently defined.  If B=0  then the
current  RAM disk  will be destroyed, loosing all data which it contained and
no error will be returned if there was no RAM disk.

   Otherwise, if  B is in the range 01h...FEh then this function will attempt
to  create a  new RAM  disk using  the number  of 16k  segments specified  in
register  B.  An  error  will  be  returned if  there is  already a  RAM disk
(".RAMDX") or  if there is not even one segment free (".NORAM"). If there are
insufficient  free RAM segments to make a RAM disk of the specified size then
the largest one possible will be created. No error is returned in this case.

   In  all cases the size of the RAM disk will be returned in register B as a
number of segments. Note that some of the RAM is used for the file allocation
tables and  the root  directory so  the size  of the RAM disk as indicated by
"DIR" or "CHKDSK" will be somewhat smaller than the total amount of RAM used.
The  RAM will  always be  assigned the  drive letter  "H:" regardless  of the
number of drives in the system.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-69h", "_buffer,dos2-fn-allocate-sector-buffers", "MSX-DOS 2: funcoes", "C = 69H  (_BUFFER)", "Funcao 69H (_BUFFER): Allocate Sector Buffers",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.85   ALLOCATE SECTOR BUFFERS (69H)

     Parameters:    C = 69H (_BUFFER)
                    B = 0 => return number of buffers
                        else number of buffers required
     Results:       A = Error
                    B = Current number of buffers

   If  B=0 then this function just returns the number of sector buffers which
are currently  allocated. If B<>0 then this function will attempt to use this
number  of sector  buffers (must always be at least 2). If it cannot allocate
as many as requested then it will allocate as many as possible and return the
number in  register B  but will  not return  an error.  The number  of sector
buffers can be reduced as well as increased.

   The  sector buffers  are allocated in a 16k RAM segment outside the normal
64k so  the number  of buffers  does not  detract from  the size  of the TPA.
However  the number of buffers does affect efficiency since with more buffers
allow more  FAT and directory sectors to be kept resident. The maximum number
of buffers will be about 20.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-6ah", "_assign,dos2-fn-logical-drive-assignment", "MSX-DOS 2: funcoes", "C = 6AH  (_ASSIGN)", "Funcao 6AH (_ASSIGN): Logical Drive Assignment",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.86   LOGICAL DRIVE ASSIGNMENT (6AH)

     Parameters:    C = 6AH (_ASSIGN)
                    B = Logical drive number (1=A: etc)
                    D = Physical drive number (1=A: etc)
     Results:       A = Error
                    D = Physical drive number (1=A: etc)

   This  function controls the logical to physical drive assignment facility.
It is  primarily intended for the "ASSIGN" command although user programs may
want to use it to translate logical drive numbers to physical drive numbers.

   If both  B and  D are  non-zero then  a new  assignment will be set up. If
register  B is  non-zero and  register D  is zero then any assignment for the
logical drive  specified by B will be cancelled. If both register B and D are
zero  then all  assignments will  be cancelled. If register B is non-zero and
register D is FFh then the current assignment for the logical drive specified
by register B will simply be returned in register D.

   All  drives used  in the  various function calls, including drive names in
strings  and  drive  numbers  as parameters  to function  calls, are  logical
drives. However  the drive number passed to disk error routines is a physical
drive  so  if  "ASSIGN"  has  been  used  these  may  be  different  from the
corresponding logical drive.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-6bh", "_genv,dos2-fn-get-environment-item", "MSX-DOS 2: funcoes", "C = 6BH  (_GENV)", "Funcao 6BH (_GENV): Get Environment Item",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.87   GET ENVIRONMENT ITEM (6BH)

     Parameters:    C = 6BH (_GENV)
                   HL = ASCIIZ name string
                   DE = Pointer to buffer for value
                    B = Size of buffer
     Results:       A = Error
                   DE = Preserved, buffer filled in if A=0

   This function gets the current value of the environment item whose name is
passed  in register  HL. A  ".IENV" error  is returned  if the name string is
invalid. If there is no environment item of that name then a null string will
be returned  in the  buffer. If  there is an item of that name then its value
string  will be  copied to  the buffer.  If the  buffer is too small then the
value string  will be truncated with no terminating null and a ".ELONG" error
will  be returned. A buffer 255 bytes will always be large enough since value
strings cannot be longer than this (including the terminating null).

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-6ch", "_senv,dos2-fn-set-environment-item", "MSX-DOS 2: funcoes", "C = 6CH  (_SENV)", "Funcao 6CH (_SENV): Set Environment Item",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.88   SET ENVIRONMENT ITEM (6CH)

     Parameters:    C = 6CH (_SENV)
                   HL = ASCIIZ name string
                   DE = ASCIIZ value string
     Results:       A = Error

   This  function sets  a new environment item. If the name string is invalid
then a ".IENV" error is returned, otherwise the value string is checked and a
".ELONG" error  returned if  it is  longer than 255 characters, or a ".NORAM"
error  if there  is insufficient memory to store the new item. If all is well
then any  old item  of this  name is deleted and the new item is added to the
beginning  of the  environment list.  If the  value string  is null  then the
environment item will be removed.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-6dh", "_fenv,dos2-fn-find-environment-item", "MSX-DOS 2: funcoes", "C = 6DH  (_FENV)", "Funcao 6DH (_FENV): Find Environment Item",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.89   FIND ENVIRONMENT ITEM (6DH)

     Parameters:    C = 6DH (_FENV)
                   DE = Environment item number
                   HL = Pointer to buffer for name string
     Results:       A = Error
                   HL = Preserved, buffer filled in

   This  function is  used to  find out  what environment items are currently
set. The  item number  in register DE identifies which item in the list is to
be  found (the  first item  corresponds to  DE=1). If there is an item number
<DE> then the name string of this item will be copied into the buffer pointed
to by  HL. If the buffer is too small then the name will be truncated with no
terminating null, and a ".ELONG" error returned. A 255 byte buffer will never
be  too small.  If there  is no  item number  <DE> then a null string will be
returned, since an item can never have a null name string.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-6eh", "_dskchk,dos2-fn-get-set-disk-check-status", "MSX-DOS 2: funcoes", "C = 6EH  (_DSKCHK)", "Funcao 6EH (_DSKCHK): Get/Set Disk Check Status",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.90   GET/SET DISK CHECK STATUS (6EH)

     Parameters:    C = 6EH (_DSKCHK)
                    A = 00H => get disk check status
                        01H => set disk check status
                    B = 00H => enable (only if A=01H)
                        FFH => disable (only if A=01H)
     Results:       A = Error
                    B = Current disk check setting

   If A=0  then the  current value  of the disk check variable is returned in
register  B. If  A=01h then the variable is set to the value in register B. A
value of 00h means that disk checking is enabled and a non-zero means that it
is disabled. The default state is enabled.

   The disk check variable controls whether the system will re-check the boot
sector of  a disk  to see  whether it  has changed,  each time a file handle,
fileinfo  block  or  FCB  is  accessed.  If  it is  enabled then  it will  be
impossible  to accidentally  access the  wrong disk by changing a disk in the
middle of  an operation,  otherwise this will be possible and may result in a
corrupted  disk. Depending  on the  type of disk interface, there may be some
additional overhead  in having  this feature enabled although with many types
of  disk (those with explicit disk change detection hardware) it will make no
difference and the additional security is well worth having.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-6fh", "_dosver,dos2-fn-get-msx-dos-version-number", "MSX-DOS 2: funcoes", "C = 6FH  (_DOSVER)", "Funcao 6FH (_DOSVER): Get Msx-Dos Version Number",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.91   GET MSX-DOS VERSION NUMBER (6FH)

     Parameters:    C = 6FH (_DOSVER)
     Results:       A = Error (always zero)
                   BC = MSX-DOS kernel version
                        DE = MSXDOS2.SYS version number

   This function allows a program to determine which version of MSX-DOS it is
running  under. Two  version numbers  are returned, one in BC for the MSX-DOS
kernel in  ROM and  the other  is DE for the MSXDOS2.SYS system file. Both of
these  version numbers  are BCD  values with  the major version number in the
high byte  and the  two digit  version number in the low byte. For example if
there were a version 2.34 of the system, it would be represented as 0234h.

   For compatibility  with MSX-DOS 1.0, the following procedure should always
be followed in using this function. Firstly if there is any error (A<>0) then
it  is not  MSX-DOS at  all. Next  look at register B. If this is less than 2
then the system is earlier than 2.00 and registers C and DE are undefined. If
register B  is 2 or greater then registers BC and DE can be used as described
above.  In general  the version  number which  should be  checked (after this
procedure) is the MSXDOS2.SYS version in register DE.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("dos2-fn-70h", "_redir,dos2-fn-get-set-redirection-state", "MSX-DOS 2: funcoes", "C = 70H  (_REDIR)", "Funcao 70H (_REDIR): Get/Set Redirection State",
        R"MAN(MSX-DOS 2 (versao 2.20) - Function Specification

  3.92   GET/SET REDIRECTION STATE (70H)

     Parameters:    C = 70H (_REDIR)
                    A = 00H => get redirection state
                        01H => set redirection state
                    B = New state.   b0 - standard input
                                     b1 - standard output
     Results:       A = Error
                    B = Redirection state before command
                         b0 set => input is redirected
                         b1 set => output is redirected

   This  function is  provided primarily  for disk  error routines  and other
character  I/O  which  must  always  go  to  the  console  regardless  of any
redirection.  When  the CP/M  character functions  (functions 01h...0Bh)  are
used, they  normally refer  to the  console. However if the standard input or
output file handles (file handles 0 and 1) have been closed and reopened to a
disk  file, then  the CP/M character functions will also go to the disk file.
However certain output such as disk error output must always go to the screen
regardless.

   This  function allows  any such redirection to be temporarily cancelled by
calling this  function with A=1 and B=0. This will ensure that any subsequent
CP/M  console I/O  will go  to the console, and will also return the previous
setting so  that this  can be  restored afterwards.  The system is a somewhat
unstable  state when  the redirection  state has  been altered  like this and
there are  many function  calls which  will reset the redirection to its real
state   over-riding  this  function.  In  general  any  function  call  which
manipulates file  handles, such  as "open",  "close", "duplicate"  and so on,
will  reset the  redirection state.  The effect of this function is therefore
purely temporary.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("z80-overview", "", "Z80", "", "Z80: visao geral e extensoes do R800",
        R"MAN(Z80

Z80

The Z80 is 8 bit processor running with a frequency of 3.57945 Mhz.

Details:

Z80 command set

Z80 instruction set summary

Lex Lechz: the following table lists the extensions from the Z80 to the
R800 processor.
These extensions are implemented in RuMSX.

R800-Processor (16 Bit, 7.15909 Mhz) opcodes

Please also be careful with illegal instructions. Officially 8 bit access to
the IX and IY register isn't guaranteed on the Z80. Using the R800 no problems
will arise. However , the object code of LD IXH,IXL is DD 65 but the assembler
will probably create DD DD 65. On the Z80 this code will run ok but this is
not guaranteed that it will work proper on the R800. So be careful with
illegal instructions.

R800-Processor (16 Bit, 7.15909 Mhz) opcodes (excerpt)

Instruction | Operation | C | Z | P/V | S | N | H | Opcode | M-Cycles | T-States
MULUW HL,BC | DEHL <- HL*BC | Chg | Chg | 0 | 0 | NoChg | NoChg | ED C3 | 36 | 36
MULUW HL,SP | DEHL <- HL*SP | Chg | Chg | 0 | 0 | NoChg | NoChg | ED F3 | 36 | 36
MULU A,B | HL <- A*B | Chg | Chg | 0 | 0 | NoChg | NoChg | ED C1 | 14 | 14
MULU A,C | HL <- A*C | Chg | Chg | 0 | 0 | NoChg | NoChg | ED C9 | 14 | 14
MULU A,D | HL <- A*D | Chg | Chg | 0 | 0 | NoChg | NoChg | ED D1 | 14 | 14
MULU A,E | HL <- A*E | Chg | Chg | 0 | 0 | NoChg | NoChg | ED D9 | 14 | 14

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("z80-opcodes", "z80-instruction-set", "Z80", "", "Z80: conjunto de instrucoes por codigo (tabela)",
        R"MAN(Z80

Z80 Instruction set summary

 Table A-3. Z80 Mnemonics and Operations in Alphabetical Order
+------------+--------------------+------+---+---+----------------------+
|Mnemonic    |Operation           |CZPSNH|Len|Tim|Comments              |
+------------+--------------------+------+---+---+----------------------+
|ADC A,r     |A:=A+r+CY           |V*0*| 1 | 4 | r - A,B,C,D,E,H,L    |
|ADC A,(HL)  |A:=A+(HL)+CY        |      | 1 | 7 |                      |
|ADC A,n     |A:=A+n+CY           |      | 2 | 7 | n - byte (0..FF)     |
|ADC A,(ii+n)|A:=A+(ii+n)+CY      |      | 3 |19 |ii - IX,IY            |
|ADC HL,rr   |HL:=HL+rr+CY        |V*0x| 2 |15 |rr - BC,DE,HL,SP      |
+------------+--------------------+------+---+---+----------------------+
|ADD A,r     |A:=A+r              |V*0*| 1 | 4 |                      |
|ADD A,(HL)  |A:=A+(HL)           |      | 1 | 7 |                      |
|ADD A,n     |A:=A+n              |      | 2 | 7 |                      |
|ADD A,(ii+n)|A:=A+(ii+n)         |      | 3 |19 |                      |
|ADD HL,rr   |HL:=HL+rr           |*...0x| 1 |11 |                      |
|ADD IX,ry   |IX:=IX+px           |      | 2 |15 |ry - BC,DE,SP,IY      |
|ADD IY,rx   |IY:=IY+py           |      | 2 |15 |rx - BC,DE,SP,IX      |
+------------+--------------------+------+---+---+----------------------+
|AND r       |A:=A and r          |0*P*01| 1 | 4 |                      |
|AND (HL)    |A:=A and (HL)       |      | 1 | 7 |                      |
|AND n       |A:=A and n          |      | 2 | 7 |                      |
|AND (ii+n)  |A:=A and (ii+n)     |      | 3 |19 |                      |
+------------+--------------------+------+---+---+----------------------+
|BIT b,r     |Z:=not rb           |.*xx01| 2 | 8 | b - bit number (0..7)|
|BIT b,(HL)  |Z:=not (HL)b        |      | 2 |12 |xb - bit b of         |
|BIT b,(ii+n)|Z:=not (ii+n)b      |      | 4 |20 |          location x  |
+------------+--------------------+------+---+---+----------------------+
|CALL nn     |PUSH PC;PC:=nn      |......| 3 |17 |nn - word (0..FFFF)   |
|CALL cc,nn  |If cc then CALL nn  |      | 3 |17 |cc - C,NC,Z,NZ,M,P,PE,|
|            |      else continue |      |   |10 |                   PO |
+------------+--------------------+------+---+---+----------------------+
|CCF         |CY:=not CY          |*...0x| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|CP r        |A-r                 |V*1*| 1 | 4 |                      |
|CP (HL)     |A-(HL)              |      | 1 | 7 |                      |
|CP n        |A-n                 |      | 2 | 7 |                      |
|CP (ii+n)   |A-(ii+n)            |      | 3 |19 |                      |
+------------+--------------------+------+---+---+----------------------+
|CPD         |A-(HL);dec HL;dec BC|.*1*| 2 |16 |PV=0 if BC=0,else PV=1|
+------------+--------------------+------+---+---+----------------------+
|CPDR        |Repeat CPD          |.*1*| 2 |21 |                      |
|            | until Z=1 or BC=0  |      |   |16 |                      |
+------------+--------------------+------+---+---+----------------------+
|CPI         |A-(HL);inc HL;dec BC|.*1*| 2 |16 |PV=0 if BC=0,else PV=1|
+------------+--------------------+------+---+---+----------------------+
|CPIR        |Repeat CPI          |.*1*| 2 |21 |                      |
|            | until Z=1 or BC=0  |      |   |16 |                      |
+------------+--------------------+------+---+---+----------------------+
|CPL         |A:=A xor 255        |....11| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|DAA         |Decimal adjust Acc. |P*.*| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|DEC r       |r:=r-1              |.*V*1*| 1 | 4 |                      |
|DEC (HL)    |(HL):=(HL)-1        |      | 1 |11 |                      |
|DEC (ii+n)  |(ii+n):=(ii+n)-1    |      | 3 |23 |                      |
|DEC rr      |rr:=rr-1            |......| 1 | 6 |                      |
|DEC ii      |ii:=ii-1            |      | 2 |10 |                      |
+------------+--------------------+------+---+---+----------------------+
|DI          |IFF:=0              |......| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|DJNZ e      |dec B;if B 0 JR e   |......| 2 |13 | e - relative address |
|            |     if B=0 continue|      |   | 8 |                      |
+------------+--------------------+------+---+---+----------------------+
|EI          |IFF:=1              |......| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|EX AF,AF'   |AF<->AF'            |......| 1 | 4 |                      |
|EX DE,HL    |DE<->HL             |      | 1 | 4 |                      |
|EX (SP),HL  |(SP)<->HL           |      | 1 | 4 |                      |
|EX (SP),ii  |(SP)<->ii           |      | 2 |23 |                      |
+------------+--------------------+------+---+---+----------------------+
|EXX         |BC<->BC';DE<->DE';  |......| 1 | 4 |                      |
|            |HL<->HL'            |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|HALT        |Halt CPU            |......| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|IM 1        |Interrupt mode 1    |......| 2 | 8 |                      |
|IM 2        |Interrupt mode 2    |      |   |   |                      |
|IM 3        |Interrupt mode 3    |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|IN A,(n)    |A:=port(n)          |......| 2 |11 |                      |
|IN r,(C)    |r:=port(C)          |.*P*0*| 2 |12 |                      |
|IN ?,(C)    |only set flags as   |.*P*0*| 2 |12 |can't be entered as a |
|            |   as IN r,(C) does |      |   |   |command;code is ED 70 |
+------------+--------------------+------+---+---+----------------------+
|INC r       |r:=r+1              |.*V*0*| 1 | 4 |                      |
|INC (HL)    |(HL):=(HL)+1        |      | 1 |11 |                      |
|INC (ii+n)  |(ii+n):=(ii+n)+1    |      | 3 |23 |                      |
|INC rr      |rr:=rr+1            |......| 1 | 6 |                      |
|INC ii      |ii:=ii+1            |      | 2 |10 |                      |
+------------+--------------------+------+---+---+----------------------+
|IND         |(HL):=port(C);      |x*xx1x| 2 |16 |Z=1 if B=0,else Z=0   |
|            |dec HL;dec B        |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|INDR        |Repeat IND          |x1xx1x| 2 |21 |                      |
|            | until B=0          |      |   |16 |                      |
+------------+--------------------+------+---+---+----------------------+
|INI         |(HL):=port(C);      |x*xx1x| 2 |16 |Z=1 if B=0,else Z=0   |
|            |inc HL;dec B        |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|INIR        |Repeat INI          |x1xx1x| 2 |21 |                      |
|            | until B=0          |      |   |16 |                      |
+------------+--------------------+------+---+---+----------------------+
|JP nn       |PC:=nn              |......| 3 |10 |                      |
|JP cc,nn    |If cc then JP nn    |......| 3 |10 |                      |
|JP (HL)     |PC:=HL              |      | 1 | 4 |                      |
|JP (ii)     |PC:=ii              |      | 2 | 8 |                      |
+------------+--------------------+------+---+---+----------------------+
|JR e        |PC:=PC+e            |......| 2 |12 |                      |
|JR cond,e   |If cond then JR e   |      | 2 |12 |cond - C,NC,Z,NZ      |
|            |        else NOP    |      |   | 7 |                      |
+------------+--------------------+------+---+---+----------------------+
|LD r,r      |r:=r                |......| 1 | 4 |                      |
|LD r,(HL)   |r:=(HL)             |      | 1 | 7 |                      |
|LD r,n      |r:=n                |      | 2 | 7 |                      |
|LD r,(ii+n) |r:=(ii+n)           |      | 3 |19 |                      |
|LD (HL),r   |(HL):=r             |      | 1 | 7 |                      |
|LD (ii+n),r |(ii+n):=r           |      | 3 |19 |                      |
|LD (HL),n   |(HL):=n             |      | 2 |10 |                      |
|LD (ii+n),n |(ii+n):=n           |      | 4 |19 |                      |
|LD A,(BC)   |A:=(BC)             |      | 1 | 7 |                      |
|LD A,(DE)   |A:=(DE)             |      | 1 | 7 |                      |
|LD A,(nn)   |A:=(nn)             |      | 3 |13 |                      |
|LD (BC),A   |(BC):=A             |      | 1 | 7 |                      |
|LD (DE),A   |(DE):=A             |      | 1 | 7 |                      |
|LD (nn),A   |(nn):=A             |      | 3 |13 |                      |
|LD A,I      |A:=I                |.*00| 2 | 9 |PV=IFF                |
|LD A,R      |A:=R                |      | 2 | 9 |PV=IFF                |
|LD I,A      |I:=A                |......| 2 | 9 |                      |
|LD R,A      |R:=A                |      | 2 | 9 |                      |
|LD rr,nn    |rr:=nn              |      | 3 |10 |                      |
|LD ii,nn    |ii:=nn              |      | 4 |14 |                      |
|LD HL,(nn)  |HL:=(nn)            |      | 3 |16 |                      |
|LD rr,(nn)  |rr:=(nn)            |      | 4 |20 |                      |
|LD ii,(nn)  |ii:=(nn)            |      | 4 |20 |                      |
|LD (nn),HL  |(nn):=HL            |      | 3 |16 |                      |
|LD (nn),rr  |(nn):=rr            |      | 4 |20 |                      |
|LD (nn),ii  |(nn):=ii            |      | 4 |20 |                      |
|LD SP,HL    |SP:=HL              |      | 1 | 6 |                      |
|LD SP,ii    |SP:=ii              |      | 1 |10 |                      |
+------------+--------------------+------+---+---+----------------------+
|LDD         |(DE):=(HL);         |..*.00| 2 |16 |PV=0 if BC=0,else PV=1|
|            |dec DE,HL,BC        |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|LDDR        |Repeat LDD          |..0.00| 2 |21 |                      |
|            | until Z=1 or BC=0  |      |   |16 |                      |
+------------+--------------------+------+---+---+----------------------+
|LDI         |(DE):=(HL);         |..*.00| 2 |16 |PV=0 if BC=0,else PV=1|
|            |inc DE,HL;dec BC    |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|LDIR        |Repeat LDI          |..0.00| 2 |21 |                      |
|            | until Z=1 or BC=0  |      |   |16 |                      |
+------------+--------------------+------+---+---+----------------------+
|NEG         |A:=0-A              |V*1*| 2 | 8 |                      |
+------------+--------------------+------+---+---+----------------------+
|NOP         |No operation        |......| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|OR r        |A:=A or r           |0*P*00| 1 | 4 |                      |
|OR (HL)     |A:=A or (HL)        |      | 1 | 7 |                      |
|OR n        |A:=A or n           |      | 2 | 7 |                      |
|OR (ii+n)   |A:=A or (ii+n)      |      | 3 |19 |                      |
+------------+--------------------+------+---+---+----------------------+
|OTDR        |Repeat OUTD         |x1xx1x| 2 |21 |                      |
|            | until B=0          |      |   |16 |                      |
+------------+--------------------+------+---+---+----------------------+
|OTIR        |Repeat OUTI         |x1xx1x| 2 |21 |                      |
|            | until B=0          |      |   |16 |                      |
+------------+--------------------+------+---+---+----------------------+
|OUT (n),A   |port(n):=A          |......| 2 |11 |                      |
|OUT (C),r   |port(C):=r          |      | 2 |12 |                      |
+------------+--------------------+------+---+---+----------------------+
|OUTD        |port(C):=(HL);      |x*xx1x| 2 |16 |Z=1 if B=0,else Z=0   |
|            |dec HL;dec B        |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|OUTI        |port(C):=(HL);      |x*xx1x| 2 |16 |Z=1 if B=0,else Z=0   |
|            |inc HL;dec B        |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|POP qq      |qq:=(SP);SP:=SP+2   |......| 1 |10 |qq - AF,BC,DE,HL      |
|POP ii      |ii:=(SP);SP:=SP+2   |      | 2 |14 |                      |
+------------+--------------------+------+---+---+----------------------+
|PUSH qq     |SP:=SP-2;(SP):=qq   |......| 1 |11 |                      |
|PUSH ii     |SP:=SP-2;(SP):=ii   |      | 2 |15 |                      |
+------------+--------------------+------+---+---+----------------------+
|RES b,r     |rb:=0               |......| 2 | 8 |                      |
|RES b,(HL)  |(HL)b:=0            |      | 2 |15 |                      |
|RES b,(ii+n)|(ii+n)b:=0          |      | 4 |23 |                      |
+------------+--------------------+------+---+---+----------------------+
|RET         |POP PC              |......| 1 |10 |                      |
|RET cc      |If cc then RET      |......| 1 |11 |                      |
|            |      else NOP      |      |   | 5 |                      |
+------------+--------------------+------+---+---+----------------------+
|RETI        |Return from interr. |......| 2 |14 |                      |
+------------+--------------------+------+---+---+----------------------+
|RETN        |Return from NMI     |......| 2 |14 |                      |
+------------+--------------------+------+---+---+----------------------+
|RL r        |+------->------+    |P*00| 2 | 8 |                      |
|RL (HL)     ||+--+ +-------+|    |      | 2 |15 |                      |
|RL (ii+n)   |++CY+<+7 <-- 0++    |      | 4 |23 |                      |
+------------+ +--+ +-------+     +------+---+---+----------------------+
|RLA         |                    |*...00| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|RLC r       |     +---->----+    |P*00| 2 | 8 |                      |
|RLC (HL)    |+--+ |+-------+|    |      | 2 |15 |                      |
|RLC (ii+n)  ||CY+<++7 <-- 0++    |      | 4 |23 |                      |
+------------++--+  +-------+     +------+---+---+----------------------+
|RLCA        |                    |*...00| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|RLD         | A    +---->---+(HL)|.*P*00| 2 |18 |                      |
|            |+---+-+-++---+-+-+  |      |   |   |                      |
|            ||7 4|3 0||7 4|3 0|  |      |   |   |                      |
|            |+---+-+-++++-+-+-+  |      |   |   |                      |
|            |      +-<-++-<-+    |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|RR r        |+------<-------+    |P*00| 2 | 8 |                      |
|RR (HL)     ||+-------+ +--+|    |      | 2 |15 |                      |
|RR (ii+n)   |++7 --> 0+>+CY++    |      | 4 |23 |                      |
+------------+ +-------+ +--+     +------+---+---+----------------------+
|RRA         |                    |*...00| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|RRC r       |+---<-----+         |P*00| 2 | 8 |                      |
|RRC (HL)    ||+-------+| +--+    |      | 2 |15 |                      |
|RRC (ii+n)  |++7 --> 0++>+CY|    |      | 4 |23 |                      |
+------------+ +-------+  +--+    +------+---+---+----------------------+
|RRCA        |                    |*...00| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|RRD         | A    +---<----+(HL)|.*P*00| 2 |18 |                      |
|            |+---+-+-++---+-+-+  |      |   |   |                      |
|            ||7 4|3 0||7 4|3 0|  |      |   |   |                      |
|            |+---+-+-++++-+-+-+  |      |   |   |                      |
|            |      +->-++->-+    |      |   |   |                      |
+------------+--------------------+------+---+---+----------------------+
|RST adr     |CALL adr            |......| 1 |11 |adr - byte (000xxx00b)|
+------------+--------------------+------+---+---+----------------------+
|SBC A,r     |A:=A-r-CY           |V*1*| 1 | 4 |                      |
|SBC A,(HL)  |A:=A-(HL)-CY        |      | 1 | 7 |                      |
|SBC A,n     |A:=A-n-CY           |      | 2 | 7 |                      |
|SBC A,(ii+n)|A:=A-(ii+n)-CY      |      | 3 |19 |                      |
|SBC HL,rr   |HL:=HL-rr-CY        |V*1x| 2 |15 |                      |
+------------+--------------------+------+---+---+----------------------+
|SCF         |CY:=1               |1...00| 1 | 4 |                      |
+------------+--------------------+------+---+---+----------------------+
|SET b,r     |rb:=1               |......| 2 | 8 |                      |
|SET b,(HL)  |(HL)b:=1            |      | 2 |15 |                      |
|SET b,(ii+n)|(ii+n)b:=1          |      | 4 |23 |                      |
+------------+--------------------+------+---+---+----------------------+
|SLA r       |+--+ +-------+      |P*00| 2 | 8 |                      |
|SLA (HL)    ||CY+<+7 <-- 0+<0    |      | 2 |15 |                      |
|SLA (ii+n)  |+--+ +-------+      |      | 4 |23 |                      |
+------------+--------------------+------+---+---+----------------------+
|SRA r       |  +-------+ +--+    |P*00| 2 | 8 |                      |
|SRA (HL)    |+>+7 --> 0+>+CY|    |      | 2 |15 |                      |
|SRA (ii+n)  |+-+-------+ +--+    |      | 4 |23 |                      |
+------------+--------------------+------+---+---+----------------------+
|SRL r       |   +-------+ +--+   |P*00| 2 | 8 |                      |
|SRL (HL)    | 0>+7 --> 0+>+CY|   |      | 2 |15 |                      |
|SRL (ii+n)  |   +-------+ +--+   |      | 4 |23 |                      |
+------------+--------------------+------+---+---+----------------------+
|SUB r       |A:=A-r              |V*0*| 1 | 4 |                      |
|SUB (HL)    |A:=A-(HL)           |      | 1 | 7 |                      |
|SUB n       |A:=A-n              |      | 2 | 7 |                      |
|SUB (ii+n)  |A:=A-(ii+n)         |      | 3 |19 |                      |
+------------+--------------------+------+---+---+----------------------+
|XOR r       |A:=A xor r          |0*P*00| 1 | 4 |                      |
|XOR (HL)    |A:=A xor (HL)       |      | 1 | 7 |                      |
|XOR n       |A:=A xor n          |      | 2 | 7 |                      |
|XOR (ii+n)  |A:=A xor (ii+n)     |      | 3 |19 |                      |
+------------+--------------------+------+---+---+----------------------+

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("z80-mnemonics", "", "Z80", "", "Z80: conjunto de instrucoes por mnemonico (tabela)",
        R"MAN(Z80

+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
|    |  00  |  01  |  02  |  03  |  04  |  05  |  06  |  07  |  08  |  09  |  0A  |  0B  |  0C  |  0D  |  0E  |  0F  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 00 |  NOP |  LD  |  LD  | INC  | INC  | DEC  |  LD  | RLCA |  EX  | ADD  |  LD  | DEC  | INC  | DEC  |  LD  | RRCA |
|    |      |bc,nn |(bc),a|  bc  |  b   |  b   | b,n  |      |af,af'| hl,bc|a,(bc)|  bc  |  c   |  c   |  c,n |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 10 | DJNZ |  LD  |  LD  | INC  | INC  | DEC  |  LD  |  RLA |  JR  | ADD  |  LD  | DEC  | INC  | DEC  |  LD  |  RRA |
|    |  dis | de,nn|(de),a|  de  |  d   |  d   | d,n  |      |  dis | hl,de|a,(de)|  de  |  e   |  e   |  e,n |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 20 |  JR  |  LD  |  LD  | INC  | INC  | DEC  |  LD  |  DAA |  JR  | ADD  |  LD  | DEC  | INC  | DEC  |  LD  |  CPL |
|    |nz,dis| hl,nn|(nn)hl|  hl  |  h   |  h   | h,n  |      | z,dis| hl,hl|hl(nn)|  hl  |  l   |  l   |  l,n |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 30 |  JR  |  LD  |  LD  | INC  | INC  | DEC  |  LD  |  SCF |  JR  | ADD  |  LD  | DEC  | INC  | DEC  |  LD  |  CCF |
|    |nc,dis| sp,nn|(nn),a|  sp  | (hl) | (hl) |(hl),n|      | c,dis| hl,sp|a,(nn)|  sp  |  a   |  a   |  a,n |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 40 |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |
|    | b,b  | b,c  | b,d  | b,e  | b,h  | b,l  |b,(hl)| b,a  | c,b  | c,c  | c,d  | c,e  | c,h  | c,l  |c,(hl)| c,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 50 |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |
|    | d,b  | d,c  | d,d  | d,e  | d,h  | d,l  |d,(hl)| d,a  | e,b  | e,c  | e,d  | e,e  | e,h  | e,l  |e,(hl)| e,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 60 |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |
|    | h,b  | h,c  | h,d  | h,e  | h,h  | h,l  |h,(hl)| h,a  | l,b  | l,c  | l,d  | l,e  | l,h  | l,l  |l,(hl)| l,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 70 |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  | HALT |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |  LD  |
|    |(hl),b|(hl),c|(hl),d|(hl),e|(hl),h|(hl),l|      |(hl),a| a,b  | a,c  | a,d  | a,e  | a,h  | a,l  |a,(hl)| a,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 80 | ADD  | ADD  | ADD  | ADD  | ADD  | ADD  | ADD  | ADD  | ADC  | ADC  | ADC  | ADC  | ADC  | ADC  | ADC  | ADC  |
|    | a,b  | a,c  | a,d  | a,e  | a,h  | a,l  |a,(hl)| a,a  | a,b  | a,c  | a,d  | a,e  | a,h  | a,l  |a,(hl)| a,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 90 | SUB  | SUB  | SUB  | SUB  | SUB  | SUB  | SUB  | SUB  | SBC  | SBC  | SBC  | SBC  | SBC  | SBC  | SBC  | SBC  |
|    |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   | a,b  | a,c  | a,d  | a,e  | a,h  | a,l  |a,(hl)|  a   |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| A0 | AND  | AND  | AND  | AND  | AND  | AND  | AND  | AND  | XOR  | XOR  | XOR  | XOR  | XOR  | XOR  | XOR  | XOR  |
|    |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| B0 |  OR  |  OR  |  OR  |  OR  |  OR  |  OR  |  OR  |  OR  |  CP  |  CP  |  CP  |  CP  |  CP  |  CP  |  CP  |  CP  |
|    |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| C0 |  RET |  POP |  JP  |  JP  | CALL | PUSH |  ADD |  RST |  RET |  RET |  JP  |  | CALL | CALL | ADC  | RST  |
|    |  nz  |  bc  | nz,nn|  nn  | nz,nn|  bc  |  a,n |   0h |   z  |      | z,nn |      | z,nn |  nn  | a,n  |  8h  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| D0 |  RET |  POP |  JP  |  OUT | CALL | PUSH |  SUB |  RST |  RET |  EXX |  JP  |  IN  | CALL |  | SBC  | RST  |
|    |  nc  |  de  | nc,nn| (n),a| nc,nn|  de  |   n  |  10h |   c  |      | c,nn | a,(n)| c,nn |      | a,n  | 18h  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| E0 |  RET |  POP |  JP  |  EX  | CALL | PUSH |  AND |  RST |  RET |  JP  |  JP  |  EX  | CALL |  | XOR  | RST  |
|    |  po  |  hl  | po,nn|(sp)hl| po,nn|  hl  |   n  |  20h |  pe  | (hl) | pe,nn| de,hl| pe,nn|      |  n   | 28h  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| F0 |  RET |  POP |  JP  |  DI  | CALL | PUSH |  OR  |  RST |  RET |  LD  |  JP  |  EI  | CALL |  |  CP  | RST  |
|    |  p   |  af  | p,nn |      | p,nn |  af  |   n  |  30h |   m  |sp,hl | m,nn |      | m,nn |      |  n   | 38h  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+

+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| CB |  00  |  01  |  02  |  03  |  04  |  05  |  06  |  07  |  08  |  09  |  0A  |  0B  |  0C  |  0D  |  0E  |  0F  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 00 | RLC  | RLC  | RLC  | RLC  | RLC  | RLC  | RLC  | RLC  | RRC  | RRC  | RRC  | RRC  | RRC  | RRC  | RRC  | RRC  |
|    |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 10 |  RL  |  RL  |  RL  |  RL  |  RL  |  RL  |  RL  |  RL  |  RR  |  RR  |  RR  |  RR  |  RR  |  RR  |  RR  |  RR  |
|    |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 20 | SLA  | SLA  | SLA  | SLA  | SLA  | SLA  | SLA  | SLA  | SRA  | SRA  | SRA  | SRA  | SRA  | SRA  | SRA  | SRA  |
|    |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 30 |      |      |      |      |      |      |      |      | SRL  | SRL  | SRL  | SRL  | SRL  | SRL  | SRL  | SRL  |
|    |      |      |      |      |      |      |      |      |  b   |  c   |  d   |  e   |  h   |  l   | (hl) |  a   |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 40 | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  |
|    | 0,b  | 0,c  | 0,d  | 0,e  | 0,h  | 0,l  |0,(hl)| 0,a  | 1,b  | 1,c  | 1,d  | 1,e  | 1,h  | 1,l  |1,(hl)| 1,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 50 | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  |
|    | 2,b  | 2,c  | 2,d  | 2,e  | 2,h  | 2,l  |2,(hl)| 2,a  | 3,b  | 3,c  | 3,d  | 3,e  | 3,h  | 3,l  |3,(hl)| 3,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 60 | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  |
|    | 4,b  | 4,c  | 4,d  | 4,e  | 4,h  | 4,l  |4,(hl)| 4,a  | 5,b  | 5,c  | 5,d  | 5,e  | 5,h  | 5,l  |5,(hl)| 5,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 70 | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  | BIT  |
|    | 6,b  | 6,c  | 6,d  | 6,e  | 6,h  | 6,l  |6,(hl)| 6,a  | 7,b  | 7,c  | 7,d  | 7,e  | 7,h  | 7,l  |7,(hl)| 7,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 80 | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  |
|    | 0,b  | 0,c  | 0,d  | 0,e  | 0,h  | 0,l  |0,(hl)| 0,a  | 1,b  | 1,c  | 1,d  | 1,e  | 1,h  | 1,l  |1,(hl)| 1,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 90 | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  |
|    | 2,b  | 2,c  | 2,d  | 2,e  | 2,h  | 2,l  |2,(hl)| 2,a  | 3,b  | 3,c  | 3,d  | 3,e  | 3,h  | 3,l  |3,(hl)| 3,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| A0 | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  |
|    | 4,b  | 4,c  | 4,d  | 4,e  | 4,h  | 4,l  |4,(hl)| 4,a  | 5,b  | 5,c  | 5,d  | 5,e  | 5,h  | 5,l  |5,(hl)| 5,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| B0 | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  | RES  |
|    | 6,b  | 6,c  | 6,d  | 6,e  | 6,h  | 6,l  |6,(hl)| 6,a  | 7,b  | 7,c  | 7,d  | 7,e  | 7,h  | 7,l  |7,(hl)| 7,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| C0 | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  |
|    | 0,b  | 0,c  | 0,d  | 0,e  | 0,h  | 0,l  |0,(hl)| 0,a  | 1,b  | 1,c  | 1,d  | 1,e  | 1,h  | 1,l  |1,(hl)| 1,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| D0 | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  |
|    | 2,b  | 2,c  | 2,d  | 2,e  | 2,h  | 2,l  |2,(hl)| 2,a  | 3,b  | 3,c  | 3,d  | 3,e  | 3,h  | 3,l  |3,(hl)| 3,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| E0 | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  |
|    | 4,b  | 4,c  | 4,d  | 4,e  | 4,h  | 4,l  |4,(hl)| 4,a  | 5,b  | 5,c  | 5,d  | 5,e  | 5,h  | 5,l  |5,(hl)| 5,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| F0 | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  | SET  |
|    | 6,b  | 6,c  | 6,d  | 6,e  | 6,h  | 6,l  |6,(hl)| 6,a  | 7,b  | 7,c  | 7,d  | 7,e  | 7,h  | 7,l  |7,(hl)| 7,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+

+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| ED |  00  |  01  |  02  |  03  |  04  |  05  |  06  |  07  |  08  |  09  |  0A  |  0B  |  0C  |  0D  |  0E  |  0F  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 00 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 10 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 20 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 30 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 40 |  IN  | OUT  | SBC  |  LD  | NEG  | RETN |  IM  |  LD  |  IN  | OUT  | ADC  |  LD  |      | RETI |      |  LD  |
|    | b,(c)| (c),b| hl,bc|(nn)bc|      |      |  0   | i,a  | c,(c)|(c),c | hl,bc|bc(nn)|      |      |      | r,a  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 50 |  IN  | OUT  | SBC  |  LD  |      |      |  IM  |  LD  |  IN  | OUT  | ADC  |  LD  |      |      |  IM  |  LD  |
|    | d,(c)| (c),d| hl,de|(nn)de|      |      |  1   | a,i  | e,(c)|(c),e | hl,de|de(nn)|      |      |  2   | a,r  |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 60 |  IN  | OUT  | SBC  |  LD  |      |      |      | RRD  |  IN  | OUT  | ADC  |  LD  |      |      |      | RLD  |
|    | h,(c)| (c),h| hl,hl|(nn)hl|      |      |      |      | l,(c)|(c),l | hl,hl|hl(nn)|      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 70 |      |      | SBC  |  LD  |      |      |      |      |  IN  | OUT  | ADC  |  LD  |      |      |      |      |
|    |      |      | hl,sp|(nn)sp|      |      |      |      | a,(c)|(c),a | hl,sp|sp(nn)|      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 80 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| 90 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| A0 | LDI  | CPI  | INI  | OUTI |      |      |      |      | LDD  | CPD  | IND  | OUTD |      |      |      |      |
|    |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| B0 | LDIR | CPIR | INIR | OTIR |      |      |      |      | LDDR | CPDR | INDR | OUTR |      |      |      |      |
|    |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| C0 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| D0 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| E0 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+
| F0 |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
+----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+

     DD -- HL is replaced by IX
     FD -- HL is replaced by IY c1

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("r800-instructions", "", "R800", "", "R800: quadro das instrucoes (Appendix A)",
        R"MAN(R800

------------------------------------------------------------------------
                          A
                R800 INSTRUCTION CHARTS
------------------------------------------------------------------------
In these charts instructions of the R800, grouped in kind of instructions
are collected. Of the mnemonics in these charts the name of the instruction,
usage of this instruction are smplified showed.
In the column of the usage of an instruction when there is an <- symbol The
details of the left side are shown on the rightside
The KAKKO (??) means the details of the memory by the shown belonging
register. (???)
For example if there is :

    r <- [.hl]

the details of the memory of the shown HL register is is going to 8 bit (???)
The in/out-put instructions [n] and [.c] are the equivalence of the in/out-put
ports.
In the flag column the flags used are shown. OPcode column the machinecode of
every different instruction is written down in HEX and BIN format.
The B stands for the length of the code in bytes, the C stands for the length
of the code in clock cycles.The shown mnemonics that have changed for the R800
can't be guaranteed to work properly on the Z80 because they are not the same
as the z80 instructions. (??)
<660                                         APPENDIX A R800 INSTRUCTION CHART>

remarks

  +-------+---------------------------------------------------------------+
  |.a{7}  |The highest bit of register .a                                 |
  |.{4..7}|Bit 4-7 of register .a                                         |
  |;      |Punctuation of the usage                                       |
  |.de:.hl|.de is the upper 16bit and .hl is the lower 16 bit and 32 bit  |
  |       |                                                               |
  |       |  formed.                                                      |
  |[.ix+d]|adres is formed when the 8bit of d is added to .ix             |
  |C      |carry flag                                                     |
  |Z      |zero flag                                                      |
  |P/v    |parity overflow                                                |
  |S      |sign flag                                                      |
  |N      |substraction flag                                              |
  |H      |half clear flag                                                |
  |      |flag don't change                                              |
  |+      |flag changed in commando used                                  |
  |0      |flag is set to 0                                               |
  |1      |flag is set to 1                                               |
  |?      |becomes uncertain                                              |
  |V      |uses the overflow flag                                         |
  |P      |uses the parity flag                                           |
  |i(iff) |value of the interrupt flipflop is attained                    |
  |r,r'   |8 bit register, .a,.b,.c,.d,.e,.h,.l                           |
  |u,u'   |8 bit register, .a,.b,.c,.d,.e,.ixh,.ixl                       |
  |v,v'   |8 bit register, .a,.b,.c,.d,.e,.iyh,.iyl                       |
  |p      |8 bit register, .ixh,.ixl                                      |
  |q      |8 bit register, .iyh,.iyl                                      |
  |ss     |16 bit register,.bc,.de,.hl,.sp                                |
  |pp     |16 bit register,.bc,.de,.ix,.sp                                |
  |rr     |16 bit register,.bc,.de,.iy,.sp                                |
  |qq     |16 bit register,.bc,.de,.hl,.af                                |
  |e      |8 bit value of short branch   (+127~128)                       |
  |k      |adres of the brk comando, 00h,08h,10h,18h,20h,28h,30h,38h      |
  |nn     |direct value of 16 bit, positive adres                         |
  |n      |direct value of 8 bit                                          |
  |b      |value shown number of bits for the bits of the mathematical    |
  |       |  operation??                                                  |
  |NOT    |bit rotation (??)                                              |
  |\/     |taking the OR of the bits                                      |
  |\-/    |taking the XOR of the bits                                     |
  |/\     |taking the AND of the bits                                     |
  |tmp    |temporary value   (??)                                         |
  |B      |number of bytes of the instruction                             |
  |C      |number of clock cycles of the instruction                      |
  +-------+---------------------------------------------------------------+
???
<A1 8 bit transfer instructions                                        661>

A.1   8 BIT TRANSFER INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|ld r,r'     | r <- r'                    |       |01 r  r'|   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|ld r,n      | r <- n                     |       |00 r 110|   |2|2|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld r,[.hl]  | r <- [.hl]                 |       |01 r 110|   |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|ld r,[.ix+d]| r <- [.ix+d]               |       |11011101|DD |3|5|
|            |                            |             |01 r 110|   | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld r,[.iy+d]| r <- [.iy+d]               |       |11111101|FD |3|5|
|            |                            |             |01 r 110|   | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [.hl],r  |[.hl] <- r                  |       |10110 r |   |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|ld [.ix+d],r| [.ix+d] <- r               |       |11011101|DD |3|5|
|            |                            |             |01110 r |   | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [.iy+d],r| [.iy+d] <- r               |       |11111101|FD |3|5|
|            |                            |             |01110 r |   | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld u,u'     | u <- u'                    |       |11011101|DD |2|2|
|            |                            |             |01 u  u'|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld v,v'     | v <- v'                    |       |11111101|FD |2|2|
|            |                            |             |01 v  v'|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld u,n      | u <- n                     |       |11011101|DD |3|3|
|            |                            |             |00 u 110|   | | |
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld v,n      | v <- n                     |       |11111101|FD |3|3|
|            |                            |             |00 v 110|   | | |
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [.hl],n  | [.hl] <- n                 |       |00110110|36 |2|3|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [.ix+d],n| [.ix+d] <- n               |       |11011101|DD |4|5|
|            |                            |             |00110110|36 | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [.iy+d],n| [.iy+d] <- n               |       |11111101|FD |4|5|
|            |                            |             |00110110|36 | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
<662                                  APPENDIX A R800 instruction chart>
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|ld .a,.i    | .a <- .i                   |+ + 0 i   0 |11101101|ED |2|2|
|            |                            |             |01010111|57 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .a,.r    | .a <- .r                   |+ + 0 i   0 |11101101|ED |2|2|
|            |                            |             |01011111|5F | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .i,.a    | .i <- .ax+d]               |       |11101101|ED |2|2|
|            |                            |             |01000111|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .r,.a    | .r <- .a                   |       |11101101|ED |2|2|
|            |                            |             |01001111|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .a,[.bc] | .a <- [.bc]                |       |00001010|0A |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|ld .a,[.de] | .a <- [.de]                |       |00011010|1A |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|ld .a,[nn]  | .a <- [nn]                 |       |00111010|3A |3|4|
|            |                            |             |<- NNl->|   | | |
|            |                            |             |<- NNh->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [.bc],.a | [.bc] <- .a                |       |00000010|02 |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|ld [.de],.a | [.de] <- .a                |       |00010010|12 |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|ld [nn],.a  | [nn] <- .a                 |       |00110010|32 |3|4|
|            |                            |             |<- NNl->|   | | |
|            |                            |             |<- NNh->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
+-+----+----+----+----+----+----+----+----+
| |000 |001 |010 |011 |100 |101 |110 |111 |
+-+----+----+----+----+----+----+----+----+
|r| .b | .c | .d | .e | .h | .l |    | .a |
+-+----+----+----+----+----+----+----+----+
|u| .b | .c | .d | .e |.ixh|.ixl|    | .a |
+-+----+----+----+----+----+----+----+----+
|v| .b | .c | .d | .e |.iyh|.iyl|    | .a |
+-+----+----+----+----+----+----+----+----+

A.2     16 bit TRANSFER INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|ld ss,nn    | ss <- nn                   |       |00ss0001|   |3|3|
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .ix,nn   | .ix <- nn                  |       |11011101|DD |4|4|
|            |                            |             |00100001|21 | | |
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .iy,nn   | .iy <- nn                  |       |11111101|FD |4|4|
|            |                            |             |00100001|21 | | |
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .sp,.hl  | .sp <- .hl                 |       |11111001|F9 |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|ld .sp,.ix  | .sp <- .ix                 |       |11011101|DD |2|2|
|            |                            |             |11111001|F9 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .sp,.iy  | .sp <- .iy                 |       |11111101|FD |2|2|
|            |                            |             |11111001|F9 | | |
+------------+----------------------------+-------------+--------+---+-+-+
<A.2 16 bit transfer instructions                                   663>
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|ld ss,[nn]  | ssh <- [nn+1]              |       |11101101|ED |4|6|
|            | ssl <- [nn]                |             |01ss1011|   | | |
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .hl,[nn] | .h <- [nn+1]               |       |00101010|2A |3|5|
|            | .l <- [nn]                 |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .ix,[nn] |.ixh <- [nn+1]              |       |11011101|DD |4|6|
|            |.ixl <- [nn]                |             |00101010|2A | | |
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld .iy,[nn] |.iyh <- [nn+1]              |       |11111101|FD |4|6|
|            |.iyl <- [nn]                |             |00101010|2A | | |
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [nn],ss  |[nn+1] <- ssh               |       |11101101|ED |4|6|
|            |[nn] <- ssl                 |             |01ss0011|   | | |
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [nn],.hl |[nn+1] <- .h                |       |00100010|22 |3|5|
|            |[nn] <- .l                  |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [nn],.ix |[nn+1] <- .ixh              |       |11011101|DD |4|6|
|            |[nn] <- .ixl                |             |00100010|22 | | |
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ld [nn],.iy |[nn+1] <- .iyh              |       |11111101|FD |4|6|
|            |[nn] <- .iyl                |             |00100010|22 | | |
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
+--+---+---+---+---+
|  |00 |01 |10 |11 |
+--+---+---+---+---+
|ss|.bc|.de|.hl|.sp|
+--+---+---+---+---+
<664                            appendix A r800 instruction chart       >

A.3    EXCHANGE INSTRUCTIONS
+-------------+----------------------------+-------------+------------+-+-+
|MNEMONIC     | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|             |                            |             +--------+---+ | |
|             |                            |S Z H P/v N C|76543210|HEX|B|C|
+-------------+----------------------------+-------------+--------+---+-+-+
|xch .de,.hl  |.de <-> .hl                 |       |11101011|EB |1|1|
+-------------+----------------------------+-------------+--------+---+-+-+
|xch .af,.af' |.af <-> .af'                |+ + + +   + +|00001000|08 |1|1|
+-------------+----------------------------+-------------+--------+---+-+-+
|xch [.sp],.hl|.l <-> [.sp];.h <-> [sp+1]  |       |11100011|E3 |1|5|
+---------- --+----------------------------+-------------+--------+---+-+-+
|xch [.sp],.ix|.ixl <-> [.sp]              |       |11011101|DD |2|6|
|             |.ixh <-> [.sp+1]            |             |11100011|E3 | | |
+-------------+----------------------------+-------------+--------+---+-+-+
|xch [.sp],.iy|.iyl <-> [.sp]              |       |11111101|FD |2|6|
|             |.iyh <-> [.sp]              |             |11100011|E3 | | |
+-------------+----------------------------+-------------+--------+---+-+-+
|xchx         |.bc <-> .bc';.de <-> de';   |       |11011001|D9 |1|1|
|             |.hl <-> .hl'                |             |        |   | | |
+-------------+----------------------------+-------------+--------+---+-+-+

A.4  STACK OPERATION INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|push qq     |[.sp-2]<-qql;[sp-1]<-qqh    |       |11qq0101|   |1|4|
|            |.sp <- .sp-2                |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|push .ix    |[.sp-2]<-.ixl;[.sp-1]<-.ixh |       |11011101|DD |2|5|
|            |.sp <- .sp-2                |             |11100101|E5 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|push .iy    |[.sp-2]<-.iyl;[.sp-1]<-.iyh |       |11111101|FD |2|5|
|            |.sp <- .sp-2                |             |11100101|E5 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|pop qq      |qql<-[.sp];qqh<-[.sp+1]     |       |11qq0001|   |1|3|
|            |.sp <- .sp+2                |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|pop .ix     |.ixl<-[.sp];.ixh<-[.sp+1]   |       |11011101|DD |2|4|
|            |.sp <- .sp+2                |             |11100001|E1 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|pop .iy     |.iyl<-[.sp];.iyh<-[.sp+1]   |       |11111101|FD |2|4|
|            |.sp <- .sp+2                |             |11100001|E1 | | |
+------------+----------------------------+-------------+--------+---+-+-+
+--+---+---+---+---+
|  |00 |01 |10 |11 |
+--+---+---+---+---+
|qq|.bc|.de|.hl|.af|
+--+---+---+---+---+
 In case of pop .af, all flags are changed

<A.5 block move instructions                                        665>

     A.5 BLOCK MOVE INSTRUCTIONS
+------------+------------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION           | FLAGS       | OPCODE     | | |
|            |                              |             +--------+---+ | |
|            |                              |S Z H P/v N C|76543210|HEX|B|C|
+------------+------------------------------+-------------+--------+---+-+-+
|move[.hl++],|[.de]<-[.hl];.de<-.de+1       |  0 +   0 |11101101|ED |2|4|
|[.de++],    |.hl<-.hl+1;.bc<-.bc-1         |      *1     |10100000|A0 | | |
+------------+------------------------------+-------------+--------+---+-+-+
|move[.hl--],|[.de]<-[.hl];.de+.de-1        |  0 +   0 |11101101|ED |2|4|
|[.de--]     |.hl<-.hl-1;.bc<-.bc-1         |      *1     |10101000|A8 | | |
+------------+------------------------------+-------------+--------+---+-+-+
|movem [.hl++|repeat;[.de]<-[.hl];.de<-.de+1|  0 0   0 |11101101|ED |2|4|
| ],[.de++]  |.hl<-.hl+1;.bc<-.bc-1;until.bc=0|           |10110000|B0 | | |
+------------+------------------------------+-+-----------+--------+---+-+-+
|movem [.hl--|repeat;[.de]<-[.hl];.de<-.de-1|  0 0   0 |11101101|ED |2|4|
| ],[.de--]  |.hl<-.hl-1;.bc<-.bc-1;until.bc=0|           |10111000|B8 | | |
+------------+--------------------------------+-----------+--------+---+-+-+
*1 when .bc-1 = 0 it will be set to 0 else it will be set to 1

A.6  BLOCK SEARCH INSTRUCTION
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|cmp         |.a-[.hl];.hl<-.hl+1         |+ + + +   1 |11101101|ED |2|4|
|  .a,[.hl++]|.bc<-.bc-1                  |  *2  *1     |10100001|A1 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|cmp         |.a-[.hl];.hl<-.hl-1         |+ + + +   1 |11101101|ED |2|4|
|  .a,[.hl--]|.bc<-.bc-1                  |  *2  *1     |10101001|A9 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|cmpm        |repeat;.a-[.hl];.hl<-.hl+1  |+ + + +   1 |11101101|ED |2|5|
|   .a,[hl++]|.bc<-.bc-1;until .bc=0      |  *2  *1     |10110001|B1 | | |
|            |  or .a=[.hl]               |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|cmpm        |repeat;.a-[.hl];.hl<-.hl-1  |+ + + +   1 |11101101|ED |2|5|
|   .a,[hl--]|.bc<-.bc-1;until .bc=0      |  *2  *1     |10111001|B9 | | |
|            |  or .a=[.hl]               |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
 *1 when bc-1=0 it will be set to 0 else it will be set to 1
 *2 when .a=[.hl] it will be set to 1 else it will be set to 0

   A.7  MULTIPLICATION INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+--+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | |  |
|            |                            |             +--------+---+ |  |
|            |                            |S Z H P/v N C|76543210|HEX|B|C |
+------------+----------------------------+-------------+--------+---+-+--+
|mulub .a,r  |.hl<-.a*r                   |0 +  0    +|11101101|ED |2|14|
|            |                            |             |11 r 001|   | |  |
+------------+----------------------------+-------------+--------+---+-+--+
|muluw .hl,ss|.de:.hl<-.hl*ss             |0 +  0    +|11101101|ED |2|36|
|            |                            |             |11ss0011|   | |  |
+------------+----------------------------+-------------+--------+---+-+--+
With the mulub function when .b,.c,.d,.e are excluded of r the usage can't be
  guaranteed. (???)
With the muluw function then .bc,.sp are excluded of ss the usage can't be
  guaranteed. (???)
<666                                    appendix a r800 instruction chart>
A.8    ADD INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|add .a,r    |.a<-.a+r                    |+ + + v   0 +|10000 r |   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|add .a,p    |.a<-.a+p                    |+ + + V   0 +|11011101|DD |2|2|
|            |                            |             |10000 p |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|add .a,q    |.a<-.a+q                    |+ + + V   0 +|11111101|FD |2|2|
|            |                            |             |10000 q |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|add .a,[.hl]|.a<-.a+[.hl]                |+ + + V   0 +|10000110|86 |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|add         |.a<-.a+[.ix+d]              |+ + + V   0 +|11011101|DD |3|5|
| .a,[.ix+d] |                            |             |10000110|86 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|add         |.a<-.a+[.iy+d]              |+ + + V   0 +|11111101|FD |3|5|
| .a,[.iy+d] |                            |             |10000110|86 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|add a,n     |.a<-.a+n                    |+ + + V   0 +|11000110|C6 |2|2|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|addc .a.r   |.a<-.a+r+C                  |+ + + V   0 +|10001 r |   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|addc .a,p   |.a<-.a+p+C                  |+ + + V   0 +|11011101|DD |2|2|
|            |                            |             |10001 p |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|addc .a,q   |.a<-.a+q+C                  |+ + + V   0 +|11111101|FD |2|2|
|            |                            |             |10001 q |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|addc.a,[.hl]|.a<-.a+[.hl]+C              |+ + + V   0 +|10001110|8E |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|addc        |.a<-.a+[.ix+d]+C            |+ + + V   0 +|11011101|DD |3|5|
| .a,[.ix+d] |                            |             |10001110|8E | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|addc        |.a<-.a+[.iy+d]+C            |+ + + V   0 +|11111101|FD |3|5|
| .a,[.iy+d] |                            |             |10001110|8E | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|addc .a,n   |.a<-.a+n+C                  |+ + + V   0 +|11001110|CE |2|2|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|addc .hl,ss |.hl<-.hl+ss+C               |+ + ? V   0 +|11101101|ED |2|2|
|            |                            |             |01ss1010|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|add .hl,ss  |.hl<-.hl+ss                 |  ?    0 +|00ss1001|   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|add .ix,pp  |.ix<-.ix+pp                 |  ?    0 +|11011101|DD |2|2|
|            |                            |             |00pp1001|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|add .iy,rr  |.iy<-.iy+rr                 |  ?    0 +|11111101|FD |2|2|
|            |                            |             |00rr1001|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
<A.8 ADD INSTRUCTIONS                                           667>

+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|inc r       |r<-r+1                      |+ + + V   0 |00 r 100|   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|inc p       |p<-p+1                      |+ + + V   0 |11011101|DD |2|2|
|            |                            |             |00 p 100|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|inc q       |q<-q+1                      |+ + + V   0 |11111101|FD |2|2|
|            |                            |             |00 q 100|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|inc [.hl]   |[.hl]<-[.hl]+1              |+ + + V   0 |00110100|34 |1|4|
+------------+----------------------------+-------------+--------+---+-+-+
|inc [.ix+d] |[.ix+d]<-[.ix+d]+1          |+ + + V   0 |11011101|DD |3|7|
|            |                            |             |00110100|34 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|inc [.iy+d] |[.iy+d]<-[.iy+d]+1          |+ + + V   0 |11111101|FD |3|7|
|            |                            |             |00110100|34 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|inc ss      |ss<-ss+1                    |       |00ss0011|   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|inc .ix     |.ix<-.ix+1                  |       |11011101|DD |2|2|
|            |                            |             |00100011|23 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|inc .iy     |.iy<-.iy+1                  |       |11111101|FD |2|2|
|            |                            |             |00100011|23 | | |
+------------+----------------------------+-------------+--------+---+-+-+
+--+---+---+---+---+
|  |00 |01 |10 |11 |
+--+---+---+---+---+
|ss|.bc|.de|.hl|.sp|
+--+---+---+---+---+
|pp|.bc|.de|.ix|.sp|
+--+---+---+---+---+
|rr|.bc|.de|.iy|.sp|
+--+---+---+---+---+
+-+----+----+----+----+----+-----+----+----+
| |000 |001 |010 |011 |100 |101  |110 |111 |
+-+----+----+----+----+----+-----+----+----+
|p|    |    |    |    |.ixh|.ixl |    |    |
+-+----+----+----+----+----+-----+----+----+
|q|    |    |    |    |.iyh|.iyl |    |    |
+-+----+----+----+----+----+-----+----+----+
<668                                   APPENDIX A R800 INSTRUCTION CHART>
   A.9  DECREASE INSTRUCTION
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|sub .a,r    |.a<.a-r                     |+ + + V   1 +|10010 r |   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|sub .a,p    |.a<-.a-p                    |+ + + V   1 +|11011101|DD |2|2|
|            |                            |             |10010 p |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|sub .a,q    |.a<-.a-q                    |+ + + V   1 +|11111101|FD |2|2|
|            |                            |             |10010 q |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|sub .a,[.hl]|.a<-.a-[.hl]                |+ + + V   1 +|10010110|96 |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|sub .a,     |.a<-.a-[.ix+d]              |+ + + V   1 +|11011101|DD |3|5|
|  [.ix+d]   |                            |             |10010110|96 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|sub .a,     |.a<-.a-[.iy+d]              |+ + + V   1 +|11111101|FD |3|5|
|  [.iy+d]   |                            |             |10010110|96 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|sub .a,n    |.a<-.a-n                    |+ + + V   1 +|11010110|D6 |2|2|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|subc .a,r   |.a<-.a-r-C                  |+ + + V   1 +|10011 r |   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|subc .a,p   |.a<-.a-p-C                  |+ + + V   1 +|11011101|DD |2|2|
|            |                            |             |10011 p |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|subc .a,q   |.a<-.a-q-C                  |+ + + V   1 +|11111101|FD |2|2|
|            |                            |             |10011 q |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|subc .a,    |.a<-.a-[.hl]-C              |+ + + V   1 +|10011110|9E |1|2|
|     [.hl]  |                            |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|subc .a,    |.A<-.A-[.IX+D]+c            |+ + + V   1 +|11011101|DD |3|5|
|   [.ix+d]  |                            |             |10011110|9E | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|subc .a,    |.a<-.a-[.iy+d]+c            |+ + + V   1 +|11111101|FD |3|5|
|   [.iy+d]  |                            |             |10011110|9E | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|subc a,n    |.a<-.a-n-C                  |+ + + V   1 +|11011110|DE |2|2|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|subc .hl,ss |.hl<-.hl-ss-C               |+ + ? V   1 +|11101101|ED |2|2|
|            |                            |             |01ss0010|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
<A.10 DECREASE INSTRUCTION                                      669>

+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|dec r       |r<-r-1                      |+ + + V   1 |00 r 101|   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|dec p       |p<-p-1                      |+ + + V   1 |11011101|DD |2|2|
|            |                            |             |00 p 101|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|dec q       |q<-q-1                      |+ + + V   1 |11111101|FD |2|2|
|            |                            |             |00 q 101|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|dec [.hl]   |[.hl]<-[.hl]-1              |+ + + V   1 |00110101|35 |1|4|
+------------+----------------------------+-------------+--------+---+-+-+
|dec [.ix+d] |[.ix+d]<-[.ix+d]-1          |+ + + V   1 |11011101|DD |3|7|
|            |                            |             |00110101|35 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|dec [.iy+d] |[.iy+d]<-[.iy+d]-1          |+ + + V   1 |11111101|FD |3|7|
|            |                            |             |00110101|35 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|dec ss      |ss<-ss-1                    |       |00ss1011|   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|dec .ix     |.ix<-.ix-1                  |       |11011101|DD |2|2|
|            |                            |             |00101011|2B | | |
+------------+----------------------------+-------------+--------+---+-+-+
|dec .iy     |.iy<-.iy-1                  |       |11111101|FD |2|2|
|            |                            |             |00101011|2B | | |
+------------+----------------------------+-------------+--------+---+-+-+

A.10  COMPARISON INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|cmp .a,r    |.a-r                        |+ + + V   1 +|10111 r |   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|cmp .a,r    |.a-p                        |+ + + V   1 +|11011101|DD |2|2|
|            |                            |             |10111 p |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|cmp .a,q    |.a-q                        |+ + + V   1 +|11111101|FD |2|2|
|            |                            |             |10111 q |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|cmp .a,[.hl]|.a-[.hl]                    |+ + + V   1 +|10111110|BE |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|cmp .a,     |.a-[.ix+d]                  |+ + + V   1 +|11011101|DD |3|5|
|  [.ix+d]   |                            |             |10111110|BE | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|cmp .a,     |.a-[.iy+d]                  |+ + + V   1 +|11111101|FD |3|5|
|  [.iy+d]   |                            |             |10111110|BE | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|cmp .a,n    |.a-n                        |+ + + V   1 +|11111110|FE |2|2|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
< 670                                 APPENDIX A R800 INSTRUCTION CHART>

A.11 LOGIC OPERATION INSTRUCTION
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|and .a,r    |.a<-.a/\r                   |+ + 1 P   0 0|10100 r |   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|and .a,p    |.a<-.a/\p                   |+ + 1 P   0 0|11011101|DD |2|2|
|            |                            |             |10100 p |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|and .a,q    |.a<-.a/\q                   |+ + 1 P   0 0|11111101|FD |2|2|
|            |                            |             |10100 q |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|and .a,[.hl]|.a<-.a/\[.hl]               |+ + 1 P   0 0|10100110|A6 |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|and .a,     |.a<-.a/\[.ix+d]             |+ + 1 P   0 0|11011101|DD |3|5|
|    [.ix+d] |                            |             |10100110|A6 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|and .a,     |.a<-.a/\[.iy+d]             |+ + 1 P   0 0|11111101|FD |3|5|
|    [.iy+d] |                            |             |10100110|A6 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|and .a,n    |.a<-.a/\n                   |+ + 1 P   0 0|11100110|E6 |2|2|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|or  .a,r    |.a<-.a\/r                   |+ + 0 P   0 0|10110 r |   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|or .a,p     |.a<-.a\/p                   |+ + 0 P   0 0|11011101|DD |2|2|
|            |                            |             |10110 p |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|or .a,q     |.a<-.a\/q                   |+ + 0 P   0 0|11111101|FD |2|2|
|            |                            |             |10110 q |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|or .a,[.hl] |.a<-.a\/[.hl]               |+ + 0 P   0 0|10110110|B6 |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|or .a,      |.a<-.a\/[.ix+d]             |+ + 0 P   0 0|11011101|DD |3|5|
|   [.ix+d]  |                            |             |10110110|B6 | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|or .a,      |.a<-.a\/[.iy+d]             |+ + 0 P   0 0|11111101|FD |3|5|
|   [.iy+d]  |                            |             |10110110|   | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|or .a,n     |.a<-.a\/n                   |+ + 0 P   0 0|11110110|F6 |2|2|
|            |                            |             |<- n  > |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|xor .a,r    |.a<-.a\-/r                  |+ + 0 P   0 0|10101 r |   |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|xor .a,p    |.a<-.a\-/p                  |+ + 0 P   0 0|11011101|DD |2|2|
|            |                            |             |10101 p |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|xor .a,q    |.a<-.a\-/q                  |+ + 0 P   0 0|11111101|FD |2|2|
|            |                            |             |10101 q |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|xor .a,[.hl]|.a<-.a\-/[.hl]              |+ + 0 P   0 0|10101110|AE |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|xor .a,     |.a<-.a\-/[.ix+d]            |+ + 0 P   0 0|11011101|DD |3|5|
|   [.ix+d]  |                            |             |10101110|AE | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|xor .a,     |.a<-.a\-/[.iy+d]            |+ + 0 P   0 0|11111101|FD |3|5|
|   [.iy+d]  |                            |             |10101110|AE | | |
|            |                            |             |<- d  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|xor .a,n    |.a<-.a\-/n                  |+ + 0 P   0 0|11101110|EE |2|2|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
<A.12 BIT OPERATION INSTRUCTION                                 671>

      A.12 BIT OPERATION INSTRUCTION
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|bit b,r     |z<-NOT r{b}                 |? + 1 ?   0 |11001011|CB |2|2|
|            |                            |             |01 b  r |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bit b,[.hl] |z<-NOT [.hl]{b}             |? + 1 ?   0 |11001011|CB |2|2|
|            |                            |             |01 b 110|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bit b,      |z<-NOT [.ix+d]{b}           |? + 1 ?   0 |11011101|DD |4|5|
|  [.ix+d]   |                            |             |11001011|CB | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |01 b 110|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bit b,      |z<-NOT [.iy+d]{b}           |? + 1 ?   0 |11111101|FD |4|5|
|  [.iy+d]   |                            |             |11001011|CB | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |01 b 110|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|set b,r     |r{b}<-1                     |       |11001011|CB |2|2|
|            |                            |             |11 b  r |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|set b,[.hl] |[.hl]{b}<-1                 |       |11001011|CB |2|5|
|            |                            |             |11 b 110|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|set b,      |[.ix+d]{b}<-1               |       |11011101|DD |4|7|
|  [.ix+d]   |                            |             |11001011|CB | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |11 b 110|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|set b,      |[.iy+d]{b}<-1               |       |11111101|FD |4|7|
|  [.iy+d]   |                            |             |11001011|CB | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |11 b 110|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|clr b,r     |r{b}<-0                     |       |11001011|CB |2|2|
|            |                            |             |10 b  r |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|clr b,[.hl] |[.hl]{b}<-0                 |       |11001011|CB |2|5|
|            |                            |             |10 b 110|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|clr b,      |[.ix+d]{b}<-0               |       |11011101|DD |4|7|
|  [.ix+d]   |                            |             |11001011|CB | | |
|            |                            |             |<= d  ->|   | | |
|            |                            |             |10 b 110|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|clr b,      |[.iy+d]{b}<-0               |       |11111101|FD |4|7|
|  [.iy+d]   |                            |             |11001011|CB | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |10 b 110|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
<672                                    APPENDIX A R800 INSTRUCTION CHART>
A.13 ROTATE INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|rol a       |C<-.a{7};.a<-.a*2;a{0}<-C   |  0    0 +|00000111|07 |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|ror a       |C<-.a{0};.a<-.a/2;a{7}<-C   |  0    0 +|00001111|0F |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|rolc a      |tmp<-C;C<-.a{7};.a<-.a*2;   |  0    0 +|00010111|17 |1|1|
|            |    .a{0}<-tmp              |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rorc a      |tmp<-C;C<-.a{0};.a<-.a/2;   |  0    0 +|00011111|1F |1|1|
|            |    .a{7}<-tmp              |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rol r       |C<-r{7}                     |+ + 0 P   0 +|11001011|CB |2|2|
|            |r<r*2;r{0}<-c               |             |00000 r |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rol [.hl]   |C<-[.hl]{7}                 |+ + 0 P   0 +|11001011|CB |2|5|
|            |[.hl]<-[.hl]*2;[.hl]{0}<-C  |             |00000110|06 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rol [.ix+d] |C<-[.ix+d]{7}               |+ + 0 P   0 +|11011101|DD |4|7|
|            |[.ix+d]<-[.ix+d]*2          |             |        |CB | | |
|            |[.ix+d]{0}<-C               |             |<- d  ->|   | | |
|            |                            |             |00000110|06 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rol [.iy+d] |C<-[.iy+d]{7}               |+ + 0 P   0 +|11111101|FD |4|7|
|            |[.iy+d]<-[.iy+d]*2          |             |        |CB | | |
|            |[.iy+d]{0}<-C               |             |<- d  ->|   | | |
|            |                            |             |00000110|06 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ror r       |C<-r{0}                     |+ + 0 P   0 +|11001011|CB |2|2|
|            |r<-r/2;r{7}<-C              |             |        |   | | |
|            |                            |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ror [.hl]   |C<-[.hl]{0}                 |+ + 0 P   0 +|11001011|CB |2|5|
|            |[.hl]<-[.hl]/2;[.hl]{7}<-C  |             |00001110|0E | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ror [.ix+d] |C<-[.ix+d]{0}               |+ + 0 P   0 +|11011101|DD |4|7|
|            |[.ix+d]<-[.ix+d]/2          |             |        |CB | | |
|            |[.ix+d]{7}<-C               |             |<- d  ->|   | | |
|            |                            |             |00001110|06 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rol [.iy+d] |C<-[.iy+d]{0}               |+ + 0 P   0 +|11111101|FD |4|7|
|            |[.iy+d]<-[.iy+d]/2          |             |        |CB | | |
|            |[.iy+d]{7}<-C               |             |<- d  ->|   | | |
|            |                            |             |00001110|06 | | |
+------------+----------------------------+-------------+--------+---+-+-+
<A.13   ROTATE INSTRUCTIONS                                            673>
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|rolc r      |tmp<-C;C<-r{7}              |+ + 0 P   0 +|11001011|CB |2|2|
|            |r<-r*2;r{0}<-tmp            |             |00010 r |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rolc [.hl]  |tmp<-C;C<-[.hl]{7}          |+ + 0 P   0 +|11001011|CB |2|5|
|            |[.hl]<-[.hl]*2;[.hl]{0}<-tmp|             |00010110|16 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rolc [.ix+d]|tmp<-C                      |+ + 0 P   0 +|11011101|DD |4|7|
|            |C<-[.ix+d]{7}               |             |11001011|CB | | |
|            |[.ix+d]<-[.ix+d]*2          |             |<- b  ->|   | | |
|            |[.ix+d]{0}<-tmp             |             |00010110|16 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rolc [.iy+d]|tmp<-C                      |+ + 0 P   0 +|11111101|FD |4|7|
|            |C<-[.iy+d]{7}               |             |11001011|CB | | |
|            |[.iy+d]<-[.iy+d]*2          |             |<- b  ->|   | | |
|            |[.iy+d]{0}<-tmp             |             |00010110|16 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rorc r      |tmp<-C;C<-r{0}              |+ + 0 P   0 +|11001011|CB |2|2|
|            |r<-r/2;r{7}<-tmp            |             |00011 r |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rorc [.hl]  |tmp<-C;C<-[.hl]{0}          |+ + 0 P   0 +|11001011|CB |2|5|
|            |[.hl]<-[.hl]/2;[.hl]{7}<-tmp|             |00011110|1E | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rorc [.ix+d]|tmp<-C                      |+ + 0 P   0 +|11011101|DD |4|7|
|            |C<-[.ix+d]{0}               |             |11001011|CB | | |
|            |[.ix+d]<-[.ix+d]/2          |             |<- b  ->|   | | |
|            |[.ix+d]{7}<-tmp             |             |00011110|16 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rorc [.iy+d]|tmp<-C                      |+ + 0 P   0 +|11111101|FD |4|7|
|            |C<-[.iy+d]{0}               |             |11001011|CB | | |
|            |[.iy+d]<-[.iy+d]/2          |             |<- b  ->|   | | |
|            |[.iy+d]{7}<-tmp             |             |00011110|1E | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rol4 [.hl]  |tmp<-.a{0..3}               |+ + 0 P   0 |11101101|ED |2|5|
|            |.a{0..3}<-[.hl]{4..7}       |             |11101111|6F | | |
|            |[.hl]{4..7}<-[.hl]{0..3}    |             |        |   | | |
|            |[.hl]{0..3}<-tmp            |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|rol4 [.hl]  |tmp<-.a{0..3}               |+ + 0 P   0 |11101101|ED |2|5|
|            |.a{0..3}<-[.hl]{4..7}       |             |11100111|67 | | |
|            |[.hl]{4..7}<-[.hl]{0..3}    |             |        |   | | |
|            |[.hl]{0..3}<-tmp            |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
<674                                    APPENDIX A R800 INSTRUCTION CHART

A.14 SHIFT INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|shl r       |C<-r{7}                     |+ + 0 P   0 +|11001011|CB |2|2|
|shla        |r<-r*2                      |             |00100 r |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shl [.hl]   |C<-[.hl]{7}                 |+ + 0 P   0 +|11001011|CB |2|5|
|shla        |[.hl]<-[.hl]*2              |             |00100110|26 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shl [,ix+d] |C<-[.ix+d]{7}               |+ + 0 P   0 +|11011101|DD |4|7|
|shla        |[.ix+d]<-[.ix+d]*2          |             |11001011|CB | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |00100110|26 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shl [,iy+d] |C<-[.iy+d]{7}               |+ + 0 P   0 +|11111101|DD |4|7|
|shla        |[.iy+d]<-[.iy+d]*2          |             |11001011|CB | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |00100110|26 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shr r       |C<-r{0}                     |+ + 0 P   0 +|11001011|CB |2|2|
|            |r<-r/2                      |             |00111 r |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shr [.hl]   |C<-[.hl]{0}                 |+ + 0 P   0 +|11001011|CB |2|5|
|            |[.hl]<-[.hl]/2              |             |00111110|3E | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shr [,ix+d] |C<-[.ix+d]{0}               |+ + 0 P   0 +|11011101|DD |4|7|
|            |[.ix+d]<-[.ix+d]/2          |             |11001011|CB | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |00111110|3E | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shr [,iy+d] |C<-[.iy+d]{0}               |+ + 0 P   0 +|11111101|DD |4|7|
|            |[.iy+d]<-[.iy+d]/2          |             |11001011|CB | | |
|            |                            |             |<- d  ->|   | | |
|            |                            |             |00111110|3E | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shra r      |tmp<-r{7};C<-r{0}           |+ + 0 P   0 +|11001011|CB |2|2|
|            |r<-r/2;r{7}<-tmp            |             |00101 r |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shra [.hl]  |tmp<-[.hl]{7};C<-[.hl]{0}   |+ + 0 P   0 +|11001011|CB |2|2|
|            |[.hl]<-[.hl]/2;[.hl]{7}<-tmp|             |00101110|2E | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shra [.ix+d]|tmp<-[.ix+d]{7}             |+ + 0 P   0 +|11011101|DD |4|7|
|            |C<-[.ix+d]{0}               |             |11001011|CB | | |
|            |[.ix+d]<-[.ix+d]/2          |             |<- d  ->|   | | |
|            |[.ix+d]{7}<-tmp             |             |00101110|2E | | |
+------------+----------------------------+-------------+--------+---+-+-+
|shra [.iy+d]|tmp<-[.iy+d]{7}             |+ + 0 P   0 +|11111101|FD |4|7|
|            |C<-[.iy+d]{0}               |             |11001011|CB | | |
|            |[.iy+d]<-[.iy+d]/2          |             |<- d  ->|   | | |
|            |[.iy+d]{7}<-tmp             |             |00101110|2E | | |
+------------+----------------------------+-------------+--------+---+-+-+
The shl instruction and the shla instruction are exectly the same and use the
same operand.
<A.15  BRANCH INSTRUCTIONS                                              675>

A.15 BRANCH INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|br nn       |.pc<-nn                     |       |11000011|C3 |3|3|
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bnz nn      |if z=0                      |       |11000010|C2 |3|3|
|            |.pc<-nn                     |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bz nn       |if z=1                      |       |11001010|CA |3|3|
|            |.pc<-nn                     |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bnc nn      |if c=0                      |       |11010010|D2 |3|3|
|            |.pc<-nn                     |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bc nn       |if c=1                      |       |11011010|DA |3|3|
|            |.pc<-nn                     |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bpo nn      |if P/v=0                    |       |11100010|E2 |3|3|
|            |.pc<-nn                     |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bpe nn      |if P/v=1                    |       |11101010|EA |3|3|
|            |.pc<-nn                     |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bp nn       |if s=0                      |       |11110010|F2 |3|3|
|            |.pc<-nn                     |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|bm nn       |if s=1                      |       |11111010|FA |3|3|
|            |                            |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|br [.hl]    |.pc<-.hl                    |       |11101001|E9 |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|br [.ix]    |.pc<-.ix                    |       |11011101|DD |2|2|
|            |                            |             |11101001|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|br [.iy]    |.pc<-.iy                    |       |11111101|FD |2|2|
|            |                            |             |11101001|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
<676                                    APPENDIX A R800 INSTRUCTION CHART>

+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|short       |.pc<-.pc+e                  |       |00011000|18 |2|3|
|br e        |                            |             |<- e-2->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|short       |if z=0                      |       |00100000|20 |2|2|
|bnz e       |.pc<-.pc+e                  |             |<- e-2->|   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|short       |if z=1                      |       |00101000|28 |2|2|
|bz e        |.pc<-.pc+e                  |             |<- e-2->|   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|short       |if c=0                      |       |00110000|30 |2|2|
|bnc e       |.pc<-.pc+e                  |             |<- e-2->|   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|short       |if c=1                      |       |00111000|38 |2|2|
|bc e        |.pc<-.pc+e                  |             |<- e-2->|   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|dbnz e      |.b<-.b-1;if .b<>0           |       |00010000|10 |2|2|
|            |.pc<-.pc+e                  |             |<- e-2->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+

A.16 CALL INSTRUCTIONS

+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|call nn     |[.sp-2]<-.spl;[.sp-1]<-.sph |       |11001101|CD |3|5|
|            |.sp<-.sp-2;.pc<-nn          |             |<-nnl ->|   | | |
|            |                            |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|call nz,nn  |if z=0                      |       |11000100|C4 |3|3|
|            |[.sp-2]<-.pcl;[.sp-1]<-.pch |             |<-nnl ->|   | |5|
|            |.sp<-.sp-2;.pc<-nn          |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|call z,nn   |if z=1                      |       |11001100|CC |3|3|
|            |[.sp-2]<-.pcl;[.sp-1]<-.pch |             |<-nnl ->|   | |5|
|            |.sp<-.sp-2;.pc<-nn          |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|call nc,nn  |if c=0                      |       |11010100|D4 |3|3|
|            |[.sp-2]<-.pcl;[.sp-1]<-.pch |             |<-nnl ->|   | |5|
|            |.sp<-.sp-2;.pc<-nn          |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|call c,nn   |if c=1                      |       |11011100|DC |3|3|
|            |[.sp-2]<-.pcl;[.sp-1]<-.pch |             |<-nnl ->|   | |5|
|            |.sp<-.sp-2;.pc<-nn          |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|call po,nn  |if P/v=0                    |       |11100100|E4 |3|3|
|            |[.sp-2]<-.pcl;[.sp-1]<-.pch |             |<-nnl ->|   | |5|
|            |.sp<-.sp-2;.pc<-nn          |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|call pe,nn  |if P/v=1                    |       |11101100|EC |3|3|
|            |[.sp-2]<-.pcl;[.sp-1]<-.pch |             |<-nnl ->|   | |5|
|            |.sp<-.sp-2;.pc<-nn          |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|call p,nn   |if s=0                      |       |11110100|F4 |3|3|
|            |[.sp-2]<-.pcl;[.sp-1]<-.pch |             |<-nnl ->|   | |5|
|            |.sp<-.sp-2;.pc<-nn          |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|call m,nn   |if s=1                      |       |11111100|FC |3|3|
|            |[.sp-2]<-.pcl;[.sp-1]<-.pch |             |<-nnl ->|   | |5|
|            |.sp<-.sp-2;.pc<-nn          |             |<-nnh ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
<A.16 CALL INSTRUCTIONS                                         677
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|ret         |.pcl<-[.sp];.pch<-[.sp+1]   |       |11001001|C9 |1|3|
|            |.sp<-.sp+2                  |             |        |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ret nz      |if z=0 ; .pcl<-[.sp]        |       |11000000|C0 |1|1|
|            |.pch<-.[.sp+1];.sp<-.sp+2   |             |        |   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|ret z       |if z=1 ; .pcl<-[.sp]        |       |11001000|C8 |1|1|
|            |.pch<-.[.sp+1];.sp<-.sp+2   |             |        |   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|ret nc      |if c=0 ; .pcl<-[.sp]        |       |11010000|D0 |1|1|
|            |.pch<-.[.sp+1];.sp<-.sp+2   |             |        |   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|ret c       |if c=1 ; .pcl<-[.sp]        |       |11011000|D8 |1|1|
|            |.pch<-.[.sp+1];.sp<-.sp+2   |             |        |   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|ret po      |if P/v=0 ; .pcl<-[.sp]      |       |11100000|E0 |1|1|
|            |.pch<-.[.sp+1];.sp<-.sp+2   |             |        |   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|ret pe      |if P/v=1 ; .pcl<-[.sp]      |       |11101000|E8 |1|1|
|            |.pch<-.[.sp+1];.sp<-.sp+2   |             |        |   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|ret p       |if s=0 ; .pcl<-[.sp]        |       |11110000|F0 |1|1|
|            |.pch<-.[.sp+1];.sp<-.sp+2   |             |        |   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|ret m       |if s=1 ; .pcl<-[.sp]        |       |11111000|F8 |1|1|
|            |.pch<-.[.sp+1];.sp<-.sp+2   |             |        |   | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|ret i       |interrupt return            |       |11101101|ED |2|5|
|            |                            |             |01001101|4D | | |
+------------+----------------------------+-------------+--------+---+-+-+
|ret n       |Non Maskable Interrupt      |       |11101101|ED |2|5|
|            |  return                    |             |01000101|45 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|brk k       |[.sp-2]<-.pcl;[.sp-1]<-.pch |       |11   111|C0 |1|4|
|            |.sp<-.sp-2;.pcl<-k;pch<-0   |             |  k/8   |   | | |
+------------+----------------------------+-------------+--------+---+-+-+
<678                                    APPENDIX A R800 INSTRUCTION CHART>

A.17      INOUT/OUTPUT INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|in .a,[n]   |.a<-[n]                     |       |11011011|DB |2|3|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|in r,[.c]   |r<-[.c]                     |+ + 0 P   0 |11101101|ED |2|3|
|            |                            |             |01 r 000|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|in .f,[.c]  |.f<-[.c]                    |+ + 0 P   0 |11101101|ED |2|3|
|            |                            |             |01110000|70 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|in          |[.hl]<-[.c];.b<-.b-1        |? + ? ?   1 |11101101|ED |2|4|
|[.hl++],[.c]|.hl<-.hl+1                  |  *1         |10100010|A2 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|in          |[.hl]<-[.c];.b<-.b-1        |? + ? ?   1 |11101101|ED |2|4|
|[.hl--],[.c]|.hl<-.hl-1                  |  *1         |10101010|AA | | |
+------------+----------------------------+-------------+--------+---+-+-+
|inm         |repeat;[.hl]<-[.c];.b<-.b-1 |? 1 ? ?   1 |11101101|ED |2|4|
|[.hl++],[.c]|.hl<-.hl+1;until .b=0       |             |10110010|B2 | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|inm         |repeat;[.hl]<-[.c];.b<-.b-1 |? 1 ? ?   1 |11101101|ED |2|4|
|[.hl--],[.c]|.hl<-.hl-1;until .b=0       |             |10111010|BA | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|out [n],.a  |[n]<-.a                     |       |11010011|D3 |2|3|
|            |                            |             |<- n  ->|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|out [.c],r  |[.c]<-r                     |       |11101101|ED |2|3|
|            |                            |             |01 r 001|   | | |
+------------+----------------------------+-------------+--------+---+-+-+
|out         |[.c]<-[.hl];.b<-.b-1        |? + ? ?   1 |11101101|ED |2|4|
|[.c],[.hl++]|.hl<-.hl+1                  |  *1         |10100011|A3 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|out         |[.c]<-[.hl];.b<-.b-1        |? + ? ?   1 |11101101|ED |2|4|
|[.c],[.hl--]|.hl<-.hl-1                  |  *1         |10101011|AB | | |
+------------+----------------------------+-------------+--------+---+-+-+
|outm        |repeat;[.c]<-[.hl];.b<-.b-1 |? 1 ? ?   1 |11101101|ED |2|4|
|[.c],[.hl++]|.hl<-.hl+1;until .b=0       |             |10110011|B3 | |3|
+------------+----------------------------+-------------+--------+---+-+-+
|outm        |repeat;[.c]<-[.hl];.b<-.b-1 |? 1 ? ?   1 |11101101|ED |2|4|
|[.c],[.hl--]|.hl<-.hl-1;until .b=0       |             |10111011|BB | |3|
+------------+----------------------------+-------------+--------+---+-+-+
*1 als .b-1=0 it is set to 1 else it is 0
dependent of the external port which the .c register of in.f,[.c] desginates,
flags will be changed. It doesn't matter where the external port is.(???)
<A.18 CPU CONTROL INSTRUCTIONS                                          679>

A.18  CPU CONTROL INSTRUCTIONS
+------------+----------------------------+-------------+------------+-+-+
|MNEMONIC    | INSTRUCTION ACTION         | FLAGS       | OPCODE     | | |
|            |                            |             +--------+---+ | |
|            |                            |S Z H P/v N C|76543210|HEX|B|C|
+------------+----------------------------+-------------+--------+---+-+-+
|adj .a      |adjust to decimal           |+ + + P    +|00100111|27 |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|not .a      |.a<-NOT .a                  |  1    1 |00101111|2F |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|neg .a      |.a<-NOT .a+1                |+ + + V   1 +|11101101|ED |2|2|
|            |                            |             |01000100|44 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|not c       |C<-NOT C                    |  ?    0 +|00111111|3F |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|set c       |C<-1                        |  0    0 1|00110111|37 |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|nop         |NO operation                |       |00000000|00 |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|halt        |HALT                        |       |01110110|76 |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|di          |IFF<-0                      |       |11110011|F3 |1|2|
+------------+----------------------------+-------------+--------+---+-+-+
|ei          |IFF<-1                      |       |11111011|FB |1|1|
+------------+----------------------------+-------------+--------+---+-+-+
|im 0        |interrupt mode 0            |       |11101101|ED |2|3|
|            |                            |             |01000110|46 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|im 1        |interrupt mode 1            |       |11101101|ED |2|3|
|            |                            |             |01010110|56 | | |
+------------+----------------------------+-------------+--------+---+-+-+
|im 2        |interrupt mode 2            |       |11101101|ED |2|3|
|            |                            |             |01011110|5E | | |
+------------+----------------------------+-------------+--------+---+-+-+
<680                                                                    >
<681                                                                    >
------------------------------------------------------------------------
                                B
                       R800 MULTIPLICATION MACRO
------------------------------------------------------------------------
B.1 MULTIPLICATION INSTRUCTIONS OF THE R800
  The following multiplication instructions are available.

B.1.1   8 bit MULTIPLICATIONS
  Two of the same kind 8 bit registers are filled and the result appears in
register HL. The multiplication is performed with an instruction.
The instruction are shown below.
----------------------------------------------------------------------------
operation     instruction          clock cycles
----------------------------------------------------------------------------
HL<-A*B         ED C1                   14
HL<-A*C         ED C9                   14
HL<-A*D         ED D1                   14
HL<-A*E         ED D9                   14
---------------------------------------------------------------------------

B.1.2    16 bit MULTIPLICATIONS
  Two of the same kind 16 bit registers are filled and the result appears in
registers DE:HL. The multiplication is performed with an instruction.
The instruction are shown below.
----------------------------------------------------------------------------
operation     instruction          clock cycles
----------------------------------------------------------------------------
DE:HL<-HL*BC    ED C3                   36
DE:HL<-HL*SP    ED F3                   36
----------------------------------------------------------------------------
<682                                    APPENDIX A R800 INSTRCUCTION CHART>

B.2      MULTIPLICATION MACRO FOR M80

When using multiplications in M80, this macro can be used.

         mult8  macro  reg
                ifidn <reg>,<b>
                defb 0edh,0c1h
                else
                ifidn <reg>,<B>
                defb 0edh,0c1h
                else
                ifidn <reg>,<c>
                defb 0edh,0c9h
                else
                ifidn <reg>,<C>
                defb 0edh,0c9h
                else
                ifidn <reg>,<d>
                defb 0edh,0d1h
                else
                ifidn <reg>,<D>
                defb 0edh,0d1h
                else
                ifidn <reg>,<e>
                defb 0edh,0d9h
                else
                ifidn <reg>,<E>
                defb 0edh,0d9h
                else
                if1
                .printx *MULT8: illegal argument*
                defb 00h,00h
                err
                endif
                endif
                endif
                endif
                endif
                endif
                endif
                endif
                endif
                endm

         mult16 macro  reg
                ifidn <reg>,<bc>
                defb   0edh, 0c3h
                else
                ifidn <reg>,<BC>
                defb   0edh,0c3h
                else
<B.2 MULTIPLICATION MACRO FOR M80                                       683>
                ifidn <reg>,<sp>
                defb   0edh, 0f3h
                else
                ifidn <reg>,<SP>
                defb   0edh,0f3h
                else
                if1
                .printx *MULT16: illegal argument*
                defb    00h, 00h
                err
                end if
                end if
                end if
                end if
                end if
                endm
<684                                                                    >
<                                                                    685>

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("r800-msx", "", "R800", "", "R800: particularidades no MSX turbo R",
        R"MAN(R800

picture 1.2  execution-comparison of the MSX Turbo R and MSX2+
-----------------------------------------------------------------------------
 INSTRUCTION    MSX2+(in u.sec) MSX Turbo R(in u.sec)   FACTOR
-----------------------------------------------------------------------------
LD     r,s      1.40            0.14                    x 10.0
LD     r,(HL)   2.23            0.42                    x 5.3
LD     r,(IX+n) 5.87            0.70                    x 8.4
PUSH   qq       3.35            0.56                    x 6.0
LDIR   (BC<>0)  6.43            0.98                    x 6.6
ADD    A,r      1.40            0.14                    x 10.0
INC    r        1.40            0.14                    x 10.0
ADD    HL,ss    3.35            0.14                    x 24.0
INC    ss       1.96            0.14                    x 14.0
JP              3.07            0.42                    x 7.3
JR              3.63            0.42                    x 8.7
DJNZ   (B<>0)   3.91            0.42                    x 9.3
CALL            5.03            0.84                    x 6.0
RET             3.07            0.56                    x 5.5
MULTU  A,r      160             1.96                    x 82
MULTUW HL,rr    361             5.03                    x 71
-----------------------------------------------------------------------------

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("turbo-basic-compiler", "turbo-basic", "Turbo-Basic", "", "Turbo-Basic Compiler (compilador de MSX-BASIC)",
        R"MAN(Turbo-Basic Compiler

MSX-BASIC-KUN (BASIC COMPILER)

    by J.Suzuki 1989
    this document & samples by Sho Endo
    translate to english by LASP
    this text file typed without changes by Nestor Soriano

MSX-BASIC-KUN is an incredible BASIC compiler. It will compile a BASIC
program on memory in few seconds and execute it 15 to 100 times faster!! It
can compile most of the statements and functions of MSX-BASIC and can handle
strings and floating numbers. Once you see it, you'd feel you'd never need to
learn the Z-80 machine language. Real time games, C.G., demo programs can be
written by the ease of BASIC for machine language speed.

* USAGE *

1. Settings & General knowledge

This compiler is sold in Japan as a cartridge for 4500 yen. You just set it
in a slot to use it. Also, you can found this compiler in MSX2+ Sanyo
machines, in ROM.

Now you are in BASIC mode as usual, except that two commands are available:

CALL RUN
CALL TURBO ON/OFF

"CALL" can be written as "_" (underscore). I will ise that from now on.

_RUN is the command to compile and execute the entire program on memory. If
it finds an error it will stop and yield the message.

_TURBO ON is the statement to define the beginning of the turbo block.
_TURBO OFF is the end of the block.

The turbo block is the part of the program you want to execute fast. When the
entire program contains some uncompilable statements, you can define the
block to be compiled using this set.

    EXAMPLE

    100 SCREEN 8:DEFINT A-Z
    110 BLOAD"PICTURE",S
    120 _TURBO ON
    130 FOR X=0 TO 255
    140 LINE(X,0)-(X,211),0
    150 NEXT X
    160 _TURBO OFF
    170 GOTO 170

This program cannot be "_RUN", because the "BLOAD" is one of the commands
that cannot be compiled. If you "RUN" this, the part lines 130 through 150
will be executed fast.

As '_RUN"FILE"' is not supported, you have to add _TURBO ON and _TURBO OFF at
the beginning and the end if you want to RUN"FILE" and have the effect.

    100 FOR I=0 TO 999
    110 ...
     .
     .
     .
    890 'END OF THE PROGRAM

So, this can be _RUN or add 10 _TURBO ON and 900 _TURBO OFF and RUN"FILE".

If you _RUN a program containing "_TURBO ON/OFF" it will be an error.

_TURBO ON/OFF can not be written in a multi-statement lines.

_TURBO ON/OFF can not be nested. But you may have many closed blocks in a
program.

Variables and arrays are handled differently in and outside of the blocks.
Once you are out of the block, variables and arrays used in the block are
lost. Only, the integer types can be defined as common.

    100 DEFINT A-Z:DIM C(2),D(2)
    110 A=1:B=2:C(0)=3:D(0)=4
    120 _TURBO ON(A,C())
    130 DIM D(2)
    140 PRINT A,B,C(0),D(0)
    150 A=5:B=6:C(0)=7:D(0)=8
    160 _TURBO OFF
    170 PRINT A,B,C(0),D(0)

    RUN
     1 0 3 0
     5 2 7 4
    Ok

Floating numbers used by the compiler is a special format 3-byte value. It's
accuracy is about 4.5 digits. Double precision is not available.

An array must be declared by a constant in the beginning.

This compiler works on the BASIC program on the RAM and creates the objects
and variables on the left RAM. So there is a limit of the size of the source
program about 10K. Big arrays, string variables (each uses 256 byte), CLEAR
???,&H???? will make the situation tighter as you can imagine. The compiled
objects can not be saved as independent programs.

Interrupts available, such as KEY(1) ON, OFF etc. But it will decrease the
efficiency of the executed object's size & speed.

Some statements may not work correctly.

    100 GOTO 130
    110 A=3/2
    120 END
    130 DEFINT A-Z
    140 GOTO 110

If you RUN this, A is 1. If you _RUN this, A is 1.5. DEF??? will be effective
when encountered during the execution in the case of interpreter, while it
depends on the order of line number in the other case.

A little complicated string operation may cause easily a "String formula too
complex" error. As this compiler has only one level of stack for it. Break a
long string formula into multiple small ones, if so.

If you _RUN an endless program, you can not stop it. Make a part to check
keyboards.

    100 GOTO 100 'Reset or power off to stop

    100 IF INKEY$="" THEN 100
    110 END
    is better.

2. Difference from MSX-BASIC interpreter

List of statements, commands and functions that can not be compiled.

AUTO, BASE, BLOAD, BSAVE, CALL, CDBL, CINT, CLEAR, CLOAD, CLOAD?, CLOSE,
CONT, CSAVE, CSNG, CVD, CVI, CVS, DEFFN, DELETE, DRAW, DSKF, EOF, ERASE, ERL,
ERR, ERROR, EQV, FIELD, FILES, FPOS, FRE, GET, IMP, INPUT#, KEY LIST, LFILES,
LINEINPUT#, LIST, LLIST, LOAD, LOC, LOF, LPRINT USING, LSET, MAXFILES, MERGE,
MOTOR, MKD$, MKI$, MKS$, NAME, NEW, ON ERROR GOTO, ON INTERVAL GOSUB (due to
a bug), OPEN, PLAY, PRINT#, PRINT USING, PUT KANJI, RENUM, RESUME, RSET,
SAVE, SPC, TAB, TRON, TROFF, WIDTH.

List of those with limits.

CIRCLE: Start, end angles and aspect ratio can't be specified.
COPY: Only graphic COPY is available.
DEFDBL: Same as DEFSNG.
DIM: Must come first in the program or in the turbo block.
INPUT: Can handle only one variable at the time.
KEY: ON KEY GOSUB, KEY(n) ON/OFF only.
LOCATE: x,y must be given in as a set. No cursor switch parameter.
NEXT: Variable names after the NEXT can not be omitted.
ON: ON STOP GOSUB, ON INTERVAL GOSUB not available.
PRINT: Commas work in a different way. No wrapping for digits.
PUT: PUT SPRITE only.
RUN: Variables won't be initialized.
SCREEN: Screen mode and sprite size only.
SET: SET PAGE only.
STOP: Same as END.
USR: Parameter type must be integer only.
VARPTR: File number can not be given as the parameter.

Otherwise there is no significant difference.

In general, I/O commands & functions, and editing commands can not be
compiled. Of course they are available in the direct mode, and outside of the
turbo block. You can edit, debug and save a program in MSX-BASIC and execute
it by _RUN.

If you want to use PRINT# to write characters on GRP:, use it outside of
turbo block. Otherwise study the sample, "PRINT.TRB".

If you want to use PLAY, use BGM compiler, and get the sound by USR(n).

3. New features added

3 special commands are available by starting a remark line with some specific
characters.

#I
Stands for INLINE. You can write a short machine-language routine.

    100 DEFINT A-Z
    110 K=1
    120 '#I &H2A,K
    130 '#I &HF3,&HCD,@150,&HFB
    140 END
    150 'SUB
    160 RETURN

120 means LD HL,(K); K must be a simple variable of integer type.
130 means DI
      CALL @150 ;Be careful, this line won't be RENUMed.
      EI

#C
Stands for CLIP. In the screen modes 5 through 8 (except for PAINT, and
CIRCLE), this will be set clipping on and off.

    10 SCREEN 8
    20 '#C-
    30 LINE(0,0)-(255,255) 'Y CLIPPED
    40 IF INKEY$="" THEN 40
    50 '#C+
    60 LINE(0,0)-(255,255) 'NOT CLIPPED
    70 IF INKEY$="" THEN 70

#N
Check if NEXT overflows.

    10 FOR I%=0 TO &H7FFF
    20 NEXT I%

This program will end up in a "Overflow error" in MSX-BASIC. And if _RUN, it
will be an endless loop. If #N+ is specififed, it will end normally. This
code will decrease the efficiency of the object, too. Better not use unless
it's really necessary. To clear, specify #N-.

NOTE: In MSX-2+ Sanyo you can found a new command:

CALL BC

This command turn on the BASIC COMPILER options.

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("fmpac-manual", "fmpac-manual", "FM-PAC", "", "FM-PAC: manual do cartucho FM Pana Amusement Cartridge",
        R"MAN(FM-PAC (Homesoft)

             ORIGINAL PANA AMUSEMENT CARTRIDGE FOR MSX2 COMPUTER's
                                 page # of 48

HOMESOFT - HOME SOFTWARE BENELUX bv
kuppersweg 63065
2031 EB Haarlem/Holland

Telefoon (023) 311 241
Telefax  (023) 318 488
Databank (023) 319 517

 Foreword:

Welcome!  Thank you  for bying our FM Pana Amusement Cartridge.  The  FM
Pana Amusement Cartridge is an FM sound adaptor with S-ram (static) that
can  be  used with either MSX2 computers that have 32K of Ram  or  more.
Please read this manual before

Keep this manual near and refer to it when necessary.

S-Ram mark for games

MSX music mark

MSX is a registrated trademark of ASCII Inc.

 Some of the things that we can do with the Cartridge

 S-RAM (-> page 7)

When  used with games that have the S-RAM mark,  the data from the  game
 may be saved to the S-RAM in the cartridge and the game may be continued
 at a later time.

 FM-sound (-> page 9)

When  used  with  games  that have the Music  mark  you  can  experience
 exciting sounds and stimulating music.

 FM- sound extended BASIC (-> page 34)

You can make use of the FM sound functions with Basic

------------------------------------------------------------------------
The  FM  Pana Amusement Cartridge consist of so many letters it  may  be
 hard  to remember.  So for convinience sake,  we will refer to it as the
 FMPAC  from  now on.  We will also refer to  the  S-RAM  PAC,  mentioned
 earlier, and the FMPAC as just
------------------------------------------------------------------------

  Contents                                                Page

Be careful of these points!                                 4

Some difficult terms                                        5

Installing and removing the FMPAC                           6

Using the PAC's with games                                  7
      Use with games taht are S-RAM compatible              7
      USE with games that have the music mark               9

Built in software (PAC commander)                           10
      The magical (PAC commander)                           10
      Starting and ending the PAC commander                 11
      Using the PAC commander                               13
      Checking the slotnumber                               15
      Spell 1: (Clear)                                      16
      Spell 2: (Copy)                                       18
      Spell 3: (Change)                                     26
      Spell 4: (Delete file)                                27
      Spell 5: (Slot)                                       29
      Spell 6: (BGM)                                        30

      What to do when you see error messages                31

FM-sound extended BASIC (MSX-MUSIC)                         34
         (MSX-MUSIC) commands                               35
         Sample programs                                    41
         MML table                                          42
         Sound data table                                   44
         Temper table                                       45

Specifications                                              46

Trouble shooting                                            47

General Care                                                48

  Be careful of theese points!

The FMPAC has it's weaknesses.

Avoid the following to keep your FMPAC:  Healthy.Ps

Keep the FMPAC out of direct sunlight

Do not spill liquids in the FMPAC

Do not drop or apply shock to the FMPAC

Do not remove the screews or open the case

If the FMPAC does not seem to be working proberley, refer to page 47

  Some difficult terms.

You  know  that your FMPAC is for use with MSX computers.  When you  are
 reading  this  manual.  You may come across some difficult  words  about
 computers or special terms.  We will explain the Important words here so
 take  note  and  remember them

 Data

Information. For example: if you save a game in the middle of plays, the
 stage number and the level will become information.

 S-RAM

Something that will preserve game data for later use.  It is like a memo
 pad.

 File

What you call data when it is read or copied from a floppy disk.  A file
 will always have a name.

 Floppy Disk

Something used to write instructions or files from a computer.  It works
 something like a casette drive.

 Save

This is what you call the writing data to the S-RAM or Floppy Disk

 Load

This is what you call the reading data (file)

  Installing and removing the FMPAC

The  FMPAC is inserted into the MSX slot.  The slot that can be used may
 be different for each game so refer to the game's manual

------------------------------------------------------------------------
When inserting and removing a cartridge into the slot,  be sure that the
 power of the computer is turned OFF!!!
------------------------------------------------------------------------

 Installation

Refer to the computeres manual and check how to insert Cartridges.

 Removal

Hold the computer dowm with one hand and remove with the other.

  Using the PAC's with games

Use the games that are S-RAM compatible

If you use this cartridge with games that have the S-RAM compatible mark
 on them, you will be able to save data into the FMPAC

------------------------------------------------------------------------
Different  games you use may have different ways of using the  S-RAM  so
 you should read the game manual for details.
------------------------------------------------------------------------

Data  that is saved in the middle of a game is stored in the portion  of
 the  FMPAC that is called the S-RAM.  The S-RAM id divided into 8  small
 rooms (segments) and each game uses a seperate room. The room to be used
 by the game is explained in th

 Example:

Pana Amusemnet Cartridge
1 2 3 4 5 6 7 8

The number that is shown in inverse white on black,  is the room that is
 used by that game.

If you save a game into a room that has been used before,  the data that
 was there before will be replaced with the new data.

------------------------------------------------------------------------
When  using  the FMPAC with 2 games or more read the following  page  to
 make sure that you don't erase data from the other games.
------------------------------------------------------------------------

  Using the FMPAC with games

When using the FMPAC with 2 or more games

One  FMPAC can save up to 8 data but games tahat use rooms which overlap
 with each other cannot be used together.
For  example  game A uses room 2 for it's data and another game  B  uses
 room 4 and 5 to save its data then since rooms 2 4 and 5 are used theese
 games  can use the same FMPAC and another game C must use rooms 1 and  2
 the data in room 2 from game A

  Use with games that have the music mark

Using  the  FMPAC with games having the MSX-MUSIC mark will  have  great
 sounds and exiting music!!

When  you  play games on your computer,  you notice that the games  have
 neat  sounds  and music.  Theese sounds are made by the build  in  sound
 functions in your computer called PSG sounds. The FM sound function that
 are  in  your  FMPAC sounds mo

If  the volume of the PSG sounds and the FMPAC sounds do not  match  up,
 use the volume control switch on the FMPAC to adjust the volume.

Move the switch to the left to lower the FM sound volume

Move the switch to the right to increase the FM volume

Volume selection switch

Turn  off  the switch before adjusting,  Set the volume switch where  it
 clicks into place.

  Build in software (PAC commander)

The magical PAC commander

the PAC commander has 6 spells that it can use

Clear     This is used to erase data saved with the FMPAC or other PAC's
Copy      This is used to copy data to another PAC or to a Floppy disk
          You can use it to also do the opposite, to copy data from a
          Floppy disk to a PAC
Change    This is used to switch data on two different PAC's
Delete    File is used to erase a file data saved onto a floppy disk
          from a PAC
Slot      This is used to check which slot the FMPAC has been installed
          into
BGM       This is used to change or stop the background music that is
          playing when using the PAC commander.

  Build in software (PAC commander)

Starting the commander

1. Make sure the power of the computer is turned off

2. Insert the FMPAC into a slot

If  you  are going to be using 2 PAC's to copy or  change,  Insert  both
 PAC's first.

 Don't insert cartridges other tham FMPAC's or PAC's !!

3. Turn ON the power on the computer

When  the  power  is  turned on and the  build  in  software  screen  is
 displayed start up BASIC

4.  When  the  Initial BASIC screen is showm enter the following at  the
     keyboard.     CALL FMPAC    and press enter

 BASIC Initial screen

5. this screen tells you that the PAC commander is active.

When  using the PAC commander,  do not use the pause key or the  control
 key functions.

  Build in software (PAC commander)

Ending the PAC commander

To end the PAC commander use the following steps

1.  Make  sure  the  screen is the same as the bottom  pictures  in  the
 previous page (The Menu Screen)

If there is a message displayed on the screen, press wither the space or
 the ESC key to erase the message.

2. Turn off the power on the computer.

If  you are not going to use the FMPAC right away,  remove it  from  the
 slot.

  Build in software (PAC commander)

Using the PAC commander

The PAC commander is very simple to use. All you have to do is press the
 keys described below to cast it's spells.

 Keys for the PAC commander

(^) (v) (cursor keys)    Used to choose which spell to cast
                         and what to cast the spell on.
(Space bar)              Used to tell the PAC commander to carry out
                         your orders.
(ESC)                    Used to tall the PAC commander that you
                         have made a mistake and you want to
                         cancel your order.

 BASIC use

Cursor on

CLEAR
COPY
CHANGE
DELETE FILE
SLOT
BGM

Lets  say that what is shown above is the PAC commander screen.  We will
 call this the spell screen.  Use the cursor keys to move the green color
 bar  up  and  down to choose which spell you wish  to  cast.  Press  the
 Spacebar when the cursor is at

 Build in software (PAC commander)

The sub-menu and messages

According to what spell you choose sometimes the spell will not be  cast
 imediatley,  but  another  sub-menu  like 1 and 2 will be shown  on  the
 screen.

1.   PAC -> PAC
     PAC -> FLOPPY
     FLOPPY -> PAC

2.   YES
     NO

Like the spell menu,  the items on the sub-menu is selected by using the
 cirsor  keys  and  pressing the Spacebar will  carry  out  your  choice.
 Pressing the ESC key will clear the sub-menu screen

3.  this spell requires another PAC
    is what MSX said

3 is called a message. The messages are shown while thr spells are being
 cast and messages will be erased when you press the spacebar and the ESC
 key.

Messages give you important information so make sure to read them!

  Build in software (PAC commander)

Checking the slot number

When  using  the PAC commander.  MSX will tell you that the FMPAC is  in
 slot  1  or  slot  2.  But sometimes the slot being  used  will  not  be
 compatible  with  the  computer being used.  When first  using  the  PAC
 commander, use the spell as sh

Now lets explain the 6 spells

It's in slot 2 but something's not right

The PAC is in slot 1 ??

whats wrong ??

is your computer allright.

  Build in software (PAC commander) / Clear

Spell 1 (CLEAR)

The  clear spell is a terrible spell that clears all data that has  been
 saved in the PAC. Use this spell carefully.

1. Set the cursor to CLEAR on the spell menu and press the spacebar
2. the following 2 menu's will appear on the screen

The passwords and data in slot 2    YES
will be erased. Are you sure        NO

says MSX

The number 1 will change according to what slot the PAC is in.

if there is more than 1 PAC installed the following will be shown

Which PAC to clear.

FMPAC    1
PAC      2

Move the cirsor to the one you want to clear.

  Build in software (PAC commander) / Clear

3   Make  sure you have choosen the right slot and PAC  then  press  the
     Spacebar

If  the PAC is not correct,  press either the ESC key or move the cirsor
 to NO and press the Spacebar.

4.  The following message will be shown on the screen

MSX has cast the CLEAR spell    CL-CL-CL-Clear

5    Press  the  spacebar and the message will be erased  and  you  will
 return to the spell menu.

  Build in software (PAC commander) / Copy

Spell 2  (COPY)

The copy spell is used to copy data from PAC to PAC and from a PAC and a
 floppy  sik.  Copy means to make the same set of data in another  place.
 This spell can also erase data so be carefull when using it.

First choose the type of copy you want to do.

You will choose what kind of copy you want to do from a submenu but what
 happens after that will depends on what you choose.

1.  Move the cursor on the spell menu to copy and press the spacebar
2.  the followint submenu will be shown on the screen

PAC -> PAC         From PAC to PAC
PAC -> FLOPPY      From a PAC to a floppy disk
FLOPPY -> PAC      From a floppy disk to a PAC

3. Make your selection by using the cursor keys and press the space bar.

  Build in software (PAC commander) / Copy

Be  carefull on theese following points when using either PAC ->  FLOPPY
 spell or FLOPPY -> PAC
If you have more that one PAC installed the computer will ask you  which
 PAC  to copy from or which PAC to copy to.  Make your selection with the
 cursor keys and press the spacebar.  Otherwise, it will be just the same
 as when you have only one PAC

If you Choose PAC -> PAC you wont be able to use thsi spell if you  have
 only 1 PAC

1  When you choose thsi type of copy the following message will be shown
 choose which PAC to copy from and press the spacebar.

FMPAC    1     Pac in slot 1
PAC      2     Pac in slot 2

2   The PAC to copy will be shown and check to see if it's correct  then
 press spacebar.

If  you  have  3 or more PAC's installed use the cursor keys  to  select
 whick PAC to copy to.

  Build in software (PAC commander) / Copy

3  The following message will be showm on the screen

The password and data in 1 will be erased
are you sure says MSX

if you are sure press the spacebar

if you have choosen the wrong PAC to copy to press either the ESC key or
 move the cursor to no and press the spacebar. Start over from number 2

4  The following message screen will be shown on the screen telling  you
 the copying has ended. Press the spacebar to erase the message.

MSX casts the COPY spell
C-c-c-c-Copy

  Build in software (PAC commander) / Copy

You cannot use this copy spell unless you have a floppy disk drive

1  Insert a formatted (initialized disk into the disk drive
   you  cannot use a brand new floppy disk as it is.  Read the  computer
    manual or the disk drive manual to learn how to format a disk.
   if you have more than 1 drive connected, insert the disk into drive A

2   Move  the  cursor  to PAC -> Floppy  and  press  the  spacebar.  The
     following message will appear on the screen.

Copy to file :________  <- input characters

This  message is asking you to give a name to the data,  you must give a
 name to the data whenever you save data to a floppy disk.  The data that
 you  named  is called a file.  If the data does not have  any  name  the
 computer will not be able to t

3  Press the letter keys and give the data a name. The letters you press
 will  appear  on  the screen.  Your name for the data can have up  to  8
 letters. You can use all the capital letters an numbers.
If you make a mistake press BS backspace key and enter the letter again.

  Build in software (PAC commander) / Copy

4  When you have named the file press the spacebar the following message
 will be shown on the screen

MSX has cast the Copy spell
C-c-c-c-Copy

when the copying has ended the message will disappear

If  you have copied files onto the disk beforem the names of  the  files
 will be shown on the screen during step 2

GAME001   1988-10-21  17:30
MYGAME    1988-10-22  19:12
LASTGAME  1988-10-22  22:02

FILE NAME DATE COPIED TIME COPIED

The date and time og the file copied is taken from the computer.  If the
 date  and  time set on the computer is wrong the date will set  to  that
 woring  time and date.  Read the computer manual to fing out how to  set
 the data and time.

  Build in software (PAC commander) / Copy

If  the file that you are going to save has the same name as a  previous
 saved file the earlier file will be lost.  This is called writing over a
 file. When there are many files on the disk it is easy to make a mistake
 and write over an old file.  B

Writing over files

1  In step 3 on page 21 pressing the corsor keys will make the cursor to
 appear  on the file list.  Move the cirsor to the file that you wish  to
 write over and press the spacebar.

2  The message shown below will appear on the screen

File name
xxxxxxxxxx write over this file
MSX asks

3   Check  to  see if the file to write over is correct then  press  the
     spacebar

If  the file is not correct press either the ESC key or move the  cursor
 to  no  and  press the spacebar and start over from step  1  and  simply
 follow the steps above.

  Build in software (PAC commander) / Copy

FLOPPY -> PAC copy

You cannot use this type of copy unless you have a disk drive.

1  Place the disk that you wish to copy from into the drive
   if you have more than 1 drive place it in drive A

2  Moce the cirsor to FLOPPY -> PAC and press the spacebar

3   The  list og files on the disk will be shown on the screen  and  the
     name of the file that the cursor is currentley at will be displayed

GAME001     1988-09-01    18:05
GAME002     1988-09-10    17:10
GAME003     1988-09-05    22.20

CURSOR      FILE TO COPY FROM GAME001   <- Character cursor.

  Build in software (PAC commander) / Copy

4  Move the cirsor to the file you want to copy and press the spacebar

5  the following message will be shown on the screen The slot the PAC is
    in

The password and data in 1 will be erased
are you sure asks MSX

6  If the PAC to copy to is correct press the spacebar

If you have the worng PAC selected, press either the ESC key or move the
 cursor to NO and press spacebar and start over

7  The following message will be shown

MSX has cast the Copy spell
C-c-c-c-Copy

When  the  copying  has ended the message will be erased  and  you  will
 return to the spell menu

The list of files is the same as PAC -> FLOPPY  look at page 22 for  the
 explanation.

  Build in software (PAC commander) / Change

Spell 3 (CHANGE)

Change  is a powerfull spell that will exchange (swap) the contents of 2
 pac's in less than a moment.  If you use this spell carelessly you  will
 loose  track  of what data is in which PAC so be carefull,  if you  have
 many  PAC's it's a good idea n

This Spel can be used only if you have 2 PAC's inserted in the slots  if
 you  have only 1 PAC installed,  turn OFF the power of the computer  and
 then insert the other PAC.

1  Move the cursor on the spell menu to Change and press the spacebar

2  The following message will be shown on the screen

MSX casts the change spell
CH-ch-ch-Change

3   Press  the spacebar to clear the message  You have just changed  the
     data

  Build in software (PAC commander) / Delete file

Spell 4 (DELETE FILE)

The delete file spell is used to erase files on a floppy disk.  Use thsi
 to remove unvanted files on a disk.  Make sure you don't erase the wrong
 file.

This spell cannot be used unless you have a disk drive.

1  Insert the disk with the unvanted files into the disk drive
   if you have more than 1 drive insert the disk in drive A

2   Move  the  cursor to Delete file on the spell  menu  and  press  the
     spacebar.  The files that have been copyed onto the disk before will
     be  listed  on the screen.  Move the cursor to the file you want  to
     erase and press spacebar.

3   The following message will be shown on the screen

FILE name

xxxxxxxx delete this file
asks MSX

  Build in software (PAC commander) / Delete file

4   Make sure the file shown in the message shows the correct  file  and
 press  the  spacebar.  The file you have choosen will be erased and  the
 following message will be displayed

MSX has cast the DELETE file spell
DE-de-de-Delete

If  you change your mind about the file to delete press the ESC  key  to
 cancel  or  move the cursor to NO and press the spacebar and start  over
 from step 2

check to see if you have the right file name.

  Build in software (PAC commander) / Slot

Spell 5 (SLOT)

This  spell  is used to find out what slot a PAC is in.  It is  used  to
 check  to see if the slot of the PAC commander matches the slot  of  the
 computer.

This  spell  can be used only if 1 PAC is installed.  If you have  other
 PAC's installed, turn off the power and remove the ther PAC's

1   Move the cirsor to slot on the spell menu and  press  spacebar.  The
     following message will be displayed on the screen.

MSX has cast thew slot spell. PAC is in slot 1  write this down
says MSX

2  After you have written down the slot number press either the spacebar
 or the ESC key to clear the message.

This  number  may be different from the slot number on the  computer  so
 write the number down or paste the number next to the slot for reference
 when using the FMPAC

The PAC commander will use this number so note this down

  Build in software (PAC commander) / BGM

Spell 6 (BGM)

This spell is used to change or stop the BGM = Back ground music playing
 when using the PAC commander

1   Move  the  cursor to BGM on the spell and press  the  spacebar.  The
     following sub-menu will be shown

SAMPLE 1
SAMPLE 2
SAMPLE 3
SAMPLE 4
SAMPLE 5
STOP

2    Move  the cursor to the song that you want to hear  and  press  the
 spacebar. The spell will be cast immediatley

If the power switch is turned OFF the music selection will automatically
 be set to music 1
Use the volume control on the television to adjust the music volume.

  What to do when you see error messages

While  using  the  PAC commander,  you may see  the  following  messages
 printed  in  yellow.  These are called error messages and they tell  you
 that something is wrong with your selections.  If any of theese messages
 should appear try one of thr f

If the trouble occeur with the drive name, format or write protect check
 with the either the computer or the disk drive manual.

Oh-oh the disk is write protected
try again

The write protect tab on the floppy disk is set to the protect position.
Remove the disk from the drive and set the tab to non protected position
 so that the hole in the disk cannot be seen.

There is no disk in the drive
try again

You must have the disk in drive A .   It will not work even if the  disk
 is placed in drive B or C.

The  disk  in the drive may not be formatted.   You must format  a  disk
 before using it.

Cannot find the file
says MSX

The  file  name  could be wrong.  Check the file name with the  list  of
 files.

  When error messages appear

This disk cannot be used with MSX computers
try again

This disk is broken
try again

Have you put in a disk from a different computer by mistake??.  The disk
 to use must be formatted initialized on your type of computer.  If  thsi
 message appear even if you have done so, then that disk may be broken so
 try another disk.

This spell requires another floppy or PAC to work
says MSX

Turn  off the power and connect the disk drive or insert another PAC and
 try again.

This spell requires a floppy
says MSX

Turn  oof  the power on the computer and attach the disk drive  and  try
 again.

  When error messages appear

This spell requires another PAC
says MSX

Turn off the power and insert another PAC and try again

There are 2 PAC installed,  turn OFF the power,  remove one of the PAC's
 and try again

Turn off the power and leave 1 FMPAC installed and try again

Not enough room on the disk
says MSX

The disk has become full. Deletesome unvanted files or use another disk.

The file type seems strange
says MSX

Files that have been saved by other means cannot be used.  Always  files
 that have been saved using the PAC format copy.

Not enough memory

The FMPAC requires at least 32K ram to work (data area)
If  this message occurs even if you have over 32Kof memeory,  disconnect
 any thing that you have attached to the computer (disk drives etc.)  and
 try again.

  FM sound extended BASIC

         MSX-MUSIC

The  FM sound extended BASIC,  MSX-MUSIC is made so that you can use the
 FM sounds in the FMPAC with your program an commands. Using MSX-MUSIC to
 produce  sounds will allow you to create beautiful sounds  not  possible
 with the PSG.

If  you have never composed music using BASIC before,  reading the BASIC
 manual  on  the play command will give you a understanding  on  creating
 music.

There  is  a semilar extension of BASIC call MSX-AUDIO but  they  are  a
 little different so take care not to mix them up.

  FM sound extended BASIC

         MSX-MUSIC

MSX-MUSIC commands

Reading the instructions

The commands will be explained as the example below.

Example:

Command: MUSIC ------ command name

Operation ( what the command does)

Format: ( how the command is used
        CALL MUSIC [(<MODE>[, ...])]

Sample  (example)

Notes   (notes on the command)

In  the  format  of the command,  the following symbols  are  used.  The
 symbols are not to be entered but mean the following.

[ ] --- characters and symbols within the bracket are optional
< > --- characters within theese must be entered in thsi position
"..." - characters that are repeated.

  FM sound extended BASIC

         MSX-MUSIC

 Command:   MUSIC

Operation: Starts up MSX-MUSIX and determines how the channels are to be
            divided and used. This command us used first.

Format:    CALL MUSIC [(<MODE>[,0[,<n1>[,<n2>[,......[,<n9>]]]]]]]]]])]

Modes may be 1 or 2
1 selects rhythm
The total number when adding n1-n9 must be 9 or less in mode 0
and 6 or less in mode 1 (n1-n2 specifies the number of channels used.

Sample:   CALL MUSIC
          3  channels are given different melodies to play and  3  other
           channels are prepeared to play rhythm
          [same as CALL MUSIC(1,0,1,1,1)]

          CALL MUSIC (0,0,1,1,1,1,1,1,1)
          9 channels are set to play different melodies ando no channels
          are set for rhuthm.

Note:     n1-n9 are allocated to MML1-MML9 for the PLAY statement.

  FM sound extended BASIC

         MSX-MUSIC

 Command BGM

Operation   Specifies to allow other commands to be executed while music
             is playing

Format:     CALL BGM(n)  n is 0 or 1 or variable of those values
                         default for MUSIC is 1

Sample:     CALL BGM(0)  no other command can be executed while
                         music is playing.

            CALL BGM(1)  commands can be executed while music
                         is playing.

 Command PITCH

Operation   This sets the pitch of the sounds to be played with the FM
            sounds. All sounds will be adjusted to this pitch.

Format:     CALL PITCH(n) n can be from 410 to 459 (or a variable)
                          the default value is 440 (standard A)

sample:      CALL PITCH(450) Sets the pitch of the sound to be played at
                              450Hz

  FM sound extended BASIC

         MSX-MUSIC

 Command PLAY

Operations:  Music will be played according to the MML settings

Format:      Play [#<MODE>,]<MML 1>[,....[,<MML n>[,<MMLm>[,<MMLp1>...[,
             <MMLp3>]]]]]]]]]]]]
             <MODE> can be 0,2,3
             <MML1> through <MMLn> are FM sound MML's
             <MML> numbers are the same as the values for CALL MUSIC
             <MML m> is for the rhythm MML
             <MML p1> through <MML p3> are for the PSG MML

Sample:      Play #2, "CD", "EF", "GA"

Notes:  If <MODE> is set to 0 or not set, PSG will be played
        only MML p1-3 can be used
        if <MODE> is set to either 2 or 3 both FM and PSG can be used
        MML 1-2 are set according to the MUSIC command as notes from
        the  starting  order.  However if a mode that does not  use  the
         rhythm  channels  is  specified,  the rhythm  channels  must  be
         omitted.

Function:     PLAY

Operation:    This  detimes  if  music is being played and  returns  the
               result as a value.

Format:       CALL PLAY (n,x)    n is the channel to check
                                 x is the name of the variable to
                                   store the value in.

Sample:       CALL PLAY(0,A):PRINT A

              When n=0 if any channels are playing, the value -1
              will be stored in A and 0 if no channels are playing.

  FM sound extended BASIC

         MSX-MUSIC

 Command STOPM

Operation:   Stop play of BGM

Format:      CALL STOPM

Sample:      CALL STOPM

 Command TEMPER (temperament)

Operation    This sets the rhythm for the FM sound

Format:      CALL TEMPER (n)   n can be from 0 to 21 or a variable
                               default value is 9

Sample:      CALL TEMPER (0)

Notes:       for a list of rhythms that can be set and their vlaues, see
              page 45 "Table of Rhythms.

 Command TRANSPOSE

Operation:   Will tranpose in units of half tones 1/100 units

Format:      CALL TRANSPOSE (n)
             n can have values from -12799 to 12799 or a variable
               However, some voices have a limited setting even
               through within the +-12799 range
               Default setting is 0

Sample:      CALL TRANSPOSE (100)  tranposes a half tone.

  FM sound extended BASIC

         MSX-MUSIC

 Command VOICE

Operation:    Stes  what  instrument <Voice> is to be played by  the  FM
               sound channels.  However,  values without * mark and array
               values  (refer  to  page  44 are limited to  2  when  used
               together.

Format:       CALL VOICE ([@n1], [@n2], .... [@n9]
              n1-n9 can be values from 0-63 (voice nr) or variables

              default setting is @0

Note:         The   voice number canbe selected from the table  on  page
               44's Table of voices numbers.

 Command VOICE COPY

Operation:    This   command  copies the voice data to number 63  or  an
               array variable.

Format:       CALL VOICE COPY (@n1, -n2)
              n1 can be voices from 0-63 that have no * mark or array
              variables.
              n2 can be 63 or an array variable name.

Sample:       DIM A%(16)
              CALL VOICE COPY (@7,A%)
              pipe organ 1 is copied to array variable A.

Note:         choose the voice from the table on page 44.

  Sample Program

Here is an example using the MSX-MUSIC. The first 8 measures of the song
 Flower  is shown below.  Refer to thsi song and the MML table and  enter
 your favorite song to be played with FM sounds.

 MML Table

This  is a table of the MML (music macro language) that can be used with
 the PLAY command.

Music MML

 Statement  Meaning                      Range       Default Value *1

Mn         Envelope cycle setting *2    1>=n<=65535 m255
Sn         Envelope shape *2            0<=n<=15    S0
Vn         Volume                       0<=n<=15    V8
Ln         Lenght                       1<=n<=64    L4
Qn         Division of sound lenght     1<=n<=8     Q8
On         Octave                       1<=n<=8     O4
>          Raise 1 Octave                  -        -
<          Lower 1 Octave                  -        -
Tn         Tempo setting                32<=n<=255  T120
Nn         Plays Note raised to n       0<=n<=96    -
Rn         Rest setting                 1<=n<=64    R4
A-G        Note                         -           -
+, #       Sharp (raise half tone)      -           -
-          Flat (Lower half tone)       -           -
(period).  Lengthen note or rest by 1.5 -           -
XA$        Plays MML stored in string   -           -
           variable A$ *3
=x;        sets parameter to x          *4          -
&          Tie                          -           -
{}n        will generate even notes of  1<=n<=64    value set
           n for the number within                  with Ln
           the {}
@n         changes the voice set to n   0<=<=63     -
@Vn        sets detail change in volume 0<=n<=127   -
@Nn        maintain lenght set by n     1<=n<=64    value set
                                                    with Ln

*1 Initial value set when CALL MUSIC is used
*2 For PSG only
*3 Adding MML after [XA$;] will result in error
*4 The   value range is dertimined by the preceeding MML but  it  cannot
    exceed the value 32767

  RHYTHM MML

There  are  5  different  voice  that  are  available  for  the   rhythm
 (percussion)  MML and up to 3 voices may be played  simultaneously.  For
 this reason the rhythm MML first lines up the instruments that are to be
 played  simultaneously and the

 Statement     Meaning                         Range     Default

B             Bass drum                       -         -
S             Snare drum                      -         -
W             Tom tom                         -         -
C             Cymbals                         -         -
H             Hi Hat                          -         -
n (number)    previous notes are played and
              n th note are rested            1<=n<=64  -
Vn            Volume                          0<=n<=15  8
!             Accents preceeding note                   -
@An           Sets volume for voices that     0<=n<=15  -
              are accented.

[Tn], [@Vn], [Rn], [XA$;], [=x;], [.(period)] is the same as music MML

example:      PLAY#2,"","","","BSH8H8H8S!H!8H8"

Bass, snare,hihat and wait an 8th note
Hi hat and wait an 8th note
snare, hi hat plays accented and waits an 8th note
Hi hat and wait an 8Th note

  VOICE DATA TABLE

This is a list of the voices available for use with Voice command or the
 MML @ statement.

Voice No.  Name               Voice no.  Name

0 *        Piano 1            32         Piano 3
1          Piano 2            33 *       Wood Bass
2 *        Violin             34         Santule 2
3 *        Flute 1            35         Brass
4 *        Clarinet           36         Flute 2
5 *        Oboe               37         Clavicord 2
6 *        Trumpet            38         Clavicord 3
7          Pipe Organ         39         Koto 2
8          Xylophone          40         Pipe Organ 2
9 *        Organ              41         P0hdsPLA
10 *       Guitar             42         P0hdsPRA
11         Santule 1          43         Church Organ L
12 *       Electric bass      44         Church Organ R
13         Clavicord 1        45         Synth Violin
14 *       Hapsicord          46         Synth Organ
15         Hapsicord          47         Synth Brass
16 *       Vibraphone         48 *       Horn
17         Koto 1             49         Shamisen
18         Taiko              50         Magical
19         Engine 1           51         Fuwawa
20         UFO                52         Wonderflat
21         Synth Bell         53         Hard Rock
22         Chime              54         Machine
23 *       Synth Bass         55         Machine V
24 *       Synthesizer        56         Comic
25         Synth drum         57         SE comic
26         Synth rhythm       58         SE laser
27         Harmo drum         59         SE noise
28         Cow Bell           60         SE star 1
29         Hi Hat             61         SE star 2
30         Snare drum         62         Engine 2
32         Bass drum          63         no sound

*  The name of the voice are given for reference only and they  may  not
 sound the same as the real instruments

*   Are sounds that are build in the FM sound chip and sounds without an
 * indicates sounds generated by software.

  RHYTHM TABLE

Table of Rhythm usable with the TEMPER statement

No.     Rhythm

0       Pythograph
1       Mintone
2       Welkmeyster
3       Welkmeyster (adjusted)
4       Welkmeyster (seperate)
5       Kilanbuger
6       Kilanbuger  (adjusted)
7       Velotte Young
8       Lamour
9       Perfect Rhythm (default)
10      Pure Rhythm  c major    (a minor)
11      Pure Rhythm  cis major  (b minor)
12      Pure Rhythm  d major    (h minor)
13      Pure Rhythm  es major   (c minor)
14      Pure Rhythm  e major    (cis minor)
15      Pure Rhythm  f major    (d minor)
16      Pure Rhythm  fis major  (es minor)
17      Pure Rhythm  g major    (e minor)
18      Pure Rhythm  gis major  (f minor)
19      Pure Rhythm  a major    (fis minor)
20      Pure Rhythm  b minor    (g minor)
21      Pure Rhythm  h minor    (gis minor)

  Specifications

Item:          Description

Machine:       MSX, MSX2 personal Computers
               32K byte of Ram

Conditions:    Temperature 10'C-35'C

Humidity:      20%-80% without condensation

Battery life:  5 years approx

Dimensions:    210 X 1008 X 17  mm

Weight: (g)    140 (g) approx

FM sound:      2 operator FM sound generation

S-RAM:         MSX game specific 8 segment S-RAM
               RAM capicity 8Kbytes (64K bit)
               back up battery function to preserve data

BUILD in SOFT: Clear
               Copy
               PAC->PAC, PAC->FLOPPY, FLOPPY->PAC
               Delete file
               Slot
               BGM
               5 Sample Songs  stop

  Trouble Shooting

Check out theese items once more. If it doesn't work again then take the
 FMPAC  to  the  store that it was purchased from and ask the  clerk  for
 advice.

Is the FMPAC plugged into the Slot firmly?

Is it in the correct Slot?

Try another slot and try again

Are you following the instructions??

Turn OFF the computer and remove the FMPAC then try again from the start

 If the data are destroyed or Changed

The content of the FMPAC S-RAM operates with the build in  battery.  The
 expected  battery  life time is 5 yesrs but something may go wrong  with
 the battery.  If data is lost or seems to be strange,  consult the store
 that you purchased the FMPAC f

  General Care

If the FMPAC should become soiled, clean it in the following manner:

Wipe the case gentley with a soft cloth,  if a rough cloth is used or it
 is wiped roughly you will damage the case.

If the case i heavely soiled,  use a little dishwashing liquid,  If  the
 case  does  not  become  clean by gentley wiping,  use  a  tiny  bit  of
 dishwashing liquid on a soft cloth an wipe the case clean with it.
Always  wipe the liquid off with a soft cotton cloth after cleaning with
 dishwashing liquid.

<HANDY MEMO>       Writing the following down.

DATE of PURCHASE .................................

Model number     .................................

Place PURCHASED  .................................  TLF ..............

Service Center   .................................  TLF ..............

FONTE
  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    return v;
}

} // namespace man
