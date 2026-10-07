# OUTLINE -- fwMSX: onde paramos e o que fazer depois

> Leia este arquivo primeiro. Ele foi escrito para retomar o trabalho em outro computador,
> ou com outra IA, sem perder o contexto. Atualizado em **2026-10-07**.

## 1. O que e' o projeto

**fwMSX** e' um emulador de MSX (MSX1, MSX2, MSX2+) feito como projeto de estudo, a partir do
fMSX (Marat Fayzullin), com o aval do autor original para a adaptacao. Regra do projeto: cada fase
relevante exercita **C, C++, Assembly (NASM, dual-ABI Win64/SysV) e Fortran**, mesmo que minimamente.

Objetivo de medio prazo: integrar o fwMSX ao **msxide** num utilitario de desenvolvimento para MSX
chamado **MSX-PoorManOS**. Ainda sem data.

Este repositorio e' **pessoal**. Midias de terceiros (ROMs, discos, fitas) podem ser versionadas por
enquanto; antes de liberar ao publico, cada midia contestada sera removida (ver `LICENSE-THIRD-PARTY.md`).

## 2. Onde estamos (2026-10-07)

- **Ultima versao publicada em `main`:** 1.19.1 "Yie Ar Kung-Fu: Fita K7: Corrigindo o Carregamento".
  A 1.19.0 (fita, primeira versao) **nunca foi comitada** -- o usuario achou dois bugs reais ao testar
  com um .TSX de verdade antes do commit, corrigidos direto na 1.19.1 (ver `doc/CHANGELOG.md`), e
  depois **confirmou pelo ouvido/jogando**: carregou o jogo completo (`A.M.C.`, Dinamic) e jogou um
  pouco, som da fita nitido. **Proximo passo avisado pelo usuario: melhoria no sistema de fitas,
  provavelmente uma 1.19.2** -- ainda sem detalhe do que e'; pedir ao usuario se nao estiver registrado
  mais abaixo nesta secao quando esta sessao for retomada.
- **Branch:** trabalhe direto em `main`. `estudo/openmsx` ja' foi mesclada (pode apagar).
- **Testes:** `ctest` com **16 suites**, todas passando (inclui `tape_load`, novo, com 46 checagens).
- **Documentos vivos:** `doc/SPEC.md` (secao 5.0 = estado atual; 5.2 = fitas), `doc/CHANGELOG.md`,
  `doc/RELEASE.md`, `doc/MANUAL.md`, e os `*-spec.md` (inclui `doc/tape-spec.md`, novo).

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
- Janela com menus do fMSX, zoom, proporcao, tela cheia, filtros de video -- **confirmados na tela pelo
  usuario em 2026-10-07**, assim como o menu ROMs, Banco de ROMs, Navegar file-hunter, Configuracao de
  disco e de slots.
- Banco de ROMs (SQLite) com downloads do fMSX 6.0, do file-hunter e do Vampier, CRUD e busca.
- **Fita** (.CAS e .TSX/.TZX): leitura completa (bloco #4B do TZX com pulsos e dados; os outros blocos
  pulados com seguranca); carregamento rapido (gancho de BIOS, sem som) e normal (pulsos de verdade,
  com som); menu "Fita" e janela visual "Fita K7". Ver `doc/tape-spec.md`. **Validado de ponta a ponta
  pelo usuario em 2026-10-07** com um .TSX real (`resource/fmsxgo/media/*.tsx`, "A.M.C.", Dinamic
  1990): inseriu pela janela, viu a "Fita K7", carregou o jogo completo (modo normal) e OUVIU o
  barulho do carregamento ("bem nitido"), e jogou um pouco depois de carregar. Primeira feature deste
  projeto validada por jogo completo, nao so' por tela de boot.
- Pacotes publicados em `dist/`: `fwMSX-1.19.1.zip`/`.tar.gz` (`main`).

### Nao funciona / limites conhecidos
- Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada (causa nao diagnosticada);
  Mega Chase validado so' ate o titulo; F-1 Spirit 3D: troca de disco pela janela nao testada.
- Som do FM e do SCC nao comparado com hardware real (constantes do OPLL sao estimativas); o som da
  fita (modo normal) ja' foi ouvido pelo usuario (nitido, jogo completo), mas nunca comparado lado a
  lado com um gravador/MSX de verdade.
- `CALL VOICECOPY` nao e' aceito pela ROM do fMSX; status/timers do OPLL nao emulados.
- BIOS Expert: a tela sai com espacos entre as letras ("G r a d i e n t e"). Nao investigado.
- Fita: sem escrita (CSAVE/BSAVE), sem banco de fitas nem download, sem navegar blocos de controle do
  TZX (grupos/lacos/saltos) -- ver `doc/tape-spec.md`, secao 5.
- Sem save-state, GameMaster2, MSX-DOS 2, efeitos de rastreio no meio do quadro.
- Cores YJK do V9958 nao conferidas com hardware real.

## 3. Como compilar e testar

