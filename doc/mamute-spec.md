# Mamute -- o monitor Z80 do fwMSX (1.40.0 "Antarctic Adventure"; Fase 2 na 1.40.1; Fase 3 na 1.40.2; Fase 4 na 1.40.3; fita na 1.40.4)

O **Mamute** e' o monitor Z80 do fwMSX: um prompt de linha onde cada comando mexe na memoria, disassembla,
executa, salva e carrega programas, no estilo dos monitores dos anos 80 (MegaAssembler, SUPER-X). Vem do
**Mamute Assembler do Paleobasic** (`resource/paleobasic/src/editor/assemblers/Mamute*.pbi`, ~21 mil linhas de
PureBasic, ~60 comandos), reescrito para o fwMSX em C + Fortran + Assembly (NASM) + C++, **sem janela separada**:
o mesmo monitor serve ao console (CLI), a TUI e a ponte de controle.

Documentacao do usuario: `man mamute` (e `man mamute-modes|memory|dump|exec|edit|screens|superx|files|config`) e a secao "Mamute" do
[MANUAL](MANUAL.md). Este documento e' o projeto e as decisoes.

## 1. Pedido e decisoes (2026-10-19)

Pedido do usuario: integrar o Mamute (depurador/montador/editor de memoria/dumper) do paleobasic ao fwMSX, convertido
para o mix de linguagens do projeto, na TUI, na CLI e na passagem de parametros, com HELP, `man` e uma opcao de
configuracao inicial; sem a janela separada do original; ligado ao fwMSX o monitor pode ou nao refletir as alteracoes de
memoria no programa em execucao -- um comando ONLINE/OFFLINE, para criar e depurar um programa no monitor e so' depois
testar no fwMSX, ou escrever uma rotina em C000h e, no BASIC, `DEFUSR=&HC000:A=USR(0)`. Todas as opcoes do Mamute.

Decisoes (perguntadas ao usuario antes de codar, como pede o `CLAUDE.md`):

| Pergunta | Decisao |
|---|---|
| Entrega | **Em fases**, uma release por fase; cada fase fecha com `ctest` 100% no Windows e no Linux |
| EDIT/A (montar o fonte numerado estilo ZX-81) | **Traduzir para o fwasm**: as linhas numeradas viram texto asMSX e o fwasm monta (evita portar o `Z80Asm.pbi` de 4298 linhas e manter dois montadores) |
| Modo offline | **Memoria privada de 16 blocos + o nucleo Z80 do fwMSX**; o online troca essa memoria pela da maquina; PUSH/PULL levam blocos entre os dois |
| Telas que eram janelas (DM, S, ZAP, SCR, XIR, XPP) | **Paineis na TUI + versao de linha no CLI**; o SCR (grafico 256x192) fica para a janela grafica |

## 2. Fases

