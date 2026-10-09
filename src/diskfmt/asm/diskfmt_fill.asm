; ---------------------------------------------------------------------------
; Modulo Assembly do diskfmt (NASM, sintaxe Intel).
;
; void diskfmt_fill(uint8_t *dst, uint8_t value, size_t count)
;
; Enche `count` bytes de `dst` com `value` (REP STOSB) -- usado para marcar a area de
; dados de um disquete novo com E5h, o byte de "formatado" do MSX-DOS. Dual-ABI (Win64 e
; SysV AMD64/Linux) no mesmo arquivo, via `%ifidn __OUTPUT_FORMAT__`, como em
; src/msxdisk/asm/name_match.asm. REP STOSB usa RDI/RCX/AL; RDI e' callee-saved no Win64,
; entao e' salvo e restaurado ali.
; ---------------------------------------------------------------------------

section .text
    global diskfmt_fill

diskfmt_fill:
%ifidn __OUTPUT_FORMAT__, win64
    push rdi
    mov rdi, rcx            ; dst
    mov eax, edx            ; value (AL)
    mov rcx, r8             ; count
    rep stosb
    pop rdi
    ret
%else
    mov eax, esi            ; value (AL)
    mov rcx, rdx            ; count (RDI ja' e' dst)
    rep stosb
    ret
%endif
