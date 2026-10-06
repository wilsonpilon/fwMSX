# OUTLINE -- fwMSX: onde paramos e o que fazer depois

> Leia este arquivo primeiro. Ele foi escrito para retomar o trabalho em outro computador,
> ou com outra IA, sem perder o contexto. Atualizado em **2026-10-06**.

## 1. O que e' o projeto

**fwMSX** e' um emulador de MSX (MSX1, MSX2, MSX2+) feito como projeto de estudo, a partir do
fMSX (Marat Fayzullin), com o aval do autor original para a adaptacao. Regra do projeto: cada fase
relevante exercita **C, C++, Assembly (NASM, dual-ABI Win64/SysV) e Fortran**, mesmo que minimamente.

Objetivo de medio prazo: integrar o fwMSX ao **msxide** num utilitario de desenvolvimento para MSX
chamado **MSX-PoorManOS**. Ainda sem data.

Este repositorio e' **pessoal**. Midias de terceiros (ROMs, discos, fitas) podem ser versionadas por
enquanto; antes de liberar ao publico, cada midia contestada sera removida (ver `LICENSE-THIRD-PARTY.md`).

## 2. Onde estamos (2026-10-06)

- **Versao publicada:** 1.17.0 "Xak: Musica FM e Slots" (tag/branch `main`, commit `dc7af78`).
- **Trabalho de hoje:** branch **`estudo/openmsx`**, commit de hoje por cima de `main`, mais o commit
  com o codigo do openMSX em `resource/openMSX/` (GPL, so' para estudo).
- **Testes:** `ctest` com **15 suites, todas passando** no estado commitado.
- **Documentos vivos:** `doc/SPEC.md` (secao 5.0 = estado atual; 5.2 = fitas, a fazer),
  `doc/CHANGELOG.md` (secao "Nao lancado"), `doc/RELEASE.md`, `doc/MANUAL.md`, e os `*-spec.md`.

### Funciona (validado)
- MSX1, MSX2, MSX2+: BIOS real ate o prompt do MSX BASIC (1.0, 2.1, 3.0).
- MSX-DOS 1.8 pelo disco (DISK.ROM no slot 3:1) **e pelas portas** com os drivers DDX 3.0 e CDX-2
  (`doc/fdc-spec.md`, secao 6).
- BIOS Gradiente Expert 1.1 sobe com a RAM de 16 KB no fim da celula (C000h-FFFFh).
- VDP completo (SCREEN 0-8 no V9938, 10-12 no V9958 com YJK/YAE e scroll).
- PSG, SCC (F1 Spirit), FM (MSX-MUSIC e FM-PAC) com comandos de BASIC, modo ritmo.
- SRAM de cartucho ASCII8/ASCII16 e FM-PAC (`.sav`).
- Layout de 16 celulas (slot:subslot) pela janela e pela CLI (`--slot`); RAM 16 KB no fim da celula,
  32 KB em duas celulas, mapper de 64 KB a 4 MB (varios mappers).
- Janela com menus do fMSX, zoom, proporcao, tela cheia, filtros de video.
- Banco de ROMs (SQLite) com downloads do fMSX 6.0, do file-hunter e do Vampier, CRUD e busca.
- Pacotes da 1.17.0 gerados em `dist/`: `fwMSX-1.17.0.zip` e `fwMSX-1.17.0-linux.tar.gz`.

### Compilado mas NAO validado na tela
- Menu **ROMs** e janelas **Banco de ROMs** / **Navegar file-hunter**.
- Janelas **Configuracao de disco** e **Configuracao de slots**.
- Tela cheia, 4:3, 16:9, filtros de video (validados so' por codigo).

### Nao funciona / limites conhecidos
- Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada (causa nao diagnosticada);
  Mega Chase validado so' ate o titulo; F-1 Spirit 3D: troca de disco pela janela nao testada.
- Som do FM e do SCC nao comparado com hardware real (constantes do OPLL sao estimativas).
- `CALL VOICECOPY` nao e' aceito pela ROM do fMSX; status/timers do OPLL nao emulados.
- BIOS Expert: a tela sai com espacos entre as letras ("G r a d i e n t e"). Nao investigado.
- Sem save-state, GameMaster2, MSX-DOS 2, efeitos de rastreio no meio do quadro, cassete (fita).
- Cores YJK do V9958 nao conferidas com hardware real.

## 3. Como compilar e testar

```powershell
# Windows (MSYS2 UCRT64 em C:\msys64). Gera dist\fwMSX.exe, dist\msxdisk.exe e dist\fwMSX-X.Y.Z.zip
.\build.ps1
cd build; ctest            # 15 suites
```

```bash
# Linux/WSL (Ubuntu): gera dist/fwMSX e o tar.gz. Usa build-linux/
wsl -d Ubuntu-26.04 -- bash -lc 'cd /mnt/c/dos/fwMSX && ./build.sh'
```

Build incremental de um alvo: `cmake --build build --target fwMSX` (ou `romdbtest`, `machinetest`,
`fdctest`, `memmaptest`, `msx2test`, `fmtest`, `scctest`, `psgtest`, `ppitest`, `vdptest`, `vdp2test`).

**Midias (nao versionadas por completo):** `dist/roms/` tem o banco de ROMs baixado. Esta pasta
e' so' local (tem 116 MB no PC de hoje); so' parte dela vai ao git (ver secao 6). Os testes que precisam
da ROM DDX pulam se ela nao estiver la (`machinetest`, secao 3e).

Para baixar as midias de novo (no PC novo):
```powershell
.\dist\fwMSX.exe --romdb fmsx                     # fMSX 6.0 Windows (BIOS, DISK.ROM, CARTS.SHA...)
.\dist\fwMSX.exe --romdb filehunter-full          # System ROMs (Full Set mais recente, 116 MB)
.\dist\fwMSX.exe --romdb vampier                  # banco do Vampier (referencia de nomes)
.\dist\fwMSX.exe --romdb cartsha roms\tabelas\CARTS.SHA
.\dist\fwMSX.exe --romdb identify
```

## 4. Mapa do codigo (onde mexer)

| Area | Caminho | Doc |
|---|---|---|
| Maquina, layout de slots, disco, mistura de audio | `src/machine/` (`machine.h/.cpp`, `cli.cpp`, `gui/emu_window.cpp`, `gui/rom_manager.cpp`) | `machine-spec.md`, `slots-spec.md` |
| Mapa de memoria (slots, mappers, SRAM, RAM em paginas) | `src/memmap/` | `memory-map-spec.md`, `sram-spec.md` |
| Z80 | `src/z80/` | `z80-core-spec.md` |
| VDP (V9938/V9958) | `src/vdp/` | `vdp-spec.md`, `msx2-spec.md`, `msx2p-spec.md` |
| PSG / SCC / FM (OPLL) | `src/psg/`, `src/scc/`, `src/fm/` | `psg-spec.md`, `scc-spec.md`, `fm-spec.md` |
| Disco (WD2793, formatos, porta Microsol) | `src/fdc/` | `fdc-spec.md` (secao 6) |
| Banco de ROMs (SQLite, downloads, CLI) | `src/romdb/` | `romdb-spec.md` |
| PPI / teclado | `src/ppi/` | `ppi-spec.md` |
| Audio ao vivo | `src/audio/` | `audio-spec.md` |
| msxdisk (utilitario de imagens) | `src/msxdisk/`, `tools/msxdisk/` | `msxdisk-spec.md` |
| Testes | `tests/z80/*.cpp`, `tests/romdb/romdb_test.cpp` | (CTest em `CMakeLists.txt`) |
| Terceiros (so' estudo) | `resource/` (fMSX, openMSX, CLK, makeTSX, fmsxgo...) | `resource/README.md` |

Versao: `src/common/version.h` (fonte unica). Nome do jogo + subtitulo a cada versao.

## 5. Pegadinhas (leia antes de mexer)

- **Quebras de linha:** o repositorio guarda LF; o Git avisa "LF will be replaced by CRLF" ao tocar nos
  arquivos. E' normal. Scripts de edicao devem normalizar para LF ao casar ancoras.
- **Scripts de edicao:** evite `python -c "..."` com aspas, crases ou `\n` dentro do shell (isso ja'
  corrompeu arquivos). Use um arquivo `.py` com string bruta.
- **WSL a partir do Git Bash:** `MSYS_NO_PATHCONV=1 wsl -d Ubuntu-26.04 -- bash /mnt/c/...` (senao o
  caminho e' reescrito).
- **Teclado do `--keys`:** SHIFT+6 = `^`, SHIFT+7 = `&`, SHIFT+8 = `*`, SHIFT+9 = `(`, SHIFT+0 = `)`,
  SHIFT+2 = `@`, SHIFT+' = `"`. Ja' corrigido; nao "corrigir" de volta.
- **Teste de disco:** `msxdos1.dsk` (na raiz, rastreado) e' o disco de teste. Os testes copiam para
  temp. Se o `copy` de um teste manual gravar nele, restaure com `git checkout -- msxdos1.dsk`.
- **Branch:** `main` = releases (1.17.0 publicado). Trabalho novo em `estudo/openmsx` (hoje). Decidir o merge.
- **`dist/`:** os `.exe` de teste sao ignorados pelo `.gitignore` (regras explicitas). `dist/fwMSX.exe`
  esta rastreado e foi regenerado no build de hoje.
- **Nao usar `find /`** nem buscas amplas: demoram minutos (ja' aconteceu).
- **Saidas com `2>&1 | tail`:** comandos longos (build, ctest) passam de 2 min; use `run_in_background`
  e espere com um `until grep -q ...` quando precisar do resultado.

## 6. O que vai ao git e o que fica no PC

- **Vai (provisorio, pessoal):** codigo, docs, testes; `resource/` inteiro (fMSX, openMSX, etc.);
  `dist/roms/` **parte**: `bios/`, `interfaces/`, `tabelas/` (fMSX), os drivers
  `ddx_3.0.rom` e `cdx-2.rom`, e `roms.db`.
- **Fica so' no PC:** o restante de `dist/roms/` (Full Set do file-hunter, `filehunter/`, ~116 MB), e as
  pastas de trabalho. Sem regra de `.gitignore` para isso ainda; os arquivos aparecem como nao rastreados.
- **Quando liberar ao publico:** revisar midias de terceiros (`LICENSE-THIRD-PARTY.md`, politica); os
  arquivos contestados saem do repositorio e dos pacotes.

## 7. Proximos passos (em ordem sugerida)

1. **Validar na tela** o que so' foi compilado: menu ROMs, Banco de ROMs, Navegar file-hunter,
   Configuracao de disco (incluindo o seletor de ROM do driver), Configuracao de slots, tela cheia, filtros.
2. **Merge** de `estudo/openmsx` no `main` (fast-forward) se a validacao passar. Depois, tag e release 1.18.0.
3. **Banco de ROMs:** conferir as ROMs baixadas contra o SHA-1 conhecido; usar o banco para escolher o
   mapper ao carregar cartucho (`CARTS.SHA` ja' importado); importar o JSON do Vampier se for util.
4. **Fitas (SPEC 5.2, ordem):** (a) leitor TZX/TSX e CAS sem janela; (b) CAS por hooks da BIOS
   (`BLOAD`/`CLOAD`); (c) escritor de TSX a partir de `.BIN`/`.BAS`; (d) porta de cassete no PPI e
   pulsos; (e) banco de fitas com metadados (sem download automatico ate ter autorizacao, ou com o
   proprio usuario iniciando); (f) port do makeTSX (WAV -> TSX, MIT); (g) CLI `--fita` e menu "Fita".
   Referencias: `resource/makeTSX/` (MIT), `resource/CLK/` (MIT), `resource/openMSX_TSXadv/` (GPL, so' estudo).
5. **FM:** ouvir o WAV (`--wav`) contra referencia; `CALL VOICECOPY`; status/timers do OPLL.
6. **Disco:** formatar disquetes; modelar FM/MFM; formatos independentes para A e B; estudar o driver
   de Sony/Philips/Spectravideo do openMSX (so' como referencia).
7. **Controle externo, estilo openMSX:** canal de controle em localhost (`status`, `reset`, `pause`,
   `type`, `cart`, `disk`, `screenshot`, `peek`/`poke`, `quit`); a thread so' enfileira comandos.
8. **Jogos:** Lode Runner + SCC; Parodius (tela fragmentada); Mega Chase; F-1 Spirit 3D (troca de disco).
9. **BIOS Expert:** texto com espacos na tela; investigar.
10. **Save-state** (PSG, SCC, OPLL, disco, VDP); **rastreio** no meio do quadro; **CPU no pior caso**.
11. **Layout de slots:** salvar/carregar em arquivo; perfis no banco.
12. **Cartuchos:** MSX-DOS 2, GameMaster2, MSX-MUSIC com BIOS propria.
13. **Depois:** frontend para jogar (biblioteca de jogos sobre o banco); integracao com o msxide (MSX-PoorManOS).

## 8. Onde esta cada decisao

- Layout de slots e regras de RAM/mapper: `doc/slots-spec.md`.
- Disco por portas (convencao Microsol, mapa de bits, drivers DDX/CDX): `doc/fdc-spec.md`, secao 6.
- Banco de ROMs (esquema, downloads, CLI): `doc/romdb-spec.md`.
- FM (OPLL, FM-PAC, comandos de BASIC): `doc/fm-spec.md`.
- Fitas (viabilidade, termos do site TSX, ordem de implementacao): `doc/SPEC.md`, secao 5.2.
- Politica de midias e licencas: `LICENSE-THIRD-PARTY.md`.