| Fase | Release | Conteudo |
|---|---|---|
| **1** | **1.40.0 "Antarctic Adventure"** | Nucleo (memoria offline/online, CPU offline/online), `mon`/`mamute`, `--mon`, prompt `MON(off)>`/`MON(on)>`, config `mamute.cfg`, `man mamute` (7 paginas) e help. Comandos: `BA QUIT XQT ONLINE OFFLINE STATUS PUSH PULL PAGE DM M SH MS C D P V T F L LP X G LOAD SAVE CFG HELP` |
| **2** | **1.40.1 "Antarctic Adventure: Mamute, EDIT e A"** | `EDIT`/`LIST`/`A`/`MAP`: editor de linhas do fonte numerado e montagem pelo fwasm (listagem, `O N P I R S D H /desloc`). `R` (fita) so' validava a sintaxe no original e continua pendente |
| **3** | **1.40.2 "Antarctic Adventure: Mamute, telas"** | Telas: `DM`/`M`/`S` interativos como paineis da TUI (setas, TAB, edicao de byte), `ZAP` (setores de `.dsk`), `SCR` (grafico, em braille), `P`/`LP`/`V`/`A P`/`A H` em PDF |
| **4** | **1.40.3 "Antarctic Adventure: Mamute, SUPER-X"** | Os comandos do **SUPER-X** (prefixo `X`, 36 comandos no original): `XD XA XI XM XH`, `XRG XGO XTR`, `XBT XRT XFL XCM XFD`, `XCS XTS`, `XCO XSD XQT`, `XDK XFS XCI XTP XSV XLD XS# XL# XL% XS%`, notas `XIM XIC XIL XIS XIR`, portas `XPP XPI XPO`; mais o `CL` (calculadora) e as variaveis `@`, que o SUPER-X usa e a Fase 1 nao tinha |
| **+** | **1.40.4 "Antarctic Adventure: Mamute, Fita"** | Fita cassete em todos os pontos: `TAPE`, `R` (agora de verdade), `TLOAD`, `TSAVE`, `LOAD`/`SAVE` de `.cas/.tsx/.tzx/.wav`, `A OT>arq.cas`; online usa a fita da maquina. Ver 3e |

## 3. Arquitetura (as 4 linguagens)

```
src/mamute/
  core/mamute_dump.{h,c}        C        uma linha de despejo (modos 0-3); chama o checksum
  fortran/mamute_sum.f90        Fortran  mamute_checksum8: soma dos bytes (+ byte baixo do endereco), modulo 256
  asm/mamute_scan.{h,asm}       NASM     mamute_find: busca de bytes COM CURINGA (laco quente do SH); ABI dupla Win64/SysV
  cpp/mamute_config.{h,cpp}     C++      mamute.cfg (chave=valor), celulas dos 16 blocos, VRAM, modo inicial
  cpp/mamute_memory.{h,cpp}     C++      IMemory + OfflineMemory (4 slots x 4 paginas de 16 KB + VRAM)
  cpp/mamute_exec.{h,cpp}       C++      IExec + OfflineExec (Z80Cpu privado sobre a memoria offline)
  cpp/mamute_online.{h,cpp}     C++      MachineMemory/MachineExec: a memoria e a CPU da maquina emulada
  cpp/mamute_monitor.{h,cpp}    C++      Monitor: uma linha entra, o texto do log sai
  cpp/mamute_source.{h,cpp}     C++      (Fase 2) o fonte numerado: Source, traducao para o asMSX, Assemble, listagem e simbolos
  cpp/mamute_edit.cpp           C++      (Fase 2) os comandos EDIT/LIST/A/MAP dentro do Monitor
  core/mamute_reloc.{h,c}       C        (Fase 4) tamanho de instrucao Z80 e onde fica o endereco absoluto / o salto relativo (base do XRT)
  fortran/mamute_total.f90      Fortran  (Fase 4) mamute_sum16: o checksum agregado de 16 bits do XTS
  asm/mamute_cmp.{h,asm}        NASM     (Fase 4) mamute_diff: compara dois blocos e lista os indices que diferem (ou batem); laco quente do XCM
  cpp/mamute_util.h             C++      (Fase 4) Trim/Upper/Hex/ParseHexN/SplitCommas compartilhados
  cpp/mamute_target.{h,cpp}     C++      (Fase 4) o enderecamento estendido (#slot-sub, #V, #S) e as variaveis @0-@3 @B @E @S
  cpp/mamute_calc.{h,cpp}       C++      (Fase 4) o CL: expressoes de 16 bits, hexa por padrao, sufixos D B H O
  cpp/mamute_superx.h           C++      (Fase 4) o estado do SUPER-X (SuperX), as notas, os paineis e o disassembler sobre um alvo
  cpp/mamute_superx_cmds.cpp    C++      (Fase 4) XD XA XI XH XM XBT XRT XFL XCM XFD XTS XCS XCO XSD XRG XGO XTR XPP XPI XPO
  cpp/mamute_superx_disk.cpp    C++      (Fase 4) XDK XFS XCI XTP XL% XS% XSV XLD XS# XL# e as notas XIM XIC XIL XIS XIR
  cpp/mamute_superx_panels.cpp  C++      (Fase 4) os paineis XA XI XH XIR XPP XTP XTR (o XD e' uma variante do GridPanel)
```

