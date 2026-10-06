# FM (OPLL) e FM-PAC -- especificacao (documento vivo)

> Mesmo espirito de `doc/scc-spec.md` e `doc/sram-spec.md`: o que foi feito,
> as decisoes e o que falta, para retomar do ponto exato onde parou.

Estado: **em 2026-10-06, nao commitado.** O chip YM2413 (OPLL) esta completo no
modo melodico e no modo ritmo; os comandos de BASIC do MSX-MUSIC funcionam via a
ROM do FM-PAC (`--fmpac`). Ficam de fora: `CALL VOICECOPY` (a ROM nao aceita, ver
secao 7) e a validacao auditiva (so' por testes e por gravacao em WAV).

## 1. O que o usuario ve

- **MSX-MUSIC / FM-PAC**: as portas `7Ch` (registrador) e `7Dh` (dado) estao
  sempre presentes, como no fMSX. Um programa que grava ali toca o chip, mesmo
  sem o cartucho.
- **BASIC do MSX-MUSIC**: com o cartucho FM-PAC, que liga por padrao (`--no-fmpac`
  ou o menu **Cartucho > FM-PAC** desligam; `--fmpac [arquivo]` escolhe o arquivo). A ROM `resource/fMSX/FMPAC.ROM` traz as
  extensoes de BASIC. Comandos verificados (secao 6): `CALL MUSIC`, `PLAY #n`,
  `CALL VOICE`, `CALL PITCH`, `CALL AUDREG`, `CALL PLAY`.
- **Som**: o FM entra na mesma saida de audio que o PSG e o SCC, em metade do ganho.

## 2. Motor (C, `src/fm/core/ym2413_state.c`)

Modelo de 9 canais, cada um com 2 operadores (modulador -> portadora):

- **Fase**: 49716 Hz (clock do OPLL / 72); `f = fnum * 49716 / 2^(19-bloco)`;
  MULT (0..15) pela tabela do chip (0 = meia oitava).
- **Envelope ADSR** por operador: ataque exponencial ate' 1; decaimento ate' SL;
  sustentacao (EGT=1 ou o bit SUS de `20h-28h`) ou liberacao com RR (EGT=0).
  Key-off passa para a liberacao. Taxas R = 4 x valor + KSR.
- **Forma de onda**: seno, ou meia onda positiva (bit WF).
- **Feedback** do modulador (FB 0..7), **KSL**, **TL** do modulador (0,75 dB por
  passo), **volume** da portadora (`30h-38h`, 3 dB por passo), **tremolo** (4,8 dB
  a 3,7 Hz) e **vibrato** (+-7 centesimos a 6,4 Hz).
- **Modo ritmo** (reg `0Eh`, bit 5): os canais 7-9 viram bateria. Cada tecla dispara
  um operador: BD (bit 4) nos dois operadores do canal 7; HH (bit 0) e SD (bit 3)
  no canal 8 (modulador e portadora); TOM (bit 2) e TC (bit 1) no canal 9. Cada
  bateria usa um dos tres timbres prontos 16-18 e o volume de `36h-38h`. Entrar
  ou sair do modo solta as notas dos canais 7-9.
- **Registradores**: `00h-07h` (timbre do usuario), `0Eh` (ritmo), `10h-18h`
  (fnum baixo), `20h-28h` (KEY, SUS, bloco, fnum bit 8), `30h-38h` (timbre e volume).

**Aproximacoes (honestas):** o chip nao e' emulado ciclo a ciclo. As constantes
de tempo do envelope (dobra de taxa a cada 8 passos de R, ataque 0,6 x 2^(R/8)),
a curva de KSL (1,5 / 3 / 6 dB por oitava), o desvio de fase da modulacao
(`MOD_INDEX` = 0,5 ciclo) e o nivel de saida (`YM2413_FULL_SCALE` = 3000 por
canal) sao **estimativas**, nao medidas num OPLL real.

## 3. Timbres prontos e origem dos dados

`src/fm/core/ym2413_patches.h` tem os 15 timbres prontos (1-15) e os tres
conjuntos de bateria (16-18). **Os dados vem da tabela `Synth2413` de
`resource/fMSX/EMULib/YM2413.c`** (fMSX, Marat Fayzullin), cujo uso para estudo e'
autorizado pelo autor. O motor **nao** e' do fMSX: o `YM2413.c` do fMSX e' um
wrapper que toca instrumentos MIDI, nao o chip, e nao foi portado.

O indice 0 e' o timbre do usuario (registradores `00h-07h`). A tabela de
instrumentos do BASIC (0-62, com o 63 programavel) e' da ROM, que escreve esses
registradores; o motor so' ve registradores.

## 4. FM-PAC (mapper `MEMMAP_MAPPER_FMPAC`, `src/memmap/core/slot_state.c`)

Protocolo do fMSX (`MAP_FMPAC`, `MSX.c`), adaptado ao motor de memoria:

- **ROM de 16KB** em `4000h-7FFFh` (obrigatoriamente 16KB; `LoadRom` recusa outro).
- **`7FF7h`** escolhe o par de bancos de 8KB (`V<<1` e `V<<1|1`, mascarado).
- **Chave da SRAM**: `5FFEh` (byte baixo) e `5FFFh` (byte alto). Com `694Dh` a SRAM
  de **8KB** aparece em `4000h-5FFFh`. Outro valor devolve a ROM. `5FFEh`/`5FFFh`
  sao sempre registradores.
- **`7FF6h`** e' ignorado (no fMSX so' guarda bits do OPL).
- **`8000h-BFFFh`** fica vazio (`0FFh`), como o cartucho real.

