! ---------------------------------------------------------------------------
! obi do fwMSX: quantos clusters os arquivos de uma receita ocupam no disco (o teste "cabe?" antes de gravar).
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/obi-spec.md.
!
! Cada arquivo gasta clusters inteiros (arredondado para cima), entao a soma dos tamanhos NAO serve para saber se cabe. Fortran porque e'
! aritmetica de vetor pura, sem estado.
! ---------------------------------------------------------------------------
module mod_obi_fit
    use iso_c_binding, only: c_int
    implicit none

contains

    function obi_clusters_needed(sizes, n, cluster_bytes) result(total) bind(c, name="obi_clusters_needed")
        integer(c_int), intent(in) :: sizes(*)
        integer(c_int), value :: n, cluster_bytes
        integer(c_int) :: total
        integer :: i

        total = -1
        if (n < 0 .or. cluster_bytes < 1) return
        total = 0
        do i = 1, n
            if (sizes(i) < 0) then
                total = -1
                return
            end if
            total = total + (sizes(i) + cluster_bytes - 1) / cluster_bytes
        end do
    end function obi_clusters_needed

end module mod_obi_fit
