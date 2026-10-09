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
| `type <texto>` | digita no teclado do MSX; `\n` = ENTER (use aspas); retorna na hora, a digitacao anda a cada quadro |
| `peek <end> [n]` | le n bytes (1-4096) como o Z80 enxerga agora; `ok 41 42 ...` |
| `poke <end> <v>...` | grava bytes (ROM nao e' alterada) |
| `regs` | registradores do Z80 |
| `cart <arq\|-> [mapper]` | troca o cartucho (reinicia); `-` retira; mapper: auto, gen8, gen16, konami5, konami4, ascii8, ascii16, msxdos2 |
| `disk <A\|B> <arq>` / `eject <A\|B>` | insere / ejeta disco |
| `tape <arq>` / `tape eject` / `tape rewind` | fita |
| `state save <arq>` / `state load <arq>` | save-state (`ok aviso: ...` se a BIOS/cartucho mudou) |
| `screenshot <arq>` | grava a tela (PNG na janela) |
| `quit` | fecha o emulador |

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
  (maquina, pausa, sair, trocar cartucho, captura de tela). A janela implementa `Host`; a TUI e a
  linha de comando usarao o MESMO `Commander` -- o que a TUI faz e o que uma ferramenta remota faz
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

- **Eventos assincronos** (`! breakpoint ...`, `! frame ...`): so' pergunta e resposta por enquanto.
- Comandos de depuracao (breakpoints, desmontar, passo a passo de instrucao), VDP/PSG, joystick,
  `fwmsx --cmd "..."` na linha de comando (sem janela) e a **TUI** que consome o mesmo
  `Commander`.
- Autenticacao por token (hoje basta ser localhost).
- Um unico caminho de captura de tela por comando: na janela e' PNG; a TUI/headless precisara de
  outro `Host`.
