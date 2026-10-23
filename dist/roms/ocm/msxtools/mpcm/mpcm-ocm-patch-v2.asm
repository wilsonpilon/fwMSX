; ---------------------------------------------------------
; MPCM-OCM Patch v2.0 by KdL 2022.08.06
; ---------------------------------------------------------
; Coded in Sjasm v0.42c - https://www.xl2s.tk
; ---------------------------------------------------------

_OCMID:     equ     0xD4

_SWIOP:     equ     0x40
_SMCMD:     equ     0x41
_VRDIP:     equ     0x42
_SPEED:     equ     0x47
_TPANA:     equ     0x48

_SC358:     equ     0x03
_SC806:     equ     0x0A
_SCNUL:     equ     0x80

; ---------------------------------------------------------
            org     0x0100

; ---------------------------------------------------------
_begin:
; save CPU speed if exist OCM ID
    call    _save_cpu_speed
; set 8.06MHz if exist OCM ID --> then loading WAV
    call    _set_806_speed
; set 3.58MHz if exist OCM ID --> then playing WAV
    call    _set_358_speed
; restore CPU speed if exist OCM ID
    call    _restore_cpu_speed
; exit to DOS
    ret
_end:

; ---------------------------------------------------------
_check_OCMID:
    ld      a, _OCMID
    out     (_SWIOP), a         ; set OCM ID
    in      a, (_SWIOP)
    cpl
    cp      _OCMID
    ret

; ---------------------------------------------------------
_save_cpu_speed:
    call    _check_OCMID
    jr      nz, _cancel_DEVID   ; OCM is not detected
    in      a, (_VRDIP)
    and     0x01                ; mask bit0
    cp      0x00
    jr      z, _check_357       ; is 3.58MHz or 10.74MHz ?
_save_1074:                     ; save Custom Speed (aka 10.74MHz)
    in      a, (_SPEED)
    and     0x07                ; mask bit2-0
_save_358:                      ; save 3.58MHz
    add     _SC358
_save_537:                      ; save 5.37MHz (Turbo Pana)
    ld      (T80STA), a
_cancel_DEVID:                  ; cancel Device ID
    xor     a
    out     (_SWIOP), a
    ret                         ; exit any call
_check_357:
    in      a, (_TPANA)
    and     0x01                ; mask bit0
    cp      0x00
    jr      z, _save_358        ; is 3.58MHz or 5.37MHz ?
_is_537:
    ld      a, _SMCMD
    jr      _save_537

; ---------------------------------------------------------
_set_806_speed:
    call    _check_OCMID
    jr      nz, _cancel_DEVID   ; OCM is not detected
    ld      a, _SC806
    out     (_SMCMD), a         ; restore CPU speed
    jr      _cancel_DEVID

; ---------------------------------------------------------
_set_358_speed:
    call    _check_OCMID
    jr      nz, _cancel_DEVID   ; OCM is not detected
    ld      a, _SC358
    out     (_SMCMD), a         ; restore CPU speed
    jr      _cancel_DEVID

; ---------------------------------------------------------
_restore_cpu_speed:
    call    _check_OCMID
    jr      nz, _cancel_DEVID   ; OCM is not detected
    ld      a, (T80STA)
    out     (_SMCMD), a         ; restore CPU speed
    jr      _cancel_DEVID

; ---------------------------------------------------------
T80STA:
    db      _SCNUL              ; null command $80
