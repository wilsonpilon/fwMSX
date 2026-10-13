# Depurador basico e eventos assincronos -- especificacao (documento vivo)

> Introduzido na **1.37.0 "Knightmare"** (2026-10-13). E' o depurador do EMULADOR (a maquina inteira: slots,
> VDP, interrupcoes), diferente da sessao `--z80dbg` (`src/z80/debug/`), que depura o nucleo Z80 isolado.
> Pedido do usuario: um depurador basico, parecido com o do proprio fMSX, "apenas para ter algo"; o
> sofisticado (openMSX + Compass/Konpass + Mega Assembler + Super-X + DEBUG do MS-DOS) vem depois, por partes.

## 1. O que existe

- **Breakpoints de execucao** (PC): a maquina para ANTES da instrucao nesse endereco.
- **Watchpoints de memoria** (leitura, escrita ou as duas, faixa de 1 a 65535 bytes): a maquina para DEPOIS da
  instrucao que tocou a memoria; o evento diz quem escreveu (PC) e o valor.
- **Passos**: `stepi [n]` (uma instrucao), `next` (passo sobre CALL/RST/LDIR.../HALT), `until <end>`, `go [end]`,
  `stop`, `jump <end>`.
- **Inspecao**: `reg` (tudo, com flags), `dis`, `mem`, `stack`, `slots`; `reg <nome> <valor>` altera.
- **Eventos assincronos**: o emulador AVISA o cliente (`! break id=1 pc=0038 hits=1`) sem ele perguntar.

Ao parar, a maquina congela: `RunFrame()` nao faz nada (o quadro nao anda, o video fica na ultima imagem,
o titulo da janela mostra "PARADO: depurador", `status` mostra `stopped=1`). Comandos de depuracao funcionam
parado ou rodando.

## 2. Comandos (ponte de controle, console `--cli`, TUI e modo terminal)

| Comando | O que faz |
|---|---|
| `break <end>` / `unbreak <end\|all>` / `breaks` | adiciona / remove / lista breakpoints (`id`, endereco, quantas vezes parou) |
| `watch <end> [tam] [r\|w\|rw]` / `unwatch <end\|all>` / `watches` | watchpoints (padrao: escrita, 1 byte) |
| `stepi [n]` | executa n instrucoes (1-1000000) e fica parado; responde `pc=XXXX  <proxima instrucao>` |
| `next` | passo sobre: CALL, CALL cc, RST, LDIR/LDDR/CPIR/CPDR/INIR/INDR/OTIR/OTDR e HALT rodam inteiros; o resto e' um `stepi` |
| `until <end>` | roda ate o endereco (ate 20 quadros na hora; se nao chegar, responde `running` e o evento `! until` avisa) |
| `go [end]` | volta a rodar (opcionalmente a partir de `end`); responde `running` na hora |
| `stop` | para na proxima fronteira de instrucao |
| `jump <end>` | muda o PC |
| `dis [end] [n]` | desmonta n linhas (padrao 8) a partir de `end` (padrao PC); `>` marca o PC, `*` um breakpoint |
| `mem <end> [n]` | hexdump com ASCII (16 bytes por linha, ate 1024) |
| `stack [n]` | n palavras a partir de SP |
| `reg` / `reg <nome> <valor>` | todos os registradores e flags / altera (af bc de hl ix iy sp pc af' bc' de' hl' a f b c d e h l i r) |
| `slots` | de que slot:subslot vem cada pagina de 16 KB agora |
| `events <on\|off>` | assina/cancela os eventos nesta conexao TCP (o console e a TUI assinam sozinhos) |
| `status` | ganhou `stopped=0\|1` |

Enderecos e numeros: `0x4010`, `$4010`, `4010h` ou decimal. Respostas de varias linhas (dis, mem, reg, stack,
slots, breaks, watches): ver `doc/control-spec.md`, secao "Respostas de varias linhas".

### Parametros do `--msx` (e de `emu start ...` no console/TUI)

| Opcao | Efeito |
|---|---|
| `--break <end>` | breakpoint inicial (pode repetir) |
| `--watch <end>[:tam[:r\|w\|rw]]` | watchpoint inicial (pode repetir) |
| `--debug` | comeca PARADA no PC 0000 (use `stepi`/`go` pelo console/TUI/ponte) |

