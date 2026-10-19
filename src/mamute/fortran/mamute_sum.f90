! ---------------------------------------------------------------------------
! Mamute do fwMSX: checksum de 8 bits dos despejos (modos 2 e 3 do comando C).
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md.
!
! mamute_checksum8(buf, n, addr_lo, with_addr): soma dos `n` bytes, mais o byte baixo do endereco da linha
! quando with_addr /= 0 (modo 2), tudo modulo 256. Fortran porque e' aritmetica de vetor curta e fora do
! caminho quente; os bytes chegam com sinal (c_signed_char) e sao reduzidos a 0..255 antes de somar.
! ---------------------------------------------------------------------------
module mod_mamute_sum
    use iso_c_binding, only: c_signed_char, c_int
    implicit none
contains

    function mamute_checksum8(buf, n, addr_lo, with_addr) bind(c, name="mamute_checksum8") result(s)
        integer(c_signed_char), intent(in) :: buf(*)
        integer(c_int), value :: n, addr_lo, with_addr
        integer(c_int) :: s
        integer :: i, b

        s = 0
        if (with_addr /= 0) s = iand(addr_lo, 255)
        do i = 1, n
            b = int(buf(i))
            s = s + iand(b, 255)
        end do
        s = iand(s, 255)
    end function mamute_checksum8

end module mod_mamute_sum
