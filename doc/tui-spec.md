# TUI do emulador -- especificacao (documento vivo)

> Introduzida na **1.34.0 "Metal Gear"** (2026-10-09). `fwMSXc.exe --tui` roda a maquina MSX
> **dentro do terminal** (FTXUI), com menu, linha de comando e os mesmos atalhos da janela.

## 1. Uso

```
fwMSXc.exe --tui                                   # MSX1 no terminal
fwMSXc.exe --tui --msx2p --cart jogo.rom msxdos2   # aceita as MESMAS opcoes do --msx
fwMSXc.exe --tui --ctl-port 0                      # e liga a ponte de controle, como a janela
```

No Windows use o **`fwMSXc.exe`** (versao de console): o `fwMSX.exe` e' de janela e o terminal nao o
espera (ver `doc/repl-spec.md`). Num terminal de 80x24 ou maior; truecolor melhora os graficos.

## 2. A tela

- **SCREEN 0, 1 e TEXT80** (o prompt do BASIC, MSX-DOS...): desenha os **caracteres de verdade**, nitidos,
  com as cores do registrador 7 do VDP.
- **Graficos** (SCREEN 2 em diante): cada celula do terminal mostra 2 pixels (meio bloco, ▀: cima = cor
  da frente, baixo = cor de fundo), reduzidos por media para caber na janela do terminal. A imagem
  inteira (inclusive a borda) e' ajustada para a proporcao **4:3**, a de uma TV/monitor de MSX, seja o
  quadro de 272x228 (MSX1) ou de 512 colunas (MSX2); o menu (F10) alterna para "pixels originais".
  A conta assume uma celula de terminal com ~1:2 (largura:altura).
- No **texto** (SCREEN 0/1/TEXT80) a proporcao nao e' esticada: um caractere nao pode ser alargado sem
  deixar as letras espacadas; ele aparece nitido, 1 caractere por celula.
- Barra de cima: atalhos. Barra de baixo: empresa/jogo da versao, SCREEN, fps, quadro, slot de estado,
  PAUSADO/mudo, porta de controle e a ultima mensagem.

## 3. Teclas

| Tecla | Faz |
|---|---|
| **F10** | abre o **menu** (setas + Enter; Esc fecha) |
| **F11** | abre a **linha de comando** (Esc fecha; setas cima/baixo = historico) |
| **F6** / **F7** | salva / carrega o estado no slot atual (`fwmsx-estado-N.sst`) |
| **F8** / **F9** | slot de estado anterior / proximo (1 a 9) |
| **F12** | captura a tela em PNG (`fwmsx-AAAAMMDD-HHMMSS.png`) |
| qualquer outra | vai para o MSX (letras, numeros, simbolos, Enter, Backspace, Tab, Esc, setas, Home, Ins, Del, **End = SELECT**, **F1-F5**) |
| **Ctrl+C** | CTRL+STOP do MSX (nao fecha a TUI) |
| **Ctrl+letra** | CTRL+letra do MSX |

Para sair: menu (F10) > **Sair**, ou o comando `quit`. As setas, a barra de espaco, **Z** e **X** tambem
valem como joystick da porta A (a mesma regra da janela).

O **menu** tem as mesmas acoes da janela: Reiniciar, Pausar/Retomar, Salvar/Carregar estado, slots,
Capturar tela, Proporcao 4:3/pixels originais, Inserir/Retirar cartucho, Inserir/Ejetar disco A: e B:, Novo disco em branco, Inserir/
Ejetar/Rebobinar fita, Linha de comando, Ajuda e Sair. Os itens que precisam de um nome de arquivo
abrem a linha de comando ja' com o comando escrito (`cart `, `disk A `, `tape `, `newdisk novo.dsk ds35`).

A **linha de comando** entende todos os comandos da ponte de controle (`doc/control-spec.md`: `reset`,
`pause`, `step`, `type`, `peek`, `poke`, `regs`, `cart`, `disk`, `eject`, `tape`, `state`, `screenshot`,
`quit`...) mais `newdisk <arq> <ss525|ds525|ss35|ds35>` e `help`.

## 4. Arquitetura

```
src/tui/tui_app.{h,cpp}   Runner (thread da maquina) + interface FTXUI
src/machine/screenshot.*  captura PNG, compartilhada com a janela
```

- A maquina roda numa **thread propria** a 60 quadros/s (o `Runner`, que implementa `control::Host`).
  A interface so' desenha um **instantaneo** (copiado ~25 vezes por segundo) e manda teclas e comandos.
- **Todo comando** (menu, linha de comando, ponte de controle) passa pelo mesmo
  `control::Commander`, executado na thread da maquina. O menu e' so' um jeito de digitar comandos.
- **Teclas**: um terminal nao manda o evento de soltar a tecla. Cada tecla pressionada vale como
  "segura" por 6 quadros; o auto-repeat do teclado renova o prazo, entao segurar uma tecla funciona.
  Shift e' acrescentado quando o caractere precisa dele (`Machine::KeysForChar`).
- Audio, ponte de controle (`--ctl-port`, `fwmsx.port`) e `quit` pela ponte funcionam como na janela.

## 5. Testes

- Teste manual com o executavel real (a TUI precisa de um terminal; o ambiente de teste so' tem pipes):
  `fwMSXc.exe --tui --ctl-port 0` com um cliente na ponte -- `type "print 6*7\n"` aparece na saida
  renderizada ("print 6*7" e " 42"), os graficos geram meio-blocos, o quadro avanca a 60/s, o
  `quit` encerra a TUI (codigo 0) e o `fwmsx.port` e' apagado.
- **Nao ha teste automatizado da TUI** (a interface depende de um terminal); o `Commander` e a ponte,
  que ela usa, sao cobertos por `controltest`.

## 6. O que falta

- **Outra TUI, a de menus (pedido do usuario):** uma TUI que NAO embute a maquina, so' coloca em menus os
  comandos da CLI/console e controla o emulador em JANELA pela ponte (inicia o `fwMSX.exe`, como o
  `emu start`). A TUI de hoje (terminal embutido) continua, e e' otima para testes rapidos de programacao;
  falta decidir o nome do modo de menus (hoje `--tui` e' o terminal embutido).
- Teclado/menu so' foram exercitados por mim via ponte e saida renderizada; a digitacao num terminal de
  verdade (TAB, setas, F-keys por terminal) depende de teste seu.
- Mouse (cliques no menu), redimensionar com graficos mais finos (braille), ajuste de proporcao.
- Configuracao de slots/disco pela TUI (hoje so' pelas opcoes de linha de comando ou comandos).
- Ferramentas integradas (depurador, montador, banco de ROMs) dentro da TUI.
