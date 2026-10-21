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

CONFIGURACAO
  As opcoes ficam no badig.cfg (ao lado do executavel), com os nomes do
  badig.ini original. No editor: Montar > Dignified setup...

EXEMPLOS
  badig jogo.dmx
  badig -ss -ca --tk_list 16 jogo.dmx
  badigrun jogo.dmx

VER TAMBEM
  dignified, asm, asmrun)MAN");

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
