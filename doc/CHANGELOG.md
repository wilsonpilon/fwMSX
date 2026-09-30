# Changelog

Resumo das alteracoes entre versoes do fwMSX, ligado ao
[README.md](../README.md). Formato inspirado em
[Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/). Para o
detalhamento completo de cada versao (nome do jogo de MSX + subtitulo,
notas de build) veja [RELEASE.md](RELEASE.md); para a especificacao viva
e o historico de fases, veja [SPEC.md](SPEC.md).

## [1.3.0] - 2026-09-30 - "SD Snatcher: Núcleo do Z80"

Início do core de emulação propriamente dito: a CPU Z80, fiel ao fMSX,
usando as quatro linguagens do projeto de verdade (não só simbolicamente)
-- ver [doc/z80-core-spec.md](z80-core-spec.md) para o histórico completo
de fases.

### Adicionado
- **Motor de despacho da CPU Z80 em C** (`src/z80/core/`), adaptado de
  `resource/fMSX/Z80/` (registradores, tabelas de ciclo/flag, opcodes com
  e sem prefixo `CB`/`ED`/`DD`/`FD`/`DDCB`/`FDCB`), por trás de um
  `Z80Bus` próprio (callbacks + contexto, no lugar das funções globais do
  fMSX -- permite múltiplas instâncias de CPU no mesmo processo).
- **Wrapper de orquestração em C++** (`Z80Cpu`/`IBus`, `src/z80/cpp/`):
  `reset()`/`run(ciclos)`/`interrupt()` e acesso a registradores, sem
  expor a união de par de registrador na API pública.
- **Tabelas de flag geradas em Fortran** (`src/z80/fortran/flag_tables.f90`):
  `ZSTable`/`PZSTable` calculadas com `POPCNT`, chamadas uma única vez no
  reset da CPU -- nunca no caminho quente do despachante.
- **Aceleração de `LDIR`/`LDDR` em Assembly** (`src/z80/asm/block_ops.asm`):
  primeiro `.asm` do projeto com Win64 **e** SysV AMD64 (Linux) no mesmo
  arquivo-fonte (`%ifidn __OUTPUT_FORMAT__`), usada só quando o bloco
  inteiro cai em RAM plana do host -- o loop byte-a-byte original do
  fMSX continua como caminho de reserva sempre que isso não vale.
- **Depurador embutido em `fwMSX.exe`** (`fwmsx --z80dbg`): REPL
  (replxx) com `reset`/`regs`/`step`/`run`/`break`/`clear`/`breaks`/
  `mem`/`peek`/`poke`/`load`/`fill`/`disasm`, rodando sobre uma RAM
  plana de 64KB de teste -- ainda sem VDP/PSG/mapa de memória real.
- **Desmontador Z80** (`src/z80/debug/z80_disasm.*`), adaptado do
  desmontador já existente em `resource/fMSX/Z80/Debug.c`, com três
  correções cosméticas documentadas (nenhuma afeta execução/timing da
  CPU) em relação ao original.
- 203 verificações automatizadas novas (`ctest -R z80`): 168 no motor
  (`z80_smoke`) e 35 no depurador/desmontador (`z80_debug_session`),
  incluindo uma varredura de completude sobre 2044 combinações de opcode
  do desmontador.

### Notas
- Ainda não existe VDP, PSG nem mapa de memória de uma máquina MSX real
  -- o núcleo roda isolado sobre RAM de teste. Ver `doc/z80-core-spec.md`
  para o que falta.

## [1.2.1] - 2026-09-29 - "Metal Gear: Ajustes de Campo"

Correções e polimento na GUI do `msxdisk` (v1.2.0), encontrados testando
de verdade em janela gráfica.

### Adicionado
- **Ejetar disco** (`Arquivo > Ejetar` ou `F12`): descarrega a imagem da
  memória sem apagar o arquivo; confirma antes só se houver alterações
  não salvas.
- **Novo/Abrir/Salvar Como** na GUI agora usam o diálogo nativo de
  arquivo do Windows (`GetOpenFileNameW`/`GetSaveFileNameW`), com
  navegação de pastas de verdade, em vez de uma caixa de texto simples.

### Corrigido
- **Diálogos de Novo/Abrir/Salvar/Renomear/Nova Pasta/Excluir não
  abriam** na GUI: `ImGui::OpenPopup`/`BeginPopupModal` calculam o ID do
  popup a partir da janela "atual" no momento da chamada — como esses
  diálogos eram disparados de dentro do menu (uma sub-janela) ou antes da
  janela principal existir no frame, o ID nunca batia com o que o
  `BeginPopupModal` esperava. Corrigido centralizando o `OpenPopup` real
  num único ponto, sempre na mesma janela.
- **Renomear/Nova Pasta/Excluir agiam no painel ou item errado**: marcar
  um arquivo só pela caixinha de seleção (sem clicar no nome da linha)
  não atualizava qual painel estava "ativo" — as teclas de função
  continuavam operando no painel local por padrão. Agora marcar a
  caixinha também ativa o painel e seleciona a linha.

## [1.2.0] - 2026-09-29 - "Maze of Galious: Gerenciador de Discos"

