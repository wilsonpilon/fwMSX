! ---------------------------------------------------------------------------
! CRC32 de uma imagem de ROM recem-carregada (mapa de memoria MSX, Fase 2).
!
! Codigo ORIGINAL do fwMSX (BSD-3-Clause) -- CRC32 e' um algoritmo publico
! padrao (poly 0xEDB88320, reflected, o mesmo do zlib/PKZIP/Ethernet), nao
! adaptado de nenhum codigo do fMSX. Uso deliberadamente MODESTO: isso e'
! so uma conveniencia do depurador para identificar "qual ROM exatamente e'
! essa" (ex.: distinguir uma BIOS padrao de uma versao remendada) -- NAO e'
! a mesma coisa que a deteccao automatica de cartucho/mapper do fMSX
! (resource/fMSX/fMSX/MSX.c usa SHA1 contra um banco de assinaturas em
! resource/fMSX/ROMs/CARTS.SHA, via EMULib/SHA1.*, para isso). Reimplementar
! SHA1 e a logica de CARTS.SHA fica para quando a Fase 3 (bank-switch mappers,
! ver doc/memory-map-spec.md, secao 6) realmente precisar identificar tipo de
! mapper automaticamente -- nao e' o que este arquivo faz.
!
! Chamado uma unica vez por MemorySystem::LoadRom() (ver
! src/memmap/cpp/memory_system.cpp) -- nunca no caminho quente do
! despachante Z80, mesmo principio ja usado em src/z80/fortran/flag_tables.f90
! (Fase 2 do nucleo Z80): calculo pontual, fora do caminho quente, tabela
! de 256 entradas como o jeito idiomatico de fazer isso em Fortran.
! ---------------------------------------------------------------------------
module mod_rom_checksum
    use iso_c_binding, only: c_int8_t, c_int32_t
    implicit none

contains

    ! data: bytes da ROM (assumed-size -- o comprimento vem em `length`,
    ! nao inferido do array, pois o lado C so passa um ponteiro cru).
    ! crc_out: resultado, mesmo algoritmo/valores que CRC32("123456789")
    ! == 0xCBF43926 (o vetor de teste padrao usado por qualquer
    ! implementacao de CRC32 -- ver tests/z80/memmap_test.cpp).
    subroutine rom_crc32(data, length, crc_out) bind(c, name="rom_crc32")
        integer(c_int8_t), intent(in) :: data(*)
        integer(c_int32_t), value, intent(in) :: length
        integer(c_int32_t), intent(out) :: crc_out

        integer(c_int32_t) :: table(0:255)
        integer(c_int32_t) :: crc, byte_val
        integer :: i

        call build_table(table)

        ! CRC32 padrao: registrador inicial todo em 1 (0xFFFFFFFF), XOR
        ! final com 0xFFFFFFFF -- equivalente a NOT() bit a bit, ver abaixo.
        crc = -1_c_int32_t
        do i = 1, int(length)
            ! data(i) chega como byte SIGNED (-128..127) do lado C -- IAND
            ! com 255 recupera o valor 0..255 correto antes de usar como
            ! indice de tabela (mesmo cuidado de sinal que to_byte() ja
            ! documentou em flag_tables.f90, so que na direcao oposta:
            ! byte->inteiro em vez de inteiro->byte).
            byte_val = iand(int(data(i), c_int32_t), 255)
            crc = ieor(shiftr(crc, 8), table(iand(ieor(crc, byte_val), 255)))
        end do
        crc_out = not(crc)
    end subroutine rom_crc32

    ! Tabela de 256 entradas do CRC32 reflected, polinomio 0xEDB88320
    ! (mesmo do zlib/PKZIP/Ethernet). Recalculada a cada chamada de
    ! rom_crc32() -- 256*8 operacoes simples, irrelevante perto do custo de
    ! ler um arquivo de ROM do disco, entao nao vale a pena a complexidade
    ! de cachear entre chamadas (isso roda uma vez por LoadRom(), nao por
    ! instrucao Z80 executada).
    subroutine build_table(table)
        integer(c_int32_t), intent(out) :: table(0:255)
        integer(c_int32_t) :: c, poly
        integer :: i, j

        ! INT() de um literal BOZ reinterpreta o padrao de bits no kind
        ! alvo (padrao Fortran 2008) -- 0xEDB88320 tem o bit mais
        ! significativo setado, entao vira um c_int32_t negativo, mas o
        ! padrao de bits e' o que importa aqui (XOR/SHIFTR nao ligam pra
        ! sinal).
        poly = int(z'EDB88320', c_int32_t)

        do i = 0, 255
            c = i
            do j = 1, 8
                if (iand(c, 1) /= 0) then
                    c = ieor(shiftr(c, 1), poly)
                else
                    c = shiftr(c, 1)
                end if
            end do
            table(i) = c
        end do
    end subroutine build_table

end module mod_rom_checksum
