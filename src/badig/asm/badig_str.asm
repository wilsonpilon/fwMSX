; ---------------------------------------------------------------------------
; Basic Dignified do fwMSX: prefixo sem diferenciar maiusculas/minusculas.
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). Mesma tecnica de ABI dupla de src/editor/asm/ed_hash.asm:
; Win64 e SysV AMD64 no mesmo arquivo, escolhidas por __OUTPUT_FORMAT__.
;
; int bd_prefix_ci(const char *s, int slen, const char *p, int plen);
;
; O tokenizador do MSX BASIC testa, em cada posicao de cada linha, ate' 170 palavras como prefixo do resto da linha: e' o laco mais quente do
; programa, por isso fica em Assembly, sem pilha e sem chamadas.
; ---------------------------------------------------------------------------

bits 64

section .text
    global bd_prefix_ci

bd_prefix_ci:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: RCX=s, EDX=slen, R8=p, R9D=plen
    mov r10, rcx
    mov r11d, edx
    ; r8 = p, r9d = plen
%else
    ; SysV AMD64: RDI=s, ESI=slen, RDX=p, ECX=plen
    mov r10, rdi
    mov r11d, esi
    mov r8, rdx
    mov r9d, ecx
%endif
    cmp r11d, r9d
    jl .no                       ; o texto e' menor que a palavra (depois disso r11d serve de temporario: rsi e' preservado no Win64)
    xor eax, eax                 ; i = 0 (o resultado fica em eax; so' vira 1 no fim)
.loop:
    cmp eax, r9d
    jge .yes
    movzx edx, byte [r10 + rax]
    movzx ecx, byte [r8 + rax]
    lea r11d, [rdx - 'A']
    cmp r11d, 25
    ja .a_ok
    add edx, 32                  ; A-Z -> a-z
.a_ok:
    lea r11d, [rcx - 'A']
    cmp r11d, 25
    ja .b_ok
    add ecx, 32
.b_ok:
    cmp edx, ecx
    jne .no
    inc eax
    jmp .loop
.yes:
    mov eax, 1
    ret
.no:
    xor eax, eax
    ret
