! ---------------------------------------------------------------------------
! Mamute do fwMSX: checksum agregado de 16 bits de um bloco (comando XTS).
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md.
!
! mamute_sum16(buf, n): soma de TODOS os `n` bytes, modulo 65536. Diferente do checksum POR LINHA dos despejos
! (mamute_sum.f90, 8 bits): serve para comparar um bloco inteiro (uma ROM, por exemplo) de uma vez.
! ---------------------------------------------------------------------------
module mod_mamute_total
    use iso_c_binding, only: c_signed_char, c_int
    implicit none
contains

    function mamute_sum16(buf, n) bind(c, name="mamute_sum16") result(s)
        integer(c_signed_char), intent(in) :: buf(*)
        integer(c_int), value :: n
        integer(c_int) :: s
        integer :: i

        s = 0
        do i = 1, n
            s = s + iand(int(buf(i)), 255)
        end do
        s = iand(s, 65535)
    end function mamute_sum16

end module mod_mamute_total
