# editor-spec -- o editor de textos do fwMSX (estilo Norton Editor 2)

> **Status: so' projeto (2026-10-20). Nada implementado.** E' o item 3 do mapa mestre (`doc/SPEC.md` 5.6.3), o PRIMEIRO do IDE (5.3.0).
> Pedido do usuario: praticamente os MESMOS recursos na TUI e na GUI; mesmos atalhos sempre que possivel (prioridade para o console); destaque de sintaxe, auto completar, snippets; estilo do **Norton Editor 2** ("leve e agradavel"). **Nao** e' para competir com VS Code/Sublime: e' um editor ajustado ao MSX.
> Referencias visuais: `images/ne-00.png` a `ne-12.png` (13 telas do NE 2). Ha' mais telas por vir; o usuario pode juntar o que faltar.

## 1. O que as 13 imagens do NE 2 mostram (levantamento)

**Tela**: 25 linhas; barra de menu no topo (`File Block Text-layout Search Print View Options` e, a direita, `F1=Help`); area de texto; barra de rolagem a direita (setas e trilho pontilhado); barra de estado embaixo:
`<- Line=1 Col=1 <nome do arquivo ou Untitled> Insert|Over WW=Off|On ->`; um `*` na barra de estado aparece depois de editar (modificado). Menus em caixa cinza com sombra pontilhada, letra de atalho em video invertido, item atual em preto. Caixas de dialogo com borda dupla e titulo (`Display setup`, `Editor setup`), botoes `[ OK ]` e `Cancel`, radio `( )` e caixas `[x]`.

**Menus e itens** (o que o NE 2 tem; todos viram requisito):

| Menu | Itens |
|---|---|
| **File** | New, Open..., Save, Save As..., Merge...; 2nd window... (F2), Close window; Load more, Write to cursor, Top of file; DOS shell, eXit |
| **Block** | Copy, Move, Delete; Set marker, Remove markers, Find marker; Indent..., Unindent...; Output to file... |
| **Text-layout** | Paragraph format, Document format..., Center line; Setup..., Word wrap; Insert page breaks..., Remove page breaks...; Expand tabs..., Put in Tabs...; Auto indent |
| **Search** | Find string... (Ctrl-F), Find and Replace... (Ctrl-R), Continue fwd (Alt-C), Continue rev (Ctrl-C); Line number..., Matching bracket (Ctrl-]), Window Differences; Text in Block, Block Marker |
| **Print** | Print, Print Block, Eject page; Setup... |
| **View** | Information display, Outline display |
| **Options** | Display..., Mouse..., Editor...; Save configuration... |
| **F1=Help** | General, Keys, File, Block, Text-layout, Search, Print, View, Options; About NE |

**Dialogo Display setup**: cor (Color 1, Color 2, VGA Monochrome, Monochrome, LCD); linhas (25, 33/35, 40/43, 50, Don't change); Display scrollbar; Blinking bracket; Zoom window effects; Large cursor; Tab spacing [8].
**Dialogo Editor setup**: modo Insert/Over (Start in insert / Start in overstrike / Never overstrike); Tab key (Inserts a tab / Inserts spaces / Moves to tab stop); caractere de backup (`{` `~` `@`); Delete backup files; **Alt alone selects menu**.

Atalhos visiveis nas telas: **Ctrl-F** achar, **Ctrl-R** trocar, **Alt-C** continuar adiante, **Ctrl-C** continuar para tras, **Ctrl-]** parentese casado, **F2** 2a janela, **F1** ajuda. Os demais (movimento, apagar, blocos) NAO aparecem nas imagens: vem de `F1 > Keys` (pedir ao usuario a tela, ou o `NE.HLP`/manual).

## 1b. LEVANTAMENTO COMPLETO do NE 2 (2026-10-20; 49 telas `images/Monitor_1_*.png` + as 13 `ne-*.png`)

> Pedido do usuario: **replicar o maximo possivel do editor original.** Esta secao e' a especificacao funcional: o que cada tela mostra. As telas `Monitor_*` seguem a ordem: (a) ajuda `F1` (General, Keys, menus), (b) About, (c) dialogos de File ate Options, (d) esquemas de cor, (e) saida.

### 1b.1 Teclas (telas "Help on keys"; todas viram requisito)

