! fwMSX -- frequencia de audio (Hz) de um pulso de fita, a partir da
! duracao em T-states e do clock da CPU. Usado pela janela (ver
! doc/tape-spec.md) para mostrar algo como "piloto: 2100 Hz" enquanto a
! fita roda no modo normal. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
module kcs_timing
    use iso_c_binding, only: c_double, c_int64_t
    implicit none
contains
    function kcs_pulse_hz(cpu_clock, pulse_t_states) result(hz) bind(c, name="kcs_pulse_hz")
        integer(c_int64_t), value :: cpu_clock
        integer(c_int64_t), value :: pulse_t_states
        real(c_double) :: hz

        if (pulse_t_states <= 0_c_int64_t) then
            hz = 0.0d0
        else
            ! um pulso = meio-periodo (ver TZX_format.md, secao 2)
            hz = dble(cpu_clock) / (2.0d0 * dble(pulse_t_states))
        end if
    end function kcs_pulse_hz
end module kcs_timing
