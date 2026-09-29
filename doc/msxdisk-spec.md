# msxdisk — especificação e acompanhamento de fases (documento vivo)

> Sub-especificação de `doc/SPEC.md`, dedicada ao utilitário **msxdisk**.
> Atualizado a cada sub-fase concluída — marque `[x]` no que já foi feito e
> mantenha `[ ]` no que falta, sem apagar histórico. Serve para retomar o
> trabalho exatamente de onde parou.

## 1. Objetivo

Construir o **msxdisk**: utilitário de manipulação de imagens de disco
`.DSK` no padrão MSX (FAT12, MSX-DOS 1 e MSX-DOS 2 com subdiretórios), com
dois papéis:

1. **Embutido no fwMSX** — invocável de dentro do emulador como utilitário
   interno de gerenciamento de disco.
2. **Standalone** — executável independente para quem só quer manipular
   imagens `.DSK`, sem o emulador.

Modos de operação previstos (ver fases 1, 3, 4, 5): CLI one-shot (CLI11),
shell interativo (replxx), TUI (FTXUI) e GUI (Dear ImGui + GLFW3/OpenGL3).

Como todo o projeto fwMSX, o msxdisk também precisa exercitar código real
em **C++, C, Assembly e Fortran** — ver a divisão por linguagem na seção 3.

## 2. Licenciamento e material de referência (regra permanente)

O fwMSX é **BSD-3-Clause**. Em `resource/` (apenas leitura/estudo, nunca
compilado) temos:

- `resource/DiskUtilities/` — dois autores distintos, tratados de forma
  diferente:
  - `Boot.h` é contribuição **direta de Marat Fayzullin**, o próprio autor
    do fMSX (mesmo autor que deu o aval citado em `README.md` para esta
    adaptação/evolução do fMSX em fwMSX). Por isso o **setor de boot real
    do MSX-DOS 1** foi incorporado verbatim em
    `src/msxdisk/core/msxdos1_boot.cpp` (ver seção 6) — está dentro do
    escopo já autorizado, ao contrário do restante deste diretório.
  - `DiskUtil.c/.h`, `rddsk.c`, `wrdsk.c` são de **Arnold Metselaar**, sob
    termos próprios (restringe distribuição comercial, pede notificação
    ao autor em caso de alteração) — **não** cobertos pelo aval de
    Fayzullin. Continuam tratados como clean-room only: só estudo do
    formato, nunca porte/tradução do código.
- `resource/msxDiskUtil/` — reescrita em **PureBasic** (GPLv3), com core
  FAT12 (`MSXDisk.pbi`), CLI (`msxdisk.pb`) e wrapper de DLL
  (`MSXDiskDLL.pb`). Bom mapa de features (create/list/add/extract/delete),
  mas sem rename/copy/save-as nem MSX-DOS 2.
- `resource/msxdos1/` (`COMMAND.COM`, `MSXDOS.SYS`) e `resource/msxdos2/`
  (`COMMAND2.COM`, `MSXDOS2.SYS`, `UTILS/*.COM`, `HELP/*.HLP`) — arquivos
  de sistema reais do MSX-DOS 1/2 (copyright ASCII/Microsoft/terceiros).
- `resource/fmsxgo/pkg/msxdisk/` e `resource/fmsxgo/third-party/disk/` —
  reimplementação em Go de outro projeto do autor; referência de
  arquitetura/testes, não de código (linguagem diferente).

**Regra:** todo o código do msxdisk é **clean-room**, escrito a partir do
entendimento público do formato FAT12/MSX-DOS (BPB, entradas de diretório
de 32 bytes, FAT de 12 bits) — nunca por tradução/porte linha-a-linha dos
fontes acima. Os binários de sistema do MSX-DOS (`COMMAND.COM`,
`MSXDOS.SYS` etc.) nunca são embutidos/redistribuídos pelo próprio
msxdisk; quando uma operação precisa deles (`create --dos1`), o caminho é
fornecido pelo usuário em tempo de execução.

**Atualização (2026-09-29):** a primeira versão da Fase 1 gravava só um
BPB genérico (sem bootstrap real), então os discos criados não davam
boot de verdade — reportado pelo autor do projeto ao testar num
disco criado com `create --dos1`. Corrigido: `CreateBlank()` agora grava
o **setor de boot real do MSX-DOS 1** (`src/msxdisk/core/msxdos1_boot.h`,
512 bytes idênticos a `resource/DiskUtilities/Boot.h`) sempre que a
geometria é a de 720KB DS/DD — inclusive em disco em branco sem
`--dos1` (o próprio bootstrap trata a ausência de `MSXDOS.SYS` com a
mensagem padrão "Boot error / Press any key for retry", igual a um disco
MSX-DOS real sem sistema). Confirmado byte a byte que
`resource/msxDiskUtil/MSXDisk.pbi` (PureBasic, testado e aprovado pelo
autor) usa exatamente o mesmo array como boot sector default — ver seção
2 para a base legal de usar esse bootstrap verbatim. Geometrias futuras
sem um setor de boot real conhecido ainda caem no fallback genérico
(`Geometry::WriteBootSector`, só BPB).

## 3. Divisão por linguagem (guia geral, pode ajustar por sub-fase)

- **C++**: arquitetura do núcleo (`DiskImage`, geometria, diretórios),
  CLI11, shell replxx, TUI FTXUI, GUI ImGui.
- **C**: empacotamento de bits da FAT12 (`fat12_c/`), `extern "C"`.
- **Assembly** (NASM, ABI Win64, padrão de `src/asm/init_asm.asm`):
  comparação de nome 8.3 com coringas MSX-DOS (`asm/name_match.asm`).
- **Fortran**: estatísticas de capacidade/espaço livre
  (`fortran/geometry_calc.f90`), via `iso_c_binding`.

## 4. Fases

### Fase 1 — Núcleo FAT12/MSX-DOS 1 + CLI one-shot

