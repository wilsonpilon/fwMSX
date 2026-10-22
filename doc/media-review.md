# Revisao das licencas de midia e de material de terceiros

Levantamento de 2026-10-22 (versao 1.43.16), a pedido do usuario ("revisao de licencas de midia", item 6 da lista de pendencias). **So' levantamento e recomendacao: nada foi removido nem reescrito.** Quem decide o que sai e' o autor do fwMSX.

## 1. O fato que muda o quadro

- O repositorio `github.com/wilsonpilon/fwMSX` e' **PUBLICO** (conferido com `gh repo view`).
- O `LICENSE-THIRD-PARTY.md` dizia que o material de `resource/` e' "incluido apenas para estudo" e que "nada disso e' distribuido". Isso vale para os **pacotes de release** (zip/tar.gz: conferidos, so' levam o programa, `badig/`, `emulator/`, a imagem de abertura, o `SUPER-X-PT.notas` e os documentos), mas **nao vale para o repositorio**: tudo que esta' rastreado no git esta' publicado para qualquer pessoa clonar, inclusive o historico (`.git` com ~1,1 GB).
- Portanto a pergunta nao e' "o que entra no pacote", e sim "o que esta' no repositorio publico".

## 2. O que o git publica (arquivos rastreados)

| Onde | Arquivos | Midia | De quem e' |
|---|---|---|---|
| `resource/fmsxgo/` | 23176 | 245 `.rom`, 143 `.dsk`, 64 `.pdf`, 48 `.ips`, 27 `.zip`, 3 `.tsx` | porte do fMSX para Go e seu material (jogos, discos, manuais) |
| `resource/msxide/` | 8497 | 101 `.rom`, 48 `.dsk`, 31 `.pdf`, 15 `.zip` | msxIDE (projeto do mesmo autor) e o que ele carrega |
| `resource/paleobasic/` | 8160 | 73 `.rom`, 50 `.dsk`, 26 `.pdf`, 9 `.zip` | PaleoBasic (projeto do mesmo autor) |
| `resource/kizuna/` | 3647 | 92 `.rom`, 18 `.dsk`, 12 `.pdf`, 2 `.cas` | Kizuna (projeto do mesmo autor) |
| `resource/openMSX/` e `openMSX_TSXadv/` | 4579 | 38 `.rom`, 32 `.ips`, 2 `.wav` | openMSX (GPL 2+) e a variante TSX |
| `resource/CLK/`, `resource/fMSX/`, `makeTSX/`, `asMSX/`, `msxDiskUtil/` | ~1900 | 16 `.rom` no fMSX | Clock Signal, fMSX (Fayzullin), makeTSX, asMSX (GPLv3), msxDiskUtil (GPLv3) |
| `resource/msxdos1/`, `msxdos2/`, `MSXDOS23.DSK` | ~117 | arquivos do sistema | Microsoft / MSX Association / The New Image |
| **`dist/roms/`** | **700** | **637 `.rom`**, 2 `.zip`, `roms.db` | banco de ROMs do usuario: BIOS do MSX/MSX2/MSX2+, ROMs de sistema (~689 do pacote "Full Set System ROMs" do file-hunter, de maquinas reais), extensoes, o CARTS.SHA |
| **`tests/media/`** | **19** | **19 `.rom` de jogos comerciais** | Konami (13), Compile (2), Irem (1)... em dump "GoodMSX" |
| `dist/*.zip`, `dist/*.tar.gz` | 96 | pacotes de releases antigos | o proprio fwMSX (+ o que cada pacote levava) |
| raiz e `images/` | ~80 | `msxdos1.dsk`, 2 `.sst` (estados de maquina), capturas de tela | disco de sistema, instantaneos de RAM, capturas |

Totais rastreados: ~50 mil arquivos em `resource/`, 813 em `dist/`; **1221 `.rom`, 266 `.dsk`, 136 `.pdf`, 113 `.ips` e 105 `.zip`** no repositorio inteiro.

## 3. Risco por grupo

