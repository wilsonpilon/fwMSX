# O projeto `.msxproject` (`doc/project-spec.md`)

Um projeto do fwMSX e' **UM arquivo SQLite** (`.msxproject`) com tudo dentro: fontes em BASIC/Dignified/Assembly/Pascal, sprites, telas, musicas, MML/DML,
notas em Markdown, PDF, discos `.dsk`, fitas `.tsx/.cas` e a configuracao (modelo do MSX, opcoes do emulador...). Para levar o projeto a outro PC basta
copiar esse arquivo. Alem dele, o projeto mantem **copias soltas** numa pasta (`src/jogo.bas`, `discos/jogo.dsk`...), em sincronia com o arquivo, para
usar editores externos e o GitHub. Entrou na 1.43.0 ("Yie Ar Kung-Fu"), fatia 1 da fase E6 parte 2.

Pedido do usuario: *"um unico arquivo que vai ter todo o projeto [...] um mega arquivo em sqlite [...] o .msxproject vai ter copias soltas no diretorio do
projeto [...] quando um usuario copiar um .msxproject de um computador para outro, ele vai criar a pasta (o usuario escolhe a raiz) e vai extrair os
arquivos [...] o editor aponta para a opcao mais nova [...] uma maneira facil de copiar um projeto entre PCs e mover para o github tambem"*.

## 1. Decisoes (respondidas pelo usuario, 2026-10-21)

1. **Conflito** = hash de base + perguntar: compara-se cada arquivo com o hash da ULTIMA sincronizacao feita naquela pasta. So' um lado mudou = a direcao
   certa e' sugerida (e aplicada pelo `sync`); os dois mudaram = **conflito**: nada e' mexido, o comando diz qual lado e' o mais novo (a ideia do usuario
   de "apontar para o mais novo") e o usuario decide. A data sozinha engana (git clone, zip, relogios diferentes), por isso nao e' o criterio.
2. **GitHub** = a PASTA (copias soltas + manifesto em texto), nao o banco: o SQLite binario nao tem diff nem merge e incha o repositorio a cada commit.
   `*.msxproject` e `.msxsync` vao no `.gitignore`; depois de um clone, `project import <pasta> <novo.msxproject>` recria o banco.
3. **Journal tradicional** (`journal_mode=DELETE`, `synchronous=FULL`), sem `-wal`/`-shm`: o `.msxproject` fechado, ou entre gravacoes, e' SEMPRE o projeto
   inteiro; copiar so' esse arquivo nunca perde dados.

## 2. Estrutura (`src/project/`)

| Arquivo | Linguagem | O que faz |
| --- | --- | --- |
| `cpp/project.h`, `project.cpp` | C++ | `prj::Project`: banco, configuracao, arquivos, estado e sincronizacao, manifesto, import/export |
| `c/prj_kind.c` | **C** | `prj_classify`: o tipo do arquivo pela extensao (`basic`, `dignified`, `asm`, `disk`, `tape`, `screen`...) e se e' texto |
| `fortran/prj_path.f90` | **Fortran** | `prj_path_ok`: o caminho serve? (relativo, sem `..`, sem componente vazio, sem `:`, sem nomes que o Windows descarta) |
| `asm/prj_text.asm` | **NASM** | `prj_strip_cr`: copia sem os CR; o hash de um TEXTO ignora o fim de linha. ABI dupla Win64/SysV |

O hash e' o SHA-1 do romdb (`romdb::Sha1Hex`). Comando de console: `src/control/project_cmd.cpp` (`project ...`, `man project`).

## 3. O banco

`meta(key,value)` (esquema, nome, id, data), `config(key,value)`, `files(id, path UNIQUE, kind, content BLOB, size, hash, mtime)`. O `kind` so' diz ao IDE
qual editor abrir; o arquivo e' guardado no formato nativo (`.bas`, `.asm`, `.dsk`...). `id` identifica o projeto: vai no manifesto e no `.msxsync`, e
sobrevive ao ciclo clone/import. Ao abrir: `PRAGMA quick_check`, recusa arquivo que nao e' projeto ou de um esquema mais novo.

## 4. As copias soltas e a sincronizacao

- A pasta padrao e' a que contem o `.msxproject`; `project root <pasta>` muda (o usuario escolhe a raiz em cada PC).
- **`.msxsync`** (na pasta, local, fora do git): o hash da ultima sincronizacao de cada arquivo. NAO vai no banco: se fosse, ao copiar o `.msxproject` para
  outro PC a pasta nova pareceria "tudo apagado". Traz o `id` do projeto; de outro projeto, e' ignorado (sem base, o que difere vira conflito, nada e' apagado).
