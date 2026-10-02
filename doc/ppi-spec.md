# PPI i8255 + teclado (portas A8h-ABh) -- especificacao (documento vivo)

> Mesmo espirito de `doc/z80-core-spec.md`, `doc/memory-map-spec.md` e
> `doc/vdp-spec.md`: o que foi feito, as decisoes e o que falta, para
> retomar do ponto exato onde parou.

Estado: **Fase 1 concluida em 2026-10-01** (chip + matriz de teclado +
integracao com o mapa de memoria + comandos de depuracao). **A BIOS real
sobe ate o prompt do MSX BASIC (v1.8.0)** -- ver a secao 5.

## 1. Objetivo

No MSX o i8255 (PPI) faz tres coisas ao mesmo tempo, todas necessarias
para a BIOS real avancar:

| Porta | Papel |
|-------|-------|
| `A8h` (porta A, saida)   | registrador de slot primario (2 bits por pagina) |
| `A9h` (porta B, entrada) | linha da matriz de teclado selecionada |
| `AAh` (porta C, saida)   | bits 0-3: linha do teclado; bit 4: rele do motor do cassete; bit 5: saida do cassete; bit 6: LED de CAPS; bit 7: click |
| `ABh` (controle)         | modo (bit 7=1) ou set/reset de um bit da porta C (bit 7=0) |

Achado da Fase 1 do VDP: a BIOS real ficava presa perto de `0x0C3C`.
O PPI era necessario, mas **nao era a causa** -- ver a secao 5.

## 2. Arquitetura (as quatro linguagens)

```
src/ppi/
├── core/ppi_state.{h,c}   C -- chip i8255 + matriz de teclado (adaptado do fMSX)
├── cpp/ppi_device.h       C++ -- PpiDevice (IBus), liga o chip ao MemorySystem
├── fortran/key_matrix.f90 Fortran -- posicao (linha, mascara) de cada uma das 87 teclas
└── asm/key_count.asm      Assembly -- ppi_pressed_count(), dual-ABI Win64/SysV
```

