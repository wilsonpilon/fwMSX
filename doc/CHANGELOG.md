# Changelog

Resumo das alteracoes entre versoes do fwMSX, ligado ao
[README.md](../README.md). Formato inspirado em
[Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/). Para o
detalhamento completo de cada versao (nome do jogo de MSX + subtitulo,
notas de build) veja [RELEASE.md](RELEASE.md); para a especificacao viva
e o historico de fases, veja [SPEC.md](SPEC.md).

## [1.43.17] - 2026-10-22 - "Yie Ar Kung-Fu: Downloads" (Konami)

### Adicionado -- janela de downloads item por item (pendencia "downloader de ROMs/discos")
- **ROMs > Downloads (File Hunter e utilitarios)...**: janela com duas abas. **File Hunter**: o site inteiro (Games, Manuals, Books, Programs, Source Codes... ~59 mil arquivos) navegado e pesquisado pelo `allfiles.txt` do proprio site, baixado uma vez (~3,5 MB); navegar e procurar nao acessam o servidor, e cada arquivo so' e' baixado quando voce clica, um por vez, com 1,2 s de intervalo minimo (o site bane quem baixa em massa). **Utilitarios**: lista curada, hoje do Konamiman (`nbasic`, `see`, `nplayer`) direto do GitHub dele.
- `fwmsx --download catalog | get <id> | index | ls | search | fetch` (e `man download`): o mesmo sem janela.
- O item antigo virou **Navegar System ROMs (entram no banco)**: continua porque extrai e cadastra as ROMs de sistema no banco.
- **`NBASIC.BIN` saiu do repositorio e dos pacotes** (o repositorio do Konamiman nao declara licenca); baixa-se por `get nbasic`, que o copia tambem para `badig/`. O Basic Dignified, sem ele, diz onde baixar.
- Seguranca: nenhum nome vindo do servidor chega a um comando de shell (URL codificada, temporario nosso), destino so' relativo e sem `..`, download interrompido nao deixa meio arquivo.
- `fwMSX-splash.png` e' arte propria (registrado no `LICENSE-THIRD-PARTY.md`).
- Teste: `downloadtest` (indice, busca, pastas, caminhos seguros, catalogo, `Fetch` com `file://`); o teste do Badig passou a usar um `NBASIC.BIN` de mentira numa biblioteca temporaria.

## [Sem versao] - 2026-10-22 - revisao das licencas de midia (so' documentacao)
- `doc/media-review.md`: o levantamento do que o repositorio PUBLICO rastreia de terceiros (1221 `.rom`, 266 `.dsk`, 136 `.pdf`, 105 `.zip`; 19 ROMs de jogos comerciais em `tests/media/`; 637 ROMs de sistema em `dist/roms/`; pacotes antigos em `dist/`), o risco por grupo e uma recomendacao em ordem de custo. Nada removido.
- **Aplicado o passo 1 da recomendacao**: 818 arquivos de midia de terceiros (`tests/media/`, `dist/roms/`, `dist/*.zip|gz`, `*.sst`, capturas soltas) deixam de ser rastreados e vao para o `.gitignore`; continuam no disco; o historico nao foi alterado.
- `LICENSE-THIRD-PARTY.md`: registra que o repositorio e' publico, aponta o levantamento e lista as bibliotecas baixadas pelo CMake que faltavam (SQLite, CLI11, replxx, FTXUI, ImGui, GLFW).

## [1.43.16] - 2026-10-22 - "Yie Ar Kung-Fu: Scripts em Lua" (Konami)

### Adicionado -- scripts no console (o spike do Lua combinado em 2026-10-22)
- **`lua <codigo|arquivo.lua|stop|status>`** e **`source <arquivo>`** no console (TUI, janela, servidor TCP). O Lua 5.4.7 (MIT) vem por `FetchContent` no build, com hash fixo; `src/script/` tem o `Engine` (C++).
- **Tabela `msx`**: `cmd` (qualquer comando do console), `peek`/`poke`/`peek16`/`poke16`, `reg`, `frame`, **`wait(n)`** (corrotinas: o script espera quadros sem travar o emulador) e os ganchos `on_frame`, `on_change(addr, fn)` e `on_stop`, mais `clear_hooks`. Nada roda por instrucao do Z80: os ganchos olham a maquina entre um quadro e outro.
- **Seguranca**: o Lua nasce so' com base/table/string/math/coroutine/utf8 (sem io, os, package, debug, dofile, require) e cada chamada tem 20 milhoes de instrucoes (um laco infinito vira erro). A saida de um script que acorda depois vira o evento `! lua ...`.
- `man lua`, `doc/script-spec.md`; o Lua entra no `LICENSE-THIRD-PARTY.md`.
- Teste: `controltest` (`TestLua`: expressoes, memoria, registradores, `msx.cmd`, sandbox, limite, corrotinas, os tres ganchos, `lua stop`, arquivo `.lua`, `source`).
- Fica para depois: `.lua` como tipo do projeto e Run > Run script no editor, ganchos por escrita com custo (`on_write`), `msx.type`/VRAM/VDP/PSG.

## [1.43.15] - 2026-10-22 - "Yie Ar Kung-Fu: Markdown colorido" (Konami)

### Adicionado -- fecha a fase E7 do editor
- **Preview do Markdown com cores, na propria janela** (`F7` alterna entre o texto puro e o renderizado; `Esc` tambem volta). Substitui o dialogo da 1.43.14. Na TUI e na janela grafica: titulo 1 (maiusculas, `===`), titulo 2 (`---`), titulo 3, **negrito**, *italico*, `codigo`, links (texto e `(url)` em cores diferentes), imagens, citacoes, marcadores de lista, blocos de codigo, tabelas e linha horizontal, cada tipo com a sua cor do esquema atual (12 classes de cor novas, `kSynMd*`). Rola com setas, PgUp/PgDn, Home/End e a roda do mouse; a barra de estado mostra `Preview`. No preview a janela e' so' leitura (nada edita o texto).
- **Arquivos maiores que a memoria** (`File > Load more`, `Write to cursor`, `Top of file`), antes desabilitados: o `Load` traz no maximo `load_limit` bytes (padrao 256 MiB, chave `load_limit_kb` no `fwedit.cfg`; 0 = tudo), cortando numa quebra de linha; **Load more** traz a proxima parte; **Write to cursor** manda as linhas acima do cursor para um temporario e libera a memoria; **Top of file** traz o topo de volta. O **Save junta tudo** -- o topo gravado, o buffer e o resto do original por ler -- num arquivo ao lado que so' substitui o original no fim (o original e' a fonte do que falta); o Ctrl-Z e o CRLF do arquivo continuam como estavam. Se o arquivo mudou no disco, o Load more recusa. Enquanto o arquivo esta' em partes a barra mostra `Part`; montar (F9) e a sessao ignoram o editor (so' veriam um pedaco do texto).
- Testes: `edittest` (arquivo em partes byte a byte, Write to cursor, Top of file, Ctrl-Z; classes do Markdown) e `editorui` (o preview, as cores e o bloqueio da edicao).
- **E7 encerrada.** Fica de fora, por decisao: preview dividido ao vivo (texto de um lado e renderizado do outro).

## [1.43.14] - 2026-10-22 - "Yie Ar Kung-Fu: Outline e preview" (Konami)

