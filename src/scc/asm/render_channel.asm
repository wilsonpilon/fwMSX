; ---------------------------------------------------------------------------
; Renderizacao de um canal do SCC (src/scc/core/scc_state.c).
; NASM, sintaxe Intel. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
;
; void scc_render_channel(int32_t *acc, int n, const int8_t *wave,
;                         uint32_t *phase, uint32_t step, int32_t level);
;
; Para cada uma das `n` amostras: le wave[(phase >> 16) & 31], multiplica
; pelo nivel de volume, desloca 7 bits (aritmetico) e SOMA em acc[i]; depois
; avanca a fase em `step` (Q16). A fase fica gravada de volta em *phase.
;
; Dual-ABI no mesmo arquivo (Win64 e SysV), mesma tecnica de block_ops.asm.
; Win64 recebe os 6 argumentos em RCX/RDX/R8/R9 e nos dois ultimos da pilha;
; SysV em RDI/ESI/RDX/RCX/R8D/R9D. A rotina normaliza para os registradores
; SysV e so' depois roda o laco, que so' usa registradores voláteis nas duas
; ABIs -- exceto RDI/RSI no Win64, que sao salvos e restaurados.
;
; Sem chamadas, entao a pilha nao precisa estar alinhada. A flag de direcao
; nao e' tocada.
; ---------------------------------------------------------------------------

bits 64

section .text
    global scc_render_channel

scc_render_channel:
%ifidn __OUTPUT_FORMAT__, win64
    push rdi
    push rsi
    ; Depois dos dois pushes, o 5o argumento (step) esta em [rsp+56] e o 6o
    ; (level) em [rsp+64].
    mov r11d, [rsp+56]      ; step
    mov r10d, [rsp+64]      ; level
    mov rdi, rcx            ; acc
    mov esi, edx            ; n
    mov rax, r8             ; wave (preservado antes de sobrescrever R8)
    mov rcx, r9             ; phase
    mov rdx, rax            ; wave
    mov r8d, r11d           ; step
    mov r9d, r10d           ; level
%endif
    ; SysV (ou ja' normalizado): RDI=acc, ESI=n, RDX=wave, RCX=phase,
    ; R8D=step, R9D=level.
    mov r10d, [rcx]         ; r10d = fase atual
    test esi, esi
    jz .store
.loop:
    mov eax, r10d
    shr eax, 16
    and eax, 31             ; indice da onda
    movsx eax, byte [rdx + rax]
    imul eax, r9d           ; amostra * nivel (cabe em 32 bits: |amostra| <= 128, nivel <= 6553)
    sar eax, 7
    add [rdi], eax
    add rdi, 4
    add r10d, r8d
    dec esi
    jnz .loop
.store:
    mov [rcx], r10d

%ifidn __OUTPUT_FORMAT__, win64
    pop rsi
    pop rdi
%endif
    ret
