! ---------------------------------------------------------------------------
! Geracao das tabelas de flags Sign/Zero e Parity/Zero/Sign do nucleo Z80
! do fwMSX.
!
! Codigo ORIGINAL do fwMSX (BSD-3-Clause) -- NAO adaptado do fMSX. O fMSX
! (resource/fMSX/Z80/Tables.h) grava essas mesmas 256 posicoes como
! literais no codigo-fonte; aqui recalculamos os MESMOS valores a partir
! da definicao factual dos bits Sign/Zero/Parity do registrador F do Z80
! (comportamento de hardware, nao "expressao" do fMSX) -- ver
! doc/z80-core-spec.md, secao 3.5. Por isso este arquivo NAO entra na
! lista de arquivos adaptados em LICENSE-THIRD-PARTY.md.
!
! Chamada uma unica vez (ver z80_tables_init() em z80_tables.c), nunca no
! caminho quente do despachante -- e' precisamente o tipo de tarefa
! numerica simples e inofensiva para desempenho que justifica Fortran
! aqui, ao contrario do proprio switch de opcodes (ver secao 4 do
! documento de design, "Por que nao usar Fortran em nada tocado pelo
! switch").
!
! g_z80_daa_table (tabela de correcao BCD do DAA) permanece como array
! literal em C (z80_tables.c) -- regenera-la por formula teria risco real
! de erro para ganho de desempenho zero (nao e' hot path de qualquer
! forma), entao nao faz parte do escopo desta rotina.
! ---------------------------------------------------------------------------
module mod_z80_flag_tables
    use iso_c_binding, only: c_int8_t, c_int
    implicit none

contains

    ! zs_table(i)  = Z_FLAG se i==0, senao S_FLAG se o bit 7 de i estiver
    !                setado, senao 0.
    ! pzs_table(i) = zs_table(i) OR'ado com P_FLAG quando i tem paridade
    !                par (numero par de bits em 1).
    !
    ! s_flag/z_flag/p_flag chegam como parametro (vindos das macros
    ! Z80_S_FLAG/Z80_Z_FLAG/Z80_P_FLAG de z80_state.h) em vez de virem
    ! hard-coded aqui -- o cabecalho C continua sendo a unica fonte de
    ! verdade sobre a posicao de cada bit de flag; o Fortran so faz a
    ! aritmetica de preenchimento do array.
    subroutine z80_build_flag_tables(zs_table, pzs_table, s_flag, z_flag, p_flag) &
            bind(c, name="z80_build_flag_tables")
        integer(c_int8_t), intent(out) :: zs_table(0:255)
        integer(c_int8_t), intent(out) :: pzs_table(0:255)
        integer(c_int), value :: s_flag, z_flag, p_flag

        integer :: i, bits, val

        do i = 0, 255
            val = 0
            if (i == 0) then
                val = ior(val, z_flag)
            else if (iand(i, 128) /= 0) then
                val = ior(val, s_flag)
            end if
            zs_table(i) = to_byte(val)

            ! POPCNT (intrinseco Fortran 2008): conta bits em 1 de i, para
            ! decidir a paridade -- em vez de um loop manual de contagem
            ! de bits, que seria a forma "C" de fazer a mesma coisa.
            bits = popcnt(i)
            if (mod(bits, 2) == 0) then
                pzs_table(i) = to_byte(ior(val, p_flag))
            else
                pzs_table(i) = to_byte(val)
            end if
        end do
    end subroutine z80_build_flag_tables

    ! Converte um valor 0..255 (resultado de IOR entre flags de ate 0x80)
    ! para o kind c_int8_t (signed de 1 byte) preservando o padrao de
    ! bits. INT() nativo do Fortran para um valor fora da faixa do kind
    ! destino e' processor-dependent pelo padrao da linguagem -- em vez
    ! de confiar nisso, normalizamos explicitamente para a faixa
    ! -128..127 antes de converter, o que e' bem definido.
    pure function to_byte(val) result(b)
        integer, intent(in) :: val
        integer(c_int8_t) :: b
        integer :: v

        v = val
        if (v > 127) v = v - 256
        b = int(v, c_int8_t)
    end function to_byte

end module mod_z80_flag_tables
