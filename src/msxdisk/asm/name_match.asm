; ---------------------------------------------------------------------------
; Modulo Assembly do msxdisk (NASM, sintaxe Intel, ABI Win64 / MinGW-w64).
;
; msxdisk_name_match(const char *pattern11, const char *name11) -> int
;
; Compara os 11 bytes de 'name11' (nome MSX-DOS 8.3 sem ponto, preenchido
; com espaco) contra 'pattern11', tratando '?' no padrao como coringa de
; um caractere. Nao chama nenhuma funcao externa -- e o proprio loop de
; comparacao que exercita Assembly de forma genuina nesta fase.
;
; Entrada (ABI Win64):
;   RCX = pattern11
;   RDX = name11
; Saida:
;   EAX = 1 (combina) ou 0 (nao combina)
; ---------------------------------------------------------------------------

section .text
    global msxdisk_name_match

msxdisk_name_match:
    xor r8, r8              ; indice do loop, 0..10

.loop:
    cmp r8, 11
    jge .match               ; percorreu os 11 bytes sem diferenca

    movzx eax, byte [rcx + r8]   ; eax = pattern[i]
    cmp al, '?'
    je .next                     ; coringa: combina com qualquer byte

    movzx r9d, byte [rdx + r8]   ; r9d = name[i]
    cmp al, r9b
    jne .no_match

.next:
    inc r8
    jmp .loop

.match:
    mov eax, 1
    ret

.no_match:
    xor eax, eax
    ret