- Estados: `igual`, `pasta mudou` (-> projeto), `projeto mudou` (-> pasta), `CONFLITO`, `so na pasta` (entra no projeto), `so no projeto` (extrai),
  `sumiu da pasta` (so' sai do projeto com `--prune`; sem ele o projeto guarda e `pull` restaura).
- **Nunca se apaga uma copia solta**: o que so' existe na pasta volta para o projeto.
- **Texto**: o hash ignora CR (LF x CRLF nao e' alteracao; o git no Windows troca sozinho). Binario: cada byte conta.
- Ignorados na pasta: pastas e arquivos que comecam com ponto (`.git`, `.msxsync`), `msxproject.txt`, o proprio `.msxproject` e seus journals.
- **Seguranca**: todo caminho passa por `PathOk` (Fortran) ao entrar no projeto E ao extrair; um banco mexido na mao com `../evil.txt` vira erro, nunca
  escreve fora da raiz (testado).
- **Manifesto** `msxproject.txt` (texto, gerado no `sync`/`export`): nome, id, esquema, `[config]` e `[files]` (tipo e caminho). E' o que o `import` le.
- Data: ao extrair, a copia solta recebe a data do projeto; ao importar da pasta, o projeto recebe a data do arquivo. So' serve para sugerir "o mais novo".

## 5. Comandos (`man project`)

`project new|open|close|info|ls|add|get|rm|set|root|status|sync [--prune]|pull|push|export|import`. `sync` aplica o que nao perde trabalho e lista os conflitos
com a sugestao; `pull <caminho>` deixa o projeto, `push <caminho>` deixa a pasta. API para o editor: `Project::Save(path, bytes, root)` grava no banco E na pasta numa vez.

## 6. Testes

`projecttest` (ctest `project`, 75 verificacoes): caminhos validos e invalidos, tipos, hash com CR, banco (criar, reabrir, sem `-wal`), sincronizacao de todos
os estados, conflito nos dois sentidos, `prune`, `Save`, copia do `.msxproject` para outro PC (extrai tudo, nada apagado), pasta de OUTRO projeto, import
da pasta sem `.msxsync` (o "clone do GitHub"), caminho malicioso. `controltest`: o comando `project` pela ponte.

## 7. Em aberto (proximas fatias)

- **Integracao com o editor**: abrir/salvar pelo projeto (`Save` ja' existe), arvore de arquivos, aviso de "o `.msxproject` e' mais novo / a pasta e' mais nova"
  ao abrir, dialogo de conflito com as duas versoes. Montar/Dignified lendo do projeto.
- A configuracao do **manifesto so' vale no import**; o `sync` a regrava a partir do banco (config editada so' no manifesto, depois de um `git pull`, nao entra).
- **Apagados no projeto**: apagar um arquivo do projeto em outro PC nao apaga a copia solta (so' volta como "so na pasta"); falta registro de remocoes.
- Renomear e' apagar + criar. Nomes que so' diferem por maiuscula (Windows x Linux): falta aviso.
- Receita de build (`Obifile`), `revisions` (migracao dos projetos do PaleoBasic/msxIDE DISPENSADA pelo usuario: serao abandonados), trava de dois processos no mesmo projeto.

## 8. O projeto no editor (1.43.1)

- **Menu Project** (`Alt-J`): New project..., Open project..., Close project, Files..., Add current file, Sync.... Sem projeto aberto os quatro ultimos ficam cinza.
- **Salvar**: o modelo do editor conta as gravacoes (`Editor::SaveCount`); `EditorApp::ProjectFlush` (depois de cada tecla, clique, dialogo e autosave) compara com `ProjectSeen` e faz `Project::Push` do arquivo salvo dentro da pasta. Fora da pasta, copia de seguranca (`.{`) e pasta com ponto: `prj::IsIgnored`.
- **Sync...**: `Status` -> lista (so' as diferencas, em ingles como o resto do editor) -> OK = `Project::Sync` (o que e' seguro) -> uma pergunta por conflito/apagado (`Dialog::ProjectConflict`, botoes Project/Folder/Skip/Cancel, o lado mais novo marcado). Abas abertas e intactas sao recarregadas; abas com alteracoes avisam.
- **Files...**: `Project::List` com tipo e caminho; Enter abre a copia solta (extraindo antes se so' existir no projeto); so' texto.
- Ao criar ou abrir um projeto a lista do Sync aparece sozinha se houver diferencas.
- Teste: `editorui` (`test_project_editor`, ~30 verificacoes: criar numa pasta com arquivos, salvar, Save As dentro/fora da pasta, backup, Files, edicao externa, conflito, apagado, reabrir).
- Em aberto: lembrar o ultimo projeto entre sessoes, arvore de arquivos na janela grafica, nome do projeto na barra de status, Build/Dignified lendo do banco (hoje leem a pasta, que o Sync mantem igual).

## 9. Lembrar o projeto, barra de status e arvore (1.43.2)

- **`fwedit.project`**: uma linha com o caminho absoluto do ultimo `.msxproject`, ao lado do `fwedit.cfg` (`ProjectMemoPath`). Gravado por `ProjectNew/Open`; `ProjectClose(true)` (o usuario fechou) o apaga; `ProjectClose(false)` e' so' a troca. `LoadConfig` chama `ProjectRestore`: reabre sem abrir o Sync, e se o arquivo sumiu nao faz nada.
- **Barra de status**: `Editor::RenderStatus(grid, y, project)`; no TUI `[nome]` na coluna 22 (9 letras); na janela `Projeto: nome`.
- **Arvore** (so' na janela grafica): `EditorPanel::DrawProjectTree`, pastas = nos, arquivos = folhas, clique = `EditorApp::OpenProjectFile`. Ordem com `/` antes dos demais caracteres para agrupar as pastas.
- **Arvore no TUI (1.43.3)**: o dialogo *Files...* e' a arvore (`ProjectFilesDialog`/`ProjectTreeEnter`): `[-] pasta/` + arquivos recuados; Enter recolhe/expande (reconstroi o dialogo no mesmo item) ou abre.
- **Menu de contexto (1.43.4)**: `ProjAct` {Open, Push, Pull, Rename, Add, Remove}; `EditorApp::ProjectDo` e' o unico caminho (popup do ImGui via `ProjectAct`; no TUI o botao Actions... do Files... abre `Dialog::ProjectActions`, e Rename/Add pedem o texto em `ProjectRename`/`ProjectAddFile`). Renomear: mover a copia solta + `Drop` + `Push`; tirar do projeto nunca apaga a copia da pasta.
- Em aberto: `Obifile`.

## 11. Build lendo do projeto (1.43.7)

- `badig::SetSourceReader(fn)` (em `badig.h`, guardado em `badig_base.cpp`): o `LoadFile` do `include` e o `SourceReadable` (a checagem "existe?" do include e da biblioteca) consultam o leitor antes do disco. Vazio = so' disco.
- Editor: `EditorApp::ProjectSourceReader()` traduz o caminho para o relativo do projeto (`ProjectRel`) e devolve `Project::Get`; `BuildDignified` liga o leitor num escopo RAII (`BadigReaderScope`) so' durante `badig::Convert`. O `DiskFiles` do `BuildAssemble` recebe o mesmo tipo de leitor (tenta `dir+path`, depois `path`) antes do disco.
- Fora do escopo: `disk_files` (arquivos do .dsk) e o Run (F5), que leem a pasta.

## 10. Historico, remocoes e nomes (1.43.5, esquema 2)

- **Tabelas** (criadas por `kTables`, tambem na migracao do esquema 1): `revisions(id,path,hash,content,size,mtime,saved)` e `tombstones(path,hash,removed)`.
- **Historico**: `Project::Archive` (chamado por `PutRaw` quando o hash muda e por `Remove`) copia a linha atual para `revisions` com `INSERT..SELECT`, pula se a ultima versao tem o mesmo hash ou se passa de 8 MiB, e corta para `revisions.keep` (padrao 10). `Restore` = `Put` (que arquiva a de agora) + `Pull`.
- **Lapide**: `Remove` arquiva, grava `(path, hash)` e apaga a linha; `PutRaw` apaga a lapide. No `StatusImpl`, arquivo so' na pasta COM lapide vira `ProjectRemoved` (copia igual ao hash) ou `RemovedChanged`, ambos `Ask`. `Sync` os lista em `removed`; com `prune` apaga so' o `ProjectRemoved`. `DeleteLoose` apaga a copia (e a base). `Drop` mantem a copia e grava `!hash` na base, que faz o Status ignorar essa copia nesta pasta.
- **Maiusculas**: `PutRaw` recusa nome com `lower(path)` igual ao de outro arquivo (ASCII).
- **Manifesto**: `AdoptManifestConfig` (no inicio do `Sync`) copia as chaves de `[config]` que o banco nao tem, se o `id=` bate.
- **Editor**: `ProjAct::History` (`Dialog::ProjectHistory`) e `ProjAct::Removed` (`Dialog::ProjectRemoved`); a pergunta da remocao reaproveita o dialogo de conflito (padrao = manter a copia).
