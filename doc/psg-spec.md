# PSG AY-3-8910 (portas A0h-A2h) -- especificacao (documento vivo)

> Mesmo espirito de `doc/z80-core-spec.md`, `doc/memory-map-spec.md`,
> `doc/vdp-spec.md` e `doc/ppi-spec.md`: o que foi feito, as decisoes e o
> que falta, para retomar do ponto exato onde parou.

Estado: **Fase 1 concluida em 2026-10-02** (motor de som + portas +
gravacao em WAV + comandos de depuracao). A BIOS real programa o PSG e o
`BEEP` do BASIC sai como onda de verdade. **Falta**: saida de audio ao
vivo (placa de som) e joystick em R14/R15 -- ver a secao 5.

## 1. Objetivo

O PSG e' o chip de som do MSX. Tres canais de tom (onda quadrada), um
gerador de ruido e um gerador de envelope, controlados por 16 registradores
acessados por tres portas:

| Porta | Papel |
|-------|-------|
| `A0h` (escrita) | latch: numero do registrador (so' os 4 bits baixos) |
| `A1h` (escrita) | escreve no registrador selecionado |
| `A2h` (leitura) | le o registrador selecionado (R14 = joystick, R15 = 4 bits altos) |

Registradores: R0-R5 periodo de tom (12 bits por canal), R6 periodo de
ruido (5 bits), R7 mixer (bits 0-2 desligam o tom de A/B/C, bits 3-5
desligam o ruido; bits 6-7 = direcao das portas de I/O), R8-R10 volume (bit
4 = usar envelope), R11-R12 periodo do envelope, R13 forma do envelope,
R14-R15 portas de I/O (joystick).

## 2. Arquitetura (C, Fortran e C++)

```
src/psg/
├── core/psg_state.{h,c}        C -- registradores, geradores, mixer e geracao de amostras
├── fortran/volume_table.f90    Fortran -- tabela de volume logaritmica (calculada por formula)
└── cpp/psg_device.h            C++ -- z80::IBus de porta (A0h-A2h) + acumulo das amostras
    cpp/wav_writer.{h,cpp}      C++ -- grava WAV mono 16 bits (par do ppm_writer do VDP)
```

Sem Assembly neste modulo: nao ha' laco quente que justifique (o motor
roda 1 tick a cada 16 ciclos de Z80). Fica registrado em vez de forcado.

**Decisao central: gerar PCM, nao repassar freq/volume.** O fMSX nao gera
amostras: `Sync8910()` calcula freq/volume por canal e chama `Sound()`
(sintetizador de alto nivel do EMULib). O fwMSX gera amostras de verdade,
avancadas por ciclos de Z80 -- o som vira *testavel sem placa de audio*
(conta-se ciclos de onda, mede-se pico) e a futura saida ao vivo so'
precisa consumir o mesmo buffer. Por isso a adaptacao do fMSX se limita as
mascaras de registrador e ao protocolo das portas (ver
`LICENSE-THIRD-PARTY.md`); os geradores sao originais.

## 3. Temporizacao

- Clock do PSG = clock do Z80 / 2 = 1.789772 MHz.
- **1 tick = 8 ciclos de PSG = 16 ciclos de Z80** (`PSG_CYCLES_PER_TICK`).
- Tom: o canal alterna a cada `periodo` ticks (periodo 0 conta como 1) --
  onda completa = 16 x periodo ciclos de PSG, ou seja
  `f = 1789772 / (16 x periodo)`. Periodo 254 -> 440.4 Hz (teste).
- Ruido: LFSR de 17 bits (realimentacao bit0 XOR bit3), desloca a cada
  `2 x periodo` ticks.
- Envelope: 16 passos por ciclo, 1 passo a cada `2 x periodo` ticks
  (`f = fclock / (256 x periodo)`).
