; ---------------------------------------------------------------------------
; Modulo Assembly do fwMSX (NASM, sintaxe Intel, ABI Win64 / MinGW-w64).
;
; Created by barney on 27/09/2026.
;
; init_asm(int major, int minor, int patch) -> uint16_t
;
; Entrada (ABI Win64: 1o/2o/3o inteiros em ECX/EDX/R8D):
;   ECX = major
;   EDX = minor
;   R8D = patch
;
; O modulo monta, na mao, uma chamada a printf() da C runtime (variadica),
; reordenando os argumentos para a convencao RCX=formato, RDX/R8/R9=valores,
; reserva os 32 bytes de "shadow space" exigidos pela ABI Win64 e mantem a
; pilha alinhada em 16 bytes no ponto do CALL. Ao final devolve em EAX a
; assinatura hexadecimal do modulo (0x0003).
; ---------------------------------------------------------------------------

default rel

section .data
    fmt_init_asm db "Loading module...Assembly [v %d.%d.%d]", 10, 0   ; 10 = '\n'

section .text
    global init_asm
    extern printf

init_asm:
    ; Prologo padrao.
    push rbp
    mov rbp, rsp

    ; Shadow space (32 bytes) exigido pela ABI Win64 para chamadas a
    ; funcoes externas; multiplo de 16, entao a pilha continua alinhada.
    sub rsp, 32

    ; Reordena major/minor/patch (ECX/EDX/R8D) para a posicao esperada por
    ; printf(fmt, a, b, c): RCX=fmt, RDX=a, R8=b, R9=c.
    mov r9d, r8d            ; patch  -> 4o argumento
    mov r8d, edx            ; minor  -> 3o argumento
    mov edx, ecx            ; major  -> 2o argumento
    lea rcx, [fmt_init_asm]      ; formato -> 1o argumento

    call printf

    ; Assinatura hexadecimal do modulo Assembly.
    mov eax, 0x0003

    ; Epilogo padrao.
    mov rsp, rbp
    pop rbp
    ret