### Adicionado -- fase E7 do editor, parte 3
- **View > Outline display** (antes desabilitado): lista as linhas que importam, com o numero, e `Enter` leva o cursor a' escolhida. Depende da linguagem: Markdown = os titulos (`#`), recuados pelo nivel e sem contar os `#` dentro de blocos de codigo; MSX-BASIC = as linhas de comentario (`REM` e `'`), que fazem papel de titulo; Assembly = rotulos, constantes e diretivas na coluna 1 (sem os comentarios); texto e demais = as linhas que comecam na coluna 1 (a regra do Norton Editor). A lista abre no item onde o cursor esta'.
- **View > Markdown preview (`F7`)**: o texto Markdown renderizado numa janela de rolagem que ocupa a tela toda (igual na TUI e na janela grafica; `Esc` fecha). Titulos 1 e 2 com sublinhado, listas com recuo pendurado e aninhadas, citacoes, blocos de codigo, tabelas, linha horizontal, negrito/italico/codigo sem as marcas, `[texto](url)` como `texto (url)` e imagens como `[imagem: alt]`. `markdown.h/.cpp` (`RenderMarkdown`, `MarkdownInline`). Preview ao vivo dividindo a tela e cores ficam para depois.
- Testes: `edittest` (Outline por linguagem e o renderizador) e `editorui` (os dois dialogos).

## [1.43.13] - 2026-10-22 - "Yie Ar Kung-Fu: Formatar texto" (Konami)

### Adicionado -- fase E7 do editor, parte 2
- **Edit > Paragraph format** (`Alt-E P`) e **Edit > Document format** (`Alt-E G`), antes desabilitados: reflui o texto pelo `Tools > Text layout setup` (largura da linha, margem esquerda e recuo da 1a linha). O paragrafo vai da linha do cursor ate' a linha em branco; o cursor vai para o paragrafo seguinte (repetir o comando formata o texto todo, um a um). O Document format pergunta se comeca do topo ou do paragrafo do cursor e trata a quebra de pagina como separador. Tudo desfaz de uma vez (`Alt-Backspace`); em modo somente leitura ficam desabilitados.
- **O pacote de distribuicao passa a levar a pasta `emulator/`** (arquivos do MSX-DOS 1 e 2 + `msxdos1.dsk` e `msxdos2.dsk`), por decisao do usuario; o manual diz onde colocar os arquivos se a pasta faltar.
- Testes: `edittest` (reflow, margens, quebra de pagina, desfazer, somente leitura) e `editorui` (dialogo do Document format).

## [1.43.12] - 2026-10-22 - "Yie Ar Kung-Fu: Discos do MSX-DOS" (Konami)

### Adicionado
- **Pasta `emulator/` ao lado do executavel** (`dist/emulator/`, criada pela build): `msxdos1/` e `msxdos2/` (copiados de `resource/`) e os discos de sistema `msxdos1.dsk` e `msxdos2.dsk`, gerados pelo proprio `obi`. Nao entra nos pacotes de distribuicao (licenca de terceiros).
- **`obi`: `dos: 1` / `dos: 2`** monta o disco com o MSX-DOS 1 ou 2 a partir dessa pasta (disco em branco + MSXDOS.SYS/COMMAND.COM, ou MSXDOS.SYS/MSXDOS2.SYS/COMMAND2.COM) mais os arquivos da receita; **`dos_tools: true`** leva tambem UTILS, HELP e o resto do MSX-DOS 2. `boot: X.COM` gera o `AUTOEXEC.BAT`. `dos: true` + `dos_disk:` continua valendo.
- **`obi dosdisk <1|2> [saida.dsk] [--tools]`** (CLI e `fwmsx --obi`): o disquete de sistema puro.
- Teste de ponta a ponta no `controltest`: `obirun` com `dos: 1` inicia o MSX-DOS 1 no emulador, que roda o `AUTOEXEC.BAT` e um `.COM` que imprime na tela. `obitest` confere os arquivos de cada variante.

## [1.43.11] - 2026-10-22 - "Yie Ar Kung-Fu: Receita de disco" (Konami)

### Adicionado -- o `obi`: receita (Obifile) que monta um disquete pronto para rodar
- **`src/obi/`** (nas 4 linguagens): C++ (receita, build e comando), C (`obi_name`: nome 8.3 e tipo pela extensao), Fortran (`obi_fit`: clusters), NASM dual-ABI (`obi_crc`: CRC-32). Sintaxe do `Obifile` do Kizuna (subconjunto), mas usando as ferramentas do fwMSX: `.dmx` -> `.BMX` (badig), `.asm` -> `.BIN`/`.COM`/`.ROM` (fwasm), o resto copiado como esta'; tudo num `.dsk` de 720 KB (`msxdisk`).
- Campos: `target`, `boot`, `dos`, `dos_disk`, `files` (`source`, `name`). `boot` gera o `AUTOEXEC.BAS` (`RUN`/`BLOAD ,R`) ou `AUTOEXEC.BAT` (.COM, com `dos: true`).
- **Comandos**: `obi build|check [Obifile]` e `obirun [Obifile]` (monta, insere em A: e reinicia) no console; `fwmsx --obi ...` na linha de comando; **Run > Build disk (`F6`)** no editor, lendo os fontes dos buffers abertos, do banco do projeto e da pasta (nesta ordem). Erros viram mensagens com arquivo e linha (F8 / Shift-F8).
- `man obi` e `doc/obi-spec.md`.
- Testes: `obitest` (novo: nomes, CRC, clusters, receita, build, erros, leitor do projeto), `editorui` (F6) e `controltest` (`obirun` roda o `.BMX` no DISK BASIC).

## [1.43.10] - 2026-10-22 - "Yie Ar Kung-Fu: Imprimir em PDF" (Konami)

### Adicionado -- fase E7 do editor, parte 1
- **File > Print / Print Block** (antes desabilitados): pedem o nome (sugerem `arquivo.pdf` / `arquivo-block.pdf`) e gravam um PDF A4 em Courier pelo gerador do Mamute. O `Print setup` agora vale: **Top margin** (linhas em branco no alto de cada folha), **Left margin** (colunas em branco) e **Page length** (linhas de texto por folha); uma linha so' com form feed (as quebras de pagina de Tools > Insert page breaks) abre outra folha; tabulacoes viram espacos; linhas mais largas que a folha continuam na linha seguinte.
- `mamute::BuildPdfLayout` / `WritePdfLayout` (`PdfLayout`: `lines_per_page`, `top_blank`, `left_cols`); `BuildPdf`/`WritePdf` do Mamute continuam iguais. `Editor::PrintLines(bloco)` monta o texto.
- `Eject page` continua desabilitado: nao faz sentido numa folha de PDF.
- Testes: `mamutetest` (layout), `editorui` (Print e Print Block, bloco so' com o bloco), `edittest`.

## [1.43.9] - 2026-10-22 - "Yie Ar Kung-Fu: Simbolos do projeto" (Konami)

### Adicionado -- fecha a fase E6 do editor (E6 parte 4)
- **Simbolos do projeto no completar** (`Alt-/`, `Ctrl-Espaco`): com um projeto aberto, os candidatos incluem os simbolos dos OUTROS arquivos dele. Assembly: so' as definicoes (`rotulo:`, `nome equ`, `nome =`, `macro`); `.sym`: o 1o nome de cada linha; BASIC, Dignified, Pascal, C, MML e DML: os identificadores. Vem depois das palavras do texto aberto. `CollectSymbols`/`SetProjectWords` em `completion.*`; `EditorApp::ProjectWordsRefresh` refaz a lista ao abrir/criar o projeto, sincronizar, salvar um fonte da pasta, ou agir pela arvore/historico (no maximo 8000 palavras, fontes de ate' 512 KiB).
- **`man editor`**: a pagina do manual do editor (menus novos, teclas do Norton, completar, projeto e Build, arquivos `fwedit.*`).
- Testes: `edittest` (extrator por tipo de arquivo; completar com e sem simbolos) e `editorui` (salvar um `.asm` leva o rotulo dele ao completar de outro arquivo).
- **E6 encerrada** com a decisao de adiar os menus Sprites/Telas/Som: os editores correspondentes ainda nao existem (SPEC 5.4), e menus de itens desabilitados so' ocupariam as 25 linhas.

## [1.43.8] - 2026-10-22 - "Yie Ar Kung-Fu: Menus reorganizados" (Konami)

### Alterado -- menus do editor mais concentrados (E6 parte 3)
- Dez menus do Norton Editor viraram **oito**, como nas IDEs e emuladores atuais: `File Edit Search View Project Run Tools` + `F1=Help`. Block e Text-layout -> Edit/Tools, Print -> File, Options -> Tools, Build -> **Run**; 2nd window -> View; DOS shell -> Tools.
- **Undo e Redo** passaram a aparecer no menu Edit.
- As teclas diretas do NE (Alt-K, Ctrl-F, Alt-C...) nao mudaram; a letra de cada item e' a mesma no menu novo (`Alt-E S` = marcar bloco, era `Alt-B S`). De-para em `doc/editor-spec.md` 1b.1b.
- A ajuda F1 tem um topico por menu; menus e titulos de ajuda traduzidos (Editar, Executar, Ferramentas).
- Testes `edittest` e `editorui` atualizados ao novo mapa (inclui a conferencia de que todo mnemonico do NE ainda existe).

## [1.43.7] - 2026-10-21 - "Yie Ar Kung-Fu: Build lendo do projeto" (Konami)

### Alterado -- Build e Basic Dignified leem o banco do projeto (E6 parte 2, fatia 7)
- Com um projeto aberto, o `include` do Basic Dignified e o `include`/`incbin` do montador saem do `.msxproject` ANTES da pasta; o que o projeto nao tem continua saindo do disco. A copia da pasta pode estar atrasada (outro PC, Sync ainda nao feito) e o Build nao se engana mais com ela.
- Badig: `SetSourceReader`/`SourceReadable` (gancho de leitura de fontes; vazio = so' disco, entao `badigcli` nao muda). Editor: `EditorApp::ProjectSourceReader` e o `DiskFiles` do montador ganharam o leitor do projeto.
- O texto do arquivo aberto continua sendo o do BUFFER (como antes); so' os arquivos incluidos mudam de origem.
- Teste `editorui`: lib so' no banco e copia quebrada na pasta -> F9 monta (`.asm`) e converte (`.dmx`); include so' na pasta cai no disco.

## [1.43.6] - 2026-10-21 - "Yie Ar Kung-Fu: Historico de ROMs grandes" (Konami)

### Alterado
- Teto do historico de versoes do projeto: 2 MiB -> 8 MiB por versao (ja existem ROMs de MSX desse tamanho; assim nao perdem versao). Acima disso (um `.dsk`) continua fora. Teste: ROM de 6 MiB entra, 9 MiB nao.

## [1.43.5] - 2026-10-21 - "Yie Ar Kung-Fu: Historico e remocoes" (Konami)

### Adicionado -- historico de versoes, remocoes que viajam entre PCs, nomes seguros (E6 parte 2, fatia 6)
- **Esquema 2 do `.msxproject`**: tabelas `revisions` e `tombstones` (projetos do esquema 1 abrem e migram sozinhos; um fwMSX antigo recusa o novo).
- **Historico por arquivo**: sobrescrever com conteudo diferente, ou tirar do projeto, guarda a versao anterior (as ultimas 10; `project set revisions.keep N`, 0 desliga; arquivo acima de 8 MiB, como um `.dsk`, nao entra). Mesmo conteudo (ou so' o fim de linha) nao cria versao. `Project::Revisions/GetRevision/Restore`; console `project rev <caminho>` e `project restore <caminho> <id>`; editor: *Actions... > History (previous versions)...* (Enter volta a ela; a versao de agora tambem vai para o historico).
- **Remocao que viaja**: tirar um arquivo deixa uma lapide com o hash. No outro PC o Sync mostra "removed from project (asks)": *Project* apaga a copia da pasta (ela fica no historico), *Folder* mantem e traz de volta; o padrao nunca apaga. Se a copia mudou depois, avisa que mudou. `project sync --prune` so' apaga a copia que e' IGUAL ao removido; `project purge <caminho>` apaga na mao. *Tirar do projeto* na propria pasta nao fica reclamando da copia que ficou (a base guarda `!hash`).
- **Arquivos tirados**: *Actions... > Removed files...* (e o clique direito na janela) lista as lapides; Enter abre o historico do arquivo para traz-lo de volta. Renomear deixa a lapide do nome antigo, que e' o que leva a troca de nome aos outros PCs.
- **Nomes que so' diferem em maiusculas** (`SRC/X.BAS` x `src/x.bas`) sao recusados com uma mensagem (colidiriam no Windows).
- **Config do manifesto**: o Sync adota as chaves de `[config]` de `msxproject.txt` que o banco ainda nao tem (nunca troca as que ele tem; manifesto de outro projeto e' ignorado).
- Testes: `project` (114 verificacoes: historico, limite, restaurar, colisao, dois PCs, migracao do esquema 1, config do manifesto), `editorui` (History/Removed files pela arvore).

## [1.43.4] - 2026-10-21 - "Yie Ar Kung-Fu: Menu de contexto" (Konami)

### Adicionado -- menu de contexto da arvore do projeto (E6 parte 2, fatia 5)
- **Janela grafica**: clique direito num arquivo ou pasta da arvore: *Abrir*, *Atualizar o projeto com a copia da pasta*, *Restaurar a copia da pasta a partir do projeto*, *Renomear...*, *Adicionar arquivo da pasta...*, *Tirar do projeto*.
- **TUI**: o *Files...* ganhou o botao **Actions...** com as mesmas acoes numa lista (a pasta so' oferece *Adicionar* e *Tirar a pasta do projeto*); depois da acao a arvore volta no mesmo item.
- *Renomear* leva junto a copia solta (se existir) e as abas abertas; o projeto fica ja' em sincronia com o nome novo. *Tirar do projeto* nunca apaga a copia da pasta. *Adicionar* aceita qualquer arquivo da pasta do projeto, nao so' os que o editor salvou.
- Teste `editorui`: Actions..., renomear (projeto + pasta), tirar, adicionar, renomear de volta.

## [1.43.3] - 2026-10-21 - "Yie Ar Kung-Fu: Arvore no TUI" (Konami)

### Alterado -- Project > Files... virou arvore (E6 parte 2, fatia 4)
- No TUI (e no dialogo da janela) o *Files...* mostra as pastas como linhas `[-] pasta/` com os arquivos recuados embaixo (`nome   (tipo)`); **Enter numa pasta recolhe** (`[+]`) ou expande e a lista volta no mesmo item; **Enter num arquivo abre**. As pastas recolhidas sao lembradas entre aberturas.
- Teste `editorui`: arvore, recolher, expandir, abrir.

## [1.43.2] - 2026-10-21 - "Yie Ar Kung-Fu: Projeto lembrado" (Konami)

### Adicionado -- o projeto fica a' vista e volta sozinho (E6 parte 2, fatia 3)
- **Ultimo projeto lembrado entre sessoes**: abrir/criar um projeto grava o caminho em `fwedit.project` (ao lado do `fwedit.cfg`, sem precisar de "Save configuration"); a proxima sessao o reabre sozinha ("Project X reopened"). *Close project* de proposito apaga o arquivo (nao volta); projeto que sumiu (outro PC, pasta apagada) e' ignorado sem reclamar. A diferenca com o disco nao abre sozinha na partida: *Project > Sync...*.
- **Nome do projeto na barra de status**: TUI `[nome]` entre `Col=` e o nome do arquivo (ate' 9 letras); janela grafica `Projeto: nome`.
- **Arvore de arquivos na janela grafica**: com um projeto aberto, painel a' esquerda com as pastas (abrem/fecham) e os arquivos do projeto; clicar abre (extrai antes, se so' existir no banco; binarios avisam). A lista e' relida a cada ~60 quadros.
- Teste `editorui`: barra de status com e sem projeto; memoria do ultimo projeto (grava, reabre, fechar esquece, projeto sumido ignorado).

## [1.43.1] - 2026-10-21 - "Yie Ar Kung-Fu: Projeto no editor" (Konami)

### Adicionado -- o `.msxproject` dentro do editor (E6 parte 2, fatia 2)
- **Menu Project** (TUI e janela; `Alt-J`): *New project...*, *Open project...*, *Close project*, *Files...*, *Add current file*, *Sync...*.
- **Salvar leva para o projeto**: com um projeto aberto, todo arquivo salvo DENTRO da pasta do projeto (Save, Save As, autosave, F5/F9) vai tambem para o `.msxproject`; arquivo novo salvo na pasta entra sozinho; fora da pasta, copia de seguranca (`.{`) e pastas com ponto (`.git`) nunca entram.
- **Sync... mostra o que difere e a direcao sugerida** ("folder changed -> project", "project changed -> folder", "new in folder -> project", "CONFLICT (newer: project)"...); OK aplica o que e' seguro. **Conflito pergunta**: dialogo com *Project / Folder / Skip / Cancel*, o lado mais NOVO e' o botao padrao; arquivo apagado da pasta tambem pergunta (*Project* restaura, *Folder* tira do projeto). Abas abertas sem alteracoes sao recarregadas quando o Sync as muda; com alteracoes o editor avisa.
- Ao criar/abrir um projeto a lista do Sync aparece sozinha se houver diferencas (o "apontar para o mais novo").
- *Files...*: lista os arquivos do projeto (tipo e caminho); Enter abre (extrai antes, se so' existir no projeto). Binarios (disco, fita, PDF) avisam.
- Os menus View/Options/Build subiram uma coluna para a esquerda para caber o Project antes do F1=Help (o NE 2 tinha View em 44 e Options em 50).
- Testes: `editorui` (fluxo completo do menu), `edittest` (barra com Build e Project).

## [1.43.0] - 2026-10-21 - "Yie Ar Kung-Fu" (Konami)

### Adicionado -- o projeto `.msxproject` (fase E6 parte 2, fatia 1)
- **Um arquivo SQLite com tudo**: fontes, sprites, telas, musicas, MML/DML, notas, PDF, discos, fitas e a configuracao do projeto; copiar o `.msxproject` leva o projeto inteiro a outro PC. Journal tradicional (sem `-wal`/`-shm`): o arquivo unico e' sempre completo.
- **Copias soltas sincronizadas**: o projeto mantem os arquivos tambem numa pasta (a do `.msxproject`, ou a que o usuario escolher em cada PC). A sincronizacao compara cada arquivo com o hash da ultima sincronizacao (`.msxsync`, local): so' um lado mudou = sugere/aplica a direcao certa; os dois mudaram = **conflito**, nada e' mexido e o comando diz qual lado e' o mais novo. Em texto, LF x CRLF nao conta como alteracao.
- **GitHub**: `project export` / `import` + manifesto `msxproject.txt` (nome, id, configuracao, tipos): versiona-se a pasta e o `.msxproject` fica no `.gitignore`; depois de um clone `project import <pasta> <novo.msxproject>` recria o banco.
- Comando `project new|open|close|info|ls|add|get|rm|set|root|status|sync|pull|push|export|import` (console, TUI, ponte de comandos) e `man project`.
- Seguranca: todo caminho passa pela validacao em Fortran (nada de `../`, `C:`, caminho absoluto) ao entrar e ao extrair.
- Nas 4 linguagens: C++ (banco/sincronizacao), C (tipo pela extensao), Fortran (caminho seguro), NASM (hash sem CR). Ver `doc/project-spec.md`.
- Testes: `projecttest` (75), `controltest` com o comando `project`.

## [1.42.2] - 2026-10-21 - "Road Fighter: Erros de execucao no fonte" (Konami)

### Adicionado -- do erro do BASIC de volta ao `.dmx`
- **Montar > BASIC line...** (editor): o programa rodou e parou com `Syntax error in 130`? Digite a mensagem (ou so' `130`) e o cursor vai para a linha do `.dmx` (ou do `.include`, abrindo a aba na janela grafica) que gerou a linha 130. Usa a tabela de linhas da ultima conversao (F9/F5), a mesma do relatorio `-lnr`. Linha gerada pelo proprio Badig (cabecalho REM...) avisa que nao tem linha de fonte.
- API: `badig::Result::line_map` (sempre preenchida, sem precisar de `-lnr`), `badig::MapBasicLine` e `badig::ParseBasicErrorLine` (o numero depois de " in "; fora de 1..65529 = nao e' linha).
- Testes: `badigtest` (mapa de linhas e extracao do numero), `editorui` (dialogo, cursor na linha certa, avisos).

## [1.42.1] - 2026-10-20 - "Road Fighter: NestorBASIC e arquivos de apoio" (Konami)

### Adicionado -- o Basic Dignified roda programas reais
- **NestorBASIC**: a biblioteca `nbasic.dmx` (as rotinas do NestorBASIC como funcoes `.NB_*`, do msxIDE) e o `NBASIC.BIN` vao na pasta **`badig/`** ao lado do executavel (copiada para `dist/badig` e para os pacotes). `include "nbasic.dmx"` acha a biblioteca quando nao ha' um arquivo com esse nome na pasta do fonte; as funcoes e os defines da biblioteca entram no **mesmo espaco de nomes** do programa (um include de arquivo do usuario continua com namespace proprio, como no original), e da biblioteca so' ficam as **funcoes chamadas** (direta ou indiretamente), sem os comentarios: um programa que usa `.NB_ReadByte` ganha ~220 bytes, nao os 26 KB da biblioteca inteira.
- **Arquivos de apoio no disco de execucao** (`badigrun`, F5): remtag `##BB:disk_files=dados.txt *.bin` (nomes ou mascaras da pasta do fonte), opcao `disk_files` do `badig.cfg` e `--disk_files`; os nomes viram 8.3 em maiusculas e o programa os le' em A: (testado: um programa BASIC le' um arquivo de apoio no emulador). Nome que nao e' 8.3, arquivo inexistente ou disco cheio = erro explicando.
- **`NBASIC.BIN` no disco**: sozinho quando o fonte inclui o `nbasic.dmx`, ou com `##BB:nbasic=true` / `--nbasic` / a caixa *NestorBASIC* da configuracao.
- **Snippet `nbload`** (Tab no editor): o carregador do NestorBASIC (`BLOAD "nbasic.bin",R` com os codigos de erro) e o `include "nbasic.dmx"` depois do `END`, porque as funcoes sao alvos de `GOSUB`.
- Tela *Montar > Dignified setup...*, pagina 2: arquivos de apoio e NestorBASIC. `badig.cfg`: secao `[FWMSX]`.
- Testes: `badigtest` 389; `editorui` 310; `controltest` com o programa que le' o arquivo de apoio.

## [1.42.0] - 2026-10-20 - "Road Fighter" (Konami)

### Adicionado -- o Basic Dignified Suite, clone nosso (`.dmx` -> `.amx` -> `.bmx` tokenizado)
- **`fwmsx --badig`** e o comando **`badig`** do console: le um programa em BASIC Dignified (`.dmx`: sem numeros de linha, com labels e loop labels, defines com argumentos, variaveis de nome longo, funcoes, includes, toggles, `true`/`false`, `++`/`+=`, Unicode -> MSX...) e gera o BASIC ASCII (`.amx`), o **BASIC tokenizado do MSX (`.bmx`)** e, se pedido, a listagem do tokenizador (`.lmx`) e os relatorios (`-lbr -lnr -var -lex -par`). Todas as opcoes e as remtags `##BB:` do `badig.py` v2.2.1.
- **Igual ao original, byte a byte**: o clone foi escrito a partir do `badig.py` e do `msxbatoken.py` (resource/basic-dignified) e conferido contra eles por `tools/badig_compare/compare.py`: 14 programas de recursos, o exemplo real **Change Graph Kit** (30 KB), 33 casos de erro (mensagens identicas, com a linha do codigo e o `^`), os numeros do tokenizador (reais simples/duplos, expoentes, `&H/&O/&B`) e as combinacoes de opcoes. Os arquivos de referencia moram em `tests/badig/golden` e o `ctest badig` os confere sem precisar do Python.
- **`badigrun <fonte.dmx>`** (console, ponte de controle e editor): converte, monta um disquete com o `.bmx` e um `AUTOEXEC.BAS`, insere em A: e reinicia; o DISK BASIC roda o programa sozinho (precisa de interface de disquete). Testado de ponta a ponta no `controltest`.
- **Editor**: `F9` converte o texto da aba `.dmx` (sem precisar salvar) e leva o cursor ao erro; `F8`/`Shift-F8`/*Messages...* andam pelos erros e avisos (o erro de um `.include` abre a aba dele); `F5` converte e executa no emulador. O menu Build escolhe pela linguagem da aba: `.dmx` -> Basic Dignified, `.asm` -> fwasm.
- **Tela de configuracao** (a do msxIDE, so' com o que vale para o MSX e o fwMSX): *Montar > Dignified setup...*, duas paginas -- linha inicial/passo/TAB, cabecalho REM, tirar espacos, maiusculas, Unicode, PRINT x `?`, THEN/GOTO, tokenizar, `.lmx`, apagar o `.amx`, relatorios, niveis de mensagem. Grava o `badig.cfg` ao lado do executavel (os nomes do `badig.ini`).
- `man badig`; o comando `help` lista `badig`/`badigrun`.
- Nucleo nas 4 linguagens: **C++** (lexer, parser em 5 passadas, tokenizador), **C** (a tabela de palavras do tokenizador), **Fortran** (arredondamento dos digitos BCD) e **NASM** (prefixo sem caixa, o laco quente do tokenizador).
- Testes: `badigtest` 376 verificacoes; `editorui` 307; `controltest` com o E2E do `badigrun`.

## [1.41.9] - 2026-10-20 - "Hyper Sports: Montar e executar" (Konami)

### Adicionado -- fase E6 do editor (primeira parte): menu Build e ajuda da palavra
- **Menu Build** (Alt-U; "Montar" em portugues) entre Options e F1=Help, tanto na TUI quanto na janela: **Assemble** (`F9`), **Run on MSX** (`F5`), **Next message** (`F8`), **Previous message** (`Shift-F8`) e **Messages...**.
- **Assemble** monta o TEXTO DO BUFFER pelo fwasm (nao o arquivo do disco): as mensagens refletem o que esta' na tela, mesmo sem salvar; `.include`/`.incbin` saem da pasta do fonte. Erros e avisos viram uma lista de mensagens com arquivo e linha; o cursor vai para a linha do **primeiro erro** (pula os avisos) e a barra de estado mostra `Assemble: 1 error(s), 2 warning(s)  [3/3] texto`. Sem erro: `Assemble OK: rom, 16384 bytes`. So' vale para fontes Z80 (outra linguagem avisa).
- **F8 / Shift-F8** andam pela lista (circular) e levam o cursor a' linha; **Messages...** abre a lista inteira (`E`/`W` + arquivo:linha + texto) e Enter vai para a linha escolhida. Mensagem de um `.include`: a janela abre a aba do arquivo; na TUI (so' 2 janelas, ambas do usuario) mostra `arquivo:linha` na barra.
- **Run on MSX** monta, grava o arquivo se estiver alterado e manda o mesmo `asmrun` do console carregar o resultado na maquina (ROM/MegaROM como cartucho, `.basic` por fita, `.msxdos` num disco), com simbolos e arquivo:linha no depurador; a janela do editor sai da frente para o programa rodar. Com erro de montagem nao executa. So' existe no editor embutido no emulador (sem maquina o item fica cinza).
- **Help > On word** (`Ctrl-F1`): abre a pagina do `man` da palavra sob o cursor (`LEFT$`, `CHPUT`, `LDIR`... as 554 rotinas da BIOS e os comandos do BASIC), numa caixa de leitura rolavel; palavra sem pagina avisa.
- Nucleo nas 4 linguagens: **C** `ed_build_msg` (le' "arquivo, line N: texto", erro ou aviso), **Fortran** `ed_cycle` (indice circular), **NASM** `ed_hash_ci` (tira as mensagens repetidas das varias passadas do montador), **C++** `editor_build.cpp`, `editor_host.cpp`.
- Testes: `editorui` ganhou `test_build` (parser, ciclo, montar bom e com erro, F8/Shift-F8, lista, executar com gancho, erro do gancho, ajuda da palavra).

## [1.41.8] - 2026-10-20 - "Hyper Sports: Completar e snippets" (Konami)

### Adicionado -- fase E5 do editor: completar palavras e snippets
- **Completar**: `Alt-/` (e `Ctrl-Space` na janela; menu Busca > Completar palavra) abre uma lista com as palavras que completam o que esta' antes do cursor: palavras da **linguagem** (as mesmas tabelas do destaque: comandos e funcoes do MSX-BASIC, extensoes `CALL`, comandos do MSXBAS2ROM, funcoes `NB_` do NestorBASIC, mnemonicos e diretivas do Z80, a **BIOS** em maiusculas, Pascal, Batch), **gatilhos de snippet** e palavras do **proprio texto**. Cima/Baixo escolhem, Enter/Tab aceitam, Esc fecha; digitar continua filtrando e apagar alarga a lista; um candidato so' completa direto. Na janela a lista e' desenhada junto da palavra e o clique aceita; na TUI e' uma caixa na grade.
- **Completar sozinho** (Opcoes > Editor... > Auto completion, desligado por padrao): a lista abre sozinha depois de 2 letras.
- **Snippets**: `Tab` depois de um gatilho (`for`, `rom`, `proc`, `ldir`, `prog`...) expande o snippet; o cursor vai ao primeiro ponto de parada (`$1`) e `Tab` pula para o seguinte ate' o `$0`; a identacao da linha vale para todas as linhas; desfazer volta tudo de uma vez. Snippets de fabrica para MSX-BASIC (e dialetos), Kun BASIC (`turbo`), Z80, Pascal, Batch e Markdown. O arquivo `fwedit.snippets` (ao lado do `fwedit.cfg`) acrescenta ou troca snippets: `[linguagem]` + `gatilho=texto` (`\n`, `\t`; secao `[all]` vale para todas).
- **Numeros de linha** (pedido do usuario: "nesta versao precisamos de linhas numeradas como os editores de programas padrao"): a janela ja' tinha; agora e' a opcao **Opcoes > Tela... > Line numbers** (ligada por padrao, gravada no `fwedit.cfg`) e a **TUI** tambem ganhou a margem (digitos alinhados a direita na cor dos comentarios; o clique do mouse e a rolagem horizontal descontam a margem).
- Nucleo nas 4 linguagens: **C** `ed_word_start` (prefixo), **Fortran** `ed_match_score` (nota do candidato: mesma caixa, outra caixa, nao serve), **NASM** `ed_hash_ci` (palavras do texto sem repetir), **C++** `completion.cpp`/`editor_complete.cpp`.
- Testes: `edittest` 229, `editorui` 260, `syntaxtest` 117 verificacoes (margem, mouse, lista, filtro, snippets de varias linhas, snippets do usuario, configuracao).

## [1.41.7] - 2026-10-20 - "Hyper Sports: Kun BASIC" (Konami)

### Corrigido/Adicionado -- a sintaxe real do MSX-BASIC KUN no destaque (esclarecimento do usuario)
O usuario esclareceu que o "Turbo Basic" nao e' o da Borland: e' o **MSX-BASIC KUN** (compilador de BASIC para Z80, J. Suzuki 1989), e indicou o manual (MSX2 Technical Handbook, KunBASIC). A lista inventada da 1.41.6 (`while wend repeat until...`) saiu; a linguagem agora se chama **MSX-BASIC KUN** (`kun`, `kbas`; e `.bas` com `_TURBO ON` ou `CALL TURBO`).
- **Blocos**: `_TURBO ON` / `CALL TURBO ON(A,C())` abre o bloco compilado e `_TURBO OFF` fecha (estado de varias linhas); `_RUN` / `CALL RUN` compila tudo.
- **Remtags**: `'#I` (Z80 em linha, com `@150` = rotulo de linha), `'#C+`/`'#C-` (recorte nos SCREEN 5 a 8) e `'#N+`/`'#N-` (estouro do NEXT).
- **O que o compilador NAO aceita**: dentro de um bloco `_TURBO`, os comandos da lista do manual (OPEN, CLOSE, PLAY, TAB, SPC, WIDTH, ERROR, BLOAD/BSAVE, CLEAR, LIST, LOAD/SAVE, MKI$..., `PRINT#`, `INPUT#`, `PRINT USING`, `CALL` que nao seja TURBO...) ficam em **vermelho** (nova classe `kSynError`); fora do bloco sao MSX-BASIC normal.
- No BASIC, `$ # %` deixaram de iniciar numero (`#1` e' numero de arquivo; hexadecimal e' `&H`).
- Testes: `syntaxtest` 117 verificacoes (blocos, remtags, erros dentro do bloco e a deteccao por conteudo).

## [1.41.6] - 2026-10-20 - "Hyper Sports: Destaque de sintaxe" (Konami)

### Adicionado -- destaque de sintaxe no editor (fase E4), na TUI e na janela
- **10 linguagens**, escolhidas pela extensao (e, para `.bas`, pelo inicio do texto): **MSX-BASIC** (`.bas`, `.asc`, `.amx`), **Basic Dignified** (`.dmx`), **MSXBAS2ROM**, **NestorBASIC**, **Turbo Basic / Basic Kun**, **Assembly Z80** (`.asm`, `.z80`, `.inc`, `.s`), **Pascal** (`.pas`),
  **Batch** (`.bat`, `.btm`, `.cmd`), **Markdown** (`.md`) e texto puro. `View > Language...` troca a linguagem na mao ("(auto)" volta a detectar); a escolha vale ate' o usuario pedir auto e fica na sessao.
- **MSX-BASIC**: numero de linha, comandos (182 palavras tiradas do man), funcoes, operadores por extenso, `REM` e `'`, strings, numeros (`&H`, `&B`, `&O`, reais com expoente), `CALL <extensao>` e `_EXTENSAO` (96 extensoes de CALL), `?` = PRINT, numeros de linha de `GOTO/GOSUB/THEN/ELSE/ON..GOTO` como rotulos e a listagem compacta em
  maiusculas (`FORI=1TO10`). **Basic Dignified**: `{rotulo}`, `nome{` / `}`, `define [alias]`, `declare`, `func`/`ret`, `.funcao(...)`, `~variavel`, `##` e `###`/`''` (blocos de varias linhas), remtags `##BB:`. **MSXBAS2ROM**: `FILE`/`TEXT`/`INCLUDE` e `CMD <comando>` (37 comandos). **NestorBASIC**: as 85 funcoes `NB_`.
- **Assembly Z80** (asMSX/fwasm): mnemonicos (incl. R800), registradores e condicoes, diretivas (com e sem ponto), **chamadas e variaveis da BIOS** (a mesma tabela do montador), rotulos (coluna 0 e locais `.nome`), `;`, `//` e `/* */` de varias linhas, strings/caracteres, numeros `#FF $FF 0xFF 0FFh %1010 101b`.
  **Pascal**: palavras-chave, tipos, `{ }`, `(* *)` e `//` (varias linhas), `{$...}`, strings com aspa dobrada, `$FF`, `#13`, e as palavras do Kizuna (`bank`, `public`, `extern`). **Batch**: `rem`, `::`, `:rotulo`, `@`, `%VAR%`/`%1`/`%%i`, palavras do MS-DOS/MSX-DOS 2/4DOS. **Markdown**: titulos, listas, codigo em linha e em bloco, negrito, links, citacoes.
- **Cores**: na TUI cada esquema do NE (LCD, azul, ciano...) ganhou cores de destaque; na janela, as cores seguem o tema (escuro: estilo VS Code Dark+; claro: tons fechados). **Display setup > Syntax highlighting** liga/desliga; a janela mostra a linguagem na barra de estado (clique para mudar).
- **Quatro linguagens**: C++ (as linguagens), C (`ed_syntax.c`: medir identificador, numero e string), Fortran (`ed_number.f90`: classificar o numero: decimal/hexa/binario/octal/real) e NASM (`ed_hash.asm`: hash FNV-1a que ignora a caixa, chave das tabelas de palavras).
- Janela: o desenho agora e' por trechos coloridos posicionados por celula (e a largura da celula e' medida em 64 letras: `CalcTextSize` arredonda cada letra para cima e os trechos se afastavam).
- Gerador das listas: `tools/editor/gen_syntax_words.py` (le' o man e escreve `syntax_words_gen.inc`).
- Testes: `syntaxtest` (102 verificacoes: nucleo, cada linguagem, estado de varias linhas, deteccao) e `editorui` (246: integracao com o editor, a grade da TUI, o dialogo Language, a sessao).

## [1.41.5] - 2026-10-20 - "Hyper Sports: Autosave do editor" (Konami)

### Adicionado -- salvamento automatico periodico e nomes noname01.. (pedidos do usuario)
- **Autosave** (Options > Editor... / "Configuracao do editor"): o campo **"Auto-save every (minutes, 0 = off)"** -- a cada N minutos o editor grava em disco (0 = nunca, o padrao). O usuario escolheu "os dois" modos, entao ha' tambem a caixa **"Also overwrite files that have a name"**:
  - **so' a sessao** (caixa desmarcada): grava TODAS as abas -- inclusive `noname01..` e o texto nao salvo -- na sessao de recuperacao `fwedit.session`; os arquivos de verdade nunca sao sobrescritos sem voce mandar;
  - **sessao e arquivos** (caixa marcada): alem da sessao, grava os arquivos que ja' tem nome (o aviso "Salvo automaticamente" aparece na barra de estado). A copia `{` do arquivo ORIGINAL e' feita so' no primeiro autosave (os seguintes nao a trocam pela versao anterior); um Save manual volta a refazer a copia.
  Os valores ficam no `fwedit.cfg` (`autosave_minutes`, `autosave_files`). O relogio roda a cada quadro mesmo com o editor escondido; na TUI (`--edit` no terminal) por um evento a cada 10 s.
- **Sem sessao em `--edit` sem argumentos**: o editor standalone e o de terminal, abertos sem arquivos, restauram a ultima sessao e o autosave grava nela; ao sair normalmente (depois das perguntas) a sessao e' apagada. Com arquivos na linha de comando o autosave so' grava neles (se a opcao permitir).
- **Abas sem nome** agora se chamam **noname01, noname02, noname03...** (o menor numero livre; fechar uma aba, ou salvar com nome, libera o numero). A sessao guarda esses nomes (e abre as sessoes antigas, sem eles). Na TUI o titulo continua "Untitled" como no NE.
- Testes: `editorui` com 221 verificacoes (nomes, relogio, copia `{`, sessao, configuracao).

## [1.41.4] - 2026-10-20 - "Hyper Sports: Reabrir o editor" (Konami)

### Corrigido -- o editor do emulador travava ao ser reaberto (relato do usuario)
- **Sintoma**: digitar no editor, fechar (o editor perguntava se salvava, "nao"), voltar ao emulador e abrir o editor de novo: a janela abria com o texto, mas nao aceitava teclas, os menus nao executavam e nao dava para fecha-la.
- **Causa**: o dialogo "Salvar?" era um popup MODAL do ImGui. Fechar o editor parava de desenhar a janela com o modal ainda aberto na pilha de popups do ImGui; ele ficava "fantasma" e bloqueava o foco e o mouse da janela quando o editor voltava.
- **Correcao**: os menus e os dialogos do editor agora sao janelas COMUNS do ImGui (sem popups nem modais: nao sobra estado preso se o editor for escondido com um aberto). O "modal" e' do proprio editor (enquanto ha' dialogo o teclado e' dele e a janela fica na frente).
  Tambem corrigido um bug que esse refatoramento teria deixado: o clique num item de menu fechava o menu antes de executar (o teste "o mouse esta' sobre o menu?" falhava porque o item ja' estava ativo no frame do clique).
- **Fechar o editor so' ESCONDE** (o X da janela e "Arquivo > Sair"): sem pergunta, os arquivos e o texto nao salvo continuam abertos para a proxima vez que o editor for aberto (para anotar enquanto se testa no MSX-BASIC e so' salvar no fim). Um dialogo ou menu aberto e' cancelado ao esconder.
- **Sessao**: `fwedit.session` (ao lado do executavel) guarda as abas -- inclusive o texto nao salvo e o cursor -- ao esconder o editor e ao fechar o emulador, e as restaura na proxima execucao; arquivo sem alteracoes volta relido do disco. Sem nada util a guardar o arquivo e' apagado.
  No `fwMSX.exe --edit` (standalone) continua perguntando ao fechar.
- Conferido com cliques e teclas simulados numa janela real: abrir, digitar, fechar pelo X, reabrir (o texto volta e aceita mais texto), Arquivo > Novo cria a aba, Arquivo > Sair esconde. `editorui`: 189 verificacoes (inclui o ciclo esconder/reabrir e a sessao).

## [1.41.3] - 2026-10-20 - "Hyper Sports: Editor GUI" (Konami)

### Corrigido o rumo da fase E3: o editor GRAFICO de verdade (substitui a "grade de caracteres em janela" da 1.41.2)
O usuario esclareceu: o editor em janela deve ser um programa GUI de verdade, integrado ao menu do fwMSX, com a aparencia do fwMSX -- e so' os atalhos e as opcoes de menu do Norton Editor, NAO a mesma tela de caracteres. Decisoes (AskUserQuestion): janela flutuante dentro do fwMSX; barra de menus propria do editor;
abas com divisao opcional em 2 paineis; tema do fwMSX, fonte monoespacada, textos em portugues.
- **Menu "Editor" na barra do fwMSX** (Mostrar o editor, Novo arquivo, Abrir arquivo...): abre uma **janela flutuante do ImGui** ao lado da tela do MSX, que continua rodando. Com o editor em foco o teclado e' dele (as teclas nao vao para o MSX; so' o F11 de tela cheia passa; o F10 abre o menu do editor).
- **`fwMSX.exe --edit [arquivos]` / `--view arquivo`**: o MESMO componente numa janela do sistema (standalone), no tema do fwMSX. `--tui` continua sendo o editor de terminal; `--gui` forca a janela.
- **Interface grafica de verdade**: barra de menus (Arquivo, Bloco, Texto, Busca, Imprimir, Exibir, Opcoes, Ajuda) com popups e os atalhos ao lado; **abas** por arquivo (indicador de alterado, fechar, "+" para novo); **divisao em 2 paineis** (F2) com Window Differences; texto com numeros de linha, rolagem do ImGui (barras vertical e horizontal), selecao e cursor
  desenhados no tema, **mouse** (clicar, arrastar, duplo clique = palavra, triplo = linha, roda, menu de contexto Recortar/Copiar/Colar/Desfazer/Refazer/Selecionar tudo); **dialogos com widgets graficos** (campos, radios, caixas de marcar, listas, botoes) sobre os MESMOS dados dos dialogos do NE; barra de estado (Lin, Col, Insert, CRLF, alterado, mensagens).
