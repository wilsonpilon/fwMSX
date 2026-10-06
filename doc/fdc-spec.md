# Disco: controladora WD2793 + DiskROM (MSX-DOS) -- especificacao (documento vivo)

> Mesmo espirito de `doc/psg-spec.md`, `doc/machine-spec.md` e
> `doc/audio-spec.md`: o que foi feito, as decisoes e o que falta, para
> retomar do ponto exato onde parou.

Estado: **Fase 1 concluida em 2026-10-02**. `fwmsx --msx --disk msxdos1.dsk`
**boota o MSX-DOS 1.8** pela controladora, le e grava arquivos no `.dsk`.

## 1. Uso

```
fwmsx --msx --disk <arq.dsk> [--diskb <arq.dsk>]   # liga a interface de disco e insere os discos
fwmsx --msx --disk-interface                       # so' a interface (sem disco)
fwmsx --msx --diskrom <DISK.ROM> ...               # outra ROM de disco (padrao: DISK.ROM ao lado da BIOS)
```

- Sem `--disk`/`--disk-interface` a maquina e' o MSX BASIC "puro" (28815 bytes
  livres). Com a interface ela vira o **Disk BASIC** (23432 bytes livres) e,
  havendo disco de boot em A:, carrega o MSX-DOS.
- Sem disco, essa DISK.ROM pergunta a data (`Enter date (M-D-Y):`) e so' entao
  vai para o Disk BASIC -- e' o kernel dela, nao um defeito.