- [x] **1a** — Núcleo do formato: `Geometry` (720KB DS/DD, valores
      conferidos contra `resource/DiskUtilities/Boot.h`), boot sector
      próprio (BPB válido, sem bootloader autoral), leitura/escrita de
      FAT12 (`fat12_c`), entrada de diretório de 32 bytes, nome 8.3
      (`dir_entry`), `DiskImage` (criar em branco / carregar / salvar),
      módulo Assembly de comparação de nome (`name_match`), módulo
      Fortran de estatísticas de capacidade (`geometry_calc`).
      **Concluída** em 2026-09-29.
- [x] **1b** — `DiskImage`: `ListRoot`, `AddFile`, `ExtractFile`/
      `ExtractMatching` (com coringas `*`/`?`), `DeleteFile`,
      `CapacityStats`; comandos CLI11 (`create` com `--dos1`, `list`,
      `add`, `extract`, `delete`, `info`); integração no `CMakeLists.txt`
      raiz (`FetchContent` do CLI11, executável `msxdisk`);
      empacotamento em `build.ps1`/`dist/`. **Concluída** em 2026-09-29 —
      testada manualmente: round-trip create/add/list/extract/delete,
      arquivo multi-cluster (3002 bytes), filtro/extração por coringa
      (`*.TXT`), detecção de nome duplicado, `create --dos1` com os
      arquivos reais de `resource/msxdos1/`, e leitura bem-sucedida da
      imagem de exemplo `resource/msxDiskUtil/msxdos.dsk` (tamanhos de
      `COMMAND.COM`/`MSXDOS.SYS` conferem com o original).

### Fase 2 — MSX-DOS 2 (subdiretórios) + rename/copy/save-as

- [x] **2a** — Extensão do núcleo para subdiretórios do MSX-DOS 2:
      `msx_path.{h,cpp}` (parsing de caminho `DIR1\DIR2\ARQ.EXT`, aceita
      `/` também, ignora prefixo de unidade `A:`); `DiskImage::DirLocation`
      abstrai raiz (área fixa) vs. subdiretório (cadeia de clusters);
      `DirSlot`/`DirSlotCount`/`FindEntryIn`/`FindFreeSlotIn` (com
      crescimento automático da cadeia quando o subdiretório enche) e
      `ResolveDir` (percorre os componentes a partir da raiz). Entradas
      especiais `.`/`..` gravadas na criação. **Concluída** em 2026-09-29.
- [x] **2b** — Comandos: `rename` (não move entre pastas, igual ao `REN`
      real), `copy` (clonar imagem inteira via `std::filesystem::copy_file`,
      validando antes que a origem é uma imagem MSX de verdade), `saveas`
      (load+save), `mkdir`/`rmdir` (rejeita diretório não vazio), `list
      --dir <caminho>` e `list --tree` (recursivo, estilo `TREE`, oculta
      `.`/`..`). **Concluída** em 2026-09-29 — testado manualmente:
      diretórios aninhados (`JOGOS\NIVEL2\...`), add/extract/list dentro de
      subdiretório, `rmdir` falhando em diretório não vazio e funcionando
      após esvaziar, `copy`/`saveas` produzindo cópias idênticas byte a
      byte, erros `PathNotFound`/`NotADirectory`/`IsADirectory`, e
      crescimento de um subdiretório além de 1 cluster (35 arquivos, 32
      entradas/cluster de 1024 bytes — força ao menos 2 clusters).
- [x] **2c** — `create --dos2` (`--sys`/`--sys2`, mesma ressalva de
      licenciamento da 1b: arquivos fornecidos pelo usuário, nunca
      redistribuídos). Reaproveita o mesmo setor de boot de
      `msxdos1_boot.h` (o bootstrap só carrega o arquivo chamado
      `MSXDOS.SYS`, independente da versão do DOS dentro dele — ver seção
      6). **Concluída** em 2026-09-29 — testado criando um disco com os
      arquivos reais de `resource/msxdos2/`.

### Fase 3 — Shell interativo (replxx)

- [x] **3a** — `FetchContent` do replxx (`release-0.0.4`). A lógica de
      construção da `CLI::App` e despacho, antes só em `tools/msxdisk/
      main.cpp`, foi extraída para `src/msxdisk/cli/app.{h,cpp}`
      (`msxdisk::cli::Dispatch(tokens)`), reconstruindo a `App` do zero a
      cada chamada para não vazar estado entre comandos — usado tanto
      pelo modo CLI one-shot quanto pelo shell. Novo
      `src/msxdisk/shell/shell.{h,cpp}`: loop REPL com `replxx::Replxx`,
      tokenizador próprio (aceita aspas simples/duplas para caminhos com
      espaço), comandos internos `help`/`?` e `exit`/`quit`, histórico
      persistente em `~/.msxdisk_history` (`$USERPROFILE` no Windows).
      `msxdisk` sem argumentos (ou `msxdisk shell`) entra no shell;
      qualquer outro argumento vai para o modo CLI one-shot de sempre.
      **Concluída** em 2026-09-29 — testado via stdin (`create`, `list`,
      `help`, `exit` em sequência) e confirmado que o histórico foi
      gravado corretamente em `~/.msxdisk_history`; regressão do modo
      one-shot também reconfirmada.
