# Downloads -- File Hunter e utilitarios, item por item

Versao 1.43.17. Codigo em `src/download/` (`download.h/.cpp`, `download_cli.cpp`, `c/dl_match.c`) e na janela `src/machine/gui/download_window.{h,cpp}`. Teste: `downloadtest` (sem internet: o `Fetch` baixa um arquivo local por `file://`).

## 1. O que e'

Pedido do usuario (2026-10-22): uma opcao do menu que abre uma janela onde se baixa, **item por item**, ROMs, DSK, PDF... do File Hunter e os utilitarios de sites especificos (por exemplo o NBASIC, o SEE e o NPLAYER do Konamiman). Decisoes (AskUserQuestion): janela unica com abas por fonte; busca por um indice baixado uma vez (`allfiles.txt`); o `NBASIC.BIN` sai dos pacotes e passa a ser baixado sob demanda.

Menu **ROMs > Downloads (File Hunter e utilitarios)...**. O item antigo de navegar foi renomeado **Navegar System ROMs (entram no banco)**: ele continua porque, alem de baixar, **extrai e cadastra** as ROMs de sistema no banco de ROMs (`doc/romdb-spec.md`), coisa que a janela nova nao faz.

## 2. File Hunter -- sem martelar o servidor

O proprio site (`Read Me & Welcome.txt`) pede que ninguem baixe agressivamente: quem o faz leva ban de IP, e a copia completa existe como torrent. Por isso:

- O site inteiro (~59 mil arquivos) e' navegado e pesquisado em cima de **um unico arquivo**, o `allfiles.txt` (~3,5 MB), baixado uma vez para `downloads/filehunter/allfiles.txt` e relido so' quando o usuario clica em **Atualizar indice**. Navegar entre pastas e procurar **nao fazem requisicao nenhuma**.
- Cada arquivo so' e' baixado quando o usuario clica em **Baixar**, **um de cada vez**, com **intervalo minimo de 1,2 s** entre dois downloads (`dl::Fetch`; vale para a janela e para a linha de comando). Nao ha' "baixar a pasta" nem "baixar tudo".
- O formato do indice: UTF-8 com BOM, um caminho por linha, CRLF, `\` entre as pastas (`Games\Konami\Nemesis.zip`). Sem tamanhos nem datas.
- A busca exige **todas** as palavras (sem diferenciar maiusculas) em qualquer parte do caminho; mostra ate' 400 resultados e avisa quando ha' mais.

## 3. Utilitarios (lista curada)

Compilada no programa (`dl::Catalog()`); cada item tem URL, destino e, opcionalmente, uma copia a mais ("install"). Hoje, todos do repositorio `github.com/Konamiman/MSX` (arquivos em `raw.githubusercontent.com`, `SRC/NBASIC`):

| Id | O que baixa | Destino |
|---|---|---|
| `nbasic` | `NBASIC.BIN`, `NBASIC.DAT`, `NBASIC.ASM`, `nbas111e.txt` | `downloads/konamiman/nbasic/`; o `NBASIC.BIN` **tambem** vai para `badig/` (onde o Basic Dignified o procura) |
| `see` | os 10 arquivos de `see/` (SEE 3.10) | `downloads/konamiman/see/` |
| `nplayer` | `NPLAYER.BAS/.DAT/.PRG/.TXT` e `NPLAYER.ASM` | `downloads/konamiman/nplayer/` |

O repositorio do Konamiman **nao declara licenca** (`NOASSERTION` na API do GitHub). Por isso o fwMSX nao redistribui nada dele: o usuario baixa do site do autor.

## 4. Seguranca

- **Nenhum nome vindo do servidor chega a um comando de shell**: a URL tem cada trecho do caminho codificado (`%XX`) e o `curl` grava num temporario nosso (`downloads/.part/N.part`); o nome final so' e' usado por `std::filesystem` depois.
- O destino passa por `SafeRelative` (recusa `..`, caminho absoluto, `C:`, barra invertida, componente vazio ou `.part`) e cada trecho por `LocalComponent` (troca `<>:"|?*\` e controles por `_`).
- Um download interrompido nunca deixa meio arquivo no destino (baixa para `.part` e so' entao renomeia).

## 5. Linha de comando

`fwmsx --download <comando>` (tambem no `fwMSXc`):

| Comando | Faz |
|---|---|
| `catalog` | lista os utilitarios e seus arquivos |
| `get <id>` | baixa os arquivos de um utilitario |
| `index [--force]` | baixa o indice do File Hunter |
| `ls [pasta]` | lista uma pasta pelo indice (sem acessar o site) |
| `search <palavras>` | procura no indice (100 resultados) |
| `fetch <caminho>` | baixa UM arquivo do File Hunter (o caminho como o `ls`/`search` mostram) |

Tudo vai para `downloads/` ao lado do executavel. Os arquivos baixados do File Hunter ficam em `downloads/filehunter/<mesmo caminho>`; para pô-los no banco de ROMs use **ROMs > Escanear pasta**.

## 6. O NBASIC.BIN

Saiu do repositorio (`.gitignore`) e dos pacotes (`build.ps1`/`build.sh` o removem do pacote mesmo que sobre um em `dist/badig/`). O Basic Dignified, quando precisa dele e nao acha, diz onde baixar: janela Downloads > Utilitarios > NestorBASIC, ou `fwmsx --download get nbasic`. O `nbasic.dmx` (as rotinas `.NB_*`, vindas do msxIDE) continua indo no pacote.

## 7. Fora desta versao

- cadastrar no banco de ROMs o que veio do File Hunter fora de `System ROMs/` (hoje: Escanear pasta);
- tamanho e data dos arquivos na lista (o indice nao os traz; exigiria a listagem de cada pasta, uma requisicao por pasta);
- barra de progresso por arquivo (o `curl` roda sem saida de progresso; a janela mostra "Baixando ...");
- mais fontes no catalogo (outros utilitarios e repositorios, conforme o usuario pedir).