| Grupo | Tecla | Acao |
|---|---|---|
| Cursor | setas | esq/dir/cima/baixo |
| | Ctrl-Esq / Ctrl-Dir | palavra anterior / proxima |
| | Home / End | inicio / fim da linha |
| | PgUp / PgDn | pagina acima / abaixo |
| | Ctrl-PgUp / Ctrl-PgDn | topo / fim da TELA |
| | Ctrl-Home / Ctrl-End | inicio / fim do ARQUIVO |
| | **Alt-Z / Ctrl-Z** | rola o texto para cima / para baixo (sem mover o cursor) |
| Apagar | Backspace / Del | caractere a esquerda / a direita |
| | Ctrl-W / Alt-W | palavra a esquerda / a direita |
| | Ctrl-L / Alt-L | ate' o inicio / ate' o fim da linha |
| | Alt-K | apaga TODOS os caracteres da linha (a linha inteira) |
| | Ctrl-U | desfaz a ultima remocao se o cursor nao se moveu (o NE nao tem desfazer geral) |
| Busca | Ctrl-F / Ctrl-R | achar / achar e trocar |
| | Alt-C / Ctrl-C | continuar adiante / para tras |
| | Ctrl-] | parentese/colchete/chave/ `<>` casado (`(<{[ ]}>)`) |
| Outros | Ctrl-A / Alt-A | inverte maiuscula/minuscula do cursor ao INICIO / ao FIM da linha |
| | Ctrl-P | insere tecla de controle/estendida literal (a proxima tecla) |
| | Ctrl-Enter | nova linha COM indentacao |
| | Ctrl-Esq / Shift-Tab | remove 1 nivel de indentacao |
| | Ctrl-2 / Alt-2 | move o divisor das janelas para cima / para baixo |
| | Ins | alterna Insert / Over |
| | F1 / F2 | ajuda / 2a janela |
Regra do NE: "muitos comandos a esquerda do cursor usam Alt; muitos a direita usam Ctrl" (Ctrl-W x Alt-W, Ctrl-L x Alt-L, Ctrl-A x Alt-A).

### 1b.2 Mnemonicos de menu (Alt+letra do menu, depois a letra do item) -- como o NE os documenta

