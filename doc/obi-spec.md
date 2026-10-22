# obi -- a receita que monta um disquete (.dsk) pronto para rodar

Versao 1.43.13. Codigo em `src/obi/` (C++, C, Fortran e NASM). Testes: `obitest`, `editorui` (Run > Build disk) e `controltest` (`obi`, `obirun`).

## 1. Para que serve

Juntar num so' comando o que um programa MSX precisa para rodar em A:: o fonte Basic Dignified (`.dmx`), o fonte Assembly (`.asm`), dados soltos (`.dat`, `.bin`), sprites (`.spr`), telas (`.scr`...) e o que mais houver. A receita e' um arquivo de texto chamado `Obifile` (ou `*.obi`) ao lado dos fontes; o `obi` converte o que precisa converter, grava tudo num `.dsk` e, se quiser, deixa o disco rodando sozinho.

A sintaxe e' o subconjunto do `Obifile` do Kizuna (`resource/kizuna`, `pkg/obi`): `chave: valor`, listas `- campo: valor`, comentarios com `#` no inicio da linha. **Decisao do usuario (2026-10-22)**: o `obi` do fwMSX usa as ferramentas que o fwMSX ja' tem (`fwasm`, `badig`, `msxdisk`); o linker/compiladores do Kizuna (`kaji80`, `wirth80`, `dignac`, `musubi`, `hako`, objetos `.MOB`) ficam para quando o Kizuna entrar no projeto. Por isso os campos do Kizuna que dependem deles (`modules`, `resources`, `link`, `libraries`, `api`) nao existem aqui.

## 2. Exemplo

```yaml
# Obifile
target: jogo.dsk          # o disquete a gerar (relativo a pasta do Obifile)
boot: JOGO.BMX            # opcional: quem roda sozinho; padrao = o primeiro .BMX

files:
  - jogo.dmx              # Basic Dignified -> JOGO.BMX
  - rotina.asm            # Assembly Z80 -> ROTINA.BIN (ou .COM / .ROM, pelo formato do fonte)
  - dados.dat             # copiado como esta'
  - source: hero.spr      # nome no disco diferente
    name: HERO.SPR
  - tela.scr
```

```
fwMSXc --obi build            # o Obifile da pasta atual
fwMSXc --obi build jogo/Obifile
fwMSXc --obi check            # monta e confere, sem gravar o .dsk
```

No console do emulador: `obi build [Obifile]`, `obi check [Obifile]` e `obirun [Obifile]` (monta, insere o disco em A: e reinicia a maquina; precisa de `--disk`). No editor: **Run > Build disk** (`F6`).

## 3. Campos

