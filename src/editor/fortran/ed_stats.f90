! ---------------------------------------------------------------------------
! Editor de textos do fwMSX: contagem de palavras de uma linha (tela "Information display").
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md.
!
! Uma palavra e' uma sequencia de caracteres que nao sao espaco, tabulacao nem os caracteres de controle
! (< 32): a mesma ideia de "words in edit buffer" do Norton Editor. Fortran porque e' uma varredura numerica
! simples, fora do caminho das teclas (so' roda quando a tela de informacao e' aberta). Cuidado de build: so'
! compara caracteres com "char()"; nao usa iachar (derruba o f951 do gfortran 16).
! ---------------------------------------------------------------------------
module mod_ed_stats
    use iso_c_binding, only: c_char, c_int
    implicit none

contains

    function ed_count_words(buf, n) result(words) bind(c, name="ed_count_words")
        character(kind=c_char), intent(in) :: buf(*)
        integer(c_int), value :: n
        integer(c_int) :: words

        integer :: i
        logical :: in_word

        words = 0
        in_word = .false.
        do i = 1, n
            if (ichar(buf(i)) <= 32) then
                in_word = .false.
            else if (.not. in_word) then
                in_word = .true.
                words = words + 1
            end if
        end do
    end function ed_count_words

end module mod_ed_stats
