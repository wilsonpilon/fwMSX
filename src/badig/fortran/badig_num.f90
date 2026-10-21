! ---------------------------------------------------------------------------
! Basic Dignified do fwMSX: o arredondamento dos digitos das constantes reais do tokenizador.
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/badig-spec.md.
!
! O MSX guarda um numero real como BCD: expoente + digitos decimais (6 na precisao simples, 14 na dupla). O tokenizador recebe os
! digitos exatos do numero e precisa corta-los na precisao, arredondando pelo primeiro digito descartado (>= 5 sobe). Fortran porque
! e' aritmetica de digitos, sem estado. Comportamento herdado do msxbatoken.py, inclusive o "vai um" que alonga o numero
! (999999 -> 1000000): o original grava assim e o fwMSX tem que gravar igual.
! ---------------------------------------------------------------------------
module mod_badig_num
    use iso_c_binding, only: c_char, c_int
    implicit none

contains

    ! digits(1:n) = os digitos decimais ('0'..'9') do numero; precision = quantos digitos cabem.
    ! out(1:outcap) recebe os digitos ja' cortados/arredondados; a funcao devolve quantos escreveu (0 se nao coube).
    function bd_round_digits(digits, n, precision, out, outcap) result(outn) bind(c, name="bd_round_digits")
        character(kind=c_char), intent(in) :: digits(*)
        integer(c_int), value :: n, precision, outcap
        character(kind=c_char), intent(out) :: out(*)
        integer(c_int) :: outn

        integer :: i, keep, rd
        character(len=64) :: buf

        outn = 0
        if (n < 0 .or. precision < 1 .or. precision > 60 .or. n > 60) return
        keep = min(n, precision)
        do i = 1, keep
            buf(i:i) = digits(i)
        end do

        rd = 0
        if (n > precision) then
            if (ichar(digits(precision + 1)) >= 48 .and. ichar(digits(precision + 1)) <= 57) rd = ichar(digits(precision + 1)) - 48
        end if

        if (rd >= 5) then
            ! soma 1 ao numero formado pelos `keep` digitos; o "vai um" pode criar um digito a mais
            i = keep
            do while (i >= 1)
                if (buf(i:i) == '9') then
                    buf(i:i) = '0'
                    i = i - 1
                else
                    buf(i:i) = char(ichar(buf(i:i)) + 1)
                    exit
                end if
            end do
            if (i < 1) then
                ! so' noves: 99 -> 100 (int("99") + 1 em Python)
                if (keep + 1 > outcap) return
                out(1) = '1'
                do i = 1, keep
                    out(i + 1) = buf(i:i)
                end do
                outn = keep + 1
                return
            end if
        end if

        if (keep > outcap) return
        do i = 1, keep
            out(i) = buf(i:i)
        end do
        outn = keep
    end function bd_round_digits

end module mod_badig_num
