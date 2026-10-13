# Splash, icone do aplicativo e configuracoes salvas -- especificacao (documento vivo)

> Introduzido na **1.37.1 "Knightmare: Splash e icone"** (2026-10-13), por pedido do usuario: uma imagem de abertura
> mostrada por alguns segundos antes do emulador entrar, desligavel pelo menu de configuracoes e por linha de comando, e
> um icone para o aplicativo reconhecer no gerenciador de arquivos, na barra de tarefas e no canto da janela.

## 1. Quando o splash aparece (politica)

Funcao pura `splash::ShouldShow(Mode, Flag, enabled_in_settings)` (`src/splash/splash.h`), coberta por `splashtest`:

| `Mode` | sem flag | `--splash` | `--no-splash` |
|---|---|---|---|
| `Window` (janela do emulador) | a opcao salva (padrao: liga) | sim | nao |
| `Terminal` (`--cli`, `--tui`, `--term`) | nao | sim | nao |
| `Command` (`--cmd`, a vir) | nao | **nao** (nunca) | nao |

`--no-splash` vence `--splash` quando vierem os dois. As duas opcoes saem da lista de argumentos antes da analise das
demais (`splash::ExtractFlag`). `emu start` (console e TUI) acrescenta `--no-splash` ao iniciar a janela, a menos que o
usuario passe `--splash`. A opcao `--cmd` ainda nao existe; a politica ja' a trata (nunca mostra).

## 2. O splash na janela (`src/machine/gui/emu_window.cpp`, `RunSplashPhase`)

- A imagem e' `fwMSX-splash.png`, procurada por `splash::FindSplashFile`: ao lado do executavel, em `images/` ao lado dele, na
  pasta de trabalho, em `dist/` e em `images/`. Sem arquivo, o programa abre sem splash (nenhum erro).
- Roda ANTES da maquina comecar (depois de criar a janela e o ImGui): 3 s, imagem inteira com a proporcao certa sobre fundo
  preto, fade nos ultimos 0,4 s; tecla ou clique pula; fechar a janela encerra o programa. As teclas apertadas durante o
  splash sao descartadas (nao vao para o MSX).
- O arquivo e' o original `images/fwMSX-02.png` (2752x1536, 7 MB) reduzido a metade (1376x768, 2,2 MB) por
  `tools/assets/make_assets.py`, que fica em `dist/fwMSX-splash.png` e entra nos pacotes (`build.ps1`, `build.sh`).

## 3. O splash no terminal

So' com `--splash` em `--cli`, `--tui` e `--term`. `splash::RenderHalfBlocks` reduz a imagem (media por caixa) ate caber no
terminal (ate' 160 colunas) e desenha com o caractere U+2580 (meio bloco superior): cor da frente = pixel de cima, cor de
fundo = pixel de baixo, ambos em 24 bits (`ESC[38;2;r;g;bm` / `ESC[48;2;r;g;bm`). Dura 2,5 s e limpa a tela. So' age se a
saida padrao e' um terminal (com pipe ou arquivo nao escreve nada, para nao sujar scripts e testes); no Windows liga o
processamento de sequencias VT e a pagina de codigo UTF-8 durante o desenho.

## 4. Icone do aplicativo

- **Windows**: `src/splash/fwmsx.rc.in` (o CMake gera `build/fwmsx.rc` com o caminho absoluto do `.ico`, porque o windres
  resolve caminhos a partir da pasta de build) embute `images/fwMSX-03.ico` duas vezes: como recurso `1` (o icone que o
  Explorer mostra para o arquivo) e como `GLFW_ICON` (o nome que o GLFW procura para o canto da janela e a barra de
  tarefas). O `fwMSXc.exe` e' o mesmo programa com outro subsistema e herda o icone. Verificado: o Windows associa ao
  `fwMSX.exe` o icone do fwMSX (`ExtractAssociatedIcon`).
- **Linux**: `glfwSetWindowIcon` com os tamanhos 16, 32, 48 e 64 de `src/splash/app_icon_data.h` (gerado de
  `images/fwMSX-03.ico` por `tools/assets/make_assets.py`). Para o gerenciador de arquivos/menu de aplicativos, o pacote leva
  `fwmsx.png` (256x256) e `fwmsx.desktop` (`packaging/linux/`); ver o MANUAL.

## 5. Configuracoes salvas (`src/common/settings.{h,cpp}`)

Arquivo de texto `fwmsx.cfg` (`chave=valor`, `#` comenta) ao lado do executavel: `splash`, `theme`, `font`, `frame`. Arquivo
ausente ou chave desconhecida = padrao; valores fora da faixa viram o limite. A janela le ao abrir e regrava assim que o
usuario muda algo em **Ferramentas > Interface** (o item novo e' **Mostrar splash ao iniciar**); sem mudanca nao cria o
arquivo. Antes desta versao o tema/letra/moldura nao eram lembrados.

## 6. Arquitetura e testes

```
src/splash/png_reader.{h,cpp}   leitor de PNG (8 bits; cinza, paleta, RGB, RGBA; sem entrelacamento) sobre o miniz
src/splash/splash.{h,cpp}       politica, flags, procura do arquivo, meios blocos para o terminal
src/splash/app_icon_data.h      GERADO: icone 16/32/48/64 em RGBA
src/splash/fwmsx.rc.in          recurso do Windows (icone)
src/common/settings.{h,cpp}     fwmsx.cfg
tools/assets/make_assets.py     gera o splash reduzido, o icone embutido e o PNG de 256 do Linux (so' biblioteca padrao)
```

`splashtest` (CTest `splash`, suite 24): a tabela da politica, `ExtractFlag`, o leitor de PNG (ida e volta com um PNG feito pelo
miniz; lixo e PNG truncado recusados), o splash real (1376x768; vira meios blocos com cor de 24 bits e cabe em 30 linhas),
meios blocos de uma imagem sintetica e as configuracoes (padroes, ida e volta, lixo, limites, caminho). Verificado a mao: a janela
leva ~3,9 s a mais com splash do que com `--no-splash`; `splash=0` no `fwmsx.cfg` pula e `--splash` volta a mostrar.

## 7. O que falta

- `--cmd` (um comando e sai) para scripts: ja' tratado pela politica, ainda nao implementado.
- O desenho do splash no terminal depende de um terminal com cor de 24 bits e UTF-8; so' foi exercitado por teste de unidade e
  pelo codigo de renderizacao (nao num terminal de verdade).
- Splash e icone no `msxdisk` (utilitario de discos em janela) e no instalador/atalho do Windows.
- Escolher a duracao do splash (hoje 3 s fixos) e outra imagem pelo menu.
