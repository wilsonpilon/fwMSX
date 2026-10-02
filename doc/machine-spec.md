# Maquina MSX1 completa + janela com teclado do host (`fwmsx --msx`) -- especificacao (documento vivo)

> Mesmo espirito de `doc/z80-core-spec.md`, `doc/memory-map-spec.md`,
> `doc/vdp-spec.md`, `doc/ppi-spec.md` e `doc/psg-spec.md`: o que foi feito,
> as decisoes e o que falta, para retomar do ponto exato onde parou.

Estado: **concluido em 2026-10-02** para MSX1 (SCREEN 0/1/2 com sprites):
`fwmsx --msx` abre uma janela com a BIOS real rodando em tempo real, teclado
do host e cartucho opcional. Ver a secao 5 para o que falta.

## 1. Uso

```
fwmsx --msx [--bios <arq>] [--cart <arq> [mapper]]      # abre a janela
fwmsx --msx --frames N --shot tela.ppm [--keys "texto"]  # sem janela
```

- `--bios`: BIOS MSX1. Padrao: `resource/fMSX/ROMs/MSX.ROM`, procurada a
  partir do diretorio de trabalho, do diretorio do executavel e dos pais
  dele (`dist/` fica dentro do repositorio).
- `--cart <arq>`: cartucho no **slot 1**. Sem mapper: ROM plana de ate' 32KB
  (posta em `4000h`; a BIOS acha o cabecalho `AB` e chama o INIT). Com
  mapper (`gen8 gen16 konami5 konami4 ascii8 ascii16`): MegaROM (ver
  `doc/memory-map-spec.md`).
- `--frames N` sem `--shot`/`--keys`: abre a janela e fecha sozinha apos N
  quadros (para validar a janela sem interacao).
- `--shot` / `--keys`: **sem janela** -- roda os quadros, digita o texto
  (letras, numeros, espaco; `|` = ENTER) e salva a tela como PPM. Serve
  para CI e para conferir a imagem sem abrir nada.

## 2. Arquitetura

```
src/machine/
├── machine.{h,cpp}        C++ -- Machine: BIOS + slots + VDP + PPI + PSG + Z80 em quadros, SEM janela
├── cli.{h,cpp}            C++ -- "fwmsx --msx": argumentos, busca da BIOS, modo sem janela
└── gui/
    ├── emu_window.{h,cpp}     C++ -- janela (GLFW + ImGui + textura OpenGL), teclado do host
    └── emu_window_stub.cpp    stub quando FWMSX_MSXDISK_GUI=OFF (mesma ideia do msxdisk)
```

**Decisao central: toda a logica de maquina fica em `Machine`, nao na janela.**
A janela so' chama `RunFrame()`, `RenderFrame()` e `KeyDown()/KeyUp()`. Foi
isso que permitiu testar a maquina inteira sem OpenGL (`machinetest`) e
gerar a imagem do modo `--shot`. `Machine` monta as pecas reutilizando
`BuildZ80DebugShellStartup({"--slots", bios, "--vdp", "--ppi", "--psg"})` --
o mesmo layout de slots do depurador (RAM de 64KB em `3:2`, regras de
subslot do MSX1), em vez de uma segunda montagem que poderia divergir.

## 3. Temporizacao

- 1 quadro = 262 linhas x 228 ciclos = **59736 ciclos de Z80** (NTSC,
  59.92 Hz). `Machine::RunFrame()` executa um quadro avancando VDP e PSG
  instrucao a instrucao, como `DriveVdp()`/`DrivePsg()` do depurador, e
  entrega a interrupcao de VBlank ao Z80.
- A janela acumula tempo real (`glfwGetTime`) e roda quantos quadros ja'
  passaram (no maximo 4 por volta; se atrasar mais que isso, descarta em
  vez de acelerar). O V-sync do monitor (60/144 Hz) so' dita quando redesenha.
- **Reset** (menu): CPU, VDP, PPI (teclas pressionadas continuam) e PSG.

## 4. Teclado do host

Mapeamento **posicional** (layout US) -- a tecla fisica no lugar da tecla
MSX: letras, numeros e simbolos (`- = \ [ ] ; ' \` , . /`); Shift = SHIFT,
Ctrl = CTRL, **Alt esquerdo = GRAPH, Alt direito = CODE**, Caps Lock = CAPS;
Enter, Espaco, Backspace = BS, Tab, Esc, setas, Home, Ins, Del; **End =
SELECT, Pause = STOP**; F1-F5; teclado numerico = teclado numerico do MSX.
**F11** alterna tela cheia (nao e' tecla MSX).

- Auto-repeat do host e' ignorado (o MSX faz o proprio).
- **Toque rapido:** a BIOS le o teclado uma vez por quadro, entao um
  press+release no mesmo lote de eventos teria sumido. O press vale na hora
  e o release so' depois de um quadro inteiro.
- Perder o foco da janela solta todas as teclas (nada fica "preso").
- Os callbacks do emulador sao instalados **antes** do ImGui, que encadeia
  o callback ja' existente -- o menu continua funcionando.

## 5. Limites conhecidos e o que falta

- **Audio:** o PSG toca ao vivo (ver `doc/audio-spec.md`); `--mute` desliga.
  Click de tecla/cassete (PPI), SCC e FM ainda sem som.
- **Modos de tela:** so' SCREEN 0/1/2 (+ sprites de modo 1). SCREEN 3
  (multicolor) e MSX2 (5-8, sprites de modo 2) caem no fallback de cor de
  fundo -- Fase 4 do VDP.
- **Sem joystick/mouse** (R14/R15 do PSG) e sem disco (FDC): jogos que
  exigem joystick ainda nao jogam; so' teclado.
- **Sem teclado de layout nao-US:** o mapeamento e' posicional; `Shift+2`
  sai `@` como no MSX, mas acentos/cedilha do ABNT2 nao tem tecla.
- **A janela interativa so' foi validada em abertura/fechamento** (contexto
  GL, ImGui, textura, loop em tempo real, `--frames`): a digitacao pelo
  teclado do host (GLFW -> `Machine`) nao tem teste automatico, so' a
  `Machine::KeyDown/KeyUp` por baixo dela (`machinetest`).
- Nao ha' selecao de cartucho/BIOS pela janela (so' pela linha de comando)
  nem save-state.