Valem tambem depois de trocar o cartucho pela janela (ficam na configuracao). Sem janela (`--frames N`), se a
maquina parou, o programa imprime `depurador: parado em PC=XXXX (motivo)` e os eventos pendentes.

## 3. Eventos

Linhas que comecam por `! `, uma por evento, a qualquer momento (entre respostas, nunca no meio de uma):

```
! break id=1 pc=0038 hits=1
! watch id=2 addr=C100 rw=w value=77 pc=C012 next=C015
! until pc=C003
```

- O servidor so' envia ao cliente que mandou `events on` (por conexao). O console (`--cli`) e a TUI mandam sozinhos
  ao conectar; no console o evento aparece como `[evento] break id=1 ...` no proximo prompt ou ao fim do comando;
  `wait [segundos]` espera um evento (ex.: depois de `go`). Na TUI, vai para o historico (a barra mostra
  `PARADO (depurador)`). No modo terminal, vai para o log e para o aviso. Na janela, vai para o stderr.
- Comandos sincronos (`stepi`, `next`, `until`) ja' devolvem o resultado na resposta e NAO geram evento.
- A fila de eventos guarda no maximo 256 (sem cliente, os mais antigos caem).

## 4. Arquitetura (as 4 linguagens)

```
src/debug/
  cpp/debugger.{h,cpp}     C++: breakpoints, watchpoints, parada, fila de eventos (nao conhece a maquina)
  cpp/debug_bus.h          C++: decorador do barramento do Z80 -- entrega cada leitura/escrita aos watchpoints
  c/dbg_flow.{h,c}         C: classifica a instrucao (CALL/RST, bloco, HALT) para o `next`
  fortran/dbg_fmt.f90      Fortran: formata o registrador F ("SZyHxPNC") para `reg`
  asm/dbg_range.{h,asm}    Assembly (NASM, Win64/SysV): teste "addr em [base, base+len)" com volta em 16 bits,
                           chamado a cada acesso de memoria enquanto houver watchpoint
```

- `Machine::RunFrame()` executa instrucao a instrucao (`StepInstruction()`), testa `Debugger::CheckPc()` antes
  de cada uma e `TakeWatchHit()` depois; parado, retorna sem fazer nada. `RunUntilStop(ciclos)` e' a versao
  sincrona (usada por `next`/`until`, limite de 20 quadros).
