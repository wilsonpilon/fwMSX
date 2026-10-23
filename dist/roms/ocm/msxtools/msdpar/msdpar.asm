;*** MSDPAR ***
;    Copyright (C) 2024 Cayce-MSX
;    based on PARSET V1.1 & PARLIST (C) 1998-1999 by Konamiman (MIT licensed)
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <https://www.gnu.org/licenses/>.


;******************************************************************************
;*                                                                            *
;*             BIOS functions                                                 *
;*                                                                            *
;******************************************************************************

; https://map.grauw.nl/resources/msxbios.php
RDSLT:  equ #000C
WRSLT:  equ #0014
CALSLT: equ #001C
ENASLT: equ #0024
RSLREG: equ #0138

; https://map.grauw.nl/resources/dos2_functioncalls.php
_DOS:       equ #0005
_STROUT:    equ #09
_DOSVER:    equ #6F
_CURDRV:    equ #19
_TERM:      equ #62 ; DOS2-specific
_ASSIGN:    equ #6A ; DOS2-specific

_ASSIGN_GET: equ #FF
DOS_KERNEL_MAJOR_V2: equ 2

; https://map.grauw.nl/resources/msxsystemvars.php
DISKSLOT:   equ #F348
DRVINF:     equ #FB21
EXPTBL:     equ #FCC1
SLTTBL:     equ #FCC5

; command line parameters
; https://map.grauw.nl/resources/dos2_environment.php
FCB:    equ #5D ;FCB for first parameter
DOS2_CMDLINE:   equ #80

ARG_1 equ 1
ARG_2 equ 2
ARG_3 equ 3


;******************************************************************************
;*                                                                            *
;*             CONSTANTS                                                      *
;*                                                                            *
;******************************************************************************

; https://en.wikipedia.org/wiki/Partition_type#List_of_partition_IDs
PARTITION_TYPE_EXT_CHS: equ 5
PARTITION_TYPE_EXT_LBA: equ #f

; https://en.wikipedia.org/wiki/Master_boot_record#Sector_layout
MAX_PRIMARY_PARTITIONS: equ 4
MBR_ENTRY_1 equ #1BE
MBR_ENTRY_2 equ #1CE
MBR_ENTRY_3 equ #1DE
MBR_ENTRY_4 equ #1EE

; https://www.msx.org/wiki/MegaROM_Mappers#ASCII_8K
A8k_PAGE0_SWITCH_ADDRESS  equ #6000

; https://www.konamiman.com/msx/megascsi/meguide.txt
MEGASCSI_READ_SECTOR_EXPAR:      equ #40 ; add ExPar id (0-15)
MEGASCSI_LOAD_DOS_PARTITION:     equ #84
MEGASCSI_LOAD_EXTRA_PARTITION:   equ #86
MEGASCSI_STORE_EXTRA_PARTITION:  equ #87

MEGASCSI_LAST_EXTRA_PARTITION:   equ 15

MEGASCSI_NOT_FOUND:  equ 17
MEGASCSI_ERR_PARTITION_DOES_NOT_EXIST:  equ 18

MEGASCSI_API_ENTRY:          equ #7FCC
MEGASCSI_SIGNATURE_ADDRESS:  equ #7FE0

; https://github.com/openMSX/openMSX/blob/master/src/memory/MegaFlashRomSCCPlusSD.cc#L245
MEGASD_DEFAULT_BANK equ 0
MEGASD_MMC_BANK equ #40 ; bank #6x seems related to OCM-PLD's EPCS FlashROM?
MEGASD_MMC_ADDRESS equ #4000 ; any in range #4000-#4FFF grants SD card access (R/W) with SD enabled

; derived by comparing megasd1s.rom vs. megasd2s.rom
MEGASD_DRIVE_COUNT_ADDRESS  equ #3f80 - #2000 + #4000; A8 page 1 (&7) at A8 bank 0, MSX-engine page 1
MEGASD_DRIVE_COUNT_BANK_A   equ #81
MEGASD_DRIVE_COUNT_BANK_B   equ #87

; https://web.archive.org/web/20140710011317/https://www.sdcard.org/downloads/pls/simplified_specs/part1_410.pdf, section 4.7.2 & 4.9.3
MMC_CMD_LEN                 equ 6
MMC_R2_RESPONSE_DATA_LEN    equ (136/8)-1
MMC_R2_RESPONSE_HEADER      equ #3f ; not received?
MMC_START_BLOCK_TOKEN       equ #fe ; .. but this instead

; ASCII
CR:     equ #0d
LF:     equ #0a
EOF:    equ #1a

; boolean
FALSE:          equ 0
TRUE:           equ #FF ; must be -1; tested to be 0 after inc

; sentinel
END_OF_LIST:    equ #FF

; 'heap'
BUFPAR:         equ #4000-255 ; final 255 Bytes of page 0 - assumption: program size < 16k-#100-255

; actions
ACTION_SHOW_USAGE       equ 0
ACTION_SHOW_DRIVE_MAP   equ 1
ACTION_MAP_PARTITION    equ 2
ACTION_LIST_PARTITIONS  equ 3
ACTION_SET_DRIVE_COUNT  equ 4

; error codes
; https://www.msx.org/wiki/MSX-DOS_2_Error_Messages
; - range 1-31 are silent; use `ECHO %_ERROR%`
; - range 32-63 show "*** User error xx"
; - also see GET_PART for MegaSCSI error codes
ERR_USAGE           equ 1 ; an error situation that led to showing usage instructions
ERR_INPUT           equ 2 ; invalid input or mismatch with disk configuration
ERR_INTERNAL        equ 3 ; internal error
ERR_MEGASCSI_BASE   equ 32


;******************************************************************************
;*                                                                            *
;*             MACROS                                                         *
;*                                                                            *
;******************************************************************************

; conditional relative jumps

MACRO jreq     aa  ;A = x
    jr  z,aa
    endm

MACRO   jrne   aa  ;A <> x
    jr  nz,aa
    endm

MACRO jrlt     aa  ;A < x
    jr  c,aa
    endm

MACRO   jrgt   aa  ;A > x
    jr  z,$+4
    jr  nc,aa
    endm

MACRO jrle     aa  ;A <= x
    jr  c,aa
    jr  z,aa
    endm

MACRO   jrge   aa  ;A >= x
    jr  nc,aa
    endm

; conditional absolute jumps

MACRO jpeq     aa  ;A = x
    jp  z,aa
    endm

MACRO   jpne   aa  ;A <> x
    jp  nz,aa
    endm

MACRO jplt     aa  ;A < x
    jp  c,aa
    endm

MACRO   jpgt   aa  ;A > x
    jr  z,$+5
    jp  nc,aa
    endm

MACRO   jple   aa  ;A <= x
    jr  c,aa
    jp  z,aa
    endm

MACRO   jpge   aa  ;A >=x
    jp  nc,aa
    endm


;******************************************************************************
;*                                                                            *
;*             PROGRAM                                                        *
;*                                                                            *
;******************************************************************************

    defpage 0,100h
    page 0
    code @ 100h

    jr main            ; type support, A:\>TYPE FILENAME.COM - inspired by KdL
    db CR,"Partition tool for OCM/MSX++ MegaSD v1.0d by Cayce-MSX 2024",EOF
main:


;;;;;;;;;;;;;;
;;; parse parameters
;;;;;;;;;;;;;;

    ld  a,ARG_1
    ld  de,BUFPAR
    call    EXTPAR
    jp  c,JP_TO_ACTION   ;No parameters? Show program usage

    ld  c,_CURDRV   ;Initialize DRIVE to default drive
    call _DOS
    ld  (DRIVE),a

    ld  a,(BUFPAR)  ;Check first character
    cp  "/" ;"/" examine options
    ld c,ARG_1
    jpeq PARSE_OPTS
    cp  ":" ;":" Show partition on default drive
    jrne PARSE_FCB
    ld a,ACTION_SHOW_DRIVE_MAP
    ld (ACTION),a
    ld c,ARG_2
    jr. PARSE_OPTS

PARSE_FCB:
    ld  a,(FCB-1)   ;Obtaining the unit by examining the FCB created in #5C from the command line
    or  a
    jr  z,OKDRIVE   ; no drive letter; use default
    ld  hl,#0801    ; drive A-H => range 1-8
    call    RANGE
    ld  de,BADDRIVS
    jp  nz,FERR
    dec a
    ld  (DRIVE),a

OKDRIVE:
    ld  a,(FCB)
    cp  " " ;If there is no partition number => show current mapping
    jrne PARSE_PRIM_EXT
    ld a,ACTION_SHOW_DRIVE_MAP
    ld (ACTION),a
    ld c,ARG_2
    jr. PARSE_OPTS

