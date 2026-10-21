! ---------------------------------------------------------------------------
! Projeto .msxproject do fwMSX: validacao do caminho de um arquivo DENTRO do projeto.
! Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/project-spec.md.
!
! O caminho vem de dentro do banco (que o usuario copia de outro PC, ou baixa do GitHub) e vira um arquivo no disco ao extrair: um caminho
! malicioso ("../../x", "C:\x", "/etc/x") escreveria fora da raiz escolhida. A regra e' simples e fica aqui, num lugar so': relativo, sem
! "..", sem componente vazio, sem letra de unidade, sem caracteres de controle nem os proibidos no Windows. Fortran porque e' so' um
! percorrimento de caracteres, sem estado.
! ---------------------------------------------------------------------------
module mod_prj_path
    use iso_c_binding, only: c_char, c_int
    implicit none

contains

    ! path(1:n): o caminho com barras "/" (o C++ ja' trocou as "\"). Devolve 1 se serve, 0 se nao.
    function prj_path_ok(path, n) result(ok) bind(c, name="prj_path_ok")
        character(kind=c_char), intent(in) :: path(*)
        integer(c_int), value :: n
        integer(c_int) :: ok

        integer :: i, c, comp_start, comp_len

        ok = 0
        if (n < 1 .or. n > 240) return
        if (path(1) == '/') return

        comp_start = 1
        do i = 1, n + 1
            if (i > n) then
                c = 47
            else
                c = ichar(path(i))
            end if
            if (c == 47) then
                comp_len = i - comp_start
                if (comp_len == 0) return
                if (ichar(path(comp_start)) == 46) then ! "." e ".." (o Fortran nao garante curto-circuito: leio so' o que existe)
                    if (comp_len == 1) return
                    if (ichar(path(comp_start + 1)) == 46 .and. comp_len == 2) return
                end if
                ! o componente nao pode acabar em ponto ou espaco (o Windows os descarta em silencio)
                if (ichar(path(i - 1)) == 46 .or. ichar(path(i - 1)) == 32) return
                comp_start = i + 1
            else
                if (c < 32 .or. c == 127) return
                ! proibidos no Windows: < > : " | ? * \  (o ":" tambem barra a letra de unidade)
                if (c == 60 .or. c == 62 .or. c == 58 .or. c == 34 .or. c == 124 .or. c == 63 .or. c == 42 .or. c == 92) return
            end if
        end do
        ok = 1
    end function prj_path_ok

end module mod_prj_path
