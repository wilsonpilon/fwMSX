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

# --- msxdisk: utilitario de imagens de disco MSX (.dsk) -- ver doc/msxdisk-spec.md ---
#
# UM UNICO executavel para os quatro modos (a pedido do autor): CLI
# one-shot, shell interativo, TUI e (Fase 5) GUI -- ver
# src/msxdisk/entry.{h,cpp} para o roteamento de linha de comando e
# src/msxdisk/shell/shell.cpp para os comandos "tui"/"call tui"/"gui"/
# "call gui" que trocam de modo sem sair do processo.
#
# Os mesmos fontes (MSXDISK_LIB_SOURCES) sao compilados tanto no alvo
# msxdisk (standalone) quanto no alvo fwMSX ("fwmsx --msxdisk ...", Fase
# 5c) -- ver a funcao msxdisk_link_target() mais abaixo. Todos os fontes
# de linguagem (C++/C/ASM/Fortran) sao listados diretamente em cada
# add_executable(), nunca via lib intermediaria -- CMake so garante a
# resolucao automatica das libs de runtime do Fortran (libgfortran/
# libquadmath) quando os objetos dessas linguagens pertencem ao proprio
# alvo que esta sendo linkado.
include(FetchContent)

FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.4.2
)
FetchContent_MakeAvailable(CLI11)

# replxx: edicao de linha/historico do shell interativo (Fase 3).
set(REPLXX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        replxx
        GIT_REPOSITORY https://github.com/AmokHuginnsson/replxx.git
        GIT_TAG release-0.0.4
)
FetchContent_MakeAvailable(replxx)

# FTXUI: biblioteca da TUI estilo Norton Commander/XTree (Fase 4).
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
        GIT_TAG v7.0.3
)
FetchContent_MakeAvailable(ftxui)

# SQLite (amalgamation oficial sqlite.org): guarda configuracoes, temas e
# metadados de imagem da TUI/GUI (Fase 4/5). Nao tem CMakeLists.txt
# proprio -- so baixa o par sqlite3.c/.h e compilamos como uma lib nossa.
FetchContent_Declare(
        sqlite3
        URL https://www.sqlite.org/2026/sqlite-amalgamation-3530400.zip
)
FetchContent_GetProperties(sqlite3)
if (NOT sqlite3_POPULATED)
    FetchContent_Populate(sqlite3)
endif()
add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${sqlite3_SOURCE_DIR})

# GUI (Dear ImGui + GLFW + OpenGL3, Fase 5) -- atras de uma opcao pra
# quem quiser compilar so a versao console, sem puxar OpenGL/GLFW.
option(FWMSX_MSXDISK_GUI "Compila a GUI do msxdisk (Dear ImGui + GLFW + OpenGL3)" ON)

set(MSXDISK_LIB_SOURCES
        src/msxdisk/entry.cpp
        src/msxdisk/core/geometry.cpp
        src/msxdisk/core/dir_entry.cpp
        src/msxdisk/core/disk_image.cpp
        src/msxdisk/core/msxdos1_boot.cpp
        src/msxdisk/core/msx_path.cpp
        src/msxdisk/fat12_c/fat12.c
        src/msxdisk/asm/name_match.asm
        src/msxdisk/fortran/geometry_calc.f90
        src/msxdisk/cli/commands.cpp
        src/msxdisk/cli/format.cpp
        src/msxdisk/cli/app.cpp
        src/msxdisk/shell/shell.cpp
        src/msxdisk/shell/session.cpp
        src/msxdisk/shell/local_fs.cpp
        src/msxdisk/config/theme.cpp
        src/msxdisk/config/config_store.cpp
        src/msxdisk/tui/pane_state.cpp
        src/msxdisk/tui/app.cpp
)

if (FWMSX_MSXDISK_GUI)
    # GLFW: janela/contexto OpenGL/entrada.
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
            glfw3
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
    )
    FetchContent_MakeAvailable(glfw3)

    # Dear ImGui nao tem CMakeLists.txt oficial -- baixa so o source e a
    # lib e montada na mao (core + backends GLFW/OpenGL3). O loader de
    # funcoes OpenGL vem embutido em imgui_impl_opengl3_loader.h, sem
    # precisar de GLAD/GLEW separado.
    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.92.9
    )
    FetchContent_GetProperties(imgui)
    if (NOT imgui_POPULATED)
        FetchContent_Populate(imgui)
    endif()
    add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
    target_link_libraries(imgui PUBLIC glfw)

    find_package(OpenGL REQUIRED)

    list(APPEND MSXDISK_LIB_SOURCES src/msxdisk/gui/app.cpp src/msxdisk/gui/style.cpp src/msxdisk/gui/file_dialog.cpp)
else ()
    list(APPEND MSXDISK_LIB_SOURCES src/msxdisk/gui/app_stub.cpp)
endif ()

