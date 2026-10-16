; ---------------------------------------------------------------------------
; MAN/HELP do fwMSX: dobra de maiuscula ASCII para minuscula.
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). Mesma tecnica de ABI dupla de
; src/debug/asm/dbg_range.asm: Win64 e SysV AMD64 no mesmo arquivo, escolhidas por __OUTPUT_FORMAT__.
;
; int man_fold(int c);
;
; Devolve c em minuscula se for 'A'..'Z'; qualquer outro valor volta igual. Sem desvio: o teste
; (c - 'A') < 26 sai de uma subtracao e de um SETB, e o resultado soma 32 so' quando casa. E' o que o
; casamento de nomes de topicos (man_match.f90) chama a cada letra comparada.
; ---------------------------------------------------------------------------

bits 64

section .text
    global man_fold

man_fold:
%ifidn __OUTPUT_FORMAT__, win64
    mov eax, ecx            ; Win64: ECX = c
%else
    mov eax, edi            ; SysV AMD64: EDI = c
%endif
    lea edx, [rax - 'A']
    cmp edx, 26
    setb dl                 ; 1 se 'A'..'Z'
    movzx edx, dl
    shl edx, 5              ; 32 ou 0
    add eax, edx
    ret
