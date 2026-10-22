// fwMSX -- MAN/HELP: paginas das ferramentas do console (newdisk, asm, romdb, cas, fitadb, msxdisk). Texto sem acento.
#include "man_internal.h"

namespace man {

std::vector<Topic> ToolTopics() {
    std::vector<Topic> v;
    auto add = [&](const char *name, const char *usage, const char *summary, const char *page) {
        v.push_back(Topic{name, "", "Ferramentas", usage, summary, page, true});
    };

    add("newdisk", "newdisk <arq.dsk> <formato>", "cria um disquete novo e formatado",
        R"MAN(DESCRICAO
  Cria uma imagem de disco .dsk vazia e ja' formatada para o MSX-DOS.

  Formatos:
    ss525   5 1/4 face simples   180 KB
    ds525   5 1/4 face dupla     360 KB
    ss35    3 1/2 face simples   360 KB
    ds35    3 1/2 face dupla     720 KB

EXEMPLOS
  newdisk novo.dsk ds35
  emu start --disk novo.dsk

VER TAMBEM
  disk, msxdisk)MAN");

    add("asm", "asm [-z] [-s] [-vv] [-o <base>] <arq.asm>", "montador Z80 do fwMSX (o fwmsx --asm)",
        R"MAN(DESCRICAO
  Monta um programa Z80 em assembly (fwasm). Mesmo programa do "fwmsx
  --asm".

OPCOES
  -z     sintaxe Zilog padrao (sem precisar da diretiva .zilog)
  -s     silencioso (sem mensagens de progresso)
  -vv    mensagens detalhadas
  -o X   saida: nome-base (sem extensao), ou uma pasta terminada em / ou \ (usa o nome do fonte)

EXEMPLOS
  asm hello.asm
  asm -z -o saida/ hello.asm

VER TAMBEM
  fwasm, cart, dis)MAN");

    add("badig", "badig [opcoes] <fonte.dmx> [saida.amx]", "Basic Dignified: converte .dmx em .amx e no BASIC tokenizado .bmx",
        R"MAN(DESCRICAO
  Clone do Basic Dignified Suite (v2.2.1, de Fred Rique) para o MSX: le o
  programa em BASIC Dignified (.dmx: sem numeros de linha, com labels,
  defines, funcoes, variaveis de nome longo, includes...) e gera

    .amx   o BASIC ASCII classico, linhas numeradas
    .bmx   o mesmo programa tokenizado, o formato do arquivo .BAS do MSX
    .lmx   a listagem do tokenizador (opcional)

  E' o mesmo programa do "fwmsx --badig". Na janela do editor: F9 converte o
  texto da aba (e leva o cursor ao erro), F5 converte e roda no emulador.

COMANDOS
  badig <opcoes> <fonte.dmx>   converte e grava ao lado do fonte
  badigrun <fonte.dmx>         converte, monta um disquete com o .bmx e um
                               AUTOEXEC.BAS que o roda, insere em A: e
                               reinicia (precisa de interface de disquete)

OPCOES (as do badig.py)
  -ls N  linha inicial (10)     -lp N  passo (10)     -rh  sem o cabecalho REM
  -ss    tira todos os espacos  -ca    maiusculas     -tl N  tamanho do TAB
  -tr    traduz Unicode para os graficos do MSX (fonte em UTF-8)
  -cp ?|p     troca PRINT por ? (ou o contrario)
  -tg t|g     tira o THEN antes do GOTO (t) ou o GOTO depois do THEN/ELSE (g)
  -lbr -lnr -var -lex -par    relatorios: labels, linhas, variaveis, tokens
  -prr   mostra os relatorios na tela em vez de gravar
  -asc   o arquivo ja' e' BASIC ASCII: so' tokeniza
  --no_tokenize   so' gera o .amx    --tk_list N   gera o .lmx (N bytes/linha)
  --tk_del_ascii  apaga o .amx depois de tokenizar

REMTAGS (no proprio fonte, valem sobre as opcoes)
  ##BB:arguments=-ss -ca     mais argumentos
  ##BB:export_file=nome      nome do .amx
  ##BB:tk_tokenize=false     ##BB:tk_list=16     ##BB:tk_del_ascii=true
  ##BB:disk_files=dados.txt *.bin   arquivos de apoio no disco do badigrun
  ##BB:nbasic=true                   leva o NBASIC.BIN ao disco (automatico
                                     com include "nbasic.dmx", que fica na
                                     pasta badig/ ao lado do executavel)

CONFIGURACAO
  As opcoes ficam no badig.cfg (ao lado do executavel), com os nomes do
  badig.ini original. No editor: Montar > Dignified setup...

EXEMPLOS
  badig jogo.dmx
  badig -ss -ca --tk_list 16 jogo.dmx
  badigrun jogo.dmx

VER TAMBEM
  dignified, asm, asmrun)MAN");

