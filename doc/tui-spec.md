# TUI de menus (`--tui`) -- especificacao (documento vivo)

> Introduzida na **1.35.0 "Vampire Killer"** (2026-10-09). `fwMSXc.exe --tui` coloca em **menus** os
> comandos do console e **controla o emulador em janela** pela ponte de controle. Ela NAO embute a
> maquina (para isso existe o modo terminal, `--term`, `doc/term-spec.md`).

## 1. Uso

```
fwMSXc.exe --tui                 # abre os menus; depois Emulador > Iniciar MSX1/MSX2/MSX2+
fwMSXc.exe --tui --attach 7777   # ja' conecta num emulador aberto com --ctl-port 7777
```

No Windows use o **`fwMSXc.exe`** (versao de console): o `fwMSX.exe` e' de janela e o terminal nao o
espera (ver `doc/repl-spec.md`).

## 2. A tela

```
 Emulador  Arquivo  Maquina  Midia  Ferramentas  Ajuda            F10 menus  F1 ajuda   <- barra de menus
 (historico: o que voce mandou, as respostas do emulador e a saida das ferramentas)
 Emulador conectado :51607 | SCREEN 0 | quadro 4210 | PAUSADO | slot 1                   <- status
 > peek 0xC000 16_                                                                       <- linha de comando
```

O **status** e' atualizado a cada segundo (`status` da ponte). A **linha de comando** entende os mesmos
comandos do console (`doc/repl-spec.md`): `emu`, `peek`, `poke`, `type`, `cart`, `disk`, `state`,
`newdisk`, `romdb`, `cas`, `fitadb`, `msxdisk`... Os comandos rodam numa thread a parte; um `romdb` que
baixa arquivos nao trava a tela (o status mostra "executando...") e a saida das ferramentas vai para o
historico.

## 3. Menus

| Menu | Itens |
|---|---|
| **Emulador** | Iniciar MSX1 / MSX2 / MSX2+ - Iniciar com opcoes... - Conectar a um emulador aberto... - Desconectar - Encerrar - Estado |
| **Arquivo** | Salvar / Carregar estado (slot N) - Slot anterior / proximo - Salvar estado em arquivo... - Carregar estado de arquivo... - Capturar tela (PNG)... - Sair da TUI |
| **Maquina** | Reiniciar - Pausar/Retomar - Avancar 1 / 10 quadros - Registradores - Ler memoria... - Gravar memoria... - Digitar texto no MSX... |
| **Midia** | Inserir / Retirar cartucho (com mapper) - Inserir / Ejetar disco A: e B: - Novo disco em branco... - Novo disco e inserir em A:... - Inserir / Ejetar / Rebobinar fita |
| **Ferramentas** | Banco de ROMs (buscar, mostrar, verificar SHA-1, identificar, estatisticas, baixar fMSX, comando livre) - Fita: listar / comando livre - Banco de fitas (listar, buscar, estatisticas) - Utilitario de discos (msxdisk) |
| **Ajuda** | Teclas e uso - Comandos do console - Versao |

Itens com `...` abrem um **assistente**: um prompt por dado (com valor padrao), na ordem; **Enter**
avanca/confirma e **Esc** cancela. Onde a pergunta e' um arquivo (cartucho, disco, fita, estado), abre um
**navegador de arquivos**: setas, **Enter** (entra na pasta ou escolhe o arquivo), **Backspace** (sobe
uma pasta), **`t`** (digita o caminho a mao), **Esc**. O navegador so' lista os arquivos da extensao
certa (`.rom/.mx1/.mx2/.bin`, `.dsk`, `.cas/.tsx/.tzx`, `.sst`).

## 4. Teclas

| Tecla | Faz |
|---|---|
| **F10** | abre/fecha a barra de menus (sempre no menu Emulador); **setas** navegam, **Enter** executa, **Esc** fecha |
| **F1** | ajuda (qualquer tecla fecha) |
| **mouse** | clicar num titulo abre o menu; clicar num item executa; clicar fora fecha |
| **Enter** | executa a linha de comando |
| **setas cima/baixo** | historico de comandos |
| **Tab** | completa o comando (prefixo comum; mostra as opcoes se houver varias) |
| **Home/End/setas/Delete/Backspace** | editam a linha |
| **PgUp/PgDn** | rola o historico |
| `exit` / `quit` | sai da TUI (o emulador continua aberto) |
| `clear` | limpa o historico |

## 5. Arquitetura

```
src/tui/tui_model.{h,cpp}   TuiModel: teclas, menus, assistentes, navegador, linha de comando -- SEM FTXUI
src/tui/tui_app.{h,cpp}     desenho (FTXUI), thread dos comandos, consulta periodica do status
src/repl/                   ReplSession / EmuLink / BuildSession(): a mesma sessao do console
```

- A TUI de menus e' o **console com menus**: cada item monta uma linha de comando e a entrega ao
  `ReplSession` (a mesma sessao de `fwMSXc.exe --cli`), que a executa ou a encaminha ao emulador pela
  ponte de controle. Nao ha' logica de emulacao aqui.
- `TuiModel` recebe teclas abstratas e devolve linhas de comando por um callback, sem terminal: e' o que
  o teste cobre.
- `emu start` abre o emulador em **janela** (`fwMSX.exe`, irmao do `fwMSXc.exe`) com a ponte ligada.

## 6. Testes

- `tuitest` (CTest `tui_model`): barra e menus, navegacao com setas (separadores pulados), todos os
  comandos montados pelos menus (`emu start`, `reset`, `pause`/`resume`, `cart`, `disk`, `newdisk`,
  `state ... slot N`, `romdb search`...), assistentes de 1 e 2 passos com valores padrao e cancelamento,
  escape de aspas no `type`, navegador de arquivos (filtro por extensao, entrar/subir pasta, digitar o
  caminho), dois comandos de um item so' (novo disco + inserir), linha de comando (historico, cursor,
  Delete, Tab, `clear`, `quit`), rolagem do historico e cliques do mouse.
- Teste de fumaca com o executavel real: `fwMSXc.exe --tui --attach <porta> --run "peek 0 4" ... --dump`
  desenha um quadro de 80x24 e sai. Mostrou os menus, a conexao ao emulador em janela, `peek`/`regs`,
  `newdisk`, o erro de um comando desconhecido e a saida de `romdb stats` capturada no historico.
- **Nao testado:** a digitacao e o mouse num terminal de verdade (aqui so' ha' pipes); a logica por tras
  deles e' a coberta pelo `tuitest`.

## 7. O que falta

- Teclas de atalho por letra nos menus (Alt+letra) e menus com mnemonicos.
- Configuracao de slots/disco/video pela TUI (hoje so' pelas opcoes de `emu start`).
- Mostrar a lista de resultados de `romdb`/`fitadb` como tabela navegavel (hoje e' texto no historico).
- Depurador e montador (do PaleoBASIC/msxIDE) como menu e comandos.
- Eventos assincronos do emulador (breakpoints) aparecendo no historico.
