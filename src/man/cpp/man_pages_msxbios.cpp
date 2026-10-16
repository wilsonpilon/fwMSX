// fwMSX -- MAN: rotinas e variaveis da BIOS/BDOS e hardware do MSX. ARQUIVO GERADO por tools/paleobasic/import_bios_help.py a partir de
// resource/paleobasic/src/editor/help/{BiosCalls,BiosDoc,Hardware}HelpData.pbi -- nao editar a mao.
//
// Origem: MSXBIOS.CHM do RuMSX (Lex Lechz, lexlechz.at), que reune material publico (MSX Assembly Page, manuais do blueMSX e do openMSX,
// livros da Aleph). Texto em ingles, como no original. Ver doc/man-spec.md, secao "Origem e direitos".
#include "man_internal.h"

namespace man {

std::vector<Topic> BiosTopics() {
    std::vector<Topic> v;
    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {
        v.push_back(Topic{name, aliases, category, usage, summary, page, false});
    };

    add(".data area for circle statement", "", "Variaveis RAM", "F92D MINDEL", "F92F MAXDEL",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F92D MINDEL
F92F MAXDEL
         .DATA AREA FOR CIRCLE STATEMENT
F931 ASPECT  Aspect ratio of the circle; set by <ratio> of CIRCLE
F933 CENCNT  End count
F935 CLINEF  Flag to draw line to centre
F936 CNPNTS  Point to be plottted
F938 CPLOTF  Plot polarity flag
F939 CPCNT   1/8 of number of points in circle
F93B CPCNT8
F93D CRCSUM
F93F CSTCNT
F941 CSCLXY  Scale of X & Y
F942 CSAVEA  reservation area of ADVGRP
F944 CSAVEM  reservation area of ADVGRP
F945 CXOFF   X offset from center
F947 CYOFF   Y offset from center
         .DATA AREA FOR PAINT STATEMENT
F949 LOHMSK
F94A LOHDIR
F94B LOHADR
F94D LOHCNT
F94F SKPCNT  Skip count
F951 MOVCNT  Movement count
F953 PDIREC  Direction of the paint
F954 LFPROG
F955 RTPROG

                 for maclng
F956 MCLTAB

This variable contains the address of the command table to be used
by the macro language parser. The "DRAW" table is at 5D83H and the
"PLAY" table at 752EH.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add(".wark for user function parameter", "", "Variaveis RAM", "F6AD AUTINC  Increment for auto", "F6AF SAVTXT Text pointer for resume",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F6AD AUTINC  Increment for auto
F6AF SAVTXT  Text pointer for resume
F6B1 SAVSTK  Save stack when error occurs
F6B3 ERRLIN  Line where last error
F6B5 DOT     CURRENT LINE for edit & list
F6B7 ERRTXT  Text pointer for use by RESUME
F6B9 ONELIN  LINE to go when error
F6BB ONEFLG  =1 if executing an error trap routine
F6BC TEMP2
F6BE OLDLIN  OLD LINE NUMBER set up ^C ...
F6C0 OLDTXT  Points st. to be executed next
F6C2 VARTAB  Pointer to start of variable space
F6C4 ARYTAB  Pointer to begging of array table
F6C6 STREND  End of storage in use
F6C8 DATPTR  DATA pointer
F6CA DEFTBL  Default valtype for each letter

        .WARK FOR USER FUNCTION PARAMETER

F6E4 PRMSTK  Previous definition block on stack
F6E6 PRMLEN  Number of bytes of objective table
F6E8 PARM1   Objective prameter definition table
F74C PRMPRV  Pointer to previous parameter block
F74E PRMLN2  Size of parameter block
F750 PARM2   For parameter storage
F7B4 PRMFLG  Flag to indicate whether PARM1 was searching
F7B5 ARYTA2  End point of search
F7B7 NOFUNS  0 if no function active
F7B8 TEMP9   Location of temporary storage for garbage collection
F7BA FUNACT  Count of active functions
F7BC SWPTMP  Value of first  variable in SWAP st.
F7C4 TRCFLG  0 MEANS NO TRACE

        .WORK FOR MATH-PACK

F7C5 FBUFFR  Buffer for fout
F7F0 DECTMP  Used to transform decimal integer to floating point number
F7F2 DECTM2  Used at division routine execution
F7F4 DECCNT  Used at division routine execution
F7F6 DAC      DECIMAL ACCUMULATOR
F806 HOLD8   Registers for decimal multiplication
F836 HOLD2
F83E HOLD
F847 ARG     Area to set the value to be calculate with DAC
F857 RNDX    Last random number

        .DATA AREA USED BY BASIC INTERPRETER

F85F MAXFIL  High legal file number
F860 FILTAB  Starting address of of file data area
F862 NULBUF  Points to file 0 buffer ( used in SAVE & LOAD st.)
F864 PTRFIL  Points to file data of currently accessing file
F866 RUNFLG  Non-zero for run after load
F866 FILNAM  Name for FILES, NAME & others
F871 FILNM2  Second name for NAME
F87C NLONLY  <>0 when loading program
F87D SAVEND  End address for BSAVE
F87F FNKSTR  FUNCTION KEY AREA

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("0000h", "", "BIOS-ROM", "0000H ? ID-Bytes", "These 2 bytes are used to identify the Subrom: 43H and 44H (\"CD\").",
        R"MAN(AREA: Sub-ROM do MSX2

0000H ? ID-Bytes

These 2 bytes are used to identify the Subrom: 43H and 44H ("CD").

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("0002h", "", "BIOS-ROM", "0002H ?", "Pointer to initialisation-routine of the MSX2-SubROM.",
        R"MAN(AREA: Sub-ROM do MSX2

0002H ?

Pointer to initialisation-routine of the MSX2-SubROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("0004h", "", "BIOS-ROM", "0004H ?", "Pointer to the CALL statement-handler for the MSX2-SubROM (MEMINI, MFILES, MKILL, ...).",
        R"MAN(AREA: Sub-ROM do MSX2

0004H ?

Pointer to the CALL statement-handler for the MSX2-SubROM (MEMINI, MFILES, MKILL, ...).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("0006h", "", "BIOS-ROM", "0006H ?", "Pointer to device-routine for \"MEM:\" and \"MEM0:\".",
        R"MAN(AREA: Sub-ROM do MSX2

0006H ?

Pointer to device-routine for "MEM:" and "MEM0:".

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("000ch", "", "BIOS-ROM", "000CH ?", "2 Bytes: reserved",
        R"MAN(AREA: Sub-ROM do MSX2

000CH ?

2 Bytes: reserved

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("000eh", "", "BIOS-ROM", "000EH ?", "2 Bytes: reserved",
        R"MAN(AREA: Sub-ROM do MSX2

000EH ?

2 Bytes: reserved

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("002bh", "", "BIOS-ROM", "002BH ?", "Byte 0 with version-information.",
        R"MAN(AREA: BIOS principal (MainROM)

002BH ?

Byte 0 with version-information.

7 6 5 4 3 2 1 0
| | | | +-+-+-+-- Character set
| | | |           0 = Japanese, 1 = International, 2=Korean
| +-+-+---------- Date format
|                 0 = Y-M-D, 1 = M-D-Y, 2 = D-M-Y
+---------------- Default interrupt frequency
                  0 = 60Hz, 1 = 50Hz

Bits 0-3 contain the character set. Possible values have the follwing meanings:
0000 | (&H0) | Japanese
0001 | (&H1) | International
0010 | (&H2) | Korean
&H3 to &HF are reserved

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("002ch", "", "BIOS-ROM", "002CH ?", "Byte 1 with version-information.",
        R"MAN(AREA: BIOS principal (MainROM)

002CH ?

Byte 1 with version-information.

7 6 5 4 3 2 1 0
| | | | +-+-+-+-- Keyboard type
| | | |           0 = Japanese, 1 = International
| | | |           2 = French (AZERTY), 3 = UK, 4 = German (DIN), ...
+-+-+-+---------- Basic version
                  0 = Japanese, 1 = International

Bits 0-3 contain the keyboard-type. Possible values have the follwing meanings:
0000 | (&H0) | Japanese
0001 | (&H1) | International (QUERY/other)
0010 | (&H2) | French (AZERTY)
0011 | (&H3) | American
0100 | (&H4) | German (DIN)
0101 | (&H5) | Korean
0110 | (&H6) | Danish
0111 | (&H7) | Norwegian
1000 | (&H8) | Swedish
1001 | (&H9) | Spanish / Argentinian
1010 | (&HA) | Russian
&HB to &HF are reserved

Bits 4-7 are indicating the MSX-Basic version. Please note that
a specific version has impact to the behavior of various statements
(e.g. USING-clause within the PRINT statement).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("002dh", "", "BIOS-ROM", "002DH ?", "Byte 2 with version information. This byte specifies the MSX version:",
        R"MAN(AREA: BIOS principal (MainROM)

002DH ?

Byte 2 with version information. This byte specifies the MSX version:
00 | MSX
01 | MSX 2
02 | MSX 2+
03 | MSX Turbo-R

AREA: Sub-ROM do MSX2

002DH ?

Probably version-information?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("002eh", "", "BIOS-ROM", "002EH ?", "Byte 3 with version-information.",
        R"MAN(AREA: BIOS principal (MainROM)

002EH ?

Byte 3 with version-information.

Bit 0: if 1 then MSX-MIDI is present internally (MSX turbo R only)

AREA: Sub-ROM do MSX2

002EH ?

Probably version-information?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("002fh", "", "BIOS-ROM", "002FH ?", "Byte 4 with version-information.",
        R"MAN(AREA: BIOS principal (MainROM)

002FH ?

Byte 4 with version-information.

Reserved

AREA: Sub-ROM do MSX2

002FH ?

Probably version-information?

European MSX 2 : 00H = 00000000B
Japanese MSX 2+: 2FH = 00111111B

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("00cdh", "", "BIOS-ROM", "00CDH ?", "00CDH ?",
        R"MAN(AREA: Sub-ROM do MSX2

00CDH ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("00d1h", "", "BIOS-ROM", "00D1H ?", "00D1H ?",
        R"MAN(AREA: Sub-ROM do MSX2

00D1H ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("00e5h", "", "BIOS-ROM", "00E5H ?", "00E5H ?",
        R"MAN(AREA: Sub-ROM do MSX2

00E5H ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("0119h", "", "BIOS-ROM", "0119H ?", "0119H ?",
        R"MAN(AREA: Sub-ROM do MSX2

0119H ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("01b9h", "", "BIOS-ROM", "01B9H ?", "Not used... Reserve entry",
        R"MAN(AREA: Sub-ROM do MSX2

01B9H ?

Not used... Reserve entry

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("1c", "", "BDOS", "1C-20 do not exist", "1C-20 do not exist",
        R"MAN(AREA: BDOS (MSX-DOS)

1C-20 do not exist

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("25", "", "BDOS", "25 does not exist", "25 does not exist",
        R"MAN(AREA: BDOS (MSX-DOS)

25 does not exist

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("29", "", "BDOS", "29 does not exist", "29 does not exist",
        R"MAN(AREA: BDOS (MSX-DOS)

29 does not exist

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("32", "", "BDOS", "32-3F do not exist", "32-3F do not exist",
        R"MAN(AREA: BDOS (MSX-DOS)

32-3F do not exist

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4020h", "", "BIOS-ROM", "4020H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4020H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4022h", "", "BIOS-ROM", "4022H ? Jump to BASIC", "SCSI: this function may not be supported by all ROMs",
        R"MAN(AREA: ROM de disco (Disk-ROM)

4022H ? Jump to BASIC

SCSI: this function may not be supported by all ROMs
(e.g. NOVAXIS simply returns from this call where HSH-SCSI
supports it).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4023h", "", "BIOS-ROM", "4023H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4023H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4025h", "", "BIOS-ROM", "4025H ? Format routine (with questions)", "IDE+SCSI: this function may not be supported by all ROMs",
        R"MAN(AREA: ROM de disco (Disk-ROM)

4025H ? Format routine (with questions)

IDE+SCSI: this function may not be supported by all ROMs
(e.g. NOVAXIS simply returns from this call where HSH-SCSI
supports it).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4026h", "", "BIOS-ROM", "4026H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4026H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4029h", "", "BIOS-ROM", "4029H ? Stop drive(s)", "Stop all drives served by all disk roms.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

4029H ? Stop drive(s)

Stop all drives served by all disk roms.

You should never use this function, for the following two reasons:

1) It can stop hard drives of people that have a disk rom that can turn of
the hard drive.

2) The DOS2 ROM of the MSX turbo R contains a small bug in the call to the
CPU switching routine, which can cause the MSX turbo R to crash.

SCSI: this function may not be supported by all ROMs
(e.g. NOVAXIS simply returns from this call where HSH-SCSI
supports it).

AREA: Hangul-ROM

4029H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("402ch", "", "BIOS-ROM", "402CH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

402CH ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("402dh", "", "BIOS-ROM", "402DH ? Where Am I (calculates slot number of DOS2)", "Entry: | none",
        R"MAN(AREA: ROM de disco (Disk-ROM)

402DH ? Where Am I (calculates slot number of DOS2)

Entry: | none
Exit: | A = slot number
Modifies: | ?

SCSI specific

The following functions are at least valid on NOVAXIS and HSH SCSI controllers:

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("402fh", "", "BIOS-ROM", "402FH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

402FH ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4032h", "", "BIOS-ROM", "4032H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4032H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4035h", "", "BIOS-ROM", "4035H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4035H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4038h", "", "BIOS-ROM", "4038H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4038H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("403bh", "", "BIOS-ROM", "403BH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

403BH ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("403eh", "", "BIOS-ROM", "403EH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

403EH ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4041h", "", "BIOS-ROM", "4041H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4041H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4044h", "", "BIOS-ROM", "4044H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4044H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4047h", "", "BIOS-ROM", "4047H-404CH ? ?", "Unused? Reserved? Version information?",
        R"MAN(AREA: Hangul-ROM

4047H-404CH ? ?

Unused? Reserved? Version information?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("404dh", "", "BIOS-ROM", "404DH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

404DH ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4050h", "", "BIOS-ROM", "4050H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4050H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4053h", "", "BIOS-ROM", "4053H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4053H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4056h", "", "BIOS-ROM", "4056H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4056H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4059h", "", "BIOS-ROM", "4059H ? ?", "Prints the character in register A. Valid in SCREEN 9 only?",
        R"MAN(AREA: Hangul-ROM

4059H ? ?

Prints the character in register A. Valid in SCREEN 9 only?

This routine is also called from the HCHPU (FDA4H) hook, if SCREEN 9 is active.

The used character-set is located in a separate ROM. The SlotID of this ROM is
defined in FD0DH. The start address of the character-set is stored in the WORD
at 401AH+401BH of this ROM.

Entry: | A=Character to output
Exit: | ?
Modifies: | ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("405ch", "", "BIOS-ROM", "405CH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

405CH ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("405fh", "", "BIOS-ROM", "405FH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

405FH ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4062h", "", "BIOS-ROM", "4062H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4062H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4065h", "", "BIOS-ROM", "4065H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4065H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4068h", "", "BIOS-ROM", "4068H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4068H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("406bh", "", "BIOS-ROM", "406BH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

406BH ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("406eh", "", "BIOS-ROM", "406EH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

406EH ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4071h", "", "BIOS-ROM", "4071H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4071H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4074h", "", "BIOS-ROM", "4074H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4074H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("4077h", "", "BIOS-ROM", "4077H ? ?", "?",
        R"MAN(AREA: Hangul-ROM

4077H ? ?

?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("407ah", "", "BIOS-ROM", "407AH ? ?", "?",
        R"MAN(AREA: Hangul-ROM

407AH ? ?

?

<!--

4020: jp     $407d
4023: jp     $4080
4026: jp     $4083
4029: jp     $4086
402C: jp     $4089
402F: jp     $408c
4032: jp     $408f
4035: jp     $40bc
4038: jp     $40dc
403B: jp     $6516
403E: jp     $6547
4041: jp     $4ec7
4044: jp     $7c4d
404D: jp     $41a2
4050: jp     $4092
4053: jp     $4095
4056: jp     $6371
4059: jp     $4098
405C: jp     $409b
405F: jp     $409e
4062: jp     $40a1
4065: jp     $40a4
4068: jp     $40a7
406B: jp     $40aa
406E: jp     $40ad
4071: jp     $40b0
4074: jp     $40b3
4077: jp     $40b6
407A: jp     $40b9

-->

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("6637h", "", "BIOS-ROM", "6637H ? Request memory for a return-string", "This routine checks if there is enough memory for a string, which has to be",
        R"MAN(AREA: BIOS principal (MainROM)

6637H ? Request memory for a return-string

This routine checks if there is enough memory for a string, which has to be
returned from the calling function. The string-length (register A) is checked
(at 668EH) and probably terminated with an "Out of string space" error.
Length and memory-address for the string are putted into DSCTMP (F698H).
String data can be written to that adress. The result-string can be returned
by calling 6654H.
Entry: | A = string length
Exit: | ?
Modifies: | ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("6654h", "", "BIOS-ROM", "6654H ? Return a result-string", "This routine is used by string-functions to create a result-descriptor.",
        R"MAN(AREA: BIOS principal (MainROM)

6654H ? Return a result-string

This routine is used by string-functions to create a result-descriptor.
The descriptor is copied from DSCTMP (F698H) to the next available location
in TEMPST (F67AH) and it's address is stored in DAC (F7F6H). If TEMPST
is full a "String formula too complex" error is generated, otherwise TEMPPT
(F678H) is increased by 3 and the routine ends.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("668eh", "", "BIOS-ROM", "668EH ? Subroutine: check if enough string-memory is available", "This subroutine checks if there is enough memory for a string which length is specified",
        R"MAN(AREA: BIOS principal (MainROM)

668EH ? Subroutine: check if enough string-memory is available

This subroutine checks if there is enough memory for a string which length is specified
in register A. After termination of this routine the DE-register points to the memory
adress, available for the string.

The string-length is first substracted from the actually available memory FRETOP (F69BH).
The result is compared with STKTOP (F674H). STKTOP contains the lowest available address
for string-memory. After updating FRETOP and if enough memory is available the routine
ends here. If there is not enough memory available a garbage collection (66B6H) is performed
and availabe memory is checked again. If there is still not enough memory available
a "Out of string space" error is generated.
Entry: | A = string length
Exit: | DE = available memory-adress
Modifies: | ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("66b6h", "", "BIOS-ROM", "66B6H ? Garbage collection", "This routine performs the garbage collection, which removes unused",
        R"MAN(AREA: BIOS principal (MainROM)

66B6H ? Garbage collection

This routine performs the garbage collection, which removes unused
strings from memory. Valid strings are moved to remove "holes" from
the memory. This generates additional memory for new strings.
This routine is automatically invoked if there is no more string-memory
available.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("absolute disk read", "", "BDOS", "2F Absolute Disk Read", "2F Absolute Disk Read",
        R"MAN(AREA: BDOS (MSX-DOS)

2F Absolute Disk Read

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("absolute disk write", "", "BDOS", "30 Absolute Disk Write", "MSX-DOS 2.x",
        R"MAN(AREA: BDOS (MSX-DOS)

30 Absolute Disk Write

MSX-DOS 2.x

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("allocate sector buffers", "", "BDOS", "69 Allocate sector buffers", "69 Allocate sector buffers",
        R"MAN(AREA: BDOS (MSX-DOS)

69 Allocate sector buffers

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("aspct1", "", "Variaveis RAM", "F40B ASPCT1  256/aspect ratio", "This variable contains the reciprocal of the default \"CIRCLE\"",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F40B ASPCT1  256/aspect ratio

This variable contains the reciprocal of the default "CIRCLE"
aspect ratio multiplied by 256. Its value is set at power-up
and thereafter unaltered.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("aspct2", "", "Variaveis RAM", "F40D ASPCT2  256*aspect ratio", "This variable contains the default \"CIRCLE\"",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F40D ASPCT2  256*aspect ratio

This variable contains the default "CIRCLE"

aspect ratio
multiplied by 256. Its value is set at power-up and thereafter
unaltered. The aspect ratio is present in two forms so that the
"CIRCLE" statement handler can select the appropiate one
immediately rather than needing to examine and possibly
reciprocate it as is the case with an operand in the program
text.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("atrbyt", "", "Variaveis RAM", "F3F2 ATRBYT  Color code in csing graphic", "This variable contains the graphics ink color used by the SETC and NSETCX",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3F2 ATRBYT  Color code in csing graphic

This variable contains the graphics ink color used by the SETC and NSETCX
standard routines.

        .WORK AREA FOR PLAY STATEMENT

F3F3 QUEUES: DEFW F959H  Points to queue table at the execution of PLAY st.

This variable contains the address of the control blocks for
the three music queues. Its value is set at power-up and
thereafter unaltered.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("atrscn", "", "BIOS-ROM", "0071H ATRSCN", "0071H ATRSCN",
        R"MAN(AREA: Sub-ROM do MSX2

0071H ATRSCN

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("autlin", "", "Variaveis RAM", "F6AB AUTLIN  Current line number for auto", "Current line number of the AUTO Basic-Statement.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F6AB AUTLIN  Current line number for auto

Current line number of the AUTO Basic-Statement.

Disk-ROM: largest used sector size (128, 256, 512 or 1024 bytes)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("aux input", "", "BDOS", "03 Aux Input", "03 Aux Input",
        R"MAN(AREA: BDOS (MSX-DOS)

03 Aux Input

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("aux output", "", "BDOS", "04 Aux Output", "04 Aux Output",
        R"MAN(AREA: BDOS (MSX-DOS)

04 Aux Output

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("auxinp", "", "Variaveis RAM", "F371 AUXINP  Aux device input.", "Aux device input.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F371 AUXINP  Aux device input.

Aux device input.
By default this routine is returns ^Z (&H1A) in [AF].

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("auxout", "", "Variaveis RAM", "F374 AUXOUT  Aux device output.", "Aux device output.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F374 AUXOUT  Aux device output.

Aux device output.
By default this routine simply returns with RET.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bakclr", "", "Variaveis RAM", "F3EA BAKCLR  Background colour", "This variable contains the current background color. Its value is set at power-up",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3EA BAKCLR  Background colour

This variable contains the current background color. Its value is set at power-up
and thereafter only alterd by the "COLOR" statement. The background color is used
by the CLS standard routine to clear the screen in the graphics modes and by the
CHGCLR standard routine to set the 0 pixel color in the text modes.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("base", "", "BIOS-ROM", "0169H BASE (Basic!)", "0169H BASE (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

0169H BASE (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("basef", "", "BIOS-ROM", "016DH BASEF (Basic!)", "016DH BASEF (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

016DH BASEF (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("basrom", "", "Variaveis RAM", "FBB1 BASROM   <>0 if basic is in rom", "This variable determines whether the ISCNTC and INLIN standard",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBB1 BASROM   <>0 if basic is in rom

This variable determines whether the ISCNTC and INLIN standard
routines will respond to the CTRL-STOP key: 00H=enable,
NZ=disable. It is used to prevent termination of a BASIC ROM
located duringthe power-up ROM search.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bdrclr", "", "Variaveis RAM", "F3EB BDRCLR  Border colour", "This variable contains the current border color. Its value is set at power-up and",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3EB BDRCLR  Border colour

This variable contains the current border color. Its value is set at power-up and
thereafter only altered by the CHGCLR standard routine in 32x24 Text Mode, Graphics
Mode and Multicolor Mode to set the border color.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("beep", "", "BIOS-ROM", "00C0H BEEP   Go beep", "00C0H BEEP Go beep",
        R"MAN(AREA: BIOS principal (MainROM)

00C0H BEEP   Go beep

AREA: Sub-ROM do MSX2

017DH BEEP

Generates a beep
Entry: | none
Exit: | none
Modifies: | All ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bigfil", "", "BIOS-ROM", "016BH BIGFIL", "Same function as FILVRM (total VRAM can be reached).",
        R"MAN(AREA: BIOS principal (MainROM)

016BH BIGFIL

Same function as FILVRM (total VRAM can be reached).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bltdm", "", "BIOS-ROM", "01A9H BLTDM", "01A9H BLTDM",
        R"MAN(AREA: Sub-ROM do MSX2

01A9H BLTDM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bltdv", "", "BIOS-ROM", "01A1H BLTDV", "01A1H BLTDV",
        R"MAN(AREA: Sub-ROM do MSX2

01A1H BLTDV

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bltmd", "", "BIOS-ROM", "01A5H BLTMD", "01A5H BLTMD",
        R"MAN(AREA: Sub-ROM do MSX2

01A5H BLTMD

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bltmv", "", "BIOS-ROM", "0199H BLTMV", "0199H BLTMV",
        R"MAN(AREA: Sub-ROM do MSX2

0199H BLTMV

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bltvd", "", "BIOS-ROM", "019DH BLTVD", "019DH BLTVD",
        R"MAN(AREA: Sub-ROM do MSX2

019DH BLTVD

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bltvm", "", "BIOS-ROM", "0195H BLTVM", "0195H BLTVM",
        R"MAN(AREA: Sub-ROM do MSX2

0195H BLTVM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bltvv", "", "BIOS-ROM", "0191H BLTVV", "0191H BLTVV",
        R"MAN(AREA: Sub-ROM do MSX2

0191H BLTVV

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bottom", "", "Variaveis RAM", "FC48 BOTTOM", "This variable contains the address of the lowest RAM location",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC48 BOTTOM

This variable contains the address of the lowest RAM location
used by the Interpreter. Its value is set at power-up and
thereafter unaltered.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("boxlin", "", "BIOS-ROM", "0081H BOXLIN", "0081H BOXLIN",
        R"MAN(AREA: Sub-ROM do MSX2

0081H BOXLIN

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("brdatr", "", "Variaveis RAM", "FCB2 BRDATR  border color for paint", "FCB2 BRDATR border color for paint",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCB2 BRDATR  border color for paint

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("breakx", "", "BIOS-ROM", "00B7H BREAKX Check CTRL-STOP key directly", "00B7H BREAKX Check CTRL-STOP key directly",
        R"MAN(AREA: BIOS principal (MainROM)

00B7H BREAKX Check CTRL-STOP key directly

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bufend", "", "Variaveis RAM", "FC18 BUFEND            ;end of key buffer", "FC18 LINWRK: DEFS 40",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC18 BUFEND            ;end of key buffer
FC18 LINWRK: DEFS 40

This buffer is used by the BIOS to hold a complete line of
screen characters.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("buffered input", "", "BDOS", "0A Buffered Input", "0A Buffered Input",
        R"MAN(AREA: BDOS (MSX-DOS)

0A Buffered Input

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("c3", "", "Variaveis RAM", "FDA4    C3 00 D0        JP      D000", "Also let the code at D000 be as shown below :",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FDA4    C3 00 D0        JP      D000

Also let the code at D000 be as shown below :

        POP     HL      GET RID OF THE FIRST ITEM ON THE
                        STACK(i.e.RETURN TO HOOK ADDRESS)
        POP     AF      RESTORE THE REGISTERS
        POP     BC
        POP     DE
        POP     HL
        RET             EXIT BIOS CALL CHPUT.

Now the item on the top of the stack is 3 more than the address from which FDA4 was called,i.e.the return address to CHPUT.Therefore,if this is removed from the stack,this will restore the stack to the state it was in immediately before calling FDA4.Also,if the next four items are popped as shown above,the stack will be as it was on entering CHPUT,so the RET instruction will exit the subroutine CHPUT.Consequently the character which was to be printed will not be!

This example demonstrates how the operating system can be intercepted.The five bytes,starting at address FDA4,are know as a hook.Similar "hook" exist at many other points in the operating system and BASIC interpreter,and a list of them,in alphabetical order,is contained in the following pages.

-----------------------------------------------------------------
+       No RET on hook
+/c     No RET on hook if network is present
+/sc    Hook doesn't change for sc 8
-----------------------------------------------------------------
+/c     FD9A+   HKEYI   0C4A  Interrupt handler

This hook is invoked on every(!) interrupt, regardless which device
has triggered it. The hook-routine is responsible to determine
which event (scanline-interrupt, ...) has occured.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("calatr", "", "BIOS-ROM", "0087H CALATR Calculate address of sprite attributes", "0087H CALATR Calculate address of sprite attributes",
        R"MAN(AREA: BIOS principal (MainROM)

0087H CALATR Calculate address of sprite attributes

AREA: Sub-ROM do MSX2

00FDH CALATR

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("calbas", "", "BIOS-ROM", "0159H CALBAS Call to BASIC from any slot", "Standard routine to call a routine in the BASIC Interpreter",
        R"MAN(AREA: BIOS principal (MainROM)

0159H CALBAS Call to BASIC from any slot

Standard routine to call a routine in the BASIC Interpreter
from any slot. Usually this will be from machine code programs
running in an extension ROM in page 1 (4000H-7FFFH).
Entry: | IX = address
Exit: | none
Modifies: | AF', BC', DE', HL', IY, DI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("callf", "", "BIOS-ROM", "0030H CALLF  Call routine in any slot", "Standard routine to call an address in any slot. The Slot ID and address are supplied as inline...",
        R"MAN(AREA: BIOS principal (MainROM)

0030H CALLF  Call routine in any slot

Standard routine to call an address in any slot. The Slot ID and address are supplied as inline parameters rather than its registers to fit inside a hook, for example:

    RST    30H
    DEFB    Slot ID
    DEFW    Address
    RET

The Slot ID is first collected and placed into the high byte of register pair IY. The address is then placed in register pair IX and control drops into the CALSLT standard routine.

Entry: | none
Exit: | none
Modifies: | AF', BC', DE', HL', IX, IY, DI

AREA: Sub-ROM do MSX2

0030H CALLF (RST 30H)

This routine is equal to the MainROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("calpat", "", "BIOS-ROM", "0084H CALPAT Calculate address of sprite pattern", "0084H CALPAT Calculate address of sprite pattern",
        R"MAN(AREA: BIOS principal (MainROM)

0084H CALPAT Calculate address of sprite pattern

AREA: Sub-ROM do MSX2

00F9H CALPAT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("calslt", "", "BIOS-ROM", "001CH CALSLT Call routine in any slot", "Standard routine to call an address in any slot. Ist operation is fundamentally the same as...",
        R"MAN(AREA: BIOS principal (MainROM)

001CH CALSLT Call routine in any slot

Standard routine to call an address in any slot. Ist operation is fundamentally the same as that of the RDSLT standard routine except that the workspace area routine CLPRIM is used rather than RDPRIM. Note that CALBAS and CALLF are just specialised entry points to this standard routine which offer a reduction in the amount of code required.

Entry: | IY (HighByte) =Slot ID
IX=Address
Exit: | none
Modifies: | AF', BC', DE', HL', DI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("capst", "", "Variaveis RAM", "FCAB CAPST   capital status", "This variable is used by the interrupt handler to hold the",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCAB CAPST   capital status

This variable is used by the interrupt handler to hold the
current caps lock status: 00H=Off, NZ=On.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("casprv", "", "Variaveis RAM", "FCB1 CASPRV", "This variable is used to hold any character returned to an I/O",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCB1 CASPRV

This variable is used to hold any character returned to an I/O
buffer by the cassette putback function.

MSX Turbo-R: this register mirrors the content of the I/O port &HA7.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("cgpnt", "", "Variaveis RAM", "F91F CGPNT   Address to store character font in ROM", "F91F: DB ??H Character set SlotID",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F91F CGPNT   Address to store character font in ROM

F91F: DB ??H   Character set SlotID

F920: DW ????H Character set address

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("cgtabl", "", "BIOS-ROM", "0004H CGTABL", "2 Bytes, adress of ROM character set",
        R"MAN(AREA: BIOS principal (MainROM)

0004H CGTABL

2 Bytes, adress of ROM character set

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("change current directory", "", "BDOS", "5A Change current directory", "5A Change current directory",
        R"MAN(AREA: BDOS (MSX-DOS)

5A Change current directory

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("check character", "", "BDOS", "5D Check character", "5D Check character",
        R"MAN(AREA: BDOS (MSX-DOS)

5D Check character

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chgcap", "", "BIOS-ROM", "0132H CHGCAP Change Caps Lock LED", "0132H CHGCAP Change Caps Lock LED",
        R"MAN(AREA: BIOS principal (MainROM)

0132H CHGCAP Change Caps Lock LED

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chgclr", "", "BIOS-ROM", "0062H CHGCLR Change VDP colors", "Standard routine to set the VDP colors.",
        R"MAN(AREA: BIOS principal (MainROM)

0062H CHGCLR Change VDP colors

Standard routine to set the VDP colors.
Entry: | foreground color in FORCLR (F3E9)
background color in BAKCLR (F3EA)
borderground color in BDRCLR (F3EB)
Exit: | none
Modifies: | AF, BC, HL, EI

AREA: Sub-ROM do MSX2

0111H CHGCLR

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chgcpu", "", "BIOS-ROM", "0180H CHGCPU Change CPU", "Changes the CPU mode.",
        R"MAN(AREA: BIOS principal (MainROM)

0180H CHGCPU Change CPU

Changes the CPU mode.
Entry: | A register
       b7 b6 b5 b4 b3 b2 b1 b0
        L  0  0  0  0  0  M  M
        |                 |  |
        --->LED            ------> CPU MODE
-mode
----------------------------------------------------
  mode     meaning
----------------------------------------------------
  00       Z80 mode
  01       R800 ROM MODE
  10       R800 DRAM
  11       system exclusive
----------------------------------------------------
-LED
----------------------------------------------------
 LED       meaning
----------------------------------------------------
  0        LED will not change
  1        LED will change
----------------------------------------------------
Exit: | none
Modifies: | none

Explanation: Depending on the value of the A register the CPU mode will change. If bit 7 is set to 1, the LED that displays the status of the CPU will change along with it. If bit 7 is set to 0, it will not change.

Besides the R register none of the registers will have been changed.

Note: The change of the CPU will only change settings in the S1990 system controller. Therefore, after you change to the Z80 or the R800 ROM MODE and if you write into the memory segments that are normally used in the R800 DRAM MODE for the system software, and after that you try to change the CPU MODE to R800 DRAM MODE the computer will crash.

The transfer of the systemsoftware will only be done during the systemboot.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chget", "", "BIOS-ROM", "009FH CHGET  Get character from keyboard buffer (wait)", "009FH CHGET Get character from keyboard buffer (wait)",
        R"MAN(AREA: BIOS principal (MainROM)

009FH CHGET  Get character from keyboard buffer (wait)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chgmod", "", "BIOS-ROM", "005FH CHGMOD Change VDP mode", "Standard routine to set a new screen mode.",
        R"MAN(AREA: BIOS principal (MainROM)

005FH CHGMOD Change VDP mode

Standard routine to set a new screen mode.
Entry: | A = requested screen-mode
Exit: | none
Modifies: | AF, BC, DE, HL, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chgsnd", "", "BIOS-ROM", "0135H CHGSND Change Key Click sound output", "0135H CHGSND Change Key Click sound output",
        R"MAN(AREA: BIOS principal (MainROM)

0135H CHGSND Change Key Click sound output

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chknew", "", "BIOS-ROM", "0165H CHKNEW", "Tests screen mode",
        R"MAN(AREA: BIOS principal (MainROM)

0165H CHKNEW

Tests screen mode

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chkram", "", "BIOS-ROM", "0000H CHKRAM Power-up, check RAM", "Standard routine to perform memory initialisation at power-up. It non-destructively tests for...",
        R"MAN(AREA: BIOS principal (MainROM)

0000H CHKRAM Power-up, check RAM

Standard routine to perform memory initialisation at power-up. It non-destructively tests for RAM in pages 2 and 3 in all sixteen possible slots then sets the primary and secondary slot registers to switch in the largest area found. The entire workspare area (F380H to FF9CH) is zeroed and EXPTBL and SLTTBL filled in to map any extension interfaces in existence. Interrupt mode 1 i set and control transfers to the ramainder of the power-up initialisation routine (7C76H).

Entry: | none
Exit: | none
Modifies: | AF, BC, DE, HL, SP

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chkslz", "", "BIOS-ROM", "0162H CHKSLZ", "Search slots for SUB-ROM",
        R"MAN(AREA: BIOS principal (MainROM)

0162H CHKSLZ

Search slots for SUB-ROM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("choice", "", "BIOS-ROM", "4019H CHOICE ?", "Entry: | none",
        R"MAN(AREA: ROM de disco (Disk-ROM)

4019H CHOICE ?

Entry: | none
Exit: | HL = Address of zero terminated character string with the text
      with choices for a DSKFMT.
      If there is no choice (only 1 format supported) return [HL]=0
IDE: this function may not be supported by all ROMs.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chput", "", "BIOS-ROM", "00A2H CHPUT  Screen character output", "00A2H CHPUT Screen character output",
        R"MAN(AREA: BIOS principal (MainROM)

00A2H CHPUT  Screen character output

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chrgtr", "", "BIOS-ROM", "0010H CHRGTR Get next BASIC program character", "Standard routine to fetch the next character from the program text. Register pair HL is...",
        R"MAN(AREA: BIOS principal (MainROM)

0010H CHRGTR Get next BASIC program character

Standard routine to fetch the next character from the program text. Register pair HL is incremented and the character placed in register A. If it's a space, TAB code (09H) or LF code (0AH) is skipped over. If it's a statement separator (3AH) or end of line character (00H) the routine terminates with flag Z, NC. If it is a digit from "0" to "9" the routine terminates with flag NZ,C. If it is any other character apart from the nueric prefix tokens the routine terminates with flag NZ, NC. If the character is one of the numeric prefix tokens then it is placed in CONSAV and the operand copied to CONLO. The code is placed in CONTYP and the address of the trailing program character in CONTXT.

Entry: | HL=Points to current program character
Exit: | A=Next program character
Modifies: | AF, HL

AREA: Sub-ROM do MSX2

0010H CHRGTR (RST 10H)

This routine is equal to the MainROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("chsns", "", "BIOS-ROM", "009CH CHSNS  Sense keyboard buffer for character", "009CH CHSNS Sense keyboard buffer for character",
        R"MAN(AREA: BIOS principal (MainROM)

009CH CHSNS  Sense keyboard buffer for character

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ckcntc", "", "BIOS-ROM", "00BDH CKCNTC Check CTRL-STOP key", "00BDH CKCNTC Check CTRL-STOP key",
        R"MAN(AREA: BIOS principal (MainROM)

00BDH CKCNTC Check CTRL-STOP key

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("clear", "", "Variaveis RAM", "F34B CLEAR", "CLEAR border MSXDOS. This address always indicates under MSXDOS to the beginning of MSXDOS.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F34B CLEAR

CLEAR border MSXDOS. This address always indicates under MSXDOS to the beginning of MSXDOS.
COMMAND.COM become as from this address - charged.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("clikfl", "", "Variaveis RAM", "FBD9 CLIKFL", "This variable is used internally by the interrupt handler to",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBD9 CLIKFL

This variable is used internally by the interrupt handler to
prevent spurious key clicks when returning multiple characters
from a single key depression such as a function key.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("cliksw", "", "Variaveis RAM", "F3DB CLIKSW  Key click switch", "This variable controls the interrupt handler key click: 00H=Off, NZ=On.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3DB CLIKSW  Key click switch

This variable controls the interrupt handler key click: 00H=Off, NZ=On.
Its value is set at power-up and thereafter only altered by the "SCREEN" statement.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("clmlst", "", "Variaveis RAM", "F3B2 CLMLST  Horizontal location in the case that items", "are divided by commas in PRINT statement",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3B2 CLMLST  Horizontal location in the case that items
             are divided by commas in PRINT statement

This variable contains the minimum number of columns that must still be available on a line
for a data item to be "PRINT"ed, if less space is available a CR,LF is issued first.
Its value is set at power-up and thereafter only altered by the "WIDTH" and "SCREEN" statements.

        .WORK FOR INITIALIZATION

-SCREEN 0
F3B3 TXTNAM  Pattern name tabte
F3B5 TXTCOL  Color table
F3B7 TXTCGP  Pattern ganarator table
F3B9 TXTATR  Unused
F3BB TXTPAT  Unused

These five variables contain the 40x24 Text Mode VDP base addresses. Their values are set at
power-up and thereafter only altered by the "BASE" statement.

-SCREEN 1
F3BD T32NAM  Pattern name tabte
F3BF T32COL  Color table
F3C1 T32CGP  Pattern ganarator table
F3C3 T32ATR  Sprite attribute table
F3C5 T32PAT  Sprite generator table

These five variables contain the 32x24 Text Mode VDP base addresses. Their values are set at
power-up and thereafter only altered by the "BASE" statement.

-SCREEN 2
F3C7 GRPNAM  Pattern name tabte
F3C9 GRPCOL  Color table
F3CB GRPCGP  Pattern ganarator table
F3CD GRPATR  Sprite attribute table
F3CF GRPPAT  Sprite generator table

These five variables contain the Graphics Mode VDP base addresses. Their values are set at
power-up and thereafter only altered by the "BASE" statement.

-SCREEN 3
F3D1 MLTNAM  Pattern name tabte
F3D3 MLTCOL  Color table
F3D5 MLTCGP  Pattern ganarator table
F3D7 MLTATR  Sprite attribute table
F3D9 MLTPAT  Sprite generator table

These five variables contain the Multicolour Mode VDP base addresses. Their values are set at
power-up and thereafter only altered by the "BASE" statement.

;

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("close file", "", "BDOS", "10 Close File", "10 Close File",
        R"MAN(AREA: BDOS (MSX-DOS)

10 Close File

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("close file handle", "", "BDOS", "45 Close file handle", "45 Close file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

45 Close file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("clprim", "", "Variaveis RAM", "F38C CLPRIM  Call basic slot", "This routine is used by the CALSLT standard routine to switch Primary Slots and call an address.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F38C CLPRIM  Call basic slot

This routine is used by the CALSLT standard routine to switch Primary Slots and call an address.
The new Primary Slot Register setting is supplied in register A, the old setting on the Z80
stack and the address to call in register pair IX.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("clrendln", "", "BIOS-ROM", "7FAAH ClrEndLn", "Clear To End Of Line (print escape sequence).",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7FAAH ClrEndLn

Clear To End Of Line (print escape sequence).
Entry: | ?
Exit: | ?
Modifies: | ?
Note: | none

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("clrspr", "", "BIOS-ROM", "0069H CLRSPR Clear all sprites", "0069H CLRSPR Clear all sprites",
        R"MAN(AREA: BIOS principal (MainROM)

0069H CLRSPR Clear all sprites

AREA: Sub-ROM do MSX2

00F5H CLRSPR

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("cls", "", "BIOS-ROM", "00C3H CLS    Clear screen", "00C3H CLS Clear screen",
        R"MAN(AREA: BIOS principal (MainROM)

00C3H CLS    Clear screen

AREA: Sub-ROM do MSX2

0115H CLS

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("cmask", "", "Variaveis RAM", "F92C CMASK", "Cursor mask.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F92C CMASK

Cursor mask.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("cnsdfg", "", "Variaveis RAM", "F3DE CNSDFG  Function key display switch", "This variable contains the current state of the function key display: 00H=Off, NZ=On.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3DE CNSDFG  Function key display switch

This variable contains the current state of the function key display: 00H=Off, NZ=On.

        .AREA TO SAVE VDP REGISTERS

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("cnvchr", "", "BIOS-ROM", "00ABH CNVCHR Convert character with graphic header", "00ABH CNVCHR Convert character with graphic header",
        R"MAN(AREA: BIOS principal (MainROM)

00ABH CNVCHR Convert character with graphic header

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("color", "", "BIOS-ROM", "0155H COLOR (Basic!)", "0155H COLOR (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

0155H COLOR (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("console input", "", "BDOS", "01 Console Input", "01 Console Input",
        R"MAN(AREA: BDOS (MSX-DOS)

01 Console Input

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("console output", "", "BDOS", "02 Console Output", "02 Console Output",
        R"MAN(AREA: BDOS (MSX-DOS)

02 Console Output

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("console status", "", "BDOS", "0B Console Status", "0B Console Status",
        R"MAN(AREA: BDOS (MSX-DOS)

0B Console Status

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("create file", "", "BDOS", "16 Create File", "16 Create File",
        R"MAN(AREA: BDOS (MSX-DOS)

16 Create File

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("create file handle", "", "BDOS", "44 Create file handle", "44 Create file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

44 Create file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("create or destroy ram disk", "", "BDOS", "68 Create or destroy RAM disk", "68 Create or destroy RAM disk",
        R"MAN(AREA: BDOS (MSX-DOS)

68 Create or destroy RAM disk

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("crtcnt", "", "Variaveis RAM", "F3B1 CRTCNT  Number of lines of current screen", "This variable contains the number of rows on the screen. Its value is set at power-up and",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3B1 CRTCNT  Number of lines of current screen

This variable contains the number of rows on the screen. Its value is set at power-up and
thereafter unaltered.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("cs2400", "", "Variaveis RAM", "F401 CS2400", "DB LOW02,HIGH02,LOW12,HIGH22,HEDLEN*2/256 ; FOR 2400 BAUD",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F401 CS2400
     DB LOW02,HIGH02,LOW12,HIGH22,HEDLEN*2/256 ; FOR 2400 BAUD

These five variables contain the 2400 baud cassette parameters.
Their values are set at power-up and thereafter unaltered.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("csrsw", "", "Variaveis RAM", "FCA9 CSRSW   cursor display switch", "FCA9 CSRSW cursor display switch",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCA9 CSRSW   cursor display switch

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("csrx", "", "Variaveis RAM", "F3DD CSRX    X-coordinate of cursor", "This variable contains the column coordinate (from 1 to LINLEN) of the text mode cursor.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3DD CSRX    X-coordinate of cursor

This variable contains the column coordinate (from 1 to LINLEN) of the text mode cursor.
Note that the BIOS cursor coordinates for the home position are 1,1 whatever the screen width.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("csry", "", "Variaveis RAM", "F3DC CSRY    Y-coordinate of cursor", "This variable contains the row coordinate (from 1 to CTRCNT) of the text mode cursor.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3DC CSRY    Y-coordinate of cursor

This variable contains the row coordinate (from 1 to CTRCNT) of the text mode cursor.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("cstyle", "", "Variaveis RAM", "FCAA CSTYLE  cursor style", "FCAA CSTYLE cursor style",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCAA CSTYLE  cursor style

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ctrlcad", "", "Variaveis RAM", "F323 CTRLCAD", "Ctrl+C handler: this routine is called if ^C is pressed.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F323 CTRLCAD

Ctrl+C handler: this routine is called if ^C is pressed.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("curlin", "", "Variaveis RAM", "F41C CURLIN  Current execution line number of BASIC", "This variable contains the current interpreter line number. A",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F41C CURLIN  Current execution line number of BASIC

This variable contains the current interpreter line number. A
value of FFFFH denotes direct mode.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("data area used by rs-232c or disk drive", "", "Variaveis RAM", "FAFE XSAVE   [10000000 XXXXXXXX]", "FB00 YSAVE [*0000000 YYYYYYYY] *-light pen interrupt request",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FAFE XSAVE   [10000000 XXXXXXXX]
FB00 YSAVE   [*0000000 YYYYYYYY] *-light pen interrupt request
FB02 LOGOPR  Logical operation code
                Data area used by RS-232C or disk drive
FB03 RSTMP
FB03 TOCNT
FB04 RSFCB
FB06 RSIQLN
FB07 MEXBIH  Hook
FB0C OLDSTT  Hook
FB12 OLDINT  Hook

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("dcompr", "", "BIOS-ROM", "0020H DCOMPR Compare register pairs HL and DE", "Standard routine used by the BASIC Interpreter to check the relative values of register pairs...",
        R"MAN(AREA: BIOS principal (MainROM)

0020H DCOMPR Compare register pairs HL and DE

Standard routine used by the BASIC Interpreter to check the relative values of register pairs HL and DE.

Entry: | HL
DE
Exit: | Flag NC, if HL>DE
Flag Z, if DL=DE
Flag C, if HL<DE
Modifies: | AF

AREA: Sub-ROM do MSX2

0020H DCOMPR (RST 20H)

This routine is equal to the MainROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("define abort exit routine", "", "BDOS", "63 Define abort exit routine", "63 Define abort exit routine",
        R"MAN(AREA: BDOS (MSX-DOS)

63 Define abort exit routine

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("define disk error handler routine", "", "BDOS", "64 Define disk error handler routine", "64 Define disk error handler routine",
        R"MAN(AREA: BDOS (MSX-DOS)

64 Define disk error handler routine

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("delete file", "", "BDOS", "13 Delete File", "13 Delete File",
        R"MAN(AREA: BDOS (MSX-DOS)

13 Delete File

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("delete file handle", "", "BDOS", "52 Delete file handle", "52 Delete file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

52 Delete file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("delete file or sudirectory", "", "BDOS", "4D Delete file or sudirectory", "4D Delete file or sudirectory",
        R"MAN(AREA: BDOS (MSX-DOS)

4D Delete file or sudirectory

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("delln0", "", "BIOS-ROM", "0121H DELLN0", "0121H DELLN0",
        R"MAN(AREA: Sub-ROM do MSX2

0121H DELLN0

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("device", "", "Variaveis RAM", "FD99 DEVICE  device ID for cartrige 0-3", "This variable is used to pass the device code, from 0 to 3, to an",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FD99 DEVICE  device ID for cartrige 0-3

This variable is used to pass the device code, from 0 to 3, to an
extension ROM.

THE USE OF HOOKS

Hooks provide a way to intercept the BASIC interpreter or operating system at certain points,enabling additional,or alternative,processing to be carried out.To expand on the last sentence it i s easiest to use an to be carried out.To expand on the last sentence it iseasiest to use an example.Consider for example the BIOS call CHPUT(see section 2.3).This routine is used to print text on the screen.Here is a disassembly of the first few instructions of CHPUT :

        PUSH    HL      SAVE ALL THE REGISTERS USED
        PUSH    DE
        PUSH    BC
        PUSH    AF
        CALL    FDA4    CALL THE HOOK HCHPU

First of all registers HL,BC,DE and AF are pushed on to the stack.Then memory address FDA4 is called.Normally,the contents of FDA4 and the following 4 bytes,is a Z80 RET instruction,immediately passing control to the instruction following the CALL.However,FDA4 is in RAM,so its contents can be altered.Let the 3 bytes,starting at FDA4 be changed,so that control is passed to another subroutine at D000,i.e.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("devnum", "", "Variaveis RAM", "FB16       DEVNUM", "FB17-FB19 DATCNT",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FB16       DEVNUM
FB17-FB19  DATCNT
FB1A       ERRORS
FB1B       FLAGS
FB1C       ESTBLS
FB1D       COMMSK
FB1E       1STCOM
FB1F       1STMOD

Inconsistent documentation: MILC describes the above variables,
where the section below is from other sources!

AREA: Variavel da area de trabalho (RAM)

FB17 DEVNUM
FB18 DATCNT  Hook
FB1B ERRORS
FB1C FLAGS
FB1D ESTBLS
FB1E COMMSK
FB1F LSTCOM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("dimflg", "", "Variaveis RAM", "F662 DIMFLG  ARRAY FLAG", "F663 VALTYP Type indicator",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F662 DIMFLG  ARRAY FLAG
F663 VALTYP  Type indicator
F664 OPRTYP  Store operator number in the extended momentarily before
             operator application
F664 DORES   Indicates whether stored word can be crunched
F665 DONUM   flag for crunch
F666 CONTXT  Text address used by CNRGET
F668 CONSAV  Store token of constant after calling CHRGET
F669 CONTYP  Type of stored constant
F66A CONLO   Valu of stored constant
F672 MEMSIZ  HIGHEST LOCATION IN MEMORY USED BY BASIC
F674 STKTOP  TOP LOCATION TO BE USED FOR THE STACK
F676 TXTTAB  Starting address of BASIC text area
F678 TEMPPT  Starting address of unused area of
             temporary descriptor
F67A TEMPST  Temporary descriptors
F698 DSCTMP  String descriptor wich is the result of string fun.
F69B FRETOP  Starting address of unused area of string area
F69D TEMP3   Used for garbage collection or by USR function
F69F TEMP8   Used for garbage collection
F6A1 ENDFOR  Next address of FOR st.
F6A3 DATLIN  Line number of DATA st.read by READ st.
F6A5 SUBFLG  Flag for array for USR fun.
F6A6 USFLG
F6A6 FLGINP  Flag for INPUT or READ
F6A7 TEMP    Location for temporary reservation for st.code
F6A9 PTRFLG  =0 IF NO LINE NUMBER CONVERTED TO POINTERS
F6AA AUTFLG  AUTO MODE FLAG

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("direct console i/o", "", "BDOS", "06 Direct Console I/O", "06 Direct Console I/O",
        R"MAN(AREA: BDOS (MSX-DOS)

06 Direct Console I/O

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("direct input", "", "BDOS", "07 Direct Input", "07 Direct Input",
        R"MAN(AREA: BDOS (MSX-DOS)

07 Direct Input

AREA: BDOS (MSX-DOS)

08 Direct Input

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("disk reset", "", "BDOS", "0D Disk Reset", "0D Disk Reset",
        R"MAN(AREA: BDOS (MSX-DOS)

0D Disk Reset

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("disscr", "", "BIOS-ROM", "0041H DISSCR Disable screen", "Standard routine to disable screen. This simply involves resetting bit 6 of VDP mode register 1.",
        R"MAN(AREA: BIOS principal (MainROM)

0041H DISSCR Disable screen

Standard routine to disable screen. This simply involves resetting bit 6 of VDP mode register 1.

Entry: | none
Exit: | none
Modifies: | AF, BC, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("doboxf", "", "BIOS-ROM", "0079H DOBOXF", "0079H DOBOXF",
        R"MAN(AREA: Sub-ROM do MSX2

0079H DOBOXF

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("dogrph", "", "BIOS-ROM", "0085H DOGRPH", "0085H DOGRPH",
        R"MAN(AREA: Sub-ROM do MSX2

0085H DOGRPH

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("doline", "", "BIOS-ROM", "007EH DOLINE", "007EH DOLINE",
        R"MAN(AREA: Sub-ROM do MSX2

007EH DOLINE

AREA: Sub-ROM do MSX2

007EH DOLINE

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("downc", "", "BIOS-ROM", "0108H DOWNC  Move current pixel physical address down", "0108H DOWNC Move current pixel physical address down",
        R"MAN(AREA: BIOS principal (MainROM)

0108H DOWNC  Move current pixel physical address down

AREA: Sub-ROM do MSX2

00B5H DOWNC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("dpblst", "", "Variaveis RAM", "F355 DPBLST  List of DRIVE PARAMETERS BLOCK", "F355 adress of DPB for drive A:",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F355 DPBLST  List of DRIVE PARAMETERS BLOCK
F355         adress of DPB for drive A:
F357         adress of DPB for drive B:
F359         adress of DPB for drive C:
F35B         adress of DPB for drive D:
F35D         adress of DPB for drive E:
F35F         adress of DPB for drive F:
F361         adress of DPB for drive G:
F363         adress of DPB for drive H:

Adresses of drive parameters block for drive A: to H:.
Every of the drive parameter blocks has the following layout:

             +00    DRIVE    Drive (0=A:, 1=B:, ...)
             +01    ID       Media ID byte (0F8h/0F9h)
             +02/03 SECSIZ   Sector size (200h = 512 byte)
             +04    DIRMSK   (Directory entries in 1 sector)-1 = (SECSIZ/32)-1
             +05    DIRSHFT  Number of 1-bits in DIRMSK
             +06    CLUSMSK  (Number of sectors per cluster)-1
             +07    CLUSSHFT (Number of 1-bits in CLUSMSK)+1
             +08/09 FIRFAT   First sector of the FAT
             +0A    FATCNT   Number of FATs
             +0B    MAXENT   Maximum number of directory entries
             +0C/0D FIRREC   First sector of the data-area
             +0E/0F MAXCLUS  Maximum number of clusters
             +10    FATSIZ   Number of sectors per FAT
             +11    FIRDIR   First sector of the directory
             +12    FATADR   Address of the FAT rise in RAM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("drive", "", "Variaveis RAM", "F2E1 DRIVE   Current drive", "Current drive for absolute disk read/write.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F2E1 DRIVE   Current drive

Current drive for absolute disk read/write.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("drvoff", "", "BIOS-ROM", "401FH DRVOFF Drives-off", "Stop all drives served by this disk rom.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

401FH DRVOFF Drives-off

Stop all drives served by this disk rom.

This function can be used safely. Though, before calling it, you should check
if the disk rom serves a floppy drive or a hard drive. You should not call
this function for hard drives. Some hard drive interfaces actually turn of
the hard drive when this function is called and will never turn the hard
drive on again.

Entry: | none
Exit: | none
Modifies: | ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("drwang", "", "Variaveis RAM", "FCBD DRWANG", "FCBD DRWANG",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCBD DRWANG

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("drwflg", "", "Variaveis RAM", "FCBB DRWFLG", "FCBB DRWFLG",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCBB DRWFLG

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("drwscl", "", "Variaveis RAM", "FCBC DRWSCL", "FCBC DRWSCL",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCBC DRWSCL

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("dskchg", "", "BIOS-ROM", "4013H DSKCHG Check for Disk-Change", "Checks if the disk was changed.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

4013H DSKCHG Check for Disk-Change

Checks if the disk was changed.
If the disk has been changed or may have been changed (unknown)
read the boot sector or the FAT sector for disk media descriptor
and transfer a new DPB as with GETDPB.
Entry: | A = Drive number (0=A:, 1=B:, ...)
B = media-descriptor
C = media-descriptor
HL = Base address of DPB
Exit: | F = CARRY if error
A = errorcode (if error)
B = on success: 1=unchanged, 0=unknown, -1=changed
Modifies: | ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("dskfmt", "", "BIOS-ROM", "401CH DSKFMT Format disk", "It formats the disk in a specified drive.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

401CH DSKFMT Format disk

It formats the disk in a specified drive.
Entry: | A = Specified choice (1-9). See CHOICE
D = Drive number (0=A:, 1=B:, ...)
HL = Begin address of work area
BC = Length of work area
Exit: | F = CARRY if error
A = errorcode (if error)
Modifies: | ?
IDE: this function may not be supported by all ROMs.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("dskfnk", "", "BIOS-ROM", "00CFH DSKFNK Display function keys", "00CFH DSKFNK Display function keys",
        R"MAN(AREA: BIOS principal (MainROM)

00CFH DSKFNK Display function keys

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("dspfnk", "", "BIOS-ROM", "011DH DSPFNK", "011DH DSPFNK",
        R"MAN(AREA: Sub-ROM do MSX2

011DH DSPFNK

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("duplicate file handle", "", "BDOS", "47 Duplicate file handle", "47 Duplicate file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

47 Duplicate file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("enascr", "", "BIOS-ROM", "0044H ENASCR Enable screen", "Standard routine to enable screen. This simply involves setting bit 6 of VDP mode register 1.",
        R"MAN(AREA: BIOS principal (MainROM)

0044H ENASCR Enable screen

Standard routine to enable screen. This simply involves setting bit 6 of VDP mode register 1.

Entry: | none
Exit: | none
Modifies: | AF, BC, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("enaslt", "", "BIOS-ROM", "0024H ENASLT Enable any slot permanently", "Standard routine to switch in a page permanently from any slot. Unlike RDSLT, WRSLT and CALSLT...",
        R"MAN(AREA: BIOS principal (MainROM)

0024H ENASLT Enable any slot permanently

Standard routine to switch in a page permanently from any slot. Unlike RDSLT, WRSLT and CALSLT standard routines the primary slot switching is performed directly and not by a workspace area routine. Consequently addresses in page 0 (0000H to 3FFFH) will cause an immediate system crash.

Entry: | A=Slot ID
HL=Address
Exit: | none
Modifies: | AF, BC, DE, DI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("endbuf", "", "Variaveis RAM", "F660 ENDBUF  Prevent overflow of BUF", "F660 ENDBUF Prevent overflow of BUF",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F660 ENDBUF  Prevent overflow of BUF

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("enstop", "", "Variaveis RAM", "FBB0 ENSTOP   <>0 if warm start enabled", "This variable determines whether the interrupt handler will",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBB0 ENSTOP   <>0 if warm start enabled

This variable determines whether the interrupt handler will
execute a warm start to the Interpreter upon detecting the keys
CODE, GRPH, CTRL and SHIFT depressed together: 00H=disable,
NZ=enable.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ensure file handle", "", "BDOS", "46 Ensure file handle", "46 Ensure file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

46 Ensure file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("eol", "", "BIOS-ROM", "0168H EOL", "Deletes to the end of the line",
        R"MAN(AREA: BIOS principal (MainROM)

0168H EOL

Deletes to the end of the line

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("erafnk", "", "BIOS-ROM", "00CCH ERAFNK Erase function key display", "00CCH ERAFNK Erase function key display",
        R"MAN(AREA: BIOS principal (MainROM)

00CCH ERAFNK Erase function key display

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("erradr", "", "Variaveis RAM", "F323 ERRADR", "Errorhandler: this routine is called if an error occured.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F323 ERRADR

Errorhandler: this routine is called if an error occured.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("esccnt", "", "Variaveis RAM", "FCA7 ESCCNT  escape sequence counter", "FCA7 ESCCNT escape sequence counter",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCA7 ESCCNT  escape sequence counter

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("exbrsa", "", "Variaveis RAM", "FAF8 EXBRSA  SUB-ROM slot address", "Slot-ID of the SubROM.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FAF8 EXBRSA  SUB-ROM slot address

Slot-ID of the SubROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("explain error code", "", "BDOS", "66 Explain error code", "66 Explain error code",
        R"MAN(AREA: BDOS (MSX-DOS)

66 Explain error code

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("exptbl", "", "Variaveis RAM", "FCC1 EXPTBL  flag for expanded slot", "FCC1 DB 00H ; Primary slot 0",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCC1 EXPTBL  flag for expanded slot
FCC1         DB 00H     ; Primary slot 0
FCC2         DB 00H     ; Primary slot 1
FCC3         DB 00H     ; Primary slot 2
FCC4         DB 00H     ; Primary slot 3

Each of these four variables is normally zero but is set to 80H during the power-up
RAM search if the associated Primary Slot is found to be expanded.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("extrom", "", "BIOS-ROM", "015FH EXTROM Call specified address in SUB-ROM", "This is the routine to call BASIC SUB-ROM. The difference",
        R"MAN(AREA: BIOS principal (MainROM)

015FH EXTROM Call specified address in SUB-ROM

This is the routine to call BASIC SUB-ROM. The difference
between this and SUB-ROM above is the point whether the
IX register value is pushed.
Entry: | IX = address to be called
Exit: | depends on the result of the called program
Modifies: | background register and IX, IY registers are reserved

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f24f", "", "Variaveis RAM", "F24F", "Called by the \"Insert disk for drive\" routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F24F

Called by the "Insert disk for drive" routine.
AF has the drive character (41h = "A").

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f252", "", "Variaveis RAM", "F252", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F252

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f255", "", "Variaveis RAM", "F255", "Called by the fix-filename routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F255

Called by the fix-filename routine.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f258", "", "Variaveis RAM", "F258", "Is called by various routines. It is possible to adapt the F2DC, for example under Basic",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F258

Is called by various routines. It is possible to adapt the F2DC, for example under Basic
to see subdirectoies in the FILES-statement. It's also possible to open (always).

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f25b", "", "Variaveis RAM", "F25B", "Called by the DIRENT increase routine. Plane hereafter the new DIRENT is stored (now in [AF]).",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F25B

Called by the DIRENT increase routine. Plane hereafter the new DIRENT is stored (now in [AF]).

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f25e", "", "Variaveis RAM", "F25E", "Called by the routine which will load the following directory-sector.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F25E

Called by the routine which will load the following directory-sector.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f261", "", "Variaveis RAM", "F261", "Called by the fix-filename routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F261

Called by the fix-filename routine.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f264", "", "Variaveis RAM", "F264", "Called by the OPEN routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F264

Called by the OPEN routine.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f267", "", "Variaveis RAM", "F267", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F267

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f26a", "", "Variaveis RAM", "F26A", "Called by the GETDPB routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F26A

Called by the GETDPB routine.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f26d", "", "Variaveis RAM", "F26D", "Called by the CLOSE routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F26D

Called by the CLOSE routine.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f270", "", "Variaveis RAM", "F270", "DiskRead (read sector, BDOS 2Fh): HL=DMA, DE=sector, B=number of sectors.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F270

DiskRead (read sector, BDOS 2Fh): HL=DMA, DE=sector, B=number of sectors.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f273", "", "Variaveis RAM", "F273", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F273

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f276", "", "Variaveis RAM", "F276", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F276

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f279", "", "Variaveis RAM", "F279", "DiskWrite (write sector, BDOS 30h): HL=DMA, DE=sector, B=number of sectors.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F279

DiskWrite (write sector, BDOS 30h): HL=DMA, DE=sector, B=number of sectors.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f27c", "", "Variaveis RAM", "F27C", "Called by the HL=DE*BC routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F27C

Called by the HL=DE*BC routine.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f27f", "", "Variaveis RAM", "F27F", "Called by the BC=BC/DE; HL=rest routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F27F

Called by the BC=BC/DE; HL=rest routine.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f282", "", "Variaveis RAM", "F282", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F282

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f285", "", "Variaveis RAM", "F285", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F285

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f288", "", "Variaveis RAM", "F288", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F288

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f28b", "", "Variaveis RAM", "F28B", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F28B

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f28e", "", "Variaveis RAM", "F28E", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F28E

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f291", "", "Variaveis RAM", "F291", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F291

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f294", "", "Variaveis RAM", "F294", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F294

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f297", "", "Variaveis RAM", "F297", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F297

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f29a", "", "Variaveis RAM", "F29A", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F29A

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f29d", "", "Variaveis RAM", "F29D", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F29D

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f2a0", "", "Variaveis RAM", "F2A0", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F2A0

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f2a3", "", "Variaveis RAM", "F2A3", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F2A3

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f2a6", "", "Variaveis RAM", "F2A6", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F2A6

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f2a9", "", "Variaveis RAM", "F2A9", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F2A9

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f2ac", "", "Variaveis RAM", "F2AC", "Called by the BUFINP (BDOS 10h) routine, plane for the DIRIN2 call (BDOS 02h).",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F2AC

Called by the BUFINP (BDOS 10h) routine, plane for the DIRIN2 call (BDOS 02h).

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f2af", "", "Variaveis RAM", "F2AF", "Called at the begin of the CONOUT routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F2AF

Called at the begin of the CONOUT routine.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f2b2", "", "Variaveis RAM", "F2B2", "Default: C9 C9 C9 (3* ret)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F2B2

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f2b5", "", "Variaveis RAM", "F2B5", "Called by the february (=28/29) day calculation routine.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F2B5

Called by the february (=28/29) day calculation routine.

Default: C9 C9 C9 (3* ret)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f338", "", "Variaveis RAM", "F338", "Clockchip indicator: NZ=clockchip, Z=no clockchip",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F338

Clockchip indicator: NZ=clockchip, Z=no clockchip

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f341", "", "Variaveis RAM", "F341         Slot-Id of RAM in page 0", "F342 Slot-Id of RAM in page 1",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F341         Slot-Id of RAM in page 0
F342         Slot-Id of RAM in page 1
F343         Slot-Id of RAM in page 2
F344         Slot-Id of RAM in page 3

For every page: slot-Id of RAM. Is only valid if a DiskROM is present and active.

Note: In case of active Disk-Basic 2 or MSX-DOS 2 this refers always to a memory-mapper.
If Disk-Basic 2 or MSX-DOS 2 is not active a RAM-expansion in a prior slot (before the
memory-mapper) may take precedence, so that it's not used - even if a mapper is available
in a slot with higher Slot-Id.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f345", "", "Variaveis RAM", "F345", "It is used for the provision of the number of free buffers (x 025h).",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F345

It is used for the provision of the number of free buffers (x 025h).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f346", "", "Variaveis RAM", "F346", "Indicates if the system started with DOS.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F346

Indicates if the system started with DOS.
This flag controls the behavior of the CALL SYSTEM statement: NZ->allowed, Z->not allowed.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f348", "", "Variaveis RAM", "F348", "Slot-Id of the Disk-ROM. If there are multiple Disk-ROMs this is the slot-id,",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F348

Slot-Id of the Disk-ROM. If there are multiple Disk-ROMs this is the slot-id,
of the ROM that handles requests to disk-devices.

Note: active MSX-DOS 2.x cartridges or IDE-controllers will take precedence before
internal Disk-Basic 1 ROMs. Therefore the MSX-DOS 2.x ROM (or IDE-controller, ...) can be
found here - and provide the necessary extensions!

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f34d", "", "Variaveis RAM", "F34D", "2 bytes with address of temporary FAT-sector.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F34D

2 bytes with address of temporary FAT-sector.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f34f", "", "Variaveis RAM", "F34F", "2 bytes with address of temporary DATA-sector.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F34F

2 bytes with address of temporary DATA-sector.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f351", "", "Variaveis RAM", "F351", "2 bytes with address of temporary DIRECTORY-sector.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F351

2 bytes with address of temporary DIRECTORY-sector.

This address is also used by the MSX-Basic statements DSKI$ and DSKO$ (read-only).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f353", "", "Variaveis RAM", "F353", "2 bytes with address to ???.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F353

2 bytes with address to ???.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f368", "", "Variaveis RAM", "F368 SETROM/ROMUSE  Set DISK-BASIC ROM", "Activates the DiskROM under page 1. Only works during DOS.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F368 SETROM/ROMUSE  Set DISK-BASIC ROM

Activates the DiskROM under page 1. Only works during DOS.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f36b", "", "Variaveis RAM", "F36B SETRAM/RAMUSE  Set system RAM configuration", "Activates the RAM under page 1. Only works during DOS.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F36B SETRAM/RAMUSE  Set system RAM configuration

Activates the RAM under page 1. Only works during DOS.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f377", "", "Variaveis RAM", "F377         jump to BLOAD routine", "F377 jump to BLOAD routine",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F377         jump to BLOAD routine

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f37a", "", "Variaveis RAM", "F37A         jump to BSAVE routine", "F37A jump to BSAVE routine",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F37A         jump to BSAVE routine

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f3df", "", "Variaveis RAM", "F3DF RG0SAV: DEFB 00H", "F3E0 RG1SAV: DEFB F0H",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3DF RG0SAV: DEFB 00H
F3E0 RG1SAV: DEFB F0H
F3E1 RG2SAV: DEFB 00H
F3E2 RG3SAV: DEFB 00H
F3E3 RG4SAV: DEFB 01H
F3E4 RG5SAV: DEFB 00H
F3E5 RG6SAV: DEFB 00H
F3E6 RG7SAV: DEFB F4H

These eight variables mimic the state of the eight write-only VDP Mode
Registers. These values shown are for 40x24 Text Mode.

Copies of registers 0-7 are located at RG8SAV (FFFE7H), registers 25-27 at RG25SA (FFFAH).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f40f", "", "Variaveis RAM", "F40F ENDPGM,ENDPRG  DB \":\" Dummy program end for RESUME NEXT st.", "F410 DB 00H",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F40F ENDPGM,ENDPRG  DB ":" Dummy program end for RESUME NEXT st.
F410                DB 00H
F411                DB 00H
F412                DB 00H
F413                DB 00H

These five bytes form a dummy program line. Their values are
set at power-up and thereafter unaltered. The line exists in
case an error occurs in the Interpreter Mainloop before any
tokenised text is available in KBUF. If an "ON ERROR GOTO" is
active at this time then it provides some text for the "RESUME"
statement to terminate on.

        .WORK USED BY BASIC INTERNALLY

F414 ERRFLG  Error number

This variable is used by the Interpreter error handler to save
the error number.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f41f", "", "Variaveis RAM", "F41F KBUF: DEFS 318  Crunch buffer;translated into intermediate", "language from BUF",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F41F KBUF: DEFS 318  Crunch buffer;translated into intermediate
                     language from BUF

This buffer contain the tokenised for of the input line
collected by the Interpreter Mainloop. When a direct statement
is executed the contents of this buffer form the program text.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f55d", "", "Variaveis RAM", "F55D BUFMIN: DB \",\"  used in INPUT st.", "This byte provides a dummy prefix to the text contained in BUF.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F55D BUFMIN: DB ","  used in INPUT st.

This byte provides a dummy prefix to the text contained in BUF.
It is used to synchonize the "INPUT" statement handler as it
starts to analyse the input text.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f55e", "", "Variaveis RAM", "F55E BUF: DEFS 258  Buffer to store characters typed( in ASCII code)", "This buffer contains the text collected from the console by the",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F55E BUF: DEFS 258  Buffer to store characters typed( in ASCII code)

This buffer contains the text collected from the console by the
INLIN standard routine.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f971", "", "Variaveis RAM", "F971 QUEBAK: DEFB 00H   ;AQ Putback character", "DEFB 00H ;BQ Putback character",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F971 QUEBAK: DEFB 00H   ;AQ Putback character
             DEFB 00H   ;BQ Putback character
             DEFB 00H   ;CQ Putback character
             DEFB 00H   ;RQ Putback character

These four variables are used to hold any unwanted character returned to the
associated queue. Althrough the putback facility is implemented in the MSX
ROM it is currently unused.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("f975", "", "Variaveis RAM", "F975 VOICAQ: DEFS 128", "F9F5 VOICBQ: DEFS 128",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F975 VOICAQ: DEFS 128
F9F5 VOICBQ: DEFS 128
FA75 VOICCQ: DEFS 128

These buffers contain the three music queues.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("faf5", "", "Variaveis RAM", "FAF5 RS2IQ:  DEFS 64", "MSX1 only: RS232 queue. It's unused and redefined since MSX2.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FAF5 RS2IQ:  DEFS 64

MSX1 only: RS232 queue. It's unused and redefined since MSX2.

                   FOR MSX 2 ONLY
FAF5 DPPAGE  Display page number
FAF6 ACPAGE  active page number
FAF7 AVCSAV  Reserves AV control port

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fb20", "", "Variaveis RAM", "FB20 LSTMOD, HOKVLD", "If bit-0 (LSB) is 0, there is no extended BIOS nor the mapper support. Otherwise,",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FB20 LSTMOD, HOKVLD

If bit-0 (LSB) is 0, there is no extended BIOS nor the mapper support. Otherwise,
"EXTBIO" entry (see below) has been set up and it can be called with various parameters.
Note that this test is unnecessary for the applications which are based on MSX-DOS (such as
the program which is loaded from the disk), and the program may proceed to the next step.

                Data area used by MSX-DOS
FB21-FB28 DRVINF: DEFS 8

2 bytes entry per DiskROM:

00: number of drives handled by this DiskROM

01: slot ID of this DiskROM

for each of all 4 possible disk-ROMs.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fb29", "", "Variaveis RAM", "FB29-FB34: DEFS 12", "Table with slot ID and start-adress of the interrupt-routines for all 4 possible disk-ROMs.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FB29-FB34: DEFS 12

Table with slot ID and start-adress of the interrupt-routines for all 4 possible disk-ROMs.

00: slot ID of this DiskROM

01+02: adress of the interrupt-routine

                  music
FB35 PRSCNT
FB36 SAVSP
FB38 VOICEN
FB39 SAVVOL
FB3B MCLLEN
FB3C MCLPTR
FB3E QUEUEN
FB3F MUSICF
FB40 PLYCNT
FB41 VCBA
FB66 VCBB
FB8B VCBC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fbb2", "", "Variaveis RAM", "FBB2 LINTTB: DEFS 24   ;line terminator table", "Each of these twenty-four variables is normally non-zero but is",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBB2 LINTTB: DEFS 24   ;line terminator table

Each of these twenty-four variables is normally non-zero but is
zeroed if the contents of the corresponding screen-row have
overflowed to the next row. They are maintained by the BIOS
but only actually used by the INLIN standard routine (the
screen editor) to discriminate between logical and physical
lines.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fbca", "", "Variaveis RAM", "FBCA FSTPOS: DEFW 0000H   ;first position for inlin", "This variable is used to hold the cursor coordinates upon entry",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBCA FSTPOS: DEFW 0000H   ;first position for inlin

This variable is used to hold the cursor coordinates upon entry
to the INLIN standard routine. Its function is to restrict the
extent of backtracking performed when the text is collected
from the screen at termination.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fbcc", "", "Variaveis RAM", "FBCC CURSAV,CODSAV   code save area for cursor", "This variable is used to hold the screen character replaced by",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBCC CURSAV,CODSAV   code save area for cursor

This variable is used to hold the screen character replaced by
the text cursor.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fbcd", "", "Variaveis RAM", "FBCD FNKSWI: DB   00H   ;indicate which func key is displayed", "This variable is used by the CHSNS standard routine to",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBCD FNKSWI: DB   00H   ;indicate which func key is displayed

This variable is used by the CHSNS standard routine to
determine whether the shifted or unshifted function keys
are currently displayed: 00H=shifted, 01H=unshifted.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fbce", "", "Variaveis RAM", "FBCE FNKFLG: DEFS 10   ;fkey which have subroutine", "Each of these ten variables is normally zero but is set to 01H",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBCE FNKFLG: DEFS 10   ;fkey which have subroutine

Each of these ten variables is normally zero but is set to 01H
if the associated function key has been turned on by a "KEY(n)
ON" statement. They are used by the interrupt handler to
determine whether, in program mode only, it should return a
character string or update the associated entry in TRPTBL.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fbda", "", "Variaveis RAM", "FBDA OLDKEY: DEFS 11", "This buffer is used by the interrupt handler to hold the",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBDA OLDKEY: DEFS 11

This buffer is used by the interrupt handler to hold the
previous state of the keyboard matrix, each byte contains one
row of keys starting with row 0.

Note: bits of pressed keys are 0, where bits of released keys are 1.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fbe5", "", "Variaveis RAM", "FBE5 NEWKEY: DEFS 11", "This buffer is used by the interrupt handler to hold the",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBE5 NEWKEY: DEFS 11

This buffer is used by the interrupt handler to hold the
current state of the keyboard matrix. The key transitions are
detected by comparison with the contents of OLDKEY after which
OLDKEY is updated with the current state.

Note: bits of pressed keys are 0, where bits of released keys are 1.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fbf0", "", "Variaveis RAM", "FBF0 KEYBUF: DEFS 40   ;key code buffer", "This buffer contains the decoded keyboard characters produced",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBF0 KEYBUF: DEFS 40   ;key code buffer

This buffer contains the decoded keyboard characters produced
by the interrupt handler. Note that the buffer is organized as
a circular queue driven by GETPNT and PUTPNT and consequently
has no fixed starting point.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fc40", "", "Variaveis RAM", "FC40 PATWRK: DEFS 8", "This buffer is used by the BIOS to hold an 8x8 pixel pattern.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC40 PATWRK: DEFS 8

This buffer is used by the BIOS to hold an 8x8 pixel pattern.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fcall", "", "Variaveis RAM", "FFCA    FCALL   or EXTBIO: extended BIOS call", "To use the extended BIOS, the program should test \"HOKVLD\" flag at FB20h in page-3.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FFCA    FCALL   or EXTBIO: extended BIOS call

To use the extended BIOS, the program should test "HOKVLD" flag at FB20h in page-3.
If bit-0 (LSB) is 0, there is no extended BIOS nor the mapper support. Otherwise,
"EXTBIO" entry has been set up and it can be called with various parameters.
Note that this test is unnecessary for the applications which are based on MSX-DOS (such as
the program which is loaded from the disk), and the program may proceed to the next step.

Next, the program sets the device number of the extended BIOS in register D, the function number
in register E, and required parameters in other registers, and then calls "EXTBIO" at FFCAh in page-3.
In this case, the stack pointer must be in page-3.
If there is the extended BIOS for the specified device number, the contents of the registers
AF, BC and HL are modified according to the function; otherwise, they are preserved.
Register DE is always preserved. Note that in any cases the contents of the alternative
registers (AF', BC', DE' and HL') and the index registers (IX and IY) are corrupted.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fcc9", "", "Variaveis RAM", "FCC9 SLTATR: DEFS 4     ; PS0,SS0", "DEFS 4 ; PS0,SS1",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCC9 SLTATR: DEFS 4     ; PS0,SS0
             DEFS 4     ; PS0,SS1
             DEFS 4     ; PS0,SS2
             DEFS 4     ; PS0,SS3

             DEFS 4     ; PS1,SS0
             DEFS 4     ; PS1,SS1
             DEFS 4     ; PS1,SS2
             DEFS 4     ; PS1,SS3

             DEFS 4     ; PS2,SS0
             DEFS 4     ; PS2,SS1
             DEFS 4     ; PS2,SS2
             DEFS 4     ; PS2,SS3

             DEFS 4     ; PS3,SS0
             DEFS 4     ; PS3,SS1
             DEFS 4     ; PS3,SS2
             DEFS 4     ; PS3,SS3

These 64 variables contain the attributes of any extension ROMs found during the power-up ROM search.
The extension characteristics of each 16KB ROM are encoded into a single byte so four bytes are
required for each possible slot. The encoding is:

  Bit 7 set = BASIC program

  Bit 6 set = Device handler

  Bit 5 set = Statement handler

Note that the entries for page 0 (0000H to 3FFFH) and page 3 (C000H to FFFFH) will always be zero
as only page 1 (4000H to 7FFFH) and page 2 (8000H to BFFFH) are actually examined. The MSX convention
is that machine code extension ROMs are placed in page 1 and BASIC program ROMs in page 2.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fd09", "", "Variaveis RAM", "FD09 SLTWRK: DEFS 128", "FD09 DEFW 0, 0, 0, 0 ;PS0, SS0",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FD09 SLTWRK: DEFS 128
FD09         DEFW 0, 0, 0, 0   ;PS0, SS0
FD11         DEFW 0, 0, 0, 0   ;PS0, SS1
FD19         DEFW 0, 0, 0, 0   ;PS0, SS2
FD21         DEFW 0, 0, 0, 0   ;PS0, SS3
FD29         DEFW 0, 0, 0, 0   ;PS1, SS0
FD31         DEFW 0, 0, 0, 0   ;PS1, SS1
FD39         DEFW 0, 0, 0, 0   ;PS1, SS2
FD41         DEFW 0, 0, 0, 0   ;PS1, SS3
FD49         DEFW 0, 0, 0, 0   ;PS2, SS0
FD51         DEFW 0, 0, 0, 0   ;PS2, SS1
FD59         DEFW 0, 0, 0, 0   ;PS2, SS2
FD61         DEFW 0, 0, 0, 0   ;PS2, SS3
FD69         DEFW 0, 0, 0, 0   ;PS3, SS0
FD71         DEFW 0, 0, 0, 0   ;PS3, SS1
FD79         DEFW 0, 0, 0, 0   ;PS3, SS2
FD81         DEFW 0, 0, 0, 0   ;PS3, SS3

This buffer provides two bytes of logical workspace for each of
the sixty-four possible extension ROMs.

Exceptions: on Korean MSX computers the following bytes are reserved:

FD0D         DEFS 1            ;SlotID of ROM with characters for SCREEN 9
                               ;(located in 4000-7FFF)
FD0E         DEFS 1            ;SlotID of Hangul ROM
                               ;(located in 4000-7FFF on MSX1, in 4000-BFFF on MSX2)
FD10         DEFS 1            ;?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fd89", "", "Variaveis RAM", "FD89 PROCNM: DEFS 16  name of expanded statement", "This buffer is used to hold a device or statement name for",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FD89 PROCNM: DEFS 16  name of expanded statement

This buffer is used to hold a device or statement name for
examination by an extension ROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fd9f", "", "Variaveis RAM", "FD9F+   HTIMI   0C53  Interrupt handler", "This hook is invoked on VDP-interrupts, after the screen has been",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FD9F+   HTIMI   0C53  Interrupt handler

This hook is invoked on VDP-interrupts, after the screen has been
refreshed (50 or 60 times per second, depending on the refresh
frequency).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fda4", "", "Variaveis RAM", "FDA4+/scHCHPU   08C0  CHPUT standard routine (A-character;SAVE ALL)", "FDA9+/scHDSPC 09E6 Display cursor (no param.)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FDA4+/scHCHPU   08C0  CHPUT standard routine (A-character;SAVE ALL)
        FDA9+/scHDSPC   09E6  Display cursor    (no param.)
        FDAE+/scHERAC   0A33  Errase cursor     (no param.)
        FDB3+/scHDSPF   0B2B  DSPFNK std routine(no param.)
        FDB8+/scHERAF   0B15  ERAFNK std routine(no param.)
        FDBD    HTOTE   0842  TOTEXT std routine

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fdc2", "", "Variaveis RAM", "FDC2+   HCHGE   10CE  CHGET  std routine", "Not verified: this hook procedure is invoked from CHGET before",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FDC2+   HCHGE   10CE  CHGET  std routine

Not verified: this hook procedure is invoked from CHGET before
the CHSNS routine(s) are executed and check if characters are arrived.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fdc7", "", "Variaveis RAM", "FDC7+   HINIP   071E  Copy character set to VDP", "FDC7+ HINIP 071E Copy character set to VDP",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FDC7+   HINIP   071E  Copy character set to VDP

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fetchc", "", "BIOS-ROM", "0114H FETCHC Fetch current pixel physical address", "0114H FETCHC Fetch current pixel physical address",
        R"MAN(AREA: BIOS principal (MainROM)

0114H FETCHC Fetch current pixel physical address

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ffcf", "", "Variaveis RAM", "FFCF    HBGFD,DISKON   before physical operation with disk controller", "FFD4 HENFD after physical operation with disk controller",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FFCF    HBGFD,DISKON   before physical operation with disk controller
        FFD4    HENFD          after physical operation with disk controller

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ffe7", "", "Variaveis RAM", "FFE7 RG8SAV: DEFB 00H  ;Copy of VDP-register  8  (Basic:  9)", "FFE8 RG9SAV: DEFB 00H ;Copy of VDP-register 9 (Basic: 10)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FFE7 RG8SAV: DEFB 00H  ;Copy of VDP-register  8  (Basic:  9)
FFE8 RG9SAV: DEFB 00H  ;Copy of VDP-register  9  (Basic: 10)
FFE9 RG10SA: DEFB 00H  ;Copy of VDP-register 10  (Basic: 11)
FFEA RG11SA: DEFB 00H  ;Copy of VDP-register 11  (Basic: 12)
FFEB RG12SA: DEFB 00H  ;Copy of VDP-register 12  (Basic: 13)
FFEC RG13SA: DEFB 00H  ;Copy of VDP-register 13  (Basic: 14)
FFED RG14SA: DEFB 00H  ;Copy of VDP-register 14  (Basic: 15)
FFEE RG15SA: DEFB 00H  ;Copy of VDP-register 15  (Basic: 16)
FFEF RG16SA: DEFB 00H  ;Copy of VDP-register 16  (Basic: 17)
FFF0 RG17SA: DEFB 00H  ;Copy of VDP-register 17  (Basic: 18)
FFF1 RG18SA: DEFB 00H  ;Copy of VDP-register 18  (Basic: 19)
FFF2 RG19SA: DEFB 00H  ;Copy of VDP-register 19  (Basic: 20)
FFF3 RG20SA: DEFB 00H  ;Copy of VDP-register 20  (Basic: 21)
FFF4 RG21SA: DEFB 00H  ;Copy of VDP-register 21  (Basic: 22)
FFF5 RG22SA: DEFB 00H  ;Copy of VDP-register 22  (Basic: 23)
FFF6 RG23SA: DEFB 00H  ;Copy of VDP-register 23  (Basic: 24)

The above Bytes contain a copy of the write-only VDP-registers 8 to 23 (since MSX2).
Copies of registers 0-7 are located at RG0SAV (F3DFH), registers 25-27 at RG25SAV (FFFAH).

Caution: because the statusregister VDP(8) is only valid for Basic the register-numbers are shifted.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fff8", "", "Variaveis RAM", "FFF8: DW ????H  ?", "??? (#17A6, GT rom)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FFF8: DW ????H  ?

??? (#17A6, GT rom)

Byte on &HFFF8 may contain a copy of the optional A/V-Controller byte
(see also I/O-port &HF8).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fffa", "", "Variaveis RAM", "FFFA RG25SA: DEFB 00H  ;Copy of VDP-register 25  (Basic: 26)", "FFFB RG26SA: DEFB 00H ;Copy of VDP-register 26 (Basic: 27)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FFFA RG25SA: DEFB 00H  ;Copy of VDP-register 25  (Basic: 26)
FFFB RG26SA: DEFB 00H  ;Copy of VDP-register 26  (Basic: 27)
FFFC RG27SA: DEFB 00H  ;Copy of VDP-register 27  (Basic: 28)

The above Bytes contain a copy of the write-only VDP-registers 25 to 27 (since MSX2+).
Copies of registers 0-7 are located at RG0SAV (F3DFH), registers 8-23 at RG8SAV (FFE7H).

Caution: because the statusregister VDP(8) is only valid for Basic the register-numbers are shifted.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fffd", "", "Variaveis RAM", "FFFD ?: DW ????", "These 16bits are used for saving the stack-pointer while switching the CPU",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FFFD ?: DW ????

These 16bits are used for saving the stack-pointer while switching the CPU
between Z80 and R800, which is performed by the CHGCPU routine.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ffff", "", "Variaveis RAM", "FFFF ?", "This byte controls the Slot-Expander of the currently active primary slot. Because each primary...",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FFFF ?

This byte controls the Slot-Expander of the currently active primary slot. Because each primary slot contains it's own slot-expander page 3 (C000H-FFFFH) should be changed first, by using WSLREG (013BH). Now the secondary-slot register can be changed and -if necessary- page 3 can be restored to the original primary slot.

Important: if a slot-expander is present the read value is the binary complement of the correct value. If writing to this register the correct value must be used!

Source: information on this page are collected from the "The MSX Red Book"
and various web-sites.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("filvrm", "", "BIOS-ROM", "0056H FILVRM Fill block of VRAM with data byte", "Standard routine to fill a block of the VDP VRAM with a single data byte.",
        R"MAN(AREA: BIOS principal (MainROM)

0056H FILVRM Fill block of VRAM with data byte

Standard routine to fill a block of the VDP VRAM with a single data byte.
Entry: | A = data byte
BC = length
HL = write address
Exit: | none
Modifies: | AF, BC, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("find environment item", "", "BDOS", "6D Find environment item", "6D Find environment item",
        R"MAN(AREA: BDOS (MSX-DOS)

6D Find environment item

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("find first entry", "", "BDOS", "40 Find first entry", "40 Find first entry",
        R"MAN(AREA: BDOS (MSX-DOS)

40 Find first entry

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("find new entry", "", "BDOS", "42 Find new entry", "42 Find new entry",
        R"MAN(AREA: BDOS (MSX-DOS)

42 Find new entry

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("find next entry", "", "BDOS", "41 Find next entry", "41 Find next entry",
        R"MAN(AREA: BDOS (MSX-DOS)

41 Find next entry

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("flbmem", "", "Variaveis RAM", "FCAE FLBMEM  0 if loading basic programm", "This variable is set by the file I/O error generators but is",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCAE FLBMEM  0 if loading basic programm

This variable is set by the file I/O error generators but is
otherwise unused.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("flush disk buffers", "", "BDOS", "5F Flush disk buffers", "5F Flush disk buffers",
        R"MAN(AREA: BDOS (MSX-DOS)

5F Flush disk buffers

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fmtunit", "", "BIOS-ROM", "7F9EH FmtUnit", "FORMAT UNIT.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F9EH FmtUnit

FORMAT UNIT.
Entry: | A = LUN + SCSI Target ID
B = 000FCDDD
                    ||+++---- Defectlist format
                    |+--------- Complete
                    +------------ Format data
DE = interleave (msb - lsb)
HL = data address
Exit: | A = SCSI STATUS
D = Target Status
E = Message
Modifies: | AF,BC,DE,HL
Note: | See ANSI 8.2.1

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fnksb", "", "BIOS-ROM", "00C9H FNKSB  Check if function key display on", "00C9H FNKSB Check if function key display on",
        R"MAN(AREA: BIOS principal (MainROM)

00C9H FNKSB  Check if function key display on

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("forclr", "", "Variaveis RAM", "F3E9 FORCLR  Foreground colour", "This variable contains the current foreground color. Its value is set at power-up",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3E9 FORCLR  Foreground colour

This variable contains the current foreground color. Its value is set at power-up
and thereafter only alterd by the "COLOR" statement. The foreground color is used
by the CLRSPR standard routine to set the sprite color and the CHGCLR standard
routine to set the 1 pixel color in the text modes. It also functions as the
graphics ink color as it is copied to ATRBYT by the GRPPRT standard routine and
used throughout the interpreter as the default value for any optional color operand.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("fork a child process", "", "BDOS", "60 Fork a child process", "60 Fork a child process",
        R"MAN(AREA: BDOS (MSX-DOS)

60 Fork a child process

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("format", "", "BIOS-ROM", "0147H FORMAT Format disk", "Standard routine for use by Disk BASIC. If not present the routine returns without action.",
        R"MAN(AREA: BIOS principal (MainROM)

0147H FORMAT Format disk

Standard routine for use by Disk BASIC. If not present the routine returns without action.
Otherwise it formats a floppy disk.

If more than one floppy drive is present the routine prompts for the drive
to be formatted. If the drive supports multiple disk-formats also the requested
format has to be selected. In all cases the user must confirm the "Strike a key when ready"
prompt to finally format the drive.

This is the prefered method to format drives, because all installed Disk-ROMs are
considered by this routine! The MSX-Standard supports up to 4 Disk-ROMs with a maximum
of 8 drive-letters (A:-H:). In contrast to the DSKFMT (401CH) routine of the Disk-ROM
this routine does not support pre-selection of drive-letters, disk-formats or work-areas.
This routine is very similar to the DSKFMT routine in the
Disk-ROM, so more information about it's parameters
and return-values can be found there!
Note: harddisks do not support formatting, but may implement different functionality
in this routine (e.g. enable/disable write-protection, e.g. in HSH or NOVAXIS SCSI ROMs).
Entry: | none?
Exit: | F = CARRY if error
A = errorcode (if error)
Modifies: | ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("format a disk", "", "BDOS", "67 Format a disk", "67 Format a disk",
        R"MAN(AREA: BDOS (MSX-DOS)

67 Format a disk

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("frcnew", "", "Variaveis RAM", "F3F5 FRCNEW  DB 255", "This variable contains a flag to distinguish the two statements",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3F5 FRCNEW  DB 255

This variable contains a flag to distinguish the two statements
in the "CLOAD/CLOAD?" statement handler: 00H=CLOAD, FFH=CLOAD?.

        .WORK AREA FOR KEY INPUT

F3F6 SCNCNT  Interval for the key scan

This variable is used as a counter by the interrupt handler to
control the rate at which keyboard scans are performed.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get allocation", "", "BDOS", "1B Get Allocation", "1B Get Allocation",
        R"MAN(AREA: BDOS (MSX-DOS)

1B Get Allocation

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get current directory", "", "BDOS", "59 Get current directory", "59 Get current directory",
        R"MAN(AREA: BDOS (MSX-DOS)

59 Get current directory

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get date", "", "BDOS", "2A Get Date", "2A Get Date",
        R"MAN(AREA: BDOS (MSX-DOS)

2A Get Date

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get default drive name", "", "BDOS", "19 Get Default Drive Name", "Returns the default drive number. 0 is drive A:, 1 drive B:, ...",
        R"MAN(AREA: BDOS (MSX-DOS)

19 Get Default Drive Name

Returns the default drive number. 0 is drive A:, 1 drive B:, ...
Entry: | none
Exit: | drive-number in A
CP/M: | yes

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get disk parameters", "", "BDOS", "31 Get disk parameters", "31 Get disk parameters",
        R"MAN(AREA: BDOS (MSX-DOS)

31 Get disk parameters

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get disk transfer address", "", "BDOS", "57 Get disk transfer address", "57 Get disk transfer address",
        R"MAN(AREA: BDOS (MSX-DOS)

57 Get disk transfer address

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get environment item", "", "BDOS", "6B Get environment item", "6B Get environment item",
        R"MAN(AREA: BDOS (MSX-DOS)

6B Get environment item

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get file size", "", "BDOS", "23 Get File Size", "23 Get File Size",
        R"MAN(AREA: BDOS (MSX-DOS)

23 Get File Size

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get login vector", "", "BDOS", "18 Get Login Vector", "18 Get Login Vector",
        R"MAN(AREA: BDOS (MSX-DOS)

18 Get Login Vector

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get msx-dos version number", "", "BDOS", "6F Get MSX-DOS version number", "6F Get MSX-DOS version number",
        R"MAN(AREA: BDOS (MSX-DOS)

6F Get MSX-DOS version number

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get previous error code", "", "BDOS", "65 Get previous error code", "65 Get previous error code",
        R"MAN(AREA: BDOS (MSX-DOS)

65 Get previous error code

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get time", "", "BDOS", "2C Get Time", "2C Get Time",
        R"MAN(AREA: BDOS (MSX-DOS)

2C Get Time

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get verify flag setting", "", "BDOS", "58 Get verify flag setting", "58 Get verify flag setting",
        R"MAN(AREA: BDOS (MSX-DOS)

58 Get verify flag setting

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get version number", "", "BDOS", "0C Get Version Number", "0C Get Version Number",
        R"MAN(AREA: BDOS (MSX-DOS)

0C Get Version Number

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get whole path string", "", "BDOS", "5E Get whole path string", "5E Get whole path string",
        R"MAN(AREA: BDOS (MSX-DOS)

5E Get whole path string

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get/set disk check status", "", "BDOS", "6E Get/set disk check status", "6E Get/set disk check status",
        R"MAN(AREA: BDOS (MSX-DOS)

6E Get/set disk check status

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get/set file attributes", "", "BDOS", "50 Get/set file attributes", "50 Get/set file attributes",
        R"MAN(AREA: BDOS (MSX-DOS)

50 Get/set file attributes

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get/set file date and time", "", "BDOS", "51 Get/set file date and time", "51 Get/set file date and time",
        R"MAN(AREA: BDOS (MSX-DOS)

51 Get/set file date and time

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get/set file handle attributes", "", "BDOS", "55 Get/set file handle attributes", "55 Get/set file handle attributes",
        R"MAN(AREA: BDOS (MSX-DOS)

55 Get/set file handle attributes

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get/set file handle date and time", "", "BDOS", "56 Get/set file handle date and time", "56 Get/set file handle date and time",
        R"MAN(AREA: BDOS (MSX-DOS)

56 Get/set file handle date and time

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("get/set redirection status", "", "BDOS", "70 Get/set redirection status", "More functions",
        R"MAN(AREA: BDOS (MSX-DOS)

70 Get/set redirection status

More functions

Beside these MSX-DOS functions also the Basic routines RDSLT (000C), WRSLT (0014), CALSLT (001C),
ENASLT (0024) and CALLF (0030) are available. If you call one of these routines please ensure that
the stack-area remains valid. Calling a rom routine switches page 0 (0000-3FFF).
If - during this time - an interrupt occurs probably page 1 (4000-7FFF) will be switched too.

It's safe and common to set the stackpointer during program initialisation to the top of
available memory. To do so the bytes 6 and 7 can be used:

LD   SP,(0006)

If you change the stackpointer in this way its not possible to use RET to return from your

program. In this case the MSX-DOS function "System Reset" has to be used:

LD   C,0
JP   0005

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getbyt", "", "BIOS-ROM", "0028H GETBYT (RST 28H)", "This routine is equal to the MainROM.",
        R"MAN(AREA: Sub-ROM do MSX2

0028H GETBYT (RST 28H)

This routine is equal to the MainROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getcpu", "", "BIOS-ROM", "0183H GETCPU Get CPU", "Check in what CPU mode the system currently is.",
        R"MAN(AREA: BIOS principal (MainROM)

0183H GETCPU Get CPU

Check in what CPU mode the system currently is.
Entry: | none
Exit: | A
----------------------------------------------------
value  meaning
----------------------------------------------------
 0      Z80 MODE
 1      R800 ROM MODE
 2      R800 DRAM MODE
----------------------------------------------------
Modifies: | F

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getdpb", "", "BIOS-ROM", "4016H GETDPB Get disk-parameter-block", "Gets the disk-format.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

4016H GETDPB Get disk-parameter-block

Gets the disk-format.
Entry: | A = Drive number (0=A:, 1=B:, ...)
B = 1st byte of FAT (media descriptor)
C = media-descriptor
HL = Base address of DPB
Exit: | [HL+1] ... [HL+18] = DPB for specified drive
DPB consists of:
MEDIA    0     1   Media type (F8..FF)
SECSIZ   1     2   Sector size (must be 2^n)
DIRMSK   3     1   (SECSIZE/32)-1
DIRSHFT  4     1   Number of one bits in DIRMSK
CLUSMSK  5     1   (Sectors per cluster)-1
CLUSSHFT 6     1   (Number of one bits in CLUSMSK)+1
FIRFAT   7     2   Logical sector number of first FAT
FATCNT   8     1   Number of FATs
MAXENT   A     1   Number of directory entries
                   (max. 254)
FIRREC   B     2   Logical sector number of first data
MAXCLUS  D     2   Number of clusters
                   (not including reserved, FAT and
                   directory sectors)+1
FATSIZ   F     1   Number of sectors used
FIRDIR   10    2   FAT logical sector number of start
                   of directory
Modifies: | ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getpat", "", "BIOS-ROM", "0105H GETPAT", "0105H GETPAT",
        R"MAN(AREA: Sub-ROM do MSX2

0105H GETPAT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getplt", "", "BIOS-ROM", "0149H GETPLT", "0149H GETPLT",
        R"MAN(AREA: Sub-ROM do MSX2

0149H GETPLT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getpnt", "", "Variaveis RAM", "F3FA GETPNT  DW KEYBUF  points to adress to read from the key buffer", "This variable contains the address of the get position in",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3FA GETPNT  DW KEYBUF  points to adress to read from the key buffer

This variable contains the address of the get position in
KEYBUF.

        .PARAMETERS FOR CASSETTE
F3FC CS1200   Reading parameters
     DB LOW01,HIGH01,LOW11,HIGH11,HEDLEN*2/256 ; FOR 1200 BAUD

These five variables contain the 1200 baud cassette parameters.
Their values are set at power-up and thereafter unaltered.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getput", "", "BIOS-ROM", "01B1H GETPUT (Basic!)", "01B1H GETPUT (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

01B1H GETPUT (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getvc2", "", "BIOS-ROM", "0153H GETVC2 Get music voice pointer", "Standard routine to return the address of any byte in the voice buffer",
        R"MAN(AREA: BIOS principal (MainROM)

0153H GETVC2 Get music voice pointer

Standard routine to return the address of any byte in the voice buffer
(VCBA, BCBB or VCBC) specified by the voice number in VOICEN (FB38H).
Entry: | L = byte number (0 to 36)
Exit: | HL = address in voice buffer
Modifies: | AF, HL

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getvcp", "", "BIOS-ROM", "0150H GETVCP Get music voice pointer", "Standard routine to return the address of byte 2 in the specified",
        R"MAN(AREA: BIOS principal (MainROM)

0150H GETVCP Get music voice pointer

Standard routine to return the address of byte 2 in the specified
voice buffer (VCBA, VCBB or VCBC).
Entry: | L = voice number (0, 1, 2)
Exit: | HL = address in voice buffer
Modifies: | AF, HL

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("getypr", "", "BIOS-ROM", "0028H GETYPR Get BASIC operand type", "Standard routine to return the type of the current operand, determined by VALTYP as follows:",
        R"MAN(AREA: BIOS principal (MainROM)

0028H GETYPR Get BASIC operand type

Standard routine to return the type of the current operand, determined by VALTYP as follows:

Integer | A=FFH | Flag M,NZ,C
String | A=00H | Flag P,Z,C
Single precision | A=01H | Flag P,NZ,C
Double precision | A=05H | Flag P,NZ,NC

Entry: | AF=Type
Exit: | none
Modifies: | AF

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ghgmdp", "", "BIOS-ROM", "01B5H GHGMDP", "01B5H GHGMDP",
        R"MAN(AREA: Sub-ROM do MSX2

01B5H GHGMDP

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gicini", "", "BIOS-ROM", "0090H GICINI Initialize PSG (GI chip)", "0090H GICINI Initialize PSG (GI chip)",
        R"MAN(AREA: BIOS principal (MainROM)

0090H GICINI Initialize PSG (GI chip)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gline", "", "BIOS-ROM", "0075H GLINE", "0075H GLINE",
        R"MAN(AREA: Sub-ROM do MSX2

0075H GLINE

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("grpacx", "", "Variaveis RAM", "FCB7 GRPACX", "FCB7 GRPACX",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCB7 GRPACX

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("grpacy", "", "Variaveis RAM", "FCB9 GRPACY", "FCB9 GRPACY",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCB9 GRPACY

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("grphed", "", "Variaveis RAM", "FCA6 GRPHED  flag for graph. char", "FCA6 GRPHED flag for graph. char",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCA6 GRPHED  flag for graph. char

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("grpprt", "", "BIOS-ROM", "008DH GRPPRT Print character on graphic screen", "008DH GRPPRT Print character on graphic screen",
        R"MAN(AREA: BIOS principal (MainROM)

008DH GRPPRT Print character on graphic screen

AREA: Sub-ROM do MSX2

0089H GRPPRT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gspsiz", "", "BIOS-ROM", "008AH GSPSIZ Get sprite size", "008AH GSPSIZ Get sprite size",
        R"MAN(AREA: BIOS principal (MainROM)

008AH GSPSIZ Get sprite size

AREA: Sub-ROM do MSX2

0101H GSPSIZ

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gtaspc", "", "BIOS-ROM", "0126H GTASPC Get aspect ratio", "0126H GTASPC Get aspect ratio",
        R"MAN(AREA: BIOS principal (MainROM)

0126H GTASPC Get aspect ratio

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gtpad", "", "BIOS-ROM", "00DBH GTPAD Get touchpad status", "Turbo-R: The lightpen function has been removed. When 8-11 is put in register A and a call is...",
        R"MAN(AREA: BIOS principal (MainROM)

00DBH GTPAD Get touchpad status

Turbo-R: The lightpen function has been removed. When 8-11 is put in register A and a call is made, it will normally return with 0.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gtpdl", "", "BIOS-ROM", "00DEH GTPDL  Get paddle status", "Turbo-R: The paddle function has been removed. the return value will always be 0.",
        R"MAN(AREA: BIOS principal (MainROM)

00DEH GTPDL  Get paddle status

Turbo-R: The paddle function has been removed. the return value will always be 0.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gtstck", "", "BIOS-ROM", "00D5H GTSTCK Get joystick status", "00D5H GTSTCK Get joystick status",
        R"MAN(AREA: BIOS principal (MainROM)

00D5H GTSTCK Get joystick status

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gttrig", "", "BIOS-ROM", "00D8H GTTRIG Get trigger status", "00D8H GTTRIG Get trigger status",
        R"MAN(AREA: BIOS principal (MainROM)

00D8H GTTRIG Get trigger status

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gxpos", "", "Variaveis RAM", "FCB3 GXPOS", "FCB3 GXPOS",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCB3 GXPOS

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("gypos", "", "Variaveis RAM", "FCB5 GYPOS", "FCB5 GYPOS",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCB5 GYPOS

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("highmem", "", "Variaveis RAM", "F349 HIGHMEM", "All from here to &HF37F is used by the Disk-ROM.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F349 HIGHMEM

All from here to &HF37F is used by the Disk-ROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("himem", "", "Variaveis RAM", "FC4A HIMEM", "This variable contains the address of the byte following the",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC4A HIMEM

This variable contains the address of the byte following the
highest RAM location used by the interpreter. Its value is
set at power-up and thereafter only altered by the "CLEAR"
statement.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("hisfl", "", "Variaveis RAM", "FEDF    HISFL   145F  ISFLIO std routine", "FEE4 HOUTD 1B46 OUTDO std routine",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FEDF    HISFL   145F  ISFLIO std routine
        FEE4    HOUTD   1B46  OUTDO std routine
        FEE9    HCRDO   7328  CR,LF to OUTDO
        FEEE    HDSKC   7374  Mainloop line input
        FEF3    HDOGR   593C  Line draw
        FEF8    HPRGE   4039  Program end
        FEFD+   HERRP   40DC  Error handler
        FF02    HERRF   40FD  Error handler
        FF07    HREAD   4128  Mainloop "OK"
        FF0C    HMAIN   4134  Mainloop
        FF11    HDIRD   41A8  Mainloop direct statement
        FF16    HFINI   4237  Mainloop finished
        FF1B    HFINE   4247  Mainloop finished
        FF20    HCRUN   42B9  Tokenise
        FF25    HCRUS   4353  Tokenise
        FF2A    HISRE   437C  Tokenise
        FF2F    HNTFN   43A4  Tokenise
        FF34    HNOTR   44EB  Tokenise
        FF39    HSNGF   45D1  "FOR"
+/c     FF3E+   HNEWS   4601  Runloop new statement
        FF43    HGONE   4646  Runloop execute
        FF48    HCNRG   4666  CHRGTR std routine
        FF4D    HRETU   4821  "RETURN"
        FF52    HPRTF   4A5E  "PRINT"
        FF57    HCOMP   4A94  "PRINT"
        FF5C    HFINP   4AFF  "PRINT"
        FF61    HTRMN   4B4D  "READ/INPUT" error
        FF66    HFRME   4C6D  Expression Evaluator
        FF6B    HNTPL   4CA6  Expression Evaluator
        FF70    HEVAL   4DD9  Factor Evaluator
        FF75    HOKNO   4F2C  Factor Evaluator
        FF75    MDIN          MIDI IN interrupt
        FF7A    HFING   4F3E  Factor Evaluator
        FF7F    HISMI   51C3  Runloop execute
        FF84    HWIDT   51CC  "WIDTH"
        FF89    HLIST   522E  "LIST"
        FF8E    HBUFL   532D  Detokenise
        FF93    HFRQI   543F  Convert to integer
        FF93    MDTM          8253 timer interrupt (MIDI)
        FF98    HSCNE   5514  Line number to pointer
        FF9D    HFRET   67EE  Free descriptor
        FFA2    HPTRG   5EA9  Variable search
        FFA7+   HPHYD   148A  PHYDIO std routine
        FFAC+   HFORM   148E  FORMAT std routine
        FFB1    HERRO   406F  Error handler
        FFB6    HLPTO   085D  LPTOUT std routine
        FFBB    HLPTS   0884  LPTSTT std routine
        FFC0    HSCRE   79CC  "SCREEN"
        FFC5    HPLAY   73E5  "PLAY" statement

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("hkeyc", "", "Variaveis RAM", "FDCC    HKEYC   1025  Keyboard decoder", "Not verified: This hook procedure is invoked to determine which",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FDCC    HKEYC   1025  Keyboard decoder

Not verified: This hook procedure is invoked to determine which
group of keys the key number supplied in register A belongs to.
This hook is called for every input of the keyboard-matrix to be
recognized (even if the key does not generate a character, or it
generates multiple characters).

Commonly defined groups are: "Home", "Stop", "Shift, Ctrl, Graph, Code/Kana,
Esc, Tab, Bs, CR, Del, Cursor", "Caps", "rows 0 to 5" and "F1 to F5".

The key code is available in register A. The mapping table
(next higher key number, internal offset for jump into group-handler)
is available in HL.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("hkyea", "", "Variaveis RAM", "FDD1    HKYEA   0F10  Keyboard decoder", "Not verified: this hook is called by the keyboard decoder",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FDD1    HKYEA   0F10  Keyboard decoder

Not verified: this hook is called by the keyboard decoder
to enable pre-processing of the keys:

SHIFT, CTRL, GRAPH, CODE (Kana?), ESC,
TAB, BS, CR, SEL, SPACE, INS, DEL, CURSOR.
The key code is available in register A.
In contrast to HKEYC this hook is called for the given keys only!

This hook is invoked to processes key numbers
30H to 57H apart from the CAP, F1 to F5, STOP and HOME keys.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("hnmi", "", "Variaveis RAM", "FDD6    HNMI    1398  NMI std routine", "FDDB HPINL 23BF PINLIN std routine",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FDD6    HNMI    1398  NMI std routine
        FDDB    HPINL   23BF  PINLIN std routine
        FDE0    HQINL   23CC  QINLIN std routine
        FDE5    HINLI   23D5  INLIN std routine
        FDEA    HONGO   7810  "ON DEVICE GOSUB"
        FDEF+   HDSKO   7C16  "DSKO$"
        FDF4    HSETS   7C1B  "SET"
        FDF9+   HNAME   7C20  "NAME"
        FDFE+   HKILL   7C25  "KILL"
        FE03    HIPL    7C2A  "IPL"
        FE08+   HCOPY   7C2F  "COPY"
        FE0D    HCMD    7C34  "CMD"
        FE12+   HDSKF   7C39  "DSKF"
        FE17+   HDSKI   7C3E  "DSKI$"
        FE1C    HATTR   7C43  "ATTR$"
        FE21+   HLSET   7C48  "LSET"
        FE26+   HRSET   7C4D  "RSET"
        FE2B+   HFIEL   7C52  "FIELD"
        FE30+   HMKI$   7C57  "MKI$"
        FE35+   HMKS$   7C5C  "MKS$"
        FE3A+   HMKD$   7C61  "MKD$"
        FE3F+   HCVI    7C66  "CVI"
        FE44+   HCVS    7C6B  "CVS"
        FE49+   HCVD    7C70  "CVD"
        FE4E+   HGETP   6A93  Locate FCB
        FE53    HSETF   6AB3  Locate FCB
        FE58+   HNOFO   6AF6  "OPEN": not found
        FE5D+   HNULO   6B0F  "OPEN"
        FE62+   HNTFL   6B3B  Close I/O buffer 0
        FE67    HMERG   6B63  "MERGE/LOAD"
        FE6C    HSAVE   6BA6  "SAVE"
        FE71+   HBINS   6BCE  "SAVE"
        FE76+   HBINL   6BD4  "MERGE/LOAD"
        FE7B+   HFILE   6C2F  "FILES"
        FE80+   HDGET   6C3B  "GET/PUT"
        FE85+   HFILO   6C51  Sequential output
        FE8A+   HINDS   6C79  Sequential input
        FE8F    HRSLF   6CD8  "INPUT$"
        FE94    HSAVD   6D03"LOC",6D14"LOF",6D25"EOF",6D39"FPOS"
        FE99+   HLOC    6D0F  "LOC"
        FE9E+   HLOF    6D20  "LOF"
        FEA3+   HEOF    6D33  "EOF"
        FEA8    HFPOS   6D43  "FPOS"
        FEAD+   HBAKU   6E36  "LINE INPUT#"
        FEB2+   HPARD   6F15  Parse device name
        FEB7+   HNODE   6F33  Parse device name
        FEBC+   HPOSD   6F37  Parse device name
        FEC1    HDEVN   This hook not used
        FEC6    HGEND   6F8F  I/O function despatcher
        FECB    HRUNC   629A  Run-clear
        FED0    HCLEA   62A1  Run-clear
        FED5    HLOPD   62AF  Run-clear

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("hstke", "", "Variaveis RAM", "FEDA    HSTKE   62F0  Reset stack", "This hook can be set during power-up. If set the hook is invoked after the system (all...",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FEDA    HSTKE   62F0  Reset stack

This hook can be set during power-up. If set the hook is invoked after the system (all ROM-extensions, ...) are fully initialized.

This hook is usually set by cartridge-ROM initialisation routines,
if the cartridge-ROM may also need to access the Disk-ROM
(which would be initialized later in many cases).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("i/o control for devices", "", "BDOS", "4B I/O control for devices", "4B I/O control for devices",
        R"MAN(AREA: BDOS (MSX-DOS)

4B I/O control for devices

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("in", "", "Variaveis RAM", "F365         IN A,(0A8H); RET", "F365 IN A,(0A8H); RET",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F365         IN A,(0A8H); RET

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("inifk", "", "BIOS-ROM", "003EH INIFK  Initialize function key strings", "Standard routine to initialise the ten function key strings to their power-up values. The one...",
        R"MAN(AREA: BIOS principal (MainROM)

003EH INIFK  Initialize function key strings

Standard routine to initialise the ten function key strings to their power-up values. The one hundred and sixty bytes of data are copied to the FNKSTR buffer in the workspace area.

Entry: | none
Exit: | none
Modifies: | BC, DE, HL

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("inigrp", "", "BIOS-ROM", "0072H INIGRP Initialize VDP to Graphics Mode", "0072H INIGRP Initialize VDP to Graphics Mode",
        R"MAN(AREA: BIOS principal (MainROM)

0072H INIGRP Initialize VDP to Graphics Mode

AREA: Sub-ROM do MSX2

00DDH INIGRP

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("inimlt", "", "BIOS-ROM", "0075H INIMLT Initialize VDP to Multicolor Mode", "0075H INIMLT Initialize VDP to Multicolor Mode",
        R"MAN(AREA: BIOS principal (MainROM)

0075H INIMLT Initialize VDP to Multicolor Mode

AREA: Sub-ROM do MSX2

00E1H INIMLT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("iniplt", "", "BIOS-ROM", "0141H INIPLT", "Initializes the color-palette with standardized values. A copy of these colors is also kept in...",
        R"MAN(AREA: Sub-ROM do MSX2

0141H INIPLT

Initializes the color-palette with standardized values. A copy of these colors is also kept in VRAM memory.
Palettenumber | Color | Red | Green | Blue
0 (0H) | transparent | 0 | 0 | 0
1 (1H) | black | 0 | 0 | 0
2 (2H) | green | 1 | 6 | 1
3 (3H) | lightgreen | 3 | 7 | 3
4 (4H) | darkblue | 1 | 1 | 7
5 (5H) | lightblue | 2 | 3 | 7
6 (6H) | darkred | 5 | 1 | 1
7 (7H) | cyan | 2 | 6 | 7
8 (8H) | red | 7 | 1 | 1
9 (9H) | lightred | 7 | 3 | 3
10 (AH) | darkyellow | 6 | 6 | 1
11 (BH) | lightyellow | 6 | 6 | 4
12 (CH) | darkgreen | 1 | 4 | 1
13 (DH) | magenta | 6 | 2 | 5
14 (EH) | gray | 5 | 5 | 5
15 (FH) | white | 7 | 7 | 7
The VRAM address where those 32 data-bytes are stored depends on the current screen-mode: see RSTPLT (0145H)
Entry: | none
Exit: | none
Modifies: | EI ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("init32", "", "BIOS-ROM", "006FH INIT32 Initialize VDP to 32x24 Text Mode", "006FH INIT32 Initialize VDP to 32x24 Text Mode",
        R"MAN(AREA: BIOS principal (MainROM)

006FH INIT32 Initialize VDP to 32x24 Text Mode

AREA: Sub-ROM do MSX2

00D9H INIT32

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("initialise", "", "BIOS-ROM", "7FA4H Initialise", "Print Init Text.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7FA4H Initialise

Print Init Text.
Entry: | ?
Exit: | ?
Modifies: | ?
Note: | Do not use this call

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("initio", "", "BIOS-ROM", "003BH INITIO Initialize I/O devices", "Standard routine to initialise the PSG and the Centronics status port. PSG register 7 is first...",
        R"MAN(AREA: BIOS principal (MainROM)

003BH INITIO Initialize I/O devices

Standard routine to initialise the PSG and the Centronics status port. PSG register 7 is first set to 80H making PSG port B=Output and PSG port A=Input. PSG register 15 is set to CFH to initialise the joystick connector control hardware. PSG register 14 is then read and the keyboard mode bit placed in KANAMD, this has no relevance for UK machines.

Finally a value of FFH is output to the Centronics status port (I/O port 90H) to set the STROBE signal high. Control then drops into GICINI standard routine to complete initialisation.

Entry: | none
Exit: | none
Modifies: | AF, E, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("inittxt", "", "BIOS-ROM", "00D5H INITTXT", "00D5H INITTXT",
        R"MAN(AREA: Sub-ROM do MSX2

00D5H INITTXT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("initxt", "", "BIOS-ROM", "006CH INITXT Initialize VDP to 40x24 Text Mode", "006CH INITXT Initialize VDP to 40x24 Text Mode",
        R"MAN(AREA: BIOS principal (MainROM)

006CH INITXT Initialize VDP to 40x24 Text Mode

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("inlin", "", "BIOS-ROM", "00B1H INLIN  Get line from console (editor)", "00B1H INLIN Get line from console (editor)",
        R"MAN(AREA: BIOS principal (MainROM)

00B1H INLIN  Get line from console (editor)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("inquiry", "", "BIOS-ROM", "7F92H Inquiry", "REQUEST SENSE.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F92H Inquiry

REQUEST SENSE.
Entry: | A = LUN + SCSI Target ID
HL = transfer address
Exit: | A = SCSI STATUS
D = Target Status
E = Message
Modifies: | AF,BC,DE,HL
Note: | Returns 24h bytes of information,
      see ANSI 7.2.5

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("insflg", "", "Variaveis RAM", "FCA8 INSFLG  insert mode flag", "FCA8 INSFLG insert mode flag",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCA8 INSFLG  insert mode flag

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("insln0", "", "BIOS-ROM", "0125H INSLN0", "0125H INSLN0",
        R"MAN(AREA: Sub-ROM do MSX2

0125H INSLN0

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("inswork", "", "BIOS-ROM", "7FA7H InsWork", "Install Workspace.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7FA7H InsWork

Install Workspace.
Entry: | ?
Exit: | ?
Modifies: | ?
Note: | Do not use this call

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("intcnt", "", "Variaveis RAM", "FCA2 INTCNT", "FCA2 INTCNT",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCA2 INTCNT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("intflg", "", "Variaveis RAM", "FC9B INTFLG", "This variable is normally zero but set to 03H or 04H if the",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC9B INTFLG

This variable is normally zero but set to 03H or 04H if the
CTRL-STOP or STOP key are detected by the interrupt handler.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("intval", "", "Variaveis RAM", "FCA0 INTVAL", "FCA0 INTVAL",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCA0 INTVAL

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("iscntc", "", "BIOS-ROM", "00BAH ISCNTC Check CTRL-STOP key", "00BAH ISCNTC Check CTRL-STOP key",
        R"MAN(AREA: BIOS principal (MainROM)

00BAH ISCNTC Check CTRL-STOP key

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("isflio", "", "BIOS-ROM", "014AH ISFLIO Check for file I/O", "Standard routine to check whether the BASIC Interpreter is",
        R"MAN(AREA: BIOS principal (MainROM)

014AH ISFLIO Check for file I/O

Standard routine to check whether the BASIC Interpreter is
currently directing its input or output via an I/O buffer.

This is determined by examining PTRFIL. It is normally zero
but will contain a buffer FCB (File Control Block) address
while statements such as "PRINT#1", "INPUT#1", etc. are
being executed by the Interpreter.
Entry: | none
Exit: | Flag NZ if file I/O active
Modifies: | AF

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("jiffy", "", "Variaveis RAM", "FC9E JIFFY", "FC9E JIFFY",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC9E JIFFY

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("kanamd", "", "Variaveis RAM", "FCAD KANAMD", "This variable holds a keyboard mode on Japanese machines only.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCAD KANAMD

This variable holds a keyboard mode on Japanese machines only.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("kanast", "", "Variaveis RAM", "FCAC KANAST  russian key status", "This variable is used to hold the keyboard Kana lock status on",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCAC KANAST  russian key status

This variable is used to hold the keyboard Kana lock status on
Japanese machines and the DEAD key status on European machines.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("kbfmin", "", "Variaveis RAM", "F41E KBFMIN  DB \":\"", "This byte provides a dummy prefix to the tokenized text contained",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F41E KBFMIN  DB ":"

This byte provides a dummy prefix to the tokenized text contained
in KBUF. Its function is similar to that of ENDPRG
but is used for the situation where an error occurs within a
direct statement.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("keyint", "", "BIOS-ROM", "0038H KEYINT Interrupt handler, keyboard scan", "Standard routine to process Z80 interrupts, these are generated by VDP once every 20ms on a UK...",
        R"MAN(AREA: BIOS principal (MainROM)

0038H KEYINT Interrupt handler, keyboard scan

Standard routine to process Z80 interrupts, these are generated by VDP once every 20ms on a UK machine. The VDP status register is first read and bit 7 checked to ensure that this is a frame rate interrupt, if not routine terminates with no action.

Entry: | none
Exit: | none
Modifies: | EI

AREA: Sub-ROM do MSX2

0030H KEYINT (RST 38H)

This routine is equal to the MainROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("kilbuf", "", "BIOS-ROM", "0156H KILBUF Clear keyboard buffer", "Standard routine to clear the forty character type-ahead keyboard buffer",
        R"MAN(AREA: BIOS principal (MainROM)

0156H KILBUF Clear keyboard buffer

Standard routine to clear the forty character type-ahead keyboard buffer
KEYBUF.

There are two pointers into this buffer, PUTPNT where the interrupt handler
places characters and GETPNT where application programs fetch them from.
As the number of characters in the buffer is indicated by the difference
between these two pointers KEYBUF is emptied simply by making them both
equal.
Entry: | none
Exit: | none
Modifies: | HL

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("knjprt", "", "BIOS-ROM", "01BDH KNJPRT Print Kanji character", "Prints a Kanji character onto the graphic screen (5-8). On international MSX-computers this...",
        R"MAN(AREA: Sub-ROM do MSX2

01BDH KNJPRT Print Kanji character

Prints a Kanji character onto the graphic screen (5-8). On international MSX-computers this routine does nothing.
Entry: | BC = JIS kanji code (first standard on MSX2, first and second standard on MSX2+)
A = display mode

      The display mode has following meanings in the same way with the PUT KANJI command of BASIC.
      0    Displays by 16x16 dots
      1    Displays even numbered dots
      2    Displays odd numbered dots
[GRPACX(FCB7H)] X coordinate
[GRPACY(FCB9H)] Y coordinate
Exit: | ?
Modifies: | AF

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ldirmv", "", "BIOS-ROM", "0059H LDIRMV Copy block of memory from VRAM", "Standard routine to copy a block from VRAM into memory.",
        R"MAN(AREA: BIOS principal (MainROM)

0059H LDIRMV Copy block of memory from VRAM

Standard routine to copy a block from VRAM into memory.
Entry: | BC = length
DE = start RAM address
HL = start VRAM address
Exit: | none
Modifies: | AF, BC, DE, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ldirvm", "", "BIOS-ROM", "005CH LDIRVM Copy block to VRAM from memory", "Standard routine to copy a block from memory into VRAM.",
        R"MAN(AREA: BIOS principal (MainROM)

005CH LDIRVM Copy block to VRAM from memory

Standard routine to copy a block from memory into VRAM.
Entry: | BC = length
DE = start VRAM address
HL = start RAM address
Exit: | none
Modifies: | AF, BC, DE, HL, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("leftc", "", "BIOS-ROM", "00FCH LEFTC  Move current pixel physical address left", "00FCH LEFTC Move current pixel physical address left",
        R"MAN(AREA: BIOS principal (MainROM)

00FCH LEFTC  Move current pixel physical address left

AREA: Sub-ROM do MSX2

00ADH LEFTC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("lftq", "", "BIOS-ROM", "00F6H LFTQ   Space left if music queue", "00F6H LFTQ Space left if music queue",
        R"MAN(AREA: BIOS principal (MainROM)

00F6H LFTQ   Space left if music queue

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("linl32", "", "Variaveis RAM", "F3AF LINL32  Screen width per line in SCREEN 1", "This variable contains the 32x24 Text Mode screen width. Its value is set at power-up and",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3AF LINL32  Screen width per line in SCREEN 1

This variable contains the 32x24 Text Mode screen width. Its value is set at power-up and
thereafter only altered by the "WIDTH" statement.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("linl40", "", "Variaveis RAM", "F3AE LINL40  Screen width per line in SCREEN 0", "This variable contains the 40x24 Text Mode screen width. Its value is set at power-up and",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3AE LINL40  Screen width per line in SCREEN 0

This variable contains the 40x24 Text Mode screen width. Its value is set at power-up and
thereafter only altered by the "WIDTH" statement.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("linlen", "", "Variaveis RAM", "F3B0 LINLEN  Current screen width per line", "This variable contains the current text mode screen width. Its value is set from LINL40 or",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3B0 LINLEN  Current screen width per line

This variable contains the current text mode screen width. Its value is set from LINL40 or
LINL32 whenever the VDP is initialized to text mode via the INITXT or INIT32 standard routines.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("list output", "", "BDOS", "05 List Output", "05 List Output",
        R"MAN(AREA: BDOS (MSX-DOS)

05 List Output

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("logical drive assignment", "", "BDOS", "6A Logical drive assignment", "6A Logical drive assignment",
        R"MAN(AREA: BDOS (MSX-DOS)

6A Logical drive assignment

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("low", "", "Variaveis RAM", "F406 LOW", "DB LOW01,HIGH01 Width of LOW & HIGH wich represents bit 0",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F406 LOW
     DB LOW01,HIGH01    Width of LOW & HIGH wich represents bit 0
F408 HIGH
     DB LOW11,HIGH11    Width of LOW & HIGH wich represents bit 1
F40A HEADER
     DB HEDLEN*2/256    Header bit for the short header

These five variables contain the current cassette parameters.
Their values are set to 1200 baud at power-up and theirafter
only altered by the "CSAVE" and "SCREEN" statements.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("lowlim", "", "Variaveis RAM", "FCA4 LOWLIM", "FCA4 LOWLIM",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCA4 LOWLIM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("lptout", "", "BIOS-ROM", "00A5H LPTOUT Line printer character output", "00A5H LPTOUT Line printer character output",
        R"MAN(AREA: BIOS principal (MainROM)

00A5H LPTOUT Line printer character output

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("lptpos", "", "Variaveis RAM", "F415 LPTPOS  Position of printer head", "This variable is used by the \"LPRINT\" statement handler to hold",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F415 LPTPOS  Position of printer head

This variable is used by the "LPRINT" statement handler to hold
the current position of the printer head.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("lptstt", "", "BIOS-ROM", "00A8H LPTSTT Line printer status test", "00A8H LPTSTT Line printer status test",
        R"MAN(AREA: BIOS principal (MainROM)

00A8H LPTSTT Line printer status test

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("mapxy", "", "BIOS-ROM", "0111H MAPXY  Map graphics coordinates to physical address", "0111H MAPXY Map graphics coordinates to physical address",
        R"MAN(AREA: BIOS principal (MainROM)

0111H MAPXY  Map graphics coordinates to physical address

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("mapxyc", "", "BIOS-ROM", "0091H MAPXYC", "0091H MAPXYC",
        R"MAN(AREA: Sub-ROM do MSX2

0091H MAPXYC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("maxupd", "", "Variaveis RAM", "F3EC MAXUPD  JMP 0000 (used by CIRCLE statement)", "The address bytes are filled in by the \"LINE\" statement handler for form a Z80 JP",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3EC MAXUPD  JMP 0000 (used by CIRCLE statement)

The address bytes are filled in by the "LINE" statement handler for form a Z80 JP
to the RIGHTC, LEFTC, UPC or DOWNC standard routines.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("mclflg", "", "Variaveis RAM", "F958 MCLFLG  play / draw", "This variable is zero if the macro language parser is being used",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F958 MCLFLG  play / draw

This variable is zero if the macro language parser is being used
by the "DRAW" statement handler and non-zero if it is used by "PLAY".

                 for play
F959 QUETAB: DEFB 00H   ;AQ Put position
F95A         DEFB 00H   ;AQ Get position
F95B         DEFB 00H   ;AQ Putback flag
F95C         DEFB 7FH   ;AQ Size
F95D         DEFW F975H ;AQ Address

F95F         DEFB 00H   ;BQ Put position
F960         DEFB 00H   ;BQ Get position
F961         DEFB 00H   ;BQ Putback flag
F962         DEFB 7FH   ;BQ Size
F963         DEFW F9F5H ;BQ Address

F965         DEFB 00H   ;CQ Put position
F966         DEFB 00H   ;CQ Get position
F967         DEFB 00H   ;CQ Putback flag
F968         DEFB 7FH   ;CQ Size
F969         DEFW FA75H ;CQ Address

F96B         DEFB 00H   ;RQ Put position
F96C         DEFB 00H   ;RQ Get position
F96D         DEFB 00H   ;RQ Putback flag
F96E         DEFB 00H   ;RQ Size
F96F         DEFW 0000H ;RQ Address

These twenty-four variables form the control blocks for the three music-queues
(VOICAQ, VOICBQ and VOICCQ) and the RS232 queue.
The three music control blocks are initialized by the GICINI standard routine and
thereafter maintained by the interrupt handler and the PUTQ standard routine.
The RS232 control block is unused in the current MSX ROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("minupd", "", "Variaveis RAM", "F3EF MINUPD  JMP 0000 (used by CIRCLE statement)", "The address bytes are filled in by the \"LINE\" statement handler for form a Z80 JP",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3EF MINUPD  JMP 0000 (used by CIRCLE statement)

The address bytes are filled in by the "LINE" statement handler for form a Z80 JP
to the RIGHTC, LEFTC, UPC or DOWNC standard routines.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("mode", "", "Variaveis RAM", "FAFC MODE    The mode switch for romaji-kana conversion, and VRAM", "Bit 7: 0-hiragana, 1-katakana (romaji-kana conversion)",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FAFC MODE    The mode switch for romaji-kana conversion, and VRAM size.

  Bit 7: 0-hiragana, 1-katakana (romaji-kana conversion)

  Bit 6: Presence of level 2 kanji rom: 0-yes, 1-no (2+)

  Bit 5: 0-screen 10, 1-screen 11 (2+)

  Bit 4: Clipping: 0-doesn't, 1- does (2+)

  Bit 3: Masks the SCREEN 0-3 VRAM addresses: 0-doesn't, 1-does

  Bit 2,1: VRAM capacity: 0,0-16K 0,1-64K 1,1-128K

  Bit 0: Romaji-kana conversion: 0-doesn't, 1-does

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("modesel", "", "BIOS-ROM", "7F9BH ModeSel", "MODE SELECT.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F9BH ModeSel

MODE SELECT.
Entry: | A = LUN + SCSI Target ID
B = Parameter List length
HL = transfer address
Exit: | A = SCSI Target ID
D = Target Status
E = Message
Modifies: | AF,BC,HL,IX
Note: | See ANSI 7.2.8

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("modesense", "", "BIOS-ROM", "7F98H ModeSense", "MODE SENSE.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F98H ModeSense

MODE SENSE.
Entry: | A = LUN + SCSI Target ID
HL = transfer address
B = PPCCCCCC
                 ||++++++-- Page Code
                 ++-------- Page Control Field
Exit: | A = SCSI STATUS
D = Target Status
E = Message
Modifies: | AF,BC,DE,HL
Note: | See ANSI 7.2.10

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("move file handle", "", "BDOS", "54 Move file handle", "54 Move file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

54 Move file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("move file handle pointer", "", "BDOS", "4A Move file handle pointer", "4A Move file handle pointer",
        R"MAN(AREA: BDOS (MSX-DOS)

4A Move file handle pointer

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("move file or subdirectory", "", "BDOS", "4F Move file or subdirectory", "4F Move file or subdirectory",
        R"MAN(AREA: BDOS (MSX-DOS)

4F Move file or subdirectory

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("msx-bios: extbio", "", "BIOS-ROM", "Expanded BIOS hook was designed to expand BIOS (then why it has a so", "strange name?? X-D) in order to manage new devices. So it is used for",
        R"MAN(AREA: EXTBIO

Expanded BIOS hook was designed to expand BIOS (then why it has a so
strange name?? X-D) in order to manage new devices. So it is used for
example by RS-232 and DOS 2 mapper support routines. Nowadays, also TSR
programs (MemmMan, NestorAcentos...) use it, because it provides a fixed
memory location to CALL in page 3.

How it works? You put a device ID number in register D, a function
number in register E, and registers AF, BC, HL depending of the calling
routine (you can't use alternative registers, nor indexed ones). DE is
always preserved when returning; AF, BC, DE depends on the calling
routine; alternative and indexed are corrupted.

To use the extended BIOS, the program should test "HOKVLD" flag at FB20h
in  page-3. If  bit-0 (LSB)  is 0,  there is  no extended BIOS nor the mapper
support. Otherwise,  "EXTBIO" entry (see below) has been set up and it can be
called  with various  parameters. Note  that this test is unnecessary for the
applications which  are based on MSX-DOS (such as the program which is loaded
from the disk), and the program may proceed to the next step.

Next,  the  program  sets  the  device  number of  the extended  BIOS in
register D,  the function  number in  register E,  and required parameters in
other  registers, and  then calls  "EXTBIO" at FFCAh in page-3. In this case,
the stack  pointer must  be in  page-3. If there is the extended BIOS for the
specified  device number,  the contents  of the  registers AF,  BC and HL are
modified according  to the  function; otherwise, they are preserved. Register
DE  is  always  preserved.  Note  that  in  any  cases  the  contents  of the
alternative registers (AF', BC', DE' and HL') and the index registers (IX and
IY) are corrupted.

Device number (register D) | Device
00 | RS-232 BIOS
01 | RS-232 BIOS
04 | Memory mapper-support of MSX-DOS 2
0A | MSX-Audio
10 | Kanji Driver
22 | NestorAcentos, NestorCDPlayer and all TSRs which name is Nestor*.*
34 | MWMPLAY by Ramones
4D | MemMan
F0 | MGSDRV

It is recommended that once selected, you use always this number in your
programs, modifying register E for every program. For example
DE=#2200 for NestorAcentos, and DE=#2201 for NestorCDPlayer.

Now you must patch hook #FFCA, of course saving previous value. The
new jump routine must do this:

Check D. If it does not match with your ID number, jump to the old
hook without any register modified.
If it matches, check register E for required function, do anything and
return preserving DE.

This jump routine should be placed in page 3 if the program is a TSR.
How to reserve memory on page 3 is another question.

See also:

RAM-Variables (EXTBIO, HOKVLD)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("msx-je", "", "BIOS-ROM", "Section 5 MSX-JE", "Chapter 1 What is MSX-JE?",
        R"MAN(AREA: MSX-JE

Section 5 MSX-JE
Chapter 1 What is MSX-JE?
Chapter 2 How to Boot MSX-JE
Chapter 3 How to Use MSX-JE
Chapter 4 Virtual Terminal Interface
Chapter 5 Cautions upon Using MSX-JE

This page contains a document concerning
how an MSX with a kanji input system installed accepts inputs
and turns them into fonts (Japanese language utilizes multiple
kinds of character sets in order to scribe itself) you prefer
to write.

This relatively large document was
originally authored for Japanese readers (which can naturally
understand Japanese language) who want to introduce kanji input
sequences integrated in their software.

However, this part is inevitable for
development of true emulators.
Up until now, no one has emulated renbunsetsu henkan (sequential
bunsetsu conversion). This info might help such developers too.

And this document describes procedures
and effects a boot of MSX-JE system, therefore giving you clues
to successfully reverse-engineer Japanese software of recent release,
which occasionally use CALL KANJI statement or similar means to
boot MSX-JE and enter kanji input mode.

Definitions

* Zenkaku and hankaku

"zen" means
"whole", "han" means "half", while
"kaku" means for "square" in Japanese. In
MSX or in any word processing related with computers, hankaku
is an adjective to describe 1-byte characters (English alphabets,
numbers, western punctuation, unaccented katakanas, Japanese punctuation,
Japanese accents; MSX supports unaccented hiraganas, unlike PC)
while zenkaku, "2-byte" characters, include all hiraganas
and katakanas, both accented and unaccented, English alphabets,
special pictograms like large circles and measurement terms, Greek
and Russian fonts, and thousands of kanjis.

* Bunsetsu and candidate

"bun" stands
for "paragraph" or "sentence", while "setsu"
is a segment. In word-processing sense, this indicates one specific
domain in one input which is waiting to be converted. MSX guesses
how many characters in one input string is best to convert into
any kanji string MSX knows. MSX treats that part of a input as
1 "bunsetsu", and thinks up several "candidates"
or "candidates" it thinks best to replace the kanas
in bunsetsu, and show them to you, and let you to select the one
he or she wanted to write. You can manually adjust length or location
of bunsetsu coverage (i.e. from what character a bunsetsu begins
and at what character it ends; referred to as candidate block
in this document) if you want to, through manipulation with keyboard.
When you finally get the kanji+kanas proposal from MSX which satisfy
you, you press RETURN, and MSX-JE sends the converted bunsetsu
to MSX as actual finalized data. Then MSX-JE guesses where next
bunsetsu it must convert correspond to what part of input.

Section 5 MSX-JE

Here we describe MSX Standard Japanese
Input Front End Processor, commonly called MSX-JE.

I would like to mention that I referred
to VJE-80A Specifications; 2nd Edition (from ASCII Corp.) while
authorizing these resources.

Chapter 1 What is MSX-JE?

"Japanese input front end processor"
includes means to input Japanese with word processors, how to
do so, and programs which control such procedures. Before coming
of MSX-JE as standard, every word processors had their own Japanese
input front end processors. This was fine in old days, but in
these cases, let's say, when you want your modem connection software
to support Japanese input, you must make your own Japanese input
front end processor by yourself. However, this is a tremendous
amount of work. Moreover, this costs users, who have to learn
how to use different kind of Japanese front end processor each
time they acquire another software. It's naturally convenient
if you could use same input system with your modem connection
software, and with your word processor. Therefore, a standard
was established concerning means to input Japanese, and how to
call them from programs. This is the, "MSX Standard Japanese
Input Front End Processor" or "MSX-JE" (commonly
called MJE).

For example, once you have registered
a term into internal SRAM while using Bunsho Sakuzaemon, a word
processor software on your Sony HB-F1XDJ, you can use that term
in occasions you conduct kanji input with kanji BASIC (during
CALL KANJI status), too. And also usable when you are using modem
through Canon's modem connection cartridge VM-300. Kanji conversion
method is standardized as one, too.

Chapter 2 How to Boot MSX-JE

Use expanded BIOS call, as usual,
in order to use MSX-JE.

Firstly, you need to check whether
you are able to use expanded BIOS call or not. If lowest bit in
address FB20H of the work area is standing, and value of FFCAH
is other than C9H, expanded BIOS call is possible. If these conditions
are not met, it means there are no machines which rely on expanded
BIOS calls; which means that MSX-JE does not exist, accordingly.

If above conditions are met, then
input 10H to D register, 0 to E register, header address of the
work area to HL register, slot no. which the work area is present
into B register. Then call address FFCAH. Be noted that, work
area should be placed on page 2 or 3, and stack should be placed
on page 3. Size of the work area is 64 bytes.

If Japanese front end processors exist,
then value of HL will be incremented by 4, and following table
is originated. If there are 2 or more Japanese front end processors
connected, then equal number of tables are originated.

Former HL =>

Capability vector

Slot no.is not compatible.

If bit 1 is 0, then virtual terminal
interface is exists. If 1, it does not exist.

If bit 2 is 0, then dictionary interface
exists. If 1, it does not exist.

If bit 3 is 0, then dictionary registration
and deletion function is available. If 1, they are not available.

Bits 4 to 7 are for future expansion,
and they are all fixed to 1 this point.

Check statuses of these bits to check
whether functions you want to use are there when you use MSX-JE.
For example, a capability vector of VJE-80 (ASCII-made MSX-JE)
is F8H (dictionary cannot be changed), while VJE-80A (new version
of ASCII-made MSX-JE) is F0H (all functions described above are
available).

Let your software to prompt a menu
and let users to select which MSX-JE they want to use if there
are 2 or more MSX-JEs. In some machines, MSX-JE embedded in the
machines always boots, thus negating the use of external MSX-JEs.
For example, though you insert MSX-WRITE cartridge into a Panasonic
FS-4600F, internal MSX-JE is always chosen to be booted. :(

Chapter 3 How to Use MSX-JE

To use MSX-JE, input value to registers,
and insterslot-call the aforementioned routines.

However, you need to use rather uncommon
procedure to substitute character strings. I would like to explain
firstly.

5.3.1 Format of Character Strings

Format of character strings are;

[number of characters] + "character
string".
Character strings are in SHIFT-JIS format. For instance, 97H+""("red
balloon"). Then, specify its header address to BC or DE register.
Zenkaku characters are always regarded as 2-bytes even if they
are not kanjis. Below are several examples.

08H+""
("red balloon" in kanjis and kanas, 4 zenkaku characters
overall)

0DH+""("spaghetti
with fermented beans", 2 kanjis + 9 hankaku katakanas)

7H+""
("club-house style sandwiches", 7 hankaku katakanas)

0EH+""
("red balloon" in 7 zenkaku hiraganas)

07H+"TOUKYOU"
(7 hankaku characters)

1-byte accents (""
and "")
are treated as single characters. You cannot treat hankaku hiraganas
(which are supported by MSX but not with PCs) and kanas which
are not supported by MSX in non-kanji modes - like "",
"",
"",
"" or ""
- as hankaku characters and use them. Length of a character string
is 64 bytes at maximum.

Note on Japanese accents: Japanese
hiraganas and katakanas require 2 kind of "accents"
to be added on upper-right of kana characters. They are essential
for recognition and pronunciation, and should not be omitted in
any cases.

In non-kanji mode MSX, there are only
hiraganas and katakanas without accents, and these 2 accents (DEH
and DFH in ASCII) available, therefore you are required to represent
one accented kana using 2 characters. However, 2-byte characters
support both kanas with and without accents.

5.3.2 MSX-JE Function Calls

Inquiry (Function 01H)

Input: 1 to A

Output: MAX to HL

MIN to DE

MIN2 to BC

This returns necessary size of work
area to enable execution of MSX-JE. MAX is a maximum size of work
area. Even if you grant any more than MAX to MSX-JE, MSX-JE uses
only this size. Least necessary size goes into DE, while least
necessary size in case learning option is chosen goes into BC.
Value is counted by unit of bytes. For example, in case of VJE-80,
MAX=MIN2=3328 bytes and MIN=2304. In case of VJE-80A, MAX=MIN2=520
and MIN=512. In case of SONY JFEP, MAX=MIN=MIN2=2560.

invoke (02H)

Input: 2 to A

Header address of BW to HL

Length of BW to DE

Output: None

This command allocates work area for
MSX-JE. You should call this whenever you boot MSX-JE. BW stands
for work area of MSX-JE. You should always allocate it in page
2 or 3. It seems that, several restrictions occur if you place
ASCII VJE-80 in page 2 and use it, but I do not think it's worth
being afraid of. You must check necessary length of work area
using function 1 (Inquiry) before calling this function. As a
point of fact, since MSX-JE never needs more than 2560 bytes as
its least necessary work area to boot, so it's OK if you allocate
2560 bytes.

release (03H)

Input: 3 to A

Header address of BW to HL

Output: None

This frees work area allocated for
MSX-JE. You should call this whenever you quit any program which
use MSX-JE. If you do not call this, learning dictionary might
be damaged.

han_zen (40H)

Input: 40H to A

Address of BW to HL

Address of source buffer (hankaku
character string) to DE

Address of destination (zenkaku character
string) to BC

Output: If A is 0, it means conversion
was successfully done.

If A is other than 0 then conversion
was a failure. The value of A then stands for position of the
character that prevented the conversion, in buffer.

This converts a hankaku character
string to a zenkaku character string. Both accents will be converted
into independent characters (though rarely used, 2-byte character
table does include accents as characters by their own).

Example: Input 05H+""
to DE

Output 0AH+""
to BC

zen_han (41H)

Input: 41H to A

Address of BW to HL

Address of source buffer (zenkaku
character string) to DE

Address of destination (hankaku character
string) to BC

Output: If A is 0, it means conversion
was successfully done.

IF A is other than 0 then conversion
was a failure. The value of A then stands for position of the
character that prevented the conversion, in buffer.

This converts a zenkaku character
string to a hankaku character string. Both accents, which were
integrated as a single kana to a certain kana when they were zenkau,
will be converted into independent character.

Example: Input 058+""
to DE

Output 05H+""
to BC

han_kata (42H)

Input: 42H to A

Address of BW to HL

Address of source buffer (hankaku
character string) to DE

Address of destination (katakana character
string) to BC

Output: If A is 0, it means conversion
was successfully done.

IF A is other than 0 then conversion
was a failure. The value of A then stands for position of the
character that prevented the conversion, in buffer.

This converts a hankaku character
string into zenkaku katakana string. Input is expected to be katakanas
or romaji string.

Example: Input 07H+"AKATUKA"
to DE

Output 08H+""
to BC

han_hira (43H)

Input: 43H to A

Address of BW to HL

Address of source buffer (hankaku
character string) to DE

Address of destination (hiragana character
string) to BC

Output: If A is 0, it means conversion
was successfully done.

IF A is other than 0 then conversion
was a failure. The value of A then stands for position of the
character that prevented the conversion, in buffer.

This converts a hankaku character
string into zenkaku hiragana string. Input is expected to be in
katakanas (MSX 1-byte hiraganas are not accepted) or romaji string.

Example: Input 0AH+"KAIRAKUENN"
to DE

Output 08H+""
to BC

kata_hira (44H)

Input: 44H to A

Address of BW to HL

Address of source buffer (katakana
character string) to DE

Address of destination (hiragana character
string) to BC

Output: None.

This converts a zenkaku katakana string
into zenkaku hiragana string. If input string included characters
other than zenkaku katakanas, that character will not be converted.
Conversion result is not certified if there were hankaku characters
in input string. :(

Example: Input 0AH+""
to DE

Output 0AH+""
to BC

kata_hira (45H)

Input: 45H to A

Address of BW to HL

Address of source buffer (hiragana
character string) to DE

Address of destination (katakana character
string) to BC

Output: None.

Same as kata_hira, except in aspect
that this converts a zenkaku hiragana string into zenkaku katakana
string.

Example: Input 0AH+""
to DE

Output 0AH+""
to BC

open_dic (46H)

Users are not expected to call this.

henkan1 (47H)

Input: 47H to A

Address of BW to HL

Address of read data (zenkaku katakana
string)

Output: If A=0, it means there is
no suitable candidate.

If A is other than 0 then it is a
number of candidates for header bunsetsu

This converts a zenkaku katakana string
into a kanji+kana passage. Acquire the result through ji_koho
or other means.

Example: Input 10H+""
to DE

Output number of candidates for ""
to A

ji_koho (48H)

Input: 48H to A

Address of BW to HL

Candidate buffer to DE

Proceeding bunsetsu buffer to BC

Output: If A=0, it means there is
no suitable candidate.

If A is than 0 then it's a lot number
within a candidate block

This gets next candidate (jikoho)
for header bunsetsu. You must read with henkan1 to set the data
before you call this function.

Example: Suppose you called this function
after trying to convert an input ""
using henkan1.

Output: Lot number within a candidate
block to A

04H+""
to DE

06H+""
to BC

zen_koho (49H)

Input: 49H to A

Address of BW to HL

Candidate buffer to DE

Proceeding bunsetsu buffer to BC

Output: If A=0, it means there is
no suitable candidate.

If A is than 0 then it's a lot number
within a candidate block

This gets prior candidate (zenkoho)
for header bunsetsu. You must read with henkan1 to set the data
before you call this function.

Example: Suppose you tried to convert
an input ""
using henkan1 prior.

Output: After execution of 1st ji_koho

04H+""
to DE

02H+""
to BC

After execution of 2nd ji_koho

04H+""
to DE

02H+""
to BC

After execution of zen_koho

Lot number within a candidate block
to A

04H+""
to DE

02H+""
to BC

ji_block (4AH)

Input: 4AH to A

Address of BW to HL

Output: If A is 0, it means there
is no suitable block group.

If A is other than 0 then it's a number
of candidates.

This sends a group of candidate block
to next level, by retracting length of header bunsetsu, regarding
sense of terms in different way and other means, and remakes group
of candidate blocks. You must read with henkan1 to set the data
before you call this function. Acquire the result through ji_koho
or other means.

Example: Suppose you tried to convert
an input ""
using henkan1 prior.

Output: After execution of 1st ji_koho

06H+""
to DE

08H+""
to BC

Execute ji_block, then execute a 2nd
ji_koho

04H+""
to DE

08H+""
to BC

zen_block (4BH)

Input: 4BH to A

Address of BW to HL

Output: Output: If A is 0, it means
there is no suitable block group.

If A is other than 0 then it's a number
of candidates.

This sends a group of candidate back
to prior level, and remakes group of candidate blocks. You must
read with henkan1 to set the data before you call this function.
You are not requested to call ji_block before execution of this
function, however.

Example: Suppose you tried to convert
an input ""
using henkan1 prior.

Output: After execution of 1st ji_koho

06H+""
to DE

08H+""
to BC

Execute ji_block, then execute a 2nd
ji_koho

04H+""
to DE

08H+""
to BC

Then execute zen_block, then execute
ji_koho

06H+""
to DE

08H+""
to BC

kakutei1 (4CH)

Input: 4CH to A

Address of BW to HL

Lot number within a candidate block
to E

Address of conversion result container
buffer to BC

Output: Conversion result will be
contained in the buffer stated by BC.

This finalizes ("kakutei")
the candidate specified by E register, as the result of conversion.
Result enters into address stated by BC. You must read with henkan1
to set the data before you call this function. You are not requested
to call ji_block or similar functions before execution of this
function.

Example: Suppose you tried to convert
an input ""
using henkan1 prior.

Input: Lot number within a candidate
block to E

Output: 0AH+""
to E

kakutei2 (4DH)

Input: 4DH to A

Address of BW to HL

Lot number within a candidate block
to E

Address of conversion result container
buffer to BC

Output: Pronounced length of header
bunsetsu (byte length)

Conversion result will be contained
in the buffer stated by BC.

This finalizes ("kakutei")
the candidate specified by E register. Pronounced length of the
finalized header bunsetsu enters A, while conversion result enters
into address stated by BC. A "pronounced length" is
a length of conversion result when it is read in all kanas (no
kanjis). You must read with henkan1 to set the data before you
call this function. You are not requested to call ji_block or
similar functions before execution of this function.

Example: Suppose you tried to convert
an input ""
using henkan1 prior.

Input: Lot number within a candidate
block to E

Pronounced length of header bunsetsu
(08H) to A

0AH+""
to E

close_dic (4EH)

Users are not expected to call this
function.

touroku (4FH)

Input: 4FH to A

Address of BW to HL

Address of pronunciation buffer to
DE

Address of term registration buffer
to BC

Output: If A=0, it means registration
was successful.

A=1 means registration failed, because
there was no empty space.

A=2 means registration failed, because
there were too many homonyms.

A=4 means registration failed because
pronunciation data was improper.

A=8 means registration failed because
term data was improper.

A=16 means registration failed because
grammar (sense) was improper.

This registers ("touroku")
a term into the user dictionary.

Formats for pronunciation data and
term data are as follows.

[number of characters]+"[pronunciation
data (zenkaku katakanas, max. 64 bytes)]" to DE

[number of characters]+"[term
data (zenkaku characters, max. 64 bytes)]"+[sense code]"

Pronunciation data should be in zenkaku
katakanas, 2 bytes or more, 64 bytes or less (which means between
1 to 32 characters long), and should not include characters "",
"",
"",
"",
"" or "".
Term data should be in SHIFT-JIS codes, with characters restriction
between 2 to 64 bytes, like pronunciation data. You can register
up to 255 homonyms (there are quite many homonyms in Japanese
language, which are spelled with different kanjis). Choose a sense
out of following 5.

1 for nouns (like natto (fermented
beans)), 2 for surnames (like Tokugawa), 3 for first names (like
Mitsukuni), 4 for location names (like Mito, a city which houses
both natto and Mitsukuni Tokugawa;), and 5 for verbs which require
"suru" suffix (like shipansuru ("to judge")
or kagakusuru ("let scientific mind work"))

If your MSX-JE does not support term
registration (VJE-80 from ASCII for example), then this function
returns FFH to A.

Example: You registered term "",
which is pronounced ""
and has a sense of surname.

Input: 0AH+""
to DE

06H+""+02H
to BC

Output: 0 to A (successful registration)

sakujo (50H)

Input: 50H to A

Address of BW to HL

Address of pronunciation buffer to
DE

Address of term registration buffer
to BC

Output: If A=0, it means deletion
was successful.

A=1 means deletion failed because
it was not found in dictionary.

A=2 means deletion failed because
pronunciation data was improper.

A=8 means deletion failed because
term data was improper.

A=16 means deletion failed because
sense was improper.

This deletes ("sakujo")
a term from the user dictionary. Formats of data are same as touroku.
If your MSX-JE does not support this function, then it returns
FFH to A.

These so far are function calls defined
to MSX-JE. Normally, these functions are enough, but since VJE-80A
defines several other function calls as expansion, so we describe
them too.

henkan2 (58H)

Input: 58H to A

Address of BW to HL

Address of zenkaku katakana string
to DE

Output: Number of candidates to A

Execution is exactly identical to
henkan1. VJE-80 returns presence/absence of candidates (1: at
least one candidates is available).

number of candidates (59H)

Input: 59H to A

Address of BW to HL

Output: Number of candidates for current
candidate block

This returns number of candidates
available for candidate block currently under conversion.

henkan3 (5AH)

Input: 5AH to A

Address of BW to HL

Output: Number of candidates for header
bunsetsu of proceeding bunsetsu

This repeats the conversion over the
part which was regarded as proceeding bunsetsu.

Example: Suppose you tried to convert
an input ""
using henkan1 prior.

After execution of 1st ji_koho

04H+""
to DE

06H+""
to BC

Execute henkan3, then execute ji_koho

04H+""
to DE

02H+""
to BC

hiragana (60H)

Input: 60H to A

Address of BW to HL

Candidate buffer to DE

Proceeding bunsetsu buffer to BC

Output: A=0 means successful completion.

A=FFH means operation quitted abnormally
(error).

This converts the header bunsetsu
which was most lately converted into zenkaku hiragana string of
equivalent pronunciation. You must acquire candidates through
functions like ji_koho or zen_koho before using this function.

Example: Suppose you tried to convert
an input ""
using henkan1 prior.

After execution of 1st ji_koho

04H+""
to DE

06H+""
to BC

Then execute hiragana

06H+""
to DE

06H+""
to BC

katakana (61H)

Input: 61H to A

Address of BW to HL

Candidate buffer to DE

Proceeding bunsetsu buffer to BC

Output: A=0 means successful completion.

A=FFH means operation quitted abnormally
(error).

This converts the header bunsetsu
which was most lately converted into zenkaku katakana string of
equivalent pronunciation. You must acquire candidates through
functions like ji_koho or zen_koho before using this function.
All others are same with hiragana function

Descriptions of expanded function
calls ends here. Note, these expanded function calls return FFH
to A if your MSX-JE does not support them. For instance, since
hiragana and katakana functions are not supported by VJE-80, so
they will return FFH to A if you execute them on VJE-80.

For your notice, if in future the
functions not defined yet will be added, they will be assigned
to functions 62H and later, by youngest order.

Chapter 4 Virtual Terminal Interface

We described about basic usage of
MSX-JE, but they are for use of word processor software and like
when they want to convert kanjis. They are still too much a workload
when a simple user want use a little bit of MSX-JE functions in
their programs. Therefore, MSX-JE supports a facility called virtual
terminal interface, which allows you to use MSX-JE functions with
relative ease. In this, a virtual terminal do all kanji conversion
processes, and what program should do is to display character
strings which the virtual terminal return. When virtual terminal
is used, work flow is as follows.

1) When a program wants to input Japanese,
it opens a window in a screen, and call virtual terminal.

2) User looks at it (the window in
this case), and input, say, YUUGURE.

3) Virtual terminal converts that
to ,
and returns the converted one to program.

4) The program displays
on the window it opened, and gives the responsibility of execution
back to virtual terminal.

5) User looks at ,
and press conversion key.

6) Virtual terminal, knowing that
conversion key was pressed, converts
to ,
and returns it to the program.

7) Program displays
in window, and gives the execution responsibility back to virtual
terminal.

8) User looks at it, and press finalizing
key.

9) Virtual terminal, knowing that
finalizing key was pressed, returns the notion that result was
finalized as
to the program.

10)
is input.

Kanji BASIC supported by MSX2+ and
later does this way, so if you cannot understand above, try booting
kanji BASIC or so and convert kanjis. A program (BASIC interpreter
in this case) calls virtual terminal when CTRT+SPACE are pressed,
then window opens in lowest line, and tells you that MSX entered
kanji conversion mode. When you input the characters, virtual
terminal converts, while program (BASIC interpreter) displays
them in the window. If you press finalizing key (RETURN), then
character string present in window then are assumed as your input.
Finally, when you press CTRL+SPACE, program quits out of kanji
input mode.

If you try to use MSX-JE functions
directly, then your program should process everything including
whether conversion or finalizing keys are pressed or not. On the
other hand, if you use virtual terminal, if you make a window
and call the terminal, then what you should do is to display ongoing
processes and final result, so kanji input will cost you far less.
As a price to pay, window size has restriction, and keys to finalize
or to convert are fixed, since virtual terminal processes such
steps.

I recommend you to use MSX-JE when
you make your own word processor, and virtual terminal in cases
you want to prompt some kanji input in your modem software or
so.

5.4.2 How to Boot Virtual Terminal

Boot process for virtual terminal
is exactly same with that for MSX-JE. Call address FFCAH to check
interslot call address for MSX-JE (virtual terminal), call function
1 (see 5.3.2, MSX-JE Function Calls) to check necessary size of
work area, and call function 2 to boot MSX-JE (virtual terminal).
When you have finished using virtual terminal facility, call function
3 to quit virtual terminal.

5.4.3 Character Strings Outputted
by Virtual Terminal

As explained in 5.4.1,
a virtual terminal does not display character strings it converted,
but prefers to return them to the program and let it to display
them. This is, to allow program to choose position of the window,
and to enable use of virtual terminal in any screen mode. You
can use virtual terminal in interlaced screen 6 or in screen 2
because of this feature. While window is mostly located on lowest
line, but you are free to locate it, say, in middle of the screen.

Character string data are in a little
strange format, for occasions like when a virtual terminal wants
cursor to vanish. The format is called STB format. Programs are
supposed to display character strings of STB format to the window.

Format of STB

1: SHIFT-JIS character string

Displays a character on cursor position,
and cursor advances. Hankaku western and katakanas are possible.

2: NULL (0)

End of text.

3: ^L (12)

Wipes away the window. Cursor returns
to top of the screen.

4: ^E (5)

Wipes away all characters later than
the cursor.

5: ^H (8)

A character before the cursor is deleted,
and cursor moves back one space.

6: ^R (18)

Reverse the characters by length stated
by next 1 byte, from cursor position.

7: ^Z (26)

Upper 4 bits of next 1 byte becomes
color of the character, while lower 4 bits are selected background
color.

8: ^X (24)

Move the cursor. Next 1 byte is X-axis.

9: ^Y (25)

Move the cursor. Next 1 byte is Y-axis.
(optional)

10: ^W (23)

Open the window specified by next
1 byte. (optional)

11: Any value between 0 to 15 and
is not mentioned above will be skipped. Any value between 16 to
31 and is not mentioned above will be skipped, and 1 byte next
to it will be skipped too.

Any positions of characters should
be specified by units of hankaku (1-byte). Be noted though, that
when you specify X-axis of a cursor, 1 is the initiating value,
and 0 means cursor will not be displayed.

Size of a window supported by virtual
terminal is 1 line * 28 characters (14 zenkaku characters), but
a program can use windows of other sizes. Function call 9 which
will be explained later is used in such cases. If you are going
to use default 1 line * 28 characters window, then you have no
need to support ^Y or ^W.

If you open a window, the graphic
there will be wiped out, of course. If you do not want graphic
to be gone, then you must let program to reserve such contents.

A virtual terminal assumes that information
like contents of windows or cursor position are kept when it is
working, so make sure you do not harm such information. For instance,
when you add one character on end of a character string, virtual
terminal only returns one character long STB.

5.4.4 Virtual Terminal Function
Calls

Virtual terminal functions can be
called by same procedure you follow to use MSX-JE normally.

Interslot-call the address you acquired
by calling address FFCAH. Function 05H, 08H, 09H and 0AH are optional;
some MSX-JEs lack them.

clear (Function 04H)

Input: 04H to A

BW to HL

Output: None

This clears buffer for kanji conversion.
Use this when you want to clear inputted text so far.

set TTB (05H; optional. Some MSX-JEs
lack this function)

Input: 05H to A

Address of BW to HL

Address of text data you want to re-convert
to DE

Address of TTB to BC

Output: If A=0, it means re-conversion
is possible.

FFH means re-conversion is impossible.

Use this when you want to convert
a specific character string. If your MSX-JE does not support this
function (like VJE-80A), it returns FFH to A.

dispatch (06H)

Input: 06H to A

Address of BW to HL

Output: Status to A

Address of STB to HL

This dispatches processor resource
from application to virtual terminal. Or, simply said, it starts
kanji conversion. A virtual terminal sets the parameters and temporarily
gives away execution responsibility to program in cases like when
it displays characters in window. The program then displays STB
the virtual terminal returned on window, looks at statuses, and
decide what program has to do afterward. STB is within BW.

Meaning of statuses are as follows.

Bit 0: if 1, then program displays
STB.

Bit 1: if 1, then program can get
conversion result.

Bit 2: If 1, it means conversion at
virtual terminal is completed.

Meaning for each statuses;

000 Virtual terminal ignored key inputs,
or no key inputs occured

001 In middle of inputs, or in middle
of conversion

010 Partially finalized

011 Partially finalized

100 Conversion was interrupted and
quitted

101 Conversion was interrupted and
quitted

110 Wholly finalized

111 Wholly finalized

You must let your program to look
at 3 bits independently. For instance, if all 3 bits are standing,
then your program firstly should display STB, acquire the result
of conversion using function 07H, and quit dispatch function.
You can easily see that programs should repeat displaying to screen
and calling function 06H infinitely until bit 2 stands.

get_result (07H)

Input: 07H to A

Address of BW to HL

Output: Header address of acquired
conversion result to HL

This returns result of conversion.
The result will be returned in SHIFT-JIS format which ends with
00H.

get_TTB (08H; optional. Some MSX-JEs
lack this function)

Input: 07H to A

Address of BW to HL

Output: Address of TTB to HL

This gets the pronunciation data of
the text you acquired using get_result. The result will be returned
in TTB format. If your MSX-JE does not support this function (like
VJE-80A), it returns pointer to 0 to HL.

* translator's note: "TTB format"
might be misspelled "STB format".

Inquire_window_size (09H; optional.
Some MSX-JEs lack this function)

Input: 07H to A

Address of BW to HL

Maximum length of tail format to E

Maximum height of window format to
B

Maximum length of window format to
C

Output: Window specification data
to HL

Window used by a virtual terminal
is 1 line * 28 characters (14 zenkaku characters) at default,
but you can use this function when you want to use windows of
other sizes. Enter size of window that your program can handle,
and call. You have no need to call this if you are going to use
a window of default size.

A window specification data is an
array of several window specification items, and ends with 0.

3 bytes in one window specification
item is as follows.

1st byte: Window type

2nd byte: Window length

3rd byte: Window height

About window type, if you want to
let the input goes back to left tip of next line when it reaches
the rightmost tip of the screen, choose 2 (a window of this type
is called tail format), and if you do not want the input to back
to left, then choose 1 (a window of this type is called independent
format). Default window is of independent format.

If your virtual terminal is not supporting
the window of specified size and format, then it returns header
address of next data. In another word, it specified the default
window back again. [01H+1CH+01H+00H]

There is no guarantee that every virtual
terminals support the window of the size you want to use, so you
are not recommended to use this function in normal situations.

conflict_detect (0AH; optional. Some
MSX-JEs lack this function)

Input: 07H to A

Address of BW to HL

Output: If A is 0, then conflict is
not detected.

If A is FFH, it means conflict is
detected.

This is used to transfer control responsibility
to virtual terminal, when you prefer to let the terminal to pick
up key input that program is supposed to pick up. This function
is for purpose of avoiding conflicts between keys which program
uses, and keys which virtual terminal uses. I do not expect much
use of this in normal occasions. ;)

The explanations for virtual terminal
function calls end here.

5.4.5 Specified Control Keys for
Virtual Terminal

Virtual terminal processes
key inputs by itself. Therefore, if your program and virtual terminal
try assign same key for their functions, then program ends up
to hangup (a sort of malfunction).

For instance, suppose you made the
program so it will transfer control responsibility to terminal
if you press SPACE key. At the same time, if a virtual terminal
assumed it will return the execution to program by pressing SPACE,
then if you press SPACE key then execution will keep bouncing
back and forth like badminton match;), never to return, leaving
a stroke SPACE of key buffer. To prevent these kind of things to
happen, there are several fixed agreements about keys that a virtual
terminal and application programs should take out of buffer.

For virtual terminal;

Keys that interrupt conversion without
removing contents of key buffer: F6 to F10 or CTRL+STOP

Keys that interrupt conversion and
remove contents of key buffer: ESC

Keys that always remove contents of
key buffer: F1 to F5, SPACE or any character keys including graphic
characters

For application programs;

Keys that always take out contents
of key buffer: F6 to F10, CTRL+STOP or any character keys including
graphic characters

Keys that initiate conversion and
do not take out contents of key buffer: F1 to F5, SPACE or any
character keys including graphic characters

A program needs to check these keys
during the period between the time it called _clear_ function
and the time it calls _dispatch_. Virtual terminal will do the
processes later then.

By the way, you must check the first
character in key buffer without taking it out of key buffer if
you do not want to transgress this rule. For this purpose, use
following method.

Firstly, use the CHSNS (009CH) bios
to check whether there is any content in key buffer.

If it is not empty, you can find the
next key code to be taken out, in an address specified by 2 bytes
from address F3FAH.

Also, since an MSX cannot check whether
its function keys are pressed, virtual terminal sets following
codes to function keys to recognize function key strokes. These
values are set at the time you call _clear_ function. If you do
not want original contents to be gone, let your program to reserve
initial contents of function keys.

F1: 01H+31H, F2: 01H+32H, F3: 01H+33H,
F4: 01H+34H, F5: 01H+35H

Moreover, if you set following values
to F6 to F10m then a virtual terminal processes then as F6 to
F10. Inputs of other values are regarded as input of equivalent
character strings, and will be processed so.

F6: FFH+36H, F7: FFH+37H, F8: FFH+38H,
F9: FFH+39H, F10: FFH+3AH

5.4.7 Keys Assigned by Virtual
Terminal

[F1] | Switch input mode (romaji / western characters)
[F2] | Hiragana conversion
[F3] | Katakana conversion
[F4] | Convert to hankaku
[F5] | Convert using JIS code
[CTRL]+[F5] | Finalize all
[BS] | Step back and delete (backspace)
[DEL] | Delete a character on cursor position
[ESC] | Quit editing (keystrokes do not remain in buffer)
[RETURN] | Finalize partially (same as CTRL+F5 during edit)
[UP] | Start conversion (same as SPACE)
[DOWN] | Start conversion (same as SPACE)
[RIGHT] | Move cursor to right
[SHIFT]+[RIGHT] | Move cursor to rightmost
[LEFT] | Move cursor to left
[SHIFT]+[LEFT] | Move cursor to leftmost
[SPACE] | Start conversion
[SHIFT]+[SPACE] | Input zenkaku space (which is different with 1-byte space)
[CTRL]+[STOP] | Quit editing (keystrokes remain in buffer)
[F6] to [F10] | Quit editing (keystrokes remain in buffer)

If there are any character strings
in buffer, and they are not keys mentioned above, such keystrokes
will be ignored and removed out of buffer. Even in such cases,
virtual terminal return the execution responsibility to your program
anyway (status is 000B in such state). It returns execution responsibility
when there is not key input at all, too. This is to enable your
program to process other jobs simultaneously with Japanese inputs.
If you do not need simultaneous tasktaking, then call _dispatch_
as is. If keys 00H to 1FH are pressed when there is no character
string during editing, then conversion quits while still leaving
such inputs in buffer. (status will be 100B)

If you press SPACE when there is no
character string during editing, then virtual terminal finalizes
(generates) zenkaku space, and returns execution responsibility
to program.

The characters you can add to editing
character strings are limited to western characters and hiraganas.
Katakanas will be processed as equivalent hiraganas.

During conversion

[F1] to [F5] | Same as editing
[CTRL]+[F5] | Finalize all (finalizes the domain up to cursor position)
[ESC] | Quit conversion (keystrokes do not remain in buffer)
[RETURN] | Finalize partially (finalizes header bunsetsu)
[UP] | Move to prior block
[DOWN] | Move to proceeding block
[RIGHT] | Quit conversion
[LEFT] | Quit conversion
[SPACE] | Next candidate
[SHIFT]+[SPACE] | Prior candidate
[CTRL]+[STOP] | Quit editing (keystrokes remain in buffer)
[F6] to [F10] | Quit editing (keystrokes remain in buffer)

Key buffers will be cleared when "move
to prior block" and "move to proceeding block"
were selected.

The characters up to one position
before the cursor will be subject to conversion and finalizing.

Be noted that key assignment described
here are, for ASCII VJE-80 and 80A only. As long as you keep rules
noted in  5, they should not be exactly this way. Therefore, some
function keys might differ with ASCII's assignment on MSX-JEs
of other machines. For example, "next candidate" is
[GRAPH] instead of [SHIFT]+[SPACE] in case of Sony machines. Accordingly,
[F2] converts to katakanasm and [F3] converts to hankakus. There
are several more differences.

Chapter 5 Cautions upon Using MSX-JE

Difference between MSX-JE and VJE-80

It seems too many people are confusing
MSX-JE and VJE-80, so I explain that, MSX-JE is a name of standard
when MSX performs Japanese input, while VJE-80 is a name of "a
Japanese front end which satisfies MSX-JE standard".

"MSX-WRITE II" released
by ASCII has a backup option for its internal S-RAM dictionary.
I thought it was a useful option, but specification book did not
state any explanations about this option. It seems, implementation
of this option was possible simply because MSX-WRITEII has word
processor software and S-RAM stuffed in a same cartridge. You
can see this vividly, when you insert a Sony MSX-JE cartridge
and this MSX-WRITE II, MSX-WRITE II boots as word processor software,
while Sony's MSX-JE happens to boot. ;) If you perform S-RAM backup
here, what is backed up is not Sony's, but MSX-WRITE II's internal
SRAM. I was bit disappointed, since I expected functions like
dictionary backups and summary of current dictionary were available.
Because you cannot remember all the terms you have registered,
can you?

A least necessary size of work area
a MSX-JE requests to boot never exceeds 2560 bytes. Therefore,
if your software has limitations on memories, then it's recommended
to allocate 2560 bytes of domain for MSX-JE when the software
boots.

About character colors a STB specifies.
STB select character colors using ^Z. This option is supported
as follows.

Alignment of character string Character/background
color

Editing character string 1FH

Character string subject to conversion
F5H

Proceeding bunsetsu 1FH

Bunsetsu that is not converted yet
1FH

If software which have customized
palettes, or software which runs on SCREEN6 displays the MSX-JE
windows in these colors as they are, the outlook will be strange,
so you better let such software do some clever configuration about
these settings and display using color you think suitable.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("nambas", "", "Variaveis RAM", "F922 NAMBAS  Base address of current pattern name table", "F924 CGPBAS Base address of current pattern generator table",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F922 NAMBAS  Base address of current pattern name table
F924 CGPBAS  Base address of current pattern generator table
F926 PATBAS  Base address of current sprite generator table
F928 ATRBAS  Base address of current sprite attribute table

F92A CLOC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("newpad", "", "BIOS-ROM", "01ADH NEWPAD", "Entry:",
        R"MAN(AREA: Sub-ROM do MSX2

01ADH NEWPAD

Entry:
Exit:
Modifies:

Turbo-R: The lightpen function has been removed. When 8-11 is put in register A and a call is made, it will normally return with 0.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("nmbdrv", "", "Variaveis RAM", "F347 NMBDRV  total number of drives", "Total number of logical drives (e.g. used by CALL FORMAT).",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F347 NMBDRV  total number of drives

Total number of logical drives (e.g. used by CALL FORMAT).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("nmi", "", "BIOS-ROM", "0066H NMI    Non Maskable Interrupt handler", "Standard routine to process a Z80 Non Maskable interrupt.",
        R"MAN(AREA: BIOS principal (MainROM)

0066H NMI    Non Maskable Interrupt handler

Standard routine to process a Z80 Non Maskable interrupt.
Entry: | none
Exit: | none
Modifies: | none

AREA: Sub-ROM do MSX2

0066H NMI Non Maskable Interrupt

This routine is equal to the MainROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("noruse", "", "Variaveis RAM", "FAFD NORUSE  Used by kanji driver", "Bit 7: 1-graphic and text mixture mode",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FAFD NORUSE  Used by kanji driver

  Bit 7: 1-graphic and text mixture mode

  Bit 6: 1-Vertically scross the screen by pushing SHIFT+cursor key

  Bit 5,4: Used internally in the kanji driver

  Bit 3-0: VDP logical operation.

Notes from Takamichi Suzukawa:
The romaji-kana conversion is available both under ank (1-byte) and kanji
mode. This input mode is activated/activated by pushing certain key
combination. Under romaji-kana conversion mode, you push pronunciations (two
or three key strokes) sequentially, then you get one kana. e.g. push "s" and
"e", you get "se", though in non romaji-kana conversion mode, you push "P"
key to get "se" kana.

You might think this mode is much slower than non conversion mode, but
actually, 70% Japanese PC (Windows, Macintosh, Linux...) users rely on
romaji-kana conversion, because under non conversion mode, you have to push
"kana" "numerals" "latin"... keys quite frequently. You probably know a lot
of Japanese keyboard keys have four characters written on each. ;)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("nrdvrm", "", "BIOS-ROM", "0174H NRDVRM", "Reads VRAM like in RDVRM.(with full 16 bits VRAM-address)",
        R"MAN(AREA: BIOS principal (MainROM)

0174H NRDVRM

Reads VRAM like in RDVRM.(with full 16 bits VRAM-address)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("nsetcx", "", "BIOS-ROM", "0123H NSETCX Set attribute of number of pixels", "0123H NSETCX Set attribute of number of pixels",
        R"MAN(AREA: BIOS principal (MainROM)

0123H NSETCX Set attribute of number of pixels

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("nsetrd", "", "BIOS-ROM", "016EH NSETRD", "Same function as SETRD.(with full 16 bits VRAM-address)",
        R"MAN(AREA: BIOS principal (MainROM)

016EH NSETRD

Same function as SETRD.(with full 16 bits VRAM-address)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("nstwrt", "", "BIOS-ROM", "0171H NSTWRT", "Same function as SETWRT.(with full 16 bits VRAM-address)",
        R"MAN(AREA: BIOS principal (MainROM)

0171H NSTWRT

Same function as SETWRT.(with full 16 bits VRAM-address)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ntmsxp", "", "Variaveis RAM", "F417 NTMSXP  =0 If MSX printer", "This variable determines whether the OUTDO standard routine",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F417 NTMSXP  =0 If MSX printer

This variable determines whether the OUTDO standard routine
will replace headered graphics characters directed to the
printer with spaces: 00H=Graphics, NZ=Spaces. Its value is set
at power-up and thereafter only altered by the "SCREEN"
statement.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("nvbxln", "", "BIOS-ROM", "00C9H NVBXLN", "00C9H NVBXLN",
        R"MAN(AREA: Sub-ROM do MSX2

00C9H NVBXLN

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("nwrvrm", "", "BIOS-ROM", "0177H NWRVRM", "Writes to VRAM like in WRTVRM.(with full 16 bits VRAM-address)",
        R"MAN(AREA: BIOS principal (MainROM)

0177H NWRVRM

Writes to VRAM like in WRTVRM.(with full 16 bits VRAM-address)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("oldscr", "", "Variaveis RAM", "FCB0 OLDSCR  old screen mode", "This variable holds the screen mode of the last text mode set.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCB0 OLDSCR  old screen mode

This variable holds the screen mode of the last text mode set.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ongsbf", "", "Variaveis RAM", "FBD8 ONGSBF   global event flag", "This variable is normally zero, but is incremented by the",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FBD8 ONGSBF   global event flag

This variable is normally zero, but is incremented by the
interrupt handler whenever a device has achieved the conditions
necessary to generate a program interrupt. It is used by the
Runloop to determine whether any program interrupts are pending
without having to search TRPTBL.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("open file", "", "BDOS", "0F Open File", "0F Open File",
        R"MAN(AREA: BDOS (MSX-DOS)

0F Open File

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("open file handle", "", "BDOS", "43 Open file handle", "43 Open file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

43 Open file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("outdlp", "", "BIOS-ROM", "014DH OUTDLP Formatted output to line printer", "Standard routine to output a character to the printer. If the character is",
        R"MAN(AREA: BIOS principal (MainROM)

014DH OUTDLP Formatted output to line printer

Standard routine to output a character to the printer. If the character is
a TAB code (09H) spaces are issued to the OUTDLP standard routine until LPTPOS
(F415H) is a multiple of eight (0, 8, 16, etc.). If the character is a CR code
(0DH) LPTPOS is zeroed, if it is any other control code LPTPOS is unaffected,
if it is a displayable character LPTPOS is incremented.

If NTMSXP (F417H) is zero, meaning an MSX-specific printer is connected,
the character is passed directly to the printer. Assuming a normal printer
is connected the CNVCHR standard routine is used to check for graphic characters.
If the character is a header code (01H) the routine terminates with no action.
If it is a converted character it is replaced by a space, all other characters
are passed to the printer.
Entry: | A = character to output
Exit: | none
Modifies: | EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("outdo", "", "BIOS-ROM", "0018H OUTDO  Output to current device", "Standard routine used by the BASIC Interpreter to output a character to the current device. The...",
        R"MAN(AREA: BIOS principal (MainROM)

0018H OUTDO  Output to current device

Standard routine used by the BASIC Interpreter to output a character to the current device. The ISFLIO standard routine is first used to check whether output is currently directed to anI/O buffer, if so control transfers to the sequential output driver via the CALBAS standard routine. If PRTFLG is zero control transfers to the CHPUT standard routine to output the character to the screen. Assuming the printer is active RAWPRT is checked. If this is non-zero the character is passed directly to the printer, otherwise the control drops into the OUTDLP standard routine.

Entry: | A=Character to output
Exit: | none
Modifies: | EI

AREA: Sub-ROM do MSX2

0018H OUTDO (RST 18H)

This routine is equal to the MainROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("padx", "", "Variaveis RAM", "FC9D PADX", "FC9D PADX",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC9D PADX

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("pady", "", "Variaveis RAM", "FC9C PADY", "FC9C PADY",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC9C PADY

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("paint", "", "BIOS-ROM", "0069H PAINT", "This routine is used by the Basic-Interpreter to execute the PAINT-statement. It fills the...",
        R"MAN(AREA: Sub-ROM do MSX2

0069H PAINT

This routine is used by the Basic-Interpreter to execute the PAINT-statement. It fills the surrounded areaof the graphic-screen (modes 5-8) with the specified color.

This function is usually not applicable for machinecode-programs.
Entry: | HL = Basic-pointer
Exit: | HL = new pointer
Modifies: | all

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("parse filename", "", "BDOS", "5C Parse filename", "5C Parse filename",
        R"MAN(AREA: BDOS (MSX-DOS)

5C Parse filename

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("parse pathname", "", "BDOS", "5B Parse pathname", "5B Parse pathname",
        R"MAN(AREA: BDOS (MSX-DOS)

5B Parse pathname

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("pcmply", "", "BIOS-ROM", "0186H PCMPLY play PCM sound", "Replay PCM data",
        R"MAN(AREA: BIOS principal (MainROM)

0186H PCMPLY play PCM sound

Replay PCM data
Entry: | A
    b7 b6 b5 b4 b3 b2 b1 b0
     R  0  0  0  0  0  S  S
     |                 |  |
     |                  ----->sampling rate
      -------> VRAM/MAIN RAM

-sampling rate
Settings for the sampling rate
----------------------------------------------------
value   sampling rate (Khz)
----------------------------------------------------
 00       15.75
 01        7.875
 10        5.25
 11        3.9375
----------------------------------------------------

-memmory
Selects the memmory where the data to be replayed is located.
----------------------------------------------------
memmory    meaning
----------------------------------------------------
 0         mainram
 1         VRAM
----------------------------------------------------

HL    address to the PCM data.
      When the VRAM is selected also the E register is used
      to create together with HL a 3 byte address. The E
      register contains the highest position.
BC    length of the PCM data.
      When the VRAM is selected also the D register is used
      to create together with BC a 3 byte address. The D
      register contains the highest position.
Exit: | Carry flag
----------------------------------------------------
value     meaning
----------------------------------------------------
 0         normal end
 1         unnormal end
              A = reason for unnormal end
                 1 = error in setting of the samplingfrequency
                 2 = the STOP key has been pressed
              HL = the address when the interruption occurred
                   When using the VRAM, the E register
                   will contain the highest address position.
----------------------------------------------------
Modifies: | all

Note: When running in the Z80 MODE, this routine will switch to the R800 ROM mode automatically but it will return to the Z80 MODE when the routine is finished. While replaying the PCM data, no interrupts can be generated. When STOP is pressed, the replaying will be interrupted. In this BIOS, the program that executes the replaying of samples is located in page 1 (04000h-07fffh). So when you place sample

data in the MAIN RAM you must place then above 08000H.

Function: Plays PCM. A new command supplied to turboR. Use bit 7 of A register to specify whether the PCM data is in RAM or in VRAM. Among sampling frequencies, 15.75 kHz is valid only when CPU is in R800 DRAM mode. If 15.75 kHz is chosen in other CPU mode, it is treated as an error. Note D and E registers are useful only when PCM data is in VRAM.

Input: If bit7 of A is 0, it means data is in main RAM; 1 means that data is in VRAM. Bits 6 to 2 are 0. If bits 1 and 0 are, 0 and 0, then frequency is 15.75 kHz; 0 and 1 means 7.875 kHz, 1 and 0 means 5.25 kHz, and 1 and 1 means 3.9375 kHz.

Use registers EHL to specify starting address of PCM data being played

Use DBC to specify length of data (number of bytes)

Output: If Cy reset occurs, it means PCM playing quitted normally.

If set, it means playing quitted abnormally. Moreover;

Register A tells the cause of abnormality. If A is 0, it means you specified wrong frequency. If A is 1, it means CTRL+STOP halt occurred.

And EHL would contain address of which halt occurred there.

Refer to: PCM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("pcmrec", "", "BIOS-ROM", "0189H PCMREC record PCM sound", "Record PCM data",
        R"MAN(AREA: BIOS principal (MainROM)

0189H PCMREC record PCM sound

Record PCM data
Entry: | A
b7 b6 b5 b4 b3 b2 b1 b0
 R  T  T  T  T  C  S  S
 |  |  |  |  |  |  |  |
 |   --------   |   --------> sampling rate
 |      |       --------> reduction
 |      -----------> trigger level
 --------------> VRAM/mainram

  -sampling rate
  Settings for the sampling rate
  ----------------------------------------------------
  value   sampling rate (Khz)
  ----------------------------------------------------
   00       15.75
   01        7.875
   10        5.25
   11        3.9375
  ----------------------------------------------------

  -reduction
  ----------------------------------------------------
  value   meaning
  ----------------------------------------------------
    0      no reduction
    1      reduction
  ----------------------------------------------------

  -trigger level
  This designates the level sound needed to start sampling

  -memmory
  Selects the memmory where the data to be replayed is located.
  ----------------------------------------------------
  memmory    meaning
  ----------------------------------------------------
   0         mainram
   1         VRAM
  ----------------------------------------------------

  HL    address to the PCM data.
        When the VRAM is selected also the E register is used
        to create together with HL a 3 byte address. The E
        register contains the highest position.
  BC    length of the PCM data.
        When the VRAM is selected also the D register is used
        to create together with BC a 3 byte address. The D
        register contains the highest position.
Exit: | clear flag
----------------------------------------------------
value     meaning
----------------------------------------------------
 0         normal end
 1         unnormal end
              A = reason for unnormal end
                 1 = error in setting of the samplingfrequency
                 2 = the STOP key has been pressed
              HL = the address when the interruption occurred
                   When using the VRAM, the E register
                   will contain the highest address position.
----------------------------------------------------
Modifies: | all

Note: When running in the Z80 MODE, this routine will switch to the R800 ROM mode automatically but it will return to the Z80 MODE when the routine is finished. When running in the Z80 MODE or the R800 ROM MODE, a 'error in setting of sample frequency' error will occur when you try to sample with a frequency of 15.75 kHz. While recording the PCM data, no interrupts can be generated. When STOP is pressed, the recording will be interrupted. When recording in the MAIN RAM, please note the start address (see PCMPLAY-note).

Reduction: Replay data is always regarded as reduced data. When recording you can determine whether to reduce the PCM data or not. But, when the sound data is equivalent to 0, the value that is recorded will become 1 (to prevent problems when replaying.)

Function: Records PCM. A new command supplied to turboR. A "trigger level" is a volume of noise that causes a recording to start when MSX hears a sound louder than the level. If the level is 0, then recording begins immediately after execution of this command. If bit 2 is set to 1, then recorded data will be compressed.

Input: If bit7 of A is 0, then data will be saved to RAM; if 1, then data will be saved to VRAM. Use bits 6 to 3 to specify trigger level. If bit2 is 0, then data compression is disabled. If 1, then data will be compressed. Use bits 1 and 0 to specify frequency, and their usage is identical with PCMPLY.

Output: If Cy reset occurs, it means recording quitted normally.

If set, it means recording has failed. Moreover;

Register A tells the cause of abnormality. If A is 0, it means you specified wrong frequency. If A is 1, it means CTRL+STOP halt occurred.

Refer to: PCM

The following entrypoints do not necessarily match the MSX-standard,
but are used by several programs:

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("phydio", "", "BIOS-ROM", "0144H PHYDIO Read/write disk sector(s)", "Standard routine for use by Disk BASIC. If not present the routine returns without action.",
        R"MAN(AREA: BIOS principal (MainROM)

0144H PHYDIO Read/write disk sector(s)

Standard routine for use by Disk BASIC. If not present the routine returns without action.
Otherwise it reads a logical sector from disk or writes a logical sector to disk.

This is the prefered method to read/write sectors, because all installed Disk-ROMs are
considered by this routine! The MSX-Standard supports up to 4 Disk-ROMs with a maximum
of 8 drive-letters (A:-H:). This routine is very similar to the PHYDIO (4010H) routine in the
Disk-ROM, so more information about it's parameters
and return-values can be found there!

Entry: | F = CARRY = 1 to write, CARRY = 0 to read
A = drive number (0=A:, 1=B:, ...)
B = number of sectors
C = media descriptor
DE = logical sector number (0 based)
HL = transfer address (sector buffer)
Exit: | CARRY = 0 on success
CARRY if error
A = errorcode (if error)
B = number of sectors transfered
Modifies: | ?

AREA: ROM de disco (Disk-ROM)

4010H PHYDIO Physical Disk-I/O

Reads/writes sectors to disk.
Entry: | F = CARRY is write
A = Drive number (0=A:, 1=B:, ...)
B = number of sectors to read/write
C = media-descriptor
DE = logical sector number (0 based)
HL = transfer address
Exit: | F = CARRY if error
A = errorcode (if error)
B = number of sectors transfered (always)
Modifies: | ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("pinlin", "", "BIOS-ROM", "00AEH PINLIN Get line from console (editor)", "00AEH PINLIN Get line from console (editor)",
        R"MAN(AREA: BIOS principal (MainROM)

00AEH PINLIN Get line from console (editor)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("pntini", "", "BIOS-ROM", "0129H PNTINI Paint initialise", "0129H PNTINI Paint initialise",
        R"MAN(AREA: BIOS principal (MainROM)

0129H PNTINI Paint initialise

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("posit", "", "BIOS-ROM", "00C6H POSIT  Set cursor position", "00C6H POSIT Set cursor position",
        R"MAN(AREA: BIOS principal (MainROM)

00C6H POSIT  Set cursor position

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("prompt", "", "BIOS-ROM", "0181H PROMPT", "Shows a prompt (normal \"Ok\")",
        R"MAN(AREA: Sub-ROM do MSX2

0181H PROMPT

Shows a prompt (normal "Ok")
Entry: | none
Exit: | none
Modifies: | All ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("prtflg", "", "Variaveis RAM", "F416 PRTFLG  Flag whether to send to printer", "This variable determines whether the OUTDO standard routine",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F416 PRTFLG  Flag whether to send to printer

This variable determines whether the OUTDO standard routine
directs its output to the screen or printer: 00H=Screen,
01H=Printer.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("pset", "", "BIOS-ROM", "0069H PSET", "This routine is used by the Basic-Interpreter to execute the PSET-statement. It places a pixel...",
        R"MAN(AREA: Sub-ROM do MSX2

0069H PSET

This routine is used by the Basic-Interpreter to execute the PSET-statement. It places a pixel of the specified color on the graphic-screen (mode 5-8).

This function is usually not applicable for machinecode-programs.
Entry: | HL = Basic-pointer
Exit: | HL = new pointer
Modifies: | all

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("putchr", "", "BIOS-ROM", "0139H PUTCHR", "0139H PUTCHR",
        R"MAN(AREA: Sub-ROM do MSX2

0139H PUTCHR

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("putpnt", "", "Variaveis RAM", "F3F8 PUTPNT  DW KEYBUF  points to adress to write in the key buffer", "This variable contains the address of the put position in",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3F8 PUTPNT  DW KEYBUF  points to adress to write in the key buffer

This variable contains the address of the put position in
KEYBUF.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("putq", "", "BIOS-ROM", "00F6H PUTQ   Put byte in music queue", "00F6H PUTQ Put byte in music queue",
        R"MAN(AREA: BIOS principal (MainROM)

00F6H PUTQ   Put byte in music queue

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("putspr", "", "BIOS-ROM", "0151H PUTSPR (Basic!)", "0151H PUTSPR (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

0151H PUTSPR (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("putvrm", "", "BIOS-ROM", "0129H PUTVRM", "0129H PUTVRM",
        R"MAN(AREA: Sub-ROM do MSX2

0129H PUTVRM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("qinlin", "", "BIOS-ROM", "00B4H QINLIN Display \"?\", get line from console (editor)", "00B4H QINLIN Display \"?\", get line from console (editor)",
        R"MAN(AREA: BIOS principal (MainROM)

00B4H QINLIN Display "?", get line from console (editor)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("random block read", "", "BDOS", "27 Random Block Read", "27 Random Block Read",
        R"MAN(AREA: BDOS (MSX-DOS)

27 Random Block Read

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("random block write", "", "BDOS", "26 Random Block Write", "26 Random Block Write",
        R"MAN(AREA: BDOS (MSX-DOS)

26 Random Block Write

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("random read", "", "BDOS", "21 Random Read", "21 Random Read",
        R"MAN(AREA: BDOS (MSX-DOS)

21 Random Read

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("random write", "", "BDOS", "22 Random Write", "22 Random Write",
        R"MAN(AREA: BDOS (MSX-DOS)

22 Random Write

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("random write with zero fill", "", "BDOS", "28 Random Write With Zero Fill", "28 Random Write With Zero Fill",
        R"MAN(AREA: BDOS (MSX-DOS)

28 Random Write With Zero Fill

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rawprt", "", "Variaveis RAM", "F418 RAWPRT  DB 00H  <>0 If printing is in 'RAW MODE'", "This variable determines whether the OUTDO standard routine",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F418 RAWPRT  DB 00H  <>0 If printing is in 'RAW MODE'

This variable determines whether the OUTDO standard routine
will modify control and headered graphics characters directed
to the printer: 00H=Modify, NZ=RAW. Its value is set at power-up
and thereafter unaltered.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rdlogblk", "", "BIOS-ROM", "7F89H RdLogBlk", "READ logical block(s) from HDD.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F89H RdLogBlk

READ logical block(s) from HDD.
Entry: | A = LUN + SCSI Target ID
B = # of blocks to read
C D E = 21 bit logical block #
HL = transfer address
Exit: | A = SCSI STATUS
D = Target Status
E = Message
Modifies: | AF, DE, IX, IY, HL
Note: | See ANSI 8.2.5

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rdprim", "", "Variaveis RAM", "F380 RDPRIM  Read from basic slot", "This routine is used by the RDSLT standard routine to switch Primary Slots and read a byte",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F380 RDPRIM  Read from basic slot

This routine is used by the RDSLT standard routine to switch Primary Slots and read a byte
from memory. The new Primary Slot Register setting is supplied in register A, the old setting
in register D and the byte read returned in register E.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rdpsg", "", "BIOS-ROM", "0096H RDPSG  Read from any PSG register", "0096H RDPSG Read from any PSG register",
        R"MAN(AREA: BIOS principal (MainROM)

0096H RDPSG  Read from any PSG register

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rdres", "", "BIOS-ROM", "017AH RDRES Read Reset-state", "Determine if MSX was restarted by software-reset (JP 0000H) or by hardware-reset.",
        R"MAN(AREA: BIOS principal (MainROM)

017AH RDRES Read Reset-state

Determine if MSX was restarted by software-reset (JP 0000H) or by hardware-reset.

System reserved
Entry: | ?
Exit: | A = if bit7 is 0: hardware-reset, if bit7 is 1: software-reset
Modifies: | ?

Refer to: Software-Reset

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rdslt", "", "BIOS-ROM", "000CH RDSLT  Read RAM in any slot", "Standard routine to read a single byte from memory in any slot. The slot identifier is composed...",
        R"MAN(AREA: BIOS principal (MainROM)

000CH RDSLT  Read RAM in any slot

Standard routine to read a single byte from memory in any slot. The slot identifier is composed of a primary slot number and a flag:

    7       6       5       4       3       2       1       0
+-------+-------+-------+-------+-------+-------+-------+-------+
| Flag  |   0   |   0   |   0   |    SSLOT #    |    PSLOT #    |
+-------+-------+-------+-------+-------+-------+-------+-------+

The flag is normally 0 but must be 1 if the secondary slot number is included in the Slot ID. The memory address and Slot ID are first processed to yield a set of bit masks to apply to the relevant slot register. If a secondary slot number is specified then the secondary slot register is first modified to select the relevant page from the secondary slot. The primary slot is then switched in to the Z80 address space, the byte read and the primary slot restored to it's original setting via the RDPRIM routine in the workspace area. Finally, if a secondary slot number is included in the Slot ID, the original secondary slot register setting is restored.

Note that, unless it is the slot containing the workspace area, any attempt to access page 3 (C000H to FFFFH) will cause the system to crash as RDPRIM will switch itself out. Note also that interrupts are left disabled by all the memory switching routines.

Entry: | A=Slot ID
HL=Address
Exit: | A=Byte read
Modifies: | AF, BC, DE, DI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rdvdp", "", "BIOS-ROM", "013EH RDVDP  Read VDP Status Register", "013EH RDVDP Read VDP Status Register",
        R"MAN(AREA: BIOS principal (MainROM)

013EH RDVDP  Read VDP Status Register

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rdvrm", "", "BIOS-ROM", "004AH RDVRM  Read byte from VRAM", "Standard routine to read a single byte from the VDP VRAM.",
        R"MAN(AREA: BIOS principal (MainROM)

004AH RDVRM  Read byte from VRAM

Standard routine to read a single byte from the VDP VRAM.
Entry: | HL = address to read
Exit: | A = read value
Modifies: | AF, EI

AREA: Sub-ROM do MSX2

010DH RDVRM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("read from file handle", "", "BDOS", "48 Read from file handle", "48 Read from file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

48 Read from file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("readc", "", "BIOS-ROM", "011DH READC  Read attribute of current pixel", "011DH READC Read attribute of current pixel",
        R"MAN(AREA: BIOS principal (MainROM)

011DH READC  Read attribute of current pixel

AREA: Sub-ROM do MSX2

0095H READC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("readcap", "", "BIOS-ROM", "7F95H ReadCap", "READ CAPACITY.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F95H ReadCap

READ CAPACITY.
Entry: | A = LUN + SCSI Target ID
HL = transfer address
Exit: | A = SCSI STATUS
D = Target Status
E = Message
Modifies: | AF,BC,DE,HL
Note: | Returns 8 bytes,
      see ANSI 8.2.7

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("reboot", "", "Variaveis RAM", "F340 REBOOT", "If 0 DOS will re-initialize itself.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F340 REBOOT

If 0 DOS will re-initialize itself.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("redclk", "", "BIOS-ROM", "01F5H REDCLK read Clock-IC data", "Reads CLOCK-IC register in the address specified by the",
        R"MAN(AREA: Sub-ROM do MSX2

01F5H REDCLK read Clock-IC data

Reads CLOCK-IC register in the address specified by the
C register and stores in the A register. Since the address
specification includes the block selection information as
described in the RTC description, it is not necessary to set the MODE
register and then read the objective register.
Entry: | C
address ( . . b5 b4 b3 b2 b1 b0
              |-+-| |---------|
                |     register
                |
                +---> block
Exit: | A=data (only 4 lower bits valid)
Modifies: | ?

Refer to: Clockchip (RTC)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rejoin parent process", "", "BDOS", "61 Rejoin parent process", "61 Rejoin parent process",
        R"MAN(AREA: BDOS (MSX-DOS)

61 Rejoin parent process

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rename file", "", "BDOS", "17 Rename File", "17 Rename File",
        R"MAN(AREA: BDOS (MSX-DOS)

17 Rename File

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rename file handle", "", "BDOS", "53 Rename file handle", "53 Rename file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

53 Rename file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rename file or subdirectory", "", "BDOS", "4E Rename file or subdirectory", "4E Rename file or subdirectory",
        R"MAN(AREA: BDOS (MSX-DOS)

4E Rename file or subdirectory

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("repcnt", "", "Variaveis RAM", "F3F7 REPCNT  Delay until the auto-repeat of the key begins", "This variable is used as a counter by the interrupt handler to",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3F7 REPCNT  Delay until the auto-repeat of the key begins

This variable is used as a counter by the interrupt handler to
control the key repeat rate.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("reqsense", "", "BIOS-ROM", "7F8FH ReqSense", "REQUEST SENSE.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F8FH ReqSense

REQUEST SENSE.
Entry: | A = LUN + SCSI Target ID
Exit: | A = DOS error code
IX = address of sectorbuffer filled with Sense Data
Modifies: | AF,BC,DE
Note: | Sense data will be delivered in Extended Sense Format (12h bytes).
      See ANSI 7.2.14

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("reserved", "", "BIOS-ROM", "7FB6H Reserved", "7FB6H Reserved",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7FB6H Reserved

AREA: ROM de disco (Disk-ROM)

7FB9H Reserved

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rightc", "", "BIOS-ROM", "00F9H RIGHTC Move current pixel physical address right", "00F9H RIGHTC Move current pixel physical address right",
        R"MAN(AREA: BIOS principal (MainROM)

00F9H RIGHTC Move current pixel physical address right

AREA: Sub-ROM do MSX2

00A5H RIGHTC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rombdos", "", "Variaveis RAM", "F37D ROMBDOS ROM BDOS vector", "Main work-area",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F37D ROMBDOS ROM BDOS vector

Main work-area

        .SUBROUTINES FOR READ/WRITE CALLS OF THE INTER-SLOT
;

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("romslt", "", "Variaveis RAM", "FFF7 ROMSLT  Slot-ID of the Main-ROM", "Slot-ID of the MainROM.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FFF7 ROMSLT  Slot-ID of the Main-ROM

Slot-ID of the MainROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rslreg", "", "BIOS-ROM", "0138H RSLREG Read Primary Slot Register", "0138H RSLREG Read Primary Slot Register",
        R"MAN(AREA: BIOS principal (MainROM)

0138H RSLREG Read Primary Slot Register

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rstplt", "", "BIOS-ROM", "0145H RSTPLT", "Restores palette-colors by using values, which are stored in VRAM. Their location depends on...",
        R"MAN(AREA: Sub-ROM do MSX2

0145H RSTPLT

Restores palette-colors by using values, which are stored in VRAM. Their location depends on the current screen-mode:
Screen 0 (40 chars) | 0400H-041FH
Screen 0 (80 chars) | 0F00H-0F1FH
Screen 1 | 2020H-203FH
Screen 2 | 2020H-203FH
Screen 3 | 2020H-203FH
Screen 4 | 2020H-203FH
Screen 5 | 7680H-769FH
Screen 6 | 7680H-769FH
Screen 7 | FA80H-FA9FH
Screen 8 | FA80H-FA9FH
The palette requires 2 bytes for each color. The parts for R/G/B (Red, Green, Blue) have to be in range from 0 to 7 and are defined as following:
Byte 0: | 0 R R R 0 B B B
Byte 1: | 0 0 0 0 0 G G G
Entry: | none
Exit: | none
Modifies: | EI ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("rtycnt", "", "Variaveis RAM", "FC9A RTYCNT", "This variable is unused by the current MSX ROM.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC9A RTYCNT

This variable is unused by the current MSX ROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("runbnf", "", "Variaveis RAM", "FCBE RUNBNF  doing bload bsave or not", "FCBE RUNBNF doing bload bsave or not",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCBE RUNBNF  doing bload bsave or not

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("savent", "", "Variaveis RAM", "FCBF SAVENT  start address for bsave", "FCBF SAVENT start address for bsave",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCBF SAVENT  start address for bsave

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("scalxy", "", "BIOS-ROM", "010EH SCALXY Scale graphics coordinates", "010EH SCALXY Scale graphics coordinates",
        R"MAN(AREA: BIOS principal (MainROM)

010EH SCALXY Scale graphics coordinates

AREA: Sub-ROM do MSX2

008DH SCALXY

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("scanl", "", "BIOS-ROM", "012FH SCANL  Scan pixels to left", "012FH SCANL Scan pixels to left",
        R"MAN(AREA: BIOS principal (MainROM)

012FH SCANL  Scan pixels to left

AREA: Sub-ROM do MSX2

00C5H SCANL

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("scanr", "", "BIOS-ROM", "012CH SCANR  Scan pixels to right", "012CH SCANR Scan pixels to right",
        R"MAN(AREA: BIOS principal (MainROM)

012CH SCANR  Scan pixels to right

AREA: Sub-ROM do MSX2

00C1H SCANR

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("scopy", "", "BIOS-ROM", "018DH SCOPY (Basic!)", "018DH SCOPY (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

018DH SCOPY (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("screen", "", "BIOS-ROM", "0159H SCREEN (Basic!)", "0159H SCREEN (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

0159H SCREEN (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("screen-modes", "", "Variaveis RAM", "0 | Text | 40x24, 80x24 | no sprites", "1 | Text | 32x24 | 4 sprites per line",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

0 | Text | 40x24, 80x24 | no sprites
1 | Text | 32x24 | 4 sprites per line
2 | Graphics | 256x192x16 | 4 sprites per line
3 | Multicolor | 64x48x16 | 4 sprites per line
4 | Graphics | 256x192x16 | 8 sprites per line
5 | Graphics | 256x212x16 | 8 sprites per line
6 | Graphics | 512x212x4 | 8 sprites per line
7 | Graphics | 512x212x16 | 8 sprites per line
8 | Graphics | 256x212x256 | 8 sprites per line
9 | Text | 80x24 | no sprites
10 | Graphics | 256x212xALot16 | 8 sprites per line
11 | Graphics | 256x212xALot256 | 8 sprites per line
12 | Graphics | 256x212x19268 | 8 sprites per line

Notes:

Screens 4-8 are available on MSX2 computers only!
Screen 9 is available on Korean MSX2 computers only!
Screen 10 and 12 are identical, but uses different color-ranges in BIOS-calls: Screen 10 from 0 to 15, Screen 11 from 0 to 255. Screen 12 can display up to 19268 colors simultaneously, Screens 10 and 11 are using a color-subset, the remaining bits can be used to define a own 16color-palette...

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("scrmod", "", "Variaveis RAM", "FCAF SCRMOD  screen mode", "This variable contains the current screen mode: 0=40x24 Text or 80x24 Text Mode,",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCAF SCRMOD  screen mode

This variable contains the current screen mode: 0=40x24 Text or 80x24 Text Mode,
1=32x24 Text Mode, 2=Graphics Mode, 3=Multicolor Mode.

See also: Screen-modes

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("scsi bios rom id", "", "BIOS-ROM", "7F80H SCSI BIOS ROM ID", "\"HD#\" (in ASCII) indicates that the ROM is an SCSI controller.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F80H SCSI BIOS ROM ID

"HD#" (in ASCII) indicates that the ROM is an SCSI controller.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("sdfscr", "", "BIOS-ROM", "0185H SDFSCR", "Recovers screen-parameters of clock-chip. When C-flag is set",
        R"MAN(AREA: Sub-ROM do MSX2

0185H SDFSCR

Recovers screen-parameters of clock-chip. When C-flag is set
function-key text will be displayed.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("search first", "", "BDOS", "11 Search first", "11 Search first",
        R"MAN(AREA: BDOS (MSX-DOS)

11 Search first

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("search next", "", "BDOS", "12 Search next", "12 Search next",
        R"MAN(AREA: BDOS (MSX-DOS)

12 Search next

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("select disk", "", "BDOS", "0E Select Disk", "0E Select Disk",
        R"MAN(AREA: BDOS (MSX-DOS)

0E Select Disk

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("selmlt", "", "BIOS-ROM", "0081H SELMLT Set VDP to Multicolor Mode", "0081H SELMLT Set VDP to Multicolor Mode",
        R"MAN(AREA: BIOS principal (MainROM)

0081H SELMLT Set VDP to Multicolor Mode

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("sequential read", "", "BDOS", "14 Sequential Read", "14 Sequential Read",
        R"MAN(AREA: BDOS (MSX-DOS)

14 Sequential Read

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("sequential write", "", "BDOS", "15 Sequential Write", "15 Sequential Write",
        R"MAN(AREA: BDOS (MSX-DOS)

15 Sequential Write

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("set date", "", "BDOS", "2B Set Date", "2B Set Date",
        R"MAN(AREA: BDOS (MSX-DOS)

2B Set Date

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("set dma address", "", "BDOS", "1A Set DMA Address", "1A Set DMA Address",
        R"MAN(AREA: BDOS (MSX-DOS)

1A Set DMA Address

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("set environment item", "", "BDOS", "6C Set environment item", "6C Set environment item",
        R"MAN(AREA: BDOS (MSX-DOS)

6C Set environment item

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("set random record", "", "BDOS", "24 Set Random Record", "24 Set Random Record",
        R"MAN(AREA: BDOS (MSX-DOS)

24 Set Random Record

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("set time", "", "BDOS", "2D Set Time", "2D Set Time",
        R"MAN(AREA: BDOS (MSX-DOS)

2D Set Time

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("set/reset verify flag", "", "BDOS", "2E Set/Reset Verify Flag", "2E Set/Reset Verify Flag",
        R"MAN(AREA: BDOS (MSX-DOS)

2E Set/Reset Verify Flag

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setatr", "", "BIOS-ROM", "011AH SETATR Set attribute byte", "011AH SETATR Set attribute byte",
        R"MAN(AREA: BIOS principal (MainROM)

011AH SETATR Set attribute byte

AREA: Sub-ROM do MSX2

0099H SETATR

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setc", "", "BIOS-ROM", "0120H SETC   Set attribute of current pixel", "0120H SETC Set attribute of current pixel",
        R"MAN(AREA: BIOS principal (MainROM)

0120H SETC   Set attribute of current pixel

AREA: Sub-ROM do MSX2

009DH SETC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setgrp", "", "BIOS-ROM", "007EH SETGRP Set VDP to Graphics Mode", "007EH SETGRP Set VDP to Graphics Mode",
        R"MAN(AREA: BIOS principal (MainROM)

007EH SETGRP Set VDP to Graphics Mode

AREA: Sub-ROM do MSX2

00EDH SETGRP

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setmlt", "", "BIOS-ROM", "00F1H SETMLT", "00F1H SETMLT",
        R"MAN(AREA: Sub-ROM do MSX2

00F1H SETMLT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setpag", "", "BIOS-ROM", "013DH SETPAG", "013DH SETPAG",
        R"MAN(AREA: Sub-ROM do MSX2

013DH SETPAG

AREA: Sub-ROM do MSX2

013DH SETPAG

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setplt", "", "BIOS-ROM", "014DH SETPLT", "014DH SETPLT",
        R"MAN(AREA: Sub-ROM do MSX2

014DH SETPLT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setrd", "", "BIOS-ROM", "0050H SETRD  Set up VDP for read", "Standard routine to set up the VDP for subsequent reads from VRAM",
        R"MAN(AREA: BIOS principal (MainROM)

0050H SETRD  Set up VDP for read

Standard routine to set up the VDP for subsequent reads from VRAM
via the data port.

MSX1: addresses greater than 3FFFH will wrap around past zero as
the most 2 significant bits of the address are ignored.
Entry: | HL = read address
Exit: | none
Modifies: | AF, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("sets", "", "BIOS-ROM", "0179H SETS (Basic!)", "Sets BEEP, ADJUST, TIME and DATE",
        R"MAN(AREA: Sub-ROM do MSX2

0179H SETS (Basic!)

Sets BEEP, ADJUST, TIME and DATE
Entry: | HL = Basic textpointer
Exit: | HL = Adapted Basic textpointer
Modifies: | All ?

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setscr", "", "BIOS-ROM", "0189H SETSCR", "0189H SETSCR",
        R"MAN(AREA: Sub-ROM do MSX2

0189H SETSCR

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("sett32", "", "BIOS-ROM", "007BH SETT32 Set VDP to 32x24 Text Mode", "007BH SETT32 Set VDP to 32x24 Text Mode",
        R"MAN(AREA: BIOS principal (MainROM)

007BH SETT32 Set VDP to 32x24 Text Mode

AREA: Sub-ROM do MSX2

00E9H SETT32

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("settxt", "", "BIOS-ROM", "0078H SETTXT Set VDP to 40x24 Text Mode", "0078H SETTXT Set VDP to 40x24 Text Mode",
        R"MAN(AREA: BIOS principal (MainROM)

0078H SETTXT Set VDP to 40x24 Text Mode

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setwd3393", "", "BIOS-ROM", "7F83H SetWD3393", "Initialise SCSI controller.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F83H SetWD3393

Initialise SCSI controller.
Entry: | none
Exit: | none
Modifies: | All
Note: | none

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("setwrt", "", "BIOS-ROM", "0053H SETWRT Set up VDP for write", "Standard routine to set up the VDP for subsequent writes to VRAM",
        R"MAN(AREA: BIOS principal (MainROM)

0053H SETWRT Set up VDP for write

Standard routine to set up the VDP for subsequent writes to VRAM
via the data port.

MSX1: addresses greater than 3FFFH will wrap around past zero as
the most 2 significant bits of the address are ignored.
Entry: | HL = write address
Exit: | none
Modifies: | AF, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("sltmov", "", "Variaveis RAM", "F36E SLTMOV  inter slot move (HL/DE/BC)", "Copies the ROM to RAM: HL=source, DE=dest, BC=len. Only works during DOS.",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F36E SLTMOV  inter slot move (HL/DE/BC)

Copies the ROM to RAM: HL=source, DE=dest, BC=len. Only works during DOS.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("slttbl", "", "Variaveis RAM", "FCC5 SLTTBL  current expanded slot reg", "FCC5 DB 00H ; Primary slot 0",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCC5 SLTTBL  current expanded slot reg
FCC5         DB 00H     ; Primary slot 0
FCC6         DB 00H     ; Primary slot 1
FCC7         DB 00H     ; Primary slot 2
FCC8         DB 00H     ; Primary slot 3

These four variables duplicate the contents of the four possible Secondary Slot Registers.
The contents of each variable should only be regarded as valid if EXPTBL shows the
associated Primary Slot to be expanded.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("snddiag", "", "BIOS-ROM", "7FB3H SndDiag", "SEND DIAGNOSTIC.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7FB3H SndDiag

SEND DIAGNOSTIC.
Entry: | A = LUN + ID
Exit: | A = SCSI Status
D = Target Status
E = Message
Modifies: | ?
Note: | On MSX this function always performs a Self Test,
      see ANSI 7.2.1

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("snsmat", "", "BIOS-ROM", "0141H SNSMAT Read row of keyboard matrix", "Standard routine to read a complete row of the keyboard matrix. PPI port C is read in then...",
        R"MAN(AREA: BIOS principal (MainROM)

0141H SNSMAT Read row of keyboard matrix

Standard routine to read a complete row of the keyboard matrix. PPI port C is read in then written back with the row number occupying the four keyboard row select bits. PPI port B is then read into register A to return the eight column inputs. The four miscellaneous control outputs of PPI port C are unaffected by this routine.

Entry: | A=Keyboard row number
Exit: | A=Column data of keyboard row
Modifies: | AF, C, EI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("statfl", "", "Variaveis RAM", "F3E7 STATFL  Stores VDP status (MSX- VDP status register 0)", "This variable is continously updated by the interrupt handler with",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3E7 STATFL  Stores VDP status (MSX- VDP status register 0)

This variable is continously updated by the interrupt handler with
the contents of the VDP Status Register.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("stmotr", "", "BIOS-ROM", "00F3H STMOTR Turn motor ON/OFF", "Sets the cassette motor action.",
        R"MAN(AREA: BIOS principal (MainROM)

00F3H STMOTR Turn motor ON/OFF

Sets the cassette motor action.
Entry: | A  - #00 stop motor
           #01 start motor
           #FF reverse the current action
Modifies: | AF

Turbo-R: Returns without doing anything.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("storec", "", "BIOS-ROM", "0117H STOREC Store current pixel physical address", "0117H STOREC Store current pixel physical address",
        R"MAN(AREA: BIOS principal (MainROM)

0117H STOREC Store current pixel physical address

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("string output", "", "BDOS", "09 String Output", "09 String Output",
        R"MAN(AREA: BDOS (MSX-DOS)

09 String Output

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("strtms", "", "BIOS-ROM", "0099H STRTMS Start music dequeuing", "0099H STRTMS Start music dequeuing",
        R"MAN(AREA: BIOS principal (MainROM)

0099H STRTMS Start music dequeuing

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("strtstpun", "", "BIOS-ROM", "7FB0H StrtStpUn", "START STOP UNIT.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7FB0H StrtStpUn

START STOP UNIT.
Entry: | A = LUN + ID
B = 0 > Stop Unit
B = 1 > Start Unit
Exit: | A = SCSI Status
D = Target Status
E = Message
Modifies: | ?
Note: | See ANSI 8.2.17

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("subrom", "", "BIOS-ROM", "015CH SUBROM Call specified address in SUB-ROM", "This is the routine to call BASIC SUB-ROM especially. The",
        R"MAN(AREA: BIOS principal (MainROM)

015CH SUBROM Call specified address in SUB-ROM

This is the routine to call BASIC SUB-ROM especially. The
slot where SUB-ROM resides is automatically examined.
Normally, EXTROM, described below, is used.
Entry: | IX = address to be called
Exit: | depends on the result of the called program
Modifies: | background register and IX, IY registers are reserved

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("synchr", "", "BIOS-ROM", "0008H SYNCHR Check BASIC program character", "Standard routine to check the current program text character, whose address is supplied in...",
        R"MAN(AREA: BIOS principal (MainROM)

0008H SYNCHR Check BASIC program character

Standard routine to check the current program text character, whose address is supplied in register pair HL, agains a reference character. The reference character is supplied as single byte immediately following the CALL or RST instruction, for example:

    RST    8
    DEFB    ","

If the character does not match a "Syntax error" is generated, otherwise control transfers to the CHRGTR standard routine to fetch the next program character.

Entry: | HL=points to the character to check
Exit: | A=Next program character
Modifies: | AF, HL

AREA: Sub-ROM do MSX2

0008H SYNCHR (RST 08H)

This routine is equal to the MainROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("system reset", "", "BDOS", "00 System Reset", "Terminates the running program.",
        R"MAN(AREA: BDOS (MSX-DOS)

00 System Reset

Terminates the running program.
Entry: | none
Exit: | none
CP/M: | yes

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tapin", "", "BIOS-ROM", "00E4H TAPIN  Tape input", "Read data from the tape.",
        R"MAN(AREA: BIOS principal (MainROM)

00E4H TAPIN  Tape input

Read data from the tape.
Entry: | none
Exit: | A  - read value
      C-flag set if failed
Modifies: | All

Turbo-R: Normally this will set the clearflag and return with an error.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tapiof", "", "BIOS-ROM", "00E7H TAPIOF Tape input OFF", "Stops reading from the tape.",
        R"MAN(AREA: BIOS principal (MainROM)

00E7H TAPIOF Tape input OFF

Stops reading from the tape.

Turbo-R: Normally this will set the clearflag and return with an error.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tapion", "", "BIOS-ROM", "00E1H TAPION Tape input ON", "Reads the header block after turning the cassette motor on.",
        R"MAN(AREA: BIOS principal (MainROM)

00E1H TAPION Tape input ON

Reads the header block after turning the cassette motor on.
Entry: | none
Exit: | C-flag set if failed
Modifies: | All

Turbo-R: Normally this will set the clearflag and return with an error.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tapoof", "", "BIOS-ROM", "00F0H TAPOOF Tape output OFF", "Stops writing on the tape.",
        R"MAN(AREA: BIOS principal (MainROM)

00F0H TAPOOF Tape output OFF

Stops writing on the tape.

Turbo-R: Normally this will set the clearflag and return with an error.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tapoon", "", "BIOS-ROM", "00EAH TAPOON Tape output ON", "Turns on the cassette motor and writes the header.",
        R"MAN(AREA: BIOS principal (MainROM)

00EAH TAPOON Tape output ON

Turns on the cassette motor and writes the header.
Entry: | A  - #00 short header
       not #00 long header
Exit: | C-flag set if failed
Modifies: | All

Turbo-R: Normally this will set the clearflag and return with an error.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tapout", "", "BIOS-ROM", "00EDH TAPOUT Tape output", "Entry: | A - data to write",
        R"MAN(AREA: BIOS principal (MainROM)

00EDH TAPOUT Tape output

Entry: | A  - data to write
Exit: | C-flag set if failed
Modifies: | All

Turbo-R: Normally this will set the clearflag and return with an error.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tdownc", "", "BIOS-ROM", "010BH TDOWNC Test then DOWNC if legal", "010BH TDOWNC Test then DOWNC if legal",
        R"MAN(AREA: BIOS principal (MainROM)

010BH TDOWNC Test then DOWNC if legal

AREA: Sub-ROM do MSX2

00B1H TDOWNC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("termact", "", "BIOS-ROM", "7F86H TermAct", "Terminate HDD actions.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F86H TermAct

Terminate HDD actions.
Entry: | none
Exit: | A = SCSI STATUS
D = Target Status
E = Message
Modifies: | AF, D, E
Note: | none

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("terminate with error code", "", "BDOS", "62 Terminate with error code", "62 Terminate with error code",
        R"MAN(AREA: BDOS (MSX-DOS)

62 Terminate with error code

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("test file handle", "", "BDOS", "4C Test file handle", "4C Test file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

4C Test file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tleftc", "", "BIOS-ROM", "00A9H TLEFTC", "00A9H TLEFTC",
        R"MAN(AREA: Sub-ROM do MSX2

00A9H TLEFTC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("totext", "", "BIOS-ROM", "00D2H TOTEXT Return VDP to text mode", "00D2H TOTEXT Return VDP to text mode",
        R"MAN(AREA: BIOS principal (MainROM)

00D2H TOTEXT Return VDP to text mode

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("trgflg", "", "Variaveis RAM", "F3E8 TRGFLG  Stores trigger button status of joystick", "This variable is continously updated by the interrupt handler with",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F3E8 TRGFLG  Stores trigger button status of joystick

This variable is continously updated by the interrupt handler with
the state of the four joystick trigger inputs and the space key.

7 6 5 4 3 2 1 0
| | | |       +-- Space bar, trig(0) (0 = pressed)
| | | +---------- Stick 1, Trigger 1 (0 = pressed)
| | +------------ Stick 1, Trigger 2 (0 = pressed)
| +-------------- Stick 2, Trigger 1 (0 = pressed)
+---------------- Stick 2, Trigger 2 (0 = pressed)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tright", "", "BIOS-ROM", "00A1H TRIGHT", "00A1H TRIGHT",
        R"MAN(AREA: Sub-ROM do MSX2

00A1H TRIGHT

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("trptbl", "", "Variaveis RAM", "FC4C TRPTBL  DEFS 3     ;KEY 1", "FC4F DEFS 3 ;KEY 2",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FC4C TRPTBL  DEFS 3     ;KEY 1
FC4F         DEFS 3     ;KEY 2
FC52         DEFS 3     ;KEY 3
FC55         DEFS 3     ;KEY 4
FC58         DEFS 3     ;KEY 5
FC5B         DEFS 3     ;KEY 6
FC5E         DEFS 3     ;KEY 7
FC61         DEFS 3     ;KEY 8
FC64         DEFS 3     ;KEY 9
FC67         DEFS 3     ;KEY 10
FC6A         DEFS 3     ;STOP
FC6D         DEFS 3     ;SPRITE
FC70         DEFS 3     ;STRIG 0
FC73         DEFS 3     ;STRIG 1
FC76         DEFS 3     ;STRIG 2
FC79         DEFS 3     ;STRIG 3
FC7C         DEFS 3     ;STRIG 4
FC7F         DEFS 3     ;INTERVAL
FC82         DEFS 3     ;unused
FC85         DEFS 3     ;unused
FC88         DEFS 3     ;unused
FC8B         DEFS 3     ;unused
FC8E         DEFS 3     ;unused
FC91         DEFS 3     ;unused
FC94         DEFS 3     ;unused
FC97         DEFS 3     ;unused

These twenty-six the byte variables hold the current state of
the interrupt generating devices. The first byte of each entry
contains the device status (bit 0=On, bit 1=Stop, bit 2=Event
active) and is updated by the interrupt handler, the Runloop
interrupt processor and the "DEVICE ON/OFF/STOP" and "RETURN"
statement handlers. The remaining two bytes of each entry are
set by the "ON DEVICE GOSUB" statement handler and contain the
address of the program line to execute upon a program
interrupt.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tstunitrdy", "", "BIOS-ROM", "7FA1H TstUnitRdy", "TEST UNIT READY.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7FA1H TstUnitRdy

TEST UNIT READY.
Entry: | A = LUN + SCSI Target ID
Exit: | A = SCSI STATUS
D = Target Status
E = Message
Modifies: | AF,DE
Note: | See ANSI 7.2.16

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("ttypos", "", "Variaveis RAM", "F661 TTYPOS  Virtual cursor location internally retained by BASIC", "This variable is used by the \"PRINT\" statement handler to hold",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F661 TTYPOS  Virtual cursor location internally retained by BASIC

This variable is used by the "PRINT" statement handler to hold
the current screen position (Teletype!).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("tupc", "", "BIOS-ROM", "0105H TUPC   Test then UPC if legal", "0105H TUPC Test then UPC if legal",
        R"MAN(AREA: BIOS principal (MainROM)

0105H TUPC   Test then UPC if legal

AREA: Sub-ROM do MSX2

00B9H TUPC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("upc", "", "BIOS-ROM", "0102H UPC    Move current pixel physical address up", "0102H UPC Move current pixel physical address up",
        R"MAN(AREA: BIOS principal (MainROM)

0102H UPC    Move current pixel physical address up

AREA: Sub-ROM do MSX2

00BDH UPC

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("used in roman-kana translation", "", "Variaveis RAM", "FAF9 CHRCNT  Character counter in the buffer;", "used in Roman-Kana translation",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FAF9 CHRCNT  Character counter in the buffer;
             used in Roman-Kana translation
FAFA ROMA    Area to store character (Japan version only)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("usrtab", "", "Variaveis RAM", "F39A USRTAB  USR function starting addresses", "These ten variables contain the \"USR\" function addresses. Their values are set to the...",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F39A USRTAB  USR function starting addresses

These ten variables contain the "USR" function addresses. Their values are set to the interpreter
"Illegal function call" error generator at power-up and thereafter only altered by the "DEFUSR"
statement.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("vdp", "", "BIOS-ROM", "0161H VDP (Basic!)", "0161H VDP (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

0161H VDP (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("vdp.dr", "", "BIOS-ROM", "0006H VDP.DR", "One byte, VDP data port-number (read port)",
        R"MAN(AREA: BIOS principal (MainROM)

0006H VDP.DR

One byte, VDP data port-number (read port)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("vdp.dw", "", "BIOS-ROM", "0007H VDP.DW", "One byte, VDP data port-number (write port)",
        R"MAN(AREA: BIOS principal (MainROM)

0007H VDP.DW

One byte, VDP data port-number (write port)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("vdpf", "", "BIOS-ROM", "0165H VDPF (Basic!)", "0165H VDPF (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

0165H VDPF (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("vdpsta", "", "BIOS-ROM", "0131H VDPSTA", "0131H VDPSTA",
        R"MAN(AREA: Sub-ROM do MSX2

0131H VDPSTA

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("verify", "", "BIOS-ROM", "7FADH Verify", "VERIFY.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7FADH Verify

VERIFY.
Entry: | A = LUN + ID
B = Verification Length (Blocks)
C D E = 21 bit Logical Block #
HL = data address
Exit: | A = SCSI Status
D = Target Status
E = Message
Modifies: | AF,BC,HL,IX
Note: | none

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("vlzadr", "", "Variaveis RAM", "F419 VLZADR  DW 0000H  ADDRESS OF CHARACTER REPLACED BY VAL", "F41B VLZDAT DB 00H CHARACTER REPLACED WITH 0 BY VAL",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F419 VLZADR  DW 0000H  ADDRESS OF CHARACTER REPLACED BY VAL
F41B VLZDAT  DB 00H    CHARACTER REPLACED WITH 0 BY VAL

These variables contain the address and value of any character
temporarily removed by the "VAL" function.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("vpeek", "", "BIOS-ROM", "0175H VPEEK (Basic!)", "0175H VPEEK (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

0175H VPEEK (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("vpoke", "", "BIOS-ROM", "0171H VPOKE (Basic!)", "0171H VPOKE (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

0171H VPOKE (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("widths", "", "BIOS-ROM", "015DH WIDTHS (Basic!)", "015DH WIDTHS (Basic!)",
        R"MAN(AREA: Sub-ROM do MSX2

015DH WIDTHS (Basic!)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("winwid", "", "Variaveis RAM", "FCA5 WINWID", "FCA5 WINWID",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

FCA5 WINWID

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("write to file handle", "", "BDOS", "49 Write to file handle", "49 Write to file handle",
        R"MAN(AREA: BDOS (MSX-DOS)

49 Write to file handle

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("wrlogblk", "", "BIOS-ROM", "7F8CH WrLogBlk", "WRITE logical block(s) to HDD.",
        R"MAN(AREA: ROM de disco (Disk-ROM)

7F8CH WrLogBlk

WRITE logical block(s) to HDD.
Entry: | A = LUN + SCSI Target ID
B = # of blocks to write
C D E = 21 bit logical block #
HL = transfer address
Exit: | A = SCSI STATUS
D = Target Status
E = Message
Modifies: | AF, BC, DE, IX, IY, HL
Note: | See ANSI 8.2.5

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("wrprim", "", "Variaveis RAM", "F385 WRPRIM  Write to basic slot", "This routine is used by the WRSLT standard routine to switch Primary Slots and write a byte",
        R"MAN(AREA: Variavel da area de trabalho (RAM)

F385 WRPRIM  Write to basic slot

This routine is used by the WRSLT standard routine to switch Primary Slots and write a byte
to memory. The new Primary Slot Register setting is supplied in register A, the old setting
in register D and the byte to write in register E.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("wrres", "", "BIOS-ROM", "017DH WRRES Write Reset-state", "Specify if MSX is restarted by software-reset (JP 0000H) or by hardware-reset.",
        R"MAN(AREA: BIOS principal (MainROM)

017DH WRRES Write Reset-state

Specify if MSX is restarted by software-reset (JP 0000H) or by hardware-reset.

System reserved
Entry: | A = if bit7 is 0: hardware-reset, if bit7 is 1: software-reset
Exit: | ?
Modifies: | A

Refer to: Software-Reset

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("wrslt", "", "BIOS-ROM", "0014H WRSLT  Write to RAM in any slot", "Standard routine to write a single byte to memory in anyslot. Ist operation is fundamentally...",
        R"MAN(AREA: BIOS principal (MainROM)

0014H WRSLT  Write to RAM in any slot

Standard routine to write a single byte to memory in anyslot. Ist operation is fundamentally the same as that of the RDSLT standard routine except that the workspace area routine WRPRIM is used rather than RDPRIM.

Entry: | A=Slot ID
HL=Address
E=Byte to write
Exit: | none
Modifies: | AF, BC, D, DI

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("wrtclk", "", "BIOS-ROM", "01F9H WRTCLK write Clock-IC data", "write the contents of the A register in the CLOCK-IC at the",
        R"MAN(AREA: Sub-ROM do MSX2

01F9H WRTCLK write Clock-IC data

write the contents of the A register in the CLOCK-IC at the
address specified by the C register. The address is specified
in the format explained in the RTC documentation as REDCLK.
Entry: | C=address
Exit: | A=data
Modifies: | ?

Refer to: Clockchip (RTC)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("wrtpsg", "", "BIOS-ROM", "0093H WRTPSG Write to any PSG register", "0093H WRTPSG Write to any PSG register",
        R"MAN(AREA: BIOS principal (MainROM)

0093H WRTPSG Write to any PSG register

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("wrtvdp", "", "BIOS-ROM", "0047H WRTVDP Write to any VDP register", "Standard routine to write a data byte to any VDP mode register.",
        R"MAN(AREA: BIOS principal (MainROM)

0047H WRTVDP Write to any VDP register

Standard routine to write a data byte to any VDP mode register.
Entry: | B = data to write
C = VDP-register number
Exit: | none
Modifies: | AF, B, EI

AREA: Sub-ROM do MSX2

012DH WRTVDP

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("wrtvrm", "", "BIOS-ROM", "004DH WRTVRM Write byte to VRAM", "Standard routine to write a single byte to the VDP VRAM.",
        R"MAN(AREA: BIOS principal (MainROM)

004DH WRTVRM Write byte to VRAM

Standard routine to write a single byte to the VDP VRAM.
Entry: | HL = address to write
A = byte to write
Exit: | none
Modifies: | EI

AREA: Sub-ROM do MSX2

0109H WRTVRM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("wslreg", "", "BIOS-ROM", "013BH WSLREG Write to Primary Slot Register", "013BH WSLREG Write to Primary Slot Register",
        R"MAN(AREA: BIOS principal (MainROM)

013BH WSLREG Write to Primary Slot Register

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.

VER TAMBEM
  bios, sysvars, hooks)MAN");

    add("bios-doc-printer-esc-sequences", "", "Doc BIOS", "", "Documentation about ESC sequences specific to MSX- and Epson 9-pin printers (to print graphics...",
        R"MAN(Documentation about ESC sequences specific to MSX- and Epson 9-pin printers (to print graphics and control other printer properties):



MSX-Printers

This table lists all control-sequences, known for MSX-printers. This should give an overview, but must not be fully available on all MSX-printers. Control-sequences are typically preceded by ESC (CHR$(27), 1BH). A "n" within the code represents any ASCII-digit from 0-9.

Symbol | Code | Name
CR | 13 | Carriage return
LF | 10 | Carriage return + Line feed
VT | 11 | Vertical tabulator (like LF)
FF | 12 | Form feed
ESC, "N" | Font: Courier 1/10" (Pica)
ESC, "E" | Font: Courier 1/12" (Elite)
ESC, "Q" | Font: Courier 1/17" (Condensed)
ESC, "P" | Font: Proportional
SO | 14 | Double character-width on
SI | 15 | Double character-width off
ESC, "!" | Letter Quality on
ESC, 34 | (ESC, ") Letter Quality off
ESC, "CS" | Superscript on
ESC, "Cs" | Superscript off
ESC, "CU" | Subscript on
ESC, "Cu" | Subscript off
ESC, "CI" | Italic on
ESC, "Ci" | Italic off
ESC, "CB" | Bold on
ESC, "Cb" | Bold off
ESC, "CD" | Double strike on
ESC, "Cd" | Double strike off
HT | 9 | Horizontal tabulator
ESC, "(" | Set a horizontal tab position
ESC, ")" | Partially delete a horizontal tab position
ESC, "2" | Delete all horizontal tab positions
ESC, "O", "nnn" | Set page-height (lines)
ESC, "OI", "nn" | Set page-height(inches)
ESC, "OS", "nn" | Perforation-skip setting
ESC, "OS00" | Perforation-skip off
ESC, "\", "nnn" | Right border setting
ESC, "L", "nnn" | Left border setting
ESC, "A" | Line-feed 1/6"
ESC, "B" | Line-feed 1/9"
ESC, "T", "nn" | Line-feed nn/144"
ESC, "Z", "nn" | Line-feed nn/216"
ESC, "[" | Uni-directional printing
ESC, "]" | Bi-directional printing
ESC, "p" | PAPER OUT -Detector on
ESC, "q" | PAPER OUT - Detector off
ESC, 13 | (ESC, CR) Move printer-head to end-position
ESC, "@" | Printer reset
BEL | 7 | Audible beep (buzzer, 0.3s)
CAN | 24 | Clear printer buffer
ESC, "G", "nnn" | Graphic: Density nnn dots/inch
ESC, "S" | Graphic: Density depending on font
SOH | 1 | A symbolcode preceding code
ESC, "X" | Underline on
ESC, "Y" | Underline off
BS | 8 | Backstep (1 Character)
ESC, "r" | Scroll paper back
ESC, "f" | Scroll paper forward
ESC, "&" | Toggle International ASCII/Japanese
ESC, "$" | Toggle International ASCII/Japanese



Epson-Printers
This section lists only ESC-sequences for 9-pin printers. Available information is not complete yet, but provided as basic reference.
Symbol | Code | Name
Setting the page-format
ESC, "C" | Set page length in lines
ESC, "C", NUL | Set page length in inches
ESC, "N" | Set bottom margin
ESC, "O" | Cancel bottom margin
ESC, "Q" | Set right margin
ESC, "I" | Set left margin
Moving the print position
CR | 13 | Carriage return
LF | 10 | Line feed
FF | 12 | Form feed
ESC, "$" | Set absolute horizontal print position
ESC, "\" | Set relative horizontal print position
ESC, "J" | Advance print position vertically
HT | 9 | Tab horizontally
VT | 11 | Tab vertically
ESC, "f" | Horizontal/vertical skip
BS | 8 | Backspace
Setting the units
ESC, "0" | Select 1/8-inch line spacing
ESC, "2" | Select 1/6-inch line spacing
ESC, "3" | Set n/216-inch line spacing
ESC, "A" | Set n/72-inch line spacing
ESC, "1" | Set 7/72-inch line spacing
ESC, "D" | Set horizontal tabs
ESC, "B" | Set vertical tabs
ESC, "b" | Set vertical tabs in VFU channels
ESC, "f" | Select vertical tab channel
ESC, "e" | Set fixed tab increment
Selecting characters
ESC, "(t" | Assign character table
ESC, "t" | Select character table
ESC, "R" | Select an international character-set
ESC, "&" | Define user-defined characters
ESC, ":" | Copy ROM to RAM
ESC, "%" | Select user-defined set
ESC, "x" | Select NLQ or draft
ESC, "k" | Select tapeface
ESC, "P" | Select 10-cpi
ESC, "M" | Select 12-cpi
ESC, "g" | Select 15-cpi
ESC, "p" | Turn proportional mode on/off
ESC, SP | Set intercharacter space
ESC, "E" | Select bold font
ESC, "F" | Cancel bold font
ESC, "4" | Select italic font
ESC, "5" | Cancel italic font
ESC, "!" | Master select
ESC, "G" | Select double-strike printing
ESC, "H" | Cancel double-strike printing
ESC, "-" | Turn underline on/off
ESC, "S" | Select superscript/subscript printing
ESC, "T" | Cancel superscript/subscript printing
SI | 15 | Select condensed printing
ESC, SI | Select condensed printing
DC2 | Cancel condensed printing
SO | 14 | Select double-width printing (one line)
ESC, SO | Select double-width printing (one line)
DC4 | Cancel double-width printing
ESC, "W" | Turn double-width printing on/off
ESC, "w" | Turn double-width printing on/off
Control-code character printing
ESC, "6" | Enable printing of upper control-codes
ESC, "7" | Enable upper control-codes
ESC, "l" | Enable printing of control-codes
ESC, "m"" | Select printing of upper control-codes
Mechanical control
ESC, "EM" | Control paper loading/ejecting
ESC, "U" | Turn unidirectional mode on/off
ESC, "<" | Unidirectional mode (one line)
BEL | 7 | Beeper
ESC, "8" | Disable paper-out detector
ESC, "9" | Enable paper-out detector
ESC, "s" | Select low-speed mode
Printing color and graphics
ESC, "*" | Select bit image
ESC, "?" | Reassign bit-image mode
ESC, "K" | Select 60-dpi graphics
ESC, "L" | Select 120-dpi graphics
ESC, "Y" | Select 120-dpi, double-speed graphics
ESC, "Z" | Select 240-dpi graphics
ESC, "^" | Select 60/120-dpi, 9-pin graphics
ESC, "r" | Select printing color
Printing bar codes
ESC, "(B" | Bar code setup and print
Data and memory control
ESC, "@" | Initialize printer
CAN | 24 | Cancel line
DEL | Delete last character in buffer
DC1 | Select printer
DC3 | Deselect printer
ESC, "#" | Cancel MSB control
ESC, "=" | Set MSB to 0
ESC, ">" | Set MSB to 1
Deleted commands
ESC, "j" | Reverse paper feed
ESC, "i" | Select immediate print mode

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("bios-doc-reset", "", "Doc BIOS", "", "* This document concerns MSXes from 1. Note turboR was not released when this document was...",
        R"MAN(* This document concerns MSXes from 1. Note turboR was not released when this document was authored.

2.2.5 Software Reset

If you reset an MSX by jumping to 0 address (this is called a software reset) you might disable kanji ROM. Device enable function is the cause of this symptom.

Device enable is a name of function to prevent bus conflict. For example, if you connect an external kanji ROM to a MSX equipped with internal kanji ROM, this means two devices are stuck into one I/O port, therefore causing conflicts between signals coming from these kanji ROMs, and in worst case, MSX itself will crush. To prevent this, F5H of I/O port is supplied as device enable function, and it works as follows.

When you turn power of an MSX on, MSX assumes its internal kanji ROM as disconnected, and BIOS checks kanji ROM status. Since internal kanji ROM is disconnected, if MSX detects a kanji ROM presence, the detected kanji ROM should be an external one.

If no external kanji ROM was found, MSX writes 1 to bit 0 of F5H I/O port and enables internal kanji ROM (refer to page 391 of MSX2 Technical Handbook - I don't know correspondent page no. in English version;). These procedures so far allow you to insert an external kanji ROM into an MSX with internal kanji ROM. RS-232C is supported by similar function too.

But, if a program jumps to 0 address through commands like JP 0H (C3 00 00), MSX will go to the rountine to check kanji ROM status, while assuming internal kanji ROM is connected. When this happens, MSX thinks its internal kanji ROM as external one, and disconnects it. There is no problem if an MSX goes to the routine to check external device at the time of power on and pressing a reset button, but if you want to let a program to reset, in another word, if you want to jump to 00H, you must perfrom some preventive measure. Practical method is as follows;

* If you want to let a software to reset, execute the following.

LD   A,00H
OUT (F5H),A
JP  0H

Basically, as MSX2 Technical Handbook insists, users are forbidden to read or write F5H of I/O, so this is not a very appreciated method, but there are no other means to abort the problem. Note even this procedure cannot solve misguided execution; in that case, it means the I/O F5H of your MSX operates dissimilar to other MSXes. (This is possible to happen, since direct accesses to F5H are not in MSX developer's mind)

Note, with MSX2+, external devices are always disconnected by writing 0 into F5H I/O, so you can safely write 0.

Also, since MSX2+, you can check reset status. Use added BIOS to do so.

* If you want to check reset status with MSX2+, do as follows.

CALL 017AH

After doing this, if bit 7 of returned A register is standing, you can learn that MSX was restarted by JP 0H reset (software reset). If the bit 7 is not standing, it means an MSX was resetted by hardware reset. I guess this function was added because if software cannot detect between hardware reset and software reset, many troubles are likely to happen. In fact, a hardware to check reset is added to F4H of I/O, and inside of the BIOS here reads and write the F4H port.

To safely use this function, you must let a software which is going to execute software reset to write a value. Use a BIOS which was newly implemented since MSX2+, too.

* If you want to reset an MSX2+, do as follows.

CALL 017AH  ; Write current reset status into A register.
OR   80H    ; Set reset flag.
CALL 017DH  ; Write a new value.
JP   0H     ; Jump to address 0.

Note, there is no way to detect between software reset and hardware reset with MSX1 and MSX2.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    return v;
}

std::vector<Topic> HardwareTopics() {
    std::vector<Topic> v;
    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {
        v.push_back(Topic{name, aliases, category, usage, summary, page, false});
    };

    add("clockchip-rtc", "hw-rtc,rtc", "Hardware", "", "MSX-BIOS: Clockchip (RTC): MSX2 uses a CLOCK-IC to for its timer function. Since this IC...",
        R"MAN(HARDWARE: Clockchip (RTC)
MSX-BIOS: Clockchip (RTC)

MSX2 uses a CLOCK-IC to for its timer function. Since this IC is
battery-powered, it remains active even after MSX2 is turned off. MSX2 uses a
small amount of RAM inside to set the PASSWORD or to set the screen mode at
startup automatically, in addition to the CLOCK functions.

1. CLOCK-IC Functions

This IC has the following three functions:

CLOCK function

set/read the settings of "year, month, day, day of week, hour, minute, second"
for the expression of time, 24-hour clock/12-hour clock available
for months, months of 31 days and of 30 days are distinguished
(leap years are also recognised)

Alarm function

when the time for alarm is set, CLOCK generates signals at that time.
the time for alarm is set as "XXday XXhour XXminute".

Battery-powered memory function

has 26 sets of 4-bit memory, and can be battery-powered.
MSX2 stores the following data in this memory:

    1. adjustment value of CRT display width and height

    2. initial values of SCREEN, WIDTH, colour

    3. BEEP tone and volume

    4. title screen colour

    5. country code

    6. password      --+

    7. BASIC prompt    | (one of 6 to 8)

    8. title caption      --+

2. Structure of the CLOCK-IC

The CLOCK-IC has four blocks inside as shown in Figure 1. Each block
consists of 13 sets of 4-bit registers, which are specified by addresses from
0 to 12. In addition, it has three 4-bit registers for selecting the block or
controlling functions; they are specified by the addresses from 13 to 15.

The registers inside the block (#0 to #12) and the MODE register (#13) can be
read from and written to. The TEST register (#14) and RESET register (#15)
can only be written to.

   Figure 1    Clock IC structure

       BLOCK 0          BLOCK 1         BLOCK 2        BLOCK 2
       (CLOCK)          (ALARM)         (RAM-1)        (RAM-2)
   ----------------   ----------------     ----------------   ----------------
   | Seconds (the |   |          |     |        |   |           |
 0 | 1st decimal  |   |   ________   |     |        |   |           |
   | place)      |   |          |     |        |   |           |
   |--------------|   |--------------|     |-           -|   |-          -|
   | Seconds (the |   |          |     |        |   |           |
 1 | 2nd decimal  |   |   ________   |     |        |   |           |
   | place)      |   |          |     |        |   |           |
   |--------------|   |--------------|     |-           -|   |-          -|
 . |      .      |   |      .         |     |   Any data    |   |    Any data   |
 . |      .      |   |      .         |     |        |   |           |
 . |      .      |   |      .         |     |        |   |           |
 . |      .      |   |      .         |     |        |   |           |
 . |      .      |   |      .         |     |        |   |           |
   |--------------|   |--------------|     |-           -|   |-          -|
   | Year (the      |   |          |     |        |   |           |
 12| 2nd decimal  |   |   ________   |     |        |   |           |
   | place)      |   |          |     |        |   |           |
   ----------------   ----------------     ----------------   ----------------
   ::   ::     ::   ::

   ----------------
13 |     MODE      |
   |--------------| --+
14 |     TEST      |   |
   |--------------|   |-- Write only
15 |     RESET      |   |
   ---------------- --+
   ::

3. MODE Register Functions

The MODE register has the following 3 functions:

Selecting block

To read from or write to registers from #0 to #12, select the block to be
used and then access the objective address. The 2 low order bits of the MODE
register are used to select the block.

Registers from #13 to #15 are accessible whichever block is selected.

Alarm output ON/OFF

To switch the alarm input ON/OFF, use bit 2 of the MODE register. Since the
standard MSX2 does not support the alarm, modifying this bit causes nothing
to happen in general.

Terminating CLOCK count

By writing "0" in bit 3 of the MODE register, the count in seconds is stopped
(the stages before the seconds are not stopped) and the clock function is
terminated. By writing "1" in bit 3, the count is resumed.

   Figure 2    MODE register functions

  B3   B2   B1     B0
---------------------
| TE | AE | M1 : M0 |  MODE register (#13)
---------------------
  |    |       |       00: select block 0
  |    |       |       01: select block 1
  |    |       +---->  10: select block 2
  |    |           11: select block 3
  |    |
  |    +------------>  0:  alarm output OFF
  |               1:  alarm output ON
  |
  +----------------->  0:  CLOCK count stop (in seconds)
               1:  CLOCK count start

4. TEST Register functions

The TEST register (#14) is used to increment the upper counter quickly and to
confirm that date and time carries are done correctly. Setting "1" in each
bit of the register, the pulse of 2^14 (=16384)[Hz] is directly set in day,
hour, minute, and second counters.

   Figure 3    TEST register functions

  B3   B2   B1     B0
---------------------
| T3 | T2 | T1 | T0 |  TEST register (#14)
---------------------
  |    |    |     |
  |  Hours  |  Seconds ........ the location for the pulse to be placed
 Day      Minutes

5. RESET Register Functions

The RESET register (#15) has the following functions:

Resetting the alarm

Setting "1" in bit 0 causes all alarm registers to be reset to 0.

Setting the seconds

Setting "1" in bit 1 causes the stage before the seconds to be reset. Use
this function to set the seconds correctly.

Clock pulse ON/OFF

Setting "1" in bit 2 turns the 16Hz clock pulse output ON, and setting "0" in
bit 3 turns the 1Hz clock pulse output ON. Note that both are not supported
by the MSX2 standard.

   Figure 4    RESET register function

  B3   B2   B1     B0
---------------------
| C1 | C16| CR | AR |  RESET register (#15)
---------------------
  |    |    |     |
  |    |    |     +-->  When "1", all alarm registers are reset
  |    |    +------->  When "1", fractions smaller than a second are reset
  |    +------------>  When "0", 16[Hz] clock pulse is ON
  +----------------->  When "0", 1[Hz] clock pulse is ON

6. Setting the Clock and Alarm

Setting date and time

Block 0 is used to set the clock. Selecting block 0 in the MODE register
and writing data in the objective register causes the date and the time to be
set. The current time is acquired by reading the contents of the register.
See Figure 5 for the meaning of the register and its address.

Block 1 is used to set the alarm. Note that the time of the alarm can be set
only in days, hours, and minutes. Nothing happens, in general, when the time
of the clock meets the time of the alarm.

In the clock, the year is represented by 2 digits (registers #11 and #12). In
MSX-BASIC, the 2 low order digits of the year is represented by adding the
offset 80 to this value. For example, after setting register #11 to 0 and
register #12 to 0, the year would be 80, as "80/XX/XX", when the date is read
by using the GET DATE instruction of BASIC.

The day of the week is represented by 0 to 6. This is only a mod 7 counter
which is renewed alomg with the date, and the correspondence between the
actual day of the week and the number value 0 to 6 is not defined.

   Figure 5    Setting the CLOCK and ALARM

               block 0 : CLOCK
   ---------------------------------------------------------
   |                   |  B3  |  B2  |    B1  |  B0  |
   |---------------------------+---------------------------|
 0 |  Seconds               |               |
   |  (the 1st decimal place)  |  X     X    X      X   |
   |---------------------------+---------------------------|
 1 |  Seconds               |               |
   |  (the 2nd decimal place)  |  .     X    X      X   |
   |---------------------------+---------------------------|
 2 |  Minutes               |               |
   |  (the 1st decimal place)  |  X     X    X      X   |
   |---------------------------+---------------------------|
 3 |  Minutes               |               |
   |  (the 2nd decimal place)  |  .     X    X      X   |
   |---------------------------+---------------------------|
 4 |  Hours               |               |
   |  (the 1st decimal place)  |  X     X    X      X   |
   |---------------------------+---------------------------|
 5 |  Hours               |               |
   |  (the 2nd decimal place)  |  .     .    X      X   |
   |---------------------------+---------------------------|
 6 |  Day of               |               |
   |  the week               |  .     X    X      X   |
   |---------------------------+---------------------------|
 7 |  Day               |               |
   |  (the 1st decimal place)  |  X     X    X      X   |
   |---------------------------+---------------------------|
 8 |  Day               |               |
   |  (the 2nd decimal place)  |  .     .    X      X   |
   |---------------------------+---------------------------|
 9 |  Month               |               |
   |  (the 1st decimal place)  |  X     X    X      X   |
   |---------------------------+---------------------------|
10 |  Month               |               |
   |  (the 2nd decimal place)  |  .     .    .      X   |
   |---------------------------+---------------------------|
11 |  Year               |               |
   |  (the 1st decimal place)  |  X     X    X      X   |
   |---------------------------+---------------------------|
12 |  Year               |               |
   |  (the 2nd decimal place)  |  X     X    X      X   |
   ---------------------------------------------------------

               block 1 : ALARM
   ---------------------------------------------------------
   |                   |  B3  |  B2  |    B1  |  B0  |
   |---------------------------+---------------------------|
 0 |     _________________     |               |
   |                   |  .     .    .      .   |
   |---------------------------+---------------------------|
 1 |     _________________     |               |
   |                   |  .     .    .      .   |
   |---------------------------+---------------------------|
 2 |  Minutes               |               |
   |  (the 1st decimal place)  |  X     X    X      X   |
   |---------------------------+---------------------------|
 3 |  Minutes               |               |
   |  (the 2nd decimal place)  |  .     X    X      X   |
   |---------------------------+---------------------------|
 4 |  Hours               |               |
   |  (the 1st decimal place)  |  X     X    X      X   |
   |---------------------------+---------------------------|
 5 |  Hours               |               |
   |  (the 2nd decimal place)  |  .     .    X      X   |
   |---------------------------+---------------------------|
 6 |  Day of               |               |
   |  the week               |  .     X    X      X   |
   |---------------------------+---------------------------|
 7 |  Day               |               |
   |  (the 1st decimal place)  |  X     X    X      X   |
   |---------------------------+---------------------------|
 8 |  Day               |               |
   |  (the 2nd decimal place)  |  .     .    X      X   |
   |---------------------------+---------------------------|
 9 |     _________________     |               |
   |                   |  .     .    .      .   |
   |---------------------------+---------------------------|
10 |  12 or               |               |
   |  24 hours               |  .     .    .      X   |
   |---------------------------+---------------------------|
11 |  Leap year            |               |
   |  counter               |  .     .    X      X   |
   |---------------------------+---------------------------|
12 |     _________________     |               |
   |                   |  .     .    .      .   |
   ---------------------------------------------------------

    Bits indicated by an "." are always 0 and cannot be modified.

Selecting 12-hour clock/24-hour clock

Two clocks can be selected; one is a 24-hour clock which represents one
o'clock in the afternoon as 13 o'clock, and the other is a 12-hour clock
which represents it as 1 p.m. Register #10 is used to select between them. As
shown in Figure 6, the 12-hour clock is selected when B0 is "0" and the
24-hour clock when B0 is "1".

   Figure 6    Selecting 12-hour clock/24-hour clock

  B3   B2   B1     B0
---------------------
|  . |    . |  . | B0 |  Register #10 (block 1)
---------------------
         |
         +-->  0: 12-hour clock
               1: 24-hour clock

   Figure 7    Morning/afternoon flag for 12-hour clock

  B3   B2   B1     B0
---------------------
|  . |    . | B1 |  X |  register #5 (block 0)
---------------------
        |
        +------->  0: before noon
               1: after noon

Leap year counter

Register #11 of block 1 is a mod 4 counter which is renewed along with the
count of the year. When the 2 low order bits of this register are 00H, that
is considered as a leap year and 29 days are counted in February.

   Figure 8    Leap year determination

  B3   B2   B1     B0
---------------------
|  . |    . | B1 | B0 |  Register #11 (block 1)
---------------------
        |     |
        +----+-->  Both bits 0 represents leap year.

7. Contents of the Battery-powered Memory

Blocks 2 and 3 of the CLOCK-IC are used as the battery-powered 4-bit x 13
memory blocks. MSX2 uses this area as shown below.

Contents of block 2

   Figure 9    Contents of block 2

   |       B3        |        B2         |         B1       |       B0       |
   |-------------------------------------------------------------------|
 0 |                     ID                    |
   |-------------------------------------------------------------------|
 1 |                Adjust X (-8 to +7)                |
   |-------------------------------------------------------------------|
 2 |                Adjust Y (-8 to +7)                |
   |-------------------------------------------------------------------|
 3 |   __________   |    __________   | Interlace mode |  Screen mode   |
   |-------------------------------------------------------------------|
 4 |                 WIDTH value (Lo)                   |
   |-------------------------------------------------------------------|
 5 |                 WIDTH value (Hi)                   |
   |-------------------------------------------------------------------|
 6 |                 Foreground color                   |
   |-------------------------------------------------------------------|
 7 |                 Background color                   |
   |-------------------------------------------------------------------|
 8 |                   Border color                   |
   |-------------------------------------------------------------------|
 9 | Cassette speed |  Printer mode  |     Key click    |   Key ON/OFF   |
   |---------------------------------+---------------------------------|
10 |        BEEP tone         |         BEEP volume           |
   |---------------------------------+---------------------------------|
11 |   __________   |    __________   |         Title colour           |
   |-------------------------------------------------------------------|
12 |                Native code                   |
   ---------------------------------------------------------------------

Contents of block 3

Block 3 has three functions, depending on the contents of the ID value
(register #0). Figure 10 shows the functions.

   Figure 10    Contents of block 3

ID=0: displays the title (within 6 characters) on the initial screen
   ------------------------------------------------------
 0 |  0                         |
   |----------------------------------------------------| --+
 1 |  Lo 1 --+--- 1st character of the title        |   |
   |         |                        |   |
 2 |  Hi 1 --+                        |   |
   |----------------------------------------------------|   |
 . |       .                        |   |
 . |       .                        |   | 6 characters
 . |       .                        |   |
   |----------------------------------------------------|   |
11 |  Lo 6 --+--- 6th character of the title        |   |
   |         |                        |   |
12 |  Hi 6 --+                        |   |
   ------------------------------------------------------ --+

ID=1: sets the password
   ------------------------------------------------------
 0 |  1                         |
   |----------------------------------------------------|
 1 |  Usage ID=1                    |
   |                            |
 2 |  Usage ID=2                    |
   |                            |
 3 |  Usage ID=3                    |
   |----------------------------------------------------|
 4 |  Password    --+                    |
   |          |                    |
 5 |  Password      |   Password data is stored        |
   |          |-- compressed in 4bits x 4 bits    |
 6 |  Password      |                    |
   |          |                    |
 7 |  Password    --+                    |
   |----------------------------------------------------|
 8 |  Key cartridge flag                |
   |----------------------------------------------------|
 9 |  Key cartridge value                |
   |                            |
10 |  Key cartridge value                |
   |                            |
11 |  Key cartridge value                |
   |                            |
12 |  Key cartridge value                |
   ------------------------------------------------------

ID=2: sets the prompt on BASIC
   ------------------------------------------------------
 0 |  2                         |
   |----------------------------------------------------| --+
 1 |  Lo 1 --+--- 1st character of the prompt        |   |
   |         |                        |   |
 2 |  Hi 1 --+                        |   |
   |----------------------------------------------------|   |
 . |       .                        |   |
 . |       .                        |   | 6 characters
 . |       .                        |   |
   |----------------------------------------------------|   |
11 |  Lo 6 --+--- 6th character of the prompt        |   |
   |         |                        |   |
12 |  Hi 6 --+                        |   |
   ------------------------------------------------------ --+

8. Access to the CLOCK-IC

The following BIOS routines are offered to access the clock and the
battery-powered memory. Since these routines reside in SUB-ROM, they are
called by using the inter-slot call:

REDCLK (01F5H/SubROM) - read Clock-IC data

WRTCLK (01F9H/SubROM) - write Clock-IC data

Refer to: MSX2-Subrom

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("psg-programmable-sound-generator", "hw-psg", "Hardware", "", "PSG (Programmable Sound Generator): As well as controlling the three sound channels the...",
        R"MAN(HARDWARE: PSG
PSG (Programmable Sound Generator)

As well as controlling the three sound channels the 8910 PSG contains two eight bit data ports, called A and B, through which it interfaces the joysticks and the cassette input. The PSG appears to the Z80 as three I/O ports called Adresses Port, the Data Write Port and the Data Read Port.

A0 | O | adress port
A1 | O | data write-port
A2 | I | data read-port

Adress Port

The PSG contains sixteen internal registers which completely define it's operation. A specific register is selected by writing ist number, from 1 to 15, to this port. Once selected, repeated accesses to that register may be made via the two data ports.

Data Write Port

This port is used to write to any register once it has been selected by the Adress port.

Data Read Port

This port is used to read any register once it has been selected by the Adress port.

Registers 0 and 1:

+--------------------------------------------------+
|            Channel A Frequency LSB               | R0
+--------------------------------------------------+
|  x  |  x  |  x  |  x  |  Channel A Frequency MSB | R1
+--------------------------------------------------+

These two registers are used to define the frequency of the Tone Generator for Channel A. Variable frequencies are produced by dividing a fixed master frequency with the number held in Register 0 and 1, this number can be in range 1 to 4095. Register 0 holds the least significant eight bits and Register 1 the most significant four. The PSG divides an external 1,7897725MHz frequency of 111.861Hz. The output of the Tone Generator can therefore range from 111.861Hz  (divide by 1) down to 27.3Hz (divide by 4095). As an example to produce a middle "A" (440Hz) the divider value in Registers 0 and 1 would be 254.

Registers 2 and 3:

These two registers control the channel B Tone Generator as for Channel A.

Registers 4 and 5:

These two registers control the channel B Tone Generator as for Channel A.

Register 6:

+--------------------------------------------------+
|  x  |  x  |  x  |        Noise Frequency         |
+--------------------------------------------------+

In addition to three square wave Tone Generators the PSG contains a single Noise Generator. The fundamental frequency of the noise source can be controlled in a similar fashion to the Tone Generators. The five least significant bits of Register 6 hold a divider value from 1 to 31. The Noise Generator master frequency is 111.861 as before.

Register 7:

+--------------------------------------------------------------------+
| Port  | Port  | C     | B      | A      | C      | B      | A      |
| B Dir | A Dir | Noise | Noise  | Noise  | Tone   | Tone   | Tone   |
+--------------------------------------------------------------------+

This register enables or disables the Tone Generator and Noise Generator of each of the three channels: 0=Enable, 1=Disable. It also controls the direction of interface ports A and B, to which the joysticks and cassette are attached: 0=Input, 1=Output. Register 7 must always contain 10xxxxxx or possible damage could result to the PSG, there are active devices connected to the I/O pins. The BASIC "SOUND" statement will force these bits to the correct value for Register 7 but there is no protection at the machine code level.

Register 8:

+--------------------------------------------------+
|  x  |  x  |  x  | Mode|   Channel A Amplitude    |
+--------------------------------------------------+

The four Amplitude bits determine the amplitude of Channel A from a minimum 0 to a maximum 15. The Mode bit selects either fixed or modulated amplitude: 0=Fixed, 1=Modulated. When modulated amplitude is selected the fixed amplitude value is ignored and the channel is modulated by the output from the Envelope Generator.

Register 9:

This register controls the Aplitude of Channel B as for Channel A.

Register 10:

This register controls the Aplitude of Channel C as for Channel A.

Registers 11 and 12:

+--------------------------------------------------+
|              Envelope Frequency LSB              | R11
+--------------------------------------------------+
|              Envelope Frequency MSB              | R12
+--------------------------------------------------+

These two registers control the frequency of the single Envelope Generator used for amplitude modulation. As for the Tone Generators this frequency is determined by placing a divider count in the registers. The divider value may range from 1 to 65535 with Register 11 holding the least significant eight bits and Register 12 the most significant. The master frequency for the Envelope Generator is 6991Hz so the envelope frequeny may range from 6991Hz (divide by 1) to 0,11Hz (divide by 65536).

Register 13:

+--------------------------------------------------+
|     |     |     |     |     Envelope Shape       |
+--------------------------------------------------+

The four Envelope Shape bits determine the shape of the amplitude modulation envelope produced by the Envelope Generator.

Register 14:

+--------------------------------------------------------------------+
| Cas   | Kbd   | Joy   | Joy    | Joy    | Joy    | Joy    | Joy    |
| Input | Mode  | Trg.B | Trg.A  | Right  | Left   | Back   | Fwd    |
+--------------------------------------------------------------------+

This register is used to read in PSG Port A.The six joystick bits reflect the state of the four direction switches and two trigger buttons on a joystick: 0=Pressed, 1=Not pressed. Alternatively up to six Paddles may be connected instead of one joystick. Although most MSX machines have two 9 pin joystick connectors only one can be read at a time. The one to be selected for reading is determined in the Joystick select bit in PSG Register 15.

The Keyboard mode bit is unused on UK machines. On Japanese machines it is tied to a jumper link to determine the keyboard's character set.

The Casette Input is used to read the signal from the cassette EAR output. This is passed through a comparator to clean the edges and to convert to digital levels but is otherwise uncompressed.

Register 15:

+--------------------------------------------------------------------+
| Kana  | Joy   | Pulse | Pulse  |   1    |   1    |   1    |   1    |
| LED   | Sel   |   2   |    1   |        |        |        |        |
+--------------------------------------------------------------------+

This register is used to output to PSG port B. The four least significant bits are connected via TTL open-collector buffers to pins 6 and 7 of each joystick connector. They are normally set 1, when a paddle or joystick is connected, so that the pins are can function as inputs. When a touchpad is connected they are used as handshaking outputs.

The two Pulse bits are used to generate a short positive-going pulse to any paddles attached to joystick connectors 1 or 2. Each paddle contains a monostable timer with a variable resistor controlling it's pulse length. Once the timer is triggered the position of the variable resistor can be determined by counting until the monostable times out.

The Joystick Select bit determines which joystick connector is connected to PSG Port A for input: 0=Connector 1, 1=Connector 2.

The KanaLED output is unused on UK machines. On Japanese machines it is used to drive a keyboard mode indicator.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("scc", "hw-scc", "Hardware", "", "MSX-BIOS: SCC: To use SCC:",
        R"MAN(HARDWARE: SCC
MSX-BIOS: SCC

To use SCC:

Write xx111111b at address 9000-97FFh on a slot where SCC exist,
then SCC appears at 9800-9FFFh.

9800-981Fh | wave memory 0(ch.A)(32 * signed char)
9820-983Fh | wave memory 1(ch.B)
9840-985Fh | wave memory 2(ch.C)
9860-987Fh | wave memory 3(ch.D,E)
9880-9889h | frequency of ch.A-E(12bit)F=CLOCK/32/(1+nn)
9880-9889h | frequency of ch.A-E(12bit)F=CLOCK/32/(1+nn)
988A-988Eh | output level of ch.A-E(max 15)
988Fh | output switch(1 is ON)(b0:ch.A,...,b4:ch.E)
98E0-98FFh | test?write 0 is to initialize, 1 is to be silent.
9800-987Fh | be able to read/write.
9880-988Fh | write only.
9890-98DFh | dummy.
98E0-98FFh | write only.

Wave data is waveform of the sound itself.

ex: square wave(7Fh is 127, 80h is -128)

7f 7f 7f 7f 80 80 80 80 7f 7f 7f 7f 80 80 80 80...

ref: 'MCTR-SCC.TXT' by TSUJIKAWA.K.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("msx-music-opll-fm-pac", "hw-opll,opll,msx-music,fmpac", "Hardware", "", "MSX-BIOS: MSX-Music (OPLL, FM-PAC): To use OPLL (FM-PAC):",
        R"MAN(HARDWARE: MSX-Music (OPLL, FM-PAC)
MSX-BIOS: MSX-Music (OPLL, FM-PAC)

To use OPLL (FM-PAC):

I/O port | 7Ch | OPLL register address.
7Dh | OPLL data address.

3.36 micro seconds wait needed after writing 7Ch.

23.52 micro seconds wait needed after writing 7Dh.

OPLL registers:

reg. |  b7   |  b6   |  b5   |  b4   |  b3   |  b2   |  b1   |  b0   |
00h  |  AM   |  VIB  | EGTYP |  KSR  |          MULTIPLE             |  (MOD)
01h  |  AM   |  VIB  | EGTYP |  KSR  |          MULTIPLE             |  (CAR)
02h  |      KSL      |               TOTAL LEVEL                     |  (MOD)
03h  |      KSL      |       |  DM   |  DC   |      FEEDBACK         |  (CAR)
04h  |           ATTACK RATE         |          DECAY RATE           |  (MOD)
05h  |           ATTACK RATE         |          DECAY RATE           |  (CAR
06h  |          SUSTAIN LEVEL        |        RELEASE RATE           |  (MOD)
07h  |          SUSTAIN LEVEL        |        RELEASE RATE           |  (CAR)
0Eh  |       |       | RHYTHM|  BD   |  SD   |  TOM  |TOP-CY |  HH   |
10h- |                         F-Number(b7-b0)                       |  ch.A-
18h  |                         F-Number(b7-b0)                       |  ch.I
20h- |       |       |SUS-ON |KEY-ON |         BLOCK         |F-Num  |  ch.A-
28h  |       |       |SUS-ON |KEY-ON |         BLOCK         |F-Num  |  ch.I
30h- |            INST               |           VOL                 |  ch.A-
38h  |            INST               |           VOL                 |  ch.I

when RHYTHM mode:

36h  |       |       |       |       |          BASS DRUM LEVEL      |
37h  |         HIHAT LEVEL           |         SNARE DRUM LEVEL      |
38h  |         TOM-TOM LEVEL         |         TOP-CYMBAL LEVEL      |

VIB: | frequency vibration ON/OFF.
AM: | amplitude moduration ON/OFF.
EG-TYP: | 1 - DECAYING envelop pattern, 0 - SUSTAINING envelop pattern.
DM: | rectification to output of modulator opelator ON/OFF.
DC: | rectification to output of carrier opelator ON/OFF.
RHYTHM: | RHYTHM mode ON/OFF. (ch.G,H,I is used for RHYTHM.)
BLOCK: | octave data
F-Number: | (frequency * 2^18 / 50000) / 2^(BLOCK-1)
INST: | OPLL voice number.

     0 User           1 Violin         2 Guitar         3 Piano
     4 Flute          5 Clarinet       6 Oboe           7 Trumpet
     8 Organ          9 Tube          10 Synthesizer   11 Harpsicode
    12 Vibraphone    13 Synth bass    14 Wood bass     15 Elec. bass

ref: MSX2+ powerful katsuyouhou(ASCII pub.) by SUGIYA Seiichi

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("msx-audio", "hw-msx-audio,y8950", "Hardware", "", "MSX-Audio: The MSX-Audio standard was defined for MSX 2 computers, but the hardware was...",
        R"MAN(HARDWARE: MSX-Audio
MSX-Audio

The MSX-Audio standard was defined for MSX 2 computers, but the hardware was too expensive at this time. Therefore this cartridge was separately sold (also for use in MSX 1 computers). The MSX-Audio is hardware compatible with the Philips Music Module (please notice that the MSX-Audio provides a special BIOS, where the Music Module uses special firmware only).

C0 | I/O | I | statusregister read-port
O | register select write-port
C1 | O | O | data write-port

Because of the nature of the OPLL2 you must wait 3.3usec after a register select write and 23usec after a data write to the selected register.

Register Map of the FM Synthesizer

Status-Register:

+------+------+------+------+------+------+------+------+
| Bit7 | Bit6 | Bit5 | Bit4 | Bit3 | Bit2 | Bit1 | Bit0 |
+------+------+------+------+------+------+------+------+
| IRQ  | Flag | Flag |      |      |      |      |      |
|      |  T1  |  T2  |      |      |      |      |      |
+------+------+------+------+------+------+------+------+

IRQ:?

Flag T1: 1 = Interrupt because of Timer 1 (MIDI timer)

Flag T2: 1 = Interrupt because of Timer 2 (MIDI data available)

Data-Register:

+--------+------+------+------+------+------+------+------+------+
|Reg.Addr| Bit7 | Bit6 | Bit5 | Bit4 | Bit3 | Bit2 | Bit1 | Bit0 |
+--------+------+------+------+------+------+------+------+------+
|   01   |             |      | TEST                             |
+--------+------+------+------+------+------+------+------+------+
|   02   | TIMER-1                                               |
+--------+------+------+------+------+------+------+------+------+
|   03   | TIMER-2                                               |
+--------+------+------+------+------+------+------+------+------+
|   04   | IRQ  | MASK | MASK |                    | START/STOP  |
|        | RESET|  T1  |  T2  |                    |  T2  |  T1  |
+--------+------+------+------+------+------+------+------+------+
|   08   | CSM  | SEL  |                                         |
+--------+------+------+------+------+------+------+------+------+
| 20-35  |  AM  | VIB  |  EG  | KSR  | MULTIPLE                  |
+--------+------+------+------+------+------+------+------+------+
| 40-55  | KSL         | TOTAL LEVEL                             |
+--------+------+------+------+------+------+------+------+------+
| 60-75  | ATTACK RATE               | DECAY RATE                |
+--------+------+------+------+------+------+------+------+------+
| 80-95  | SUSTAIN LEVEL             | RELEASE RATE              |
+--------+------+------+------+------+------+------+------+------+
| A0-A8  | F-NUMBER (L)                                          |
+--------+------+------+------+------+------+------+------+------+
| B0-B8  |             |KEY ON|BLOCK               | F-NUMBER (H)|
+--------+------+------+------+------+------+------+------+------+
|   BD   |DEPTH |DEPTH |RHYTHM| BASS |SNARE | TOM  | TOP  | HI   |
|        |  AM  | VIB  |      | DRUM | DRUM | TOM  | CYM  | HAT  |
+--------+------+------+------+------+------+------+------+------+
| C0-C8  |                           | FEEDBACK           | CON  |
+--------+------+------+------+------+------+------+------+------+
| E0-F5  |                                         | WAVE SEL    |
+--------+------+------+------+------+------+------+------+------+

Operator Cell Number and Channel Number:

The FM synthesizer is capable of voicing 9 FM channel sound. Each of these channel has two operator cells. The relationship between channels and operators is shown below. When addressing a operator cell, care must taken because of holes in the adressing map.

+-----------+----+----+----+----+----+----+----+----+----+
| OP CELL   | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8  | 9  |
+-----------+----+----+----+----+----+----+----+----+----+
| CHANNEL   | 1  | 2  | 3  | 1  | 2  | 3  | 4  | 5  | 6  |
+-----------+----+----+----+----+----+----+----+----+----+
| FUNCTIION | MODULATOR    | CARRIER      | MODULATOR    |
+-----------+----+----+----+----+----+----+----+----+----+
| OFFSET    | 00 | 01 | 02 | 03 | 04 | 05 | 08 | 09 | 0A |
+-----------+----+----+----+----+----+----+----+----+----+

+-----------+----+----+----+----+----+----+----+----+----+
| OP CELL   | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 |
+-----------+----+----+----+----+----+----+----+----+----+
| CHANNEL   | 4  | 5  | 6  | 7  | 8  | 9  | 7  | 8  | 9  |
+-----------+----+----+----+----+----+----+----+----+----+
| FUNCTIION | CARRIER      | MODULATOR    | CARRIER      |
+-----------+----+----+----+----+----+----+----+----+----+
| OFFSET    | 0B | 0C | 0D | 10 | 11 | 12 | 13 | 14 | 15 |
+-----------+----+----+----+----+----+----+----+----+----+

Register functions

All functions of the FM synthesizer are controlled by data written into it's register array. The shape of the envelope, modulation factor, frequency and other parameters are determined according to the data which is written to the registers. The following section describes the function of various registers.

Test (01H)

This register must be initialized to zero

Timer-1 (02H)

Timer- is an 8 bit presettable counter. It has a resolution of 80usec. A preset value N1 is written to Timer-1 register at address 02H. Before counting starts, this preset value is loaded into the counter. If an overflow occurs, the Timer-1 flag in the Status register is set, and the preset value is reloaded into the counter. I  addition to normal timer functions, Timer-1 is also used for composite speech synthesis. When an overflow occurs in this mode, all cells are set to KEY-ON and then immediately to KEYOFF. This operation allows for composite speech synthesis.

Toverflow=(256-N1)*0.08msec

Timer-2 (03H)

Timer-2 is similar to Timer-1 except that it's resolution is 320usec.

Toverflow=(256-N2)*0.32msec

where N2 is the preset value of Timer-2.

Timer Control (04H)

This register is used to start, stop and control flags of Timer-1 and Timer-2.

+--------+------+------+------+------+------+------+------+------+
|Reg.Addr|  D7  |  D6  |  D5  |  D4  |  D3  |  D2  |  D1  |  D0  |
+--------+------+------+------+------+------+------+------+------+
|   04   | IRQ  | MASK | MASK |                    | START/STOP  |
|        | RESET|  T1  |  T2  |                    |  T2  |  T1  |
+--------+------+------+------+------+------+------+------+------+

D0Start/Stop Timer-1)

When this bit is "1", the preset value is loaded into Timer-1 and counting starts. When this bit is "0", Timer-1 does not operate.

D1(Start/Stop Timer-2)

Performs the same operation as D0 for Timer-2

D5(Mask Timer-2)

When this bit is "1", the flag for Timer-2 in the status register is masked (always "0").

D6(Mask Timer-1)

This bit mask the flag of Timer-1.

D7(IRQ-RESET)

When set to "1", the IRQ and Timer flags in the status register are reset to "0". This bit then automatically resets itself to "0".

CSM Mode/Keyboard Split (08H)

This register cause the synthesizer to operate in music mode or speech synthesis mode and determines the keyboard split for keyboard scanling of rate.

+--------+------+------+------+------+------+------+------+------+
|Reg.Addr|  D7  |  D6  |  D5  |  D4  |  D3  |  D2  |  D1  |  D0  |
+--------+------+------+------+------+------+------+------+------+
|   08   | CSM  | SEL  |                                         |
+--------+------+------+------+------+------+------+------+------+

D6(NOTE SELECT)

This bit controls the split point of the keyboard. When "0", the keyboard split is the second bit from the most significant bit of the F-Number (F-Number 8). The MSB of the F-Number (F-Number 9) is used when "1". This is illustrated in the table below:

NOTE SELECT=0

+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| OCTAVE     |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |
+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| BLOCK DATA |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |
+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| F-NUMBER 9 |  1  |  1  |  1  |  1  |  1  |  1  |  1  |  1  |
+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| F-NUMBER 8 | 0| 1| 0| 1| 0| 1| 0| 1| 0| 1| 0| 1| 0| 1| 0| 1|
+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| SPLIT NUM  | 0| 1| 2| 3| 4| 5| 6| 7| 8| 9|10|11|12|13|14|15|
+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

NOTE SELECT=1

+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| OCTAVE     |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |
+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| BLOCK DATA |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |
+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| F-NUMBER 9 | 0| 1| 0| 1| 0| 1| 0| 1| 0| 1| 0| 1| 0| 1| 0| 1|
+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| SPLIT NUM  | 0| 1| 2| 3| 4| 5| 6| 7| 8| 9|10|11|12|13|14|15|
+------------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

D7(CSM)

The composite sine wave speech synthesis mode is selected when "1". All channels must be in the KEYOFF state when this mode is selected.

AM/VIB/EG/KSR/MULTIPLE (20H-35H)

This group of registers (1 for each operator cell) controls several parameters affecting the timbre of the output signal.

+--------+------+------+------+------+------+------+------+------+
|Reg.Addr|  D7  |  D6  |  D5  |  D4  |  D3  |  D2  |  D1  |  D0  |
+--------+------+------+------+------+------+------+------+------+
| 20-35  |  AM  | VIB  |  EG  | KSR  | MULTIPLE                  |
+--------+------+------+------+------+------+------+------+------+

D0-D3(MULTIPLE)

The basic FM equation with the multiplication factor included as follows:

F(t)=A sin ( Mc wc t + I sin ( Mm wm t ) )

whereA is the output amplitude

I is the modulation index (amplitude of the Modulator)

wc is the carrier frequency

wm is the modulator frequency

Mc is the multiplication factor for Carrier cell

Mm is the multiplication factor for the Modulator cell

The relationship between D0-D3 (MULTIPLE) and multiplication factor is as follows:

.

.

.

MSX-Audio registers:

1Amicrophone-input, line-input: level of external signal (-128 to +127)

Music Module registers:

05keyboard row read (bit = 0 if key pressed)

06keyboard row select (8 rows; &H01=left, &H80=right)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("moonsound", "hw-moonsound,opl4", "Hardware", "", "MSX-BIOS: MoonSound:...",
        R"MAN(HARDWARE: MoonSound
MSX-BIOS: MoonSound

-------------------------------------------------------------------------------

MoonSound technical documentation

-------------------------------------------------------------------------------

This document describes the MoonSound and especially the main component, the Yamaha OPL4 chip. It is a copy of the official documentation from Yamaha with some additional notes made by me where I thought that the original descriptions were not very clear. Note that this document does not describe the FM (OPL3) part of the OPL4. You can find that information in a seperate text file. All diagrams are drawn with ASCII characters to enable easy viewing and printing of this document on MSX as well as on PC.

Included with this document are three small GIF files. These are scans of some pictures in the original document. Two more GIFs of the hardware pin assignment are also available but not included here because of their size and their very limited use for the average programmer.

I would like to thank Marcel Delorme for typing and scanning most of the original document.

Remco Schrijvers

-------------------------------------------------------------------------------

I replaced the three scans with a single GIF file that contains all the

envelope diagrams. The image quality is better and it takes less disk space.

Maarten ter Huurne

-------------------------------------------------------------------------------

MoonSound description

-------------------------------------------------------------------------------

The MoonSound cartridge has 3 important components:

- The OPL4 (YMF278B).

- The Yamaha YRW801 2MB ROM with samples for the OPL4.

- 128K SRAM to enable you to use your own samples.

The I/O of the OPL4 can be done on the following I/O ports:

07Eh wave register

07Fh wave data

0C4h FM bank 1 register

0C5h FM data

0C6h FM bank 2 register

0C7h is a mirror of the FM data register but use of 0C5h is preferred.

The status register is also located at 0C4h at all times.

The OPL4 is 100% compatible with the OPL3 which is described in a seperate document and meets the OPL(tm) standard. OPL(tm) is a trademark of Yamaha Corporation which represents a full register compatibility with Yamaha YMF3812 (OPL2).

Important: To enable access to the Wave registers and the extra bit in the status register, you have to set bit D0 and D1 in register 5 of FM bank 2. In basic this would look like this:

OUT &HC6,5

OUT &HC5,&B11

There is no documentation available about the contents of the ROM chip. If you want to use these for (e.g. your own music editor or replayer) you can contact me and I can give you some of the info I found in the two weeks of long hard and boring work I spent on it.

-------------------------------------------------------------------------------

OPL4 Overview

-------------------------------------------------------------------------------

+ FM synthesizer Unit (same as YMF262)

1. Sound Generation Modes

* 2-operator mode

2-operator mode with eighteen sounds are produced at the same time. Or

2-operator with fiftheen sounds, and five rhythm sounds are produced at

the same time.

* 4-operator mode

4-operator mode with six sounds and 2-operator mode with six sounds are

produced at the same time. Or 4-operator mode with six sounds, 2-operator

mode with three sounds and five rhythm sounds are produced at the same

time.

2. Eight types of waves.

3. Stereo output.

+ PCM synthesizer Unit

1. Simultaneous generation of 24 sounds.

2. Voice output data sampling frequency of 44.1 Khz.

3. Waveform data length: 8 bits, 12 bits and 16 bits.

4. Stereo output (16-level pan can be specified for each sound)

+ Wave Data for PCM Synthesizer Unit

1. Maximum of 4M-byte external memory can be conected.

2. Maximum of 512 tones.

3. External ROM or SRAM memory. If SRAM is connected, wave data can be

downloaded from CPU.

4. Chip select signals for memory can be output: 128K bytes, 512K bytes,

1M bytes and 2M bytes.

+ Miscellaneous

1. YMF278 has six channels for sound output, and can be connected to the

YAC513.

2. YMF278 can be connected to the YSS225(EP) witch gives various effects

to sound data.

3. YMF278 can read all registers.

4. Pack contains 80-pin QFP.

-------------------------------------------------------------------------------

CPU INTERFACE

-------------------------------------------------------------------------------

Sound generation control for the YMF278 is performed by writing data into registers. An 8-bit parallel CPU interface is provided to write data into and read status form each register. D0-D7 are bi-directional data buses, and /CS,/RD,/WR,/WR,A0,A1,A2 are data bus control signal input.

The data buses are set as shown below by the data control signal:

+-------------------------------------------------------------------+
|     | /CS | /RD | /WR | /A0 | /A1 | /A2 |          MODE           |
+-----+-----+-----+-----+-----+-----+-----+-------------------------+
|     |  H  |  X  |  X  |  X  |  X  |  X  |    Inactive mode        |
|     |  L  |  L  |  H  |  L  |  L  |  L  |    Status read mode     |
+-----+-----+-----+-----+-----+-----+-----+-------------------------+
|     |  L  |  H  |  L  |  L  | L/H |  L  |    Address write mode   |
| FM  |  L  |  H  |  L  |  H  |  X  |  L  |    Data write mode      |
|     |  L  |  L  |  H  |  H  |  X  |  L  |    Data read mode       |
+-----+-----+-----+-----+-----+-----+-----+-------------------------+
|     |  L  |  H  |  L  |  L  |  L  |  H  |    Address write mode   |
| PCM |  L  |  H  |  L  |  H  |  L  |  H  |    Data wrtie mode      |
|     |  L  |  L  |  H  |  H  |  L  |  H  |    Data read mode       |
+-----+-----+-----+-----+-----+-----+-----+-------------------------+
                                                X: Don't Care

(a)  Inactive Mode

Data buses D0-D7 are set to high impedance when /CS is H

(b)  Address Write Mode

This mode specifies the write address. Output the address data of the

register to be written to the data bus. Then, setting data is written

in data write mode. However, a wait period of 17 cycles of the master

clock is required during this period. To specify register array 0 with

FM, set A1 = L. To use register array 1, set A1 = H.

(c)  Data Write Mode

This mode writes data at the address which is set in address write mode

immediately before. Output setting data to the datat bus. A wait period

of 56 master-clock cycles is required for FM, and a wait period of 88

master-clock cycles is required for PCM until the next address write or

data write.

(d)  Data Read Mode

This mode reads data at the address which is set in address write mode

immediately before.

(e)  Status Read Mode

This mode outputs the status of the YMF278 to the data bus.

------------------------------------------------------------------------------

PCM SYNTHESIZER AND MIX UNIT REGISTER

------------------------------------------------------------------------------

+-----+----+----+----+----+----+----+----+----+
|     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
+-----+----+----+----+----+----+----+----+----+
| 00H |                TEST                   |
| 01H |                                       |
+-----+--------------+----+----+----+----+----+
|     |              | Tone header  |    |    |    MT = Memory type
| 02H |   DeviceID   | 2    1    0  | MT | MA |    MA = Memory access
+-----+----+----+----+----+----+----+----+----+
|     |         |Memory address register high |
| 03H | --------|MA21 MA20 MA19 MA18 MA17 MA16|
+-----+---------+----+----+----+----+----+----+
|     |     Memory address register mid.      |
| 04H |MA15 MA14 MA13 MA12 MA11 MA10 MA9  MA8 |
+-----+----+----+----+----+----+----+----+----+
|     |     Memory address register low       |
| 05H |MA7  MA6  MA5  MA4  MA3  MA2  MA1  MA0 |
+-----+----+----+----+----+----+----+----+----+
|     |       Memory data register            |
| 06H | 7    6    5    4    3    2    1    0  |
+-----+----+----+----+----+----+----+----+----+
|     |                                       |
| 07H | ------------------------------------- |
+-----+---------------------------------------+
| 08H-|             Tone number               |
|  1FH| 7    6    5    4    3    2    1    0  |
+-----+----+----+----+----+----+----+----+----+
| 20H-|             F-NUM              Tone no|
|  37H| F6   F5   F4   F3   F2   F1   F0   8  |
+-----+----+----+----+----+----+----+----+----+
| 38H |   Octave          pseudo   F-NUM      |
|  4FH| O3   O2   O1   O0 reverb F9   F8   F7 |
+-----+----+----+----+----+----+----+----+----+
| 50H-|     Total level                  Level|
|  67H| 6    5    4    3    2    1    0 direct|
+-----+----+----+----+----+----+----+----+----+
| 68H | Key  damp LFO  CH      Pan pot        |
|  7FH| on        RES     3      2    1    0  |
+-----+----+----+----+----+----+----+----+----+
| 80H-|              LFO         VIB          |
|  97H| --------  S2   S1   S0   V2   V1   V0 |
+-----+---------+----+----+----+----+----+----+
| 98H-|        AR                D1R          |
|  AFH| 3    2    1    0    3    2    1    0  |
+-----+----+----+----+----+----+----+----+----+
| B0H-|        DL                D2R          |
|  C7H| 3    2    1    0    3    2    1    0  |
+-----+----+----+----+----+----+----+----+----+
| C8H-|  Rate compensaton        RR           |
|  DFH| 3    2    1    0    3    2    1    0  |
+-----+----+----+----+----+----+----+----+----+
| E0H-|                            AM         |
|  F7H| -----------------------  2    1    0  |
+-----+------------------------+----+----+----+
|     |           Mix control    Mix control  |
|     |             (FM-R)         (FM-L)     |
| F8H | --------  2    1    0    2    1    0  |
+-----+---------+----+----+----+----+----+----+
|     |           Mix control    Mix control  |
|     |             (PCM-R)         (PCM-L)   |
| F9H | --------  2    1    0    2    1    0  |
+-----+---------+----+----+----+----+----+----+

Notes) 1. Be sure to set unused bits and TEST registers to 0.

2. Mix control registers (FM-R, FM-L) of F8h are set to 3 (-9dB), other

registers are set to '0' by initial clear.

------------------------------------------------------------------------------

REGISTERS

------------------------------------------------------------------------------

In the (OPL4), a sound which is output is called a channel. In the PCM synthesizer unit, 1-24 channels can be output simultaneosly. 08H-F7H of registers are grouped into 24-byte units, and each group corresponds to 1-24 channels sequentially.

+ 00H-01H TEST

This register is used for testing LSI. set

this registers to "0" for normal use.

                               +-----+----+----+----+----+----+----+----+----+
                               |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
                               | 00- +----+----+----+----+----+----+----+----+
                               | 01H |                 TEST                  |
                               +-----+---------------------------------------+

+ 02H MEMORY ACCESS

0: Normal sound generation mode

1: CPU reads and writes the

     external memory. Sound    +-----+----+----+----+----+----+----+----+----+
     generation cannot be      |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
     performed in this mode.   | 02H +----+----+----+----+----+----+----+----+
                               |     |                                  | MA |
                               +-----+----------------------------------+----+

+ 02H MEMORY TYPE

Specify the connection method of the external memory

0: ROM (always on MoonSound)

1: SRAM and ROM

                               +-----+----+----+----+----+----+----+----+----+
  Refer to the Chip select     |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  Signals of External Memory   | 02H +----+----+----+----+----+----+----+----+
  for details.                 |     |                             | MT |    |
                               +-----+-----------------------------+----+----+

Important: When you are using the MoonSound, always select the ROM mode. Even

if you are using SRAM and ROM!!

+ 02H TONE HEADER

This register sets the range of setting in every 512K byte unit for the

128-tone header (no.384-no.511 out of 512 tones)

                               +-----+----+----+----+----+----+----+----+----+
  If D2-D4 = 0, set headers    |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  for 512 tones starting with  | 02H +----+----+----+----+----+----+----+----+
  addres 0 in the external     |     |              | Tone header  |         |
  memory.                      +-----+--------------+--------------+---------+

In the MoonSound the last 128 headers will be located after the 2MB ROM.

Since this is 4*512, the tone header value must be 4 (100b).

+ 02H DEVICE ID

                               +-----+----+----+----+----+----+----+----+----+
  Used for ID register, no     |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  further information          | 02H +----+----+----+----+----+----+----+----+
  available.                   |     |   DevicdID   |                        |
                               +-----+--------------+------------------------+

This is probably used to identify what kind of chip you are using. My guess

is that it will have the following values:

0 = not used

1 = OPL4

2 = future OPL5?

+ 03H-5H MEMORY ADDRESS REGISTER

  These registers set the ex-  +-----+----+----+----+----+----+----+----+----+
  ternal memory addresses when |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  data is written into or read | 03H +----+----+----+----+----+----+----+----+
  from the external memory.    |     | ------- |   MA21-MA16                 |
                               +-----+---------+-----------------------------+

  Set from the highest address +-----+----+----+----+----+----+----+----+----+
  in sequence because address  |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  setting for MA21-MA0 comple- | 04H +----+----+----+----+----+----+----+----+
  tes by the setting of 05H.   |     |              MA15-MA8                 |
                               +-----+---------------------------------------+

  Addresses are automatically  +-----+----+----+----+----+----+----+----+----+
  incremented by data write or |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  data read from the external  | 05H +----+----+----+----+----+----+----+----+
  memory.                      |     |               MA7-MA0                 |
                               +-----+---------------------------------------+

+ 06H MEMORY DATA REGISTER

  By writing into this reg,    +-----+----+----+----+----+----+----+----+----+
  the data at the external     |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  memory address which is      | 03H +----+----+----+----+----+----+----+----+
  currently set can be written.|     | ------- |      MA21-MA16              |
                               +-----+---------+-----------------------------+

A wait period of 28 master-clock cycles

is required until the next data write.

By reading this register, the data at the external memory address which is

currently set can be read.

A wait period of 38 master-clock cycles is required until the next data

read.

+ 08H-1FH TONE NUMBER

  The OPL4 supports a maximum of 512 tones. Select the
  number of the tone to be     +-----+----+----+----+----+----+----+----+----+
  produced from 512 tones, and |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  set the selected number in   | 08- +----+----+----+----+----+----+----+----+
  the tone number register     | 1FH |              Tone number              |
  of the channel from which    +-----+---------------------------------------+
  tone is to be produced.
  The tone header is then internally loaded automaticlly.

  The loading time is approxi- +-----+----+----+----+----+----+----+----+----+
  mately 300 msec from the     |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  setting of the tone number   | 20- +----+----+----+----+----+----+----+----+
  in the register. Do not      | 37H |                                  |no8.|
  access the tone number, LFO, +-----+----------------------------------+----+
  V1B,AR,D1R,DL,D2R, rate
  compensation, RR and AM register of the channel during loading.
  However, access can be made to other register even when the tone header is
  being loaded.

The flag which indicates loading is located at bit D1 of the status

register.

Always set the tone numbers (7-0) after the tone number (8) because tone

number setting completes by the setting of the tone numbers (7-0).

+ 20H-4FH F-NUM AND OCTAVE

  The F-NUM and octave reg.    +-----+----+----+----+----+----+----+----+----+
  control sound generation     |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  pitch.                       | 20- +----+----+----+----+----+----+----+----+
                               | 37H |             F-Number             |    |
  Set positive integers from   +-----+----------------------------------+----+
  0-1023 in the F-NUM registers,
  and set two's complement     +-----+----+----+----+----+----+----+----+----+
  from -7 to 7 in the octave   |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  register.                    | 38- +----+----+----+----+----+----+----+----+
                               | 4FH |                   |    F-Number       |
  Always set F-NUM (f6-f0)     +-----+-------------------+-------------------+
  after F-NUM (f9-f7) because
  F-NUm setting completes by   +-----+----+----+----+----+----+----+----+----+
  setting of F-NUM (f6-f0).    |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
                               | 38- +----+----+----+----+----+----+----+----+
  If F-NUM is 0 and octave is  | 4FH |       Octave      |                   |
  1, waveform data will be     +-----+-------------------+-------------------+
  regenerated in sequence at
  44.1 Khz. This is used as the reference (F(c) = 0).

The formula to obtain the pitch variation for the reference is as follows:

F (c) = 1200 * (octave-1) + 1200 * log2 [(1024 + F-NUM) /1024 ]

Where 1 octave = 1200 c

+ 38H-4FH PSEUDOREVERB

  Set the reverb rate (RevR)   +-----+----+----+----+----+----+----+----+----+
  to 5 when the envelope beco- |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  mes smaller than 18 dB so    | 38- +----+----+----+----+----+----+----+----+
  that pseudoreverb effect is  | 4FH |                   | ps |              |
  on.                          +-----+-------------------+----+--------------+

0: Pseudoreverb off                    ps = Pseudoreverb

1: Pseudoreverb on

Refer to the "Envelopes" for details.

+ 50H-67H TOTAL LEVEL

                               +-----+----+----+----+----+----+----+----+----+
  Set the total level. The     |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  table shown below shows the  | 50- +----+----+----+----+----+----+----+----+
  correspondence of each bit   | 67H |              Total level         |    |
  of the total level.          +-----+----------------------------------+----+

  +---------+-----+-----+-----+-----+-----+-----+------+
  | Bit No. |  6  |  5  |  4  |  3  |  2  |  1  |  0   |
  +---------+-----+-----+-----+-----+-----+-----+------+
  | ATT.(dB)| -24 | -12 | -6  | -3  | -1.5|-0,75|-0,375|
  +---------+-----+-----+-----+-----+-----+-----+------+

+ 50H-67H LEVEL DIRECT

  Select the method of change  +-----+----+----+----+----+----+----+----+----+
  when the total level is      |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  changed.                     | 50- +----+----+----+----+----+----+----+----+
                               | 67H |                                  | LD |
  0: Changes while the total   +-----+----------------------------------+----+
     level interpolates

1: The total level is immediately used as the current value.

If interpolation is performed, the time required for the sound volume to

change from the minimum to the maximum is ?? msec, and the time required

to change from the maximum to the minimum is ?? msec.

+ 68H-7FH KEY ON

  0: Key off                 +-----+----+----+----+----+----+----+----+----+
                             |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  1: Key on                  | 68- +----+----+----+----+----+----+----+----+
                             | 7FH |KEY |                                  |
                             +-----+----+----------------------------------+

+ 68H-7FH DAMP

  Forced damp starts by setting+-----+----+----+----+----+----+----+----+----+
  D6 of this register to 1:    |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
                               | 68- +----+----+----+----+----+----+----+----+
  0: Normal rate is valid      | 7FH |    |Damp|                             |
                               +-----+----+----+-----------------------------+
  1: Damp rate is valid

Damp is performed as shown in the table below at the time elapses:

  +-----------+-----+-----+-----+-----+
  | Time(msec)| 5.8 | 8.0 | 9.4 | 10.9|
  +-----------+-----+-----+-----+-----+
  | ATT.(dB)  | -12 | -48 | -72 | -96 |
  +-----------+-----+-----+-----+-----+

+ 68H-7FH LFO RES

  The operation of LFO is as +-----+----+----+----+----+----+----+----+----+
  follows:                   |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
                             | 68- +----+----+----+----+----+----+----+----+
  0: Start operation         | 7FH |         |LFOR|                        |
                             +-----+---------+----+------------------------+
  1: Stops operation and resets

+ 68H-7FH CH

  Output channel:            +-----+----+----+----+----+----+----+----+----+
                             |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  0: Outputs to MIX          | 68- +----+----+----+----+----+----+----+----+
                             | 7FH |              | CH |                   |
  1: Outputs to PCM-EXT      +-----+--------------+----+-------------------+

For the MoonSound all channels need to be set to PCM-EXT.

+ 68H-7FH PAN POT

  Set the pan pot. The pan pot +-----+----+----+----+----+----+----+----+----+
  changes as shown in the table|     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  below according to the value | 68- +----+----+----+----+----+----+----+----+
  to be written:               | 7FH |                   |     Pan Pot       |
                               +-----+-------------------+-------------------+

+-----------+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| Bit No.   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11| 12| 13| 14| 15|
+-----------+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| ATT.(dB) L| 0 |-3 |-6 |-9 |-12|-15|-18| oo| oo| 0 | 0 | 0 | 0 | 0 | 0 | 0 |
+-----------+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| ATT.(dB) R| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | oo| oo|-18|-15|-12|-9 |-6 |-3 |
+-----------+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

+ 80H-97H LFO

  Set the speed for LFO. The   +-----+----+----+----+----+----+----+----+----+
  speed changes as shown in the|     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  table below according to the | 80- +----+----+----+----+----+----+----+----+
  value to be written:         | 97H | ------- |     LFO      |              |
                               +-----+---------+--------------+--------------+

  +--------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | Value              |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |
  +--------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | LFO Frequency (Hz) |0.168|2.019|3.196|4.206|5.215|5.888|6.224|7.066|
  +--------------------+-----+-----+-----+-----+-----+-----+-----+-----+

+ F8H FM MIXING CONTROL

  Sets the output level of the +-----+----+----+----+----+----+----+----+----+
  FM channels. 0 means maximum |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  output. Initial value is 3   |     +----+----+----+----+----+----+----+----+
  (-9 dB).                     | F8H | ------- |     FM-R     |     FM-L     |
                               +-----+---------+--------------+--------------+

+ F9H WAVE MIXING CONTROL

  Sets the output level of the +-----+----+----+----+----+----+----+----+----+
  PCM channels. 0 means        |     | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
  maximum output. Initial      |     +----+----+----+----+----+----+----+----+
  value is 0.                  | F9H | ------- |    PCM-R     |    PCM-L     |
                               +-----+---------+--------------+--------------+

-------------------------------------------------------------------------------

STATUS REGISTER

-------------------------------------------------------------------------------

  +------+------+------+------+------+------+------+------+------+
  |      |  D7  |  D6  |  D5  |  D4  |  D3  |  D2  |  D1  |  D0  |
  |------+------+------+------+------+------+------+------+------+
  |Status|  IRQ |  FT1 |  FT2 | ------------------ |  LD  | BUSY |
  +------+------+------+------+------+------+------+------+------+

See the OPL3 document for more info on IRQ, FT1 and FT2.

BUSY: The BUSY flag is valid while NEW2=1. This flag becomes 1 while writing

address and data. BUSY flag automatically returns to 0 when writing is

completed.

LD  : Indicates that a Wave Table header is loaded. Returns to 0 when loading

is completed. See reg. 8 for more info.

Although there is no info in the manual about this I think that the BUSY flag is not only for writing, but also for reading. It works for FM, Wave and Wave data transfer! On the Z80A waiting for the BUSY flag is never neccessary, but on the 7Mhz Z80 or the R800 it is neccessary. Not always, but on unpredictable moments so it MUST be used if you are making something and you are planning to use it for R800 or 7Mhz. The disadvantage for Z80 is that it takes some time to check this every time and the Z80 needs all the speed it can get.

-------------------------------------------------------------------------------

ENVELOPES

-------------------------------------------------------------------------------

+ Envelope Graphs

The envelopes are shown in the GIF file.

To produce continuous sound, set D2R = 0 (RATE = 0, real time = inf).

+ Pseudoreverb effect

When the pseudoreverb in 38H-4FH is set to "1", pseudoreverb effect can be

applied. The envelope when pseudoreverb effect is applied is shown in the

GIF file. When the envelope decreases below -18 dB, D1R, D2R and RR are

disregarded,and RevR (fixed at 5) is set. Pseudoreverb can also be applied

to continuous at below -18 dB in the same manner.

-------------------------------------------------------------------------------

WAVE DATA IN EXTERNAL MEMORY

-------------------------------------------------------------------------------

         +----+----+----+----+----+----+----+----+
Address  | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
         +----+----+----+----+----+----+----+----+ -------------------------
   0     | Data bit|        Start address        | D1-D0 = 0 for  8 bit data
         | D1   D0 | S21  S20  S19| S18  S17  S16| D1-D0 = 1 for 12 bit data
         +----+----+----+----+----+----+----+----+ D1-D0 = 2 for 16 bit data
   1     |             Start address             |
         | S15  S14  S13  S12  S11  S10  S9   S8 |
         +----+----+----+----+----+----+----+----+
   2     |             Start address             | Header for tone No.0
         | S7   S6   S5   S4   S3   S2   S1   S0 |
         +----+----+----+----+----+----+----+----+
   3     |             Loop address              |
         | L15  L14  L13  L12  L11  L10  L9   L8 |
         +----+----+----+----+----+----+----+----+
   4     |             Loop address              |
         | L7   L6   L5   L4   L3   L2   L1   L0 |
         +----+----+----+----+----+----+----+----+
   5     |             End address               | End address must be
         | E15  E14  E13  E12  E11  E10  E9   E8 | inverted!
         +----+----+----+----+----+----+----+----+
   6     |             End address               |
         | E7   E6   E5   E4   E3   E2   E1   E0 |
         +----+----+----+----+----+----+----+----+
   7     |         |   LFO speed  |     VIB      |
         | ------- | S2   S1   S0 |  V2  V1   V0 |
         +---------+----+----+----+----+----+----+
   8     |  Attack rate (AR) | 1st decay (D1R)   |
         |  3    2    1    0 |  3    2    1   0  |
         +---------+----+----+----+----+----+----+
   9     |  Decay level (DL) | 2st decay (D2R)   |
         |  3    2    1    0 |  3    2    1   0  |
         +---------+----+----+----+----+----+----+
  10     | Rate compensation | Release rate (RR) |
         |  3    2    1    0 |  3    2    1   0  |
         +----+----+----+----+----+----+----+----+
  11     |                        |      AM      |
         | ---------------------- | A2   A1   A0 |
         +------------------------+----+----+----+ -------------------------
         |                                       | Header for tone No.1
         |                                       |
         |                                       | -------------------------
         |The following is the waveform data for | Header for tone No.2-283
         |tone numbers 0-383                     |
         +---------------------------------------+ -------------------------
         |                                       |
         | D15  D14  D13  D12  D11  D10  D9   D8 |
         +----+----+----+----+----+----+----+----+ For 16 bit data
         |                                       |
         | D7   D6   D5   D4   D3   D2   D1   D0 |
         +----+----+----+----+----+----+----+----+ -------------------------
         |             1st data                  |
         | D11  D10  D9   D8   D7   D6   D5   D4 |
         +----+----+----+----+----+----+----+----+
         |     1st data      |    2nd data       |
         | D3   D2   D1   D0 | D3   D2   D1   D0 | For 12 bit data
         +----+----+----+----+----+----+----+----+
         |              2nd data                 |
         | D7   D6   D5   D4   D3   D2   D1   D0 |
         +----+----+----+----+----+----+----+----+ -------------------------
         |                                       |
         | D7   D6   D5   D4   D3   D2   D1   D0 | For 8 bit data
         +----+----+----+----+----+----+----+----+ -------------------------

Note that the end and loop addresses are NOT in bytes but indicate the number of samples! This means that the maximum size for a 8-bit sample is 64K, for a 12-bit sample 96K and for a 16-bit sample 128K.

See also: MoonSound (Pre-Release documentation)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("moonsound-pre-release-documentation", "", "Hardware", "", "MoonSound (geral) - MoonSound (Pre-Release documentation): FM:",
        R"MAN(HARDWARE: MoonSound
MoonSound (geral) - MoonSound (Pre-Release documentation)

FM:

- Upto eighteen 2-Operator FM channels or fifteen FM channels with five drum sounds

- Upto six 4-Operator FM channels (together with another six 2-Operator channels)

- Eight different waveforms

- Possibility to specifiy right, left or both for each channel

PCM:

- Upto 24 waves simultaneously! (this means 24 channels)

- Replayfrequency of 44.1 kHz

- 8, 12 or 16 bits wavedata Stereo signal in 16 positions for each channel

THE FM SYNTHESIZERS

The best way to judge the FM synthesizer of the OPL4 is by comparing it to MSX-MUSIC (FM-PAC) and MSX-AUDIO (Music Module). The possibilities of these chips should be known by everybody nowadays. The OPL4 resembles the MSX-AUDIO the most. This is not so strange because the MSX-AUDIO is actually the OPL1. The FM synthesizer of the OPL4 is 100% compatible with the FM synthesizer of MSX-AUDIO. But the OPL4 has twice as many channels!

On top of that there are the so-called 4-Operator instruments. Normally an instrument is created by adjusting two operators in the right way. Compare this to the Make Own Voice part of MoonBlaster. Now it is also possible to create voices using four operators. It speaks for itself that you can create much better instruments than with only two operators. Another improvement is

that the waveform can be selected for each operator. On the MSX-AUDIO the waveform is always a sinus, now that can be changed.

Finally a stereo possibility was added to the FM synthesizer. With MoonBlaster we had to go through a lot of trouble using two different soundchips to get stereo, now you can select for each channel if it should be heard left, right or both.

THE PCM SYNTHESIZERS

This part of the OPL4 makes it possible to play samples. A sample is a piece of digitized (sampled) sound. You can use this to get very realistic instruments, because it's an exact copy! A drawback of samples is that a large difference between the record and play samplefrequency will give bad quality: the instrument doesn't sound like the original. To get a good piano for example, it would be necessary to sample each note seperately. To prevent this the sample will be influenced by the PCM synthesizer. A lot of things can be done with this, a lot of them are also used for FM synthesis, like attack, decay, etc. An extra advantage of this synthesizer is that the samples can be short because, for example, the decay of the tone can be done by the synthesizer. Playing a sample with these extra 'effects' is known as Wave Table Synthesis and a sample with this extra information is called a 'wave'.

The quality of a sample is determined by two things: de sample resolution and the sample frequency. With a sample resolution of 16 bits and a frequency of 44.1 kHz the OPL4 has sound of CD quality!

The PCM synthesizer has stereo possibilities too. These are even better than those of the FM synthesizer, because here there is no left/right/both but a real balance with 16 positions! This balance can be set for each of the 24 channels seperately.

PCM data takes a lot of memory, even with the synthesizer stuff of the OPL4. Especially on an MSX it is not nice at all to load 300 kB of wavedata for one song. The solution for this is the 2 megabyte ROM which will be in the cartridge too. This ROM was designed by Yamaha especially for the OPL4 and it contains all General MIDI sounds! These are 128 instruments and 47 drum sounds. On top of that there will be 128 kB RAM in the cartridge so can you can use your own samples too. For the real freaks it will be possible to replace this 128 kB RAM by 512 kB RAM.

To erase any doubts: the OPL4 is not able to digitize sound, it can only replay it. This is not really a problem because wavedata is just PCM data with some extra information. It should be simple to use samples created by a Music Module, turbo R or another computer.

SPEED

Programmers will be interested to know that the OPL4 is much faster than MSX-MUSIC and MSX-AUDIO. Even the quite slow Z80 had to wait when accessing those chips. But the OPL4 is so fast that even the R800 doesn't have to wait anymore. Another advantage is that all (!) registers can be read.

It is a mayor advantage for the relatively slow MSX computer that the OPL4 handles almost everything itself. It would never be possible to play 24 waves simultaneously if this hadn't been the case. Just think about the MODplayer by XelaSoft. It takes all the CPU time on a turbo R to play 4 samples. The OPL4 makes it possible to create demos and games with fabulous background music. The 128 kB RAM can be used perfectly for sound effects in games. It should be possible to make a MODplayer for Z80 that leaves enough CPU time for other programs to run simultaneously!

(c) Sunrise Foundation 1994

P.O. Box 178

1530 AD Wormer

Netherlands

E-mail: smboer@cs.vu.nl

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("moonblaster", "", "Hardware", "", "MoonBlaster: Moonblaster is not a soundcard, it's a program that takes full advantage of...",
        R"MAN(HARDWARE: MoonSound
MoonBlaster

Moonblaster is not a soundcard, it's a program that takes full advantage of the Music-Module and the FM-PAC and uses them both at the same time. So you can create a stereo-effects on the MSX, you can switch soundcards during playtime to increase the effect. This program needs 128KB of RAM memory, 128KB of VRAM and can make use of the Music-Module, the FM-PAC or both.

The MoonBlaster program was created by Remko Schrijvers who is now the programmer of WOOM! which is also an MSX-emulator.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("msx-video-v9958", "hw-v9958,v9958", "Hardware", "", "MSX-Video: V9958: Data port (&H98)",
        R"MAN(HARDWARE: MSX-Video (V9958)
MSX-Video: V9958

Data port (&H98)

The data port is used to read or write single bytes to the VRAM. The VDP possesses an internal address register pointing to a location in the VRAM. Reading the data port will input the byte from this VRAM location while writing to the data port will store a byte there. After a read or write the adress register is automatically incremented to point to the next VRAM location. Sequential bytes can be accessed simply by continous reads or writes to the data port.

Command port (&H99)

The command port is used for three purposes:

to set up the data port adress registers
to read the VDP status register
to write one of the VDP mode registers

Palette port (&H9A)

The palette port is used to define the palette color specified in VDP register 16.

VDP port (&H9B)

Using this port you can write to the VDP-register set in VDP-Register 17. If the AII bit is 0, the target-register is incremented automatically.

Address register

The data port adress register must be setup in different ways depending on whether the subsequent access is to be a read or a write. The adress register can be setup to any value from 0x0000 to 0x3FFF by first writing the LBS (Least Significant Byte) and then MSB (Most Significant Byte) to the command port. Bits 6 and 7 of the MSB are used by the VDP to determine whether the address is being setup for subsequent reads or writes as follows:

 Read          xxxxxxxx 00xxxxxx
 Write         xxxxxxxx 01xxxxxx

It is important to know that no other accesses are made to the VDP in between writing the LSB and the MSB as this will upset ist synchronisation. The MSX ROM interrupt handler is continously reading the VDP status register as a background task so interrupts should be disabled as necessary.

VDP mode registers

The VDP has write-only registers which control it's general operation. A particular register is set by first writing a data byte then a register selection byte to the command port. The register selection byte contains the register number in the lower bits (3 to 7, depending on the video-processor version: 1RRRRRRR). As the mode-registers are write-only, and cannot be read, the MSX ROM maintains an exact copy of these registers in the workspace area RAM. Using MSX-BIOS routines for VDP ensures that this register image is correctly updated.

                             THE V9958-REGISTERS
                            =====================
               VDP() REG   b7   b6   b5   b4   b3   b2   b1   b0

                 8    0     F   SD    C   S4   S3   S2   S1   S0
                -1    1     0    0   I4   I3   I2   I1   I0   FH
                -2    2    TR   VR   HR   BD    0    0   EO   CE
                -3    3    X7   X6   X5   X4   X3   X2   X1   X0
                -4    4     0    0    0    0    0    0   X9   X8
                -5    5    Y7   Y6   Y5   Y4   Y3   Y2   Y1   Y0
                -6    6     0    0    0    0    0    0   Y9   Y8
                -7    7    C7   C6   C5   C4   C3   C2   C1   C0
                -8    8    BX7  BX6  BX5  BX4  BX3  BX2  BX1  BX0
                -9    9     0    0    0    0    0    0   BX9  BX8

                 0    0     0   DG    0   IE1  M5   M4   M3    0
                 1    1     0   BLK  IE0  M1   M2    0   SZ   MAG
                 2    2     0   A16  A15  A14  A13  A12  A11  A10
                 3    3    B13  B12  B11  B10  B9   B8   B7   B6
                 4    4     0    0   C16  C15  C14  C13  C12  C11
                 5    5    D14  D13  D12  D11  D10  D9   D8   D7
                 6    6     0    0   E16  E15  E14  E13  E12  E11
                 7    7    TC3  TC2  TC1  TC0 BDC3 BDC2 BDC1 BDC0
                 9    8     0    0   TP   CBD VRS1 VRS0  SPD  B/W
                10    9    LN    0  SYM1 SYM0  IL   EO  NTSC  DCD
                11   10     0    0    0    0    0   B16  B15  B14
                12   11     0    0    0    0    0    0   D16  D15
                13   12    C3   C2   C1   C0   BC3  BC2  BC1  BC0
                14   13    ON3  ON2  ON1  ON0  OF3  OF2  OF1  OF1
                15   14     0    0    0    0    0   F16  F15  F14
                16   15     0    0    0    0   RS3  RS2  RS1  RS0
                17   16     0    0    0    0   C3   C2   C1   C0
                18   17    AII   0   RC5  RC4  RC3  RC2  RC1  RC0
                19   18    dV3  dV2  dV1  dV0  dH3  dH2  dH1  dH0
                20   19    IL7  IL6  IL5  IL4  IL3  IL2  IL1  IL0
                21   20     0    0  CBX5 CBX4 CBX3 CBX2 CBX1 CBX0
                22   21     0    0  CBY5 CBY4 CBY3 CBY2 CBY1 CBY0
                23   22     0    0  CBZ5 CBZ4 CBZ3 CBZ2 CBZ1 CBZ0
                24   23    DO7  DO6  DO5  DO4  DO3  DO2  DO1  DO0
                26   25     0   CMD  VDS  YAE  YJK  WTE  MSK  SP2
                27   26     0    0    H8   H7   H6   H5   H4   H3
                28   27     0    0    0    0    0    H0   H1   H2

                33   32    SX7  SX6  SX5  SX4  SX3  SX2  SX1  SX0
                34   33     0    0    0    0    0    0    0   SX8
                35   34    SY7  SY6  SY5  SY4  SY3  SY2  SY1  SY0
                36   35     0    0    0    0    0    0   SY9  SY8
                37   36    DX7  DX6  DX5  DX4  DX3  DX2  DX1  DX0
                38   37     0    0    0    0    0    0    0   DX8
                39   38    DY7  DY6  DY5  DY4  DY3  DY2  DY1  DY0
                40   39     0    0    0    0    0    0   DY9  DY8
                41   40    NX7  NX6  NX5  NX4  NX3  NX2  NX1  NX0
                42   41     0    0    0    0    0    0   NX9  NX8
                43   42    NY7  NY6  NY5  NY4  NY3  NY2  NY1  NY0
                44   43     0    0    0    0    0    0   NY9  NY8
                45   44    CL7  CL6  CL5  CL4  CL3  CL2  CL1  CL0
                46   45     0   MXC  MXD  MXS  DIY  DIX  EQ   MAJ
                47   46    CM3  CM2  CM1  CM0  LO3  LO2  LO1  LO0

F       - Vertical scanning interrupt flag (1: interrupt)
SD      - 5th or 9th sprite detected
C       - Sprite-collision detected
S4/0    - Sprite-number of 5th or 9th sprite
I4/0    - ID-number of the MSX-Video
FH      - Horizontal scanning interrupt flag (1: interrupt)
TR      - Transfer ready flag (1: ready)
VR      - Vertical scanning line timing flag (1: scanning)
HR      - Horizontal scanning line timing flag (1: scanning)
BD      - Boundary colour detect flag of search command (1: colour found)
EO      - Display field flag (0: display first field)
CE      - Command Executing flag (0: ready)
X9/0    - X-coordinate of sprite-collision, mouse or lightpen
Y9/0    - Y-coordinate of sprite-collision, mouse or lightpen
C7/0    - Colour-read register of point command
BX9/0   - Border X-coordinate of search command

DG      - Digitize
IE0     - Enable Vertical Retrace Interrupt
IE1     - Enable Horizontal Retrace Interrupt
D       - External VDP-input
M5/1    - Screen select
          00000 Screen 1
          00001 Screen 0 (WIDTH 40)
          00010 Screen 3
          00100 Screen 2
          01000 Screen 4
          01001 Screen 0 (WIDTH 80)
          01100 Screen 5
          10000 Screen 6
          10100 Screen 7
          11100 Screen 8
BLK     - Enable Display
SZ      - Set sprite size 16 x 16
MAG     - Magnify sprites
A16/10  - Pattern name table base address
B16/6   - Color table base address
C16/11  - Pattern generator table base address
D16/7   - Sprite attribute tabele base adress
E16/11  - Sprite pattern generator table base adress
TC3/0   - Text Colour
BCD3/0  - Back Drop Colour
TP      - Non-transparant mode
CBD     - Set Color Bus to input mode
VRS1/0  - Video RAM Select
          00 - 1*16 KB
          01 - 4*16 KB
          10 - 1*64 KB
          11 - 64 KB High Speed
SPD     - Disable sprite display
B/W     - Set black and white in 32 tones
LN      - Set screen height to 212 (0 = 192)
SYM1/0  - Synchronization mode
          00 - Intern
          01 - Mix
          10 - Extern (=> Digitize)
          11 - None
IL      - Interlace
EO      - Even or Odd Display  (0: one page; 1: two pages)
NTSC    - TV mode select (0: NTSC; 1: PAL)
DCD     - Dot Clock Direction (0: output)
C3/0    - Colour
BC3/0   - Back Colour
ON3/0   - Blink on  (1 period = 0.20 s)
OF3/0   - Blink off (1 period = 0.20 s)
F16/14  - VRAM acess base address
RS3/0   - Status-register number
C3/0    - Color code
AII     - Auto increment (0: on; 1:inhibit)
RC5/0   - Control-register number
dV3/0   - Vertical adjust display
dH3/0   - Horizontal adjust display
IL7/0   - Interrupt line
CBX5/0  - Color burst value of phase 0   (Preset: 00000000b)
CBY5/0  - Color burst value of phase 1/3 (Preset: 00111011b)
CBZ5/0  - Color busrt value of phase 2/3 (Preset: 00000101b)
DO7/0   - Display offset
CMD     - Command mode (0: normal; 1: screen 2-4 as screen 8)
VDS     - ???
YAE     - Colour palette RGB output
YJK     - YJK system
WTE     - Send a wait-signal to the CPU
MSK     - Mask 8 pixels at edges
SP2     - Screen width for hor. scroll (0: one page; 1: two pages)
H8-0    - Scroll screen horizontall

SX8/0   - Source x-coordinate
SY9/0   - Source y-coordinate
DX8/0   - Destination x-coordinate
DY9/0   - Destination y-coordinate
NX9/0   - Number of x-dots
NY9/0   - Number of y-dots
CL7/0   - Colour code
MXC     - Select access memory
          0: Video RAM
          1: Expansion RAM
MXD     - Select destination memory
          0: Video RAM
          1: Expansion RAM
MXS     - Select source memory
          0: Video RAM
          1: Expansion RAM
DIX     - Direction for NX from X-coordinate
          0: Right
          1: Left
DIY     - Direction for NY from y-coordinate
          0: Down
          1: Up
EQ      - Select execution stop for search command
          0: Stop if another colour is found
          1: Stop if colour is found
MAJ     - Direction for long-side for line command
          0: Along x-axis
          1: Along y-axis
CM3/0   - Command                                    SR DS NM CL MD MS DI LO
          1111 HMMC - High-speed move CPU  to VRAM    -  *  *  *  *  -  *  -
          1110 YMMM - High-speed move VRAM to VRAM    Y  *  Y  -  *  -  *  -
          1101 HMMM - High-speed move VRAM to VRAM    *  *  *  -  *  *  *  -
          1100 HMMV - High-speed move VDP  to VRAM    -  *  *  *  *  -  *  -
          1011 LMMC - Logical move CPU to VRAM        -  *  *  *  *  -  *  *
          1010 LMCM - Logical move VRAM to CPU        *  -  *  -  *  -  *  -
          1001 LMMM - Logical move VRAM to VRAM       *  *  *  -  *  *  *  *
          1000 LMMV - Logical move VDP  to VRAM       -  *  *  *  *  -  *  *
          0111 LINE - Draws a line                    -  *  *  *  * MA  *  *
          0110 SRCH - Searches for a colour           *  -  -  *  * EQ  X  -
          0101 PSET - Draws a dot                     -  *  -  *  *  -  -  *
          0100 PINT - Returns the colour of a dot     *  -  -  -  -  *  -  -
          0000 STOP - Aborts any command              -  -  -  -  -  -  -  -
LO3/1   - Logical operation
          0000 IMP         1000 TIMP
          0001 AND         1001 TAND
          0010 OR          1010 TOR
          0011 XOR         1011 TXOR
          0100 NOT         1100 TNOT

------------------

Alias:Zelly

Group:Mayhem

Tasks:Code + Gfx

--------------------------------

Email:m.zellenrath@stud.tue.nl

Snail:Zelly

Hoefakker 2

4901 GC Oosterhout Nbr.

The Netherlands

--------------------------------

See also:

VRAM-Layout

VDP-behaviour

Screen 3

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("vdp-behaviour", "", "Hardware", "", "Comportamento do VDP - VDP-behaviour: First of all a few general notes which seem to...",
        R"MAN(HARDWARE: MSX-Video (V9958)
Comportamento do VDP - VDP-behaviour

First of all a few general notes which seem to apply to all commands:

1) The SX and DX registers are 9 bits wide, at least when the registers are initialized. There might be a 10th bit or a carry bit or something like that which is used for border detection. On screen 5 and screen 8, the highest bit is ignored when a byte or pixel is written to the VRAM or read from the VRAM

2) The SY and DY registers are 10 bits wide. On screen 7 and 8, the highest bit is ignored when a byte or pixel is written to the VRAM or read from the VRAM

3) The NX and NY registers are 10 bits wide. The highest bit of NY is never ignored or clipped of. Not even on screen 7 and 8. And for the NX register: I know that the full range is used for the LINE command, though I'm not sure about the other commands.

4) The byte commands (like HMMV and YMMM) always work on entire bytes. The lowest bits of both the X coordinate (DX, SX) and the width (NX) are simply ignored.

5) Commands that can step in the X direction, like SRCH, LINE and HMMV, first perform the operation they need to do and then they check if they need to do an additional step. Am I still clear? Maybe a little pseudocode shows what I mean:

for (y conditions) {
  do {
    some operation on a pixel/byte
    update x parameters (sx/dx/nx)
  } while (x and other conditions)
}

6) The end condition for stepping in the X direction is either NX=0 or DX/SX is out of the legal X range for the current screen mode. On screen 5/8: DX/SX<0 or >255 (bit 8 is set). On screen 6/7: DX/SX<0 or >511 (bit 9 or a carry bit or something like that has been set).

7) When you combine note 1), note 5) and note 6) you get some very interesting behaviours on screen 5 and screen 8. For example for LMMV, when you start with DX=256 and x-direction is to the left, the following will happen in the X-loop: The pixel at x=256 will be set. Since the highest bit is ignored, in reality the pixel at x=0 is set. The x-coordinate is decreased with one and becomes 255. The x-coordinate is checked to see if it is in the legal range. Guess what: it is!. So, the vdp will set the pixel at coordinate x=255, x=254, and so on until NX=0.

8) Point 7) is also valid for the SRCH command, the LINE command and the HMMV command.

9) When stepping downwards in the Y-direction, the Y coordinate wraps from 1023 to 0. When stepping up, the command does not wrap from 0 to 1023. In stead, the VDP stops after decreasing from 0 to whatever (call it -1 or 1023 or Carry or whatever you like).

Now that you know these general guidelines, I will explain the details of the commands one by one. I will reference the commands by their official names as I'm more used to the official names. Between brackets I will mention the names which you have used in your source (if there is a difference):

1) POINT (PGET):

SX and SY are mapped into the legal range of the screen mode (using an AND mask). The real vdp will most probably NOT apply any AND mask to the coordinates but simply ignore the highest bits on the screenmodes where they are not needed.

2) PSET:

Same story as for POINT

3) SRCH:

The main work is done in an X-loop which is following the generic structure I mentioned in note 5). That is: the VDP will fetch the point at (SX, SY), compare it to the search color, abort the loop if the color matches (or mismatches, depending on the search flag), decrease or increase SX (depending on the search direction) and only after that, the vdp will check if the border has been reached. Please notice again that SX will not be masked to the range 0...255 on screen 5 and screen 8! The highest bit will simply be ignored by the address generation logic when fetching the point.

4) LINE:

The well-known incremental line algorithm is used. NX is always used as the major length and NY is always used as the minor length. When VDP[45]&1 is 0 (X-axis is major direction), DX will be increased/decreased each time and DY will be increased/decreased only when sum(minor_direction)>major_direction. When VDP[45]&1 is 1 (Y-axis is major direction), DY will be increased/decreased each time and DX will be increased/decreased only when sum(minor_direction)>major_direction. Please notice that the variable which is used as the sum(minor_direction) has a 10-bits range. This does impact the exact behaviour if somebody has put a larger value in the minor length then in the major length. As usual, DX is NOT masked to a 8 bits range on screen 5 and 8. The end condition for the line algorithm is always: NX=0 or DX is out of range.

5) HMMV (FFILL):

This command always works with complete bytes. Which means that for example on screen 5, setting NX=1 is the same as setting NX=0. Or setting DX=7 is the same as DX=6. To make life easy, I always divide NX and DX by the number of pixels per byte in my routine. Then I don't need to worry further about the number of pixels per byte. The real vdp always works with NY in a 10-bits range. For example, when you start with NY=612 on screen 7, 100 lines will be filled-up twice. At least, when you are filling in the downward direction as DY only wraps from 1023 to 0 when going down. The highest bit of DY will simply be ignored at the moment that the real VDP generates the address. Though, as you can not see that a line is drawn twice I say in the emulator that NY=512 when it is initialized with a larger value then 512.

6) LMMV (FILL):

Almost same story as for the HMMV command. Only difference is that now I do not limit NY to 512 on screen 7 and 8. With LMMV there can be a difference when you draw a line twice. For example if you are drawing with the XOR mask. At the moment that the line is drawn for the second time, the screen will be changed back to its original state. Please notice that you can really see that happen on a real MSX, as the LMMV command is rather slow. It takes something like 6 interrupts to fill one page of vram with the LMMV command...

7) YMMM (HCOPY):

I'm still working on this one. I have already made a few straightforward testcases of which I assumed that they would pose no problem to the emulator. Unfortunately, they did.

The information provided above was discovered and documented by Alex Wulms/XelaSoft

Lex Lechz: found also a strange behaviour of VDP read- and write-adresses. As we know read-adresses and write-adresses must be set separately. Independent from this definition these adresses seems to have some sort of connection:

1) If setting a VDP read-adress the write-adress becomes the value+1. This means that setting the VRAM read-adress to 0x35FF sets also the VRAM write-adress to 0x3600.

2) Reading or writing to VRAM increases both - read- and write-adress - of VRAM (fact).

3) If setting a VDP-write adress the read-address becomes the value-1. This means that setting the VRAM write-address to 0x3600 sets also the VRAM read-adress to 0x35FF (not verified).

4) The behaviour described above is verified on V9938 and V9958 video-processors. I don't know if the TMS9918-video processor has the same behaviour or handles read-/write-addresses in a different way.

The described behaviour is very strange and can also interpreted in another way, which is not verified yet: (a) setting the VRAM read-adress saves this address as single read/write-address; (b) setting the VRAM write-adress saves the adress-value -1 as single read/write-adress; (c) reading a byte from VRAM reads from the specified address and increments the current address - where (d) writing a byte to VRAM increments the address first and then writes the byte to the new address.

This really strange behaviour of VRAM can be verified by running the "Fony Demo Disk 1" on MSX-emulations. Emulators which are taking care of the described behaviour will display the graphics correctly, where other Emulators are corrupting the screen-display (smashing the contents of screen 5 and 8, where sprite-animation should be displayed).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("screen-3", "", "Hardware", "", "VDP Screen 3 - SCREEN 3: F-nano2 had problems while executed on RuMSX prior V0.40.",
        R"MAN(HARDWARE: MSX-Video (V9958)
VDP Screen 3 - SCREEN 3

F-nano2 had problems while executed on RuMSX prior V0.40.
This are:

- Strange behaviour of original VDP in screen 3 using high VRAM area (.

Cause:

VDP in screen 3 has another undocumented behaviour, which is the reason that this game does not display it's graphics properly.

Technical notes:

K-KAZ playtested RuMSX 0.12b with his F-NANO2' and returned following comments:

Most important suggestions are:

1. Refer to value of VDP#23 when displaying sprites in screen 3

2. Real MSX has a bug which is not noted in specification books, and F-nano2 relies on it.

I have seen Fnano2' screen with RuMSX 0.12b.. Though the display is almost as garbles, assignments of screen-modes are proper upper quarter is screen 4, and lower half is screen 3. Furthermore, even if a small racing-course map (something on upper-left) is displayed! Amazing.

As far as I can see, it is a matter of time before I can see my game exactly being emulated.

About background in screen 5, it looks OK at first sight, but if you look at it carefully, you'll find they are little improper. (However, I admit they are double-scrolling for real!)

Firstly, about screen 3 area. Sprites are displayed too high. I guess RuMSX is not refering value of VDP#23 for screen 3 when it displays sprite.

I cannot understand why area displayed in screen 3 is incorrect., but do find fragments of colors which are supposed to appear in screen contained in the garbled screen 3 display.

Therefore my opinion, you are not properly emulating pattern name table.

Also about screen 4 area, I can see a number "3" filling it up. So, I think pattern generator table is working properly. Pattern name table must be causing problem.

About reasons of these, I assume, because I programmed F-nano2' with bug of VDP in real MSX in mind. And since you emulated MSX to act exactly as stated in specification books, my game is not working properly on RuMSX.

I explain, in game screen of F-nano2', over all of ist screen, value of VDP#2 (pattern name table address) is 01111111B. This is supposed to specify page 3 (018000H) in screen 5 mode, and in screen 3 or 4, this is supposed to specify 01FC00H.

However, in this state, real VDP understands pattern name table for screen 3 or 4, as 017C00H. I don't know the reason. Biggest possiblity: when you are using horizontal scroll facility of V9958, when MSX is displaying an odd-number page, it appears on scroll concatenated to even-number page at it's side (if value of horizontal scroll is 0 then even number page appears on screen). An I think this behaviour is affecting screen 3 or 4, thereby causing MSX to read what is supposed to be 01FC00H as 017C00H. This is, after all, only an estimate.

I suppose, if you correct this glitch, and sprite display position, RuMSX can display F-nano2' screen properly...

There are too many unknown facts about MSX.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("vram-layout", "", "Hardware", "", "Layout da VRAM - VRAM-Layout: Extra information about VRAM usage in different graphic...",
        R"MAN(HARDWARE: MSX-Video (V9958)
Layout da VRAM - VRAM-Layout

Extra information about VRAM usage in different graphic screens.

First to illustrate this problem, run the following program on an MSX computer.

10 screen 8:set page 1,1:cls
20 line (0,0)-(15,15),15,bf
30 screen 5:set page 1,1
40 a$=input$(1):' keypress
50 set page 3,3
60 a$=input$(1):' keypress

As you can see this program draws a white filled square on page 1 of screen 8. After that screen 5 is displayed, first page 1, after a keypress page 3. What you will on screen 5, page 1 and page 3, is 8 vertical stripes.

Screen 8 | Screen 5

So apparently page 1 of screen 8 is stored at the same absolute location as the page 1 and page 3 of screen 5 together.

Screen 7:

When in screen 7 the first byte of VRAM contains the first two pixels (0,0) and (1,0) of page 0. And the second byte contains the following two pixels (2,0) and (3,0) of page 0. This is exacly want you would expect, so nothing strange so far.

Screen 8:

When in screen 8 the first byte of VRAM contains the first pixel (0,0) of page 0. And the second byte contains the following pixel (1,0) of page 0. Again, this is exacly want you would expect, so nothing strange so far.

==> So except for the color-depth, you could say screen 7 and 8 use the VRAM the same way.

Now for screen 5 and 6, it's a little different:

Screen 5:

When in screen 5 the first byte of VRAM contains the first two pixels (0,0) and (1,0) of page 0.

The second byte contains the first two pixels (0,0) and (1,0) of PAGE 2 !!!

Screen 6:

When in screen 6 the first byte of VRAM contains the first four pixels (0,0),(1,0),(2,0),(3,0) of page 0. The second byte contains the first four pixels of PAGE 2 !!!

====> The graphic pages or stored as follows:

Absolute
address:       SCREEN 5    SCREEN 6    SCREEN 7    SCREEN 8

$0000-$7FFF    PAGE 0+2    PAGE 0+2    PAGE 0      PAGE 0

$8000-$FFFF    PAGE 1+3    PAGE 1+3    PAGE 1      PAGE 1

$0 in VRAM (*)

                    PAGE 0                             PAGE 2
                 +---------+                       +---------+
SCREEN_5:      (0,0)     (1,0)                   (0,0)     (1,0)
SCREEN_6:   (0,0)(1,0) (2,0)(3,0)             (0,0)(1,0) (2,0)(3,0)
            +----+----+----+----+             +----+----+----+----+
            |      BYTE #0      |             |      BYTE #1      |
            +----+----+----+----+             +----+----+----+----+
SCREEN_7:      (0,0)     (1,0)                   (2,0)     (3,0)
SCREEN_8:           (0,0)                             (1,0)
                      +------------ PAGE 0 -------------+

$8000 in VRAM

                    PAGE 1                             PAGE 3
                 +---------+                       +---------+
SCREEN_5:      (0,0)     (1,0)                   (0,0)     (1,0)
SCREEN_6:   (0,0)(1,0) (2,0)(3,0)             (0,0)(1,0) (2,0)(3,0)
            +----+----+----+----+             +----+----+----+----+
            |      BYTE #0      |             |      BYTE #1      |
            +----+----+----+----+             +----+----+----+----+
SCREEN_7:      (0,0)     (1,0)                   (2,0)     (3,0)
SCREEN_8:           (0,0)                             (1,0)
                      +------------ PAGE 1 -------------+

(*) NOTE: the dollar sign in front of a number indicates a hexadecimal number.

The information provided above was discovered and documented by Jan Wilmans.

Additional notes by Lex Lechz: the behaviour described above is the one that can be noticed by trying to find out the VRAM-layout in the documented manner. After implementing this layout for the 1st time subsequent tests (with "F-nano2" and "Space Manbow") showed that this description is not 100% correct. Additional facts are:

The described layout for SCREEN 5 and SCREEN 6 seems also to be valid for SCREEN 0 (40 and 80), SCREEN 1, SCREEN 2, SCREEN 3 and SCREEN 4. So only SCREEN 7, SCREEN 8 and other 64KB modes are exceptions from this rule.

So we could say that basic modes (SCREEN 0 till SCREEN 6) are using "flat" linear adresses and higher modes have the "tiled" adress-space...

Also: invalid display modes are compatible to the MSX1 (SCREEN 0-3) VRAM layout.
This has e.g. impact to the MSX1 game "Hunchback", which temporarily disables the
display by (mistakenly?) selecting an invalid mode (while filling data in VRAM).

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("v9990", "hw-v9990,v9990", "Hardware", "", "V9990: The V9990 videochip is implemented in the GFX9000 video-board.",
        R"MAN(HARDWARE: V9990 (VideoChip do futuro)
V9990

The V9990 videochip is implemented in the GFX9000 video-board.

The Videochip of the future (Sunrise Foundation)

The following sections a containing descriptions of all V9990 registers, both the ordinary  known and some 'secret' ones which I found during my work with the GFX9000 PowerBasic, Paint9000, Xbasic and more.

(c) Henrik Gilvad 1996. May be used as long as it's for free!

V9990 commands
V9990 ports
V9990 registers

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("v9990-commands", "", "Hardware", "", "Comandos - V9990 commands: COMMAND",
        R"MAN(HARDWARE: V9990 (VideoChip do futuro)
Comandos - V9990 commands

COMMAND

11.1 Command execution method

After setting the  necessary parameter register, set the command to the operation code register, and it will be executed. As soon as execution is started, the status CE is set to "1" and upon completion, it is reset to "0". Furthermore, the interrupt flag CE is set to "1".

Most commands are issued after the necessary registers out of the following parameters are set. Also some commands require output and input of the necessary data at and through the command data port after they are issued.

  1. Source (for transfer) coordinate.
  2. Destination (for transfer) coordinate.
  3. Transfer range coordinate.
  4. Argument, logical operation and write mask.
  5. Font color

11.2 Parameter setting.

Source (for transfer) coordinate or address

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#32 | SX7  | SX6  | SX5  | SX4  | SX3  | SX2  | SX1  | SX0  |
     +------+------+------+------+------+------+------+------+
     | SA7  | SA6  | SA5  | SA4  | SA3  | SA2  | SA1  | SA0  |
     +------+------+------+------+------+------+------+------+
     | KA7  | KA6  | KA5  | KA4  | KA3  | KA2  | KA1  | KA0  |
     +------+------+------+------+------+------+------+------+
     -------+------+------+------+------+------+------+------+
R#33 | 0    |  0   |  0   |  0    |  0  | SX10 | SX9  | SX8  |
     +------+------+------+------+------+------+------+------+
     -------+------+------+------+------+------+------+------+
R#34 | SY7  | SY6  | SY5  | SY4  | SY3  | SY2  | SY1  | SY0  |
     +------+------+------+------+------+------+------+------+
     | SA15 | SA14 | SA13 | SA12 | SA11 | SA10 | SA9  | SA8  |
     +------+------+------+------+------+------+------+------+
     | KA15 | KA14 | KA13 | KA12 | KA11 | KA10 | KA9  | KA8  |
     +------+------+------+------+------+------+------+------+
     -------+------+------+------+------+------+------+------+
R#35 | 0    |  0   |  0   |  0   | SY11 | SY10 | SY9  | SY8  |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   |  0   | SA18 | SA17 | SA16 |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   |  0   |  0   | KA17 | KA16 |
     +------+------+------+------+------+------+------+------+

There are 3 ways of setting depending on types of command as follows.
1) LMCM, LMMM, BMLX, SRCH, POINT
   SX0-10: Specified with the X coordinate of the image coordinate space (by
           dots) and rolling to "0" occurs when a larger value than the image
           size is used for setting. (When in P1 mode, screen "A" is selected
           at SX9=0 and screen "B" at SX9=1.)
   SY0-11: Specified with the Y coordinate of the image coordinate space (by
           dots) and rolling to "0" occurs when a larger value than the image
           size is used for setting.

2) CMMM, BMXL, BMLL
   SA0-18: Specified with the address on the VRAM map (by bytes).

3) CMMK
   KA0-17: Specified with the kanji ROM address (by bytes).

Destination (for transfer) coordinate or address

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#36 | DX7  | DX6  | DX5  | DX4  | DX3  | DX2  | DX1  | DX0  |
     +------+------+------+------+------+------+------+------+
     | DA7  | DA6  | DA5  | DA4  | DA3  | DA2  | DA1  | DA0  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#37 | 0    |  0   |  0   |  0   |  0   | DX10 | DX9  | DX8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#38 | DY7  | DY6  | DY5  | DY4  | DY3  | DY2  | DY1  | DY0  |
     +------+------+------+------+------+------+------+------+
     | DA15 | DA14 | DA13 | DA12 | DA11 | DA10 | DA9  | DA8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#39 | 0    |  0   |  0   |  0   | DY11 | DY10 | DY9  | DY8  |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   |  0   | DA18 | DA17 | DA16 |
     +------+------+------+------+------+------+------+------+

There are 2 ways of setting depending on types of command as follows.
1) LMMC, LMMV, LMMM, CMMC, CMMK, CMMM, BMXL, LINE, PSET, ADVN
   DX0-10: Specified with the X coordinate of the image coordinate space (by
           dots) and rolling to "0" occurs when a larger value than the image
           size is used for setting (When in P1 mode, screen "A" is selected
           at DX9=0 and screen "B" at DX9=1.)
   DY0-11: Specified with the Y coordinate of the image coordinate space (by
           dots) and rolling to "0" occurs when a larger value than the image
           size is used for setting.

2) BMLX, BMLL
   DA0-18: Specified with the address on the VRAM map (by bytes)

Transfer range coordinate or address

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#40 | NX7  | NX6  | NX5  | NX4  | NX3  | NX2  | NX1  | NX0  |
     +------+------+------+------+------+------+------+------+
     | NA7  | NA6  | NA5  | NA4  | NA3  | NA2  | NA1  | NA0  |
     +------+------+------+------+------+------+------+------+
     | MJ7  | MJ6  | MJ5  | MJ4  | MJ3  | MJ2  | MJ1  | MJ0  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#41 | 0    |  0   |  0   |  0   |  0   | NX10 | NX9  | NX8  |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   | MJ11 | MJ10 | MJ9  | MJ8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#42 | NY7  | NY6  | NY5  | NY4  | NY3  | NY2  | NY1  | NY0  |
     +------+------+------+------+------+------+------+------+
     | NA15 | NA14 | NA13 | NA12 | NA11 | NA10 | NA9  | NA8  |
     +------+------+------+------+------+------+------+------+
     | MI7  | MI6  | MI5  | MI4  | MI3  | MI2  | MI1  | MI0  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#43 | 0    |  0   |  0   |  0   | NY11 | NY10 | NY9  | NY8  |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   |  0   | NA18 | NA17 | NA16 |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   | MI11 | MI10 | MI9  | MI8  |
     +------+------+------+------+------+------+------+------+

There are 3 ways of setting depending on types of command as follows.
1) LMMC, LMMV, LMCM, LMMM, CMMC, CMMK, CMMM, BMXL, BMLX
   NX0-10: The number of dots in X direction of the image coordinate space is
           specified and rolling to "0" occurs when the specified range
           exceeds the image size. (Maximum values is 2048 dots when all "0".)
   NY0-11: The number of dots in Y direction of the image coordinate space is
           specified and rolling to "0" occurs when the specified range
           exceeds the image size. (Maximum value is 4096 when all "0".)

2) BMLL
   NA0-18: The number of bytes on the VRAM map is specified and rolling to
           address "0" occurs when the specified range exceeds the VRAM
           capacity. (Maximum value is 512k bytes when all "0".)
           Note) 256 bytes when NA7 to NA0 are all "0"

3) LINE
   MJ0-11: The length of the longer side of the rectangle whose diagonal line
           is the same as the drawn line is specified by dots. When the
           specified range exceeds the image size, rolling to "0" occurs.
   MI0-11: The length of the shorter side of the rectangle whose line is the
           same as the drawn line is specified by dots. When the specified
           range exceeds the image size, rolling to "0" occurs.

Argument

       b7     b6     b5     b4       b3      b2     b1    b0
     +------+------+------+------+------+------+------+------+
R#44 | 0    |  0   |  0   |  0   | DIY  | DIX  | NEQ  | MAJ  |
     +------+------+------+------+------+------+------+------+

DIY DIX: The transfer direction (plus direction at "0" and minus direction at
         "1") from the base coordinates is specified. With BMXL and BMLX,
         however, the linear address specified side is fixed to "plus" and
         with BMLL, both X and Y are specified to the same direction.
NEQ:     In the border color specification for SRCH, "0" is for specified
         color detection and "1" is for non-specified color detection.
MAJ:     In the line inclinatino specification for LINE, X is the longer side
         and Y the shorter side at "0" while Y is the longer side and X the
         shorter side at "1".

Logical operation

       b7     b6     b5     b4       b3      b2     b1    b0
     +------+------+------+------+------+------+------+------+
R#45 | 0    |  0   |  0   | TP   | L11  | L10  | L01  | L00  |
     +------+------+------+------+------+------+------+------+

When  drawing  by means  of commands,  it is possible to write the value (WC) obtained  through the  logical operation using the value to be drawn (SC) and the targer value (DC). The content of the operation is specified as follows.
  L00: WC value for the bit with SC=0 and DC=0
  L01: WC value for the bit with SC=0 and DC=1
  L10: WC value for the bit with SC=1 and DC=0
  L11: WC value for the bit with SC=1 and DC=1
  TP : When this bit is "1", the data with SC as all "0" (by dot for the XY
       coordinates and by byte for the linear address) is not transferred.

<Example>
        ------------------+---+---+---+---+-----+--+
        |Logical operation|L11|L10|L01|L00|Name |# |
        +-----------------+---+---+---+---+-----+--+
        |WC = SC          | 1 | 1 | 0 | 0 | PSET|12|
        +-----------------+---+---+---+---+-----+--+
        |WC = !SC         | 0 | 0 | 1 | 1 | NOT |3 |
        +-----------------+---+---+---+---+-----+--+
        |WC = (SC AND DC) | 1 | 0 | 0 | 0 | AND |8 |
        +-----------------+---+---+---+---+-----+--+
        |WC = (SC OR DC)  | 1 | 1 | 1 | 0 | OR  |14|
        +-----------------+---+---+---+---+-----+--+
        |WC = (SC XOR DC) | 0 | 1 | 1 | 0 | XOR |6 |
        +-----------------+---+---+---+---+-----+--+

Write mask

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#46 | WM7  | WM6  | WM5  | WM4  | WM3  | WM2  | WM1  | WM0  |
     +------+------+------+------+------+------+------+------+
R#47 | WM15 | WM14 | WM13 | WM12 | WM11 | WM10 | WM9  | WM8  |
     +------+------+------+------+------+------+------+------+

When drawing  by means    of commands,  writing can be prohibited by bits. With WM0-7 and WM8-15, VRAM0 bits and VRAM1 bits are specified respectively. Then "1" is for write enable and "0" is for write prohibit.
In P1  mode, writing  is prohibited on the side not specified as the transfer destination. (Screen "A":R#46, Screen "B":R#47)

Font color

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#48 | FC7  | FC6  | FC5  | FC4  | FC3  | FC2  | FC1  | FC0  |
     +------+------+------+------+------+------+------+------+
R#49 | FC15 | FC14 | FC13 | FC12 | FC11 | FC10 | FC9  | FC8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#50 | BC7  | BC6  | BC5  | BC4  | BC3  | BC2  | BC1  | BC0  |
     +------+------+------+------+------+------+------+------+
R#51 | BC15 | BC14 | BC13 | BC12 | BC11 | BC10 | BC9  | BC8  |
     +------+------+------+------+------+------+------+------+

The font color (color code) is specified for CMMC, CMMK and CMMM. Also, the drawing color  for LMMV, LINE and PSET and the border color for SRCH are specified by using FC0-15.
Correspondance with VRAM bit position is the same as write mask.
 FC0-15: Color code of font data "1"
 BC0-15: Color code of font data "0"

Note)  Depending on  conditions of R#6 (CLRM1, CLRM0), use the following data setting for FC0-15 of R#48 and R#49 (BC0-15 of R#50 and R#51).
  16 bit/dot --- Write 16 bit data for FC0-FC15
   8 bit/dot --- Write the same data for FC0-7 and FC8-15
   4 bit/dot --- Write the same data for FRC0-3, FC4-7, FC8-11 and FC12-15.
   2 bit/dot --- Fill FC0-15 with the same eight 2-bit unit data.

11.3 Setting of operation code

Operation code

       b7     b6     b5     b4       b3      b2     b1    b0
     +------+------+------+------+------+------+------+------+
R#52 | OP3  | OP2  | OP1  | OP0  | AYM  | AYE  | AXM  | AXE  |
     +------+------+------+------+------+------+------+------+

When a code from the list below is written in this register, the corresponding command will be executed.

+---------------+------+----------------------------------------------------+
|OP3 OP2 OP1 OP0| Name | Operation                                          |
+---------------+------+----------------------------------------------------+
| 0   0   0   0 | STOP |Command being executed is stopped.                  |
+---------------+------+----------------------------------------------------+
| 0   0   0   1 | LMMC |Data is transferred from CPU to VRAM rectangle area.|
+---------------+------+----------------------------------------------------+
| 0   0   1   0 | LMMV |VRAM rectangle area is painted out with FC0-15      |
+---------------+------+----------------------------------------------------+
| 0   0   1   1 | LMCM |VRAM rectangle area data is transferred to CPU.     |
+---------------+------+----------------------------------------------------+
| 0   1   0   0 | LMMM |Rectangle area data is transferred from VRAM to VRAM|
+---------------+------+----------------------------------------------------+
| 0   1   0   1 | CMMC |CPU character data is colordeveloped and transferred|
|               |      |to VRAM rectangle area.                             |
+---------------+------+----------------------------------------------------+
| 0   1   1   0 | CMMK |Kanji ROM data is colordeveloped and transferred to |
|               |      |VRAM rectangle area.                                |
+---------------+------+----------------------------------------------------+
| 0   1   1   1 | CMMM |VRAM character data is colordeveloped and transfer- |
|               |      |red to VRAM rectangle area.                         |
+---------------+------+----------------------------------------------------+
| 1   0   0   0 | BMXL | Data on VRAM linear address is transferred to VRAM |
|               |      | rectangle area.                                    |
+---------------+------+----------------------------------------------------+
| 1   0   0   1 | BMLX | VRAM rectangle area data is transferred onto VRAM  |
|               |      | linear address.                                    |
+---------------+------+----------------------------------------------------+
| 1   0   1   0 | BMLL | Data on VRAM linear address is transferred onto    |
|               |      | VRAM linear address.                               |
+---------------+------+----------------------------------------------------+
| 1   0   1   1 | LINE | Straight line is drawn on XY coordinates.          |
+---------------+------+----------------------------------------------------+
| 1   1   0   0 | SRCH | Search horizontally for Equal or Nonequal color    |
+---------------+------+----------------------------------------------------+
| 1   1   0   1 | POINT| Color code of specified point is read out.         |
+---------------+------+----------------------------------------------------+
| 1   1   1   0 | PSET | Drawing is executed at drawing point on X-Y coor.  |
+---------------+------+----------------------------------------------------+
| 1   1   1   1 | ADVN | Drawing point on X-Y coordinates is shifted        |
+---------------+------+----------------------------------------------------+
With PSET and ADVN, the drawing point can be shifted after executing the command.
  AXE AXM
   0   0    (DX,DY) is used as a pointer.
   0   1    The pointer is not shifted.
   1   0    The pointer is shifted to the right by 1 dot.
   1   1    The pointer is shifted to the left by 1 dot.

  AYE AYM
   0   0    The pointer is not shifted.
   0   1    The pointer is not shifted.
   1   0    The pointer is shifted down by 1 dot.
   1   1    The pointer is shifted up by 1 dot.

11.4 Command data

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("v9990-2", "", "Hardware", "", "Futuro - V9990: - THE VIDEOCHIP OF THE FUTURE -",
        R"MAN(HARDWARE: V9990 (VideoChip do futuro)
Futuro - V9990

                       - THE VIDEOCHIP OF THE FUTURE -

                                  V 9 9 9 0
                                 ===========

There have been no real new hardware developments for the MSX since the MSX turbo R. But now the MSX-Hndlergemein- schaft from Switzerland are working on the "Graphics 9000", a 16-bit videocard for the MSX turbo R with the new V9990 by Yamaha. The "Graphics 9000" will be available in October or November 1994 for approximately US$ 290.

EVALUATION BOARD

Since the International MSX Fair in Tilburg on March 26th 1994 I'm the proud owner of one of the 10 V9990 evaluation boards which are in Europe at the moment. The evaluation board is a print (appr. 6" x 6") with a lot of electronic stuff I don't understand with in the middle the V9990 itself which is square like the R800. The V9938 (MSX2 videochip) and V9958 (MSX2+) were rectangular. The connections on the board include a connect for an interface to the MSX turbo R and one for a scart lead.

You can connect the board to a monitor or TV directly with a scart lead, so the image of the V9990 goes directly to the monitor. This is a small disadvantage if you have only one monitor like me, because this means you have to switch the leads when you want to work on the V9990 after working on the V9958 or vice versa. This problem will probably be solved with the Graphics 9000.

You can connect a VGA monitor (640x400 or 640x480 in 16 colours) or even an LCD to the V9990, but unfortunately I haven't been able to test this yet.

V9990 BASIC

If you connect the V9990 to your turbo R and monitor and you switch the computer on, the screen will remain blank at first. This is because the operating system of the turbo R still uses the V9958. You have to execute a V9990 program before you can see anything.

Henrik Gilvad from Denmark modified the BASIC of the turbo R so it works with the V9990. He also modified the KUN compiler (also known as XBASIC). The V9990 BASIC, BIOS and KUN are loaded into the DRAM (*) of the turbo R. The BASIC is not completely compatible with the normal BASIC, it doesn't have to be and it's not possible due to some hardware differences. But there aren't so many differences so it's not hard at all to program something in the V9990 (KUN)BASIC.

(*) Note: Because ROM is significantly slower then RAM on the turbo R, it has the possibility to copy the ROMs to the upper 64 kB of the main RAM which is called DRAM. The engine of the turbo R (S1990) makes it possible to use this DRAM as "ROM".

An important difference between the V9938 and the V9990 is that the V9990 hasn't got a textmode. Henrik solved this problem by programming his own textmode in the P2-mode of the V9990. It's a very fast 64-column textmode with International character set, so the Japanese characters of the turbo R can't bother you anymore.

The V9990 BASIC supports the following screen modes:

screen  remark

0       64 column textmode
1       will be 32 column textmode
2       P1 mode (256 x 212 dualplane patternmode)
3       P2 mode (512 x 212 patternmode)
4       identical to SCREEN 2
5..12   identical to MSX2/2+ except the sprites, only 2 sprites in
        one colour are possible

The V9990 supports a lot more screen modes, which can easily be selected by a few VDP commands. For example, you can select the overscan mode by OUT &H67,1.

In the overscan mode you can use the entire screen, including the borders! The resolution is 384x240 in stead of 256x212 or 768x240 in stead of 512x212.

The following graphical instructions are supported by the V9990 BASIC:

BLOAD,S, BSAVE,S, CIRCLE, COLOR, COPY, DRAW, KEY ON/OFF, LINE, LOCATE, PAINT, POINT, PRESET, PSET, PRINT, PRINT USING, PRINT#, PUT SPRITE, SCREEN, SET ADJUST, SET PAGE, SET SCROLL, VDP, VPEEK, VPOKE and WIDTH. Of course the non-graphical instructions are not modified.

512 kB VRAM

The V9990 has a lot of VRAM: 512 kB!!! This means that you have 16 pages in SCREEN 5 and 6 and 8 pages in SCREEN 7..12. Doesn't something like SET PAGE 13,15 look great?!

SPEEDY GONZALES

The V9990 is extremely fast!!! For example, you can make a full screen smooth scroll in SCREEN 5 with a BASIC program using COPY! The copy command of the V9990 is 9 times faster than the "high-speed" copy command of the V9938! On the V9938 you have to use a "logical" copy if you want to copy pixels in stead of bytes or if you want to use logical operations. On the V9990 this isn't necessary so those copies are 23 times faster!!! Note: In MSX-BASIC all copies are treated as logical copies on the MSX2/2+/turbo R, so the COPY command in BASIC is 23 times faster with the V9990 BASIC!

I/O SPEED

The V9990 has faster commands but it also allows faster I/O! The V9938 can hardly keep up with the Z80, so the R800 of the turbo R is slowed down when it communicates with the V9958. This isn't necessary with the V9990, so you can really use the speed of the R800.

The evaluation board and Graphics 9000 use I/O addresses &H60..&H6F. These ports are called P#0..P#F. P#0..P#7 are used for normal VDP operations, P#8..P#B are for the kanji ROMs (not supported on Graphics 9000, but the turbo R has its own kanji ROMs) and P#C..P#F are reserved for future use.

FLI-PLAYER

On the International MSX Fair in Tilburg on April 3rd 1993 Anarchy from Germany/Switzerland released their Ray Tracing demo. This demo has a lot of beautiful ray-traced animations in SCREEN 8, which are real-time decrunched. It looks nice but it's not smooth because the V9938 is too slow.

Mi-Chi (a member of Anarchy) made a FLI-Player for the V9990. It works the same, some of the animations were used in the Ray Tracing demo too. But on the V9990 the animations are really SMOOTH!!! A fast harddisk (with SCSI interface from Henrik Gilvad) is recommended because some of the .FLI files are 400 kB and there is even a new .FLI player with files upto 2 MB. Unfortunately I can't watch these large files because I haven't got enough RAM.

DRAWING PROGRAMS

The Graphics 9000 will be sold with two drawing programs. One for B1-mode, 256x212 with 64 colours out of a palette of 32768, and one for P2-mode, 512x212 with 16 colours out of a palette of 32768 colours. The B1-program looks like DD-Graph and the P2-program will have a lot of options and used pull down menus.

VIDEO

The Graphics 9000 will be capable of super imposing, which makes it ideal for videofreaks. Mi-Chi is working on a program for animations, wipes, etc. The advantages of the Graphics 9000 compared to the Philips 8280 are:

- the V9990 is much faster

- on the 8280 you can't use the border, the V9990 has the overscan mode

- the V9990 has better graphics (upto 512x424 with 32768 colours) for digis

Digitizing is not possible yet with the V9990, but it will be possible and that will be amazing because you can digitize in the 512x424 mode with 32768 colours!!! This means that you can't see the difference between the digi and the original!

One of the demodisk are two 512x424 YUV digis. These digis are really brilliant, you can't see the difference with normal TV quality! YUV is a system for encoding 19268 colours using only 8 bits per pixel similar to the YJK system used in SCREEN 12 on the MSX2. It has the same disadvantage as YJK (colour spill because 4 adjacent pixels have the same U and V values, only the brightness (Y) can be different for each pixel), but you can hardly see it in such a high resolution. You can imagine the quality of pictures with 32768 colours without colourspill!!!

GAMES

I'm the programmer of UMAX, famous from our games Pumpkin Adventure and The Witch's Revenge. It's very frustrating if you can't program what you want because the possibilities and/or speed of the videochip. This happens a lot if you program something for the MSX2! The number of characters moving on the screen and the size of them is a problem, and a nice scrolling routine is even worse.

The V9990 has two modes ideal for games: P1 and P2. These are patternmodes, the screen consists of patterns of 8x8 pixels each. The resolutions are 256x212 and 512x212. With these pattern modes it is much more easy to make much nicer games for the V9990 than for the MSX2!

DUALPLANE

The P1-mode is better known as the dualplane mode, because it has two screens which can scroll independently! You see the back screen where the front screen is transparent. It's very easy to make fantastic multi layer scrolls using this dualplane mode! Each plane can have one of the 4 palettes of 16 colours out of 32768.

A very special feature of the P1-mode is a kind of hardware screensplit. You can select a kind "border" in 64-pixel units. Above and on the left-hand side of this border plane A is the front plane, and below and on the right-hand side of this border plane B is the front plane.

PATTERNS

Because the V9990 has so much VRAM it can store over 15000 patterns. The format of the patterns is very nice, it's equal to a SCREEN 5 picture with 8x8 blocks. So it's very easy to use the patterns!

Games for the MSX2 are often in SCREEN 5 and use 8x8 blocks. The screen construction routine for such games is rather complicated, because for each pattern you have to calculate at which coordinates it is in the VRAM, and it has to be copied to the right position on the visible screen. This is slow and difficult.

On the V9990 it's both very simple and fast: just VPOKE the pattern number (16 bits) in the pattern nametable. Are you now beginning to understand what I mean with "it is much more easy to make much nicer games for the V9990 than for the MSX2"? But this is not all!

SPRITES

In P1 and P2 modes you can use upto 125 (one hundred and twenty five!) sprites, which is almost 4 times as much as the 32 sprites of the V9938. Upto 16 sprites can be displayed on a single horizontal line, so the times of blinking sprites are over!

But the main advantage of the sprites is that, just like the patterns, they are SCREEN 5 graphics! So you can draw 16x16 sprites in DD-Graph and just load that into the VRAM. Without any conversion you can use those 16x16 blocks as sprites.

On the MSX2 each horizontal line of a sprite can have only 1 colour, on the V9990 all colours out of one of the four palettes of 16 colours out of 32768 can be used. It is possible to get multi-coloured sprites on the MSX2, but you have to use 2 sprites for one "object" to get 3 colours on a horizontal line so you can only have 16 of those 3-colour sprites!

If you want the sprite to move behind a fence and still see the part of the sprite not covered by the fence, that's very hard to do on the MSX2. But all you need to do on the V9990 is putting the fence at the front plane and setting one bit of the sprite attributes!

Another advantage of the V9990 is that in the 512x212 P2-mode the spritepixels have the same size as screenpixels. On the MSX2 one sprite pixel is two screen pixels wide, and the x-coordinate can only be 0..255, on the V9990 the x- coordinate can be 0..511.

The B-modes haven't got sprites because they were not designed to use for games. They do have two special 32x32 cursor sprites.

SOFTWARE

The V9990 is ideal for games, business applications and video applications. Mountainsoft from Switzerland are working on a game, there will be a V9990 version of the DTP program Aladin and a lot more software (including the drawing and video programs I already mentioned) are being developed.

Henrik made a program to watch pictures from a Photo CD using a CD-ROM drive and V9990 and Mi-Chi made a very professional viewer for a lot of formats including GIF, BMP and PCX. The GIF, BMP and PCX pictures look a lot better on the V9990 than on the MSX2 because the V9990 can display more colours (32768) at a high resolution (512x424).

A lot of groups, including UMAX, have plans to make a game for the V9990.

INTERRUPTS

These paragraph is interesting for programmers, if you don't know anything about interrupts you'd better skip it.

The V9990 can generate interrupts caused by the following events:

1) Vertical blank (50/60 Hz)

2) When displaying a certain line (line interrupt)

3) When a command is finished

4) When displaying at a certain HORIZONTAL position

5) At the start of displaying EACH line

The V9938 only supports 1 and 2. The advantage of 3 is clear. With 4 you can make a "vertical screensplit", which is not possible on the V9938 (not a stable one) and 5 is very nice for "waves".

To see what caused the interrupt you have to read a status register on the MSX2, on the V9990 you only have to read the interrupt flag port (P#6) which is much faster.

PROGRAMMING

Programming for the V9990 is more simple or at least as simple as programming for the V9958. A lot of things take less IN/OUT instructions and so they take less time and less bytes.

I already mentioned the interrupt flag port, the V9990 also has a status port with a lot of nice things like "vertical non-display period", "horizontal non-display period", "command being executed", etc. Reading from/writing to the VRAM is more simple too, just compare the following routines to see what I mean!

; SetWriteV9938
; In: CHL = VRAM address (17 bits)

        LD      A,H
        AND     #3F
        OR      #40
        EX      AF,AF'
        LD      A,H
        OR      C
        RLCA
        RLCA
        OUT     (#99),A
        LD      A,128+14
        OUT     (#99),A
        LD      A,L
        OUT     (#99),A
        EX      AF,AF'
        OUT     (#99),A
        RET

; SetWriteV9990
; In: EHL = VRAM address (19 bits)

        LD      C,#64
        XOR     A
        OUT     (C),A
        DEC     C
        OUT     (C),L
        OUT     (C),H
        OUT     (C),E
        RET

RESOLUTIONS AND COLOURS

The V9990 supports the following resolutions:

mode   resolution     remark

P1     256x212(424)   dualplane patternmode
P2     512x212(424)   patternmode
B1     256x212(424)
B2     384x240(480)   overscan
B3     512x212(424)
B4     768x240(480)   overscan
B5     640x400
B6     640x480

Interlaced vertical resolutions are shown in parentheses "()". These resolutions can be combined with the palette systems below, so a lot of different types of screen modes are possible. Systems using 8 or 16 bits per pixel are not possible for B4..B6, P1 and P2 only work with 4 bits per pixel.

bits/pixel  colours   system

16           32768    simultaneously (TrueColour)
 8              64    out of a palette of 32768
 8             256    like SCREEN 8
 8           19268    like SCREEN 12 (YJK)
 8           19268    YUV
 4              16    out of a palette of 32768
 2               4    out of a palette of 32768

DISPLAY SIZE VS IMAGE SIZE

A very nice feature of the V9990 is the image size, which is much larger than the actual display size most of the time, so you can only see a part of the total "image" on your screen. Which part is determined by the scroll registers, so you can scroll smoothly in all directions in all screen modes.

The image size depends of the number of bits used per pixel. The following table is valid for the bitmap modes (B1..B6).

bits/pixel      resolutions

2               256x8192, 512x4096, 1024x2048, 2048x1024
4               256x4096, 512x2048, 1024x1024, 2048x 512
8               256x2048, 512x1024, 1024x 512, 2048x 256
16              256x1024, 512x 512, 1024x 256

In the P1 mode there are two images of 64x64 patterns each (512x512 pixels), the P2 mode has one image of 128x64 patterns (1024x512).

If you use the B1 mode for a game you can put an entire level of the game into the VRAM, because there is space for 16 screens if you use 16 colours. You don't have to use blocks but you can just draw the entire level. Of course the P1 and P2 modes are better for games because they have sprites.

INTERLACED

The V9938 also had an interlaced possibility, but it wasn't used very often because the odd lines of the picture have to be in one page and the even lines in another. Because of this a picture has to be converted before it can be shown in interlaced mode. Because of this animations are almost impossible.

The interlaced mode of the V9990 is much nicer, because it just doubles the display size of the picture. No conversions are necessary and animations are just as easy as in a non interlaced mode.

By the way, the V9990 has a undocumented mode with 16 colours and a horizontal resolution of 1024. Together with the interlaced possibility the maximum resolution a V9990 can show on a normal TV is 1024x424! I must admit the quality isn't very high on my TV, but this should improve if you use a better monitor.

FINALLY

I didn't cover all the possibilities of the V9990, but I think it is more than clear that this videochip offers revolutionary possibilities on an MSX.

A lot of things that are not possible at all on a V9958 or only with a lot of trouble, can be programmed for the V9990 in just a few lines of BASIC! You can imagine what the possibilities are if you use machine code!

I'm going to start with a V9990 programming course in both BASIC and ML as soon as the Graphics 9000 is available.

It's a pity the V9990 wasn't finished soon enough (autumn of 1991) to be used in the MSX turbo R, which was released in the autumn of 1990. Most MSX users have ancient hardware nowadays. The V9938 was developed in 1985, so it's almost ten years old. On other systems it's very normal to upgrade your system every few years.

This has been possible on the MSX (MSX1 1983, MSX2 1985, MSX2+ 1988, MSX turbo R 1990), and now it will be possible again to upgrade your system with a Graphics 9000 and a Moonsound (16-bit soundcard with OPL4). I hope that a lot of MSX users around the world will buy a Graphics 9000 and a Moonsound.

Of course there isn't so much software at first, and because of this people don't buy the Graphics 9000, there will be no more software for it, etc. We have to break this vicious circle! Just buy a Graphics 9000 (and a turbo R if you haven't got one yet), then the software will follow soon. I can assure you a lot of groups who develop software can't wait to start programming for the V9990!

A lot of groups who are developing software for the MSX2 at the moment are not challenged anymore, they made a few games and they want to move on. This is not possible with the MSX2. If we don't upgrade our hardware they will start developing software for other systems like the PC. Unfortunately a lot of groups already made that decision. The only way to stop more groups to take this bad step is to support the new Graphics 9000 and Moonsound.

Stefan Boer

smboer@cs.vu.nl

(c) Sunrise Foundation 1994

Sunrise Foundation

P.O. Box 178

1530 AD  Wormer

The Netherlands

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("v9990-ports", "", "Hardware", "", "Portas - V9990 ports: V9990 I/O port specifications",
        R"MAN(HARDWARE: V9990 (VideoChip do futuro)
Portas - V9990 ports

V9990 I/O port specifications

Port GFX9000 Description
---- ---------------------------------------------------------------
P#0  #60 VRAM DATA
P#1  #61 PALETTE DATA
P#2  #62 COMMAND DATA
P#3  #63 REGISTER DATA
P#4  #64 REGISTER SELECT
P#5  #65 STATUS
P#6  #66 INTERRUPT FLAG
P#7  #67 SYSTEM CONTROL
P#8  #68 Primary standard kanji ROM lower order address
P#9  #69 Primary standard kanji ROM higher order address
P#9  #69 Primary standard kanji ROM data
P#A  #6A Secondary standard kanji ROM lower order address
P#B  #6B Secondary standard kanji ROM higher order address
P#B  #6B Secondary standard kanji ROM data
------------------------------------------------------------------------

VRAM DATA PORT (Read / Write)

      b7  b6  b5  b4  b3  b2  b1  b0
     +---+---+---+---+---+---+---+---+
P#0  |VD7|VD6|VD5|VD4|VD3|VD2|VD1|VD0|
     +---+---+---+---+---+---+---+---+

The data written in this port is written in the VRAM address specified by using #0 to 3. If writing to this port has taken place before completion of data write in VRAM, a WAIT signal is output. By reading through this port, the VRAM address data specified by using R#3 to 5 is obtained. If reading has taken place at this port before data preparation, a WAIT signal is output. When VRAM read address has been modified to prepare the VRAM address data at completion of data write to R#5, it must be written in R#5.

PALETTE DATA PORT (Read / Write)

      b7  b6  b5  b4  b3  b2  b1  b0
     +---+---+---+---+---+---+---+---+
P#1  |PD7|PD6|PD5|PD4|PD3|PD2|PD1|PD0|
     +---+---+---+---+---+---+---+---+

The data written in this port is written in the internal palette RAM. By reading through this port, the internal palette RAM data can be obtained. A WAIT signal is output till the data has been prepared. The palette RAM address is specified by using R#14. The data format is as follows.

       b7  b6  b5  b4  b3  b2  b1  b0
      +---+---+---+---+---+---+---+---+
RED   |!YS| - | - | R4| R3| R2| R1| R0|
      +---+---+---+---+---+---+---+---+
GREEN | - | - | - | G4| G3| G2| G1| G0|
      +---+---+---+---+---+---+---+---+
BLUE  | - | - | - | B4| B3| B2| B1| B0|
      +---+---+---+---+---+---+---+---+

COMMAND DATA PORT (Read / Write)

      b7  b6  b5  b4  b3  b2  b1  b0
     +---+---+---+---+---+---+---+---+
P#2  |CD7|CD6|CD5|CD4|CD3|CD2|CD1|CD0|
     +---+---+---+---+---+---+---+---+

The command data is read and written through this port. Access to this port when the status TR bit is "0" while the command is being executed will result in an output of WAIT signal.

REGISTER DATA PORT (Read / Write)

      b7  b6  b5  b4  b3  b2  b1  b0
     +---+---+---+---+---+---+---+---+
P#3  |RD7|RD6|RD5|RD4|RD3|RD2|RD1|RD0|
     +---+---+---+---+---+---+---+---+

The data written in this port is written in the register specified by using P#4. By reading through this port, the register data specified by using P#4 can be obtained.

REGISTER SELECT PORT (Write only)

      b7  b6  b5  b4  b3  b2  b1  b0
     +---+---+---+---+---+---+---+---+
P#4  |WII|RII|RA5|RA4|RA3|RA2|RA1|RA0|
     +---+---+---+---+---+---+---+---+

Using RA5 to 0, the register No. to which access is made through P#3 is specified. The register No. undergoes a plus 1 increment through P#3 access. However, when WII is "1", increment in writing at P#3 is prohibited and when RII is "1", increment in reading at P#3 is prohibited.

STATUS PORT (Read only)

      b7  b6  b5  b4  b3  b2  b1  b0
     +---+---+---+---+---+---+---+---+
P#5  |TR |VR |HR |BD | 0 |MCS|E0 |CE |
     +---+---+---+---+---+---+---+---+

Various types of status data can be read out as follows. TR: Command data transfer ready. It is reset through P#2 access. VR: Vertical non-display period. HR: Horizontal non-display period. BD: Border color detect at completion of SRCH command (becomes "1" when detected) EO: In the second field period during interlace. CE: Command being executed. MCS: Content of P#7

MCS INTERRUPT FLAG PORT (Read / Write)

      b7  b6  b5  b4  b3  b2  b1  b0
     +---+---+---+---+---+---+---+---+
P#6  | 0 | 0 | 0 | 0 | 0 |CE |HI |VI |
     +---+---+---+---+---+---+---+---+

The interrupt flags can be read out as follows. CE: Command completion flag HI: Display position flag VI: Vertical display period completion flag By writing "1", the related bit flag is reset.

SYSTEM CONTROL PORT (Write only)

      b7  b6  b5  b4  b3  b2  b1  b0
     +---+---+---+---+---+---+---+---+
P#7  | 0 | 0 | 0 | 0 | 0 | 0 |SRS|MCS|
     +---+---+---+---+---+---+---+---+

SRS:Writing "1" will set all ports except this one in "power ON reset" state. "0" should be written to cancel it. MCS:The internally used master clock is selected. 1:MCKIN terminal (14.318MHz Overscan on GFX9000) 0:XTAL1 terminal (21.4772MHz Normal on GFX9000) NOTE) XTAL1 terminal is selected when "power ON reset". If specification is not made properly when either XTAL1 or MCKIN terminal is used, specifi- cations for external input clock timing will not be fulfilled and a chip will be damaged in the worst case. As a measure to prevent this, it is necessary to supply clock signals to both terminals. The above does not apply when both terminals are used.

Primary standard kanji ROM ADDRESS PORT (Write only)

      b7  b6  b5    b4    b3    b2    b1    b0
     +---+---+-----+-----+-----+-----+-----+-----+
P#8  | - | - |K1A10| K1A9| K1A8| K1A7| K1A6| K1A5|
     +---+---+-----+-----+-----+-----+-----+-----+
P#9  | - | - |K1A16|K1A15|K1A14|K1A13|K1A12|K1A11|
     +---+---+-----+-----+-----+-----+-----+-----+

The font (16x16 bits) address for the primary standard kanji ROM which is read through P#9 is specified. It is necessary to write in the order of P#9 and then P#8 to prepare data for the kanji ROM address at completion of writing to P#8.

Primary standard kanji ROM DATA PORT (Read only)

      b7   b6   b5   b4   b3   b2   b1   b0
     +----+----+----+----+----+----+----+----+
P#9  |K1D7|K1D6|K1D5|K1D4|K1D3|K1D2|K1D1|K1D0|
     +----+----+----+----+----+----+----+----+

It is possible to read the data of the font address specified by using P#8 and P#9 of the primary standard kanji ROM. A WAIT signal is output till the data has been prepared. By reading 32 times continuously, it is possible to obtain 32 bytes data in the order of the upper left, upper right, lower left and lower right blocks (of 8 bytes each) of the 16x16 bits font.

Secondary standard kanji ROM ADDRESS PORT (Write only)

      b7  b6  b5    b4    b3    b2    b1    b0
     +---+---+-----+-----+-----+-----+-----+-----+
P#A  | - | - |K2A10| K2A9| K2A8| K2A7| K2A6| K2A5|
     +---+---+-----+-----+-----+-----+-----+-----+
P#B  | - | - |K2A16|K2A15|K2A14|K2A13|K2A12|K2A11|
     +---+---+-----+-----+-----+-----+-----+-----+

The font (16x16 bits) address for the secondary standard kanji ROM which is read through P#9 is specified. It is necessary to write in the order of P#B and then P#A to prepare data for the kanji ROM address at completion of writing to P#A.

Secondary standard kanji ROM DATA PORT (Read only)

     b7   b6   b5   b4   b3   b2   b1   b0
    +----+----+----+----+----+----+----+----+
P#C |K2D7|K2D6|K2D5|K2D4|K2D3|K2D2|K2D1|K2D0|
    +----+----+----+----+----+----+----+----+

It is possible to read the data of the font address specified by using P#A and P#B of the secondary standard kanji ROM. A WAIT signal is output till the data has been prepared. By reading 32 times continuously, it is possible to obtain 32 bytes data in the order of the upper left, upper right, lower left and lower right blocks (of 8 bytes each) of the 16x16 bits font. Note) GFX9000 does not contain a kanji ROM. GFX9000 I/O port #6F is used for Superimpose control.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("v9990-registers", "", "Hardware", "", "Registradores - V9990 registers: This TED file contains a description of all V9990...",
        R"MAN(HARDWARE: V9990 (VideoChip do futuro)
Registradores - V9990 registers

This TED file contains a description of all V9990 registers, both the ordinary  known and  some 'secret' ones which I found during my work with the GFX9000 PowerBasic, Paint9000, Xbasic and more.

(c) Henrik Gilvad 1996. May be used as long as it's for free!

V9990   Register specifications
------------------------------------------------------------------------
R#0-2   VRAM WRITE ADDRESS                    (W)
R#3-5   VRAM READ ADDRESS                     (W)
R#6,7   SCREEN MODE                           (R/W)
R#8     CONTROL                               (R/W)
R#9-12  INTERRUPT                             (R/W)
R#13    PALETTE CONTROL                       (W)
R#14    PALETTE POINTER                       (W)
R#15    BACK DROP COLOR                       (R/W)
R#16    DISPLAY ADJUST                        (R/W)
R#17-24 SCROLL CONTROL                        (R/W)
R#25    SPRITE PATTERN GENERATOR BASE ADDRESS (R/W)
R#26    LCD CONTROL                           (R/W)
R#27    PRIORITY CONTROL                      (R/W)
R#28    SPRITE PALETTE CONTROL                (W)
R#32-52 COMMAND                               (W)
R#53-54 COMMAND                               (R)
------------------------------------------------------------------------

VRAM WRITE ADDRESS (Write only)
===============================
      b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#0  |CVWA7 |CVWA6 |CVWA5 |CVWA4 |CVWA3 |CVWA2 |CVWA1 |CVWA0 |
     +------+------+------+------+------+------+------+------+
R#1  |CVWA15|CVWA14|CVWA13|CVWA12|CVWA11|CVWA10|CVWA9 |CVWA8 |
     +------+------+------+------+------+------+------+------+
R#2  |CVWAIH|  0   |  0   |  0   |  0   |CVWA18|CVWA17|CVWA16|
     +------+------+------+------+------+------+------+------+
CVWA0-18 : VRAM write address
CVWAIH =0: CVWA undergoes an increment of plus 1 only at the end of writing
           to P#0.
       =1: No increment.

VRAM READ ADDRESS (Write only)
==============================
      b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#3  |CVRA7 |CVRA6 |CVRA5 |CVRA4 |CVRA3 |CVRA2 |CVRA1 |CVRA0 |
     +------+------+------+------+------+------+------+------+
R#4  |CVRA15|CVRA14|CVRA13|CVRA12|CVRA11|CVRA10|CVRA9 |CVRA8 |
     +------+------+------+------+------+------+------+------+
R#5  |CVRAIH|  0   |  0   |  0   |  0   |CVRA18|CVRA17|CVRA16|
     +------+------+------+------+------+------+------+------+
CVRA0-18 : VRAM read address
CVRAIH =0: CVRA undergoes an increment of plus 1 only at the end of reading
           at P#0.
       =1: No increment.

SCREEN MODE (Read / Write)
==========================
     b7    b6    b5    b4    b3    b2    b1    b0
    +-----+-----+-----+-----+-----+-----+-----+-----+
R#6 |DSPM1|DSPM0|DCKM1|DCKM0|XIMM1|XIMM0|CLRM1|CLRM0|
    +-----+-----+-----+-----+-----+-----+-----+-----+
R#7 |  0  |C25M | SM1 | SM  | PAL | EO  | IL  | HSCN|
    +-----+-----+-----+-----+-----+-----+-----+-----+
DSPM1-DSPM0:    Display mode selection
  1     1       Stand-by mode (non-display, no VRAM refresh, kanji readable)
  1     0       Bit map mode (B1~B6)
  0     1       P2 mode
  0     0       P1 mode

DCKM1-DCKM0     Dot clock selection (usind in combination with MCS of P#7)
                MCS=0   DOTS    MCS=1   DOTS
  1     0       XTAL1   1024    MCKIN    768
  0     1       XTAL1/2  512    MCKIN/2  384
  0     0       XTAL1/4  256    MCKIN/4  192

MCS=1 means that we are in the socalled 'Overscan' mode. This means that most of the 'border' area are also used for drawing pixel data. In the Y direction NTSC will  have 240  dots in  Overscan mode (480 in Interlace), PAL will have around 290 dots (580 in Interlace).

This gives us 'new' screenmodes: (not mentioned by Yamaha)
1024 x 212 (424) dots
 768 x 290 (580) dots (PAL)

XIMM1-XIMM0     Selection of number of dots in X direction of image space
  1     1       2048 dots
  1     0       1024 dots
  0     1        512 dots
  0     0        256 dots

CLRM1-CLRM0     Selection of bit number per dot
  1     1       16 bits/dot
  1     0        8 bits/dot
  0     1        4 bits/dot
  0     0        2 bits/dot

C25M            Selection of 640x480 mode, valid when HSCN is "1"
  1             B6 mode (Requires VGA oscillator instead of 21.4772)
  0             P1, P2, B1~B5 mode

SM1             Selection of total number of vertical lines during non-
                interlace, NTSC
  1             263 lines (in combination with SM, the sub-carrier phase is
                inverted for each frame.)
  0             262 lines

SM              Selection of horizontal frequency (invalid when in B5 and B6)
  1             1H = fsc/227.5 (the sub-carrier phase is inverted for each
                line.)
  0             1H = fsc/228

PAL             Selection of PAL mode (invalid when in B5 and B6 modes)
  1             Vertical frequency becomes PAL specification.
  0             Vertical frequency becomes NTSC specification.

EO              Selection of vertical resolution during interlace.
                (invalid when in B5 and B6 modes)
  1             Twice the vertical resolution during Non-interlace
  0             Same as during Non-interlace

IL              Selection of interlace mode (invalid when in B5 and B6 modes)
  1             Interlace
  0             Non-interlace

HSCN            Selection of horizontal high scan mode (VGA)
  1             B5, B6 modes
  0             P1, P2, B1~B4 modes

CONTROL (Read/Write)
====================
      b7  b6  b5  b4  b3  b2  b1  b0
    +----+---+---+----+---+----+----+----+
R#8 |DISP|SPD|YSE|VWTE|VWM|DMAE|VSL1|VSL0|
    +----+---+---+----+---+----+----+----+
DISP    Screen display enable
  1     Display appears on screen according to the VRAM content.
  0     Back drop color is displayed all over the screen.

SPD     Sprite (Cursor in B5~B6 modes) non-display
  1     Sprite (cursor) is not displayed
  0     Sprite (cursor) is displayed

YSE     /YS signal output enable (Super impose layer select)
  1     /YS signal is output.
  0     /YS signal is not output (/YS terminal constantly low)

VWTE    VRAM serial data bus input/output control during digitization.
  1     Dummy write transfer is executed during horizontal retrace line
        interval.
        (The serial data bus of VRAM becomes an input terminal.)
  0     Read transfer is executed during horizontal retrace line interval.
        (The serial data bus of VRAM becomes an output terminal)

VWM     VRAM write control during digitization
   1    Write transfer is executed during horizontal retrace line interval.
        (The data inputted into the serial data bus of VRAM during display
        period undergoes write transfer.)
   0    Write transfer is not executed.

DMAE    /DREQ signal output enable
   1    The signal is synchronized to the status TR while command is executed
        and output at /DREQ terminal.
   0    /DREQ terminal remains as high level.

VSL1 VSL0       Selection of video memory configuration
  1    0        256k word x 4 bit VRAM, 4 units (512k byte)
  0    1        128k word x 8 bit VRAM, 2 units (256k byte)
  0    0         64k word x 4 bit VRAM, 4 units (128k byte)

Note)    Always use the 256k word x 4 bit VRAM on GFX9000 !

INTERRUPT (Read/Write)
======================
       b7  b6  b5  b4  b3  b2   b1  b0
     +----+---+---+---+---+----+---+---+
R#9  | 0  | 0 | 0 | 0 | 0 |IECE|IEH|IEV|
     +----+---+---+---+---+----+---+---+
R#10 |IL7 |IL6|IL5|IL4|IL3| IL2|IL1|IL0|
     +----+---+---+---+---+----+---+---+
R#11 |IEHM| 0 | 0 | 0 | 0 |  0 |IL9|IL8|
     +----+---+---+---+---+----+---+---+
R#12 | 0  | 0 | 0 | 0 |IX3| IX2|IX1|IX0|
     +----+---+---+---+---+----+---+---+
IECE    Command end interrupt enable control
  1     /INT0 terminal becomes low level when CE flag of P#6 is "1"
  0     /INT0 terminal does not change according to CE flag

IEV     Interrupt enable during vertical retrace line interval.
  1     /INT0 terminal becomes low level when VI flag of P#6 is "1"
  0     /INT0 terminal does not change according to VI flag

IEH     Display position interrupt enable (Interrupt position is specified
        with IL0-9, IX0-3 and IEHM.)
  1     /INT1 terminal becomes low level when HI flag of P#6 is "1"
  0     /INT1 terminal does not change according to HI flag.

IL0-9   Specification of vertical position where display position interrupt
        occurs (Specified by means of LINE no. with the display start as 0)

IX0-3   Specification of horizontal position where display position interrupt
        occurs (Specified  by unit  of 64 master clock with the display start
        as "0")

IEHM    Selection of vertical position for display position interrupt
   1    Every line (IL0-9 are ignored.)
   0    Specified line

PALETTE CONTROL (Write only)
============================
       b7    b6   b5   b4    b3    b2    b1    b0
    +-----+-----+---+------+-----+-----+-----+-----+
R#13|PLTM1|PLTM0|YAE|PLTAIH|PLT05|PLT04|PLT03|PLT02|
    +-----+-----+---+------+-----+-----+-----+-----+
PLTM1 PLTM0     Selection of color palette mode
  1     1       YUV mode (8 bit / dot)
  1     0       YJK mode (8 bit / dot)
  0     1       256 color mode (like Screen 8 on MSX)
  0     0       Palette mode (2, 4, 16 or 64 colors)

YAE     Selection of YJK (YUV) and RGB mixing mode (valid when in YUV/YJK)
  1     YJK (YUV) and RGB images are displayed together (Like screen 10/11)
  0     Only YJK (YUV) image is displayed.

PLTAIH  Palette Number increment control at color palette read-out.
  1     R#14 palette pointer is not changed by P#1 read-out.
  0     After P#1 read-out. R#14 palette pointer undergoes an increment.

PLT05-2 Still Screen palette number offset.

PALETTE POINTER (Write only)
============================
      b7    b6    b5    b4    b3    b2    b1    b0
    +-----+-----+-----+-----+-----+-----+-----+-----+
R#14|PLTA5|PLTA4|PLTA3|PLTA2|PLTA1|PLTA0|PLTP1|PLTP0|
    +-----+-----+-----+-----+-----+-----+-----+-----+
PLTA5-0         Palette number (color code)

PLTP1 PLTP0     Specification of RGB
  1     0       Blue
  0     1       Green
  0     0       Red

After completion of access to P#1, an increment takes place by plus 1 only.
PLT1 and 0 are used as a 0 to 2 ternary counter.

BACK DROP COLOR (Read/Write)
      b7  b6  b5  b4  b3  b2  b1  b0
    +---+---+----+----+----+----+----+----+
R#15| 0 | 0 |BDC5|BDC4|BDC3|BDC2|BDC1|BDC0|
    +---+---+----+----+----+----+----+----+
BDC5-0  Palette number of back drop color

DISPLAY ADJUST (Read/Write)
===========================
      b7  b6  b5  b4  b3  b2  b1  b0
    +-----+-----+-----+-----+-----+-----+-----+-----+
R#16|ADJV3|ADJV2|ADJV1|ADJV0|ADJH3|ADJH2|ADJV1|ADJV0|
    +-----+-----+-----+-----+-----+-----+-----+-----+

ADJV3-0 Vertical display position adjustment 8 9 ...15 0 1 ... 6 7
        (by line)                           Down    Standard     Up

ADJH3-0 Horizontal display position adjustment 8 9 ... 15 0 1 ... 6 7
                                             Right    Standard    Left
        P1 and B1 by 1 dot unit
        P2, B2 and B3 by 2 dot unit
        B4, B5 and B6 by 4 dot unit

SCROLL CONTROL (Read/Write)
===========================
       b7     b6    b5    b4     b3     b2     b1    b0
     +------+-----+-----+------+------+------+-----+-----+
R#17 | SCAY7|SCAY6|SCAY5| SCAY4| SCAY3| SCAY2|SCAY1|SCAY0|
     +------+-----+-----+------+------+------+-----+-----+
R#18 | R512 |R256 |  0  |SCAY12|SCAY11|SCAY10|SCAY9|SCAY8|
     +------+-----+-----+------+------+------+-----+-----+
R#19 |  0   |  0  |  0  |  0   |  0   |SCAX2 |SCAX1|SCAX0|
     +------+-----+-----+------+------+------+-----+-----+
R#20 |SCAX10|SCAX9|SCAX8|SCAX7 |SCAX6 |SCAX5 |SCAX4|SCAX3|
     +------+-----+-----+------+------+------+-----+-----+
R#21 | SCBY7|SCBY6|SCBY5| SCBY4| SCBY3| SCBY2|SCBY1|SCBY0|
     +------+-----+-----+------+------+------+-----+-----+
R#22 |DisB  |DisA |  0  |  0   |  0   |  0   |  0  |SCBY8|
     +------+-----+-----+------+------+------+-----+-----+
R#23 |  0   |  0  |  0  |  0   |  0   |SCBX2 |SCBX1|SCBX0|
     +------+-----+-----+------+------+------+-----+-----+
R#24 |  0   |  0  |SCBX8|SCBX7 |SCBX6 |SCBX5 |SCBX4|SCBX3|
     +------+-----+-----+------+------+------+-----+-----+
SCAY12-0        As display start coordinates for screen "A" of P1 mode and
SCAX10-0        screens of other modes, X-Y coordinates in the image space
                are specified by dots. (When 16 bit/dot is used in B2 and B3
                modes, however, SCAX0 is ignored and X coordinate is specified
                by 2 dot unit.)

SCBY8-0         For screen "B" of P1 mode, X-Y coordinates are specified in
SCBX8-0         the same way as for screen "A".

R512 R256       Number of roll page line in Y direction is specified.
  1    0        Displayed page is rolled by 512 lines.
  0    1        Displayed page is rolled by 256 lines.
  0    0        Rolling by image space size takes place.

DisB            Disable display of screen "B"
  1             Screen "B" is disabled
  0             Screen "B" is enabled

DisA            Disable display of screen "A"
  1             Screen "A" is disabled
  0             Screen "A" is enabled

SPRITE PATTERN GENERATOR TABLE BASE ADDRESS (Read/Write)
========================================================
       b7  b6  b5  b4   b3     b2     b1     b0
     +---+---+---+---+------+------+------+------+
R#25 | 0 | 0 | 0 | 0 |SGBA17|SGBA16|SGBA15|  0   |  P1 mode!
     +---+---+---+---+------+------+------+------+
R#25 | 0 | 0 | 0 | 0 |SGBA18|SGBA17|SGBA16|SGBA15|  P2 mode!
     +---+---+---+---+------+------+------+------+
SGBA18-15       Base address of P1 and P2 mode sprite pattern generator.

LCD CONTROL (Read/Write)
========================
       b7  b6  b5  b4  b3   b2   b1   b0
     +---+---+---+---+----+----+-----+----+
R#26 | 0 | 0 | 0 |VRI|PNSL|PLV0|PDUAL|PNEN|
     +---+---+---+---+----+----+-----+----+
VRI     Selection of number of lines during vertical retrace line interval
        when in panel mode
  1     1 line (Frame frequency increases)
  0     Same as CRT mode

PNSL    Selection of number of vertical direction dot in panel
  1     480 dots
  0     400 dots

PLV0    Selection of Gray Scaling
  1     Display data is output at color codes to terminals of CB7 to 0.
  0     Binary converted data is output to terminals of D3 to 0.

PDUAL   Selection of panal type
  1     2 screen panel (single drive type)
  0     1 screen panel

PNEN    Selection of panel display mode
  1     Display cycle and terminal function used for panel
  0     Display cycle and terminal function used for CRT

PRIORITY CONTROL (Read/Write)
=============================
      b7  b6  b5  b4  b3   b2   b1   b0
    +---+---+---+---+----+----+----+----+
R#27| 0 | 0 | 0 | 0 |PRY1|PRY0|PRX1|PRX0|
    +---+---+---+---+----+----+----+----+
PRY1,0  When in P1 mode, the display Y coordinate to exchange display priority
        order between screens "A" and "B" is specified.
        A unit of 64 lines is used and in the range lower than the specified
        line (including that line), screen "B" has a priority. At "00", screen
        "A" has a priority throughout the range.
PRX1,0  When in P1 mode, the display X coordinate to exchange display priority
        order between screens "A" and "B" is specified.
        A unit of 64 dots is used and in that range to the right of the
        specified dot (including that dot), screen "B" has a priority.
        At "00", screen "A" has a priority throughout the range.

SPRITE PALETTE CONTROL (Write only)
===================================
      b7  b6  b5  b4  b3    b2    b1    b0
    +---+---+---+---+-----+-----+-----+-----+
R#28| 0 | 0 | 0 | 0 |CSPO5|CSPO4|CSPO3|CSPO2|
    +---+---+---+---+-----+-----+-----+-----+
CSPO5-2 Palette number offset of sprite (cursor in B5/B6)

COMMAND REGISTERS
=================

SOURCE XY-Coordinate        (SX,SY)         (All Write only!)
       Linear address       (SA)
       Kanji-rom address    (KA)

        b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#32 | SX7  | SX6  | SX5  | SX4  | SX3  | SX2  | SX1  | SX0  |
     +------+------+------+------+------+------+------+------+
     | SA7  | SA6  | SA5  | SA4  | SA3  | SA2  | SA1  | SA0  |
     +------+------+------+------+------+------+------+------+
     | KA7  | KA6  | KA5  | KA4  | KA3  | KA2  | KA1  | KA0  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#33 | 0    |  0   |  0   |  0   |  0   | SX10 | SX9  | SX8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#34 | SY7  | SY6  | SY5  | SY4  | SY3  | SY2  | SY1  | SY0  |
     +------+------+------+------+------+------+------+------+
     | SA15 | SA14 | SA13 | SA12 | SA11 | SA10 | SA9  | SA8  |
     +------+------+------+------+------+------+------+------+
     | KA15 | KA14 | KA13 | KA12 | KA11 | KA10 | KA9  | KA8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#35 | 0    |  0   |  0   |  0   | SY11 | SY10 | SY9  | SY8  |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   |  0   | SA18 | SA17 | SA16 |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   |  0   |  0   | KA17 | KA16 |
     +------+------+------+------+------+------+------+------+

DESTINATION XY-Coordinate       (DX,DY)  (All Write only!)
            Linear address      (DA)

        b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#36 | DX7  | DX6  | DX5  | DX4  | DX3  | DX2  | DX1  | DX0  |
     +------+------+------+------+------+------+------+------+
     | DA7  | DA6  | DA5  | DA4  | DA3  | DA2  | DA1  | DA0  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#37 | 0    |  0   |  0   |  0   |  0   | DX10 | DX9  | DX8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#38 | DY7  | DY6  | DY5  | DY4  | DY3  | DY2  | DY1  | DY0  |
     +------+------+------+------+------+------+------+------+
     | DA15 | DA14 | DA13 | DA12 | DA11 | DA10 | DA9  | DA8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#39 | 0    |  0   |  0   |  0   | DY11 | DY10 | DY9  | DY8  |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   |  0   | DA18 | DA17 | DA16 |
     +------+------+------+------+------+------+------+------+

TRANSFER DOT NUMBER XY          (NX,NY) (Write only)
                    LINEAR      (NA)
                    LINE MINER  (MI)
                    LINE MAJOR  (MJ)

        b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#40 | NX7  | NX6  | NX5  | NX4  | NX3  | NX2  | NX1  | NX0  |
     +------+------+------+------+------+------+------+------+
     | NA7  | NA6  | NA5  | NA4  | NA3  | NA2  | NA1  | NA0  |
     +------+------+------+------+------+------+------+------+
     | MJ7  | MJ6  | MJ5  | MJ4  | MJ3  | MJ2  | MJ1  | MJ0  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#41 | 0    |  0   |  0   |  0   |  0   | NX10 | NX9  | NX8  |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   | MJ11 | MJ10 | MJ9  | MJ8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#42 | NY7  | NY6  | NY5  | NY4  | NY3  | NY2  | NY1  | NY0  |
     +------+------+------+------+------+------+------+------+
     | NA15 | NA14 | NA13 | NA12 | NA11 | NA10 | NA9  | NA8  |
     +------+------+------+------+------+------+------+------+
     | MI7  | MI6  | MI5  | MI4  | MI3  | MI2  | MI1  | MI0  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#43 | 0    |  0   |  0   |  0   | NY11 | NY10 | NY9  | NY8  |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   |  0   | NA18 | NA17 | NA16 |
     +------+------+------+------+------+------+------+------+
     | 0    |  0   |  0   |  0   | MI11 | MI10 | MI9  | MI8  |
     +------+------+------+------+------+------+------+------+

ARGUMENT (Write only)

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#44 | 0    |  0   |  0   |  0   | DIY  | DIX  | NEQ  | MAJ  |
     +------+------+------+------+------+------+------+------+

LOGICAL OPERATION (Write only)

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#45 | 0    |  0   |  0   | TP   | L11  | L10  | L01  | L00  |
     +------+------+------+------+------+------+------+------+

WRITE MASK (Write only)

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#46 | WM7  | WM6  | WM5  | WM4  | WM3  | WM2  | WM1  | WM0  |
     +------+------+------+------+------+------+------+------+
R#47 | WM15 | WM14 | WM13 | WM12 | WM11 | WM10 | WM9  | WM8  |
     +------+------+------+------+------+------+------+------+

FONT COLOR (Write only)

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#48 | FC7  | FC6  | FC5  | FC4  | FC3  | FC2  | FC1  | FC0  |
     +------+------+------+------+------+------+------+------+
R#49 | FC15 | FC14 | FC13 | FC12 | FC11 | FC10 | FC9  | FC8  |
     +------+------+------+------+------+------+------+------+
     +------+------+------+------+------+------+------+------+
R#50 | BC7  | BC6  | BC5  | BC4  | BC3  | BC2  | BC1  | BC0  |
     +------+------+------+------+------+------+------+------+
R#51 | BC15 | BC14 | BC13 | BC12 | BC11 | BC10 | BC9  | BC8  |
     +------+------+------+------+------+------+------+------+

OPERATION CODE (Write only)

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#52 | OP3  | OP2  | OP1  | OP0  | AYM  | AYE  | AXM  | AXE  |
     +------+------+------+------+------+------+------+------+

BORDER X-COORDINATE (Read only)

       b7     b6     b5     b4     b3     b2     b1     b0
     +------+------+------+------+------+------+------+------+
R#53 | BX7  | BX6  | BX5  | BX4  | BX3  | BX2  | BX1  | BX0  |
     +------+------+------+------+------+------+------+------+
R#54 | 0    |  0   |  0   |  0   |  0   | BX10 | BX9  | BX8  |
     +------+------+------+------+------+------+------+------+

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("keyboard-matrix", "hw-keyboard", "Hardware", "", "MSX-BIOS: Keyboard-Matrix: Key-Matrix table:",
        R"MAN(HARDWARE: Keyboard-Matrix
MSX-BIOS: Keyboard-Matrix

Key-Matrix table:

 bit     7     6     5     4     3     2     1     0    row
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBE5 |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |  0
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBE6 |  ;  |  ]  |  [  |  \  |  =  |  -  |  9  |  8  |  1
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBE7 |  B  |  A  | Acc |  /  |  .  |  ,  |  `  |  '  |  2
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBE8 |  J  |  I  |  H  |  G  |  F  |  E  |  D  |  C  |  3
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBE9 |  R  |  Q  |  P  |  O  |  N  |  M  |  L  |  K  |  4
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBEA |  Z  |  Y  |  X  |  W  |  V  |  U  |  T  |  S  |  5
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBEB |  F3 |  F2 |  F1 | code| caps|graph| ctrl|shift|  6
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBEC | ret |selec|  bs | stop| tab | esc |  F5 |  F4 |  7
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBED |right| down|  up | left| del | ins | hom |space|  8
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBEE |  4  |  3  |  2  |  1  |  0  |  /  |  +  |  *  |  9
      +-----+-----+-----+-----+-----+-----+-----+-----+
#FBEF |  .  |  ,  |  -  |  9  |  8  |  7  |  6  |  5  | 10
      +-----+-----+-----+-----+-----+-----+-----+-----+

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("i-o-port-map", "hw-ports,hw-io", "Hardware", "", "I/O-Port map: Overview about all used MSX I/O ports (hex):",
        R"MAN(HARDWARE: I/O-Port map
I/O-Port map

Overview about all used MSX I/O ports (hex):

Informational: if there's a Telcom modem connected to the computer there could be different values on ports 00-7F.

00-3F | for expansion by users. Commercial applications are not allowed to use these.
00-03 | Fac MIDI interface
00-04 | Music Module MIDI
04-05 | Music Module MIDI input ports?
00-02 | MIDI saurus
Exact ports of the above components are not known. But it's sure that I/O ports within this area are used!
00-07 | MT Telcom modem
08-0F | Classroom Network (YM3802)
18-19 | Philips Barcode Reader NMS1170/20
20-28 | 2nd installed Philips NMS 1251 modem
Miniware M4000 modem
GREEN/MAK SCSI-interface
CD-ROM interface
28-2F | 2nd installed Philips NMS1210 RS232 interface
30-38 | Philips NMS1251 modem
Miniware M4000 modem
GREEN/MAK SCSI-interface
CD-ROM interface
34-36 | HSH SCSI-Interface, Novaxis SCSI-Interface (WD33C93A)
34 | O select WD33C93A register
34 | I read WD33C93A status-register
35 | O write selected WD33C93A register
35 | I read selected WD33C93A register
36 | O reset WD33C93A
40-4F | Switched I/O portsfor expansion by corporate bodies. One of them is for 12 pixels kanji font ROM standardized by Matsushita.
40 | Maker/Device ID port
41 | Adressregisters 00-0F
42 | Data registers
46 | Foreground- and background colors
47 | Bitpatrone for colors
For which component are these ports used?
60-6F | Graphics9000 / v9990  V9990-board as sold by "Handlergemeinschaft"(I/O ports can be configured)
60 | I/O VRAM data port
61 | I/O Palette data port
62 | I/O Command data port
63 | I/O Register data port
64 | O   Register select port
65 | I   Status port
66 | I/O Interrupt flag port
67 | O   System control port
68-69 | O   Primary standard Kanji ROM address port (not used in Gfx9000)
69 | I   Primary standard Kanji ROM data port (not used in Gfx9000)
6A-6B | O   Secondary standard Kanji ROM address port (not used in Gfx9000)
6B | I   Secondary standard Kanji ROM data port (not used in Gfx9000)
6C-6F | ?   Reserved
70-73 | MIDI Saurus (standardized by Bit2)
7C/7D | MSX-Music:
7C | O   register write-port
7D | O   data write-port
7E/7F | MoonSound:
7E | O   MoonSound: register
7F | I/O MoonSound: data
7F | Chakkari Copy control register
80-87 | RS232C (Spectravideo):
80-83 | USART 8251:
80 | I/O data-register
81 | I/O status-/command-register
82 | I   connection-status
83 | O   interrupt-mask
84-87 | programmable timer 8253:
84 | I/O teller 1
85 | I/O teller 2
86 | I/O teller 3
87 | I/O settings
88-8B | for V9938 VDP of MSX2-versionup adapter [MSX-1 adapter for VDP]
(not supported in Turbo-R)
8C-8D | Sony modem
90-91 | printer-port:
90 | I/O I bit 1 = printer busy
O bit 0 = strobe (negative logic)
91 | O   data
98-9B | Video Display Processor (MSX1 and above: TMS9918, V9938, V9958):
98 | I/O Video RAM read/write
99 | I/O I statusregister read-port
O VDP-register/data write-port
O Video RAM adress write-port
9A | O   palette write-port (V9938, V9958 only)
9B | O   indirect access write-port (V9938, V9958 only)
A0-A3 | Programmable Sound Generator (AY-3-8910):
A0 | O   register write-port
A1 | O   data write-port
A2 | I   data read-port
A3 | ?
A4/A5 | Turbo R sampler (PCM):
A4 | O   bit 0-4 audio-level?? (D/A converter for PCM)
I   bit 0/1 teller 16 kHz, used by sampler (D/A converter for PCM)
A5 | O   bit 1 = sound off
bit 3 = all sound-input is directly forwarded to sound-output
A6 | ?
A7 | I   bit 0 = pause switch (1=on, 0=off)    Works only in R800 mode (connected to WAIT of Z80)
O   bit 0 = pause LED (0=off, 1=on)
O   bit 7 = turbo LED (0=off, 1=on)
A8-AB | MSX-Engine (Programmable Peripheral Interface, 8255):
A8 | I/O PPI port A: slotselect-register
A9 | I   PPI port B: keyboard column input
AA | I/O PPI port C: keyclick, CapsLED, tape, keyboard
AB | O   PPI mode port:
AC-AF | MSX engine (a custom chip dependent on machines, S3527 / S1985)
B0-B3 | SRAM cartridge which came bundled with Sony HB-55 (Sony data cartridge 8255):
B0 | adress-line A0-A7
B1 | adress-line A8-A10, A13-A15, CONTROL, R/W
B2 | adress-line A11-A12, data-line D0-D7
B3 | ?
B4-B5 | clockchip (RP-5C01). Both external and embedded ones use same ports:
B4 | adressregister
B5 | data read/write
B6/B7 | card reader (= ???)
B8-BB | for lightpen of Sanyo standard. Both external and embedded ones use same ports
BC-BF | for VHD controller of Nihon Victor standard (Videocontroller JVC)
C0/C1 | MSX-Audio -or- MoonSound / OPL4 - FM-bank 1 alternate addressing mode
C0 | I/O I statusregister read-port
O register write-port
C1 | O   data write-port
C2/C3 | Same functionality for optional 2nd MSX-Audio -or- MoonSound / OPL4 - FM-bank 2 alternate addressing mode
C4/C5 | MoonSound / OPL4 - FM-bank 1
C6/C7 | MoonSound / OPL4 - FM-bank 2
C8-CF | VDP of MSX2 module for MSX1 (MSX Interface)
D0-D7 | for Floppy disk controller, but are never actually used
D8-D9 | 1st standard KANJI-ROM:
D8 | O   adress-lines A5-A0 (least significant)
D9 | I/O I data-lines D7-D0
O adress-lines A5-A0 (most significant)
DA-DB | 2nd standard KANJI-ROM:
DA | O   adress-lines A5-A0 (least significant)
DB | I/O I data-lines D7-D0
O  adress-lines A5-A0 (most significant)
DC | reserved for other kanji purposes (for future expansion)
E4-E5 | Used at times of changing a CPU mode (S1990):
E4 | O   Register select port
E5 | I/O Register access port
E6-E7 | I   16 bits system-timer, frequence 255682 Hz, E6=LSB
E6 | O   16-bit counter reset (write to reset)
E8-EF | for MSX-MIDI (since Turbo-R):
E9 | I/O bit 0 = MIDI OUT enable
bit 1 = MIDI timer enable
bit 2 = MIDI IN enable
bit 5 = enable interrupt als MIDI-byte ontvangen
bit 7 = 8253 timer interrupt (MIDI) - (if 1, 0 if not)
EC | MIDI??? (0x20 is written during Turbo-R MIDI initialisation)
EE-EF | Has to do with GT-MIDI. In ieder geval kan je de MIDI-teller er mee instellen.
F3 | Screen mode concerning VDP.
bit 7: YAE
bit 6: YUV
bit 5: TP
bit 4: M1
bit 3: M2
bit 2: M5
bit 1: M4
bit 0: M3
F4 | Reset status to control the boot (when turning MSX power on). Refer to Software-Reset for more details.
bit 7: Boot method (1=soft boot, doesn't show start logo, 0=hard boot)
bit 5: CPU boot mode (1=R800, 0=Z80)
F5 | Device enable (write only):
bit 0: kanji ROM
bit 1: 2nd standard kanji ROM (MSX 2+ and later)
bit 2: MSX-Audio
bit 3: AV-control (for super-impose)
bit 4: MSX-interface
bit 5: RS232C-interface (without relation with modems themselves)
bit 6: lightpen
bit 7: clock IC
F6 | color bus I/O. Used by optional audio/visual functions. Since MSX 2
F7 | Audio/Visual (AV) control. Used by optional audio/visual functions. Since MSX 2
bit 0: audio right
bit 1: audio left
bit 2: video input select
bit 3: video input sense (read)
bit 4: A/V-control
bit 5: Ym control
bit 6: Ys control
bit 7: Video Select
F8 | Optional A/V controller for PAL version
FC-FF | Memory mappers of page 0-3
FC | for switching memory mapper page 0 (write only)
FD | for switching memory mapper page 1 (write only)
FE | for switching memory mapper page 2 (write only)
FF | for switching memory mapper page 3 (write only)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("switched-i-o-ports", "", "Hardware", "", "Portas I/O comutadas - Switched I/O-ports: According to the MSX2 Hardware Specification,...",
        R"MAN(HARDWARE: I/O-Port map
Portas I/O comutadas - Switched I/O-ports

According to the MSX2 Hardware Specification, ports #40-#4F are forming the so-called
'switched I/O ports'. With these the limitation of a maximum number of 256 I/O ports
can be overcome, although it does require a little additional logic on the hardware's
behalf. The port numbers #41-#4F are the actual switched ports, and the device 'connected'
(e.g. listening to) to those ports is determined by the device ID written to port #40.
This device ID can be a number from 1-254, and when port #40 is read it returns the
complement of the current device ID.

Port range | Description
40 | Device ID register
41-4F | Switched I/O ports

ID numbers between 1 and 127 are manufacturer ID numbers, and ID numbers 128 to 254 are
device ID's. As a basic rule, those devices which are designed specifically for one
machine should contain the manufacturer's company ID, while periperhal devices which can
be used on all MSX computers should have a device ID number. Also, the Z80 actually has a
16-bit I/O addressing space so it is recommended to access in 16-bit by decoding the upper
8 bits for those ID's which might be expanded in the future (although I have personally only
seen this done in the ADVRAM hardware). Especially for devices which are connected with their
make ID it can be expected that they might eventually need more addressing space, and with this
it will be future-proofed.

List of BIOS extension device ID's:
ID | Maker/Device
1 | ASCII/Microsoft
2 | Canon
3 | Casio
4 | Fujitsu
5 | Fujitsu
6 | Hitachi
7 | Kyocera
8 | Matsushita (Panasonic)
9 | Mitsubishi
10 | NEC
11 | Nippon Gakki
12 | JVC
13 | Philips
14 | Pioneer
15 | Sanyo
16 | Sharp
17 | SONY
18 | Spectravideo
19 | Toshiba
20 | Spectravideo
128 | Image Scanner (Matsushita)
247 | Lex Lechz: Required for Kanji modes 1 and 3 (on MSX2, MSX2+ and TR)
254 | MPS2 (ASCII)

Some example code, this enables the 6MHz mode on Panasonic FS-A1WX/WSX/FX computers:

Port | Panasonic extensions
40 | = 8 (Panasonic manufacturer ID)
41 | Available on FS-A1FX/WX/WSX only:
      bit 0: CPU speed mode (0=5.37MHz, 1=3.57MHz)
Available on FS-A1FX/WX/WSX/ST/GT only:
      bit 7: Firmware switch status (0=on, 1=off) (read only)

    LD    A,8
    OUT     (040H),A    ;out the manufacturer code 8 (Panasonic) to I/O port 40h
    IN    A,(040H)    ;read the value you have just written
    CPL            ;complement all bits of the value
    CP    8        ;if it does not match the value you originally wrote,
    JR    NZ,Not_WX    ;it is not a WX/WSX/FX.
    XOR    A        ;write 0 to I/O port 41h
    OUT    (041H),A    ;and the mode changes to high-speed clock

So you can check if certain extensions are available and ready to be accessed by reading
back the value you just wrote to the device ID register. If that value is not the complement
of what you wrote, then the extensions are not present.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("system-timer", "", "Hardware", "", "Timer do sistema (Turbo-R) - System timer: Available since: MSX Turbo-R",
        R"MAN(HARDWARE: I/O-Port map
Timer do sistema (Turbo-R) - System timer

Available since: MSX Turbo-R

Because the timelength of the execution of commands is not uniform, it is not possible to make some sort of wait-timing with the execution time of commands. For making in possible to check timelength between 10 usec and 10 msec, a system timer has been build in. The systemtimer will be increased every 42 clockpulses of the system clock (10.738635 MHz), (10.738635MHz/42 = 255.68KHz) with a writeable 16bit cleardata timer.

The least significant bit will be increased every 3.911 usec. The time that elapse between the clearance of the counter until it becomes 1, is between 0 and 3.911 usec. When reading the counter, there is not latch function of the data. Please notice that when a clockpulse enters while you are reading the lower or upper byte of the counter, the correct value of the counter can not be read.

       figure 1.6 I/O port of the systemtimer
  ----------------------------------------------------
  I/O port   R/W    function
  ----------------------------------------------------
  0E6H        W      writing any data will clear the counter
  0E6H        R      reading the lower 8 bits of the counter
  0E7H        R      reading the higher 8 bits of the counter
  ----------------------------------------------------

Clearing the counter may have some effect on programs that are called from and interrupt routine etc and that use this timer. A program could basically look like the program below:

        countlow        equ     0E6H    ;lowbyte counter
        counthigh       equ     0E7H    ;highbyte counter

        ;The program will wait for the time designated with B
        ;The time that will be waited is B*3.911 usec.
        ;The time difference will be about -3.911 - +0 usec.
        ;It is not allowed to assign 0 to the B register.
        ;It is not allowed to generate interrupts while using this program.
        ;The contents of register C,A,F will be damaged.

        wait:
                        in      a,(countlow)  ;get current counter value
                        ld      c,a           ;save this
        wait_loop:
                        in      a,(countlow)  ;get current counter value
                        sub     c             ;check time elapsed
                        cp      b             ;waittime already elapsed?
                        jr      c,wait_loop   ;if not , loop
                        ret

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("chakkari-copy", "", "Hardware", "", "Chakkari Copy - MSX-BIOS: Chakkari Copy:...",
        R"MAN(HARDWARE: I/O-Port map
Chakkari Copy - MSX-BIOS: Chakkari Copy

-------------------------------------------------------------------------------

Chakkari Copy hardware description

-------------------------------------------------------------------------------

This document describes the functionality of the Chakkari Copy cartridge hardware,
created by reverse engineering the 8KB ROM.

+--------------------------------------------+
|          0000-3FFF: 16 KB RAM in COPY mode |
| Page 0:  available in COPY mode only       |
|          can be toggled to read-only       |
+--------------------------------------------+
|          4000-5FFF: 8KB ROM code           |
| Page 1:  +                                 |
|          6000-67FF: 2KB internal RAM       |
+--------------------------------------------+

The shown 16KB RAM in page 0 are only available in COPY mode, but can be toggled to read-only.
The location of the RAM in RAM mode is probably page 2 (but could not be confirmed, so it's just a guess).

The memory at 6800-7FFF is unused by the cartridge.

Internal handling: during cartridge initialisation Chakkari Copy copies the contents of BIOS
page 0 into it's own RAM and patches some entries to capture the input of PAUSE and COPY buttons of the
cartridge. Afterwards the patched memory is set to read-only.

PAUSE and COPY itself are processed by the routines in the ROM. In case the page 0 is (for any reason)
switched to the original MSX-BIOS  the cartridge buttons are no longer working. In this case
CALL SCHANGE switches the BIOS to the patched version again.

In case a program switches screen modes in a way that prevents Chakkara Copy from detecting
the correct parameters Chakkara Copy can be started by keeping the COPY button pressed. In this
case the screen-mode autodetect will be skipped and the parameters can be entered manually.

This appears to be an undocumented feature, confirmed by typos and a very rudimentary screen handling.

In case the printer is not ready or not available the printing can be aborted by pressing Ctrl+Stop.

The Chakkari Copy cartridge has one register, which controls it's complete behavior.
This register is available at MSX port &H7F.

READ only:

+-----------------------------------------+
|  1  |  1  |  1  |  1  |  1  | PSE | CPY |
+-----+-----+-----+-----+-----+-----+-----+

CPY: 0 = Copy button pressed, 1 = Copy button released

PSE: 0 = Pause button pressed, 1 = Pause button released

WRITE only:

+-----+-----+-----+-----+-----+-----+-----+
|  1  |  1  |  1  |  1  | RAM | PSE | CPY |
+-----+-----+-----+-----+-----+-----+-----+

CPY: 0 = Copy LED on, 1 = Copy LED off

PSE: 0 = Pause LED on, 1 = Pause LED off

RAM: 0 = Page 0 in RAM mode, 1 = Page 0 in ROM mode (applies to COPY mode only)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("ppi-programmable-peripheral-interface", "", "Hardware", "", "PPI (8255) - PPI (Programmable Peripheral Interface): A8 | I/O | PPI port A:...",
        R"MAN(HARDWARE: I/O-Port map
PPI (8255) - PPI (Programmable Peripheral Interface)

A8 | I/O | PPI port A: slotselect-register (2 bits per page/slot)Page 3  Page 2  Page 1  Page 0PSLOT#  PSLOT#  PSLOT#  PSLOT#
A9 | I | PPI port B: keyboard column input
AA | I/O | PPI port C: keyclick, CapsLED, tape, keyboardBits 0-3 = Keyboard row select
      Bit 4 = CasMotor (0=on, 1=off)Bit 5 = CasOutBit 6 = Caps LED (0=on, 1=off)
      Bit 7 = KeyClick
AB | O | PPI mode port:can be used to directly set or reset any bit of port C, when bit 7 is 0.
      Bit 0 = set or reset bitBits 1-3 = bit number (0-7)Bits 4-6: not used
      Bit 7: must be 0 tp perform the single bit set/reset functionis used to set the operating mode of the PPI, when bit 7 is 1.
      Bit 0 = C Dir; direction of lower four bits only of port C: 0=Output (MSX), 1=Input
      Bit 1 = B Dir; direction of Port B: 0=Output, 1=Input (MSX)
      Bit 2 = B&C Mode; operating mode of Port B and the lower bits only of Port C: 0=Normal mode (MSX), 1=Strobed mode
      Bit 3 = C Dir; direction of the upper four bits only of Port C: 0=Output (MSX), 1=Input
      Bit 4 = A Dir; direction of Port A: 0=Output (MSX), 1=Input
      Bits 5-6 = A&C Mode; operating mode of Port A and the upper four bits only of Port C: 00=Normal mode (MSX), 01=Strobed mode, 01=Bidirectional mode
      Bit 7: must be 1 in order to alter the PPI mode

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("s1990", "", "Hardware", "", "S1990: The S1990 chip is used to switch between different CPU's and CPU-modes (available...",
        R"MAN(HARDWARE: I/O-Port map
S1990

The S1990 chip is used to switch between different CPU's and CPU-modes (available since Turbo-R).

&HE4: | Register write (write only)
&HE5: | Register contents (read/write)

The S1990 seems to:

increment the register-number automatically, after a value was written
keep register-number unchanged, after a value was read
have 16 registers available for access.

Registers:

5 | &H00=right&H40=left (front-switch active)
     Note: the front-switch is usually set to boot the firmware instead of MSX-Basic
6 | Bit 5: Processor more (0=R800, 1=Z80)
     Bit 6: ROM mode (0=ROM, 1=DRAM)
      &H00=R800 DRAM
      &H40=R800 ROM
      &H60=Z80
13 | ? (3 perhaps a primary slot # ?)
14 | Contains last but one byte written to memory
15 | Contains last byte written to memory

Note: MSX engines other than S1990 behaves different compared to S1990 (for Turbo-R, of course). Engines other than S1990 never waits until the next VDP command arrives. S1990 waits.

It is rumoured the turboR R800 mode's VDP slowdown can be disabled through one of the S1990's registers.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("kanji-rom", "hw-kanji,kanji", "Hardware", "", "MSX-BIOS: Kanji ROM: Here we explain mostly about how to access Kanji-ROM",
        R"MAN(HARDWARE: Kanji ROM's
MSX-BIOS: Kanji ROM

Here we explain mostly about how to access Kanji-ROM

Cautions upon use of Kanji driver

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("about-how-to-access-kanji-rom", "", "Hardware", "", "Acessando a ROM Kanji - About how to access Kanji ROM: 2. 4. 1 Input/output of Kanji ROM",
        R"MAN(HARDWARE: Kanji ROM's
Acessando a ROM Kanji - About how to access Kanji ROM

2. 4. 1 Input/output of Kanji ROM

If you want to access kanji ROM on MSX, directly access D8H to DBH of I/O.

To read a kanji font data, you should use 12-bit data called kanji codes. These codes are different with JIS codes and kuten codes, so you should find out the kanji code of the one you want to display.

#You can convert a kanji into JIS code by executing CALL JIS in kanji mode.

CALL JIS (A$,kanjistring$)

then A$ will be a string of 4-digit hexadecimal. Kanjistring$ can be any long, but what you get is always a JIS of the first character.

How to Calculate a Kanji Code

Deduct 32 from the upper byte of a kanji's JIS code and call the value as ku. Deduct 32 from the lower byte of a kanji's JIS code and call the value as ten. (these are so-called kuten codes).

In case of 1st class kanjis, if ku is 15 or less, a kanji code is ku*96+ten. If ku is 16 or greater, a kanji code is ku*96+ten-512.

In case of 2nd class kanjis, a kanji code is ((ku-48)*96+ten).

Then, access I/O port using the kanji code you calculated. This differs depending on 1st and 2nd class.

To Read a 1st Class Kanji

Output lower 6 bit of a kanji code to D8H of I/O port, and upper 6 bit to D9H. And read the value 32 times uninterrupted from D9H to get the font for the kanji.

To Read a 2nd Class Kanji

Output lower 6 bit of a kanji code to DAH of I/O port, and upper 6 bit to DBH. And read the value 32 times uninterrupted from DBH to get the font for the kanji.

Font data are arranged this way. First, 8 bytes for upper-left, then 8 bytes for upper-right. Then 8 bytes for lower-left, and 8 bytes for lower-right.

Order of Data

1st to 8th    A
9th to 16th   B
17th to 24th  C
25th to 32th  D

Font Dots

1                     9
to    A        B     to
8                    16
17                   25
to    C        D     to
24                   32

Usually, direct input/output to I/O port is forbidden. But there is no other way in case of kanji ROM access. BASIC command PUT KANJI and BIOS called KNJPRT, which were added for MSX2 and later only supports 1st Class kanjis (MSX2+ and later supports 2nd class too) and also useless when you want to directly tamper with kanji fonts, say, when you want to compress 16*16 dot fonts into 12*16 dots, or when you want to directly print out the kanji font as bit images. Also, they do not support screen 2 or 4. And, there is no BIOS to access kanji ROM available in MSX1. Therefore, we access I/O port directly in case of kanji ROM access.

Beside, as many of you might know, if you access blank part of kanji ROM, you do not get a blank space but a font of kanij you didn't want to at all. To get a 2-byte space, make sure to use that of JIS code 2121H. About hankaku letters (narrow katakanas, alphabets and puctuations), use only the left part (A and C in the diagram up) of fonts between JIS code 2021H to 207EH, and 2921H to 295FH. These hankaku fonts are not supported by kanji drivers and PUT KANJI command.

2. 4. 2 Existence of Kanji ROM

If you try kanji input/output without kanji ROM, bean curds (16*16 dots white square) appear on the screen and you will feel really sorry. Therefore, calibrating the existence of kanji ROM becomes necessary.

To Check Whether 1st Class Kanji ROM Exist

First 8 bytes of the font with JIS code 2140H (ku-1, ten-32) of the kanji ROM is always in order of 00H, 40H, 20H, 10H, 08H, 04H, 02H, 01H. This means that, if you read the out font for this code, and if all first 8 bytes correspond to these, there must be 1st Class kanji ROM present.

To Check Whether 2nd Class Kanji ROM Exist

Read out the font data of the one with JIS code 737EH (ku-83, ten-94), sum up the values of first 8 bytes and divide it by 256, and if mod is 149 (95H), there must be 2nd Class kanji ROM present.

When making a software, make sure it checks the existence of kanji ROM, and to make it available on machines without kanji ROMs, prepare 1-byte messages. A software which uses tons of rare kanjis on 2nd-Class equipped machines and display simple alternative kanjis on machines without is possible, but I have never seen such.

2. 4. 3 SHIFT JIS Code

In MSX, kanji files use SHIFT JIS code (Microsoft kanji code). This is because since in MS-DOS kanji files are standardized by Microsoft kanji code, so on MSX which use same format, same codes are used.

In shift JIS code, there are no kanji start and end codes, and you can use both hankaku and 2 byte kanjis together.

Here, we present an algorithm to convert from JIS code into SHIFT JIS.

Assume the upper byte of a JIS code as JIH, and lower byte as JIL. And assume upper byte of SHIFT JIS code as SJH, and lower byte as SJL.

To Convert from JIS Code into SHIFT JIS Code

1. If JIH is 5EH or below,  INT((JIH-1)/2+71H) is SJH. Otherwise, INT((JIH-1)/2+B1H) is SJH..

2. If JIH is even number, (JIL+7EH) is SJL. Otherwise, temporary suppose (JIL+1FH) as SJL. If this value is 7FH or more, add 1 to it. If it is smaller than 7FH, leave it as is.

To convert from SHIFT JIS into JIS

1. If SJH is smaller than A0H, subtract 71H from SJH and name it as H. If it is more than A0H, subtract B1H from SJH and name it as H instead. Whichever the case, substitute H*2+1 into H again.

2. If SJL is greater than 7FH, then SJL-1 is L. Otherwise, SJL itself is. Then, if L is smaller than 9EH, L-1FH becomes L. If L is 9EH or more, L-7DH becomes L, and H+1 becomes H.

    Now value of H is upper byte of JIS code, and L is the lower byte of it.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("cautions-upon-use-of-kanji-driver", "", "Hardware", "", "Usando o driver Kanji - Cautions upon use of Kanji driver: Chapter 8 Cautions upon Use of...",
        R"MAN(HARDWARE: Kanji ROM's
Usando o driver Kanji - Cautions upon use of Kanji driver

Chapter 8 Cautions upon Use of Kanji Driver

Refer to "MSX2+ Powerful Application" from ASCII Corp. to see how to use kanji driver. Here we explain information not noted in the document.

When kanji driver is active, screen is put into SCREEN 5 or 7 status. This means, when kanji driver is active, even if you execute something like SCREEN 0, the change will be limited to number of characters per line; screen remains in graphic mode. If you execute, say, SCREEN 5 in this state, you will enter what is called kanji graphic mode, where you can use LINE or PAINT statements normally, while at the same time you can display kanjis with PRINT statements. However, you cannot input kanjis in this mode. (your can do so by coding your own handmade machine code program, though).

Other things you need to pay attention include, that once a kanji driver is booted, free area decreases by 4 kilobytes . (type of MSX-JE you booted might result in decrease of less than 4 kilobytes. But make sure to reserve 4 kilobytes for kanji driver when you program, for safety purpose.) This means, if a software is using memory to limit, it might malfunction once it boots kanji driver. And one more; all contents of program stacks will be cleared on the first occasion you boot kanji driver using CALL KANJI statement. This means, information like the original line number you were before jumping out with GOSUB statement, and statuses and variable contents of FOR-to-NEXT loops, will be altogether deleted. It is recommended to execute CALL KANJI on first lines in programs which use kanji driver.

And, you will find yourself that you can use kanjis for file names when kanji driver is active. But if you do so, then files with their names which you can never call in non-kanji modes happen to be created; so avoid using kanjis for file names. For instance, you will get a "Bad file name" error if you try to direct a file beginning with a kanji "yo" (4024H in JIS kanji code) in non-kanji mode. (but DOS kernel of Ver. 2.31 or later does not regard this as an error)

We introduce you hooks which a kanji driver overwrites, for your notice. Be careful when your program uses following hooks.

FDA4H(H.CHPU),FDA9H(H.DSPC)
FDAEH(H.ERAC),FDB3H(H.DSPF)
FDB8H(H.ERAF),FDBDH(H.TOTE)
FDC2H(H.CHGE),FDDBH(H.PINL)
FDE5H(H.INLI),FFB6H(H.LPTO)

If one of these hooks is called, then contents of "hidden" registers and IX and IY registers are destroyed. Therefore, a software which expects contents of registers stay as they are, does not run properly if you boot kanji driver.

Note, in addition to above, there are more hooks which a kanji driver overwrites, as noted below. I am not sure whether hidden registers and index registers stay preserved or not when these hooks are called. Probably they will be preserved. But I cannot answer, since I'm not an ASCII person ;)

FD9FH(H.TIMI),FF84H(H,WIDT)
FFC0H(H.SCRE),FFCAH(FCALL)

--------------Out-of-space article--------------

About locations of pattern name tables when kanji driver is in use; they are placed on following VRAM addresses.

SCREEN 2    1E20H+40*Y+X

(if you equip MSX1 machine with kanji driver, then screen 2 or 3 is chosen)

SCREEN 3    0B00H+16*Y+X
SCREEN 4    1B00H+40*Y+X    (0<=Y<=2)
    1B08H+40*Y+X    (3<=Y<=11)
SCREEN 5    (6A00H+40*Y+X+RG23SA*128) AND 7FFFH
SCREEN 6    (6A00H+80*Y+X+RG23SA*128) AND 7FFFH
SCREEN 7    D400H+80*Y+X+RG23SA*256 AND FFFFH
SCREEN 8    D400H+40*Y+X+RG23SA*256 AND FFFFH

Normally, one of SCREEN 5 to 7 is used. (refer to kanji BASIC manual) Since kanji driver utilizes hardware scrolls, so pattern name tables slither like slugs;) for each time a scroll occur.

Screen 2 or 3 is used when you equip MSX1 machine with a kanji driver.

Takamichi's note: what is "pattern name table? ^^;

About CALL KANJI statement and screen modes which a MSX enters into then.

About size of one kanji character;

CALL KANJI016 x 16 pixels (default)

CALL KANJI1Standard MSX kanji ROM is used. Characters are compressed to 12 x 16 pixels.

CALL KANJI2Same as in CALL KANJI 0. But vertical compression occurs due to interlace mode, so lines you can display increase.

CALL KANJI3Same as in CALL KANJI 1. But vertical compression occurs due to interlace mode, so lines you can display increase.

About screen modes;

CALL KANJI0 or CALL KANJI2 ... SCREEN 5 if WIDTH statement states 20 to 32 characters per line, SCREEN 7 if it states 33 to 64 characters per line.

CALL KANJI1 or CALL KANJI3 ... SCREEN 5 if WIDTH statement states 20 to 40 characters per line, SCREEN 7 if it states 41 to 80 characters per line.

(I don't know when screen 8 is chosen. Maybe it's possible only by executing SCREEN 8 statement after entering kanji mode using CALL KANJI)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("pcm", "hw-pcm", "Hardware", "", "MSX-BIOS: PCM: 6.3.6 PCM Sound",
        R"MAN(HARDWARE: PCM
MSX-BIOS: PCM

6.3.6 PCM Sound

Users are permitted to access turboR PCM using I/O ports.

If you control PCM by yourself, you can change sampling rates to any frequency you desire, or replay the recorded sound data through different frequency to achieve special sound effect. Here we would like to describe how to control I/O ports directly to handle PCMs. See later for BIOSes.

I/O Ports Specifications;

I/O Port Nos. bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0
A4H (read)    0    0    0    0    0    0    CT1  CT0
A4H (write)   DA7  DA6  DA5  DA4  DA3  DA2  DA1  DA0
A5H (read)    COMP 0    0    SMPL SEL  FILT MUTE ADDA
A5H (write)   0    0    0    SMPL SEL  FILT MUTE ADDA

Summary;

CT1, CT0 (counter data)

These counters counts every 63.5 microseconds. PCM BIOSes reads/writes sync to these counters. Once you write a value into A4H of I/O, these counters will be cleared.

DA'7 to DA0 (D/A output data)

PCM data will be played when you write a 8-bit data here. Format of the data is absolute binary; 127 is equal to 0 level (see my description about BASIC statements for what the "level" is). In times of recording, MSX outputs a value here and checks recorded data by learning whether the value is greater or lesser than the coming data.

COMP (comparator output signal)

MSX compares sample hold output signal and D/A converter output signal. If D/A output is smaller than sample/hold output, this COMP bit stands up. If D/A output is equal or greater than sample/hold output, COMP bit does not stand up. Use this at times of recording.

SMPL (sample/hold signal)

Use this to select whether MSX will sample the input signal, or hold the value. If the input value changes while very time you are checking the value itself;), there will be a big trouble, so you should keep this SMPL bit at 1 when you are in middle of checking input value.

SEL (selection of filter input signal)

Use this bit to decide whether D/A converter output or an output signal from microphone amplifier is used as input signal to low path filter. If SEL bit is 0, then D/A converter output signal is used; if 1, then microphone amplifier output signal is used.

FILT (selection of sample/hold circuit input signal)

Use this bit to decide whether filter output signal or standard signal is used as input signal to sample hold circuit at times of A/D. If FILT bit is 0, then standard signal (default after resetting an MSX); if 1, filter output signal.

MUTE (muting control)

This signal is responsible for turning on/off sound output over whole system. If 0, then sound output is off (default after resetting an MSX), while 1 means sound output is on.

ADDA (buffer mode)

This bits specifies D/A converter output mode. Specify 0 (double buffer; a default after resetting an MSX) to do D/A, while you should specify 1 (single buffer) to do A/D.

To play a PCM sound, output 03H to address A5H, and continuously output the data from memory to address A4H. Use system timer wisely to sync well during such output. By using counters at A4H, you can use 4 types of frequencies supported by BIOS to play. It will be even better if you do not use these counters but delicately changes frequencies relying on system timers, and adjust the tone pitch while playing.

To record a PCM sound, firstly, output 06H to address A5H (a startup, to turn FILT and MUTE to 1), then

<< output 16H to A5H (SMPL is 1), 80H to A4H, and immediately read a value of A5H. If bit 7 of the value is standing (=if COMP bit is standing), it means signal in question is 80H or more, so add 40H to output value 80H, and output this value to A4H. If bit7 is not standing, it means signal in question is less than 80H, so deduct 40H from the output value 80H, and output the value back again to A4H. Next, read value of A5H again, and check if bit7 is standing or not, and add/subtract 20H to the output value, and output the result value back to A4H, then read A5H, check bit7, add/subtract 10H from ... (snip) ... repeat such procedures 8 times, that's what you are required to perform 1 byte of data. Save this value into the memory, and output 03H to A5H >>

and adjust sync using system timer or something, repeat all the process enclosed by <<>>, read next 1 byte, then next byte (snip)... recording is done this way.

If you allocate sampling data into VRAM, and if you use different screen modes at times of recording and playing, you may suffer incorrect playing. This happens because MSX VRAM allotment varies upon screen modes. While I recommend you to see other sources for details, I simply explain, that 0001H address in SCREEN5 and 0001H address in SCREEN7 are different locations.

Routines to deal with PCM routines:

PCMPLY (0186H/MainROM) - play PCM sound

PCMREC (0189H/MainROM) - record PCM sound

Refer to: MainROM

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("midi", "hw-midi", "Hardware", "", "MSX-BIOS: MIDI: The MSX MIDI I/O ports (&HE8 and &HE9) are available since MSX Turbo-R:",
        R"MAN(HARDWARE: MIDI
MSX-BIOS: MIDI

The MSX MIDI I/O ports (&HE8 and &HE9) are available since MSX Turbo-R:

           +----+----+----+----+----+----+----+----+
E8 (read)  |RXD7|RXD6|RXD5|RXD4|RXD3|RXD2|RXD1|RXD0|
           +----+----+----+----+----+----+----+----+
E8 (write) |TXD7|TXD6|TXD5|TXD4|TXD3|TXD2|TXD1|TXD0|
           +----+----+----+----+----+----+----+----+

RXD7-RXD0  =  8251 receive data

TXD7-TXD0  =  8251 send data

           +----+----+----+----+----+----+----+----+
E9 (read)  |DSR |BRK | FE | OE | PE |EMPT|RDDY|RTDY|
           +----+----+----+----+----+----+----+----+

The first data that is written after a reset into the command register
is the mode instruction. The mode instruction looks as following:

           +----+----+----+----+----+----+----+----+
           | S2 | S1 | EP |PEN | L2 | L1 | B2 | B1 |
           +----+----+----+----+----+----+----+----+

B2, B1 | bit 1,0 | 00 = synchrone mode
 01 = baudrate factor is 1
 10 = baudrate factor is 16
 11 = baudrate factor is 64
L2, L1 | bit 3,2 | 00 = 5 data bits
 01 = 6 data bits
 10 = 7 data bits
 11 = 8 data bits
PEN | bit 4 | 0 = no parity
 1 = parity present
EP | bit 5 | 0 = no even parity
 1 = even parity
(not necessary if PEN = 0)
S2, S1 | bit 7,6 | 00 = no stop
 01 = 1 stop bit
 10 = 1.5 stop bit
 11 = 2 stop bits
 (valid for the sender. The receiver always has 1 stop bit)
Indien | bit 1,0 | 00 = synchrone mode, in this case bits 6 and 7 have the following meaning:


   bit 6 | ESD | 0 = internal synchronisation
   1 = external synchronisation
bit 7 | SCS | 0 = sync. character
   1 = synch. characters
   (the sync characters have to be written immediately after the mode instruction
   into the command register)


After the mode instruction the command instruction can be written
at any desired time.

The mode instruction looks as following:

           +----+----+----+----+----+----+----+----+
           | EH | IR |RIE | ER |SBRK| RE |TIE |TEN |
           +----+----+----+----+----+----+----+----+

EH | 0 = no effect
 1 = hunt mode
IR | 0 = no effect
 1 = internal reset
RIE | RTS?
ER | 0 = no effect
 1 = reset error flags PE,OE,FE
SBRK | 0 = no effect
 1 = send break character
PE | 0 = disable recipent
 1 = enable recipient
TIE | CTS?
TEN | 0 = disable sender
 1 = enable sender
For initialisation of in- and output of the MIDI port the following
bytes have to be written to port &H9E: 00,00,00,40.

The program sees

8253 Out2 interrupt (&HEA, &HEB):

           +----+----+----+----+----+----+----+----+
EA (write) | -  | -  | -  | -  | -  | -  | -  | -  |
           +----+----+----+----+----+----+----+----+

Interrupt, reset

8253 counters (&HEC-&HEF):

           +----+----+----+----+----+----+----+----+
EF (read)  |SC1 |SC0 |RW1 |RW0 | M2 | M2 | M0 |BCD |
           +----+----+----+----+----+----+----+----+

SC1,SC0 | 00 = select counter 0
 01 = select counter 1
 10 = select counter 2
 11 = not permitted
RW1,RW0 | 00 = latch counter
 01 = read/write MSB only
 01 = read/write LSB only
 11 = read/write LSB and MSB
M2,M1,M0 | 000 = mode 0 (interrupt on terminal count, time out of a modem)
 001 = mode 1 (programmable one shot)
 X10 = mode 2 (rate generator, real time clock)
 X11 = mode 3 (blockgolf generator, baudrate generator)
 100 = mode 4 (software triggered strobe)
 101 = mode 5 (hardware triffered strobe, pulse-width measuring)
BCD | 0 = binary counting
 1 = BCD counting

           +----+----+----+----+----+----+----+----+
EC (R/W)   |CT7 |CT6 |CT5 |CT4 |CT3 |CT2 |CT1 |CT0 |
           +----+----+----+----+----+----+----+----+
ED (R/W)   |CT17|CT16|CT15|CT14|CT13|CT12|CT11|CT10|
           +----+----+----+----+----+----+----+----+
ED (R/W)   |CT27|CT26|CT25|CT24|CT23|CT22|CT21|CT20|
           +----+----+----+----+----+----+----+----+

CT7-CT0 | data counter 0
CT17-CT10 | data counter 1
CT27-CT20 | data counter 2

           +----+----+----+----+----+----+----+----+
E2 (write) | EN | -  | -  | -  | -  | -  | -  | E8 |
           +----+----+----+----+----+----+----+----+

EN | MIDI interface on/off (0=on, 1=off)
E8 | MIDI interface on E8/E9 or E0/E1 (0=E8/E9, 1=E0/E1)

           +----+----+----+----+----+----+----+----+
E0 (read)  |RXD7|RXD6|RXD5|RXD4|RXD3|RXD2|RXD1|RXD0|
           +----+----+----+----+----+----+----+----+
E0 (write) |TXD7|TXD6|TXD5|TXD4|TXD3|TXD2|TXD1|TXD0|
           +----+----+----+----+----+----+----+----+

RXD7-RXD0  =  8251 receive data

TXD7-TXD0  =  8251 send data

           +----+----+----+----+----+----+----+----+
E1 (read)  |DSR |BRK | FE | OE | PE |EMPT|RDDY|RTDY|
           +----+----+----+----+----+----+----+----+

DSR | 8253 data set ready
BRK | 8251 sync detection
FE | 8251 framing error
OE | 8251 overrun error
PE | 8251 parity error
EMPT | 8251 transmitter empty
RDDY | 8251 receiver ready
TRDY | 8251 transmitter ready

Check MIDI:

Available since Turbo-R (verify byte on 002DH of the Main-ROM,
which must be have at least the value 3).

Internal MSX MIDI of U-Pack (no FM-Basic)

Address | internal | external
4018 | 41 (A) | ?? (?)
4019 | 50 (P) | ?? (?)
401A | 52 (R) | ?? (?)
401B | 4C (L) | ?? (?)
401C | 4F (O) | 4D (M)
401D | 50 (P) | 49 (I)
401E | 4C (L) | 44 (D)
401F | 4C (L) | 49 (I)

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("scsi-wd33c93a", "hw-scsi,scsi", "Hardware", "", "SCSI: WD33C93A: The WD33C93A is the SCSI-Controller chip used in many SCSI-Interfaces",
        R"MAN(HARDWARE: SCSI Controller (WD33C93A)
SCSI: WD33C93A

The WD33C93A is the SCSI-Controller chip used in many SCSI-Interfaces
for the MSX (e.g. HSH and NOVAXIS). This document describes only
features that are specific to the WD33C93A and does not contain
any (public) SCSI specification!

Register select / read auxilary status (&H34)

Writing to this port (#34) selects a register of the WD33C93A,
which can be written or read by accessing port #35.
Reading from this port (#34) returns the auxilary status of the WD33C93A.

Read/write register (&H35)

Writing into this port writes to the WD33C93A register,
selected in port (#34). Reading from this port returns
the contents of the corresponding register.

After every access to this port the register-number
is automatically incremented by one, except for the following
locations: Auxilary Status Register ($1F), Data Register ($19) and the
Command Register ($18).

Reset (&H36)

Writing any(!) byte to port #36 resets the WD33C93A.
The BSY-flag in the status register is set until the reset is finished!

Undocumented(?) feature: reading from this port seems to return some sort of
status bits. This is essential for the functionality, but the association of
these bits is not known yet! (probably some output of the QueueTag?).

Unknown: are connected devices reset too?

Note: after the reset is finished the WD33C93 contains the value $00 in its "CDB 1" register ($03).
The WD33C93A and WD33C93B are containing the value $01. The difference between the two
chips is that the WD33C93B provides the "Queue tag" register ($1A), where the WD33C93A doesnt.

Registers:

The WD33C93A has 32 registers with the following meaning:
Register | Contents
0 | 00 | OWN ID/CDB SIZE REGISTER

  +---+---+---+---+---+---+---+---+
  |FS1|FS0| 0 |EHP|EAF|ID2|ID1|ID0|
  +---+---+---+---+---+---+---+---+
Bit 0-2: IDn SCSI ID Bits 0-2 set the SCSI bus ID number
that the WD33C93A will use during arbitration and selection.
Bit 3: EAF ENABLE ADVANCED FEATURES, when set to one, causes the
WD33C93A to enable certain advanced features. When this bit is
zero those features are disabled.
Bit 4: EHP ENABLE HOST PARITY, when set to one, enables odd parity
checking on the host bus; the PE bit in the Auxilary status register
will indicate parity errors detected on the host bus, and the HHP bit
in the Control register will be used. When this bit is zero, no checking
is performed on the host bus: the PE bit is not set when a parity error
is detected on the host bus, and the HHP bit must be set to zero.
NOTE: Parity is always generated on the host parity bit (DP), regardless
of the state of this bit.
Bit 5: RAF, not used, will be zero.
=$00: FS 8
Bit 6: FS 12
Bit 7: FS 16
FSn FREQUENCY SELECT 0-1 select the divisor that is applied to the
input clock. The resulting clock is used for data transfer timing and for
SCSI bus arbitration timing. The table below shows input clock frequency
ranges and the corresponding divisors. The correct divisor for the input
clock must be used, or SCSI bus timing specifications may not met.

+-------------+-----+-----+-----------+
| INPUT CLOCK |     |     | RESULTING |
|  FREQUENCY  | FS1 | FS0 |  DIVISOR  |
|    (MHZ)    |     |     |           |
+-------------+-----+-----+-----------+
|    8 - 10   |  0  |  0  |     2     |
|   12 - 15   |  0  |  1  |     3     |
|      16     |  1  |  0  |     4     |
|      xx     |  1  |  1  | undefined |
+-------------+-----+-----+-----------+
Note that an 11 HMZ clock rate should not be used, as the resulting SCSI bus
cleay delay may violate SCSI specifications. The formula for computing the
maximum SCSI data transfer rate is:

                             INPUT CLOCK FREQUENCY
MAXIMUM SCSI TRANSFER RATE = --------------------- [MByte/sec]
                                 CLOCK DIVISOR
1 | 01 | CONTROL REGISTER

  +---+---+---+---+---+---+---+---+
  |DM2|DM1|DM0|HHP|EDI|IDI|HA |HSP|
  +---+---+---+---+---+---+---+---+

Bit 0: HSP The HALT on SCSI PARITY ERROR bit enables the WD33C93A to terminate a Receive
or Transfer command if a parity error is detected on an incoming SCSI data byte. In the
Initiator role, termination due to a SSI parity error causes the ACK- pin to be the left
in the active state in order to inhibit any additional data transfers (REQs) by the Target;
this facilitates error handling with Target. Synchronous data transfers check parity every
4096 bytes, or at the end when the remaining transfer count, whichever is less.
Asynchronous transfers check parity on every byte.
Bit 1: HA The HALT on ATTENTION bit (in Target mode only) enables the WD33C93A to terminate
a Send or Receive command if the ATN- input is asserted. This normally indicates that the
Initiator detected a parity error while receiving data from the WD33C93A. The ATN- input is
tested before the start of the data-transfer, every 4096 bytes if the transfer count is
greater than 4096, and after the end of the transfer. These rules apply to both synchronous
and asynchronous transfers.
Bit 2: IDI
Bit 3: EDI
Bit 4: HHP
=$00: POLLED
Bit 5: BURST
Bit 6: BUS
Bit 7: DMA
2 | 02 | Timeout period
Timeout is value*10 ms (e.g. 125*10=1250ms)
3 | 03 | CDB 1
4 | 04 | CDB 2
5 | 05 | CDB 3
6 | 06 | CDB 4
7 | 07 | CDB 5
8 | 08 | CDB 6
9 | 09 | CDB 7
10 | 0A | CDB 8
11 | 0B | CDB 9
12 | 0C | CDB 10
13 | 0D | CDB 11
14 | 0E | CDB 12
15 | 0F | Target LUN
16 | 10 | Command phase
17 | 11 | Synchronous transfer
18 | 12 | Transfer count (MSB)
19 | 13 | Transfer count
20 | 14 | Transfer count (LSB)
21 | 15 | Destination id
Bit 6: DPD
Bit 7: SCC
22 | 16 | Source id
Bit 0-2: id
Bit 3: SIV
Bit 5: DSP
Bit 6: ES
Bit 7: ER
23 | 17 | SCSI status
$00 - Good Status.
$02 - Check Condition.
$04 - Condition Met/Good.
$08 - Busy.
$10 - Intermediate/Good.
$14 - Intermediate/Condition Met/Good.
$18 - Reservation Conflict.
$22 - Command Terminated.
$28 - Queue Full.
$xx - Other Codes Are Reserved
24 | 18 | Command
$00 Reset
$01 Abort
$02 Assert ATN
$03 Negate ACK
$04 Disconnect
$05 Reselect
$06 Select-With-ATN
$07 Select-Without-ATN
$08 Select-W/ATN-And-Transfer
$09 Select-w/o ATN-And-Transfer
$0A Reselect-And-Receive-Data
$0B Reselect-And-Send-Data
$0C Wait-For-Select-And-Receive
$0D Send-Status-And-Command-Complete
$0E Send-Disconnect-Message
$0F Set IDI
$10 Receive Command
$11 Receive Data
$12 Receive Message Out
$13 Receive Unspecified Info Out
$14 Send Status
$15 Send Data
$16 Send Message In
$17 Send Unspecified Info In
$18 Translate Address
$20 Transfer Info
25 | 19 | Data
26 | 1A | Queue tag (not available in WD33C93A?)
27 | 1B
28 | 1C
29 | 1D
30 | 1E
31 | 1F | Auxiliary status

Auxilary Status:

The Auxilary STATUS-register of the WD33C93A is read-only can be accessed by reading port #34.
Please do not confuse with the SCSI-Status, which can be retrieved from
the WD33C93A register #17.

Important: reading this status-register returns the current state and resets
the interrupt (INT) flag, therefore reading the status-register twice may
return different results (at least the INT-flag is zero in the 2nd query).

+-----+-----+-----+-----+-----+-----+-----+-----+
| INT | LCI | BSY | CIP |  0  |  0  | PE  | DBR |
+-----+-----+-----+-----+-----+-----+-----+-----+

Bit | Mask | Meaning
0 | $01 | DBR | 1=data buffer ready (read from register #19)
  Note: if a command ends with no more data (0 bytes result or DBR switching from 1 to 0)
  an interrupt is generated (INT bit set) and the resultcode can be retrieved from
  the SCSI status (register #17).
  Data Buffer Ready is used during programmed I/O to indicate to the processor
  whether or not the Dta register is available for reading or writing. During
  Send or Transfer commands which transmit data oder the SCSI bus, the DBR bit
  is set when the WD33C93A is ready to take a byte from the host; the bit is
  reset when the processor writes the byte to the data-register. During Receive
  or Transfer commands which receive data over the SCSI bus the DBR is set when
  a byte is received; it is reset when the processor reads the byte from the
  DATA register.
1 | $02 | PE | Parity Error status indicates that even parity was detected on a data
  byte received during an information transfer. Parity is checked on data received
  from the host bus during transfers out to the SCSI bus, and is checked on data
  received from the SCSI bus during transfers out to the host bus. Detection of
  parity error will set the PE status bit regardless of the state of the HHP or HSP
  bits in the Control register. The PE bit is cleared when a new command is issued.
2 | $04 | Not used, will be zero.
3 | $08 | Not used, will be zero.
4 | $10 | CIP | COMMAND IN PROGRESS, when set, indicates that the WD33C93A is interpreting the
  last command entered into the Command register and therefore this register is unavailable.
  When this bit is reset, a command may be written into the Command register.
5 | $20 | BSY | 1=busy, 0=ready
  BUSY indicates that a Level II command is currently executing and therefore only the
  Command register(when CIP=0), the DATA register, and the Auxilary Status register are
  accessible by the host. A Level II command may not be written to the Command register
  when this bit is one.
6 | $40 | LCI | LAST COMMAND IGNORED indicates that a command was issued by the host just prior to
  or concurrent with a pending interrupt, and therefore the command will be ignored and
  this status set.
7 | $80 | INT | 1=interrupt, 0=no interrupt
  Note: if the interrupt is set the SCSI status (register #17) must be read!
  INTERRUPT PENDING indicates that the Intrq pin is asserted. The host should read the SCSI
  Status register to clear the Intrq prior to issuing any commands.

Lex Lechz: Unfortunately Western Digital does no longer sell the WD33C93A, nor
have copies of the original documentation. Therefore I want to thank Hans Otten for scanning and publishing a copy of
the manual. I also want to thank Manuel Bilderbeek for his effort to find a copy of this
manual.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("sanyo-lightpen", "hw-lightpen", "Hardware", "", "MSX-BIOS: SANYO LIGHTPEN: The photoresistor attached to tip of the lightpen catches the...",
        R"MAN(HARDWARE: Lightpen (Sanyo compatible)
MSX-BIOS: SANYO LIGHTPEN

The photoresistor attached to tip of the lightpen catches the scanline of the TV display,
and finds which part of the screen is pointed by the pen.

This documentation has been created by reverse-engineering the MSX2/2+ BIOS
and the firmware of the Sanyo Lightpen cartridge. Therefore not all information may
be 100% correct.

The MSX MIDI I/O ports (&HB8 to &HBB) are assigned to the Sanyo Lightpen interface,
which is available for MSX1 and standardized for MSX 2/2+:

Registers
Data registers

                 +----+----+----+----+----+----+----+----+
B8 (read/write)  | Y7 | Y6 | Y5 | Y4 | Y3 | Y2 | Y1 | Y0 |   DATA0
                 +----+----+----+----+----+----+----+----+
B9 (read/write)  | X6 | X5 | X4 | X3 | X2 | X1 | X0 | Y8 |   DATA1
                 +----+----+----+----+----+----+----+----+
BA (read/write)  | ?? | ?? |Vrfy|Posn|Butn| X9 | X8 | X7 |   DATA2
                 +----+----+----+----+----+----+----+----+

Command/Mode register

                 +----+----+----+----+----+----+----+----+
BB (read/write)  | M7 | M6 | M5 | M4 | M3 | M2 | M1 | M0 |   CMND
                 +----+----+----+----+----+----+----+----+

Y0-Y8 | Y-coordinate (0-511)
 Not calibrated, top-most pixel has usually position 38 ($26)
X0-X9 | X-coordinate (0-1023)
 Not calibrated, left-most pixel has usually position 44 ($2C)
Butn | 0 if button pressed
Posn | 0 if X/Y coordinates are known (lightpen position valid)
Vrfy | 0 if position verification was successful.
                  If 1 X/Y coordinates are invalid and have to be ignored
                  (same as if Posn=1).
M0-M7

Interrupts

The Lightpen hardware appears to be able to generate interrupts.
Where the MSX-BIOS does not use them the Sanyo Lightpen Interface
relies on these interrupts.

The interrupt handling is not documented yet!

Operation
Get button-state/position

Read DATA2-register.

If Butn=0 the lightpen-button is pressed.

If Posn=0 the lightpen-position is known and DATA0 and DATA1 can be read to get the coordinates.
Registers should be read in this order. Under some circumstances the coordinates may not be valid, therefore
a validation might be required (not always, but when?).

Retrieved coordinates are non-calibrated, therefore the position of the left-most and top-most pixel is most likely not zero.
The BIOS-variables XSAVE and YSAVE (valid since MSX2) are keeping the (calibrated) position of the left/top pixel.

Verify position

This behavior is implemented in the Sanyo Lightpen Interface, but not in the MSX-BIOS:

Write retrieved (non-calibrated) coordinate-values X-18 and Y-3
into DATA0, DATA1, DATA2 (in this order). Bits that do not belong to coordinates should be
(need to be?) 0. Maybe a short cycle of some ms is necessary until the command is processed.

Then read DATA2 and check the contents of the Vrfy bit, which should be 0.
If 1 the coordinates are invalid and have to be ignored.

MSX BIOS
Lightpen support is available on MSX2 and MSX2+. Even if there is no explicit lightpen support
for MSX1 computers it can be used with the original "Sanyo Lightpen" cartridge which provides
provides the corresponding firmware (16KB ROM).

Settings

FAFE XSAVE   [10000000 XXXXXXXX]
FB00 YSAVE   [*0000000 YYYYYYYY] *-light pen interrupt request

More precisely:

             +-----+-----+-----+-----+-----+-----+-----+-----+
FAFE         |  X7 |  X6 |  X5 |  X4 |  X3 |  X2 |  X1 |  X0 |
             +-----+-----+-----+-----+-----+-----+-----+-----+
FAFF         |  LP | CX6 | CX5 | CX4 | CX3 | CX2 | CX1 | CX0 |
             +-----+-----+-----+-----+-----+-----+-----+-----+
FB00         |  Y7 |  Y6 |  Y5 |  Y4 |  Y3 |  Y2 |  Y1 |  Y0 |
             +-----+-----+-----+-----+-----+-----+-----+-----+
FB01         |  0  | CY6 | CY5 | CY4 | CY3 | CY2 | CY1 | CY0 |
             +-----+-----+-----+-----+-----+-----+-----+-----+

X0-X7 | Lightpen X position (0-255)
                  Updated by PAD(8) in MSX-Basic respectively the corresponding BIOS routines.
X0-X7 | Lightpen Y position (0-211)
                  Updated by PAD(8) in MSX-Basic respectively the corresponding BIOS routines.
LP | Lightpen flag
                  1 if lightpen is present, 0 if lightpen is not present.
                  This flag is set during MSX2/2+ boot and not modified later.
CX0-CX6 | Position of left-most pixel (0-127)
                  This value is set to 44 ($2C) during MSX2/2+ boot and not modified later.
CY0-CY6 | Position of top-most pixel (0-127)
                  This value is set to 38 ($26) during MSX2/2+ boot and not modified later.

Behavior

X position is in range 0-255 and Y-position is in range 0-211.
If the lightpen is placed outside of the drawable area
the PAD(8) command still succeeds and resulting coordinates
are automatically adjusted to the best-matching position.

Boot

???

Related
Bit 6 of port &HF5 is be set if a lightpen is available.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("tape-i-o", "hw-tape", "Hardware", "", "Tape I/O: BIT AND BYTE ENCODING",
        R"MAN(HARDWARE: Tape I/O
Tape I/O

BIT AND BYTE ENCODING
=====================
Information on tape is stored as square waves of 1200 or 2400 baud. The
encoding used is FSK (frequency shifting key). The next table shows the
relation between the selected recording speed, the bits and the pulses
put on tape:

bps   bit   encoding          freq (Hz)
                    _______
1200   0    _______|       |  1200
                ___     ___
1200   1    ___|   |___|   |  2400

                ___
2400   0    ___|   |          2400
              _   _
2400   1    _| |_| |          4800

Each byte is put on tape as a sequence of 11 bits:

 0 b0 b1 b2 b3 b4 b5 b6 b7 1 1  (one startbit, 2 stopbits, LSb first)

Futhermore 2 kinds of headers are used: short and long, a long header being
4 times as long as a short one. They consist of a series of square wave pulses
(bit value 1):

bps   header   freq (Hz)   nr of pulses
1200  short    2400        4000             (5/3  sec)
1200  long     2400        16000            (20/3 sec)
2400  short    4800        8000
2400  long     4800        32000

FILE FORMATS
============
Files on tape are stored in two blocks:
- the first block is the filedescriptor,
- the second contains the filedata.

Three types of files exist:
  BASIC file, memory dump and data file.

BASIC File (CSAVE/CLOAD)
  filedescriptor: long header
                  10x #D3       (filetype)
                  6 bytes filename

  file data:      short header
                  tokenized BASIC
                  7x #00

memory dump (BSAVE/BLOAD)
  filedescriptor: long header
                  10x #D0
                  6 bytes filename

  file data:      short header
                  2 bytes start address
                  2 bytes end address
                  2 bytes exec address
                  memory dump

data file (SAVE/LOAD/OPEN)
  filedescriptor: long header
                  10x #EA
                  6 bytes filename

  file data:      short header
                  block of 256 bytes

                  short header
                  block of 256 bytes

                  ....
                  last block contains 256x #1A (^Z/EoF)

MSX BIOS - TAPE Device Routines
are located at #00E1 to #00F3 in the MAIN-ROM.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    add("msx-rom", "hw-rom", "Hardware", "", "MSX ROM: The MSX ROM can start in page 1 (&H4000) or page 2 (&H8000) of any slot.",
        R"MAN(HARDWARE: ROM (geral)
MSX ROM

The MSX ROM can start in page 1 (&H4000) or page 2 (&H8000) of any slot.
Each ROM has a 16 byte header with the following meaning:

4000 or 8000 &H41 ('A') Magic ID

4001 or 8001 &H41 ('B')

4002 or 8002 Init adress LSB

4003 or 8003 Init adress MSB

4004 or 8004 Call Statement handler adress LSB

4005 or 8005 Call Statement handler adress MSB

4006 or 8006 Device handler adress LSB

4007 or 8007 Device handler adress MSB

4008 or 8008 Basic text adress LSB

4009 or 8009 Basic text adress MSB

400A or 800A Reserved

400B or 800B

400C or 800C Reserved

400D or 800D

400E or 800E Reserved

400F or 800F

See also: MILC: Call Cartridge

Magic ID

Each valid MSX-ROM is identified by "AB".

Init adress

This routine initializes the ROM-cartridge.

Call Statement handler adress

This routine handles MSX-Basic CALL statements.

The name of the CALL statement is stored in PROCNM (&HFD89), where all characters are
automatically converted to uppercase.

Device handler adress

This routine handles device-extensions, such as "COM:", "CAS:" or "A:".

Recognize device: Entry with A=0FFH indicates that the device is not yet recognized.

Perform device operation: Entry with A different from 0FFH indicates that the device
is already known.
Basic text adress

This is MSX-Basic program text, which can be interpreted.

FONTE
  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.)MAN");

    return v;
}

} // namespace man
