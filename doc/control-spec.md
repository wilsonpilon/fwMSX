# Ponte de controle externa -- especificacao (documento vivo)

> Introduzida na **1.32.0** (2026-10-09). Permite que outros programas (editor de texto,
> depurador, montador, a futura TUI...) mandem comandos ao emulador em execucao.

## 1. Ideia

Em vez do XML do openMSX, um **protocolo de texto de uma linha por comando**, sobre **TCP em
127.0.0.1**. Da' para testar com `telnet`/`nc`, e qualquer linguagem fala isso com um socket e
`readline`.

```
> peek 0xC000 4
< ok 00 01 02 03
> poke 0xC000 0x41
< ok
> cart nao-existe.rom
< err nao foi possivel abrir ...
```

- Requisicao: `comando arg arg...`, terminada por `\n` (aceita `\r\n`).
- Resposta: UMA linha, `ok [texto]` ou `err texto`.
- Argumentos com espaco entre aspas duplas: `state save "C:\meus jogos\a.sst"`. Dentro das aspas so'
  `\"` e' escape; a barra invertida dos caminhos do Windows e' literal.
- Numeros: `123`, `0x7B`, `$7B`, `7Bh`.

## 2. Comandos

| Comando | O que faz |
|---|---|
| `help` / `version` | lista os comandos / versao do fwMSX |
| `status` | `frame=N paused=0 screen=M pc=XXXX typing=0 cart=... diskA=... tape=...` |
| `reset` | reinicia a maquina |
| `pause` / `resume` | pausa / retoma a emulacao |
| `step [n]` | avanca n quadros (1-3600); so' com a maquina pausada |
| `type <texto>` | digita no teclado do MSX; `\n` = ENTER (use aspas); teclas em `{}` fora das aspas ({enter} {esc} {f1} {ctrl}+{c} {ctrl}{c} {enter*3}, ver secao 2b); retorna na hora, a digitacao anda a cada quadro |
| `peek <end> [n]` | le n bytes (1-4096) como o Z80 enxerga agora; `ok 41 42 ...` |
| `poke <end> <v>...` | grava bytes (ROM nao e' alterada) |
| `regs` | registradores do Z80 |
| `cart <arq\|-> [mapper]` | troca o cartucho (reinicia); `-` retira; mapper: auto, gen8, gen16, konami5, konami4, ascii8, ascii16, msxdos2 |
| `disk <A\|B> <arq>` / `eject <A\|B>` | insere / ejeta disco |
| `tape <arq>` / `tape eject` / `tape rewind` | fita |
| `state save <arq>` / `state load <arq>` | save-state (`ok aviso: ...` se a BIOS/cartucho mudou) |
| `screenshot <arq>` | grava a tela (PNG na janela) |
| `quit` | fecha o emulador |

Depurador (1.37.0, ver [debug-spec.md](debug-spec.md)): `break`, `unbreak`, `breaks`, `watch`, `unwatch`, `watches`,
`stepi`, `next`, `until`, `go`, `stop`, `jump`, `dis`, `mem`, `stack`, `reg`, `slots`, `events` e `dbgui` (o painel Z80 Debugger como texto,
[debug-spec.md](debug-spec.md), secao 6c).

### Respostas de varias linhas

Os comandos que listam coisas (`dis`, `mem`, `reg`, `stack`, `slots`, `breaks`, `watches`) respondem
`ok +N` e, nas N linhas seguintes, o texto. Os comandos antigos nunca emitem `+N`, entao clientes que so'
conhecem os comandos antigos continuam funcionando.

```
> dis 0 2
< ok +2
<  0000  F3           DI
<  0001  C3 D7 02     JP 02D7h
```

### Eventos assincronos

O cliente que manda `events on` (por conexao; `events off` cancela) recebe, a qualquer momento, linhas
`! evento chave=valor ...` -- hoje os do depurador (`! break`, `! watch`, `! until`). Nunca no meio de uma
resposta. O console (`--cli`) e a TUI assinam sozinhos ao conectar.

```
> go
< ok running
< ! break id=1 pc=0038 hits=1        <- chegou sem ninguem pedir
```

## 2b. Sintaxe do `type` (1.37.7)

O argumento e' lido da linha crua, em pedacos:

- **"texto"**: cada caractere vira a tecla do layout da BIOS (shift quando precisa); `\n` = Enter, `\\` = barra, `\"` = aspas.
- **texto solto** (sem aspas): igual; palavras separadas por espacos viram UM espaco.
- **{tecla}**: uma digitacao (desce, 3 quadros, sobe). Nomes = a matriz do MSX (a-z, 0-9, enter, esc, tab, bs, del, ins, home, stop,
  select, space, f1-f5, ctrl, shift, graph, caps, code, up/down/left/right, pad0-pad9...), sem diferenciar maiusculas; apelidos: return,
  escape, backspace, delete, insert, control, cls (=home), break (=stop).
- **{a}+{b}+{c}**: UMA digitacao com as teclas juntas (descem na ordem, sobem na inversa): `{ctrl}+{c}` = Control-C.
  `{ctrl}{c}` sao duas digitacoes (Control apertado e solto, depois o C). `{ctrl+c}` e' atalho de `{ctrl}+{c}`.
- **{tecla*N}**: repete N vezes (1-200); numa combinacao vale para a combinacao toda.
- **`--keys`** (linha de comando, sem janela) usa o MESMO interpretador (`ParseTypeSpec`, modo literal): `{tecla}`, `{a}+{b}`, `{tecla*N}` e barra-n valem;
  aspas e espacos sao texto comum e `|` continua sendo Enter. Um `{` que antes era digitado como texto agora abre uma tecla.
- Erros: `tecla desconhecida`, `'{' sem '}'`, `aspas sem fechar`. Chaves DENTRO das aspas sao texto.

## 3. Como ligar

```
fwMSX.exe --msx --ctl-port 0        # 0 = o sistema escolhe uma porta livre
fwMSX.exe --msx --ctl-port 7777     # porta fixa
```

A porta aberta e' impressa em stderr e escrita em **`fwmsx.port`** (pasta de trabalho), que e'
apagado ao sair. Escuta **so' em 127.0.0.1**, nunca na rede.

## 4. Arquitetura

```
src/control/
+-- command.{h,cpp}   Commander: Execute(linha) -> Result; Tick() por quadro (fila do "type")
+-- server.{h,cpp}    Server: TCP (Winsock/POSIX), uma thread por cliente
```

- **Uma camada de comandos para tudo.** `Commander` fala com a maquina pela interface `Host`
  (maquina, pausa, sair, trocar cartucho, captura de tela). A janela implementa `Host`; a TUI, o modo
  terminal e a linha de comando usam o MESMO `Commander` -- o que a TUI faz e o que uma ferramenta remota faz
  e' a mesma coisa.
- **Concorrencia.** As threads do servidor so' leem/escrevem no socket e ENFILEIRAM; quem executa
  e' a thread do emulador, chamando `Server::Poll()` uma vez por volta do laco (nenhum codigo do
  emulador roda fora dela). Um cliente espera ate' 10 s pela resposta (`err o emulador nao
  respondeu a tempo`).
- A maquina pode ser recriada pelo `Host` (trocar de cartucho reinicia), por isso o `Commander`
  nunca guarda um ponteiro para ela.
- `Machine::ReadMemory()/WriteMemory()` expoem a memoria como o Z80 enxerga.

## 5. Testes

- `controltest` (CTest `control`): o `Commander` sobre uma `Machine` MSX1 de verdade (numeros,
  quebra de linha, `peek`/`poke` inclusive em ROM, `pause`/`step`, `type` -- o BASIC executa
  `print 1234` digitado pela ponte -, `state`, `disk`/`tape`/`cart`, erros) e o servidor TCP real
  (cliente numa thread, comandos executados via `Poll()`).
- Smoke test manual com a janela real (`fwMSX.exe --msx --ctl-port 0` + cliente Python):
  `version`, `status`, `type`, `peek`, `poke`, `regs`, `screenshot`, `pause`/`step`/`resume`,
  `quit` -- e `fwmsx.port` removido ao sair.

## 6. O que falta

- **Mais eventos** (`! frame ...`, E/S, interrupcoes): hoje so' os do depurador (1.37.0); ver `doc/debug-spec.md`.
- Depurador avancado (condicoes, simbolos, trace -- `doc/debug-spec.md`, secao 7), VDP/PSG, joystick,
  `fwmsx --cmd "..."` na linha de comando (sem janela) e a **TUI** que consome o mesmo
  `Commander`.
- Autenticacao por token (hoje basta ser localhost).
- Um unico caminho de captura de tela por comando: na janela e' PNG; a TUI/headless precisara de
  outro `Host`.