PARSE_PRIM_EXT:
    ld  a,(FCB+1)   ;Is the specified partition absolute or primary + extended?
    cp  "-"
    jreq PARPREXT

    ; absolute; transform to prim+ext
    ld  hl,FCB
    call    EXTNUM  ;BC = absolute partition number
    ex  de,hl
    ld  de,BADPARS  ;Error (incorrect parameter) when:
    jp  c,FERR  ;- The number is greater than 65535
    ld  a,l
    cp  " "
    jp  nz,FERR ;- The terminating character is not " "
    ld  a,b
    or  c
    jp  z,FERR  ;- The number is 0
    dec bc
    ld  a,b
    or  a
    jp  nz,FERR ;- The number is greater than 256

    ld  a,1
    ld  (P_PRIM),a
    ld  a,c ; if absolute partion -1 = 0 -> Partition 1
    or  a
    jpeq HAVE_ARGS
    ld  (P_EXT),a   ;If not, primary partition = 2
    ld  a,2 ; and extended partition = absolute partition -1
    ld  (P_PRIM),a
    jp  HAVE_ARGS

PARPREXT:
    ld  a,(FCB)
    ld  hl,"4"*#100+"1"; can't we do this nicer in sjasm?
    call    RANGE   ;Primary partition must be in range "1"-"4"
    ld  de,BADPARS
    jp  nz,FERR
    sub "0"
    ld  (P_PRIM),a

    ld  hl,FCB+2
    call    EXTNUM
    ld  de,BADPARS
    jp  c,FERR  ;Error if extended partion >65535
    ld  a,b
    or  a
    jp  nz,FERR ;Error if extended partion >255
    ld  a,c
    ld  (P_EXT),a

HAVE_ARGS:
    ld a,ACTION_MAP_PARTITION
    ld (ACTION),a
    ld c,ARG_2
    jr. PARSE_OPTS


;;;;;;;;;;;;;;
;;; Parse options
;;; in: C=first parameter that can contain an option
;;;;;;;;;;;;;;
PARSE_OPTS:
    ld a,c
    ld  de,BUFPAR
    call    EXTPAR
    jr  c,JP_TO_ACTION  ; no more options => perform selected action
    ld  a,(BUFPAR)  ;Check first character
    cp  "/"
    ld  de,TOOPARS
    jpne    FERR    ;too many (regular) parameters - expected only options

    ld a,(BUFPAR+1)
    cp "?"
    jreq OPTION_SHOW_USAGE
    res 5,a ; to_upper
    cp "H"
    jreq OPTION_SHOW_USAGE
    cp "L"
    jreq OPTION_LIST
    cp "E"
    jreq OPTION_ENABLE_DRIVES
    cp "Q"
    ld  de,TOOPARS
    jpne    FERR    ;unknown option
    ; (fall thru to /q handling)
    ; note: we ignore any characters after /x...
OPTION_QUIET:
    ld a,TRUE
    ld (QUIET),a
    inc c
    jr PARSE_OPTS

OPTION_SHOW_USAGE:
    ld a,ACTION_SHOW_USAGE
    ld (ACTION),a
    inc c
    jr PARSE_OPTS

OPTION_ENABLE_DRIVES:
    ld a,(BUFPAR+2)
    sub "0"
    ld  hl,#0801
    call    RANGE   ;# drives must be in range 1-8
    ld  de,BADDRVCS
    jp  nz,FERR
    ld (DRIVE_COUNT),a
    ld a,ACTION_SET_DRIVE_COUNT
    ld (ACTION),a
    inc c
    jr PARSE_OPTS

OPTION_LIST:
    ld a,ACTION_LIST_PARTITIONS
    ld (ACTION),a
    inc c
    jr PARSE_OPTS


;;;;;;;;;;;;;;
;;; prepare & select action to perform
;;;;;;;;;;;;;;
JP_TO_ACTION:
    ld  de,PRESEN
    call    PRINT

    ld a,(ACTION)
    cp ACTION_SHOW_USAGE
    jpeq SHOWUSE

    CALL DETECT_MEGASD
    jp  c,MEGAERR

    ld a,(ACTION)
    cp ACTION_LIST_PARTITIONS
    jreq LIST_PARTITIONS
    cp ACTION_SHOW_DRIVE_MAP
    jpeq SHOW_DRIVE_MAP
    cp ACTION_MAP_PARTITION
    jpeq MAP_PARTITION
    cp ACTION_SET_DRIVE_COUNT
    jpeq SET_DRIVE_COUNT

    ld b, ERR_INTERNAL
    jp EXIT_WITH_ERROR


;;;;;;;;;;;;;;
;;; ACTION: List all disk partitions
;;; input:
;;;   ID
;;;   SLOT
;;;;;;;;;;;;;;
LIST_PARTITIONS:
    ld  de,LISTING
    call    PRINT

    xor a           ; start at 1-0
    ld  (P_EXT),a
    inc a
    ld  (P_PRIM),a

SHOWLOOP:
    ld  hl,PARNS    ;Clean information area
    ld  de,PARNS+1
    ld  bc,LONSHOW-1
    ld  (HL),32
    ldir

    ld  a,(P_PRIM)  ;Convert primary partition to text
    add "0"
    ld  (PARNS),a

    ld  a,(P_EXT)   ;Convert extended partition to text
    or  a
    jr  z,NOEXT
    ld  e,a
    ld  d,0
    ld  hl,PARNS+2
    ld  b,3
    ld  c,0
    xor a
    call    NUMTOASC
    ld  a,"-"
    ld  (PARNS+1),a

LEFT:
    ld  a,(PARNS+2) ; left-align extended partition number
    or  a
    jr  nz,OKLEFT
    ld  a,(PARNS+3)
    ld  (PARNS+2),a
    ld  a,(PARNS+4)
    ld  (PARNS+3),a
    ld  a," "
    ld  (PARNS+4),a
    jr  LEFT
OKLEFT: ;
NOEXT:  ;

    ld  a,(ID)
    ld  d,a
    ld  a,(P_EXT)
    ld  b,a
    ld  a,(P_PRIM)

    call    GET_PART
    jr  nc,OKGPAR
    cp  MEGASCSI_ERR_PARTITION_DOES_NOT_EXIST   ;can we go to next primary partition?
    jp  nz,MEGAERR
    ld  a,(P_PRIM)
    inc a
    cp  MAX_PRIMARY_PARTITIONS+1    ;No more primary partition? Stop.
    jp  z,FINNN
    ld  (P_PRIM),a
    xor a ; set extended partition to 0
    ld  (P_EXT),a
    jp  SHOWLOOP
OKGPAR: ;

    ; Show partition type
    exx
    ld  c,a
    ld  (TYPEPAR),a
    ld  hl,TABTYPE  ;go through the table of types.
TYPELOOP:
    ld  a,(hl)
    cp  END_OF_LIST; type unknown
    jr  z,NOFTYP
    cp  c
    jreq FTYP
    ld  de,LONTYPE+1
    add hl,de
    jr  TYPELOOP

NOFTYP:
    ld  e,c
    ld  d,0
    ld  hl,TYPBYT+1
    ld  b,2
    ld  c,"0"
    ld  a,%00000001
    call    NUMTOASC
    ld  hl,NOFTS    ;create text or "unknown"
    jr  SETTYPS
FTYP:
    inc hl
SETTYPS:
    ld  de,TYPES
    ld  bc,LONTYPE
    ldir
    exx

    ; Establish initial sector
    ld  a,(TYPEPAR) ;Type 0 (not in use) or 5/#f (extended)? => don't show initial sector or size
    or  a
    jr  z,EXTOUN
    cp  PARTITION_TYPE_EXT_CHS
    jr  z,EXTOUN
    cp  PARTITION_TYPE_EXT_LBA
    jr  z,EXTOUN

    ld  a,c
    exx
    ld  e,a
    ld  d,0
    ld  hl,FIRSS+1
    ld  b,2
    ld  a,%00000001
    ld  c,"0"
    call    NUMTOASC    ;Byte 1
    exx
    push    de
    exx
    pop de
    ld  hl,FIRSS+3
    ld  b,4
    ld  a,%10000001
    call    NUMTOASC
    ld  a,"#"
    ld  (FIRSS),a

    ; Show size
    exx
    ld  a,b
    or  a
    jr  z,ENKAS
ENMEGS:
    ld  a,"M"
    ld  (SIZES+5),a
    ld  e,h
    ld  d,b
    ld  b,3
