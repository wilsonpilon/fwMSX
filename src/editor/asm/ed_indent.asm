; ---------------------------------------------------------------------------
; Editor de textos do fwMSX: tamanho da indentacao de uma linha.
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). Mesma tecnica de ABI dupla de
; src/debug/asm/dbg_range.asm: Win64 e SysV AMD64 no mesmo arquivo, escolhidas por __OUTPUT_FORMAT__.
;
; int ed_leading_ws(const unsigned char *s, int len);
;
; Conta os bytes 32 (espaco) e 9 (tabulacao) do inicio de s[0, len). Roda a cada Enter com auto-indent, a cada
; "Unindent" e na conversao de espacos em tabulacoes: uma varredura curta, sem pilha.
; ---------------------------------------------------------------------------

bits 64

section .text
    global ed_leading_ws

ed_leading_ws:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: RCX=s, EDX=len
    mov r8, rcx
    mov r9d, edx
%else
    ; SysV AMD64: RDI=s, ESI=len
    mov r8, rdi
    mov r9d, esi
%endif
    xor eax, eax
.loop:
    cmp eax, r9d
    jge .done
    movzx r10d, byte [r8 + rax]
    cmp r10d, 32
    je .next
    cmp r10d, 9
    jne .done
.next:
    inc eax
    jmp .loop
.done:
    ret
