; ---------------------------------------------------------------------------
; obi do fwMSX: CRC-32 (zip/PNG) de um bloco, para o log de cada arquivo que vai ao disco.
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). Mesma tecnica de ABI dupla de
; src/editor/asm/ed_hash.asm: Win64 e SysV AMD64 no mesmo arquivo, escolhidas por __OUTPUT_FORMAT__.
;
; unsigned int obi_crc32(const unsigned char *data, int len);
;
; Bit a bit (8 passos por byte, sem tabela): um disco tem no maximo 720 KiB e a conta roda uma vez por arquivo, entao a tabela de 1 KiB
; nao compensaria; assim a rotina nao usa memoria alem do bloco.
; ---------------------------------------------------------------------------

bits 64

section .text
    global obi_crc32

obi_crc32:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: RCX=data, EDX=len
    mov r8, rcx
    mov r9d, edx
%else
    ; SysV AMD64: RDI=data, ESI=len
    mov r8, rdi
    mov r9d, esi
%endif
    mov eax, 0xFFFFFFFF
    xor r10d, r10d               ; i = 0
.byte_loop:
    cmp r10d, r9d
    jge .done
    movzx edx, byte [r8 + r10]
    xor eax, edx
    mov ecx, 8
.bit_loop:
    mov edx, eax
    shr eax, 1
    and edx, 1
    neg edx                      ; 0 ou 0xFFFFFFFF: so' aplica o polinomio se o bit saido era 1
    and edx, 0xEDB88320
    xor eax, edx
    dec ecx
    jnz .bit_loop
    inc r10d
    jmp .byte_loop
.done:
    not eax
    ret
