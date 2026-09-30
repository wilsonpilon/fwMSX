; ---------------------------------------------------------------------------
; Modulo Assembly do msxdisk (NASM, sintaxe Intel).
;
; msxdisk_name_match(const char *pattern11, const char *name11) -> int
;
; Compara os 11 bytes de 'name11' (nome MSX-DOS 8.3 sem ponto, preenchido
; com espaco) contra 'pattern11', tratando '?' no padrao como coringa de
; um caractere. Nao chama nenhuma funcao externa -- e o proprio loop de
; comparacao que exercita Assembly de forma genuina nesta fase.
;
; Dual-ABI (Win64 e SysV AMD64/Linux) no mesmo arquivo-fonte, via
; `%ifidn __OUTPUT_FORMAT__` -- mesma tecnica de src/z80/asm/block_ops.asm.
; Corrigido em 2026-09-30 ao validar o build no Linux/WSL2 pela primeira
; vez: este arquivo so' tinha a branch Win64 (RCX/RDX) desde a Fase 2 do
; msxdisk -- montava sem erro para `elf64` (NASM nao valida convencao de
; chamada), mas os dois ponteiros de entrada chegariam nos registradores
; ERRADOS rodando de verdade no Linux (SysV entrega em RDI/RSI, nao
; RCX/RDX), um bug silencioso de runtime (leitura de lixo), nao um erro
; de build -- so' apareceria testando `list`/`extract` com padrao de
; coringa de verdade no Linux. So' precisa mapear os registradores de
; entrada certos por ABI; sem chamada externa nem uso de pilha, nao ha'
; mais nada dependente de ABI aqui.
;
; Saida (as duas ABIs): EAX = 1 (combina) ou 0 (nao combina)
; ---------------------------------------------------------------------------

section .text
    global msxdisk_name_match

msxdisk_name_match:
%ifidn __OUTPUT_FORMAT__, win64
    %define pattern_ptr rcx
    %define name_ptr    rdx
%else
    %define pattern_ptr rdi
    %define name_ptr    rsi
%endif

    xor r8, r8              ; indice do loop, 0..10

.loop:
    cmp r8, 11
    jge .match               ; percorreu os 11 bytes sem diferenca

    movzx eax, byte [pattern_ptr + r8]   ; eax = pattern[i]
    cmp al, '?'
    je .next                     ; coringa: combina com qualquer byte

    movzx r9d, byte [name_ptr + r8]   ; r9d = name[i]
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
