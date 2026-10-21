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
- Receita de build (`Obifile`), `revisions`, migracao dos `.msxproject` do PaleoBasic e `.msxproj` do msxIDE, trava de dois processos no mesmo projeto.