- O Z80 enxerga a memoria por `dbg::DebugBus` (sem watchpoints custa um teste de booleano por acesso; com
  watchpoints o caminho rapido de LDIR/LDDR e' desligado para nao deixar copias passarem sem ser vistas).
- `control::Commander` ganhou os comandos; `Result::Line()` ganhou o formato multilinha; `control::Server`
  ganhou a assinatura por conexao e `PushEvent()`; `EmuLink` entende `ok +N` e separa os eventos.
- A disassembly usa `z80::debug::Disassemble` (adaptado do `Debug.c` do fMSX, ver `LICENSE-THIRD-PARTY.md`).

## 5. Testes

`controltest` (CTest `control`): breakpoint (para ANTES da instrucao, o quadro nao anda, evento, `*` no `dis`),
`stepi`, `go` (a primeira instrucao pula o teste do PC), `next` sobre um CALL real na RAM, `until`, watchpoint de
escrita (evento com endereco, valor e PC; a escrita aconteceu), `reg`/`mem`/`stack`/`slots`, argumentos
invalidos, `--debug`/`--break`/`--watch` vindos da configuracao, e pelo TCP: `events on`, resposta `ok +2` de
duas linhas e o evento `! break` chegando sem ninguem pedir. `tuitest`: menu Depurador. Fumaca real (janela +
console + TUI): sessao completa com `--ctl-port`.

## 6. Decisoes

- **Pare ANTES do breakpoint, DEPOIS do watchpoint**: como em qualquer depurador (o PC do breakpoint ainda nao
  executou; o watchpoint so' sabe do acesso quando a instrucao termina).
- **Resposta de varias linhas = `ok +N`** e N linhas, em vez de escapar a quebra de linha: continua legivel no
  `telnet`, nao ambigua com barras invertidas de caminhos do Windows, e compativel para tras (comandos antigos
  nunca emitem `+N`).
- **Eventos por conexao, resolvidos no servidor**: `events on` nao passa pela thread do emulador.
- **Sem condicoes, sem simbolos, sem trace** nesta primeira parte (de proposito: "apenas para ter algo").

## 6b. Depurador na janela (Z80 Debugger, 1.37.2)

Painel visual no estilo do fMSX (`resource/fMSX/Z80/ConDebug.c`), aberto por **F10** ou pelo menu Depurador, ou sozinho quando a
maquina para por breakpoint/watchpoint/`until`/`--debug`. Teclas: ENTER (passo), Cima/Baixo, Esquerda/Direita (pagina), G, D, M, S,
J, R, C, B (extra: breakpoint no cursor), Q e Esc. Layout: titulo, 4 linhas de registradores (AF BC DE HL / IX IY SP PC / sombra / I R
IM EI flags), uma linha em branco e o corpo -- `ENDERECO: bytes instrucao` na desmontagem, `ENDERECO: 8 bytes + os 8 caracteres` no dump,
`SP+xx ENDERECO: palavra` na pilha -- e 4 linhas de rodape (mensagem/pergunta e a ajuda).

- **Fonte do MSX**: `LoadFontFromMachine` copia a tabela de caracteres da BIOS (CGTABL, o endereco esta' em 0004h-0005h da ROM
  principal); `RenderRgba` desenha a grade de 44 x 32 caracteres em 264 x 256 pixels (6 x 8 por celula, so' os 6 bits de cima de
  cada linha da fonte, como o texto do MSX) numa textura mostrada em escala inteira sobre a janela. Sem fonte valida, a janela
  mostra o mesmo texto com a fonte da interface (`GridRowText`).
- **Modelo sem OpenGL** (`src/debug/cpp/debug_view.{h,cpp}`): `DebugView` recebe teclas ja' traduzidas (`Key`), age na `Machine`
  (`StepInstruction`, `Debugger::SetUntil/Resume/AddBreak`) e desenha numa `Grid`; por isso e' testado em `controltest` (teclas,
  paginas, pergunta do G, dump com 8 bytes e caracteres, pilha, J/R/C/Q/B, fonte e pixels). Ligacao com o ImGui em
  `emu_window.cpp` (`IsKeyPressed` + `InputQueueCharacters` para a pergunta; desenho pela lista de desenho de primeiro plano).
- **Teclado**: com o painel aberto, `g_input.dbg_capture` faz o `KeyCallback` descartar tudo (exceto F10 e F11), as teclas do MSX
  sao soltas ao abrir, e nada chega ao MSX nem ao joystick.
- **Entrada e saida**: abrir para a maquina; fechar com Esc/F10 a deixa PARADA (marca a parada como "manual", senao reabriria
  sozinho); C e R retomam; se a maquina volta a rodar por outro caminho (menu, console) o painel fecha.
- **Q encerra o emulador** (como no fMSX), sem confirmacao.

## 7. O que falta (o depurador sofisticado, por partes)

Ideias a misturar, do que o usuario citou:
- **openMSX**: breakpoints com condicao (`if` sobre registradores/memoria), pontos de parada em E/S (`in`/`out`)
  e em interrupcoes, visualizacao de VDP/PSG/slots/mapper, simbolos (`.sym`), `trace` / `log` de instrucoes,
  paineis de memoria/registradores/desmontagem em tempo real.
- **Compass / Konpass, Mega Assembler, Super-X**: depurador integrado ao montador (quando o `fwasm` gerar
  `.sym`/mapa, `dis` e `break` aceitarem rotulos), montagem em linha de instrucoes, edicao de memoria em tela cheia.
- **DEBUG do MS-DOS**: comandos curtos (`a` montar, `s` procurar, `f` preencher, `m` mover, `c` comparar, `t`/`p`
  passo/proc, `g` go, `r` registradores) como apelidos dos comandos atuais.
- Painel do depurador tambem na TUI (registradores + desmontagem + pilha atualizados a cada parada) e atalhos de teclado
  (F5/F9/F10/F11 como nos depuradores classicos); na janela o painel ja' existe (secao 6b).
- Breakpoint que sobrevive a troca de cartucho pela janela ja' sobrevive quando vem de `--break`; os criados pelos
  comandos se perdem (a maquina e' recriada) -- guardar na configuracao.
- `fwmsx --cmd "..."` e scripts (`source`) para automatizar sessoes.