- [x] **3b** — Sessão "estilo FTP" e comandos locais de filesystem
      (pedido extra do autor, além do que já estava planejado):
      - `src/msxdisk/shell/session.{h,cpp}`: `Session` guarda uma
        `DiskImage` carregada em memória + rastreamento de "sujo"
        (`dirty`). `load <imagem>` abre a sessão (avisa, sem bloquear, se
        havia alterações não salvas); `save` grava de volta no mesmo
        caminho; `saveas <novo>` grava em outro caminho **e** esse passa a
        ser o alvo do próximo `save`. Com sessão carregada, `list/dir`,
        `add`, `extract`, `delete`, `rename/ren`, `mkdir`, `rmdir`, `info`
        passam a operar direto na imagem em memória, **sem** repetir o
        caminho do `.dsk` e **sem** gravar em disco a cada comando (só no
        `save`/`saveas`) — sem sessão carregada, esses mesmos nomes de
        comando continuam caindo no modo CLI one-shot de sempre (Dispatch),
        preservando 100% do que já funcionava. `create` também carrega
        automaticamente a imagem recém-criada na sessão.
      - `src/msxdisk/shell/local_fs.{h,cpp}`: comandos do lado do
        HOST (não tocam a imagem MSX), inspirados no lado "local" de um
        cliente FTP: `ls [padrão]` (aceita coringas `*`/`?`, comparação
        genérica de string — não usa o `name_match` em Assembly, que só
        entende os 11 bytes fixos do formato 8.3 MSX), `cd` (muda o
        diretório do processo, aceita `cd D:` no Windows), `md` (cria
        diretório local), `rm` (remove só arquivo, nunca diretório — por
        segurança), `pwd`.
      - `src/msxdisk/cli/format.{h,cpp}`: extraído de `commands.cpp` pra
        compartilhar mensagens de erro e impressão de listagem/árvore
        entre o modo one-shot e a sessão do shell sem duplicar código.
      - Prompt do shell muda para `msxdisk [nome.dsk]>` (com `*` se houver
        alterações não salvas) quando há uma sessão carregada.
      **Concluída** em 2026-09-29 — testado todo o fluxo: `pwd`/`cd`/`ls`/
      `ls *.bas` no host, `create` auto-carregando a sessão, `add`/`mkdir`
      sem caminho de imagem, `info` mostrando "alterações não salvas",
      `save`/`saveas` (confirmado que `saveas` retarget a sessão: uma
      mudança feita depois de um `saveas backup2.dsk` foi parar só em
      `backup2.dsk`, não no arquivo original), aviso de alterações não
      salvas ao `load` outra imagem por cima, `md`/`rm` locais, e
      confirmado que um `delete` em memória sem `save` não altera o
      arquivo `.dsk` no disco.
      Autocompletar (nomes de comando/arquivo) ficou de fora desta
      sub-fase — não pedido explicitamente e replxx exigiria um callback
      de completion mais elaborado; pode virar uma 3c se fizer falta.

      **Extensão (2026-09-29, mesmo dia)**: relato de erro real do autor
      — `add` depois de `load` dava "erro de E/S (arquivo/imagem
      inacessivel)". Investigado: não era bug de lógica (o `add` já
      reportava o caminho exato tentado), mas a mensagem genérica não
      deixava óbvio que o problema era o **arquivo local** não encontrado
      (não a imagem). Corrigido com `cli::CheckLocalFileReadable()`
      (`format.{h,cpp}`), usado por `add`/`put`/`create --dos1/--dos2`
      antes de tentar abrir o arquivo, com mensagem explícita apontando
      `pwd`/`ls` como próximo passo. Aproveitando, adicionados os pares
      **`put`/`get`** (um arquivo por vez, sintaxe posicional estilo FTP:
      `put local.bas [nome_msx]` / `get ARQ.MSX [destino_local]`) e
      **`mput`/`mget`** (`Session::CmdMput`/`CmdMget` em
      `session.{h,cpp}`, aceitam coringa: `mput *.bas` varre o diretório
      LOCAL atual com `LocalWildcardMatch` — a mesma comparação genérica
      de `ls`, movida para `local_fs.h` — e `mget *.bin` varre o
      diretório da IMAGEM), confirmando arquivo a arquivo (como
      `smbclient`); comando **`prompt`** liga/desliga essa confirmação
      (default: ligada). A confirmação lê de `std::cin` direto (não usa o
      replxx — não precisa de histórico/edição pra uma resposta s/n).
      Testado: reprodução do erro original com mensagem nova e clara;
      `put`/`get` de ida e volta batendo byte a byte; `mput`/`mget` com
      prompt ligado (aceitando/recusando por arquivo) e com `prompt`
      desligado (silencioso).

      **Bug real encontrado e corrigido (2026-09-29, mesmo dia)**: `save`
      gravava no arquivo errado depois de um `cd` — reportado pelo autor
      (`create teste.dsk` em `C:\dos`, `cd C:\msx`, `mput *.bas`, `save`
      → salvou vazio em `C:\dos\teste.dsk` e criou o disco de verdade em
      `C:\msx\teste.dsk`). Causa: `image_path_` guardava o caminho
      exatamente como digitado (relativo); como o `cd` local muda o
      diretório do PROCESSO inteiro, um `save` posterior reabria esse
      caminho relativo contra o diretório atual (errado), não o de quando
      foi feito o `load`/`create`. Corrigido resolvendo pra absoluto
      (`std::filesystem::absolute`) no momento do `load`/`create`/
      `saveas` (`Session::Attach`/`ResolveForStorage` em
      `session.cpp`) — `image_path_` nunca mais é reinterpretado contra
      um diretório diferente. Bônus: `info` agora mostra o caminho
      absoluto, deixando claro pra onde o próximo `save` vai escrever.
      Reproduzido e reconfirmado corrigido com o cenário exato relatado.

### Fase 4 — TUI (FTXUI), estilo XTree/Norton Commander

Pedido do autor: barra de comandos de disco (novo/abrir/carregar/salvar)
no topo, gerenciador de arquivos de duas colunas embaixo (local ↔ imagem
MSX, navegando e "marcando" arquivos pra enviar/receber, igual ao
F5=Copiar do Norton Commander ou à marcação com espaço do XTree/PC
Tools), suporte a temas, e **todas** as configurações/opções/detalhes
guardados num banco SQLite (não em INI/JSON solto).