### 3e. A fita (1.40.4 "Mamute: Fita")

Pedido do usuario (2026-10-19): suporte total a fita cassete em todos os pontos, com comandos novos se preciso. O Mamute do paleobasic nao tinha nada alem da sintaxe do `R`; aqui o `R` carrega de verdade e ha' comandos novos.

- `cpp/mamute_tape.cpp` (C++): `TAPE`, `R`, `TLOAD`, `TSAVE`, `LOAD`/`SAVE` com `.cas/.tsx/.tzx/.wav` e `StoreOnTape` (a opcao `T` do `A`). A logica de formato e' do `src/tape` (`tape_io`, `tape_deck`); aqui so' a sintaxe e a colocacao do bloco na memoria
  (`ChooseSlot` + `PlaceBlock`, que o `LOAD` tambem usa).
- **Qual fita**: ONLINE e' a da MAQUINA (`IMemory::Tape()` devolve `&Machine::tape()`): o que o Mamute grava o BASIC enxerga e a posicao e' uma so'; `TAPE MODE FAST|NORMAL` troca o carregamento da maquina. OFFLINE o `Monitor` tem um `tape::TapeDeck` proprio
  (sem Z80, sem BIOS, sem pulsos). `TAPE` sem argumento diz qual esta' valendo (`[FITA DA MAQUINA]` ou `[FITA DO MONITOR]`).
- **`R [desloc]`**: carrega o PROXIMO arquivo binario a partir de `tape_next_` no endereco do cabecalho (+ desloc, hexa) e mostra o EXEC (+ desloc); avanca `tape_next_` e o ponto da fita (na maquina o `CLOAD` seguinte continua dali). `TAPE REW|SEEK|NEW|<arq>` zeram/movem `tape_next_`.
- **`TLOAD n|nome[,slot[,end]]`**: por numero (do `TAPE LIST`) ou nome (sem diferenciar maiusculas). Binario vai para o endereco do cabecalho; com `<end>` vai so' o programa nesse endereco; BASIC e texto so' com `<end>`.
- **`TSAVE nome,ini,fim[,exec]`**: acrescenta um arquivo binario (cabecalho com o nome de 6 letras + dados) respeitando protecao e `TAPE WRITE`. **`SAVE arq.cas|.tsx|.wav,ini,fim[,exec]`** cria um arquivo de fita; **`LOAD arq.cas|...[,slot[,end]]`** abre o arquivo SEM inseri-lo e carrega o primeiro binario.
- **`A OT>jogo.cas`**: a opcao `T` grava o codigo-objeto montado na fita (cria `jogo.cas`/`.tsx`/`.wav` se o `>nome` tem essa extensao; senao acrescenta na fita inserida).
- **Achado**: `ChooseSlot` online sem slot usava `PageMap()[1]` (slot da pagina 1, ROM) e nao gravava em C000; agora usa o slot da pagina do endereco (offline continua: o primeiro slot com RAM).
- Testes: `mamutetest` de 407 para 455 checagens (offline completo, ONLINE na maquina, `.cas/.tsx/.wav`, `A OT>`).

### 3d. Os comandos do SUPER-X (Fase 4)

