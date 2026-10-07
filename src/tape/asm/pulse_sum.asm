; ---------------------------------------------------------------------------
; Soma das duracoes de pulso (T-states) de uma fita -- usado para calcular a
; duracao total da reproducao (modo normal, barra de progresso da janela
; "Fita K7", ver doc/tape-spec.md). NASM, sintaxe Intel. Codigo ORIGINAL do
; fwMSX (BSD-3-Clause).
;
; uint64_t tape_sum_cycles(const uint32_t *pulses, uint32_t count);
;
; Dual-ABI no mesmo arquivo (Win64 e SysV), mesma tecnica de
; src/ppi/asm/key_count.asm. So' RAX/RCX/RDX/R8/R9 sao usados (volateis
; nas duas ABIs -- nada a salvar).
; ---------------------------------------------------------------------------

bits 64

section .text
    global tape_sum_cycles

tape_sum_cycles:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: RCX = pulses, EDX = count
    mov r8, rcx
    mov r9d, edx
%else
    ; SysV: RDI = pulses, ESI = count
    mov r8, rdi
    mov r9d, esi
%endif
    xor rax, rax             ; rax = soma (64 bits, nao satura em 32)
    xor ecx, ecx             ; ecx = indice
.next:
    cmp ecx, r9d
    jae .done
    mov edx, [r8 + rcx*4]
    add rax, rdx
    inc ecx
    jmp .next
.done:
    ret
