# Console (REPL) do fwMSX -- especificacao (documento vivo)

> Introduzido na **1.33.0 "Penguin Adventure"** (2026-10-09). `fwmsx --cli` abre um console
> interativo, no estilo do console do openMSX: o terminal fica livre, o emulador roda em outro
> processo e o console manda comandos para ele pela ponte de controle (`doc/control-spec.md`).

## 1. Uso

> **No Windows use o `fwMSXc.exe`** (versao de console) para o console interativo. O `fwMSX.exe` e'
> do subsistema "janela" (sem console preto no duplo clique), e o terminal nao espera um programa
> assim: ele e o console disputam o teclado -- letras trocadas, lentidao, o console "encerra sozinho".
> O build gera os dois a partir do MESMO programa (so' muda o campo Subsystem do cabecalho PE);
> `emu start` abre sempre a janela (`fwMSX.exe`). No Linux existe um unico `fwMSX`.

```
fwMSXc.exe --cli                  # abre o console
fwMSXc.exe --cli --attach 7777    # ja' conecta num emulador aberto com --ctl-port 7777

fwmsx> emu start --msx2p --cart jogo.rom msxdos2     # abre a janela do emulador e conecta
fwmsx:51607> peek 0xC000 4
00 00 00 00
fwmsx:51607> type "print 6*7\n"
fwmsx:51607> state save meu.sst
fwmsx:51607> emu stop
fwmsx> newdisk novo.dsk ds35
```

## 2. Comandos

**Do console (locais):**

| Comando | O que faz |
|---|---|
| `emu start [opcoes]` | inicia o emulador como processo independente (`fwMSX --msx --ctl-port <porta livre> <opcoes>`) e conecta; as opcoes sao as do `--msx` |
| `emu attach <porta>` | conecta num emulador ja' aberto com `--ctl-port` |
| `emu detach` | desconecta (o emulador continua aberto) |
| `emu stop` | manda `quit` ao emulador e desconecta |
| `emu status` | estado do emulador conectado |
| `help` / `exit` / `quit` | ajuda / sai (o emulador continua aberto) |

**Ferramentas** (as opcoes soltas que ja' existiam, agora como comandos; funcionam sem emulador):
`newdisk` (= `--disknew`), `romdb`, `cas`, `fitadb`, `msxdisk`.

**Todo o resto** vai para o emulador conectado como esta' (peek, poke, type, cart, disk, eject, tape,
state, screenshot, pause, resume, step, regs, reset, quit...; ver `doc/control-spec.md`). A
resposta `ok texto` aparece como `texto`; `err texto` como `erro: texto`. O prompt mostra a porta:
`fwmsx:51607> `.

## 3. Arquitetura

```
src/repl/
+-- emu_link.{h,cpp}        EmuLink: cliente TCP da ponte; FindFreePort(); SpawnDetached() (Win32 / posix_spawn)
+-- repl_session.{h,cpp}    ReplSession: comandos locais + encaminhamento + ferramentas registradas
+-- repl.{h,cpp}            RunReplCommand(): replxx (historico em ~/.fwmsx_history, TAB) + registro das ferramentas
```

- O console **nao embute a maquina**: e' um cliente. Por isso o mesmo caminho serve a um emulador
  iniciado por aqui, por um atalho ou por outra ferramenta (editor, depurador do msxIDE/PaleoBASIC).
- `emu start` reserva uma porta livre, inicia o emulador com stdin/stdout/stderr em NUL (o console
  nao e' sujo por mensagens dele) e tenta conectar por ate' 15 s.
- Se o emulador for fechado (pela janela, por exemplo), o proximo comando avisa `erro: o emulador
  foi encerrado` e o console volta ao prompt sem emulador.
- No Windows ha' DOIS executaveis do mesmo programa: `fwMSX.exe` (subsistema janela) e `fwMSXc.exe`
  (subsistema console), gerado no pos-link pelo utilitario `tools/pe_subsystem` (equivale a
  `editbin /SUBSYSTEM:CONSOLE`). O `fwMSX.exe --cli` num terminal recusa e manda usar o `fwMSXc.exe`.
  `attach_parent_console()` tambem aponta os handles do sistema para o console (o replxx usa
  `GetStdHandle`).
- O `fwMSXc.exe` tambem serve para todas as ferramentas de linha de comando (`--romdb`, `--cas`,
  `--disknew`...): o terminal espera o fim e o codigo de saida e' o certo.

## 4. Testes

- `repltest` (CTest `repl`): comandos sem emulador, `help`, ferramentas registradas, `emu attach`/
  `detach`/`stop`, encaminhamento contra um servidor de controle REAL (com uma Machine MSX1),
  `emu start` com um iniciador falso (confere `--msx --ctl-port <porta> <opcoes>`) e o emulador que
  cai no meio.
- Smoke test manual com o executavel real (entrada por pipe): `emu start` abriu a janela, `status`,
  `peek`, `screenshot`, `emu stop`, `exit`.

## 5. O que falta

- O modo interativo de verdade (terminal com TAB/setas) so' foi exercitado por pipe neste ambiente.
- Comandos de **depuracao** (breakpoints, desmontar, passo de instrucao) e **montador**: a ideia e'
  trazer para o console as funcoes do depurador e do montador do PaleoBASIC/msxIDE (chamando o asMSX
  ou reescrevendo em C++) -- rodadas proprias.
- **Eventos assincronos** do emulador para o console (`! breakpoint ...`).
- `fwmsx --cmd "..."` (um comando e sai) para scripts, e arquivo de script (`source`).
- A **TUI** (FTXUI) com as mesmas opcoes da GUI, sobre o mesmo `Commander`.
