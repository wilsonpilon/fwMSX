! ---------------------------------------------------------------------------
! Posicao de cada tecla na matriz de teclado do MSX (linha 0-10, mascara de
! 1 bit). Codigo ORIGINAL do fwMSX (BSD-3-Clause): as coordenadas sao um
! fato de hardware da matriz MSX internacional (conferido contra Keys[] em
! resource/fMSX/fMSX/MSX.c), calculadas aqui por formula onde ha' padrao
! (letras e digitos) e por faixa nos demais grupos, em vez de copiadas
! como literais -- mesma ideia das tabelas de flag do Z80 e da paleta do
! VDP: trabalho numerico simples, uma unica vez, fora do caminho quente.
!
! Os ids seguem a ORDEM de kKeyNames[] em src/ppi/core/ppi_state.c:
!    0-25  a..z            47-54 shift ctrl graph caps code f1 f2 f3
!   26-35  0..9            55-62 f4 f5 esc tab stop bs select enter
!   36-41  - = \ [ ] ;     63-70 space home ins del left up down right
!   42-46  ' ` , . /       71-80 pad0..pad9, 81-86 pad* pad+ pad/ pad- pad, pad.
! ---------------------------------------------------------------------------
module mod_ppi_key_matrix
    use iso_c_binding, only: c_int8_t, c_int
    implicit none

    integer, parameter :: key_count = 87

contains

    subroutine ppi_build_key_matrix(rows, masks) bind(c, name="ppi_build_key_matrix")
        integer(c_int8_t), intent(out) :: rows(0:key_count - 1)
        integer(c_int8_t), intent(out) :: masks(0:key_count - 1)

        integer :: id, row, bit

        do id = 0, key_count - 1
            call locate(id, row, bit)
            rows(id) = to_byte(row)
            masks(id) = to_byte(2**bit)
        end do
    end subroutine ppi_build_key_matrix

    pure subroutine locate(id, row, bit)
        integer, intent(in) :: id
        integer, intent(out) :: row, bit

        if (id <= 1) then               ! a, b: linha 2, bits 6-7
            row = 2; bit = 6 + id
        else if (id <= 9) then          ! c..j: linha 3
            row = 3; bit = id - 2
        else if (id <= 17) then         ! k..r: linha 4
            row = 4; bit = id - 10
        else if (id <= 25) then         ! s..z: linha 5
            row = 5; bit = id - 18
        else if (id <= 33) then         ! 0..7: linha 0
            row = 0; bit = id - 26
        else if (id <= 35) then         ! 8, 9: linha 1, bits 0-1
            row = 1; bit = id - 34
        else if (id <= 41) then         ! - = \ [ ] ;: linha 1, bits 2-7
            row = 1; bit = id - 34
        else if (id <= 46) then         ! ' ` , . /: linha 2, bits 0-4
            row = 2; bit = id - 42
        else if (id <= 54) then         ! shift..f3: linha 6
            row = 6; bit = id - 47
        else if (id <= 62) then         ! f4..enter: linha 7
            row = 7; bit = id - 55
        else if (id <= 70) then         ! space..right: linha 8
            row = 8; bit = id - 63
        else if (id <= 75) then         ! pad0..pad4: linha 9, bits 3-7
            row = 9; bit = id - 71 + 3
        else if (id <= 80) then         ! pad5..pad9: linha 10, bits 0-4
            row = 10; bit = id - 76
        else if (id <= 83) then         ! pad* pad+ pad/: linha 9, bits 0-2
            row = 9; bit = id - 81
        else                            ! pad- pad, pad.: linha 10, bits 5-7
            row = 10; bit = id - 84 + 5
        end if
    end subroutine locate

    ! Mesma tecnica de flag_tables.f90/palette_table.f90: normaliza para a
    ! faixa -128..127 antes de converter para c_int8_t.
    pure function to_byte(val) result(b)
        integer, intent(in) :: val
        integer(c_int8_t) :: b
        integer :: v

        v = val
        if (v > 127) v = v - 256
        b = int(v, c_int8_t)
    end function to_byte

end module mod_ppi_key_matrix
