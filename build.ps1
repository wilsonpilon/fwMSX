<#
    fwMSX - script de build (PowerShell)

    Compila o projeto usando o toolchain MSYS2 UCRT64 (gcc/g++/gfortran/nasm),
    gera dist\fwMSX.exe e empacota o ZIP de distribuicao em dist\.

    Uso:
        .\build.ps1

    Pre-requisitos: MSYS2 instalado em C:\msys64 com o grupo de pacotes
    ucrt64 (mingw-w64-ucrt-x86_64-toolchain, -cmake, -ninja, -nasm).
    Ver doc\MANUAL.md para detalhes.
#>

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ucrt64 = "C:\msys64\ucrt64\bin"

if (-not (Test-Path $ucrt64)) {
    Write-Error "Toolchain MSYS2 UCRT64 nao encontrado em $ucrt64. Veja doc\MANUAL.md."
    exit 1
}

# Garante que o compilador/linker/montador corretos (UCRT64) sejam usados,
# a frente de qualquer outro gcc/cmake que esteja no PATH do sistema.
$env:PATH = "$ucrt64;$env:PATH"

$buildDir = Join-Path $root "build"
$distDir  = Join-Path $root "dist"

Write-Host "==> Configurando (CMake + Ninja)..." -ForegroundColor Cyan
cmake -S $root -B $buildDir -G "Ninja"

Write-Host "==> Compilando..." -ForegroundColor Cyan
cmake --build $buildDir

$exe = Join-Path $distDir "fwMSX.exe"
if (-not (Test-Path $exe)) {
    Write-Error "Build concluido mas $exe nao foi gerado."
    exit 1
}

# Le a versao corrente diretamente de src\common\version.h para nomear o ZIP.
$versionHeader = Get-Content (Join-Path $root "src\common\version.h") -Raw
$major = [regex]::Match($versionHeader, 'FWMSX_VERSION_MAJOR\s+(\d+)').Groups[1].Value
$minor = [regex]::Match($versionHeader, 'FWMSX_VERSION_MINOR\s+(\d+)').Groups[1].Value
$patch = [regex]::Match($versionHeader, 'FWMSX_VERSION_PATCH\s+(\d+)').Groups[1].Value
$version = "$major.$minor.$patch"

$zipPath = Join-Path $distDir "fwMSX-$version.zip"
$stageDir = Join-Path $distDir "_package"

Write-Host "==> Empacotando fwMSX-$version.zip..." -ForegroundColor Cyan
if (Test-Path $stageDir) { Remove-Item $stageDir -Recurse -Force }
New-Item -ItemType Directory -Path $stageDir | Out-Null

Copy-Item $exe $stageDir
Copy-Item (Join-Path $root "README.md") $stageDir
Copy-Item (Join-Path $root "LICENSE") $stageDir
Copy-Item (Join-Path $root "doc\MANUAL.md") $stageDir
Copy-Item (Join-Path $root "doc\RELEASE.md") $stageDir

if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
Compress-Archive -Path (Join-Path $stageDir "*") -DestinationPath $zipPath
Remove-Item $stageDir -Recurse -Force

Write-Host "==> Pronto:" -ForegroundColor Green
Write-Host "    $exe"
Write-Host "    $zipPath"