> **Unificação em um único executável (2026-09-29, a pedido do autor)**:
> `msxdisk.exe` agora é o **único** binário para os quatro modos —
> `msxdisk-tui.exe` separado foi eliminado. Roteamento em
> `tools/msxdisk/main.cpp`: `msxdisk <comando> ...` = CLI one-shot;
> `msxdisk` (sem argumentos) ou `msxdisk --cli` = shell interativo;
> `msxdisk --tui [imagem.dsk]` = TUI direto. De dentro do shell, os
> comandos `tui`/`call tui` (nome inspirado no `CALL` do MSX-BASIC)
> abrem a TUI **sem sair do processo** — se havia uma imagem carregada na
> sessão do shell, a TUI já abre com ela, e ao voltar (`F10`/Esc) a
> sessão recarrega do disco pra pegar o que a TUI tiver salvo. A lógica
> antes em `tools/msxdisk-tui/main.cpp` virou `tui::LaunchTui()` em
> `src/msxdisk/tui/app.cpp`, reaproveitada pelos dois pontos de entrada.
> No `CMakeLists.txt`, os dois `add_executable` viraram um só — CLI11,
> replxx, FTXUI e SQLite linkados todos no mesmo alvo `msxdisk` (e como
> agora só existe um executável compilando `geometry_calc.f90`, o problema
> de `Fortran_MODULE_DIRECTORY` duplicado da Fase 4a deixou de existir).
> **Fase 5** (GUI) deve seguir o mesmo esquema: `--gui`/`call gui`.

> **Interpretação confirmada com o autor**: o SQLite guarda configuração/
> estado da aplicação (tema ativo, imagens recentes, opções, textos de
> ajuda) **e também metadados por imagem `.dsk`** (descrição, data de
> criação, notas livres do usuário sobre cada disco) — tabela
> `image_metadata` (seção 4a). **Não** inclui mover a documentação-fonte
> do projeto (`doc/*.md`) para dentro do banco; `doc/msxdisk-spec.md`
> continua o documento vivo de especificação/acompanhamento de sempre.

- [x] **4a** — Infraestrutura: `FetchContent` do FTXUI (`v7.0.3`) e do
      SQLite (amalgamation oficial `sqlite3.c`/`sqlite3.h` via
      `FetchContent_Populate`, sem `FetchContent_MakeAvailable` porque a
      amalgamation não tem `CMakeLists.txt` — compilada como lib própria
      `sqlite3`). Módulo `src/msxdisk/config/`: `theme.{h,cpp}` (struct
      `Theme` com cores RGB simples, sem depender do FTXUI, + dois temas
      embutidos: `ClassicTheme()` azul estilo Norton Commander/XTree
      Gold, `DarkTheme()` escuro moderno) e `config_store.{h,cpp}`
      (`ConfigStore` sobre a API C do SQLite: tabelas `settings`
      chave/valor, `themes`, `recent_images`; banco em
      `~/.msxdisk/config.sqlite3`). Novo alvo `msxdisk-tui` no
      `CMakeLists.txt`, com um `main.cpp` de fumaça (Fase 4a) que abre o
      `ConfigStore`, lê o tema ativo e desenha uma tela FTXUI com as
      cores do tema — só pra provar que as duas peças (banco + interface)
      funcionam juntas antes de construir as telas de verdade.
      **Concluída** em 2026-09-29 — rodado com `timeout` (TUI em tela
      cheia não solta sozinha sem teclado de verdade) e confirmado: tema
      "classic" carregado do SQLite e desenhado com as cores certas (fundo
      azul, texto branco), `Temas conhecidos no SQLite: 2` e `Imagens
      recentes registradas: 1` corretos, e o arquivo
      `~/.msxdisk/config.sqlite3` foi criado de verdade.
      **Achado de build**: `msxdisk` e `msxdisk-tui` compilam o mesmo
      `geometry_calc.f90` de forma independente (mesma decisão de não ter
      lib intermediária); sem separar `Fortran_MODULE_DIRECTORY` por alvo,
      os dois geram o mesmo `mod_geometry_calc.mod` no mesmo lugar e o
      Ninja recusa o build ("multiple rules generate"). Corrigido com
      `set_target_properties(<alvo> PROPERTIES Fortran_MODULE_DIRECTORY
      ...)` diferente para cada alvo.
      **Extensão (mesmo dia)**: adicionada a tabela `image_metadata`
      (path absoluto como chave, `description`, `notes`, `created_at`,
      `updated_at`) e os métodos `RecordImageSeen`/`SetImageMetadata`/
      `GetImageMetadata` em `ConfigStore`, a pedido do autor (metadados
      por imagem `.dsk`, além da config geral da app). Testado no
      smoke-test: grava descrição+notas, lê de volta, confere.
- [x] **4b** — Esqueleto de layout FTXUI: `src/msxdisk/tui/pane_state.{h,cpp}`
      (estado puro, sem FTXUI: `LocalPaneState`/`DiskPaneState`,
      `RefreshLocalEntries`/`RefreshDiskEntries`, `NavigateLocal`/
      `NavigateDisk` -- dirs antes de arquivos, alfabético, `..` no topo
      quando aplicável) e `src/msxdisk/tui/app.{h,cpp}` (renderização
      manual via `CatchEvent`+`Renderer`, sem a árvore de `Component` do
      FTXUI — mais simples de controlar pra um clone de Norton Commander
      com estado próprio). Barra de comandos no topo (`F2 Novo`, `F3
      Abrir`, `F5 Salvar`, `F6 SalvarComo`, `F7 Enviar>`, `F8 <Receber`,
      `F9 Tema`, `F10 Sair` — como rótulos por enquanto, a maioria ainda
      não funcional), duas colunas (local ↔ imagem MSX carregada), `Tab`
      troca a coluna ativa, setas movem o cursor, `Enter` desce
      em diretório (`..` sobe), `Espaço` já marca/desmarca visualmente
      (envio/recebimento de verdade é 4c). `F9` (trocar tema) e `F10`/Esc
      (sair) já funcionam de verdade. `msxdisk-tui.exe [imagem.dsk]` abre
      com a imagem já carregada, se informada.
      **Concluída** em 2026-09-29 — testado renderizando com uma imagem
      real (`local1.txt`/`JOGOS`/arquivo dentro de `JOGOS` no disco):
      as duas colunas mostraram o conteúdo certo, tamanhos e cores do
      tema aplicados. **Limitação conhecida**: não consegui automatizar
      teste de teclado (setas/Tab/Enter/Espaço) por aqui — no Windows o
      FTXUI lê entrada via console de verdade, não funciona injetando
      bytes por um pipe de stdin. Precisa o autor confirmar
      interativamente que a navegação/marcação/troca de tema funcionam
      como esperado antes de eu seguir pra 4c.
