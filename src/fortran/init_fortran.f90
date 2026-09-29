! ---------------------------------------------------------------------------
! Modulo Fortran do fwMSX.
!
! Created by barney on 27/09/2026.
!
! init_fortran(major, minor, patch) result(signature)
!
! Exposto para C/C++ via ISO_C_BINDING com bind(c), o que desativa o
! name mangling do Fortran e adota a ABI do C (mesma tecnica usada no
! protótipo original deste projeto). A rotina monta a mensagem de
! carregamento com um WRITE formatado numa string interna (trabalho
! genuino de Fortran, sem depender de outra linguagem) e devolve a
! assinatura hexadecimal do modulo (0x0004).
! ---------------------------------------------------------------------------
module mod_init_fortran
    use iso_c_binding, only: c_int, c_short
    use iso_fortran_env, only: output_unit
    implicit none

contains

    function init_fortran(major, minor, patch) result(signature) &
            bind(c, name="init_fortran")
        integer(c_int), value :: major, minor, patch
        integer(c_short) :: signature

        character(len=80) :: line

        write(line, '(A,I0,A,I0,A,I0,A)') &
            'Loading module Fortran [v ', major, '.', minor, '.', patch, ']'
        print '(A)', trim(line)

        ! O runtime do Fortran (libgfortran) usa um buffer de E/S proprio,
        ! independente do buffer do C/C++. Sem este FLUSH explicito, a
        ! linha acima so sairia no encerramento do processo, depois de
        ! tudo o que o C++ ja tiver impresso -- fora de ordem.
        flush(output_unit)

        ! Assinatura hexadecimal do modulo Fortran (0x0004).
        signature = 4_c_short
    end function init_fortran

end module mod_init_fortran
