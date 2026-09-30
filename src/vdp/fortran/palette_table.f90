! ---------------------------------------------------------------------------
! Tabela de conversao da paleta do V9938 (RGB de 3+3+3 bits) para RGB888,
! todas as 512 combinacoes possiveis.
!
! Codigo ORIGINAL do fwMSX (BSD-3-Clause) -- NAO adaptado do fMSX. O fMSX
! (resource/fMSX/fMSX/MSX.c, caso 9Ah de WrZ80) so calcula UMA entrada de
! paleta por vez, quando o software escreve nela; aqui pre-calculamos as
! 512 combinacoes de uma vez, mesmo principio ja usado duas vezes no
! projeto (ZSTable/PZSTable do nucleo Z80, CRC32 do mapa de memoria):
! trabalho numerico simples, uma unica vez, fora de qualquer caminho
! quente -- nada ainda consome esta tabela (Fase 1 nao renderiza nenhum
! pixel, ver doc/vdp-spec.md, secao 4), construida aqui para uso futuro
! (Fase 2+).
!
! Formula EXATA do fMSX (verificada em WrZ80, porta 9Ah):
!   R = (PLatch&0x70)*255/112   -- PLatch bits 6-4 = componente R (0-7),
!                                  JA' deslocado 4 bits para a esquerda
!                                  antes da mascara (por isso /112=16*7,
!                                  nao /7)
!   G = (Value&0x07)*255/7      -- Value bits 2-0 = componente G (0-7),
!                                  SEM deslocamento
!   B = (PLatch&0x07)*255/7     -- PLatch bits 2-0 = componente B (0-7),
!                                  SEM deslocamento
! Matematicamente, (c<<4)*255/112 == c*255/7 para c em 0..7 (112=16*7,
! nenhuma precisao e' perdida truncando de um jeito ou de outro -- 4080/112
! e 255/7 dao o mesmo resultado inteiro para todo c em 0..7, conferido a
! mao para os 8 valores antes de generalizar). Por isso esta tabela usa
! UMA unica formula de componente (c*255/7) para R, G e B -- nao precisa
! de duas formulas diferentes so por causa de onde cada bit mora no byte
! de origem.
!
! Indexacao das 512 entradas (escolha propria, documentada aqui -- o fMSX
! nunca constroi uma tabela assim, entao nao ha' convencao dele a seguir):
!   indice = r3*64 + g3*8 + b3,  r3,g3,b3 em 0..7
! ---------------------------------------------------------------------------
module mod_vdp_palette_table
    use iso_c_binding, only: c_int8_t, c_int
    implicit none

contains

    subroutine vdp_build_palette_table(r_table, g_table, b_table) &
            bind(c, name="vdp_build_palette_table")
        integer(c_int8_t), intent(out) :: r_table(0:511)
        integer(c_int8_t), intent(out) :: g_table(0:511)
        integer(c_int8_t), intent(out) :: b_table(0:511)

        integer :: r3, g3, b3, idx

        do r3 = 0, 7
            do g3 = 0, 7
                do b3 = 0, 7
                    idx = r3 * 64 + g3 * 8 + b3
                    r_table(idx) = to_byte(component_to_rgb888(r3))
                    g_table(idx) = to_byte(component_to_rgb888(g3))
                    b_table(idx) = to_byte(component_to_rgb888(b3))
                end do
            end do
        end do
    end subroutine vdp_build_palette_table

    ! c*255/7 -- mesma divisao inteira (truncada) que o C faz; c em 0..7.
    pure function component_to_rgb888(c) result(v)
        integer, intent(in) :: c
        integer :: v
        v = (c * 255) / 7
    end function component_to_rgb888

    ! Mesma tecnica de src/z80/fortran/flag_tables.f90: normaliza para a
    ! faixa -128..127 antes de converter para c_int8_t (signed), em vez de
    ! confiar em INT() nativo para um valor fora da faixa do kind destino
    ! (processor-dependent pelo padrao da linguagem).
    pure function to_byte(val) result(b)
        integer, intent(in) :: val
        integer(c_int8_t) :: b
        integer :: v

        v = val
        if (v > 127) v = v - 256
        b = int(v, c_int8_t)
    end function to_byte

end module mod_vdp_palette_table
