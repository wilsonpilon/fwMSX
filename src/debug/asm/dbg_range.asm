; ---------------------------------------------------------------------------
; Depurador do fwMSX: teste de faixa de endereco com volta em 16 bits.
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). Mesma tecnica de ABI dupla de
; src/z80/asm/block_ops.asm: Win64 e SysV AMD64 no mesmo arquivo, escolhidas por __OUTPUT_FORMAT__.
;
; int dbg_addr_in_range(uint16_t addr, uint16_t base, uint16_t len);
;
; resultado = ((uint16_t)(addr - base)) < len. A subtracao de 16 bits ja' da' a volta certa quando a faixa
; atravessa FFFFh -> 0000h, e len == 0 nunca casa (nada e' menor que 0). Chamado a cada leitura/escrita do Z80
; enquanto houver watchpoints: por isso e' um punhado de instrucoes, sem pilha.
; ---------------------------------------------------------------------------

bits 64

section .text
    global dbg_addr_in_range

dbg_addr_in_range:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: ECX=addr, EDX=base, R8D=len
    movzx eax, cx
    movzx edx, dx
    movzx r8d, r8w
    sub eax, edx
    movzx eax, ax        ; volta em 16 bits
    cmp eax, r8d
%else
    ; SysV AMD64: EDI=addr, ESI=base, EDX=len
    movzx eax, di
    movzx esi, si
    movzx edx, dx
    sub eax, esi
    movzx eax, ax        ; volta em 16 bits
    cmp eax, edx
%endif
    setb al
    movzx eax, al
    ret
