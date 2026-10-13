! ---------------------------------------------------------------------------
! Funcoes matematicas do montador (fwasm): as de ponto flutuante de dupla precisao que as expressoes
! aceitam (SIN, COS, TAN, ACOS, ASIN, ATAN, SQR, SQRT, EXP, POW, LOG, LN, ABS) e as conversoes de
! ponto fixo 8.8 (FIX, FIXMUL, FIXDIV, INT). Codigo ORIGINAL do fwMSX (BSD-3-Clause), escrito a partir do
! manual do asMSX (secao 2.3.2) e do comportamento observado do programa, nao do fonte dele.
!
! Fortran e' o lugar natural para isto: trabalho numerico puro, sem estado, chamado de C++ por
! bind(C). Convencoes (conferidas contra o asMSX de verdade, ver tests/asm/golden):
!   - LOG e' o logaritmo na base 10; LN, o natural; SQR(x) = x*x.
!   - INT e FIX TRUNCAM para zero (INT(-1.5) = -1).
!   - FIX(x) = trunc(x*256), em complemento de 2 de 16 bits (FIX(-1.5) = FE80h).
!   - FIXMUL(a,b) = (a*b)/256 e FIXDIV(a,b) = (a*256)/b, em inteiros (divisao truncada).
! ---------------------------------------------------------------------------
module mod_asm_math
    use iso_c_binding, only: c_double, c_int64_t
    implicit none

contains

    pure function asm_sin(x) bind(c, name="asm_sin") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = sin(x)
    end function asm_sin

    pure function asm_cos(x) bind(c, name="asm_cos") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = cos(x)
    end function asm_cos

    pure function asm_tan(x) bind(c, name="asm_tan") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = tan(x)
    end function asm_tan

    pure function asm_acos(x) bind(c, name="asm_acos") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = acos(x)
    end function asm_acos

    pure function asm_asin(x) bind(c, name="asm_asin") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = asin(x)
    end function asm_asin

    pure function asm_atan(x) bind(c, name="asm_atan") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = atan(x)
    end function asm_atan

    pure function asm_sqr(x) bind(c, name="asm_sqr") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = x * x
    end function asm_sqr

    pure function asm_sqrt(x) bind(c, name="asm_sqrt") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = sqrt(x)
    end function asm_sqrt

    pure function asm_exp(x) bind(c, name="asm_exp") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = exp(x)
    end function asm_exp

    pure function asm_pow(x, y) bind(c, name="asm_pow") result(r)
        real(c_double), value, intent(in) :: x, y
        real(c_double) :: r
        r = x**y
    end function asm_pow

    pure function asm_log(x) bind(c, name="asm_log") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = log10(x)
    end function asm_log

    pure function asm_ln(x) bind(c, name="asm_ln") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        r = log(x)
    end function asm_ln

    pure function asm_abs(x) bind(c, name="asm_abs") result(r)
        real(c_double), value, intent(in) :: x
        real(c_double) :: r
        ! O asMSX trunca antes (abs de inteiro): ABS(-1.5) = 1.
        r = real(abs(int(x, c_int64_t)), c_double)
    end function asm_abs

    ! INT(x): trunca para zero.
    pure function asm_int(x) bind(c, name="asm_int") result(r)
        real(c_double), value, intent(in) :: x
        integer(c_int64_t) :: r
        r = int(x, c_int64_t)
    end function asm_int

    ! FIX(x): ponto fixo 8.8 (trunca x*256 para zero).
    pure function asm_fix(x) bind(c, name="asm_fix") result(r)
        real(c_double), value, intent(in) :: x
        integer(c_int64_t) :: r
        r = int(x * 256.0_c_double, c_int64_t)
    end function asm_fix

    ! FIXMUL(a,b) = (a*b)/256 (divisao inteira truncada).
    pure function asm_fixmul(a, b) bind(c, name="asm_fixmul") result(r)
        integer(c_int64_t), value, intent(in) :: a, b
        integer(c_int64_t) :: r
        r = (a * b) / 256_c_int64_t
    end function asm_fixmul

    ! FIXDIV(a,b) = (a*256)/b. Divisor zero devolve 0 (o chamador avisa antes).
    pure function asm_fixdiv(a, b) bind(c, name="asm_fixdiv") result(r)
        integer(c_int64_t), value, intent(in) :: a, b
        integer(c_int64_t) :: r
        if (b == 0_c_int64_t) then
            r = 0_c_int64_t
        else
            r = (a * 256_c_int64_t) / b
        end if
    end function asm_fixdiv

end module mod_asm_math
