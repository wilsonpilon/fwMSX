; ---------------------------------------------------------------------------
; Modulo Assembly do hd (NASM, sintaxe Intel). Dual-ABI (Win64 e SysV AMD64/Linux) no mesmo
; arquivo, via `%ifidn __OUTPUT_FORMAT__`, como em src/diskfmt/asm/diskfmt_fill.asm.
;
; void hd_fill(uint8_t *dst, uint8_t value, size_t count)
;     enche `count` bytes de `dst` com `value` (REP STOSB): a area de dados do HD novo.
; void hd_swap_pairs(uint8_t *dst, const uint8_t *src, size_t count)
;     copia `count` bytes (par) de src para dst trocando cada par de bytes: as strings do
;     IDENTIFY DEVICE do ATA guardam o 1o caractere no byte alto de cada palavra.
; RDI/RSI sao callee-saved no Win64: salvos ali.
; ---------------------------------------------------------------------------

section .text
    global hd_fill
    global hd_swap_pairs

hd_fill:
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

hd_swap_pairs:
%ifidn __OUTPUT_FORMAT__, win64
    push rdi
    push rsi
    mov rdi, rcx            ; dst
    mov rsi, rdx            ; src
    mov rcx, r8             ; count
%else
    mov rcx, rdx            ; count (RDI = dst, RSI = src)
%endif
    shr rcx, 1              ; pares
    jz .done
.loop:
    mov ax, [rsi]
    xchg al, ah
    mov [rdi], ax
    add rsi, 2
    add rdi, 2
    dec rcx
    jnz .loop
.done:
%ifidn __OUTPUT_FORMAT__, win64
    pop rsi
    pop rdi
%endif
    ret
