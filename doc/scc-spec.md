# Chip de som SCC (Konami, cartuchos 8KB) -- especificacao (documento vivo)

> Mesmo espirito de `doc/psg-spec.md`, `doc/memory-map-spec.md` e
> `doc/machine-spec.md`: o que foi feito, as decisoes e o que falta, para
> retomar do ponto exato onde parou.

Estado: **concluido em 2026-10-05** (fase do SCC, v1.13.0). O chip gera
amostras PCM de verdade, liga pelo protocolo do cartucho, e sai pela mesma
saida de audio que o PSG. O **F1 Spirit** (Konami5 com SCC) programa os cinco
canais reais; o **Lode Runner + Konami SCC** ainda nao sobe o jogo (ver a
secao 5).

## 1. Objetivo

Varios cartuchos Konami de 128KB trazem um segundo chip de som, o SCC: cinco
canais de onda de 32 amostras, com frequencia de 12 bits e volume de 4 bits.
Sem ele, esses jogos tocam sem a trilha (o F1 Spirit era citado como "sem o
som do SCC" nas versoes anteriores).

## 2. Arquitetura (C, C++, Assembly e Fortran)

| Camada | Arquivo | Papel |
|---|---|---|
| C | `src/scc/core/scc_state.{h,c}` | Registradores e geracao de amostras. Port fiel de `EMULib/SCC.c` do fMSX, sem `Sound()`. |
| Assembly | `src/scc/asm/render_channel.asm` | Soma de um canal em N amostras (dual-ABI Win64/SysV). |
| Fortran | `src/scc/fortran/scc_volume_table.f90` | Tabela de volume linear (nivel 0-15 -> amplitude). |
| C++ | `src/scc/cpp/scc_device.h` | `SccDevice`: protocolo de ativacao do cartucho, saida ao vivo. Header-only. |
| C++ | `src/memmap/cpp/slot_memory_bus.h` | Interface `SlotCartIo` e `AttachCart()`: o barramento entrega as leituras/escritas do slot de cartucho ao dispositivo antes do mapper. |
| C++ | `src/machine/machine.{h,cpp}` | Liga o SCC ao slot 1:0, avanca por ciclos, zera no reset e mistura o PSG com o SCC na saida ao vivo. |

### Por que o chip gera PCM e nao usa um sintetizador

O fMSX repassa frequencia e volume a um sintetizador (`Sound()`). Aqui o
motor gera as amostras, do mesmo jeito que o PSG (ver `doc/psg-spec.md`).
Isso deixa o som testavel sem placa de audio, e a saida ao vivo e a gravacao
passam pelo mesmo caminho.

### Licenca

`scc_state.{h,c}` e o port do `SCC.c` do fMSX continuam sob a licenca
original do fMSX (nao-comercial, aviso ao autor em caso de mudanca), como
ja ocorre com o Z80, o mapa de memoria, o VDP, o PSG e o FDC. Ver
`LICENSE-THIRD-PARTY.md`.

## 3. Protocolo do cartucho (como o fMSX)

- **Konami5**: escrever `3Fh` em `9000h` liga o chip; qualquer outro valor
  desliga. A mesma escrita ainda troca o banco da pagina 2 (ela continua
  indo para o mapper).
- **Gen8**: escrever `3Fh` em qualquer endereco de `8000h-9FFFh` liga o chip,
  e a escrita tambem troca o banco.
- **Cartucho sem mapper** (ROM plana): o chip nao responde.
- **Ligado**: `9800h-98FFh` (mascara `DF00h`) vai para o chip, em leitura e em
  escrita. A escrita nessa faixa **nao** chega ao mapper nem a ROM.
- **Desligado**: `9800h-98FFh` cai na ROM como qualquer outro endereco.

A faixa `B800h-B8FFh` (bit `2000h` setado) seleciona o modo SCC+ dos
registradores, com a mesma logica do fMSX.

## 4. Mapa de registradores e detalhes de fidelidade

Modo generico (`9800h-989Fh`, a partir do fMSX):

| Endereco | Conteudo |
|---|---|
| `9800h-981Fh` / `9820h-983Fh` / `9840h-985Fh` / `9860h-987Fh` | Ondas dos canais 0, 1, 2 e 3 (32 bytes cada, com sinal) |
| `9880h-9889h` | Periodo de 12 bits, em pares (LSB, depois 4 bits altos): canal 0 em `9880h/9881h`, canal 1 em `9882h/9883h`, ... canal 4 em `9888h/9889h` |
| `988Ah-988Eh` | Volume (4 bits) dos canais 0-4 |
| `988Fh` | Mixer: bit N = canal N ligado |

O canal 4 **nao** tem enderecamento proprio no modo generico: ele reusa a onda
do canal 3 (o fMSX grava a mesma onda nas duas posicoes internas), entao
escrever `9860h-987Fh` atualiza os canais 3 e 4 juntos.

Regras copiadas do fMSX, linha a linha: escritas em `B0h-BFh` sao espelhadas
em `A0h-AFh`; leituras acima de `7Fh` no modo generico devolvem `FFh`
(registradores so de escrita); escrita em `E0h+` e descartada.

**Frequencia**: um canal avanca uma amostra da onda a cada `periodo` ciclos
de Z80 (clock 3579545 Hz). Frequencia = clock / (32 * periodo), a mesma
conta do fMSX (`SCC_BASE = clock/32`). Periodo 0 deixa o canal mudo.

**Volume**: linear. Nivel `v` vale `6553*v/15`, de modo que cinco canais no
maximo somam 32510 e nao estouram 16 bits. A amostra e `(onda * nivel) >> 7`.

**Mixagem**: PSG e SCC somados na saida ao vivo, com saturacao em 16 bits
(`Machine::TakeLiveAudio`). O SCC nao tem o filtro de caixa do PSG: gera uma
amostra por periodo de amostragem diretamente, entao frequencias altas podem
sofrer aliasing. E' uma escolha consciente de simplicidade; um filtro fica
como melhoria futura se for audivel.

**Precisao**: a fase da onda usa Q16 e o incremento (truncado) e calculado a
cada chamada. O erro de frequencia fica em torno de 0,002% no pior caso, bem
abaixo do que o teste mede (1%).

## 5. O que falta e o que ficou de fora

- **Lode Runner + Konami SCC** (`resource/fmsxgo/dist/media/`): o cartucho
  nao sobe o jogo. Com o mapper detectado (Konami5) cai no BASIC com
  "Illegal function call in 10"; com ASCII8, ASCII16 e Konami4 fica em tela
  azul; com Gen8 cai no BASIC limpo. Em 1500 quadros nenhum dos mappers chega
  a ligar o SCC. O problema parece ser de boot do cartucho, nao do chip: o
  F1 Spirit, com o mapper detectado como Konami5, liga o SCC normalmente.
  Nao investigado a fundo.
- **Modo SCC+ de cartuchos**: o nucleo tem o modo SCC+ (enderecos com `2000h`),
  mas nenhum cartucho de referencia que temos o usa, e nao ha teste dedicado.
- **FM (OPLL/FMPAC)**: outro chip, fase propria. O `FMPAC` continua sem som.
- **Clock do SCC**: o motor usa o clock do Z80 do NTSC. Um MSX PAL tem
  clock diferente e nao e' modelado.

## 6. Testes

`tests/z80/scc_test.cpp` (CTest `scc_sound`, executavel `scctest`):

1. Protocolo Konami5: escrita em `9800h` com o chip desligado cai na ROM;
   `3Fh` em `9000h` liga o chip e ainda troca o banco; leitura de `9800h`
   devolve a onda escrita; `9880h` le `FFh`; outro valor desliga.
2. Protocolo Gen8: `3Fh` em `8800h` liga; `0` em `9000h` desliga.
3. ROM plana (sem mapper): nao responde ao protocolo.
4. Frequencia: um quadrado de periodo 100 gera `clock/(16*100)` trocas de
   sinal por segundo (+-1%).
5. Nivel: pico de ~6502 no nivel 15; nivel 7 vale ~7/15 do nivel 15.
6. Mixer zerado e volume 0 silenciam o canal.
7. Espelho `B0h` de `A0h`.
8. Teste diferencial: `render_channel.asm` contra `scc_render_channel_ref`
   em 2000 casos aleatorios (passo, nivel, fase, tamanho e onda).

**Verificacao em jogo real** (fora do CTest, por depender de ROM proprietaria):
`fwMSX.exe --msx --cart "resource\fmsxgo\dist\media\F1 Spirit ... .rom"`.
Apos 4000 quadros o jogo deixa os cinco canais ligados (mixer `1Fh`, volumes
e periodos reais) e o estado renderiza 44085 amostras nao-nulas por segundo,
pico 21874.
