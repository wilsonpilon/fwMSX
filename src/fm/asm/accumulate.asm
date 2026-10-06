; ---------------------------------------------------------------------------
; Soma de canal do chip FM (src/fm/core/ym2413_state.c). NASM, sintaxe Intel.
; Codigo ORIGINAL do fwMSX (BSD-3-Clause).
;
; void ym2413_accumulate(int32_t *acc, const int32_t *src, int n);
;
; acc[i] += src[i] para i = 0..n-1. Chamado uma vez por canal e por bloco de
; amostras; o laco e' o mesmo nas duas ABIs.
;
; Dual-ABI no mesmo arquivo (Win64 e SysV), mesma tecnica de render_channel.asm.
; Win64 recebe acc/src/n em RCX/RDX/R8D; SysV em RDI/RSI/EDX. RDI e RSI sao
; nao-volateis no Win64, por isso sao salvos. Sem chamadas, entao a pilha nao
; precisa estar alinhada. A flag de direcao nao e' tocada.
; ---------------------------------------------------------------------------

bits 64

section .text
    global ym2413_accumulate

ym2413_accumulate:
%ifidn __OUTPUT_FORMAT__, win64
    push rdi
    push rsi
    mov rdi, rcx            ; acc
    mov rsi, rdx            ; src
    mov edx, r8d            ; n
%endif
    ; SysV (ou ja' normalizado): RDI=acc, RSI=src, EDX=n.
    test edx, edx
    jz .done
.loop:
    mov eax, [rsi]
    add [rdi], eax
    add rsi, 4
    add rdi, 4
    dec edx
    jnz .loop
.done:

%ifidn __OUTPUT_FORMAT__, win64
    pop rsi
    pop rdi
%endif
    ret
