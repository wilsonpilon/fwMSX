! ---------------------------------------------------------------------------
! Modulo Fortran do msxdisk (fwMSX).
!
! msxdisk_geometry_stats: calcula espaco livre (bytes e percentual) de um
! disco FAT12 a partir do total de clusters de dados e de quantos estao em
! uso. Trabalho numerico genuino em Fortran (aritmetica inteira de 64 bits
! e ponto flutuante), exposto ao C/C++ via ISO_C_BINDING (mesma tecnica de
! src/fortran/init_fortran.f90).
! ---------------------------------------------------------------------------
module mod_geometry_calc
    use iso_c_binding, only: c_int32_t, c_int64_t, c_float
    implicit none

contains

    subroutine msxdisk_geometry_stats(total_clusters, used_clusters, &
            bytes_per_cluster, free_bytes, free_percent) &
            bind(c, name="msxdisk_geometry_stats")
        integer(c_int32_t), value :: total_clusters, used_clusters, bytes_per_cluster
        integer(c_int64_t), intent(out) :: free_bytes
        real(c_float), intent(out) :: free_percent

        integer(c_int32_t) :: free_clusters

        free_clusters = max(total_clusters - used_clusters, 0_c_int32_t)
        free_bytes = int(free_clusters, c_int64_t) * int(bytes_per_cluster, c_int64_t)

        if (total_clusters > 0) then
            free_percent = 100.0_c_float * real(free_clusters, c_float) &
                    / real(total_clusters, c_float)
        else
            free_percent = 0.0_c_float
        end if
    end subroutine msxdisk_geometry_stats

end module mod_geometry_calc
