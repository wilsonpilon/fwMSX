! ---------------------------------------------------------------------------
! Depurador do fwMSX: formatacao do registrador de flags (F) do Z80.
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/debug-spec.md.
!
! "SZ-H-PNC": uma letra por flag quando ligada, '-' quando desligada, na ordem dos bits 7..0 (S Z Y H X P N C;
! Y e X sao os bits nao documentados 5 e 3, mostrados como 'y'/'x'). Fortran porque e' exatamente o tipo de
! rotina numerica/de texto simples e fora do caminho quente do despachante em que a linguagem se da' bem.
! ---------------------------------------------------------------------------
module mod_dbg_fmt
    use iso_c_binding, only: c_char, c_int, c_null_char
    implicit none

contains

    ! buf recebe 8 letras + NUL (9 posicoes).
    subroutine dbg_format_flags(f, buf) bind(c, name="dbg_format_flags")
        integer(c_int), value :: f
        character(kind=c_char), intent(out) :: buf(9)

        character(len=8), parameter :: names = "SZyHxPNC"
        integer :: bit

        do bit = 0, 7
            if (btest(f, 7 - bit)) then
                buf(bit + 1) = names(bit + 1:bit + 1)
            else
                buf(bit + 1) = "-"
            end if
        end do
        buf(9) = c_null_char
    end subroutine dbg_format_flags

end module mod_dbg_fmt