- **File** `Alt-F`: N New, O Open, S Save, A Save As, M Merge, 2 2nd window (F2), C Close window, L Load more, W Write to cursor, T Top of file, D DOS shell, X eXit.
- **Block** `Alt-B`: C Copy, M Move, D Delete, S Set marker, R Remove markers, F Find marker, I Indent, U Unindent, O Output to file, **Del** apaga bloco SEM confirmar, **L** marca a linha (marcadores em volta), **E** marca do cursor ate' o fim da linha.
- **Text-layout** `Alt-T`: P Paragraph format, D Document format, C Center line, S Setup, W Word wrap, I Insert page breaks, B Remove page breaks, E Expand tabs, T Put in Tabs, A Auto indent.
- **Search** `Alt-S`: F/R/C pelos atalhos diretos; **L** Line number, **D** Window Differences, **B** Text in Block, **M** Block Marker.
- **Print** `Alt-P`: P Print, B Print Block, E Eject page, S Setup.
- **View** `Alt-V`: I Information display, O Outline display, **A About** (o About tambem esta' no menu F1).
- **Options** `Alt-O`: D Display, M Mouse, E Editor, S Save configuration.
- Sair: "Esc, depois Alt-F X". Dentro de um dialogo: Tab/Shift-Tab entre campos, espaco/setas mudam, **Enter = botao padrao `[ ]`**, **Esc = Cancel**, a **inicial** do botao o aciona (ex.: `N` = Next).

### 1b.3 Dialogos (todas as telas; campos exatos)

- **General help / Help on keys / File|Block|Text-layout|Search|Print|View|Option menu help**: paginas encadeadas com botoes `Next` / `Previous` / `Cancel` e o rodape `More`; cada item mostra o nome em video invertido, o atalho a direita (`Alt-F N`) e a descricao.
- **About**: "The Norton Editor / Published by Symantec Corporation / Version 2.0 -- 11/28/90 / (C) Copyright 1990, S. Reifel & Co / All Rights Reserved" + `[ OK ]`. **(No fwMSX: texto proprio, nunca o da Symantec.)**
- **Open file / Save file as / Merge file**: `File name: [*.ME]` (mascara da extensao atual); lista de arquivos do diretorio atual (`c:\ne\`) com barra de rolagem horizontal; `Dirs:` com `..`, `[-A-]`, `[-B-]`, `[-C-]` (unidades) e barra vertical; `[ OK ]` `Cancel`. Os tres sao o MESMO dialogo com titulos diferentes.
- **2nd window**: o mesmo dialogo de arquivo; a tela divide em **duas janelas horizontais**, a de baixo com a rolagem propria; uma barra cinza no meio mostra o nome do arquivo; a barra de estado mostra a janela ATIVA.
- **Text-layout setup**: `Line Length [65]`, `1st line margin [0]`, `Left Margin [0]`, "Begin formatting at: ( ) Cursor (•) Top of paragraph", OK/Cancel.
- **Insert page breaks starting at page... / Remove page breaks starting at... / Format document starting at... / Expand tabs to spaces starting at... / Replace left-hand spaces with tabs starting at...**: botoes `[ Top ]` `Cursor` `Cancel`.
- **Search for text**: `Find: [ ]`, `[ ] Upper/lower case must match`, `[ ] Search backwards`, `[ OK ]` `F1=Help` `Cancel`. **Search & Replace**: acrescenta `Replace: [ ]`; mesmas opcoes. **Find line**: `Line number: [ ]`.
- **Print setup**: `Top margin [2]`, `Left margin [0]`, `Page length [62]`, `[x] Formfeed between pages`, "Formfeed after printing: ( ) No (•) One page ( ) Two pages".
- **Information display**: arquivo, diretorio, hora e data atuais, caracteres/palavras/linhas no buffer, espaco livre no buffer, caracteres nao lidos no arquivo de entrada, espaco livre no disco de saida, "Character at cursor: 'W' = 87 = 0x57".
- **Display setup**: cores (Color 1, Color 2, VGA Monochrome, Monochrome, LCD), linhas (25, 33/35, 40/43, 50, Don't change), Display scrollbar, Blinking bracket, Zoom window effects, Large cursor, `Tab spacing [8]`.
- **Mouse setup**: botao (esquerdo/direito), atraso de rolagem (No delay, 12/18/26 ms, Sync to clock), `[x] Adj mouse cursor speed`.
- **Editor setup**: modo (Start in insert / Start in overstrike / Never overstrike); Tab key (Inserts a tab / Inserts spaces / Moves to tab stop); caractere de backup (`{` `~` `@`); Delete backup files; Alt alone selects menu.
- **Save configuration**: "Update: (•) current config file ( ) config for this directory" -- ha' **configuracao por diretorio**.
- **Saida**: "This file has been changed. Save it before exiting?" `Save` `[Don't Save]` `Cancel`; e, se a configuracao mudou, "The editor's configuration has been changed. Save this configuration?" `[ Save ]` `Don't Save`.

### 1b.4 Tela e visual

- 80x25 (ate 50 linhas). Linha 1: menu; linha final: estado. Barras de rolagem a direita (com `↑`/`↓` e trilho pontilhado) e uma seta `←`/`→` na barra de estado (rolagem horizontal).
- **Barra de estado**: `← [*] Line=N  Col=N  <arquivo>  Insert|Over  WW=On|Off  [Auto] →`. O `*` = arquivo modificado; `Auto` = auto-indent ligado; `WW` = word wrap.
- Menus: caixa com sombra pontilhada; letra de atalho destacada; item atual invertido; **itens indisponiveis em outra cor** (laranja no esquema colorido: "Close window", "Load more", "Write to cursor", "Top of file" quando nao se aplicam); bloco marcado em video diferente; marcadores de bloco como um pequeno quadrado na posicao.
- **Esquemas de cor** vistos: **LCD / monocromatico** (texto claro no preto, menus cinza), **azul** (texto branco no azul, barras cinza), **verde-agua/ciano** (menus e dialogos ciano com letras-atalho amarelas; sombra preta) e uma variante com letras-atalho **vermelhas**. Mapeiam as opcoes Color 1, Color 2, VGA Monochrome, Monochrome, LCD.
- "Zoom window effects": os dialogos abrem crescendo (animacao); "Blinking bracket": o parentese casado pisca ao digitar `)}]`.

### 1b.5 O que o NE 2 NAO tem (e o fwMSX acrescentara, sem tirar nada)

desfazer/refazer geral; area de transferencia do sistema; selecao com Shift+setas; destaque de sintaxe; completar; snippets; abas; multiplos cursores. **Conflitos de tecla a resolver**: Ctrl-Z (NE: rolar) x desfazer comum; Ctrl-C (NE: continuar para tras) x copiar comum; Ctrl-F/Ctrl-R sao do NE. Proposta: o **perfil "Norton" e' o padrao** (teclas do NE exatas) e o desfazer/copiar/colar modernos entram em teclas ALTERNATIVAS nao conflitantes (ex.: `Alt-Backspace`/`Ctrl-U` repetido, `Ctrl-Ins`/`Shift-Ins`/`Shift-Del`), com um segundo perfil "Moderno" opcional. Decisao do usuario pendente.
**Adaptacoes para o MSX/fwMSX** (o NE e' de DOS): "DOS shell" abre o console do fwMSX; unidades `[-A-]..` viram as unidades reais (e as imagens `.dsk` montadas); "Print" imprime em arquivo/PDF/impressora; "Load more/Write to cursor/Top of file" valem para arquivos maiores que o buffer (opcional, ver 6).

## 1c. Decisoes do usuario sobre as telas (2026-10-20)

1. **Perfis de teclas**: o padrao e' o perfil **"Norton"** (teclas EXATAS do NE 2, secao 1b). As teclas modernas (desfazer/refazer geral, copiar/colar do sistema, Shift+setas, etc.) entram como **alternativas nao conflitantes** dentro do proprio perfil Norton. Alem dele existe um segundo perfil **"fwEditor"** (nosso, a desenhar: teclas mais modernas, estilo editores atuais, com as ferramentas do MSX), escolhido pelo usuario em Options > Editor; **por padrao entra o Norton**. Cada perfil e' uma tabela de dados (`editor_keys`), documentada em `man editor` (um quadro por perfil) e editavel.
2. **Teclas que o terminal entrega mal** (Ctrl-2/Alt-2 divisor, Ctrl-Enter, Ctrl-PgUp/PgDn, Ctrl-Home/End, Alt sozinho, Ctrl-Esq/Dir, Ctrl-]): o fwMSX define uma **tecla alternativa documentada** para cada uma, igual na TUI e na GUI; a do NE continua valendo onde o terminal a entrega (a GUI entrega todas). O usuario ja' esperava isso (o NE e' do tempo do MS-DOS).
3. **Load more / Write to cursor / Top of file**: **implementar de verdade** (util para documentacao e arquivos grandes), prioridade BAIXA, depois do resto (E7). Nao e' esperado uso frequente: arquivos de MSX raramente passam da memoria de um PC atual.
4. **Modo SOMENTE LEITURA** (ideia nova do usuario): `view programa.bas` (comando do console e opcao `--view`; tambem pelo menu) abre o mesmo editor em modo de visualizacao. Regra: **o que nao e' destrutivo fica habilitado; o que e' destrutivo fica desabilitado, e se o usuario acionar a tecla nada acontece** (sem erro, sem aviso intrusivo; a barra de estado mostra `View`/`Read-only` no lugar de Insert/Over).
   - **Habilitado**: toda navegacao (setas, Home/End, PgUp/PgDn, palavras, topo/fim, rolar, ir para linha, parentese casado); busca (Find, Continue fwd/rev, Text in Block); marcar bloco e **copiar para a area de transferencia**; Block > Output to file (gravar o bloco); **Save As** (grava uma COPIA); imprimir (Print, Print Block, Eject, Setup); Information display; Outline display; 2a janela e Window Differences; Options (Display, Mouse); Help; eXit (sem perguntar, pois nada muda); destaque de sintaxe, completar nao se aplica.
   - **Desabilitado** (item cinza/laranja no menu, tecla ignorada): digitar, Del/Backspace, todas as teclas de apagar (Ctrl/Alt-W, L, K, Ctrl-U), Ctrl-A/Alt-A, Ctrl-P, Ctrl-Enter, Tab/indentacao, Save, New (do arquivo atual), Merge, Block Copy/Move/Delete e Indent/Unindent, Text-layout inteiro (format, center, tabs, page breaks, word wrap, auto indent), Find and Replace, colar, desfazer, snippets.
   - Tambem vale quando o arquivo tem o atributo somente leitura do disco (abrir como `view` automaticamente, com a opcao de liberar a edicao).
   - Implementacao: um **flag no modelo** (`read_only`) e uma tabela de comandos com a propriedade `destrutivo`: o modelo ignora o comando e o menu o pinta como desabilitado. Testavel sem terminal.

## 2. Decisao de arquitetura: UM MODELO, UMA GRADE, DOIS DESENHISTAS

O mesmo padrao do Mamute e do Z80 Debugger (modelo sem interface, testavel sem terminal):
- **`ed::Editor` (modelo)**: buffer, cursor, selecao/bloco, marcadores, desfazer/refazer, busca, janelas (ate' 2), destaque, completar, snippets. `Key(nome)` entra; uma **grade de celulas** (caractere + atributo/cor) sai, mais barra de menu, barra de estado e dialogos tambem como grade.
- **Desenhistas finos**: TUI (FTXUI) e GUI (ImGui, fonte de largura fixa; opcionalmente a fonte do proprio MSX, CGTABL) so' pintam a grade e repassam teclas/mouse. **Isso garante recursos e atalhos IGUAIS nos dois modos por construcao**: nao existe recurso "so' da GUI" no editor.
- Fica fora da grade so' o que o terminal nao faz: fonte proporcional, PDF, imagens (ver 5.3.6 do SPEC).
- Testes: sequencias de teclas -> grade/texto esperado, como `tuitest` e o painel do depurador.

## 3. Atalhos: a regra "console primeiro"

O console tem limites que decidem a tabela:
- **Ctrl+letra** chega como byte de controle: `Ctrl-I`=Tab, `Ctrl-M`=Enter, `Ctrl-H`=Backspace, `Ctrl-[`=Esc, `Ctrl-J`=LF. Nao da' para distinguir `Ctrl-Shift-letra`.
- **Alt+letra** chega como ESC+letra (Linux) ou evento proprio (Windows); **Alt sozinho** abrir o menu (o NE faz) **nao e' detectavel** em terminal Unix e e' fragil no Windows Terminal -> alternativa: **F10** e **Esc+letra**; na GUI o Alt sozinho funciona. A opcao "Alt alone selects menu" fica, mas vale so' onde o terminal deixa.
- **F-keys** com Shift/Ctrl variam de terminal para terminal.
- **Ctrl-C** e' interrupcao em console cru; no editor precisa ser tratado como tecla (modo raw) -- o NE o usa para "Continue rev".
- Regra: escolher a tabela do NE onde ela cabe; onde nao cabe, dar uma **tecla alternativa** documentada e a MESMA na GUI. Tudo mapeado em uma **tabela unica de atalhos** (`editor_keys`), editavel, para o usuario reatribuir.

## 4. O que vai alem do NE (o pedido novo)

1. **Destaque de sintaxe** por linguagem, guiado por DADOS (uma definicao por linguagem: palavras-chave, comentarios, textos, numeros, rotulos, diretivas, estado que atravessa linhas). Linguagens: **MSX-BASIC** (palavras do `man`, 172 reservadas), **Basic Dignified**, **ASM** (Z80, fwasm/asMSX; depois Kaji80/M80), **Pascal** (Kizuna/TP), **`.BAT`/`.BTM`** (MSX-DOS 2 e estilo 4DOS), **`.DMX`**, **`.AMX`** (a esclarecer com o usuario), Markdown, texto puro, mais as futuras (Obifile, MML...).
2. **Auto completar**: lista flutuante (Ctrl-Space ou ao digitar), com fontes de DADOS que o fwMSX JA' TEM: palavras do BASIC, mnemonicos/diretivas do fwasm, **554 rotinas/variaveis da BIOS/BDOS** do `man`, simbolos do arquivo e do projeto, palavras ja' digitadas no buffer.
3. **Snippets**: gatilho + Tab, com pontos de parada `$1 $2 $0` e valores padrao; arquivo de snippets por linguagem (texto simples, editavel), com um conjunto inicial MSX (ex.: `loop`, `romhdr` cabecalho de cartucho, `bdos` chamada do MSX-DOS, `screen2`, `sprite`).
4. **Integracao com o fwMSX**: "Executar no MSX" (asmrun: ROM, `.basic`, `.msxdos`), erros do montador pulam para `arquivo:linha`, simbolos e `.break` do depurador (5.3.5).
5. **Arquivos do MSX**: fim de arquivo **Ctrl-Z (0x1A)** do MSX-DOS, CRLF, BASIC tokenizado x ASCII (abrir `.bas` tokenizado e mostrar como texto), BOM/UTF-8 (o fwasm ja' trata), codificacao do MSX para caracteres especiais. Mapeamento de caracteres para a tela.

## 4b. Respostas do usuario (2026-10-20)

- **`.DMX` / `.AMX` / `.BMX`** (cadeia do **Basic Dignified Suite**, "BaDig"): `.DMX` = fonte no formato do BaDig (com a sintaxe de preprocessamento); `.AMX` = versao compactada e com linhas numeradas, resultado do preprocessamento; `.BMX` = arquivo final **tokenizado**, pronto para carregar no MSX. O editor da suporte a `.DMX` (destaque, completar e snippets da sintaxe de preprocessamento) e abre `.AMX` (ASCII numerado) como texto; `.BMX` e' binario tokenizado (visualizar como texto, como o `.bas` tokenizado). Exigencia do usuario: **compatibilidade TOTAL com a sintaxe do Basic Dignified Suite**, que o fwMSX pode depois AMPLIAR. A ajuda do BaDig ja' esta' no `man` (21 topicos, 1.38.3); ela e' a fonte das palavras e diretivas.
- **Dialetos de BASIC a suportar** (alem do MSX-BASIC padrao): **Basic Dignified Suite**, **MSXBAS2ROM** ("MSX Turbo Basic"), **NestorBASIC** (chamadas `USR`/funcoes) e **Turbo Basic / Basic Kun**. Cada um e' uma definicao de linguagem por dados (palavras-chave, completar, snippets); o dialeto pode ser escolhido pela extensao ou por uma linha de cabecalho.
- **Menus novos separados (decidido)**: o editor ganha menus proprios ALEM dos do NE, nao so' "Executar": **Executar**, **Montar** (asMSX/fwasm), e as ferramentas **Sprites, Telas, Draw, Som, Musica, SEE...** (os editores visuais de 5.4). O NE tem 7 menus + Help; com os novos a barra pode estourar 80 colunas: prever abreviacao/rolagem da barra ou agrupar (ex.: um menu **Ferramentas** com sub-menus), a decidir no E0.
- **Espera**: o usuario vai gerar as telas de **F1 > Keys / Help** do NE 2. A tabela de teclas (E0) so' fecha depois delas.

## 5. Fases (cada uma e' uma release 1.x, sem quebrar o resto)

- **E0 -- Estudo fechado**: tabela de atalhos do NE 2 (de `F1 > Keys`), lista de recursos finais, linguagens (DMX/AMX), decisao do menu extra. Escrever os testes de teclas.
- (O modo `view` somente leitura entra ja' no E1: e' so' um flag e a propriedade `destrutivo` de cada comando; os testes cobrem "tecla destrutiva nao muda nada".)
- **E1 -- Nucleo**: buffer de linhas, cursor, insercao/apagamento, Insert/Over, desfazer/refazer, selecao/blocos, marcadores, abrir/salvar, CRLF/Ctrl-Z/backup, busca e troca, ir para linha, parentese casado. Modelo + grade; teste sem terminal. Comando `edit <arq>` no console.
- **E2 -- Tela do NE na TUI**: barra de menu, menus e dialogos como a imagem, barra de estado, rolagem, 2a janela (F2) e diferencas entre janelas; mouse.
- **E3 -- GUI**: mesma grade em ImGui; mesmos atalhos; paridade de testes.
- **E4 -- Destaque de sintaxe**: motor de dados + as linguagens (comecando por MSX-BASIC e ASM).
- **E5 -- Completar e snippets**: lista flutuante, fontes de dados, snippets com pontos de parada.
- **E6 -- Integracao**: Executar no MSX, erros -> linha, projeto (`.msxproject`), ajuda `F1` ligada ao `man`.
- **E7 -- Texto "de escritorio"**: formatar paragrafo/documento, centralizar, quebras de pagina, tabs, imprimir (os menus Text-layout e Print do NE).

Regra das 4 linguagens: modelo e UI em **C++**, tokenizador de destaque em **C** (kernel rapido por linha), tabelas/estatisticas (ex.: frequencia de palavras para ordenar sugestoes) em **Fortran**, varredura rapida de busca em **NASM**.

## 6. Decisoes tomadas x em aberto

Tomadas (do usuario): estilo Norton Editor 2; mesmos recursos TUI/GUI; atalhos iguais com prioridade ao console; destaque + completar + snippets; foco em MSX, nao em clonar o VS Code.
Respondidas em 4b: o que sao `.DMX/.AMX/.BMX`; menus novos separados (Executar, Montar, editores...).
Fechadas pelas telas (2026-10-20): (1) a tabela de teclas do NE 2 esta' completa em 1b.1/1b.2; (5) o tema padrao: o NE tem 5 esquemas de cor, replicamos todos (padrao: LCD/mono das capturas, mais o azul e o ciano); o usuario quer replicar o MAXIMO do original.
Respondidas em 1c: perfil Norton padrao + teclas modernas alternativas + perfil "fwEditor" opcional; teclas alternativas para o que o terminal entrega mal; Load more/Write to cursor/Top of file serao implementados (baixa prioridade); modo somente leitura `view`.
Em aberto: (3) como a barra de menus comporta os novos menus (80 colunas); (10) desenhar o perfil "fwEditor" (so' depois do Norton funcionar); (11) a tabela das teclas alternativas (uma por tecla problematica); (4) arquivos grandes (o NE tem "Load more"): e' preciso ou basta carregar tudo?; (5) tema padrao: cinza do NE ou escuro moderno?; (6) varios arquivos abertos: 2 janelas como o NE, ou abas tambem?; (7) NE 2 e' da Symantec: o fwMSX copia o JEITO (menus/teclas), nunca codigo nem textos de ajuda do original.

## 7. Estado (atualizado em 2026-10-20)

**E1 entregue na 1.41.0**: `src/editor/{cpp,c,fortran,asm}`, `tests/editor/editor_test.cpp` (`ctest` editor). Feito: buffer e cursor, Insert/Over, desfazer/refazer, blocos, busca/troca, ir para linha, parentese casado, as teclas do NE 2 (40) e as alternativas, os 49 mnemonicos de menu, modo somente leitura (`ItemEnabled`), arquivo
(CRLF/LF, Ctrl-Z, BOM, backup, Merge, Output to file), Text-layout simples (Center line, Word wrap, Auto indent, tabs, quebras de pagina), Information, grade 80x25 como a tela do NE.
**Escolhas do E1 a rever com o usuario** (as telas nao diziam): Alt-K apaga a linha inteira; Ctrl-Backspace = desindentar; Ctrl-U so' a ultima remocao; busca sem dar a volta; `Col=` e' coluna de tela; CRLF conta 2 na Information; "Moves to tab stop" so' move o cursor; quebra de pagina = linha com 0Ch; backup troca o 1o caractere da extensao;
Alt-B S na terceira vez recomeca o bloco; copiar/mover bloco deixam o cursor no fim do texto inserido.
**Falta**: E2 em diante (tela, dialogos, 2a janela e Window Differences, mouse, `edit`/`view` no console, Print, Outline, Paragraph/Document format, Load more/Write to cursor/Top of file, Help paginado do NE em `F1`, perfil fwEditor, menus novos).

**E2 entregue na 1.41.1** (`fwMSXc --edit`/`--view`): `editor_ui.*` (aplicacao), `editor_dialogs.cpp` (caixas e ajuda), `editor_keys.*` (terminal -> tecla), `editor_tui.cpp` (FTXUI). Feito: menus abertos (Alt+letra/F10), F1=Help paginada, os dialogos do NE (arquivo, Find, Replace interativo, linha, indent, output,
Text-layout/Print/Display/Mouse/Editor setup, Save configuration, saida, page breaks/tabs "starting at"), 2a janela com divisor e Window Differences, mouse, 5 esquemas de cor, `fwedit.cfg`. **Falta de E2**: `edit`/`view` dentro do console e da TUI de menus (so' `--edit`/`--view` por enquanto), `man editor`, conferir no terminal real (Alt/Ctrl+setas/mouse no Windows Terminal e no Linux).

**E3 entregue na 1.41.2** (`fwMSX.exe --edit`/`--view` em janela): `src/editor/gui/editor_window.cpp` (ImGui/GLFW/OpenGL3), `editor_gui_input.*` (nomes de tecla e paleta RGB, sem OpenGL), `--gui`/`--tui` em `main.cpp`. A janela e' fina: teclado/mouse -> `EditorApp`, grade -> `ImDrawList`; ornamentos desenhados a mao; Alt sozinho abre o menu; area de transferencia do sistema.
**Falta de E3**: zoom da fonte, icone, conferir mouse/Alt sozinho/AltGr numa janela de verdade, `edit`/`view` dentro do console e da TUI de menus, `man editor`.

**CORRECAO (2026-10-20): a E3 da 1.41.2 (grade de caracteres numa janela) NAO era o que o usuario queria e foi substituida na 1.41.3.** Pedido esclarecido: um programa GUI de verdade, integrado ao menu do fwMSX, visual igual/parecido com o fwMSX; "nao quero o mesmo editor, quero apenas os mesmos atalhos e opcoes do menu".
Decisoes: janela flutuante dentro do fwMSX (+ standalone `--edit`); barra de menus propria do editor; abas + divisao opcional em 2 paineis; tema do fwMSX, fonte monoespacada, textos em portugues (`ed::Tr`; a TUI fica em ingles como o NE).
**E3b entregue na 1.41.3**: `ed::gui::EditorPanel` (src/editor/gui/editor_panel.*) -- menus, abas, paineis de texto com rolagem/mouse, dialogos com widgets reais e barra de estado, sobre o mesmo `EditorApp`; menu "Editor" e janela flutuante em `emu_window.cpp`; `editor_window.cpp` = host standalone;
`editor_i18n.*` (portugues). A grade/paleta da 1.41.2 saiu da GUI (`editor_gui_input` ficou com os nomes de tecla; a paleta RGB so' e' testada).
**Falta**: mouse/Alt sozinho/AltGr conferidos numa janela real; opcoes de visual (numeros de linha, fonte, divisao empilhada); icone; E4-E7; menus novos (Executar, Montar, Sprites...).

**1.41.4: fechar o editor EMBUTIDO so' esconde (sem pergunta) e as abas, com o texto nao salvo, sao guardadas em `fwedit.session`** (pedido do usuario: anotar enquanto trabalha no MSX e salvar so' no fim). Menus e dialogos da GUI sao janelas comuns do ImGui, nunca popups/modais (um modal esquecido na pilha do ImGui travava a reabertura). O editor standalone (`--edit`) continua perguntando ao sair.

**1.41.5: autosave** (Options > Editor: minutos, 0 = desligado; caixa "tambem sobrescrever os arquivos com nome") **e abas sem nome noname01, noname02...** (menor numero livre). Decisoes do usuario: fechar sem perguntar esta' ok; uma sessao por pasta do executavel esta' ok; o autosave tem os dois modos (so' sessao, ou tambem os arquivos com nome).

**E4 entregue na 1.41.6 (destaque de sintaxe)**: `syntax.h`/`syntax_langs.cpp` (10 linguagens como dados + varredores), `ed_syntax.c` (C), `ed_number.f90` (Fortran), `ed_hash.asm` (NASM), `Editor::HighlightLine`/`Language`, dialogo View > Language, cores na TUI (`SynPal`) e na janela (`SyntaxRgb`). Pendencias: sintaxe real do Turbo Basic/Basic Kun (o usuario informa); `.bmx` binario; o completar (E5) vai reutilizar as mesmas tabelas de palavras e a BIOS.

**1.41.7: MSX-BASIC KUN** (era "Turbo Basic / Basic Kun" com lista inventada): `_TURBO ON/OFF`, `_RUN`, remtags `'#I` `'#C` `'#N`, `@linha` e os comandos nao compilaveis em vermelho dentro do bloco (classe `kSynError`). Fonte: o manual KunBASIC do MSX2 Technical Handbook (so' a sintaxe foi usada, como dados).

**1.41.8: fase E5 (completar e snippets) + numeros de linha.** `Alt-/` / `Ctrl-Space`: lista de candidatos (palavras da linguagem via `Lang::Words`, a BIOS, gatilhos de snippet, palavras do texto), nota pelo Fortran (`ed_match_score`), prefixo pelo C (`ed_word_start`), palavras do texto sem repetir pelo hash NASM. Snippets: `Tab` apos o gatilho, `$1..$9` e `$0`, `fwedit.snippets` do usuario (`[linguagem]`, `gatilho=texto`). Completar sozinho (`Settings.auto_complete`, desligado) abre apos 2 letras. Numeros de linha: `Settings.line_numbers` (ligado), `Editor::Gutter()` na grade da TUI e a margem propria da janela. Em aberto: `${1:padrao}` e espelhamento, simbolos do projeto (E6), listar/inserir snippet por menu.

**1.41.9: fase E6, parte 1 (menu Build + ajuda da palavra).** Menu Build (Alt-U) entre Options e F1=Help: `Assemble` F9, `Run on MSX` F5, `Next/Previous message` F8/Shift-F8, `Messages...`; Help > `On word` Ctrl-F1. Monta o texto do buffer pelo fwasm (`fwasm::Assemble` + provedor de arquivos da pasta do fonte); `BuildMsg{file,line,warning,text}` vem de `ed_build_msg` (C), a navegacao usa `ed_cycle` (Fortran), a deduplicacao `ed_hash_ci` (NASM). `Run` e `Help` entram por ganchos (`SetRunHook` = `asmrun` do Commander no emu_window; `SetHelpHook` = `man::Resolve/Render` em `editor_host.cpp`), assim o modelo nao depende da maquina e os testes usam ganchos falsos. Em aberto da E6: projeto `.msxproject` (SQLite), receita/`Obifile`, simbolos do projeto no completar, menus Sprites/Telas/Draw/Som/Musica, `man editor`.