- **Alvo (`Target`)**: `4000#3` (slot 3), `#3-1` (sub-slot 1), `#V` (VRAM), `#S` (o PAGE). `IMemory` ganhou `ReadSub`/`WriteSub`: offline, o sub-slot 0 e' o proprio slot (so' grava onde e' RAM) e os sub-slots 1-3 sao 64 KB de RAM
  cada, alocados na primeira escrita (ate' 768 KB); online, `PeekSlot/PokeSlot(slot, sub)` da maquina, e **`#3` sem sub-slot vale o sub-slot que a CPU enxerga naquele slot** (numa maquina com o slot 3 expandido a RAM nao esta' no sub-slot 0).
  `MachineMemory::WriteVram` passou a gravar (`Machine::PokeVram`). `TRead/TWrite/TargetWrap/TargetMax` escondem o alvo; o `<fim>` de um bloco e' sempre um endereco puro no MESMO alvo e nada da a volta.
- **Variaveis**: `Vars` com 7 posicoes (endereco + alvo). XD/XA/XI/XM/XH gravam o endereco aberto em `@0`; XLD/XL# preenchem `@B` `@E` `@S`. No `CL` a variavel vale so' o numero.
- **Tela x listagem**: `XD`/`XA`/`XI` com UM endereco abrem a tela (painel na TUI; no console a mesma tela sem cursor); com dois ou tres campos listam (`<fim>` vazio = 256 bytes, ate' o teto do alvo; terceiro campo = arquivo:
  bytes crus no XD/XA, texto no XI). `?XD` manda a listagem para a "impressora" = PDF. `XD` usa `mamute_checksum8` (Fortran) por linha; `XCM` usa `mamute_diff` (Assembly); `XTS` usa `mamute_sum16` (Fortran); `XRT` usa `mamute_insn_info` (C).
- **Sessao do XM**: `Monitor::sx_.xm_active` troca o prompt para `XM <end>> `; a linha e' instrucao (montada pelo fwasm no endereco atual, num `Source` de duas linhas), dados (`. : ; [ "`, itens pelo `CL`), um endereco (o alvo continua) ou `I [n]`.
  `DAA`/`CCF` sozinhos sao a instrucao. A `ReplSession` trata `QUIT` como "volta ao monitor" em qualquer sessao (prompt que nao comeca em `MON(`).
- **XRG/XGO/XTR**: usam a CPU do modo (`IExec` ganhou `StepOne` e `Halted`). 5 breakpoints nomeados (`BP BP1 BP2 BP3 BPF`) no `SuperX`; `XGO` offline roda ate' o breakpoint da vez, o RET que devolve alem da pilha de entrada (soma de 16 bits, para nao
  falhar com SP perto de 0000), HALT ou 2.000.000 instrucoes; `XGO <end>` reinicia o HALT. Online, `XGO` solta a CPU da maquina com o breakpoint e `XTR` para a maquina e executa uma instrucao. `XTR` na TUI abre o painel (ENTER = proxima).