| Campo | Significado |
|---|---|
| `target:` | o `.dsk` a gerar. Obrigatorio. |
| `files:` | a lista de arquivos (`- caminho` ou `- source: caminho` + `name:`). Obrigatoria. Caminhos relativos a pasta do Obifile. |
| `boot:` | o arquivo, **pelo nome no disco**, que inicia sozinho. Vazio = o primeiro `.BMX`; sem `.BMX`, o disco nao inicia nada. |
| `dos:` | `1` ou `2` = o disco leva o **MSX-DOS 1 ou 2**, montado a partir dos arquivos da pasta `emulator/msxdos1` ou `emulator/msxdos2` (secao 5b). `true` (forma antiga) = parte de um disco de sistema pronto, dado por `dos_disk`. |
| `dos_tools:` | `true` (so' com `dos: 2`) = leva tambem `UTILS\`, `HELP\`, `AUTOEXEC.BAT` e o resto da pasta do MSX-DOS 2 (cabe: sobram uns 300 KiB). |
| `dos_disk:` | com `dos: true`: o disco de sistema pronto. Vazio = procura `msxdos1.dsk` (na pasta do Obifile, em `dos/` e em `disks/`). |

Campo desconhecido e' erro com a linha (`Obifile, line 4: unknown key 'foo'`).

## 4. O que cada tipo de arquivo vira

| Origem | No disco |
|---|---|
| `.dmx` | `NOME.BMX`: o BASIC tokenizado pelo clone do badig (as opcoes vem do `badig.cfg` e das remtags do fonte). Os `disk_files` e o NBASIC.BIN do fonte vao junto, como no F5. |
| `.asm` | `NOME.COM` (fonte `.MSXDOS`), `NOME.BIN` (`.BASIC`/`.BIN`) ou `NOME.ROM`. Sem diretiva de formato, os bytes crus vao como `.BIN` com um aviso. |
| qualquer outra | os bytes, sem mexer (`.dat`, `.bin`, `.spr`, `.scr`, `.bas`...). Quando os editores graficos tiverem formato proprio, a conversao entra aqui. |

Nomes: o nome do disco e' o do arquivo em maiusculas, no formato 8.3 do MSX-DOS. Se nao couber (mais de 8 letras, caractere proibido), a build para e pede `name:`. Dois arquivos com o mesmo nome no disco tambem param a build.

## 5. Como o disco inicia

`boot:` decide o `AUTOEXEC.BAS` (o DISK BASIC o executa ao ligar):

| `boot` | AUTOEXEC.BAS |
|---|---|
| `X.BMX` ou `X.BAS` | `10 RUN "X.BMX"` |
| `X.BIN` | `10 BLOAD "X.BIN",R` |
| `X.COM` | exige `dos: true`; gera `AUTOEXEC.BAT` com `X` |

Com `dos: 1`/`2` (ou `true`) e um `.BMX`/`.BIN`, o disco leva tambem um `AUTOEXEC.BAT` com `BASIC` (o MSX-DOS 1 cai no COMMAND.COM; o comando BASIC entra no BASIC, que roda o `AUTOEXEC.BAS`). **Este caminho (`BASIC` no AUTOEXEC.BAT do DOS) nao foi exercitado no emulador ainda.** Listar na receita um arquivo chamado `AUTOEXEC.BAS`/`.BAT` junto com `boot:` e' erro.

## 5b. A pasta `emulator/` e os discos do MSX-DOS

Ao lado do executavel (`dist/` na pasta de trabalho) existe a pasta **`emulator/`**, criada pela build:

```
emulator/
  msxdos1/     MSXDOS.SYS, COMMAND.COM            (copiados de resource/msxdos1)
  msxdos2/     MSXDOS.SYS, MSXDOS2.SYS, COMMAND2.COM, UTILS\, HELP\...   (de resource/msxdos2)
  msxdos1.dsk  disquete de sistema do MSX-DOS 1, gerado pelo proprio obi
  msxdos2.dsk  idem, MSX-DOS 2 (so' o essencial)
```

- `obi dosdisk <1|2> [saida.dsk] [--tools]` gera o disquete de sistema (padrao: `emulator/msxdosN.dsk`); a build roda isso uma vez para cada versao.
- Um `Obifile` com `dos: 1` monta o disco **em branco + arquivos do sistema + os seus** (o setor de boot do `msxdisk` carrega o `MSXDOS.SYS`). Os arquivos do MSX-DOS entram antes dos seus, entao um arquivo seu de mesmo nome (um `AUTOEXEC.BAT`, por exemplo) vence.
- `boot: X.COM` com `dos: 1`: o `AUTOEXEC.BAT` gerado chama `X`; o MSX-DOS 1 nao pergunta a data quando ha' `AUTOEXEC.BAT` e roda direto. **Exercitado de ponta a ponta no `controltest`** (o DOS 1 inicia no emulador, roda o AUTOEXEC.BAT e o `.COM` imprime na tela).
- **Licenca**: os arquivos do MSX-DOS sao de terceiros. Desde a 1.43.13 a pasta `emulator/` **vai nos pacotes** (zip/tar.gz), por decisao do autor, e sera retirada se algum titular dos direitos pedir (LICENSE-THIRD-PARTY.md). Se a pasta faltar (instalacao propria, build do codigo-fonte sem `resource/`), basta por os arquivos do sistema em `emulator/msxdos1/` e `emulator/msxdos2/` (procurada ao lado do executavel, depois em `emulator/` e `resource/` a partir da pasta atual).
- **Nao verificado**: o MSX-DOS 2 so' inicia numa maquina com a ROM do kernel DOS2; o disco `dos: 2` sai com os arquivos certos (testado), mas nao foi iniciado no emulador.

## 6. De onde saem os fontes

Em ordem: os **buffers abertos no editor** (o que esta' na tela, salvo ou nao), o banco do **projeto** `.msxproject` (como a 1.43.7) e a **pasta**. No console e na linha de comando, so' a pasta. O `.include` do Dignified e o `.include`/`.incbin` do Assembly seguem a mesma ordem.

## 7. Conferencias antes de gravar

- cada arquivo e' lido, convertido e nomeado antes de tocar no disco; se um falha, nada e' gravado;
- o disco e' de 720 KB (unico formato que o `msxdisk` cria); a conta de clusters (Fortran, `obi_clusters_needed`: cluster de 1 KiB, arredondado por arquivo) diz quanto falta quando nao cabe (`the files need 812 KiB and the disk has 708 KiB free`);
- o log mostra, para cada arquivo, o nome no disco, o tamanho, o CRC-32 (Assembly, `obi_crc32`) e de onde veio.

## 8. Mensagens e o editor

Os erros viram mensagens com arquivo e linha: do Obifile (`Obifile, line 3: cannot read x.dat`), do Badig (`.dmx`/`.include`) e do montador (`rotina.asm, line 5: ...`). No editor, `F8` / `Shift-F8` andam por elas e levam o cursor ao lugar (na janela grafica abrindo a aba do fonte; no terminal, so' se o arquivo ja' esta' aberto).

## 9. Nas 4 linguagens

| Linguagem | Arquivo | Papel |
|---|---|---|
| C++ | `obi_parse.cpp`, `obi_build.cpp`, `obi_cli.cpp` | receita, build, comando |
| C | `obi_name.c` | nome 8.3 valido e o tipo pela extensao |
| Fortran | `obi_fit.f90` | clusters que os arquivos ocupam |
| NASM (Win64/SysV) | `obi_crc.asm` | CRC-32 de cada arquivo |

## 10. Fora de escopo por enquanto

- varios formatos de disco (360 KB, 1,44 MB): o `msxdisk` so' cria 720 KB;
- `modules`/`resources`/`link`/`libraries`/`api` do Kizuna (dependem do linker deles);
- conversao dos formatos dos editores graficos (`.SPRITE`, `.SCR`): copiados como estao ate' existirem;
- Obifile no menu de contexto da arvore do projeto, destaque de sintaxe do Obifile.