```powershell
# Windows (MSYS2 UCRT64 em C:\msys64). Gera dist\fwMSX.exe, dist\msxdisk.exe e dist\fwMSX-X.Y.Z.zip
.\build.ps1
cd build; ctest            # 16 suites
```

```bash
# Linux/WSL (Ubuntu): gera dist/fwMSX e o tar.gz. Usa build-linux/
wsl -d Ubuntu-26.04 -- bash -lc 'cd /mnt/c/dos/fwMSX && ./build.sh'
```

Build incremental de um alvo: `cmake --build build --target fwMSX` (ou `romdbtest`, `machinetest`,
`fdctest`, `memmaptest`, `msx2test`, `fmtest`, `scctest`, `psgtest`, `ppitest`, `vdptest`, `vdp2test`,
`tapetest`).

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
| Mapa de memoria (slots, mappers, SRAM, RAM em paginas, patch de ROM da fita) | `src/memmap/` | `memory-map-spec.md`, `sram-spec.md` |
| Z80 (inclui o gancho "ED FE" usado pela fita) | `src/z80/` | `z80-core-spec.md` |
| VDP (V9938/V9958) | `src/vdp/` | `vdp-spec.md`, `msx2-spec.md`, `msx2p-spec.md` |
| PSG (inclui a entrada de cassete, R14) / SCC / FM (OPLL) | `src/psg/`, `src/scc/`, `src/fm/` | `psg-spec.md`, `scc-spec.md`, `fm-spec.md` |
| Disco (WD2793, formatos, porta Microsol) | `src/fdc/` | `fdc-spec.md` (secao 6) |
| **Fita** (.CAS, .TSX/.TZX, gancho de BIOS, pulsos) | `src/tape/` | `tape-spec.md` |
| Banco de ROMs (SQLite, downloads, CLI) | `src/romdb/` | `romdb-spec.md` |
| PPI / teclado (inclui o motor da fita) | `src/ppi/` | `ppi-spec.md` |
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
- **Branch:** trabalhe direto em `main`. `estudo/openmsx` ja' foi mesclada e pode ser apagada.
- **`dist/`:** os `.exe` de teste sao ignorados pelo `.gitignore` (regras explicitas, inclusive
  `tapetest`). `dist/fwMSX.exe` esta rastreado e precisa ser regenerado a cada release.
- **Gancho "ED FE" do Z80**: existia no nucleo desde antes (`bus->patch`, ver
  `src/z80/core/opcodes_ed.h`), mas nunca tinha sido usado -- a fita foi o primeiro uso. Se outra feature
  precisar interceptar outra rotina da BIOS, o mecanismo e' esse (so' um gancho por barramento por
  enquanto -- `SlotMemoryBus::AttachTapeHook()` -- um segundo uso precisaria de uma lista/despachante).
- **Entrada de cassete NAO fica no PPI** -- fica no bit 7 do R14 do PSG (confirmado no openMSX,
  `MSXPSG.cc`). Facil de errar se for por memoria sem checar a fonte de novo.