**Diferencas deliberadas em relacao ao fMSX:** com a SRAM ligada, `6000h-7FFFh`
continua no banco de ROM (o fMSX mostra `0FFh`). Software que le essa faixa com a
SRAM ligada pode ver outro valor; nao foi verificado com um programa real.

**`.sav` da SRAM**: `FMPAC.sav` ao lado da ROM (8192 bytes), gravado com o mesmo
criterio da SRAM de cartucho. Como o FM-PAC padrao esta em `resource/fMSX/`, o
`.sav` cai nessa pasta; `*.sav` esta no `.gitignore`.

## 5. Mistura de audio

`Machine::TakeLiveAudio` soma PSG + SCC + (FM / 2) com saturacao em 16 bits. O FM
soma ate' 9 canais, por isso entra na metade. O ganho foi escolhido por estimativa.

## 6. Verificacao

- **`fmtest`** (CTest `fm_sound`, 24 checagens): tabelas (Fortran), kernel de soma
  em Assembly contra C, timbres prontos, frequencia (fnum 256 no bloco 4 mede
  388,1 Hz contra 388,4), volume, ataque com taxa 0, liberacao, sustentacao,
  modulador, portas `7Ch`/`7Dh`, reset e **modo ritmo** (BD liga o canal 7 e
  gera som; HH solta o bumbo; 20h-28h nao vale no ritmo; sair do ritmo solta tudo).
- **`memmaptest` secao 24** (13 checagens): FM-PAC.
- **`machinetest`** (5 checagens): FM ligado pela maquina gera audio ao vivo; o
  FM-PAC aparece com `AB` e o boot chega ao prompt do BASIC.
- **BASIC com o FM-PAC** (`fwmsx --msx --fmpac --keys ... --text --fmstat`):
  - `CALL MUSIC (0,0,1)` + `PLAY#2,"V15C"` sem erro; o canal 1 toca. Depois do
    `Ok`, a musica continua em segundo plano (fnum e bloco mudam entre 5, 60 e 180
    quadros), entao a interrupcao da ROM funciona.
  - `CALL MUSIC (1,0,1,1,1,1,1)` liga o modo ritmo (`FM ritmo: LIGADO`) e a ROM
    dispara bumbo e chimbal (teclas `17` = 0x11).
  - `CALL VOICE (@00,@03,@43)`, `CALL PITCH (440)`, `CALL AUDREG (24,0)` e
    `CALL PLAY (0,A)` aceitos; `CALL PLAY` devolve o estado do canal.
- **Gravacao**: `--wav arquivo.wav` grava a mistura. Uma melodia de `PLAY#2` de 4
  notas tem pico 1498 e RMS perto de 900 na nota (medido no WAV).

**Limite honesto:** nada disso prova que o som sai *certo* numa caixa. Isso so' se
confirma ouvindo o WAV gerado e comparando com um MSX-MUSIC real.

## 7. O que falta

- **`CALL VOICECOPY`**: a ROM do fMSX responde "Syntax error" para `CALL
  VOICECOPY(@1,A#)` (testado com `A#` e `A%`). Pelos bytes da ROM, essa versao
  nao tem o comando. Consequencia: a voz programavel por BASIC (63) so' pode ser
  feita por `CALL AUDREG`, e mesmo assim nao foi testada com musica.
- **Status e timers**: `7Ch` le `0`. Os jogos e a ROM funcionaram sem o status,
  mas o estado de IRQ do OPLL nao e' emulado.
- **Validacao auditiva**: ouvir o WAV e os programas reais, e ajustar as
  constantes da secao 2 e o ganho da secao 5.
- **ROM do MSX-MUSIC sem FM-PAC**: sem `--fmpac` nao ha BASIC do MSX-MUSIC. As
  portas funcionam, mas os comandos so' existem com o cartucho.
- **FM-PAC**: a diferenca da secao 4 (6000h com SRAM ligada).
- **Save-state** do chip (estado do OPLL e dos envelopes), junto com o da maquina.
- **Custo de CPU** no pior caso nao medido (600 quadros rodam no ritmo real).

## 8. Arquivos

- `src/fm/core/ym2413_state.{c,h}` -- motor (C), incluindo o modo ritmo.
- `src/fm/core/ym2413_patches.h` -- timbres (dados do fMSX), 1-15 e bateria 16-18.
- `src/fm/fortran/ym2413_tables.f90` -- seno e atenuacao em dB (Fortran).
- `src/fm/asm/accumulate.asm` -- soma de canal (Assembly dual-ABI).
- `src/fm/cpp/fm_device.h` -- adaptador `z80::IBus` para `7Ch`/`7Dh`.
- `src/memmap/core/slot_state.{c,h}`, `src/memmap/common/memmap_types.h` -- mapper FM-PAC.
- `src/machine/machine.{h,cpp}`, `src/machine/cli.cpp`, `src/machine/gui/emu_window.cpp` -- integracao.
  No CLI: `--fmpac [arquivo]`, `--text` (tela em texto), `--fmstat` (estado do FM),
  `--wav arquivo` (grava a mistura).
- `tests/z80/fm_test.cpp` (`fmtest`), `tests/z80/memmap_test.cpp` (secao 24),
  `tests/z80/machine_test.cpp` (secoes 3b e 3c).
