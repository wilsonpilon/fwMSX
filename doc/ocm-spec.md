# Maquina OCM (1chipMSX / MSX++) -- especificacao (documento vivo)

> Introduzida na **1.43.25** (2026-10-24). Mesmo espirito dos outros documentos vivos: o que foi feito, as decisoes e o
> que falta.

## 1. Pedido e fonte

Pedido do usuario: "sera que com este material voce consegue emular o OCM como uma das maquinas disponiveis? MSX1, MSX2,
MSX2+ e OCM?" -- com o pacote do KdL (OCM-PLD v3.9.2 e SDBIOS) baixado em `third-party/ocm` (fora do git). Resposta: sim, como uma
MSX2+ com o mapa de slots e os dispositivos do OCM. A fonte e' o proprio `emsx_top.vhd` (mapa de slots e decodificacao de E/S),
`swioports.vhd`, `megasd.v` e as ROMs do pacote (`SDBIOS/make/roms`, copiadas para `dist/roms/ocm/roms`, fora do git).

O OCM e' um FPGA; o que o fwMSX emula e' o **comportamento visto pelo programa**: o mapa de slots, as ROMs, a ROM de Kanji, as
portas comutadas e o MegaSD. A velocidade da CPU (3,58 / 5,37 / 10,7 MHz), o MIDI, o UART, o Wi-Fi, o LED e a configuracao
de DIP-SW ficam de fora.

## 2. O mapa de slots (do `emsx_top.vhd`)

| Slot | Conteudo no OCM | No fwMSX |
|---|---|---|
| 0-0 | BIOS MSX2+ de 32 KB (`a1wsxbsl.rom` teclado BR, `a1wsxyen.rom`, `a1wsxwst.rom`; ou a do MSX turbo R) | `Rom` 32 KB, pagina 0 |
| 0-1 | mapper extra de ate' 4096 KB | **nao** (use `--ram2`; ver `msx2-spec.md`) |
| 0-2 | MSX-Music (`msx2pmus.rom`, 16 KB) | `Rom` 16 KB, pagina 1 |
| 0-3 | ROM opcional de 16 KB | vazio |
| 1 e 2 | ESE-SCC de 2 MB (MegaRAM / SCC) | vazios (o 1 aceita `--cart`); o SCC-I como ESE-RAM nao e' emulado |
| 3-0 | mapper interno de 4096 KB | `Mapper` de `mapper_kb` (4096 KB por padrao) |
| 3-1 | sub-ROM (`2pextrtc.rom`, 16 KB) + o Kanji BASIC (`knmsxppl.rom`, 32 KB) | `Rom` de 48 KB em 0000h-BFFFh |
| 3-2 | ESE-RAM de 1 MB com o MegaSD e o Nextor (`nextsd15.rom`) | `MegaSd` (ver `hd-spec.md` 7b), sem cartao se nao houver `--sd` |
| 3-3 | XBASIC (`xbasic21.rom`, 16 KB) | `Rom` 16 KB, pagina 1 |

E/S: VDP V9958 (98h-9Bh), PSG (A0h-A3h), PPI (A8h-ABh), OPLL (7Ch/7Dh, sempre presente no fwMSX), RTC (B4h/B5h), mapper (FCh-FFh),
**Kanji** (D8h-DBh) e as **portas comutadas** (40h-4Fh).

## 3. O que foi feito (src/ocm, nas 4 linguagens)

| Arquivo | Linguagem | O que faz |
|---|---|---|
| `core/kanji_core.{h,c}` | C | O endereco da ROM de Kanji: 17 bits = (6 bits altos << 11) \| (6 bits baixos << 5) \| contador de linha (5 bits) |
| `fortran/ocm_swio.f90` | Fortran | As portas comutadas: a 40h devolve o complemento do ID escolhido (D4h = OCM/MSX++, 08h = Panasonic; outros FFh); regs 41h-4Fh |
| `asm/ocm_compose.asm` | NASM (Win64/SysV) | `ocm_copy` (REP MOVSB): monta a imagem de 48 KB da celula 3-1 (sub-ROM + Kanji BASIC) |
| `cpp/ocm_devices.{h,cpp}` | C++ | `KanjiDevice` (D8h-DBh, JIS1 e JIS2 de 128 KB cada) e `SwitchedIo` (40h-4Fh) |

No `machine`: `MachineConfig::ocm/ocm_dir/ocm_main/ocm_logo`, `DefaultLayout` com o ramo do OCM, `ConfigureOcm`, `LoadRomCell` com sub-ROM de 16 KB +
ROM de 32 KB, `Machine::kanji()/switched_io()`. `--ocm [--ocm-main a1wsxyen.rom] [--ocm-logo knsonyv2.rom]` na linha de comando; item *Maquina > Modelo >
OCM / MSX++* na janela. Precisa das ROMs de `<pasta de ROMs>/ocm/roms`: `ConfigureOcm` diz qual falta.

## 4. Achados

- **O logo do Kanji BASIC espera pela colisao de sprites.** A ROM `kn*.rom` anima o logo com um efeito de rastreio: poe dois sprites sobrepostos,
  espera o bit de colisao do S#0 (bit 5) e so' entao muda R#26/R#27 (rolagem horizontal do V9958), linha a linha. O VDP do fwMSX so' avaliava a colisao
  **uma vez por quadro** (linha 192, como o fMSX), entao cada espera custava um quadro e o boot levava ~4000 quadros (70 s). Agora a colisao e' avaliada **por
  linha** (`vdp_sprites_collision_on_line`, no passo de cada linha visivel) e o OCM sobe em ~600 quadros (10 s, o logo inclusive).
- **As portas sem dispositivo leem FFh** (1.43.23), que o Nextor tambem exige.
- O teclado do `a1wsxbsl.rom` e' o **brasileiro**: o `:` e outras teclas nao ficam onde o layout internacional do teste as poe; o teste digita pela tabela
  de teclas da propria maquina (`Machine::KeysFor`).

## 5. Testes

`msx2test` 8e: o endereco do Kanji (D8h/D9h, contador de linha que da' a volta em 32 bytes, nivel 2 em DAh/DBh, sem a ROM do nivel 2 FFh), as portas comutadas
(complemento do ID D4h = 2Bh, 08h = F7h, outros FFh), `ConfigureOcm` (com e sem as ROMs) e o **boot real**: "MSX BASIC version 3.0", "Copyright 1988 by Microsoft",
o Nextor do MegaSD assume o disco e `OUT 64,212 / PRINT INP(64)` = 43 (pulado sem as ROMs). `vdp2test`: a colisao por linha.

## 6. O que falta

- A velocidade da CPU (turbo 5,37 e 10,7 MHz) e os "DIP-SW virtuais" da porta 42h (hoje leem 0).
- O ESE-SCC de 2 MB nos slots 1 e 2 (MegaRAM / SCC-I), o mapper extra em 0-1 e o slot 0-3 opcional.
- A BIOS do MSX turbo R (`msxtr*.rom`) e as variantes de teclado alem do `a1wsxbsl/yen/wst` ja' escolhidas por `--ocm-main`.
- MIDI, UART, Wi-Fi (ESP8266) e o modo C-BIOS do OCM.
- O estado salvo (`state save`) da maquina OCM (Kanji, portas comutadas e MegaSD nao entram).
- Como o usuario baixa o pacote do KdL e a opcao de download "INTERNAL" (o link e' enviado por e-mail pelo KdL; ver `OUTLINE.md`).
