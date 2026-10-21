; ---------------------------------------------------------------------------
; Projeto .msxproject do fwMSX: copia sem os CR (CR = 13), a normalizacao de fim de linha do hash de um arquivo de TEXTO.
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). Mesma tecnica de ABI dupla de src/editor/asm/ed_hash.asm:
; Win64 e SysV AMD64 no mesmo arquivo, escolhidas por __OUTPUT_FORMAT__. Ver doc/project-spec.md.
;
; int prj_strip_cr(uint8_t *dst, const uint8_t *src, int n);
;
; Copia src[0..n) para dst sem os bytes 13 e devolve quantos bytes escreveu. dst pode ser igual a src (copia sobre si mesma, sempre para tras).
; O hash de um texto nao pode mudar quando o git (ou o Windows) troca LF por CRLF: sem isso, todo arquivo vira "alterado" ao trocar de PC.
; Percorre o arquivo inteiro a cada comparacao do projeto, por isso fica em Assembly, sem pilha e sem chamadas.
; ---------------------------------------------------------------------------

bits 64

section .text
    global prj_strip_cr

prj_strip_cr:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: RCX=dst, RDX=src, R8D=n
    mov r10, rcx
    mov r11, rdx
    mov r9d, r8d
%else
    ; SysV AMD64: RDI=dst, RSI=src, EDX=n
    mov r10, rdi
    mov r11, rsi
    mov r9d, edx
%endif
    xor eax, eax                 ; saida: quantos bytes ja' foram escritos
    xor r8d, r8d                 ; i = 0
.loop:
    cmp r8d, r9d
    jge .done
    movzx edx, byte [r11 + r8]
    inc r8d
    cmp edx, 13
    je .loop                     ; CR: nao copia
    mov [r10 + rax], dl
    inc eax
    jmp .loop
.done:
    ret
