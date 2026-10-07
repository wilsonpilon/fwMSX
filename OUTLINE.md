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

- **Versao em preparo:** 1.20.1 "King's Valley: Gravacao em K7: Corrigindo o Piloto" -- codigo e docs
  prontos, build e testes passando (ver secao 9); ainda NAO comitada/taggeada se esta nota ainda estiver
  aqui.
- **Ultima versao publicada em `main`:** 1.19.1. A 1.19.0 (fita, leitura) **nunca foi comitada** -- o
  usuario achou dois bugs reais ao testar com um .TSX de verdade antes do commit, corrigidos direto na
  1.19.1, e depois confirmou pelo ouvido/jogando: carregou o jogo completo (`A.M.C.`, Dinamic) e jogou
  um pouco, som nitido. Pediu a melhoria seguinte (gravacao, fita nova, protecao, 3 modos, marcar o
  ponto, contagiros) -- isso virou a 1.20.0 (minor, nao "1.19.2" como ele sugeriu, pela politica do
  projeto). **A 1.20.0 TAMBEM nunca foi comitada**: o usuario testou pela janela de verdade (criou uma
  fita nova, gravou `CSAVE"TESTE"`, rebobinou, deu `CLOAD` no modo normal) e achou outro bug real --
  ouvia o chiado do piloto, mas o programa nunca carregava. Corrigido direto na 1.20.1 (ver secao 9 e
  `doc/CHANGELOG.md`): os pulsos de ZERO e UM do bloco #4B estavam TROCADOS (a convencao certa do MSX
  e' zero = 2x o pulso de um, confirmada no proprio gerador do makeTSX), e o piloto era curto demais
  (2000 pulsos, ~0,48s) para a BIOS de verdade calibrar -- subido para 8000 (~1,9s).
- **Branch:** trabalhe direto em `main`. `estudo/openmsx` ja' foi mesclada (pode apagar).
- **Testes:** `ctest` com **16 suites**, todas passando (`tape_load` com **71 checagens**: 22 de
  gravacao/protecao/modos/marcacao/contagiros + 3 sobre a relacao zero/um/piloto).
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
- **Fita, leitura/carregamento** (.CAS e .TSX/.TZX): leitura completa (bloco #4B do TZX com pulsos e
  dados; os outros blocos pulados com seguranca); carregamento rapido (gancho de BIOS, sem som) e
  normal (pulsos de verdade, com som). **Validado de ponta a ponta pelo usuario em 2026-10-07** com um
  .TSX real (`resource/fmsxgo/media/*.tsx`, "A.M.C.", Dinamic 1990): inseriu pela janela, viu a "Fita
  K7", carregou o jogo completo (modo normal) e OUVIU o barulho do carregamento ("bem nitido"), e jogou
  um pouco depois de carregar. Primeira feature deste projeto validada por jogo completo.
- **Fita, gravacao (1.20.0/1.20.1, 2026-10-07/08)**: `CSAVE`/`BSAVE "CAS:"` sempre pelo gancho de BIOS;
  fita nova em branco (.tsx); protecao contra gravacao por padrao (fita de arquivo trava, fita nova
  nao); 3 modos de gravacao (incluir no final, sobrescrever o ponto marcado, nova fita); marcar um
  arquivo da lista (janela "Fita K7", clicavel) como ponto de carga/gravacao; contagiros simulado.
  **Validado pelo usuario na janela de verdade em 2026-10-08**: criou uma fita nova, gravou
  `CSAVE"TESTE"`, rebobinou e confirmou `CLOAD` nos dois modos (depois da correcao do piloto -- ver
  acima). Primeira gravacao de fita deste projeto confirmada de ponta a ponta por um humano.
- Pacotes gerados em `dist/`: `fwMSX-1.19.1.zip`/`.tar.gz` (publicados) e `fwMSX-1.20.1.*` (ver secao 9).

### Nao funciona / limites conhecidos
- Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) mostra tela fragmentada (causa nao diagnosticada);
  Mega Chase validado so' ate o titulo; F-1 Spirit 3D: troca de disco pela janela nao testada.
- Som do FM e do SCC nao comparado com hardware real (constantes do OPLL sao estimativas); o som da
  fita (modo normal) ja' foi ouvido pelo usuario (nitido, jogo completo), mas nunca comparado lado a
  lado com um gravador/MSX de verdade.
