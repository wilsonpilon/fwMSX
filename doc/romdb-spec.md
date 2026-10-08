# Banco de ROMs -- especificacao (documento vivo)

> Mesmo espirito dos outros documentos vivos: o que foi feito, as decisoes e o que
> falta.

Estado: **em 2026-10-06, nao lancado.** Um banco SQLite com as ROMs que o usuario
tem no disco (pelo SHA-1), a tabela de mappers do `CARTS.SHA` do fMSX e o banco de
referencia do Vampier (msxromdb). Downloads do fMSX 6.0 para Windows, do System ROMs
do file-hunter (com navegacao pelas pastas) e do Vampier, pela CLI e pelo menu
**ROMs** da janela.

**O repositorio nao distribui ROMs.** As ROMs (BIOS, extensoes, jogos) sao baixadas
pelo usuario, de fontes publicas, e ficam fora do git (`roms/` e `dist/roms/` estao
no `.gitignore`). O banco guarda o hash, o nome e o caminho local, nao o conteudo.

## 1. Pasta de ROMs

- Padrao: `roms/` ao lado do executavel (`dist/roms/` quando rodado de `dist/`).
  Opcao `--roms <pasta>` na CLI.
- Dentro dela, uma pasta por tipo (`bios`, `interfaces`, `cartuchos`, `discos`,
  `tabelas`, `outros`) e `filehunter/<data>/` para o pacote System ROMs com a
  estrutura de pastas original.
- `roms.db` e' o banco; `_tmp/` existe so' durante os downloads.

## 2. Banco (`src/romdb/store/romdb.{h,cpp}`, SQLite)

- `roms`: `sha1` (unico, minusculo), `crc32`, `size`, `name` (vazio ate' ser
  identificado), `category` (bios, interface, cartucho, disco, tabela, outro),
  `hardware` (texto livre), `mapper` (numero do fMSX, -1 = nao informado), `path`
  (relativo a pasta de ROMs, sempre com `/`), `source` (fmsx, filehunter, vampier,
  scan, manual), `notes`, `updated`.
