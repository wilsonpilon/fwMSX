# Layout de slots e configuracao da maquina -- especificacao (documento vivo)

> Mesmo espirito dos outros documentos vivos: o que foi feito, as decisoes e o
> que falta.

Estado: **em 2026-10-06, nao commitado.** A maquina monta o layout de slots a
partir de uma tabela de 16 celulas (primario:secundario). Cada celula diz o que
esta ligado nela. O menu **Maquina > Configuracao de slots...** edita a tabela, e
o padrao reproduz exatamente o que a maquina ja fazia.

## 1. Modelo

`src/machine/machine.h`:

- `SlotKind`: `Empty`, `Rom`, `SubRom`, `Ram`, `Mapper`, `Disk`, `FmPac`.
- `SlotItem` (uma celula):
  - `path`: ROM principal (BIOS, BASIC, cartucho), DISK.ROM ou FMPAC.ROM.
  - `path2`: segunda ROM de 16KB que vai para a pagina 1 (ex.: BASIC, com a BIOS na
    pagina 0), ou, numa celula `Disk`, a sub-ROM do MSX2 na pagina 0.
  - `page`: para ROM de 16KB ou 32KB, pagina 0 (0000h) ou 1 (4000h).
  - `mapper`: mapper de cartucho (`NONE` = detecta pelo tamanho, como antes).
  - `size_kb`: RAM 16, 32 ou 64; mapper 64, 128, 256, 512 ou 1024.
- `SlotLayout`: `cell[primario][secundario]`.
- `MachineConfig::layout` e `layout_set`. Enquanto `layout_set` for falso, o layout
  sai dos campos antigos (`bios_path`, `cart_path`, `disk_*`, `ext_rom_path`,
  `fmpac_rom_path`) pela regra de `DefaultLayout()`. Depois de editado, vale o
  `layout`.

## 2. Regras de montagem

- **BIOS obrigatoria em 0:0** (o Z80 comeca la'). `ValidateLayout()` recusa o
  layout sem ela, com a razao.
- **BIOS de 32KB**: ocupa a pagina 0 (0000h-3FFFh, a BIOS) e a pagina 1
  (4000h-7FFFh, o BASIC) da mesma celula. Um arquivo de 32KB em `page = 0`.
- **BIOS e BASIC em arquivos separados**: `path` (16KB) vai para a pagina 0 e
  `path2` (16KB) para a pagina 1 da mesma celula. Cada arquivo tem de ter 16KB.
- **Cartucho**: ROM de 16KB em `page = 1` (4000h), como antes; ROM de 32KB em
  `page = 1` ocupa 4000h-BFFFh; acima de 32KB e' MegaROM (mapper detectado ou o
  escolhido). Uma ROM com a assinatura `AB` cujo INIT aponta para 8000h+ vai para
  8000h, como antes.
- **RAM**: 16, 32 ou 64 KB numa celula. Quatro bancos de 64KB no slot 2 sao quatro
  celulas (2:0 a 2:3), cada uma com RAM de 64KB. Isso exige a regra de subslot do
  MSX2; `Create()` liga a regra sozinho quando ha subslot fora do slot 3.
- **Mapper (RAM mapeada)**: uma so' por maquina (as portas FCh-FFh sao unicas).
  Tamanhos de 64 a 1024 KB; 1024 KB sao 64 segmentos de 16KB.
- **Disco**: DISK.ROM na pagina 1 e a controladora WD2793 nos enderecos 7FF8h-7FFFh
  da celula. Uma so' por maquina. Com `path2`, a sub-ROM do MSX2 vai para a pagina 0.
- **FM-PAC**: ROM de 16KB com SRAM de 8KB, na celula dele (padrao 2:0).
- **SCC**: liga-se ao primeiro cartucho do layout (ou ao slot 1:0 se nao houver).
- **Expansao**: no MSX1 so' o slot 3 e' expandido (regra de hardware). Um subslot
  fora dele liga a regra do MSX2 (`msx1_subslot_rules = 2`).

## 3. Padrao (o que a maquina sempre fez)