- `CALL VOICECOPY` nao e' aceito pela ROM do fMSX; status/timers do OPLL nao emulados.
- BIOS Expert: a tela sai com espacos entre as letras ("G r a d i e n t e"). Nao investigado.
- Fita: sem banco de fitas nem download, sem navegar blocos de controle do TZX (grupos/lacos/saltos),
  sem a ferramenta de linha de comando para empacotar .BIN/.BAS em .TSX sem o emulador -- ver
  `doc/tape-spec.md`, secao 5. A sintese de pulsos do modo normal trata o preenchimento de alinhamento
  (zeros antes de um cabecalho) como dado de verdade -- so' afeta a REPRODUCAO por pulsos de uma fita
  GRAVADA por este emulador, o modo rapido nao sofre com isso.
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
2. **[FEITO em 2026-10-07/08, 1.20.0/1.20.1] Gravacao em fita** -- a melhoria que o usuario pediu:
   CSAVE/BSAVE, fita nova, protecao, 3 modos de gravacao, marcar o ponto, contagiros; e o bug do
   piloto (zero/um trocados + piloto curto) que o usuario achou testando pela janela, corrigido no
   mesmo dia. Ver secao 2 e `doc/tape-spec.md`, secoes 6-7. **Ainda nao testados na janela:** os 3
   modos de gravacao (so' "incluir no final" foi usado pelo usuario), marcar/desmarcar na lista, o
   contagiros girando visualmente.
3. **Fitas, o que ainda falta** (`doc/SPEC.md`, secao 5.2): ferramenta de linha de comando para
   empacotar `.BIN`/`.BAS` em `.TSX` sem passar pelo emulador (resto do item c); (e) banco de fitas
   (metadados, sem download automatico ate ter autorizacao); (f) port do makeTSX (WAV -> TSX, MIT);
   navegar os blocos de controle do TZX (grupos/lacos/saltos); corrigir a sintese de pulsos do modo
   normal para nao tratar o preenchimento de alinhamento como dado de verdade (ver
   `doc/tape-spec.md`, secao 5). Referencias: `resource/makeTSX/` (MIT), `resource/CLK/` (MIT),
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

**Depois disso, o usuario pediu a melhoria no sistema de fitas**: gravacao (CSAVE/BSAVE "CAS:"), fita
nova em branco (.tsx), protecao contra gravacao por padrao (fita de arquivo trava, fita nova nao),
tres modos de gravacao (incluir no final/sobrescrever o ponto marcado/nova fita), marcar um arquivo da
lista na janela "Fita K7" como ponto de carga/gravacao, e um contagiros simulado. Tudo implementado e
testado (`tapetest`, 68 checagens, 16/16 no `ctest`) nesta mesma sessao, escrito como versao `1.20.0`
(o usuario sugeriu "1.19.2 ou algo assim"; expliquei que a politica do projeto -- Y sobe a cada
feature nova -- pede uma MINOR aqui, nao um patch).

**O usuario testou pela janela de verdade ANTES do commit** (como fez com a fita de leitura): criou
uma fita nova, escreveu um programinha pequeno, deu `CSAVE"TESTE"`, rebobinou e tentou `CLOAD` no modo
normal. Ouviu o chiado do piloto, mas o programa nunca carregava -- achou um bug real de novo. Causa
(ver `doc/CHANGELOG.md`, `[1.20.1]`, e o comentario em `src/tape/cpp/cas_format.h`): os pulsos de ZERO
e UM do bloco #4B estavam TROCADOS (a convencao certa do MSX e' zero = 2x o pulso de um, confirmada
no proprio gerador do makeTSX, `resource/makeTSX/rippers/MSX4B_Ripper.h/.cpp` -- a nota antiga do
SPEC.md vinha dos defaults GENERICOS de ZX Spectrum, nunca verificada contra o codigo certo), e o
piloto era curto demais (2000 pulsos, ~0,48s) para a BIOS real calibrar -- subido para 8000 (~1,9s).
Corrigido no mesmo dia (3 checagens novas em `tapetest`), confirmado regravando o `teste.tsx` do
proprio usuario e testando `CLOAD` nos dois modos -- escrito como `1.20.1`.

**Se o `git log` de `main` nao mostrar um commit de release da 1.20.1 ainda**, essa e' a proxima acao:
`git add` dos arquivos certos (nao `git add -A`), commit com `Co-Authored-By`, `git tag 1.20.1`,
`git branch v1.20.1`, `git push origin main 1.20.1 v1.20.1`.

**Ainda nao testados na janela:** os 3 modos de gravacao (so' "incluir no final", o padrao, foi
usado); marcar/desmarcar um arquivo na lista; o contagiros girando visualmente.

**Licoes para a proxima vez:**
- Ao implementar um leitor/escritor de formato binario a partir so' da especificacao escrita (sem um
  arquivo real para testar), desconfiar de deslocamentos em hexadecimal que "parecem" decimais (ex.:
  "0x10" = 16, nao 10) e de qualquer suposicao de alinhamento entre partes concatenadas de tamanho
  variavel. Testar contra pelo menos um arquivo real do formato, nao so' contra casos sinteticos que o
  proprio autor do teste construiu (eles tendem a repetir os mesmos enganos do codigo que testam).
- **Uma nota do SPEC.md dizendo "conferir no codigo antes de usar" e' um aviso serio, nao decoracao.**
  Os valores de pulso ZERO/UM ficaram errados por TRES versoes (1.19.0 a 1.20.0) porque essa
  verificacao nunca foi feita antes de escrever `cas_format.h` -- os defaults que eu copiei eram de
  OUTRO bloco (#10/#11, ZX Spectrum generico), nao do #4B (MSX) que o codigo realmente usa. Quando uma
  nota antiga desse tipo aparecer, ir direto na fonte (aqui, `resource/makeTSX/rippers/
  MSX4B_Ripper.h/.cpp`) antes de copiar o numero para um lugar novo.
- Testar SO' a leitura (com arquivos de terceiros) nao prova que a ESCRITA esta' certa -- os dois usam
  os mesmos campos do formato, mas um bug na GERACAO so' aparece ao reler o que o proprio codigo
  escreveu. O modo RAPIDO tambem escondeu esse bug (nao usa pulso nenhum): so' apareceu testando o
  MODO NORMAL de uma fita GRAVADA, nao de uma fita so' lida.
