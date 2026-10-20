! ---------------------------------------------------------------------------
! Editor de textos do fwMSX: classificacao de um token numerico (destaque de sintaxe).
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md (fase E4).
!
! O varredor em C (ed_hl_number_end) so' mede ate' onde o token "parece" numero; aqui se decide SE e' numero e de que tipo:
!   0 = nao e' numero (ex.: 1abc)   1 = decimal inteiro   2 = hexadecimal   3 = binario   4 = octal   5 = real
! Formas: 123  12%  1.5  1E5  1.5E-3  0.5!  3#   &H1F &B101 &O17 &17   0x1F  $FF  #FF   %101   0FFh  12h  101b
! Fortran porque e' uma rotina pequena de texto/aritmetica, fora do caminho do teclado. Cuidado de build: usa ichar (nunca iachar).
! ---------------------------------------------------------------------------
module mod_ed_number
    use iso_c_binding, only: c_char, c_int
    implicit none

contains

    logical function is_hex_digit(c)
        character(len=1), intent(in) :: c
        integer :: k
        k = ichar(c)
        is_hex_digit = (k >= 48 .and. k <= 57) .or. (k >= 65 .and. k <= 70) .or. (k >= 97 .and. k <= 102)
    end function is_hex_digit

    logical function is_dec_digit(c)
        character(len=1), intent(in) :: c
        integer :: k
        k = ichar(c)
        is_dec_digit = k >= 48 .and. k <= 57
    end function is_dec_digit

    ! todos os caracteres de s(a:b) sao digitos validos na base dada (2, 8, 10 ou 16); a > b nao vale (vazio)
    logical function all_digits(s, a, b, base)
        character(kind=c_char), intent(in) :: s(*)
        integer, intent(in) :: a, b, base
        integer :: i, k
        all_digits = b >= a
        do i = a, b
            k = ichar(s(i))
            select case (base)
            case (2)
                if (k /= 48 .and. k /= 49) all_digits = .false.
            case (8)
                if (k < 48 .or. k > 55) all_digits = .false.
            case (10)
                if (k < 48 .or. k > 57) all_digits = .false.
            case default
                if (.not. is_hex_digit(s(i))) all_digits = .false.
            end select
        end do
    end function all_digits

    function ed_number_kind(buf, n) result(kind) bind(c, name="ed_number_kind")
        character(kind=c_char), intent(in) :: buf(*)
        integer(c_int), value :: n
        integer(c_int) :: kind

        integer :: i, last, first, dots, es, lim
        character(len=1) :: c0, cl

        kind = 0
        if (n < 1) return
        c0 = buf(1)
        last = n
        cl = buf(last)

        ! &H1F  &B101  &O17  &17
        if (c0 == '&') then
            if (n < 2) return
            select case (ichar(buf(2)))
            case (72, 104)
                if (all_digits(buf, 3, n, 16)) kind = 2
            case (66, 98)
                if (all_digits(buf, 3, n, 2)) kind = 3
            case (79, 111)
                if (all_digits(buf, 3, n, 8)) kind = 4
            case default
                if (all_digits(buf, 2, n, 8)) kind = 4
            end select
            return
        end if
        ! 0x1F
        if (n > 2 .and. c0 == '0' .and. (buf(2) == 'x' .or. buf(2) == 'X')) then
            if (all_digits(buf, 3, n, 16)) kind = 2
            return
        end if
        ! $FF  #FF
        if (c0 == '$' .or. c0 == '#') then
            if (all_digits(buf, 2, n, 16)) kind = 2
            return
        end if
        ! %101
        if (c0 == '%') then
            if (all_digits(buf, 2, n, 2)) kind = 3
            return
        end if
        ! daqui em diante o token comeca com digito (ou com ponto seguido de digito)
        if (.not. (is_dec_digit(c0) .or. (c0 == '.' .and. n > 1))) return
        ! 0FFh  12h
        if (cl == 'h' .or. cl == 'H') then
            if (n > 1 .and. is_dec_digit(c0)) then
                if (all_digits(buf, 1, n - 1, 16)) kind = 2
            end if
            return
        end if
        ! 101b (so' 0 e 1 antes do b)
        if ((cl == 'b' .or. cl == 'B') .and. n > 1 .and. is_dec_digit(c0)) then
            if (all_digits(buf, 1, n - 1, 2)) then
                kind = 3
                return
            end if
        end if
        ! decimal: digitos, ponto, expoente, sufixo de tipo (% ! #)
        lim = n
        if (cl == '%' .or. cl == '!' .or. cl == '#') lim = n - 1
        if (lim < 1) return
        dots = 0
        es = 0
        first = 1
        i = 1
        do while (i <= lim)
            if (is_dec_digit(buf(i))) then
                i = i + 1
            else if (buf(i) == '.' .and. dots == 0 .and. es == 0) then
                dots = 1
                i = i + 1
            else if ((buf(i) == 'E' .or. buf(i) == 'e') .and. es == 0 .and. i > first .and. i < lim) then
                es = 1
                i = i + 1
                if (i <= lim) then
                    if (buf(i) == '+' .or. buf(i) == '-') i = i + 1
                end if
                if (i > lim) return
            else
                return
            end if
        end do
        if (dots == 1 .or. es == 1 .or. cl == '!' .or. cl == '#') then
            kind = 5
        else
            kind = 1
        end if
    end function ed_number_kind

    ! Nota de um candidato do completar para o prefixo digitado: 0 = nao serve, 3 = prefixo com a mesma caixa, 2 = prefixo em outra caixa.
    ! Um candidato igual ao prefixo (mesma caixa) tambem e' 0: nao ha' o que completar.
    function ed_match_score(pre, np, cand, nc) result(score) bind(c, name="ed_match_score")
        character(kind=c_char), intent(in) :: pre(*), cand(*)
        integer(c_int), value :: np, nc
        integer(c_int) :: score

        integer :: i, a, b
        logical :: same_case

        score = 0
        if (np < 1 .or. nc < np) return
        same_case = .true.
        do i = 1, np
            a = ichar(pre(i))
            b = ichar(cand(i))
            if (a /= b) then
                same_case = .false.
                if (a >= 65 .and. a <= 90) a = a + 32
                if (b >= 65 .and. b <= 90) b = b + 32
                if (a /= b) return
            end if
        end do
        if (nc == np .and. same_case) return
        score = 2
        if (same_case) score = 3
    end function ed_match_score

    ! Proximo indice de uma lista circular de `count` itens (base 0) a partir de `cur`, andando `dir` (+1 ou -1). cur < 0 = ainda nao
    ! escolhido: +1 vai ao primeiro e -1 ao ultimo. Usado pelo "proxima/anterior mensagem" do menu Build.
    function ed_cycle(count, cur, dir) result(idx) bind(c, name="ed_cycle")
        integer(c_int), value :: count, cur, dir
        integer(c_int) :: idx

        idx = 0
        if (count < 1) return
        if (cur < 0 .or. cur >= count) then
            if (dir < 0) idx = count - 1
            return
        end if
        idx = modulo(cur + dir, count)
    end function ed_cycle

end module mod_ed_number
