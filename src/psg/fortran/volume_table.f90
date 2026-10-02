! ---------------------------------------------------------------------------
! Tabela de volume do PSG AY-3-8910: amplitude (0-15) -> nivel PCM. Codigo
! ORIGINAL do fwMSX (BSD-3-Clause). O DAC do AY e' logaritmico: cada passo
! de volume e' ~3 dB (fator de amplitude de 1/sqrt(2)) -- em vez de uma
! tabela de literais (como Volumes[] do fMSX, que e' uma aproximacao), ela e'
! calculada aqui por formula, uma unica vez, fora do caminho quente (mesma
! ideia das tabelas de flag do Z80 e da paleta do VDP).
!
! Nivel 15 vale 10922 (= 32767/3): os 3 canais somados nunca estouram 16
! bits. Nivel 0 e' silencio absoluto.
! ---------------------------------------------------------------------------
module mod_psg_volume_table
    use iso_c_binding, only: c_int16_t
    implicit none

contains

    subroutine psg_build_volume_table(levels) bind(c, name="psg_build_volume_table")
        integer(c_int16_t), intent(out) :: levels(0:15)

        integer :: i
        real :: amp

        levels(0) = 0_c_int16_t
        do i = 1, 15
            amp = 10922.0 * 2.0 ** (-real(15 - i) / 2.0)
            levels(i) = int(amp + 0.5, c_int16_t)
        end do
    end subroutine psg_build_volume_table

end module mod_psg_volume_table