| Celula | MSX1 | MSX2 / MSX2+ |
|---|---|---|
| 0:0 | BIOS (32KB, pagina 0) | BIOS MSX2 (32KB, pagina 0) |
| 3:2 | RAM 64KB | mapper 128KB |
| 3:1 | DISK.ROM (se houver disco) | sub-ROM MSX2 (pagina 0), + DISK.ROM (pagina 1) se houver disco |
| 1:0 | cartucho (se houver) | cartucho (se houver) |
| 2:0 | FM-PAC (se o FMPAC.ROM existir) | FM-PAC (se o FMPAC.ROM existir) |

O FM-PAC liga por padrao quando `resource/fMSX/FMPAC.ROM` existe. `--no-fmpac`
desliga na linha de comando, e o menu **Cartucho > FM-PAC (Panasonic, slot 2:0)**
liga e desliga (recria a maquina).

## 4. Interface

- **Maquina > Configuracao de slots...**: uma tabela com as 16 celulas. Cada linha
  tem o conteudo (combo), o arquivo principal (Procurar...), o segundo arquivo (BASIC
  na BIOS, sub-ROM MSX2 no disco) e uma opcao (pagina, tamanho da RAM ou do mapper).
  Botoes: **Padrao** (volta ao layout padrao), **Aplicar e reiniciar** (valida e recria
  a maquina; se nao montar, mostra a razao em vermelho).
- **Cartucho** e **FM-PAC** no menu continuam funcionando e agora editam o layout.
- **Trocar o modelo** (MSX1, MSX2, MSX2+) volta o layout ao padrao; cartucho e
  FM-PAC sao mantidos.

## 5. Verificacao

`machinetest`, secao 3d (7 checagens + 4 de RAM em subslot):

- BIOS de 16KB em pagina 0 e BASIC de 16KB em pagina 1, em arquivos separados, na
  celula 0:0; pagina 0 e pagina 1 conferidas byte a byte; a maquina chega ao prompt
  do BASIC.
- Mapper de 1024KB em 3:1 (64 segmentos de 16KB).
- Quatro bancos de RAM de 64KB em 2:0 a 2:3: montam e chegam ao prompt do BASIC.
- Recusados com razao: sem BIOS em 0:0; duas RAMs mapeadas; RAM de 48KB.

Suite completa do CTest: 14/14.

## 6. O que falta

- **CLI**: o layout so' se edita pelo menu. Falta uma opcao de linha de comando
  para o layout (ex.: `--slot 2:0=ram:64`), util para testes automatizados.
- **Outros cartuchos e ROMs**: MSX-DOS 2 (MSXDOS2.ROM), GameMaster2, o cartucho
  MSX-MUSIC com a propria BIOS. Os tipos atuais cobrem ROM, sub-ROM, RAM, mapper,
  disco e FM-PAC; um tipo novo entra como um `SlotKind` com a sua regra de carga.
- **BIOS de outras maquinas** (ex.: Gradiente Expert 1.1): o arquivo entra no slot
  0:0 e o layout monta, mas a BIOS so' funciona se o hardware que ela espera (portas,
  VDP, disco) existir aqui. Isso nao foi testado com nenhuma BIOS fora do fMSX.
- **Uma BIOS fora de 0:0**: nao e' suportada, porque o Z80 comeca no slot 0:0.
- **Mapper unico**: nao ha duas RAMs mapeadas (as portas FCh-FFh sao unicas).
- **Pagina 0 de cartucho em 16KB** e o caso de ROM de 16KB em pagina 0 (que nao e'
  BIOS) nao tem teste dedicado.
- **Salvar o layout** em arquivo (para reabrir a mesma configuracao) ainda nao existe.

## 7. Arquivos

- `src/machine/machine.h` -- `SlotKind`, `SlotItem`, `SlotLayout`, `MachineConfig::layout`.
- `src/machine/machine.cpp` -- `DefaultLayout`, `EffectiveLayout`, `ValidateLayout`,
  `SetCartridge`, `SetFmPac`, `Machine::Create` (monta o layout).
- `src/machine/gui/emu_window.cpp` -- menus Cartucho, FM-PAC e Maquina, e a janela
  **Configuracao da maquina**.
- `src/memmap/core/slot_state.{c,h}`, `src/memmap/cpp/memory_system.{cpp,h}` --
  `memmap_clear_slot()` e `MemorySystem::ClearSlot()` (esvazia uma celula).
- `tests/z80/machine_test.cpp` -- secao 3d.