### Adicionado
- **msxdisk**: utilitário completo de manipulação de imagens de disco MSX
  (`.dsk`, FAT12, MSX-DOS 1 e 2 com subdiretórios), num único executável
  (`dist/msxdisk.exe`) com quatro modos de uso -- ver
  [doc/msxdisk-spec.md](msxdisk-spec.md) para o histórico completo de
  fases:
  - **CLI one-shot**: `create`, `list`, `add`, `extract`, `delete`,
    `rename`, `mkdir`, `rmdir`, `copy`, `saveas`, `info`.
  - **Shell interativo** (`msxdisk` sem argumentos, ou `--cli`): sessão
    estilo FTP (`load`/`save`/`saveas`), `put`/`get`/`mput`/`mget` com
    confirmação por arquivo, comandos locais de filesystem (`ls`, `cd`,
    `md`, `rm`, `pwd`), histórico persistente (replxx).
  - **TUI** (`--tui`, estilo Norton Commander/XTree, FTXUI): duas colunas
    (local ↔ imagem), marcar e enviar/receber arquivos, criar/abrir/
    salvar, renomear/criar pasta/excluir, troca de tema.
  - **GUI** (`--gui`, Dear ImGui + GLFW + OpenGL3): mesma interface de
    duas colunas em janela gráfica, com identidade visual moderna própria
    (paleta escura/clara com acento azul, cantos arredondados, fonte
    Segoe UI) -- diferente de propósito do visual retrô da TUI.
  - De dentro do shell, `tui`/`call tui` e `gui`/`call gui` trocam de
    modo sem sair do processo.
- Configuração, temas e metadados de imagem do msxdisk guardados num
  banco SQLite (`~/.msxdisk/config.sqlite3`), compartilhado entre TUI e
  GUI (tema ativo, último diretório/imagem, notas por disco).
- `fwmsx --msxdisk <argumentos>`: o `fwMSX.exe` agora também dá acesso a
  todos os modos do msxdisk embutido, sem precisar do executável
  separado -- `msxdisk.exe` standalone continua existindo pra quem só
  quer o utilitário de disco.
- Nova opção de build `FWMSX_MSXDISK_GUI` (default `ON`) para compilar
  sem a GUI (sem depender de GLFW/OpenGL) quando não for necessária.

### Corrigido
- `DiskImage::RenameFile` aceitava renomear um arquivo para um nome
  diferente do atual normalmente, mas recusava com "já existe" ao
  renomear para o **próprio nome atual** (a busca por conflito achava a
  própria entrada). Agora é tratado como no-op bem-sucedido.

## [1.1.2] - 2026-09-28 - "Nemesis: Renomeacao"

### Alterado
- Renomeados os arquivos e funcoes de cada modulo, de `module_<lang>.*` /
  `load_module_<lang>()` para `init_<lang>.*` / `init_<lang>()`, deixando
  claro que cada funcao e o ponto de "inicializacao" daquele modulo:
  - `src/cpp/module_cpp.{h,cpp}` -> `src/cpp/init_cpp.{h,cpp}`
    (`load_module_cpp` -> `init_cpp`)
  - `src/c/module_c.{h,c}` -> `src/c/init_c.{h,c}` (`load_module_c` ->
    `init_c`)
  - `src/asm/module_asm.{h,asm}` -> `src/asm/init_asm.{h,asm}`
    (`load_module_asm` -> `init_asm`; label interno `fmt_asm` ->
    `fmt_init_asm`)
  - `src/fortran/module_fortran.{h,f90}` -> `src/fortran/init_fortran.{h,f90}`
    (`load_module_fortran` -> `init_fortran`; modulo Fortran interno
    `module_fortran` -> `mod_init_fortran`, para nao colidir com o nome
    da funcao)
- `CMakeLists.txt` e `src/cpp/main.cpp` atualizados para os novos nomes de
  arquivo/funcao.
- Documentacao (`SPEC.md`, `MANUAL.md`, `README.md`) atualizada para
  refletir a nova convencao de nomes.

## [1.1.1] - 2026-09-28 - "Knightmare: Alicerce"

### Adicionado
- Reestruturacao completa do projeto em `src/` (por linguagem: `cpp/`,
  `c/`, `asm/`, `fortran/`, `common/`), `doc/`, `dist/` e `resource/`.
- `main.cpp` (C++) como ponto de entrada do projeto, recebendo o nome do
  produto e a versao (major.minor.patch) como parametros de linha de
  comando, com defaults em `src/common/version.h`.
- Quatro modulos de "carregamento", um por linguagem, cada um imprimindo
  sua propria mensagem com a versao entre colchetes e devolvendo uma
  assinatura hexadecimal: C++ (`load_module_cpp`, `0x0001`), C
  (`load_module_c`, `0x0002`), Assembly (`load_module_asm`, `0x0003`,
  NASM/ABI Win64) e Fortran (`load_module_fortran`, `0x0004`).
- Aviso de copyright `Copyright (c) 1972-2026 Cybernostra, Inc.` impresso
  no inicio da execucao, e resumo das quatro assinaturas impresso ao
  final.
- Documentacao viva do projeto: `SPEC.md`, `MANUAL.md`, `CHANGELOG.md`
  (este arquivo) e `RELEASE.md`.
- `CMakeLists.txt` raiz (C/C++/ASM_NASM/Fortran) e `build.ps1`
  (compilacao via MSYS2 UCRT64 + empacotamento automatico do ZIP em
  `dist/`).

### Corrigido
- Saida do modulo Fortran aparecendo fora de ordem (buffer de E/S proprio
  do `libgfortran`, sem sincronizacao automatica com `std::cout`/`printf`)
  -- corrigido com `FLUSH(output_unit)` explicito apos o `PRINT`.
- Assinaturas hexadecimais impressas erradas (`0x1000` em vez de
  `0x0001` etc.) -- o alinhamento `std::left` usado para a coluna do
  rotulo "vazava" para a coluna do valor hexadecimal; corrigido com
  `std::right` explicito antes do `std::setw` do valor.

### Removido
- Prototipo original solto na raiz do repositorio (`main.cpp`, `hello.c`,
  `hello.h`, `module_c.*`, `module_cpp.*`, `module_fortran.f90`,
  `rotinas.asm`), migrado e reescrito dentro de `src/`.
