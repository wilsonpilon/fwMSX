# Mamute -- o monitor Z80 do fwMSX (1.40.0, "Antarctic Adventure")

O **Mamute** e' o monitor Z80 do fwMSX: um prompt de linha onde cada comando mexe na memoria, disassembla,
executa, salva e carrega programas, no estilo dos monitores dos anos 80 (MegaAssembler, SUPER-X). Vem do
**Mamute Assembler do Paleobasic** (`resource/paleobasic/src/editor/assemblers/Mamute*.pbi`, ~21 mil linhas de
PureBasic, ~60 comandos), reescrito para o fwMSX em C + Fortran + Assembly (NASM) + C++, **sem janela separada**:
o mesmo monitor serve ao console (CLI), a TUI e a ponte de controle.

Documentacao do usuario: `man mamute` (e `man mamute-modes|memory|dump|exec|files|config`) e a secao "Mamute" do
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
| 2 | (a definir) | `EDIT`/`A`: editor de linhas do fonte numerado, traduzido para o fwasm; `R` (referencia de fita) |
| 3 | (a definir) | Telas: `DM`/`M`/`S` interativos como paineis da TUI (setas, TAB, edicao de byte), `ZAP` (setores de `.dsk`), `SCR` (grafico), `P`/`LP` em PDF |
| 4 | (a definir) | Comandos do **SUPER-X** (prefixo `X`, 36 comandos): `XD XA XI XM XH`, `XRG XGO XTR`, `XBT XRT XFL XCM XFD`, `XCS XTS`, `XCO XSD`, `XDK XFS XCI XTP XSV XLD XS# XL# XL% XS%`, notas `XIM XIC XIL XIS XIR`, portas `XPP XPI XPO` |

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
```

- **`Monitor::Execute(linha) -> texto`** nao sabe de TUI, de console nem de janela. `IMemory` esconde se atras esta a
  memoria privada ou a viva; `IExec` o Z80 privado ou o da maquina. O mesmo comando vale nos dois modos.
- **Offline**: 16 blocos de 16 KB (slot 0-3 x pagina 0-3). Tipos de celula: `VAZIO`, `RAM`, `ROM`, `BASIC` (so' `RAM`
  aceita escrita; escrita em outra celula e' ignorada, sem erro, e o log conta os bytes ignorados). `PAGE` escolhe o slot de
  cada pagina. A CPU e' o `Z80Cpu` do fwMSX (portas: `IN` le FFh, `OUT` e' ignorado -- o painel de portas fica para a Fase 4).
- **Online**: leitura/escrita pela vista da CPU da maquina (`ReadMemory`/`WriteMemory`, escrita so' onde a pagina e'
  gravavel), registradores da CPU dela, `G` poe o PC e solta pelo depurador (`AddBreak` + `Resume`). `PAGE` so' mostra
  (o mapeamento e' o da maquina). A VRAM online e' so' leitura.
- **Onde o monitor mora**: no `Commander` (comando `mon <linha>`, `mon @prompt`) -- assim vale na TUI, no CLI e pelo TCP; a
  maquina so' e' ligada ao monitor DURANTE cada comando (o host pode recria-la). Sem emulador, a `ReplSession` roda um monitor
  **local** (so' offline). O modo (`mon`, prompt `MON(..)>`, `BA` sai, `!comando` roda um comando do console) e' da `ReplSession`,
  que a TUI tambem usa -- a linha de entrada da TUI mostra o prompt do monitor.
- **Parametros**: `fwmsx --mon` (alias `--mamute`) abre o console ja' dentro do monitor; `--mon --attach <porta>` liga a um
  emulador. O arquivo `mamute.cfg` fica ao lado do executavel (`SetConfigDir` no `main`).

## 4. Diferencas em relacao ao Mamute do Paleobasic

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
- `mantest`: as 7 paginas do manual do Mamute e a categoria "Mamute" no help.

## 6. O que falta

Fases 2, 3 e 4 acima. Tambem: completar o TAB do console com os comandos do monitor; `R` (carregar assemblado de fita); `V`/`SCR` com
escrita na VRAM; ligar as portas de E/S da CPU offline ao painel (Fase 4); e continuar a comparar o disassembler com o do Mamute original.