- **Nao usar `find /`** nem buscas amplas: demoram minutos (ja' aconteceu).
- **Saidas com `2>&1 | tail`:** comandos longos (build, ctest) passam de 2 min; use `run_in_background`
  e espere com um `until grep -q ...` quando precisar do resultado.

## 6. O que vai ao git e o que fica no PC

- **Vai agora (provisorio, pessoal):** codigo, docs, testes; `resource/` inteiro (fMSX, openMSX, CLK,
  makeTSX, openMSX_TSXadv, etc.); `dist/roms/` **inteiro**, inclusive o Full Set do file-hunter
  (`filehunter/15-08-2026/`, ~116 MB) e `roms.db`.
- **Sem regra de `.gitignore` para midias.** Decisao do usuario: isso sera' revertido na versao final,
  quando as midias de terceiros deixarem o repositorio.
- **Clonar em outro PC:** `git clone` do `main` ja' traz tudo; nao precisa baixar as ROMs de novo.
- **Quando liberar ao publico:** revisar midias de terceiros (`LICENSE-THIRD-PARTY.md`, politica); os
  arquivos contestados saem do repositorio e dos pacotes.

## 7. Proximos passos (em ordem sugerida)

1. **[FEITO em 2026-10-07] Ver a janela "Fita K7" e ouvir o modo normal** -- o usuario confirmou:
   carregou o jogo completo (`A.M.C.`, Dinamic) pelo modo normal, ouviu o barulho do carregamento
   ("bem nitido") e jogou um pouco. Ver secao 2.
2. **Melhoria no sistema de fitas pedida pelo usuario (2026-10-07), provavelmente 1.19.2** -- o
   usuario avisou que o proximo passo e' uma melhoria no sistema de fitas, SEM detalhar ainda o que' e.
   Se esta nota ainda estiver aqui sem mais detalhe quando a sessao for retomada, **perguntar ao
   usuario o que ele tem em mente** antes de supor (pode ser qualquer um dos itens 3/5 abaixo, ou algo
   novo que ele so' comentou de boca).
3. **Fitas, o que falta** (`doc/SPEC.md`, secao 5.2): (c) escritor de TSX a partir de `.BIN`/`.BAS`;
   (e) banco de fitas (metadados, sem download automatico ate ter autorizacao); (f) port do makeTSX
   (WAV -> TSX, MIT); navegar os blocos de controle do TZX (grupos/lacos/saltos); saida de cassete
   (CSAVE). Referencias: `resource/makeTSX/` (MIT), `resource/CLK/` (MIT),
   `resource/openMSX_TSXadv/` (GPL, so' estudo).
4. **Banco de ROMs:** conferir as ROMs baixadas contra o SHA-1 conhecido; usar o banco para escolher o
   mapper ao carregar cartucho (`CARTS.SHA` ja' importado); importar o JSON do Vampier se for util.
5. **FM e fita:** ouvir o WAV (`--wav`) e o modo normal da fita contra referencia (hardware real, nao
   so' "parece certo"); `CALL VOICECOPY`; status/timers do OPLL.
6. **Disco:** formatar disquetes; modelar FM/MFM; formatos independentes para A e B; estudar o driver
   de Sony/Philips/Spectravideo do openMSX (so' como referencia).
7. **Controle externo, estilo openMSX:** canal de controle em localhost (`status`, `reset`, `pause`,
   `type`, `cart`, `disk`, `fita`, `screenshot`, `peek`/`poke`, `quit`); a thread so' enfileira comandos.
   Ainda nao comecou.
8. **Jogos:** Lode Runner + SCC; Parodius (tela fragmentada); Mega Chase; F-1 Spirit 3D (troca de disco).
9. **BIOS Expert:** texto com espacos na tela; investigar.
10. **Save-state** (PSG, SCC, OPLL, disco, fita, VDP); **rastreio** no meio do quadro; **CPU no pior caso**.
11. **Layout de slots:** salvar/carregar em arquivo; perfis no banco.
12. **Cartuchos:** MSX-DOS 2, GameMaster2, MSX-MUSIC com BIOS propria.
13. **Depois:** frontend para jogar (biblioteca de jogos sobre o banco) com fitas E discos; integracao
    com o msxide (MSX-PoorManOS).

## 8. Onde esta cada decisao

- Layout de slots e regras de RAM/mapper: `doc/slots-spec.md`.
- Disco por portas (convencao Microsol, mapa de bits, drivers DDX/CDX): `doc/fdc-spec.md`, secao 6.
- Banco de ROMs (esquema, downloads, CLI): `doc/romdb-spec.md`.
- FM (OPLL, FM-PAC, comandos de BASIC): `doc/fm-spec.md`.
- **Fita** (.CAS, .TSX/.TZX, enderecos da BIOS, layout do bloco #4B, pulsos, limites): `doc/tape-spec.md`.
- Politica de midias e licencas: `LICENSE-THIRD-PARTY.md`.

## 9. Nota de fechamento desta sessao (2026-10-07)

Os itens 2 (release 1.18.0) e 3 (fita) do pedido do usuario foram feitos e fechados nesta sessao.
1.18.0 foi comitada/taggeada/com push. A fita (item 3) foi implementada, testada e documentada como
`1.19.0` -- mas o usuario testou com um `.tsx` real (`resource/fmsxgo/media/*.tsx`) **antes de
qualquer commit** e achou dois bugs reais (janela "Fita K7" mostrando vazia; `RUN"CAS:"` com "Device
I/O error" ou travando a maquina no modo normal). Os dois foram corrigidos no mesmo dia (ver
`doc/CHANGELOG.md`, `[1.19.1]`, e os comentarios em `src/tape/cpp/tzx_reader.cpp` nos casos `0x35` e
`0x4B`) e tem teste de regressao em `tapetest`. A versao 1.19.0 **nunca foi comitada**; a 1.19.1 (ja'
corrigida) foi comitada, taggeada e com push para `main` (commit `2fb6d23`). Depois disso o usuario
**confirmou pelo ouvido e jogando**: carregou o jogo completo e jogou um pouco, som nitido -- a sessao
foi encerrada com `main` em dia, sem nada pendente de commit (so' esta atualizacao final do
`OUTLINE.md` registrando a confirmacao).

**O usuario avisou que o proximo passo (quando a sessao for retomada) e' uma melhoria no sistema de
fitas, possivelmente virando a 1.19.2** -- sem detalhar ainda o que' e'. Pergunte a ele qual melhoria
tem em mente antes de supor.

**Licao para a proxima vez:** ao implementar um leitor de formato binario a partir so' da
especificacao escrita (sem um arquivo real para testar), desconfiar de deslocamentos em hexadecimal
que "parecem" decimais (ex.: "0x10" = 16, nao 10) e de qualquer suposicao de alinhamento entre partes
concatenadas de tamanho variavel. Testar contra pelo menos um arquivo real do formato, nao so' contra
casos sinteticos que o proprio autor do teste construiu (eles tendem a repetir os mesmos enganos do
codigo que testam).
