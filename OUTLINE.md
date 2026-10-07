# OUTLINE -- fwMSX: onde paramos e o que fazer depois

> Leia este arquivo primeiro. Ele foi escrito para retomar o trabalho em outro computador,
> ou com outra IA, sem perder o contexto. Atualizado em **2026-10-08**.

## 1. O que e' o projeto

**fwMSX** e' um emulador de MSX (MSX1, MSX2, MSX2+) feito como projeto de estudo, a partir do
fMSX (Marat Fayzullin), com o aval do autor original para a adaptacao. Regra do projeto: cada fase
relevante exercita **C, C++, Assembly (NASM, dual-ABI Win64/SysV) e Fortran**, mesmo que minimamente.

Objetivo de medio prazo: integrar o fwMSX ao **msxide** num utilitario de desenvolvimento para MSX
chamado **MSX-PoorManOS**. Ainda sem data.

Este repositorio e' **pessoal**. Midias de terceiros (ROMs, discos, fitas) podem ser versionadas por
enquanto; antes de liberar ao publico, cada midia contestada sera removida (ver `LICENSE-THIRD-PARTY.md`).

## 2. Onde estamos (encerrado em 2026-10-08, pronto para continuar amanha)

- **Ultima versao publicada em `main`:** **1.23.0** "King's Valley: Navegacao de Blocos do TZX"
  -- commit `527f763`, tag `1.23.0`, branch `v1.23.0`, tudo com push feito. `git status` limpo,
  `main` local == `origin/main`. Nao ha' nada pendente de commit/push desta sessao.
- **Branch:** trabalhe direto em `main`. `estudo/openmsx` ja' foi mesclada (pode apagar).
- **Testes:** `ctest` com **17 suites**, todas passando em Windows E Linux (WSL) -- inclui
  `tape_load` (leitura/gravacao de fita + navegacao de TZX) e `cas_pack` (empacotador + ripper de
  WAV, novo nesta sessao).
- **Documentos vivos, todos sincronizados com a 1.23.0 nesta sessao:** `README.md`,
  `doc/MANUAL.md`, `doc/SPEC.md` (secao 5.0 = estado atual), `doc/CHANGELOG.md`, `doc/RELEASE.md`,
  `doc/tape-spec.md` (secoes 1-10) e este arquivo.
- **Resumo da sessao de hoje (ver secao 9 para o detalhamento completo, rodada por rodada):**
  corrigidos 3 bugs reais de gravacao achados pelo usuario testando pela janela (1.20.1 a 1.20.3:
  piloto/ZERO/UM trocados, `Device I/O error` ao reinserir fita destravada, preenchimento de
  alinhamento gravado como dado, corte repetido em "sobrescrever o ponto"); depois, os 3 itens do
  "passo 1" pedido pelo usuario: empacotador `.BIN`/`.BAS` -> `.TSX` (`fwmsx --cas pack`, 1.21.0);
  port do makeTSX, WAV -> TSX (`fwmsx --cas rip`, 1.22.0); navegacao de verdade dos blocos de
  controle do TZX (grupos/lacos/saltos/chamadas/selecao, 1.23.0). O "passo 1" esta' COMPLETO.

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
- **Fita, 3 bugs da 1.20.2 (2026-10-08)**: o usuario testou os outros dois modos de gravacao
  (sobrescrever/nova fita) e a marcacao de ponto pela janela de verdade e achou dois bugs reais
  ("Device I/O error" ao trocar o modo depois de marcar um ponto) mais o contagiros parecendo
  "parado" no CLOAD. Investigados e corrigidos no mesmo dia -- ver secao 9 e `doc/CHANGELOG.md`,
  `[1.20.2]`: o contagiros nao era bug; o resto era `TapeEngine::Insert()` travando a fita de novo
  ao reinserir a MESMA fita ja' destravada, e o preenchimento de alinhamento sendo gravado como
  dado de verdade ao reler uma fita gravada por este emulador.
- **Fita, bug da 1.20.3 (2026-10-08)**: logo depois de testar a 1.20.2, o usuario testou
  "sobrescrever o ponto marcado" de verdade e achou mais um bug: o programa gravado "sumia" por
  completo (nem o antigo nem o novo apareciam). Causa: um CSAVE de verdade chama TAPOON/TAPOOF
  duas vezes (nome + dados), e o corte reaplicava nas duas, apagando o cabecalho que a 1a chamada
  tinha acabado de escrever. Corrigido fazendo o corte rodar uma so' vez. Ver secao 9 e
  `doc/CHANGELOG.md`, `[1.20.3]`. O usuario tambem confirmou que a mudanca na UI do menu Fita
  (1.20.2) nao precisava ser desfeita -- foi um engano dele usando o menu antigo, nao um problema
  da mudanca. **Revalidado pelo usuario na janela em 2026-10-08**: gravou num ponto no meio da
  fita, sobrescrevendo um programa existente -- funcionou. So' o modo "nova fita" ainda nao foi
  testado pela janela.