- Saida: filtro de caixa (media dos ticks dentro de cada periodo de
  amostra) a 44100 Hz, mono, 16 bits. Nivel 15 = 10922 (= 32767/3), entao
  os tres canais somados nunca estouram.

A sessao de depuracao avanca o PSG apos cada instrucao (`DrivePsg()`, par
de `DriveVdp()`), com os mesmos ciclos que o Z80 acabou de consumir. Uma
escrita nos registradores vale a partir da instrucao seguinte (resolucao
de instrucao, nao de ciclo).

## 4. Detalhes de fidelidade

- **Mascaras** (de `Write8910()`): R1/R3/R5 4 bits, R6 5 bits, R8-R10 5
  bits, R13 4 bits; R7/R11/R12/R14/R15 8 bits. Registrador > 15 e'
  ignorado.
- **Reset** (`RegInit[]`): R7 = `FDh`, R14 = `FFh`, o resto 0.
- **Leitura** (de `InZ80()` 0xA2): R14 devolve o joystick (`7Fh` sem nada pressionado), R15
  so' os 4 bits altos, os demais como estao.
- **Escrever em R13 sempre reinicia o envelope.** Formas 0-3 -> como 9
  (decay e silencio), 4-7 -> como 15 (attack e silencio), 8-15 como o
  datasheet. As 12 formas distintas tem teste.
- Mixer com tom e ruido **ambos desligados** deixa o canal em nivel alto
  constante (DC), como o hardware -- por isso volume > 0 nessa situacao
  nao e' silencio. Silencio e' volume 0.

## 5. Comandos de depuracao e o que falta

`fwmsx --z80dbg --slots [<rom>] --psg` (exige `--slots`, como `--vdp`/`--ppi`):

| Comando | Efeito |
|---------|--------|
| `psgregs` | R0-R15, tom/ruido/volume de cada canal, estado da gravacao |
| `psgpoke <reg> <byte>` | escreve num registrador |
| `psgrec start\|stop\|clear` | liga/desliga/descarta a gravacao (acumula enquanto `run`/`step` rodam o Z80) |
| `psgrec save <arq.wav>` | salva a gravacao como WAV mono 16 bits, 44100 Hz |

Exemplo: `--z80dbg --slots MSX.ROM --vdp --ppi --psg`, `psgrec start`, rodar
a BIOS, `keydown b` ... `keydown enter` (digitar `BEEP`), `psgrec save
beep.wav`.

**Teste de aceite com a BIOS real** (`psgtest`, secao 9 do teste): depois
de 100M de ciclos a BIOS programou o mixer (R7 = `B8h`) e zerou os
volumes; digitar `BEEP` + ENTER no BASIC programa o canal A em **1316 Hz,
volume 7** (periodo 85) e o audio gravado tem pico 683 (= 10922/16, nivel
7). A duracao do BEEP gravado (~20 ms de som, ~51 ciclos de onda) **nao foi
conferida contra um MSX real** -- o teste so' exige que exista uma onda.

**Falta / adiado:**
- ~~**Saida de audio ao vivo**~~ **Feita em 2026-10-02** -- ver
  `doc/audio-spec.md` (`PsgDevice::EnableLive()/TakeLive()` + miniaudio).
- ~~**Joystick em R14/R15**~~ **Feito (v1.10)**: `psg_set_joystick()`; R14 le
  os 6 bits do joystick da porta escolhida por R15 bit 6 (logica invertida,
  bit 6 sempre 1), R15 bits 4/5 cortam as linhas. **Mouse** continua sem
  suporte.
- **Click do PPI** (`AAh` bit 7) e `Drum()`: ainda sem som.
- **SCC** (do mapa de memoria, adiado) passa a ter um destino de audio
  possivel, mas e' um chip a parte.
- Filtro de reamostragem melhor que a caixa (hoje ha' aliasing leve nos
  tons agudos); curva de volume medida em chip real em vez da ideal de
  3 dB/passo.
