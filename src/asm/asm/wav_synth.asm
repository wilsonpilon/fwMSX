; ---------------------------------------------------------------------------
; fwasm (montador): sintese dos pulsos da fita do MSX para o arquivo .wav.
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). Mesma tecnica de ABI dupla de
; src/debug/asm/dbg_range.asm: Win64 e SysV AMD64 no mesmo arquivo, escolhidas por __OUTPUT_FORMAT__.
;
; size_t fwasm_wav_render(const uint8_t *kinds, size_t n, uint8_t *out);
;
; `kinds[i]` descreve o i-esimo CICLO da fita: 0 = curto (2400 Hz, o bit 1 vale dois), 1 = longo (1200 Hz, o bit 0).
; Cada ciclo e' meio-periodo baixo (0) + meio-periodo alto (255) a 44100 amostras/s, e a fronteira de cada meio-periodo
; cai na amostra ARREDONDADA do tempo acumulado -- por isso o tempo e' contado em 1/16 de amostra (meio periodo curto
; = 147/16 = 9,1875 amostras; longo = 294/16): inteiro, sem erro de ponto flutuante. Foi assim que se reproduziu,
; amostra por amostra, a saida do asMSX. Devolve quantas amostras escreveu em `out` (que tem de ter lugar).
;
; Todos os registradores usados sao volateis (so' o RSI, nao volatil no Win64, e' salvo). Funcao-folha, sem pilha alem disso.
; ---------------------------------------------------------------------------

bits 64
default rel

; um meio-periodo: avanca o tempo, escreve as amostras ate' a fronteira arredondada e inverte o nivel
%macro HALF 0
    add r8, rax
    lea rcx, [r8 + 8]
    shr rcx, 4              ; rcx = amostra arredondada onde o meio-periodo termina
    sub rcx, r9             ; rcx = quantas amostras escrever nesse nivel
    jbe %%skip
%%fill:
    mov [r10 + r9], dl
    inc r9
    dec rcx
    jnz %%fill
%%skip:
    xor dl, 0xFF
%endmacro

section .text
    global fwasm_wav_render

fwasm_wav_render:
%ifidn __OUTPUT_FORMAT__, win64
    push rsi
    mov rsi, rcx            ; Win64: RCX = kinds, RDX = n, R8 = out
    mov r11, rdx
    mov r10, r8
%else
    mov r10, rdx            ; SysV AMD64: RDI = kinds, RSI = n, RDX = out
    mov r11, rsi
    mov rsi, rdi
%endif
    xor r8d, r8d            ; r8  = tempo acumulado, em 1/16 de amostra
    xor r9d, r9d            ; r9  = posicao atual (amostras ja' escritas)
    xor edx, edx            ; dl  = nivel do meio-periodo (0 ou 255)
    test r11, r11
    jz .done

.cycle:
    movzx eax, byte [rsi]   ; 0 ou 1
    inc rsi
    imul eax, eax, 147
    add eax, 147            ; rax = meio periodo em 1/16: 147 (curto) ou 294 (longo)

    HALF                    ; meio-periodo baixo
    HALF                    ; meio-periodo alto

    dec r11
    jnz .cycle

.done:
    mov rax, r9
%ifidn __OUTPUT_FORMAT__, win64
    pop rsi
%endif
    ret
