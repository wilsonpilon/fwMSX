! ---------------------------------------------------------------------------
! Tabela de volume do SCC: nivel (0-15) -> amplitude PCM. Codigo ORIGINAL do
! fwMSX (BSD-3-Clause). Diferente do PSG (DAC logaritmico), o volume do SCC e'
! linear: nivel v vale 6553*v/15 (o fMSX usa 255*v/15 no seu sintetizador).
!
! Nivel 15 vale 6553, de modo que 5 canais no maximo somam 5 * 6502 = 32510,
! sem estourar 16 bits -- o calculo de amostra faz (onda * nivel) >> 7, e
! com onda em -128..127 o pico por canal fica em 127*6553/128 = 6502.
! ---------------------------------------------------------------------------

module mod_scc_volume_table
    use iso_c_binding, only: c_int32_t
    implicit none

contains

    subroutine scc_build_volume_table(levels) bind(c, name="scc_build_volume_table")
        integer(c_int32_t), intent(out) :: levels(0:15)

        integer :: i

        do i = 0, 15
            levels(i) = int(6553 * i / 15, c_int32_t)
        end do
    end subroutine scc_build_volume_table

end module mod_scc_volume_table