- Menu **Disco** da janela: inserir/ejetar A: e B: a qualquer momento (o
  dialogo de arquivo e' o do Windows, o mesmo do msxdisk; no Linux use `--disk`).
- **As escritas do MSX-DOS vao direto para o arquivo da imagem**, setor a
  setor, como o fMSX. Faca uma copia antes se o disco importa -- ou use
  **`--disk-ro`** (v1.11.1): os discos entram protegidos contra gravacao, o MSX-DOS
  le normalmente, responde `Write protect error writing drive A` ao tentar gravar e
  o arquivo nunca e' alterado (vale tambem para os discos inseridos depois, pelo menu).
- Tamanhos aceitos: 160K, 180K, 320K, 360K, 720K e 1.44M, ou o que o BPB do
  setor de boot descrever (ex.: 360K de 80 trilhas e 1 lado).

## 2. Arquitetura

```
src/fdc/
├── core/fdc_state.{h,c}   C -- WD1793/WD2793 (adaptado do WD1793.c do fMSX) + geometria
└── cpp/
    ├── disk_image.{h,cpp}  C++ -- imagem .dsk em memoria, gravacao imediata no arquivo
    └── fdc_device.h        C++ -- registradores mapeados em memoria (SlotMmio)
src/memmap/cpp/slot_memory_bus.h   SlotMmio + AttachMmio(): dispositivo MMIO dentro de um slot
```

**O DiskROM e' ROM de verdade (a do fMSX) e a controladora fica *dentro* dela.**
A `DISK.ROM` vai no **slot 3:1** (como o fMSX), em 4000h-7FFFh, e os registradores
do WD2793 aparecem sobre ela enquanto esse slot esta visivel na pagina:

| Endereco | Leitura | Escrita |
|----------|---------|---------|
| `7FF8h` (`BFF8h`) | status | comando |
| `7FF9h` / `7FFAh` | trilha / setor | trilha / setor |
| `7FFBh` | dados | dados |
| `7FFCh` | -- | lado (bit 0) |
| `7FFDh` | -- | drive (bit 0) |
| `7FFFh` | DRQ (40h) / IRQ (80h) | -- |

Isso exigiu um mecanismo novo no barramento: `SlotMmio` (um dispositivo que atende
leituras/escritas em certos enderecos *so'* se o slot dele esta visivel na
pagina -- confere `psl[pagina]`/`ssl[pagina]`). O motor de slots em C nao mudou.

**Emulacao real da controladora, nao "patch de BDOS".** O fMSX tambem oferece
simular o DiskROM (`ED FE C9` nas rotinas), que e' mais rapido mas nao e'
hardware. Aqui roda a ROM de disco original falando com o WD2793 -- o MSX-DOS
nao sabe a diferenca.

## 3. O motor (`fdc_state.c`)

Adaptado do `WD1793.c` do fMSX, com uma diferenca de projeto: o fMSX acessa as
imagens por `FDIDisk` (varios formatos, varredura de cabecalhos de setor); aqui
a imagem e' um `.dsk` **cru** (setores em sequencia: trilha, lado, setor) descrito
por `FdcDisk`. O endereco de um setor e' `((trilha x lados + lado) x setores/trilha
+ setor-1) x 512`.

- **Tipo 1:** RESTORE, SEEK, STEP / STEP-IN / STEP-OUT (com e sem update do
  registrador de trilha); status TRACK0, INDEX (alterna a cada leitura), HEADLOAD.
- **Tipo 2:** READ SECTOR(S) e WRITE SECTOR(S), multiplos (avanca o registrador
  de setor); NOT FOUND, protecao contra gravacao.
- **Tipo 3:** READ ADDRESS. **READ TRACK / WRITE TRACK nao suportados** (como no
  fMSX) -- entao **FORMAT de baixo nivel nao funciona**; use discos ja
  formatados (`msxdisk create`).
- **Tipo 4:** FORCE INTERRUPT.
- **Sem temporizacao fisica:** um comando termina na hora. So' o protocolo
  DRQ/IRQ e o *watchdog* do fMSX: ler `7FFFh` 255 vezes sem consumir os dados
  aborta o comando com LOST DATA (e' o que impede a ROM de travar esperando).
- **Persistencia:** `DiskImage` grava cada setor no arquivo ao fim dele
  (`write_cb`). Sem permissao de escrita no arquivo, o disco entra protegido
  e o MSX-DOS ve' o erro em vez de perder dados em silencio.

## 4. Verificacao

- `fdctest`: geometria (6 tamanhos + BPB mandando sobre o tamanho), todos os
  comandos acima, erros, o watchdog, persistencia no arquivo, e o mapeamento
  MMIO no barramento de slots (so' com o slot 3:1 visivel; espelho em `BFF8h`).
- `machinetest` (com o **MSX-DOS 1 real**, `msxdos1.dsk` do repositorio): boot
  `MSX-DOS version 1.8` + `COMMAND version 1.12`, `dir` lista `COMMAND.COM` e
  `MSXDOS.SYS`, e `copy command.com x.com` **grava no arquivo .dsk**: a entrada
  `X.COM` com 7168 bytes aparece no diretorio e os setores de dados de
  `COMMAND.COM` existem em dobro, byte a byte. Conferido tambem fora do teste:
  `msxdisk extract` de `X.COM` tem o mesmo hash do `COMMAND.COM` original.

## 5. Limites e o que falta

- **Sem disco no BASIC puro:** a interface so' liga com `--disk`/`--disk-interface`
  (nao da' para "encaixar" a DISK.ROM com a maquina ligada).
- **Sem FORMAT** (READ/WRITE TRACK), sem disco de dupla densidade alem dos
  tamanhos acima, sem imagens que nao sejam `.dsk` cru (`.fdi`, `.dmk`...).
- **Sem DiskROM do MSX-DOS 2** (precisa de MSX2 + mapper de RAM).
- **Troca de disco "a quente":** funciona (o menu insere/ejeta), mas o
  MSX-DOS 1 so' percebe a troca na proxima leitura do disco; nao ha' o sinal
  de "disk changed" do hardware real alem do que a ROM ja' faz.
- **Velocidade:** o `copy` de 7KB leva ~300 quadros emulados (5 s) porque o
  DiskROM/FDC rodam em tempo de emulacao, sem aceleracao de acesso a disco.
- Lode Runner + Konami SCC (ROM que espera disco) **nao foi retestado** com a
  interface ligada.

## 6. Controladora por memoria ou por portas, e formatos de disco (nao lancado)

Referencias de estudo: o openMSX tem duas controladoras por porta, a **DDX 3.0** (Digital Design)
e a **Microsol CDX-2**, ambas com a mesma convencao (`src/fdc/MicrosolFDC.cc`, por Ricardo Bittencourt).
A ROM do driver (`ddx_3.0.rom`, `cdx-2.rom`) fica em 4000h-7FFFh do slot, como o DISK.ROM, e chama
as portas. Os SHA-1 dos arquivos que temos batem com os do openMSX.

**Acesso.** Menu **Maquina > Configuracao de disco...**, ou `--disk-acesso mem|porta`,
`--disk-porta`, `--diskrom <driver>`:

- **Pela memoria** (padrao): DISK.ROM no slot 3:1; o WD2793 aparece em 7FF8h-7FFFh.
- **Pelas portas**: a ROM do driver (DDX 3.0 ou CDX-2) no slot da celula Disco, e o WD2793 nas
  portas `base` a `base+4` (padrao D0h). Sem controladora por memoria.

**Mapa de portas (modo por portas, como o MicrosolFDC):**

| Porta | Leitura | Escrita |
|---|---|---|
| base+0 | status | comando |
| base+1 | trilha | trilha |
| base+2 | setor | setor |
| base+3 | dados | dados |
| base+4 | bit 7 = IRQ; bit 6 = DRQ (**0 quando ha' dado pedido**) | controle: bit 0 = drive A; bit 1 = drive B; bit 4 = lado (1 = lado 1); bit 5 = motor; bit 6 = espera; bit 7 = densidade |

Bits 5 a 7 do controle (motor, espera e densidade) sao aceitos e nao modelados: o formato e' o do drive.
Portas base+5 a base+7 nao respondem (FFh).

**Verificado:** o MSX-DOS 1.8 sobe com o DDX 3.0 e com o CDX-2 pelas portas D0h, mostra o banner
da interface, lista o diretorio (`dir`) e grava (`copy`). O teste automatico (`machinetest`, secao 3e)
faz o boot com o DDX 3.0 quando a ROM esta em `dist/roms`.

**Formatos (`src/fdc/cpp/disk_format.h`).** O drive e' escolhido por tipo, faces e densidade:

| Tipo | Faces | Densidade | Formato |
|---|---|---|---|
| 5 1/4 | simples | simples (FM) | 180 KB (40 trilhas x 1 lado x 9 setores) |
| 5 1/4 | dupla | dupla (MFM) | 360 KB (40 x 2 x 9) |
| 3 1/2 | simples | dupla (MFM) | 360 KB (80 x 1 x 9) |
| 3 1/2 | dupla | dupla (MFM) | 720 KB (80 x 2 x 9) |

"Automatico" (padrao) aceita os tres tamanhos. Um formato escolhido recusa imagens de outro
tamanho, com a razao na tela. Um 360 KB de 3 1/2 e' lido com 80 trilhas de 1 lado.

**Testes:** `fdctest` (formatos e escolhas; porta e memoria lendo o mesmo setor byte a byte; lado,
drive e motor pelo `base+4`), `machinetest` (secao 3e: controladora por portas, formato recusando
imagem errada, conflito de porta, e o boot do MSX-DOS com o DDX 3.0).

**Ainda falta:** modelar FM/MFM; formatar disquetes (so' le e grava setores de imagens existentes);
drives A e B com formatos diferentes (hoje usam o mesmo); o driver do openMSX para a controladora
de porta fica como referencia, sem codigo copiado.