    add("project", "project <new|open|close|info|ls|add|get|rm|set|root|status|sync|pull|push|export|import> ...",
        "o projeto .msxproject: UM arquivo com tudo, e as copias soltas numa pasta",
        R"MAN(DESCRICAO
  Um projeto e' UM arquivo SQLite (.msxproject) que guarda tudo: fontes em
  BASIC/Dignified/Assembly/Pascal, sprites, telas, musicas, MML/DML, notas em
  Markdown, PDF, discos .dsk, fitas .tsx/.cas e a configuracao (modelo do MSX,
  opcoes do emulador...). Para levar o projeto a outro PC basta copiar esse
  arquivo.

  Alem disso o projeto mantem COPIAS SOLTAS numa pasta (src/jogo.bas,
  discos/jogo.dsk...), em sincronia com o arquivo. A pasta padrao e' a que
  contem o .msxproject; "project root" muda.

COMANDOS
  new <arq> [nome]         cria o projeto (o arquivo nao pode existir)
  open <arq> | close       abre / fecha
  info | ls [prefixo]      resumo e lista de arquivos (tipo, tamanho, caminho)
  add <arq> [caminho]      poe um arquivo no projeto (e na pasta)
  get <caminho> <destino>  tira um arquivo do projeto para um destino qualquer
  rm <caminho>             tira do projeto e apaga a copia solta
  set [chave [valor]]      configuracao do projeto; set -d chave apaga
  root [pasta]             a pasta das copias soltas

SINCRONIZAR (projeto x pasta)
  status [pasta]           mostra o que difere e o que fazer
  sync [--prune] [pasta]   aplica o que nao perde trabalho:
                             so' a pasta mudou     -> pasta para o projeto
                             so' o projeto mudou   -> projeto para a pasta
                             arquivo novo na pasta -> entra no projeto
                             so' no projeto        -> extrai para a pasta
                           Se os DOIS mudaram e' CONFLITO: nada e' mexido, o
                           comando diz qual lado e' o mais novo e voce decide
                             pull <caminho>   fica o projeto
                             push <caminho>   fica a pasta
                           O que sumiu da pasta so' sai do projeto com --prune.
                           O que foi TIRADO do projeto (em outro PC) e ainda
                           tem copia aqui so' e' listado; --prune apaga a copia
                           se ela nao mudou (o conteudo fica no historico).
  rev <caminho>            historico do arquivo: as ultimas versoes (padrao 10,
                           project set revisions.keep N; 0 desliga)
  restore <caminho> <id>   volta o arquivo a uma versao (projeto e pasta); serve
                           tambem para um arquivo tirado
  purge <caminho>          apaga a copia da pasta de um arquivo que saiu do projeto
  export <pasta>           extrai TUDO (o projeto vence) e grava o manifesto
  import <pasta> <novo>    cria um projeto novo a partir de uma pasta (o
                           clone do GitHub): usa o manifesto msxproject.txt

  O "mais novo" e' so' uma sugestao: a comparacao usa o hash da ultima
  sincronizacao (guardado em .msxsync, na pasta), nao so' a data, porque a data
  muda com git clone, zip e relogios diferentes. Em arquivos de TEXTO o fim de
  linha (LF x CRLF) nao conta como alteracao.

GITHUB
  Versione a PASTA (as copias soltas + msxproject.txt), nao o .msxproject: o
  banco binario nao tem diff nem merge. Ponha *.msxproject e .msxsync no
  .gitignore. Depois de um clone: project import <pasta> <novo.msxproject>.

EXEMPLOS
  project new jogo.msxproject "Meu Jogo"
  project add src/jogo.bas
  project set machine MSX2
  project sync
  project status

VER TAMBEM
  badig, asm)MAN");

