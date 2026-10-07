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
    # Linux: o GLFW 3.4 compila por padrao o backend Wayland, que exige
    # wayland-scanner (+ libwayland-dev, libxkbcommon-dev, wayland-protocols).
    # Sem isso o configure falha ("Failed to find wayland-scanner"). Se a
    # ferramenta nao existe, cai para X11 so' (funciona tambem via XWayland).
    if (UNIX AND NOT APPLE)
        find_program(FWMSX_WAYLAND_SCANNER wayland-scanner)
        if (NOT FWMSX_WAYLAND_SCANNER)
            message(STATUS "wayland-scanner nao encontrado -- GLFW sera compilado so' com X11")
            set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
        endif()
    endif()
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
    set(MACHINE_WINDOW_SOURCES src/machine/gui/emu_window.cpp src/machine/gui/video_filters.cpp
            src/machine/gui/rom_manager.cpp)
else ()
    list(APPEND MSXDISK_LIB_SOURCES src/msxdisk/gui/app_stub.cpp)
    set(MACHINE_WINDOW_SOURCES src/machine/gui/emu_window_stub.cpp)
endif ()

# --- Audio ao vivo (miniaudio) -- ver doc/audio-spec.md -------------------
#
# miniaudio e' um unico header de dominio publico/MIT-0 (baixado como o
# ImGui); so' o dispositivo de saida e' compilado (ver audio_output.cpp).
# Com FWMSX_AUDIO=OFF entra um stub e o emulador roda mudo.
option(FWMSX_AUDIO "Compila a saida de audio ao vivo (miniaudio)" ON)
if (FWMSX_AUDIO)
    FetchContent_Declare(
            miniaudio
            GIT_REPOSITORY https://github.com/mackron/miniaudio.git
            GIT_TAG 0.11.22
            GIT_SHALLOW TRUE
    )
    FetchContent_GetProperties(miniaudio)
    if (NOT miniaudio_POPULATED)
        FetchContent_Populate(miniaudio)
    endif ()
    set(AUDIO_LIB_SOURCES src/audio/audio_output.cpp)
    set_source_files_properties(src/audio/audio_output.cpp PROPERTIES INCLUDE_DIRECTORIES ${miniaudio_SOURCE_DIR})
else ()
    set(AUDIO_LIB_SOURCES src/audio/audio_output_stub.cpp)
endif ()

# miniaudio no Linux precisa de pthread/dl/m; no Windows carrega tudo em
# tempo de execucao (nada a linkar).
function(audio_link_target target)
    if (FWMSX_AUDIO AND UNIX AND NOT APPLE)
        find_package(Threads REQUIRED)
        target_link_libraries(${target} PRIVATE Threads::Threads ${CMAKE_DL_LIBS} m)
    endif ()
endfunction()

# Aplica a um alvo (msxdisk ou fwMSX) tudo que os fontes do msxdisk
# precisam: include dir, libs externas e o link estatico do runtime
# MinGW/UCRT (pra rodar sem depender de DLLs do MSYS2 na maquina destino).
function(msxdisk_link_target target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${target} PRIVATE CLI11::CLI11 replxx ftxui::component ftxui::dom ftxui::screen sqlite3)
    if (FWMSX_MSXDISK_GUI)
        target_link_libraries(${target} PRIVATE imgui OpenGL::GL)
        # comdlg32 (dialogos nativos de arquivo) so existe no Windows --
        # descoberto validando o build no Linux/WSL2 pela primeira vez
        # (2026-09-30), junto com a guarda de _WIN32 em
        # src/msxdisk/gui/file_dialog.cpp (que ja' cobre o lado de
        # compilacao; isto cobre o lado de link).
        if (WIN32)
            target_link_libraries(${target} PRIVATE comdlg32)
        endif ()
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
        src/memmap/cpp/rom_guess.cpp
        src/memmap/fortran/rom_checksum.f90
)

