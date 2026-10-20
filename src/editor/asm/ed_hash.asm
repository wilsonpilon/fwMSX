; ---------------------------------------------------------------------------
; Editor de textos do fwMSX: hash FNV-1a que ignora maiusculas/minusculas (destaque de sintaxe).
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). Mesma tecnica de ABI dupla de
; src/debug/asm/dbg_range.asm: Win64 e SysV AMD64 no mesmo arquivo, escolhidas por __OUTPUT_FORMAT__.
;
; unsigned int ed_hash_ci(const unsigned char *s, int len);
;
; FNV-1a de 32 bits sobre os bytes de s[0, len) com as letras A-Z dobradas para a-z. E' a chave da tabela de palavras reservadas de
; cada linguagem: roda uma vez por palavra de cada linha desenhada, entao fica em Assembly, sem pilha.
; ---------------------------------------------------------------------------

bits 64

section .text
    global ed_hash_ci

ed_hash_ci:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: RCX=s, EDX=len
    mov r8, rcx
    mov r9d, edx
%else
    ; SysV AMD64: RDI=s, ESI=len
    mov r8, rdi
    mov r9d, esi
%endif
    mov eax, 2166136261          ; deslocamento inicial do FNV-1a
    xor r10d, r10d               ; i = 0
.loop:
    cmp r10d, r9d
    jge .done
    movzx edx, byte [r8 + r10]
    lea ecx, [rdx - 'A']
    cmp ecx, 25
    ja .keep
    add edx, 32                  ; A-Z -> a-z
.keep:
    xor eax, edx
    imul eax, eax, 16777619      ; primo do FNV-1a
    inc r10d
    jmp .loop
.done:
    ret