    add("editor", "editor [--edit|--view] [arquivo [arquivo2]]", "o editor de textos do fwMSX (estilo Norton Editor 2) com menus, projeto e Build",
        R"MAN(DESCRICAO
  Editor de textos para fontes de MSX (BASIC, Basic Dignified, Assembly Z80,
  Pascal...), no estilo do Norton Editor 2, em duas versoes do MESMO codigo:
  na tela de texto (fwMSXc --edit) e numa janela grafica (fwMSX --edit, ou
  embutido no emulador). As mesmas teclas valem nas duas.

COMO ABRIR
  fwMSXc --edit [arquivo [arquivo2]]   um arquivo novo abre vazio com esse nome
  fwMSXc --view arquivo                somente leitura
  fwMSXc --edit --dump --keys "A-f o"  imprime a tela e sai (para testes)

MENUS (Alt+letra, ou F10; setas, Enter, a letra do item, Esc)
  File (F)     New, Open, Save, Save As, Merge, Close window, Load more, Write to
               cursor, Top of file, Print (PDF), Print Block, Eject page, Print
               setup, eXit
  Edit (E)     Undo, Redo, Copy, Move, Delete, marcadores de bloco, Indent,
               Unindent, Output to file, Paragraph format, Document format,
               Center line
  Search (S)   Find, Find and Replace, Continue, Line number, Matching bracket,
               Window Differences, Text in Block, Complete word
  View (V)     Information, Outline, Markdown preview (F7), Language, 2nd window
               (F2)
  Project (J)  New/Open/Close project, Files, Add current file, Sync
  Run (R)      Assemble/Convert (F9), Run on MSX (F5), Build disk (F6), Next/Previous
               message
               (F8, Shift-F8), Messages, BASIC line, Dignified setup
  Tools (T)    Display, Mouse, Editor, Save configuration, Text layout setup,
               Word wrap, Auto indent, Expand tabs, Put in Tabs, Insert/Remove
               page breaks, DOS shell
  F1=Help      ajuda paginada: um topico por menu, Keys e On word (Ctrl-F1)
  (ate' a 1.43.7 eram os dez menus do NE: Block e Text-layout foram para Edit e
  Tools, Print para File, Options para Tools e Build para Run.)

TECLAS DO NORTON (continuam as mesmas)
  Ctrl-F / Ctrl-R buscam e trocam, Alt-C / Ctrl-C continuam, Ctrl-] acha o
  parentese, Ctrl-W / Alt-W apagam palavra, Alt-K apaga a linha, Ctrl-U restaura
  o apagado, Alt-E S (era Alt-B S) marca o bloco. Modernas: Alt-Backspace
  desfaz, Ctrl-Ins / Shift-Ins / Shift-Del copiam, colam e recortam.

INDICE E PREVIEW
  View > Outline display lista os titulos (Markdown), as linhas REM (MSX-BASIC),
  os rotulos (Assembly) ou as linhas da coluna 1, e Enter salta para a escolhida.
  View > Markdown preview (F7) mostra o .md renderizado; Esc fecha.

FORMATAR
  Edit > Paragraph format (Alt-E P) refaz as quebras de linha do paragrafo do
  cursor pelo Tools > Text layout setup (largura, margem esquerda, recuo da
  1a linha) e vai para o seguinte; Edit > Document format (Alt-E G) faz o
  mesmo com todos. Linha em branco e quebra de pagina separam paragrafos.

IMPRIMIR
  File > Print grava o arquivo todo em um PDF A4 (Courier) e Print Block so' o
  bloco marcado; o nome sugerido e' o do arquivo com .pdf. Print setup define a
  margem superior, a margem esquerda e as linhas por folha, e as quebras de
  pagina (Tools > Insert page breaks) abrem folha nova.

COMPLETAR
  Alt-/ ou Ctrl-Espaco lista os candidatos para a palavra sob o cursor: as
  palavras da linguagem, os gatilhos de snippet (Tab expande), as palavras do
  texto e, com um PROJETO aberto, os SIMBOLOS dos outros arquivos dele: em
  Assembly os rotulos e as constantes (nome:, nome equ, nome =, macro), nos
  arquivos .sym o primeiro nome de cada linha, e nos demais fontes os
  identificadores. A lista e' refeita ao abrir o projeto, sincronizar e salvar.

PROJETO E BUILD
  Com um projeto aberto (man project) o arquivo salvo na pasta vai tambem para o
  .msxproject, o nome aparece na barra de estado, a arvore de arquivos fica na
  janela (e em Project > Files... na tela de texto, com o menu de contexto) e o
  F9 / F5 leem os include / incbin do proprio projeto antes da pasta.

ARQUIVOS
  fwedit.cfg       configuracao (Tools > Save configuration)
  fwedit.project   o ultimo projeto aberto (reaberto na proxima sessao)
  fwedit.session   as abas abertas
  fwedit.snippets  seus snippets: [idDaLinguagem] e gatilho=texto

VER TAMBEM
  project, badig, asm, asmrun)MAN");

    add("obi", "obi <build|check> [Obifile]", "monta um disquete .dsk pronto para rodar a partir de uma receita (Obifile)",
        R"MAN(DESCRICAO
  O obi le uma receita de texto (Obifile) com os arquivos do seu programa -- o
  fonte Basic Dignified (.dmx), o fonte Assembly (.asm), dados (.dat, .bin),
  sprites (.spr), telas (.scr)... -- converte o que precisa e grava tudo num
  disquete .dsk de 720 KB que o fwMSX roda sozinho.

COMANDOS
  obi build [Obifile]      monta e grava o .dsk (sem argumento: o Obifile da
                           pasta atual, ou o primeiro *.obi)
  obi check [Obifile]      monta e confere tudo, sem gravar o .dsk
  obi dosdisk <1|2> [saida.dsk] [--tools]
                           gera o disquete de sistema do MSX-DOS 1 ou 2 (padrao:
                           emulator/msxdosN.dsk; a build ja' gera os dois)
  obirun [Obifile]         no console do emulador: monta, insere o disco em A: e
                           reinicia (o emulador precisa de --disk)
  fwmsx --obi build ...    o mesmo, fora do emulador
  Run > Build disk (F6)    no editor: usa o Obifile aberto, o do projeto ou o da
                           pasta do arquivo; os fontes vem dos buffers abertos

O OBIFILE
  # comentario
  target: jogo.dsk          o disco a gerar
  boot: JOGO.BMX            quem roda sozinho (padrao: o primeiro .BMX)
  dos: 1                    1 ou 2: o disco leva o MSX-DOS 1 ou 2 (arquivos de
                            emulator/msxdos1 e emulator/msxdos2)
  dos_tools: false          true (com dos: 2): leva tambem UTILS, HELP...
  files:
    - jogo.dmx              Basic Dignified -> JOGO.BMX
    - rotina.asm            Assembly -> ROTINA.BIN, .COM ou .ROM
    - dados.dat             copiado como esta'
    - source: hero.spr      outro nome no disco
      name: HERO.SPR

COMO O DISCO INICIA
  boot: X.BMX  -> AUTOEXEC.BAS com RUN "X.BMX"
  boot: X.BIN  -> AUTOEXEC.BAS com BLOAD "X.BIN",R
  boot: X.COM  -> AUTOEXEC.BAT (precisa de dos: 1 ou 2)

NOMES
  No disco o nome e' o do arquivo em maiusculas, no formato 8.3. Se nao couber a
  build para e pede "name:". O log mostra cada arquivo com tamanho e CRC-32.

VER TAMBEM
  badig, asm, asmrun, project, editor)MAN");

    add("romdb", "romdb <comando> [opcoes]", "banco de ROMs: downloads, busca, verificacao",
        R"MAN(DESCRICAO
  Banco de ROMs (roms/roms.db ao lado do executavel; --roms <pasta> muda a
  pasta). Identifica as ROMs pelo SHA-1 e guarda o mapper de cada uma, que o
  "cart" usa sozinho.

DOWNLOADS
  fmsx                       baixa o fMSX 6.0 e separa as ROMs por tipo
  filehunter [caminho]       lista a pasta System ROMs do file-hunter
  filehunter-full [--data DD-MM-AAAA]
                             baixa o Full Set System ROMs
  filehunter-get <url>       baixa um arquivo ou ZIP do file-hunter
  vampier                    baixa e importa o banco do Vampier (msxromdb)

BANCO
  scan <pasta> [--origem x]  le e cadastra as ROMs de uma pasta
  cartsha <arquivo>          importa o CARTS.SHA do fMSX (mapper por SHA-1)
  add <arq> [--cat c] [--hw h] [--nome n] [--notas t]
  list [--cat c]             lista as ROMs
  search <texto> [--cat c]   busca por nome, SHA-1, hardware, notas, caminho
  show <id|sha1>             mostra uma ROM
  edit <id> [--cat c] [--hw h] [--nome n] [--mapper n] [--notas t]
  del <id>                   remove do banco (o arquivo fica)
  vsearch <texto>            busca no banco do Vampier
  identify                   preenche os nomes que o Vampier conhece
  verify [--cat c]           recalcula o SHA-1 (faltando/alterada: saida 1)
  stats                      contagem por categoria

  Categorias: bios, interface, cartucho, disco, tabela, outro.

EXEMPLOS
  romdb scan D:\msx\roms
  romdb search nemesis
  romdb stats

VER TAMBEM
  cart)MAN");

    add("cas", "cas pack|rip|list ...", "ferramentas de fita .CAS/.TSX",
        R"MAN(DESCRICAO
  pack --tipo bin|bas --nome NOME [opcoes] <entrada> <saida.tsx|.cas>
       Empacota um arquivo solto numa fita valida, pronta para BLOAD/CLOAD "CAS:".
       --tipo bin  precisa de --inicio/--fim/--exec (hexadecimal 0xNNNN)
       --tipo bas  a entrada ja' tem de estar tokenizada (os bytes do CSAVE)
       --anexar <arq>  acrescenta a uma fita existente
  rip [--tolerancia N] [--anexar <arq>] <entrada.wav> <saida.tsx|.cas>
       Converte uma gravacao real de fita (.wav PCM mono 8/16 bits, formato KCS do MSX) em .TSX.
       --tolerancia 1-90 (padrao 25): suba para gravacoes ruidosas.
  list <arq.tsx|.tzx|.cas>
       Lista os arquivos da fita (indice, tipo, nome, tamanho).

EXEMPLOS
  cas list jogo.tsx cas pack --tipo bin --nome JOGO --inicio 0x8000 --fim
  0x8FFF --exec 0x8000 jogo.bin jogo.tsx

VER TAMBEM
  tape, fitadb)MAN");

    add("fitadb", "fitadb <comando> [opcoes]", "banco de fitas (so' cadastra o que voce tem)",
        R"MAN(DESCRICAO
  Banco de fitas (fitas/fitas.db). So' CADASTRA fitas que voce ja' tem no
  disco; nao baixa nada.
  scan <pasta> [--origem x]   cadastra as fitas de uma pasta (.cas/.tsx/.tzx, pelo SHA-1)
  add <arq> [--titulo t] [--empresa e] [--ano a] [--notas t]
  list | search <texto> | show <id|sha1> | stats
  edit <id> [--titulo t] [--empresa e] [--ano a] [--notas t]
  del <id>                    remove do banco (o arquivo fica)

VER TAMBEM
  tape, cas)MAN");

    add("msxdisk", "msxdisk", "utilitario de imagens de disco (shell proprio)",
        R"MAN(DESCRICAO
  Abre o shell do msxdisk: cria, lista e edita imagens de disco do MSX-DOS.
  Comandos de imagem: create, load, save, saveas, list/dir, add, extract,
  delete, rename, mkdir, rmdir, info, copy, put, get, mput, mget, prompt.
  Locais (como um FTP): ls, cd, md, rm, pwd. "tui" / "gui" abrem a TUI / GUI
  sem sair; "help" lista tudo; "exit" ou Ctrl-D sai.

VER TAMBEM
  newdisk, disk)MAN");

    return v;
}

} // namespace man