TOMEGS:
    srl d
    rr  e
    djnz    TOMEGS
    jr  SHOWSIZ
ENKAS:
    ld  a,"K"
    ld  (SIZES+5),a
    ex  de,hl
    srl d
    rr  e
SHOWSIZ:
    ld  hl,SIZES
    ld  b,5
    ld  c," "
    xor a
    call    NUMTOASC
    ld  a,"B"
    ld  (SIZES+6),a

    ; Show information
EXTOUN:
    ld  de,PARNS2
    call    PRINT

    ; Next partition
    ld  a,(TYPEPAR)
    cp  PARTITION_TYPE_EXT_CHS
    jreq IS_EXTENDED
    cp  PARTITION_TYPE_EXT_LBA
    jreq IS_EXTENDED

    ld  a,(P_EXT)
    or  a
    jr  z,NOT_EXTENDED

    inc a
    ld  (P_EXT),a
    jp  SHOWLOOP

NOT_EXTENDED:
    ld  a,(P_PRIM)
    inc a
    cp  MAX_PRIMARY_PARTITIONS+1
    jreq FINNN
    ld  (P_PRIM),a
    jp  SHOWLOOP

IS_EXTENDED:
    ld  a,1
    ld  (P_EXT),a
    jp  SHOWLOOP

FINNN:
    ret


;;;;;;;;;;;;;;
;;; ACTION: Show partition mapped to selected drive
;;; input:
;;;   DRIVE
;;;;;;;;;;;;;;
SHOW_DRIVE_MAP:
    ld  a,(DRIVE)
    add "A"
    ld  (DRIVES),a

    ; Display drive, slot, ID & MMC/SD CID

    ld  a,(SLOT)    ;Convert slot to text
    ld  b,a
    and %00000011   ; primary
    add "0"
    ld  (SLCAD),a
    bit 7,b
    jr  z,OKSLCAD   ; unexpanded
    ld  a,"-"
    ld  (SLCAD+1),a
    ld  a,b
    and %00001100   ; secondary
    sra a
    sra a
    add "0"
    ld  (SLCAD+2),a
OKSLCAD:
    ld  a,(PARTABLE)    ;Get the ID and convert it to string
    ld  b,-1
IDLOOP:
    inc b   ;The ID is encoded weirdly
    srl a
    jr  nc,IDLOOP
    ld  a,b
    ld  (ID),a
    add "0"
    ld  (IDS),a
    call FORMAT_MMC_CID

    ld  de,DEVICE_NFO ; Displays Slot, ID & Partition number+start+size
    call    PRINT

    ; Sweep all partitions
    ld  a,1
    ld  (P_PRIM),a
    xor a
    ld  (P_EXT),a

SWEEP:
    ld  a,(ID)  ;Obtain partition data
    ld  d,a
    ld  a,(P_EXT)
    ld  b,a
    ld  a,(P_PRIM)
    call    GET_PART
    jr  nc,OKGET
    cp  MEGASCSI_ERR_PARTITION_DOES_NOT_EXIST
    jpne    MEGAERR

NEXT_PRIMARY:
    ld  a,(P_PRIM)  ;Error? There are no more extended partitions
    cp  MAX_PRIMARY_PARTITIONS  ;We move on to the next primary
    jpeq PNOTF   ;If it doesn't exist, we haven't found it
    inc a
    ld  (P_PRIM),a
    xor a
    ld  (P_EXT),a
    jr  SWEEP

OKGET:
    or  a   ;Type 0? Unused: Move to Next
    jr  z,NEXT_PRIMARY
    cp  PARTITION_TYPE_EXT_CHS
    jreq FIRST_EXTENDED
    cp  PARTITION_TYPE_EXT_LBA
    jrne    OKGET2
FIRST_EXTENDED:
    ld  a,1
    ld  (P_EXT),a
    jr  SWEEP

OKGET2:
    ex af,af ; store partition type in A'
    ld  ix,FIRST_SEC    ;Compare the initial sector with the one in the MegaSD table
    ld  a,c
    cp  (ix)
    jrne    SWEEP_NEXT
    ld  a,d
    cp  (ix+1)
    jrne    SWEEP_NEXT
    ld  a,e
    cp  (ix+2)
    jreq FOUNDPAR

SWEEP_NEXT:
    ld  a,(P_EXT)   ;If it doesn't match, examine next, Primary or Extended
    or  a
    jr  z,NEXT_PRIMARY
    inc a
    ld  (P_EXT),a
    jr  SWEEP

PNOTF:
    ld  de,PNOTFS   ;None of them match?
    call    PRINT   ;Show error and return
    JP EXIT_NOERR

FOUNDPAR:   ;Found? Show
    push    bc ; C-DE = First sector of partition
    push    hl ; B-HL = Partition size in sectors
    push    hl
    push    bc
    push    de
    push    bc
    ld  a,(P_PRIM)
    add "0"
    ld  (P_PRIMS),a ;set primary partition string text
    ld  a,(P_EXT)
    ld  e,a
    ld  a,"-"
    ld  (P_PRIMS+1),a
    ld  d,0
    xor a
    ld  b,3
    ld  c,0
    ld  hl,P_PRIMS+2
    call    NUMTOASC    ;set extended partition string text
FOUNDP2:
    ld  de,CURRPARS
    call    PRINT

    ld  a,(P_PRIM)  ;Print absolute number if the primary partition is 2 and is extended
    cp  2
    jr  nz,NOSHOWABS
    ld  a,(P_EXT)
    or  a
    jr  z,NOSHOWABS
    ld  a,(P_EXT)
    ld  e,a
    ld  d,0
    inc de
    xor a
    ld  b,3 ; max 3 chars
    ld  c,0
    ld  hl,P_ABSS
    call    NUMTOASC
    ld  de,ABSPS
    call    PRINT
NOSHOWABS:

    ; print partition start sector
    pop de  ; DE = hi Byte of First sector of partition
    ld  d,0
    ld  c,'0' ; '0'-left-padded
    ld  a,1 ; hex
    ld  b,2 ; 2 hex chars
    ld  hl,ST_SEC_TXT
    call    NUMTOASC
    inc hl
    inc hl
    ld  a,#81 ; hex, 0-padded
    ld  b,4 ; 4 hex chars
    pop de  ; DE = lo Byte of First sector of partition
    call    NUMTOASC
    ld  de,ST_SEC
    call    PRINT

    ; print partition length
    pop de  ; DE = hi Byte of partition size
    ld  e,d
    ld  d,0
    ld  c,'0'
    ld  a,1
    ld  b,2
    ld  hl,P_LEN_TXT
    call    NUMTOASC
    inc hl
    inc hl
    ld  a,#81 ; hex, 0-padded
    ld  b,4
    pop de  ; DE = lo Byte of partition size
    call    NUMTOASC
    ld  de,P_LEN
    call    PRINT

    ; print partition type
    ex af,af ; restore partition type into A
    ld d,0
    ld e,a
    ld  a, #01 ; hex, unpadded
    ld  b,2
    ld hl,P_TYPE_TXT
    call    NUMTOASC
    ld  de,P_TYPE
    call    PRINT

    ; Compare drive vs. partition size; show warning if they are not the same
    pop hl
    pop bc
    ld  ix,NUM_SECS
    ld  a,b
    cp  (ix)
    jrne    SIZEWARN
    ld  a,h
    cp  (ix+1)
    jrne    SIZEWARN
    ld  a,l
    cp  (ix+2)
    jreq OKSIZE
SIZEWARN:
    ld  de,SIZEWARNS
    call    PRINT
OKSIZE:
    JP EXIT_NOERR


;;;;;;;;;;;;;;
;;; ACTION: Set the number of MegaSD drives
;;; input:
;;;   DRIVE_COUNT
;;;   SLOT
;;;;;;;;;;;;;;
SET_DRIVE_COUNT:
    ld e,MEGASD_DRIVE_COUNT_BANK_A      ; update #3F80
    ld ix,MEGASD_DRIVE_COUNT_ADDRESS
    ld a,(DRIVE_COUNT)
    ld c,a
    call POKE_ESERAM
    ld e,MEGASD_DRIVE_COUNT_BANK_B      ; update #FF80
    ld a,(DRIVE_COUNT)
    ld c,a
    call POKE_ESERAM
    ld de,RESETS
    JP. PRINT


;;;;;;;;;;;;;;
;;; ACTION: Map specified partition to selected drive
;;; input:
;;;   DRIVE
;;;   ID
;;;   P_PRIM
;;;   P_EXT
;;;;;;;;;;;;;;
MAP_PARTITION:
    ; cvt ID# to bit pattern
    ld  a,(ID)
    ld  b,a
    inc b
    ld  a,%10000000
