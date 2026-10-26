# Disco rigido (IDE + Nextor) e imagens de HD/SD -- especificacao (documento vivo)

> Introduzido na **1.43.23** (2026-10-24). Mesmo espirito dos outros documentos vivos: o que foi
> feito, as decisoes e o que falta.

## 1. Pedido e decisoes

Pedido do usuario: HD para o MSX, "IDE e SCSI se voce puder", inclusive as ROMs **Sunrise** com o
kernel do **Nextor** (o MSX-DOS 2 novo do Konamiman); tamanho maximo da imagem 2, 4 ou 8 GB, o que
for mais facil (o Nextor so' le particoes de ate' 2 GB, entao um disco de 8 GB precisaria de 4);
SD tanto por Nextor/Sunrise quanto OCM/SCSI se possivel; criar disco pela CLI/parametros como o
`diskmanipulator create` do openMSX, e tambem na TUI e na GUI.

Respostas (AskUserQuestion, 2026-10-24):
- **IDE primeiro, SCSI depois.** Achado da pesquisa: o banco de ROMs tem os Nextor 2.0, 2.0.1, 2.1.0
  (duas) e 2.1.1 **SunriseIDE** (128 KB), mas **nenhuma ROM de driver de SCSI**: a MEGA-SCSI nao tem
  ROM propria, o driver vem em SRAM/disco. Sem driver nao da' para validar o boot, entao o SCSI
  (MB89352 + MEGA-SCSI, uns 800 + 600 linhas no openMSX) fica para uma release propria.
- **Imagem ate' 4 GB, particoes de ate' 2 GB** (ate' 4 particoes, soma ate' 4 GB).
- **SD = outra imagem** na mesma interface IDE (cartao SD/CF por adaptador aparece ao Nextor como
  disco ATA). O SD "de verdade" do OCM/MegaSD (interface propria na memoria) fica com o SCSI.

## 2. O que existe (src/hd, nas 4 linguagens)

| Arquivo | Linguagem | O que faz |
|---|---|---|
| `core/hd_format.{h,c}` | C | Plano do FAT16 (cluster, FAT), setor de boot com o BPB, FAT e raiz vazias, MBR com 4 entradas, LBA -> CHS |
| `fortran/hd_geometry.f90` | Fortran | `hd_fat_sectors`: o tamanho da FAT depende do numero de clusters, que depende do tamanho da FAT; itera ate' estabilizar |
| `asm/hd_fill.asm` | NASM (Win64/SysV) | `hd_fill` (REP STOSB) e `hd_swap_pairs` (troca os pares de bytes das strings do IDENTIFY) |
| `cpp/hd_image.{h,cpp}` | C++ | `HdImage` (acesso por setor, escrita direta no arquivo), `CreateHdImage`, `ParseSize`, `ValidatePartitionSizes`, `ReadPartitions` |
| `cpp/ide_disk.{h,cpp}` | C++ | O disco ATA: registradores, PIO, LBA e CHS |
| `cpp/sunrise_ide.{h,cpp}` | C++ | A interface Sunrise ATA-IDE como `SlotCartIo` (ROM em bancos + mapa dos registradores) |
| `cpp/cli.{h,cpp}` | C++ | `fwmsx --hdnew` |

Codigo **original** (BSD-3-Clause), escrito a partir da especificacao ATA e do mapa de enderecos da
Sunrise; o openMSX (`resource/openMSX/src/ide`) foi lido so' para conferir quais comandos o Nextor
usa e o comportamento do barramento sem disco.

## 3. Formato da imagem

- Setor 0: **MBR** (assinatura 55AAh), ate' 4 entradas, tipo **06h** (FAT16), a 1a ativa. A 1a
  particao comeca no setor 2048 (1 MiB), as seguintes logo depois uma da outra.
- Cada particao: setor de boot com o BPB (512 bytes/setor, 2 FATs, 512 entradas na raiz, media F8h,
  setores escondidos = inicio da particao), duas FATs (F8 FF FF FF no inicio) e a raiz vazia.
- Cluster: o menor que deixe o numero de clusters dentro do FAT16 (entre 4085 e 65524). **2048 MB
  nao cabe inteiro**: com clusters de 32 KB dariam 65527 clusters, o que quem le tomaria por FAT32;
  o sistema de arquivos fica com 65524 clusters, o BPB declara so' o que usa, e os poucos setores que
  sobram no fim da particao ficam sem uso. 8 MB e' o minimo (abaixo disso a conta cairia em FAT12).
- O arquivo e' esparso quando o sistema deixa (o ultimo setor fixa o tamanho). Tamanho = 1 MiB + soma
  das particoes.
- Geometria dita ao BPB e ao IDENTIFY: 16 cabecas x 63 setores (o Nextor usa LBA).

## 4. O disco ATA