- `cart_mappers`: `sha1` -> `mapper`, importado do `CARTS.SHA`. Quando uma ROM
  e' cadastrada, o mapper e' preenchido por esta tabela. **Desde a 1.25.0**, tambem e' consultada
  ao CARREGAR um cartucho: `--msx --cart <arquivo>` sem mapper explicito calcula o SHA-1 do
  arquivo e chama `RomDb::CartMapper()` ANTES de cair na heuristica por tamanho/conteudo de sempre
  (`memmap::GuessMapper()`) -- ver `src/machine/cli.cpp::TryMapperFromRomDb()`. O numero do mapper
  do fMSX (0-5, `CARTS.SHA`) e' convertido para o enum publico `MemMapMapperType` somando 1
  (`FmsxMapperToMemMap()`, ja' que `MEMMAP_MAPPER_NONE=0` vem antes de `GEN8` no enum).
- `meta`: reservado.
- `msxdb_romdetails`, `msxdb_rominfo`, `msxdb_company`: as tabelas do Vampier, criadas
  pelo proprio dump SQL e recriadas a cada importacao.
- **Add** com SHA-1 ja cadastrado atualiza caminho, origem e tamanho, e mantem nome,
  hardware e notas do usuario.
- **Identificar** preenche o nome das ROMs com nome vazio, pelo SHA-1 no Vampier. Nao
  troca nomes que ja existem.

## 3. Classificacao (`src/romdb/store/classify.{h,cpp}`)

Pelo nome do arquivo: `.sha`/`.db`/`.json`/`.sql` -> tabela; `.dsk` -> disco;
`DISK`, `FDC`, `EXT`, `FMPAC*`, `PAINTER*` -> interface; `MSX*.ROM` sem `EXT` -> bios;
demais `.rom`/`.bin`/`.mx1`/`.mx2`/`.eprom`/`.dat` -> cartucho; o resto -> outro.
A categoria e' editavel depois.

## 4. Downloads (`src/romdb/service.{h,cpp}`, `src/romdb/net/`, `src/romdb/archive/`)

| Fonte | O que faz |
|---|---|
| `https://fms.komkon.org/fMSX/` | acha o pacote `fMSXNN-Windows-bin.zip` de versao mais alta (hoje 6.0), extrai e separa as ROMs por tipo |
| `https://download.file-hunter.com/System%20ROMs/` | lista as pastas e os arquivos; o "Full Set System ROMs for OpenMSX" mais recente (ou o de uma data) e' baixado, extraido em `filehunter/<data>/` e cadastrado |
| `https://romdb.vampier.net/Archive/sql-msxromdb.zip` | baixa o dump SQL e o executa no banco (o JSON nao e' usado) |

- **HTTP**: pelo programa `curl` (vem no Windows 10 1803+ e na maioria dos Linux).
  O User-Agent e' de navegador, porque o file-hunter responde 405 ao padrao do curl.
  Sem curl, os downloads falham com a mensagem explicando.
- **ZIP**: biblioteca miniz 3.0.2 (dominio publico/MIT, baixada pelo CMake).
  Caminhos com `..`, absolutos ou com letra de unidade sao ignorados.
- **Indice do file-hunter**: HTML estilo Abyss ("Index of"). O parser le as linhas
  `<TR>`, o `HREF`, o tamanho e o tipo, e decodifica entidades HTML (`&#39;`) e `%XX`.
  Se o servidor mudar o layout, o parser precisa de ajuste (ver secao 8).
- **Fontes da listagem de datas**: o Full Set e' escolhido pela data no nome
  (`DD-MM-AAAA`); a mais recente vence.

## 5. CLI (`fwmsx --romdb <comando>`)

Downloads: `fmsx`, `filehunter [caminho]`, `filehunter-full [--data DD-MM-AAAA]`,
`filehunter-get <url>`, `vampier`.
Banco: `scan <pasta>`, `cartsha <arquivo>`, `add <arquivo> [--cat --hw --nome --notas]`,
`list [--cat]`, `search <texto> [--cat]`, `show <id|sha1>`,
`edit <id> [--cat --hw --nome --mapper --notas]`, `del <id>` (o arquivo nao e' apagado),
`vsearch <texto>`, `identify`, `verify [--cat]` (recalcula o SHA-1 de cada ROM e confere contra o
banco -- codigo de saida 1 se achar arquivo faltando ou alterado, ver secao 8), `stats`.

## 6. Janela (`src/machine/gui/rom_manager.{h,cpp}`)

- **Menu ROMs**: Baixar fMSX 6.0, Baixar System ROMs (mais recente), Navegar
  file-hunter, Baixar banco do Vampier, Banco de ROMs, Escanear pasta, Identificar.
- **Tarefas**: os downloads rodam numa thread de trabalho, com a propria conexao do
  banco. A janela mostra o progresso e o erro, e a maquina continua rodando.
- **Banco de ROMs**: busca por texto e categoria, lista, edicao (nome, categoria,
  hardware, mapper, notas), exclusao do cadastro, adicionar arquivo, e busca no
  banco do Vampier.
- **Navegar file-hunter**: pastas (Abrir, Subir, Raiz) e arquivos (Baixar).

## 7. Verificacao

- `romdbtest` (CTest `romdb_store`): SHA-1 e CRC32 com vetores padrao; classificacao;
  URL; indice do file-hunter (inclusive entidades HTML); data e escolha do Full Set;
  mapper do fMSX; ZIP (escrever, extrair, recusar `..`); banco (Add, Get, Update,
  Search com filtro, Delete, CARTS.SHA, dump do Vampier, identificar, escanear).
- **Real, pela CLI, com a rede**: `fmsx` baixou o fMSX 6.0 e separou 9 arquivos;
  `cartsha` importou 751 linhas; `vampier` importou 11.304 ROMs; `filehunter-full`
  baixou o Full Set de 15-08-2026 (687 arquivos processados, 534 ROMs unicas);
  `identify` deu nome a 531 ROMs; `filehunter-get` baixou um arquivo avulso; o CRUD
  (`show`, `edit`, `del`) foi conferido numa ROM real.
- **Janela**: menu ROMs, Banco de ROMs, Navegar file-hunter -- **confirmados na tela pelo usuario
  em 2026-10-07** (ver `doc/SPEC.md`, secao 5.0).

## 8. O que falta

- **Janela**: barra de progresso; cancelar um download; continuar um download interrompido.
- **Vampier em JSON**: so' o SQL foi usado. O JSON (`json-msxromsdb.zip`) nao foi
  importado.
- **Parser do file-hunter**: depende do HTML atual do Abyss. Se mudar, o parser
  precisa de ajuste.
- [x] **Verificar as ROMs baixadas contra o SHA-1 conhecido** (1.25.0): `fwmsx --romdb verify`
  recalcula o SHA-1 de cada ROM cadastrada e confere contra o banco -- reporta arquivo faltando ou
  alterado desde que foi cadastrado, codigo de saida 1 se achar algum problema. Ver secao 5
  (comandos) e `doc/CHANGELOG.md`, `[1.25.0]`.
- [x] **Carregar pelo banco: escolher o mapper** (1.25.0): `--cart <arquivo>` SEM mapper explicito
  agora consulta o banco (`CARTS.SHA` ja' importado) pelo SHA-1 do cartucho ANTES da heuristica por
  tamanho/conteudo de sempre (`memmap::GuessMapper()`, que continua intacta como fallback). Montar
  a maquina pelo banco (layout por nome) continua pendente.
- **Frontend para jogar** (biblioteca de jogos e imagens): depois, sobre este banco.
- **Controle externo** (estilo openMSX): outra fase, em `doc/SPEC.md`.
- **Politica de distribuicao**: as ROMs nao vao no pacote. Esta regra precisa ser
  confirmada antes de qualquer publicacao com ROMs.

## 9. Arquivos

- `src/romdb/core/hash.{h,cpp}` -- SHA-1 e CRC32.
- `src/romdb/net/web.{h,cpp}` -- HTTP pelo curl.
- `src/romdb/net/listing.{h,cpp}` -- indice de pasta (Abyss), Full Set, fMSX.
- `src/romdb/archive/unzip.{h,cpp}` -- ZIP (miniz).
- `src/romdb/store/romdb.{h,cpp}` -- banco SQLite.
- `src/romdb/store/classify.{h,cpp}` -- categorias e pastas.
- `src/romdb/service.{h,cpp}` -- fluxos de download.
- `src/romdb/cli.{h,cpp}` -- `fwmsx --romdb` (inclui `verify`, 1.25.0).
- `src/machine/gui/rom_manager.{h,cpp}` -- menu ROMs e janelas.
- `src/machine/cli.cpp` -- `TryMapperFromRomDb()`/`FmsxMapperToMemMap()` (1.25.0): consulta o
  banco de ROMs para escolher o mapper do cartucho, ver secao 2.
- `tests/romdb/romdb_test.cpp` -- `romdbtest`.