- **C** (`PpiState`): `ppi_write()`/`ppi_read()` sao `Write8255()`/
  `Read8255()` de `EMULib/I8255.c` (set/reset de bit do controle, pinos de
  saida so' dirigidos em modo saida) + o `PPI.Rin[1]=KeyState[Rout[2]&0x0F]`
  de `InZ80()` do `MSX.c`. Reset do chip = `Reset8255()` (controle `9Bh`,
  tudo em entrada).
- **C++** (`ppi::PpiDevice`): dispositivo so' de porta, registrado com
  `CompositeBus::RegisterPortRange(0xA8, 0xAB, ...)`. O slot primario muda
  quando o **pino** de saida da porta A muda (`if(PPI.Rout[0]!=PSLReg)
  PSlot(...)` do fMSX), nao a cada escrita crua em `A8h` -- consequencia
  fiel ao hardware: **antes de a BIOS escrever `82h` em `ABh`, a porta A
  esta' em modo entrada e `OUT (A8h)` nao troca slot nenhum.**
- **Fortran** (`key_matrix.f90`): as coordenadas das teclas sao calculadas
  por formula onde ha' padrao (letras, digitos) e por faixa no resto --
  mesma ideia das tabelas de flag do Z80 e da paleta do VDP. Os ids seguem
  a ordem de `kKeyNames[]` em `ppi_state.c` (a-z, 0-9, pontuacao, shift/
  ctrl/graph/caps/code, f1-f5, esc/tab/stop/bs/select/enter, space/home/
  ins/del/setas, pad0-pad9 e operadores do teclado numerico).
- **Assembly** (`key_count.asm`): conta as teclas pressionadas (bits em 0
  das linhas 0-10) com `POPCNT`. Primeiro uso real e' o comando `keys` do
  depurador. Mesma tecnica dual-ABI de `src/z80/asm/block_ops.asm`.

## 3. Depurador

`fwmsx --z80dbg --slots [rom] [--vdp] --ppi` (a flag `--ppi` e'
independente de posicao, exige `--slots`, e e' ignorada com aviso caso
contrario -- mesmo padrao de `--vdp`):

- `ppiregs` -- modo de cada porta, valores/pinos de A/B/C, slot primario
  decodificado por pagina, linha de teclado, motor/cassete/LED/click.
- `keys` -- matriz em binario (linhas 0-10) + quantas teclas estao
  pressionadas (contagem vinda do Assembly) e quais.
- `keydown <tecla>...` / `keyup <tecla>...|all` -- pressiona/solta teclas
  pelo nome (`a`..`z`, `0`..`9`, `shift`, `ctrl`, `enter`, `space`, `f1`..`f5`,
  `esc`, `left`/`up`/`down`/`right`, `pad0`..`pad9`, ...). Nao diferencia
  maiusculas. Teclas "shiftadas" (ex.: `!`) sao `keydown shift 1`.
- `reset` tambem reseta o PPI (volta ao slot primario 0, tudo em entrada);
  **teclas pressionadas continuam pressionadas** (o teclado e' o mundo
  externo).

## 4. Layout MSX1 padrao com `--ppi` + BIOS

Com `--slots <rom> --ppi` o startup monta, como o fMSX: BIOS em `0:0` e
**RAM de 64KB no slot `3:2`** (`MemMap[3][2]`), com as **regras de subslot
do MSX1** ligadas (`SlotState.msx1_subslot_rules`, `SSlot()` do fMSX): os
slots 0, 1 e 2 nao tem subslot -- escrever em `FFFFh` neles e' forcado a 0,
entao a leitura continua `0xFF` e a BIOS ve o slot como nao expandido; so'
o slot 3 e' expandido. Sem `--ppi` (ou sem ROM) nada muda: o mapa de
memoria continua aceitando subslot livremente (decisao da Fase 1 do mapa de
memoria, que os 102 testes de `memmaptest` dependem).

## 5. BIOS real: boot completo ate o prompt do MSX BASIC (v1.8.0)

`fwmsx --z80dbg --slots resource/fMSX/ROMs/MSX.ROM --vdp --ppi` e `run
100000000` agora leva a BIOS MSX1 real ate o prompt, com a tela de
abertura na VRAM (SCREEN 0, exportavel com `vdpshot`):

```
MSX BASIC version 1.0
Copyright 1983 by Microsoft
28815 Bytes free
Ok
```

Teclas de `keydown`/`keyup` chegam ao BASIC (a BIOS le a matriz pelo PPI
durante a interrupcao de VBlank e escreve o caractere na tela).

**A causa raiz da "BIOS presa" (v1.4.0-v1.7.0) NAO era o PPI.** A saga,
para nao repetir o erro de diagnostico:

1. Fase 1 do VDP: "presa em `0x0C3C`, deve ser o PPI" -- hipotese plausivel,
   mas incompleta.
2. v1.7.0: com PPI, a BIOS programa o chip mas reiniciava o boot em ciclo;
   somando RAM em `3:2` e as regras de subslot, ficava na varredura de RAM
   (`0x0305`-`0x0331`). Mais uma hipotese incompleta.
3. **Causa real: `Z80Cpu` nao chamava `z80_reset()` na construcao.** O
   estado inicial era zerado -- incluindo `g_z80_zs_table`/`g_z80_pzs_table`
   (tabelas de flag Sinal/Zero/Paridade, preenchidas por `z80_tables_init()`
   dentro de `z80_reset()`). Quem nunca digitava `reset` rodava com flags de
   `AND`/`OR`/`XOR`/`CP`/`INC`/`DEC` erradas (Z e P nunca setavam). O fio
   que levou ate' la': a BIOS fazia `LD A,B / AND A / JR Z,...` com `B=0` e
   o `JR Z` nao desviava -- `AND A` com A=0 devolvia `F=10h` em vez de
   `54h`. Os 168 testes do `z80test` nao pegaram porque todos chamam
   `cpu.reset()` antes. **Correcao:** o construtor de `Z80Cpu` agora chama
   `z80_reset()`; teste de regressao em `tests/z80/debug_session_test.cpp`
   (CPU sem `reset` explicito).

O que cada peca de fato era necessaria para o boot:

- PPI: necessario (a BIOS escreve `82h` em `ABh` logo no inicio).
- RAM de 64KB no slot `3:2`: necessario (sem RAM a BIOS nao monta a area de
  trabalho).
- Regras de subslot do MSX1 (`msx1_subslot_rules`): **testado -- a BIOS
  sobe com e sem elas**. Mantidas por fidelidade ao `SSlot()` do fMSX.

Teste de aceite: `tests/z80/ppi_test.cpp`, teste 9 (deixou de ser
informativo): 100M de ciclos, confere VBlank habilitado (R#1 bit 5) e IFF1
ligado, a abertura do MSX BASIC na VRAM, e que `keydown z` escreve `z` na
tela.

## 6. Decisoes e simplificacoes

- **Sem som** de click/rele (`PPIOut()` do fMSX chama `Drum()`): o estado
  dos bits 4-7 da porta C e' exposto em `ppiregs`, nada mais.
- **Sem teclado do host ainda**: as teclas vem so' de `keydown`/`keyup`.
  Ligar o teclado real (GLFW) e' trabalho da fase de janela (VDP Fase 4).
- **Sem joystick/PSG** (portas `A0h`-`A2h`): fora de escopo; a leitura de
  joystick pela BIOS depende do PSG.
- Teclado **internacional** (matriz de 11 linhas): teclas dedicadas de
  layouts japones/europeu (acentos) nao tem nome.
- `ppi_pressed_count()` usa `POPCNT` (qualquer x86-64 desde 2008).

## 7. Proximos passos

1. **PSG (`A0h`-`A2h`)**: a BIOS ja' sobe sem ele (leituras de porta sem
   dispositivo devolvem 0), mas joystick, som e o resto do teclado de
   cassete dependem dele.
2. **Janela real + teclado do host** (VDP Fase 4): o prompt ja' e'
   renderizado; falta mostrar numa janela e mapear o teclado do host para
   `ppi_key_set()`.
3. Cartuchos: carregar uma ROM de jogo em `1:0`/`2:0` (o `loadrom` ja'
   existe) e deixar a BIOS achar e iniciar o cartucho.