- [x] **4c** — Marcação (Espaço) e transferência: `F7` envia os arquivos
      locais marcados (ou o arquivo sob o cursor, se nada marcado — igual
      ao F5=Copiar do Norton Commander sem tags) para o diretório atual
      da imagem; `F8` faz o mesmo no sentido inverso (imagem → local).
      **Decisão de design**: ao contrário do `mput`/`mget` do shell (Fase
      3b), a TUI **não** pede confirmação por arquivo — marcar já é o
      passo deliberado de seleção (igual ao Norton Commander real: F5
      copia os marcados direto, sem perguntar de novo por um). `F2`
      (Novo), `F3` (Abrir) e `F6` (Salvar Como) abrem um diálogo modal de
      texto (`DialogKind`, `Enter` confirma, `Esc` cancela — implementado
      na mão, sem o `Input` do FTXUI, pra manter a mesma arquitetura de
      `CatchEvent`+estado próprio já usada desde a 4b); `F5` (Salvar) e
      direto, sem diálogo. Barra de status mostra o resultado/erro de
      cada ação. Envio/recebimento fica só em memória (`disk.dirty=true`)
      até `F5`. **Concluída** em 2026-09-29.
- [x] **4d** — `F4` renomear (funciona nos dois painéis: local via
      `std::filesystem::rename`, disco via `RenameFile`), `F11` criar
      pasta (`std::filesystem::create_directory` / `MakeDirectory`),
      `Del` excluir (com diálogo de confirmação de uma tecla — s/n —
      diferente dos diálogos de texto; pastas locais não podem ser
      excluídas, mesma regra de segurança do `rm` do shell). `F9` já
      trocava tema desde a 4b (ciclando `classic`/`dark`). Persistência
      no SQLite: `last_local_dir` (salvo ao sair) e `last_image_path`
      (salvo a cada novo/abrir/salvar-como; usado para reabrir a última
      imagem automaticamente se `msxdisk --tui` for chamado sem
      argumento). **Concluída** em 2026-09-29 — nesse meio tempo, achei e
      corrigi um bug pré-existente em `DiskImage::RenameFile` (desde a
      Fase 2): renomear um arquivo para o **próprio nome atual** dava
      erro de "já existe" em vez de ser um no-op — o código achava a
      própria entrada e confundia com um conflito. Corrigido comparando
      o ponteiro da entrada encontrada com a que está sendo renomeada.
      Reconfirmado com testes que o caso de conflito de verdade (renomear
      para o nome de OUTRO arquivo) continua barrado corretamente.

      **Limitação conhecida em ambos**: quando várias entradas marcadas
      são enviadas/recebidas/excluídas e uma falha no meio, a mensagem
      de status final (contagem de sucesso) sobrescreve a mensagem de
      erro do item que falhou — só fica visível a última. Cosmético, não
      é perda de dados (os itens que falharam simplesmente não são
      afetados), mas vale melhorar numa polida futura.
      **Mesma limitação de teste da 4b**: não consegui validar
      interativamente (diálogos, F-keys, exclusão) por aqui — só
      compilação + renderização inicial. Precisa o autor confirmar na
      prática.

### Fase 5 — GUI (Dear ImGui) + integração final no fwMSX

- [x] **5a** — `FetchContent` do GLFW (`3.4`) e do Dear ImGui (`v1.92.9`
      — sem `CMakeLists.txt` oficial, lib `imgui` montada na mão a partir
      de `imgui.cpp`/`imgui_draw.cpp`/`imgui_tables.cpp`/
      `imgui_widgets.cpp` + backends `imgui_impl_glfw.cpp`/
      `imgui_impl_opengl3.cpp`; o loader de funções OpenGL vem embutido em
      `imgui_impl_opengl3_loader.h`, confirmado presente nessa versão —
      não precisou de GLAD/GLEW separado). Opção de CMake
      `FWMSX_MSXDISK_GUI` (default `ON`): quando `OFF`, `src/msxdisk/gui/
      app_stub.cpp` entra no lugar de `app.cpp` (mesma função `LaunchGui`,
      só avisa que a build não tem GUI) — `tools/msxdisk/main.cpp` e
      `shell.cpp` não precisam de `#ifdef` nenhum, a escolha é só de qual
      `.cpp` entra no `add_executable`. `msxdisk --gui [disco.dsk]` e
      `gui`/`call gui` de dentro do shell, no mesmo esquema do `--tui`/
      `call tui`. Tema do SQLite mapeado para o estilo do ImGui
      (`ApplyTheme`), e o `DiskImage` reaproveitado pra listar a raiz da
      imagem, se uma for passada — mesmo espírito do smoke-test da 4a,
      agora pro lado gráfico. **Concluída** em 2026-09-29 — validado com
      **captura de tela de verdade** (não só timeout): janela real
      abrindo, tema "classic" aplicado (fundo azul), e a listagem do
      disco de teste batendo com o conteúdo real (`RENOMEAD.TXT`,
      `[JOGOS]`, `JOGOSGAM.BAS`).
