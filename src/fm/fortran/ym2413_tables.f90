! ---------------------------------------------------------------------------
! Tabelas do chip FM (src/fm/core/ym2413_state.c): seno de 1024 pontos e
! ganho linear de atenuacoes em dB, em passos de 0.25 dB (0 a -256 dB).
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/fm-spec.md.
! ---------------------------------------------------------------------------

module mod_ym2413_tables
    use iso_c_binding, only: c_double
    implicit none

contains

    subroutine ym2413_build_tables(sine, db_gain) bind(c, name="ym2413_build_tables")
        real(c_double), intent(out) :: sine(0:1023)
        real(c_double), intent(out) :: db_gain(0:1023)

        real(c_double), parameter :: pi = 3.14159265358979323846_c_double
        integer :: i

        do i = 0, 1023
            sine(i) = sin(2.0_c_double * pi * real(i, c_double) / 1024.0_c_double)
            db_gain(i) = 10.0_c_double ** (-real(i, c_double) * 0.25_c_double / 20.0_c_double)
        end do
    end subroutine ym2413_build_tables

end module mod_ym2413_tables