Tudo termina na hora (sem BSY entre comando e resultado) e por PIO, um setor de 256 palavras por
vez. Comandos: **READ SECTORS** (20h/21h) e **WRITE SECTORS** (30h/31h) em LBA ou CHS, **IDENTIFY
DEVICE** (ECh: disco fixo, LBA, total de setores, modelo "fwMSX hard disk"), **INITIALIZE DEVICE
PARAMETERS**, **SET FEATURES**, **EXECUTE DEVICE DIAGNOSTIC**, **READ NATIVE MAX** (F8h), recalibrar,
seek, verify, flush e os comandos de energia (aceitos sem fazer nada). O resto responde ABRT, e o
ultimo comando desconhecido fica registrado (`ide` mostra; o Nextor tenta o DEVICE RESET 08h, que e'
so' de dispositivo de pacote, e o openMSX tambem o recusa). Leitura/escrita alem do fim: IDNF.
Imagem aberta so' para leitura (`--disk-ro`): WRITE SECTORS recusado.

## 5. A interface Sunrise

Dispositivo de slot de cartucho (`SlotCartIo`), uma instancia por slot (a 1.43.23 trocou o `SlotMemoryBus`
de "um dispositivo so'" para uma tabela por slot, para o SCC e o IDE conviverem). Mapa:
- Leitura de 4000h-7FFFh: o banco atual da ROM (128 KB = 8 bancos de 16 KB). No reset o controle vale
  FFh: registradores ligados e banco 7.
- Escrita em `(addr & BF04h) == 0104h` (4104h): **controle**. Bit 0 liga os registradores IDE no
  lugar da ROM; bits 7-3 = numero do banco com os bits em ordem invertida (bit 7 = bit 0 do banco).
- 7C00h-7DFFh: dados (16 bits; o endereco par le/escreve o byte baixo, o impar o alto, com trava);
  7E00h-7EFFh: registradores do ATA (`addr & 0Fh`; 0Eh = estado alternativo / controle do
  dispositivo, bit 2 = SRST). Os dois blocos aparecem tambem em 3C00h/BC00h por espelhamento.
- Sem disco escravo, os registradores dele leem 7Fh (o barramento flutua; o Nextor reconhece).
- Contadores de diagnostico (leituras da ROM, escritas no controle, acessos ao ATA, setores lidos e
  gravados) no comando `ide`.

**Onde fica: slot 3:0.** O usuario pediu uma celula propria em 2:1, mas a regra de hardware da
MSX2 (`msx1_subslot_rules = 2`) deixa os slots 1 e 2, de cartucho, sem subslot: a BIOS nao os
enxerga expandidos e nao acharia o Nextor. O slot 3 e' o expandido; 3:0 era a celula livre (3:1 =
DISK.ROM, 3:2 = mapper, 3:3 = 2o mapper). `--ide-slot P:S` e a janela de slots mudam.

