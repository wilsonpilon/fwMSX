! ---------------------------------------------------------------------------
! MAN/HELP do fwMSX: casamento de nome de topico com o que o usuario digitou.
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/man-spec.md.
!
! man_match_score(nome, consulta): 3 = iguais, 2 = a consulta e' o INICIO do nome, 1 = a consulta aparece
! DENTRO do nome, 0 = nao casa. Sem diferenciar maiusculas (a dobra vem do Assembly, man_fold). Fortran
! porque e' uma rotina de texto pequena e fora de qualquer caminho quente, onde a linguagem e' confortavel.
! ---------------------------------------------------------------------------
module mod_man_match
    use iso_c_binding, only: c_char, c_int
    implicit none

    interface
        function man_fold(c) bind(c, name="man_fold") result(r)
            use iso_c_binding, only: c_int
            integer(c_int), value :: c
            integer(c_int) :: r
        end function man_fold
    end interface

contains

    function man_match_score(a, la, b, lb) bind(c, name="man_match_score") result(score)
        character(kind=c_char), intent(in) :: a(*), b(*)
        integer(c_int), value :: la, lb
        integer(c_int) :: score
        integer :: i, k
        integer(c_int) :: ca, cb
        logical :: same

        score = 0
        if (lb <= 0 .or. lb > la) return
        ! i = posicao (1-based) de `a` onde a consulta comeca; o primeiro casamento decide (comeco vale mais)
        do i = 1, la - lb + 1
            same = .true.
            do k = 1, lb
                ca = ichar(a(i + k - 1))
                cb = ichar(b(k))
                if (man_fold(ca) /= man_fold(cb)) then
                    same = .false.
                    exit
                end if
            end do
            if (same) then
                if (i == 1 .and. la == lb) then
                    score = 3
                else if (i == 1) then
                    score = 2
                else
                    score = 1
                end if
                return
            end if
        end do
    end function man_match_score

end module mod_man_match