ID2LOOP:
    rlc a
    djnz    ID2LOOP
    ld  (PARTABLE),a

    ; Get data from the partition to be set
    ld  a,(ID)
    ld  d,a
    ld  a,(P_EXT)
    ld  b,a
    ld  a,(P_PRIM)
    call    GET_PART    ;Obtaining the table
    jr  nc,OKGP2
    cp  MEGASCSI_ERR_PARTITION_DOES_NOT_EXIST
    jpne    MEGAERR

    ld  de,PARNOEXS ;Partition doesn't exist? Error.
    jp  FERR

OKGP2:
    exx
    or  a
    ld  de,NOTUSEDS ;Partition unused? Error.
    jp  z,FERR
    cp  PARTITION_TYPE_EXT_CHS
    ld  de,ISEXTS   ;Partition is primary entry of an extended one?
    jpeq FERR    ;Error.
    cp  PARTITION_TYPE_EXT_LBA
    jpeq FERR

    ; Constructing the New Table

    ld  ix,PARTABLE
    exx
    ld  (ix+2),c    ;First sector
    ld  (ix+3),d
    ld  (ix+4),e
    ld  (ix+5),b    ;Sector count
    ld  (ix+6),h
    ld  (ix+7),l
    ld  a,2         ;Sector size fixed #0200
    ld  (ix+8),a
    xor a           ;fill the remaining 7 bytes with zeros
    ld  (ix+9),a
    ld  hl,PARTABLE+10
    ld  de,PARTABLE+11
    ld  (hl),a
    ld  bc,5
    ldir
    res 7,(ix+1)    ;To take on a disc change

    ; Setting the Table

    ld  a,(DRIVE)
    ld  c,a
    ld  a,#85
    ld  hl,PARTABLE
    call    MEGA
    jp  c,MEGAERR

    ; all done!
    JP. EXIT_NOERR


;******************************************************************************
;*                                                                            *
;*             SUBROUTINES                                                    *
;*                                                                            *
;******************************************************************************

; Terminate with failure? Then show usage.
FERRSHOW:
    call    FERR
SHOWUSE:
    ld  de,USAGE
    call    PRINT
    ld b, ERR_USAGE
    JP EXIT_WITH_ERROR
FERR:
    push    de
    ld  de,ERRORS
    call    PRINT
    pop de
    call    PRINT
    ld b, ERR_INPUT
    JP EXIT_WITH_ERROR

EXIT_NOERR:
    ld b,0
    ; (fall thru)
; B must contain error code
EXIT_WITH_ERROR:
    ; first check if we can return an exit code (DOS2-only)
    push bc
    ld  c,_DOSVER
    call    _DOS
    or  a
    pop hl
    ret nz ; no DOS2; can't use _TERM
    ld  a,b
    cp  DOS_KERNEL_MAJOR_V2
    jpge TERM
    ret

TERM:
    ld b,h
    ld c, _TERM
    call _DOS ; should not return
    ret       ; .. but it can happen when a user abort routine forces this!


; Terminating with error returned by the MegaSD
MEGAERR:
    ld  hl,MEGAERT
    ld  c,a
    push af
ERRTLOOP:
    ld  a,(hl)
    or  a   ;FF? End of table, unknown type.
    jr  z,NOFERR
    cp  c
    jreq FERRM
    ld  de,LONERMG+1
    add hl,de
    jr  ERRTLOOP

NOFERR:
    ld  e,c
    ld  d,0
    ld  hl,ERRBYT+1
    ld  b,2
    ld  c,"0"
    ld  a,%00000001
    call    NUMTOASC
    ld  hl,NOFES    ;set string, or "unknown"
    jr  SETERRS
FERRM:
    inc hl
SETERRS:
    ld  de,MEGAERS2
    ld  bc,LONERMG
    ldir

    ld  de,MEGAERS
    call    PRINT

    pop af
    add ERR_MEGASCSI_BASE
    ld b,a
    JP EXIT_WITH_ERROR


;--- DETECT_MEGASD
;    Detect MegaSD & load selected DRIVE's partition data
;
;    Input:   DRIVE=0-7 for drive A~H:
;    Output:  (SLOT) = MegaSD slot
;    Changes:  All registers
DETECT_MEGASD:
    ld  a,(DRIVE)   ;Transforming the Logical Unit into Physical Controller
    inc a
    call    BUSCONT
    ld  de,DRNOEXS
    jp  c,FERR  ;Error if the specified drive does not exist
    ld  (DRIVE),a
    ld  a,b ;DRIVE and SLOT now refer to the controller of the specified drive
    ld  (SLOT),a

    call    CHK_SLOT    ;Check that the resulting slot is MegaSD
    jr  nc,HAVEMEGA3
    pop de ; drop return address
    ld  de,ERRNMS2
    jp  FERR

HAVEMEGA3:
    ld  a,(DRIVE)   ;Get Partition data
    ld  c,a
    ld  a,MEGASCSI_LOAD_DOS_PARTITION
    ld  hl,PARTABLE
    jp    MEGA

;--- GET_PART
;    Obtain the Starting Sector, Size, and Type of a Partition from disk
;
;    Input:   A = primary partition number, starting at 1
;             B = extended partition number (0=unextended)
;             D = device number, 0 - 7
;             (SLOT) = MegaSD slot
;    Output:  C-DE = First sector of partition
;             B-HL = Partition size in sectors
;             A = Error (if Cy = 1):
;               0: Write-protected (this should never happen)
;               2: Not ready
;               4: Data transfer error
;               8: Reservation conflict
;              12: Other error/Arbitration error
;              16: Format error
;              17: There is no MegaSD in this slot
;              18: Partition does not exist
;              19: ID incorrect
;             A = Partition type (if Cy=0)
;               If the partition is extended (A=5), B-HL may not be valid
;               If the partition is not extended, B must be 0
;    Changes:  All registers
GET_PART:
    ld  (PREV_STACK),sp
    ld  (PAR_PRIM),a
    ld  a,b
    ld  (PAR_EXT),a
    xor a
    ld  (RESTORE_TABLE),a
    ld  a,d
    ld  (IDN),a
    and %11111000
    ld  a,19    ; ID incorrect
    jp  nz,FIN_ERROR

    ;--- Is there a MegaSD in the selected slot?
    call    CHK_SLOT
    ld  a,MEGASCSI_NOT_FOUND
    jp  c,FIN_ERROR

    ;--- We save the extended partition 15 table entry, and set it as the entire disk
    ld  a,MEGASCSI_LOAD_EXTRA_PARTITION
    ld  c,MEGASCSI_LAST_EXTRA_PARTITION
    ld  hl,OLD_TABLE
    call    MEGA
    jp  c,FIN_ERROR

    ;set first bit of the new table based on the input
    ld  a,(IDN)
    ld  b,a
    inc b
    ld  a,%10000000
SETID_LOOP:
    rlca
    djnz    SETID_LOOP
    ld  (NEW_TABLE),a

    ld  a,MEGASCSI_STORE_EXTRA_PARTITION
    ld  c,MEGASCSI_LAST_EXTRA_PARTITION
    ld  hl,NEW_TABLE
    call    MEGA
    jp  c,FIN_ERROR
    ld  a,TRUE ; signal to restore ExPart 15 on exit
    ld  (RESTORE_TABLE),a

    ;--- read primary partition table
    xor a
    ld  (SEC_INI),a
    ld  (SEC_INI+1),a
    ld  (SEC_INI+2),a

    ld  a,(PAR_PRIM)
    or  a
    ld  a,MEGASCSI_ERR_PARTITION_DOES_NOT_EXIST
    jp  z,FIN_ERROR ;Error if indicated partition > 4 or = 0
    ld  a,(PAR_PRIM)
    cp  MAX_PRIMARY_PARTITIONS+1
    ld  a,MEGASCSI_ERR_PARTITION_DOES_NOT_EXIST
    jp  nc,FIN_ERROR

    ld  a,(PAR_PRIM)
    ld  b,a
    ld  a,-16
MUL16LOOP:
    add 16
    djnz    MUL16LOOP
    ld  c,a
    ld  b,0
    ld  hl,MBR_ENTRY_1
    add hl,bc   ;HL = start position of partition

    ld  (TABLE_START),hl
    ld  a,MEGASCSI_READ_SECTOR_EXPAR+MEGASCSI_LAST_EXTRA_PARTITION
    ld  bc,#0100 ; B=1 sector to read / C-DE=sector 0
    ld  de,0
    ld  hl,SECTOR
    or  a
    call    MEGA
    jp  c,FIN_ERROR

    ld  ix,(TABLE_START)
    ld  bc,SECTOR
    add ix,bc   ;IX = partition table start

    ld  a,(ix+4)
    cp  PARTITION_TYPE_EXT_CHS
    jr  z,CHECK_IS_EXT
    cp  PARTITION_TYPE_EXT_LBA
    jr  nz,IS_PRIMARY

