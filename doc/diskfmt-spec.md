# Disquete novo em branco e FORMAT -- especificacao (documento vivo)

> Mesmo espirito de `doc/fdc-spec.md` e `doc/msxdisk-spec.md`: o que foi feito, as decisoes e o
> que falta. Introduzido na **1.31.0** (2026-10-09).

## 1. O que existe

- **Criar um disquete novo, em branco e FORMATADO**, pela janela (**Midia > Disco > Novo disco em
  branco em A:/B:**, escolhe o formato, depois o nome do arquivo; o disco ja' entra no drive) ou
  pela linha de comando (`fwmsx --disknew <arq.dsk> <ss525|ds525|ss35|ds35>`).
- **FORMAT / CALL FORMAT dentro do MSX**: a controladora WD2793 agora aceita o comando
  **WRITE TRACK** (F0h/F4h), entao o `CALL FORMAT` do Disk BASIC (e o FORMAT do MSX-DOS) formata
  o disco inserido.

## 2. Formatos (so' os 4 que o MSX tem)

| Chave | Disco | Lados | Trilhas | Capacidade | Media | Cluster | Raiz | Setores/FAT |
|---|---|---|---|---|---|---|---|---|
| `ss525` | 5 1/4, densidade simples | 1 | 40 | 180 KB | FCh | 1 | 64 | 2 |
| `ds525` | 5 1/4, densidade dupla | 2 | 40 | 360 KB | FDh | 2 | 112 | 2 |
| `ss35` | 3 1/2, densidade dupla | 1 | 80 | 360 KB | F8h | 2 | 112 | 2 |
| `ds35` | 3 1/2, densidade dupla | 2 | 80 | 720 KB | F9h | 2 | 112 | 3 |

Todos com 512 bytes/setor, 9 setores/trilha, 1 setor reservado e 2 copias da FAT. **Nao existe
3 1/2 de 1,44 MB** (dupla face + alta densidade): o MSX nao suporta, a chave nao existe e um teste
confirma isso.

## 3. Arquitetura (C + Assembly + C++)

```
src/diskfmt/
+-- core/diskfmt.{h,c}      C   -- tabela de formatos, BPB, FAT12, setor de boot, diretorio raiz
+-- asm/diskfmt_fill.asm    ASM -- diskfmt_fill(): REP STOSB, enche a area de dados com E5h (dual-ABI Win64/SysV)
+-- cpp/
    +-- disk_creator.{h,cpp}  C++ -- CreateBlankDisk(): monta a imagem e grava o arquivo
    +-- cli.{h,cpp}           C++ -- fwmsx --disknew
```

- E' uma **reescrita** do `CreateDisk()` de `resource/msxDiskUtil/MSXDisk.pbi` (PureBasic) -- pedido
  explicito do usuario, embora o `src/msxdisk` ja' fosse um port que so' criava 720 KB. As duas
  implementacoes convivem: `msxdisk create` continua como estava.
- O setor de boot reaproveita o bootstrap Z80 do MSX-DOS 1 (`msxdos1_boot.cpp`) com o BPB
  reescrito para o formato; o disco novo inicializa assim que receber `MSXDOS.SYS` e `COMMAND.COM`.
- A area de dados vem com **E5h** (o byte de "formatado" do FORMAT), a FAT com `media FF FF`, a
  raiz vazia.

## 4. WRITE TRACK no WD2793 (`src/fdc/core/fdc_state.c`)

O `.dsk` e' cru (setores em sequencia), entao nao ha' o que "criar": o fluxo de formatacao e'
lido byte a byte (`track_byte()`) e cada campo de dados (marca FBh/F8h) e' COPIADO para o setor
que o campo de ID anterior (marca FEh: trilha, lado, setor, tamanho) aponta; lacunas, sincronismo
e CRC (F5h/F6h/F7h) sao descartados. IDs de setor inexistentes sao ignorados. Termina com INTRQ
apos `FDC_TRACK_BYTES` (6250, uma trilha MFM) bytes. Disco protegido responde Write Protect; sem
disco, Not Ready. Os campos novos ficam depois de `disk[]` em `Fdc`, fora do save-state (uma
formatacao em andamento e' abortada ao carregar). READ TRACK continua nao suportado.

## 5. Testes

- `diskfmttest` (CTest `diskfmt`): os 4 formatos (tamanho, BPB, FAT, copias iguais, raiz vazia, E5h
  pelo Assembly, geometria reconhecida pelo FDC), inexistencia do 1,44 MB, `CreateBlankDisk()`, e o
  WRITE TRACK (entrelacamento, ID inexistente, lado 1, protegido, sem disco).
- `machinetest`, secao 7d: o **DISK.ROM de verdade** roda `CALL FORMAT` sobre uma imagem zerada
  ("Drive name?" -> `a` -> "Strike a key when ready" -> "Format complete"); confere o boot sector
  (BPB), a FAT e os E5h no arquivo. O Disk BASIC 1.0 do fMSX formata em **face simples** (media F8h,
  720 setores) e nao pergunta o tipo.

## 6. O que falta

- FORMAT do MSX-DOS 2 / drivers que perguntam o tipo (1 ou 2 faces) nao foram exercitados.
- A densidade (MFM/FM) nao e' modelada; o WRITE TRACK ignora a densidade.
- Criar disco sem formatar (imagem zerada) so' pelo seletor de arquivo; nao ha' opcao dedicada.
