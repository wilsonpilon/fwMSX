# fwMSX -- instrucoes para o Claude Code

Este arquivo e' lido automaticamente pelo Claude Code ao abrir uma sessao
nesta pasta -- e' o jeito de retomar o trabalho em OUTRO COMPUTADOR sem
perder contexto, sem depender de `claude --resume` (que so' funciona na
mesma maquina onde a sessao rodou).

**Leia `OUTLINE.md` logo em seguida, sempre, antes de qualquer trabalho.**
E' o "onde paramos" oficial do projeto -- versao publicada, o que falta, o
pedido mais recente do usuario, e o historico narrativo de cada rodada.
Este arquivo (CLAUDE.md) so' guarda as regras FIXAS que nao mudam de rodada
pra rodada; o estado atual mora no OUTLINE.md, nao aqui.

## O que e' o projeto

**fwMSX** e' um emulador de MSX (MSX1, MSX2, MSX2+) escrito em C, C++,
Assembly (NASM, dual-ABI Win64/SysV) e Fortran, feito como projeto de
estudo a partir do fMSX (Marat Fayzullin), com o aval do autor original
para adaptar/estudar o codigo dele. Regra de design do projeto: cada fase
relevante exercita as 4 linguagens, mesmo que minimamente. Repositorio
**pessoal** do usuario (Wilson Pilon) -- hobby/estudo, nao trabalho de
equipe. Objetivo de medio prazo (sem data): integrar ao **msxide** num
utilitario chamado **MSX-PoorManOS**.

## Regras do workflow de release (ja' causaram retrabalho quando ignoradas)

1. **Politica de numeracao e de NOMES (definida pelo usuario em 2026-10-09)**: `X.Y.Z`, cada
   nivel com um nome proprio:
   - **X (major)**: um conjunto grande esta' praticamente todo feito. Recebe o nome de uma
     **EMPRESA de MSX**. 1 = **Konami** (atual). Proximas: 2 = ASCII, 3 = Compile, 4 = Hudson
     Soft, 5 = T&E Soft, 6 = Microsoft... (nao repetir).
   - **Y (minor)**: bloco maior de funcionalidade, publicado como release. Recebe o nome de um
     **JOGO NOVO de MSX**, nunca repetido. Zera o Z. Para o X = Konami, jogos reservados na
     ordem: ~~Nemesis (1.32)~~, Penguin Adventure (1.33), Metal Gear (1.34), Vampire Killer
     (1.35), Salamander (1.36), Knightmare (1.37), Road Fighter (1.38), Yie Ar Kung-Fu (1.39),
     Antarctic Adventure (1.40), Hyper Sports (1.41)... (as versoes 1.19 a 1.31 ficaram todas
     como "King's Valley", do esquema antigo -- nao renomear).
   - **Z (patch)**: feature pequena, ajuste ou correcao. Leva o nome do jogo do Y **mais um
     subtitulo** ("Nemesis: Ponte de controle"). A versao X.Y.0 leva so' o nome do jogo.
   Na duvida entre patch e minor, usar patch. Em `version.h`: `FWMSX_COMPANY`,
   `FWMSX_CODENAME` (jogo) e `FWMSX_SUBTITLE` (vazio quando Z = 0). Ao subir o Y, trocar o
   jogo e esvaziar o subtitulo; ao subir o X, trocar a empresa.
   Titulo nos docs/release: `vX.Y.Z -- "Jogo: Subtitulo"` (ou so' `"Jogo"` com Z = 0).

   **Bumpar `src/common/version.h`** (conforme a politica acima)
   **ANTES do primeiro build de QUALQUER
   rodada nova** -- sem excecao, mesmo rodada pequena. O build deriva o
   nome do pacote em `dist/` do numero de versao do header; um build feito
   sem bumpar primeiro SOBRESCREVE o pacote ja publicado da versao
   anterior. Isso ja' aconteceu 2 vezes na mesma sessao (2026-10-08) --
   sempre checar `src/common/version.h` ANTES de rodar `build.ps1`/
   `build.sh`, nunca confiar em lembrar depois.
2. **Build Windows (`.\build.ps1`) E Linux** (WSL, `./build.sh`) -- os dois
   precisam fechar com `ctest` 100%. Nunca liberar so' com um dos dois.
3. **Atualizar TODOS os docs vivos no mesmo commit da feature**:
   `doc/SPEC.md` (secao 5.0 = estado atual), `doc/CHANGELOG.md`,
   `doc/RELEASE.md`, o `doc/*-spec.md` especifico da area tocada,
   `README.md`, `doc/MANUAL.md`, `OUTLINE.md`.
4. `git add` com **caminhos explicitos**, nunca `git add -A`/`git add .`.
5. Commit termina com `Co-Authored-By: Claude Sonnet 5 <noreply@anthropic.com>`.
6. `git tag X.Y.Z` (SEM "v") **e** `git branch vX.Y.Z` (COM "v"), os dois
   apontando pro mesmo commit.
7. `git push origin main X.Y.Z vX.Y.Z`.

## Convencoes de codigo/teste

- **Smoke-test de CLI que mexe em estado persistente** (banco de ROMs,
  banco de fitas, `.sav`, `.sst`): SEMPRE com um executavel ISOLADO num
  diretorio temporario, NUNCA contra o `dist/roms/roms.db` real (rastreado
  no git, tem dados de verdade do usuario).
- **Antes de implementar um item de pendencia exatamente como foi escrito
  antes, pesquisar primeiro.** Se a premissa original mudou ou estava
  errada (ja' aconteceu: o JSON do Vampier era redundante com o SQL; o
  MSX-DOS 2 generico NAO precisava de mapper de RAM, so' de um cartucho
  comum), apresentar a descoberta ao usuario (`AskUserQuestion`) antes de
  codar, em vez de implementar cegamente ou decidir sozinho.
- **Strings de UI/docs em portugues sao SEM ACENTO** (convencao do projeto
  inteiro, ASCII puro) -- "nao" nao "não", "e'" nao "é", "secao" nao
  "seção", "midia" nao "mídia". Vale pra texto novo em qualquer arquivo.
- Comentarios em C/C++/Fortran/Assembly seguem o mesmo padrao de portugues
  sem acento, explicando o PORQUE (nao o que o codigo faz).

## Onde esta' cada coisa

- **`OUTLINE.md`**: estado atual, o que falta, pedido mais recente do
  usuario, historico narrativo por rodada (secao "Dia N, continuacao").
  **Sempre o primeiro arquivo a ler.**
- **`doc/SPEC.md`**: especificacao viva completa do projeto (secao 5.0 =
  "onde paramos" tecnico, mais detalhado que o OUTLINE).
- **`doc/CHANGELOG.md`** / **`doc/RELEASE.md`**: historico de versoes, um
  resumido (estilo Keep a Changelog) e um detalhado (decisoes, achados,
  build usado pra validar).
- **`doc/*-spec.md`**: um design doc por subsistema (`vdp-spec.md`,
  `memory-map-spec.md`, `savestate-spec.md`, `tape-spec.md`, `fdc-spec.md`,
  `romdb-spec.md`, `fm-spec.md`, `scc-spec.md`, `sram-spec.md`,
  `slots-spec.md`, `msx2-spec.md`, `msx2p-spec.md`, `ppi-spec.md`,
  `z80-core-spec.md`...) -- mesmo formato: o que foi feito, decisoes
  tomadas, testes, o que falta.
- **`README.md`** / **`doc/MANUAL.md`**: docs voltados pro usuario final
  (nao pra quem esta' desenvolvendo) -- "o que o emulador faz hoje" e "como
  usar", respectivamente.