# --- VDP (V9938/TMS9918, Fases 0.5-2) -- ver doc/vdp-spec.md ----------
#
# Motor "digital" do VDP em C (adaptado do fMSX, ver
# LICENSE-THIRD-PARTY.md): registradores, protocolo de porta 98h-9Bh,
# maquina de estados de scanline/interrupcao. VdpDevice (C++, design
# proprio) adapta isso para z80::IBus; z80::CompositeBus (Fase 0.5,
# src/z80/cpp/, tambem design proprio) e' o que permite o VDP e o mapa
# de memoria coexistirem no mesmo barramento do Z80Cpu. Tabela de
# conversao de paleta em Fortran, consumida desde a Fase 2 (escrita de
# paleta via porta 9Ah). Fase 2 acrescenta renderizacao de pixel de
# verdade para SCREEN 0/1/2 (vdp_render.c, adaptado de Common.h) e
# exportacao PPM (ppm_writer.cpp, design proprio) para inspecao/testes.
set(VDP_LIB_SOURCES
        src/vdp/core/vdp_state.c
        src/vdp/core/vdp_tables.c
        src/vdp/core/vdp_render.c
        src/vdp/core/vdp_sprites.c
        src/vdp/core/vdp_cmd.c
        src/vdp/cpp/ppm_writer.cpp
        src/vdp/fortran/palette_table.f90
)

# --- PPI i8255 + teclado (portas A8h-ABh) -- ver doc/ppi-spec.md -------
#
# Chip em C (adaptado de EMULib/I8255.c e dos casos A8h-ABh de MSX.c --
# ver LICENSE-THIRD-PARTY.md); PpiDevice (C++, design proprio) liga o
# chip ao z80::IBus e ao MemorySystem (slot primario). Tabela de posicao
# das teclas na matriz em Fortran (key_matrix.f90) e contagem de teclas
# pressionadas em Assembly dual-ABI (key_count.asm) -- as quatro
# linguagens do projeto, de novo, num unico modulo.
set(PPI_LIB_SOURCES
        src/ppi/core/ppi_state.c
        src/ppi/fortran/key_matrix.f90
        src/ppi/asm/key_count.asm
)

# --- PSG AY-3-8910 (portas A0h-A2h) -- ver doc/psg-spec.md -------------
#
# Motor em C (psg_state.c, adaptado de EMULib/AY8910.c -- ver
# LICENSE-THIRD-PARTY.md) que gera amostras PCM de verdade, avancado por
# ciclos de Z80; tabela de volume logaritmica em Fortran
# (volume_table.f90); PsgDevice e gravador de WAV (C++, design proprio).
set(PSG_LIB_SOURCES
        src/psg/core/psg_state.c
        src/psg/fortran/volume_table.f90
        src/psg/cpp/wav_writer.cpp
)

# --- Chip de som SCC (cartuchos Konami5/Gen8, 9800h-98FFh) -- ver doc/scc-spec.md
#
# Motor em C adaptado do SCC.c do fMSX (ver LICENSE-THIRD-PARTY.md) que gera
# amostras PCM; soma de canal em Assembly dual-ABI (render_channel.asm);
# tabela de volume linear em Fortran (scc_volume_table.f90). SccDevice (C++,
# header-only) liga o chip ao slot de cartucho via memmap::SlotCartIo.
set(SCC_LIB_SOURCES
        src/scc/core/scc_state.c
        src/scc/asm/render_channel.asm
        src/scc/fortran/scc_volume_table.f90
)

# --- Chip FM OPLL (MSX-MUSIC em 7Ch/7Dh e FM-PAC) -- ver doc/fm-spec.md
#
# Motor em C de 9 canais com 2 operadores (ym2413_state.c), timbres prontos
# (dados do fMSX), tabelas de seno/atenuacao em Fortran (ym2413_tables.f90) e
# soma de canal em Assembly dual-ABI (accumulate.asm). FmDevice (C++, header-only)
# liga o chip ao barramento de portas. O mapper do FM-PAC fica no memmap.
set(FM_LIB_SOURCES
        src/fm/core/ym2413_state.c
        src/fm/asm/accumulate.asm
        src/fm/fortran/ym2413_tables.f90
)

