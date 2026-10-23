# Releases

Versionamento no formato `X.Y.Z`, cada nivel com um nome (esquema de 2026-10-09; ver
`CLAUDE.md`, regra 1):

- **X** (major): um conjunto grande esta' praticamente todo feito. Leva o nome de uma **empresa
  de MSX** (1 = Konami).
- **Y** (minor): bloco maior de funcionalidade publicado como release. Leva o nome de um
  **jogo novo de MSX**, nunca repetido (1.32 = Nemesis).
- **Z** (patch): feature pequena, ajuste ou correcao. Leva o nome do jogo do Y + um subtitulo
  ("Nemesis: Ponte de controle"). X.Y.0 leva so' o nome do jogo.

As versoes 1.19 a 1.31 (esquema antigo) ficam todas como "King's Valley".

Resumo curto de cada versao tambem em [CHANGELOG.md](CHANGELOG.md);
especificacao completa e historico de fases em [SPEC.md](SPEC.md).

---

## v1.43.26 -- "Yie Ar Kung-Fu: Documentacao" (Konami) (2026-10-24)

**Pedido do usuario**: "ajuste a documentacao agora, atualize tudo e manuais, gere o release e mande para o GitHub": no README, o *Historico das Versoes* nao saia linha a linha (saia um bolo de texto sem salto de linha); renovar as screenshots, colocar mais algumas e o **splash bem no topo** como cartao de visita.
- **Causa do bolo de texto**: o historico era um paragrafo Markdown (linhas seguidas viram uma so') misturado com itens em negrito. Agora e' uma lista (`- **1.43.25** -- ...`), da mais nova para a mais antiga; as de 1.43 a 1.40 ficam abertas e as anteriores dentro de `<details>`.
- **Screenshots**: o script `tools/assets/make_screenshots.py` ganhou `extra_shots()` (e o argumento `extra` para refazer so' as novas): 09 OCM no prompt, 10 logo MSX++, 11 MSX-DOS 2, 12 Nextor + IDE, 13 Nextor + SD, 14 a 17 jogos. Conferidas uma a uma; a captura do Metal Gear 2 mostrou um quadro mal desenhado na abertura (nao entrou; anotado em `OUTLINE.md`).
- **Docs**: README, MANUAL (secao nova), man `ocm`, CHANGELOG, RELEASE, SPEC 5.0, OUTLINE, `machine-spec.md` e `LICENSE-THIRD-PARTY.md` (o pacote do KdL: uso local, nao redistribuido).
- Sem mudanca de comportamento do emulador. Validado: Windows e Linux, `ctest` 100% (35).

## v1.43.25 -- "Yie Ar Kung-Fu: Maquina OCM" (Konami) (2026-10-24)

**Pedido do usuario**: "sim, vamos para o OCM" (depois da analise do pacote do KdL: o OCM como maquina e' viavel). **Achados**: (1) o mapa de slots completo esta' nos comentarios do `emsx_top.vhd`; (2) as ROMs do pacote bastam (BIOS A1WSX, sub-ROM, MSX-Music, XBASIC, Kanji JIS1/JIS2, Kanji BASIC); (3) **o Kanji BASIC anima o logo esperando a colisao de sprites linha a linha** - o VDP so' a avaliava no fim do quadro, e a inicializacao levava ~70 s; com a colisao por linha leva ~10 s; (4) o teclado do `a1wsxbsl` e' o brasileiro.
- **Feito**: `src/ocm` (C/Fortran/ASM/C++), `MachineConfig::ocm*`, `ConfigureOcm`, o ramo do OCM em `DefaultLayout`, a celula 3-1 de 48 KB, `--ocm/--ocm-main/--ocm-logo`, o item do menu de modelos, `vdp_sprites_collision_on_line`, `doc/ocm-spec.md`.
- **Testes**: `msx2test` 8e (Kanji, portas comutadas, `ConfigureOcm` e o boot real do OCM com o Nextor; pulado sem as ROMs), `vdp2test` (colisao por linha).
- **Nao feito**: velocidade da CPU, DIP-SW virtuais, ESE-SCC nos slots 1 e 2, mapper extra em 0-1, BIOS do turbo R, MIDI/UART/Wi-Fi, estado salvo do OCM, documentacao de como baixar o pacote e a opcao de download "INTERNAL".
- **Nao verificado**: o item do menu na janela grafica (so' compila); o desenho do logo (so' o texto e o boot foram conferidos).
- Validado: Windows e Linux, `ctest` 100% (35).

## v1.43.24 -- "Yie Ar Kung-Fu: Cartao SD" (Konami) (2026-10-24)

**Pedido do usuario**: vasculhar o pacote do KdL (`third-party/ocm`, OCM-PLD e SDBIOS) e dizer se serve para a ESE-MegaSCSI e se da' para emular o OCM como maquina. Resposta: a MEGA-SCSI **nao** (nenhum driver, e o OCM nem emula o MB89352), mas o **MegaSD** sim, e o OCM como maquina e' viavel (mapa de slots no `emsx_top.vhd`). Decisao do usuario: copiar para `dist/roms` o que for necessario (utilitarios, ROMs, discos) e ir para a 1.43.24; a documentacao de como baixar do site do KdL e a opcao de download (modo INTERNAL) ficam para depois.
- **Achados**: (1) o MegaSD e' simples: mapper ESE-RAM + SPI em memoria (`megasd.v`, 342 linhas); (2) o driver (`nextsd15.rom`, fonte Z80 incluso) mostra o protocolo; (3) o driver descarta dois bytes depois do CRC do comando, entao o cartao tem de atrasar a resposta (dois FFh) - sem isso "Disk offline"; (4) acima de 1 GB os SDSC usam blocos de leitura de 1024 bytes no CSD.
- **Feito**: `src/hd/sd_card.{h,cpp}` (cartao SPI), `src/hd/megasd.{h,cpp}` (interface), `SlotKind::MegaSd`, `MachineConfig::sd_image/megasd_rom/megasd_slot`, `SetSd`, `romdb::FindMegaSdRom`, `--sd/--megasd-rom/--megasd-slot`, comando `sd`, menus da janela e do TUI, `Machine::SdInfo`; `third-party/ocm/` no `.gitignore`; copia do material para `dist/roms/ocm/`.
- **Testes**: `hdtest` (protocolo SPI completo: CMD0/8/55/41/58/9/17/18/12/24, erros, SDSC e SDHC, e o mapper do MegaSD), `msx2test` 8d (boot real do Nextor + MegaSDHC sobre uma imagem do fwMSX, grava `SD.TXT`; pulado sem a ROM), `controltest` (`sd`).
- **Nao feito**: a maquina OCM (Kanji, portas comutadas, ESE-SCC, mapa de slots), a MEGA-SCSI, o download do pacote (modo INTERNAL), o estado salvo do MegaSD.
- **Nao verificado**: os menus na janela grafica e o TUI (so' compilam).
- Validado: Windows e Linux, `ctest` 100% (35).

## v1.43.23 -- "Yie Ar Kung-Fu: Disco rigido" (Konami) (2026-10-24)

**Pedido do usuario**: HD, "com suporte para IDE e SCSI se voce puder, inclusive suportando ROMs da Sunrise com o kernel do Nextor"; imagens de 2, 4 ou 8 GB (o Nextor so' le particoes de ate' 2 GB); SD pelo Nextor/Sunrise ou OCM/SCSI; criar disco pela CLI/parametros como o openMSX, e na TUI e na GUI. **Respostas**: IDE primeiro e SCSI depois (nao ha' ROM de driver de SCSI no banco: a MEGA-SCSI nao tem ROM propria); imagem ate' 4 GB com particoes de 2 GB; ROM do Nextor escolhida por opcao ou, sem ela, a do banco.
- **Achados na pesquisa**: (1) o banco de ROMs tem 5 Nextor SunriseIDE (2.0 a 2.1.1, 128 KB, 8 bancos); (2) a interface Sunrise e' pequena (registrador de controle em 4104h, dados em 7C00h, ATA em 7E00h) e o disco ATA so' precisa de IDENTIFY/READ/WRITE em PIO; (3) o `SlotMemoryBus` so' aceitava um `SlotCartIo` (o SCC), que sobrescrevia o IDE: virou uma tabela por slot; (4) **uma celula 2:1 e' impossivel** na MSX2 (os slots 1 e 2 sao de cartucho, sem subslot, e a BIOS nao os enxerga expandidos): ficou em **3:0**; (5) o Nextor travava em "Initializing :" por causa do temporizador do turbo R em E6h/E7h; portas sem dispositivo agora leem FFh.
- **Feito**: `src/hd` (C, Fortran, Assembly, C++), `SlotKind::Ide`, `MachineConfig::ide_*`/`nextor_rom`, `SetIde`, `romdb::FindNextorRom`, `--ide/--ide-slave/--nextor/--ide-slot`, `--hdnew`, comandos `ide`/`newhd`, menu da janela (com a janela de novo disco) e do TUI, `Machine::IdeInfo`.
- **Testes**: `hdtest` (novo), `msx2test` 8c (boot real do Nextor 2.1.1 sobre uma imagem do fwMSX; pulado sem a ROM), `controltest` (`ide`/`newhd`); o teste do `CompositeBus` passou a esperar FFh.
- **Nao feito**: SCSI e o SD do OCM/MegaSD (precisam do driver), copiar o MSX-DOS para dentro do HD pelo `msxdisk`, estado salvo da interface IDE, o `MAPDRV` do Nextor (a 2a particao nao ganha letra sozinha).
- **Nao verificado**: o menu e a janela de novo disco na janela grafica (so' compilam); o TUI; o boot do MSX-DOS 2 a partir do HD (precisa dos arquivos do sistema na particao).
- Validado: Windows e Linux, `ctest` 100% (35).

## v1.43.22 -- "Yie Ar Kung-Fu: Mapper de RAM" (Konami) (2026-10-23)

**Pedido do usuario**: o proximo passo do MSX-DOS 2 e' a RAM - opcoes de mapper de 128 a 4096 KB para a MSX2/2+, e, se possivel, **um mapper interno e outro externo** (p. ex. 512 KB + 1024 KB): ha' programas que so' identificam o interno, outros so' o maior e alguns os dois, e a separacao ajuda a depurar e corrigir.
- **Achados na pesquisa**: o motor ja' suportava ate' 4096 KB (`ValidateLayout`: 64 a 4096 KB) e varios mappers (`MapperPorts` escreve em todos e cada `RamMapperDevice` mascara pelo proprio tamanho). Faltava so' a configuracao: o tamanho padrao era fixo (128 KB) e so' dava para mudar pelo `--slot`/janela de slots. **8192 KB nao existe no mapper padrao**: o registrador e' de 8 bits (256 x 16 KB = 4096 KB); as ROMs de 8192 KB que o usuario lembra sao MegaROMs, outro mecanismo.
- **Feito**: campos `mapper_kb`, `mapper2_kb`, `mapper2_slot` no `MachineConfig`; `SetMapperRam`/`MapperRamKb`; `--ram`, `--ram2`, `--ram2-slot`; menu da janela; comando `mapper`; correcao do `LoadState` (registrador do mapper).
- **Testes**: `msx2test` - todos os tamanhos de 64 a 4096 KB, 512 KB + 1024 KB (mascara de cada um, leitura das portas, estado salvo, boot do BASIC 2.1), recusa de 100/8192/32 KB e de slot ocupado, layout fixado, e o boot do MSX-DOS 2 com 512 KB + 1024 KB.
- **Nao feito**: o MSX-DOS 2 usar de fato a RAM grande (o kernel 2.20 e' que decide; `mapper` mostra o que ele paginou), a leitura das portas com dois mappers vem do primeiro da lista de slots (o hardware real varia), HD.
- Validado: Windows e Linux, `ctest` 100% (34).

## v1.43.21 -- "Yie Ar Kung-Fu: MSX-DOS 2" (Konami) (2026-10-23)

**Pedido do usuario**: com a ROM do MSX-DOS 2 identificada (1.43.20), seguir na implementacao do MSX-DOS 2. Respostas do usuario: o MSX-DOS 2 e' um **cartucho no slot 1** e o `dos: 2` do Obifile deve passar a iniciar o disco com ele; o alvo dele e' MSX2/2+ com 512 KB de RAM ou mais e MSX-DOS 2 (ate' com HD); o MSX1/MSX-DOS 1 fica para projetos esporadicos.
- **Achado na pesquisa (regra do CLAUDE.md)**: o suporte que faltava **ja' existia**. O mapper `msxdos2` (`MEMMAP_MAPPER_MSXDOS2`, adaptado do `RomMSXDOS2` do openMSX) e a interface de disquete bastavam; nunca tinha sido testado com a ROM de verdade porque o repositorio nao a tem. Com o `msxdos22.rom` do Full Set + `emulator/msxdos2.dsk` o boot foi direto: `ver` -> kernel 2.20, `MSXDOS2.SYS` 2.32, `COMMAND2.COM` 2.44; `mkdir sub`, `echo ola > sub\x.txt`, `dir sub`, `type`, `date`. Em MSX1 (sem relogio) o COMMAND2 pergunta a data; na MSX2 nao.
- **O que esta' na release** (so' integracao): `romdb::FindMsxDos2Rom`; comando `dos2`; `--dos2`; item de menu; `BuildResult::dos_version` e o `obirun` com `dos: 2`; `Host::FindDos2Rom/BootDos2`.
- **Testes**: `controltest` (dos2 com host de teste), `romdbtest` (FindMsxDos2Rom), `msx2test` (boot do MSX-DOS 2 com a ROM e o disco reais; pulado se nao existirem).
- **Nao feito**: HD (IDE/SCSI; o alvo do usuario), mapper de RAM maior que 128 KB na MSX2, `asmrun`/F5 de `.com` com o DOS 2 (continua usando `msxdos1.dsk`), disco de sistema do DOS 2 gerado pelo F6 sem o `obirun`.
- Validado: Windows e Linux, `ctest` 100% (34).

## v1.43.20 -- "Yie Ar Kung-Fu: Download e identificacao" (Konami) (2026-10-23)

**Pedido do usuario** (testando o Download de System ROMs): abre uma janela de console e o download roda em segundo plano ate terminar -- queria um indicador/barra de progresso na GUI e no TUI; depois do ZIP descompactado em `dist/roms/*`, aproveitar as **assinaturas do openMSX** para classificar as ROMs no SQLite (ex.: `MSXDOS22.ROM` = MSX-DOS 2), identificando pelo nome ou pelo nome/SHA-1 oficial; e, com essa ROM, seguir para o MSX-DOS 2 (proxima release). Respostas do usuario: assinaturas **baixadas pela janela Downloads/CLI** (nada embutido); o banco ganha nome oficial + papel, hardware, reclassificacao por categoria e um comando/botao Identificar.
- **Achados na pesquisa**: (1) a janela de console era o `_popen("curl ...")`; (2) a janela de status do banco de ROMs ja' existia mas so' mostrava linhas de texto; (3) o openMSX traz 434 SHA-1 distintos de ROMs de sistema em `share/machines` (238) e `share/extensions` (117), cada um com o nome oficial do arquivo e o papel (`<ROM id=...>` ou o nome da extensao); (4) o repositorio do openMSX pesa ~115 MB, entao baixar o ZIP inteiro estava fora: a arvore pela API do GitHub + um unico `curl -K` com ~350 XML.
- **Desenho**: ver `doc/romdb-spec.md`, secao 7b. `CreateProcess` + `CREATE_NO_WINDOW`; andamento pelo tamanho do arquivo contra o `Content-Length`; tokenizador de XML em C; `signatures` + 3 colunas em `roms`.
- **Nao verificado**: a barra e o menu na janela grafica (so' compilam); o download real das assinaturas e do Full Set na sessao grafica.
- Validado: Windows e Linux, `ctest` 100% (34).

## v1.43.19 -- "Yie Ar Kung-Fu: Comandos do VDP" (Konami) (2026-10-23)

**Pedido do usuario**: "alem do screen, voce poderia ter um comando VDP que mostre os valores dos registradores do VDP? talvez ate permitindo alterar? e quando dar screen, alem do estado, poderia mostrar quais os enderecos das tabelas? padroes, formacao, cores, sprites, atributos de sprite, etc?" Respostas do usuario as perguntas de desenho: `vdp` + `screen info`; **dois** comandos de gravacao (um igual ao OUT, outro com o espelho do BIOS); Lua (`msx.vdp`, `msx.screen`) e man/manual.
- **Desenho**: `Machine::WriteVdpRegister` chama `vdp_write_register` (o mesmo caminho da porta 99h); os enderecos das tabelas saem do cache do proprio VDP (`chr_tab`, `col_tab`, `chr_gen`, `spr_tab`, `spr_gen`, ja' calculados de R#2..R#6, R#10, R#11); as cores de sprite do modo 2 ficam 512 bytes antes dos atributos. No MSX1 o limite do registrador e' 7 e a paleta e' fixa.
- **Nao verificado**: os tamanhos das tabelas do SCREEN 3 e dos modos bitmap (nao mostrados); o espelho do BIOS so' vale com o BIOS padrao do MSX.
- Validado: Windows e Linux, `ctest` 100% (34).

## v1.43.18 -- "Yie Ar Kung-Fu: Erro do BASIC no editor" (Konami) (2026-10-23)

**Pedido do usuario**: "da lista qual seria o mais rapido, para fazer ainda hoje" -> "entao vamos no Dignified" (item pendente desde a 1.42.2: "salto automatico lendo a mensagem da tela do MSX depois do F5").
- **Desenho**: `ScreenText(Machine&)` (control) le a tabela de nomes da VRAM pelas variaveis do BIOS (SCRMOD FCAF, TXTNAM F3B3 / T32NAM F3BD, LINLEN F3B0, CRTCNT F3B1); funciona com a maquina parada e so' em SCREEN 0/1. `badig::FindBasicError` procura "<mensagem> in <n>" com a mensagem numa **lista fechada** das mensagens do MSX BASIC/DISK BASIC e o prompt `Ok` nas duas linhas seguintes (assim "Hello in 5" impresso pelo programa nao conta). `EditorApp::RunWatchTick` (chamado todo quadro por `EditorPanel::Tick`, com o editor visivel ou escondido) le a tela a cada 0,3 s, depois de 2 s de boot, exige o mesmo erro em duas leituras seguidas, chama `BasicLineGoto` e pede para a janela voltar (`TakeShowRequest`). Prazo de 180 s.
- **Nao verificado**: a volta automatica da janela ImGui numa sessao grafica real (so' o modelo esta' no teste); erros que o programa imprime em tela grafica (SCREEN 2+) nao sao vistos.
- Validado: Windows e Linux, `ctest` 100% (34).

## v1.43.17 -- "Yie Ar Kung-Fu: Downloads" (Konami) (2026-10-22)

**Pedido do usuario**: "fwMSX-splash.png e arte nossa, NBASIC.BIN e importante, mas pode ser baixado de github.com/Konamiman/MSX/tree/master/SRC/NBASIC [...] que tal criar uma opcao no menu que baixa ROMs do File Hunter, DSK, PDF... tudo no file hunter, e os utilitarios dos sites especificos, fazemos uma opcao que abre uma janela onde o usuario vai poder baixar item por item". Decisoes (AskUserQuestion): janela unica com fontes; busca pelo `allfiles.txt`; NBASIC.BIN baixado sob demanda.
- **Achados na pesquisa**: (1) o File Hunter pede, no `Read Me & Welcome.txt`, que nao se baixe em massa (ban de IP): por isso o desenho todo gira em torno de UM indice e downloads manuais com intervalo; (2) a raiz do site so' responde com User-Agent de navegador (405 com o do curl; o `romdb/net/web.cpp` ja' tratava); (3) o `allfiles.txt` traz 59.237 caminhos, sem tamanho nem data; (4) a API do GitHub devolve licenca `NOASSERTION` para o repositorio do Konamiman: nao ha' base para redistribuir o `NBASIC.BIN`.
- **Desenho**: `dl::Index` (BOM/CRLF/`\`), `dl::Fetch` (temporario, intervalo, `install`), catalogo compilado; a janela e' uma thread de trabalho por vez. O `curl` continua sendo o transporte (`romdb::HttpDownload`). `SafeRelative`/`LocalComponent`/temporario `.part` evitam que um nome do servidor chegue a um comando de shell ou saia da pasta de downloads.
- **Nao verificado**: a janela ImGui compilou mas nao foi aberta numa sessao grafica real; o download real do File Hunter e do GitHub nao esta' no `ctest` (sem rede nos testes): so' o `Fetch` por `file://`.
- Validado: Windows e Linux, `ctest` 100% (34).

---

## v1.43.16 -- "Yie Ar Kung-Fu: Scripts em Lua" (Konami) (2026-10-22)

**Pedido do usuario**: "2" (o spike do Lua, item 2 da lista de pendencias). Decisoes (AskUserQuestion): o Lua vem por FetchContent; o primeiro passo entrega os degraus 1 e 2 enxutos (`source` e a API `msx` com ganchos), nao o editor.
- **Desenho**: `on_change` e' por polling de um byte a cada quadro, e nao um `on_write` de verdade: o `watch` do depurador PARA a maquina quando dispara (nao serve de gancho) e um gancho por escrita no barramento custaria em toda escrita. O tradeoff esta' escrito no `doc/script-spec.md`.
- **Armadilhas**: `luaL_error` usa `longjmp` e pula os destrutores C++, entao as funcoes C da API nao guardam `std::string` vivas quando podem dar erro (`msx.reg` usa buffer fixo); um script pode chamar `lua`/`lua stop` por `msx.cmd`, entao o `Tick` nao segura referencias para dentro da lista de corrotinas e o `stop` pedido de dentro de um script so' vale quando ele volta.
- **Dependencia nova na configuracao**: o primeiro `cmake` baixa o tarball do lua.org (como ja' fazia com SQLite, FTXUI e CLI11).
- Validado: Windows e Linux, `ctest` 100% (33).

---

## v1.43.15 -- "Yie Ar Kung-Fu: Markdown colorido" (Konami) (2026-10-22)

**Pedido do usuario**: "Faca o markdown bonito no TUI tambem, com cores para simular os tipos e tudo mais, permita editar apenas o .MD puro e Visualizar o .MD renderizado, acho melhor do que dividir a tela, salvo se nao der muito trabalho ai deixe como duas opcoes. Faca o ultimo item do E7 para fecharmos este ponto de vez".
- **Preview**: virou um MODO da janela (nao mais um dialogo): `Editor::SetPreview`, `RenderPreview` (grade da TUI) e `EditorPanel::DrawPreviewPane` (ImGui) leem o mesmo `RenderMarkdownStyled`. Para as cores reaproveitei o caminho do destaque de sintaxe: 12 classes novas (`kSynMd*`) com cor na TUI e na janela. Italico/negrito so' mudam de COR (a grade de celulas nao tem atributo de fonte). A edicao e' bloqueada pelo mesmo caminho do modo somente leitura (`Writable()`).
- **Preview dividido**: nao feito. Na TUI exigiria uma segunda arvore de desenho (a TUI e' uma grade de celulas, a janela grafica desenha os paineis sozinha), e o usuario preferiu a alternancia. Fica registrado como opcao futura.
- **Arquivos em partes** (decisao minha entre as duas propostas, pelo "faca o ultimo item"): a variante completa. Seguranca do Save: grava num `.fwtmp` e troca no fim; o `tail` aponta para o original e e' recalculado depois de cada Save; mudanca do arquivo em disco entre o Load e o Load more e' detectada pelo tamanho. O undo e' zerado no Write to cursor / Top of file (as posicoes mudam).
- Validado: Windows e Linux, `ctest` 100%.

---

## v1.43.14 -- "Yie Ar Kung-Fu: Outline e preview" (Konami) (2026-10-22)

**Pedido do usuario**: "pode seguir neste caminho" (Outline e preview de Markdown depois do Document format).
- **Decisao minha (a rever)**: o preview e' texto simples numa lista de rolagem, sem cores nem a tela dividida com atualizacao ao vivo que o SPEC 5.3.6 descreve (F7 alternando edicao / dividido / leitura): assim a TUI e a janela grafica mostram exatamente o mesmo, sem novo widget. O Outline do NE (so' coluna 1) ficou como regra geral e ganhou regras por linguagem.
- **Falta da E7**: Load more / Write to cursor / Top of file (arquivos maiores que a memoria; prioridade baixa do usuario). Design a decidir antes de codar: carregar so' parte do arquivo mexe no Save.
- Validado: Windows e Linux, `ctest` 100%.

---

## v1.43.13 -- "Yie Ar Kung-Fu: Formatar texto" (Konami) (2026-10-22)

**Pedido do usuario**: "Pode incluir o msxdos1 e o msxdos2 eles estao amplamente disponiveis na internet, caso alguem reclame os direitos eu tiro, e coloque no manual que precisa colocar os arquivos na pasta emulator. Dito isso, pode seguir com o E7" (escopo: Document format, Outline, Load more / Write to cursor / Top of file, preview de Markdown).
- **Pacote**: `build.ps1`/`build.sh` copiam `dist/emulator` para o zip/tar.gz; `LICENSE-THIRD-PARTY.md` registra a excecao e a promessa de retirar a pedido.
- **Descoberta**: `Paragraph format` e `Document format` estavam na tabela como "ainda nao" (so' `Center line` existia). `Paragraph format` entrou junto, porque o Document format e' o mesmo reflow sobre todos os paragrafos.
- **Definicao** (nao havia no NE descrito no projeto): a largura da linha e' o campo que comeca na margem esquerda (igual ao `Center line`); o recuo da 1a linha conta dentro dela. Linhas em branco e quebras de pagina separam paragrafos e ficam como estao.
- **Falta da E7**: Outline, Load more / Write to cursor / Top of file, preview de Markdown.
- Validado: Windows e Linux, `ctest` 100%.

---

## v1.43.12 -- "Yie Ar Kung-Fu: Discos do MSX-DOS" (Konami) (2026-10-22)

**Pedido do usuario**: "em resource/msxdos1 e resource/msxdos2 temos os arquivos do MSX-DOS 1 e 2, da para montar o DSK com eles, a partir do executavel, crie uma pasta emulator e voce pode criar os discos por ali".
- **Desenho**: `obi::SetEmulatorDir` (como o `badig::SetLibraryDir`); `LoadSystem` poe o essencial no disco em branco antes dos arquivos da receita. O MSX-DOS 2 completo (HELP 433 KiB + UTILS) so' entra com `dos_tools` para sobrar espaco. A build copia `resource/msxdosN` para `dist/emulator` e roda `fwMSX --obi dosdisk 1|2`.
- **Licenca**: os arquivos do MSX-DOS sao de terceiros (`resource/` e' so' estudo); a pasta `emulator/` fica de fora dos pacotes zip/tar.gz, e o LICENSE-THIRD-PARTY diz isso.
- **Nao verificado**: iniciar o MSX-DOS 2 no emulador (precisa da ROM do kernel DOS2 na maquina).
- Validado: Windows e Linux, `ctest` 100% (33).

---

## v1.43.11 -- "Yie Ar Kung-Fu: Receita de disco" (Konami) (2026-10-22)

**Pedido do usuario**: "apenas o obi com nossas ferramentas, ainda nao esta na hora de mexer com o kizuna, so quero algo facil para ter uma receita que pega o .DMX, o .ASM, um .DAT/.BIN, um .SPRITE, um .SCR, etc.. para gerar um .DSK pronto para rodar no fwMSX". Decisoes (AskUserQuestion): boot por `AUTOEXEC.BAS` gerado **e** opcao `dos: true`; aparecer no console/CLI, no menu Run do editor e como arquivo do `.msxproject`.
- **Achado ao ler o Kizuna**: o `Obifile` dele e' so' a receita; quem trabalha sao `kaji80`/`wirth80`/`dignac`/`musubi`/`hako` e o objeto `.MOB`, que o fwMSX nao tem. Converter so' o `obi` nao montaria nada, entao o `obi` do fwMSX fala com `fwasm`/`badig`/`msxdisk` e mantem so' a sintaxe.
- **Desenho**: `obi::Build` recebe um `Reader` (como o `badig::SourceReader` da 1.43.7): o editor passa buffers abertos + banco do projeto e a build enxerga o que esta' na tela. As mensagens saem estruturadas (`obi::Message`: arquivo, linha) para o F8 do editor.
- **Nao testado**: `dos: true` (falta um `msxdos1.dsk` nos testes); esse caminho so' tem os testes de erro.
- Validado: Windows e Linux, `ctest` 100% (33 testes, com o `obi` novo).

---

## v1.43.10 -- "Yie Ar Kung-Fu: Imprimir em PDF" (Konami) (2026-10-22)

**Pedido do usuario**: "Vamos entao para a E7". Escopo escolhido (AskUserQuestion): imprimir **em PDF reaproveitando o `mamute_pdf`** (nao impressora do sistema); depois Document format, Outline, Load more / Write to cursor / Top of file e preview de Markdown, em versoes seguintes. O `Obifile` parte do Kizuna (Go, `resource/kizuna/pkg/obi`), a converter depois da E7.
- **Desenho**: a paginacao mora no gerador (`BuildPdfLayout`), nao no editor, para o Mamute e o editor compartilharem o PDF; `BuildPdf` antigo ficou intacto. A pagina e' A4 fixa com 9 pt, entao o Page length do NE (62) cabe e qualquer valor maior e' cortado pelo que cabe na folha (o form feed continua mandando).
- Validado: Windows e Linux, `ctest` 100%.

---

## v1.43.9 -- "Yie Ar Kung-Fu: Simbolos do projeto" (Konami) (2026-10-22)

**Pedido do usuario**: "siga para fecharmos E6" (depois dos menus reorganizados).
- **Desenho**: a lista de simbolos e' GLOBAL em `completion.cpp` (como os snippets do usuario) e o `EditorApp` a refaz so' quando o projeto muda (flag `project_words_dirty_`, tratada no fim do `ProjectFlush`), nunca a cada tecla. Para assembly so' contam as definicoes: o resto das palavras de uma linha asm e' mnemonico e registrador, que lotariam a lista.
- **Achado no teste**: o 1o teste de integracao falhou porque a lista era refeita no INICIO do `ProjectFlush`, antes do `Push` que acabara de marcar a lista como suja; passou a ser refeita DEPOIS do laco, entao o fonte recem-salvo ja' conta na tecla seguinte. O segundo erro foi do proprio teste (digitar no inicio do arquivo transformava o rotulo em comentario).
- **Decisao minha (a rever)**: Sprites/Telas/Som ficaram de fora do E6; entram junto com os editores graficos (SPEC 5.4). Um guarda (`WordsGuard`) zera a lista global quando o `EditorApp` e' destruido.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100% (32).

---

## v1.43.8 -- "Yie Ar Kung-Fu: Menus reorganizados" (Konami) (2026-10-22)

**Pedido do usuario**: comecar pelo E6 do editor/IDE e reorganizar os menus, concentrando as opcoes como as IDEs e emuladores modernos. Perguntei (opcoes: 8, 7 ou 6 menus; teclas) e ele escolheu **8 menus modernos** com os atalhos diretos do NE mantidos.
- **Desenho**: so' a tabela `Menus()` mudou (mais traducoes e titulos da ajuda); nenhum comando mudou. A tela tem 25 linhas, e os menus nao tem submenus, entao Edit (15 linhas) e Tools (14) foram montados para caber; por isso as quebras de pagina e as tabulacoes foram para Tools e o Print para File.
- **Letras**: cada item manteve a letra que tinha, para o de-para ser so' trocar o prefixo (B->E, U->R, O->T, P->F); as colisoes em Tools e Edit foram resolvidas com letras novas (Setup T, Expand tabs X, Put in Tabs P, Document format G, DOS shell H, Undo Z, Redo Y).
- **Achado nos testes**: o teste de leitura esperava o 1o item habilitado na posicao 3 (agora 5, Undo/Redo/Copy/Move/Delete sao desabilitados); `RunMenu(3,0)` do teste do painel grafico agora e' `RunMenu(2,0)`.
- **Fica para depois (E6)**: menus Sprites/Telas/Som (os editores ainda nao existem), simbolos do projeto na completacao, `man editor`.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100% (32).

---

## v1.43.7 -- "Yie Ar Kung-Fu: Build lendo do projeto" (Konami) (2026-10-21)

**Pedido do usuario**: fazer o Build e o Dignified lerem do projeto e encerrar a parte de projetos do dia.
- **Desenho**: o banco e' a fonte da verdade (o Sync so' o espelha na pasta). Um gancho de leitura no Badig e no `FileProvider` do montador consulta o projeto primeiro e cai no disco se o projeto nao tem o arquivo. Sem projeto aberto o gancho e' vazio: nada muda para `badigcli`, `asmrun` ou quem nao usa projeto. O gancho e' ligado so' durante a conversao (escopo RAII) para nao vazar.
- **Nao mexido**: o `disk_files` do Dignified (arquivos para o .dsk) continua lendo da pasta; o Run (F5) continua pedindo ao `asmrun` o arquivo gravado em disco, que o Sync mantem igual.
- **Achado no teste**: o F9 do Dignified grava `.amx`/`.bmx` ao lado do fonte; esses arquivos novos fazem o Open do projeto abrir o dialogo de Sync (so' na pasta), entao o teste os apaga ao limpar.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100% (32).
- **Fica para depois (projetos)**: `Obifile` (receita de build; precisa do desenho do que ela guarda).

---

## v1.43.6 -- "Yie Ar Kung-Fu: Historico de ROMs grandes" (Konami) (2026-10-21)

**Pedido do usuario**: subir o limite de 2 MiB para 8 MiB, pois ja existem ROMs desse tamanho. Constante `kRevMaxBytes` em `project.cpp`. Validado: Windows e Linux (WSL), `ctest` 100% (32).

---

## v1.43.5 -- "Yie Ar Kung-Fu: Historico e remocoes" (Konami) (2026-10-21)

**Pedido do usuario**: esgotar a parte de projetos; escolheu *Remocoes propagando* e *Revisoes + nomes com maiusculas* (Build lendo do banco e Obifile ficam para depois).
- **Desenho**: a lapide guarda o HASH do que foi tirado, nao so' o nome: o outro PC so' oferece apagar a copia se ela ainda for igual (sem perder trabalho) e distingue a que mudou. Apagar continua sendo sempre decisao do usuario (padrao do dialogo = manter); `--prune` so' apaga o que e' igual e esta' no historico.
- **Historico no mesmo banco** (nao arquivo a parte): continua sendo UM arquivo para copiar entre PCs. Custo controlado: 10 versoes, teto de 8 MiB por versao (subiu de 2 na 1.43.6).
- **Decisao minha** (a rever): renomear deixa lapide do nome antigo (propaga a troca de nome, mas o outro PC pergunta antes de apagar a copia antiga); o historico nao cobre arquivos grandes.
- **Achado no teste**: `--prune` num PC que nao tem a pasta toda tira do projeto o que "sumiu" da pasta (comportamento que ja' existia para `DiskDeleted`) e agora deixa lapide; o teste de dois PCs passou a copiar a pasta inteira.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100% (32).

---

## v1.43.4 -- "Yie Ar Kung-Fu: Menu de contexto" (Konami) (2026-10-21)

**Pedido do usuario**: "Menu de contexto na arvore, vamos esgotar a parte de projetos antes de irmos para os outros itens".
- **Desenho**: uma so' implementacao (`EditorApp::ProjectDo`) para as duas interfaces: o popup do ImGui chama `ProjectAct`, o botao Actions... do TUI abre a lista e chama o mesmo `ProjectDo`. Renomear = mover a copia solta + `Drop` do nome antigo + `Push` do novo (a base do `.msxsync` acompanha).
- **Decisao minha** (a rever): *Restaurar a copia da pasta* sobrescreve sem confirmar (e' o que o nome diz; abas abertas sem alteracoes recarregam, com alteracoes avisam); renomear pasta nao existe.
- **Falha minha (de novo)**: um build de teste saiu com a versao ainda em 1.43.3 e sobrescreveu o `dist/fwMSX-1.43.3.zip` local (nao rastreado; o tag 1.43.3 no GitHub nao e' afetado).
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100% (32).

---

## v1.43.3 -- "Yie Ar Kung-Fu: Arvore no TUI" (Konami) (2026-10-21)

**Pedido do usuario**: "Arvore na TUI por hora".
- **Desenho**: sem painel lateral (a grade de 80 colunas do TUI nao tem lugar para ele): a arvore mora no dialogo *Files...*, que ja' existia. Como o dialogo fecha no Enter, recolher/expandir o reconstroi no mesmo item (`ProjectTreeEnter`), com as pastas recolhidas guardadas em `tree_folded_`.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100% (32).

---

## v1.43.2 -- "Yie Ar Kung-Fu: Projeto lembrado" (Konami) (2026-10-21)

**Pedido do usuario**: "Lembrar do ultimo projeto e arvore de arquivos e nome do projeto na barra de status".
- **Decisao**: o ultimo projeto vai num arquivo proprio (`fwedit.project`), nao no `fwedit.cfg`: a configuracao so' e' gravada por pedido ("Save configuration"), e abrir um projeto e' um fato que deve valer na sessao seguinte sozinho. Fechar de proposito esquece; New/Open (troca) nao.
- **Decisao minha** (a rever): ao reabrir na partida so' avisa, nao abre a lista do Sync (nao interromper quem so' quer editar).
- A arvore existe so' na janela grafica (no TUI o *Files...* ja' lista); `ProjectOpenFile` e' reaproveitado, entao clicar tem o mesmo comportamento do *Files...*.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100% (32).

---

## v1.43.1 -- "Yie Ar Kung-Fu: Projeto no editor" (Konami) (2026-10-21)

**Pedido do usuario**: "vamos para o editor" (fatia 2 do `.msxproject`); migracao dos projetos do PaleoBasic/msxIDE dispensada (serao abandonados); renomear o fwMSX fica para depois.
- **Desenho**: o editor continua trabalhando nos arquivos da PASTA (modelo, Build e Dignified nao mudam); o projeto acompanha por `Project::Push` depois de cada gravacao (contador `Editor::SaveCount` x `ProjectSeen`, conferido depois de cada tecla/clique/dialogo). Assim todos os caminhos de salvamento (menu, atalho, autosave, F5/F9) passam pelo mesmo gancho e o modelo nao ganhou dependencia do projeto.
- **Decisoes minhas** (a rever): a lista do Sync abre sozinha ao criar/abrir um projeto com diferencas; arquivo salvo na pasta entra no projeto sozinho (sem perguntar); *Open project...* propoe o ultimo projeto; so' arquivos de texto abrem pelo *Files...*.
- **Achado**: o editor deixa uma copia de seguranca (`READ.{E`) ao lado do arquivo; entrava no projeto como "novo na pasta". `IsIgnored` agora a recusa (e o `Sync` tambem).
- **Falha minha**: o primeiro build desta rodada saiu antes de subir a versao e sobrescreveu o `dist/fwMSX-1.43.0.zip` local (nao rastreado; o tag 1.43.0 no GitHub nao e' afetado).
- **Nao feito**: lembrar o ultimo projeto entre sessoes (arquivo de configuracao do editor), arvore de arquivos na janela grafica, barra de status com o nome do projeto, Build/Dignified lendo do banco, `Obifile`.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100% (32).

---

## v1.43.0 -- "Yie Ar Kung-Fu" (Konami) (2026-10-21)

**Pedido do usuario**: o `.msxproject` como UM arquivo SQLite com todo o projeto + copias soltas na pasta, extraidas ao copiar para outro PC, com sugestao do lado mais novo; facil de copiar entre PCs e de mover para o GitHub.
- **Pesquisa antes de codar** (apresentada ao usuario): "o mais novo ganha" pela data engana (git clone, zip, relogios); SQLite binario no git nao tem diff/merge e incha; WAL deixa `-wal/-shm` e copiar so' o arquivo perde gravacoes. O usuario escolheu: hash de base + perguntar, pasta + manifesto no git, journal tradicional.
- **Achado de desenho**: o hash de base NAO pode ficar no banco (copiar o `.msxproject` para outro PC faria a pasta nova parecer "tudo apagado" e o sync apagaria o projeto); fica no `.msxsync`, dentro da pasta, com o id do projeto. Sem base, o que difere vira conflito e nada e' apagado.
- **Regras de seguranca de dados**: nunca se apaga copia solta; o que sumiu da pasta so' sai do projeto com `--prune`; conflito nao e' resolvido sozinho.
- **Escolhas minhas** (a rever): so' a configuracao do manifesto e' lida no import (o sync a regrava a partir do banco); remocoes em outro PC nao propagam; tipos por extensao em tabela fixa (C); a pasta padrao e' a do `.msxproject`.
- **Nao feito** (proximas fatias): integracao com o editor (abrir/salvar pelo projeto, arvore, dialogo de conflito), build/Dignified lendo do projeto, `Obifile`, migracao dos projetos do PaleoBasic/msxIDE, `revisions`.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100% (32).

---

## v1.42.2 -- "Road Fighter: Erros de execucao no fonte" (Konami) (2026-10-21)

**Pedido do usuario**: "levar erros de execucao do BASIC de volta a linha do .dmx" (item em aberto da 1.42.1).
- **Desenho**: o original faz isso no openMSX lendo a saida do emulador; aqui o fwMSX nao tem canal de texto da tela para o editor, entao o caminho e' o mais simples e robusto: a tabela de linhas (a do `-lnr`) agora vem sempre no `Result` e o editor a guarda da ultima conversao; o usuario informa a mensagem (ou o numero) em *Montar > BASIC line...*.
- **Nao feito (decisao a rever)**: ler a mensagem direto da tela do MSX depois do F5 e saltar sozinho; precisaria de um gancho que leia a VRAM/nome-table da maquina.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100%.

---

## v1.42.1 -- "Road Fighter: NestorBASIC e arquivos de apoio" (Konami) (2026-10-20)

**Pedido do usuario**: "refinar o Dignified (NBasic e arquivos de apoio sao os mais uteis para rodar programas reais)".
- **Achado que mudou o desenho** (pesquisa antes de codar): no `badig.py` original as funcoes de um `include` NAO valem no arquivo principal (namespaces separados: o mesmo erro `Function not defined` sai do original e do clone). O `nbasic.dmx` do msxIDE depende de o include ser TEXTUAL (o compilador do msxIDE e' outro). Solucao: um include que resolve na **biblioteca do fwMSX** (`badig/`) e' emendado como tokens, antes do parser, no namespace do programa; includes do usuario ficam como o original. O desvio fica restrito a' biblioteca.
- **Poda**: o `nbasic.dmx` tem ~90 funcoes; so' as chamadas ficam (fecho transitivo) e os comentarios da biblioteca saem, senao o programa estoura a memoria do BASIC.
- **Atencao ao fluxo**: as funcoes viram `GOSUB` e o codigo cai onde o `include` esta': o include tem que ficar depois do `END` (o snippet `nbload` ja' faz assim).
- **Escolhas minhas** (a rever): os nomes dos arquivos de apoio sao os da pasta do fonte, em maiusculas, 8.3 estrito (sem truncar); com NBASIC.BIN procura na pasta do fonte e depois em `badig/`; o `NBASIC.BIN` (58 KB, do pacote do Nestor ja' presente em `resource/paleobasic`) vai versionado em `src/badig/lib`.
- **Nao feito**: NestorBASIC rodando de verdade no emulador (precisa de MSX2 com mapper de RAM e DOS; so' o disco e a conversao sao testados); erros de execucao do BASIC -> linha do `.dmx`; Basic DignifieR; projeto `.msxproject`.
- Validado: Windows (`build.ps1`) e Linux (WSL), `ctest` 100%.

---

## v1.42.0 -- "Road Fighter" (Konami) (2026-10-20)

**Pedido do usuario**: colocou o Basic Dignified Suite em `resource/basic-dignified` e o msxIDE em `resource/msxide`: "nao vamos usar o badig em python, vamos fazer um clone nosso nas nossas linguagens do projeto. Quero o programa processando o arquivo DMX e gerando o BMX tokenizado e processado [...] procure recriar a tela de configuracao [...] Importante e tokenizar o arquivo ASCII no final".
- **Pesquisa antes de codar** (regra do CLAUDE.md): o `compiler.bas` do msxIDE e' uma reescrita por linhas e parcial (sem remtags, toggles, `true`/`false`, compostos); o `badig.py` v2.2.1 e' completo (Lexer + Parser em 5 passadas + modulo MSX + tokenizador). O usuario escolheu (AskUserQuestion) portar o **original**, guardar a configuracao num `badig.cfg` global (mais as remtags do fonte) e executar por **disco temporario + RUN**.
- **Como ficou igual**: o original e' um lexer por prefixo que cresce enquanto algum grupo regex casa o prefixo inteiro; `Description::Classify` faz o papel do `commands.match` com as listas de palavras e padroes escritos a mao (nenhum `std::regex` no caminho quente). Os tokens sao compartilhados entre as listas (`shared_ptr`), `Position.copy()` soma 1 a coluna, e as quirks viram requisito: o `PROGRAM` final vira `zz`, os nomes curtos descem de `zz` a `aa`. No tokenizador o texto hexadecimal do original foi mantido (a aritmetica de digitos fica igual) e so' as contas dos reais usam `strtod`/`pow`/`printf("%.*f")` como o Python faz com `int * float` e `'%.*f'`.
- **Divergencias deliberadas**: tokenizar e' padrao; `.amx` sempre CRLF; so' o sistema MSX; `exit` fora de loop avisa em vez de quebrar; colunas do `-tr` contam caracteres.
- **Escolhas minhas** (a rever): o disco de execucao leva so' o `.bmx` e o `AUTOEXEC.BAS` (sem os arquivos de apoio da pasta); o nome no disco e' o nome do fonte em 8 caracteres maiusculos; o `badig.cfg` fica ao lado do executavel (como o `fwedit.cfg`); a tela de configuracao tem duas paginas porque cabe nos 80x25 da TUI; `Montar > Messages...` deixou de ter separador acima.
- **Nao feito**: Basic DignifieR (classico -> Dignified) e o tokenizador inverso; NBasic do msxIDE (`include "nbasic.dmx"` e a rotina de carga); copiar arquivos de apoio para o disco; levar os erros de execucao do BASIC de volta a' linha do `.dmx`; projeto `.msxproject`.
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 100%; captura da janela real (`fwMSX.exe --edit t.dmx`, F9 sobre um `goto {nowhere}`) e do `fwMSXc.exe --edit --dump` com a tela de configuracao; `controltest`: o programa convertido roda no DISK BASIC e imprime `badig run ok` e `6`.

---

## v1.41.9 -- "Hyper Sports: Montar e executar" (Konami) (2026-10-20)

**Pedido do usuario**: "sim pode seguir por a E6". A E6 do `editor-spec.md` (Integracao) e' grande; esta versao entrega a parte que usa o que o fwMSX ja' tem (fwasm, `asmrun`, `man`) e deixa o projeto `.msxproject` para a proxima.
- **Modelo**: o editor NAO conhece a maquina nem o `man`: `EditorApp::SetRunHook` (recebe o caminho do `.asm` gravado, devolve "" ou a mensagem de erro) e `SetHelpHook` (palavra -> texto). Quem liga: `emu_window.cpp` (Run = `ctl_commander.Execute("asmrun ...")`) e `editor_host.cpp` (`AttachManHelp`, so' no executavel). Os testes usam ganchos falsos.
- **Montar o buffer, nao o disco**: `fwasm::Assemble(nome, texto, ...)` com um provedor de arquivos da pasta do fonte. O fwasm PARA no primeiro erro (como o asMSX), por isso a lista tem quase sempre um erro e varios avisos.
- **Mensagens**: `BuildMsg{file, line, warning, text}`; `ed_build_msg` (C) reconhece "arquivo, line N: ..." procurando o ULTIMO ", line N:" (o nome do arquivo pode ter virgula) e distingue "Warning:". Mensagens gerais ("Parsing error", "only 2 out of 4 megaROM pages") ficam na lista sem linha.
- **Escolhas minhas** (a rever): `F9`/`F5`/`F8`/`Shift-F8`/`Ctrl-F1` como teclas (nao ha' NE para copiar; `Ctrl-F1` depende do terminal entregar F-keys com Ctrl, a janela entrega sempre; o menu Help > On word e' a alternativa); o Run grava o arquivo sem refazer o `.bak`; a janela do editor embutido se esconde ao executar; "Build" so' monta Assembly (BASIC nao tem o que montar; o MSXBAS2ROM/NestorBASIC/Kizuna entram com o projeto).
- **Nao feito da E6**: projeto `.msxproject` (SQLite) com receita/`Obifile`; simbolos do projeto no completar; menus Sprites/Telas/Draw/Som/Musica (dependem dos editores visuais); `man editor`.
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 30/30; captura da janela real (`fwMSX.exe --edit t.asm`, F9 leva o cursor ao erro) e do `fwMSXc.exe --edit --dump` com Ctrl-F1 abrindo a pagina de `LEFT$`.

---

## v1.41.8 -- "Hyper Sports: Completar e snippets" (Konami) (2026-10-20)

**Pedido do usuario**: "pode passar para a fase E5" (completar e snippets) e "precisa ter numeracao de linhas nos editores, coisa que o NE nao possuia".
- **Modelo**: `completion.h/.cpp` (sem estado: candidatos, snippets, arquivo do usuario, expansao) e `editor_complete.cpp` (a lista aberta, aceitar, pontos de parada, a margem de numeros). `Lang::Words()` entrega as palavras de cada linguagem (a BIOS em maiusculas; o BASIC em maiusculas, com o `$`). Os pontos de parada do snippet acompanham as edicoes (`AdjustForInsert/Delete`).
- **Teclas**: `Alt-/` (como o dabbrev do Emacs; funciona no terminal) e `Ctrl-Space` (so' na janela: no terminal Ctrl-Space e' o mesmo byte do Ctrl-2, que no NE e' "Divider up"). Dentro da lista: Cima/Baixo/PgUp/PgDn, Enter/Tab, Esc.
- **Numeros de linha**: `Settings.line_numbers` (padrao ligado), `Editor::Gutter()` na grade; a janela le' a mesma opcao e pede `SetGridGutter(false)` porque desenha a margem dela. Os testes que conferem as colunas do NE 2 e as coordenadas do mouse a partir da coluna 0 desligam a opcao.
- **Escolhas minhas** (a rever): so' os pontos `$1..$9` e `$0` (sem valor padrao `${1:x}` e sem espelhamento: dois `$2` iguais param so' no primeiro); o gatilho digitado por inteiro ainda aparece na lista (e' assim que se descobre que ele existe); a palavra sob o cursor nao e' candidata de si mesma; os snippets de fabrica sao poucos e de MSX (`for`, `scr5`, `sprite`, `rom`, `chput`, `wrtvdp`...); completar sozinho vem desligado.
- **Nao feito**: completar com os simbolos de outros arquivos do projeto (depende do projeto `.msxproject`, fase E6); menu para listar/inserir snippet; rolagem na lista (mostra 8 e sobe o item escolhido).
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 30/30; captura da janela real com a lista aberta sobre o texto e a margem de numeros.

---

## v1.41.7 -- "Hyper Sports: Kun BASIC" (Konami) (2026-10-20)

**Pedido do usuario**: o "Turbo Basic" nao e' o da Borland, e' o MSX-BASIC KUN ("uma especie de compilador on the fly de basic para assembly"; poucos extras: `call turbo on`, `call turbo off`); manual indicado. Um subagente leu a pagina e devolveu a sintaxe (usada como DADOS; nada do texto do manual foi copiado).
- **Do manual**: `_TURBO ON/OFF` e `CALL TURBO ON/OFF` (bloco, sem aninhar, uma instrucao por linha), `_TURBO ON(A,C())` (variaveis comuns), `_RUN`/`CALL RUN`, remtags `'#I` `'#C+/-` `'#N+/-`, `@linha`, a lista de comandos nao compilaveis e a de limitados.
- **Implementacao**: `BasicKind::Kun` em `syntax_langs.cpp`; `MultiLine()` verdadeiro (o bloco atravessa linhas: `HlState.mode == 1`); `unsupported_` (tabela) + as regras de `CALL`, `PRINT#`/`INPUT#` e `USING`; nova classe `kSynError` (vermelho, negrito) na TUI e na janela; a deteccao de `.bas` por conteudo (`_turbo `, `call turbo`).
- **Escolhas minhas** (a rever): so' a lista de comandos NAO compilaveis e' destacada (a de "limitados", com restricoes de parametros, nao); dentro do bloco `DEFFN`, `KEY LIST`, `PUT KANJI` e `LINE INPUT#` nao sao marcados (o primeiro e' `DEF` + `FN`, os demais tem 2-3 palavras); `ON INTERVAL GOSUB` tambem nao.
- Corrigido junto: no BASIC `#1` (numero de arquivo) era pintado como hexadecimal; agora so' `&H`/`&B`/`&O` e decimais sao numeros.
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 30/30 nos dois; captura da janela com um `.kun` (OPEN/USING/CALL em vermelho dentro do bloco).

---

## v1.41.6 -- "Hyper Sports: Destaque de sintaxe" (Konami) (2026-10-20)

**Pedido do usuario** (fase E4 do editor): destaque de sintaxe para MSX-BASIC, `.BAT`/`.BTM`, `.ASM`, `.PAS`, `.BAS`, `.DMX`, `.AMX` e assim por diante; Basic Dignified com compatibilidade total; MSXBAS2ROM, NestorBASIC e Turbo Basic/Basic Kun como dialetos.

- **Arquitetura**: `ed::Lang` (syntax.h): uma linguagem recebe UMA linha e o estado da anterior (`HlState.mode`: comentario de varias linhas, bloco, cerca de codigo) e devolve `Span{de, ate', classe}`; as classes (`SynClass`, 13) viram estilos de celula (`SynStyle`) na TUI e cores do tema na janela.
  O editor (`Editor::HighlightLine`) guarda o estado do INICIO de cada linha so' ate' onde ja' foi pedido e o invalida quando o texto muda (`Revision`); linguagens sem estado (BASIC, Batch) nem varrem as linhas de cima. A linguagem do documento (`Language`, `SetLanguage`, `DetectLanguage`) vem do nome/inicio do arquivo e a escolha manual persiste.
- **Dados x codigo**: as palavras sao tabelas (`WordTable`: hash NASM + comparacao) preenchidas de listas; MSX-BASIC/CALL/MSXBAS2ROM/NestorBASIC vem do man (`syntax_words_gen.inc`), a BIOS vem de `src/asm/cpp/bios_tables.inc` (a tabela do fwasm: o editor e o montador concordam), o resto esta' em `syntax_langs.cpp`.
- **Achados**: (1) o tokenizador do MSX-BASIC acha palavras DENTRO de nomes (TOTAL = TO + TAL); o editor fatia assim so' corridas todas em MAIUSCULAS (listagens antigas `FORI=1TO10`) e nunca palavras em minusculas ou mistas; (2) `[pk][poke 100,[10]]` do Dignified tem colchetes aninhados: o varredor conta a profundidade;
  (3) o `.bas` pode ser MSX-BASIC, MSXBAS2ROM ou NestorBASIC: o inicio do texto decide (`FILE "..."`, `CMD <cmd estendido>`, `NB_`); (4) `ImGui::CalcTextSize("M")` arredonda PARA CIMA em pixels inteiros, o que desalinha trechos posicionados por celula.
- **Escolhas minhas / lacunas** (a rever): a lista de palavras do **Turbo Basic / Basic Kun** e' inicial (so' `while wend repeat until do loop exit turbo`) -- o usuario precisa informar a sintaxe verdadeira; o `.bmx` (tokenizado) nao tem destaque (e' binario); NestorBASIC nao tem lista de palavras alem das funcoes `NB_` e das extensoes CALL; no Markdown nao ha' destaque de HTML nem de tabelas;
  as cores da TUI usam 16 cores do terminal (os esquemas LCD/mono ganharam cores de destaque, que o NE original nao tinha); a barra de estado da TUI nao mostra a linguagem (so' `View > Language...` mostra a atual).
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 30/30 nos dois; captura da janela com `.asm`, `.bas` e `.pas`.
- **Fora de escopo / proximos**: E5 completar e snippets (as palavras e a BIOS destas tabelas alimentam o completar), E6 integracao (Executar no MSX, erros -> linha, projeto), E7 (Paragraph/Document format, Print, Outline, Load more...); a sintaxe do Turbo Basic/Basic Kun; `.bmx` como visualizacao.

---

## v1.41.5 -- "Hyper Sports: Autosave do editor" (Konami) (2026-10-20)

**Pedidos do usuario**: (1) "Sem Nome" nao agrada: **noname01**, e as outras abas noname02, noname03...; (2) fechar sem perguntar esta' ok (editores modernos fazem isso); (3) antes da E4, uma opcao de configuracao com "a cada quantos minutos o editor salva automaticamente o seu conteudo em disco, 0 nao salva periodicamente".
Pergunta feita (AskUserQuestion) porque "salvar o conteudo" tem dois sentidos: o usuario escolheu **os dois, escolhendo em Options**.

- **`EditorApp::AutosaveTick(now)`**: `autosave_minutes` (0..1440) em `ed::Settings`; o primeiro quadro so' arma o relogio; ao mudar o intervalo recomeca a contar; passado o prazo chama `AutosaveNow` (sessao se `SetSessionPath`, e com `autosave_files` os arquivos com nome, alterados e nao somente-leitura). Tempo em segundos de qualquer relogio crescente (testavel sem esperar).
- **Copia `{`**: `Editor::Save(err, make_backup)`; o autosave faz a copia so' na primeira gravacao (`AutosavedOnce`), um Save manual a refaz (e zera a marca). Sem isso o `{` viraria "a versao de 2 minutos atras" e nao o original.
- **`noname01..`**: `Editor::Title()` (nome do arquivo, ou o nome automatico, ou "Untitled") e `EditorApp::EnsureUntitled()` (menor numero livre entre as abas sem nome; roda em `NewTab`/`RemoveTab`/`OpenTab`/`LoadSession`/`SetGuiMode` e a cada quadro do painel). Aba, titulo da janela e cabecalho do painel usam `Title()`. Sessao: nova linha `UNT <nome>` (opcional ao ler: as sessoes da 1.41.4 abrem).
- **Dialogo**: Editor setup ganhou o campo numerico (id 12) e a caixa (id 13); a TUI mostra o mesmo dialogo em ingles; a GUI traduz ("Salvar automaticamente a cada (minutos, 0 = nao):").
- **Relogio**: emulador chama `editor_panel.Tick()` todo quadro (editor visivel ou escondido); editor standalone idem; TUI: uma thread so' posta `Event::Custom` a cada 10 s e o tratamento roda na thread da interface.
- **Escolhas minhas** (a rever): o relogio so' conta enquanto o programa esta' aberto (fechar e reabrir reinicia a contagem); o autosave de arquivos nao pergunta se o arquivo mudou no disco por fora (sobrescreve); a sessao do standalone e a do emulador sao o MESMO arquivo (`fwedit.session`).
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 29/29 nos dois; captura da caixa Editor setup na GUI.

---

## v1.41.4 -- "Hyper Sports: Reabrir o editor" (Konami) (2026-10-20)

**Relato do usuario**: o editor aberto pelo menu do fwMSX, fechado e reaberto, abria com o texto anterior mas travado (sem teclado, menus sem efeito, impossivel fechar). Ele tambem quer MANTER o ultimo buffer ("o programador esta trabalhando direto no MSX e quer fazer anotacoes ... para salvar so' no final"), desde que a reabertura deixe continuar editando com os menus funcionais.

- **Diagnostico**: popup modal "fantasma" do ImGui. `OpenPopup`/`BeginPopupModal` mantem o estado na PILHA DE POPUPS do ImGui, nao na janela; quando a janela do editor deixou de ser submetida (escondida) com o dialogo aberto, o modal ficou na pilha e, ao voltar, bloqueava foco/mouse. Reproduzivel: abrir, digitar, X, "Nao salvar", reabrir.
- **Correcao**: `DrawMenuPopup` e `DrawDialog` usam `ImGui::Begin` de janelas comuns (`SetNextWindowFocus` ao abrir; fechamento por clique fora implementado a mao). Sem popup nao ha' estado fora do painel. **Armadilha achada no caminho**: `IsWindowHovered()` chamado DEPOIS de desenhar o item devolve falso no frame do clique (o item ja' e' o `ActiveId`);
  e' preciso `ImGuiHoveredFlags_AllowWhenBlockedByActiveItem` -- sem isso o menu fecha antes de executar o item.
- **Esconder, nao sair**: `EditorApp::SetEmbedded(true)` faz `Cmd::Exit` pedir para esconder (`TakeHideRequest`); `DrawWindow` cancela dialogo/menu e esconde. Os arquivos e o texto ficam. O editor standalone nao e' "embutido" e segue perguntando.
- **Sessao** (`EditorApp::SaveSession/LoadSession`, formato texto `FWEDIT-SESSION 1`: por aba o nome, alterado?, CRLF, cursor e o TEXTO): grava ao esconder e ao fechar o emulador; restaura no inicio so' se o editor ainda esta' vazio; abas sem alteracoes sao relidas do disco. Sem nada util, apaga o arquivo.
- **Escolhas minhas** (a rever): o texto de abas "sem nome" e' mantido para sempre ate' o usuario fechar a aba (nao ha' limite nem aviso); a sessao fica em `fwedit.session` ao lado do executavel (nao por usuario); fechar o emulador nao pergunta pelo texto do editor (ele volta na proxima vez).
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 29/29 nos dois; simulacao de mouse/teclado numa janela real (`tools/wincap` + clique por `mouse_event`).

---

## v1.41.3 -- "Hyper Sports: Editor GUI" (Konami) (2026-10-20)

**Correcao de interpretacao do usuario sobre a fase E3**: "eu quero que o Editor Janela/Windows/GUI seja um programa realmente GUI, integrado inclusive ao menu do fwMSX, ... com a interface igual ou similar graficamente ao fwMSX. Nao quero o mesmo editor, quero apenas os mesmos atalhos e opcoes do menu."
A 1.41.2 desenhava a grade de caracteres do terminal numa janela (nao era isso). A 1.41.3 a substitui por um componente grafico de verdade.

- **Decisoes do usuario** (AskUserQuestion): (1) janela flutuante dentro do fwMSX; (2) menus do NE numa barra de menus PROPRIA do editor (do fwMSX entra so' o item "Editor"); (3) abas, com divisao opcional em 2 paineis; (4) tema do fwMSX, fonte monoespacada, textos em portugues.
- **Arquitetura**: `ed::gui::EditorPanel` (src/editor/gui/editor_panel.*) LE o estado do `EditorApp`/`Editor` e DESENHA com widgets do ImGui. O texto, os comandos, os atalhos e os dialogos continuam como DADOS no modelo (nada foi reescrito); o painel traduz teclado/mouse para o modelo e os dados dos dialogos (`DialogBox`) para widgets.
  Dialogos: o `DialogBox` (widgets em posicoes de celula) e' desenhado com widgets reais em posicoes escaladas (`Input` e' um campo proprio com cursor/selecao, `Radio`/`Check`/`Button`/`List` sao widgets do ImGui); o teclado dos dialogos segue as mesmas regras da TUI (Tab, Enter, Esc, a inicial do botao).
  Menus: o estado do menu aberto mora no `EditorApp` (setas, letra do item, Alt+letra) e o painel so' mostra o popup; passar o mouse troca de menu.
- **Texto**: rolagem do ImGui (`SetNextWindowScroll` quando o MODELO move a vista; o modelo recebe `SetTopLine/SetLeftCol` quando o usuario rola), largura do conteudo pelo maior comprimento de linha (cache por `Revision`), linhas desenhadas so' no intervalo visivel, numeros de linha fixos a esquerda, selecao/cursor/parentese no tema.
- **Integracao no emulador** (`emu_window.cpp`): menu "Editor", janela flutuante, `g_input.ed_capture` (o editor em foco toma o teclado: o MSX nao recebe as teclas). Conferido por captura: a janela abre sobre a tela do MSX com o menu "Editor > Mostrar o editor".
- **Fonte**: Consolas/Courier New (Windows), DejaVu/Liberation Mono (Linux) ou a embutida do ImGui. Fonte do texto: 17 px.
- **Escolhas minhas** (a rever): as abas ficam em cima e os paineis lado a lado (nao empilhados como no NE); numeros de linha ligados por padrao (sem opcao de menu ainda); os menus mostram "Alt+F, N" quando nao ha' atalho direto; o menu de contexto usa a area de transferencia do sistema; fechar o X da janela flutuante pergunta
  pelos arquivos alterados (como Arquivo > Sair) e so' esconde a janela, mantendo os arquivos abertos.
- **Conferido**: o editor standalone abre README.md/CLAUDE.md lado a lado (captura), o menu Arquivo e o dialogo Abrir arquivo (lista de arquivos e pastas, campo com mascara) e a janela flutuante no emulador. **Nao conferido**: mouse (clicar/arrastar/duplo clique), Alt sozinho, AltGr, colar do sistema, Linux em janela (so' compilou).
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 29/29 nos dois.
- **Fora de escopo / proximos**: E4 destaque de sintaxe, E5 completar e snippets, E6 integracao (Executar no MSX, erros -> linha, projeto, menus Montar/Sprites/...), E7 (Paragraph/Document format, Print, Outline, Load more...); opcoes de visual (numeros de linha, tamanho da fonte, divisao empilhada); icone; `man editor`.

---

## v1.41.2 -- "Hyper Sports: Editor em janela" (Konami) (2026-10-20)

**Pedido do usuario**: "fase E3": o mesmo editor na janela (GUI), com os mesmos recursos e atalhos da TUI. O usuario testou a TUI (1.41.1) no terminal e disse que "funcionou bem".

- **Arquitetura**: `src/editor/gui/editor_window.cpp` (ImGui/GLFW) so' faz tres coisas: (1) le' o teclado/mouse do ImGui e chama `EditorApp::Key/Text/MouseDown/...`; (2) pede a grade com `Render`; (3) desenha as celulas com `ImDrawList` (fundo por trecho de mesmo estilo, texto por trecho, ornamentos a mao). Nenhuma regra do editor mora na janela.
- **Teclas**: `GuiKeyName(tecla, ctrl, alt, shift)` (editor_gui_input.cpp, testavel): letra/digito/pontuacao SEM Ctrl/Alt sao texto (vem da fila de caracteres do ImGui, que ja' trata Shift, acento e teclado internacional); Enter, Tab, setas, Home/End, PgUp/PgDn, Ins, Del, Backspace, Esc e F1-F12 sempre sao tecla.
  Alt sozinho: apertar e soltar sem outra tecla nem clique = F10 (abre/fecha o menu). Ctrl+Alt = AltGr: so' texto.
- **Roteamento** `--edit`/`--view` (main.cpp): `--gui`/`--tui` explicitos; sem eles, no Windows o subsistema do executavel decide (`IsConsoleExe`: le' o cabecalho PE; o `fwMSXc.exe` e' o mesmo programa com o subsistema "console"); no Linux o padrao e' o terminal. `--dump` e' sempre do terminal.
  Sem a GUI compilada (`FWMSX_MSXDISK_GUI=OFF`) entra um stub que manda usar o terminal.
- **Escolhas minhas** (a rever): fonte de 18 px e celula = largura de "M" x 1,2 da altura; piscar o cursor a cada 0,5 s; roda do mouse rola 3 linhas; `Display lines` nao tem efeito na janela (o tamanho e' o da janela); o icone da janela ainda nao foi posto; sem zoom da fonte (Ctrl+roda) por enquanto.
- **Conferido**: a janela abre e desenha o menu File como o NE (captura com `tools/wincap/wincap.ps1`, que manda a tecla F10); o resto da interacao e' o mesmo codigo ja' testado (`editorui`). **Nao conferido em janela**: mouse, Alt sozinho e AltGr (so' o modelo e a traducao de teclas foram testados).
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 29/29 nos dois.
- **Fora de escopo / proximos**: E4 destaque de sintaxe, E5 completar e snippets, E6 integracao (Executar no MSX, erros -> linha, projeto), E7 (Paragraph/Document format, Print, Outline, Load more...); `edit`/`view` dentro do console e da TUI de menus; zoom da fonte; icone; `man editor`; perfil fwEditor.

---

## v1.41.1 -- "Hyper Sports: Editor na TUI" (Konami) (2026-10-20)

**Pedido do usuario**: "fase E2" do editor de textos (`doc/editor-spec.md`): a tela do Norton Editor 2 na TUI com menus, dialogos, 2a janela e mouse, e os comandos `edit`/`view`. Replicar o MAXIMO do original (62 telas de referencia em `images/`).

- **Arquitetura**: `ed::EditorApp` (src/editor/cpp/editor_ui.*, editor_dialogs.cpp) sobre o modelo `ed::Editor`: recebe teclas por nome e mouse, devolve uma grade. O FTXUI so' entra em `editor_tui.cpp` (entrada do terminal -> teclas/mouse; grade -> cores). Tudo o resto e' testado sem terminal.
- **O modelo ganhou**: `RenderBody`/`RenderStatus` (varias janelas), `SetViewSize`, `ScreenToPos`/`SelectBlock`/`ScrollView` (mouse), `ReplaceHere` (busca interativa), parentese que casa realcado, `SetName`; o menu **F1=Help** entrou em `Menus()`.
- **Dialogos como dados**: `Widget` (Label, Input, Radio, Check, Button, List) em posicoes absolutas, `DialogBox::Key/ClickAt`; cada caixa do NE tem uma funcao `Build*`; o desenho e' uma borda dupla com sombra. Os dialogos de arquivo listam o diretorio (mascara com `*` e `?`, `[sub]`, `..`, unidades `[-C-]` no Windows).
- **Teclas do terminal**: convencoes do console decididas aqui: 0x08 = Ctrl-Backspace (desindentar), 0x7F = Backspace, 0x00 = Ctrl-2, 0x1D = Ctrl-]; ESC + caractere = Alt; sequencias `ESC [ 1 ; m X` e `ESC [ n ; m ~` do xterm para setas e Home/End/Ins/Del/PgUp/PgDn/F1-F12 com Shift/Alt/Ctrl.
- **Escolhas minhas** (a rever): a ajuda e' o nosso texto (a do NE e' da Symantec); o Replace pergunta ocorrencia por ocorrencia (o NE faz "optionally replaces"); Open com arquivo alterado pergunta antes; a mensagem passageira cobre o nome do arquivo na barra de estado; "Display lines" so' limita a altura usada (o terminal manda no tamanho); os textos da interface ficam em INGLES, como os menus do NE.
- **Nao verificado em terminal real** (so' modelo e `--dump`): como o FTXUI entrega Alt+letra, Ctrl+setas e o mouse no Windows Terminal/conhost e no Linux; cursor piscando; as cores nos esquemas. Quem testar: `fwMSXc.exe --edit arquivo`.
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 29/29 nos dois.
- **Fora de escopo / proximos**: E3 (janela ImGui), E4 destaque de sintaxe, E5 completar e snippets, E6 integracao (Executar no MSX, erros -> linha, projeto), E7 (Paragraph/Document format, Print, Outline, Load more/Write to cursor/Top of file); `edit`/`view` dentro do console/TUI de menus (hoje so' como programa `--edit`/`--view`); a pagina `man editor`; o perfil fwEditor.

---

## v1.41.0 -- "Hyper Sports" (Konami) (2026-10-20)

**Pedido do usuario**: "siga pela fase E1" do editor de textos (projeto em `doc/editor-spec.md`): o nucleo, estilo Norton Editor 2, com os mesmos recursos e atalhos na TUI e na janela. Decisoes anteriores: perfil de teclas Norton como padrao + teclas modernas alternativas + perfil opcional "fwEditor";
teclas alternativas para o que o terminal entrega mal; Load more/Write to cursor/Top of file depois; **modo somente leitura `view`** (so' o nao destrutivo funciona). O levantamento veio de 62 telas do NE 2 (`images/ne-*.png`, `images/Monitor_1_*.png`).

- **Arquitetura**: um MODELO sem interface (`Key(nome)`/`Type(texto)` entram, `Render(grade)` sai), como o Mamute e o Z80 Debugger; a TUI e a janela so' pintam a grade. Comandos com caixa de dialogo deixam um `Dialog` pendente (`TakeDialog`); a interface devolve o resultado por `Find`, `GoToLine`, `Indent`, `Apply*`, `Load`...
- **Dados, nao codigo**: `Info(Cmd)` (nome, destrutivo?, dialogo, implementado?), `Menus()` (itens, letra, atalho, comando) e `Bindings()` (tecla -> comando, `alt` = alternativa). O modo `view` e' UMA regra: comando destrutivo + `read_only_` = ignorado.
- **Escolhas minhas onde as telas nao diziam (a rever com o usuario)**: Alt-K apaga a linha inteira (a ajuda diz "kills all characters on line"); Ctrl-Backspace = desindentar (a ajuda mostra `Ctrl <-` e o glifo da seta e' o do Backspace); Ctrl-U desfaz so' a ULTIMA remocao (se o cursor nao mexeu);
  a busca nao da' a volta ao fim do arquivo; `Col=` e' a coluna de TELA (tab expandido); Information conta CRLF como 2 caracteres; "Moves to tab stop" so' move o cursor; quebra de pagina = linha com so' o FF (0Ch); o backup troca o 1o caractere da extensao; Alt-B S pela terceira vez recomeca o bloco.
- **Teclas**: Alt-F/B/T/S/P/V/O seguidas da letra do item (49 mnemonicos conferidos por teste); Esc cancela; `F10` pede a barra de menus (`TakeMenuRequest`).
- **Achado de ferramenta**: nenhum de build; o Fortran usa `ichar` (nunca `iachar`: derruba o f951 do gfortran 16).
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 28/28 nos dois.
- **Fora de escopo / proximos**: E2 (tela do NE na TUI: menus, dialogos, 2a janela, mouse; comando `edit`/`view`), E3 (GUI ImGui), E4 destaque de sintaxe, E5 completar e snippets, E6 integracao (Executar no MSX, erros -> linha, projeto), E7 Text-layout completo (paragrafo, documento), Load more/Write to cursor/Top of file e Print.

---

## v1.40.5 -- "Antarctic Adventure: Montador na TUI" (Konami) (2026-10-19)

**Pedido do usuario**: integrar o montador (asm) ao painel TUI, seguir `.debug`/`.break` para chegar perto do depurador avancado, garantir BOM/UTF-8 no fwasm e levar a fita para a TUI se preciso (ja' estava, 1.40.4). Decisoes (AskUserQuestion):
"montar e rodar" cobre TUDO, inclusive MSX-DOS; entram os marcadores no Z80, os rotulos do `.sym` e o arquivo:linha do fonte. O fonte do asMSX nao foi lido; o `resource/asmsx.exe` serviu de oraculo para o BOM/UTF-8 e para os bytes de `.debug`/`.break`.

- **BOM/UTF-8**: com BOM o asMSX nao gera nada (para em silencio apos "Assembling source file", `rc=0`, deixando `~tmppre.4`); o fwasm agora ignora o BOM (fonte e `.include`). UTF-8 em comentario e string sai cru, byte a byte, igual ao asMSX; acento em rotulo e' erro nos dois.
- **fwasm**: `AsmResult::symbols` (rotulos e constantes), `format`, `mapper` (MegaROM), `uses_markers`; `AsmOptions::want_tape` (monta o `.cas` de um `.basic` sem a diretiva); `fwasm::AssembleFile`/`SaveOutputs` (cli.cpp, agora tambem no `FWASM_LIB_SOURCES`) para montar do disco com includes relativos.
- **`dbg::Symbols`**: simbolos + mapa endereco -> arquivo:linha no `Debugger`; `Resolve` (`nome+4`), `Describe`, `AddressOfLine`, `ParseSym` (`.sym` do asMSX/fwasm, incl. `01h:8000h`). `need_addr` da ponte aceita rotulos e `arquivo:linha`.
- **Marcadores**: `Machine::CheckMarkers()` antes de cada instrucao quando ligado (`LD B,B`+`JR 0` = parar; `+JR 2 +word` = breakpoint em endereco; `LD D,D`+`JR n`+texto = evento de texto). `Debugger::TakeMarkerSkip` evita parar de novo no mesmo marcador depois do `go`
  (o `skip_once_` so' funciona com breakpoints armados). Desligados por padrao: custam uma leitura de memoria por instrucao.
- **`asmrun`** (`Commander::AsmRun`): ROM/MegaROM (`Host::LoadCartridge`; konami -> `konami4`, konamiscc -> `konami5`), `.basic` (fita temporaria + `type`), `.msxdos` (copia do disco de sistema com `msxdisk::DiskImage` + ENTER da data + nome). A espera do boot e' uma fila de teclas vazias.
  **Achado de teste**: depois de um `asmrun` de ROM com loop infinito a maquina nunca chega ao BASIC; o teste do `.basic` precisa de `cart -` antes. **Achado de teste 2**: o MSX-DOS 1 pergunta a data na partida; sem o ENTER o nome do programa vai parar no prompt de data.
- **Interfaces**: menu Montador na TUI (8 menus; `Ajuda` virou o 8o), janela (menu Depurador), modo terminal, `kWords` do console, painel Z80 Debugger com o rotulo na coluna da direita. `man asmrun/sym/src/markers`.
- **Achado de ferramenta**: o `.debug` do asMSX gera `JR (tamanho+4)`: salta 4 bytes alem do texto. Reproduzido byte a byte (golden); para o depurador imprimir o texto basta ler `d-4` caracteres a partir de `pc+3`.
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 27/27 nos dois.
- **Fora de escopo / proximos**: breakpoints condicionais e em E/S, trace, passo por linha de fonte, simbolos de MegaROM por subpagina, breakpoints que sobrevivem a troca de cartucho (`asmrun` recria a maquina e os breakpoints criados antes se perdem).

---

## v1.40.4 -- "Antarctic Adventure: Mamute, Fita" (Konami) (2026-10-19)

**Suporte total a fita cassete** (pedido do usuario depois de perguntar onde a fita existia: emulador sim, Mamute so' um `R` de mentira, fwasm so' gerava). Decisoes (AskUserQuestion): patch 1.40.4 e `.wav` incluido.
Mapa do que existia e do que faltava: motor completo (leitura rapida/normal, gravacao pela BIOS, TZX, ripper de WAV em ferramenta), fwasm gerava `.cas`/`.wav`, depurador so' mostrava o motor do PSG/PPI, Mamute nada (`R` e `LOAD .cas` recusavam).

- **`tape_io`** (`src/tape/cpp`): `LoadAnyTape`/`SaveAnyTape` por extensao (`.cas`, `.tsx`/`.tzx`, `.wav`), `ReadTapeFile`, `AppendTapeFile`, `MakeBinaryContent`, `TruncateAtFile`, `PulsesFromMarks`, `WriteWavFromPulses`. O `.wav` gravado tem silencio de 0,5 s antes, depois e entre blocos; o ripper o le de
  volta sem perder um byte (testado `.cas`/`.tsx`/`.wav` ida e volta byte a byte).
- **`TapeDeck`**: extrai do `TapeEngine` o que nao e' hardware (inserida, protecao, modo de gravacao, ponto, `AppendFile`/`ReadFile`/`FindFile`/`SaveAs`). O motor herda e liga os cursores pelos ganchos `OnReplaced`/`OnPulsesChanged`. O corte de uma vez do TAPOON
  virou `CutForWrite`, compartilhado com o `AppendFile`. `Rewind` e `SeekToFile` viraram virtuais para o Mamute poder rebobinar/marcar a fita da maquina por um `TapeDeck*`.
- **Ponte**: `tape` ganhou `new list seek protect write mode save put load` (`TapeCommand` em `command.cpp`); as entradas `tapion tapin tapiof tapoon tapout tapoof` valem como endereco em `break/unbreak/watch/until/go/jump/dis/mem`.
- **Mamute** (`mamute_tape.cpp`): `TAPE`, `R [desloc]`, `TLOAD`, `TSAVE`, `LOAD`/`SAVE` por extensao, `A` opcao `T`. `IMemory::Tape()` devolve a fita da maquina no ONLINE (nullptr no offline, onde o monitor tem um `TapeDeck` proprio). **Achado**: o `LOAD` online sem slot usava
  `PageMap()[1]` (o slot da pagina 1, ROM) e entao nao conseguia gravar em C000; `ChooseSlot` agora usa a pagina do endereco. O `R` avanca o ponto da fita da maquina, entao o `CLOAD` seguinte do BASIC continua dali.
- **fwasm**: `.TSX`. Para nao arrastar o motor, o montador so' ganhou `tsx_writer.cpp` (`BuildTsx`), a mesma funcao do escritor de arquivo. Os 6606 casos dourados passam sem mudanca (o asMSX nao conhece `.tsx`).
- **Prova de ponta a ponta** (`asme2etest`): o fwasm monta um programa de BASIC com `.cas`, `.tsx` e `.wav`; o emulador carrega cada um por `BLOAD"CAS:",R` e o programa RODA (texto na VRAM). O `.tsx` tambem foi carregado no modo NORMAL, pelos pulsos de verdade.
  O `.wav` de 8 bits do fwasm passa pelo ripper do emulador sem ajuste (nao conferido num MSX real).
- **Interfaces**: TUI (Midia: nova fita, listar, proteger/liberar, modo, copia), janela (filtros com `.wav`, "Nova fita", "Gravar uma copia"), `--fita` e `--cas` aceitam `.wav`, banco de fitas varre `.wav`. `man tape` reescrito, `man mamute-fita` novo (11a pagina do Mamute), `man fwasm` com `.tsx`.
- Validado: Windows (`build.ps1`) e Linux (WSL, `build.sh`), `ctest` 27/27 nos dois.
- **Fora de escopo**: gravar em "modo normal" decodificando pulsos (CSAVE/BSAVE sempre pelo gancho); blocos de TZX diferentes do #4B continuam sem efeito.

---

## v1.40.3 -- "Antarctic Adventure: Mamute, SUPER-X" (Konami) (2026-10-19)

**Mamute, Fase 4.** Os comandos do SUPER-X (prefixo X) do Mamute original. Especificacao lida na ajuda e no codigo do Mamute do paleobasic (`MamuteHelpData.pbi`, `MamuteAssemblerGui.pbi`, `MamuteSupport.pbi`); o fonte do
SUPER-X em Z80 (`others/superx/SUPER-X.ASM`) nao foi usado. Decisoes do usuario (Fase 1): paineis na TUI + versao de linha no CLI; offline com memoria privada.

- **Alvo e variaveis**: `Target` (`#slot`, `#slot-sub`, `#V`, `#S`) e `Vars` (`@0`-`@3` `@B` `@E` `@S`); `IMemory` ganhou `ReadSub/WriteSub`. Offline, o sub-slot 0 e' o proprio slot e os sub-slots 1-3 sao 64 KB de RAM cada (alocados
  quando usados). **Achado (online)**: numa maquina com o slot 3 expandido a RAM nao esta' no sub-slot 0, entao `#3` sem sub-slot passou a valer "o sub-slot que a CPU enxerga naquele slot" (`MachineMemory::Sec`); isso tambem corrige o `LOAD`/`WriteSlot`
  da Fase 1 nessas maquinas. `MachineMemory::WriteVram` passou a gravar (`Machine::PokeVram`), entao `XFL 0000#V,...` escreve na VRAM da maquina.
- **Telas e listagens**: `XD XA XI XH` com um endereco = tela (painel na TUI; no console a mesma tela sem cursor); com dois ou tres campos = listagem (fim vazio = 256 bytes, ate' o teto do alvo; terceiro campo = arquivo). `XD` e' uma
  variante do `GridPanel` (digitacao ASCII com `"`, `@` repete o byte); `XA XI XH XIR XPP XTP XTR` sao paineis novos (`mamute_superx_panels.cpp`). Todos testados por teclas, sem terminal.
- **`XM`**: uma SESSAO (o prompt vira `XM C100> `), nao uma janela: instrucao Z80 (montada pelo fwasm num `Source` de duas linhas: `ORG` e a linha), dados (`. : ; [ "`, itens pelo `CL`), endereco (o alvo continua; so' `#S` volta ao PAGE) ou `I [n]`.
  O console passou a tratar `QUIT` como "volta ao monitor" em qualquer sessao (EDIT> e XM), pelo prompt que nao comeca em `MON(`.
- **`XRT`**: o kernel em C (`mamute_insn_info`) diz o tamanho de cada instrucao e onde esta' o endereco absoluto (`JP`, `CALL`, `LD dd,nn`, `LD (nn),r`, `ED`/`DD`/`FD`) ou o deslocamento (`JR`/`DJNZ`); so' o que aponta para DENTRO do bloco anda.
  Um `JR` que sai do bloco e' recalculado e, se nao couber em -128..127, nada e' gravado. Limite conhecido: dados no meio do codigo podem ser lidos como instrucao (o original avisa o mesmo).
- **`XGO`**: o criterio de "fim do programa" (o RET que devolve alem da pilha de entrada) usa a diferenca de 16 bits do SP, nao a comparacao sem sinal do original, que nunca disparava com `SP=0000` (o valor de `XRG +`). **Achado**: o HALT
  ficava ligado entre programas e o `XGO <end>` seguinte parava na hora; `XGO <end>` e `XTR <end>` agora reiniciam o HALT.
- **Portas**: a `PortTable` e' ligada ao barramento da CPU OFFLINE (`IN` le a Saida, `OUT` grava a Entrada e marca a porta); online, as instrucoes `IN`/`OUT` sao da maquina e o painel so' guarda o que o usuario digita (`XPI`/`XPO`).
- **Disco**: o nucleo do msxdisk (geometria, entradas de diretorio, `DiskImage`, FAT12) entrou no `CONTROL_LIB_SOURCES` (so' o nucleo: sem CLI/TUI/GUI dele). `XL%`/`XS%` usam o arquivo da imagem direto. O dialogo de arquivo do original virou
  argumento; sem disco corrente os comandos pedem o `XDK`.
- **Notas**: `SUPER-X-PT.notas` (471 notas traduzidas pelo usuario no paleobasic; texto ASCII) fica em `dist/` e entra no pacote (`build.ps1`/`build.sh`); `LoadNotes` aceita BOM e CRLF; o `.TNK` japones nao e' usado.
- **Kernels**: C `mamute_reloc.c`, Fortran `mamute_total.f90` (`mamute_sum16`), NASM `mamute_cmp.asm` (`mamute_diff`, ABI dupla Win64/SysV).
- **Nao entrou (registrado)**: a "cruz" de troca entre XD/XA/XI/XM/XH, o `CLS`, o `R` (fita) e as cores do `XCO` pintando de verdade (hoje so' ficam guardadas no `mamute.cfg`).
- `ctest` 27/27 (Windows e Linux); `mamutetest` 407 checagens.

## v1.40.2 -- "Antarctic Adventure: Mamute, telas" (Konami) (2026-10-19)

**Mamute, Fase 3.** As janelas do Mamute original (DM, M, S, ZAP, SCR) e o PDF. Decisao do usuario (Fase 1): **paineis na TUI + versao de linha no CLI** (o SCR grafico fica para a janela grafica; aqui vira braille).
Especificacao lida no manual do Mamute original (DM, M e S, ZAP, SCR).

- **Modelo sem tela**: `Panel` (mamute_panel.h) recebe teclas por NOME (`up`, `tab`, `enter`, `esc`, `bs`, `plus`, `minus`, `space`, `save` = Ctrl+S, `c:<char>`) e devolve as linhas; o cursor vem entre `\x01` e `\x02` (a TUI
  inverte as cores). Por isso o mesmo codigo e' testado por teclas, sem terminal, e a TUI so' desenha e repassa teclas.
- **Protocolo**: o hospedeiro liga os paineis (`mon @panels on`, a `ReplSession` da TUI faz sozinha, uma vez por conexao); `DM`/`M`/`S`/`ZAP`/`SCR` devolvem `action=show panel=mamute title=NOME` + as linhas; cada tecla e' `mon @key <nome>`
  e a resposta e' o painel de novo ou `action=close panel=mamute`. Sem `@panels on` (console, scripts, TCP) os mesmos comandos sao as versoes de linha.
- **Achado -- o espaco**: as camadas de cima aparam espacos das pontas da linha, entao a tecla espaco ganhou nome (`space`); sem isso o ESPACO do SCR (inverter pixel) chegava como `c:` e sumia.
- **Achado -- bug de teste**: um `std::string(ReadAll(x).begin(), ReadAll(x).end())` (iteradores de dois vetores temporarios) abortava o `mamutetest` em ~50% das execucoes (`length_error`); apareceu so' em repeticao (25 execucoes).
- **PDF proprio** (`mamute_pdf.cpp`): PDF 1.4, A4, Courier 9 pt, cabecalho com a pagina de cada folha, parenteses escapados, `xref` correto; ~68 linhas por folha.
- **ZAP**: a imagem inteira vai para a memoria; as alteracoes ficam em um conjunto de setores sujos; Ctrl+S grava so' o setor do cursor (`*` no titulo ate' salvar). **SCR**: azulejos de dx x dy caracteres, modo 0 (linha) e 1 (coluna);
  zoom 1 (128x48 colunas) e zoom 2 (64x24, cada ponto e' um bloco 2x2); moldura 16x16 e edicao com desfazer total pelo snapshot dos 32 bytes.
- `ctest` 27/27 (Windows e Linux).

## v1.40.1 -- "Antarctic Adventure: Mamute, EDIT e A" (Konami) (2026-10-19)

**Mamute, Fase 2.** O `EDIT` (editor de linhas numeradas do MegaAssembler) e o `A` (montar). Decisao do usuario (Fase 1): o `A` monta pelo fwasm, nao pelo `Z80Asm.pbi` do paleobasic.
Especificacao lida no manual do Mamute original (secoes EDIT e A); o `R` do original so' validava a sintaxe e continua fora.

- **Como funciona**: o fonte do Mamute (`Label: instrucao`, hexa por padrao, `DEFB/DEFW/DEFM/DEFS/EQU/ORG/END`, `(HL)` em vez de `[HL]`) e' traduzido linha a linha para o asMSX (`.zilog` ligado) e entregue ao fwasm;
  numeros viram decimais, `DEFM 'texto'` vira `db "texto"`, `Label: EQU x` vira `Label equ x`. Uma linha do asMSX por linha do Mamute, na mesma ordem: um erro "line N" do fwasm vira "linha NN" do Mamute.
- **fwasm ganhou**: `AsmOptions::want_listing` (por linha de fonte, o endereco `$` onde comecou e os bytes que gerou; EQU devolve o valor) e `fwasm::IsDirectiveName`. Achado: `PRINT`, `START`, `SIZE`... sao diretivas do fwasm
  e davam "reserved word used as identifier" ate' num operando (`LD HL,PRINT`); a traducao renomeia (`PRINT_m`) so' os rotulos que colidem e o `.sym` volta com o nome certo.
- **Referencia cruzada (`A R`)**: o fwasm nao guarda os usos, entao o Mamute procura o simbolo nos operandos das linhas (palavra inteira) e mostra o endereco de cada uma.
- **Diferencas do original**: os dialogos viram argumentos (`SAVE arq`, `A ...>nome`); `P` e `H` gravam texto (nao PDF); a opcao `I` grava o `.bin` direto (o original abria a janela do SAVE); `QUIT` no EDIT volta ao monitor e `BA` sai de tudo.
- **Parte tecnica**: `CONTROL_LIB_SOURCES` passou a levar o fwasm (o Mamute monta); o console decide `QUIT` dentro do EDIT consultando o prompt do monitor (`EDIT> `).
- `ctest` 27/27 (Windows e Linux).

## v1.40.0 -- "Antarctic Adventure" (Konami) (2026-10-19)

**O Mamute no fwMSX (Fase 1).** Pedido do usuario: trazer o Mamute Assembler (debugger/montador/editor de memoria/dumper) do paleobasic, escrito em PureBasic, para dentro do
fwMSX, convertido para o mix C/C++/Fortran/Assembly e integrado a TUI, a CLI e a passagem de parametros, com HELP, `man` e configuracao inicial; sem a janela separada; com um modo
online (reflete na maquina em execucao) e um offline. Pesquisa antes de codar: ~21 mil linhas de PureBasic, ~60 comandos, uma maquina simulada propria (256 KB, Z80 proprio) que o fwMSX
ja' tem de verdade -- o online e' o monitor operando a maquina emulada. Decisoes do usuario (AskUserQuestion): entregar **em fases**; EDIT/A **via fwasm**; offline com **memoria privada +
nucleo Z80 do fwMSX**; telas como **paineis da TUI + versao de linha no CLI**. Detalhes em [mamute-spec.md](mamute-spec.md).

- **Fase 1 (esta)**: nucleo + 31 comandos (`BA QUIT XQT ONLINE OFFLINE STATUS PUSH PULL PAGE DM M SH MS C D P V T F L LP X G LOAD SAVE CFG HELP`), `mon`/`mamute`, `--mon`, prompt
  `MON(off)>`/`MON(on)>`, `mamute.cfg`, `man mamute` (7 paginas), categoria "Mamute" no help.
- **Online de verdade**: o `repltest` conecta o console a uma maquina real (BIOS), entra no monitor, escreve `3E 2A C9` em C000h e le o byte de volta pelo `peek`; o `mamutetest` faz o `G C000` e
  a CPU da maquina passa a rodar o programa (A = 2A). Escrita em ROM e' ignorada, como num MSX de verdade.
- **Achados**: o `Commander` recria a maquina entre comandos (o host pode troca-la), por isso o monitor so' a recebe DURANTE cada comando (`AttachMachine`/`DetachMachine`) e guarda o modo
  proprio; na TUI o prompt aparece na linha de entrada (campo `monitor_prompt`).
- **Diferencas do original**: sem janelas (a grade do `DM` sai no log), `P`/`LP` gravam texto (nao PDF), `SAVE .rom` sai sem cabecalho, padrao com RAM no slot 3. Ainda nao existem: `ZAP SCR S EDIT A R` e os
  comandos do SUPER-X (Fases 2 a 4).
- `ctest` 27/27 (Windows e Linux).

## v1.39.2 -- "Yie Ar Kung-Fu: Amazonia" (Konami) (2026-10-19)

Segundo programa real no fwasm: o jogo **Amazonia** (Renato Degiovani) em versao cartucho de 48 KB, adaptado pelo Wilson a partir da versao em disco (o autor nao tem mais os fontes
originais). Fontes em `resource/Amazonia64kV5/source`, montado por `MakeWin.bat` (`asmsx source\amazonia.asm` a partir da raiz do projeto). Copiado para `tests/asm/real/amazonia/`.

- **Tres achados, todos corrigidos**: (1) fonte em subpasta: o CLI duplicava o diretorio e nada era incluido; (2) `.include "source/x.asm"` / `.incbin "64k/x"` relativos ao diretorio ATUAL
  (o asMSX tenta ao lado do fonte e depois no diretorio atual; confirmado no oraculo); (3) hexadecimal `#F3AE`.
- **Resultado**: `.rom` (49152 bytes, CRC 0254F631) e `.sym` (4217 bytes, CRC B036A9F4) iguais aos do asMSX v1.2.0.
- **Fonte do usuario**: `msxsys.asm` define `LINL40` duas vezes (linhas 39 e 52, mesmo valor). O asMSX v1.2.0 (e o fwasm) recusam com "symbol redefinition"; o asMSX antigo que o
  `MakeWin.bat` usa (`bin/asmsx.exe`) aceitava. A copia de teste tem a linha 52 removida; o fonte original do usuario NAO foi alterado.
- **Diferenca contra o `build/amazonia.rom` antigo**: so' 2 bytes (o INIT cabecalho, offsets 2-3: 0010h no antigo, F3E7h na v1.2.0 e no fwasm) e a linha duplicada do `LINL40` no `.sym`.
  O fwasm segue a v1.2.0 por decisao do usuario.
- `ctest` 26/26 (Windows e Linux).

## v1.39.1 -- "Yie Ar Kung-Fu: Replayer PT3" (Konami) (2026-10-16)

Primeiro teste do fwasm com um programa REAL escrito para o asMSX por outra pessoa: o replayer de musica PT3 (Vortex/Arkos) de SapphiRe, que veio na pasta
`resource/Amazonia64kV5/musica/Nestruo asMSX` (`example.asm` + `PT3-ROM.ASM` + `PT3-RAM.ASM` + `AMAZONIA.PT3`). Copiado para `tests/asm/real/pt3/`.

- **Dois achados, ambos corrigidos**: `ROTULO: .include "arq"` na mesma linha (o `.include` era ignorado e dava "undefined identifier") e `nome .equ valor` (so' `equ` era aceito).
- **Resultado**: com as correcoes, o `.rom` (8192 bytes, CRC 898867E9) e o `.sym` (CRC 5E6BB47B) saem iguais aos do `resource/asmsx.exe` (asMSX v1.2.0).
- **Versao alvo**: so' o asMSX v1.2.0 (decisao do usuario; o `asmsx.exe` v0.19.2 que vinha na pasta foi removido do repositorio de trabalho).
- **Nao serve de corpus**: `musica.asm` (fonte original do Vortex Tracker, nao e' para o asMSX; o proprio asMSX sai sem gerar nada) e o `msx-msxlib-master` (outra familia de montador).
- **Teste**: `asme2etest` ganhou `TestPt3Replayer` -- confere os CRCs e roda a ROM por 600 quadros vendo o volume ligar e o tom do canal A mudar no PSG. `ctest` 26/26 (Windows e Linux).

## v1.39.0 -- "Yie Ar Kung-Fu" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- o fwasm como ferramenta completa de compilacao do sistema. Era o item "Falta" do `asm-spec.md` desde a 1.36.0.

### Metodo
O `resource/asmsx.exe` (o oraculo ja' usado nos 6224 casos anteriores) foi a fonte da verdade: para cada diretiva, primeiro varreduras de sondagem (`/tmp/probe_*.py`, caixa-preta)
para descobrir as regras, depois o codigo, depois os casos dourados em `tools/asmdiff/gen_formats.py`, e iterar ate' 382/382. Nada do fonte GPL do asMSX foi lido.

### Descobertas (ver doc/asm-spec.md para a lista completa)
- O `.rom` perde o ultimo byte quando o bloco tem 8 KB + 1 (tamanho calculado de len-1).
- MegaROM: o deslocamento no arquivo e' `subpagina*tamanho + (endereco - X)` (o X do `.subpage` so' e' a origem); `.select` por mapper; o aviso "only N out of M" conta as paginas que FALTAM.
- `.start` depois do `.rom`: ignorado no arquivo, MAS com fita pedida o INIT final entra no `.rom` e na fita.
- `.wav`: ciclos de 2400/1200 Hz, fronteiras na amostra arredondada do tempo acumulado (em 1/16: 147/294, exato, sem ponto flutuante); termina com numero PAR de amostras.
- `.sinclair`: o carregador BASIC tem checksums errados no original (cabecalho: sem o nome e +11h; dados: +11h e o XOR do nome) -- reproduzidos; o nome da fita e' o do primeiro `.cas`/`.wav`.
- O CLI `--asm` fixava o nome-base sempre (o `.filename` nunca valia): corrigido.

### Validacao
`ctest` 26/26 (Windows e Linux; o NASM roda nas duas ABIs). Novo `asme2etest`: monta com o fwasm e RODA no emulador um cartucho ROM, as MegaROMs dos 4 mappers (a troca de subpagina funciona de verdade) e a fita `.cas` (`BLOAD"CAS:",R` acha "HELLO" e executa). `asmtest`: 6606 casos (4170 + 1590 + 279 + 185 + 382). `mantest` com as paginas novas do manual.
Nao conferido: os `.wav` num MSX de verdade; o `.cas` carregado no emulador alem do que o teste de CRC garante (ver OUTLINE); os cantos "nao reproduzidos".

---

## v1.38.5 -- "Road Fighter: Nestor, SEE e fwasm" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- trazer do paleobasic o manual do SEE Tracker, do NestorBASIC e do "MSX Turbo Basic"; no lugar do manual do asMSX, o manual do NOSSO montador (que e' baseado no asMSX); N80/LK80/LIBSTOR ficam para depois.

### Interpretacoes e decisoes
- **"MSX Turbo Basic" = MSXBAS2ROM** (item de menu "MSXBas2Rom..." do paleobasic: compilador de MSX-BASIC para ROM, 54 paginas md). O "Turbo-Basic Compiler" do RuMSX (7 KB) ja' estava no man desde a 1.38.4 (`turbo-basic-compiler`, apelido `turbo-basic`); o MSXBAS2ROM ganhou `msxbas2rom` e `msx-turbo-basic`. Se o usuario quis dizer outra coisa, e' trocar.
- **Manual do fwasm**: escrito em portugues a partir de `resource/asmsx.md` (o manual em que o proprio fwasm foi baseado), mas descrevendo o estado REAL: o `asm-spec.md` e os 1590+ casos dourados dizem o que funciona (rotulos, equ, variaveis, `$`, db/dw/ds/dt/byte/word, org/page/phase, comentarios dos 5 estilos,
  zilog, print*, if/ifdef/rept/macro, include/incbin, bios/biosvars, filename, todas as expressoes e instrucoes). Faltam os formatos de saida e select/subpage/search/callbios/calldos/size/debug/break; `.start` so' valida. A pagina `fwasm-directives` marca cada uma.
- As tabelas `fwasm-bios` e `fwasm-biosvars` incluem `src/asm/cpp/bios_tables.inc` direto: nao ha' segunda copia que possa divergir do montador.
- **Exemplo verificado ponta a ponta**: `hello.asm` (cabecalho "AB" + INIT + CHPUT) -> `asm` -> copiar o `.z80` para `.rom` -> `--msx --cart hello.rom` mostra "Ola do fwasm!" na tela (SCREEN 1). Esse e' o caminho para ROMs ate' o `.rom` existir no fwasm.
- Correcoes durante a escrita, vistas nos casos dourados: estouro de `db`/`dw` e' AVISO ("Cropping to fit"), nao erro; `org` para tras e' permitido.

### O que foi feito
- `import_tools_help.py`: NestorBASIC (99 -> 88 paginas: introducao + 87 funcoes; os grupos sem texto saem; apelido pelo wrapper `.NB_*` do Basic Dignified), SEE Tracker (24 -> 23, sem o plano "Rumo a um tracker compativel"), MSXBAS2ROM (54, markdown -> texto).
- `man_pages_fwasm.cpp`: o manual do fwasm; `asmsx` virou apelido de `fwasm`; `man asm` aponta para ele; `bios`/`sysvars` apontam para `fwasm-bios`/`fwasm-biosvars`.

### Validacao
`ctest` 25/25 (Windows e Linux). `mantest`: paginas do fwasm (11), tabelas geradas (CHPUT = 00A2h, JIFFY = FC9Eh), estado das diretivas, NestorBASIC (funcao 23 e apelido pelo wrapper), SEE (23), MSXBAS2ROM (54), `turbo-basic` nao mudou de dono.
Nao conferido: a fidelidade dos textos importados; o estado de `.size` (tratado como FALTA) e dos apelidos menos usados das diretivas (`printdec`/`printstring`/`deft`).

---

## v1.38.4 -- "Road Fighter: MSX-DOS 2 e Z80" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- estudar `MsxManualsHelpData.pbi` (MSX-DOS 2 e outros) e `MsxSoftwareHelpData.pbi` (help mais simples do MSX-BASIC, MSX-DOS, CP/M), acrescentar o que falta e ver o que
complementa o que ja' existe. O `OpenMsxHelpData.pbi` fica de fora (o projeto nao usa o openMSX).

### Estudo das fontes
- **MsxManualsHelpData.pbi** (MANUALS.CHM): 18 paginas gigantes (1,8 MB): MSX-DOS 2 (referencia de comandos 129 KB, interface de programa 19 KB, codigos de funcao 114 KB), Z80 (3), R800 (2), Turbo-Basic, FM-PAC
  e **8 capitulos do MSX2 Technical Handbook (700 KB), que ja' existem no man desde a 1.38.3** (edicao Markdown do Konamiman, mais limpa) -> nao importados.
- **MsxSoftwareHelpData.pbi** (SOFTWARE.CHM): 359 topicos curtos em ingles: 148 palavras do MSX-BASIC 1, 85 extensoes CALL de firmware, 45 comandos do MSX-DOS 1 (25 sao esqueletos sem texto), 28 do Disk-BASIC,
  21 do MSX-BASIC 2, 17 referencias (cores, erros, modos de tela, vozes do MSX-Music...), 10 "outros" (operadores, variaveis, USING, PLAY...), Turbo-R, 2+ e CP/M.
- **Sobreposicao com o que o man ja' tinha**: as palavras do BASIC (livro da Gradiente, portugues, mais completo) coincidem; a versao em ingles traz exemplos de saida e "veja tambem" -> vira secao extra.
  O MSX2 Technical Handbook ja' cobre os mesmos assuntos do MANUALS.CHM -> nao duplicado. Tudo o mais (MSX-DOS 2, MSX-DOS 1, CP/M, Z80, R800, FM-PAC, Turbo-Basic, CALL de firmware, referencias) e' NOVO.

### O que foi feito
- `tools/paleobasic/import_manuals_help.py`: **partiu** os manuais grandes -- o MSX-DOS 2 por capitulo e por comando (um comando = linha sozinha em maiusculas seguida de "Format") e por chamada de funcao (cabecalho "3.n NOME (XXH)",
  simbolo `_NOME` lido de "Parameters: C = 09H (_STROUT)" vira apelido). Rotulos `**Sintaxe:**` viram `SINTAXE`; tabulacoes viram espacos; ASCII puro; esqueletos descartados.
- `man.cpp`: a fusao de nomes iguais agora tambem olha apelidos, mas so' os da mesma categoria (senao `screen` seria absorvido pela pagina de referencia `screens`).
- Testes: `dos2-copy` (so' o COPY), `dos2-fn-09h`/`_strout`, 00h a 70h, `dos-dir`/`cpm`, `call kanji`, `abs` com a referencia em ingles, `ref-*`, `z80-opcodes`, R800, FM-PAC, Turbo-Basic e nenhum esqueleto.

### Validacao
`ctest` 25/25 (Windows e Linux). Visto na tela: `man _strout`, `man dos2-chdir`, `man ref-screen-modes`.
Nao conferido: fidelidade ao texto dos manuais (herdada do paleobasic); a partida dos manuais em casos de borda (um comando cujo titulo nao segue o padrao ficaria dentro do anterior).

---

## v1.38.3 -- "Road Fighter: Livros de referencia" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- importar o Red Book, o Technical Handbook e a ajuda do Basic Dignified (ideia dele para uso do Basic Dignified).

### O que foi feito
- `tools/paleobasic/import_books_help.py`: le `RedBookHelpData.pbi` (973), `Th2HandbookHelpData.pbi` (1356) e `BasicDignifiedHelpData.pbi` (21). Limpeza do Markdown do paleobasic:
  `[[[texto|||ancora]]]` -> texto; `[[[[Ver Figura: X|||img:x]]]]` -> `[Ver Figura: X]` (as figuras nao existem no terminal); `@@@` (linha pre-formatada) -> 4 espacos;
  `**`, crases e cercas ``` saem; `<a>`/`<br>`; escapes `\|`, `\*`. ASCII puro.
- **Links -> VER TAMBEM**: os mapas `X_AddAnchor(ancora, indice)` do paleobasic dizem para que topico cada link apontava; o conversor lista ate' 12 destinos no fim da pagina.
- Nomes: `rb-`, `th2-`, `bd-` + titulo (sem acento, ate' 60 caracteres, numerados quando repetem, ex.: `th2-index-2`). Categorias "Red Book: <capitulo>", "Handbook: <capitulo>",
  "Basic Dignified: <grupo>".
- `man.cpp`: `Find` com tabelas `std::map` (antes: varredura de ~5000 topicos por chamada), fusao de nomes sem O(n^2), `ListCategory` com varias categorias (prefixo), indice com secao de familias grandes.
- Testes: contagens, links viram VER TAMBEM (>100 paginas), `man dignified`, `rb-expanders`, `man -l red/handbook`, nenhum `@@@`/`[[[` sobrando; o teste de "VER TAMBEM" ganhou nomes com hifen.

### Achados
- O texto dos livros e' em ingles, como no original; o Basic Dignified e' em portugues e fala da IDE paleobasic ("esta IDE", "Configurar -> Basic Dignified...") -- lido como referencia da sintaxe do dialeto.
- Titulos repetidos entre capitulos (`Index`, `Contents`) ficam `th2-index`, `th2-index-2`...: pouco uteis; o caminho e' `man -l <capitulo>` ou `man -k`.
- O executavel cresceu ~2,3 MB com os textos (de 19,7 para ~22 MB no Windows).

### Validacao
`ctest` 25/25 (Windows e Linux). Visto na tela: indice, `man -l`, `man rb-expanders` (com VER TAMBEM), `man dignified`.
Nao conferido: a fidelidade do texto aos livros (herdada do paleobasic); tabelas largas do Handbook em terminal estreito.

---

## v1.38.2 -- "Road Fighter: BIOS e hardware" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- importar tambem `BiosCallsHelpData.pbi`, `BiosDocHelpData.pbi` e `HardwareHelpData.pbi` do paleobasic. O usuario informou que o
responsavel pela Aleph autorizou explicitamente a distribuicao de material de MSX/ZX81 e que o restante vem de fontes publicas (MSX Assembly Page, manuais do
blueMSX e do openMSX, livros da Aleph).

### O que foi feito
- `tools/paleobasic/import_bios_help.py`: mesmas regras do conversor do BASIC (so' literais PureBasic; qualquer outro termo e' erro) para o formato
  `X_Begin()/X_L("...")/X_Commit(titulo, grupo)`. 597 verbetes de BIOS/BDOS/RAM -> 554 topicos (os de mesmo nome em areas diferentes se juntam); 2 de
  documentacao; 33 de hardware. Tabulacoes viram espacos; ASCII puro. Hardware: apelidos curtos para o primeiro topico de cada grupo (`hw-rtc`, `rtc`, `opll`, `v9990`...).
- `man.cpp`: fusao generica de nomes iguais (`==== categoria ====`), `ListCategory` (`man -l`), `Apropos` (`man -k`), indice com contagem das familias grandes.
- Testes (`mantest`): `chput`, `wrtvdp` (MainROM + Sub-ROM), `beep` (BASIC + BIOS), `jiffy` (FC9E), `hw-rtc`/`rtc`/`v9990`/`hw-psg`, contagens, `man -l`, `man -k`.

### Achados
- Muitas variaveis de RAM e rotinas do paleobasic tem so' o endereco e o nome (ex.: `jiffy` = "FC9E JIFFY"); e' o que a fonte tem. A tabela escrita a mao em `man bios`/`man sysvars`
  continua util para registradores de entrada/saida e foi conferida contra as ROMs.
- Diagramas ASCII mais largos que o terminal (ex.: a estrutura do RTC) quebram de forma feia; em terminal de 100+ colunas ficam inteiros.

### Validacao
`ctest` 25/25 (Windows e Linux). Visto na tela: `man chput`, `man -l`, `man -k rtc`, `man rtc`.
Nao conferido: a fidelidade ao MSXBIOS.CHM original (herdada do paleobasic); os diagramas em terminal estreito.

---

## v1.38.1 -- "Road Fighter: Manual do BASIC" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- o paleobasic ja' tem o help completo das palavras-chave do MSX-BASIC (livro da Gradiente, em portugues); importar
para o `man` e para o `help`.

### O que foi feito
- **Achado**: `resource/paleobasic/src/editor/help/` tem `MsxBasicDictData.pbi` (140 verbetes do livro), `MsxBasic2PlusDictData.pbi` (41, do manual
  ACVS MSX2+/FM), `MsxBasicManualData.pbi` (16 topicos de prosa/apendices) e `MsxBasic2PlusManualData.pbi` (7). Sao chamadas PureBasic
  (`MSXDict_Add(...)`) com argumentos que sao concatenacoes de literais; ha' ainda `BiosCallsHelpData.pbi`, `BiosDocHelpData.pbi` e `HardwareHelpData.pbi`
  (nao importados ainda -- ver doc/man-spec.md, secao 5).
- `tools/paleobasic/import_basic_help.py` interpreta so' esses literais (`"..."`, `#CRLF$`, `MSXQ`, `Chr(34)`, `#True/#False`, inteiros; qualquer outro termo
  e' erro), junta os verbetes MSX1 e "(MSX2+)" numa pagina so', gera apelidos e escreve `man_pages_basic.cpp` (ASCII puro; o `¡` e o `¿` do
  apendice de ASCII viraram "! (invertido)"/"? (invertido)").
- `man.cpp`: `Find` em duas passadas (nome exato antes de apelido, espacos repetidos valem um), `Resolve` (prefixo unico), fusao com comandos de mesmo nome,
  indice compacto das palavras do BASIC.
- A pagina `basic` (resumo escrito a mao) ganhou um aviso apontando para os manuais por palavra.

### Origem e direitos
Os textos sao transcricoes de obras de terceiros (livro Aleph/Gradiente 1986; manual ACVS) feitas no paleobasic, cujo repositorio (com os PDFs) ja' vive em
`resource/paleobasic`. O `man` cita a fonte e a pagina em cada verbete. Se for preciso, e' so' tirar `man_pages_basic.cpp` do CMake: o resto do `man` funciona sem ele.

### Validacao
`ctest` 25/25 (Windows e Linux). `mantest` confere: 150+ palavras e 23 topicos importados, apelidos, `Resolve`, fusao stop/wait, erros e mapa da memoria presentes,
ASCII puro e ate' 78 colunas. Visto na tela: `help print`, `man for`, `man put sprite`.
Nao conferido: a fidelidade das transcricoes ao livro (herdada do paleobasic, que diz ter transcrito "letra a letra"); o indice no console paginado.

---

## v1.38.0 -- "Road Fighter" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- organizar o `help` e criar um `man <comando>` com janelinha de leitura, ja' pensando em abrigar a referencia
do MSX (comandos do BASIC, enderecos importantes, ganchos, rotinas da BIOS, codigos de cor...).

### O que foi feito
- `src/man`: registro de topicos (`Topic`: nome, apelidos, categoria, uso, resumo, pagina), busca por nome parcial (Fortran + Assembly),
  quebra de linhas com alinhamento de colunas (C + C++), `HelpTable`/`HelpFor`/`Index`/`Render`.
- `ReplSession`: `help [x]`, `man [x]`, `SetViewer`, `SetWidth`. Console: pager (`Page()` em `repl.cpp`, tecla sem eco: `_getch` no Windows,
  termios no POSIX; largura/altura do terminal). TUI: `Mode::Man` + `OpenMan` + `ManDialog`; o worker chama o visualizador no meio do
  `Execute` sem segurar o mutex do modelo.
- Paginas: 49 comandos (com sintaxe, exemplos, VER TAMBEM) e 11 topicos de referencia. BIOS/variaveis/ganchos saem de tabelas de dados.
- **Conferencia contra a realidade** (`mantest`): entradas da BIOS = `JP` nas ROMs; ganchos = `CALL`/`JP` nas ROMs; variaveis = valores da
  maquina depois do boot. Isso pegou 2 erros meus durante a escrita (RDBTST/WRBTST so' existem no MSX2+; uma expectativa de quebra de linha).
- Revisao de fatos nas paginas (bits do R#9, bit 6 do mixer do PSG, SCREEN 10/11...) antes de publicar.

### Validacao
`ctest` 25/25 (Windows e Linux). Visto na tela: `help` (tabela agrupada), `man type`/`status`/`emu`/`portas`/`vdpregs`/`teclado` no console e a
janelinha do `man type` na TUI (`--tui --run "man type" --dump`).
Nao conferido: o pager em um terminal de verdade (teclas de pagina), a janelinha na TUI viva (so' o quadro `--dump`), as paginas que citam
comportamento de hardware real alem do que as ROMs/maquina confirmam (ex.: bits do VDP, formatos de PLAY) -- escritas de memoria, revisadas, sem fonte externa aqui.

---

## v1.37.8 -- "Knightmare: Teclas no --keys" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- `--keys` com a mesma sintaxe de teclas do `type`.

### O que foi feito
- `Commander::ParseTypeSpec(Machine&, spec, literal, out, erro)` virou estatico e publico; `literal = true` (o `--keys`) trata aspas e espacos como
  texto comum, mantendo `{tecla}`, `{a}+{b}`, `{tecla*N}` e a barra-n. `cli.cpp` digita cada "digitacao" (teclas juntas, 6 quadros apertadas e 6 soltas).
- Teste em `controltest` (texto literal + `{enter}` + `{ctrl}+{c}` = 12 digitacoes; tecla desconhecida = erro) e uma execucao real:
  `--keys "print \"oi\" {enter}print 2+3{enter}" --text` mostra `oi` e `5` na tela.
- Efeito colateral documentado: `{` em `--keys` deixou de ser texto.

### Validacao
`ctest` 24/24 (Windows e Linux).

---

## v1.37.7 -- "Knightmare: Teclas no type" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- mandar caracteres de controle com o `type` (ex.: `KEY OFF` + Enter). Desenho dele: `{tecla}` fora das aspas,
`{ctrl}+{c}` junto x `{ctrl}{c}` em sequencia.

### O que foi feito
- `Commander::ParseTypeSpec` le o argumento da linha CRUA (o tokenizador perde a diferenca entre texto entre aspas e `{tecla}`).
  `TypedKey` agora e' uma LISTA de teclas (desce na ordem, sobe na inversa) em vez de tecla + shift. `Machine::IsKeyName` valida os nomes.
- Apelidos (return, escape, backspace, delete, insert, control, cls, break); `{tecla*N}`; `{ctrl+c}`.
- `type` sem argumento / tecla desconhecida / `{` sem `}` = erro claro; nada e' digitado pela metade (a fila so' recebe se tudo valida).

### Validacao
`controltest` com casos novos (BASIC executa `print 77` + `{enter}`; contagem de teclas de `{ctrl}+{c}` x `{ctrl}{c}`; erros). Conferido na TUI real com
`tools/conperf`: `type "key off" {enter}` = 8 teclas. `ctest` 24/24 (Windows e Linux).
Nao conferido a olho: o efeito de Control-C/STOP dentro de um programa BASIC em execucao (so' a contagem e a fila).

---

## v1.37.6 -- "Knightmare: TUI sem sujeira" (Konami) (2026-10-16)

**Fase:** relato do usuario -- na TUI, comandos com saida mostravam codigos ANSI e caracteres desalinhados.

### Causa e correcao
- `WorkerLoop` trocava `std::cout.rdbuf()` (global, vale para todas as threads) por um buffer de captura durante o comando; o FTXUI escreve o
  quadro em `std::cout`, entao um redesenho nesse intervalo ia parar no historico (e a tela real ficava sem o quadro).
- `ThreadCaptureBuf` (instalado uma vez em cout/cerr): se a thread corrente tem `g_capture` (thread_local), grava la'; senao repassa ao
  buffer original. So' o worker captura.
- Limite conhecido: uma ferramenta que escreva em cout de OUTRA thread propria durante o comando vai para o terminal, nao para o historico.

### Validacao
`ctest` 24/24 (Windows e Linux); `regs` conferido na TUI real com `tools/conperf` (`CONPERF_CMD`).

---

## v1.37.5 -- "Knightmare: TUI mais leve" (Konami) (2026-10-16)

**Fase:** pedido do usuario -- a digitacao no console estava "quase impossivel" (letras perdidas). Investigada com medicao; nao reproduzida,
mas a TUI deixou de redesenhar a tela inteira 12 vezes por segundo.

### O que foi feito
- `tui_app.cpp`: o `refresher` so' redesenha se algo mudou (`TakeDirty()`) ou se ha' comando rodando (`busy()`); o worker marca "sujo" a cada volta (a cada 0,4 s ou
  resposta). Teclas e cliques ja' redesenham por conta propria.
- `tools/conperf/conperf.cpp`: mede a latencia de digitacao do `--cli`/`--tui` (teclas injetadas no console, leitura do buffer da tela).
- Medidas (conhost, 8 nucleos): 36/36 letras a 3, 15 e 30 ms; `--cli` mediana 16 ms, TUI 32-47 ms; emulador ~30% de um nucleo e ~350 quadros/s
  headless. Atras de um ConPTY (Windows Terminal) NAO consegui medir (o harness ficou instavel) -- e' o ponto em aberto.

### Validacao
`ctest` 24/24 (Windows e Linux).

---

## v1.37.4 -- "Knightmare: Teclado japones" (Konami) (2026-10-16)

**Fase:** o bug do teclado achado ao tirar as capturas do README, mais a conferencia em tela do F10 e do splash.

### O que foi feito
- `Machine::KeysForCharLayout(c, layout, ...)` com duas tabelas (internacional e japones/JIS); `keyboard_layout()` le o byte 002Ch da BIOS;
  `type`, `--keys` e o modo terminal passam a usar o layout da BIOS carregada. Tecla nova `ro` na matriz (Fortran + tabela de nomes).
- Verificacao: as tabelas sao conferidas contra as tabelas reais das BIOS do repositorio (nao contra uma lista escrita a mao).
- `tools/wincap/wincap.ps1` (teclas por PostMessage + captura so' da janela) e a conferencia em tela do painel F10 e do splash.

### Validacao
`ctest` 24/24 (Windows e Linux). Janela real: F10, ENTER, setas, B, M, G (com digitos), C e Q funcionam; splash visivel com `--splash`.
Nao conferido: o teclado frances/ingles/alemao (usam o internacional); a pontuacao no modo terminal (so' o codigo muda, igual ao `type`).

---

## v1.37.3 -- "Knightmare: Depurador na TUI" (Konami) (2026-10-13)

**Fase:** o painel Z80 Debugger tambem na TUI, a pedido do usuario; fecha o dia com a documentacao e o README atualizados.

### O que foi feito
- `control::Commander`: `dbgui open|show|key|char` (guarda um `dbgui::DebugView`, o modelo da janela); a TUI so' envia teclas e desenha o texto.
- `TuiModel`: modo `Debugger`, `OpenPanel`/`SetPanel`/`KeyDebugger`; `tui_app`: o painel (`PanelDialog`, adaptado a altura do terminal), a saida de
  `dbgui` roteada para o painel e a abertura automatica em eventos de parada.
- Documentacao: MANUAL (depurador na TUI), debug-spec 6c, control-spec, tui-spec, README (estado atual, o que funciona hoje, limites corrigidos).

### Validacao
`ctest` 24/24 (Windows e Linux); `controltest` e `tuitest` ganharam os casos do painel. Fumaca real: emulador em janela com `--debug --break 0x0038`,
TUI com `--attach --run go --dump`: o evento de breakpoint abriu o painel sozinho (registradores, `*>0038`, cursor, ajuda) num terminal de 80x24.

---

## v1.37.2 -- "Knightmare: Depurador na janela" (Konami) (2026-10-13)

**Fase:** o depurador visual da janela (Z80 Debugger do fMSX), pedido do usuario, em cima do depurador basico da 1.37.0.

### O que foi feito
- `DebugView` (modelo, teclas, 3 vistas, pergunta de endereco), `Grid`/`Font`/`RenderRgba` (a fonte do MSX tirada da BIOS por CGTABL,
  6x8 pixels por caractere) e a ligacao na janela (F10, menu Depurador, abertura automatica, captura do teclado, textura em escala inteira).
- Layout pedido: registradores nas primeiras linhas; `ENDERECO: bytes instrucao`; dump com 8 bytes e os caracteres do MSX.

### Validacao
`ctest` 24/24 (Windows e Linux); `controltest` ganhou ~45 verificacoes do painel. Conferido a olho: as vistas de desmontagem e de dump
desenhadas com a fonte da BIOS (PNG gerado pelo teste com `FWMSX_DBGVIEW_PPM`). Fumaca real: a janela com `--debug` sobe com a maquina
parada pelo depurador e responde ao console (`stepi`, `go`, `status`); ao dar `go` o painel fecha sozinho. Nao conferido a olho: o painel NA JANELA (o teclado/F10 real).

---

## v1.37.1 -- "Knightmare: Splash e icone" (Konami) (2026-10-13)

**Fase:** desvio pedido pelo usuario (identidade visual do aplicativo): splash de abertura e icone proprio; mais a documentacao
do depurador no manual.

### O que foi feito
- `src/splash/` (leitor de PNG, politica, meios blocos para o terminal, icone RGBA), `src/common/settings` (`fwmsx.cfg`),
  `RunSplashPhase` na janela, `--splash`/`--no-splash`, item no menu Interface, `emu start` sem splash.
- Icone: recurso do Windows (`fwmsx.rc.in`, gerado com o caminho absoluto), `glfwSetWindowIcon` no Linux, `fwmsx.png`/`fwmsx.desktop`.
- Pacotes: `fwMSX-splash.png` ao lado do executavel (Windows e Linux) e icone/atalho no tar.gz.
- MANUAL: depurador (passo a passo + referencia) e splash/icone/configuracoes.

### Validacao
`ctest` 24/24 (Windows e Linux). A janela leva ~3,9 s a mais com splash; `--no-splash` e `splash=0` pulam; `--splash` forca. O
Windows associa ao `fwMSX.exe` o icone do fwMSX. Nao verificado a olho: o desenho do splash na janela e no terminal (so' por
teste de unidade e pelo tempo).

---

## v1.37.0 -- "Knightmare" (Konami) (2026-10-13)

**Fase:** o depurador basico do emulador e os eventos assincronos da ponte de controle (primeiro passo rumo a integracao
com msxIDE/PaleoBASIC e a um depurador sofisticado -- openMSX + Compass/Konpass + Mega Assembler + Super-X + DEBUG do MS-DOS).

### O que foi feito
- `src/debug/`: `Debugger` (breakpoints, watchpoints, parada, fila de eventos), `DebugBus` (decorador do barramento do Z80),
  `dbg_flow.c` (classificacao de instrucao), `dbg_fmt.f90` (flags), `dbg_range.asm` (teste de faixa, NASM dual-ABI).
- `Machine::RunFrame()` agora usa `StepInstruction()`; para antes do breakpoint e depois do watchpoint; `RunUntilStop()`.
- `Commander`: `break unbreak breaks watch unwatch watches stepi next until go stop jump dis mem stack reg slots events`.
- Servidor: `events on|off` por conexao, `PushEvent()`, respostas `ok +N`. Cliente (`EmuLink`): `ok +N`, fila de eventos,
  `PollEvents()`. Console: `[evento]`, `wait`. TUI: menu Depurador, eventos no historico. Modo terminal e janela: eventos.
- Parametros `--break`, `--watch`, `--debug`.

### Validacao
`ctest` 23/23 (Windows e Linux). Fumaca real: janela com `--debug --break`, console por pipe (`stepi`, `go`, `wait`
recebendo `[evento] break ...`) e TUI (`--attach --run --dump`).

---

## v1.36.2 -- "Salamander: MegaROM com AB no fim" (Konami) (2026-10-13)

**Fase:** primeiro bug de jogo achado pela comparacao com o fMSX usando o acervo do file-hunter: o R-Type nao iniciava.

### O que foi feito
- `MemorySystem::LoadRom` (`src/memmap/cpp/memory_system.cpp`): MegaROM com banco-de-8KB nao potencia de 2 e' completada
  para 2^n bancos espelhando o fim (como `LoadCart` do fMSX) e a mascara de banco vira 2^n-1.
- `memmap_attach_megarom` (`src/memmap/core/slot_state.c`): sem "AB" no inicio e com "AB" nas duas ultimas paginas,
  comeca em N-2, N-1, N-2, N-1 (como `ResetMSX` do fMSX).
- Teste novo (`memmaptest` secao 27) e 19 jogos provisorios em `tests/media/` na comparacao.

### Validacao
`ctest` 23/23 (Windows e Linux). Comparacao com o fMSX original: 26/26 casos aprovados (R-Type 100% pixel a pixel;
Metal Gear passou a ser testado como MSX1, que e' o modelo dele).

---

## v1.36.1 -- "Salamander: Texto de SCREEN 0 como no fMSX" (Konami) (2026-10-13)

**Fase:** primeiro fruto da comparacao com o fMSX original: o texto de 40 colunas saia 1 pixel a esquerda.

### O que foi feito
- `Machine::RenderFrame` (`src/machine/machine.cpp`): texto de 40 colunas (240) comeca no pixel 9 da area de 256 (MSX1:
  `+1`; MSX2/MSX2+, tela de 512: `+2`, tambem no TEXT80), como `RefreshLine0`/`RefreshLineTx80` do fMSX.
- Ferramenta `tools/fmsx_ref/Headless.c` + `tools/fmsxgo_ref/compare.py`: o fMSX original compilado sem janela como
  referencia (o fmsxgo, ainda em desenvolvimento, como segunda opiniao). Armadilha achada: o fMSX grava
  `CMOS.ROM` e le na partida seguinte (o `compare.py` apaga entre os casos).

### Validacao
`ctest` 23/23 (Windows e Linux). Comparacao com o fMSX original: 7/7 casos a 100% pixel a pixel (antes, 3 deles
tinham deslocamento de 1 pixel). TEXT80 sem caso de teste ainda.

---

## v1.36.0 -- "Salamander" (Konami) (2026-10-13)

**Fase:** primeiro bloco do montador Z80 integrado (`fwmsx --asm`), reescrita clean-room do asMSX 1.2.0 -- passo
rumo ao MSX-PoorManOS. Esta versao entrega o nucleo (instrucoes, expressoes, pre-processador, saidas `.z80`/`.sym`/
`.txt`); os formatos de ROM/BASIC/fita ficam para as proximas patches.

### O que foi feito
- C: `src/asm/core/z80_enc.*` (todas as instrucoes Z80, inclusive as nao documentadas IXH/IXL); Fortran:
  `asm_math.f90`; C++: lexer, expressoes (precedencia e erros do asMSX), pre-processador (`include`, macros,
  repeticao), montador de ate 4 passadas + passada final, CLI. Assembly (NASM): ainda previsto (sintese de pulsos).
- `fwmsx --asm`, comando `asm` do console, secao no MANUAL.
- Oraculo de teste: `tools/asmdiff/` (geradores Python) roda o `asmsx.exe` real e grava os casos em
  `tests/asm/golden/*.golden` (expr, instr, misc, lang); `asmtest` repete todos.
- Fechamento da rodada (7 casos que faltavam): `.filename` (as saidas passam a `<nome>.<ext>`), `.start`
  (valida a expressao; sem efeito ate existir ROM/BASIC), `org -1` (o asMSX grava um byte antes do endereco 0 e a
  saida o inclui; abaixo disso e' lixo de heap la' -- aqui continua "memory overflow") e `im <rotulo adiante>`
  (um valor ruim de simbolo ainda nao definido nao aborta mais a passada de simbolos).

### Validacao
`ctest` 23/23 (Windows e Linux). `asmtest`: todos os casos dos 4 arquivos golden (5 ignorados por dependerem de
opcoes de CLI que o teste nao simula).

---

## v1.35.0 -- "Vampire Killer" (Konami) (2026-10-09)

**Fase:** a TUI de menus que o usuario tinha em mente desde o inicio: colocar em menus os comandos da CLI
e controlar o emulador em janela (o modo terminal `--term` ficou como estava).

### O que foi feito
- `src/tui/tui_model.*` (modelo, sem FTXUI) e `tui_app.*` (desenho FTXUI + thread de comandos + status).
- Menus: Emulador, Arquivo, Maquina, Midia, Ferramentas, Ajuda; assistentes de varios passos; navegador
  de arquivos com filtro por extensao; linha de comando com historico/cursor/Tab; mouse.
- A saida das ferramentas (que escrevem em `std::cout`) e' capturada para o historico.
- `--dump` / `--run` / `--attach` para teste de fumaca e automacao sem terminal.

### Validacao
`ctest` 22/22 (Windows e Linux). Fumaca real: menus, conexao ao emulador em janela, `peek`/`regs`,
`newdisk`, erro, e `romdb stats` no historico. Teclado/mouse num terminal de verdade: teste do usuario.

---

## v1.34.0 -- "Metal Gear" (Konami) (2026-10-09)

**Fase:** o modo terminal do emulador (`--term`), combinado com o usuario: a maquina dentro do terminal, as mesmas opcoes da
janela e a mesma camada de comandos (Commander) da ponte de controle e do console.

### O que foi feito
- `src/term/term_app.{h,cpp}`: `Runner` (thread da maquina a 60 quadros/s, implementa `control::Host`) e a
  interface FTXUI (tela do MSX, barra de status, menu, linha de comando).
- Tela: texto de verdade em SCREEN 0/1/TEXT80; meios blocos reduzidos nos graficos.
- Teclado: terminal nao envia "soltar"; cada tecla vale por 6 quadros, renovada pelo auto-repeat.
- Captura PNG movida para `src/machine/screenshot.*` (janela e modo terminal).

### Validacao
`ctest` 21/21. Modo terminal validado pela ponte de controle e pela saida renderizada; digitacao num terminal de
verdade depende de teste do usuario. Ver `doc/term-spec.md`.

---

## v1.33.0 -- "Penguin Adventure" (Konami) (2026-10-09)

**Fase:** pedido do usuario: um console tipo openMSX (`fwmsx --cli`) de onde se inicia o emulador e
que continua livre para mandar comandos a ele pela conexao que ja' existe (a ponte de controle).
Decisao: o REPL e' um CLIENTE, nao embute a maquina (emulador em processo separado).

### O que foi feito
- `src/repl/`: `EmuLink` (cliente TCP, porta livre, iniciar processo sem herdar o console),
  `ReplSession` (comandos locais + encaminhamento + ferramentas registradas) e `repl.cpp` (replxx).
- Ferramentas existentes como comandos: `newdisk`, `romdb`, `cas`, `fitadb`, `msxdisk`.
- Windows: handles do sistema apontam para o console (replxx).
- **`fwMSXc.exe`**: achado ao testar o console interativo de verdade (letras trocadas, lentidao,
  fechando sozinho): um programa de janela nao e' esperado pelo shell, que disputa o teclado com ele.
  Solucao: o build gera tambem o mesmo programa com o subsistema console (`tools/pe_subsystem`).

### Validacao
`ctest` 21/21 (Windows e Linux). Smoke test com o executavel real por pipe. Ver `doc/repl-spec.md`.

---

## v1.32.0 -- "Nemesis" (Konami) (2026-10-09)

**Fase:** primeiro passo de "TUI + ferramentas integradas": a ponte para outros programas falarem
com o emulador. Metodo escolhido com o usuario: protocolo de texto, uma linha por comando, TCP em
localhost (em vez do XML do openMSX); so' pergunta/resposta, sem eventos assincronos ainda.

### O que foi feito
- `src/control/command.{h,cpp}` (Commander + interface Host) e `server.{h,cpp}` (TCP Winsock/POSIX).
- Janela: `--ctl-port N`; ganchos de Host (pausa, sair, trocar cartucho, captura PNG); `Tick()` por
  quadro para a fila do `type`; `fwmsx.port`.
- Achado de projeto: dentro de aspas a barra invertida e' literal (so' `\"` e' escape), senao
  `"C:\temp"` virava `C:<tab>emp`; o `\n` e' tratado so' pelo comando `type`.

### Validacao
`ctest` 20/20 (Windows e Linux). Smoke test com a janela real via socket. Ver `doc/control-spec.md`.

---

## v1.31.0 -- "King's Valley: Disco novo em branco e FORMAT" (2026-10-09)

**Fase:** pedido do usuario: criar discos novos em branco e formatados para A: ou B: (180/360/720 KB,
5 1/4 e 3 1/2, sem 1,44 MB), convertendo o msxDiskUtil (PureBasic) para C++ misturado com C e
Assembly, e aceitar FORMAT / CALL FORMAT. **Descoberta antes de codar:** o `src/msxdisk` ja' era um
port do msxDiskUtil (so' criava 720 KB); o usuario preferiu refazer do zero mesmo assim, entao
nasceu `src/diskfmt/` em paralelo (as duas convivem).

### O que foi feito
- `src/diskfmt/`: nucleo em C (formatos, BPB, FAT12, boot, raiz), Assembly (`diskfmt_fill`, E5h),
  C++ (arquivo + `fwmsx --disknew`). Menu **Midia > Disco > Novo disco em branco em A:/B:**.
- WD2793: comando **WRITE TRACK** (`track_byte()` em `src/fdc/core/fdc_state.c`).
- Achado: o DISK.ROM do fMSX (Disk BASIC 1.0) formata em **face simples** (F8h, 720 setores) e nao
  pergunta o tipo; o teste `machinetest` 7d confirma o ciclo completo.

### Validacao
`ctest` 19/19 (Windows e Linux). Ver `doc/diskfmt-spec.md`.

---

## v1.30.1 -- "King's Valley: Atalhos de save-state, captura de tela e janela sem console" (2026-10-08)

**Fase:** primeira versao pela NOVA politica de numeracao (patch para feature pequena -- ver
`CLAUDE.md`, regra 1): pedido do usuario depois de notar que 1.19 -> 1.30 tinha gerado releases
demais. Tambem junta pequenas melhorias pedidas na mesma sessao.

### O que foi feito

- **Save-state: 9 slots + atalhos.** F6 salva / F7 carrega o slot atual (`fwmsx-estado-N.sst`), F8/F9
  trocam de slot; menu Arquivo > Slot de estado. F1-F5 sao teclas do MSX, por isso comecam em F6.
- **Aviso na tela** (toast): salvou/carregou/troca de slot/captura; laranja e mais longo para
  erro ou aviso.
- **Validacao da midia no save-state**: secao `MEDA` (CRC32 da BIOS e do cartucho); midia
  diferente -> o estado carrega mesmo assim, com `Machine::state_warning()` e aviso na tela. Disco
  e fita ficam de fora. Teste novo em `machinetest` (secao 5b).
- **F12 / Arquivo > Capturar tela (PNG)**: `fwmsx-AAAAMMDD-HHMMSS.png`, proporcao correta (miniz).
- **Janela sem console no Windows**: `fwMSX.exe` e' do subsistema "windows" (`-mwindows`);
  `main.cpp` religa o console do terminal quando ha' um (so' as saidas nao redirecionadas).
  Efeito colateral: `cmd`/PowerShell nem sempre esperam o fim do programa.
- **Ajuda > Teclado** lista os atalhos do emulador.
- **BIOS Gradiente Expert 1.1**: investigado -- o texto "G r a d i e n t e" com espacos esta'
  gravado na propria ROM (offset 32513), nao e' bug do emulador.

### Validacao

Build Windows e Linux (WSL), `ctest` 18/18 nos dois. Atalhos, toast, captura e janela sem
console testados pelo usuario na janela em 2026-10-08.

---

## v1.30.0 -- "King's Valley: Menus reorganizados + mapper do cartucho na janela" (2026-10-08)

**Fase:** feedback direto do usuario depois de usar a 1.29.0 (save-state) e tentar testar a 1.28.0
(MSX-DOS 2) pela janela: "save state validade e funcionando normalmente, Nao descobri como inserir
o cartucho do MSX-DOS2 na configuracao para testar ele. Outra coisa os menus estao ficando muito
separados" -- com a sugestao explicita de agrupar Disco/Fita/Cartucho num menu Midia, Video/Exibir
num menu Tela, e Configuracoes dentro de Ferramentas.

### O que foi feito

- **Mapper do cartucho pela janela**: a causa raiz do problema do usuario era que
  `Machine::SetCartridge()` sempre resetava o mapper do cartucho para `MEMMAP_MAPPER_NONE`
  (deteccao automatica) -- nao havia NENHUM caminho na GUI para escolher um mapper explicito, so'
  a CLI (`--cart <rom> msxdos2`) tinha essa opcao. `SetCartridge()` ganhou um parametro `mapper`
  (default preserva o comportamento antigo) e `CartridgeMapper()` novo para ler de volta. A GUI
  ganhou DOIS pontos de acesso -- um submenu **Mapper** no menu Cartucho (lista completa: auto +
  6 mappers de sempre + MSX-DOS 2; com um cartucho ja' carregado, trocar o mapper reinicia na hora
  com o MESMO arquivo) e um combo equivalente na linha da ROM dentro da **Configuracao de
  slots...** (onde o usuario provavelmente tinha procurado primeiro, pela palavra "configuracao").
- **Menus reagrupados** (`src/machine/gui/emu_window.cpp`): os 13 menus do topo viraram 9.
  - **Midia**: envolve os 3 menus existentes (Disco, Fita, Cartucho) como submenus, sem mudar
    nada do conteudo de cada um.
  - **Tela**: envolve os 2 menus existentes (Exibir, Video -- os filtros/interpolacao/scanlines)
    como submenus.
  - **Ferramentas**: incorporou o unico item do antigo menu Configuracoes (**Interface** --
    tema, fonte, moldura) como um submenu a mais, antes dos placeholders "(em breve)" que ja'
    existiam; o menu Configuracoes (que so' tinha isso) foi removido.
  - Arquivo, Maquina, Som, Joystick, ROMs e Ajuda continuam como estavam.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.30.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.30.0-linux.tar.gz`.
- `ctest`: 18 suites, incluindo a secao 2b nova do `machinetest`/`machine_frames`. Windows: 18/18.
  Linux: 18/18.

## v1.29.0 -- "King's Valley: Save-state" (2026-10-08)

**Fase:** item 5 de "vamos fazer o 1, o 4 e o 5 na sequencia" (item 4, MSX-DOS 2, foi a 1.28.0) --
o ultimo item dessa sequencia pedida pelo usuario. Pedido explicito logo depois da 1.28.0:
"prepare o item 4 e 5 entao na sequencia".

### Decisao de design: sobrepor na maquina ja' rodando, nao recriar do zero

A alternativa "completa" seria serializar o `MachineConfig` inteiro (caminhos de BIOS/
cartucho/disco/fita + layout de slots) e recriar a `Machine` do zero ao carregar. Decisao: NAO
fazer isso em v1 -- `LoadState()` aplica direto sobre a `Machine` ja' rodando, pressupondo que a
MESMA midia continua carregada (como a maioria dos emuladores reais: carregar o estado errado
sobre o jogo errado produz imagem/som estranhos, nunca um erro). Isso eliminou toda a
complexidade de serializar/validar caminhos de arquivo e reconstruir o layout de slots.

A pesquisa de onde a feature deveria entrar na interface tambem veio do PROPRIO codigo: o menu
**Arquivo** ja tinha um item desabilitado "Salvar estado (em breve)" desde a janela com menus
(1.16.0) -- a localizacao e o padrao de dialogo de arquivo (`ShowSaveFileDialog`/
`ShowOpenFileDialog`, o mesmo ja usado por "Nova fita...") ja estavam decididos pelo proprio
projeto, sem precisar perguntar ao usuario.

### O que foi feito

- `Machine::SaveState()`/`Machine::LoadState()` (`src/machine/machine.{h,cpp}`): formato
  binario proprio TLV (tag de 4 bytes + tamanho + payload), versionado, com secoes para Z80,
  VDP (ate' `scanline_snapshot`, que e' derivado e nunca salvo), PSG (pulando `joy[]`/
  `cassette_in`, entrada externa), SCC, OPLL, PPI (so' os registradores do chip, sem
  `key_state`), FDC (registradores do WD2793, pela memoria e/ou pelas portas, sem o ponteiro
  `ptr` que aponta pra dentro do buffer da imagem -- uma transferencia em andamento e' abortada
  ao carregar em vez de arriscar um ponteiro invalido), fita (modo/protecao/modo de gravacao/
  ponto marcado/rele do motor, sem a posicao exata de um pulso em andamento), mapper(es) de RAM
  (registrador de segmento de cada um) e o conteudo de toda RAM/RAM de mapper.
- `MemorySystem::MapperRamBase()` (`src/memmap/cpp/memory_system.h`): novo -- `PeekSlot`/
  `PokeSlot` so' enxergam a vista PAGINADA agora (os 4 segmentos visiveis em 0000h-FFFFh), que
  para um mapper de ate 1024KB (64 segmentos) deixaria de fora tudo que nao esta' paginado
  no instante exato do save. O novo metodo devolve o buffer inteiro.
- `RamMapperDevice::primary()`/`secondary()` (`src/memmap/cpp/ram_mapper.h`): accessors novos
  (faltavam) para o save-state poder identificar a combinacao de slot de cada mapper.
- Menu **Arquivo > Salvar estado.../Carregar estado...** (`src/machine/gui/emu_window.cpp`):
  dialogo de arquivo (`.sst`), substituindo o placeholder desabilitado.
- Testes: `machinetest`/`machine_frames`, secao 5b -- salva depois do boot + `print 1234`,
  MUTILA a maquina de proposito (300 quadros a mais, `cls`, PC do Z80 e R7 do PSG escritos
  direto para valores errados -- prova que a restauracao NAO e' coincidencia), recarrega e
  confere PC/R7/VRAM inteira restaurados byte a byte, mais os 2 caminhos de erro (arquivo
  inexistente, assinatura errada).

### Erro cometido e corrigido nesta rodada (de novo)

A mesma licao da 1.28.0 se repetiu: a primeira build desta feature (so' para rodar os testes)
foi feita ANTES de bumpar a versao, sobrescrevendo `dist/fwMSX-1.28.0.zip`/`dist/fwMSX.exe` ja
publicados. Detectado com `git status --short dist/` ANTES de qualquer commit; corrigido com
`git checkout -- dist/fwMSX-1.28.0.zip dist/fwMSX.exe`, so' DEPOIS bumpando a versao para 1.29.0
e reconstruindo do zero. Nenhum dado publicado foi perdido.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.29.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.29.0-linux.tar.gz`.
- `ctest`: 18 suites, incluindo a secao 5b nova do `machinetest`/`machine_frames`. Windows:
  18/18. Linux: 18/18.
- **Nao verificado ainda**: o fluxo da janela de verdade (so' a API tem teste automatizado).

## v1.28.0 -- "King's Valley: MSX-DOS 2 (cartucho generico)" (2026-10-08)

**Fase:** item 4 de "vamos fazer o 1, o 4 e o 5 na sequencia" (item 1, efeitos de rastreio, foi a
1.27.0). Pesquisa ja' tinha sido feita numa rodada anterior: um `MSXDOS2.ROM` real ja' estava no
repositorio (`resource/kizuna/.../Emulicious/MSX/MSXDOS2.ROM`), e o mapper de referencia do
openMSX (`RomMSXDOS2`, GPL, so' estudo) ja' tinha sido lido.

### Investigacao: o cartucho e' mais simples do que a nota antiga supunha

A nota antiga em `doc/fdc-spec.md` dizia "Sem DiskROM do MSX-DOS 2 (precisa de MSX2 + mapper de
RAM)" -- supondo que MSX-DOS 2 precisasse substituir a DiskROM do slot 3:1 e de uma RAM extra
mapeada. Lendo a extensao `msxdos2.xml` do openMSX (`resource/openMSX/share/extensions/`), a
realidade e' mais simples: o "cartucho generico de MSX-DOS 2" (como foi vendido de verdade na
Europa) e' so' mais um CARTUCHO, em QUALQUER slot, com um mapper proprio -- nao mexe na DiskROM
nem precisa de RAM mapeada. `RomMSXDOS2::reset()` deixa a pagina 0 (0000h-3FFFh) e a pagina 2
(8000h-BFFFh) vazias, so' a pagina 1 (4000h-7FFFh) mostra o banco 0 da ROM; `writeMem()` troca
esse banco quando o endereco certo (lido do byte 94h da propria ROM) e' escrito.

### O que foi feito

- `MEMMAP_MAPPER_MSXDOS2` (`src/memmap/common/memmap_types.h`): novo valor no enum, sem numero
  `MAP_*` de referencia (nao existe no fMSX original).
- `src/memmap/core/slot_state.c`: `memmap_attach_megarom()` agora le `data[0x94]` (so' quando o
  mapper e' MSXDOS2) e guarda num campo novo (`msxdos2_range`); os quartos 2 e 3 (8000h-BFFFh)
  ficam sempre vazios para esse mapper, igual ao FM-PAC ja' fazia para a sua propria janela.
  `memmap_try_bank_switch()` ganhou o case `MEMMAP_MAPPER_MSXDOS2`: confere o endereco contra o
  `range` guardado (00h -> so' 7FF0h; 60h -> qualquer escrita em 6000h-6FFFh; 7Fh -> so' 7FFEh) e,
  se bater, troca os quartos 0+1 juntos (granularidade de 16KB, igual GEN16/ASCII16 ja' faziam
  para as suas proprias trocas "largas").
- `src/memmap/cpp/memory_system.cpp`: `LoadRom()` recusa a ROM se o byte 94h nao for um dos 3
  valores conhecidos (mesma checagem que `RomMSXDOS2` faz via excecao no openMSX).
- `src/machine/cli.cpp`: `--cart <arquivo> msxdos2` reconhecido pelo parser de mapper.
- Testes: `memmaptest`/`memmap_slots`, secao 25 nova -- ROM sintetica de 64KB com um byte
  distinto por banco de 16KB, testando os 3 valores de `range`, a pagina 8000h-BFFFh sempre
  vazia, e a recusa de um byte 94h desconhecido (12 checagens).
- **Smoke test com hardware real** (executavel isolado, `C:\dos\temp\msxdos2_smoke\`): um
  `MSXDOS2.ROM` real (64KB, `range=60h`) e um disco de 720KB real
  (`resource/kizuna/.../MSX-DOS/MSXDOS-230.DSK`, com `MSXDOS2.SYS`/`COMMAND2.COM`/subdiretorios
  `KHELP`/`UTILS`/`HELP`) rodados via `--cart MSXDOS2.ROM msxdos2 --disk MSXDOS-230.DSK --keys
  "dir|cd khelp|dir|"`: o kernel MSX-DOS 2 sobe, `dir` lista arquivos e subdiretorios
  corretamente, `cd khelp` navega para o subdiretorio e `dir` la' dentro mostra os arquivos
  `.HLP` certos -- tudo sem precisar de NENHUM mapper de RAM.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.28.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.28.0-linux.tar.gz`.
- `ctest`: 18 suites, incluindo a secao 25 nova do `memmaptest`/`memmap_slots`. Windows: 18/18.
  Linux: 18/18.

## v1.27.0 -- "King's Valley: Efeitos de rastreio (paleta/scroll por linha)" (2026-10-08)

**Fase:** primeiro item de "vamos fazer o 1, o 4 e o 5 na sequencia", pedido pelo usuario depois
de uma pergunta exploratoria sobre o que falta no basico do emulador -- efeitos de rastreio no
meio do quadro (paleta/scroll trocados por uma interrupcao de linha, R#19/IE1), ate agora "fora de
escopo" em `doc/vdp-spec.md`/`doc/msx2-spec.md`/`doc/msx2p-spec.md`.

### Investigacao primeiro, antes de codar

Antes de supor que fosse um problema de timing de interrupcao, uma investigacao dedicada
confirmou: `vdp_step_scanline()` ja' gera a interrupcao IE1 no scanline certo (comparando `R#19`
ajustado por `R#23`/VScroll), e `Machine::RunFrame()` ja' intercala o Z80 com o VDP T-state a
T-state, entregando a interrupcao (`cpu_->interrupt()`) no instante exato -- qualquer escrita de
registrador feita por uma ISR ja' acontece na hora certa DURANTE a execucao do quadro. A lacuna
real estava inteiramente do lado da renderizacao: `Machine::RenderFrame()` roda DEPOIS do quadro
inteiro ter terminado, e le o `VdpState` FINAL (unico) para desenhar TODAS as linhas -- uma troca
de paleta no meio do quadro aparecia retroativa a tela inteira, nunca so' a partir da linha onde
realmente aconteceu.

### O que foi feito

- **`VdpScanlineSnapshot`** (`src/vdp/core/vdp_state.h`): struct com os campos que
  `vdp_render.c`/`vdp_sprites.c` de fato leem para decidir a cor de um pixel -- `regs[64]`,
  paleta RGB (`palette_r/g/b[16]`), `scr_mode`, `model`, `x_fg`/`x_bg` (cores do "blink"),
  `chr_tab`/`col_tab`/`chr_gen`/`spr_tab`/`spr_gen` e as 3 mascaras de tabela. Confirmado por
  `grep` exaustivo (com cuidado extra para nao misatribuir linha de arquivo errado) que `vram[]`
  e `status[]` NAO entram: `vram` nao muda por linha, e todo `v->status` e' escrito so' por
  `vdp_sprites_update_status()`/`check_collision()` (visiveis pela porta 99h ao depurador/CPU),
  nunca lido por renderizacao.
- `vdp_capture_snapshot()`/`vdp_apply_snapshot()` (`vdp_state.c`): copia os campos acima de/para
  um `VdpScanlineSnapshot`. `vdp_step_scanline()` chama `vdp_capture_snapshot()` ao fim do
  processamento de CADA linha (guardado em `VdpState::scanline_snapshot[VDP_MAX_SCANLINES]`,
  `VDP_MAX_SCANLINES = 313` cobre 0-311 PAL + margem).
- `Machine::RenderFrame()` (`src/machine/machine.cpp`) passou a copiar `VdpState` POR VALOR (nao
  mais `const VdpState&` para o estado vivo da maquina -- seguro porque o struct e' POD, sem
  ponteiro nenhum) e, nos dois lacos de linha (MSX1 e MSX2), chama
  `vdp_apply_snapshot(&v, &v.scanline_snapshot[y])` imediatamente antes de `vdp_render_line(&v, y,
  ...)`. Zero mudanca de assinatura publica ou de chamador (`cli.cpp`, `emu_window.cpp`).
- Testes: `vdp2test`/`vdp_msx2`, secao 10 nova -- simula 100 scanlines (`vdp_step_scanline()` duas
  vezes por linha, replicando o alternar HRefresh/HBlank real), troca a paleta do indice 1 na
  linha 50, e confirma: snapshot da linha 10 (antes) mostra a cor ANTIGA; snapshot da linha 80
  (depois) mostra a cor NOVA; e, de controle, renderizar a linha 10 do estado FINAL sem snapshot
  (o comportamento antigo) mostra a cor nova -- prova direta de que o snapshot corrige o problema.
- Docs: `doc/msx2-spec.md`/`doc/msx2p-spec.md` (secao 6) deixaram de listar isso como "fora de
  escopo"; `doc/SPEC.md` (secao 5) marcou o item como feito.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.27.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.27.0-linux.tar.gz`.
- `ctest`: 18 suites, incluindo a secao 10 nova do `vdp2test`/`vdp_msx2`. Windows: 18/18. Linux:
  18/18.

## v1.26.0 -- "King's Valley: Vampier: Platform, CRC32 e Tamanho" (2026-10-08)

**Fase:** ultimo item da lista original de pendencias de ROMs -- "importar o JSON do Vampier, se
for util" -- pedido pelo usuario em sequencia logo depois do mapper/verify.

### Investigacao primeiro, antes de codar

Baixado e comparado os dois formatos reais que o Vampier oferece (`romdb.vampier.net/downloads.php`):
- **JSON** (`json-msxromsdb.zip`, 641 KB): estrutura aninhada jogo -> lista de hashes, com
  `gamename`/`year`/`publisher`/`platform` por jogo e `sha1`/`romtype`/`dump`/`remark`/`FileName`
  por hash.
- **SQL** (`sql-msxromdb.zip`, 851 KB, JA' importado desde a 1.18.0): tabelas `msxdb_rominfo`
  (inclui `Platform`, `GenMSXId`, `RomCat`...) e `msxdb_romdetails` (inclui `CRC32`, `FileSize`,
  `StartBytes`, `Suspect`...) com MAIS colunas que o JSON.

Conclusao: o JSON e' um SUBCONJUNTO do SQL (mesma base, menos campos) -- importa-lo nao traria
dado novo nenhum. O ganho de valor real estava em `VampierSearch()`: a consulta SQL interna so'
selecionava 7 das muitas colunas ja' importadas, ignorando `Platform`/`CRC32`/`FileSize` que
estavam no banco desde sempre. Essa foi a mudanca de fato implementada (com a concordancia do
usuario, apresentada como pergunta antes de codar).

### O que foi feito

- `VampierHit` (`src/romdb/store/romdb.h`) ganhou `platform`, `crc32` e `file_size`.
  `RomDb::VampierSearch()` (`store/romdb.cpp`) agora seleciona `i.Platform`, `d.CRC32` e
  `d.FileSize` tambem (com `IFNULL`/checagem de `NULL` -- um campo ausente no dump vira string
  vazia ou `-1`, nunca quebra a consulta). `fwmsx --romdb vsearch` imprime as 3 colunas novas.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.26.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.26.0-linux.tar.gz`.
- `ctest`: 18 suites (`romdbtest`/`romdb_store` com 2 checagens novas: campos presentes quando o
  dump tem os dados, vazio/-1 quando o dump tem `NULL`). Windows: 18/18. Linux: 18/18.
- Smoke test manual (executavel isolado): `--romdb vampier` de verdade (11.313 ROMs, dump real do
  site) seguido de `--romdb vsearch Nemesis` confirmou `platform`/`crc32`/`file_size` certos nas
  linhas devolvidas.

## v1.25.0 -- "King's Valley: Banco de ROMs: Mapper e Verificacao" (2026-10-08)

**Fase:** "passo 3" desta leva de sessoes, pedido pelo usuario em sequencia logo depois do banco
de fitas: os dois itens pendentes do banco de ROMs listados em `doc/romdb-spec.md`, secao 8 --
"Carregar pelo banco" (escolher o mapper) e "Verificar as ROMs baixadas contra o SHA-1 conhecido".

### O que foi feito

- **Auto-mapper**: `src/machine/cli.cpp::RunMachineCommand` ganhou `TryMapperFromRomDb()`, chamada
  depois do parse de argumentos e antes de montar a maquina -- SO' quando `--cart` foi dado SEM
  mapper explicito (nova flag local `cart_mapper_explicit` distingue "usuario escolheu" de "auto"/
  omitido, que antes eram indistinguiveis). Calcula o SHA-1 do arquivo do cartucho e consulta
  `RomDb::CartMapper()` (a tabela `cart_mappers`, ja' preenchida por `fwmsx --romdb cartsha`); se
  encontrar, converte o numero do mapper do fMSX original (0-5) para o enum publico
  `MemMapMapperType` (`FmsxMapperToMemMap()`, so' soma 1 -- `MEMMAP_MAPPER_NONE` fica antes de
  `GEN8` no enum) e usa esse mapper. Decisao importante: o ponto de insercao fica em `cli.cpp`
  (dentro do executavel `fwMSX`, que ja' linka SQLite/romdb), NAO em `machine.cpp`/`LoadRomCell` --
  ali o fallback por tamanho/conteudo (`memmap::GuessMapper()`) fica intacto, e `machinetest`/
  `msx2test` continuam sem precisar linkar SQLite.
- **`fwmsx --romdb verify`**: novo comando em `src/romdb/cli.cpp` (sem mudar a classe `RomDb`) --
  para cada ROM cadastrada, recalcula o SHA-1 do arquivo no caminho resolvido (`ResolveRomPath()`,
  relativo a' pasta de ROMs quando o caminho guardado for relativo) e compara com o SHA-1 gravado
  no banco. Reporta cada ROM faltando ou com SHA-1 divergente, e devolve codigo de saida 1 se
  achar algum problema (0 se tudo bater) -- pensado para uso em script.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.25.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.25.0-linux.tar.gz`.
- `ctest`: 18 suites (`romdbtest`/`romdb_store` com 3 checagens novas de `verify`). Windows:
  18/18. Linux: 18/18.
- Smoke test manual (executavel isolado, SEM tocar no banco de ROMs real do usuario): um cartucho
  sintetico de 64KB cadastrado no banco com mapper Konami5 via `--romdb cartsha` foi carregado com
  `--cart` SEM mapper e escolheu Konami5 sozinho (confirmado por `cart_info()`); um mapper
  explicito (`ascii8`) continuou tendo prioridade sobre o banco; um cartucho desconhecido caiu no
  fallback de sempre (heuristica por conteudo) sem erro.

## v1.24.0 -- "King's Valley: Banco de Fitas (Metadados)" (2026-10-08)

**Fase:** segundo item que o usuario pediu na sequencia nesta rodada (depois de confirmar que o
modo "nova fita" ja' tinha sido validado na janela): o item (e) da lista original de fitas --
banco de fitas (SQLite), SEM download (o site de referencia nao publica termos de uso, ver
`doc/SPEC.md`, secao 5.2, item 7, verificado em 2026-10-06 e nunca revertido).

### O que foi feito

- Novo modulo `src/tapedb/` (store/service/cli), estrutura espelhada em `src/romdb/` por analogia
  -- MESMO padrao de CRUD/busca/SQLite, mas um banco SEPARADO (`fitas/fitas.db`, nao uma categoria
  dentro de `roms.db`): `TapeDb::Add/Update/Delete/Get/FindBySha1/Search/Count/ScanFile/
  ScanDirectory`, reaproveitando `romdb::Sha1Hex()` (funcao generica, nao especifica de ROM) em
  vez de duplicar o hash.
- `fwmsx --fitadb scan|add|list|search|show|edit|del|stats` -- os mesmos comandos do `--romdb`,
  sem os de download (`fmsx`/`filehunter*`/`vampier`), que nao se aplicam aqui.
- **Auto-preenchimento do titulo**: `ScanFile()` abre a fita com o leitor existente (`cas_reader`/
  `tzx_reader`, o MESMO usado pelo `--cas`/pela janela) e usa o nome do 1o arquivo encontrado
  dentro dela como palpite de titulo -- so' para uma fita NOVA no banco; uma fita ja' cadastrada
  mantem o titulo que o usuario tiver editado, mesmo re-escaneando a mesma pasta depois.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.24.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.24.0-linux.tar.gz`.
- `ctest`: 18 suites (nova: `tapedbtest`/`tapedb_store` -- CRUD, busca, scan de arquivo/pasta com
  auto-preenchimento de titulo e preservacao de edicoes manuais, e a CLI completa). Windows:
  18/18. Linux: 18/18.
- Smoke test manual com o `fwMSX.exe` de verdade: `--fitadb add` numa fita real
  (`resource/fmsxgo/media/teste.tsx`) auto-detectou o titulo "teste" certo, aceitou `--empresa`/
  `--ano` manuais, e `--fitadb list`/`stats` confirmaram o cadastro.

## v1.23.0 -- "King's Valley: Navegacao de Blocos do TZX" (2026-10-08)

**Fase:** terceiro e ultimo item do "passo 1" pedido pelo usuario -- navegar os blocos de controle
do TZX (grupos/lacos/saltos), que antes eram so' pulados com seguranca (o comprimento sempre
conhecido), mas sem afetar a ordem de execucao real.

### O que foi feito

- Reescrito `src/tape/cpp/tzx_reader.cpp` para ler o TZX em DUAS passadas: a 1a (`SkipOneBlock()`)
  so' indexa onde cada bloco comeca, sem gerar pulso nenhum; a 2a (`ExecuteDataBlock()` para os
  blocos "passivos", mais um loop de navegacao dedicado para os de controle) executa de verdade,
  com um "PC" (indice do bloco atual, nao deslocamento em bytes -- saltos/lacos/chamadas se
  referem a blocos pelo NUMERO DE ORDEM, por especificacao).
- **Salto (#23)**: deslocamento relativo ao proprio bloco do salto. **Laco (#24/#25)**: o corpo
  repete N vezes (uma pilha, apesar da especificacao proibir lacos ANINHADOS -- so' por seguranca
  contra um arquivo malformado); 0 repeticoes pula o corpo inteiro. **Chamada (#26/#27)**: uma
  lista de chamadas executadas em sequencia (podem se aninhar com lacos, por especificacao), com
  retorno ao bloco depois da chamada original no final da lista. **Selecao (#28)**: sem como
  mostrar um menu de verdade numa ferramenta batch -- escolhe sempre a 1a opcao, por padrao
  (documentado). Protegido contra laco infinito com um limite de passos generoso.
- Testes: `tapetest` com 6 checagens novas que montam um `.tsx` sintetico pequeno para cada caso
  (salto, laco com N repeticoes, laco com 0, chamada com lista de 2, selecao, salto fora dos
  limites) e confirmam a ORDEM REAL de execucao (via a ordem dos arquivos reconhecidos no
  resultado).

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.23.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.23.0-linux.tar.gz`.
- `ctest`: 17 suites (`tapetest` com as 6 checagens novas de navegacao, mais as existentes de
  leitura/gravacao). Windows: 17/17. Linux: 17/17.

## v1.22.0 -- "King's Valley: Ripper de Fita (WAV -> TSX)" (2026-10-08)

**Fase:** segundo item do "passo 1" pedido pelo usuario -- port do makeTSX, a conversao WAV -> TSX
(`doc/SPEC.md`, secao 5.2, item f), depois do empacotador .BIN/.BAS (1.21.0).

### O que foi feito

- `fwmsx --cas rip [--tolerancia N] [--anexar <arquivo>] <entrada.wav> <saida.tsx|.cas>`: demodula
  uma gravacao real de fita (PCM mono) para o formato `.CAS` interno, reusando o MESMO escritor do
  `pack` depois disso. Algoritmo portado do CONCEITO do makeTSX
  (`resource/makeTSX/BlockRipper.cpp`/`rippers/MSX4B_Ripper.cpp`, MIT) -- nenhum codigo copiado, so'
  a ideia geral (deteccao de piloto por limiar adaptativo + decodificacao byte a byte com
  tolerancia), simplificada para o caso fixo do MSX (sem os modos interativo/preditivo do original,
  que pede ajuda ao usuario ou tenta adivinhar bits ambiguos "olhando para frente" -- ver
  `doc/tape-spec.md`, secao 9, para os limites completos).
- `kcs_decode_byte()` (`src/tape/core/kcs_codec.{h,c}`), o inverso de `kcs_emit_byte()` -- decidiu
  tratar o bit de INICIO de forma ESTRITA (sem tolerancia extra) e os bits de FIM de forma leniente
  (aceitos mesmo fora da tolerancia, ou ausentes no fim do arquivo). A primeira decisao evita que o
  piloto do PROXIMO bloco (puro bit 1) seja lido como um fluxo infinito de bytes `0xFF`; a segunda
  evita perder o ULTIMO byte de uma gravacao so' porque nao sobrou pulso depois do bit de fim dele
  -- os dois foram bugs reais encontrados pelo proprio teste automatizado (round-trip) desta
  rodada, antes de qualquer teste manual.
- `src/tape/cpp/wav_reader.{h,cpp}`: leitor de `.wav` PCM mono (8/16 bits) bounds-checked, usado
  pelo `rip`.
- Testado contra uma fita MSX real dos anos 80
  (`resource/openMSX/Contrib/reverse_engineering_tools/kanji/ktst31 [RUN'CAS-'].wav`, GPL, so'
  validacao manual local -- nunca versionada como parte deste projeto): 86 blocos reconhecidos
  (4 arquivos ASCII) sem nenhum erro de decodificacao.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.22.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.22.0-linux.tar.gz`.
- `ctest`: 17 suites (`castooltest`/`cas_pack` com um round-trip completo -- bytes -> audio ->
  `rip` -> bytes, via um `.wav` sintetico gerado no proprio teste -- e testes de erro: `.wav`
  estereo, silencio, argumentos invalidos). Windows: 17/17. Linux: 17/17.

## v1.21.0 -- "King's Valley: Empacotador de Fita (BIN/BAS -> TSX)" (2026-10-08)

**Fase:** primeiro item da lista de pendencias de fita (`doc/SPEC.md`, secao 5.2) depois da
gravacao (1.20.x): uma ferramenta de linha de comando para empacotar um `.BIN`/`.BAS` solto num
`.TSX` sem passar pelo emulador -- para quem ja tem o binario/programa tokenizado pronto (de um
cross-assembler, de uma extracao, ou de um `BSAVE` anterior dentro do fwMSX) e so' quer uma fita
pronta, sem precisar abrir a janela so' para isso.

### O que foi feito

- `fwmsx --cas pack --tipo bin|bas --nome NOME [opcoes] <entrada> <saida.tsx|.cas>`: monta os
  MESMOS dois blocos que um `CSAVE`/`BSAVE` de verdade grava (cabecalho com nome + dados -- ver
  `doc/tape-spec.md`, secao 6, e o bug da 1.20.3) fora do gancho de BIOS, direto a partir do
  arquivo de entrada. `--tipo bin` precisa de `--inicio`/`--fim`/`--exec` (enderecos em
  hexadecimal) para montar os 6 bytes de endereco na frente do binario puro; `--tipo bas` espera o
  arquivo ja tokenizado (decidido por ser mais simples e mais correto que tentar tokenizar texto
  solto, ja que um programa BASIC do MSX embute ponteiros de memoria que dependem de onde ele vai
  ser carregado). `--anexar <arquivo>` acrescenta no final de uma fita existente.
- `fwmsx --cas list <arquivo>`: lista os arquivos de uma fita, reaproveitando o leitor existente
  (`LoadTzxImage`/`LoadCasImage`) -- confirma o resultado do `pack` sem abrir a janela.
- Reusa o escritor existente (`WriteTsxFromCas()`), o MESMO caminho que `TapeEngine` usa para
  gravar pela BIOS -- nenhuma logica de formato duplicada. A nova logica de montagem de blocos
  (`src/tape/cpp/cas_pack.{h,cpp}`) e' so' uma fatia do que `TapeEngine::OnTapoon()`/
  `FinalizeWrite()` ja fazem, sem motor, sem BIOS, sem Z80.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.21.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.21.0-linux.tar.gz`.
- `ctest`: 17 suites (nova: `castooltest`/`cas_pack`, sem depender de Z80/mapa de memoria --
  pack de `.bin`/`.bas`, `--anexar`, `list`, validacao de argumentos). Windows: 17/17. Linux:
  17/17.

## v1.20.3 -- "King's Valley: Gravacao em K7: Um Corte So'" (2026-10-08)

**Fase:** mais um bug real relatado pelo usuario, logo depois de testar a correcao da 1.20.2 pela
janela: "Sobreescrever no ponto marcado, eu escolhi um ponto e mandei dar um csave, e como se o
programa sumisse, perdeu o anterior e o novo no ponto salvo. Incluir um novo programa no final
funcionou direitinho." (confirmou tambem que a mudanca na UI do menu Fita, feita na 1.20.2, nao
precisava ser desfeita -- foi so' um engano do proprio usuario ao usar o menu antigo).

### Causa e correcao

Um CSAVE/BSAVE de verdade chama `TAPOON`/`TAPOUT`/`TAPOOF` DUAS vezes: um bloco so' para o
cabecalho com o nome do programa, outro so' para os dados de verdade (cada bloco tem seu proprio
piloto/sincronismo, igual numa fita de cassete real). `OnTapoon()` reaplicava a logica de
"sobrescrever o ponto marcado" em TODA chamada, nao so' na primeira -- entao, na 2a chamada (bloco
de dados), "o ponto marcado" ja' nao era mais o programa antigo, e sim o CABECALHO COM NOME que a
1a chamada tinha acabado de escrever. Resultado: o cabecalho recem-escrito era apagado, e so'
sobravam os dados, sem nome nenhum -- nem o programa antigo (de verdade apagado, como esperado)
nem o novo (com o cabecalho destruido) ficavam reconheciveis. "Nova fita" tinha exatamente o mesmo
problema (limpava tudo de novo na 2a chamada, perdendo o cabecalho que tinha acabado de escrever).

Corrigido fazendo o corte/limpeza rodar UMA SO' VEZ: depois da 1a chamada que corta (ou limpa),
`OnTapoon()` agora troca o modo de volta para "incluir no final" sozinho -- qualquer bloco
seguinte (do mesmo CSAVE, ou de um CSAVE futuro, se o usuario nao marcar outro ponto) so'
acrescenta, nunca corta de novo. Equivale ao comportamento fisico real: depois de cortar a fita e
comecar a gravar, o que vem a seguir so' pode ir para a frente.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.20.3.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.20.3-linux.tar.gz`.
- `ctest`: 16 suites (`tapetest` com checagens novas que simulam os 2 blocos reais de um CSAVE --
  nome + dados -- no modo "sobrescrever o ponto", confirmando que os dois sobrevivem). Windows:
  16/16. Linux: 16/16.
- **Revalidado pelo usuario na janela de verdade em 2026-10-08**: gravou num ponto no meio da
  fita, sobrescrevendo um programa existente -- funcionou.

## v1.20.2 -- "King's Valley: Gravacao em K7: Protecao e Preenchimento" (2026-10-08)

**Fase:** tres bugs reais relatados pelo usuario ao testar a 1.20.1 na GUI de verdade: "dando CLOAD
o contagiros nao gira, porem se eu clicar em um ponto da fita, em um programa, ele atualiza o
contagiros para aquela marca. Colocando no ponto e mudando o modo para sobrescrever, ele da Device
IO error, a opcao de salvar como uma nova fita tambem da Device IO error."

### Causa e correcao

1. **Contagiros "parado" no CLOAD**: nao e' bug. `odometer()` so' avanca com o motor girando
   (`motor_on()`); numa fita curta o avanco e' pequeno demais para notar a olho nu em poucos
   segundos. Confirmado por um diagnostico direto chamando `Machine::RunFrame()` em loop e
   imprimindo `tape().odometer()`/`motor_on()` a cada frame durante um `CLOAD` real -- o contador
   sobe normalmente (0 -> 16 -> 31 -> ... -> 89) e so' congela quando o motor desliga (fim do
   carregamento). O clique num ponto da lista "pula" o contador de uma vez, por isso parece mais
   responsivo que o avanco gradual do CLOAD.
2. **"Device I/O error" ao trocar o modo de gravacao apos marcar um ponto**: a causa real nao era
   o motor de gravacao (testado em isolado via `Machine`/`TapeEngine` direto, com sucesso nos dois
   modos), e sim `TapeEngine::Insert()`: reinserir a MESMA fita que o usuario ja' tinha destravado
   voltava a trava-la (`read_only_ = true`) sempre, silenciosamente. Bastava reabrir o menu Fita
   (o que reinsere a imagem atual) para a protecao "voltar" sem aviso. Corrigido para preservar a
   protecao ao reinserir a mesma fita; o menu tambem ganhou dois itens explicitos ("Destravar para
   gravar" / "Travar contra gravacao", cada um habilitado so' quando faz sentido) no lugar do
   toggle unico e ambiguo, com o estado atual sempre visivel.
3. **Preenchimento de alinhamento (padding) gravado como dado de verdade**: ao abrir um novo
   cabecalho, `TAPOON` insere bytes zero so' para alinhar em 8 bytes -- igual a BIOS de verdade --,
   mas os sintetizadores de pulso descobriam o tamanho de cada bloco "procurando o proximo
   cabecalho" (nao havia outra forma de saber onde um bloco acabava), o que colava esse
   preenchimento no FINAL do bloco anterior quando a fita era gravada, ejetada e recarregada.
   Corrigido com uma marca exata por bloco (`TapeMark`, com o tamanho REAL do conteudo) preenchida
   pelo proprio motor de gravacao no momento em que o bloco e' fechado -- sem precisar adivinhar.
   Afeta so' fitas GRAVADAS por este emulador e depois recarregadas; leitura de `.tsx`/`.cas` de
   terceiros nunca foi afetada (ja' trazem o tamanho exato de cada bloco). Fitas `.cas` cruas
   carregadas direto do disco (sem passar por uma gravacao deste emulador) ainda dependem da busca
   pelo proximo cabecalho -- limitacao conhecida, documentada em `doc/tape-spec.md`.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.20.2.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.20.2-linux.tar.gz`.
- `ctest`: 16 suites (`tapetest` com 1 checagem nova: grava dois blocos, recarrega do ZERO a
  partir do arquivo persistido, confere que o segundo bloco tem exatamente os bytes gravados, sem
  o preenchimento do primeiro bloco misturado). Windows: 16/16. Linux: 16/16.

## v1.20.1 -- "King's Valley: Gravacao em K7: Corrigindo o Piloto" (2026-10-08)

**Fase:** correcao de um bug real na gravacao, relatado pelo usuario ao testar a 1.20.0: gravou um
programa pequeno (`CSAVE"TESTE"`), rebobinou, deu `CLOAD` no modo normal -- ouviu o chiado do piloto,
mas o programa nunca carregava ("Found:" nunca aparecia).

### Causa e correcao

1. **Pulsos de ZERO e UM trocados**: a convencao real do bloco #4B do MSX e' `zero = 2x o pulso de
   um`, com o piloto na MESMA duracao do um -- confirmado no proprio gerador do makeTSX
   (`resource/makeTSX/rippers/MSX4B_Ripper.h/.cpp`). Os valores que eu usava (ZERO=855, UM=1710) eram
   os defaults GENERICOS de ZX Spectrum (blocos #10/#11), nao os do #4B -- a nota original do
   `doc/SPEC.md` ja' avisava "conferir no codigo antes de usar", o que nao tinha sido feito.
2. **Piloto curto demais**: 2000 pulsos (~0,48s) nao davam tempo da BIOS de verdade calibrar. Subido
   para 8000 pulsos (~1,9s), confirmado contra o arquivo real do usuario e contra um .TSX comercial
   (cujo primeiro cabecalho de fita tem varios segundos de piloto).

O modo RAPIDO nunca foi afetado (nao usa pulso nenhum); so' o modo normal de uma fita GRAVADA por
este emulador. Leitura de fitas de terceiros (ja' com os proprios pulsos corretos) tambem nunca foi
afetada.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.20.1.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.20.1-linux.tar.gz`.
- `ctest`: 16 suites (`tapetest` com 3 checagens novas sobre a relacao zero/um/piloto). Windows:
  16/16. Linux: 16/16.
- Smoke test: regravei o `teste.tsx` do proprio usuario com o escritor corrigido e confirmei
  `CLOAD`/`RUN"CAS:"` com sucesso ("Found:teste") nos dois modos.

## v1.20.0 -- "King's Valley: Gravacao em K7" (2026-10-07)

**Fase:** gravacao de fita (CSAVE/BSAVE "CAS:"), fita nova em branco, protecao contra gravacao por
padrao, tres modos de gravacao e marcar o ponto de carga/gravacao na janela "Fita K7", com um
contagiros simulado. Pedido do usuario, depois de validar a 1.19.1 (leitura) jogando um jogo completo.
Numero de versao: o usuario sugeriu "1.19.2 ou algo assim", mas pela politica do proprio projeto (Y
sobe a cada feature nova incorporada, nao so' Z) isto vira 1.20.0 -- gravacao e' uma feature nova, nao
um ajuste da 1.19.x.

### Destaques

- **TAPOON/TAPOUT/TAPOOF sempre pelo gancho de BIOS** ("ED FE", o mesmo mecanismo do carregamento
  rapido), independente do modo de carregamento escolhido -- gravar sempre "funciona na hora", sem som.
- **Fita nova** (menu Fita > "Nova fita (.tsx)..."): cria uma fita vazia e destravada, gravada no disco
  na hora (so' o cabecalho).
- **Protecao contra gravacao por padrao**: toda fita inserida de um arquivo comeca travada; o usuario
  destrava pelo menu quando quiser gravar nela.
- **Tres modos de gravacao**: incluir no final (o padrao, para "ir enchendo a fita"), sobrescrever o
  ponto marcado, ou nova fita (apaga tudo).
- **Marcar um arquivo** na janela "Fita K7" (clicavel): decide de onde carregar, ou onde sobrescrever.
- **Contagiros**: um odometro simulado, girando conforme a fita "roda".
- A gravacao persiste no arquivo imediatamente (um `.tsx` valido, com blocos #4B; ou um `.cas` cru).

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.20.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.20.0-linux.tar.gz`.
- `ctest`: 16 suites (`tapetest` com 68 checagens, 22 novas). Windows: 16/16. Linux: 16/16.
- Smoke test: o leitor continua lendo o `.tsx` real do usuario sem regressao (mesmo resultado da 1.19.1).
- A gravacao em si (TAPOON/TAPOUT/TAPOOF, protecao, os 3 modos, marcar/desmarcar, contagiros) foi
  validada por um Z80 de verdade dentro do `tapetest` (round-trip completo), **mas ainda nao foi
  clicada na janela por ninguem**.

### Limitacoes conhecidas

- **Sintese de pulsos do modo normal**: o preenchimento de alinhamento (zeros antes de um cabecalho)
  entra como se fosse dado de verdade -- so' afeta a reproducao por pulsos de uma fita GRAVADA por este
  emulador; o modo rapido nao e' afetado. Ver `doc/tape-spec.md`, secao 5.
- **Sem navegar os blocos de controle do TZX** (grupos, lacos, saltos) -- herdado da 1.19.x.
- **Sem banco de fitas, download, nem o "ripper" de .BIN/.BAS independente do emulador** (itens (e)/
  (f)/parte do (c) de `doc/SPEC.md`, secao 5.2).
- **A gravacao pela janela nunca foi vista/clicada por ninguem** -- so' testada por unidade.

## v1.19.1 -- "Yie Ar Kung-Fu: Fita K7: Corrigindo o Carregamento" (2026-10-07)

**Fase:** correcao de dois bugs no leitor de .TSX, encontrados pelo usuario ao testar a 1.19.0 com um
arquivo .tsx real (`resource/fmsxgo/media/*.tsx`, Dinamic, 1990). Sem mudanca de recursos.

### O que estava errado

- A janela "Fita K7" mostrava a fita como vazia (a insercao falhava silenciosamente).
- `RUN"CAS:"` dava "Device I/O error", ou a maquina travava no modo normal (a BIOS de verdade fica
  esperando um sinal que nunca chega).
- Sem som no carregamento normal (consequencia do mesmo problema: a fita nunca chegava a carregar).

### Causa e correcao

1. **Bloco #35 (Custom info) do TZX**: a string de identificacao tem **16 bytes**, nao 10 -- o
   `TZX_format.md` usa notacao HEXADECIMAL no deslocamento do campo seguinte (`0x10` = 16). Esse bloco
   e' o primeiro de praticamente todo .TSX gerado pelo makeTSX (grava "TSX.RIPPER" nele), por isso o
   arquivo do usuario nunca passava do primeiro bloco.
2. **Alinhamento entre blocos #4B**: um .CAS de verdade (gerado por `TAPOON`) preenche com zeros até o
   proximo multiplo de 8 bytes antes de CADA cabecalho. O leitor de .TSX nao fazia esse preenchimento
   ao reconstruir o fluxo "rapido" a partir dos blocos #4B (de tamanho arbitrario) -- o SEGUNDO
   `BLOAD"CAS:"` de uma fita com varios arquivos (o caso comum: um loader BASIC curto + os binarios)
   desalinhava e nunca mais achava cabecalho nenhum.

Os dois foram confirmados byte a byte contra o arquivo real do usuario (um programa MSX com 12 blocos
#4B: um loader ASCII com 5 `BLOAD"CAS:",R` em sequencia) e tem teste de regressao em `tapetest`.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.19.1.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.19.1-linux.tar.gz`.
- `ctest`: 16 suites (2 checagens novas em `tapetest`, uma para cada bug). Windows: 16/16. Linux: 16/16.
- Smoke test com o arquivo real do usuario: `--fita <arquivo> --keys 'RUN"CAS:"|'` carrega e executa o
  loader (SCREEN2 + a cadeia de `BLOAD"CAS:",R`) sem erro, nos dois modos (rapido e normal), sem travar.

## v1.19.0 -- "Yie Ar Kung-Fu: Fita K7" (2026-10-07)

**Fase:** leitura de fita (.CAS e .TSX/.TZX) com dois modos de carregamento -- rapido (gancho de BIOS,
sem som) e normal (pulsos de verdade, com o barulho do gravador) -- e a janela visual "Fita K7". Sem
mudanca no nucleo Z80 (so' usou, por fora, um mecanismo de patch de BIOS que ja' existia la' sem uso).

### Destaques

- **Leitor de .CAS e .TSX/.TZX**: todo bloco do TZX 1.20 e' reconhecido (comprimento sempre calculado
  certo, nunca trava um arquivo valido); o bloco #4B (Kansas City Standard, o que o MSX usa de verdade)
  e' reproduzido por completo, com piloto, bits e pausa.
- **Carregamento rapido**: as rotinas TAPION/TAPIN/TAPIOF da BIOS (00E1h/00E4h/00E7h) sao interceptadas
  (o mesmo truque "ED FE" do fMSX) e devolvem os bytes direto de um buffer, sem temporizacao.
- **Carregamento normal**: os MESMOS pulsos que um gravador real produziria sao entregues na porta de
  verdade (PSG, R14 bit 7), na cadencia de T-states da CPU -- a BIOS roda a rotina ORIGINAL dela, e o
  sinal tambem e' ouvido (onda quadrada somada ao audio ao vivo).
- **Motor da fita**: a porta C do PPI (bit 4, `AAh`) liga/desliga o avanco da fita, como no hardware real.
- **Janela "Fita K7"**: dois rolos desenhados com `ImDrawList`, girando enquanto o motor esta' ligado;
  barra de progresso (posicao/duracao) e lista dos arquivos encontrados (nome, tipo, tamanho).
- `--fita <arquivo>` e `--fita-modo rapido|normal`; menu **Fita** da janela.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.19.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.19.0-linux.tar.gz`.
- `ctest`: 16 suites (nova: `tapetest`/`tape_load`). Windows: 16/16. Linux: 16/16.
- Smoke test: `fwMSX.exe --msx --fita <arquivo.cas> --frames 30` (modo rapido e normal) roda sem erro,
  com o audio ao vivo ligado.

### Limitacoes conhecidas

- **Sem escrita**: nenhum `CSAVE`/`BSAVE "CAS:"` grava fita.
- **Sem banco de fitas nem download** (itens (e)/(f) de `doc/SPEC.md`, secao 5.2).
- **Blocos de controle do TZX** (grupos, lacos, saltos, chamadas, selecao) sao pulados com seguranca mas
  NAO navegados -- o arquivo e' lido sempre em sequencia. Protecoes que dependem desses blocos (ex.:
  Bleepload) nao vao funcionar direito.
- **Fast_bytes so' a partir do #4B**: um .TSX cujo conteudo de verdade esteja em blocos #10/#11/#14 (em
  vez de so' o piloto) carrega certo no modo normal, mas nao no rapido -- nao e' o caso comum do MSX.
- **Som da fita nao comparado lado a lado com um gravador/MSX real** -- validado so' pela logica dos
  pulsos ao escrever esta secao. **Atualizacao (2026-10-07, apos o lancamento):** o usuario carregou um
  jogo completo pelo modo normal (`A.M.C.`, Dinamic 1990) e confirmou o som "bem nitido", jogando um
  pouco depois de carregar -- a comparacao formal com hardware real continua pendente, mas a leitura e
  o som em uso real estao confirmados.
- Ver `doc/tape-spec.md`, secao 5, para a lista completa.

## v1.18.0 -- "Ys: Banco de ROMs e Disco por Portas" (2026-10-06)

**Fase:** banco de ROMs em SQLite com download e identificacao, controladora de disco por portas
no estilo Microsol (DDX 3.0 e CDX-2), layout de slots com RAM e mapper ampliados, e o estudo de
viabilidade de fitas TSX/TZX/CAS. Sem mudanca no nucleo Z80.

### Destaques

- **Banco de ROMs** (SQLite, `fwmsx --romdb` e menu **ROMs** da janela): identificacao por SHA-1,
  CRUD, busca, escanear pastas. Downloads do fMSX 6.0 (Windows), do System ROMs do file-hunter
  (Full Set mais recente ou navegacao por pastas) e do banco do Vampier (nomes de jogos). Ver
  [doc/romdb-spec.md](romdb-spec.md).
- **Controladora de disco por portas** no estilo Microsol (DDX 3.0 e CDX-2, base `D0h`): o
  MSX-DOS 1.8 sobe pelos dois drivers, com os formatos 180/360/720 KB escolhidos pela configuracao
  do drive (5 1/4 ou 3 1/2, faces, densidade, ou automatico). Menu **Maquina > Configuracao de
  disco...** ou `--disk-acesso porta --disk-porta D0h --diskrom <driver> --disk-formato ...`. Ver
  [doc/fdc-spec.md](fdc-spec.md), secao 6.
- **Layout de slots ampliado**: RAM de 16 KB no fim da celula (corrige a BIOS Gradiente Expert 1.1,
  que travava com a RAM em 0000h), RAM de 32 KB em duas celulas, mapper de 64 KB a 4096 KB e mais
  de um mapper por maquina. Opcao `--slot P:S=tipo[:arg]` na linha de comando.
- **Teclado**: correcao de `&` (era SHIFT+6, no MSX e' SHIFT+7).
- **Estudo (provisorio, so' leitura)**: codigo do openMSX (GPL-2.0+), CLK e makeTSX (MIT) incluidos
  em `resource/` para consulta. Viabilidade de fitas TSX/TZX e CAS registrada como feature futura em
  `doc/SPEC.md`, secao 5.2 -- nao iniciada nesta versao.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.18.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.18.0-linux.tar.gz`.
- `ctest`: 15 suites. Windows: 15/15. Linux: 15/15 (WSL, GCC 15.2).
- Smoke tests: MSX1 chega a `MSX BASIC version 1.0`; MSX2 a `2.1`; MSX2+ a `3.0`; MSX-DOS 1.8 sobe
  do `msxdos1.dsk` ate `A>`, pelo disco e pelas portas (DDX 3.0 e CDX-2).
- Janela confirmada visualmente nesta fase: menu **ROMs**, janelas **Banco de ROMs** e **Navegar
  file-hunter**, **Configuracao de disco** e **Configuracao de slots**, tela cheia, 4:3, 16:9 e
  filtros de video.

### Limitacoes conhecidas

- **Banco de ROMs**: as ROMs baixadas ainda nao foram conferidas em lote contra o SHA-1 conhecido;
  o banco ainda nao escolhe o mapper automaticamente ao carregar um cartucho.
- **FM sem validacao por ouvido**: o WAV gerado (`--wav`) tem sinal, mas o timbre e o nivel nao
  foram comparados com um MSX-MUSIC real. `CALL VOICECOPY` nao e' aceito pela ROM do fMSX; status e
  timers do OPLL nao sao emulados.
- **Disco**: ainda nao formata disquetes (so' le e grava); FM/MFM nao modelados; um so' formato por
  maquina (nao um por drive A/B).
- **Fitas (TSX/TZX/CAS)**: so' estudo de viabilidade (`doc/SPEC.md`, secao 5.2); nenhuma linha de
  codigo de leitura/escrita ainda.
- **Jogos**: Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada, causa nao
  diagnosticada; Mega Chase so' validado ate o titulo; F-1 Spirit 3D: troca de disco pela janela nao
  testada.
- **Cores YJK** do V9958 nao conferidas com hardware real.
- **Ausentes**: save-state, efeitos de rastreio no meio do quadro, controle externo do emulador, e
  o contador de CPU no pior caso.
- **Licenca**: uso do fMSX e' so' para estudo, por acordo informal com o autor; confirmacao por
  escrito ainda pendente.

## v1.17.0 -- "Xak: Musica FM e Slots" (2026-10-06)

**Fase:** chip FM (MSX-MUSIC e FM-PAC) com os comandos de BASIC, layout de slots configuravel
pelo menu e SRAM do FM-PAC. Sem mudanca no nucleo Z80.

### Destaques

- **Chip FM (OPLL, YM2413)**: 9 canais melodicos com 2 operadores, envelope ADSR, feedback, tremolo,
  vibrato, os 15 timbres prontos e o **modo ritmo** (bumbo, chimbal, caixa, tom e prato). Portas `7Ch`/`7Dh`.
- **BASIC do MSX-MUSIC** pela ROM do FM-PAC: `CALL MUSIC`, `PLAY #n`, `CALL VOICE`, `CALL PITCH`,
  `CALL AUDREG` e `CALL PLAY`. Depois do `Ok` a musica continua tocando em segundo plano.
- **FM-PAC ligado por padrao** quando o `FMPAC.ROM` existe. O menu **Cartucho > FM-PAC** liga e desliga;
  `--no-fmpac` desliga na linha de comando.
- **Configuracao da maquina** (menu **Maquina > Configuracao de slots...**): 16 celulas (slot:subslot)
  com BIOS, BASIC, RAM de 16, 32 ou 64 KB, mapper de 64 a 1024 KB, cartucho, sub-ROM do MSX2, disco e
  FM-PAC. Uma BIOS de 32 KB ocupa a pagina 0 (BIOS) e a pagina 1 (BASIC) do slot escolhido; BIOS e BASIC
  podem ser dois arquivos de 16 KB. Padrao igual ao de antes.
- **SRAM de cartucho** (ASCII8 e ASCII16) e do **FM-PAC**, gravada no arquivo `.sav` ao lado da ROM.
- **Diagnostico pela linha de comando**: `--text` (tela em texto), `--fmstat` (estado do FM) e
  `--wav arquivo` (grava a mistura de audio).
- **Teclado do `--keys` corrigido** para o layout do MSX: `(`, `)`, `*` e `"`.

### Build usado para validar esta release

- Windows: `.\build.ps1` gerou `dist\fwMSX-1.17.0.zip`.
- Linux: `./build.sh` (WSL Ubuntu 26.04, GCC 15.2) gerou `dist/fwMSX-1.17.0-linux.tar.gz`.
- `ctest`: 14 suites. Windows: 14/14. Linux: 14/14 (WSL, GCC 15.2).
- Smoke tests: MSX1 chega a `MSX BASIC version 1.0`; MSX2 a `2.1`; MSX2+ a `3.0`; MSX-DOS 1.8 sobe
  do `msxdos1.dsk` ate `A>`.

### Limitacoes conhecidas

- **FM sem validacao por ouvido**: o WAV gerado (`--wav`) tem sinal, mas o timbre e o nivel nao foram
  comparados com um MSX-MUSIC real. Constantes de envelope, KSL e ganho sao estimativas.
- **`CALL VOICECOPY` nao e' aceito** pela ROM do FM-PAC do fMSX ("Syntax error").
- **Status e timers do OPLL** nao sao emulados (`7Ch` le 0).
- **Layout de slots**: so' pelo menu (sem opcao de linha de comando); a BIOS so' funciona em 0:0
  (o Z80 comeca la'); uma so' RAM mapeada por maquina. Uma BIOS que nao seja o fMSX (ex.: Gradiente
  Expert 1.1) monta no layout, mas so' roda se o hardware que ela espera existir -- nao foi testada.
- **Cartuchos**: MSX-DOS 2, GameMaster2 e o cartucho MSX-MUSIC com BIOS propria ainda nao existem.
- **Jogos**: Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada, causa nao
  diagnosticada; Mega Chase so' validado ate o titulo; F-1 Spirit 3D: troca de disco pela janela nao testada.
- **Janela**: menus, tela cheia, 4:3, 16:9 e filtros de video nao foram conferidos na tela (a automacao
  nao opera a janela). A janela sobe sem erro.
- **Cores YJK** do V9958 nao conferidas com hardware real.
- **Ausentes**: save-state, efeitos de rastreio no meio do quadro, e o contador de CPU no pior caso.

## v1.16.0 -- "Aleste: Janela e Video" (2026-10-05)

**Fase:** interface da janela, menus do fMSX e filtros de video. Sem mudanca no nucleo de emulacao.

### Destaques

- **Quadro com borda, como o fMSX**: 272x228 no MSX1 e 544x228 no MSX2. As teclas de funcao e o
  texto do BASIC nao encostam mais na borda.
- **Menus do fMSX**: Arquivo, Maquina (Modelo MSX1/MSX2/MSX2+, Reiniciar, Pausar), Exibir, Video, Som,
  Disco, Cartucho, Joystick, Ferramentas, Configuracoes e Ajuda. Trocar modelo ou cartucho recria a maquina.
- **Configuracoes -> Interface**: tema escuro ou claro, tamanho da letra e borda com sombra da tela.
- **Exibir**: zoom 2x, 3x, 4x e 6x; proporcao original, 4:3 corrigido ou 16:9 esticado; tela cheia com
  o menu escondido ate o mouse chegar ao topo.
- **Video**: interpolacao (Nearest Neighbor, Linear Scaling, EPX Scale 2x, Eagle, Scale 2x e 2xSal),
  scanlines (TV, LCD, LCD Raster) e filtros de cor (Monochrome, Sepia, Green CRT, Amber CRT, CMY Raster,
  RGB Raster). Escritos do zero a partir das descricoes publicas dos algoritmos.

### Build usado para validar esta release

- Windows: `.uild.ps1` gerou `distwMSX-1.16.0.zip`.
- Linux: `build.sh` (WSL Ubuntu 26.04) gerou `dist/fwMSX-1.16.0-linux.tar.gz`.
- `ctest`: 13 suites nas duas plataformas, incluindo `video_filters` (18 verificacoes) e a geometria do quadro.

### Limitacoes conhecidas

- Os menus, a tela cheia, as proporcoes 4:3 e 16:9 e os filtros de video foram conferidos
  por codigo e testes, mas nao foram vistos na tela: a automacao nao opera a janela.
- Itens do menu marcados "(em breve)": salvar estado, memoria, PAL, trapacas, POKE, DiskROM, fita,
  MIDI, gravacao de som, novo disco, slot 2, dispositivos de entrada e mostrar sprites.
- 2xSal e uma aproximacao do algoritmo original.
- Jogos: Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada sem causa
  identificada; Mega Chase validado ate o titulo; F-1 Spirit 3D validado ate o titulo com os discos
  A e B.

---

## v1.15.0 -- "Metal Gear 2: Cores YJK" (2026-10-05)

**Fase:** MSX2+ (V9958): modos YJK/YAE, scroll do SCREEN 12 e a BIOS MSX2+. Ver
`doc/msx2p-spec.md`.

### Destaques

- **`fwmsx --msx --msx2p`** sobe a BIOS MSX2+ real (MSX BASIC 3.0) com o VDP V9958.
- **SCREEN 10, 11 e 12** em YJK/YAE, com R#25. **Scroll horizontal** (R#26/R#27) em SCREEN 5-8, YJK e
  YAE, e **mascara da esquerda** (R#25 bit 1).
- **Correcao no `--keys`**: os simbolos com SHIFT do layout MSX (parenteses, `&`, aspas, `*`) saiam
  trocados. Os exemplos de `LINE (...)` da documentacao agora funcionam.

### Build usado para validar esta release

- `vdp2test`: as verificacoes novas do V9958 (secao 9) passam; os valores de YJK foram calculados a mao.
- Suite completa 12/12 (`ctest`) no Windows e no Linux.
- Verificacao com BASIC real: BIOS MSX2+ sobe; SCREEN 12 e SCREEN 10 escrevem R#25 e desenham em YJK e YAE.
- Pacote Windows: `.uild.ps1` gerou `distwMSX-1.15.0.zip`. Pacote Linux: `build.sh` (WSL Ubuntu 26.04) gerou `dist/fwMSX-1.15.0-linux.tar.gz`.

### Limitacoes conhecidas

- SCREEN 9 nao existe no MSX2+ (so' no MSX2 coreano), entao nao e' implementado.
- As cores YJK seguem o openMSX (regra do azul de um turbo R), mas nao foram comparadas com um V9958 real.
- Mesmas limitacoes da v1.14.0 para o restante (FM, SRAM, Lode Runner + SCC).

---

## v1.14.0 -- "Hydlide: Busca Rapida" (2026-10-05)

**Fase:** item menor adiado desde a Fase 3 do nucleo Z80 (`doc/SPEC.md`, secao 5.0): `CPIR`/`CPDR`
em Assembly.

### Destaques

- **CPIR e CPDR** procuram o byte em Assembly (`REPNE SCASB`, `z80_fast_block_search()`), no
  mesmo molde do LDIR/LDDR. A regra de flags (que dependia do valor do ultimo byte, o motivo do
  adiamento) fica no C, com as mesmas expressoes do loop lento.
- Caminho rapido so' quando o trecho e' RAM gravavel plana. Sobre ROM, o loop lento continua valendo.

### Build usado para validar esta release

- Windows: `z80test` com 300 checks passando; suite completa 12/12 (`ctest`).
- Linux (WSL Ubuntu 26.04, SysV): `z80test` com 300 checks passando; suite completa 12/12.
- Teste de sensibilidade: quebrar o tratamento de casamento no caminho rapido derruba 26 checks.

### Limitacoes conhecidas

- O caminho rapido nao cobre ROM (o `ram_ptr` so' devolve pedacos gravaveis), entao CPIR/CPDR sobre
  ROM continuam no loop lento. Correto, so' sem o ganho de desempenho.
- Mesmas limitacoes da v1.13.0 (Lode Runner + SCC, FM, SRAM, V9958).

---

## v1.13.0 -- "F1 Spirit: Som do SCC" (2026-10-05)

**Fase:** chip de som SCC (Konami), o proximo item de `doc/SPEC.md`, secao 5.0.

### Destaques

- **O SCC toca.** Cartuchos Konami5 e Gen8 que ligam o SCC (escrita `3Fh` em `9000h`, ou em
  `8000h-9FFFh` no Gen8) agora tocam a trilha pela mesma saida de audio do PSG. O **F1 Spirit**
  programa os cinco canais de verdade: apos 4000 quadros o mixer esta em `1Fh`, com volumes e
  periodos reais, e o estado renderiza 44085 amostras nao-nulas por segundo.
- **Quatro linguagens na fase:** o motor em **C** (`scc_state.c`, port do `SCC.c` do fMSX), a soma
  de canal em **Assembly** dual-ABI (`render_channel.asm`), a tabela de volume linear em **Fortran**
  (`scc_volume_table.f90`) e o `SccDevice` em **C++** ligado ao slot de cartucho.
- **Barramento de cartucho:** `memmap::SlotCartIo` / `AttachCart()` entrega as leituras e escritas
  do slot de cartucho ao dispositivo antes do mapper. E' o mesmo papel do `SlotMmio` do FDC, mas
  para um dispositivo que precisa ver as escritas que tambem vao ao mapper.
- **Mixagem:** PSG e SCC somados na saida ao vivo, com saturacao em 16 bits.

### Build usado para validar esta release

- Toolchain MSYS2 UCRT64: GCC/GNU Fortran 16.2.0, NASM 3.02, CMake 4.4.4, Ninja 1.13.2.
- `ctest`: 12 suites, todas passando. `scc_sound` (`scctest`) e' a nova, com 18 verificacoes:
  protocolo Konami5 e Gen8, ROM plana, frequencia (+-1%), nivel e volume linear, mixer e volume
  zerados, espelho `B0h`, e o teste diferencial do Assembly contra a referencia em C.
- Verificado com o jogo: F1 Spirit (ver Destaques). Lode Runner + Konami SCC continua sem subir
  (boot do cartucho, nao o chip; ver `doc/scc-spec.md`, secao 5).
- Build Linux (`build.sh`, WSL Ubuntu 26.04): GCC/GNU Fortran 15.2.0, NASM 3.01, CMake 4.2.3, Ninja
  1.13.2. `ctest`: 12/12 suites, incluindo `scc_sound` -- o kernel de Assembly rodou no caminho
  `elf64`/SysV. Pacote: `dist/fwMSX-1.13.0-linux.tar.gz`.
- Pacote Windows: `.\build.ps1` gerou `dist\fwMSX-1.13.0.zip`.

### Limitacoes conhecidas

- **Lode Runner + Konami SCC** cai no BASIC ("Illegal function call in 10") com Konami5, e nenhum
  mapper chega a ligar o SCC nele. Nao investigado a fundo.
- Modo SCC+ (enderecos com `2000h`) implementado, mas sem cartucho de referencia nem teste dedicado.
- Sem FM (OPLL/FMPAC), sem SRAM persistente, sem `MAP_GMASTER2`/`MAP_FMPAC`/`MAP_GUESS`.
- Sem filtro de reamostragem no SCC: frequencias altas podem ter aliasing.

---

## v1.12.0 -- "Metal Gear: Entrada Direta" (2026-10-05)

**Fase:** item menor adiado desde a v1.2.0 ("5.1 Visao registrada", `doc/SPEC.md`) -- sem
recurso novo, so' uma mudanca de comportamento padrao agora que faz sentido.

### Destaques

- **`fwMSX.exe` sem argumento nenhum abre a maquina completa em janela** (os mesmos padroes de
  `fwmsx --msx`: BIOS MSX1 ao lado do executavel, sem cartucho/disco). Ate aqui, sem argumentos,
  o executavel imprimia so' o esqueleto dos quatro modulos (C++/C/Assembly/Fortran) -- fazia
  sentido enquanto nao havia emulacao de verdade, mas deixou de fazer desde que o core existe
  (Z80 desde a v1.3.0, VDP desde a v1.5.0, maquina completa desde a v1.9.0).
- O esqueleto continua acessivel, so' que agora precisa de argumentos explicitos que nao batem
  com nenhum modo conhecido (ex.: `fwMSX.exe NomeDoProduto 1 2 3`) -- preservado por ser o
  historico do projeto, nao removido.

### Exemplo rapido

```powershell
.\dist\fwMSX.exe                 # abre a maquina MSX1 numa janela (BASIC pronto)
.\dist\fwMSX.exe fwMSX 2 0 5     # esqueleto multi-linguagem (historico)
```

### Build usado para validar esta release

- Toolchain MSYS2 UCRT64: GCC/GNU Fortran 16.2.0, NASM 3.02, CMake 4.4.4, Ninja 1.13.2.
- `.\build.ps1` gerou `dist\fwMSX.exe`, `dist\msxdisk.exe` e `dist\fwMSX-1.12.0.zip`.
- `ctest`: 11 suites, todas passando (`z80_smoke`, `z80_debug_session`, `memmap_slots`,
  `vdp_digital`, `ppi_keyboard`, `psg_sound`, `machine_frames`, `fdc_wd2793`, `vdp_msx2`,
  `msx2_machine`, `audio_live`).
- Verificado a mao: `dist\fwMSX.exe` sem argumentos abre a janela e fica rodando.

### Correcoes no script de build

- `build.ps1` agora para no primeiro erro do CMake (configuracao ou compilacao). Antes ele
  imprimia "Pronto" e empacotava um zip mesmo quando a compilacao falhava.

### Limitacoes conhecidas

- As mesmas da v1.11.1.

---

## v1.11.1 -- "Firebird: Arrumando a Casa" (2026-10-02)

**Fase:** acabamento da v1.11.0 -- sem recurso grande, so' o que sobrou pelo caminho.

### Destaques

- **`--disk-ro`:** discos somente leitura. O MSX-DOS le normalmente, recusa gravar (`Write
  protect error writing drive A`) e o arquivo `.dsk` fica byte a byte identico. Vale para os
  discos inseridos pelo menu **Disco** tambem. Fecha o maior risco de perda de dados
  documentado desde a v1.10.0.
- **Documentacao em dia:** textos que diziam "sem janela" ou listavam como pendentes coisas ja'
  feitas (audio ao vivo, joystick, janela, cartuchos) foram corrigidos, e dois `TODO(FDC)` que
  apontavam para um controlador ja' existente viraram notas.
- Verificado: jogos MSX1 sobem na maquina MSX2 (King's Valley igual ao fMSXgo).
- 800 verificacoes automatizadas (11 suites).

### Limitacoes conhecidas

- As mesmas da v1.11.0. O "Lode Runner + Konami SCC" continua caindo no BASIC ("Illegal
  function call in 10"): e' uma ROM que espera um disco proprio, e nao foi investigada.

---

## v1.11.0 -- "Firebird: MSX2 em Cena" (2026-10-02)

**Fase:** MSX2 -- VDP V9938, mapper de RAM, RTC. Nome escolhido por "Firebird"
(Hi no Tori Hououhen, Konami) ser o jogo MSX2 que motivou tudo: na v1.10.0 ele nao
rodava (e' um jogo MSX2, titulo em SCREEN 5) e agora joga.

### Destaques

- **`fwmsx --msx --msx2`:** a BIOS MSX2 real sobe ate' o **MSX BASIC 2.1**, identico ao
  fMSXgo usado como referencia. `--ext <arq>` escolhe a sub-ROM (padrao: `MSX2EXT.ROM`
  ao lado da BIOS). O MSX-DOS 1.8 tambem boota no MSX2 (a sub-ROM e a DISK.ROM dividem
  o slot 3:1) e nem pergunta a data, porque o RTC ja' a fornece.
- **VDP V9938 completo:** VRAM de 128KB com paginas (R#14); **SCREEN 3, 4, 5, 6, 7, 8 e
  TEXT80**; 212 linhas; cor 0 transparente; **sprites de modo 2** (cor por linha, bits
  CC/EC, 8 por linha, 9o sprite, colisao). SCREEN 6/7 e TEXT80 saem com 512 pixels de verdade.
- **Motor de comandos** (adaptado do `V9938.c`): POINT, PSET, SRCH, LINE, LMMV, LMMM, LMCM,
  LMMC, HMMV, HMMM, YMMM, HMMC, com operacoes logicas, **temporizacao por scanline** e o
  handshake TR. `LINE ,bf`, `CIRCLE` e `PAINT` do BASIC desenham de verdade.
- **RAM de 128KB com mapper** (portas `FCh`-`FFh`) e **relogio RTC** RP5C01 com CMOS
  (`B4h`/`B5h`), com os valores padrao da BIOS.
- **Janela:** a imagem do MSX2 e' sempre de 512 pixels de largura (modos de 256 saem
  dobrados) e a janela recria a textura ao mudar de modo.
- **Firebird joga:** logo MSX, logo Konami, titulo com kanji, floresta rolando e sprites
  coloridos de modo 2.
- `--keys` aceita pontuacao com SHIFT (`( ) : $ " ...`); `--vdplog` (sem janela) mostra, a
  cada quadro, os registradores do VDP que mudaram.
- 795 verificacoes automatizadas (11 suites): `vdp2test` (cada modo e cada comando, pixel a
  pixel) e `msx2test` (mapper, RTC, a BIOS MSX2 real e o BASIC desenhando).

### Correcoes que valem tambem para o MSX1

- **SCREEN 3 (multicolor)** agora e' desenhado (caia no fundo liso).
- A **cor 0 transparente** e a **tela desligada** (R#1 bit 6) valem em todos os modos.
- `R#6`/`R#10`/`R#11` guardam so' os bits que existem (como o `VDPOut()` do fMSX).
- Desvio deliberado do fMSX: o bit TR do motor de comandos comeca limpo a cada comando.

### Exemplo rapido

```
> .\dist\fwMSX.exe --msx --msx2
> .\dist\fwMSX.exe --msx --msx2 --cart "Firebird.rom"
> .\dist\fwMSX.exe --msx --msx2 --frames 600 --keys "screen 5:line (20,20)-(120,80),9,bf:a$=input$(1)|" --wait 900 --shot tela.ppm
```

### Limitacoes conhecidas

- **Sem V9958 (MSX2+):** SCREEN 10-12 (YJK/YAE), rolagem horizontal (R#26/R#27) e R#25.
- A imagem e' montada **no fim do quadro**, nao por scanline: efeitos de rastreio no meio
  do quadro (paleta ou rolagem por linha) nao aparecem. Sem borda/overscan nem entrelacamento.
- Sem SCC/FM (F1 Spirit e jogos da Konami rodam sem esse som); sem MSX-DOS 2, Kanji ROM e RS-232.
- Verificado visualmente contra o fMSXgo (boot do BASIC 2.1 e Firebird), **sem diff automatico
  de pixels**. Testado com um unico jogo MSX2 (o Firebird).
- A CMOS do RTC nao persiste entre execucoes.
- Demais limitacoes das versoes anteriores continuam valendo.

---

## v1.10.0 -- "Golvellius: MSX-DOS e Joystick" (2026-10-02)

**Fase:** disco, joystick e cartuchos de verdade. Nome escolhido por "Golvellius"
(Compile) ser um classico de MSX lembrado como um dos jogos de disco/cartucho
mais ambiciosos da plataforma -- e o marco aqui e' o emulador finalmente rodar
software de terceiros: MSX-DOS e jogos reais.

### Destaques

- **O MSX-DOS 1.8 boota.** `fwmsx --msx --disk msxdos1.dsk` carrega
  `MSXDOS.SYS` + `COMMAND.COM` pela **controladora WD2793** (motor em C adaptado do
  `WD1793.c` do fMSX) mapeada em `7FF8h-7FFFh` do slot da `DISK.ROM` (3:1). `dir` e
  `copy` funcionam; o arquivo copiado sai identico ao original (conferido byte a
  byte e por hash). As escritas vao direto para o `.dsk`. Menu **Disco** na
  janela (inserir/ejetar A:/B:); opcoes `--disk`, `--diskb`, `--disk-interface`,
  `--diskrom`. Novo mecanismo `SlotMmio` no barramento de slots.
- **Joystick** (R14/R15 do PSG): setas + Z/Espaco (fogo A) + X (fogo B) na porta A
  e **gamepads do GLFW** (1o -> A, 2o -> B), com menu **Joystick**.
- **Deteccao automatica de mapper** (`MAP_GUESS` do fMSX): `--cart` com mais de
  32KB acerta Konami4/Konami5/ASCII8/ASCII16/Gen8 sozinho.
- **Bug corrigido:** o estado inicial de uma MegaROM mostra os bancos 0,1,2,3 (como
  o fMSX), nao todos no banco 0 -- varios jogos chamam 6000h-7FFFh antes de trocar banco.
- **Jogos reais verificados:** King's Valley (roda; o espaco inicia o jogo) e F1 Spirit
  (roda ate' o menu). **Firebird e' um jogo MSX2** -- confirmado contra um emulador
  de referencia (fMSXgo) -- e espera pelo VDP MSX2.
- 659 verificacoes automatizadas (9 suites).

### Exemplo rapido

```
> .\dist\fwMSX.exe --msx --disk msxdos1.dsk
> .\dist\fwMSX.exe --msx --cart "King's Valley.rom"
> .\dist\fwMSX.exe --msx --disk msxdos1.dsk --frames 600 --keys "|dir|" --wait 120 --shot tela.ppm
```

### Limitacoes conhecidas

- **Disco:** sem FORMAT (READ/WRITE TRACK nao suportados, como no fMSX), so' imagens
  `.dsk` cruas, sem DiskROM do MSX-DOS 2 (exige MSX2). A interface so' liga com
  `--disk`/`--disk-interface`. O `copy` de 7KB leva ~5 s emulados.
- **As escritas do MSX-DOS vao direto para o arquivo da imagem** -- faca backup.
- Sem modos MSX2 (SCREEN 5-8), logo sem Firebird e demais jogos MSX2; SCREEN 3 ainda
  cai no fallback de cor de fundo. Sem SCC/FM (F1 Spirit roda sem o som do SCC).
- Joystick e gamepad foram testados pelo caminho do programa lendo R14 (cartucho
  sintetico); nao ha' teste com gamepad fisico nem das teclas na janela. A janela
  interativa foi validada em abertura/fechamento; digitar pelo teclado do host e o
  som audivel nao tem teste automatico.
- Demais limitacoes das versoes anteriores continuam valendo.

---

## v1.9.0 -- "Gradius 2: Janela e Som" (2026-10-02)

**Fase:** PSG + janela com teclado do host + audio ao vivo. Nome escolhido
por "Gradius 2" (Konami) ser um classico de MSX1 lembrado justamente pelo
som -- e o marco aqui e' o emulador finalmente *soar*, alem de aparecer.

### Destaques

- **`fwmsx --msx`: a maquina MSX1 completa numa janela.** A BIOS real roda
  em tempo real (Dear ImGui + GLFW + OpenGL) com o teclado do host mapeado
  para a matriz do MSX (posicional, layout US; Alt esq. = GRAPH, Alt dir. =
  CODE, End = SELECT, Pause = STOP, F11 = tela cheia). Menu com Reset,
  Pausar, Soltar teclas, Escala inteira e Som. `--cart <arq> [mapper]`
  carrega um cartucho no slot 1 (ROM plana ate' 32KB ou MegaROM).
- **PSG AY-3-8910** (portas `A0h`-`A2h`): motor em C que gera amostras PCM
  de verdade (3 tons, ruido LFSR de 17 bits, 16 formas de envelope), tabela
  de volume logaritmica em Fortran, `PsgDevice` e gravador de WAV em C++.
  Com a BIOS real, `BEEP` no BASIC toca o canal A em 1316 Hz.
- **Audio ao vivo** pelo dispositivo padrao do sistema (miniaudio, baixado
  por FetchContent), via buffer circular sem trava, com pre-enchimento
  contra estalos e menu **Som** (mudo + volume). `--mute` desliga.
- **Modo sem janela:** `fwmsx --msx --frames N --shot tela.ppm --keys
  "print 1234|"` roda, digita e salva a tela -- util para CI.
- Depurador: `--psg`, `psgregs`, `psgpoke`, `psgrec start|stop|clear|save`.
- 570 verificacoes automatizadas (8 suites).

### Exemplo rapido

```
> .\dist\fwMSX.exe --msx
> .\dist\fwMSX.exe --msx --cart jogo.rom
> .\dist\fwMSX.exe --msx --frames 400 --keys "print 1234|" --shot tela.ppm
```

### Limitacoes conhecidas

- So' SCREEN 0/1/2 (+ sprites de modo 1); SCREEN 3 e os modos MSX2 mostram
  so' a cor de fundo (Fase 4 do VDP).
- Sem joystick (R14/R15 do PSG) e sem disco (FDC): so' teclado.
- Sem click de tecla/cassete, SCC nem FM.
- Teclado posicional: acentos/cedilha do ABNT2 nao tem tecla.
- A janela interativa foi validada em abertura/fechamento e no fluxo de
  audio (`--frames N`); a digitacao pelo teclado do host nao tem teste
  automatico (so' `Machine::KeyDown/KeyUp` por baixo dela). O som foi
  validado pelo consumo do dispositivo no ritmo certo, nao ouvido.
- Audio no Linux: compilado e testado no WSL2, sem dispositivo de audio la'
  (o teste do dispositivo real vira `[SKIP]`).
- Demais limitacoes das versoes anteriores continuam valendo.

---

## v1.8.0 -- "Zanac: Prompt do BASIC" (2026-10-01)

**Fase:** a BIOS MSX1 real sobe. Nome escolhido por "Zanac" (Compile) ser
um clássico de MSX1 -- e o marco aqui é exatamente o "Ok" que o
fMSX/MSX BASIC mostra ao ligar.

### Destaques

- **A BIOS real boota até o prompt do MSX BASIC.** Com `fwmsx --z80dbg
  --slots resource/fMSX/ROMs/MSX.ROM --vdp --ppi`, `run 100000000` e
  `vdpshot tela.ppm`, a tela mostra `MSX BASIC version 1.0 / Copyright
  1983 by Microsoft / 28815 Bytes free / Ok` renderizada pelo VDP do
  projeto. `keydown`/`keyup` digitam no BASIC pelo PPI.
- **Bug do núcleo Z80 corrigido:** `Z80Cpu` não chamava `z80_reset()` na
  construção, então as tabelas de flag ficavam zeradas se ninguém desse
  `reset`. Era a causa real de a BIOS ficar presa desde a v1.4.0.
- 470 verificações automatizadas, teste de aceite com a BIOS real.

### Exemplo rápido

```
> .\dist\fwMSX.exe --z80dbg --slots resource\fMSX\ROMs\MSX.ROM --vdp --ppi
z80dbg> run 100000000
z80dbg> keydown p
z80dbg> run 1000000
z80dbg> keyup p
z80dbg> run 2000000
z80dbg> vdpshot tela.ppm
```

### Limitações conhecidas

- Sem janela (só `vdpshot`), sem teclado do host, sem PSG/som/joystick,
  sem cartuchos carregados automaticamente.
- Demais limitações das versões anteriores continuam valendo.

---

## v1.7.0 -- "Vampire Killer: Teclado e PPI" (2026-10-01)

**Fase:** PPI i8255 + teclado -- ver [ppi-spec.md](ppi-spec.md). Nome
escolhido por "Vampire Killer" (Konami) ser um dos jogos de MSX1 mais
jogados no teclado.

### Destaques

- **As quatro linguagens num único módulo:** chip i8255 em **C**
  (adaptado do fMSX), `PpiDevice` em **C++** ligado ao mapa de memória,
  tabela de posição das 87 teclas em **Fortran**, contagem de teclas
  pressionadas em **Assembly** (`POPCNT`, dual-ABI Win64/SysV).
- **O slot primário agora é do PPI:** muda quando o pino de saída da porta
  A muda, e só depois de a BIOS programar o chip (`82h` em `ABh`).
- **Depurador:** `--ppi`, `ppiregs`, `keys`, `keydown`, `keyup`.
- **Layout MSX1 com BIOS + `--ppi`:** RAM de 64KB em `3:2` e regras de
  subslot do MSX1 (slots 0/1/2 sem subslot).
- 464 verificações automatizadas (`ctest`, 5 suítes), 62 novas.

### Exemplo rápido

```
> .\dist\fwMSX.exe --z80dbg --slots resource\fMSX\ROMs\MSX.ROM --vdp --ppi
z80dbg> keydown shift a
z80dbg> keys
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` (MSYS2 UCRT64), `nasm`, `cmake` + `ninja`;
  `dist/fwMSX.exe`/`dist/msxdisk.exe` estáticos. O pacote Linux
  (`.tar.gz`) desta versão precisa ser gerado via `build.sh` numa máquina
  Linux/WSL2 (o Assembly novo já tem a branch SysV).

### Limitações conhecidas

- **A BIOS real ainda não sobe**: programa e lê o PPI, mas fica presa na
  varredura de RAM (`0x0305`-`0x0331`) e não chega a habilitar o VBlank.
  O teste com a BIOS continua informativo (`doc/ppi-spec.md`, seção 5).
- Sem teclado do host (só `keydown`/`keyup`), sem som de click/relé do
  PPI, sem PSG/joystick.
- Demais limitações das versões anteriores continuam valendo.

---

## v1.6.0 -- "Penguin Adventure: Sprites em Cena" (2026-10-01)

**Fase:** VDP, Fase 3 (sprites de modo 1) -- ver [vdp-spec.md](vdp-spec.md).
Nome escolhido por "Penguin Adventure" (Konami) ser a sequência de
"Antarctic Adventure", o nome da versão anterior: o pinguim agora tem
sprites.

### Destaques

- **Sprites de SCREEN 1/2/3 em C**, adaptados de `Sprites()` e
  `CheckSprites()` do fMSX (`src/vdp/core/vdp_sprites.{h,c}`): 8x8/16x16,
  ampliação 2x, prioridade por índice, cor 0 transparente, early clock,
  Y negativo, terminador Y=208 e R#8 bit 1.
- **Quinto sprite** (limite de 4 por linha, flag + número em S#0) e
  **colisão** (S#0 bit 5, linha 192), tudo dirigido por
  `vdp_step_scanline()`; ler S#0 pela porta `99h` limpa os flags.
- Recorte pixel a pixel no lugar das máscaras de bits do original --
  mesmo resultado, verificado nos casos de borda.
- 402 verificações automatizadas (`ctest`, 4 suítes), 20 novas.

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` (MSYS2 UCRT64), `nasm`, `cmake` + `ninja`;
  `dist/fwMSX.exe`/`dist/msxdisk.exe` estáticos.
- Linux (WSL2): pacote `dist/fwMSX-1.6.0-linux.tar.gz` gerado pelo autor
  via `build.sh`. No caminho apareceu um problema de build da GUI: o
  GLFW 3.4 exige `wayland-scanner` por padrão no Linux; o `CMakeLists.txt`
  agora cai para X11 só quando a ferramenta não existe (ver
  `build.sh`, pré-requisitos).

### Limitações conhecidas

- Sem sprites de modo 2 (SCREEN 4-8) nem modos MSX2 -- Fase 4.
- Sem borda/overscan, sem `ScreenON`, sem janela gráfica em tempo real
  (`vdpshot` exporta PPM).
- A BIOS real ainda depende de PPI/teclado para habilitar o VBlank (ver
  `doc/SPEC.md`, seção 5.0).
- Demais limitações das versões anteriores continuam valendo.

---

## v1.5.0 -- "Antarctic Adventure: Primeiros Pixels" (2026-09-30)

**Fase:** VDP (TMS9918/V9938), Fases 0.5, 1 e 2 -- ver
[vdp-spec.md](vdp-spec.md) para a especificação completa. Nome
escolhido por "Antarctic Adventure" (Konami) ser um dos MSX1 mais
lembrados justamente pelos gráficos -- um pinguim correndo sobre um
SCREEN 2 bem aproveitado -- exatamente o tipo de imagem que este
módulo agora sabe decodificar de verdade.

### Destaques

- **`CompositeBus`** (design próprio): o Z80 agora fala com mais de um
  dispositivo de I/O ao mesmo tempo (mapa de memória + VDP), sem
  acoplar os dois -- porta de I/O despachada por dispositivo registrado,
  memória sempre para um único dispositivo designado.
- **Motor "digital" do VDP em C**, adaptado de `resource/fMSX/fMSX/
  MSX.c`: registradores, VRAM, protocolo das 4 portas (`98h`-`9Bh`), e a
  máquina de estados de scanline que gera VBlank/HBlank -- **finalmente
  dá uso real ao `Z80Cpu::interrupt()`**, que existia desde a Fase 1 do
  núcleo Z80 mas nunca tinha sido exercitado. Confirmado com um
  programa sintético de 12 bytes: a interrupção chega em `0x0038` de
  verdade (`IFF1` desligando, não só PC/SP coincidindo por acaso).
- **Renderização real de SCREEN 0/1/2**, adaptada de `resource/fMSX/
  fMSX/Common.h` -- texto mono (240×192), texto colorido com o quirk
  real de "cor por grupo de 8 caracteres" da TMS9918, e bitmap 256×192
  com tabela de cor/padrão em terços. Exportável como imagem PPM
  (`vdpshot`) para inspeção/teste sem precisar de janela gráfica ainda.
- **Bug real corrigido**: a paleta padrão nunca era carregada no reset
  (ficava zerada) -- sem a correção, tudo renderizaria em preto, já que
  software MSX1 normal conta com a paleta fixa do TMS9918 estar
  presente desde o ligar, sem escrevê-la manualmente.
- **Tabela de paleta de 512 cores em Fortran**, construída na Fase 1
  sem consumidor, finalmente usada na Fase 2.
- 382 verificações automatizadas (`ctest`, 4 suítes), até 54 novas desta
  versão.

### Exemplo rápido (poke manual + captura de tela)

```
> .\dist\fwMSX.exe --z80dbg --slots --vdp
z80dbg> vdppoke 0 0x41
z80dbg> vdpshot tela.ppm 0 0
escrito tela.ppm (240x1, modo de tela 0)
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64), `nasm` 3.02, `cmake`
  4.4.3 + `ninja` 1.13.2 -- `dist/fwMSX.exe`/`dist/msxdisk.exe`
  estáticos.
- Linux (WSL2, mesma validação da v1.4.1): pacote gerado via
  `build.sh` pelo autor, com esta versão já incluída.

### Limitações conhecidas

- Sem borda/overscan, sem sprites, sem tela ligada/desligada (`ScreenON`)
  -- só a área ativa de exibição dos três modos suportados.
- Sem janela gráfica em tempo real ainda -- `vdpshot` é a única forma
  de ver o resultado, exportando para um arquivo PPM.
- A BIOS MSX1 real ainda não chega a habilitar a interrupção de VBlank
  dentro de nenhum orçamento de ciclos testado -- precisa de PPI/
  teclado (portas `A9h`-`ABh`), que ainda não existe (ver `doc/SPEC.md`,
  seção 5.0).
- Mesmas limitações já registradas nas versões anteriores (SCC/SRAM/
  `MAP_GMASTER2`/`MAP_FMPAC`/`MAP_GUESS` no mapa de memória, `CPIR`/
  `CPDR` sem aceleração em Assembly).

---

## v1.4.1 -- "Salamander: Compilando em Linux" (2026-09-30)

**Fase:** validação de portabilidade, sem features novas -- primeira vez
que o projeto foi de fato compilado, linkado e executado (não só
compilado no papel) numa máquina Linux real. Nome escolhido por
"Salamander" ser um dos jogos mais emblemáticos de portar entre
plataformas diferentes (arcade, MSX e outros sistemas japoneses),
exatamente o que esta versão prova sobre o próprio projeto.

### O que motivou esta versão

A v1.4.0 já tinha `build.sh` (equivalente Linux do `build.ps1`), mas
nunca tinha sido rodado numa máquina Linux de verdade -- a branch
`elf64`/SysV do `.asm` dual-ABI do núcleo Z80 só tinha sido *montada*
(`nasm -f elf64`), nunca linkada/executada, por falta de ambiente Linux
no desenvolvimento original (só Windows). O autor rodou `build.sh` pela
primeira vez numa máquina Linux real (WSL2) e encontramos, juntos, três
problemas reais de portabilidade -- nenhum deles no código novo do
núcleo Z80/mapa de memória (que já nasceu multiplataforma), todos em
código **mais antigo**, nunca antes testado fora do Windows.

### Bugs corrigidos

1. **`src/asm/init_asm.asm`** (módulo Assembly da própria Fase 0 do
   projeto, de 2026-09-28) -- Win64-only. Montava sem erro para `elf64`
   (o NASM não valida convenção de chamada, só gera bytes), mas tinha
   dois problemas reais rodando de verdade: os argumentos chegariam nos
   registradores errados (SysV entrega major/minor/patch em
   `EDI/ESI/EDX`, não `ECX/EDX/R8D` da Win64), e o **link falhava**:
   ```
   relocation R_X86_64_PC32 against symbol `printf@@GLIBC_2.2.5' can
   not be used when making a PIE object; recompile with -fPIE
   ```
   porque uma chamada direta a `printf` é incompatível com executável
   PIE (posição-independente), o padrão em toda distro Linux moderna.
   Corrigido com a mesma técnica `%ifidn __OUTPUT_FORMAT__` já usada em
   `src/z80/asm/block_ops.asm`, mais `call printf wrt ..plt` (chamada
   relativa à PLT, que funciona com ou sem PIE).
2. **`src/msxdisk/asm/name_match.asm`** (msxdisk) -- mesmo problema de
   registrador errado, mas **mais perigoso** porque não chama nenhuma
   função externa: o build **não falhava**, só o resultado da
   comparação de nome de arquivo (`list`/`extract` com coringa, ex.
   `*.COM`) ficaria silenciosamente errado em tempo de execução no
   Linux. Corrigido com a mesma técnica de mapeamento de registrador de
   entrada por ABI.
3. **`src/msxdisk/gui/file_dialog.cpp`** incluía `<windows.h>` sem
   nenhuma guarda de plataforma -- quebrava a compilação inteira
   (`msxdisk` e `fwMSX`, que compilam os mesmos fontes) fora do
   Windows. Guardado atrás de `#ifdef _WIN32`; fora do Windows, os
   diálogos nativos de Novo/Abrir/Salvar Como devolvem "cancelado" por
   enquanto (a GUI continua funcionando normalmente, só sem seletor de
   arquivo nativo fora do Windows ainda). `comdlg32` (a biblioteca de
   diálogo do Windows, linkada sem condição no `CMakeLists.txt`) também
   corrigida para só entrar quando `WIN32` é verdadeiro.
4. `build.sh` compartilhava `build/` com `build.ps1` -- num checkout
   acessado tanto nativamente pelo Windows quanto via WSL (`/mnt/c/...`
   para o mesmo diretório), o CMake recusava reconfigurar
   (`CMakeCache.txt` "de outro diretório"). `build.sh` agora usa
   `build-linux/`, diretório próprio, resolvendo o conflito de raiz.

### Resultado

```
$ ./build.sh
[...]
==> Rodando testes (ctest)...
Test project /mnt/c/dos/fwMSX/build-linux
    Start 1: z80_smoke
1/3 Test #1: z80_smoke ........................   Passed
    Start 2: z80_debug_session
2/3 Test #2: z80_debug_session ................   Passed
    Start 3: memmap_slots
3/3 Test #3: memmap_slots .....................   Passed
100% tests passed, 0 tests failed out of 3
==> Empacotando fwMSX-1.4.1-linux.tar.gz...
==> Pronto:
    /mnt/c/dos/fwMSX/dist/fwMSX
    /mnt/c/dos/fwMSX/dist/msxdisk
    /mnt/c/dos/fwMSX/dist/fwMSX-1.4.1-linux.tar.gz
```

As mesmas **328 verificações automatizadas** do núcleo Z80 e do mapa de
memória, incluindo o teste diferencial de `LDIR`/`LDDR` que exercita a
aceleração em Assembly de verdade, passaram no Linux.

### Build usado para validar esta release

- **Windows**: `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64), `nasm`
  3.02, `cmake` 4.4.3 + `ninja` 1.13.2 -- `dist/fwMSX.exe` e
  `dist/msxdisk.exe` estáticos.
- **Linux (novo nesta versão)**: WSL2, toolchain de sistema
  (gcc/g++/gfortran/nasm/cmake/ninja via `apt`) -- `dist/fwMSX` e
  `dist/msxdisk`, dinamicamente ligados ao runtime padrão da distro
  (sem o link estático usado no Windows). Pacote:
  `dist/fwMSX-1.4.1-linux.tar.gz`.

### Limitações conhecidas
- Diálogos nativos de arquivo (Novo/Abrir/Salvar Como na GUI do
  msxdisk) só funcionam no Windows por enquanto -- um seletor nativo
  para Linux (ex. GTK) fica para uma tarefa à parte, se fizer sentido.
- Mesmas limitações de escopo já registradas na v1.4.0 (SCC/SRAM/
  `MAP_GMASTER2`/`MAP_FMPAC`/`MAP_GUESS` no mapa de memória,
  `CPIR`/`CPDR` sem aceleração em Assembly) -- ver `SPEC.md`, seção 5.0.
- Ainda não existe VDP, PSG nem uma máquina MSX completa -- trabalho no
  core de emulação continua pausado deliberadamente (ver `SPEC.md`,
  seção 5.0).

---

## v1.4.0 -- "Illusion City: Mapa de Memória" (2026-09-30)

**Fase:** mapa de memória MSX (slots/subslots/MegaROM), sobre o núcleo
Z80 da v1.3.0 -- ver [memory-map-spec.md](memory-map-spec.md) para a
especificação completa e o histórico de todas as 4 fases. **Trabalho no
core de emulação pausado deliberadamente a partir desta versão** (não
abandono) -- ver [SPEC.md, seção 5.0](SPEC.md) para os próximos passos
já registrados, pensados para retomar sem se perder.

Nome escolhido por ser um dos MegaROMs mais emblemáticos do MSX2+ --
uma aventura gigante (16 Mbit) que depende pesadamente de troca de
slot/subslot e bank-switch pra caber no espaço de 64KB do Z80,
exatamente o que este módulo constrói.

### Destaques
- **Motor de slots/subslots em C**, adaptado de
  `resource/fMSX/fMSX/MSX.c`/`MSX.h` -- topologia real de 4 slots
  primários x 4 secundários x 8 páginas de 8KB, incluindo o quirk de
  hardware onde o registrador de slot secundário é indexado pelo slot
  primário que ocupa a página `C000h-FFFFh`, não um registrador global
  único.
- **`MemorySystem`/`SlotMemoryBus`** (C++, design próprio) com uma API
  de depuração (`PeekSlot`/`PokeSlot`/`Describe`/`CurrentView`) que
  enxerga qualquer slot **independente** do que a CPU vê agora --
  requisito vital do autor, atendido desde a primeira fase deste
  módulo, não deixado para uma fase final como aconteceu com o
  depurador do núcleo Z80.
- **Carregamento de ROM real** com checksum **CRC32 em Fortran**
  (verificado contra o vetor de teste padrão de qualquer CRC32).
- **Seis mappers MegaROM** (`Gen8`/`Gen16`/`Konami5`/`Konami4`/
  `ASCII8`/`ASCII16`, só a troca de banco de ROM) -- cobrem a grande
  maioria dos cartuchos MegaROM reais do MSX.
- **`fwmsx --z80dbg --slots [rom]`**: depurador completo com mapa de
  memória real -- `slots`/`pages`/`slotmem`/`slotpeek`/`slotpoke`/
  `loadrom`, com carregamento automático de ROM de boot na abertura.
- **A BIOS MSX1 real do fMSX rodou de verdade** no núcleo Z80 deste
  projeto: 192 endereços de PC distintos visitados em 100 mil ciclos de
  execução real de código de BIOS -- a primeira prova concreta de
  compatibilidade com software MSX real, não só casos de teste
  escritos à mão.
- **`build.sh`**: equivalente Linux do `build.ps1`.
- 328 verificações automatizadas (`ctest -R "z80|memmap"`, até 168 +
  58 + 102), todas passando.

### Exemplo rápido (BIOS real rodando)

```
> .\dist\fwMSX.exe --z80dbg --slots resource\fMSX\ROMs\MSX.ROM
ROM de boot carregada em 0:0: resource/fMSX/ROMs/MSX.ROM
z80dbg> reset
z80dbg> run 5000
parado: orcamento de ciclos esgotado (ciclos consumidos: 5003, PC=0365)
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estáticos, dependências externas
  apenas as DLLs base do Windows/UCRT.
- `build.sh` (Linux) criado nesta versão, mas ainda **não validado em
  execução real** -- só sintaxe (`bash -n`) e a extração de versão via
  `sed`, sem toolchain Linux disponível nesta máquina de
  desenvolvimento. Fica para quando o autor compilar na própria máquina
  Linux.

### Limitações conhecidas
- Ainda não existe VDP, PSG nem uma máquina MSX completa -- ver
  `SPEC.md`, seção 5.0, para os próximos passos.
- SCC, SRAM persistente, `MAP_GMASTER2`/`MAP_FMPAC` e a heurística
  `MAP_GUESS` ficaram deliberadamente de fora do mapa de memória (ver
  `memory-map-spec.md`, seção 6) -- dependem de som/save-state, que
  ainda não existem.
- `CPIR`/`CPDR` continuam sem aceleração em Assembly (só `LDIR`/`LDDR`)
  -- limitação já registrada na v1.3.0.
- A branch `elf64`/Linux do `.asm` dual-ABI do núcleo Z80 continua só
  montada, nunca linkada/executada de verdade (mesma limitação da
  v1.3.0) -- `build.sh` existe agora especificamente para resolver isso
  quando houver uma máquina Linux disponível.

---

## v1.3.0 -- "SD Snatcher: Núcleo do Z80" (2026-09-30)

**Fase:** núcleo de emulação, primeiro pedaço real (CPU Z80) -- ver
[z80-core-spec.md](z80-core-spec.md) para a especificação completa e o
histórico de todas as fases (1 a 4). `fwMSX.exe` em si continua sem
VDP/PSG/mapa de memória de uma máquina MSX real; esta release entrega a
CPU isolada, exercitada sobre uma RAM plana de teste via o depurador
embutido.

### Destaques
- **Motor de despacho do Z80 em C** (`src/z80/core/`), adaptado de
  `resource/fMSX/Z80/` (registradores, tabelas de ciclo/flag, opcodes com
  e sem prefixo `CB`/`ED`/`DD`/`FD`/`DDCB`/`FDCB`), por trás de um
  `Z80Bus` próprio (callbacks + contexto, permitindo múltiplas instâncias
  de CPU no mesmo processo -- o fMSX original usa funções globais).
- **Wrapper de orquestração em C++** (`Z80Cpu`/`IBus`): `reset()`,
  `run(ciclos)`, `interrupt()`, acesso a registradores -- sem expor a
  união de par de registrador na API pública.
- **Tabelas de flag geradas em Fortran** (`ZSTable`/`PZSTable`, usando o
  intrínseco `POPCNT` para paridade), calculadas uma única vez no reset
  da CPU, nunca no caminho quente do despachante.
- **Aceleração de `LDIR`/`LDDR` em Assembly**: primeiro `.asm` do projeto
  com Win64 **e** SysV AMD64 (Linux) no mesmo arquivo-fonte (`%ifidn
  __OUTPUT_FORMAT__`), usada só quando o bloco inteiro cai em RAM plana
  do host -- o loop byte-a-byte original do fMSX continua como caminho
  de reserva sempre que isso não vale (correção nunca depende do caminho
  rápido, só o desempenho).
- **Depurador embutido em `fwMSX.exe`** -- `fwmsx --z80dbg`: REPL
  (replxx) com `reset`/`regs`/`step`/`run`/`break`/`clear`/`breaks`/
  `mem`/`peek`/`poke`/`load`/`fill`/`disasm`, rodando sobre uma RAM plana
  de 64KB de teste.
- **Desmontador Z80**, adaptado do desmontador já existente em
  `resource/fMSX/Z80/Debug.c` (tabelas de mnemônicos + algoritmo de
  substituição de gabarito) em vez de reescrito do zero -- com três
  correções cosméticas documentadas em relação ao original (nenhuma
  afeta execução/timing da CPU, só o texto exibido pelo desmontador).
- 203 verificações automatizadas novas (`ctest -R z80`): 168 no motor
  (`z80_smoke`) e 35 no depurador/desmontador (`z80_debug_session`),
  incluindo uma varredura de completude sem crash sobre 2044 combinações
  de opcode do desmontador e um fuzz diferencial de 150+ casos
  comparando o caminho rápido de `LDIR`/`LDDR` contra o loop lento.

### Saida de referencia do esqueleto (`fwMSX.exe` sem argumentos, inalterada)

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.3.0]
----------------------------------------
Loading module... CPP [v 1.3.0]
Loading module...C [v 1.3.0]
Loading module...Assembly [v 1.3.0]
Loading module Fortran [v 1.3.0]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Exemplo rápido do depurador (`fwmsx --z80dbg`)

```
z80dbg> poke 0x0000 0x21
z80dbg> poke 0x0001 0x34
z80dbg> poke 0x0002 0x12
z80dbg> poke 0x0003 0x76
z80dbg> disasm 0x0000 2
0000: 21 34 12     LD HL,1234h
0003: 76           HALT
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estáticos, dependências externas
  apenas as DLLs base do Windows/UCRT.
- A branch `elf64` do `.asm` dual-ABI foi verificada só até a montagem
  (`nasm -f elf64`, sem link/execução -- não há máquina Linux disponível
  nesta sessão de desenvolvimento); a branch `win64` foi validada
  completa (build + link + execução + testes).

### Limitações conhecidas
- Ainda não existe VDP, PSG nem mapa de memória de uma máquina MSX real
  -- o núcleo roda isolado sobre RAM de teste via `--z80dbg`.
- `CPIR`/`CPDR` não têm aceleração em Assembly (só `LDIR`/`LDDR`) --
  decisão deliberada: as flags dessas instruções dependem do byte
  comparado, não só do contador chegar a zero, tornando o corte de lote
  mais arriscado de acertar sob o mesmo padrão de verificação usado para
  `LDIR`/`LDDR`. Ver `z80-core-spec.md`, Fase 3.
- O caminho rápido em Assembly (`elf64`/Linux) não foi testado em
  execução real, só montagem -- ver acima.
- Mesmas limitações do `msxdisk` já registradas nas releases anteriores
  (`create --dos2` não validado num emulador real).

---

## v1.2.1 -- "Metal Gear: Ajustes de Campo" (2026-09-29)

**Fase:** msxdisk (polimento pós-lançamento). Correções encontradas
testando a GUI de verdade em janela gráfica, depois da v1.2.0.

### Destaques
- **Ejetar disco** na GUI (`Arquivo > Ejetar` / `F12`), com confirmação
  só quando há alterações não salvas.
- **Novo/Abrir/Salvar Como** na GUI viraram diálogo nativo de arquivo do
  Windows (navegação de pastas de verdade), em vez de caixa de texto.
- Dois bugs reais de interface corrigidos: diálogos modais que não
  abriam (escopo de ID do ImGui) e teclas de função que agiam no
  painel/item errado depois de marcar por checkbox (painel "ativo"
  desatualizado). Ambos encontrados e confirmados corrigidos em teste
  real pelo autor.

### Saida de referencia do esqueleto (`fwMSX.exe` sem argumentos, inalterada)

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.2.1]
----------------------------------------
Loading module... CPP [v 1.2.1]
Loading module...C [v 1.2.1]
Loading module...Assembly [v 1.2.1]
Loading module Fortran [v 1.2.1]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estaticos, dependencias externas
  apenas as DLLs base do Windows/UCRT.

### Limitacoes conhecidas
- Mesmas da v1.2.0 (`create --dos2` não validado num emulador real;
  nenhuma emulação de MSX ainda).

---

## v1.2.0 -- "Maze of Galious: Gerenciador de Discos" (2026-09-29)

**Fase:** msxdisk (5 fases completas -- ver
[msxdisk-spec.md](msxdisk-spec.md)). `fwMSX.exe` em si continua na Fase 0
do emulador (esqueleto multi-linguagem, sem Z80/VDP); o que essa release
entrega e o utilitario `msxdisk` (standalone e embutido no `fwMSX.exe`).

### Destaques
- `dist/msxdisk.exe`: um unico executavel para CLI one-shot, shell
  interativo (FTP-like), TUI (Norton Commander/XTree) e GUI (Dear ImGui,
  visual moderno proprio) -- manipulacao completa de imagens `.dsk`
  MSX-DOS 1/2 (criar, listar, adicionar, extrair, renomear, apagar,
  subdiretorios, copiar disco, salvar como).
- `fwmsx --msxdisk <argumentos>`: mesmos quatro modos acessiveis direto
  pelo `fwMSX.exe`, sem precisar do binario separado.
- Configuracao/temas/metadados de imagem em SQLite
  (`~/.msxdisk/config.sqlite3`), compartilhado entre TUI e GUI.
- Novas dependencias externas (buscadas via CMake `FetchContent` no
  build, nao redistribuidas em `resource/`): CLI11, replxx, FTXUI,
  Dear ImGui, GLFW, SQLite (amalgamation).

### Saida de referencia do esqueleto (`fwMSX.exe` sem argumentos, inalterada)

```
Copyright (c) 1972-2026 Cybernostra, Inc.
fwMSX [v 1.2.0]
----------------------------------------
Loading module... CPP [v 1.2.0]
Loading module...C [v 1.2.0]
Loading module...Assembly [v 1.2.0]
Loading module Fortran [v 1.2.0]
----------------------------------------
Assinaturas dos modulos:
  C++       0x0001
  C         0x0002
  Assembly  0x0003
  Fortran   0x0004
```

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe` e `dist/msxdisk.exe`: estaticos (`-static
  -static-libgcc -static-libstdc++ -static-libgfortran`), dependencias
  externas apenas as DLLs base do Windows/UCRT.

### Limitacoes conhecidas
- Nenhuma emulacao de MSX ainda -- `msxdisk` e um utilitario de
  ferramental, nao faz parte do core do emulador em si.
- `create --dos2` (MSX-DOS 2) nao foi validado num emulador/hardware real
  (o `--dos1` foi, no openMSX, MSX1 e MSX2) -- ver
  [msxdisk-spec.md](msxdisk-spec.md), secao 6.
- Varios fluxos interativos da TUI/GUI (dialogos, teclas de funcao,
  duplo-clique) so foram validados parcialmente por automacao (capturas
  de tela pontuais); dependem de confirmacao continua de uso real.

---

## v1.1.2 -- "Nemesis: Renomeacao" (2026-09-28)

**Fase:** 0 -- Esqueleto multi-linguagem (refinamento). Sem mudanca de
comportamento observavel; foco em legibilidade do codigo-fonte.

### Destaques
- Arquivos e funcoes de cada modulo renomeados de `module_<lang>.*` /
  `load_module_<lang>()` para `init_<lang>.*` / `init_<lang>()`, deixando
  o nome do arquivo e da funcao exportada identicos e mais claros sobre o
  papel de cada um (inicializacao do modulo daquela linguagem).
- `CMakeLists.txt`, `src/cpp/main.cpp` e a documentacao (`SPEC.md`,
  `MANUAL.md`, `README.md`) atualizados de acordo.
- Saida do programa **inalterada** em relacao a v1.1.1 (mesmas mensagens
  e assinaturas), apenas com a versao impressa em `[v 1.1.2]`.

### Assinaturas dos modulos (saida de referencia)

| Modulo   | Mensagem                                   | Assinatura |
|----------|---------------------------------------------|:----------:|
| C++      | `Loading module... CPP [v X.Y.Z]`            | `0x0001`   |
| C        | `Loading module...C [v X.Y.Z]`               | `0x0002`   |
| Assembly | `Loading module...Assembly [v X.Y.Z]`        | `0x0003`   |
| Fortran  | `Loading module Fortran [v X.Y.Z]`           | `0x0004`   |

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe`: ~3.1 MB (estatico), dependencias externas apenas
  `ntdll.dll`, `KERNEL32.DLL`, `KERNELBASE.dll` e `ucrtbase.dll`.

### Limitacoes conhecidas
- Nenhuma emulacao de MSX ainda -- apenas o esqueleto de build
  multi-linguagem que servira de base para as proximas fases.
- `resource/` ainda vazio (fontes de referencia a incluir em fases
  futuras).

---

## v1.1.1 -- "Knightmare: Alicerce" (2026-09-28)

**Fase:** 0 -- Esqueleto multi-linguagem. Primeira versao da nova
estrutura de diretorios; ainda nao ha emulacao de MSX.

### Destaques
- Projeto reorganizado em `src/` (um subdiretorio por linguagem), `doc/`,
  `dist/` e `resource/`.
- `main()` em C++ (`src/cpp/main.cpp`) recebendo nome do produto e versao
  via linha de comando, "carregando" um modulo por linguagem (C++, C,
  Assembly, Fortran), cada um retornando sua propria assinatura
  hexadecimal, com resumo final impresso pelo `main`.
- Build via CMake + Ninja sobre o toolchain MSYS2 UCRT64 (GCC/G++/
  GFortran 16.2.0 + NASM 3.02), linkado estaticamente -- `dist/fwMSX.exe`
  depende apenas do UCRT (nativo do Windows 10+).

### Assinaturas dos modulos (saida de referencia)

| Modulo   | Mensagem                                   | Assinatura |
|----------|---------------------------------------------|:----------:|
| C++      | `Loading module... CPP [v X.Y.Z]`            | `0x0001`   |
| C        | `Loading module...C [v X.Y.Z]`               | `0x0002`   |
| Assembly | `Loading module...Assembly [v X.Y.Z]`        | `0x0003`   |
| Fortran  | `Loading module Fortran [v X.Y.Z]`           | `0x0004`   |

### Build usado para validar esta release

- `gcc`/`g++`/`gfortran` 16.2.0 (MSYS2 UCRT64)
- `nasm` 3.02
- `cmake` 4.4.3 + `ninja` 1.13.2
- `dist/fwMSX.exe`: ~3.1 MB (estatico), dependencias externas apenas
  `ntdll.dll`, `KERNEL32.DLL`, `KERNELBASE.dll` e `ucrtbase.dll`.

### Problemas encontrados e corrigidos nesta release
- Saida do modulo Fortran fora de ordem por buffer de E/S proprio do
  `libgfortran` -- ver [CHANGELOG.md](CHANGELOG.md) e
  [SPEC.md](SPEC.md#32-notas-de-implementacao-para-quem-retomar-o-projeto).
- Assinaturas hexadecimais mal formatadas por vazamento de
  `std::left`/`std::right` entre colunas do `std::cout`.

### Limitacoes conhecidas
- Nenhuma emulacao de MSX ainda -- apenas o esqueleto de build
  multi-linguagem que servira de base para as proximas fases.
- `resource/` ainda vazio (fontes de referencia a incluir em fases
  futuras).