- **`fwmsx --cas pack`/`list` (1.21.0, 2026-10-08)**: ferramenta de linha de comando para
  empacotar um `.BIN`/`.BAS` solto (ja no formato binario/tokenizado do MSX) num `.TSX`/`.CAS`
  valido sem passar pelo emulador -- fecha o resto do item (c) da lista de fitas pendentes.
  `--tipo bin` monta os 6 bytes de endereco (inicio/fim/exec); `--tipo bas` empacota bytes ja
  tokenizados como estao; `--anexar` acrescenta numa fita existente; `list` confirma o resultado.
  Reusa o escritor `WriteTsxFromCas()` existente. Testado via CLI direta (smoke test com `--cas
  pack`/`list` no `fwMSX.exe` de verdade) e `castooltest` (17 checagens). Ver secao 9 e
  `doc/tape-spec.md`, secao 8.
- **`fwmsx --cas rip` (1.22.0, 2026-10-08)**: port do CONCEITO do makeTSX (WAV -> TSX) --
  demodula uma gravacao real de fita (`.wav` PCM mono) detectando o piloto e decodificando os
  bytes do bloco #4B, simplificado para o caso fixo do MSX (sem os modos interativo/preditivo do
  original, ver `doc/tape-spec.md`, secao 9). `kcs_decode_byte()` novo (`src/tape/core/
  kcs_codec.{h,c}`), o inverso de `kcs_emit_byte()`. **Testado contra uma fita real de MSX dos
  anos 80** (`resource/openMSX/.../ktst31 [RUN'CAS-'].wav`, GPL, so' validacao manual local): 86
  blocos reconhecidos sem erro -- alem do round-trip automatizado (`castooltest`, bytes -> audio
  sintetico -> `rip` -> bytes).
- **Navegacao de blocos de controle do TZX (1.23.0, 2026-10-08)**: Grupo/Salto/Laco/Chamada/
  Selecao (IDs 21-28) agora sao EXECUTADOS, com um leitor em 2 passadas (indexa os blocos, depois
  navega por indice), nao so' pulados em sequencia linear como antes. Testado com 6 cenarios
  sinteticos (`tapetest`) que confirmam a ORDEM REAL de execucao. Fecha o "passo 1" pedido pelo
  usuario por completo (empacotador, ripper WAV, navegacao TZX).
- Pacotes gerados em `dist/`: `fwMSX-1.19.1.zip`/`.tar.gz` (publicados) e `fwMSX-1.23.0.*` (ver secao 9).

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
  `doc/tape-spec.md`, secao 5. O preenchimento de alinhamento gravado como dado de verdade **foi
  corrigido na 1.20.2** para fitas GRAVADAS por este emulador (via marca exata `TapeMark` por bloco);
  continua valendo so' para um `.cas` CRU carregado direto do disco, sem ter passado por uma gravacao
  deste emulador (nao ha' marca exata nesse caso, so' a busca pelo proximo cabecalho).
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
`tapetest`, `castooltest`).

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
2. **[FEITO em 2026-10-07/08, 1.20.0 a 1.20.3] Gravacao em fita** -- a melhoria que o usuario
   pediu: CSAVE/BSAVE, fita nova, protecao, 3 modos de gravacao, marcar o ponto, contagiros; o bug
   do piloto (zero/um trocados + piloto curto); os 3 bugs achados testando os outros 2 modos pela
   janela (Device I/O error ao reinserir a mesma fita destravada, preenchimento gravado como dado,
   contagiros "parado" que nao era bug); e o bug do corte repetido em "sobrescrever o ponto"
   (apagava o cabecalho que o proprio CSAVE tinha acabado de escrever). **Revalidado pelo usuario
   na janela em 2026-10-08**: gravou num ponto no meio da fita, sobrescrevendo um programa
   existente, com sucesso. Ver secao 2, secao 9 e `doc/tape-spec.md`, secoes 5-7. **Ainda nao
   testado na janela:** o modo "nova fita".
3. **[FEITO em 2026-10-08, 1.21.0] Empacotador de fita por linha de comando** -- `fwmsx --cas
   pack --tipo bin|bas` empacota um `.BIN`/`.BAS` solto num `.TSX`/`.CAS` sem abrir o emulador
   (resto do item c); `fwmsx --cas list` confirma o resultado. Ver secao 9 e `doc/tape-spec.md`,
   secao 8.
4. **[FEITO em 2026-10-08, 1.22.0] Port do makeTSX (WAV -> TSX)** -- `fwmsx --cas rip` demodula
   uma gravacao real de fita para `.TSX`, testado contra uma fita MSX de verdade dos anos 80 (86
   blocos reconhecidos). Ver secao 9 e `doc/tape-spec.md`, secao 9.
5. **[FEITO em 2026-10-08, 1.23.0] Navegacao de blocos de controle do TZX** -- Grupo/Salto/Laco/
   Chamada/Selecao (IDs 21-28) agora sao executados de verdade, nao so' pulados. Fecha o "passo 1"
   pedido pelo usuario por completo. Ver secao 9 e `doc/tape-spec.md`, secao 10.