# --- Controladora de disquete WD2793 (DiskROM em 3:1) -- ver doc/fdc-spec.md
#
# Motor em C adaptado do WD1793.c do fMSX (so' imagens .dsk cruas);
# DiskImage (gravacao imediata no arquivo) e FdcDevice (registradores em
# 7FF8h-7FFFh do slot do DiskROM, via SlotMmio) em C++, design proprio.
set(FDC_LIB_SOURCES
        src/fdc/core/fdc_state.c
        src/fdc/cpp/disk_image.cpp
)

# --- Fita (.CAS e .TSX/.TZX, carregamento rapido e normal) -- ver doc/tape-spec.md
#
# Cursor de pulsos e framing KCS em C (tape_pulse.c, kcs_codec.c, design
# proprio); frequencia do pulso em Fortran (kcs_timing.f90); soma de
# pulsos (duracao total) em Assembly dual-ABI (pulse_sum.asm); leitores de
# .CAS e .TSX/.TZX e o motor (TapeEngine) em C++, tudo design proprio.
set(TAPE_LIB_SOURCES
        src/tape/core/tape_pulse.c
        src/tape/core/kcs_codec.c
        src/tape/fortran/kcs_timing.f90
        src/tape/asm/pulse_sum.asm
        src/tape/cpp/cas_reader.cpp
        src/tape/cpp/tzx_reader.cpp
        src/tape/cpp/tsx_writer.cpp
        src/tape/cpp/tape_device.cpp
)

# --- Maquina MSX1 completa + janela (fwmsx --msx) -- ver doc/machine-spec.md
#
# machine.cpp junta BIOS + slots + VDP + PPI + PSG + Z80 e avanca por
# quadros, sem janela (e' o que os testes usam). A CLI (cli.cpp) e a janela
# (MACHINE_WINDOW_SOURCES: GLFW + ImGui, ou o stub se FWMSX_MSXDISK_GUI=OFF)
# so' entram no executavel fwMSX.
set(MACHINE_LIB_SOURCES
        src/machine/machine.cpp
)

enable_testing()

add_executable(z80test tests/z80/smoke_test.cpp ${Z80_LIB_SOURCES})
target_include_directories(z80test PRIVATE src/z80)
set_target_properties(z80test PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/z80test)
if (MINGW)
    target_link_options(z80test PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME z80_smoke COMMAND z80test)

add_executable(z80dbgtest tests/z80/debug_session_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES} ${VDP_LIB_SOURCES} ${PPI_LIB_SOURCES} ${PSG_LIB_SOURCES})
target_include_directories(z80dbgtest PRIVATE src/z80 src/memmap src/vdp)
set_target_properties(z80dbgtest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/z80dbgtest)
if (MINGW)
    target_link_options(z80dbgtest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME z80_debug_session COMMAND z80dbgtest)

add_executable(memmaptest tests/z80/memmap_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES} ${VDP_LIB_SOURCES} ${PPI_LIB_SOURCES} ${PSG_LIB_SOURCES})
target_include_directories(memmaptest PRIVATE src/z80 src/memmap src/vdp)
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

