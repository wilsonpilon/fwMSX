; ---------------------------------------------------------------------------
; Bloco de transferencia acelerado para LDIR/LDDR (Fase 3, nucleo Z80).
; NASM, sintaxe Intel. Codigo ORIGINAL do fwMSX (BSD-3-Clause) -- fMSX nao
; tem nenhuma rotina de Assembly equivalente para adaptar (ver
; doc/z80-core-spec.md, secao 3.4).
;
; PRIMEIRO arquivo .asm do projeto com DUAS ABIs no mesmo arquivo-fonte
; (Win64 e SysV AMD64/Linux), selecionadas em tempo de montagem via
; `%ifidn __OUTPUT_FORMAT__, win64` / `%else` -- a tecnica padrao do NASM
; para isso, evitando manter dois .asm divergentes (`src/asm/init_asm.asm`
; e `src/msxdisk/asm/name_match.asm` sao Win64-only ainda; este e' o
; primeiro a cobrir Linux tambem).
;
; void z80_fast_block_move(uint8_t *dst, const uint8_t *src,
;                           uint16_t len, int reverse);
;
; reverse == 0 (equivalente a LDIR): copia `len` bytes em ordem
;   CRESCENTE de endereco, byte a byte -- exatamente o que REP MOVSB com
;   DF=0 faz. E' importante que seja essa ordem LITERAL (nao um memmove()
;   "seguro" que detecta sobreposicao): software MSX de verdade as vezes
;   usa LDIR sobre regioes sobrepostas de proposito, um truque classico de
;   preenchimento de memoria que depende de cada byte escrito ficar
;   visivel para a proxima leitura, ainda dentro do mesmo bloco. Convencao
;   de ponteiros: `dst`/`src` apontam para o PRIMEIRO byte de cada regiao.
;
; reverse != 0 (equivalente a LDDR): copia `len` bytes em ordem
;   DECRESCENTE de endereco (REP MOVSB com DF=1). Convencao de ponteiros:
;   `dst`/`src` devem apontar para o ULTIMO byte de cada regiao (ou seja,
;   o chamador ja' passa `dst_base + len - 1` / `src_base + len - 1`) --
;   e' assim que o proprio REP MOVSB com DF=1 espera os ponteiros, ja' que
;   ele decrementa DI/SI a cada byte processado, entao tem que comecar do
;   topo da regiao. Um erro nesse deslocamento e' o jeito mais facil de
;   introduzir um off-by-one nesta rotina -- o lado C que chama esta
;   funcao (src/z80/core/opcodes_ed.h) precisa montar os ponteiros
;   exatamente assim para o caso reverse!=0.
;
; A flag de direcao (DF) e' sempre restaurada para 0 (CLD) antes de
; retornar, em AMBOS os casos -- as duas ABIs (Win64 e SysV) exigem DF=0
; na entrada e na saida de qualquer funcao; deixar DF=1 depois de um LDDR
; quebraria qualquer codigo (nosso ou de terceiros) rodando logo em
; seguida.
;
; RDI/RSI sao preservados via push/pop nas duas ABIs por uniformidade --
; estritamente obrigatorio so no Win64 (onde sao registradores
; nao-volateis); no SysV eles sao volateis (o chamador nao espera
; preservacao), entao ali e' trabalho a mais inofensivo, nao um bug.
; ---------------------------------------------------------------------------

bits 64

section .text
    global z80_fast_block_move

z80_fast_block_move:
    push rdi
    push rsi

%ifidn __OUTPUT_FORMAT__, win64
    ; ABI Win64: RCX=dst, RDX=src, R8=len (16 bits uteis em R8W), R9D=reverse
    mov rdi, rcx
    mov rsi, rdx
    movzx r10d, r8w      ; r10d = len (guardado -- ECX vira o contador do REP)
    mov eax, r9d         ; eax = reverse
%else
    ; ABI SysV AMD64: RDI=dst, RSI=src, RDX=len (16 bits uteis em DX), ECX=reverse
    ; RDI/RSI ja' chegam corretos para o REP MOVSB (destino/origem) --
    ; nenhum shuffle necessario ali.
    mov eax, ecx         ; eax = reverse (lido ANTES de sobrescrever ECX com o len)
    movzx r10d, dx       ; r10d = len
%endif

    mov ecx, r10d        ; ecx = contador do REP MOVSB

    test ecx, ecx
    jz .done             ; len == 0: nada a fazer

    test eax, eax
    jnz .backward

    cld                  ; LDIR: DF=0 (garantido explicitamente, nao assumido)
    rep movsb
    jmp .done

.backward:
    std                  ; LDDR: DF=1, DI/SI decrescem a cada MOVSB
    rep movsb
    cld                  ; restaura DF=0 antes de sair -- ver nota acima

.done:
    pop rsi
    pop rdi
    ret

; ---------------------------------------------------------------------------
; Busca de byte para CPIR/CPDR (Fase 3, nucleo Z80) -- design proprio.
;
; int z80_fast_block_search(const uint8_t *p, uint16_t len, uint8_t value,
;                           int reverse);
;
; Procura `value` nos `len` bytes a partir de `p`: reverse == 0 (CPIR) anda
; para cima, reverse != 0 (CPDR) anda para baixo -- por isso, no caso
; reverse!=0, `p` aponta para o ULTIMO byte da regiao, mesma convencao de
; z80_fast_block_move(). Devolve quantos bytes foram examinados (1..len, ou
; 0 se len == 0): o ultimo examinado e' o que casou, ou o ultimo da regiao
; se nao houve casamento. Quem chama calcula as flags a partir desse byte.
;
; Usa REPNE SCASB, que para no primeiro casamento; DF e' sempre restaurado
; para 0 antes de retornar, como em z80_fast_block_move().
; ---------------------------------------------------------------------------

    global z80_fast_block_search

z80_fast_block_search:
%ifidn __OUTPUT_FORMAT__, win64
    ; ABI Win64: RCX=p, RDX=len (16 bits uteis em DX), R8B=value, R9D=reverse
    push rdi
    mov rdi, rcx
    movzx ecx, dx        ; ecx = len
    movzx eax, r8b       ; al = value
    mov r10d, r9d        ; r10d = reverse
%else
    ; ABI SysV AMD64: RDI=p, RSI=len (SI), DL=value, ECX=reverse
    mov r10d, ecx        ; r10d = reverse (lido ANTES de sobrescrever ECX com o len)
    movzx ecx, si       ; ecx = len
    movzx eax, dl        ; al = value
%endif
    mov r11d, ecx        ; r11d = len (guardado para calcular quantos foram examinados)

    test r10d, r10d
    jnz .backward
    cld
    jmp .scan
.backward:
    std
.scan:
    repne scasb          ; para no primeiro byte == al (ou quando ecx chega a 0)
    cld

    mov eax, r11d
    sub eax, ecx         ; examinados = len - restantes

%ifidn __OUTPUT_FORMAT__, win64
    pop rdi
%endif
    ret