CHECK_IS_EXT:
    ld  a,(PAR_EXT)
    or  a
    jr  nz,IS_EXT

IS_PRIMARY:
    ld  a,(PAR_EXT) ;Error if the partition is not extended and B!=0 is indicated at the input
    or  a
    ld  a,MEGASCSI_ERR_PARTITION_DOES_NOT_EXIST
    jp  nz,FIN_ERROR

    call    SET_REG ;Establishes C-DE and B-HL from IX

    ld  a,(ix+4)
    jp  FIN_OK

    ;--- read loop for secondary table

IS_EXT:
    ld  a,1
    ld  (CURR_PAR),a

    ld  a,(ix+8)
    ld  (SEC_INI),a ;Establish the initial sector, which will be the basis to be added to the "first sector" field of the extended tables
    ld  a,(ix+9)
    ld  (SEC_INI+1),a
    ld  a,(ix+10)
    ld  (SEC_INI+2),a

    ld  ix,SEC_INI  ;We read the sector that contains the new partition table
    ld  c,(ix+2)
    ld  d,(ix+1)
    ld  e,(ix)
EXT_LOOP:
    ld  b,1
    ld  hl,SECTOR
    ld  a,MEGASCSI_READ_SECTOR_EXPAR+MEGASCSI_LAST_EXTRA_PARTITION
    or  a
    push    bc
    push    de
    call    MEGA
    pop de
    pop bc
    jp  c,FIN_ERROR

    ld  a,(PAR_EXT) ;check if this is the partition we want
    ld  h,a
    ld  a,(CURR_PAR)
    cp  h
    jr  nz,NEXT

    ld  ix,SECTOR+MBR_ENTRY_1   ;IX = Start of first partition table
    call    SET_REG2    ;B-HL = length (C-DE already filled)
    ld  a,(ix+8)
    add e   ;Add to C-DE the value of the "first sector" of the table
    ld  e,a
    ld  a,(ix+9)
    adc d
    ld  d,a
    ld  a,(ix+10)
    adc c
    ld  c,a
    ld  a,(ix+4)
    jp  FIN_OK

NEXT:
    ld  ix,SECTOR+MBR_ENTRY_2   ;Error if you search for the next extended partition but it doesn't exist
    ld  a,(ix+4)
    or  a
    ld  a,MEGASCSI_ERR_PARTITION_DOES_NOT_EXIST
    jp  z,FIN_ERROR
    ld  a,(CURR_PAR)
    inc a
    ld  (CURR_PAR),a
    call    SET_REG
    ld  a,(SEC_INI)
    add e
    ld  e,a
    ld  a,(SEC_INI+1)
    adc d
    ld  d,a
    ld  a,(SEC_INI+2)
    adc c
    ld  c,a
    jp  EXT_LOOP

;--- Return with or without error
FIN_ERROR:
    call    FIN
    scf
    jr  FIN2

FIN_OK:
    call    FIN
    or  a
    jr  FIN2

FIN2:
    ld  sp,(PREV_STACK)
    ret

FIN:
    push    bc
    ld  b,a
    ld  a,(RESTORE_TABLE)
    inc a
    ld  a,b
    pop bc
    ret nz

    ; restore ExPart 15
    push    af
    push    bc
    push    de
    push    hl
    ld  a,MEGASCSI_STORE_EXTRA_PARTITION
    ld  c,MEGASCSI_LAST_EXTRA_PARTITION
    ld  hl,OLD_TABLE
    call    MEGA
    pop hl
    pop de
    pop bc
    pop af
    ret

;--- Loading C-DE and B-HL for output.
;    IX should point to the beginning of the partition table
SET_REG:
    ld  e,(ix+8)
    ld  d,(ix+9)
    ld  c,(ix+10)
SET_REG2:
    ld  l,(ix+12)
    ld  h,(ix+13)
    ld  b,(ix+14)
    ret

;--- Data
IDN:        db  0
PAR_PRIM:   db  0
PAR_EXT:    db  0
CURR_PAR:   db  0
PREV_STACK: dw  0
TABLE_START: dw  0
SEC_INI:    ds  3
RESTORE_TABLE:  db  0   ;If 'TRUE': restore virtual disk 15 before finishing
OLD_TABLE:  ds  16
NEW_TABLE:  db  0           ;device Id
            db  0           ;flags
            db  0,0,0       ;Start sector
            db  #FF,#FF,#FF ;Length (entire disk)
            db  #02,#00     ;Sector size (fixed 512B)
            ds  6


;--- Checking for the presence of a MegaSD in a slot
;    Input:    (SLOT) = Slot to test
;    Output:   Cy = 0 if there is a MegaSD
;    Changes: AF
CHK_SLOT:
    ld  a,(SLOT)
    push    bc
    push    de
    push    hl
    call    SRCHMEGA
    pop hl
    pop de
    pop bc
    ccf
    ret

SRCHMEGA:
    ld  b,16 ; 16 chars to compare
    ld  c,a
    ld  de,MEGASCSI_SIGNATURE_ADDRESS
    ld  hl,MEGASD_SIGNATURE

SMS_LOOP:
    push    hl
    push    de
    push    bc
    ld  a,c
    ex  de,hl
    call    RDSLT
    pop bc
    pop de
    pop hl
    xor (hl)
    ret nz
    inc de
    inc hl
    djnz    SMS_LOOP
    scf
    ret

MEGASD_SIGNATURE:   db  "MEGASCSI ver2.15" ; with version number to distinguish MegaSD from MegaSCSI


;--- Call a MegaSD Function
;    Input: (SLOT) = MegaSD slot
;             Registers: depends on the function. Always preserves IX,IY.
;    Output:  Depends on the function
MEGA:
    push    ix
    push    iy
    ld  iy,(SLOT-1)
    ld  ix,MEGASCSI_API_ENTRY
    call    CALSLT
    pop iy
    pop ix
    ret


