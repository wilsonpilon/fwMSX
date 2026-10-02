; ---------------------------------------------------------------------------
; Contagem de teclas pressionadas na matriz de teclado do MSX (PPI).
; NASM, sintaxe Intel. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
;
; int ppi_pressed_count(const uint8_t *key_state);
;
; `key_state` aponta para a matriz (bit em 0 = tecla pressionada). Conta
; os bits em 0 das linhas 0-10 (11 bytes) -- as linhas 11-15 nao tem
; teclas e ficam sempre em 0xFF. Usa POPCNT sobre o complemento de cada
; byte.
;
; Dual-ABI no mesmo arquivo (Win64 e SysV), mesma tecnica de
; src/z80/asm/block_ops.asm. So' RAX/RCX/RDX/R8 sao usados, todos
; volateis nas duas ABIs -- nada a salvar.
; ---------------------------------------------------------------------------

bits 64

section .text
    global ppi_pressed_count

ppi_pressed_count:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: RCX = key_state
    mov r8, rcx
%else
    ; SysV: RDI = key_state
    mov r8, rdi
%endif
    xor eax, eax            ; eax = total
    xor ecx, ecx            ; ecx = indice da linha
.next:
    movzx edx, byte [r8 + rcx]
    not edx
    and edx, 0xFF           ; so' os 8 bits do byte (NOT inverte os 32)
    popcnt edx, edx
    add eax, edx
    inc ecx
    cmp ecx, 11
    jb .next
    ret