**O achado que fez tudo funcionar:** o Nextor fica preso em "Initializing :" porque usa o
temporizador do MSX turbo R (`OUT (E6h)` zera, `IN (E6h)` ate' passar de 3) para esperas de
microssegundos. Numa MSX2 as portas E6h/E7h nao existem e leem **FFh** (barramento flutuando) e a espera
passa na hora; o `CompositeBus` devolvia 0 para qualquer porta sem dispositivo. Passou a devolver FFh,
que e' o correto para o MSX (e o que o fMSX/openMSX fazem).

## 6. Como usar

- Criar: `fwmsx --hdnew sd.hdd 2G 2G` (ou `--split 4G 2 sd.hdd`, `--info sd.hdd`); no console e no TUI
  `newhd`; na janela *Midia > Disco rigido (IDE) > Novo disco rigido / cartao SD...* (1 a 4 particoes
  de 8 MB a 2 GB, total ate' 4 GB, e liga como mestre se quiser).
- Ligar: `fwmsx --msx --msx2 --ide sd.hdd [--ide-slave b.hdd] [--nextor rom] [--ide-slot P:S]`; `ide
  <img> [<img2>] [<rom>]` e `ide off` no console; menu da janela e do TUI. A ROM do Nextor, quando nao
  dada, e' a mais nova do banco de ROMs (`romdb::FindNextorRom`, pelo SHA-1).
- O Nextor numera os drives na ordem em que acha os dispositivos: com o HD ligado a 1a particao vira
  **A:** e os disquetes passam a B:. Ele so' inicia o MSX-DOS se achar COMMAND2.COM em A:; senao cai no
  BASIC ("Nextor BASIC version 2.10"). Visto no teste: a 2a particao nao ganhou letra (`files "c:"` deu
  "Bad drive name"); pelo manual do Nextor ela se liga com o comando `MAPDRV`, que **nao foi testado**
  aqui.
- **Copiar os arquivos de sistema pelo `msxdisk` (1.43.27)**, sem precisar passar pelo BASIC do Nextor:
  `msxdisk add sd.hdd:1 COMMAND2.COM` (particao 1, 1-based; `doc/msxdisk-spec.md` Fase 6) -- `list`/
  `extract`/`delete`/`rename`/`mkdir`/`rmdir`/`info` tambem funcionam direto numa particao dessas. Com
  `COMMAND2.COM` na raiz da particao A:, o Nextor pula o BASIC e vai direto para o prompt do MSX-DOS 2.

## 7. Testes

`hdtest` (ctest `hd`): `ParseSize`, limites das particoes, plano do FAT16 de 8 a 2048 MB (C + Fortran),
MBR/BPB/FAT da imagem criada, leitura por setor, o disco ATA (IDENTIFY com o modelo desembaralhado
pelo Assembly, READ/WRITE de 1 e 2 setores, LBA e CHS, erros, somente leitura), a Sunrise (banco no
power-on, controle em 4104h, registradores, latch de dados, escravo ausente, SRST, reset) e o
`--hdnew`. `msx2test` 8c: **boot de verdade do Nextor** 2.1.1 sobre uma imagem criada pelo fwMSX (pulado
sem a ROM): "Nextor BASIC version 2.xx", leu a tabela de particoes, gravou `OLA.TXT` na particao A: e o
arquivo esta' no diretorio do `.hdd`. `controltest`: `ide`/`newhd`.

## 7b. O cartao SD pelo MegaSD (1.43.24)

Material novo do usuario: o pacote do KdL (OCM-PLD v3.9.2 e SDBIOS) em `third-party/ocm` (788 MB, 3869 arquivos; fora do git). O que
serve ao SD: `SDBIOS/make/roms/nextsd15.rom` (**Nextor 2.1.4 + driver MegaSDHC v1.5**, 128 KB; ha' tambem `n3xtsd15` com o Nextor 3 beta, as
versoes 1.4 e os `megasd1s/2s.rom` antigos), o **codigo-fonte Z80 do driver** (`src/20261002 ocm-megasdhc disk drivers for nextor`) e o
hardware em Verilog (`esemsx3/src/peripheral/megasd.v`, so' como referencia). As ROMs e utilitarios uteis foram **copiados para
`dist/roms/ocm/`** (fora do git: `dist/roms/` e' ignorado e `third-party/ocm/` entrou no `.gitignore`).

Hardware (`megasd.h`): um mapper "ESE-RAM" de 1 MB com quatro janelas de 8 KB em 4000h/6000h/8000h/A000h; os bancos se escolhem escrevendo em
6000h/6800h/7000h/7800h (o do 6000h e' de 7 bits na 2a janela); bit 7 = RAM gravavel. Com o registrador do banco 0 em **40h-7Fh** a janela 4000h-5FFFh
vira o cartao: 4000h-57FFh e' o registrador de dados SPI (cada acesso troca um byte; **a leitura devolve o byte da troca anterior**, por isso os
drivers fazem uma leitura de enchimento; o bit 12 do endereco e' o /CS: 4000h seleciona, 5000h solta), 5800h-5FFFh e' o modo (escrita: bit 7 baixa
velocidade, bit 0 = 1 desliga o soquete; leitura de 5800h-5BFFh: estado, bit 0 = acabou de ligar). A ROM do Nextor vai para o inicio da RAM.

O cartao (`sd_card.h`): SPI com CMD0/8/9/10/12/13/16/17/18/23/24/25/32/33/38/55/58/59 e ACMD22/23/41, CSD v1 (SDSC, com blocos de leitura de 1024 bytes acima
de 1 GB) e v2 (SDHC), OCR com o bit 30. Imagem acima de 2 GB = SDHC (endereco em setores), senao SDSC (em bytes). **Achado:** o driver do MegaSD le e
descarta dois bytes depois do CRC do comando, entao a resposta tem de vir depois deles; o cartao poe dois FFh antes de cada resposta (o Ncr de um
cartao real e' de 1 a 8 bytes). Sem isso o Nextor dizia "Disk offline".

Verificado no emulador (MSX2+ com `--sd`): o Nextor 2.1.4 sobe ("Nextor BASIC version 2.10"), inicializa o cartao, le a tabela de particoes e **grava
arquivos** (`OPEN "A:OI.TXT"` ... `FILES` lista `OI.TXT`), tanto numa imagem SDSC de 129 MB quanto numa SDHC de 4097 MB.

## 8. O que falta

- **SCSI** (MEGA-SCSI com MB89352 e o SRAM de 1 MB, Gouda, WAVE-SCSI): continua sem driver. O pacote do KdL **nao** tem driver nem emula
  o MB89352 (no `megaram.vhd` o acesso ao SPC esta' comentado como "reservado"; so' ha' os utilitarios ESET/EP/MGLOAD, que rodam no MSX).
- Estado salvo (`state save`) nao guarda a interface IDE (os registradores do ATA); as imagens ja' estao no
  arquivo.
- Mais de 2 GB por particao (FAT16 de 64 KB) e particoes estendidas (alem de 4 primarias).
- Imagens maiores que 4 GB (o LBA de 28 bits aguenta 128 GB; o limite de 4 GB e' escolha de projeto).
- O mapper de 4096 KB para a RAM, o HD e o MSX-DOS 2 juntos nao foram exercitados com um programa grande.