- **Os mesmos atalhos do Norton Editor 2** (todas as teclas chegam com seus modificadores: Ctrl-Enter, Ctrl-PgUp, Ctrl-2...), o Alt sozinho abre o menu, e as alternativas graficas Ctrl-S/N/O/V/X (salvar, novo, abrir, colar, recortar).
- **Textos em portugues** (`ed::Tr`, sem acento) so' na GUI; a TUI mantem os textos do NE.
- `EditorApp` no modo grafico: abas (`OpenTab/NewTab/CloseTab/SetActive`), divisao (`ToggleSplit`), API de menus e dialogos para a interface (`OpenMenuAt/RunMenu/PressDialog/DialogEnter`). `Editor`: `SetTopLine/SetLeftCol/PosAt/SelectWord/SelectLine/Revision`.
- Testes: `editorui` ficou com 172 verificacoes (modo grafico, abas, divisao, traducao, extras do modelo).

## [1.41.2] - 2026-10-20 - "Hyper Sports: Editor em janela" (Konami)

### Adicionado -- o editor de textos em JANELA (fase E3)
- **`fwMSX.exe --edit [arquivo [arquivo2]]`** e **`--view arquivo`** abrem o editor numa janela (Dear ImGui + GLFW + OpenGL3), com a MESMA grade, os mesmos menus, dialogos, janelas, mouse e atalhos da TUI (e' o mesmo `ed::EditorApp`; a janela so' pinta a grade e traduz teclado/mouse). `--gui` / `--tui` escolhem a forma:
  o `fwMSX.exe` abre a janela por padrao, o `fwMSXc.exe` e o Linux abrem o terminal (`--edit --gui` no Linux abre a janela).
- **Teclado de verdade**: cada tecla chega com seus modificadores, entao **tudo do Norton Editor 2 funciona sem alternativa**: Ctrl-Enter, Ctrl-PgUp/PgDn, Ctrl-Home/End, Ctrl-Setas, Ctrl-2/Alt-2 (divisor das janelas), Ctrl-], Ctrl-Backspace, e **Alt sozinho abre a barra de menus** (a opcao "Alt alone selects menu" do NE).
  AltGr (teclados internacionais) digita o caractere em vez de virar Ctrl+Alt.
- **Desenho**: fonte de largura fixa do sistema (Consolas/Courier New, DejaVu/Liberation Mono; ou a embutida do ImGui) e os ornamentos (bordas simples e duplas, sombra pontilhada, setas, barra de rolagem, marcador de bloco) **desenhados a mao**, para nao depender de a fonte ter os glifos.
  Os 5 esquemas de cor do NE em RGB (LCD, Color 1 azul, Color 2 ciano, VGA mono, mono), cursor de barra/bloco piscando, janela redimensionavel (a grade acompanha), titulo `fwEdit - arquivo *`.
- **Area de transferencia do sistema**: copiar/recortar (Ctrl-Ins, Shift-Del) grava nela; colar (Shift-Ins) le' dela. Fechar a janela faz o mesmo que Alt-F X (pergunta por cada arquivo alterado).
- `editor_gui_input` (sem OpenGL): nomes de tecla do teclado de verdade e a paleta RGB; testados em `editorui` (+17 verificacoes).

## [1.41.1] - 2026-10-20 - "Hyper Sports: Editor na TUI" (Konami)

### Adicionado -- o editor de textos no terminal (fase E2)
- **`fwMSXc.exe --edit [arquivo [arquivo2]]`** e **`--view arquivo`** (somente leitura): o editor estilo Norton Editor 2 em tela cheia no terminal (FTXUI). Arquivo que nao existe abre como novo, com o nome dado. `--dump [--keys "A-f o ..."] [--size CxL]` imprime a tela sem terminal (teste de fumaca).
- **A tela do NE 2**: barra de menu (`File Block Text-layout Search Print View Options  F1=Help`), menus em caixa com sombra, letra de atalho em destaque, itens desabilitados em cinza (no `view`, os destrutivos), barra de estado `Line= Col= nome Insert WW= Auto`, barra de rolagem.
- **Menus abertos**: Alt+letra ou F10 abrem; setas, Enter, a letra do item, Esc; o menu **F1=Help** (General, Keys, File, Block, Text-layout, Search, Print, View, Options, About) com a ajuda paginada (Next/Previous/Cancel, rodape "More"); o texto da ajuda e' nosso e a lista de teclas e' gerada das tabelas.
- **Caixas de dialogo do NE 2** (campos, radios, caixas de marcar, botoes pela inicial, Tab/Shift-Tab, Enter = botao padrao, Esc = Cancel): Open/Save as/Merge/2nd window (lista de arquivos com mascara `*.ME`, lista de diretorios e unidades), Search for text, Search & Replace (pergunta a cada ocorrencia: Yes/No/All/Cancel),
  Find line, Indent/Unindent, Output to file, Text-layout setup, Print setup, Information display, About, Display/Mouse/Editor setup, Save configuration (arquivo atual ou do diretorio), perguntas de saida ("This file has been changed. Save it before exiting?") e "starting at..." (Top/Cursor/Cancel).
- **Duas janelas** (F2): a barra do meio com o nome da janela de cima, divisor movel (Ctrl-2/Alt-2, alternativas Alt-Seta), Close window, **Window Differences** (Alt-S D), e a saida pergunta por cada arquivo alterado.
- **Mouse**: clicar posiciona o cursor, arrastar marca um bloco, a roda rola, setas e trilho da barra de rolagem, clique nos menus, itens, botoes, campos e listas dos dialogos.
- **Cores** do NE (Options > Display): LCD, Color 1 (azul), Color 2 (ciano), VGA mono, mono; Display lines; scrollbar; parentese que casa realcado; cursor grande.
- **Configuracao** (`fwedit.cfg`, ao lado do executavel e/ou no diretorio atual): tabulacao, Tab key, backup, Insert/Over inicial, margens, cores, mouse, impressao; o editor pergunta ao sair se a configuracao mudou.
- **Teclas do terminal** (`editor_keys`): bytes de controle e sequencias de escape viram nomes de tecla (Ctrl-W, Alt-X, Ctrl-Esquerda, Shift-Direita, Del, F1-F12, Ctrl-Backspace = 0x08, Ctrl-2 = 0x00...), testadas com texto puro.
- Testes: `editorui` (117 verificacoes: teclas do terminal, menus, dialogos, configuracao, janelas, mouse, view) alem do `edittest` da 1.41.0.

## [1.41.0] - 2026-10-20 - "Hyper Sports" (Konami)

### Adicionado -- o editor de textos (fase E1: o nucleo, estilo Norton Editor 2)
- **Modelo do editor sem interface** (`src/editor/`, `ed::Editor`): buffer de linhas, cursor, Insert/Over, desfazer/refazer geral, area de transferencia, selecao e **blocos** (marcadores como no NE: Alt-B S/R/F/C/M/D/L/E, Indent/Unindent, Output to file), busca (Find, Continue fwd/rev, Text in Block, ir para linha, parentese casado, Replace)
  e a grade de celulas 80x25 desenhada como a tela do NE 2 (barra de menu, texto, barra de rolagem, barra de estado `Line= Col= nome Insert WW=`). Ainda sem tela: a TUI (E2) e a janela (E3) so' vao pintar essa grade.
- **As teclas do Norton Editor 2 como padrao** (as 40 teclas das telas de ajuda: Ctrl/Alt-W/L/A, Alt-K, Ctrl-U, Ctrl-F/R/C, Alt-C, Ctrl-], Ctrl-P, Ctrl-Enter, Alt-Z/Ctrl-Z...), os **49 mnemonicos de menu** (Alt-F N, Alt-B C, Alt-T W, Alt-S L...) e teclas ALTERNATIVAS marcadas
  (as que o terminal entrega mal: Alt-Setas, Alt-PgUp, Alt-Enter, F10; e as modernas: Alt-Backspace desfaz, Ctrl-Ins/Shift-Ins/Shift-Del, Shift+setas). Tudo em tabelas de dados (`Bindings()`, `Menus()`).
- **Modo somente leitura** (`view`): so' o que nao destroi nada funciona (navegacao, busca, copiar, Save As, imprimir, informacoes); o destrutivo e' ignorado em silencio e os itens do menu ficam desabilitados (`ItemEnabled`). Barra de estado mostra `View`.
- **Arquivos do MSX**: CRLF/LF e Ctrl-Z (0x1A) no fim reconhecidos e preservados, BOM UTF-8, backup com `{` (`READ.ME` -> `READ.{E`), Merge, Output to file.
- Menu Text-layout que ja' funciona: Center line, Word wrap, Auto indent, Expand tabs, Put in Tabs, Insert/Remove page breaks (a caixa de dialogo vem na E2; o modelo expoe `TakeDialog()` e os metodos `Apply*`).
- **Quatro linguagens**: C++ (modelo), C (`ed_text.c`: palavras, busca na linha, inverter caixa), Fortran (`ed_stats.f90`: contagem de palavras), NASM dual-ABI (`ed_indent.asm`: indentacao da linha).
- Teste `edittest` (`ctest` editor): sequencias de teclas -> texto, cursor, dialogos pedidos e grade.

## [1.40.5] - 2026-10-19 - "Antarctic Adventure: Montador na TUI" (Konami)

### Adicionado -- o montador na TUI e a base do depurador avancado
- **`asmrun [-z] [-n] [--dos-disk <arq.dsk>] <fonte.asm>`** (ponte, console, TUI, terminal e janela): monta com o fwasm, grava as saidas ao lado do fonte e CARREGA na maquina: ROM/MegaROM como cartucho (mapper certo) e reinicia; `.basic` por uma fita montada na hora
  (`BLOAD"CAS:",R` digitado sozinho); `.msxdos` num disco de sistema (copia do `msxdos1.dsk`, ENTER da data e o nome do programa digitados sozinhos). `-n` so' monta.
- **Simbolos e linhas do fonte no depurador** (`dbg::Symbols`): `break loop`, `until loop+4`, `dis inicio`, `break jogo.asm:42`; `sym` (resumo, `load`, `clear`, `list`, consulta por nome ou endereco), `src` (as linhas do fonte em volta do PC). Os eventos de parada, `breaks`, `stepi`/`next`, `dis` e o painel Z80
  Debugger mostram o rotulo (`sym=loop+2 src=jogo.asm:42`).
