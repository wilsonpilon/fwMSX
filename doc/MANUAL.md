# fwMSX -- Manual de compilacao e execucao

> Documento ainda simples: por enquanto so cobre como compilar e executar
> o esqueleto multi-linguagem da fase atual. Vai crescer conforme o
> emulador for tomando forma (ver [SPEC.md](SPEC.md)).

## Pre-requisitos

- Windows 10/11.
- [MSYS2](https://www.msys2.org/) instalado em `C:\msys64`.
- Toolchain **UCRT64** do MSYS2, com os seguintes pacotes:

  ```bash
  pacman -S \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-gcc-fortran \
    mingw-w64-ucrt-x86_64-nasm \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja
  ```

  Todo o toolchain (gcc, g++, gfortran, nasm, cmake, ninja) deve vir do
  **mesmo** grupo UCRT64 -- evita misturar versoes/ABIs de compiladores
  diferentes na mesma build.

## Compilar

### Opcao 1 -- script pronto (recomendado)

No PowerShell, a partir da raiz do projeto:

```powershell
.\build.ps1
```

O script:
1. Coloca `C:\msys64\ucrt64\bin` no inicio do `PATH` (so para o processo
   do script, sem alterar o `PATH` do sistema);
2. Configura o build com CMake + Ninja em `build\`;
3. Compila e gera `dist\fwMSX.exe`;
4. Le a versao corrente em `src\common\version.h` e empacota
   `dist\fwMSX-X.Y.Z.zip` com o executavel, `README.md`, `LICENSE`,
   `doc\MANUAL.md` e `doc\RELEASE.md`.

### Opcao 2 -- manual (MSYS2 UCRT64 shell)

Abra o **"MSYS2 UCRT64"** (nao o MSYS2 normal, nem o MINGW64) a partir do
menu iniciar, va ate a pasta do projeto e rode:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

O executavel fica em `dist/fwMSX.exe`.

## Executar

```powershell
.\dist\fwMSX.exe
```

Saida esperada (com os valores default de `src\common\version.h`):

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.1.1]
----------------------------------------
Loading module... CPP [v 1.1.1]
Loading module...C [v 1.1.1]
Loading module...Assembly [v 1.1.1]
Loading module Fortran [v 1.1.1]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Parametros opcionais

`fwMSX.exe` aceita, na linha de comando, o nome do produto e a versao a
serem exibidos (sobrescrevendo os defaults de `version.h`):

```powershell
.\dist\fwMSX.exe fwMSX 2 0 5
```

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 2.0.5]
----------------------------------------
Loading module... CPP [v 2.0.5]
Loading module...C [v 2.0.5]
Loading module...Assembly [v 2.0.5]
Loading module Fortran [v 2.0.5]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

## Distribuicao

`dist\fwMSX.exe` e linkado estaticamente (`-static -static-libgcc
-static-libstdc++ -static-libgfortran`) e depende apenas do **UCRT**
(`ucrtbase.dll`), nativo do Windows 10 versao 1607 ou mais recente -- ou
seja, roda em outra maquina Windows sem precisar instalar o MSYS2 nem
copiar DLLs. O ZIP gerado em `dist\fwMSX-X.Y.Z.zip` contem tudo o que e
necessario para rodar (executavel + documentacao + licenca).

## Problemas comuns

- **`cmake` ou `gcc` errado sendo usado / erros estranhos de link**: quase
  sempre e outro toolchain (ex.: FPC, Visual Studio) na frente do UCRT64
  no `PATH`. Use `.\build.ps1` (que forca o UCRT64 na frente) ou abra o
  shell **MSYS2 UCRT64** dedicado.
- **Saida do modulo Fortran aparece fora de ordem**: se voce mexer em
  `src/fortran/init_fortran.f90`, mantenha o `FLUSH(output_unit)` apos
  o `PRINT` -- o runtime do Fortran usa buffer de E/S proprio, separado
  do `std::cout`/`printf` do C/C++ (detalhes em
  [SPEC.md](SPEC.md#32-notas-de-implementacao-para-quem-retomar-o-projeto)).
