;***** Definicoes do MSX ******************************************************
;-- MSX BIOS ------------------------------------------------------------------
CHPUT   equ 0A2h
CALATR  equ 087h
RDVRM   equ 04ah
WRTVRM  equ 04dh
FILVRM  equ 056h
POSIT   equ 0C6h
LDIRMV  equ 059h
LDIRVM  equ 05Ch
CHGMOD  equ 05Fh
OUTDO   equ 18h
ENASLT  equ 024h
WRSLT   equ 014h
RDSLT   equ 0Ch
CALSLT  equ 01Ch
RSLREG  equ 0138h
GRPPRT  equ 08Dh
CLS     equ 0C3h
CHGET   equ 9Fh
ENASCR  equ 44h
DISSCR  equ 41h
GICINI  equ 90h
GTTRIG  equ 0D8h

;-- Constantes do MSX ---------------------------------------------------------
KEYROWS equ 11


;-- Variaveis de Sistema ------------------------------------------------------
USRTAB  equ 0F39Ah                      ;Tabela de DEF USR 0~9
FACLO   equ 0F7F8h                      ;Endereco Passagem Parametros USR
NEWKEY  equ 0FBE5h
PTRFLG  equ 0F416h
PTRFIL  equ 0F864h
FORCLR  equ 0F3E9h
BAKCLR  equ 0F3EAh
BDRCLR  equ 0F3EBh
LINL40  equ 0F3AEh
CLIKSW  equ 0F3DBh
CSRY    equ 0F3DCh
CSRX    equ 0F3DDh
EXPTBL  equ 0FCC1h
SLTTBL  equ 0FCC5h
RGSAV   equ 0F3DFh
RDPRIM  equ 0F380h
GRPACX  equ 0FCB7h
GRPACY  equ 0FCB9h
JIFFY   equ 0FC9Eh
CSRSW   equ 0FCA9h
CAPST   equ 0FCABh
LINLEN	equ #F3B0