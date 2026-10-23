; SWAPEIDI.COM v1.1 by KdL (2025.06.29)
; =====================================
; This is a useful trick that helps speed up the execution of any batch file.
; 1st execution disables the vertical interrupt by excluding input scanning.
; 2nd execution re-enables the vertical interrupt by restoring input scanning.
;
; ---------------------------------
; Coded in TASM80 v3.2ud w/ TWZ'CA3
; TASM is at https://www.ticalc.org
; ---------------------------------
;

          .ORG  $0100

CALSLT    .EQU  $001c
WRTVDP    .EQU  $012d
REG1SAV   .EQU  $f3e0
EXBRSA    .EQU  $faf8
CSRSW     .EQU  $fca9

startProg:
          ld    a,(REG1SAV)             ; A = Mirror of VDP register 1
          xor   %00100000               ; Toggle bit 5 (vertical interrupt enable flag)
          ld    b,a                     ; B = Value to write (WRTVDP)
          ld    c,$01                   ; C = Register number (WRTVDP)
          ld    iy,(EXBRSA-1)           ; IY = High byte with slot ID (SUB-ROM)
          ld    ix,WRTVDP               ; IX = The address that will be called (WRTVDP)
          call  CALSLT                  ; Inter-slot call

          xor   a                       ; A = 0
          ld    (CSRSW),a               ; Cursor display off

          ret

endProg:
          .DW   $ffff,$ffff,$ffff,$ffff
          .DB   "[ SWAPEIDI.COM ]"

          .END
