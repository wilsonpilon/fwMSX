# fwMSX - build raiz
#
# Compila os quatro modulos do projeto (C++, C, Assembly/NASM e Fortran)
# e os liga num unico executavel estatico em dist/. Ver doc/MANUAL.md
# para instrucoes de uso e doc/SPEC.md para a especificacao completa.

cmake_minimum_required(VERSION 3.20)
project(fwMSX C CXX ASM_NASM Fortran)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)

# O pacote pronto para execucao (dist/) e um dos entregaveis do projeto.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)

add_executable(fwMSX
        src/cpp/main.cpp
        src/cpp/init_cpp.cpp
        src/c/init_c.c
        src/asm/init_asm.asm
        src/fortran/init_fortran.f90
)

target_include_directories(fwMSX PRIVATE
        src/cpp
        src/c
        src/asm
        src/fortran
        src/common
)

# Linka estaticamente o runtime do MinGW/UCRT para que dist/fwMSX.exe rode
# sozinho, sem depender de DLLs do MSYS2 na maquina de destino.
if (MINGW)
    target_link_options(fwMSX PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif()
