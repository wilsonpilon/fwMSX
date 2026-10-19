; ---------------------------------------------------------------------------
; Mamute do fwMSX: comparacao byte a byte de dois blocos (comando XCM).
; NASM, sintaxe Intel; codigo ORIGINAL do fwMSX (BSD-3-Clause). ABI dupla Win64/SysV, como mamute_scan.asm.
;
; int mamute_diff(const uint8_t *a, const uint8_t *b, int n, int want_equal, int *out, int max);
;
; Devolve quantos indices i < n tem (a[i] != b[i]) -- ou (a[i] == b[i]) quando want_equal != 0 -- e grava os `max` primeiros
; em out[]. O laco e' quente porque o XCM compara 64 KB de uma vez.
; ---------------------------------------------------------------------------

bits 64

section .text
    global mamute_diff

mamute_diff:
%ifidn __OUTPUT_FORMAT__, win64
    ; Win64: RCX=a, RDX=b, R8D=n, R9D=want_equal, [RSP+40]=out, [RSP+48]=max
    mov r10, rcx            ; r10 = a
    mov r11, rdx            ; r11 = b
    movsxd rdx, r8d         ; rdx = n
    mov r8d, r9d            ; r8d = want_equal
    mov r9, [rsp + 40]      ; r9  = out
    movsxd rcx, dword [rsp + 48] ; rcx = max
%else
    ; SysV AMD64: RDI=a, RSI=b, EDX=n, ECX=want_equal, R8=out, R9D=max
    mov r10, rdi            ; r10 = a
    mov r11, rsi            ; r11 = b
    movsxd rdx, edx         ; rdx = n
    mov eax, ecx            ; eax = want_equal (provisorio)
    movsxd rcx, r9d         ; rcx = max
    mov r9, r8              ; r9  = out
    mov r8d, eax            ; r8d = want_equal
%endif
    push rbx
    push rsi                ; sil abaixo: rsi e' callee-saved no Win64
    xor eax, eax            ; rax = total encontrado
    xor ebx, ebx            ; rbx = i
    test r8d, r8d
    setnz r8b               ; r8b = 1 se queremos os iguais
.next:
    cmp rbx, rdx
    jge .done
    mov sil, [r10 + rbx]
    cmp sil, [r11 + rbx]
    setne sil               ; sil = 1 se diferem
    cmp sil, r8b
    je .skip                ; diferem == queremos iguais (ou iguais == queremos diferentes): nao casa
    cmp rax, rcx
    jge .count
    mov [r9 + rax * 4], ebx
.count:
    inc rax
.skip:
    inc rbx
    jmp .next
.done:
    pop rsi
    pop rbx
    ret
