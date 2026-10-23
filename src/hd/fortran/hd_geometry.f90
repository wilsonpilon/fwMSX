! fwMSX -- a conta do tamanho das FATs de uma particao FAT16 (Fortran). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
! O tamanho da FAT depende do numero de clusters, que depende do tamanho da FAT: itera ate' estabilizar.
! Ver doc/hd-spec.md.
function hd_fat_sectors(total, spc, reserved, root_sectors) bind(c, name="hd_fat_sectors") result(fat)
    use, intrinsic :: iso_c_binding, only: c_int32_t
    implicit none
    integer(c_int32_t), value :: total, spc, reserved, root_sectors
    integer(c_int32_t) :: fat
    integer(c_int32_t) :: clusters, next, i

    fat = 1
    do i = 1, 64
        clusters = (total - reserved - root_sectors - 2 * fat) / spc
        if (clusters < 0) clusters = 0
        ! cada entrada de FAT16 tem 2 bytes e as duas primeiras sao reservadas
        next = ((clusters + 2) * 2 + 511) / 512
        if (next == fat) return
        fat = next
    end do
end function hd_fat_sectors