- [x] **5b** — Interface de verdade: duas colunas lado a lado (local ↔
      imagem MSX, reaproveitando `src/msxdisk/tui/pane_state.{h,cpp}` sem
      nenhuma mudança — já era independente de framework de UI desde a
      Fase 4b), marcação por checkbox, duplo-clique desce em diretório,
      barra de menu (`Arquivo`/`Ação`/`Tema`) e as mesmas teclas de
      função da TUI (F2 Novo, F3 Abrir, F4 Renomear, F5 Salvar, F6
      SalvarComo, F7 Enviar, F8 Receber, F9 Tema, F11 NovaPasta, Del
      Excluir) funcionando tanto pelo menu quanto pelo teclado. Diálogos
      modais nativos do ImGui (`BeginPopupModal`/`InputText`) — mais
      simples que a máquina de estado manual da TUI, porque o ImGui já
      tem popup pronto (o FTXUI não). Ações (Novo/Abrir/Salvar/Enviar/
      Receber/Renomear/NovaPasta/Excluir) **duplicadas** de
      `src/msxdisk/tui/app.cpp` de propósito — mesma lógica, adaptada pra
      não depender do `AppState`/`DialogKind` da TUI, evitando uma
      dependência cruzada entre os módulos `gui/` e `tui/` só por causa
      de uma struct (mesmo espírito da decisão já documentada de duplicar
      os fontes de linguagem entre `msxdisk` e outros pontos de entrada).
      Persistência (`last_local_dir`, `last_image_path`, `active_theme`)
      usa as **mesmas chaves do SQLite que a TUI**, então abrir a GUI
      depois da TUI (ou vice-versa) já lembra o último lugar/imagem.
      **Concluída** em 2026-09-29 — validado com capturas de tela reais:
      **um bug de verdade foi encontrado e corrigido nesse processo** —
      as duas colunas não apareciam lado a lado (o painel local ocupava a
      tela inteira) porque a largura calculada (`pane_width`) nunca
      chegava a ser usada de fato no `BeginChild` (código antigo usava
      `PushItemWidth`, que só afeta widgets, não o tamanho do painel).
      Corrigido passando a largura direto pro `BeginChild`; reconfirmado
      com nova captura de tela que as duas colunas renderizam corretas
      lado a lado, com dados reais dos dois lados (local e imagem).
      **Não consegui automatizar teste dos diálogos/teclas de função**
      (SendKeys do Windows esbarra na restrição de "foreground lock" —
      a janela nem sempre aceita foco de um processo que não é o
      primeiro plano atual) — precisou de duas rodadas de correção com o
      autor testando na prática (ver os dois bugs reais abaixo) até tudo
      funcionar de fato.

      **Bug real encontrado e corrigido (2026-09-29, dia seguinte)**:
      relato do autor — "Nova Imagem, Abrir imagem, Salvar imagem, etc.
      não estão funcionando" (nem pelo menu, nem pelas teclas de função),
      o que também impediu testar Enviar/Receber. Investigado lendo o
      código-fonte do próprio Dear ImGui (`imgui.cpp`, `OpenPopup`/
      `BeginPopupModal`): os dois calculam o ID do popup como
      `g.CurrentWindow->GetID(nome)` — ou seja, o ID depende de **qual
      janela está "atual" no momento da chamada**, não só do texto do
      nome. No código anterior, `ImGui::OpenPopup(...)` era chamado de
      **três lugares com "janela atual" diferente**: dentro de
      `BeginMenu()` (uma sub-janela própria), nos atalhos de teclado
      (antes até do `Begin("msxdisk")` do frame), e o `BeginPopupModal`
      que verifica se abre era chamado direto dentro de `Begin("msxdisk")`
      — três IDs diferentes para o "mesmo" popup, então ele nunca
      reconhecia como aberto. Corrigido: teclado e itens de menu agora só
      marcam a intenção (`want_open_novo`, `want_open_abrir`, etc.); o
      `ImGui::OpenPopup(...)` de verdade roda uma única vez, logo após o
      `Begin("msxdisk")` e antes de qualquer `BeginMenu`/`BeginMenuBar`,
      no mesmo nível de janela onde o `BeginPopupModal` correspondente é
      checado mais abaixo — garantindo o mesmo ID nos dois lados.
      Recompilado com sucesso. **Não consegui reconfirmar com captura de
      tela desta vez** — o ambiente de teste parece ter uma aplicação em
      tela cheia (jogo) rodando, e a captura via GDI (`CopyFromScreen`)
      trouxe conteúdo de outra janela/aplicação em vez do msxdisk, mesmo
      com `GetForegroundWindow()` confirmando que o handle correto estava
      em primeiro plano (limitação conhecida de captura GDI sobre
      aplicações em fullscreen exclusivo). A correção tem alta confiança
      por ser baseada na leitura direta do código-fonte do ImGui (não é
      uma suposição). **Confirmado pelo autor** depois das correções
      seguintes (diálogos nativos + fix de painel ativo, abaixo) — "agora
      parece estar funcionando direitinho".

      **Pedidos extras do autor no mesmo dia**: (1) diálogo de "ejetar"
      o disco virtualmente (descarrega a imagem da memória sem apagar o
      arquivo) — `ActionEject` (`Arquivo > Ejetar` ou `F12`), com
      confirmação só se houver alterações não salvas (`disk.dirty`); sem
      alterações pendentes, ejeta na hora. (2) Novo/Abrir/Salvar Como
      trocados de caixa de texto simples para **diálogo nativo de arquivo
      do Windows** (`GetOpenFileNameW`/`GetSaveFileNameW` via
      `src/msxdisk/gui/file_dialog.{h,cpp}`, linkando `comdlg32`) — dá
      navegação de pastas de verdade (Explorer), filtro por `*.dsk`, e já
      é modal por conta própria (não precisa mais do cuidado de escopo de
      ID do ImGui para esses três). Renomear/NovaPasta/Excluir continuam
      com o popup de texto/confirmação do próprio ImGui, já que só pedem
      um nome ou uma confirmação, não um caminho completo.
      `GetOpenFileNameW`/`GetSaveFileNameW` podem alterar o diretório
      atual do processo como efeito colateral — como o painel LOCAL
      depende do cwd, um `CwdGuard` (RAII) restaura o diretório original
      depois de cada chamada, mesmo com `OFN_NOCHANGEDIR` já pedido.
      **Confirmado pelo autor** funcionando junto com o resto ("agora
      parece estar funcionando direitinho").

      **Segundo bug real relatado e corrigido (2026-09-29, mesmo dia)**:
      "renomear não funcionou, criar pasta parece ter criado a pasta mas
      não consigo mover arquivos pra ela, excluir também não funcionou".
      Desta vez **não era** escopo de ID do ImGui (já verificado
      correto) — era um gap de rastreamento de estado: marcar a checkbox
      de uma entrada **não** atualizava `state.active`/`cursor` (só
      clicar no *nome* da linha fazia isso, via `Selectable`). Se o
      usuário marcava um arquivo do painel do DISCO só pela caixinha,
      sem nunca clicar no nome de nenhuma linha daquele painel,
      `state.active` continuava apontando pro painel LOCAL (valor
      inicial) — então `F4`/`F7`/`F8`/`F11`/`Del` agiam no painel/item
      errado (ex.: "criar pasta" criava a pasta no sistema de arquivos
      LOCAL, não na imagem — por isso nenhum arquivo "entrava" nela pelo
      lado do disco). Corrigido: marcar a checkbox agora também ativa o
      painel e seleciona a linha (`cursor = i; state.active = pane_kind;`
      dentro do `if (ImGui::Checkbox(...))`), igual ao que clicar no nome
      já fazia. **Confirmado pelo autor** em seguida ("agora parece estar
      funcionando direitinho") — Renomear, Nova Pasta e Excluir
      funcionando corretamente na GUI.

      **Ajuste visual a pedido do autor (2026-09-29, mesmo dia)**: "os
      temas estão muito 1990" — a GUI usava o mesmo `config::Theme`
      retro da TUI (fundo azul estilo Norton Commander), que funciona
      bem em terminal mas fica datado numa janela gráfica de verdade.
      Criado `src/msxdisk/gui/style.{h,cpp}`, com uma identidade visual
      **própria e moderna** da GUI, independente do `Theme` da TUI (que
      continua intocado — o autor confirmou que a TUI "está perfeita"):
      paleta escura/clara com acento azul, cantos arredondados
      (`WindowRounding`/`FrameRounding`/etc.), espaçamento mais generoso,
      listras sutis nas linhas das tabelas, e `LoadModernFont` trocando a
      fonte padrão pixelada do ImGui pela Segoe UI do Windows (com
      fallback silencioso pra fonte embutida se não achar nenhuma
      candidata). Preferência salva em `gui_theme` no SQLite (chave
      própria, separada do `active_theme` da TUI) — menu "Tema" agora
      oferece "Escuro"/"Claro" em vez de "Classic"/"Dark". Testado com
      captura de tela dos dois modos (escuro e claro): fonte carregando
      corretamente, cores e arredondamento aplicados, layout de tabela
      com listras.
- [x] **5c** — Integração do msxdisk como utilitário interno do
      `fwMSX.exe`, via `fwmsx --msxdisk <resto dos argumentos>`. Criado
      `src/msxdisk/entry.{h,cpp}` (`RunEntryPoint`) com o roteamento dos
      4 modos, compartilhado por `tools/msxdisk/main.cpp` (standalone) e
      `src/cpp/main.cpp` (`fwMSX.exe`, atrás do sentinela `--msxdisk`
      como `argv[1]`). No `CMakeLists.txt`, `MSXDISK_LIB_SOURCES` virou
      uma lista compartilhada compilada nos dois alvos (`msxdisk` e
      `fwMSX`), com uma função `msxdisk_link_target()` aplicando as
      mesmas libs/opções de link nos dois — precisou de
      `Fortran_MODULE_DIRECTORY` separado por alvo de novo (mesma lição
      da Fase 4a, agora entre `msxdisk` e `fwMSX`). **`msxdisk.exe`
      continua existindo como binário standalone** (requisito original
      do autor). Decisão registrada também em `doc/SPEC.md`, seção 5.1 —
      a parte da visão maior sobre `fwMSX.exe` ganhar seu próprio
      REPL/TUI/GUI de configuração do emulador continua pendente
      (trabalho do lado do emulador, ainda Fase 0, não do `msxdisk`).
      **Concluído** em 2026-09-29 — testado: `fwMSX.exe` sem argumentos
      mantém a saída do esqueleto **idêntica** à anterior (sem
      regressão); `fwmsx --msxdisk list/info` batem com a saída do
      `msxdisk.exe` direto; `fwmsx --msxdisk --tui`/`--gui` abrem
      normalmente.

      **Documentação final e versionamento** (fecha o `msxdisk` como
      feature completa): `doc/SPEC.md` (seção 5.1 atualizada),
      `doc/CHANGELOG.md` e `doc/RELEASE.md` (entrada `v1.2.0`),
      `doc/MANUAL.md` (seção de uso do `msxdisk`, opção de build
      `FWMSX_MSXDISK_GUI=OFF`), `README.md` (estrutura de diretórios +
      menção ao `msxdisk`) e `resource/README.md` (marca `DiskUtilities`/
      `msxDiskUtil`/`msxdos1`/`msxdos2` como já usados, distinguindo o
      que ainda é só referência para fases futuras do emulador). Versão
      `1.1.2` → **`1.2.0`** ("Maze of Galious: Gerenciador de Discos") —
      minor bump por ser feature nova incorporada (regra de
      `doc/SPEC.md#versionamento`), não um novo "core estável" do
      emulador (esses exemplos da regra são sobre o emulador em si, não
      sobre utilitários como o `msxdisk`).

## 5. Critérios de aceite

- Round-trip completo (criar → add → list → extract → rename → delete →
  copy → saveas) sem corromper FAT nem diretório.
- Testado contra `resource/msxDiskUtil/msxdos.dsk` (imagem de exemplo) e
  contra discos criados com os arquivos de `resource/msxdos1/` e
  `resource/msxdos2/`.
- Idealmente validado lendo a imagem gerada num emulador MSX real
  (fMSX/openMSX), como confirmação de compatibilidade de formato.

## 6. Notas de implementação (para quem retomar o trabalho)

*(preenchida ao longo das sub-fases, mesmo padrão de `doc/SPEC.md`
seção 3.2)*

- **1a/1b**: geometria suportada por enquanto é só 720KB DS/DD (a única
  cujo BPB foi conferido byte a byte contra `Boot.h`). `Geometry` já é
  parametrizada e `DiskImage::Load` lê o BPB do próprio arquivo (não
  assume tamanho fixo), então imagens de outras geometrias podem ser
  *lidas* mesmo antes de `CreateBlank` suportar criá-las — só falta
  validar os valores de BPB de outros formatos (360KB etc.) antes de
  oferecer `create` para eles.
- **Arquitetura de build (importante para as próximas fases)**: os
  fontes do msxdisk (C++/C/ASM/Fortran) são listados **diretamente** no
  `add_executable(msxdisk ...)` do `CMakeLists.txt` raiz, do mesmo jeito
  que o alvo `fwMSX` já fazia — **não** existe uma biblioteca estática
  `msxdisk_core` separada. Motivo: o CMake só resolve automaticamente as
  libs de runtime do Fortran (`libgfortran`/`libquadmath`) quando os
  objetos dessas linguagens pertencem ao *próprio* alvo sendo linkado; se
  ficassem dentro de uma `STATIC` library intermediária linkada por um
  executável C++, o link final poderia falhar por símbolos não resolvidos
  do runtime do Fortran (não veio a ser testado a fundo, mas é um problema
  conhecido de projetos CMake multi-linguagem — preferimos o caminho já
  comprovado nesta base). Ao chegar na Fase 5 (embutir o msxdisk dentro do
  `fwMSX.exe`), reavaliar: manter os fontes duplicados na lista de sources
  de `fwMSX`, ou testar uma lib de verdade com `-lgfortran -lquadmath`
  explícitos no alvo consumidor.
- `DiskImage::Load` valida a imagem batendo `ImageSizeBytes()` (derivado
  do BPB lido) contra o tamanho real do arquivo — suficiente para rejeitar
  arquivos que claramente não são uma imagem MSX válida, mas não é uma
  validação forte de checksum/consistência da FAT.
- Empacotamento MSX-DOS/hora usa `localtime_s` (UCRT) diretamente — ok
  porque o msxdisk é single-threaded; se isso mudar, revisar.
- `create --dos1` **não** embute nem redistribui `COMMAND.COM`/
  `MSXDOS.SYS`: exige `--command`/`--sys` apontando para arquivos que o
  usuário já tem localmente (ex.: os de `resource/msxdos1/`, só para uso
  local/estudo).
- **Setor de boot real** (2026-09-29): ver a atualização na seção 2/4 —
  `CreateBlank()` grava `kMsxDos1BootSector720KB` (verbatim de `Boot.h`)
  para a geometria de 720KB, em vez do BPB genérico anterior.
- **Fase 2 — arquitetura de diretórios**: `DiskImage::DirLocation` (raiz
  vs. cadeia de clusters) é privado; toda a API pública continua recebendo
  caminhos como `std::string` (ex.: `"JOGOS\SUB.TXT"`), resolvidos
  internamente via `ResolveDir`. `FindFreeSlotIn` estende a cadeia de um
  subdiretório automaticamente quando enche (a raiz não pode crescer,
  continua limitada a `root_dir_entries`). `RenameFile` não move entre
  diretórios (igual ao `REN` real do MSX-DOS); mover teria que ser um
  comando separado (`move`), não implementado ainda. `ExtractMatching`
  não desce recursivamente em subdiretórios ao usar coringas — extrai só
  os arquivos do diretório indicado.
- **`create --dos2` não foi testado num emulador/hardware real ainda**
  (diferente do `--dos1`, validado no openMSX com MSX1 e MSX2 — ver
  histórico da Fase 1). A suposição de que o mesmo setor de boot serve
  para MSX-DOS 2 (por só carregar o arquivo chamado `MSXDOS.SYS`,
  independente da versão) é razoável mas ainda não confirmada na prática.
- **Fase 3a**: `Dispatch()` reconstrói a `CLI::App` inteira a cada
  chamada (em vez de reusar uma instância entre comandos do shell) de
  propósito — as variáveis ligadas às opções do CLI11 (`add_option`) só
  são reinicializadas de verdade criando uma `App` nova; reusar a mesma
  instância entre comandos arriscaria um valor de uma linha "vazar" pra
  próxima (ex.: `--as` de um `add` afetando o próximo `add` sem `--as`).
  O custo (reconstruir ~10 subcomandos a cada linha digitada) é
  irrelevante perto do tempo de I/O de disco de cada comando.
- **Gotcha de ambiente (não é bug do msxdisk)**: nesta máquina de
  desenvolvimento, o Bitdefender (Virus Shield/Ransomware Remediation)
  pode travar um `.dsk` recém-criado por um `msxdisk.exe` recém-compilado
  e não assinado, fazendo `add`/`create` falharem com "erro de E/S" (o
  `Save()` reporta certinho, só que passa despercebido). Se isso
  acontecer de novo: checar `Get-Process | Where ProcessName -match
  'bdredline|VSSERV|BDProtSrv'` e adicionar `dist/msxdisk.exe` (ou a
  pasta do projeto) nas exceções do Bitdefender.
