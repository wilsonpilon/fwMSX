; ---------------------------------------------------------------------------
; Modulo Assembly do fwMSX (NASM, sintaxe Intel).
;
; Created by barney on 27/09/2026.
;
; init_asm(int major, int minor, int patch) -> uint16_t
;
; Dual-ABI (Win64 e SysV AMD64/Linux) no mesmo arquivo-fonte, selecionado
; em tempo de montagem via `%ifidn __OUTPUT_FORMAT__` -- mesma tecnica
; usada em src/z80/asm/block_ops.asm (ver esse arquivo pra mais contexto).
;
; Corrigido em 2026-09-30 ao validar o build no Linux/WSL2 pela primeira
; vez: este arquivo so' tinha a branch Win64 desde a Fase 0 do projeto, e
; mesmo assim MONTAVA sem erro para `elf64` (o NASM nao valida convencao
; de chamada, so' gera bytes) -- mas rodando de verdade no Linux os
; argumentos chegariam nos registradores ERRADOS (major/minor/patch vem
; em EDI/ESI/EDX na SysV, nao ECX/EDX/R8D da Win64), e o link falhava
; ("relocation ... can not be used when making a PIE object") porque uma
; chamada direta a `printf` e' incompativel com executavel PIE, padrao em
; distros Linux modernas. Corrigidos os dois: registradores certos por
; ABI, e `call printf wrt ..plt` no lado SysV (chamada PLT-relativa,
; funciona com ou sem PIE).
;
; O modulo monta, na mao, uma chamada a printf() da C runtime (variadica),
; reordenando os argumentos de entrada para a convencao de printf
; (formato primeiro, valores depois).
; ---------------------------------------------------------------------------

default rel

section .data
    fmt_init_asm db "Loading module...Assembly [v %d.%d.%d]", 10, 0   ; 10 = '\n'

section .text
    global init_asm
    extern printf

init_asm:
    ; Prologo padrao -- deixa RSP alinhada em 16 bytes (mesmo requisito
    ; das duas ABIs), suficiente para a SysV chamar printf sem ajuste
    ; extra de pilha.
    push rbp
    mov rbp, rsp

%ifidn __OUTPUT_FORMAT__, win64
    ; ABI Win64: major/minor/patch chegam em ECX/EDX/R8D.
    ; Shadow space (32 bytes) exigido pela ABI Win64 para chamadas a
    ; funcoes externas; multiplo de 16, entao a pilha continua alinhada.
    sub rsp, 32
    ; Reordena para a convencao de printf(fmt,a,b,c): RCX=fmt, RDX/R8/R9=valores.
    mov r9d, r8d            ; patch  -> 4o argumento
    mov r8d, edx            ; minor  -> 3o argumento
    mov edx, ecx            ; major  -> 2o argumento
    lea rcx, [fmt_init_asm]      ; formato -> 1o argumento
    call printf
%else
    ; ABI SysV AMD64 (Linux): major/minor/patch chegam em EDI/ESI/EDX.
    ; Sem shadow space -- RSP ja' esta alinhada em 16 bytes apos o prologo.
    ; Reordena para a convencao de printf(fmt,a,b,c): RDI=fmt, RSI/RDX/RCX=valores.
    mov ecx, edx            ; patch  -> 4o argumento
    mov edx, esi            ; minor  -> 3o argumento
    mov esi, edi            ; major  -> 2o argumento
    lea rdi, [fmt_init_asm]      ; formato -> 1o argumento
    xor eax, eax            ; AL=0: nenhum argumento vetorial/float --
                             ; exigido pela ABI SysV em chamadas variadicas
    call printf wrt ..plt   ; PLT-relativo: funciona com ou sem PIE
%endif

    ; Assinatura hexadecimal do modulo Assembly.
    mov eax, 0x0003

    ; Epilogo padrao.
    mov rsp, rbp
    pop rbp
    ret
