# Audio ao vivo -- especificacao (documento vivo)

> Mesmo espirito de `doc/psg-spec.md` e `doc/machine-spec.md`: o que foi
> feito, as decisoes e o que falta, para retomar do ponto exato onde parou.

Estado: **concluido em 2026-10-02**. `fwmsx --msx` toca o PSG pelo
dispositivo de audio padrao do sistema, em tempo real. `--mute` desliga.

## 1. Arquitetura

```
PsgDevice (EnableLive)  --TakeLive() 1x por quadro-->  AudioOutput::Push()
                                                              |
                                              SampleRing (SPSC, sem trava)
                                                              |
                                          callback do miniaudio (thread de audio)
```

- `src/psg/cpp/psg_device.h`: modo **live** -- as amostras geradas ficam num
  buffer que a janela esvazia (`TakeLive`) depois de cada volta do loop. Sem
  consumidor guarda no maximo 1 s (descarta o mais antigo), entao nao cresce
  sem limite. Convive com a gravacao em WAV (`psgrec`).
- `src/audio/ring_buffer.h`: buffer circular de 1 produtor / 1 consumidor,
  sem trava (indices atomicos). O produtor **nunca bloqueia**: o que nao cabe
  e' descartado e contado.
- `src/audio/audio_output.{h,cpp}`: dispositivo via **miniaudio** (um unico
  header, dominio publico/MIT-0, baixado por FetchContent; so' o dispositivo
  de saida e' compilado). Mono, 16 bits, 44100 Hz, periodos de ~10 ms.
  `audio_output_stub.cpp` entra com `-DFWMSX_AUDIO=OFF` (o emulador roda mudo).

## 2. Sincronia

A emulacao anda pelo relogio da janela (59.92 Hz); o dispositivo puxa pelo
relogio da placa de som. Os dois divergem um pouco, entao:

- **Pre-enchimento:** o dispositivo so' comeca a tocar com 1470 amostras
  (~33 ms) no buffer; se esvaziar (underrun) completa com silencio e espera
  encher de novo -- evita estalos na partida.
- **Buffer de ~370 ms** (16384 amostras): folga para o jitter. Se a emulacao
  adiantar demais, o excedente e' descartado em vez de acumular latencia.
- **Pausar** e **Reset** (menu) fazem `Flush()` -- nao arrasta som velho.
- Volume (0-1, padrao 0.7) e "Mudo" no menu **Som**, aplicados no callback.

O PSG soma os 3 canais ate' 32766 (fundo de escala); o ganho padrao de 0.7
evita que jogos com os 3 canais no maximo soem estourados.

## 3. Verificacao (`audiotest`)

Alem do buffer circular (inclusive 2 milhoes de amostras entre duas threads)
e do modo live do PSG, o teste abre o **dispositivo real**, alimenta 1 s no
ritmo da janela (por tempo decorrido) e confere que ele consome ~44100
amostras/s com quase nenhum underrun/descarte. Sem dispositivo de audio
(CI), esse trecho vira `[SKIP]`. Com `--msx --frames N` a janela imprime no
fim as estatisticas do dispositivo; 600 quadros (10 s) tocaram 439236
amostras, 0 underruns, 0 descartadas.

**Limite honesto:** esses testes provam que o dispositivo *consome* as
amostras no ritmo certo, nao que o som sai *audivel e correto* nas caixas --
isso so' se confirma ouvindo (ex.: digitar `BEEP` na janela e ouvir os
1316 Hz).

## 4. O que falta

- **Click do teclado e do cassete** (PPI, `AAh` bit 7): ainda sem som --
  o `BEEP` e a musica do PSG tocam, o clique de tecla nao.
- **SCC e FM** (cartuchos com som proprio): chips ainda nao existem.
- **Reamostragem melhor** que o filtro de caixa do PSG (aliasing leve em
  tons agudos) e curva de volume medida em chip real.
- **Escolha de dispositivo de audio** (so' o padrao do sistema) e ajuste de
  latencia pela interface.
- Sincronia por **relogio do audio** (em vez do relogio da janela) elimina
  os descartes/underruns raros em sessoes longas; hoje o buffer so' absorve.