# vdptest: VDP "digital" (Fase 1) + Fase 0.5 (CompositeBus) -- ver
# doc/vdp-spec.md. Mesmo padrao de memmaptest (FWMSX_SOURCE_DIR para o
# teste de aceitacao com a BIOS real).
add_executable(vdptest tests/z80/vdp_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES} ${VDP_LIB_SOURCES} ${PPI_LIB_SOURCES} ${PSG_LIB_SOURCES})
target_include_directories(vdptest PRIVATE src/z80 src/memmap src/vdp)
target_compile_definitions(vdptest PRIVATE FWMSX_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(vdptest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/vdptest)
if (MINGW)
    target_link_options(vdptest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME vdp_digital COMMAND vdptest)

# ppitest: PPI i8255 + teclado -- ver doc/ppi-spec.md.
add_executable(ppitest tests/z80/ppi_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES} ${VDP_LIB_SOURCES} ${PPI_LIB_SOURCES} ${PSG_LIB_SOURCES})
target_include_directories(ppitest PRIVATE src/z80 src/memmap src/vdp src/ppi)
target_compile_definitions(ppitest PRIVATE FWMSX_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(ppitest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/ppitest)
if (MINGW)
    target_link_options(ppitest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME ppi_keyboard COMMAND ppitest)

# psgtest: PSG AY-3-8910 -- ver doc/psg-spec.md.
add_executable(psgtest tests/z80/psg_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES} ${VDP_LIB_SOURCES} ${PPI_LIB_SOURCES} ${PSG_LIB_SOURCES})
target_include_directories(psgtest PRIVATE src/z80 src/memmap src/vdp src/ppi src/psg)
target_compile_definitions(psgtest PRIVATE FWMSX_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(psgtest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/psgtest)
if (MINGW)
    target_link_options(psgtest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME psg_sound COMMAND psgtest)

# machinetest: a maquina MSX1 completa em quadros, sem janela -- ver
# doc/machine-spec.md.
add_executable(machinetest tests/z80/machine_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES} ${VDP_LIB_SOURCES} ${PPI_LIB_SOURCES} ${PSG_LIB_SOURCES} ${MACHINE_LIB_SOURCES} ${SCC_LIB_SOURCES} ${FM_LIB_SOURCES} ${FDC_LIB_SOURCES} ${TAPE_LIB_SOURCES})
target_include_directories(machinetest PRIVATE src/z80 src/memmap src/vdp src/ppi src/psg src/machine)
target_compile_definitions(machinetest PRIVATE FWMSX_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(machinetest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/machinetest)
if (MINGW)
    target_link_options(machinetest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME machine_frames COMMAND machinetest)

# fdctest: controladora de disquete WD2793 -- ver doc/fdc-spec.md.
add_executable(fdctest tests/z80/fdc_test.cpp ${MEMMAP_LIB_SOURCES} ${FDC_LIB_SOURCES})
target_include_directories(fdctest PRIVATE src/memmap src/fdc)
set_target_properties(fdctest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/fdctest)
if (MINGW)
    target_link_options(fdctest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME fdc_wd2793 COMMAND fdctest)

# tapetest: leitor de .CAS e .TSX/.TZX, cursor de pulsos (modo normal),
# gancho de BIOS "ED FE" (modo rapido) e a entrada de cassete no PSG (R14,
# bit 7) -- ver doc/tape-spec.md.
add_executable(tapetest tests/z80/tape_test.cpp ${Z80_LIB_SOURCES} ${MEMMAP_LIB_SOURCES} ${PSG_LIB_SOURCES} ${TAPE_LIB_SOURCES})
target_include_directories(tapetest PRIVATE src/z80 src/memmap src/psg src/tape)
set_target_properties(tapetest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/tapetest)
if (MINGW)
    target_link_options(tapetest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME tape_load COMMAND tapetest)

# vdp2test: VDP MSX2 (V9938) -- modos SCREEN 3-8, sprites de modo 2 e motor de
# comandos -- ver doc/vdp-spec.md (Fases 4 e 5).
add_executable(vdp2test tests/z80/vdp2_test.cpp ${VDP_LIB_SOURCES})
target_include_directories(vdp2test PRIVATE src/vdp)
set_target_properties(vdp2test PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/vdp2test)
if (MINGW)
    target_link_options(vdp2test PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME vdp_msx2 COMMAND vdp2test)

# msx2test: maquina MSX2 (mapper de RAM, RTC, V9938 pelo BASIC e disco) -- ver
# doc/msx2-spec.md.
add_executable(msx2test tests/z80/msx2_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES} ${VDP_LIB_SOURCES} ${PPI_LIB_SOURCES} ${PSG_LIB_SOURCES} ${MACHINE_LIB_SOURCES} ${SCC_LIB_SOURCES} ${FM_LIB_SOURCES} ${FDC_LIB_SOURCES} ${TAPE_LIB_SOURCES})
target_include_directories(msx2test PRIVATE src/z80 src/memmap src/vdp src/ppi src/psg src/machine src/rtc)
target_compile_definitions(msx2test PRIVATE FWMSX_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(msx2test PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/msx2test)
if (MINGW)
    target_link_options(msx2test PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME msx2_machine COMMAND msx2test)

# audiotest: buffer circular, PSG ao vivo e dispositivo de audio -- ver
# doc/audio-spec.md.
add_executable(audiotest tests/z80/audio_test.cpp ${Z80_LIB_SOURCES} ${Z80_DEBUG_SESSION_SOURCES} ${MEMMAP_LIB_SOURCES} ${VDP_LIB_SOURCES} ${PPI_LIB_SOURCES} ${PSG_LIB_SOURCES} ${MACHINE_LIB_SOURCES} ${SCC_LIB_SOURCES} ${FM_LIB_SOURCES} ${AUDIO_LIB_SOURCES} ${FDC_LIB_SOURCES} ${TAPE_LIB_SOURCES})
target_include_directories(audiotest PRIVATE src/z80 src/memmap src/vdp src/ppi src/psg src/machine src/audio)
target_compile_definitions(audiotest PRIVATE FWMSX_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(audiotest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/audiotest)
audio_link_target(audiotest)
if (MINGW)
    target_link_options(audiotest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME audio_live COMMAND audiotest)

# scctest: chip SCC (protocolo de ativacao no cartucho, onda, frequencia,
# volume e o kernel de Assembly contra a referencia em C) -- ver doc/scc-spec.md.
add_executable(scctest tests/z80/scc_test.cpp ${MEMMAP_LIB_SOURCES} ${SCC_LIB_SOURCES})
set_target_properties(scctest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/scctest)
if (MINGW)
    target_link_options(scctest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME scc_sound COMMAND scctest)

# fmtest: chip FM (tabelas, kernel de soma contra C, frequencia, volume, envelope,
# portas 7Ch/7Dh) -- ver doc/fm-spec.md.
add_executable(fmtest tests/z80/fm_test.cpp ${FM_LIB_SOURCES})
set_target_properties(fmtest PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/fmtest)
if (MINGW)
    target_link_options(fmtest PRIVATE -static -static-libgcc -static-libstdc++ -static-libgfortran)
endif ()

add_test(NAME fm_sound COMMAND fmtest)

# --- Banco de ROMs (SQLite) e downloads -- ver doc/romdb-spec.md
#
# miniz (dominio publico) para extrair os ZIPs; o download usa o programa `curl`
# (Windows 10+ e Linux). Banco: store/romdb.cpp (SQLite amalgamation, ja' usada
# pelo msxdisk). Fluxos (fMSX, file-hunter, Vampier) em service.cpp; CLI em cli.cpp.
FetchContent_Declare(
        miniz
        URL https://github.com/richgel999/miniz/archive/refs/tags/3.0.2.zip
)
FetchContent_GetProperties(miniz)
if (NOT miniz_POPULATED)
    FetchContent_Populate(miniz)
endif()
# miniz.h inclui "miniz_export.h" (gerado pelo CMake do miniz, que nao usamos): para
# uma lib estatica basta a macro MINIZ_EXPORT vazia.
file(WRITE ${CMAKE_BINARY_DIR}/miniz_include/miniz_export.h
        "#pragma once\n#define MINIZ_EXPORT\n")
add_library(miniz STATIC
        ${miniz_SOURCE_DIR}/miniz.c
        ${miniz_SOURCE_DIR}/miniz_tdef.c
        ${miniz_SOURCE_DIR}/miniz_tinfl.c
        ${miniz_SOURCE_DIR}/miniz_zip.c)
target_include_directories(miniz PUBLIC ${miniz_SOURCE_DIR} ${CMAKE_BINARY_DIR}/miniz_include)

set(ROMDB_LIB_SOURCES
        src/romdb/core/hash.cpp
        src/romdb/store/classify.cpp
        src/romdb/net/web.cpp
        src/romdb/net/listing.cpp
        src/romdb/archive/unzip.cpp
        src/romdb/store/romdb.cpp
        src/romdb/service.cpp
        src/romdb/cli.cpp
)

# romdbtest: hashes, classificacao, leitura de indices, ZIP (ida e volta) e o banco
# (CRUD, busca, CARTS.SHA e dump do Vampier) -- sem rede.
add_executable(romdbtest tests/romdb/romdb_test.cpp ${ROMDB_LIB_SOURCES})
target_include_directories(romdbtest PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(romdbtest PRIVATE sqlite3 miniz)
if (MINGW)
    target_link_options(romdbtest PRIVATE -static -static-libgcc -static-libstdc++)
endif ()
add_test(NAME romdb_store COMMAND romdbtest)

# videofiltertest: filtros de video da janela (interpolacao, scanlines, cor) -- sem janela.
add_executable(videofiltertest tests/z80/video_filter_test.cpp src/machine/gui/video_filters.cpp)
target_include_directories(videofiltertest PRIVATE src/machine/gui)
add_test(NAME video_filters COMMAND videofiltertest)

# --- fwMSX: esqueleto multi-linguagem do emulador (Fase 0) ---
#
# "fwmsx --msxdisk ..." repassa pro msxdisk embutido (mesmos
# MSXDISK_LIB_SOURCES acima) -- ver doc/SPEC.md, secao 5.1.
# "fwmsx --z80dbg" abre o REPL de depuracao do Z80 (Fase 4, ver
# doc/z80-core-spec.md) -- unico lugar que compila z80_debug_shell.cpp
# (usa replxx, por isso fora de Z80_DEBUG_SESSION_SOURCES). "--z80dbg
# --slots" liga o mapa de memoria de verdade (Fase 1, ver
# doc/memory-map-spec.md) em vez da RAM plana default. "--z80dbg --slots
# --vdp" liga tambem o VDP "digital" (Fase 0.5/1, ver doc/vdp-spec.md).
add_executable(fwMSX
        src/cpp/main.cpp
        src/cpp/init_cpp.cpp
        src/c/init_c.c
        src/asm/init_asm.asm
        src/fortran/init_fortran.f90
        ${MSXDISK_LIB_SOURCES}
        ${ROMDB_LIB_SOURCES}
        ${Z80_LIB_SOURCES}
        ${Z80_DEBUG_SESSION_SOURCES}
        ${MEMMAP_LIB_SOURCES}
        ${VDP_LIB_SOURCES} ${PPI_LIB_SOURCES} ${PSG_LIB_SOURCES}
        ${MACHINE_LIB_SOURCES} ${SCC_LIB_SOURCES} ${FM_LIB_SOURCES} ${FDC_LIB_SOURCES} ${AUDIO_LIB_SOURCES} ${TAPE_LIB_SOURCES} src/machine/cli.cpp ${MACHINE_WINDOW_SOURCES}
        src/z80/debug/z80_debug_shell.cpp
)

target_link_libraries(fwMSX PRIVATE miniz)
target_include_directories(fwMSX PRIVATE
        src/cpp
        src/c
        src/asm
        src/fortran
        src/common
        src/z80
        src/memmap
        src/vdp
        src/audio
)
msxdisk_link_target(fwMSX)
audio_link_target(fwMSX)
set_target_properties(fwMSX PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod/fwMSX)
