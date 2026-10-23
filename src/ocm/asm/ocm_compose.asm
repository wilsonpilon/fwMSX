; ---------------------------------------------------------------------------
; Modulo Assembly do ocm (NASM, sintaxe Intel). Dual-ABI (Win64 e SysV AMD64/Linux) no mesmo arquivo, via `%ifidn __OUTPUT_FORMAT__`, como em
; src/diskfmt/asm/diskfmt_fill.asm.
;
; void ocm_copy(uint8_t *dst, const uint8_t *src, size_t count)
;     copia `count` bytes (REP MOVSB): monta a imagem de ROM da celula 3-1 do OCM (sub-ROM de 16 KB + ROM do Kanji BASIC de 32 KB).
; RDI/RSI sao callee-saved no Win64: salvos ali.
; ---------------------------------------------------------------------------

section .text
    global ocm_copy

ocm_copy:
%ifidn __OUTPUT_FORMAT__, win64
    push rdi
    push rsi
    mov rdi, rcx            ; dst
    mov rsi, rdx            ; src
    mov rcx, r8             ; count
    rep movsb
    pop rsi
    pop rdi
    ret
%else
    mov rcx, rdx            ; count (RDI = dst, RSI = src)
    rep movsb
    ret
%endif