6. **Fitas, o que ainda falta** (`doc/SPEC.md`, secao 5.2): (e) banco de fitas (metadados, sem
   download automatico ate ter autorizacao); preenchimento de alinhamento ainda adivinhado so' para
   `.cas` cru carregado direto do disco; `ScanCasFiles()` so' junta 2 blocos por arquivo (um ASCII
   multi-bloco de verdade aparece fragmentado, ver `doc/tape-spec.md`, secoes 5 e 9).
   Referencias: `resource/makeTSX/` (MIT), `resource/CLK/` (MIT), `resource/openMSX_TSXadv/` (GPL,
   so' estudo).
7. **Banco de ROMs:** conferir as ROMs baixadas contra o SHA-1 conhecido; usar o banco para escolher o
   mapper ao carregar cartucho (`CARTS.SHA` ja' importado); importar o JSON do Vampier se for util.
8. **FM e fita:** ouvir o WAV (`--wav`) e o modo normal da fita contra referencia (hardware real, nao
   so' "parece certo"); `CALL VOICECOPY`; status/timers do OPLL.
9. **Disco:** formatar disquetes; modelar FM/MFM; formatos independentes para A e B; estudar o driver
   de Sony/Philips/Spectravideo do openMSX (so' como referencia).
10. **Controle externo, estilo openMSX:** canal de controle em localhost (`status`, `reset`, `pause`,
   `type`, `cart`, `disk`, `fita`, `screenshot`, `peek`/`poke`, `quit`); a thread so' enfileira comandos.
   Ainda nao comecou.
11. **Jogos:** Lode Runner + SCC; Parodius (tela fragmentada); Mega Chase; F-1 Spirit 3D (troca de disco).
12. **BIOS Expert:** texto com espacos na tela; investigar.
13. **Save-state** (PSG, SCC, OPLL, disco, fita, VDP); **rastreio** no meio do quadro; **CPU no pior caso**.
14. **Layout de slots:** salvar/carregar em arquivo; perfis no banco.
15. **Cartuchos:** MSX-DOS 2, GameMaster2, MSX-MUSIC com BIOS propria.
16. **Depois:** frontend para jogar (biblioteca de jogos sobre o banco) com fitas E discos; integracao
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

### Rodada seguinte (2026-10-08): 1.20.2 -- os outros 2 modos de gravacao, achados pela janela

Depois da 1.20.1, o usuario testou os modos que ainda faltavam ("sobrescrever o ponto"/"nova fita")
e a marcacao de arquivo pela janela de verdade, e relatou tres coisas na mesma mensagem: "dando
CLOAD o contagiros nao gira, porem se eu clicar em um ponto da fita, em um programa, ele atualiza o
contagiros para aquela marca. Colocando no ponto e mudando o modo para sobrescrever, ele da Device
IO error, a opcao de salvar como uma nova fita tambem da device io error" -- e pediu para arrumar
tambem o bug do preenchimento de alinhamento ja' identificado (secao 5 do `tape-spec.md`) antes de
seguir para o proximo item da fila.

Investigacao (via diagnostico direto chamando `Machine`/`TapeEngine`, sem depender da GUI, que nao
e' automatizavel neste ambiente):
- **Contagiros**: NAO e' bug -- `odometer()` so' avanca com o motor girando; numa fita curta o
  avanco e' pequeno demais para notar a olho. Confirmado imprimindo `tape().odometer()`/`motor_on()`
  a cada frame durante um `CLOAD` real: sobe normalmente, congela so' quando o motor para.
- **"Device I/O error"**: reproduzido chamando o motor direto (sem GUI) -- os DOIS modos funcionavam
  certo quando a fita estava destravada. A causa real era `TapeEngine::Insert()` travando a fita de
  novo (`read_only_ = true`) toda vez, mesmo ao reinserir a MESMA fita que o usuario ja' tinha
  destravado -- e o menu Fita reinsere a imagem atual so' para atualizar a lista, "destravando e
  travando de novo" sem o usuario perceber. Corrigido preservando a protecao ao reinserir a mesma
  fita; o menu tambem ganhou dois itens explicitos no lugar do toggle unico.
- **Preenchimento de alinhamento**: confirmado que o bug da secao 5 era real -- `SynthesizeCasPulses`/
  o antigo `RebuildFromFastBytes`/`WriteTsxFromCas` descobriam o tamanho de cada bloco "procurando o
  proximo cabecalho", colando o preenchimento de alinhamento (zeros que o TAPOON insere antes do
  PROXIMO cabecalho) no final do bloco ANTERIOR. Corrigido com uma marca exata por bloco
  (`TapeMark{fast_byte_offset, pulse_index, content_length}`, `tape_image.h`) que o proprio motor de
  gravacao preenche ao fechar cada bloco (`TapeEngine::FinalizeWrite()`), usada tanto na sintese em
  memoria quanto na escrita do `.tsx`. Verificado com um teste novo que grava DOIS blocos, fecha o
  arquivo, recarrega do ZERO (nao so' em memoria) e confere que o segundo bloco bate exatamente com
  os bytes gravados. So' resolvido para fitas GRAVADAS por este emulador -- um `.cas` cru carregado
  direto do disco ainda nao tem marca exata (limitacao que permanece, documentada).

Build Windows e Linux (WSL) rodados de novo, `ctest` 16/16 nos dois (`tapetest` com mais uma
checagem). Escrito como versao `1.20.2` (mesmo codename, subtitulo novo: "Protecao e Preenchimento").
Ver `doc/CHANGELOG.md`/`doc/RELEASE.md`, `[1.20.2]`, para o detalhamento completo.

**Se o `git log` de `main` nao mostrar um commit de release da 1.20.2 ainda**, essa e' a proxima
acao: `git add` dos arquivos certos (nao `git add -A`), commit com `Co-Authored-By`, `git tag
1.20.2`, `git branch v1.20.2`, `git push origin main 1.20.2 v1.20.2`.

**Ainda nao revalidado na janela apos a 1.20.2:** os modos "sobrescrever"/"nova fita" e a marcacao
de arquivo, agora que o bug que impedia o teste foi corrigido -- a correcao em si foi validada so'
por diagnostico direto (sem GUI), nao pelo usuario na janela ainda.

### Rodada seguinte (2026-10-08): 1.20.3 -- "sobrescrever o ponto" apagava tudo

Logo depois da 1.20.2, o usuario testou "sobrescrever o ponto marcado" pela janela de verdade e
relatou: "Sobreescrever no ponto marcado, eu escolhi um ponto e mandei dar um csave, e como se o
programa sumisse, perdeu o anterior e o novo no ponto salvo. Incluir um novo programa no final
funcionou direitinho." Tambem disse que a mudanca na UI do menu Fita (1.20.2) nao precisava ser
desfeita -- foi um engano dele usando o menu antigo, nao um problema da mudanca em si.

Causa: um CSAVE/BSAVE de verdade chama `TAPOON`/`TAPOUT`/`TAPOOF` DUAS vezes -- um bloco so' para
o cabecalho com o nome do programa, outro so' para os dados de verdade (cada bloco tem seu proprio
piloto/sincronismo, igual numa fita real). `OnTapoon()` reaplicava a logica de "sobrescrever o
ponto marcado" em TODA chamada, nao so' na primeira. Na 2a chamada (bloco de dados), "o ponto
marcado" ja' nao era mais o programa antigo -- era o CABECALHO COM NOME que a 1a chamada tinha
acabado de escrever. Resultado: o cabecalho recem-escrito era apagado, so' sobravam os dados sem
nome -- nem o antigo (de verdade apagado, como esperado) nem o novo (com o cabecalho destruido)
ficavam reconheciveis. "Nova fita" tinha o mesmo problema.

Corrigido fazendo o corte/limpeza rodar UMA SO' VEZ: depois da 1a chamada que corta ou limpa,
`write_mode_` volta sozinho para `AppendAtEnd` -- qualquer bloco seguinte (do mesmo CSAVE, ou de
um CSAVE futuro sem marcar outro ponto) so' acrescenta, nunca corta de novo. Equivale ao
comportamento fisico real: depois de cortar a fita e comecar a gravar, o que vem a seguir so' pode
ir para a frente. Teste de regressao novo em `tapetest` simula os 2 blocos reais de um CSAVE (nome
+ dados) no modo "sobrescrever o ponto" e confere que ambos sobrevivem. Build Windows e Linux
(WSL) rodados de novo, `ctest` 16/16 nos dois. Escrito como versao `1.20.3` (mesmo codename,
subtitulo novo: "Um Corte So'"). Ver `doc/CHANGELOG.md`/`doc/RELEASE.md`, `[1.20.3]`.

A 1.20.3 foi comitada (`48f5c88`), taggeada e com push para `main` no mesmo dia. **Revalidado pelo
usuario na janela de verdade em 2026-10-08**: gravou num ponto no meio da fita, sobrescrevendo um
programa existente -- funcionou. So' o modo "nova fita" ainda nao foi testado pela janela (so' por
teste automatizado).

### Rodada seguinte (2026-10-08): 1.21.0 -- "passo 1": empacotador .BIN/.BAS -> .TSX

Com a gravacao de fita (1.20.x) fechada e revalidada, o usuario pediu para seguir para o "passo 1"
da lista de pendencias de fita (`doc/SPEC.md`, secao 5.2): uma ferramenta de linha de comando para
empacotar um `.BIN`/`.BAS` solto num `.TSX` sem passar pelo emulador.

Decisoes de projeto tomadas nesta rodada (nao pedidas explicitamente, julgamento proprio):
- **Nova flag do executavel principal** (`fwmsx --cas ...`), nao um executavel separado --
  diferente do `msxdisk.exe`, que e' um produto grande por conta propria (shell/TUI/GUI). Este e'
  um empacotador de uma tacada so', do mesmo porte que `fwmsx --romdb`/`--z80dbg`, que tambem
  roteiam para um modulo pelo `main.cpp` em vez de ganhar um binario proprio.
- **`--tipo bas` NAO tokeniza texto solto** -- so' empacota bytes que ja estao no formato
  tokenizado do MSX (como um `BSAVE` dentro do proprio emulador produziria). Motivo: um programa
  BASIC do MSX embute ponteiros de memoria ENTRE as linhas (o proximo-endereco de cada linha), que
  dependem de onde o programa vai ficar carregado -- tokenizar de verdade seria escrever um
  tokenizador completo do MSX BASIC, um projeto bem maior que "empacotar um arquivo solto" (fora
  de escopo, documentado em `doc/tape-spec.md`, secao 8).
- **Reaproveitou `WriteTsxFromCas()`** (o MESMO escritor que `TapeEngine` usa para gravar pela
  BIOS) em vez de duplicar logica de formato -- a nova peca (`src/tape/cpp/cas_pack.{h,cpp}`) so'
  monta os bytes "crus" no formato .CAS (cabecalho de sincronismo + bloco de 16 bytes com nome +
  bloco de dados), os MESMOS dois blocos que um CSAVE/BSAVE de verdade produz (ver o bug da
  1.20.3), so' que fora do gancho de BIOS, sem motor, sem Z80.
- **Teste novo e leve** (`castooltest`/CTest `cas_pack`): como o empacotador nao depende do Z80/
  mapa de memoria (so' le/escreve arquivo e monta bytes), ganhou um source-set proprio
  (`CAS_PACK_SOURCES`, sem `tape_device.cpp`/Z80/memmap) em vez de reusar `TAPE_LIB_SOURCES` por
  completo -- mantem o teste rapido e sem dependencias desnecessarias.

Build Windows e Linux (WSL) rodados, `ctest` 17/17 nos dois (suite nova `cas_pack`, 25 checagens).
Smoke test manual com o `fwMSX.exe` de verdade (`--cas pack --tipo bas ...` seguido de `--cas
list`) confirmou o fluxo completo fora dos testes automatizados. Escrito como versao `1.21.0`
(minor, nao patch -- e' uma feature nova, nao uma correcao, pela politica do projeto) com o mesmo
codename e um subtitulo novo: "Empacotador de Fita (BIN/BAS -> TSX)". Ver
`doc/CHANGELOG.md`/`doc/RELEASE.md`, `[1.21.0]`, e `doc/tape-spec.md`, secao 8 (nova).

**Se o `git log` de `main` nao mostrar um commit de release da 1.21.0 ainda**, essa e' a proxima
acao: `git add` dos arquivos certos (nao `git add -A`), commit com `Co-Authored-By`, `git tag
1.21.0`, `git branch v1.21.0`, `git push origin main 1.21.0 v1.21.0`.

**Cuidado ao gerar pacotes de release em rodadas seguidas sem bumpar a versao primeiro**: nesta
mesma rodada, o build do Linux foi disparado ANTES de bumpar `version.h` de 1.20.3 para 1.21.0,
sobrescrevendo por engano o `.tar.gz` da 1.20.3 JA LANCADA (ja' tinha acontecido uma vez antes, na
rodada da 1.20.3 sobre a 1.20.2) -- corrigido com `git checkout -- <arquivo>` antes de gerar os
pacotes certos. Sempre bumpar `version.h` ANTES do primeiro build de uma rodada nova, nao depois.

**Ainda nao testado na janela:** o `--cas` e' so' CLI, sem equivalente na GUI -- nao se aplica. O
modo "nova fita" da gravacao continua pendente de teste na janela (ver rodada anterior).

### Rodada seguinte (2026-10-08): 1.22.0 -- port do makeTSX (WAV -> TSX)

Com o empacotador fechado, o usuario pediu o segundo item da fila de fitas: "port do makeTSX
(WAV -> TSX)". Implementado como `fwmsx --cas rip`.

**Desenho da solucao** (estudo do `resource/makeTSX/` antes de escrever qualquer codigo):
- Lido `WAV.h/.cpp` (leitor de `.wav`), `BlockRipper.h/.cpp` (deteccao de pulsos por limiar +
  piloto + silencio, generico) e `rippers/MSX4B_Ripper.h/.cpp` (a parte especifica do MSX: bits de
  inicio/fim, decodificacao byte a byte, com modos interativo/preditivo para gravacoes ruidosas).
- Decisao: portar so' o CONCEITO (deteccao de piloto por limiar adaptativo + decodificacao com
  tolerancia), fixando os parametros no caso MSX (como o `isMSX` do original) e OMITINDO os modos
  interativo (pede ajuda ao usuario pela linha de comando) e preditivo (adivinha bits ambiguos
  "olhando para frente") -- fora de escopo para uma ferramenta batch, documentado como limitacao
  conhecida. Decisao tambem de NAO guardar a velocidade medida da fita no `.tsx` de saida -- reusa
  os mesmos `AppendCasBlock()`/`WriteTsxFromCas()` do empacotador (1.21.0), que sempre escrevem nos
  parametros CANONICOS do MSX -- "ripar" tambem normaliza a velocidade.
- Nova funcao `kcs_decode_byte()` em `src/tape/core/kcs_codec.{h,c}` (o MESMO modulo do
  `kcs_emit_byte()` existente, so' o caminho inverso) -- decide bit a bit comparando a soma de um
  grupo de pulsos contra a duracao esperada, com tolerancia.

**Dois bugs reais encontrados pelo PROPRIO teste automatizado** (antes de qualquer teste manual --
um round-trip bytes -> audio sintetico -> `rip` -> bytes, comparando byte a byte):
1. O bit de INICIO, se tratado com a mesma tolerancia "assumida" dos bits de fim, fazia o piloto
   do PROXIMO bloco (uma sequencia pura de pulsos do tamanho do bit 1, sem nenhum zero) ser lido
   como um fluxo infinito de bytes `0xFF` -- o bloco nunca terminava sozinho. Corrigido exigindo
   que o(s) bit(s) de INICIO batam de verdade (sem tolerancia extra); os de FIM continuam leniente
   (um byte ja decidido pelos 8 bits de dados nao deve ser descartado so' por um bit de fim ruidoso).
2. O ULTIMO byte de uma gravacao perdia os bits de fim por falta de pulso depois dele (a gravacao
   so' termina ali) -- e a logica de entao tratava "faltam pulsos para o fallback assumido" como
   falha do byte INTEIRO, descartando os 8 bits de dados ja' corretamente decididos. Corrigido para
   aceitar o byte mesmo que o fim do arquivo chegue no meio do bit de fim.

**Testado contra uma fita real de MSX dos anos 80** (`resource/openMSX/Contrib/
reverse_engineering_tools/kanji/ktst31 [RUN'CAS-'].wav`, GPL -- so' validacao manual local, NUNCA
versionada como parte deste projeto, so' usada como referencia de estudo, igual o resto de
`resource/`): `fwmsx --cas rip` reconheceu 86 blocos (4 arquivos ASCII: `KTST31`/`KT31A`/`KT31B`/
`KT31C`) sem nenhum erro de decodificacao -- confirmacao forte de que o algoritmo funciona em
audio de gravacao real, nao so' em dados sinteticos. Nota: cada "arquivo" ASCII apareceu
fragmentado (so' 8 bytes cada) porque `ScanCasFiles()` so' junta DOIS blocos por arquivo (nome +
um bloco de dados) -- um ASCII de verdade tem VARIOS blocos de 256 bytes; a decodificacao de
pulsos em si funcionou certo, so' a juncao em "arquivos" que e' limitada (documentado, nao e' bug
do `rip`).

Build Windows e Linux (WSL) rodados, `ctest` 17/17 nos dois (`castooltest`/`cas_pack` com o
round-trip completo + testes de erro: `.wav` estereo, silencio, argumentos invalidos). Escrito
como versao `1.22.0` (minor -- feature nova) com o mesmo codename e um subtitulo novo: "Ripper de
Fita (WAV -> TSX)". Ver `doc/CHANGELOG.md`/`doc/RELEASE.md`, `[1.22.0]`, e `doc/tape-spec.md`,
secao 9 (nova).

**Desta vez a versao foi bumpada ANTES do primeiro build** (lecao da rodada anterior) -- o build
Linux em segundo plano ja' pegou `1.22.0` certo, sem precisar restaurar nenhum pacote antigo.

**Se o `git log` de `main` nao mostrar um commit de release da 1.22.0 ainda**, essa e' a proxima
acao: `git add` dos arquivos certos (nao `git add -A`), commit com `Co-Authored-By`, `git tag
1.22.0`, `git branch v1.22.0`, `git push origin main 1.22.0 v1.22.0`.

**Ainda nao testado na janela:** o `--cas` e' so' CLI, sem equivalente na GUI -- nao se aplica. O
modo "nova fita" da gravacao continua pendente de teste na janela (ver rodadas anteriores).

**Licoes para a proxima vez:**
- **Testar o round-trip completo (encode -> decode) antes de testar contra dados reais** --
  os dois bugs do decodificador KCS (piloto confundido com dados, ultimo byte perdido) so'
  apareceram no teste automatizado SINTETICO, construido ANTES de baixar/testar qualquer gravacao
  real. Se o primeiro teste tivesse sido direto contra a fita real, os mesmos bugs apareceriam so'
  como "deu errado", sem a clareza de UM teste isolado e determinista mostrando exatamente qual
  byte sumiu e por que.
- **Portar o CONCEITO, nao o codigo, de uma ferramenta de referencia com licenca permissiva** --
  o makeTSX (MIT) tem modos interativo/preditivo que resolvem casos de borda genuinamente dificeis
  (fita muito ruidosa), mas replica-los piora o retorno sobre o esforco para uma ferramenta batch
  como esta. Simplificar PARA O CASO QUE IMPORTA (o MSX fixo, aqui) e documentar o que foi deixado
  de fora e' melhor que portar tudo "por completude".

### Rodada seguinte (2026-10-08): 1.23.0 -- navegacao de blocos de controle do TZX

Terceiro e ultimo item do "passo 1": navegar os blocos de controle do TZX (grupos/lacos/saltos),
que antes eram so' pulados com seguranca (comprimento sempre conhecido) sem afetar a ordem real de
execucao.

**Desenho**: o parser original era UMA passada so' (le o ID, processa, avanca, repete do inicio ao
fim). Saltos/lacos/chamadas se referem a blocos pelo NUMERO DE ORDEM (nao pelo deslocamento em
bytes), entao navegar de verdade exige saber os limites de TODOS os blocos ANTES de poder saltar
para a frente. Reescrito como leitor em DUAS passadas: a 1a (`SkipOneBlock()`) so' indexa onde
cada bloco comeca; a 2a executa de verdade, com um "PC" (indice, nao deslocamento em bytes), uma
pilha de lacos e uma pilha de chamadas (a especificacao do TZX permite aninhar chamadas com lacos,
so' NAO permite lacos aninhados com lacos -- usamos pilha pra ambos, por seguranca, mesmo assim).
`ExecuteDataBlock()` ganhou o conteudo EXATO do switch original para os blocos "passivos" (#10 a
#20, #2A/#2B, #30-35, #4B, #5A) -- nenhuma logica de formato foi alterada, so' movida.

**Selecao (#28)** nao tem como mostrar um menu de verdade numa ferramenta sem interface -- decisao:
escolhe sempre a 1a opcao da lista, documentado como comportamento padrao (nao uma limitacao
temporaria, e' inerente a uma ferramenta batch).

**Bug pego na hora de escrever o teste, nao no codigo de navegacao**: o primeiro teste usava um
bloco #4B isolado (so' o cabecalho de 16 bytes) por "arquivo" sintetico -- mas dois cabecalhos #4B
colados direto um no outro (sem bloco de dados no meio) fazem `ScanCasFiles()` confundir o
cabecalho do PROXIMO arquivo com o inicio dos dados do ATUAL (a heuristica dela assume que um
cabecalho seguido de outro cabecalho imediatamente quer dizer "aqui comeca o bloco de dados", a
convencao real do CSAVE). Corrigido fazendo cada "arquivo" de teste ser um PAR de blocos #4B (nome
+ 1 byte de dados), igual uma gravacao de verdade.

Build Windows e Linux (WSL) rodados, `ctest` 17/17 nos dois (`tapetest` com 6 checagens novas:
salto pula 1 bloco; laco repete o corpo 3 vezes; laco com 0 repeticoes pula o corpo inteiro;
chamada com lista de 2 execucoes + retorno; selecao escolhe a 1a opcao; salto fora dos limites e'
recusado com erro). Escrito como versao `1.23.0` (minor -- feature nova) com o mesmo codename e um
subtitulo novo: "Navegacao de Blocos do TZX". Ver `doc/CHANGELOG.md`/`doc/RELEASE.md`, `[1.23.0]`,
e `doc/tape-spec.md`, secao 10 (nova).

**Desta vez tambem**: versao bumpada ANTES do primeiro build (confirmado -- o build Linux ja'
gerou `fwMSX-1.23.0-linux.tar.gz` direto, sem precisar restaurar nenhum pacote antigo).

**Se o `git log` de `main` nao mostrar um commit de release da 1.23.0 ainda**, essa e' a proxima
acao: `git add` dos arquivos certos (nao `git add -A`), commit com `Co-Authored-By`, `git tag
1.23.0`, `git branch v1.23.0`, `git push origin main 1.23.0 v1.23.0`. Essa era a ultima peca do
"passo 1" pedido pelo usuario -- depois disso, so' restam os itens (e)/(f)... espera, (f) (port do
makeTSX) ja' foi feito na 1.22.0; so' resta (e) banco de fitas/metadados na lista de pendencias de
fita, alem das limitacoes conhecidas documentadas (preenchimento de alinhamento para `.cas` cru,
ASCII multi-bloco fragmentado).

**Ainda nao testado na janela:** navegacao de TZX nao tem equivalente na GUI (e' so' parte do
leitor, usado tanto pela janela quanto pelo `--cas`) -- testado so' via `tapetest`, nao contra um
`.tzx` real do mundo ZX Spectrum que use esses blocos pesadamente (nenhum fixture assim no
repositorio ainda).

**Licoes para a proxima vez:**
- **Ao escrever um teste sintetico para um formato com convencoes implicitas** (como "dois
  cabecalhos #4B colados = cabecalho+dados", nao "dois arquivos separados"), seguir a MESMA
  convencao que o leitor de verdade espera, nao so' "o minimo que compila". O bug nao estava no
  codigo de navegacao (que passou pelo teste corrigido sem precisar de nenhuma mudanca) -- estava
  no teste assumindo uma estrutura de bytes que nenhuma fita de verdade produziria.

**Licoes para a proxima vez:**
- **Bumpar `version.h` ANTES do primeiro build de uma rodada de release, nunca depois** -- rodar o
  build (Windows ou Linux) com a versao ANTIGA ainda no `version.h` sobrescreve o pacote JA
  LANCADO daquela versao com conteudo novo (sob o nome errado). Aconteceu duas vezes nesta mesma
  sessao (1.20.2 -> 1.20.3 e 1.20.3 -> 1.21.0) antes de ser corrigido com `git checkout --
  dist/fwMSX-X.Y.Z-linux.tar.gz`. A ordem certa e': bumpar a versao, DEPOIS buildar.
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
- **Uma operacao "destrutiva" acionada por um gancho de BIOS (TAPOON aqui) precisa pensar em QUANTAS
  VEZES esse gancho e' chamado por uma UNICA operacao logica do usuario**, nao so' na primeira vez.
  Um CSAVE chama TAPOON duas vezes (nome + dados); o teste que so' simulava UM TAPOON/TAPOOF (por
  simplicidade) passava, mas escondia o bug real -- so' apareceu com um teste que simula a sequencia
  de chamadas de verdade. Pensar "o que a BIOS real faz, passo a passo, com UM comando do BASIC"
  antes de assumir que um gancho so' e' chamado uma vez por acao do usuario.

---

## 11. ENCERRAMENTO DA SESSAO DE 2026-10-08 -- leia isto primeiro ao retomar amanha

**Estado do repositorio:** tudo comitado e com push feito. `git log --oneline -1` em `main` deve
mostrar `527f763` (ou mais novo, se outra sessao continuar depois desta nota ser escrita). Nenhum
arquivo pendente (`git status` limpo). Versao publicada: **1.23.0**.

### O que foi feito hoje (nesta sessao), em ordem

1. **1.20.2** -- corrigidos 2 bugs reais de gravacao que o usuario achou testando pela janela
   (reinserir a MESMA fita destravada voltava a trava-la sem aviso; preenchimento de alinhamento
   gravado como dado de verdade ao reler uma fita gravada por este emulador). Contagiros "parado"
   no CLOAD investigado e confirmado como NAO sendo bug.
2. **1.20.3** -- corrigido mais um bug: "sobrescrever o ponto marcado" apagava o cabecalho que o
   proprio CSAVE tinha acabado de escrever (a logica de corte reaplicava em cada uma das 2
   chamadas de TAPOON que um CSAVE de verdade faz). **Revalidado pelo usuario na janela**: gravou
   num ponto no meio da fita, sobrescrevendo um programa existente, com sucesso.
3. **1.21.0** -- "passo 1", item 1: `fwmsx --cas pack` empacota um `.BIN`/`.BAS` solto num
   `.TSX`/`.CAS` sem abrir o emulador.
4. **1.22.0** -- "passo 1", item 2: `fwmsx --cas rip` demodula uma gravacao `.wav` real de fita
   para `.TSX` (port do CONCEITO do makeTSX, nao do codigo). Testado contra uma fita MSX real dos
   anos 80 (86 blocos reconhecidos sem erro).
5. **1.23.0** -- "passo 1", item 3 (ultimo): navegacao de verdade dos blocos de controle do TZX
   (grupos/lacos/saltos/chamadas/selecao) -- leitor reescrito em 2 passadas.
6. **Documentacao sincronizada** (este pedido, sem mudar a versao): `README.md`, `doc/MANUAL.md`
   (secoes "Emulador MSX", "Banco de ROMs", nova secao "Fita por linha de comando", "O que
   funciona e o que nao funciona"), `doc/SPEC.md` (secao 5.0, banners de versao), e este
   `OUTLINE.md` (secao 2 e esta secao 11) -- todos atualizados para refletir a 1.23.0 com tudo que
   funciona, o que tem limitacao, e o que falta. `doc/CHANGELOG.md`/`doc/RELEASE.md`/
   `doc/tape-spec.md` ja' estavam em dia (atualizados a cada release ao longo do dia).

O "passo 1" completo pedido pelo usuario (empacotador, ripper de WAV, navegacao de TZX) esta'
**FECHADO**.

### O que falta -- pendencias para continuar amanha (ordem sugerida, ver `doc/SPEC.md` secao 5.0 para a lista completa e atualizada)

- [ ] **Revalidar na janela de verdade**: o modo "nova fita" de gravacao (so' "sobrescrever o
  ponto" e "incluir no final" foram confirmados pelo usuario testando pela janela). O `--cas` e'
  so' CLI, sem equivalente na GUI -- nao se aplica revalidar na janela.
- [ ] **Fitas, o que resta da lista original** (`doc/SPEC.md`, secao 5.2): banco de fitas/
  metadados (item e -- sem download automatico ate ter autorizacao do usuario); preenchimento de
  alinhamento ainda adivinhado para `.cas` cru carregado direto do disco (sem ter passado por uma
  gravacao deste emulador); `ScanCasFiles()` so' junta 2 blocos por arquivo -- um ASCII
  multi-bloco de verdade (256 bytes por bloco) aparece fragmentado na lista.
- [ ] **Banco de ROMs**: verificar as ROMs baixadas contra o SHA-1 conhecido; usar o banco para
  escolher o mapper ao carregar um cartucho; importar o JSON do Vampier se for util.
- [ ] **Ouvir o FM, o SCC, o disco e a fita (modo normal) contra referencia** (hardware real, nao
  so' "parece certo"); ajustar as constantes do OPLL e os ganhos da mistura.
- [ ] **Jogos com problema conhecido**: Lode Runner + SCC nao sobe; Parodius (Smooth Scroll) tela
  fragmentada (causa nao diagnosticada); Mega Chase validado so' ate o titulo; F-1 Spirit 3D (troca
  de disco pela janela nao testada).
- [ ] **FM**: `CALL VOICECOPY` (a ROM do fMSX nao aceita); status e timers do OPLL.
- [ ] **Disco**: formatar disquetes (hoje so' le e grava); FM/MFM modelados; drives A e B com
  formatos diferentes.
- [ ] **Itens maiores, sem data definida**: save-state (PSG/SCC/OPLL/disco/fita/VDP); rastreio no
  meio do quadro; controle externo estilo openMSX (canal localhost); layout de slots salvar/
  carregar em arquivo; cartuchos MSX-DOS 2/GameMaster2/MSX-MUSIC com BIOS propria; BIOS Expert
  (texto com espacos na tela); frontend/integracao com o msxide (MSX-PoorManOS) -- ver secao 7
  para a lista completa, em ordem sugerida.

**Para retomar amanha**: nao ha' nenhuma instrucao especifica do usuario sobre qual pendencia
atacar primeiro -- a ultima mensagem dele foi so' pedir esta sincronizacao de documentacao antes
de encerrar o dia. Comecar perguntando qual item da lista acima ele quer priorizar, igual foi
feito a cada "proximo passo" ao longo desta sessao.