;--- write a Byte into ESE-RAM via bank 0
;    Input:     (SLOT) = MegaSD slot
;               E=A8 bank (#80-#87 = MegaSD writable)
;               IX=address to write (#4000-#5FFF)
;               C=value to write
;    Output:    -
;    Registers: AF,BC,DE,HL
POKE_ESERAM:
    push bc
    ld a,(SLOT)
    ld hl,A8k_PAGE0_SWITCH_ADDRESS
    call WRSLT

    ld a,(SLOT)
    push ix
    pop hl
    pop de
    call WRSLT

    ld a,(SLOT)
    ld hl,A8k_PAGE0_SWITCH_ADDRESS
    ld e,MEGASD_DEFAULT_BANK
    jp WRSLT


;--- retrieve MMC/SD Card Identification (CID)
; Linux: `cat /sys/block/mmcblk0/device/cid`
; see https://www.memorycard-lab.com/-Article/SDcard-CID-Decoder
; in: (SLOT) = MegaSD slot
; affects: AF, BC, DE, HL, IX
FORMAT_MMC_CID:
    ; set A8 bank 0 to #40 - MMC
    ld a,(SLOT)
    ld hl,A8k_PAGE0_SWITCH_ADDRESS
    ld e,MEGASD_MMC_BANK
    call WRSLT
    ; write 6 command Bytes
    ld B,MMC_CMD_LEN
    ld ix,MMC_SEND_CID
    ld hl,MEGASD_MMC_ADDRESS
CMDLOOP:
    push bc
    ld a,(SLOT)
    ld e,(ix)
    call WRSLT
    inc ix
    pop bc
    djnz CMDLOOP

    ; wait for response header
R2HEADERLOOP:
    ld a,(SLOT)
    call RDSLT
    cp MMC_START_BLOCK_TOKEN
    jrne R2HEADERLOOP

    ; read 16 response Bytes
    ld B,MMC_R2_RESPONSE_DATA_LEN
    ld ix,SECTOR
R2LOOP:
    ld a,(SLOT)
    push bc
    call RDSLT
    ld (ix),a
    inc ix
    pop bc
    djnz R2LOOP
    ; restore A8 bank 0
    ld a,(SLOT)
    ld hl,A8k_PAGE0_SWITCH_ADDRESS
    ld e,MEGASD_DEFAULT_BANK
    call WRSLT
    ;-- https://web.archive.org/web/20140710011317/https://www.sdcard.org/downloads/pls/simplified_specs/part1_410.pdf, section 4.9.3 & 5.2
    ; format manufacturer (MID)
    ld ix,SECTOR
    ld d,0
    ld e,(ix)
    ld hl,CID_MANUFACTURER_CODE
    ld b,2
    ld c,'0'
    ld a,#01
    call NUMTOASC
    ; format OEM app ID (OID) - note that the assigned ids are not disclosed by SD-3C
    ld a,(ix+1)
    ld (CID_OID+0),a
    ld a,(ix+2)
    ld (CID_OID+1),a
    ; format product name
    ld a,(ix+3)
    ld (CID_PNM+0),a
    ld a,(ix+4)
    ld (CID_PNM+1),a
    ld a,(ix+5)
    ld (CID_PNM+2),a
    ld a,(ix+6)
    ld (CID_PNM+3),a
    ld a,(ix+7)
    ld (CID_PNM+4),a
    ; format product revision - 2 BCD nibbles
    ld a, (ix+8)
    sra a
    sra a
    sra a
    sra a
    add "0"
    ld (CID_PRV), a
    ld a, (ix+8)
    and #0f
    add "0"
    ld (CID_PRV+2), a
    ; format serial no
    ld hl, CID_PSN
    ld d,(ix+9)
    ld e,(ix+10)
    ld b,4
    ld a,#81
    call NUMTOASC
    ld hl, CID_PSN+4
    ld d,(ix+11)
    ld e,(ix+12)
    ld b,4
    ld a,#81
    call NUMTOASC
    ; (we skip formatting manufacturing date & validating CRC7)
    ret


;--- print a string
; in: DE=string, terminated with "$"
; does not print when QUIET!=0
; affects: flags
PRINT:
    push bc
    ld c,a
    ld a,(QUIET)
    or a
    ld a,c
    pop bc
    ret nz

    push bc
    ld  c,_STROUT
    call _DOS
    pop bc
    ret


;--- Name: EXTPAR
;    Extracting a parameter from the command line
;    Input:     A  = Parameter to extract (starts at 1)
;               DE = Buffer to store the parameter
;    Output:    A  = parameter count (or number?)
;               CY = 1 -> parameter does not exist
;                         B undefined, buffer unchanged
;               CY = 0 -> B = parameter length (never 0)
;                         Parameter stored in DE, 0-terminated
;    REGISTERS: -
;    CALLS:  -
;    VARIABLES: Macros JR
EXTPAR:
    or  a   ;We return with error if A = 0
    scf
    ret z

    ld  b,a
    ld  a,(DOS2_CMDLINE)    ;We return with error if there are no parameters
    or  a
    scf
    ret z
    ld  a,b

    push    hl
    push    de
    push    ix
    ld  ix,0    ;IXl: parameter number
    ld  ixh,a   ;IXh: parameter to extract
    ld  hl,DOS2_CMDLINE+1

PASASPC:
    ld  a,(hl)  ;Skip spaces
    or  a
    jr  z,ENDPNUM
    cp  " "
    inc hl
    jreq PASASPC

    inc ix
PASAPAR:
    ld  a,(hl)  ;Skip parameter
    or  a
    jr  z,ENDPNUM
    cp  " "
    inc hl
    jreq PASASPC
    jr  PASAPAR

ENDPNUM:
    ld  a,ixh   ;Error if the parameter to be extracted is greater than the number of existing parameters
    cp  ixl
    jrgt    EXTPERR

    ld  hl,DOS2_CMDLINE+1
    ld  b,1 ;B = current parameter
PASAP2:
    ld  a,(hl)  ;Skip spaces until we find the next parameter
    cp  " "
    inc hl
    jreq PASAP2

    ld  a,ixh   ;If it's the one we're looking for, we'll extract it.
    cp  B   ;if not ...
    jreq PUTINDE0

    inc B
PASAP3:
    ld  a,(hl)  ;... we pass it and go back to PAPAP2
    cp  " "
    inc hl
    jrne    PASAP3
    jr  PASAP2

PUTINDE0:
    ld  b,0
    dec hl
PUTINDE:
    inc b
    ld  a,(hl)
    cp  " "
    jreq ENDPUT
    or  a
    jr  z,ENDPUT
    ld  (de),a  ;Store parameter at (DE)
    inc de
    inc hl
    jr  PUTINDE

ENDPUT:
    xor a
    ld  (de),a
    dec b

    ld  a,ixl
    or  a
    jr  FINEXTP
EXTPERR:
    scf
FINEXTP:
    pop ix
    pop de
    pop hl
    ret


;--- NAME: RANGE
;      Verify that a byte is within a range
;    Input:      H = hi value (inclusive)
;                L = lo value (inclusive)
;                A = Byte
;    Output:     Z = 1 when in range (Cy = ?)
;                Cy= 1 if it's above the range (Z = 0)
;                Cy= 0 if it's below the range (Z = 0)
RANGE:
    cp  l   ;Lower?
    ccf
    ret nc

    cp  h   ;Higher?
    jr  z,R_H
    ccf
    ret c

R_H:
    push    bc  ;=H?
    ld  b,a
    xor a
    ld  a,b
    pop bc
    ret


;--- NAME: NUMTOASC
;      Converting a 16-bit integer to a string
;    Input:      DE = Number to convert
;                HL = Buffer to place string
;                B  = Total number of characters in the string not including terminating characters
;                C  = Fill Character
;                     The number is right-justified, and the extra spaces are filled with the character (C).
;                     If the resulting number takes up more characters than those indicated in B, this record is ignored and the string takes up the required characters.
;                     The ending character, "$" or 00, is not counted for length purposes.
;                 A = &B ZPRFFTTT
;                     TTT = resultant number format
;                            0: decimal
;                            1: hexdecimal
;                            2: hexadecimal, starts with "&H"
;                            3: hexadecimal, starts with "#"
;                            4: hexadecimal, ends with "H"
;                            5: binary
;                            6: binary, starts with "&B"
;                            7: binary, ends with "B"
;                     R   = number range
;                            0: 0..65535 (unsigned)
;                            1: -32768..32767 (two's complement)
;                               If the output format is binary, the number is interpreted as an 8-bit integer and the range is 0..255.
;                               That is, the R bit and the D register are ignored.
;                     FF  = terminating character
;                            0: none
;                            1: add "$"
;                            2: add 00
;                            3: Sets the 7th bit of the last character to 1
;                     P   = "+" sign
;                            0: Do not add a "+" sign to positive numbers
;                            1: Add a "+" sign to positive numbers
;                     Z   = Leading zeros
;                            0: Remove Leading Zeros
;                            1: Do Not Remove Leading Zeros
;    Output:    String at (HL)
;               B = Number of characters in the string that make up the number, including the sign and type indicator if generated
;               C = Number of total characters in the string not counting the "$" or 00 if generated
;    REGISTERS: -
;    CALLS:  -
;    VARIABLES: -
NUMTOASC:
    push    af
    push    ix
    push    de
    push    hl
    ld  ix,WorkNTOA
    push    af
    push    af
    and %00000111
    ld  (ix+0),a    ;Type
    pop af
    and %00011000
    rrca
    rrca
    rrca
    ld  (ix+1),a    ;End
    pop af
    and %11100000
    rlca
    rlca
    rlca
    ld  (ix+6),a    ;Flags: Z(zero), P(+), R(range)
    ld  (ix+2),b    ;# final characters
    ld  (ix+3),c    ;Fill Character
    xor a
    ld  (ix+4),a    ;Length total
    ld  (ix+5),a    ;number length
    ld  a,10
    ld  (ix+7),a    ;Divide by 10
    ld  (ix+13),l   ;Buffer passed by the user
    ld  (ix+14),h
    ld  hl,BufNTOA
    ld  (ix+10),l   ;Routine Buffer
    ld  (ix+11),h

ChkTipo:
    ld  a,(ix+0)    ;Divide by 2 or 16, or leave at 10
    or  a
    jr  z,ChkBoH
    cp  5
    jp  nc,EsBin
EsHexa:
    ld  a,16
    jr  GTipo
EsBin:
    ld  a,2
    ld  d,0
    res 0,(ix+6)    ;If it's binary, it's between 0 and 255
GTipo:
    ld  (ix+7),a

ChkBoH:
    ld  a,(ix+0)    ;Check if you need to put "H" or "B"
    cp  7   ;al final
    jp  z,PonB
    cp  4
    jr  nz,ChkTip2
PonH:
    ld  a,"H"
    jr  PonHoB
PonB:
    ld  a,"B"
PonHoB:
    ld  (hl),a
    inc hl
    inc (ix+4)
    inc (ix+5)

ChkTip2:
    ld  a,d ;If the number is 0, it is never signed
    or  e
    jr  z,NoSgn
    bit 0,(ix+6)    ;Check Range
    jr  z,SgnPos
ChkSgn:
    bit 7,d
    jr  z,SgnPos
SgnNeg:
    push    hl  ;Negate number
    ld  hl,0    ;Sign=0:no sign; 1:+; 2:-
    xor a
    sbc hl,de
    ex  de,hl
    pop hl
    ld  a,2
    jr  FinSgn
SgnPos:
    bit 1,(ix+6)
    jr  z,NoSgn
    ld  a,1
    jr  FinSgn
NoSgn:
    xor a
FinSgn:
    ld  (ix+12),a

ChkDoH:
    ld  b,4
    xor a
    cp  (ix+0)
    jp  z,EsDec
    ld  a,4
    cp  (ix+0)
    jp  nc,EsHexa2
EsBin2:
    ld  b,8
    jr  EsHexa2
EsDec:
    ld  b,5

EsHexa2:
    push    de
Divide:
    push    bc
    push hl ;DE/(IX+7)=DE, rest A
    ld  a,d
    ld  c,e
    ld  d,0
    ld  e,(ix+7)
    ld  hl,0
    ld  b,16
DivLoop:
    rl  c
    rla
    adc hl,hl
    sbc hl,de
    jr  nc,$+3
    add hl,de
    ccf
    djnz    DivLoop
    rl  c
    rla
    ld  d,a
    ld  e,c
    ld  a,l
    pop hl
    pop bc

ChkRest9:
    cp  10  ;Convert the rest to character
    jp  nc,EsMay9
EsMen9:
    add a,"0"
    jr  PonEnBuf
EsMay9:
    sub 10
    add a,"A"

PonEnBuf:
    ld  (hl),a  ;Put character in buffer
    inc hl
    inc (ix+4)
    inc (ix+5)
    djnz    Divide
    pop de

ChkECros:
    bit 2,(ix+6)    ;Check if zeros need to be removed
    jr  nz,ChkAmp
    dec hl
    ld  b,(ix+5)
    dec b   ;B=# digits to check
Chk1Cro:
    ld  a,(hl)
    cp  "0"
    jr  nz,FinECeros
    dec hl
    dec (ix+4)
    dec (ix+5)
    djnz    Chk1Cro
FinECeros:  inc hl

ChkAmp:
    ld  a,(ix+0)    ;Insert "#", "&H" or "&B" if necessary
    cp  2
    jr  z,PonAmpH
    cp  3
    jr  z,PonAlm
    cp  6
    jr  nz,PonSgn
PonAmpB:
    ld  a,"B"
    jr  PonAmpHB
PonAlm:
    ld  a,"#"
    ld  (hl),a
    inc hl
    inc (ix+4)
    inc (ix+5)
    jr  PonSgn
PonAmpH:
    ld  a,"H"
PonAmpHB:
    ld  (hl),a
    inc hl
    ld  a,"&"
    ld  (hl),a
    inc hl
    inc (ix+4)
    inc (ix+4)
    inc (ix+5)
    inc (ix+5)

PonSgn:
    ld  a,(ix+12)   ;Insert sign
    or  a
    jr  z,ChkLon
SgnTipo:
    cp  1
    jr  nz,PonNeg
PonPos:
    ld  a,"+"
    jr  PonPoN
    jr  ChkLon
PonNeg:
    ld  a,"-"
PonPoN
    ld  (hl),a
    inc hl
    inc (ix+4)
    inc (ix+5)

ChkLon:
    ld  a,(ix+2)    ;Put fill characters if necessary
    cp  (ix+4)
    jp  c,Invert
    jr  z,Invert
PonCars:
    sub (ix+4)
    ld  b,a
    ld  a,(ix+3)
Pon1Car:
    ld  (hl),a
    inc hl
    inc (ix+4)
    djnz    Pon1Car

Invert:
    ld  l,(ix+10)
    ld  h,(ix+11)
    xor a   ;Reverse string
    push    hl
    ld  (ix+8),a
    ld  a,(ix+4)
    dec a
    ld  e,a
    ld  d,0
    add hl,de
    ex  de,hl
    pop hl  ;HL=buffer start, DE=buffer end
    ld  a,(ix+4)
    srl a
    ld  b,a
InvLoop:
    push    bc
    ld  a,(de)
    ld  b,(hl)
    ex  de,hl
    ld  (de),a
    ld  (hl),b
    ex  de,hl
    inc hl
    dec de
    pop bc
    djnz    InvLoop
ToBufUs:
    ld  l,(ix+10)
    ld  h,(ix+11)
    ld  e,(ix+13)
    ld  d,(ix+14)
    ld  c,(ix+4)
    ld  b,0
    ldir
    ex  de,hl

ChkFin1:
    ld  a,(ix+1)    ;Check if it should end in "$" or 0
    and %00000111
    or  a
    jr  z,Fin
    cp  1
    jr  z,PonDolar
    cp  2
    jr  z,PonChr0

PonBit7:
    dec hl
    ld  a,(hl)
    or  %10000000
    ld  (hl),a
    jr  Fin

PonChr0:
    xor a
    jr  PonDo0
PonDolar:
    ld  a,"$"
PonDo0:
    ld  (hl),a
    inc (ix+4)

Fin:
    ld  b,(ix+5)
    ld  c,(ix+4)
    pop hl
    pop de
    pop ix
    pop af
    ret

WorkNTOA:   defs    16
BufNTOA:    ds  10


;--- NAME: EXTNUM
;      Extracting a 5-digit number stored in ASCII format
;    Input:      HL = start of ASCII string
;    Output:     CY-BC = 17 bits number
;                D  = Number of digits that make up the number
;                     The number is considered extracted when a non-numeric character is found, or when five digits have been extracted.
;                E  = Incorrect first character (or sixth digit)
;                A  = error:
;                     0 => No error
;                     1 => The number is longer than 5 digits.
;                          CY-BC then contains the number formed by the first five digits
;    REGISTERS:  -
;    CALLS:      -
;    VARIABLES:  -
EXTNUM:
    push    hl
    push ix
    ld  ix,ACA
    res 0,(ix)
    set 1,(ix)
    ld  bc,0
    ld  de,0
BUSNUM:
    ld  a,(hl)  ;Jump to FINEXT if the character is not:
    ld  e,a ;IXh = Last character read so far
    cp  "0" ;a number, or if it's the sixth character
    jr  c,FINEXT
    cp  "9"+1
    jr  nc,FINEXT
    ld  a,d
    cp  5
    jr  z,FINEXT
    call    POR10

SUMA:
    push    hl  ;BC = BC + A
    push    bc
    pop hl
    ld  bc,0
    ld  a,e
    sub "0"
    ld  c,a
    add hl,bc
    call    c,BIT17
    push    hl
    pop bc
    pop hl

    inc d
    inc hl
    jr  BUSNUM

BIT17:
    set 0,(ix)
    ret
ACA:    db  0   ;b0: num>65535. b1: more than 5 digits

FINEXT:
    ld  a,e
    cp  "0"
    call    c,NODESB
    cp  "9"+1
    call    nc,NODESB
    ld  a,(ix)
    pop ix
    pop hl
    srl a
    ret

NODESB:
    res 1,(ix)
    ret

POR10:
    push    de
    push    hl  ;BC = BC * 10
    push    bc
    push    bc
    pop hl
    pop de
    ld  b,3
ROTA:
    sla l
    rl  h
    djnz    ROTA
    call    c,BIT17
    add hl,de
    call    c,BIT17
    add hl,de
    call    c,BIT17
    push    hl
    pop bc
    pop hl
    pop de
    ret


;--- NAME: BUSCONT
;      Gets the driver associated with a logical drive
;    Input:     A = Logical Unit (0: default)
;    Output:    Cy= 0: No error
;               Cy= 1: Error: Logical unit does not exist
;               B = controller slot
;               C = Unit with respect to the first one associated with that (or ESE?) controller
;                   (first=0)
BUSCONT:
    or  a
    jr  nz,NODEFD   ;If it's the default drive, find out what it is
    ld  c,_CURDRV
    call _DOS
    inc a

NODEFD:
    push    af
    ld  c,_DOSVER
    call _DOS
    ld  a,b
    cp  2
    jr  c,IS_DOS1    ;If DOS 2, find out the physical unit corresponding to the logic
    pop bc
    ld  d,_ASSIGN_GET
    ld  c,_ASSIGN
    call _DOS
    push    de

IS_DOS1:
    pop af  ;A = Physical unit, either DOS1 or DOS2
    dec a
    ld  b,4
    ld  hl,DRVINF

CONTROLLERLOOP:
    sub (hl)    ;Subtract the number of units of each controller from the physical drive until you get a negative number
    jr  c,CONTFND
    inc hl
    inc hl
    djnz    CONTROLLERLOOP
    xor a
    ld  c,a
    scf     ;Return C=0 and Cy=1 if the unit doesn't exist
    ret

CONTFND:
    add a,(hl)  ;A = Num. of the unit with respect to that controller (the first is 0)
    inc hl
    ld  b,(hl)  ;B = controller slot
    ld  c,a
    or  a   ;Return with Cy=0
    ret


;******************************************************************************
;*                                                                            *
;*             DATA ZONE                                                      *
;*                                                                            *
;******************************************************************************

SLOT:       db  0   ;MegaSD slot
ID:         db  0   ;MegaSD device ID - always 0
P_PRIM:     db  0   ;primary partition number
P_EXT:      db  0   ;extended partition number
DRIVE:      db  0   ;drive 0-7 indicating A:~H:
TYPEPAR:    db  0   ;partition type
ACTION:     db  ACTION_SHOW_USAGE
DRIVE_COUNT: db 0   ;# drives to set (1-8) for ACTION_SET_DRIVE_COUNT

; flags based on cmdline options
QUIET:          db FALSE

ERRORS: db  "*** ERROR: $"
PRESEN: db  "MSDPAR - Partition tool for OCM/MSX++ MegaSD v1.0d",13,10
        db  "(C) 2024 Cayce-MSX, based on PARSET by Konamiman.",13,10
        db  "This program comes with ABSOLUTELY NO WARRANTY.",13,10
        db  "It's free software, you're welcome to redistribute under certain conditions",13,10
        db  10,"$"

USAGE:  db  "Usage: MSDPAR [<drive>]:",13,10
        db  "         Shows number of partition currently connected to the drive",13,10
        db  "         ",34,":",34," selects the default drive",13,10
        db  "       MSDPAR [<drive>:]<abs>|<prim>-<ext>",13,10
        db  "         abs:  Absolute partition number (1 to 256)",13,10
        db  "         prim: Primary partition number (1 to 4)",13,10
        db  "         ext:  Extended partition number (1 to 255)",13,10
        db  "               * If partition is not extended, use ext=0",13,10
        db  "               * Absolute partitions and primary + extended match as follows:",13,10
        db  "                 prim + ext       abs",13,10
        db  "                   1-0             1",13,10
        db  "                   2-1             2",13,10
        db  "                   2-2             3",13,10
        db  "                   2-3             4   etc...",13,10
        db  "                   3-x & 4-x      Can't use absolute number",13,10
        db  "       MSDPAR /L              Show all partitions on the disk",13,10
        db  "       MSDPAR [<drive>:] /Ei  Enable i MegaSD drives (1-8)",13,10
        db "$"
BADPARS:    db  "Invalid partition specification",13,10,10,"$"
BADDRIVS:   db  "Invalid drive specification",13,10,10,"$"
BADDRVCS:   db  "Invalid drive count",13,10,10,"$"
TOOPARS:    db  "Too many parameters",13,10,10,"$"
ERRNMS:     db  "No MegaSD found in the specified slot",13,10,"$"
ERRNMS2:    db  "The specified drive is not controlled by a MegaSD",13,10,"$"
DRNOEXS:    db  "The specified drive does not exist",13,10,"$"
PARNOEXS:   db  "The specified partition does not exist",13,10,"$"
NOTUSEDS:   db  "The specified partition is undefined on this disk",13,10,"$"
ISEXTS:     db  "The specified partition is extended.",13,10
            db  "           Please specify extended partition number",13,10
            db  "           in the range 1-255, or use absolute partition number.",13,10,"$"
PNOTFS:     db  13,10,"WARNING: No valid partition connected to this drive",13,10,"$"
SIZEWARNS:  db  13,10,"WARNING: Drive size and partition size are not equal",13,10,"$"
RESETS:     db  "Please RESET to activate drive change (don't power off!)",13,10,"$"
DEVICE_NFO: db  "Partition mapped to drive "
DRIVES:     db  " :",13,10,10
            db  "MegaSD slot:       "
SLCAD:      db  "   ",13,10
DEVNS:      db  "Device ID:         "
IDS:        db  0,13,10
MMC_CID:    db  "SD Card Id:       #"
CID_MANUFACTURER_CODE:
            ds  2," " ; 27=Philips, 1B=Samsung
            db  " "
CID_OID:    ds  2," " ; PH=Philips, SM=Samsung
CID_PNM:    ds  5," "
            db  " v"
CID_PRV:    db  "_._ serial #"
CID_PSN:    ds  8," "
            db  13,10,"$"
CURRPARS:   db  "Partition number:  "
P_PRIMS:    db  "  "
P_EXTS:     db  "   ",13,10,"$"
ST_SEC:     db  "Start sector:     #"
ST_SEC_TXT: ds  6
            db  13,10,"$"
P_LEN:      db  "Partition length: #"
P_LEN_TXT:  ds  6
            db  13,10,"$"
P_TYPE:     db  "Partition type:   #"
P_TYPE_TXT: ds  2
            db  13,10,"$"
ABSPS:      db  "Absolute partition number: "
P_ABSS:     db  "   ",13,10,"$"
LONERMG:    equ 32
MEGAERS:    db  "*** MegaSD ERROR: "
MEGAERS2:   ds  LONERMG
            db  13,10,"$"
MEGAERT:    db  2,"Device not ready                "
            db  4,"Data transfer error             "
            db  8,"Reservation conflict            "
            db  12,"Other error / arbitration error "
            db  16,"Format error                    "
            db  19,"Invalid ID number               "
            db  0
NOFES:      db  "Unknown error (code "
ERRBYT:     db  "#  )        "
LISTING:    db  "Par. num.        Type (*=unsupported)         First sector         Size",13,10
            db  "---------        --------------------         ------------         ----",13,10,"$"
LONSHOW:    equ 69
PARNS2:     db  "  "
PARNS:      ds  15
TYPES:      ds  29
FIRSS:      ds  18
SIZES:      db  "      B",13,10,"$"
LONTYPE:    equ 24
TABTYPE: ; https://en.wikipedia.org/wiki/Partition_type#List_of_partition_IDs
    db    0," --- unused / empty --- "
    db    1,"MS(X)-DOS, FAT12        " ; floppy & unpatched MSX-DOS
    ;db   2,"XENIX                   "
    ;db   3,"XENIX                   "
    db    4,"MS(X)-DOS, FAT16 <32MiB "
    db    5,"Extended (CHS)          "
    db    6,"MS(X)-DOS, FAT16 >32MiB " ; regular FAT16
    db    7,"exFAT *                 " ; also OS/2 & NTFS
    db  #0b,"W95 FAT32 *             "
    db  #0c,"W95 FAT32 (LBA) *       "
    db  #0e,"W95 FAT16 (LBA)         " ; Nextor since 2.1.2
    db  #0f,"Extended (LBA)          " ; Nextor since 2.1.2
    db  #11,"Hidden FAT12            "
    db  #14,"Hidden FAT16 <32MiB     "
    db  #16,"Hidden FAT16            "
    db  #1b,"Hidden W95 FAT32 *      "
    db  #1c,"Hidden W95 FAT32 *      "
    db  #1e,"Hidden W95 FAT16        "
    ;db #63,"UNIX V                  "
    ;db #64,"Net                     "
    ;db #75,"PC/IX                   "
    ;db #DB,"Concurrent DOS          "
    db  END_OF_LIST
NOFTS:  db  "-- Unknown (byte "
TYPBYT: db  "#  ) * "
; https://web.archive.org/web/20140710011317/https://www.sdcard.org/downloads/pls/simplified_specs/part1_410.pdf, section 4.5 & 4.7.2
MMC_SEND_CID: db #4A,0,0,0,0,%00011011 ; CMD 10d, no args, CRC7 with b0 set

PARTABLE:   ;
DEVICE_ID:  equ PARTABLE+0
MSD_FLAGS:  equ PARTABLE+1   ; bit 7: Disk change flag. It is 0 when a disk change was made but a disk change status was not requested yet.
                             ; bit 6: Write protection bit (1=write protected partition, 0=write enabled partition)
FIRST_SEC:  equ PARTABLE+2
NUM_SECS:   equ PARTABLE+5
SECTOR:     equ PARTABLE+16
; 512 Bytes after this will be used to store a disk sector