- **Marcadores `.break` e `.debug` do fonte** (blueMSX) funcionando no Z80: `.break` para a maquina, `.break <end>` cria um breakpoint, `.debug "texto"` imprime como evento; `markers on|off` (o `asmrun` liga sozinho quando o fonte os usa).
- **Menu Montador na TUI** (Montar e rodar, Montar so', Montar arquivos, simbolos, breakpoint em rotulo/arquivo:linha, fonte em volta do PC, marcadores); a janela ganhou "Montar e rodar um fonte .asm..." no menu Depurador.
- **fwasm**: o BOM UTF-8 do inicio do fonte (e dos `.include`) e' ignorado -- o asMSX para sem gerar nada e o fwasm dava erro de sintaxe; UTF-8 em comentarios e strings passa cru (igual ao asMSX). `AsmResult` ganhou `symbols`, `format`, `mapper`, `uses_markers`; `AsmOptions::want_tape`.
- Manual: `man asmrun`, `man sym`, `man src`, `man markers`; `break` explica rotulos e arquivo:linha.
- Testes: `controltest` (+33, inclusive um programa MSX-DOS que RODA num disco de sistema real), `asmtest` (+10), `tuitest` (+9), `mantest`.

### Achado
- O `.debug "texto"` do asMSX emite `JR (tamanho+4)`, que salta 4 bytes ALEM do texto (o fwasm reproduz byte a byte). Vale conferir no blueMSX se esse deslocamento e' intencional. Detalhes em `doc/debug-spec.md`, secao 8.3.

## [1.40.4] - 2026-10-19 - "Antarctic Adventure: Mamute, Fita" (Konami)

### Adicionado -- fita cassete em todos os pontos (`.cas`, `.tsx`/`.tzx` e `.wav`)
- **Uma camada so'** (`tape_io`): o formato vem da extensao em qualquer lugar (motor, `--cas`, banco de fitas, Mamute). O `.wav` passa a ser lido (pelo ripper) e GRAVADO (PCM 16 bits mono, 44100 Hz, pulsos do MSX). `fwmsx --cas pack/rip/list` aceitam `.wav` dos dois lados.
- **`TapeDeck`**: a fita sem hardware (inserida, protecao, modo de gravacao, ponto, ler/gravar arquivos). O `TapeEngine` herda dele.
- **Comando `tape`** (console, TUI, terminal e ponte): alem de `<arq>`/`eject`/`rewind`, agora `new`, `list`, `seek`, `protect`, `write`, `mode`, `save` (converte entre formatos), `put` (memoria -> arquivo da fita) e `load` (fita -> memoria). Menu da TUI e da janela com os itens novos e `.wav`.
- **Depurador**: `break tapin`, `until tapoon`, `dis tapion`... (as 6 entradas da fita na BIOS tem nome).
- **Mamute**: `TAPE` (inserir, criar, listar, ponto, protecao, modo de gravacao, copiar, modo de carga), **`R`** (agora carrega de verdade o proximo arquivo binario; no original so' validava a sintaxe), `TLOAD`, `TSAVE`, `LOAD`/`SAVE` de `.cas/.tsx/.tzx/.wav`
  e a opcao `T` do `A` (`A OT>jogo.cas` monta e grava a fita). Online usa a fita da MAQUINA (o que o Mamute grava, o `CLOAD` do BASIC enxerga); offline tem um toca-fitas proprio. `man mamute-fita`.
- **fwasm**: diretiva `.TSX ["nome"]` (a fita em blocos #4B; extensao nossa).
- Banco de fitas: tambem varre `.wav`.
- Testes: `castooltest` (+32), `mamutetest` (+48), `controltest` (+17), `asme2etest` (o `.cas`/`.tsx`/`.wav` do fwasm carrega e RODA no emulador; o `.tsx` tambem pelos pulsos), `mantest`.

### Corrigido
- O `LOAD`/`R`/`TLOAD` ONLINE sem slot usava o slot da pagina 1 mesmo para enderecos em C000; agora usa o slot que a maquina tem na pagina do ENDERECO.

## [1.40.3] - 2026-10-19 - "Antarctic Adventure: Mamute, SUPER-X" (Konami)

### Adicionado -- Mamute, Fase 4: os comandos do SUPER-X (prefixo X)
- **Enderecamento estendido**: depois do endereco, `#3` (slot 3), `#3-1` (sub-slot 1 do slot 3: RAM expandida, ate' 1 MB), `#V` (VRAM, 5 digitos) ou `#S` (o PAGE) escolhem onde ler e gravar. As **variaveis** `@0`-`@3`, `@B`, `@E`, `@S`
  guardam um endereco com o alvo (`@0=8000#3-1`, `XD @0`). Novos tambem: **`CL`**, a calculadora de 16 bits (hexa por padrao, sufixos D B H O, `+ - * / % | & ^ !`, parenteses, `@0`).
- **Telas**: `XD` (128 bytes, os dois blocos editaveis; `"` digita ASCII, `@` repete o byte), `XA` (256 bytes em ASCII), `XI` (disassembly rolando), `XH` (4 caracteres/sprites 8x8 pixel a pixel, com miniatura) -- paineis na TUI, a tela
  uma vez no console; com dois ou tres campos viram listagens (`XD 4000,40FF` com checksum por linha, `XD 4000,` = 256 bytes, terceiro campo = arquivo) e `?XD`/`?XA`/`?XI` mandam a listagem para um PDF. **`XM`** abre uma sessao de montagem
  (`XM C100`, prompt `XM C100> `): instrucoes Z80 montadas pelo fwasm, dados (`.cd,4dh` `:1234` `;expr` `[expr` `"texto`), um endereco ou `I [n]`.
- **Blocos**: `XBT` (copia entre slots/sub-slots/VRAM), **`XRT` (copia relocando** `JP`/`CALL`/`LD` com endereco absoluto que apontam para dentro do bloco e recalculando `JR`/`DJNZ`), `XFL`, `XCM` (diferentes ou, com `,S`, iguais),
  `XFD` (acha uma instrucao Z80, nao bytes soltos), `XTS` (checksum de 16 bits em HEX/BIN/DEC/OCT), `XCS` (checksum por linha: normal ou +ADDR), `XSD` (listagem assembly, `DEFB`, `DATA` do BASIC com o laco FOR/READ/POKE ou dados do X-BASIC).
- **Execucao**: `XRG` (todos os registradores, os alternados, `XRG *`, `XRG +`, `XRG reg,valor` e os 5 breakpoints `BP BP1 BP2 BP3 BPF`), `XGO` (executa; para no breakpoint da vez, no RET que devolve alem da pilha, no HALT ou em 2.000.000
  instrucoes) e `XTR` (trace passo a passo; na TUI, ENTER = proxima instrucao). **Portas**: `XPP` (painel de portas de E/S), `XPI`, `XPO`; a CPU offline passou a ler e gravar essas portas nas instrucoes `IN`/`OUT`.
- **Disco**: `XDK` (disco corrente), `XFS` (diretorio), `XCI` (clusters livres), `XTP` (um arquivo do disco como texto, com busca na TUI), `XL%`/`XS%` (setores direto para/da memoria), `XSV`/`XLD` (BSAVE/BLOAD, com deslocamento),
  `XS#`/`XL#` (so' os bytes). **Notas**: `XIL`/`XIS`/`XIM`/`XIC`/`XIR`; o `SUPER-X-PT.notas` (471 notas da BIOS traduzidas, do paleobasic) vai ao lado do executavel e o `XIL` sem argumento o carrega. `XCO` guarda as cores no `mamute.cfg`.
- Online: os alvos valem na maquina (`#3` e' o sub-slot que a CPU enxerga; `#V` escreve na VRAM da maquina), `XRG` edita a CPU dela e `XTR` executa uma instrucao nela.
- Manual: `man mamute-superx` (a 10a pagina do Mamute). Os kernels novos: C (`mamute_reloc.c`, tamanho de instrucao e operandos de endereco), Fortran (`mamute_total.f90`, soma de 16 bits) e Assembly (`mamute_cmp.asm`, comparacao de blocos).
- Internos: `IMemory::ReadSub/WriteSub`, `IExec::StepOne/Halted`, `PortTable`, `Machine::PokeVram`, o nucleo do msxdisk em `CONTROL_LIB_SOURCES`, `color=` no `mamute.cfg`.
- Testes: `mamutetest` (de 193 para 407 checagens), `repltest` (a sessao do XM), `mantest` (10 paginas).

## [1.40.2] - 2026-10-19 - "Antarctic Adventure: Mamute, telas" (Konami)

### Adicionado -- Mamute, Fase 3: as telas e o PDF
- **Paineis na TUI** (`fwmsx --tui`, comando `mon`): `DM`, `M`, `S`, `ZAP` e `SCR` abrem um painel interativo, com as teclas de verdade (setas, TAB hexa/texto, PgUp/PgDn +-128 bytes, `+`/`-` deslocamento do texto,
  ENTER abre o campo de edicao, `M`/`S` gravam digitando dois digitos hexa, Ctrl+S grava o setor no ZAP). No console e em scripts sao **comandos de linha** (a grade uma vez; as mudancas por argumentos).
- **`S end[,teclas]`**: o teclado numerico configuravel do Mamute (`1234 qwer asdf zxcv` = `1234 5678 9ABC DEF0`); `skeys=` no `mamute.cfg` e `CFG SKEYS`.
- **`ZAP arq.dsk,setor[,desloc]`**: editor de setores de uma imagem de disco (512 bytes por setor, rotulo = deslocamento no setor, `*` no titulo ate' salvar, Ctrl+S grava so' o setor do cursor, ESC pede confirmacao se houver
  alteracao). `ZAPW arq,setor,byte,b1..` grava direto (versao de linha).
- **`SCR end,dx,dy[,modo[,zoom]]`**: a memoria como uma tela de 256x192 pixels em braille (azulejos de dx x dy caracteres, leitura horizontal ou vertical), moldura 2x2, endereco base e a **edicao 16x16 pixel a pixel**
  (espaco inverte, I inverte tudo, L apaga, ESC desfaz tudo).
- **PDF**: `P`, `LP`, `V arq`, `A P` e `A H` gravam um **PDF A4 em Courier** quando o nome termina em `.pdf` (padroes `mamute.pdf`, `mamute-dis.pdf`, `NOME.pdf`, `NOME_labels.pdf`); `>nome.lst` no `A` e qualquer outro nome
  nos demais continuam texto. Gerador proprio, sem biblioteca (`mamute_pdf.cpp`).
- `DM` sozinho continua 128 bytes depois do ultimo. Manual: `man mamute-screens` (a 9a pagina do Mamute).
- Internos: `mamute_panel.cpp` (o modelo dos paineis: recebe teclas por nome e devolve linhas, sem tela), protocolo `mon @panels|@key|@panel`, `Mode::Mamute` no modelo da TUI.
- Testes: `mamutetest` (de 128 para 193 checagens: PDF, DM/S/ZAP de linha, todos os paineis por teclas, ZAP com arquivo, SCR com os dois modos de azulejo e a edicao), `tuitest` (as teclas do painel) e `repltest`.

## [1.40.1] - 2026-10-19 - "Antarctic Adventure: Mamute, EDIT e A" (Konami)

### Adicionado -- Mamute, Fase 2: o editor de fonte e a montagem
- **`EDIT`**: editor de fonte Z80 de linhas NUMERADAS (estilo MegaAssembler/ZX-81): `NN [Label:] instrucao operandos [;comentario]`, numeros no texto em HEXA por padrao (sufixos `H`, `B` binario, `D` decimal).
  Comandos: `LIST NEW DELETE RENUM CHANGE SEARCH/FIND LSEARCH SAVE LOAD MERGE` (arquivo `.mza` em ASCII), `QUIT` (volta ao monitor, o programa fica) e `HELP`. Prompt `EDIT> `.
- **`A`** monta pelo **fwasm** (nao ha' um segundo montador): cada linha e' traduzida para o asMSX, na mesma ordem, e os erros voltam com o numero da linha do Mamute (com o cursor `>`).
  Mostra `PASSO-1`/`PASSO-2` e a listagem (linha, endereco ou valor do EQU, ate' 4 bytes, texto). Opcoes `O` (grava na memoria, tambem a da MAQUINA no modo online), `N`, `P` (listagem em arquivo texto),
  `I` (BSAVE `.bin`), `R` (referencia cruzada), `S`, `D`, `H` e `/desloc` (soma a todo ORG); `>nome` escolhe o nome-base dos arquivos. `MAP` mostra inicio e fim do ultimo codigo montado.
- **fwasm**: `AsmOptions::want_listing` devolve, por linha de fonte, o endereco e os bytes gerados (`AsmResult::listing`), e `fwasm::IsDirectiveName` diz se um nome e' de diretiva (rotulos do Mamute como `PRINT`/`START` sao renomeados por dentro).
- Manual: `man mamute-edit` (a 8a pagina do Mamute). Projeto: [mamute-spec.md](mamute-spec.md).
- Testes: `mamutetest` (de 71 para 128 checagens: hexa/B/D, erros por linha, RENUM/DELETE/CHANGE/SEARCH, SAVE/LOAD/MERGE, P/I/R/S/D/H, `/desloc`, e o online: `A O` escreve na RAM da maquina e o programa roda la'), `repltest` (QUIT/BA dentro do EDIT) e `mantest`.

## [1.40.0] - 2026-10-19 - "Antarctic Adventure" (Konami)

### Adicionado -- o Mamute, o monitor Z80 do fwMSX (Fase 1)
- **Mamute** (do Mamute Assembler do Paleobasic, reescrito em C + Fortran + Assembly + C++, sem janela separada): um prompt de linha onde
  cada comando mexe na memoria, disassembla, executa, salva e carrega. Comandos: `BA QUIT XQT ONLINE OFFLINE STATUS PUSH PULL PAGE DM M SH MS C D P V T F L LP X G LOAD SAVE CFG HELP`.
- **Dois modos**: **offline** (memoria privada de 16 blocos de 16 KB -- 4 slots x 4 paginas -- e CPU privada; criar e depurar sem a maquina) e **online**
  (a memoria e a CPU da maquina emulada: o que se escreve aparece na hora no programa em execucao -- uma rotina em C000h e, no BASIC, `DEFUSR=&HC000:A=USR(0)`).
  `ONLINE`/`OFFLINE` trocam; `PUSH`/`PULL` levam um bloco de um lado para o outro.
- **Como abrir**: `mon` (ou `mamute`) no console e na TUI entra no monitor (prompt `MON(off)> ` / `MON(on)> `; `BA` volta; `!comando` roda um comando do console);
  `mon <linha>` executa um comando so'; `fwmsx --mon` (alias `--mamute`) abre o console ja' dentro do monitor. Pela ponte de controle: `mon <linha>`.
- **Configuracao inicial**: `mamute.cfg` ao lado do executavel (modo, VRAM de 16/128/192 KB, formato de despejo, tipo e arquivo de cada uma das 16 celulas) e o comando `CFG`.
- **Manual**: `man mamute` (+ `mamute-modes`, `mamute-memory`, `mamute-dump`, `mamute-exec`, `mamute-files`, `mamute-config`); o `help` ganhou a categoria "Mamute".
- Quatro linguagens: C (`mamute_dump.c`, formato do despejo), Fortran (`mamute_sum.f90`, checksum), Assembly NASM dual-ABI (`mamute_scan.asm`, busca com curinga) e C++ (o monitor).
- Testes: `mamutetest` (71 checagens, offline e online com a maquina real), `repltest` (modo monitor, local e conectado) e `mantest` (as paginas). `ctest` 27/27.
- Projeto e fases (2: `EDIT`/`A` pelo fwasm; 3: telas `DM`/`ZAP`/`SCR`; 4: os 36 comandos do SUPER-X): [mamute-spec.md](mamute-spec.md).

## [1.39.2] - 2026-10-19 - "Yie Ar Kung-Fu: Amazonia" (Konami)

### Corrigido -- achados ao montar o jogo Amazonia (cartucho de 48 KB, `tests/asm/real/amazonia`)
- **Fonte numa subpasta** (`fwmsx --asm source/amazonia.asm`): o CLI prefixava o diretorio do fonte duas vezes e nenhum `.include`/`.incbin` era achado.
- **Caminhos relativos ao diretorio atual**: `.include "source/x.asm"` e `.incbin "64k/x"` agora tambem sao procurados a partir do diretorio atual (como o asMSX; ao lado do fonte tem prioridade).
- **`#F3AE`** (hexadecimal com `#`, alem de `$F3AE` e `0F3AEh`) aceito.
- Teste novo no `asme2etest`: o `.rom` (48 KB) e o `.sym` do Amazonia saem iguais aos do asMSX v1.2.0 (CRC).

## [1.39.1] - 2026-10-16 - "Yie Ar Kung-Fu: Replayer PT3" (Konami)

### Corrigido -- achados ao montar um programa real (replayer PT3 do SapphiRe, `tests/asm/real/pt3`)
- **`ROTULO: .include "arq"` na mesma linha**: o pre-processador so' reconhecia `.include` no inicio da linha; agora o rotulo marca o inicio do texto incluido (como no asMSX).
- **`nome .equ valor`** (com ponto) passa a ser aceito, alem de `nome equ valor`.
- Teste novo no `asme2etest`: monta `example.asm` + `PT3-ROM.ASM` + `PT3-RAM.ASM` + `AMAZONIA.PT3`, confere o `.rom` e o `.sym` (CRC) contra o asMSX v1.2.0 e RODA a ROM na
  maquina emulada, vendo a musica tocar no PSG.

## [1.39.0] - 2026-10-16 - "Yie Ar Kung-Fu" (Konami)

### Adicionado -- o fwasm vira um compilador completo para o MSX
- **Cartuchos**: `.rom` (cabecalho `AB` + INIT, tamanho em multiplos de 8 KB ou `.size`) e **MegaROM** com os quatro mappers (`.megarom konami|konamiscc|ascii8|ascii16`,
  `.subpage n at X`, `.select n|registrador at X`), `.search`.
- **Programas**: `.basic` (arquivo `.bin` para `BLOAD"x",R`), `.msxdos` (`.com` em 0100h) com `.calldos`, `.callbios`.
- **Fitas**: `.cas`/`.cassette` (fita para emuladores), `.wav` (fita para um MSX de verdade, 44,1 kHz 8 bits, sintetizada em Assembly NASM) e `.sinclair` (`.tap` do ZX Spectrum,
  com carregador BASIC quando ha' `.start`).
- `.start` (cabecalho da ROM, MegaROM, BASIC, `.tap`), `.size n` (tamanho do arquivo em KB), `.debug "texto"` e `.break [X]`/`.breakpoint` (marcadores do depurador do blueMSX).
- O `.sym` da MegaROM mostra a subpagina de cada rotulo (`01h:8000h nome`).
- Tudo conferido **byte a byte contra o `asmsx.exe` de verdade**: 382 casos novos em `tests/asm/golden/formats.golden` (os `.wav` por tamanho + CRC-32), alem dos 6224 anteriores.
- Manual dentro do fwMSX: `man fwasm-formats` e `man fwasm-megarom` (e `man fwasm`, `man fwasm-directives` agora sem nada em falta).
- Novo modulo **Assembly (NASM dual-ABI)** `src/asm/asm/wav_synth.asm`: `fwasm_wav_render`, o sintetizador dos pulsos da fita.

### Corrigido
- O CLI `--asm` fixava o nome-base sempre; por isso o `.filename "x"` nunca valia na linha de comando. Agora so' o `-o` fixa o nome.

### Nao reproduzido (do asMSX)
- Codigo em 0000h antes de `.sinclair` (bloco de 32 KB) e a violacao de acesso do asMSX com `.org` abaixo do inicio de uma subpagina; detalhes em `doc/asm-spec.md`.

## [1.38.5] - 2026-10-16 - "Road Fighter: Nestor, SEE e fwasm" (Konami)

### Adicionado
- **Manual do nosso montador (`man fwasm`)**, em portugues, no lugar do manual do asMSX: 11 paginas -- `fwasm` (uso, saidas, estado, exemplo de cartucho de 16 KB TESTADO), `fwasm-syntax`, `fwasm-labels`,
  `fwasm-numbers`, `fwasm-operators`, `fwasm-data`, `fwasm-directives` (cada diretiva com o ESTADO no fwasm: PRONTA / PARCIAL / FALTA), `fwasm-conditional`, `fwasm-macros`, e `fwasm-bios` / `fwasm-biosvars`
  (124 rotinas e 144 variaveis, geradas das MESMAS tabelas que o montador usa, `bios_tables.inc`). Baseado no manual do asMSX 1.2.0 (BSD-3), ajustado ao que o fwasm de fato faz hoje (sem os formatos de saida rom/megarom/basic/msxdos/cas/wav ainda).
- **NestorBASIC** (`man nestorbasic`, `man nb-23`, ou pelo wrapper `man nb_copyvramtovram`): a introducao e as 87 funcoes da biblioteca do Nestor Soriano (RAM mapeada, disco, VRAM, som, Moonblaster), em portugues, com
  entrada/saida de cada funcao.
- **SEE Tracker** (`man seetracker`, `man see-...`): 22 paginas sobre o Sound Effect Editor 3.10: uso do editor, teclas, formato do arquivo .SEE, motor de replay (bits do evento, registradores do PSG, slides), integracao com o BASIC.
- **MSXBAS2ROM** (`man msxbas2rom`, tambem `msx-turbo-basic`): compilador de MSX-BASIC para ROM (54 paginas: instalacao, uso, comandos e funcoes estendidas, recursos, musica, depuracao...). Em ingles.
- `tools/paleobasic/import_tools_help.py` (gera `src/man/cpp/man_pages_tools_help.cpp`).

### Nao importado (por escolha)
- Manual do asMSX do paleobasic (substituido pelo `fwasm`), N80/LK80/LIBSTOR (por enquanto), openMSX, e o grupo "Rumo a um tracker compativel" do SEE (plano de trabalho do editor do paleobasic).

## [1.38.4] - 2026-10-16 - "Road Fighter: MSX-DOS 2 e Z80" (Konami)

### Adicionado
- **MSX-DOS 2** (manual 2.20, ingles), partido para consulta: **um topico por comando** (`man dos2-copy`, `dos2-dir`, `dos2-xcopy`, `dos2-chdir`... 40 comandos, com Format/Purpose/Use/Examples),
  os capitulos (`dos2-notation`, `dos2-redirection-and-piping`, `dos2-batch-files`, `dos2-environment-items`, `dos2-errors-and-messages`, `dos2-command-summary`, `dos2-disk-basic-2-0`),
  a interface de programa (`dos2-program-interface`) e **uma pagina por chamada de funcao** (`man dos2-fn-09h`; tambem pelo simbolo, `man _strout`, e pelo nome, `man dos2-fn-string-output`; 92 chamadas).
- **MSX-DOS 1 e CP/M** (`man dos-dir`, `dos-copy`, `dos-format`... e `man cpm`).
- **Z80 e R800**: tabelas de instrucoes (`man z80-opcodes`, `z80-mnemonics`, `z80-overview`, `r800-instructions`, `r800-msx`).
- **FM-PAC** (`man fmpac-manual`) e **Turbo-Basic Compiler** (`man turbo-basic-compiler`).
- **Extensoes do MSX-BASIC em ingles** (SOFTWARE.CHM do RuMSX): as 85 extensoes de firmware `CALL ...` (`man call kanji`, `call music`, `call pcmplay`, `call tohijri`...), Disk-BASIC, MSX2/2+/Turbo-R
  (`set video`, `color sprite$`, `call pcmrec`), `operators`, `variables`, `using`, `copy screen`... e as referencias `ref-color-table`, `ref-error-codes`, `ref-screen-modes`, `ref-msx-music-voices`,
  `ref-kanji-basic`... Quando a palavra ja' existia (a do livro da Gradiente), a referencia em ingles entra como uma SECAO a mais da mesma pagina (complemento: exemplos de saida, "veja tambem").
- A fusao de nomes iguais tambem considera os apelidos da MESMA categoria (`for` entra em `for-next`, `interval` em `interval on/off/stop`).
- `tools/paleobasic/import_manuals_help.py` (gera `src/man/cpp/man_pages_manuals.cpp`, ~780 KB).

### Mudou
- Apelidos de referencia escritos a mao que conflitavam com topicos importados (`chput`/`chget` em `bios`, `variaveis` em `sysvars`) foram retirados.

### Nao importado
- `OpenMsxHelpData.pbi` (o fwMSX nao usa o openMSX) e os 8 capitulos do MSX2 Technical Handbook que vem no MANUALS.CHM (ja' temos o livro inteiro, 1.38.3). 25 verbetes "ainda sem pagina propria" do MSX-DOS 1.

## [1.38.3] - 2026-10-16 - "Road Fighter: Livros de referencia" (Konami)

### Adicionado
- **The MSX Red Book** (Avalon Software / Kuma Computers, 1985): **973 topicos** em 9 capitulos (PPI, VDP, PSG, ROM BIOS, ROM BASIC, mapa de memoria, rotinas em
  codigo de maquina). `man rb-expanders`, `man rb-exptbl`... (nomes `rb-<titulo>`).
- **MSX2 Technical Handbook** (ASCII Corporation, 1987): **1356 topicos** em 15 capitulos (BASIC, MSX-DOS, VDP, BIOS, listagem da BIOS, math-pack, bit block
  transfer, area de trabalho, mapa da VRAM e de E/S, codigos de controle...). Nomes `th2-<titulo>`.
- **Ajuda do Basic Dignified** (21 topicos, em portugues): sintaxe do dialeto (labels, defines, proto-funcoes, includes...), configuracoes, remtags e
  a suite original. `man dignified` abre a introducao; nomes `bd-<titulo>`.
- Os **links internos dos livros viram "VER TAMBEM"** (`man <nome>`): navegar pelo Red Book/Handbook pelo terminal.
- `man -l red` / `man -l handbook` listam os capitulos (varias categorias de uma vez); o indice (`man`) resume as familias grandes numa secao
  "Livros e listas grandes".
- `tools/paleobasic/import_books_help.py` (gera `src/man/cpp/man_pages_books.cpp`, ~2 MB de texto).

### Mudou
- `Find` e a fusao de nomes usam tabelas montadas uma vez (milhares de topicos).

## [1.38.2] - 2026-10-16 - "Road Fighter: BIOS e hardware" (Konami)

### Adicionado
- **Rotinas, variaveis e hardware do MSX no `man`**, importados do paleobasic (MSXBIOS.CHM do RuMSX, texto em ingles como no original):
  **554 topicos de BIOS/BDOS/RAM** (`man chput`, `man wrtvdp`, `man jiffy`, `man linl40`; rotinas da MainROM, SubROM, Disk-ROM, Hangul-ROM, BDOS e as
  variaveis da area de trabalho) e **33 de hardware** (RTC, PSG, SCC, MSX-Music/OPLL, MSX-Audio, MoonSound, V9958, V9990, matriz do teclado, mapa de portas,
  Kanji, PCM, MIDI, SCSI, caneta optica, fita): `man hw-rtc`, `man hw-psg`, `man hw-v9990`, `man opll`... Nomes iguais em areas diferentes (WRTVDP na
  MainROM e na SubROM; BEEP na BIOS e no BASIC) viram secoes da mesma pagina.
- **`man -l [categoria]`**: sem argumento, as categorias e quantos topicos tem cada; com uma (`man -l bios-rom`), os nomes de todos os topicos.
- **`man -k texto`**: procura o texto nos nomes, apelidos e resumos (`man -k sprite`).
- `tools/paleobasic/import_bios_help.py` (gera `src/man/cpp/man_pages_msxbios.cpp`).

### Mudou
- O indice (`man`) mostra so' a CONTAGEM das familias grandes (BIOS-ROM, BDOS, Variaveis RAM), com a dica de como listar.
- A fusao de nomes iguais ficou generica (secao `==== categoria ====` dentro da pagina), no lugar do texto fixo "NO MSX-BASIC".

## [1.38.1] - 2026-10-16 - "Road Fighter: Manual do BASIC" (Konami)

### Adicionado
- **Manual completo do MSX-BASIC no `man`**, em portugues, importado do paleobasic (`resource/paleobasic/src/editor/help/*.pbi`): **172 palavras
  reservadas** (`man print`, `man screen`, `man put sprite`, `man for`, `man call music`...), cada uma com tipo (instrucao/funcao), formato, exemplo,
  descricao e programa exemplo, do livro *Linguagem BASIC MSX* (Denise Santoro Cruz, Aleph/Gradiente, 1986) e do *Manual MSX 2+ FM* (ACVS) --
  `screen`, `color`, `width`, `circle`... mostram o verbete do MSX1 e, em seguida, o do MSX2+. Mais **23 topicos do manual**: estrutura do BASIC
  (modos, caracteres, constantes, variaveis, operadores, edicao), interrupcoes, arquivos, rotinas em linguagem de maquina, apendices (ASCII,
  **codigos de erro**, **mapa da memoria**, funcoes trigonometricas) e o FM-Music (instrumentos, dicas, exemplos).
- `help <palavra>` mostra o formato e o resumo (`help print`); `stop` e `wait` (que tambem sao comandos do fwMSX) mostram o comando e,
  na mesma pagina, a palavra do BASIC.
- Nomes de varias palavras (`man put sprite`), apelidos para os verbetes compostos (`man for`/`man next` -> for-next, `man err` -> erl/err,
  `man interval`...) e prefixo unico (`man interval` abre `interval on/off/stop`).
- `tools/paleobasic/import_basic_help.py`: o conversor (importacao unica; gera `src/man/cpp/man_pages_basic.cpp`, que e' o que vai ao git).

## [1.38.0] - 2026-10-16 - "Road Fighter" (Konami)

### Adicionado
- **`man <topico>`** (console e TUI): manual completo com exemplos. Console: pager pela altura do terminal (espaco/Enter/q); TUI: janelinha
  rolavel (setas, PgUp/PgDn, Home/End, Esc/Q). Aceita nome parcial ("man sysv" sugere sysvars). Ver `doc/man-spec.md`.
- **`help <comando>`**: uso e resumo do comando (o manual completo e' o `man`).
- **Topicos de referencia do MSX** no `man`: `bios` (entradas 0000h-0189h e as do MSX2), `sysvars` (variaveis do sistema), `hooks` (ganchos
  FD9Ah-FFC9h e como encadear), `basic`, `cores`, `screens`, `vdpregs`, `psgregs`, `portas`, `slotsmsx` e `teclado` (a matriz, gerada
  das tabelas do proprio emulador). As tabelas foram CONFERIDAS contra as BIOS do repositorio (104 entradas = JP; 110 de 111 ganchos
  chamados nas ROMs) e contra a maquina emulada depois do boot.
- Modulo novo `src/man` nas quatro linguagens: C++ (registro e formatacao), C (`man_wrap_break`), Fortran (`man_match_score`) e Assembly
  (`man_fold`, NASM dual-ABI). Teste proprio `mantest` (ctest 25 testes).
- `clear` e todos os comandos do depurador no TAB do console.

### Mudou
- **`help`** deixou de ser uma linha so': agora e' uma tabela agrupada (Console, Emulador, Midia e estado, Depurador, Ferramentas), com
  separadores e o resumo de cada comando alinhado; ferramentas registradas e nao conhecidas pelo man aparecem em "Outras ferramentas".
- Janela de terminal, `help` e `man` seguem a largura do terminal (ate' 100 colunas).

### Notas de build
- O gfortran 16.2 do MSYS2 so' compila no ambiente do PowerShell/`build.ps1`; no Git Bash o PATH carrega DLLs erradas e o `f951` da' falha
  de segmentacao. Alem disso `iachar` (e uma conversao `int(ichar(...), c_int)` dentro de uma chamada) derruba o `f951`: usar `ichar` e
  variaveis intermediarias (ver `man_match.f90`).

## [1.37.8] - 2026-10-16 - "Knightmare: Teclas no --keys" (Konami)

### Adicionado
- **`--keys` aceita a sintaxe de teclas do `type`**: `--keys "print 1{enter}"`, `{esc}`, `{f1}`, `{ctrl}+{c}`, `{ctrl}{c}`, `{enter*3}`. O mesmo
  interpretador (`Commander::ParseTypeSpec`, agora estatico, com modo literal: aspas e espacos continuam texto comum e `|` continua Enter).
  Erro de sintaxe (tecla desconhecida, `{` sem `}`) sai com mensagem e codigo 2, antes de digitar. Atencao: um `{` que antes era texto agora abre uma tecla.

## [1.37.7] - 2026-10-16 - "Knightmare: Teclas no type" (Konami)

### Adicionado
- **`type` aceita teclas entre chaves, fora das aspas**: `type "key off" {enter}`, `{esc}`, `{f1}`, `{stop}`... `{ctrl}+{c}` aperta as teclas
  JUNTAS (Control-C); `{ctrl}{c}` aperta o Control, solta, e depois o C; `{enter*3}` repete; `{ctrl+c}` = `{ctrl}+{c}`. Nomes = a matriz do MSX,
  com apelidos (return, escape, backspace...). Funciona no console, na TUI e na porta de controle do emulador.
  Texto entre aspas e palavras soltas continuam como antes. Detalhes em `doc/control-spec.md` secao 2b.

## [1.37.6] - 2026-10-16 - "Knightmare: TUI sem sujeira" (Konami)

### Corrigido
- **TUI: comandos com saida (`regs`, `status`...) enchiam o historico de codigos ANSI e desalinhavam a tela.** O FTXUI desenha escrevendo em
  `std::cout`, e o worker trocava o `rdbuf` GLOBAL do `std::cout` para capturar a saida do comando: qualquer redesenho da thread da
  interface nesse intervalo (tecla, resposta) caia no historico como texto. Agora um `streambuf` roteador (`ThreadCaptureBuf`) captura so' o
  que a thread do worker escreve; a thread da interface continua escrevendo no terminal.
- Conferido na TUI real (`tools/conperf`, novo `CONPERF_CMD=regs`): a saida do `regs` aparece limpa.

## [1.37.5] - 2026-10-16 - "Knightmare: TUI mais leve" (Konami)

### Mudou
- **TUI**: deixou de se redesenhar 12 vezes por segundo SEMPRE (inclusive parada, com o terminal inteiro a cada quadro). Agora redesenha a cada
  tecla/clique, a cada resposta ou estado novo do emulador (lido a cada 0,4 s) e enquanto um comando roda. Em repouso sao ~2,5 redesenhos/s
  em vez de 12,5 -- menos saida disputando o terminal com a digitacao. (Pedido: "a digitacao no CLI esta quase impossivel".)

### Investigacao da lentidao de digitacao (2026-10-16)
- Medida com `tools/conperf/conperf.cpp` (injeta teclas num console oculto com `WriteConsoleInput` e le o buffer): o console `--cli` e a TUI
  mostram 36/36 letras, mesmo a 3 ms entre teclas, **com e sem o emulador rodando e com audio** (latencia mediana 16 ms no `--cli`, 32-47 ms na
  TUI). O emulador usa ~30% de um nucleo (idle) e roda a ~350 quadros/s headless (6x o tempo real). **Nao reproduzi a lentidao relatada**: ela
  deve depender do terminal usado (Windows Terminal/ConPTY, cmd, PowerShell...) ou da maquina -- pedi os detalhes ao usuario.

## [1.37.4] - 2026-10-16 - "Knightmare: Teclado japones" (Konami)

### Corrigido
- **`type`, `--keys` e o modo terminal digitavam a pontuacao no teclado INTERNACIONAL** mesmo com a BIOS de teclado japones (a do MSX2+ do
  fMSX): `(` saia `)`, `=` saia `^`, `"` saia `*`... Agora o layout vem da BIOS carregada (byte 002Ch, bits 0-3: 0 = japones/JIS; os demais usam o
  internacional) -- `Machine::keyboard_layout()`, `KeysFor()` e `KeysForCharLayout()`. As duas tabelas conferem com as da PROPRIA BIOS em
  `controltest` (MSX.ROM 0DA5h; MSX2P.ROM 0DC9h/0DE3h/0DEFh), e um teste de ponta a ponta digita `print (1+2)*3:rem a=b` no BASIC do MSX1 e do MSX2+.
- O layout internacional ganhou `\ { } ~`.

### Adicionado
- Tecla `ro` (linha 2, bit 5 da matriz): o `_` do teclado japones (88 teclas na matriz).
- `tools/wincap/wincap.ps1`: manda teclas a UMA janela por PostMessage e grava uma captura so' dela (PrintWindow) -- verificacao visual sem
  fotografar a area de trabalho.

### Verificado em tela (janela real, via `wincap`)
- **F10 / Z80 Debugger**: abre sobre o jogo com a fonte do MSX; ENTER passo, setas (Cima/Baixo movem o cursor), B (breakpoint `*`), M (dump com os
  graficos do MSX), G (pergunta e vai ao endereco), C (continua e o painel fecha) e Q (encerra o emulador) funcionam.
- **Splash**: aparece com `--splash`; `splash=0` no `fwmsx.cfg` o desliga.

## [1.37.3] - 2026-10-13 - "Knightmare: Depurador na TUI" (Konami)

### Adicionado
- **Painel Z80 Debugger na TUI** (`--tui`): o mesmo painel da janela (registradores no topo, `ENDERECO: bytes instrucao`, ENTER/setas/G/D/M/S/J/R/C/B/Q),
  como caixa de texto no terminal. Menu **Depurador > Painel Z80 Debugger**; abre sozinho quando o emulador para num breakpoint, watchpoint ou
  "rodar ate'". Num terminal baixo mostra o cabecalho, o rodape (se couber) e a parte do corpo com o cursor.
- **Comando `dbgui`** (ponte de controle): `open`, `show`, `key <tecla>`, `char <c>` -- o painel como texto (`ok +33`, linha de estado e 32 linhas com
  letra de estilo), calculado no emulador pelo mesmo modelo da janela; qualquer cliente pode usar.
- MANUAL: secao "O painel Z80 Debugger na TUI"; `doc/debug-spec.md`, secao 6c; README atualizado ao estado de hoje.

## [1.37.2] - 2026-10-13 - "Knightmare: Depurador na janela" (Konami)

### Adicionado
- **Z80 Debugger na janela do emulador** (F10, ou menu Depurador), no estilo do depurador do fMSX: painel com os registradores nas
  primeiras linhas, uma linha em branco e `ENDERECO: bytes em hexa  instrucao com os parametros`; ENTER executa a proxima instrucao,
  Cima/Baixo, Esquerda/Direita (pagina), G (ir ao endereco), D (desmontagem), M (dump: 8 bytes em hexa e os mesmos 8 como caracteres do
  MSX), S (pilha), J (pular para o cursor), R (rodar ate' o cursor), C (continuar), B (breakpoint no cursor), Q (encerrar o emulador).
  Desenhado com a FONTE DO PROPRIO MSX (a tabela de caracteres da BIOS). Abre sozinho em breakpoint/watchpoint/`until`/`--debug`.
- `src/debug/cpp/debug_view` (modelo e desenho, sem OpenGL, testado em `controltest`); `LoadFontFromMachine`, `RenderRgba`.
- MANUAL: secao "O Z80 Debugger na janela (F10)" com o layout e a tabela de teclas; `doc/debug-spec.md`, secao 6b.

### Mudou
- F10 passou a ser do depurador (nao e' tecla do MSX); com o painel aberto o teclado e' dele.

## [1.37.1] - 2026-10-13 - "Knightmare: Splash e icone" (Konami)

### Adicionado
- **Splash de abertura** (`images/fwMSX-02.png`, reduzido a 1376x768 em `dist/fwMSX-splash.png`): a janela do emulador mostra a
  imagem por 3 s antes de ligar o MSX (fade no fim; tecla ou clique pula). Desligavel em **Ferramentas > Interface > Mostrar
  splash ao iniciar** (gravado) e por linha de comando: `--no-splash` (nao mostra) e `--splash` (mostra). `--cli`, `--tui` e
  `--term` NAO mostram, salvo `--splash` (imagem em meios blocos coloridos no terminal); `emu start` abre a janela sem splash;
  `--cmd` (por vir) nunca mostra. Ver `doc/splash-spec.md`.
- **Icone do aplicativo** (`images/fwMSX-03.ico`): embutido no `fwMSX.exe`/`fwMSXc.exe` (Explorer, barra de tarefas, Alt+Tab, canto da
  janela) e, no Linux, `glfwSetWindowIcon` + `fwmsx.png`/`fwmsx.desktop` no pacote.
- **`fwmsx.cfg`**: tema, tamanho da letra, moldura e splash agora ficam gravados (antes voltavam ao padrao a cada abertura).
- Leitor de PNG (`src/splash/png_reader`), politica do splash testavel, ferramenta `tools/assets/make_assets.py`; teste
  `splashtest` (CTest `splash`, suite 24).
- **MANUAL**: o depurador ganhou um passo a passo completo, a referencia dos comandos, os parametros e os avisos; secao nova sobre o
  splash, o icone e as configuracoes.

## [1.37.0] - 2026-10-13 - "Knightmare" (Konami)

### Adicionado
- **Depurador basico do emulador** (`doc/debug-spec.md`): breakpoints de execucao e watchpoints de memoria
  (leitura/escrita/faixa), passo (`stepi`), passo sobre (`next`: CALL/RST/LDIR.../HALT), `until`, `go`, `stop`,
  `jump`, `dis`, `mem`, `stack`, `reg` (com flags e alteracao), `slots`. Pela ponte de controle, pelo console
  (`--cli`), pelo menu **Depurador** da TUI e pela linha de comando do modo terminal. Estilo do depurador do fMSX.
- **Eventos assincronos**: `events on` assina, por conexao, linhas `! break id=1 pc=0038 hits=1`, `! watch ...`,
  `! until ...` enviadas pelo emulador sem o cliente perguntar. O console mostra `[evento] ...` (e tem `wait`), a TUI poe
  no historico (barra `PARADO (depurador)`), o modo terminal no log, a janela no stderr.
- **Respostas de varias linhas** na ponte: `ok +N` e N linhas (comandos antigos inalterados).
- **Parametros do `--msx`**: `--break <end>`, `--watch <end>[:tam[:r|w|rw]]`, `--debug` (comeca parado no PC 0000).
- `status` ganhou `stopped=0|1`; titulo da janela mostra `PARADO: depurador`.
- Codigo em C (`dbg_flow`), Fortran (`dbg_fmt`), Assembly (`dbg_range`, NASM Win64/SysV) e C++ (`Debugger`,
  `DebugBus`); `Machine::StepInstruction()`/`RunUntilStop()`. Testes: `controltest` (depurador e eventos por TCP) e `tuitest`.

### Mudou
- A TUI ganhou o menu **Depurador** (7 menus); o status do emulador e' lido a cada 0,4 s (era 1 s).
- Parado pelo depurador, `RunFrame()` nao anda o quadro.

### Falta
- Depurador sofisticado (condicoes, simbolos do `fwasm`, trace, E/S e interrupcoes, painel na TUI): `doc/debug-spec.md`, secao 7.

## [1.36.2] - 2026-10-13 - "Salamander: MegaROM com AB no fim" (Konami)

### Corrigido
- **R-Type (e MegaROMs parecidas) nao iniciava**: duas diferencas em relacao ao fMSX, achadas pela comparacao
  quadro a quadro com jogos do acervo do file-hunter (`tests/media/`):
  1. MegaROM cujo tamanho nao e' potencia de 2 (R-Type: 48 bancos de 8KB; Metal Gear: 20) passa a ser completada para
     2^n bancos espelhando o fim, e a mascara de banco vira 2^n-1 (`MemorySystem::LoadRom`). Antes a mascara
     (bancos-1) nao era uma mascara de bits e o teste de SRAM do ASCII16/ASCII8 (`valor & (mascara+1)`) pegava
     bancos validos (17h em R-Type), travando o INIT.
  2. MegaROM SEM "AB" no inicio mas COM "AB" nas duas ultimas paginas de 8KB (da imagem ja' completada) comeca
     nessas paginas (`memmap_attach_megarom`; `ResetMSX` do fMSX: "Some MegaROMs default to last pages on reset").
  R-Type agora bate 100% pixel a pixel com o fMSX.

### Adicionado
- `tests/media/`: 19 ROMs de jogo do acervo do usuario, **provisorias** (saem na versao final, que nao leva midia
  nenhuma): a comparacao com o fMSX original passou a ter 26 casos, todos aprovados.
- `compare.py`: correspondencia de cores um-para-um, metrica de detalhe, `CARTS.SHA` para os dois emuladores.
- Teste `memmap_slots` secao 27 (MegaROM de 48 bancos: espelho, mascara, AB no fim e no inicio).

## [1.36.1] - 2026-10-13 - "Salamander: Texto de SCREEN 0 como no fMSX" (Konami)

### Corrigido
- O texto de SCREEN 0 (40 colunas) e o TEXT80 passam a comecar onde o fMSX os desenha: no pixel 9 da area de 256
  (9 de borda a esquerda e 7 a direita; `RefreshLine0`/`RefreshLineTx80`), e nao centrados em 8/8. Achado pela
  comparacao quadro a quadro com o fMSX original (`tools/fmsxgo_ref/compare.py`): o texto saia 1 pixel a esquerda.
  `Machine::RenderFrame`, `src/machine/machine.cpp`.

### Adicionado
- **Comparacao de quadros com o fMSX original** (ferramenta de desenvolvimento, fora do `ctest`):
  `tools/fmsx_ref/Headless.c` compila o nucleo ORIGINAL de `resource/fMSX` sem janela; `tools/fmsxgo_ref/
  compare.py` roda a mesma ROM nele e no fwMSX, alinha os quadros e compara por pixel e por forma. 7 casos (BIOS
  MSX1/MSX2/MSX2+, King's Valley, F1 Spirit, Lode Runner SCC, Firebird): **100% pixel a pixel** depois desta
  correcao. O port em Go (fmsxgo) fica como segunda opiniao (`--ref fmsxgo`). Ver `doc/fmsx-compare-spec.md`.

### Observacao
- O TEXT80 foi ajustado pelo mesmo criterio do fMSX (`P+=18` na tela larga), mas ainda nao tem caso na comparacao
  (o `Headless.c` nao injeta teclas).

## [1.36.0] - 2026-10-13 - "Salamander" (Konami)

### Adicionado
- **Montador Z80 `fwasm`** (`fwmsx --asm [-z] [-s] [-vv] [-o <caminho>] <arquivo.asm>`; no console, o comando `asm`):
  reescrita clean-room do asMSX 1.2.0 a partir do manual, em C (`src/asm/core/z80_enc.*`, codificador das
  instrucoes), Fortran (`src/asm/fortran/asm_math.f90`, funcoes das expressoes) e C++ (`src/asm/cpp/`: lexer,
  expressoes, pre-processador com `.include`/macros/`REPT`, montador em varias passadas, CLI). Saidas `.z80`,
  `.sym` e `.txt`; diretivas `org`, `page`, `phase`/`dephase`, `zilog`, `bios`/`biosvars`, `db`/`dw`/`ds`/`byte`/
  `word`, `incbin`, `print*`, `filename` e `start`. Ver `doc/asm-spec.md`.
- Teste `asmtest` (CTest `asm`, suite 23): ~6.200 casos comparados com saidas geradas pelo `asmsx.exe` de
  verdade (`tools/asmdiff/`, o oraculo, so' como caixa-preta; o fonte do asMSX ficou em `resource/asMSX/` apenas
  como referencia de estudo).

### Falta (proximas patches do montador)
- Formatos de saida `rom`/`megarom`/`basic`/`msxdos`/`cas`/`wav`, `select`/`subpage`/`search`, `debug`/`break`,
  e o modulo em Assembly (sintese de pulsos/WAV/CAS).

## [1.35.0] - 2026-10-09 - "Vampire Killer" (Konami)

### Adicionado
- **TUI de menus** (`fwMSXc.exe --tui`, FTXUI): os comandos do console em menus (Emulador, Arquivo,
  Maquina, Midia, Ferramentas, Ajuda), com barra de menus (F10 ou mouse), linha de comando com historico
  e Tab, assistentes de varios passos e navegador de arquivos. Controla o emulador em **janela** pela
  ponte de controle (nao embute a maquina); `Emulador > Iniciar MSX1/MSX2/MSX2+` abre a janela. As
  ferramentas (`romdb`, `cas`, `fitadb`, `msxdisk`, `newdisk`) rodam numa thread a parte e a saida vai para
  o historico. Ver `doc/tui-spec.md`.
- `repl::BuildSession()`: o console e a TUI usam a mesma sessao, com as mesmas ferramentas.
- Testes: `tuitest` (CTest `tui_model`, suite 22): todo o comportamento dos menus sem terminal; teste de
  fumaca real com `--tui --attach <porta> --run "..." --dump`.

### Mudou
- `--tui` deixou de ser "reservado": e' a TUI de menus. O terminal embutido continua em `--term`.

## [1.34.0] - 2026-10-09 - "Metal Gear" (Konami)

### Adicionado
- **Modo terminal do emulador** (`fwMSXc.exe --term [opcoes do --msx]`, FTXUI; o nome `--tui` fica reservado para uma TUI de menus futura): a maquina roda dentro do
  terminal. Texto de verdade em SCREEN 0/1/TEXT80; graficos em meios blocos reduzidos. Menu (F10),
  linha de comando (F11) e os mesmos atalhos da janela (F6/F7 estado, F8/F9 slot, F12 captura). Todo
  comando passa pelo `control::Commander`; aceita `--ctl-port`. Ver `doc/term-spec.md`.
- Graficos do modo terminal na proporcao **4:3** (a imagem inteira e' ajustada), com alternancia para pixels
  originais no menu F10.
- `src/machine/screenshot.{h,cpp}`: a captura PNG virou compartilhada (janela e modo terminal).

### Observacao
- O modo terminal nao tem teste automatizado (precisa de um terminal); foi validada pela ponte de controle e pela
  saida renderizada. O `Commander` que ela usa e' coberto por `controltest`.

## [1.33.0] - 2026-10-09 - "Penguin Adventure" (Konami)

### Adicionado
- **Console interativo (REPL)**: `fwmsx --cli` (`src/repl/`, replxx com historico e TAB). O
  terminal fica livre e o emulador roda em outro processo: `emu start [opcoes do --msx]` abre a
  janela e conecta pela ponte de controle; `emu attach <porta>`, `emu detach`, `emu stop`,
  `emu status`. Todo comando que o console nao conhece vai para o emulador (`peek`, `poke`, `type`,
  `cart`, `state`...). Ver `doc/repl-spec.md`.
- As opcoes soltas viram comandos do console: `newdisk` (= `--disknew`), `romdb`, `cas`, `fitadb`,
  `msxdisk`.
- Testes: `repltest` (CTest `repl`, suite 21): sessao contra um servidor de controle real e
  `emu start` com iniciador falso; smoke test manual com o executavel real.

- **`fwMSXc.exe`** (Windows): o mesmo programa com o subsistema "console", gerado no build por
  `tools/pe_subsystem`. O `fwMSX.exe` (janela) nao e' esperado pelo terminal e disputa o teclado com
  ele, o que quebrava o console interativo (letras trocadas, lentidao, saida sozinho); o
  `fwMSXc.exe` e' o certo para `--cli` e para as ferramentas de linha de comando. Vai no `.zip`.

### Corrigido
- Windows: ao abrir de um terminal, os handles do sistema (nao so' o stdio do C) passam a apontar
  para o console -- o replxx usa `GetStdHandle`.
- `fwMSX.exe --cli` num terminal agora avisa para usar o `fwMSXc.exe` em vez de se comportar mal.

## [1.32.0] - 2026-10-09 - "Nemesis" (Konami)

> **Novo esquema de nomes (2026-10-09):** X = empresa de MSX (1 = Konami), Y = jogo novo de MSX
> (1.32 = Nemesis), Z = nome do jogo + subtitulo. As versoes antigas (1.19 a 1.31) ficam como
> "King's Valley".

### Adicionado (alem do que segue: o `fwmsx.port` tambem e' gravado ao lado do executavel e a porta aparece no titulo da janela)

### Adicionado
- **Ponte de controle externa** (`src/control/`): outros programas (editor, depurador, montador, a
  futura TUI) mandam comandos ao emulador por **TCP em 127.0.0.1**, num protocolo de texto de uma
  linha por comando (`peek 0xC000 4` -> `ok 00 01 02 03`). Ligar com `fwMSX.exe --msx --ctl-port 0`
  (0 = porta livre; a porta vai para `fwmsx.port`). Comandos: help, version, status, reset, pause,
  resume, step, type, peek, poke, regs, cart, disk, eject, tape, state, screenshot, quit. Ver
  `doc/control-spec.md`.
- **Um despachante de comandos para tudo** (`control::Commander`): a janela, e depois a TUI e a
  linha de comando, usam a mesma camada. As threads do servidor so' enfileiram; os comandos rodam
  na thread do emulador, entre quadros.
- `Machine::ReadMemory()/WriteMemory()`, `ParseMapperName()` publico.
- Testes: `controltest` (CTest `control`, suite 20): despachante sobre uma Machine de verdade e
  servidor TCP real; smoke test manual com a janela real.

## [1.31.0] - 2026-10-09 - "King's Valley: Disco novo em branco e FORMAT"

### Adicionado
- **Disco novo em branco e formatado** (`src/diskfmt/`, C + Assembly + C++, reescrita do
  `CreateDisk()` do msxDiskUtil em PureBasic): **Midia > Disco > Novo disco em branco em A:/B:**
  e `fwmsx --disknew <arq.dsk> <ss525|ds525|ss35|ds35>`. Os 4 formatos do MSX: 5 1/4 face simples
  180 KB, 5 1/4 face dupla 360 KB, 3 1/2 face simples 360 KB, 3 1/2 face dupla 720 KB. **Nao** ha'
  3 1/2 de 1,44 MB (o MSX nao suporta). `diskfmt_fill()` (Assembly dual-ABI) enche a area de dados
  com E5h. Ver `doc/diskfmt-spec.md`.
- **FORMAT / CALL FORMAT**: o WD2793 aceita WRITE TRACK (F0h/F4h); o fluxo de formatacao do
  driver e' copiado para os setores que os campos de ID apontam. Testado com o DISK.ROM de
  verdade (`machinetest`, secao 7d: `CALL FORMAT` -> "Format complete").
- Testes: `diskfmttest` (CTest `diskfmt`, suite 19) e `machinetest` secao 7d.

## [1.30.1] - 2026-10-08 - "King's Valley: Atalhos de save-state, captura de tela e janela sem console"

### Adicionado
- **Atalhos de save-state com 9 slots**: **F6** salva e **F7** carrega o slot atual
  (`fwmsx-estado-N.sst`, N = 1 a 9, na pasta de trabalho); **F8/F9** trocam de slot (tambem pelo
  menu **Arquivo > Slot de estado**). F1-F5 sao teclas do MSX, por isso os atalhos comecam em F6.
- **Aviso na tela** (canto inferior esquerdo, ~2 s; ~5 s e laranja para erro/aviso): "Estado salvo
  (slot N)", "Estado carregado", troca de slot, captura de tela.
- **Validacao da midia no save-state**: o `.sst` guarda o CRC32 da BIOS e do cartucho (secao
  `MEDA`, ignorada por leitores antigos). Ao carregar sobre outra BIOS/cartucho, o estado e'
  aplicado mesmo assim, mas `Machine::state_warning()` vem preenchido e a janela avisa. Disco e
  fita ficam de fora (o disco muda a cada gravacao). Testado em `machinetest` (secao 5b).
- **F12 / Arquivo > Capturar tela (PNG)**: grava `fwmsx-AAAAMMDD-HHMMSS.png` na pasta de
  trabalho, com a proporcao certa (MSX2 de 512 colunas ganha linhas duplicadas), via
  `tdefl_write_image_to_png_file_in_memory()` do miniz (ja' linkado).
- **Sem janela de console no duplo clique (Windows)**: `fwMSX.exe` agora e' do subsistema
  "windows" (`-mwindows`); `main.cpp` religa stdout/stderr/stdin ao console do terminal quando
  aberto de dentro de um (so' as saidas NAO redirecionadas -- pipe/arquivo seguem intactos).
  Efeito colateral conhecido: `cmd`/PowerShell nao ESPERAM o fim do programa nem sempre
  (use `Start-Process -Wait` ou um pipe, ex.: `| Out-Host`).

### Investigado (sem mudanca de codigo)
- **BIOS Gradiente Expert 1.1, texto com espacos ("G r a d i e n t e")**: nao e' bug do
  emulador -- a string esta' gravada assim na propria ROM (`expert_1.1_basic-bios1.rom`, offset
  32513). Sobe normalmente com RAM de 64KB no slot 2:0.

## [1.30.0] - 2026-10-08 - "King's Valley: Menus reorganizados + mapper do cartucho na janela"

### Adicionado
- **Seletor de mapper do cartucho na janela**: `MSX-DOS 2` (e os outros 6 mappers de bank-switch)
  agora podem ser escolhidos sem a CLI. `Machine::SetCartridge()` ganhou um parametro `mapper`
  opcional (default `MEMMAP_MAPPER_NONE`, preserva o comportamento de sempre) e
  `Machine::CartridgeMapper()` novo. Dois pontos de acesso: submenu **Mapper** no menu Cartucho
  (com um cartucho ja' inserido, trocar o mapper reinicia na hora com o MESMO arquivo), e um
  combo na linha da ROM da **Configuracao de slots...**.
- **Menus reagrupados**: de 13 menus no topo da janela para 9 -- **Midia** (Disco/Fita/Cartucho
  como submenus), **Tela** (Exibir/Video como submenus), e **Ferramentas** incorporou
  **Configuracoes > Interface** (o antigo menu Configuracoes, so' com esse item, foi removido).
  Pedido do usuario apos usar a janela: "os menus estao ficando muito separados".
- Testes: `machinetest`/`machine_frames`, secao 2b nova -- `SetCartridge()`/`CartridgeMapper()`
  com e sem mapper explicito, e retirar o cartucho zera o mapper tambem.

### Corrigido
- **Achado pelo usuario testando a 1.28.0/1.29.0 pela janela**: o `MSXDOS2.ROM` so' carregava com
  o mapper certo pela CLI (`--cart <rom> msxdos2`) -- nem o menu rapido de cartucho nem a
  Configuracao de slots tinham como escolher um mapper explicito, entao a heuristica automatica
  (que nao reconhece o MSX-DOS 2) sempre vencia pela janela.

## [1.29.0] - 2026-10-08 - "King's Valley: Save-state"

### Adicionado
- **`Machine::SaveState()`/`Machine::LoadState()`** (`src/machine/machine.{h,cpp}`): gravam/
  restauram um retrato do estado AO VIVO da maquina (registradores do Z80, VDP incl. VRAM/
  paleta/modo, PSG, SCC, OPLL, PPI, controladora de disco, conteudo de RAM/RAM de mapper) num
  arquivo binario proprio (`.sst`, formato TLV versionado, NAO compativel com outros
  emuladores). `LoadState()` aplica por cima da maquina JA' RODANDO -- NAO recarrega BIOS/
  cartucho/disco/fita, pressupoe que a mesma midia continua inserida (como a maioria dos
  emuladores). Fora do escopo, documentado: conteudo de disco/fita/SRAM (ja persistem
  sozinhos, em arquivo proprio), CMOS do RTC (preferencia de BIOS), teclas pressionadas e
  joystick (entrada do mundo externo), e transferencias de disco/fita em andamento no instante
  exato do save (abortadas ao carregar, nunca corrompidas). Ver `doc/savestate-spec.md`.
- **`MemorySystem::MapperRamBase()`** (`src/memmap/cpp/memory_system.h`): novo acessor ao
  buffer INTEIRO de uma RAM de mapper (ate 1024KB) -- `PeekSlot`/`PokeSlot` so' enxergam os 4
  segmentos PAGINADOS agora, insuficiente para dumpar/restaurar um mapper completo de uma vez.
- **Menu Arquivo > Salvar estado.../Carregar estado...** (`src/machine/gui/emu_window.cpp`):
  dialogo de arquivo (`.sst`), substituindo o item desabilitado "Salvar estado (em breve)" que
  existia como placeholder desde a janela com menus (1.16.0).
- Testes: `machinetest`/`machine_frames`, secao 5b nova -- salva o estado, MUTILA a maquina de
  proposito (PC do Z80 e registrador do PSG escritos direto para valores errados, tela
  limpa), recarrega e confere PC/PSG/VRAM inteira restaurados byte a byte, mais os dois
  caminhos de erro (arquivo inexistente, assinatura errada).

## [1.28.0] - 2026-10-08 - "King's Valley: MSX-DOS 2 (cartucho generico)"

### Adicionado
- **`MEMMAP_MAPPER_MSXDOS2`** (`src/memmap/common/memmap_types.h`, `src/memmap/core/slot_state.{h,c}`):
  novo mapper para o cartucho generico de MSX-DOS 2, adaptado de `RomMSXDOS2` do openMSX (GPL,
  so' estudo -- `resource/openMSX/src/memory/RomMSXDOS2.cc`/`.hh`). NAO faz parte da lista
  original do fMSX (sem numero `MAP_*`). Diferente dos outros mappers MegaROM: granularidade de
  **16KB** (nao 8KB), so' a pagina 4000h-7FFFh e' trocavel (8000h-BFFFh fica sempre vazia, como
  0000h-3FFFh/C000h-FFFFh), e o endereco que dispara a troca vem de um byte gravado na propria
  ROM (offset 94h): 00h = so' 7FF0h; 60h = qualquer escrita em 6000h-6FFFh; 7Fh = so' 7FFEh.
  `MemorySystem::LoadRom()` recusa a ROM se esse byte nao for um dos 3 valores conhecidos.
- **`--cart <arquivo> msxdos2`**: novo valor aceito pelo parser de mapper da CLI
  (`src/machine/cli.cpp`), ao lado de `gen8`/`gen16`/`konami5`/`konami4`/`ascii8`/`ascii16`.
- **Testado contra o MSX-DOS 2.30 de verdade** (`MSXDOS2.ROM` real, 64KB, 4 bancos de 16KB +
  um disco de 720KB real com `MSXDOS2.SYS`/`COMMAND2.COM`/subdiretorios, ambos ja' presentes no
  repositorio): `--cart MSXDOS2.ROM msxdos2 --disk <disco>` sobe o kernel de verdade, `dir` lista
  arquivos e subdiretorios, `cd` entra num subdiretorio e `dir` dentro dele funciona -- sem
  precisar de NENHUM mapper de RAM (a suposicao antiga em `doc/fdc-spec.md`, "precisa de MSX2 +
  mapper de RAM", estava errada: um cartucho generico de MSX-DOS 2 so' precisa da interface de
  disco de sempre).
- Testes: `memmaptest`/`memmap_slots`, secao 25 nova (12 checagens: reset no banco 0, troca de
  banco de 16KB, pagina 8000h-BFFFh sempre vazia, os 3 valores de `range` do byte 94h, e recusa de
  um byte 94h desconhecido).

## [1.27.0] - 2026-10-08 - "King's Valley: Efeitos de rastreio (paleta/scroll por linha)"

### Adicionado
- **Efeitos de rastreio no meio do quadro** (paleta e scroll trocados por uma interrupcao de
  linha, IE1, tipico de alguns jogos MSX2): uma investigacao dedicada confirmou que a maquina de
  estados de scanline (`vdp_step_scanline()`) e a intercalacao de Z80/VDP T-state a T-state
  (`Machine::RunFrame()`) ja' entregavam a interrupcao IE1 no instante certo -- a lacuna real era
  so' de renderizacao: `Machine::RenderFrame()` lia UNICA o estado FINAL de `VdpState` para
  desenhar TODAS as linhas do quadro, entao qualquer mudanca feita por uma ISR no meio do quadro
  acabava retroativa a tela inteira, nao so' a partir da linha onde aconteceu.
- **`VdpScanlineSnapshot`** (`src/vdp/core/vdp_state.h`): copia dos campos que a renderizacao de
  fato le para decidir a cor de um pixel (registradores, paleta, cache de tabela de
  caractere/cor/padrao/sprite e suas mascaras, modo de tela, modelo, cores de frente/fundo do
  "blink"). `vdp_capture_snapshot()` grava um desses ao fim do processamento de CADA linha, dentro
  de `vdp_step_scanline()`; `vdp_apply_snapshot()` faz o inverso. Campos deliberadamente FORA do
  snapshot (nao mudam por linha ou nao afetam pixel): `vram[]`, `status[]` (so' lido por
  depurador/CPU, nunca pela renderizacao), campos de protocolo de porta, paginacao de VRAM e
  estado do motor de comandos V9938.
- **`Machine::RenderFrame()`** passou a trabalhar sobre uma COPIA local de `VdpState` (nao mais
  uma referencia ao estado vivo da maquina) e aplica o snapshot da linha `y` (`vdp_apply_snapshot`)
  imediatamente antes de desenhar essa linha (`vdp_render_line`) -- sem mudar a assinatura publica
  nem nenhum chamador (`cli.cpp`, `emu_window.cpp`).
- Testes: `vdp2test`/`vdp_msx2`, nova secao 10 -- troca a paleta do indice 1 na metade de um
  quadro simulado (50 scanlines antes, 50 depois) e confirma que o snapshot da linha 10 mostra a
  cor ANTIGA, o da linha 80 mostra a cor NOVA, e que renderizar sem snapshot (comportamento
  antigo) mostraria a cor nova na linha 10 tambem -- prova direta da diferenca.

## [1.26.0] - 2026-10-08 - "King's Valley: Vampier: Platform, CRC32 e Tamanho"

### Adicionado
- **`VampierHit` ganhou `platform`, `crc32` e `file_size`**: campos que o dump SQL do Vampier
  (`fwmsx --romdb vampier`) ja' importava desde sempre (`msxdb_rominfo.Platform`,
  `msxdb_romdetails.CRC32`/`FileSize`), mas que `VampierSearch()` nunca lia -- a consulta so'
  selecionava SHA-1/nome/ano/empresa/tipo/dump/observacao. `vsearch` agora imprime as 3 colunas
  novas tambem. Testado contra o dump REAL do Vampier (11.313 ROMs, 2026-10-08): `platform`
  ("MSX"), `crc32` e `file_size` vem certos.
- **Decisao registrada** (pedido original era "importar o JSON do Vampier, se for util"): baixado
  e comparado o JSON (`json-msxromsdb.zip`) contra o SQL (`sql-msxromdb.zip`) de verdade -- o JSON
  tem MENOS colunas que o SQL (que ja' era importado), entao nao haveria dado novo em importa-lo.
  A melhoria de valor real era expor os campos que o SQL ja' trazia mas a consulta ignorava --
  feito aqui. O JSON continua nao importado (ver `doc/romdb-spec.md`, secao 8).
- Testes: `romdbtest`/`romdb_store` com 2 checagens novas (platform/CRC32/FileSize presentes; e
  os mesmos campos NULL no dump nao travam, vem vazio/-1).

## [1.25.0] - 2026-10-08 - "King's Valley: Banco de ROMs: Mapper e Verificacao"

### Adicionado
- **Auto-deteccao de mapper pelo banco de ROMs**: ao carregar um cartucho com `--cart <arquivo>`
  SEM escolher o mapper a dedo, o emulador agora consulta o banco de ROMs (`fwmsx --romdb
  cartsha`, se ja' importado) pelo SHA-1 do arquivo ANTES de cair na heuristica por tamanho/
  conteudo de sempre (`memmap::GuessMapper()`, que continua intacta como fallback). Fecha o item
  "Carregar pelo banco" de `doc/romdb-spec.md`, secao 8. Falha em qualquer etapa (sem banco, SHA-1
  desconhecido) e' silenciosa -- e' uma camada OPCIONAL sobre o comportamento existente, nunca um
  requisito.
- **`fwmsx --romdb verify [--cat c]`**: recalcula o SHA-1 de cada ROM cadastrada e confere contra
  o que esta' gravado no banco -- detecta arquivo faltando ou alterado/corrompido desde que foi
  cadastrado. Fecha o item "Verificar as ROMs baixadas contra o SHA-1 conhecido" de
  `doc/romdb-spec.md`, secao 8. Codigo de saida 1 se algo faltar/divergir (script-friendly), 0 se
  tudo bater.
- Testes: `romdbtest`/`romdb_store` com 3 checagens novas para o `verify` (tudo ok; arquivo
  alterado + arquivo faltando; restaurado). Smoke test manual do auto-mapper com um cartucho
  sintetico cadastrado no banco, confirmando que o mapper escolhido pelo banco e' usado, que uma
  escolha explicita do usuario continua tendo prioridade, e que um cartucho desconhecido cai no
  fallback de sempre sem erro.

## [1.24.0] - 2026-10-08 - "King's Valley: Banco de Fitas (Metadados)"

### Adicionado
- **`fwmsx --fitadb`**: banco de fitas (SQLite) com metadados -- titulo, empresa, ano e SHA-1 --
  das fitas (`.cas`/`.tsx`/`.tzx`) que o usuario ja' tem no disco. **SEM download nenhum**: o site
  de referencia (tsx.eslamejor.com) nao publica termos de uso (verificado em 2026-10-06, ver
  `doc/SPEC.md`, secao 5.2, item 7) -- este banco so' cadastra o que o usuario ja' tem localmente,
  fecha o item (e) da lista de pendencias de fita.
  - `scan <pasta>`: cadastra todas as fitas de uma pasta (recursivo, so' `.cas`/`.tsx`/`.tzx`).
  - `add`/`list`/`search`/`show`/`edit`/`del`/`stats`: CRUD e busca, mesma estrutura do
    `fwmsx --romdb` (reaproveitada por analogia -- bancos SQLite separados, ja' que fita nao e'
    uma categoria de ROM).
  - **Auto-preenche o titulo** a partir do leitor de fita existente (`cas_reader`/`tzx_reader`):
    o nome do 1o arquivo encontrado dentro da fita, so' quando ela e' NOVA no banco (um titulo que
    o usuario edite manualmente nunca e' sobrescrito por um re-scan).
- Testes: `tapedbtest` (CTest `tapedb_store`) -- CRUD, busca, `ScanFile`/`ScanDirectory` (com o
  auto-preenchimento de titulo e a preservacao de edicoes manuais), e a CLI completa. Ver
  `doc/tape-spec.md`, secao 11.

## [1.23.0] - 2026-10-08 - "King's Valley: Navegacao de Blocos do TZX"

### Adicionado
- **Navegacao de verdade dos blocos de controle do TZX** (IDs 21-28, `src/tape/cpp/tzx_reader.cpp`):
  Grupo (#21/#22), Salto (#23), Laco (#24/#25), Chamada (#26/#27) e Selecao (#28) agora sao
  EXECUTADOS, nao so' pulados -- um arquivo que usa esses blocos pesadamente (ex.: protecoes/
  loaders customizados do mundo ZX Spectrum que o TZX tambem cobre) agora navega pela ordem real
  em vez de tocar os blocos em sequencia linear, do primeiro ao ultimo, ignorando saltos/lacos.
  Implementado como leitor em DUAS passadas: a 1a indexa onde cada bloco comeca (saltos/lacos se
  referem a blocos pelo NUMERO DE ORDEM, nao pelo deslocamento em bytes -- precisa saber os
  limites de TODOS os blocos antes de navegar); a 2a executa de verdade, com uma pilha de lacos e
  uma de chamadas (podem se aninhar entre si, so' lacos aninhados com lacos nao sao permitidos pela
  especificacao). "Selecao" (#28), sem como mostrar um menu de verdade numa ferramenta batch,
  escolhe sempre a 1a opcao por padrao. Protegido contra laco infinito (um "Salto 0", que a propria
  especificacao do TZX diz que "nunca deveria acontecer") com um limite generoso de passos.
- Testes: `tapetest` com 6 checagens novas (salto pula um bloco; laco repete o corpo 3 vezes; laco
  com 0 repeticoes pula o corpo inteiro; chamada com lista de 2 execucoes + retorno; selecao
  escolhe a 1a opcao; salto fora dos limites e' recusado com erro claro). Ver `doc/tape-spec.md`,
  secao 5 (limitacao removida).

## [1.22.0] - 2026-10-08 - "King's Valley: Ripper de Fita (WAV -> TSX)"

### Adicionado
- **`fwmsx --cas rip`**: "ripa" uma gravacao real de fita (`.wav` PCM mono, 8 ou 16 bits) para um
  `.TSX` valido -- detecta o piloto e decodifica os bytes do bloco #4B (Kansas City Standard),
  portando o CONCEITO do makeTSX (`resource/makeTSX/BlockRipper.cpp`/`rippers/MSX4B_Ripper.cpp`,
  MIT) de forma simplificada para o caso fixo do MSX (sem os modos interativo/preditivo do
  original). `--tolerancia N` (1-90%, padrao 25) ajusta a tolerancia de casamento dos pulsos;
  `--anexar <arquivo>` acrescenta numa fita existente, igual o `pack`.
- **`kcs_decode_byte()`** (`src/tape/core/kcs_codec.{h,c}`): o inverso de `kcs_emit_byte()` --
  decodifica um byte a partir de duracoes de pulso MEDIDAS (com tolerancia), usado pelo ripper.
  O bit de inicio precisa bater de verdade (sem tolerancia extra -- evita confundir o piloto do
  PROXIMO bloco com um fluxo infinito de bytes `0xFF`); os bits de fim sao aceitos mesmo fora da
  tolerancia, inclusive faltando pulso no fim do arquivo (um byte ja decidido pelos dados nao deve
  ser descartado so' por faltar o fim).
- **`src/tape/cpp/wav_reader.{h,cpp}`**: leitor de `.wav` PCM mono (8/16 bits), bounds-checked.
- Testes: `castooltest` (CTest `cas_pack`) ganhou um round-trip completo (bytes -> audio -> `rip`
  -> bytes, via um `.wav` sintetico gerado no proprio teste) e testes de erro (`.wav` estereo,
  silencio, argumentos invalidos). Testado manualmente tambem contra uma gravacao real de MSX dos
  anos 80 (so' como validacao local, nunca versionada): 86 blocos reconhecidos sem erro. Ver
  `doc/tape-spec.md`, secao 9.

## [1.21.0] - 2026-10-08 - "King's Valley: Empacotador de Fita (BIN/BAS -> TSX)"

### Adicionado
- **`fwmsx --cas pack`**: empacota um `.BIN`/`.BAS` solto (ja no formato binario/tokenizado do
  MSX) num `.TSX` (ou `.CAS`) valido, pronto para `BLOAD`/`CLOAD "CAS:"`, sem precisar abrir o
  emulador. `--tipo bin` monta o cabecalho de 6 bytes de endereco (inicio/fim/execucao,
  `--inicio`/`--fim`/`--exec` em hexadecimal) na frente do binario; `--tipo bas` empacota os bytes
  tokenizados como estao (um programa BASIC do MSX embute ponteiros de memoria entre linhas, por
  isso a ferramenta nao tokeniza texto solto -- so' empacota bytes que ja estao no formato certo,
  por exemplo extraidos com `BSAVE` dentro do proprio emulador). `--anexar <arquivo>` carrega uma
  fita existente e acrescenta o novo arquivo no final, em vez de criar uma fita so' com ele.
- **`fwmsx --cas list`**: lista indice, tipo, nome e tamanho dos dados de cada arquivo de uma
  fita (`.cas`/`.tsx`/`.tzx`) -- util para confirmar o resultado do `pack` sem abrir a janela
  "Fita K7".
- Reaproveita o MESMO escritor (`WriteTsxFromCas()`) que `TapeEngine` usa para gravar pela BIOS --
  os blocos gerados (cabecalho com nome + dados) sao exatamente os dois blocos que um
  `CSAVE`/`BSAVE` de verdade produziria. Primeiro item da lista de pendencias de fita (`doc/SPEC.md`,
  secao 5.2) a ser fechado.
- Testes: `castooltest` (CTest `cas_pack`) -- pack de `.bin`/`.bas`, `--anexar`, `list`, e
  validacao de argumentos, sem precisar de Z80/mapa de memoria (`CAS_PACK_SOURCES`, so' leitura/
  escrita de arquivo). Ver `doc/tape-spec.md`, secao 8.

## [1.20.3] - 2026-10-08 - "King's Valley: Gravacao em K7: Um Corte So'"

### Corrigido
- **"Sobrescrever o ponto marcado" apagava TUDO (o antigo e o novo)**: um CSAVE/BSAVE de verdade
  chama TAPOON/TAPOUT/TAPOOF DUAS vezes -- um bloco so' para o cabecalho com o nome do programa,
  outro so' para os dados -- e `OnTapoon()` reaplicava o corte ("sobrescrever o ponto marcado") em
  CADA chamada, nao so' na primeira. Na 2a chamada (bloco de dados), "o ponto marcado" passava a
  ser o PROPRIO cabecalho com nome que a 1a chamada tinha acabado de escrever -- apagando-o e
  deixando so' os dados, sem nome nenhum. Resultado: nem o programa antigo (de verdade apagado,
  como esperado) nem o novo (com o cabecalho destruido) ficavam reconheciveis -- "como se o
  programa tivesse sumido". "Nova fita" tinha o mesmo problema (limpava tudo de novo na 2a
  chamada). Corrigido: depois de cortar/limpar UMA VEZ, o modo volta sozinho para "incluir no
  final" -- qualquer bloco seguinte (do mesmo CSAVE ou de um CSAVE futuro) so' acrescenta, nunca
  corta de novo. Equivale ao comportamento fisico de uma fita de verdade: depois de cortar e
  comecar a gravar, o que vem a seguir so' pode ir para a frente. Relatado pelo usuario testando
  "sobrescrever o ponto marcado" pela janela de verdade; teste de regressao novo em `tapetest`
  simula os 2 blocos de um CSAVE real e confere que o nome e os dados do novo programa sobrevivem
  os dois.

## [1.20.2] - 2026-10-08 - "King's Valley: Gravacao em K7: Protecao e Preenchimento"

### Corrigido
- **Contagiros parado no CLOAD**: nao era bug -- o contador so' avanca enquanto o motor gira
  (`motor_on()`), e numa fita curta (poucos segundos) o avanco e' sutil. Confirmado via
  diagnostico direto (`Machine::RunFrame()` + `tape().odometer()`): sobe normalmente durante
  todo o CLOAD e congela quando o motor para (fim do carregamento).
- **"Device I/O error" ao trocar para "sobrescrever" ou "nova fita" apos marcar um ponto**:
  `TapeEngine::Insert()` sempre travava a fita de novo (`read_only_ = true`), MESMO ao reinserir
  a fita que o usuario ja' tinha destravado -- bastava reabrir o menu Fita para a protecao voltar
  silenciosamente. Corrigido: reinserir a MESMA fita preserva o estado de protecao atual. O menu
  tambem trocou o unico item ambiguo ("Fita protegida contra gravacao") por dois itens explicitos
  ("Destravar para gravar" / "Travar contra gravacao"), cada um habilitado so' quando faz sentido,
  deixando o estado atual sempre visivel.
- **Preenchimento de alinhamento (padding) gravado como dado**: `TAPOON` insere bytes zero antes
  de cada cabecalho novo so' para alinhar em 8 bytes (como a BIOS de verdade) -- esses bytes NUNCA
  foram conteudo real, mas os sintetizadores de pulso (`SynthesizeCasPulses`, o antigo
  `RebuildFromFastBytes` e `WriteTsxFromCas`) descobriam o tamanho de cada bloco "procurando o
  proximo cabecalho", o que misturava esse preenchimento com o bloco ANTERIOR de verdade. Corrigido
  com uma nova marca exata (`TapeMark{fast_byte_offset, pulse_index, content_length}`) que o motor
  de gravacao preenche com o tamanho REAL de cada bloco, sem adivinhar -- usada tanto na sintese de
  pulsos em memoria quanto na escrita do `.tsx` persistido. Bug relatado pelo usuario ao reinserir
  uma fita gravada e dar CLOAD no modo normal; confirmado tanto em memoria quanto reabrindo o
  arquivo do disco (teste novo em `tapetest`: grava dois blocos, recarrega do zero, confere que o
  segundo bloco tem EXATAMENTE os bytes gravados, sem contaminacao do preenchimento do primeiro).
  Limitacao que CONTINUA valendo para `.cas` carregado direto do disco (sem passar pela gravacao
  deste emulador): ali ainda nao ha' tamanho exato por bloco, so' seguro para fitas geradas por este
  fluxo -- ver doc/tape-spec.md.

## [1.20.1] - 2026-10-08 - "King's Valley: Gravacao em K7: Corrigindo o Piloto"

### Corrigido
- **Pulsos de ZERO e UM do bloco #4B estavam TROCADOS** (`src/tape/cpp/cas_format.h`): a convencao
  real do MSX e' zero = 2x o pulso de um, com o piloto na MESMA duracao do um -- confirmado contra o
  proprio gerador do makeTSX (`resource/makeTSX/rippers/MSX4B_Ripper.h/.cpp`, `bit0len = bit1len*2`).
  A nota antiga (ZERO=855, UM=1710) vinha dos defaults GENERICOS de ZX Spectrum (blocos #10/#11), nao
  do #4B do MSX, e nunca tinha sido conferida contra o codigo (como a propria nota avisava).
- **Piloto curto demais** para a BIOS de verdade calibrar (2000 pulsos, ~0.48s) -- aumentado para 8000
  pulsos (~1,9s), confirmado empiricamente contra o arquivo real do usuario e contra um .TSX comercial.
- Resultado pratico: uma fita GRAVADA por este emulador (1.20.0) carregava certo no modo rapido (nao
  usa pulso nenhum), mas o modo normal nunca achava o programa -- so' o barulho do piloto, sem
  calibrar. Relatado pelo usuario ao testar `CSAVE"TESTE"` + `CLOAD`. Testes de regressao em
  `tapetest` (3 novas checagens sobre a relacao zero/um/piloto).

## [1.20.0] - 2026-10-07 - "King's Valley: Gravacao em K7"

### Adicionado
- **Gravacao de fita** (`CSAVE`/`BSAVE "CAS:"`): TAPOON/TAPOUT/TAPOOF (00EAh/00EDh/00F0h) sempre pelo
  gancho de BIOS "ED FE" -- independente do modo de carregamento escolhido (nao ha' "modo normal" de
  gravar; precisaria de um "ripper" de pulsos em tempo real, fora de escopo).
- **Fita nova em branco** (`.tsx`, menu Fita > "Nova fita..."): cria uma imagem vazia e grava um .tsx
  valido no disco na hora, para o usuario salvar programas do zero.
- **Protecao contra gravacao**: uma fita inserida de um arquivo entra sempre travada (so' leitura); uma
  fita nova entra destravada. Travar/destravar pelo menu Fita, a qualquer momento.
- **Tres modos de gravacao** (fitas sao lineares, como as de verdade -- gravar a partir de um ponto
  destroi o que vinha depois): **incluir no final** (padrao, "ir enchendo a fita com programas
  pequenos"), **sobrescrever o ponto marcado**, ou **nova fita** (apaga tudo e comeca do zero).
- **Marcar um arquivo da fita**: a janela "Fita K7" agora lista os arquivos de forma clicavel -- marcar
  um deles decide de onde o proximo carregamento comeca a procurar, e onde a proxima gravacao (no modo
  "sobrescrever o ponto") trunca e escreve.
- **Contagiros**: um odometro simulado na janela "Fita K7", avancando conforme a fita "roda" -- a
  mesma sensacao dos gravadores de fita antigos.
- Escritor generico de .CAS para .TSX (`src/tape/cpp/tsx_writer.cpp`), usado pela gravacao e pela fita
  nova; diálogo "Salvar Como" generico na GUI (`ShowSaveFileDialog`, antes so' existia para `.dsk`).
- Testes: `tapetest` ganhou 22 checagens novas (gravacao round-trip, protecao, os 3 modos, marcar/
  desmarcar, contagiros) -- 68 no total.

### Limitacoes conhecidas
- A sintese de pulsos do modo normal (`SynthesizeCasPulses`) trata o preenchimento de alinhamento
  (zeros inseridos pelo proprio TAPOON antes de um cabecalho) como se fosse dado de verdade -- so'
  afeta a reproducao por PULSOS (modo normal) de uma fita GRAVADA por este emulador; o modo rapido nao
  e' afetado. Ver `doc/tape-spec.md`, secao 5.
- A gravacao/protecao/marcacao so' foi validada por teste automatizado; ninguem clicou nos novos itens
  de menu ainda.

## [1.19.1] - 2026-10-07 - "Yie Ar Kung-Fu: Fita K7: Corrigindo o Carregamento"

### Corrigido
- **Leitor de .TSX**: o bloco #35 (Custom info) usava 10 bytes para a string de identificacao em vez
  de 16 -- o `TZX_format.md` escreve o deslocamento do campo seguinte em HEXADECIMAL (`0x10` = 16, nao
  10). Qualquer .TSX real com esse bloco (makeTSX grava "TSX.RIPPER" nele) tinha a leitura inteira
  corrompida a partir dali. Confirmado com um arquivo `.tsx` real do usuario.
- **Reconstrucao do fluxo "rapido" a partir do .TSX**: faltava o preenchimento até o proximo multiplo
  de 8 bytes ANTES de cada cabecalho (igual ao TAPOON de verdade, que alinha ao gravar um .CAS) --
  sem isso, o SEGUNDO `BLOAD"CAS:"`/`CLOAD` de uma fita com mais de um arquivo desalinhava o TAPION e
  nunca mais achava cabecalho nenhum ("Device I/O error"; em modo normal, a BIOS ficava esperando um
  sinal que nunca chegava, travando a maquina). Testes de regressao em `tapetest` para os dois casos.

## [1.19.0] - 2026-10-07 - "Yie Ar Kung-Fu: Fita K7"

### Adicionado
- **Fita** (`.cas`, `.tsx` e `.tzx`): leitura completa do TZX 1.20 (bloco #4B, o unico que o MSX usa de
  verdade, com pulsos e dados completos; os demais blocos pulados com seguranca, sem travar o arquivo).
  Ver [doc/tape-spec.md](tape-spec.md).
- **Dois modos de carregamento**, trocaveis sem reiniciar a maquina: **rapido** (gancho de BIOS "ED FE"
  em TAPION/TAPIN/TAPIOF, sem som) e **normal** (pulsos de verdade na cadencia de T-states da CPU, com o
  barulho do gravador -- a BIOS roda a rotina ORIGINAL dela, sem patch nenhum).
- **Entrada de cassete no PSG** (R14, bit 7, conferido contra o openMSX) e **motor no PPI** (porta C,
  bit 4); audio sintetizado (onda quadrada) somado ao PSG/SCC/FM.
- `--fita <arquivo>` e `--fita-modo rapido|normal`; menu **Fita** da janela (inserir, ejetar, rebobinar,
  trocar de modo) e a janela visual **"Fita K7"** (dois rolos que giram enquanto o motor esta' ligado,
  barra de progresso e os arquivos encontrados na fita).
- Testes: `tapetest` (CTest `tape_load`, 39 checagens: cursor de pulsos, framing KCS, leitores de .CAS e
  .TSX, o gancho de BIOS de ponta a ponta com um Z80 de verdade, e a entrada de cassete no PSG).

### Limites conhecidos
- Sem escrita (`CSAVE`/`BSAVE "CAS:"`); sem banco de fitas nem download; blocos de controle do TZX
  (grupos, lacos, saltos) sao pulados mas nao navegados. Ver [doc/tape-spec.md](tape-spec.md), secao 5.

## [1.18.0] - 2026-10-06 - "Ys: Banco de ROMs e Disco por Portas"

### Adicionado
- **Banco de ROMs** (SQLite, `fwmsx --romdb`): ROMs do disco pelo SHA-1, com nome, categoria, hardware, mapper,
  notas e caminho. CRUD, busca, escanear pastas, adicionar arquivos. Ver [doc/romdb-spec.md](romdb-spec.md).
- **Downloads** pelo menu **ROMs** da janela e pela CLI: fMSX 6.0 para Windows (separado por tipo), System ROMs
  do file-hunter (o Full Set mais recente, ou navegar pelas pastas e baixar arquivos avulsos) e o banco do
  Vampier (dump SQL importado no SQLite). Hoje as ROMs baixadas ficam versionadas em `dist/roms/`, decisao
  provisoria do projeto pessoal (ver `LICENSE-THIRD-PARTY.md`).
- **Tabela de mappers** do `CARTS.SHA` do fMSX no banco.
- **Identificacao**: ROMs cadastradas recebem o nome do jogo pelo banco do Vampier (SHA-1).
- **Controladora de disco por portas** no estilo Microsol (DDX 3.0 e CDX-2): WD2793 em `D0h`..`D4h`, com a
  ROM do driver no slot. O MSX-DOS sobe com os dois drivers. Escolha no menu Maquina > Configuracao de disco
  ou com `--disk-acesso porta --disk-porta D0h --diskrom <driver>`.
- **Formatos de disco 180, 360 e 720 KB** pela configuracao do drive: 5 1/4 ou 3 1/2, face simples ou dupla,
  densidade simples ou dupla (ou automatico). `--disk-formato auto|ss525|ds525|ss35|ds35`.
- Opcao `--slot P:S=tipo[:arg]` para montar o layout pela linha de comando.
- Testes: `romdbtest` (CTest `romdb_store`); regressao de RAM em C000h (`memmaptest`, secao 25).
- **Estudo**: codigo do openMSX (GPL-2.0+), CLK e makeTSX (MIT) em `resource/`, provisorio, so' para
  consulta -- nenhum codigo copiado. Viabilidade de suporte a fitas TSX/TZX e CAS registrada em
  `doc/SPEC.md`, secao 5.2 (feature a desenvolver em breve, nao iniciada).

### Alterado
- **RAM 32KB** ocupa a celula escolhida (pagina 2) e a seguinte (pagina 3). **RAM 64KB** ocupa a celula inteira.
- **Mapper** de 64 KB a 4096 KB (potencias de 2); os segmentos 0 a 3 vao para as paginas 0 a 3 no reset. Pode haver mais de um mapper no layout.

### Corrigido
- **RAM de 16KB no slot**: ficava em 0000h da celula, e a BIOS nao subia (ex.: Gradiente Expert 1.1 com RAM em 3:x). Agora a RAM ocupa as ultimas paginas (C000h-FFFFh).
- **Tecla `&`**: era digitada como `^` (SHIFT+6); no MSX e' SHIFT+7.
- Nomes de arquivo com entidades HTML (`ROM&#39;s`) no indice do file-hunter.

## [1.17.0] - 2026-10-06 - "Xak: Musica FM e Slots"

### Adicionado
- **Chip FM (OPLL, YM2413)**: 9 canais melodicos com 2 operadores, envelope ADSR, feedback, tremolo,
  vibrato e os 15 timbres prontos. Portas `7Ch`/`7Dh` (MSX-MUSIC), sempre presentes como no fMSX.
  Entra no audio ao vivo junto com o PSG e o SCC. Ver [doc/fm-spec.md](fm-spec.md).
- **FM-PAC** (`--fmpac [arquivo]` ou menu Cartucho > FM-PAC): ROM de 16KB no slot 2:0, com a SRAM
  de 8KB liberada pela chave 694Dh e o arquivo `FMPAC.sav`.
- **Modo ritmo do OPLL** (`CALL MUSIC (1,...)`): BD, HH, SD, TOM e TC nos canais 7-9, com os tres
  timbres de bateria.
- **Comandos de BASIC do MSX-MUSIC** (`CALL MUSIC`, `PLAY #n`, `CALL VOICE`, `CALL PITCH`, `CALL AUDREG`,
  `CALL PLAY`) funcionam com `--fmpac`: a ROM do FM-PAC faz o trabalho, e a musica continua em segundo plano.
- `--text` (tela em texto no fim da execucao), `--fmstat` (estado do FM) e `--wav arquivo` (grava a mistura).
- **Layout de slots e configuracao da maquina** (menu Maquina > Configuracao de slots...): 16 celulas
  com BIOS, BASIC, RAM (16/32/64 KB), mapper (64 a 1024 KB), cartucho, sub-ROM do MSX2, disco e FM-PAC.
  Uma BIOS de 32KB ocupa as paginas 0 e 1 do slot escolhido; BIOS e BASIC podem ser dois arquivos de 16KB.
  Padrao igual ao de antes. Ver [doc/slots-spec.md](slots-spec.md).
- **FM-PAC ligado por padrao** quando o `FMPAC.ROM` existe; `--no-fmpac` e o menu Cartucho desligam.
- Testes: `fmtest` (CTest `fm_sound`, 24 checagens) e secao 24 da `memmaptest`; cinco checagens novas na `machinetest`.
- **SRAM de cartucho** nos mappers ASCII8 (8KB) e ASCII16 (2KB espelhada): o bit de SRAM do
  registrador de banco seleciona a memoria de bateria, que grava como no fMSX. Os registradores de
  banco continuam valendo com a SRAM selecionada.
- **Arquivo .sav** ao lado da ROM: lido ao carregar o cartucho e gravado quando ha alteracao (a cada
  300 quadros na janela, ao trocar cartucho ou modelo, ao fechar a janela e ao fim de `--frames`).
- Testes na `memmaptest` (secoes 19 e 20) para selecao, escrita, espelhamento, persistencia e tamanho errado.

- **SRAM de cartucho** tambem para o FM-PAC (`FMPAC.sav`), ver [doc/sram-spec.md](sram-spec.md).

### Alterado
- **FM-PAC ligado por padrao** quando o `FMPAC.ROM` existe (antes era preciso `--fmpac`). `--no-fmpac`
  e o menu Cartucho desligam.
- **Portas 7Ch/7Dh do FM sempre presentes**, como no fMSX.
- **A maquina monta um layout de 16 celulas** (slot:subslot) em vez de posicoes fixas. O padrao e' o
  mesmo de antes; trocar de modelo volta o layout ao padrao (cartucho e FM-PAC sao mantidos).
- **Mistura de audio**: PSG, SCC e FM somados, com o FM em metade do ganho.
- **Versao 1.17.0**: pacote Windows (`fwMSX-1.17.0.zip`) e Linux (`fwMSX-1.17.0-linux.tar.gz`), com o
  manual e as notas desta versao.

### Corrigido
- **`--keys` e teclado do MSX**: `(`, `)`, `*` e `"` eram digitados com outras teclas. Agora seguem o
  layout do MSX: SHIFT+9 = `(`, SHIFT+0 = `)`, SHIFT+8 = `*`, SHIFT+' = `"`.
- **Erro do FM-PAC no menu**: a mensagem aparece no proprio menu Cartucho.

### Limites conhecidos
- **Som do FM nao validado por ouvido**: o WAV gerado (`--wav`) tem sinal, mas nao foi comparado com um
  MSX-MUSIC real. Timbres e niveis sao estimativas.
- **`CALL VOICECOPY` nao e' aceito** pela ROM do FM-PAC do fMSX ("Syntax error").
- **Status e timers do OPLL** nao sao emulados (`7Ch` le 0).
- **Jogos**: Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada, causa nao
  diagnosticada; Mega Chase so' validado ate o titulo; F-1 Spirit 3D: troca de disco pela janela nao testada.
- **Janela**: menus, tela cheia, 4:3, 16:9 e filtros de video nao foram conferidos na tela (a automacao
  nao opera a janela).
- **Cores YJK** do V9958 nao conferidas com hardware real.
- **Layout de slots** so' pelo menu (sem opcao de linha de comando); BIOS so' em 0:0; GameMaster2 e
  save-state ausentes; efeitos de rastreio no meio do quadro ausentes.

## [1.16.0] - 2026-10-05 - "Aleste: Janela e Video"

Janela com menus do fMSX, zoom, proporcao, tela cheia e filtros de video.

### Adicionado
- **Quadro com borda, como o fMSX**: 272x228 no MSX1 e 544x228 no MSX2; borda de 18 linhas (8 em 212 linhas) e cor de fundo. As teclas de funcao e o texto do BASIC deixam de encostar na borda.
- **Menus do fMSX na janela**: Arquivo, Maquina (Modelo MSX1/MSX2/MSX2+, Reiniciar, Pausar), Exibir, Video, Som, Disco, Cartucho, Joystick, Ferramentas, Configuracoes e Ajuda. Os itens sem implementacao aparecem desabilitados com "(em breve)".
- **Trocar modelo ou cartucho** recria a maquina (os discos montados entram de novo).
- **Configuracoes -> Interface**: tema escuro ou claro, tamanho da letra (Normal, Grande, Muito grande; padrao Grande) e borda e sombra da tela.
- **Exibir -> Janela**: zoom 2x, 3x, 4x e 6x (padrao 3x), com a janela redimensionada. Janela inicial limitada a area util do monitor.
- **Exibir -> Proporcao**: Original (pixels), 4:3 corrigido e 16:9 esticado.
- **Tela cheia com menu oculto**: a barra aparece com o mouse no topo ou com um submenu aberto.
- **Video -> Interpolate Video**: Nearest Neighbor, Linear Scaling, EPX Scale 2x, Eagle Algorithm, Scale 2x Algorithm e 2xSal Algorithm (2xSal e uma aproximacao).
- **Video -> Scanlines**: TV, LCD e LCD Raster.
- **Video -> Color Filter**: Monochrome, Sepia, Green CRT, Amber CRT, CMY Raster e RGB Raster.
- Dialogo de abertura de arquivo generico (Windows), usado para cartuchos.
- Testes: `video_filters` (filtros de video) e a geometria do quadro em `machine_test` e `msx2_test`.

### Corrigido
- Janela inicial centralizada no topo da area util, para a barra de titulo nao ficar fora da tela.

### Pendente (proximas versoes)
- Itens "(em breve)" do menu: salvar estado, memoria, PAL, trapacas, POKE, DiskROM, fita, MIDI, gravacao de som, novo disco, slot 2, dispositivos de entrada e mostrar sprites.
- Validacao visual dos menus, tela cheia, 4:3, 16:9 e dos filtros de video (a automacao nao consegue operar a janela).

## [1.15.0] - 2026-10-05 - "Metal Gear 2: Cores YJK"

**Agora e' um MSX2+**: `fwmsx --msx --msx2p` roda a BIOS MSX2+ real (MSX BASIC 3.0) com o VDP
V9958. Ver [doc/msx2p-spec.md](msx2p-spec.md).

### Adicionado
- **Modelo MSX2+** (`--msx2p`): BIOS `MSX2P.ROM`, sub-ROM `MSX2PEXT.ROM`, RAM de 128KB com mapper e
  RTC, como o `--msx2`, com o VDP V9958. Bit 2 de S#1 ligado, como o fMSX.
- **SCREEN 10, 11 e 12** (YJK/YAE) em `vdp_render.c`, a partir do `RefreshLine10()`/`RefreshLine12()`
  e do `YJKColor()` do fMSX, com as correcoes descritas em `doc/msx2p-spec.md`. O modo e' ligado por
  R#25 (bit 3 YJK, bit 4 YAE) em scr 7/8.
- **Scroll horizontal** (R#26/R#27, 9 bits) em SCREEN 5-8, YJK e YAE, com HScroll512 (R#25 bit 0).
- **Mascara da esquerda** (R#25 bit 1): 8 pixels (16 em modo de 512) na cor de fundo.
- Teste novo no `vdp2test` (secao 9): verificacoes de YJK, scroll, mascara, YAE e um diferencial
  SCREEN 5-8 contra o renderizador da V9938.

### Corrigido
- `--keys`: os simbolos com SHIFT do layout MSX estavam trocados. `(` e `)` saiam como `)` e `-`,
  e `&`, `"` e `*` tambem estavam errados. Agora seguem o layout MSX: `SHIFT+8` = `(`, `SHIFT+9` =
  `)`, `SHIFT+6` = `&`, `SHIFT+2` = `"`, `SHIFT+'` = `*`.

### Verificado
- BIOS MSX2+ sobe com `MSX BASIC version 3.0`.
- Programa BASIC `screen 12` / `line (10,10)-(200,100),5,bf` escreve R#25 = `08h` e desenha em YJK;
  `screen 10` escreve R#25 = `18h`. Na MSX2 comum o mesmo programa nao chega a trocar para o SCREEN 12.
- `vdp2test` com as verificacoes novas do V9958 (secao 9) e suite completa 12/12.

### Limitacoes conhecidas
- Cores YJK conferidas contra o openMSX (regra do azul de um turbo R), e nao contra um V9958 real.
- Atributo de SCREEN 10 e 11 nao e' distinguido (como no fMSX).
- SCREEN 9 nao existe no MSX2+ (so' no MSX2 coreano); nao foi implementado.

---

## [1.14.0] - 2026-10-05 - "Hydlide: Busca Rapida"

**CPIR e CPDR ganham caminho rapido em Assembly**, no mesmo molde do LDIR/LDDR. Ver
[doc/z80-core-spec.md](z80-core-spec.md), secao 6.

### Adicionado
- `z80_fast_block_search()` (`src/z80/asm/block_ops.asm`, dual-ABI Win64/SysV): a busca do byte
  em `REPNE SCASB`, para frente (CPIR) ou para tras (CPDR).
- Caminho rapido de CPIR/CPDR em `opcodes_ed.h`: so' a busca vai para o Assembly. Contagem, HL/BC,
  PC, ciclos e flags saem das mesmas expressoes do loop lento, a partir do ultimo byte examinado.
- Teste diferencial CPIR/CPDR em `tests/z80/smoke_test.cpp` (teste 6): 12 casos fixos e 120 aleatorios,
  com casamento e sem casamento, orcamento grande e pequeno, nos dois sentidos. Comparam registradores,
  flags e a RAM inteira contra o loop lento.

### Verificado
- Teste de sensibilidade: quebrar o tratamento de casamento no caminho rapido derruba 26 checks.
- `z80test`: 300 checks passam no Windows e no Linux (SysV).
- Suite completa: 12/12 suites nas duas plataformas.

## [1.13.0] - 2026-10-05 - "F1 Spirit: Som do SCC"

**O SCC toca**: cartuchos Konami5 e Gen8 que ligam o chip de som SCC agora tocam a trilha.
O F1 Spirit programa os cinco canais de verdade. Ver [doc/scc-spec.md](scc-spec.md).

### Adicionado
- **Chip de som SCC** (`src/scc/`): motor em **C** adaptado do `EMULib/SCC.c` do fMSX, gerando
  amostras PCM (como o PSG); soma de canal em **Assembly** dual-ABI (`render_channel.asm`); tabela
  de volume linear em **Fortran**; `SccDevice` em **C++** ligado ao slot de cartucho.
- Protocolo do cartucho: `3Fh` em `9000h` (Konami5) ou em `8000h-9FFFh` (Gen8) liga o chip, e a
  faixa `9800h-98FFh` vai para ele enquanto estiver ligado. A escrita nessa faixa nao chega ao mapper.
- `memmap::SlotCartIo` / `SlotMemoryBus::AttachCart()`: o barramento entrega leituras e escritas do
  slot de cartucho ao dispositivo antes do mapper (o FDC usa o `SlotMmio`, de outra natureza).
- Saida de audio ao vivo soma PSG e SCC, com saturacao em 16 bits (`Machine::TakeLiveAudio`).
- Teste novo `scctest` (CTest `scc_sound`): 18 verificacoes, inclusive o teste diferencial do
  kernel de Assembly contra a referencia em C (2000 casos aleatorios).

### Verificado
- F1 Spirit (Konami5, 128KB): apos 4000 quadros, o jogo liga os cinco canais (mixer `1Fh`) e o
  estado renderiza 44085 amostras nao-nulas por segundo, pico 21874.
- Lode Runner + Konami SCC continua sem subir o jogo (ver `doc/scc-spec.md`, secao 5): nenhum mapper
  chega a ligar o SCC nesse cartucho. O problema parece ser o boot do cartucho, nao o chip; nao
  investigado a fundo nesta versao.

## [1.12.0] - 2026-10-05 - "Metal Gear: Entrada Direta"

Versao pequena: fecha uma decisao de comportamento registrada desde a v1.2.0.

### Alterado
- **`fwMSX.exe` sem argumento nenhum agora abre a maquina completa em janela**, com os mesmos
  padroes de `--msx` (BIOS MSX1 ao lado do executavel, sem cartucho/disco). Antes, sem
  argumentos, imprimia o esqueleto dos quatro modulos (C++/C/Assembly/Fortran) -- decisao
  registrada em `doc/SPEC.md`, secao 5.1 ("sem argumentos deve abrir em GUI por padrao, uma
  vez que exista emulacao de verdade"), adiada ate agora por nao fazer sentido antes do core
  existir. O esqueleto continua alcancavel com argumentos explicitos que nao batem com nenhum
  modo conhecido (ex.: `fwMSX.exe NomeDoProduto 1 2 3`), preservado por ser o historico do
  projeto.

### Corrigido
- `build.ps1` para no primeiro erro do CMake (configuracao ou compilacao). Antes imprimia
  "Pronto" e empacotava o zip mesmo com a compilacao falhando.

## [1.11.1] - 2026-10-02 - "Firebird: Arrumando a Casa"

Versao pequena de acabamento sobre a 1.11.0.

### Adicionado
- **`--disk-ro`**: discos somente leitura. O MSX-DOS le normalmente e responde `Write protect
  error writing drive A` ao tentar gravar; o arquivo `.dsk` nunca e' alterado. Vale tambem
  para discos inseridos depois, pelo menu **Disco**. Era o maior risco de perda de dados da
  v1.10.0 (as gravacoes vao direto para o arquivo). Teste novo em `machinetest`.

### Corrigido (documentacao e comentarios)
- Textos que ficaram para tras: o `MANUAL.md` ainda dizia "nao ha' janela grafica", o `README`
  "sem janela ainda", o `psg-spec` listava audio ao vivo e joystick como pendentes, o
  `ppi-spec` listava PSG/janela/cartuchos como proximos passos, e dois `TODO(FDC)` em
  `slot_state.c/.h` apontavam para um controlador que ja' existe (mora em `SlotMmio`).

- 800 verificacoes automatizadas (11 suites, +5 da `--disk-ro`).

### Verificado
- Jogos MSX1 rodando na maquina MSX2: o King's Valley mostra o mesmo logo "Konami Software"
  sobre azul que o fMSXgo de referencia.

## [1.11.0] - 2026-10-02 - "Firebird: MSX2 em Cena"

**O fwMSX agora e' tambem um MSX2**: `fwmsx --msx --msx2` roda o MSX BASIC 2.1 e jogos
MSX2 de verdade, como o Firebird. Ver [doc/msx2-spec.md](msx2-spec.md).

### Adicionado
- **MSX2** (`fwmsx --msx --msx2`) -- ver [doc/msx2-spec.md](msx2-spec.md). A BIOS MSX2 real
  roda ate' o **MSX BASIC 2.1** (identico ao fMSXgo de referencia), e o **Firebird**
  (jogo MSX2) joga: logos, titulo, floresta rolando e sprites coloridos.
  - **VDP V9938:** VRAM de 128KB com paginas (R#14); **SCREEN 3, 4, 5, 6, 7, 8 e TEXT80**;
    212 linhas; cor 0 transparente; tela desligada (R#1 bit 6); **sprites de modo 2**
    (cor por linha, bits CC/EC, 8 por linha, 9o sprite, colisao).
  - **Motor de comandos** (`vdp_cmd.c`, adaptado de `V9938.c`): POINT, PSET, SRCH, LINE,
    LMMV, LMMM, LMCM, LMMC, HMMV, HMMM, YMMM, HMMC, com operacoes logicas, temporizacao
    por scanline e handshake TR. `LINE`/`CIRCLE`/`PAINT` do BASIC desenham de verdade.
  - **Maquina:** RAM de 128KB com **mapper** (portas `FCh`-`FFh`), **RTC** RP5C01 com CMOS
    (portas `B4h`/`B5h`), sub-ROM `MSX2EXT.ROM` em 3:1 dividindo o slot com a DISK.ROM
    (o MSX-DOS 1.8 boota no MSX2 e nem pergunta a data). A imagem do MSX2 e' sempre de 512
    pixels de largura (modos de 256 saem dobrados) e a janela recria a textura ao mudar de modo.
  - `--keys` aceita pontuacao com SHIFT (`( ) : $ " ...`); `--vdplog` mostra as mudancas
    dos registradores do VDP por quadro.
- Novos alvos de teste `vdp2test` (VDP isolado, cada modo e cada comando) e `msx2test`
  (mapper, RTC e a maquina com a BIOS MSX2 real, inclusive o BASIC desenhando).
- 795 verificacoes automatizadas (11 suites).

### Corrigido
- **SCREEN 3 (multicolor)** do MSX1 agora e' desenhado (caia no fundo liso).
- A **cor 0 transparente** (mostra a cor de fundo) e a **tela desligada** (R#1 bit 6) agora
  valem em todos os modos; antes o renderizador as ignorava.
- `R#10`/`R#11`/`R#6` guardam so' os bits que existem (como o `VDPOut()` do fMSX).
- Desvio deliberado do fMSX: o bit TR do motor de comandos comeca limpo a cada comando (no
  fMSX o LMCM seguinte a um HMMC devolvia um pixel velho).

## [1.10.0] - 2026-10-02 - "Golvellius: MSX-DOS e Joystick"

**O MSX-DOS 1.8 boota, e os jogos de cartucho ganham joystick e deteccao de
mapper.** Verificado com jogos e discos reais -- ver
[doc/fdc-spec.md](fdc-spec.md) e [doc/machine-spec.md](machine-spec.md), secao 4b.

### Adicionado
- **Disco: controladora WD2793 + DiskROM -- o MSX-DOS 1.8 boota.** `fwmsx --msx
  --disk msxdos1.dsk` roda o **MSX-DOS** de verdade: a `DISK.ROM` no slot 3:1
  fala com um WD2793 mapeado em `7FF8h-7FFFh` (motor em C adaptado do
  `WD1793.c` do fMSX, `.dsk` cru, gravacao direta no arquivo). `dir` e `copy`
  funcionam, e o arquivo copiado sai identico ao original. Novo mecanismo
  `SlotMmio` no barramento de slots; menu **Disco** na janela (inserir/ejetar
  A:/B:); opcoes `--disk`, `--diskb`, `--disk-interface`, `--diskrom`, `--wait`.
  Ver [doc/fdc-spec.md](fdc-spec.md). Novo alvo `fdctest`.
- **Joystick** (R14/R15 do PSG): `psg_set_joystick()`, leitura de R14 com a
  selecao de porta (bit 6 de R15) e o corte das linhas (bits 4/5), como o
  `InZ80()` 0xA2 do fMSX. Na janela: **setas + Z/Espaco (fogo A) + X (fogo B)**
  na porta A e **gamepads do GLFW** (1o -> porta A, 2o -> porta B), com menu
  **Joystick**. Um toque mais curto que um quadro ainda vale 1 quadro. Reset
  de maquina nao solta o joystick.
- **Deteccao automatica de mapper** (`memmap::GuessMapper`, o `MAP_GUESS` do
  fMSX, so' a heuristica de `LD (nnnn),A`): `--cart <arq>` com mais de 32KB
  detecta Konami4/Konami5/ASCII8/ASCII16/Gen8 sozinho (`--cart <arq> auto` ou
  um nome de mapper para forcar). O CLI imprime o que detectou.
- Cartucho de ROM plana de ate' 16KB cujo cabecalho `AB` aponta para
  8000h+ vai para a pagina 2.

### Corrigido
- **Estado inicial de MegaROM**: os bancos 0,1,2,3 aparecem em 4000h/6000h/
  8000h/A000h (como `SetMegaROM(J,0,1,2,3)` do fMSX), nao todos no banco 0.
  Varios jogos chamam rotinas em 6000h-7FFFh antes de trocar qualquer banco.

- 659 verificacoes automatizadas (9 suites).

### Verificado com jogos reais (fora do repositorio)
- **King's Valley** (16KB): roda, tela de titulo e entrada pelo teclado.
- **F1 Spirit** (128KB, Konami5 detectado): roda ate' o menu do jogo.
- **Firebird / Hi no Tori** (128KB, Konami4 detectado): **e' um jogo MSX2**
  (titulo em SCREEN 5) -- na maquina MSX1 ele nao chega a lugar nenhum, e e'
  o que se esperava: confirmado rodando o mesmo jogo num emulador de
  referencia (fMSXgo, em Go) em modo MSX1 (tambem trava) e em modo MSX2 (roda).
  Nao era bug do nucleo; ele passa a funcionar com o VDP MSX2 (Fase 4).
- **Lode Runner + Konami SCC**: cai no BASIC ("Illegal function call in 10") --
  e' uma ROM que espera disco (FDC), que ainda nao existe.

## [1.9.0] - 2026-10-02 - "Gradius 2: Janela e Som"

**O fwMSX agora e' uma maquina que da' para usar**: `fwmsx --msx` abre uma
janela com a BIOS real rodando em tempo real, teclado do host e som. Ver
[doc/machine-spec.md](machine-spec.md), [doc/psg-spec.md](psg-spec.md) e
[doc/audio-spec.md](audio-spec.md).

### Adicionado
- **`fwmsx --msx`: a maquina MSX1 completa numa janela, com teclado do
  host** -- ver [doc/machine-spec.md](machine-spec.md). `machine::Machine`
  (`src/machine/`) junta BIOS + slots + VDP + PPI + PSG + Z80 e avanca por
  quadros (59736 ciclos, 59.92 Hz) sem depender de janela; a janela
  (Dear ImGui + GLFW + OpenGL) roda em tempo real, com menu (Reset, Pausar,
  tela cheia com F11) e mapeamento posicional do teclado do host para a
  matriz do MSX. `--cart <arq> [mapper]` carrega um cartucho no slot 1
  (ROM plana ou MegaROM); `--frames N --shot arq.ppm [--keys "texto"]` roda
  sem janela e salva a tela.
- **Audio ao vivo**: `fwmsx --msx` toca o PSG pelo dispositivo de audio
  padrao (miniaudio, um header de dominio publico baixado por FetchContent),
  via buffer circular sem trava e `PsgDevice::EnableLive()/TakeLive()`.
  Menu **Som** (mudo + volume), `--mute`, `-DFWMSX_AUDIO=OFF` para um stub.
  Ver [doc/audio-spec.md](audio-spec.md). Novo alvo `audiotest` (inclui o
  dispositivo real: ~44100 amostras/s, 0 underruns em 1 s).
- 570 verificacoes automatizadas (8 suites).
- Novo alvo de teste `machinetest` (boot em quadros, RenderFrame, digitar
  `print 1234`, Reset, cartucho com INIT).
- **PSG AY-3-8910 (portas `A0h`-`A2h`)** -- ver [doc/psg-spec.md](psg-spec.md).
  Motor em C (`src/psg/core/psg_state.{h,c}`) que **gera amostras PCM de
  verdade** (3 tons, ruido LFSR de 17 bits, 16 formas de envelope) avancadas
  por ciclos de Z80; tabela de volume logaritmica em Fortran
  (`volume_table.f90`); `psg::PsgDevice` e gravador de WAV em C++.
- **Depurador:** flag `--psg` (exige `--slots`) e comandos `psgregs`,
  `psgpoke`, `psgrec start|stop|clear|save <arq.wav>`; `reset` tambem
  reseta o PSG.
- Novo alvo de teste `psgtest`. Com a BIOS real: `GICINI` programa o mixer
  (R7=`B8h`) e `BEEP` no BASIC toca o canal A em 1316 Hz, volume 7.

### Notas
- Sem joystick em R14/R15 (R14 le `7Fh`). A duracao do BEEP gravado nao foi conferida
  contra um MSX real.

## [1.8.0] - 2026-10-01 - "Zanac: Prompt do BASIC"

**A BIOS MSX1 real sobe ate o prompt do MSX BASIC** e o teclado do PPI
chega ate' ela -- primeira vez que o fwMSX roda software MSX de verdade.
Ver [doc/ppi-spec.md](ppi-spec.md), seção 5.

### Corrigido
- **`Z80Cpu` agora chama `z80_reset()` na construção.** Antes, quem não
  digitasse `reset` rodava com as tabelas de flag Sinal/Zero/Paridade
  **zeradas** (elas são preenchidas por `z80_tables_init()`, dentro de
  `z80_reset()`): `AND`/`OR`/`XOR`/`CP`/`INC`/`DEC` davam flags erradas
  (`AND A` com A=0 devolvia `F=10h` em vez de `54h`). Era a causa real de
  a BIOS ficar "presa" desde a v1.4.0 -- o diagnóstico anterior ("falta
  PPI", depois "falta RAM/subslot") estava errado/incompleto. Os 168
  testes do `z80test` não pegaram porque todos chamam `cpu.reset()` antes.
  Teste de regressão novo em `debug_session_test.cpp`.

### Adicionado
- Teste de aceite com a BIOS real (`ppitest`, antes informativo): 100M de
  ciclos, VBlank habilitado, a abertura "MSX BASIC version 1.0 ... Bytes
  free" na VRAM e `keydown z` aparecendo na tela.
- 470 verificações automatizadas (5 suítes).

### Notas
- Com BIOS + `--ppi`: RAM de 64KB em `3:2` (necessária). As regras de
  subslot do MSX1 foram testadas -- a BIOS sobe com e sem elas; mantidas
  por fidelidade ao fMSX.

## [1.7.0] - 2026-10-01 - "Vampire Killer: Teclado e PPI"

PPI i8255 + matriz de teclado (portas `A8h`-`ABh`) -- ver
[doc/ppi-spec.md](ppi-spec.md).

### Adicionado
- **Chip i8255 em C** (`src/ppi/core/ppi_state.{h,c}`), adaptado de
  `EMULib/I8255.c` e das portas `A8h`-`ABh` de `MSX.c`: modos das portas,
  set/reset de bit da porta C, pinos de saída só dirigidos em modo saída,
  e a matriz de teclado de 11 linhas lida por `A9h`.
- **`ppi::PpiDevice` (C++)**: o slot primário passa a mudar quando o pino
  de saída da porta A muda (como `PSlot(PPI.Rout[0])` no fMSX). Antes de a
  BIOS escrever `82h` em `ABh`, `OUT (A8h)` não troca slot -- fiel ao
  hardware.
- **Tabela de posição das 87 teclas em Fortran**
  (`src/ppi/fortran/key_matrix.f90`) e **contagem de teclas pressionadas
  em Assembly** dual-ABI Win64/SysV (`src/ppi/asm/key_count.asm`).
- **Depurador:** flag `--ppi` (exige `--slots`) e comandos `ppiregs`,
  `keys`, `keydown <tecla>...`, `keyup <tecla>...|all`; `reset` também
  reseta o PPI (teclas pressionadas continuam).
- **Layout MSX1 com BIOS + `--ppi`**: RAM de 64KB no slot `3:2` e regras
  de subslot do MSX1 (`SlotState.msx1_subslot_rules`, `SSlot()` do fMSX:
  slots 0/1/2 sem subslot). Sem `--ppi` nada muda.
- Novo alvo de teste `ppitest` (62 verificações; 464 no total, 5 suítes).

### Achado
- A BIOS real agora programa e lê o PPI e testa a expansão de slots, mas
  **ainda não sai da varredura de RAM** (`0x0305`-`0x0331`). O
  diagnóstico anterior ("só falta o PPI") estava incompleto -- também
  faltavam RAM e as regras de subslot. Próximo passo registrado em
  `doc/ppi-spec.md`, seção 7.

## [1.6.0] - 2026-10-01 - "Penguin Adventure: Sprites em Cena"

Fase 3 do VDP -- sprites de "modo 1" do TMS9918 (SCREEN 1/2/3). Ver
[doc/vdp-spec.md](vdp-spec.md), secao 6, Fase 3.

### Adicionado
- **Sprites em C** (`src/vdp/core/vdp_sprites.{h,c}`), adaptados de
  `Sprites()`/`CheckSprites()` do fMSX: 8x8 e 16x16, ampliação 2x,
  prioridade por índice, cor 0 transparente, early clock, Y negativo,
  terminador Y=208, desligamento por R#8 bit 1.
- **Limite de 4 sprites por linha** com flag/número do "quinto sprite"
  em S#0, e **flag de colisão** (S#0 bit 5) sinalizada na linha 192 por
  `vdp_step_scanline()`; ler S#0 pela porta `99h` devolve e limpa.
- `vdp_render_line()` desenha os sprites por cima do fundo em SCREEN 1/2
  (o `vdpshot` já os mostra).
- 20 novas verificações em `vdptest` (402 no total, 4 suítes).

### Corrigido
- Build Linux: sem `wayland-scanner` o configure do GLFW 3.4 falhava;
  agora o `CMakeLists.txt` desliga o backend Wayland nesse caso e usa só
  X11 (via XWayland em sessões Wayland). Pré-requisitos documentados no
  cabeçalho do `build.sh`.
- Pacote Linux `dist/fwMSX-1.6.0-linux.tar.gz` incluído.

### Notas
- Sem sprites de modo 2 (SCREEN 4-8) -- Fase 4, com os modos MSX2.
- Quirk do fMSX preservado: VScroll (R#23) somado duas vezes ao Y dos
  sprites em SCREEN 1 (irrelevante com R#23=0).

## [1.5.0] - 2026-09-30 - "Antarctic Adventure: Primeiros Pixels"

Primeira vez que o projeto desenha pixels de verdade -- o VDP
(TMS9918/V9938), Fases 0.5, 1 e 2 -- ver
[doc/vdp-spec.md](vdp-spec.md) para o histórico completo de fases.

### Adicionado
- **`CompositeBus`** (`src/z80/cpp/composite_bus.h`, design próprio):
  multiplexa I/O de porta entre vários dispositivos (mapa de memória em
  `A8h`, VDP em `98h`-`9Bh`), enquanto leitura/escrita de memória vai
  sempre para um único dispositivo designado.
- **Motor "digital" do VDP em C** (`src/vdp/core/`), adaptado de
  `resource/fMSX/fMSX/MSX.c`: registradores (64), status (16), VRAM,
  protocolo das 4 portas (`98h` dados, `99h` latch de endereço/
  registrador, `9Ah` latch de paleta, `9Bh` acesso indireto), cache de
  ponteiro de tabela (`ChrTab`/`ColTab`/`ChrGen`/etc., mesmo padrão já
  validado no Z80 e no mapa de memória), e a máquina de estados de
  scanline que gera as interrupções de VBlank (`IE0`)/HBlank-
  coincidência (`IE1`). Preservado um quirk real do fMSX: o latch de
  endereço (`VKey`) não é resetado numa leitura de status, só numa
  leitura/escrita de dados -- o comentário original diz que isso
  "quebra Sir Lancelot no ColecoVision" se mudado.
- **Finalmente dá uso real ao `Z80Cpu::interrupt()`**, que existe desde
  a Fase 1 do núcleo Z80 mas nunca tinha sido exercitado contra nada --
  confirmado com um programa sintético de 12 bytes recebendo a
  interrupção de VBlank em `0x0038` de verdade (`IFF1` desligando,
  não só coincidência de PC/SP).
- **Renderização real de pixel** (`src/vdp/core/vdp_render.*`),
  adaptada de `resource/fMSX/fMSX/Common.h` (`RefreshLine0/1/2`): SCREEN
  0 (texto 40×24 mono, 240×192), SCREEN 1 (texto 32×24 com cor
  compartilhada por grupo de 8 caracteres -- quirk real do "Graphics 1"
  da TMS9918), SCREEN 2 (bitmap 256×192 com tabela de cor/padrão
  dividida em terços). Sem borda, sprites ou tela ligada/desligada
  ainda.
- **Tabela de paleta de 512 cores em Fortran**, construída na Fase 1
  sem consumidor, finalmente usada pela escrita de paleta via porta
  `9Ah` na Fase 2.
- **`vdpshot <arquivo.ppm> [linha_inicial] [linha_final]`**: exporta o
  frame atual como imagem PPM binária (`P6`), sem depender de biblioteca
  de imagem externa -- testável byte a byte, visualizável em qualquer
  leitor de PPM.
- Comandos de depuração `vdpregs`/`vdpmem`/`vdppeek`/`vdppoke`/
  `vdpstep` em `--z80dbg --slots --vdp` (requer `--slots`; avisa e
  ignora `--vdp` sem ele).
- 54 verificações automatizadas novas (382 no total, `ctest`, 4 suítes):
  protocolo de porta, cache de tabela por modo de tela, paleta padrão
  (as 16 entradas), os três modos de renderização com posições de pixel
  específicas conferidas à mão, PPM round-trip byte a byte, e o teste
  de aceitação com o programa sintético de interrupção.

### Corrigido
- `vdp_reset()` zerava a paleta em vez de carregar a paleta padrão do
  TMS9918 (`PalInit[16]` do fMSX) -- sem essa correção, toda cor FG/BG
  cairia em preto, já que software MSX1 normal nunca escreve os 16
  registradores de paleta na mão (espera a paleta padrão desde o ligar).

### Achado, não corrigido nesta versão
- A BIOS MSX1 real ainda não chega a habilitar a interrupção de VBlank
  dentro de nenhum orçamento de ciclos testado -- ela poliniza hardware
  de teclado/PPI (portas `A9h`-`ABh`) que ainda não existe no projeto.
  Não é um bug do VDP; é a próxima peça que desbloquearia testes bem
  mais realistas contra a BIOS (ver `doc/SPEC.md`, seção 5.0).

## [1.4.1] - 2026-09-30 - "Salamander: Compilando em Linux"

Primeira validação real (build + link + execução, não só montagem) do
projeto numa máquina Linux (WSL2) -- ver [doc/SPEC.md](SPEC.md), seção
5.0. Sem features novas; dois bugs reais de portabilidade corrigidos em
código Assembly que nunca tinha sido testado fora do Windows.

### Corrigido
- **`src/asm/init_asm.asm`** (módulo Assembly da Fase 0 do projeto,
  desde 2026-09-28) era Win64-only -- montava sem erro para `elf64`
  (NASM não valida convenção de chamada), mas rodando de verdade no
  Linux os argumentos chegariam nos registradores errados (SysV entrega
  major/minor/patch em EDI/ESI/EDX, não ECX/EDX/R8D da Win64), e o
  *link* falhava (`relocation ... can not be used when making a PIE
  object`) por causa de uma chamada direta a `printf` incompatível com
  executável PIE (padrão em distros Linux modernas). Corrigido com a
  mesma técnica `%ifidn __OUTPUT_FORMAT__` do núcleo Z80
  (`src/z80/asm/block_ops.asm`) e `call printf wrt ..plt` (chamada
  PLT-relativa, funciona com ou sem PIE).
- **`src/msxdisk/asm/name_match.asm`** (msxdisk, Win64-only desde a
  Fase 2 dele) tinha o mesmo problema de registradores errados no
  Linux -- mais sutil que o de cima, porque não chama nenhuma função
  externa: o build **não falhava**, só o resultado ficaria errado em
  tempo de execução (`list`/`extract` com padrão de coringa). Corrigido
  com a mesma técnica de troca de registrador de entrada por ABI.
- `src/msxdisk/gui/file_dialog.cpp` incluía `<windows.h>` sem nenhuma
  guarda de plataforma, quebrando a compilação inteira fora do Windows.
  Guardado atrás de `#ifdef _WIN32`; noutras plataformas os diálogos
  nativos de arquivo devolvem "cancelado" por enquanto (um seletor
  nativo para Linux fica para uma tarefa à parte).
- `comdlg32` (biblioteca de diálogo nativo do Windows) era linkada sem
  condição no `CMakeLists.txt`, quebrando o link fora do Windows.
  Agora só entra quando `WIN32` é verdadeiro.

### Adicionado
- `build.sh` agora usa `build-linux/` como diretório de build (não
  `build/`, que é onde `build.ps1` grava o cache do CMake) -- evita o
  erro "`CMakeCache.txt` directory is different" ao alternar entre
  compilar pelo Windows nativo e pelo WSL no mesmo checkout.
- Pacote de distribuição Linux (`dist/fwMSX-X.Y.Z-linux.tar.gz`) agora
  versionado no repositório, igual ao `.zip` do Windows.

### Validado
- `./build.sh` rodado numa máquina Linux real (WSL2): build completo +
  `ctest -R "z80|memmap"` com as **328 verificações passando** --
  primeira confirmação de que o núcleo Z80 (incluindo a aceleração
  `LDIR`/`LDDR` em Assembly, branch `elf64`) e o mapa de memória
  funcionam de verdade fora do Windows, não só compilam.

## [1.4.0] - 2026-09-30 - "Illusion City: Mapa de Memória"

Mapa de memória MSX (slots/subslots/MegaROM) sobre o núcleo Z80 da
v1.3.0 -- ver [doc/memory-map-spec.md](memory-map-spec.md) para o
histórico completo de fases. **Trabalho no core de emulação pausado
deliberadamente a partir daqui** -- ver [SPEC.md, seção 5.0](SPEC.md)
para os próximos passos registrados.

### Adicionado
- **Motor de slots/subslots em C** (`src/memmap/core/`), adaptado de
  `resource/fMSX/fMSX/MSX.c`/`MSX.h` (`MemMap[4][4][8]`, `PSL`/`SSL`/
  `SSLReg`, incluindo o quirk real de hardware onde o registrador de
  slot secundário é indexado pelo slot primário que ocupa a página
  `C000h-FFFFh`, não um registrador global único).
- **`MemorySystem`/`SlotMemoryBus` em C++** (design próprio), implementa
  o `IBus` do núcleo Z80 e expõe uma API de inspeção
  (`PeekSlot`/`PokeSlot`/`Describe`/`CurrentView`) que enxerga qualquer
  uma das 16 combinações de slot **independente** do que está
  atualmente visível para a CPU -- tratado como requisito vital desde a
  primeira fase deste módulo, por pedido explícito do autor.
- **Carregamento de ROM real** (`LoadRom`) com **CRC32 em Fortran**
  (tabela de 256 entradas, verificado contra o vetor de teste padrão
  `CRC32("123456789") == 0xCBF43926`) como conveniência do depurador.
- **Seis mappers MegaROM de troca de banco** (`Gen8`/`Gen16`/`Konami5`/
  `Konami4`/`ASCII8`/`ASCII16`, só a parte de ROM -- sem SCC/SRAM),
  adaptados de `MapROM()` do fMSX com as condições exatas de
  endereço/máscara de cada protocolo, incluindo o quirk de
  compatibilidade do ASCII16 para escrita "lixo" em endereço alinhado.
- **`fwmsx --z80dbg --slots [rom]`**: comandos `slots`/`pages`/
  `slotmem`/`slotpeek`/`slotpoke`/`loadrom` (com mapper opcional), e
  carregamento automático de uma ROM de boot no slot 0:0 na abertura.
  `--z80dbg` sem `--slots` continua exatamente como antes (RAM plana),
  decisão deliberada para não exigir localizar uma BIOS no caso de uso
  mais simples.
- **Validado contra a BIOS MSX1 real do fMSX**
  (`resource/fMSX/ROMs/MSX.ROM`, já presente no repositório para
  estudo): 192 endereços de PC distintos visitados em 100 mil ciclos de
  execução real de código de BIOS.
- **`build.sh`**: equivalente Linux do `build.ps1`, para compilar/testar
  também fora do Windows -- em particular para validar em execução real
  (não só montagem) a branch `elf64`/SysV AMD64 do `.asm` dual-ABI do
  núcleo Z80.
- 328 verificações automatizadas novas desde a v1.3.0 (`ctest -R
  "z80|memmap"`): 168 no motor Z80 (inalterado), 58 no depurador
  (+23 desde a v1.3.0), 102 no mapa de memória.

### Adiado, com motivo registrado (ver `doc/memory-map-spec.md`, seção 6)
- Chip de som SCC e SRAM persistente (`ASCII8`/`ASCII16`) -- dependem de
  subsistemas (áudio, save-state) que ainda não existem.
- `MAP_GMASTER2`/`MAP_FMPAC` inteiros -- só interessantes por causa de
  SRAM/som, sem isso viram um `Konami4` trivial.
- Heurística `MAP_GUESS` de auto-detecção de mapper -- precisaria de
  bancos de assinatura SHA1/CRC externos (`CARTS.SHA`/`CARTS.CRC`);
  `loadrom` exige o tipo de mapper explícito por enquanto.

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
