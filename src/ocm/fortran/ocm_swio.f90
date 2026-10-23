! fwMSX -- as portas de E/S comutadas do MSX (40h-4Fh) que o OCM/MSX++ responde (Fortran). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
! A porta 40h escolhe o dispositivo pelo ID escrito; ler a 40h devolve o complemento do ID quando o dispositivo existe (e FFh quando nao). O
! OCM e' o ID D4h (212; hoje "MSX++"), e o ID 08h e' o do fabricante Panasonic (o OCM tambem responde). Ver doc/ocm-spec.md.
function ocm_swio_read40(sel) bind(c, name="ocm_swio_read40") result(v)
    use, intrinsic :: iso_c_binding, only: c_int32_t
    implicit none
    integer(c_int32_t), value :: sel
    integer(c_int32_t) :: v

    if (sel == 212 .or. sel == 8) then
        v = iand(not(sel), 255)
    else
        v = 255
    end if
end function ocm_swio_read40

! O valor lido nas portas 41h-4Fh do ID D4h: 42h e' o estado dos "DIP-SW virtuais" (aqui todos desligados, o padrao de fabrica) e as outras leem 0.
function ocm_swio_regs(sel, reg) bind(c, name="ocm_swio_regs") result(v)
    use, intrinsic :: iso_c_binding, only: c_int32_t
    implicit none
    integer(c_int32_t), value :: sel, reg
    integer(c_int32_t) :: v

    v = 255
    if (sel /= 212) return
    select case (reg)
    case (1)   ! 41h: comandos "smart" (so' escrita)
        v = 255
    case (2)   ! 42h: DIP-SW virtuais
        v = 0
    case default
        v = 0
    end select
end function ocm_swio_regs