- **Portas**: `PortTable` (256 portas: `mon`, `in`=Entrada, `out`=Saida, `changed`) ligada ao `Bus` da CPU OFFLINE: `IN` le a Saida (FF se a porta nao esta' no painel), `OUT` grava a Entrada, cria a porta e marca. Nao ha' hardware simulado.
- **Disco**: o "disco corrente" (`XDK`) e' uma imagem `.dsk`; `XFS`/`XCI` usam `msxdisk::DiskImage` (o nucleo do msxdisk entrou em `CONTROL_LIB_SOURCES`), `XTP` extrai o arquivo para um temporario e mostra como texto, `XL%`/`XS%` leem e gravam setores
  direto no arquivo. Sem o dialogo de arquivo do original, sem disco corrente os comandos pedem o `XDK`.
- **Notas**: formato de texto `AAAA;slot;tipo;texto` (o `SUPER-X-PT.notas` do paleobasic, 471 notas traduzidas, vai em `dist/` e no pacote ao lado do executavel); `XIL` sem argumento le esse arquivo. O `.TNK` japones original nao e' usado.
- **XCO**: guarda as cores (indices 0-15 do MSX1) no `mamute.cfg` (`color=F,B,R`) e as mostra; nao ha' janela para pintar.

### 3c. Os paineis (Fase 3)

- **`Panel`** (`mamute_panel.h`): `Render(mem)` devolve as linhas (a primeira e' o titulo; o cursor entre `\x01` e `\x02`) e `Key(mem, tecla)` aplica uma tecla por NOME (`up down left right pgup pgdn home end tab enter esc bs plus minus space save c:<char>`);
  `false` = fechar. O painel nao guarda o `IMemory` (o adaptador da maquina e' temporario): recebe o atual a cada chamada.
- **`GridPanel`** (DM, M, S, ZAP): 16 linhas de 8 bytes + texto; `DM`/`ZAP` com campo de edicao (ENTER abre/confirma, ESC cancela; hexa pre-preenchido que o 1o digito substitui; texto = `(caractere - desloc) & FF` e o cursor avanca);
  `M`/`S` com digito pendente (`3_`); escrita em celula nao-RAM so' avisa; ZAP guarda a imagem inteira, o conjunto de setores sujos e grava so' o setor do cursor.
- **`ScreenPanel`** (SCR): `CharAddr(cx,cy)` = azulejos de dx x dy caracteres (`per_row = ceil(32/dx)`), indice dentro do azulejo `iy*dx+ix` (modo 0) ou `ix*dy+iy` (modo 1), 8 bytes por caractere; braille por pontos (no zoom 2 cada ponto e' o OU de um bloco 2x2);
  moldura = 2x2 caracteres no canto; edicao 16x16 com snapshot dos 32 bytes para o ESC.
- **Protocolo e versao de linha**: ver `RELEASE.md` v1.40.2 e `tui-spec.md` 6e. `Monitor::panels_` liga (`@panels on`); sem ele os mesmos comandos sao de linha (`DM` continua 128 bytes depois, `S end,teclas`, `ZAP`/`ZAPW`, `SCR`).
- **PDF** (`mamute_pdf.cpp`): PDF 1.4 minimo, A4, Courier 9 pt; `WriteListing` escolhe PDF quando o nome termina em `.pdf`.

### 3b. O editor de fonte e a montagem (Fase 2)

- **Fonte**: `Source` guarda as linhas numeradas (decimais, 1-65535). Formato `NN [Label:] instrucao operandos [;comentario]`; numeros no texto sao hexa por padrao (`H`, `B` binario, `D` decimal).
- **Montar = traduzir**: uma linha do asMSX por linha do Mamute, na mesma ordem (um erro `line N` do fwasm vira o numero da linha do Mamute). `.zilog` ligado (o Mamute escreve `(HL)`); numeros viram decimais;
  `DEFM 'x'` vira `db "x"`; `Label: EQU v` vira `Label equ v`; `ORG` com `/desloc` soma direto no numero (parenteses em sintaxe Zilog seriam acesso a memoria); `END` e' ignorado. O apostrofo so' abre
  texto quando nao esta' colado numa letra (`EX AF,AF'` e' o registrador alternado; `DEFM 'TEXTO'` tem espaco antes).
- **Rotulos que colidem**: `PRINT`, `START`, `SIZE`... sao diretivas do fwasm (`fwasm::IsDirectiveName`) e davam "reserved word used as identifier" ate' num operando; a traducao renomeia (`PRINT_m`) so' os rotulos
  definidos que colidem (nunca registradores/condicoes) e o `.sym` volta com o nome certo.
- **Listagem**: `AsmOptions::want_listing` faz o fwasm devolver, por linha de fonte, o endereco (`$`) onde ela comecou e os bytes gerados; `EQU` devolve o valor. A listagem mostra linha, endereco (ou valor do EQU), ate' 4 bytes (linhas
  extras para o resto) e o texto; `ORG`/`END` ficam de fora. Simbolos: valores do `.sym`, ordem de aparicao do proprio fonte; a **referencia cruzada** procura o simbolo (palavra inteira) nos operandos das outras linhas.
- **Saidas**: `O` escreve a imagem pela memoria ativa do monitor (offline, ou a RAM da maquina no online); `I` grava `NOME.bin` (BSAVE `FE ini fim exec`, buracos com zero); `P` `NOME.lst`; `H` `NOME.sym`.

- **`Monitor::Execute(linha) -> texto`** nao sabe de TUI, de console nem de janela. `IMemory` esconde se atras esta a
  memoria privada ou a viva; `IExec` o Z80 privado ou o da maquina. O mesmo comando vale nos dois modos.
- **Offline**: 16 blocos de 16 KB (slot 0-3 x pagina 0-3). Tipos de celula: `VAZIO`, `RAM`, `ROM`, `BASIC` (so' `RAM`
  aceita escrita; escrita em outra celula e' ignorada, sem erro, e o log conta os bytes ignorados). `PAGE` escolhe o slot de
  cada pagina. A CPU e' o `Z80Cpu` do fwMSX (portas: `IN` le a "Saida" do painel de portas do `XPP`, ou FFh; `OUT` grava a "Entrada" -- Fase 4).
- **Online**: leitura/escrita pela vista da CPU da maquina (`ReadMemory`/`WriteMemory`, escrita so' onde a pagina e'
  gravavel), registradores da CPU dela, `G` poe o PC e solta pelo depurador (`AddBreak` + `Resume`). `PAGE` so' mostra
  (o mapeamento e' o da maquina). A VRAM online e' so' leitura nos comandos da Fase 1; `#V` dos comandos do SUPER-X (Fase 4) escreve nela.
- **Onde o monitor mora**: no `Commander` (comando `mon <linha>`, `mon @prompt`) -- assim vale na TUI, no CLI e pelo TCP; a
  maquina so' e' ligada ao monitor DURANTE cada comando (o host pode recria-la). Sem emulador, a `ReplSession` roda um monitor
  **local** (so' offline). O modo (`mon`, prompt `MON(..)>`, `BA` sai, `!comando` roda um comando do console) e' da `ReplSession`,
  que a TUI tambem usa -- a linha de entrada da TUI mostra o prompt do monitor.
- **Parametros**: `fwmsx --mon` (alias `--mamute`) abre o console ja' dentro do monitor; `--mon --attach <porta>` liga a um
  emulador. O arquivo `mamute.cfg` fica ao lado do executavel (`SetConfigDir` no `main`).

## 4. Diferencas em relacao ao Mamute do Paleobasic

- Fase 4: os dialogos de arquivo (`XDK`, `XSV`, `XLD`, `XS#`, `XL#`, `XSD`, `XIL`, `XIS`, `?XD`) viram argumentos; sem disco corrente os comandos de disco pedem o `XDK`; o `XFD` recebe a instrucao no comando;
  as janelas (`XD XA XI XH XIR XPP XTP` e o trace do `XTR`) viram paineis na TUI e telas de linha no console. Alem do original: os alvos valem tambem no `XI`, `XSD`, `XFD`, `XBT`; `#3` online e' o sub-slot visivel;
  `#V` escreve na VRAM da maquina; `XRG BP,-` limpa um breakpoint; `?XD` aceita o nome do arquivo; `XGO <end>` reinicia o HALT. Ainda nao existem a "cruz" de troca entre XD/XA/XI/XM/XH e o `CLS`.
- Fase 2: os dialogos do `SAVE`/`LOAD`/`MERGE`/`A ...I` viram argumentos; `P` e `H` gravam texto (nao PDF); `QUIT` no EDIT volta ao monitor.
- Sem janelas: `DM`/`M` mostram a grade de 128 bytes no log; `M end,b1,b2...` grava (a edicao interativa e' a Fase 3).
- `P`/`LP`/`V` gravam um arquivo **texto** (o original gerava PDF A4).
- `SAVE` de arquivo `.rom` sai **sem cabecalho** (`RAW`, uma imagem de cartucho de verdade); no original saia `AB`+enderecos.
  O formato `ROM` (AB) continua disponivel por `SAVE ...,ROM`. `LOAD` aceita `arq[,slot[,end]]` (o original abria uma janela de arquivo).
- `LOAD`/`SAVE`/`CFG` trabalham por argumentos (nao ha' janela "Salvar como").
- Padrao do offline: **slot 3 todo RAM** (o original comecava com tudo vazio, sem como escrever).
- Alem do original: **ONLINE/OFFLINE/STATUS/PUSH/PULL**, a ligacao com a maquina, e o `G`/`X`/`L` funcionam de verdade (no original
  `G` so' validava a sintaxe e o `L` era so' do disassembler).
- O disassembler e' o do fwMSX (`z80_disasm`, derivado do fMSX), nao o do Mamute; as formas nao documentadas (`IXH`...) seguem o que o fMSX mostra.

## 5. Testes

- `mamutetest` (`tests/z80/mamute_test.cpp`, ~70 checagens): offline (PAGE, M/DM, formatos 0-3 e checksums, MS/SH com
  deslocamento e curinga, T com sobreposicao, escrita em celula nao-RAM, L, X/G com breakpoint e HALT, LOAD/SAVE com cabecalho BLOAD e RAW, CFG
  e arquivo de configuracao) e **online com a maquina real** (BIOS ligada: `PAGE ?`, leitura da BIOS, escrita so' em RAM,
  PUSH/PULL, `G C000` faz a CPU da maquina rodar o programa escrito no monitor).
- `repltest`: modo monitor sem emulador (prompt, `!comando`, `BA`, `mon <linha>`) e **conectado a um emulador** (ONLINE, escrita
  na maquina conferida por `peek`).
- `mantest`: as 10 paginas do manual do Mamute e a categoria "Mamute" no help.
- Fase 4: `mamutetest` (de 193 para 407 checagens: `CL`, `@`, alvos `#slot-sub`/`#V` offline e online, XD/XA/XI de dois e tres campos e `?XD`, XCS/XTS, XBT/XCM, XRT com JP/CALL/JR dentro e fora do bloco e o caso invalido, XFD, a sessao do XM
  (instrucao, dados, endereco, `I`, `DAA`, sub-slot), XH, XRG/XGO/XTR com breakpoints, retorno do RET, HALT e teto, portas IN/OUT, XCO, XSD nos 4 modos, XSV/XLD/XS#/XL# com desloc e VRAM, disco (XDK/XFS/XCI/XTP/XL%/XS% sobre um `.dsk` criado
  no teste), as 471 notas reais, e todos os paineis por teclas) e `repltest` (o prompt `XM <end>>`, `QUIT` volta ao monitor).
- Fase 3: `mamutetest` (PDF, DM/S/ZAP/SCR de linha, todos os paineis por teclas `@key`, ZAP com arquivo, SCR modos 0 e 1 e a edicao 16x16), `tuitest` (teclas -> `mon @key`) e `repltest` (`EnablePanels`).
- Fase 2: `mamutetest` (EDIT/A: hexa, B, D, erros por linha, RENUM/DELETE/CHANGE/SEARCH, SAVE/LOAD/MERGE, P/I/R/S/D/H, `/desloc`, e `A O` na maquina real) e `repltest` (QUIT/BA no EDIT).

## 6. O que falta

As fases 1 a 4 e a fita (1.40.4) estao prontas. Falta: o SCR grafico (pixels de verdade) na janela ImGui; o TAB do console com os comandos do monitor; a "cruz" de troca entre
XD/XA/XI/XM/XH e o `CLS`; o `SCR` com escrita na VRAM; comparar o disassembler com o do Mamute original; e as cores do `XCO` valerem de fato (TUI e janela). Depois do Mamute vem o depurador sofisticado.