# Aplica a um alvo (msxdisk ou fwMSX) tudo que os fontes do msxdisk
# precisam: include dir, libs externas e o link estatico do runtime
# MinGW/UCRT (pra rodar sem depender de DLLs do MSYS2 na maquina destino).
function(msxdisk_link_target target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${target} PRIVATE CLI11::CLI11 replxx ftxui::component ftxui::dom ftxui::screen sqlite3)
    if (FWMSX_MSXDISK_GUI)
        target_link_libraries(${target} PRIVATE imgui OpenGL::GL comdlg32)
    endif ()
    if (MINGW)
        target_link_options(${target} PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
    endif ()
endfunction()

add_executable(msxdisk tools/msxdisk/main.cpp ${MSXDISK_LIB_SOURCES})
msxdisk_link_target(msxdisk)
set_target_properties(msxdisk PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/msxdisk)

# --- Nucleo Z80 (Fases 1-3) -- ver doc/z80-core-spec.md --------------
#
# Motor de despacho em C (adaptado do fMSX, ver LICENSE-THIRD-PARTY.md),
# tabelas de flag geradas em Fortran e aceleracao de LDIR/LDDR em
# Assembly (dual-ABI Win64/SysV) + wrapper de orquestracao em C++
# (design proprio). Z80_LIB_SOURCES e' compilado tanto no alvo de teste
# isolado (z80test) quanto, a partir da Fase 4, no proprio fwMSX.
set(Z80_LIB_SOURCES
        src/z80/core/z80_tables.c
        src/z80/core/z80_core.c
        src/z80/cpp/z80_cpu.cpp
        src/z80/fortran/flag_tables.f90
        src/z80/asm/block_ops.asm
)

# Sessao de depuracao do Z80 (Fase 4; comandos de slot da Fase 1 do mapa
# de memoria, ver doc/memory-map-spec.md): RAM plana ou mapa de memoria
# real + Z80Cpu + comandos reset/regs/step/run/break/mem/peek/poke/load/
# fill/slots/pages/slotmem/slotpeek/slotpoke, sem nenhuma dependencia de
# replxx -- testavel isoladamente (ver tests/z80/debug_session_test.cpp e
# tests/z80/memmap_test.cpp). z80_debug_shell.cpp (o REPL interativo em
# si) fica de fora desta lista de proposito: depende de replxx/stdin,
# entao so entra no alvo fwMSX, nunca nos alvos de teste.
set(Z80_DEBUG_SESSION_SOURCES
        src/z80/debug/z80_debug_session.cpp
        src/z80/debug/z80_disasm.cpp
        src/z80/debug/z80_debug_shell_startup.cpp
)

# --- Mapa de memoria MSX (slots/subslots, Fases 1-2) -- ver
# doc/memory-map-spec.md ---
#
# Motor de slots em C (adaptado do fMSX, ver LICENSE-THIRD-PARTY.md) +
# MemorySystem/SlotMemoryBus em C++ (design proprio) -- API de inspecao
# de qualquer combinacao de slot independente da vista atual da CPU
# (requisito vital do autor para o depurador). Fase 2 (LoadRom) soma um
# CRC32 em Fortran (rom_checksum.f90) -- conveniencia do depurador, nao
# deteccao de mapper (ver a nota no proprio arquivo .f90). Sem ASM -- ver
# doc/memory-map-spec.md, secao 3.1 (troca de slot e' busca em tabela de
# ponteiros, nao ha' loop quente para acelerar).
set(MEMMAP_LIB_SOURCES
        src/memmap/core/slot_state.c
        src/memmap/cpp/memory_system.cpp
        src/memmap/fortran/rom_checksum.f90
)

enable_testing()

add_executable(z80test tests/z80/smoke_test.cpp ${Z80_LIB_SOURCES})
target_include_directories(z80test PRIVATE src/z80)
set_target_properties(z80test PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/z80test)
if (MINGW)
    target_link_options(z80test PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME z80_smoke COMMAND z80test)

add_executable(z80dbgtest tests/z80/debug_session_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES})
target_include_directories(z80dbgtest PRIVATE src/z80 src/memmap)
set_target_properties(z80dbgtest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/z80dbgtest)
if (MINGW)
    target_link_options(z80dbgtest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME z80_debug_session COMMAND z80dbgtest)

add_executable(memmaptest tests/z80/memmap_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES})
target_include_directories(memmaptest PRIVATE src/z80 src/memmap)
# FWMSX_SOURCE_DIR: caminho absoluto da raiz do repositorio, para o teste
# de aceitacao com a BIOS real (resource/fMSX/ROMs/MSX.ROM) achar o
# arquivo independente de qual seja o diretorio de trabalho do CTest ao
# rodar o executavel (nao e' garantido ser a raiz do repo).
target_compile_definitions(memmaptest PRIVATE FWMSX_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(memmaptest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/memmaptest)
if (MINGW)
    target_link_options(memmaptest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME memmap_slots COMMAND memmaptest)

# --- fwMSX: esqueleto multi-linguagem do emulador (Fase 0) ---
#
# "fwmsx --msxdisk ..." repassa pro msxdisk embutido (mesmos
# MSXDISK_LIB_SOURCES acima) -- ver doc/SPEC.md, secao 5.1.
# "fwmsx --z80dbg" abre o REPL de depuracao do Z80 (Fase 4, ver
# doc/z80-core-spec.md) -- unico lugar que compila z80_debug_shell.cpp
# (usa replxx, por isso fora de Z80_DEBUG_SESSION_SOURCES). "--z80dbg
# --slots" liga o mapa de memoria de verdade (Fase 1, ver
# doc/memory-map-spec.md) em vez da RAM plana default.
add_executable(fwMSX
        src/cpp/main.cpp
        src/cpp/init_cpp.cpp
        src/c/init_c.c
        src/asm/init_asm.asm
        src/fortran/init_fortran.f90
        ${MSXDISK_LIB_SOURCES}
        ${Z80_LIB_SOURCES}
        ${Z80_DEBUG_SESSION_SOURCES}
        ${MEMMAP_LIB_SOURCES}
        src/z80/debug/z80_debug_shell.cpp
)

target_include_directories(fwMSX PRIVATE
        src/cpp
        src/c
        src/asm
        src/fortran
        src/common
        src/z80
        src/memmap
)
msxdisk_link_target(fwMSX)
set_target_properties(fwMSX PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/fwMSX)