1. **ROMs de jogos comerciais** (`tests/media/`, 19; mais as que vierem nas pastas de `resource/` e `dist/roms/`): direitos de Konami, Compile, Irem etc. Nao ha' licenca de redistribuicao. **Risco ALTO**, e e' o caso classico de pedido de remocao (DMCA). Nenhum teste do `ctest` aponta para `tests/media/` (nao ha' referencia nos `.cpp` nem no CMake): so' servem para teste manual.
2. **ROMs de BIOS e de sistema de maquinas reais** (`dist/roms/bios`, `filehunter/...`, `interfaces/`): direitos de fabricantes (Sony, Philips, Panasonic, Microsoft...). Algumas sao firmware de cartuchos modernos (Carnivore2...) cuja redistribuicao o autor pode permitir, mas nao esta' registrado. **Risco ALTO a medio.** O emulador precisa de uma BIOS para rodar, mas o fwMSX nao embute nenhuma: o usuario aponta a dele.
3. **Discos e ROMs dentro de `resource/`** (fmsxgo, msxide, paleobasic, kizuna, openMSX): material de outros projetos. Os de projetos do proprio usuario (msxIDE, PaleoBasic, Kizuna) e' dele, **menos o que cada um carrega de terceiros** (jogos em `.dsk`, ROMs de sistema, PDFs de manuais). Risco **medio**.
4. **Projetos GPL em `resource/`** (openMSX, asMSX, msxDiskUtil): ja' documentados como "so' estudo, fora do build, sera retirado". Publicar o fonte de um projeto GPL com a licenca junto e' permitido; o que nao pode e' misturar no fwMSX (BSD-3), e nao ha' mistura. Risco **baixo**, mas a promessa de retirar continua valendo.
5. **fMSX (Fayzullin)**: tem aval do autor para adaptar/estudar; a licenca dele **proibe distribuicao comercial**. O fwMSX ja' registra isso (secao fMSX do `LICENSE-THIRD-PARTY.md`) e se trata como nao-comercial. **Sem mudanca**; so' lembrar que o aval e' para adaptar, nao para relicenciar.
6. **MSX-DOS 1 e 2** (`resource/msxdos*`, `emulator/` no pacote, `msxdos1.dsk` na raiz): decisao do usuario de 2026-10-22 (distribuir, retirar se alguem reclamar). Registrada. **Sem mudanca.** O `MSXDOS23.DSK` e o `msxdos1.dsk` da raiz estao no mesmo caso.
7. **Estados `.sst` e capturas** (`fwmsx-estado-*.sst`, `fwmsx-*.png`, `images/`): um `.sst` guarda a RAM/VRAM da maquina com o jogo rodando, ou seja, **trechos do jogo**; as capturas mostram telas de jogos. Risco **baixo a medio**; sao artefatos de teste sem uso no repositorio.
8. **Pacotes antigos em `dist/`** (96 `.zip`/`.tar.gz`): cada um e' o fwMSX daquela versao (alguns levaram mais coisas no passado, como `roms/`). Alem do risco, **incham o repositorio** (sao binarios que o git guarda para sempre). Os releases oficiais ja' estao no GitHub Releases.
9. **Dependencias baixadas na configuracao** (SQLite: dominio publico; CLI11: BSD-3; replxx: BSD-3; FTXUI: MIT; Dear ImGui: MIT; GLFW: zlib; miniaudio: dominio publico/MIT-0; miniz: MIT; Lua: MIT): todas permissivas e **nenhuma copiada para o repositorio**. Risco **nenhum**. Faltavam no `LICENSE-THIRD-PARTY.md`: SQLite, CLI11, replxx, FTXUI, ImGui, GLFW (registrados agora).
10. **O que vai nos pacotes** (conferido em 1.43.16): `badig/nbasic.dmx` e `badig/NBASIC.BIN` (NestorBASIC, de Konamiman, via msxIDE: **a licenca de redistribuicao nao esta' registrada no repositorio**), `SUPER-X-PT.notas` (traduzido do PaleoBasic: texto do usuario sobre notas do SUPER-X, de origem ja' do PaleoBasic), `fwMSX-splash.png` (**origem da imagem nao registrada**; conferir se e' arte propria) e `emulator/` (MSX-DOS). Os quatro primeiros pedem uma linha de origem/licenca cada.

## 4. Recomendacao (em ordem de custo/beneficio)

1. **Parar de rastrear o que e' midia de terceiros, sem mexer no historico**: `git rm --cached` + `.gitignore` para `tests/media/`, `dist/roms/` (inclui o `roms.db`, que o `CLAUDE.md` chama de "dados de verdade do usuario"), `*.sst`, as capturas soltas da raiz e `dist/*.zip|*.tar.gz`. Os arquivos continuam no disco do usuario; deixam de ser publicados daqui para frente. Custo: baixo; reversivel; nao muda releases nem tags.
2. **`resource/`**: decidir por pasta. Sugestao: manter o que e' fonte (`kizuna`, `msxide`, `paleobasic`, `fmsxgo` sem as midias, `openMSX`, `asMSX`...) e parar de rastrear as extensoes de midia dentro delas (`*.rom`, `*.dsk`, `*.cas`, `*.tsx`, `*.ips`, `*.wav`), ou tirar do repositorio tudo que nao e' do autor quando o fwMSX estiver pronto (como ja' prometido para openMSX e asMSX).
3. **Reescrever o historico** (`git filter-repo` + push forcado) so' se o autor quiser que as ROMs deixem de existir no historico tambem. E' a unica forma de apagar o que ja' foi publicado, mas **muda os hashes de todos os commits e tags**, invalida clones e os links dos releases, e e' irreversivel no remoto. Nao recomendo sem um pedido de titular de direitos; ate' la' o passo 1 basta como boa-fe.
4. **Registrar a origem** de `NBASIC.BIN`/`nbasic.dmx` (NestorBASIC), `fwMSX-splash.png` e `SUPER-X-PT.notas` no `LICENSE-THIRD-PARTY.md` (precisa da informacao do autor).
5. **Texto do README publico**: o aviso "se o detentor dos direitos pedir, o arquivo e' removido" ja' esta' previsto (`LICENSE-THIRD-PARTY.md`); copiar para o `README.md`, com um contato.
6. **Para os testes manuais**: documentar que as ROMs de `tests/media/` sao do usuario e como obte-las (a pasta fica fora do git).

## 4b. Aplicado (2026-10-22, decisao do usuario: "1")

Passo 1 da recomendacao: `git rm --cached` (os arquivos **continuam no disco**) e entradas no `.gitignore` para `tests/media/` (19 ROMs de jogos), `dist/roms/` (637 ROMs de sistema, `roms.db` e o resto do banco: 700 arquivos), os 96 pacotes antigos `dist/*.zip` e `dist/*.tar.gz`, `*.sst` e as capturas soltas `/fwmsx-*.png`. Total: 818 arquivos deixam de ser rastreados. **O historico NAO foi mexido**: o que ja' foi publicado continua nos commits antigos (so' o passo 3, que nao foi feito, apagaria isso). Fica para decidir: `resource/` (passo 2), a origem dos tres arquivos do passo 4 e o texto do README (passo 5).

Consequencia para quem clonar o repositorio: os testes que precisam de uma BIOS (`controltest` com `dist/roms/bios/MSX2P.ROM`, `asme2etest` e `audiotest` com `resource/fMSX/ROMs/MSX.ROM`) so' rodam onde a BIOS existir; a do autor continua em `dist/roms/` no disco dele.

## 4c. Respostas do autor (2026-10-22) e o que mudou

- **`fwMSX-splash.png`**: arte do proprio projeto. Registrado no `LICENSE-THIRD-PARTY.md` ("Arte propria").
- **`NBASIC.BIN`** (Konamiman; o repositorio dele **nao declara licenca**): o autor indicou `github.com/Konamiman/MSX/tree/master/SRC/NBASIC` como origem. Em vez de redistribui-lo, ele **saiu** do repositorio (`git rm --cached` + `.gitignore`) e dos pacotes (`build.ps1`/`build.sh` o removem do que sobrar em `dist/badig/`) e passou a ser **baixado sob demanda** pela janela Downloads (1.43.17, `doc/download-spec.md`). O `nbasic.dmx` (rotinas `.NB_*`, do msxIDE do proprio autor) continua nos pacotes.
- **`SUPER-X-PT.notas`** e **`resource/`**: seguem pendentes (origem do arquivo; decisao pasta por pasta).

## 5. O que este documento nao resolve

- Nao e' parecer juridico. A tabela diz o que esta' publicado e de quem parece ser; a decisao de risco e' do autor.
- Nao verifiquei arquivo por arquivo o conteudo de `resource/fmsxgo/` etc. (50 mil arquivos): a contagem e' por extensao.
- A origem de `fwMSX-splash.png`, `NBASIC.BIN` e `nbasic.dmx` depende de informacao que so' o autor tem.
