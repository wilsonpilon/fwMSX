; ---------------------------------------------------------------------------
; Mamute do fwMSX: busca de uma sequencia de bytes COM CURINGA numa regiao de memoria.
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). Mesma tecnica de ABI dupla de
; src/man/asm/man_fold.asm: Win64 e SysV AMD64 no mesmo arquivo, escolhidas por __OUTPUT_FORMAT__.
;
; int mamute_find(const uint8_t *hay, int hay_len, const uint8_t *pat, const uint8_t *mask, int pat_len);
;
; Devolve o menor indice i tal que, para todo k < pat_len, ((hay[i+k] ^ pat[k]) & mask[k]) == 0; ou -1.
; mask[k] = FFh exige o byte, 00h e' curinga ("esse byte pode ser qualquer um", a virgula dupla do SH).
; E' o laco quente do SH: 64 KB de memoria x ate 256 deslocamentos de texto.
; ---------------------------------------------------------------------------

bits 64

section .text
    global mamute_find

mamute_find:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: RCX=hay, EDX=hay_len, R8=pat, R9=mask, [RSP+40]=pat_len
    mov r10, rcx            ; r10 = hay
    movsxd rdx, edx         ; rdx = hay_len
    mov r11, r8             ; r11 = pat
    mov r8, r9              ; r8  = mask
    movsxd r9, dword [rsp + 40] ; r9 = pat_len
%else
    ; SysV AMD64: RDI=hay, ESI=hay_len, RDX=pat, RCX=mask, R8D=pat_len
    mov r10, rdi            ; r10 = hay
    movsxd rax, esi
    mov r11, rdx            ; r11 = pat
    mov rdx, rax            ; rdx = hay_len
    movsxd r9, r8d          ; r9  = pat_len
    mov r8, rcx             ; r8  = mask
%endif
    push rbx
    push rsi
    test r9, r9
    jle .none
    cmp r9, rdx
    jg .none
    sub rdx, r9             ; rdx = ultimo indice de partida possivel
    xor eax, eax            ; rax = i
.start:
    cmp rax, rdx
    jg .none
    lea rbx, [r10 + rax]    ; rbx = &hay[i]
    xor ecx, ecx            ; rcx = k
.byte:
    cmp rcx, r9
    jge .found
    movzx esi, byte [rbx + rcx]
    xor sil, [r11 + rcx]
    and sil, [r8 + rcx]
    jnz .next
    inc rcx
    jmp .byte
.next:
    inc rax
    jmp .start
.found:
    pop rsi
    pop rbx
    ret
.none:
    mov eax, -1
    pop rsi
    pop rbx
    ret
