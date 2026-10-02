#!/usr/bin/env bash
#
# fwMSX - script de build (Bash/Linux)
#
# Equivalente ao build.ps1 (Windows/MSYS2 UCRT64), para compilar e testar
# o projeto tambem em Linux -- em particular para validar de verdade a
# branch elf64/SysV AMD64 do Assembly dual-ABI
# (src/z80/asm/block_ops.asm), que ate agora so foi montada
# (`nasm -f elf64`) e nunca linkada/executada, por falta de maquina Linux
# no desenvolvimento original (ver doc/z80-core-spec.md, Fase 3).
#
# Uso:
#   ./build.sh                # configura, compila, testa (ctest) e empacota
#   ./build.sh --no-gui       # compila sem a GUI (Dear ImGui/GLFW/OpenGL3)
#   ./build.sh --no-test      # pula o `ctest` ao final
#
# Pre-requisitos (nomes de pacote no Debian/Ubuntu -- ajuste para o seu
# gerenciador em outras distros: dnf/pacman/zypper/etc.):
#   sudo apt install build-essential gfortran nasm cmake ninja-build
# Para a GUI (Dear ImGui + GLFW + OpenGL3), tambem:
#   sudo apt install libgl1-mesa-dev libx11-dev libxrandr-dev \
#                     libxinerama-dev libxcursor-dev libxi-dev
# (Wayland e' opcional: sem `wayland-scanner` o CMake compila o GLFW so' com
# X11, que tambem roda em sessoes Wayland via XWayland. Para Wayland nativo:
#   sudo apt install libwayland-dev libxkbcommon-dev wayland-protocols)
#
# Ver doc/MANUAL.md para instrucoes completas (o essencial de Linux e o
# mesmo fluxo do Windows: CMake + Ninja, mesma arvore de fontes).

set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# build-linux/ (NAO build/) de proposito: build/ e' onde o build.ps1
# (Windows) grava o CMakeCache.txt dele. Num checkout compartilhado entre
# Windows nativo e WSL (ex.: repo em C:\... acessado tambem como
# /mnt/c/... de dentro do WSL), o MESMO diretorio fisico apareceria com
# dois caminhos absolutos diferentes pro CMake -- que entao recusa
# reconfigurar ("CMakeCache.txt directory is different than the directory
# ... where CMakeCache.txt was created"). Diretorios separados evitam
# esse conflito por completo, sem precisar apagar cache na mao toda vez
# que se alterna de ambiente.
build_dir="$root/build-linux"
dist_dir="$root/dist"

gui_flag=()
run_tests=1
for arg in "$@"; do
    case "$arg" in
        --no-gui)
            gui_flag=("-DFWMSX_MSXDISK_GUI=OFF")
            ;;
        --no-test)
            run_tests=0
            ;;
        *)
            echo "Uso: $0 [--no-gui] [--no-test]" >&2
            exit 1
            ;;
    esac
done

echo "==> Verificando toolchain..."
missing=()
for tool in gcc g++ gfortran nasm cmake ninja; do
    command -v "$tool" >/dev/null 2>&1 || missing+=("$tool")
done
if [ "${#missing[@]}" -ne 0 ]; then
    echo "Faltando no PATH: ${missing[*]}." >&2
    echo "Veja o cabecalho deste script ou doc/MANUAL.md para os pacotes necessarios." >&2
    exit 1
fi

echo "==> Configurando (CMake + Ninja)..."
cmake -S "$root" -B "$build_dir" -G Ninja "${gui_flag[@]}"

echo "==> Compilando..."
cmake --build "$build_dir"

exe="$dist_dir/fwMSX"
if [ ! -f "$exe" ]; then
    echo "Build concluido mas $exe nao foi gerado." >&2
    exit 1
fi

msxdisk_exe="$dist_dir/msxdisk"
if [ ! -f "$msxdisk_exe" ]; then
    echo "Build concluido mas $msxdisk_exe nao foi gerado." >&2
    exit 1
fi

if [ "$run_tests" -eq 1 ]; then
    echo "==> Rodando testes (ctest)..."
    ctest --test-dir "$build_dir" --output-on-failure
fi

# Le a versao corrente direto de src/common/version.h para nomear o pacote
# (sed em vez de grep -P, pra nao depender de PCRE estar disponivel).
version_header="$root/src/common/version.h"
major=$(sed -n 's/.*FWMSX_VERSION_MAJOR[[:space:]]\+\([0-9]\+\).*/\1/p' "$version_header")
minor=$(sed -n 's/.*FWMSX_VERSION_MINOR[[:space:]]\+\([0-9]\+\).*/\1/p' "$version_header")
patch=$(sed -n 's/.*FWMSX_VERSION_PATCH[[:space:]]\+\([0-9]\+\).*/\1/p' "$version_header")
version="$major.$minor.$patch"

pkg_name="fwMSX-$version-linux"
stage_dir="$dist_dir/_package"
tar_path="$dist_dir/$pkg_name.tar.gz"

echo "==> Empacotando $pkg_name.tar.gz..."
rm -rf "$stage_dir"
mkdir -p "$stage_dir"

cp "$exe" "$stage_dir/"
cp "$msxdisk_exe" "$stage_dir/"
cp "$root/README.md" "$stage_dir/"
cp "$root/LICENSE" "$stage_dir/"
cp "$root/LICENSE-THIRD-PARTY.md" "$stage_dir/"
cp "$root/doc/MANUAL.md" "$stage_dir/"
cp "$root/doc/RELEASE.md" "$stage_dir/"

rm -f "$tar_path"
tar -czf "$tar_path" -C "$stage_dir" .
rm -rf "$stage_dir"

echo "==> Pronto:"
echo "    $exe"
echo "    $msxdisk_exe"
echo "    $tar_path"
