// Teste do Mamute (monitor Z80 do fwMSX): offline puro (memoria e CPU privadas) e online sobre a maquina emulada.
// Ver doc/mamute-spec.md.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "../../src/machine/machine.h"
#include "../../src/mamute/cpp/mamute_config.h"
#include "../../src/mamute/cpp/mamute_monitor.h"
#include "../../src/mamute/cpp/mamute_online.h"
#include "../../src/mamute/cpp/mamute_pdf.h"
#include "../../src/msxdisk/core/disk_image.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

bool has(const std::string &text, const std::string &part) { return text.find(part) != std::string::npos; }

// roda um comando e mostra a saida quando o teste falha (ajuda a depurar sem reexecutar)
std::string Run(mamute::Monitor &mon, const std::string &line) {
    const std::string out = mon.Execute(line);
    return out;
}

void checkOut(mamute::Monitor &mon, const std::string &cmd, const std::string &expect, const std::string &what) {
    const std::string out = Run(mon, cmd);
    const bool ok = has(out, expect);
    check(ok, what);
    if (!ok) std::printf("    '%s' => '%s' (esperava '%s')\n", cmd.c_str(), out.c_str(), expect.c_str());
}

std::string TempPath(const std::string &name) {
    const char *t = std::getenv("TEMP");
    return (t ? std::string(t) : std::string("/tmp")) + "/" + name;
}

std::vector<uint8_t> ReadAll(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

void TestOffline() {
    mamute::Config cfg; // padrao: slot 3 todo RAM
    mamute::Monitor mon(cfg);
    check(mon.Prompt() == "MON(off)> ", "offline: prompt MON(off)>");
    check(!mon.online(), "offline: comeca offline");
    checkOut(mon, "ONLINE", "SEM MAQUINA", "offline: ONLINE sem maquina recusa");
    checkOut(mon, "PAGE ?", "PAGINA 2 (8000-BFFF): SLOT 3 RAM", "offline: o boot poe tudo no slot 3 (RAM)");

    // memoria: M grava, DM mostra, D/C formatam
    checkOut(mon, "M C000,3E,2A,C9", "GRAVADOS 3 BYTES EM C000", "M grava bytes");
    checkOut(mon, "DM C000", "C000  3E 2A C9 00 00 00 00 00  >*.....", "DM mostra hexa e ASCII");
    checkOut(mon, "DM C000,+10", "DESLOC.: +10", "DM mostra o deslocamento");
    checkOut(mon, "D C000,C007", "C000  3E 2A C9 00  >*..", "D modo 0: 4 bytes por linha com ASCII");
    checkOut(mon, "C 1", "MODO 1", "C 1 troca o formato");
    checkOut(mon, "D C000,C00F", "C000  3E 2A C9 00 00 00 00 00 00 00 00 00 00 00 00 00  >*..", "D modo 1: 16 bytes por linha");
    Run(mon, "C 2");
    checkOut(mon, "D C001,C008", "C001  2A C9 00 00 00 00 00 00  F4", "D modo 2: checksum = soma + byte baixo do endereco");
    Run(mon, "C 3");
    checkOut(mon, "D C001,C008", "C001  2A C9 00 00 00 00 00 00  F3", "D modo 3: checksum = so' a soma");
    Run(mon, "C 0");

    // texto, deslocamento, busca
    Run(mon, "MS D000,'Mamute");
    checkOut(mon, "SH D000,'Qwerty", "NAO ENCONTRADO", "SH de texto sem resultado");
    checkOut(mon, "SH D000,'Mamute", "ACHADO EM D000 DESLOC +00", "SH de texto");
    Run(mon, "MS E000,20,'Zebra");
    checkOut(mon, "SH E000,'Zebra", "ACHADO EM E000 DESLOC +20", "SH acha texto cifrado por deslocamento");
    checkOut(mon, "SH C000,3E,,C9", "ACHADO EM C000", "SH de bytes com curinga");
    checkOut(mon, "SH C000,DE,AD,BE,EF", "NAO ENCONTRADO", "SH sem resultado");
    Run(mon, "M D010,5A");
    Run(mon, "M D020,5A");
    checkOut(mon, "SH D000,5A", "ACHADO EM D010", "SH acha o primeiro byte");
    checkOut(mon, "SH ,5A", "ACHADO EM D020", "SH sem endereco continua depois do ultimo achado");

    // T e F (com sobreposicao) e escrita em celula que nao e' RAM
    Run(mon, "F D100,D10F,AA");
    Run(mon, "M D100,01,02,03,04");
    checkOut(mon, "T D100,D10F,D102", "COPIADOS 16 BYTES", "T copia");
    checkOut(mon, "D D102,D105", "D102  01 02 03 04", "T com sobreposicao copia na ordem certa");
    checkOut(mon, "PAGE 0,0,0,0", "PAGINA 0 (0000-3FFF): SLOT 0 VAZIO", "PAGE troca o mapeamento");
    checkOut(mon, "M 0000,11", "IGNORADOS", "escrita em celula vazia e' ignorada");
    Run(mon, "PAGE");
    checkOut(mon, "PAGE ?", "PAGINA 0 (0000-3FFF): SLOT 3 RAM", "PAGE sozinho volta ao boot");

    // disassembler e execucao: LD B,5 / DEC B / JR NZ,-3 / LD A,77 / HALT
    Run(mon, "M C100,06,05,05,20,FD,3E,77,76");
    checkOut(mon, "L C100,C101", "C100  06 05", "L mostra endereco e bytes");
    checkOut(mon, "L C100,C101", "LD B", "L mostra o mnemonico");
    checkOut(mon, "X A 00", "A=00", "X grava um registrador de 8 bits");
    checkOut(mon, "G C100,C105", "PARADO NO BREAKPOINT C105", "G para no breakpoint");
    checkOut(mon, "X A", "A=00", "o breakpoint vale ANTES da instrucao (LD A,77 nao rodou)");
    checkOut(mon, "X B", "B=00", "o laco DEC B/JR NZ rodou ate zerar B");
    checkOut(mon, "G C100", "PARADO EM HALT", "G roda ate o HALT");
    checkOut(mon, "X A", "A=77", "o programa offline rodou de verdade (A = 77)");
    checkOut(mon, "X BC 1234", "BC=1234", "X grava um par de 16 bits");

    // LOAD e SAVE
    const std::string bin = TempPath("mamute_t.bin");
    const std::string rom = TempPath("mamute_t.rom");
    checkOut(mon, "SAVE " + bin + ",C000,C002", "FORMATO BIN", "SAVE grava com cabecalho BSAVE");
    const std::vector<uint8_t> b = ReadAll(bin);
    check(b.size() == 10 && b[0] == 0xFE && b[1] == 0x00 && b[2] == 0xC0 && b[3] == 0x02 && b[4] == 0xC0 && b[7] == 0x3E && b[8] == 0x2A && b[9] == 0xC9, "SAVE: cabecalho FE C000 C002 C000 + bytes");
    checkOut(mon, "SAVE " + rom + ",C000,C002", "FORMATO RAW", "SAVE de .rom sai sem cabecalho");
    check(ReadAll(rom).size() == 3, "SAVE .rom: 3 bytes crus");
    Run(mon, "F C000,C002,00");
    checkOut(mon, "LOAD " + bin, "CARREGADO NO SLOT 3 EM C000", "LOAD usa o cabecalho BLOAD");
    checkOut(mon, "D C000,C003", "C000  3E 2A C9", "LOAD gravou os bytes");
    checkOut(mon, "LOAD " + rom, "CARREGADO NO SLOT 3 EM 4000", "LOAD de .rom vai para 4000");
    std::remove(bin.c_str());
    std::remove(rom.c_str());

    // configuracao: uma ROM em slot 0 pagina 1 aparece depois de PAGE
    const std::string romfile = TempPath("mamute_rom.bin");
    {
        std::vector<uint8_t> big(0x4000, 0);
        big[0] = 'A';
        big[1] = 'B';
        std::ofstream f(romfile, std::ios::binary);
        f.write(reinterpret_cast<const char *>(big.data()), static_cast<std::streamsize>(big.size()));
    }
    checkOut(mon, "CFG SET 0,1,ROM," + romfile, "SLOT 0 PAGINA 1: ROM", "CFG SET configura uma celula");
    checkOut(mon, "PAGE 3,0,3,3", "PAGINA 1 (4000-7FFF): SLOT 0 ROM", "PAGE liga a ROM");
    checkOut(mon, "D 4000,4001", "4000  41 42", "a ROM da configuracao esta na memoria");
    checkOut(mon, "M 4000,00", "IGNORADOS", "ROM nao aceita escrita");

    const std::string cfgfile = TempPath("mamute_t.cfg");
    mamute::Config c2;
    c2.start_online = true;
    c2.vram_kb = 128;
    c2.dump_mode = 2;
    c2.cell[0][0] = mamute::CellType::Rom;
    c2.cell[3][0] = mamute::CellType::Empty;
    c2.file[0][0] = "bios.rom";
    c2.half[0][1] = true;
    check(mamute::SaveConfig(cfgfile, c2), "config: grava");
    const mamute::Config c3 = mamute::LoadConfig(cfgfile);
    check(c3.start_online && c3.vram_kb == 128 && c3.dump_mode == 2 && c3.cell[0][0] == mamute::CellType::Rom && c3.file[0][0] == "bios.rom" && c3.half[0][1],
          "config: ida e volta preserva modo, VRAM, formato, celulas e arquivos");
    check(c3.cell[3][0] == mamute::CellType::Empty, "config: com celulas no arquivo, o padrao 'slot 3 = RAM' nao vale");
    std::remove(cfgfile.c_str());
    std::remove(romfile.c_str());

    // erros e ajuda
    checkOut(mon, "QUUX", "COMANDO DESCONHECIDO", "comando desconhecido");
    checkOut(mon, "D 4000,3FFF", "FIM NAO PODE SER MENOR", "D recusa fim < inicio");
    checkOut(mon, "HELP", "COMANDOS DO MAMUTE", "HELP lista os comandos");
    bool quit = false;
    mon.Execute("BA", &quit);
    check(quit, "BA encerra o monitor");
}

void TestEdit() {
    mamute::Config cfg;
    mamute::Monitor mon(cfg);
    checkOut(mon, "EDIT", "EDITOR DE FONTE Z80", "EDIT entra no editor");
    check(mon.Prompt() == "EDIT> " && mon.editing(), "EDIT: prompt EDIT>");
    // programa do manual do MegaAssembler (numeros em HEXA por padrao)
    const char *const prog[] = {"10 ORG 0C100H",     "20 CHPUT: EQU 0A2H", "30 START: LD HL,PRINT", "40 LOOP: LD A,(HL)", "50 OR A",
                                "60 RET Z",          "70 CALL CHPUT",      "80 INC HL",             "90 JR LOOP",         "100 PRINT: DEFM 'MEGA ASSEMBLER'",
                                "110 DEFB 0",        "120 END"};
    for (const char *l : prog) mon.Execute(l);
    checkOut(mon, "LIST 30-50", "30   START: LD HL,PRINT", "LIST mostra o intervalo");
    checkOut(mon, "A", "PASSO-1", "A: PASSO-1/PASSO-2 e listagem");
    checkOut(mon, "A", "   30 C100  21 0C C1     START: LD HL,PRINT", "A: numero, endereco, bytes e a linha (LD HL,PRINT = 21 0C C1)");
    checkOut(mon, "A", "   20 00A2", "A: a coluna do endereco mostra o valor do EQU");
    checkOut(mon, "A", "  100 C10C  4D 45 47 41", "A: DEFM vira os caracteres");
    checkOut(mon, "A", "C110  20 41 53 53", "A: linha extra para o que passa de 4 bytes");
    check(!has(Run(mon, "A"), "ORG 0C100H"), "A: ORG e END nao aparecem na listagem");
    checkOut(mon, "MAP", "INICIO C100 FIM C11A (27 BYTES)", "MAP: inicio e fim do codigo montado");
    checkOut(mon, "A N", "C100  21 0C C1     START: LD HL,PRINT", "A N: sem a coluna do numero da linha");
    checkOut(mon, "A R", "LOOP         C103     C10A", "A R: referencia cruzada (LOOP e' usado em C10A)");
    checkOut(mon, "A S", "SIMBOLOS (ORDEM ALFABETICA)", "A S: lista alfabetica");
    checkOut(mon, "A D", "SIMBOLOS (ORDEM DE APARICAO)", "A D: lista em ordem de aparicao");
    checkOut(mon, "A H", "EXIGE S OU D", "A H sem S nem D recusa");

    // O grava na memoria no endereco do ORG; /desloc soma a todo ORG
    checkOut(mon, "A O", "GRAVADOS 27 BYTES NA MEMORIA (OFFLINE) EM C100-C11A", "A O escreve o objeto na memoria");
    mon.Execute("QUIT");
    check(!mon.editing() && mon.Prompt() == "MON(off)> ", "QUIT volta ao monitor (o prompt muda)");
    checkOut(mon, "D C100,C102", "C100  21 0C C1", "o codigo montado esta' na memoria");
    checkOut(mon, "L C100,C100", "LD HL,C10Ch", "e disassembla de volta");
    checkOut(mon, "A/10", "C110  21 1C C1", "A /10 desloca todo ORG (e os rotulos junto)");
    checkOut(mon, "MAP", "INICIO C110", "MAP depois do deslocamento");

    // numeros: hexa padrao, H, B (binario), D (decimal)
    mon.Execute("EDIT NEW");
    mon.Execute("EDIT 10 ORG 8000H");
    mon.Execute("EDIT 20 LD A,10");
    mon.Execute("EDIT 30 LD A,10D");
    mon.Execute("EDIT 40 LD A,1010B");
    mon.Execute("EDIT 50 LD A,0FFH");
    mon.Execute("EDIT 60 LD (IX+5),A");
    mon.Execute("EDIT 70 EX AF,AF'");
    checkOut(mon, "EDIT A", "3E 10", "numero sem sufixo e' hexa (LD A,10 = 3E 10)");
    checkOut(mon, "EDIT A", "3E 0A        LD A,10D", "sufixo D e' decimal");
    checkOut(mon, "EDIT A", "3E 0A        LD A,1010B", "sufixo B e' binario");
    checkOut(mon, "EDIT A", "3E FF", "sufixo H");
    checkOut(mon, "EDIT A", "DD 77 05", "indexado (IX+5)");
    checkOut(mon, "EDIT A", "08", "EX AF,AF' (o apostrofo do registrador alternado nao e' texto)");

    // erros voltam com o numero da linha do Mamute
    mon.Execute("EDIT 55 FOO BAR");
    checkOut(mon, "EDIT A", "ERRO NA LINHA 55", "erro de montagem aponta a linha do Mamute");
    checkOut(mon, "EDIT A", "> 55   FOO BAR", "e mostra a linha com o cursor >");
    check(has(Run(mon, "MAP"), "NADA MONTADO"), "MAP depois de um erro: nada montado");
    mon.Execute("EDIT 55");
    checkOut(mon, "EDIT LIST", "50   LD A,0FFH", "linha so' com o numero apaga");
    check(!has(Run(mon, "EDIT LIST"), "FOO"), "a linha 55 foi apagada");
    mon.Execute("EDIT 35 EQU 5");
    checkOut(mon, "EDIT A", "EQU EXIGE UM ROTULO", "EQU sem rotulo");
    mon.Execute("EDIT 35");

    // gerenciamento: RENUM, DELETE, CHANGE, SEARCH, SAVE/LOAD/MERGE
    checkOut(mon, "EDIT RENUM", "RENUMERADO", "RENUM");
    checkOut(mon, "EDIT LIST 10-30", "20   LD A,10", "RENUM: passo 10 a partir de 10");
    checkOut(mon, "EDIT CHANGE 'LD A','LD B'", "TROCADAS 4", "CHANGE troca em todas as linhas");
    checkOut(mon, "EDIT SEARCH 'LD B,0FFH'", "LD B,0FFH", "SEARCH entre aspas");
    checkOut(mon, "EDIT SEARCH ld b,0ffh", "LD B,0FFH", "SEARCH sem aspas ignora maiusculas");
    checkOut(mon, "EDIT SEARCH 'ld b,0ffh'", "NAO ENCONTRADO", "SEARCH entre aspas conta maiusculas");
    const std::string mza = TempPath("mamute_t.mza");
    checkOut(mon, "EDIT SAVE " + mza, "SALVO", "SAVE grava o fonte em ASCII");
    mon.Execute("EDIT DELETE 30-");
    checkOut(mon, "EDIT LIST", "20   LD B,10", "DELETE a- apaga ate' o fim");
    check(!has(Run(mon, "EDIT LIST"), "LD B,0FFH"), "o fim do programa foi apagado");
    checkOut(mon, "EDIT MERGE " + mza, "FUNDIDO", "MERGE funde");
    checkOut(mon, "EDIT LIST", "LD B,0FFH", "MERGE traz as linhas do arquivo de volta");
    mon.Execute("EDIT NEW");
    checkOut(mon, "EDIT LOAD " + mza, "CARREGADO", "LOAD substitui o programa");
    checkOut(mon, "EDIT LIST 10-10", "ORG 8000H", "LOAD: o conteudo do arquivo");
    std::remove(mza.c_str());

    // P (listagem em arquivo), H, I (BSAVE): o nome-base vem de >nome
    const std::string base = TempPath("mamute_asm");
    checkOut(mon, "EDIT A PIR>" + base + ".lst", "GRAVADO", "A P/I gravam arquivos (>nome.lst = texto)");
    check(!ReadAll(base + ".lst").empty(), "A P: NOME.lst com a listagem");
    const std::vector<uint8_t> bin = ReadAll(base + ".bin");
    check(bin.size() >= 7 && bin[0] == 0xFE && bin[1] == 0x00 && bin[2] == 0x80 && bin[7] == 0x06 && bin[8] == 0x10, "A I: BSAVE FE 8000 ... e o codigo (LD B,10 = 06 10)");
    std::remove((base + ".lst").c_str());
    std::remove((base + ".bin").c_str());
    checkOut(mon, "EDIT HELP", "EDITOR DE FONTE Z80", "HELP do editor");
    checkOut(mon, "EDIT BLA", "COMANDO DESCONHECIDO NO EDITOR", "comando desconhecido no editor");

    // o programa sobrevive a sair e voltar
    mon.Execute("EDIT");
    mon.Execute("QUIT");
    checkOut(mon, "LIST", "ORG 8000H", "LIST no monitor: o programa continua depois de sair do EDIT");
    bool quit = false;
    mon.Execute("EDIT", &quit);
    mon.Execute("BA", &quit);
    check(quit && !mon.editing(), "BA dentro do EDIT sai do monitor");
}

// Os paineis e as telas da Fase 3: o modelo recebe teclas por nome (@key), como a TUI manda.
void TestScreens() {
    const std::string nl = "\n";
    mamute::Config cfg;
    mamute::Monitor mon(cfg);

    // ---- PDF: P, LP, V, A P e A H ----
    check(mamute::IsPdfName("x.PDF") && !mamute::IsPdfName("x.txt"), "IsPdfName");
    {
        std::vector<std::string> many;
        for (int i = 0; i < 150; ++i) many.push_back("linha " + std::to_string(i) + " (com parenteses)");
        const std::string pdf = mamute::BuildPdf("TESTE", many);
        check(pdf.rfind("%PDF-1.4", 0) == 0 && has(pdf, "/Count 3") && has(pdf, "%%EOF") && has(pdf, "linha 149 \\(com parenteses\\)") && has(pdf, "/BaseFont /Courier"),
              "PDF: 150 linhas = 3 folhas A4 em Courier, com os parenteses escapados");
    }
    {
        // paginacao do editor (Print setup): margem superior/esquerda, linhas por folha e o form feed do texto
        std::vector<std::string> txt;
        for (int i = 0; i < 5; ++i) txt.push_back("l" + std::to_string(i));
        txt.push_back("\f");
        txt.push_back("depois");
        txt.push_back(std::string(300, 'x'));
        mamute::PdfLayout lay;
        lay.lines_per_page = 3;
        lay.top_blank = 1;
        lay.left_cols = 4;
        const std::string pdf = mamute::BuildPdfLayout("T", txt, lay);
        // l0..l2 | l3 l4 (o form feed fecha a folha) | depois + o resto da linha larga dobrada em duas
        check(has(pdf, "/Count 4") && has(pdf, "(    l4) Tj") && has(pdf, "(    depois) Tj"), "PDF com layout: linhas por folha, margem esquerda e form feed");
        check(!has(pdf, "(" + std::string(300, 'x')), "PDF com layout: a linha mais larga que a folha e' dobrada");
    }
    const std::string pdfp = TempPath("mamute_t_dump.pdf");
    mon.Execute("M C000,3E,2A,C9");
    checkOut(mon, "P C000,C00F," + pdfp, "(PDF)", "P grava um PDF quando o nome termina em .pdf");
    {
        const std::vector<uint8_t> pdf = ReadAll(pdfp);
        check(pdf.size() > 200 && pdf[0] == '%' && pdf[1] == 'P', "P: o arquivo e' um PDF");
    }
    const std::string txtp = TempPath("mamute_t_dump.txt");
    checkOut(mon, "P C000,C00F," + txtp, "GRAVADO", "P com outro nome grava texto");
    {
        const std::vector<uint8_t> txt = ReadAll(txtp);
        check(has(std::string(txt.begin(), txt.end()), "C000  3E 2A C9"), "P: o texto tem o despejo");
    }
    std::remove(pdfp.c_str());
    std::remove(txtp.c_str());
    {
        const std::string pdf2 = TempPath("mamute_t_lst.pdf");
        checkOut(mon, "LP C000,C002," + pdf2, "(PDF)", "LP grava um PDF");
        std::remove(pdf2.c_str());
        mon.Execute("EDIT NEW");
        mon.Execute("EDIT 10 ORG 8000H");
        mon.Execute("EDIT 20 LBL: LD A,1");
        const std::string base = TempPath("mamute_t_asm");
        checkOut(mon, "EDIT A PSH>" + base, "mamute_t_asm.pdf (PDF)", "A P: a listagem em PDF");
        check(has(Run(mon, "EDIT A PSH>" + base), "_labels.pdf (PDF)"), "A H: as listas de simbolos num PDF separado");
        std::remove((base + ".pdf").c_str());
        std::remove((base + "_labels.pdf").c_str());
    }

    // ---- versoes de linha ----
    mon.Execute("F D000,D0FF,00");
    checkOut(mon, "DM D000", "ENDERECO: D000", "DM de linha");
    checkOut(mon, "DM", "ENDERECO: D080", "DM sozinho continua 128 bytes depois");
    mon.Execute("M D000,11");
    checkOut(mon, "S D010,1q", "GRAVADOS 1 BYTES EM D010", "S de linha: as teclas do teclado S (1 q = 1 5 = 15)");
    checkOut(mon, "D D010,D010", "D010  15", "S gravou 15");
    checkOut(mon, "S D012,k1", "TECLA FORA DO TECLADO S", "S recusa tecla fora do teclado");
    checkOut(mon, "CFG SKEYS ABCDEFGHIJKLMNOP", "TECLADO S", "CFG SKEYS");
    mon.Execute("CFG SKEYS 1234qwerasdfzxcv");

    // ---- paineis: o hospedeiro liga (@panels on) e manda as teclas ----
    mon.Execute("@panels on");
    std::string r = mon.Execute("DM C000");
    check(r.rfind("action=show panel=mamute title=DM", 0) == 0, "DM com paineis ligados abre o painel (protocolo da TUI)");
    check(has(r, "\x01" "3E\x02") && has(r, "ENDERECO: C000") && has(r, "DESLOC.: +00"), "painel DM: o cursor marcado e as linhas de status");
    r = mon.Execute("@key right");
    check(has(r, "\x01" "2A\x02"), "seta direita move o cursor");
    r = mon.Execute("@key down");
    check(has(r, "CURSOR: C009"), "seta baixo anda uma linha (8 bytes)");
    r = mon.Execute("@key pgdn");
    check(has(r, "ENDERECO: C080"), "PgDn pula 128 bytes");
    r = mon.Execute("@key pgup");
    check(has(r, "ENDERECO: C000"), "PgUp volta 128 bytes");
    mon.Execute("@key plus");
    r = mon.Execute("@key plus");
    check(has(r, "DESLOC.: +02"), "+ aumenta o deslocamento");
    mon.Execute("@key minus");
    mon.Execute("@key minus");
    mon.Execute("@key up");
    mon.Execute("@key left");
    mon.Execute("@key left");
    r = mon.Execute("@key enter"); // campo de edicao no byte C000
    check(has(r, "EDITANDO (HEXA): 3E_"), "ENTER abre o campo com o valor atual");
    mon.Execute("@key c:F");
    mon.Execute("@key c:F");
    r = mon.Execute("@key enter");
    checkOut(mon, "D C000,C000", "C000  FF", "o painel DM gravou FF (campo hexa)");
    check(has(r, "\x01" "2A\x02"), "depois de gravar o cursor avanca");
    mon.Execute("@key tab");
    mon.Execute("@key enter");
    mon.Execute("@key c:H");
    mon.Execute("@key c:i");
    r = mon.Execute("@key enter");
    checkOut(mon, "D C001,C002", "C001  48 69", "o painel DM no bloco de texto grava os caracteres (H i = 48 69)");
    mon.Execute("@key enter");
    mon.Execute("@key c:Z");
    r = mon.Execute("@key esc"); // cancela o campo
    check(!has(r, "action=close"), "ESC com o campo aberto cancela o campo e nao fecha");
    checkOut(mon, "D C003,C003", "C003  00", "e nao gravou nada");
    r = mon.Execute("@key esc");
    check(r == "action=close panel=mamute", "ESC fecha o painel");

    // M: dois digitos hexa gravam direto
    mon.Execute("M C100");
    r = mon.Execute("@key c:3");
    check(has(r, "3_"), "M: o primeiro digito fica pendente (3_)");
    mon.Execute("@key c:F");
    mon.Execute("@key c:A");
    mon.Execute("@key c:B");
    checkOut(mon, "D C100,C101", "C100  3F AB", "M: dois digitos gravam e o cursor avanca (3F AB)");
    r = mon.Execute("@key enter");
    check(r == "action=close panel=mamute", "M: ENTER sempre sai");
    // S: as teclas do teclado numerico (q w = 5 6)
    mon.Execute("S C200");
    mon.Execute("@key c:q");
    mon.Execute("@key c:w");
    checkOut(mon, "D C200,C200", "C200  56", "S: q w = 5 6");
    r = mon.Execute("@key c:1");
    check(has(r, "TECLAS S:"), "S mostra a legenda do teclado");
    mon.Execute("@key esc");
    // escrita em celula que nao e' RAM: nada e' gravado e o painel avisa
    mon.Execute("PAGE 0,0,0,0");
    mon.Execute("M 0000");
    mon.Execute("@key c:1");
    r = mon.Execute("@key c:2");
    check(has(r, "SOMENTE LEITURA"), "painel M sobre ROM/vazio avisa e nao grava");
    mon.Execute("@key esc");
    mon.Execute("PAGE");

    // ---- ZAP sobre uma imagem de disco ----
    const std::string img = TempPath("mamute_t.dsk");
    {
        std::vector<uint8_t> bytes(2048);
        for (size_t i = 0; i < bytes.size(); ++i) bytes[i] = static_cast<uint8_t>(i & 0xFF);
        std::ofstream f(img, std::ios::binary);
        f.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }
    mon.Execute("@panels off");
    checkOut(mon, "ZAP " + img + ",1", "SETOR: 001  BYTE: 000", "ZAP de linha mostra o setor");
    checkOut(mon, "ZAP " + img + ",1", "000  00 01 02 03 04 05 06 07", "ZAP: o rotulo e' o deslocamento no setor");
    checkOut(mon, "ZAP " + img + ",9", "FORA DA IMAGEM", "ZAP recusa setor fora da imagem");
    checkOut(mon, "ZAPW " + img + ",1,10,DE,AD", "GRAVADOS 2 BYTES NO SETOR 1", "ZAPW grava bytes no setor");
    {
        const std::vector<uint8_t> b = ReadAll(img);
        check(b[512 + 0x10] == 0xDE && b[512 + 0x11] == 0xAD && b[512 + 0x12] == 0x12, "ZAPW: bytes certos no arquivo");
    }
    mon.Execute("@panels on");
    r = mon.Execute("ZAP " + img + ",0");
    check(has(r, "title=ZAP") && has(r, "SETOR: 000"), "ZAP com paineis ligados abre o painel");
    mon.Execute("@key enter");
    mon.Execute("@key c:A");
    r = mon.Execute("@key c:B");
    check(has(r, "EDITANDO (HEXA): AB_"), "ZAP: o primeiro digito substitui o valor pre-preenchido");
    r = mon.Execute("@key enter");
    check(has(r, "ZAP - mamute_t.dsk *"), "ZAP: o titulo ganha * enquanto ha' alteracoes nao salvas");
    check(ReadAll(img)[0] == 0x00, "ZAP: editar nao grava no arquivo sozinho");
    r = mon.Execute("@key save");
    check(has(r, "SETOR 000 GRAVADO") && !has(r, "mamute_t.dsk *") && ReadAll(img)[0] == 0xAB, "ZAP: Ctrl+S grava so' o setor do cursor");
    mon.Execute("@key enter");
    mon.Execute("@key c:7");
    mon.Execute("@key enter");
    r = mon.Execute("@key esc");
    check(has(r, "ALTERACOES NAO SALVAS") && !has(r, "action=close"), "ZAP: ESC com alteracao pendente avisa");
    r = mon.Execute("@key esc");
    check(r == "action=close panel=mamute", "ZAP: ESC de novo descarta e sai");
    std::remove(img.c_str());

    // ---- SCR ----
    mon.Execute("@panels off");
    mon.Execute("F C000,C00F,00");
    mon.Execute("M C000,FF,81,81,81,81,81,81,FF");
    r = mon.Execute("SCR C000,1,1,0,1");
    {
        std::vector<std::string> lines;
        size_t pos = 0;
        while (pos <= r.size()) {
            const size_t e = r.find('\n', pos);
            lines.push_back(r.substr(pos, e == std::string::npos ? std::string::npos : e - pos));
            if (e == std::string::npos) break;
            pos = e + 1;
        }
        check(lines.size() == 1 + 48 + 1, "SCR zoom 1: titulo + 48 linhas de braille + ajuda (" + std::to_string(lines.size()) + ")");
        check(lines.size() > 1 && lines[1].rfind("\xE2\xA1\x8F", 0) == 0, "SCR: o primeiro caractere braille e' U+284F (os pixels do glifo FF 81 81 81)");
    }
    r = mon.Execute("SCR C000,1,1");
    check(has(r, "SCR - C000,01,01,0 (OFFLINE)"), "SCR: titulo com endereco e azulejo");
    mon.Execute("F C000,C00F,00");
    mon.Execute("M C008,FF,FF,FF,FF,FF,FF,FF,FF");
    auto first_row_col = [&](const std::string &scr, int col) {
        const size_t a = scr.find('\n') + 1;
        return scr.substr(a + static_cast<size_t>(col) * 3, 3);
    };
    check(first_row_col(Run(mon, "SCR C000,2,2,0,1"), 4) == "\xE2\xA3\xBF", "SCR modo 0 (horizontal): o segundo caractere do azulejo esta' 8 bytes depois (pixels 8-15 da linha 0)");
    check(first_row_col(Run(mon, "SCR C000,2,2,1,1"), 4) == "\xE2\xA0\x80", "SCR modo 1 (vertical): o mesmo byte cai embaixo do primeiro, nao ao lado");
    checkOut(mon, "SCR 1,0,1", "SINTAXE", "SCR recusa dx 0");
    mon.Execute("@panels on");
    mon.Execute("F C000,C00F,00");
    mon.Execute("M C000,FF,FF,FF,FF,FF,FF,FF,FF");
    r = mon.Execute("SCR C000,1,1");
    check(r.rfind("action=show panel=mamute title=SCR", 0) == 0, "SCR com paineis ligados abre o painel");
    r = mon.Execute("@key right");
    check(has(r, "SCR - C001,01,01,0"), "SCR: seta direita desloca o endereco 1 byte");
    mon.Execute("@key left");
    r = mon.Execute("@key tab");
    check(has(r, "\x01"), "SCR: TAB liga a moldura (cursor invertido)");
    mon.Execute("@key tab");
    mon.Execute("@key c:e");
    r = mon.Execute("@key enter");
    check(has(r, "EDICAO 16x16"), "SCR: ENTER abre a edicao 16x16");
    mon.Execute("@key space"); // inverte o pixel (0,0): FF -> 7F
    checkOut(mon, "D C000,C000", "C000  7F", "SCR: ESPACO inverte o pixel sob o cursor");
    mon.Execute("@key right");
    mon.Execute("@key space");
    checkOut(mon, "D C000,C000", "C000  3F", "SCR: o cursor de pixel anda e inverte o proximo");
    mon.Execute("@key esc");
    checkOut(mon, "D C000,C000", "C000  FF", "SCR: ESC na edicao cancela TODAS as alteracoes");
    mon.Execute("@key enter");
    mon.Execute("@key c:l");
    checkOut(mon, "D C000,C000", "C000  00", "SCR: L apaga os 16x16 pixels");
    mon.Execute("@key c:i");
    checkOut(mon, "D C000,C000", "C000  FF", "SCR: I inverte tudo");
    r = mon.Execute("@key enter");
    check(!has(r, "EDICAO 16x16") && !has(r, "action=close"), "SCR: ENTER sai da edicao e mantem as alteracoes");
    r = mon.Execute("@key esc");
    check(r == "action=close panel=mamute", "SCR: ESC fecha");
    check(mon.Execute("@key up") == "action=close panel=mamute", "@key sem painel aberto responde que fechou");
}

// Fita: TAPE, TSAVE, R, TLOAD, LOAD/SAVE de .cas/.tsx/.wav e a opcao T do A, com o deck PROPRIO do monitor (offline).
void TestTape() {
    mamute::Config cfg;
    mamute::Monitor mon(cfg);
    const std::string tape_path = TempPath("mamute_tape_test.tsx");
    std::remove(tape_path.c_str());

    checkOut(mon, "TAPE", "NENHUMA INSERIDA", "fita: sem fita ao abrir");
    checkOut(mon, "R", "NENHUMA FITA", "fita: R sem fita avisa");
    checkOut(mon, "TSAVE JOGO,C000,C003", "NAO FOI POSSIVEL GRAVAR NA FITA", "fita: TSAVE sem fita recusa");
    checkOut(mon, "TAPE NEW " + tape_path, "0 ARQUIVO(S)", "fita: TAPE NEW cria a fita (.tsx) vazia e gravavel");
    checkOut(mon, "TAPE", "GRAVAVEL", "fita: a fita nova nao e' protegida");

    Run(mon, "M C000,3E,2A,C9,00");
    Run(mon, "M C100,11,22,33");
    checkOut(mon, "TSAVE JOGO,C000,C003,C000", "GRAVADO NA FITA 'JOGO' - C000-C003 - TAMANHO 0004 - EXEC C000", "fita: TSAVE grava o 1o arquivo");
    checkOut(mon, "TSAVE DADOS,C100,C102", "GRAVADO NA FITA 'DADOS'", "fita: TSAVE grava o 2o");
    std::string r = Run(mon, "TAPE LIST");
    check(has(r, "0 BIN") && has(r, "JOGO") && has(r, "C000-C003 EXEC C000") && has(r, "1 BIN") && has(r, "DADOS") && has(r, "C100-C102"), "fita: TAPE LIST mostra os 2 arquivos");
    if (!has(r, "DADOS")) std::printf("    '%s'\n", r.c_str());

    // R carrega em sequencia, no endereco do cabecalho (+ desloc)
    Run(mon, "F C000,C1FF,00");
    checkOut(mon, "TAPE REW", "REBOBINADA", "fita: REW");
    checkOut(mon, "R", "'JOGO'", "fita: R carrega o 1o arquivo");
    checkOut(mon, "D C000,C003", "C000  3E 2A C9 00", "fita: o 1o arquivo foi para C000");
    checkOut(mon, "R 100", "EM C200", "fita: R com desloc soma ao endereco do arquivo");
    checkOut(mon, "D C200,C202", "C200  11 22 33", "fita: o 2o arquivo foi para C100+100");
    checkOut(mon, "R", "FIM DA FITA", "fita: R no fim avisa");

    // TLOAD por nome e por numero, com endereco explicito
    Run(mon, "F C000,C1FF,00");
    checkOut(mon, "TLOAD dados", "EM C100", "fita: TLOAD por nome (sem caixa)");
    checkOut(mon, "TLOAD 0,3,D000", "EM D000", "fita: TLOAD com endereco explicito poe so' o programa");
    checkOut(mon, "D D000,D003", "D000  3E 2A C9 00", "fita: TLOAD 0,3,D000 gravou o programa sem o cabecalho");
    checkOut(mon, "TLOAD ZZZ", "NAO TEM O ARQUIVO", "fita: TLOAD de nome inexistente");

    // protecao, modo de gravacao e ponto
    Run(mon, "TAPE PROT ON");
    checkOut(mon, "TSAVE X,C000,C001", "PROTEGIDA", "fita: protegida recusa TSAVE");
    Run(mon, "TAPE PROT OFF");
    checkOut(mon, "TAPE SEEK DADOS", "PONTO: 1 DADOS", "fita: SEEK por nome");
    Run(mon, "TAPE WRITE OVER");
    checkOut(mon, "TSAVE NOVO,C000,C001", "GRAVADO NA FITA 'NOVO'", "fita: sobrescrever no ponto");
    r = Run(mon, "TAPE LIST");
    check(has(r, "JOGO") && has(r, "NOVO") && !has(r, "DADOS"), "fita: o 2o arquivo foi cortado e substituido");

    // reinsere do disco: o que foi gravado esta' no arquivo
    checkOut(mon, "TAPE EJECT", "EJETADA", "fita: EJECT");
    r = Run(mon, "TAPE " + tape_path);
    check(has(r, "2 ARQUIVO(S)") && has(r, "PROTEGIDA"), "fita: reinserir do disco traz os 2 arquivos, protegida");

    // LOAD/SAVE por extensao
    for (const char *ext : {".cas", ".tsx", ".wav"}) {
        const std::string f = TempPath(std::string("mamute_tape_save") + ext);
        std::remove(f.c_str());
        Run(mon, "M C300,AA,BB,CC,DD");
        checkOut(mon, std::string("SAVE ") + f + ",C300,C303,C300", std::string("FORMATO ") + (ext + 1 == std::string("cas") ? "CAS" : ext + 1 == std::string("tsx") ? "TSX" : "WAV"), std::string("fita: SAVE ") + ext);
        Run(mon, "F C300,C3FF,00");
        checkOut(mon, "LOAD " + f, "EM C300", std::string("fita: LOAD ") + ext + " usa o endereco do cabecalho");
        checkOut(mon, "D C300,C303", "C300  AA BB CC DD", std::string("fita: LOAD ") + ext + " trouxe os bytes");
    }

    // A opcao T do montador: cria a fita ou acrescenta na inserida
    Run(mon, "EDIT NEW");
    Run(mon, "EDIT 10 ORG 8000H");
    Run(mon, "EDIT 20 LD A,1");
    Run(mon, "EDIT 30 RET");
    const std::string asm_cas = TempPath("mamute_tape_asm.cas");
    std::remove(asm_cas.c_str());
    checkOut(mon, "EDIT A OT>" + asm_cas, "GRAVADO " + asm_cas, "fita: A OT>arq.cas cria a fita com o programa montado");
    checkOut(mon, "LOAD " + asm_cas + ",3", "EM 8000", "fita: o .cas do montador volta pelo LOAD");
    checkOut(mon, "D 8000,8002", "8000  3E 01 C9", "fita: o programa montado esta' certo");
    Run(mon, "TAPE PROT OFF");
    checkOut(mon, "EDIT A T>prog", "GRAVADO NA FITA 'PROG'", "fita: A T acrescenta na fita inserida");

    for (const char *ext : {".tsx", ".cas", ".wav"}) std::remove((TempPath("mamute_tape_save") + ext).c_str());
    std::remove(tape_path.c_str());
    std::remove(asm_cas.c_str());
}

void TestOnline() {
#ifdef FWMSX_SOURCE_DIR
    const std::string bios = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
    {
        std::ifstream probe(bios, std::ios::binary);
        if (!probe) {
            std::printf("[SKIP] BIOS nao encontrada em '%s'\n", bios.c_str());
            return;
        }
    }
    machine::MachineConfig mc;
    mc.bios_path = bios;
    std::string error;
    std::unique_ptr<machine::Machine> m = machine::Machine::Create(mc, error);
    check(m != nullptr, "online: maquina criada " + error);
    if (!m) return;
    for (int i = 0; i < 250; ++i) m->RunFrame(); // ate' o BASIC subir

    mamute::Config cfg;
    mamute::Monitor mon(cfg);
    mamute::MachineMemory mem(*m);
    mamute::MachineExec exec(*m);
    mon.AttachMachine(&mem, &exec);
    check(mon.has_machine(), "online: maquina ligada ao monitor");
    checkOut(mon, "ONLINE", "MODO ONLINE", "ONLINE liga o modo");
    check(mon.Prompt() == "MON(on)> ", "online: prompt MON(on)>");
    checkOut(mon, "PAGE ?", "PAGINA 0 (0000-3FFF): SLOT 0 ROM", "online: a BIOS esta' na pagina 0 (ROM)");
    checkOut(mon, "PAGE ?", "PAGINA 3 (C000-FFFF): SLOT 3 RAM", "online: a pagina 3 e' RAM");
    checkOut(mon, "PAGE 1,1,1,1", "USA O MAPEAMENTO DA MAQUINA", "online: PAGE nao troca o mapeamento");
    checkOut(mon, "D 0000,0003", "0000  F3", "online: le a BIOS real (DI em 0000)");
    checkOut(mon, "M 0000,11", "IGNORADOS", "online: a BIOS e' ROM, escrita ignorada");

    // o que o monitor escreve aparece na maquina na hora
    checkOut(mon, "M C000,3E,2A,18,FE", "GRAVADOS 4 BYTES EM C000", "online: M escreve na RAM da maquina");
    check(m->ReadMemory(0xC000) == 0x3E && m->ReadMemory(0xC001) == 0x2A, "online: a maquina enxerga os bytes (C000 = 3E 2A)");
    checkOut(mon, "L C000,C001", "LD A,2A", "online: L disassembla a memoria da maquina");

    // PUSH leva um bloco do offline para a maquina; PULL traz de volta
    Run(mon, "OFFLINE");
    Run(mon, "M C200,11,22,33");
    checkOut(mon, "PUSH C200,C202", "ENVIADOS 3 BYTES", "PUSH offline -> maquina");
    check(m->ReadMemory(0xC200) == 0x11 && m->ReadMemory(0xC202) == 0x33, "PUSH: a maquina recebeu os bytes");
    Run(mon, "F C200,C202,00");
    checkOut(mon, "PULL C200,C202", "TRAZIDOS 3 BYTES", "PULL maquina -> offline");
    checkOut(mon, "D C200,C203", "C200  11 22 33", "PULL: o offline recebeu os bytes");

    // G online: a CPU da maquina passa a rodar o programa (LD A,2A / JR $)
    Run(mon, "ONLINE");
    checkOut(mon, "G C000", "EXECUTANDO NA MAQUINA", "online: G solta a CPU da maquina");
    for (int i = 0; i < 4; ++i) m->RunFrame();
    check(m->cpu().state().af.b.hi == 0x2A, "online: o programa escrito no monitor rodou na maquina (A = 2A)");
    checkOut(mon, "X A", "A=2A", "online: X le os registradores da maquina");
    // A O no modo ONLINE: o programa montado pelo fwasm vai para a RAM da maquina e roda la'
    mon.Execute("EDIT NEW");
    mon.Execute("EDIT 10 ORG 0C200H");
    mon.Execute("EDIT 20 LD A,5A");
    mon.Execute("EDIT 30 JR $");
    checkOut(mon, "EDIT A O", "GRAVADOS 4 BYTES NA MEMORIA (ONLINE) EM C200-C203", "online: A O escreve o objeto na RAM da maquina");
    check(m->ReadMemory(0xC200) == 0x3E && m->ReadMemory(0xC201) == 0x5A, "online: a maquina enxerga o codigo montado");
    checkOut(mon, "G C200", "EXECUTANDO NA MAQUINA", "online: G solta a CPU no programa montado");
    for (int i = 0; i < 4; ++i) m->RunFrame();
    check(m->cpu().state().af.b.hi == 0x5A, "online: o programa montado com A O rodou na maquina (A = 5A)");
    // SUPER-X no modo ONLINE: os alvos (#slot, #V) e a CPU da maquina
    checkOut(mon, "XRG", "AF=", "online: XRG mostra os registradores da maquina");
    checkOut(mon, "XFL C300#3,C30F,77", "PREENCHIDOS 16 BYTES", "online: XFL grava na RAM do slot 3 da maquina");
    check(m->ReadMemory(0xC300) == 0x77 && m->ReadMemory(0xC30F) == 0x77, "online: a maquina enxerga o XFL");
    checkOut(mon, "XD C300#3,C307", "C300 77 77 77 77 77 77 77 77", "online: XD le o slot 3 da maquina");
    checkOut(mon, "XFL 0000#0,0003,11", "4 IGNORADOS", "online: a BIOS e' ROM, o XFL no slot 0 e' ignorado");
    checkOut(mon, "XD 0000#0,0003", "0000 F3", "online: XD le a BIOS pelo slot 0");
    checkOut(mon, "XFL 0000#V,0003,5A", "PREENCHIDOS 4 BYTES", "online: XFL na VRAM da maquina");
    check(m->vdp_state().vram[0] == 0x5A && m->vdp_state().vram[3] == 0x5A, "online: a VRAM da maquina recebeu o XFL");
    checkOut(mon, "XTR C000", "C000  3E 2A", "online: XTR executa um passo na CPU da maquina");
    // Fita no modo ONLINE: o deck e' o da MAQUINA (a mesma fita do CLOAD/CSAVE)
    {
        const std::string tp = TempPath("mamute_tape_online.cas");
        std::remove(tp.c_str());
        checkOut(mon, "TAPE NEW " + tp, "[FITA DA MAQUINA]", "online: TAPE NEW cria a fita na MAQUINA");
        check(m->tape().inserted() && m->tape().path() == tp, "online: a maquina emulada tem a fita inserida");
        Run(mon, "M C400,01,02,03,04");
        checkOut(mon, "TSAVE ONL,C400,C403,C400", "GRAVADO NA FITA 'ONL'", "online: TSAVE grava na fita da maquina");
        check(m->tape().files().size() == 1 && m->tape().files()[0].name == "ONL", "online: a maquina enxerga o arquivo gravado pelo Mamute");
        Run(mon, "F C400,C4FF,00");
        checkOut(mon, "TAPE REW", "REBOBINADA", "online: REW rebobina a fita da maquina");
        checkOut(mon, "R", "'ONL'", "online: R le da fita da maquina");
        check(m->ReadMemory(0xC400) == 1 && m->ReadMemory(0xC403) == 4, "online: o R pos os bytes na memoria da maquina");
        checkOut(mon, "TAPE MODE NORMAL", "NORMAL", "online: TAPE MODE NORMAL");
        check(m->tape().mode() == tape::TapeMode::Normal, "online: a maquina passou para o carregamento normal");
        checkOut(mon, "TAPE MODE FAST", "RAPIDO", "online: TAPE MODE FAST");
        Run(mon, "TAPE EJECT");
        check(!m->tape().inserted(), "online: EJECT ejeta da maquina");
        std::remove(tp.c_str());
    }
    checkOut(mon, "OFFLINE", "MODO OFFLINE", "OFFLINE volta para a memoria privada");
#endif
}

// ---------------------------------------------------------------------------------------------------------------------
// Fase 4: os comandos do SUPER-X (prefixo X)
// ---------------------------------------------------------------------------------------------------------------------
void TestSuperX() {
    mamute::Config cfg;
    mamute::Monitor mon(cfg);

    // ---- CL: a calculadora ----
    checkOut(mon, "CL 4000", "HEX  : 4000H", "CL: hexa por padrao");
    checkOut(mon, "CL 4000", "DEC+ : 16384", "CL: decimal");
    checkOut(mon, "CL (100H+2ADH)*3-1", "HEX  : 0B06H", "CL: o exemplo do manual (parenteses, sufixo H)");
    checkOut(mon, "CL 10D", "DEC+ : 10", "CL: 10D e' decimal 10");
    checkOut(mon, "CL 10DH", "DEC+ : 269", "CL: 10DH e' hexa 10D");
    checkOut(mon, "CL 101B", "DEC+ : 5", "CL: sufixo B (binario)");
    checkOut(mon, "CL 17O", "DEC+ : 15", "CL: sufixo O (octal)");
    checkOut(mon, "CL -1", "DEC+-: -1", "CL: negativo, com volta de 16 bits");
    checkOut(mon, "CL !0", "DEC+ : 65535", "CL: NOT");
    checkOut(mon, "CL 'A'", "DEC+ : 65", "CL: literal ASCII");
    checkOut(mon, "CL 1/0", "DIVISAO POR ZERO", "CL: divisao por zero");
    checkOut(mon, "CL 12G", "NUMERO INVALIDO", "CL: numero invalido");
    checkOut(mon, "CL 6|1&3^2", "DEC+ : 7", "CL: precedencia (& antes de ^ antes de |)");

    // ---- @: as variaveis de debugger ----
    checkOut(mon, "@0=8000#3-1", "@0 = 8000#3-1", "@0 guarda endereco E alvo");
    checkOut(mon, "@", "@B = ----", "@ mostra as 7 variaveis");
    checkOut(mon, "CL @0+1", "HEX  : 8001H", "CL usa so' o numero da variavel");
    checkOut(mon, "@1=1234", "@1 = 1234", "@1 sem alvo");
    checkOut(mon, "@Z=1", "SINTAXE", "@ com nome invalido");

    // ---- alvos: sub-slot, slot explicito, VRAM ----
    checkOut(mon, "XFL 1000#3-1,10FF,AA", "PREENCHIDOS 256 BYTES", "XFL em sub-slot (RAM expandida)");
    checkOut(mon, "XD 1000#3-1,100F", "1000 AA AA AA AA AA AA AA AA : 50 : ........", "XD de dois enderecos com checksum e alvo");
    checkOut(mon, "XD 1000,100F", "1000 00 00 00 00 00 00 00 00 : 00 :", "o PAGE continua independente do sub-slot");
    checkOut(mon, "XFL 0000#0,0003,11", "4 IGNORADOS", "slot 0 vazio nao grava");
    checkOut(mon, "XFL 0000#V,0003,5A", "PREENCHIDOS 4 BYTES", "XFL na VRAM");
    checkOut(mon, "XD 0000#V,0003", "00000 5A 5A 5A 5A : 68 : ZZZZ", "XD na VRAM: 5 digitos");
    checkOut(mon, "XFL 4000#9,4001,00", "SINTAXE", "slot invalido");
    checkOut(mon, "XD C000,", "C0F8", "XD sem fim: 256 bytes");

    // ---- XD / XA / XI com arquivo e impressora ----
    Run(mon, "M C000,3E,05,CD,A2,00,C9");
    const std::string t = TempPath("mamute_sx_");
    checkOut(mon, "XD C000,C00F," + t + "d.bin", "BYTES CRUS", "XD <ini>,<fim>,<arquivo> salva os bytes");
    check(ReadAll(t + "d.bin").size() == 16 && ReadAll(t + "d.bin")[0] == 0x3E, "XD: o arquivo tem os 16 bytes");
    checkOut(mon, "XD C000," + t + "d2.bin", "TAMANHO 0100", "XD <ini>,<arquivo>: o segundo campo que nao e' endereco e' o arquivo (256 bytes)");
    checkOut(mon, "XA C000,C03F", "C000: >", "XA: 64 bytes por linha em ASCII");
    checkOut(mon, "XI C000,C005", "C000  3E 05        LD A,05", "XI: disassembly");
    checkOut(mon, "XI C000,C005", "C002  CD A2 00     CALL", "XI: instrucao de 3 bytes");
    checkOut(mon, "XI C000,C005," + t + "i.txt", "GRAVADO", "XI <ini>,<fim>,<arquivo> grava a listagem");
    checkOut(mon, "?XD C000,C03F," + t + "d.pdf", "(PDF)", "?XD manda para a impressora (PDF)");
    check(mamute::IsPdfName(t + "d.pdf") && ReadAll(t + "d.pdf").size() > 100, "?XD: o PDF foi gravado");
    checkOut(mon, "?XD C000", "SO' FUNCIONA COM DOIS ENDERECOS", "?XD de um endereco so' nao imprime");
    checkOut(mon, "?XM C000", "SO' VALE PARA XD, XA E XI", "? so' vale para os que listam");
    checkOut(mon, "XD C000,C00F,", "SINTAXE", "XD com virgula sobrando depois de um fim explicito");
    checkOut(mon, "XD C010,C000", "SINTAXE", "XD com fim menor que o inicio");

    // ---- XCS / XTS ----
    Run(mon, "M D400,01,02,03,FF");
    checkOut(mon, "XD D401,D404", "D401 02 03 FF 00 : 04 :", "XD: checksum NORMAL");
    checkOut(mon, "XCS", "+ADDR", "XCS alterna para +ADDR");
    checkOut(mon, "XD D401,D404", "D401 02 03 FF 00 : 05 :", "XD: checksum +ADDR soma o byte baixo do endereco");
    checkOut(mon, "XCS", "NORMAL", "XCS volta para NORMAL");
    checkOut(mon, "XTS D400,D403", "HEX  : 0105H", "XTS: soma de 16 bits");
    checkOut(mon, "XTS D400,D403", "OCT  : 000405", "XTS: octal");

    // ---- XBT / XCM ----
    Run(mon, "M C900,01,02,03,04");
    checkOut(mon, "XBT C900,C903,CA00", "COPIADOS 4 BYTES", "XBT copia");
    checkOut(mon, "XCM C900,C903,CA00", "0 DIFERENCAS EM 4 BYTES", "XCM: blocos iguais");
    Run(mon, "M CA02,FF");
    checkOut(mon, "XCM C900,C903,CA00", "C902: 03 <> CA02: FF", "XCM lista a diferenca");
    checkOut(mon, "XCM C900,C903,CA00,S", "C900: 01 == CA00: 01", "XCM,S lista os iguais");
    checkOut(mon, "XCM C900,C903,CA00,S", "3 BYTES IGUAIS", "XCM,S conta os iguais");
    checkOut(mon, "XBT C900,C903,FFFE", "PASSA DO FIM", "XBT nao da' a volta");
    checkOut(mon, "XBT C900,C903,0010#V", "COPIADOS 4 BYTES", "XBT para a VRAM (alvos diferentes)");
    checkOut(mon, "XD 0010#V,0013", "00010 01 02 03 04 : 0A :", "a VRAM recebeu o bloco");
    checkOut(mon, "XBT C900,C903,2000#3-2", "COPIADOS 4 BYTES", "XBT para um sub-slot");
    checkOut(mon, "XCM C900,C903,2000#3-2", "0 DIFERENCAS", "XCM entre o PAGE e um sub-slot");
    checkOut(mon, "XBT C900,C903", "SINTAXE", "XBT com argumento faltando");

    // ---- XRT: relocar ----
    Run(mon, "M D000,C3,05,D0,00,00,CD,A2,00,18,FB,C9");
    checkOut(mon, "XRT D000,D00A,E000", "1 ENDERECOS ABSOLUTOS E 0 SALTOS RELATIVOS", "XRT ajusta so' o que aponta para dentro");
    checkOut(mon, "DM E000", "E000  C3 05 E0 00 00 CD A2 00", "XRT: JP ajustado, CALL da BIOS intocado");
    checkOut(mon, "DM E008", "E008  18 FB C9", "XRT: JR para dentro do bloco intocado");
    Run(mon, "M D100,18,1E,00,C9");
    checkOut(mon, "XRT D100,D103,D110", "1 SALTOS RELATIVOS", "XRT recalcula o JR que sai do bloco");
    checkOut(mon, "DM D110", "D110  18 0E 00 C9", "XRT: deslocamento do JR recalculado");
    checkOut(mon, "XRT D100,D103,D400", "RELOCACAO INVALIDA", "XRT: JR que nao alcanca depois do movimento");
    checkOut(mon, "DM D400", "D400  01 02 03 FF", "XRT invalido nao grava nada");

    // ---- XFD: procurar uma instrucao ----
    Run(mon, "M D300,CD,12,80,00,00,CD,12,80,00");
    checkOut(mon, "XFD D300,D308,CALL 8012H", "D300", "XFD acha a primeira");
    checkOut(mon, "XFD D300,D308,CALL 8012H", "D305", "XFD acha a segunda");
    checkOut(mon, "XFD D300,D308,CALL 8012H", "2 OCORRENCIAS", "XFD conta");
    checkOut(mon, "XFD D300,D308,CALL 9000H", "0 OCORRENCIAS", "XFD sem ocorrencias");
    checkOut(mon, "XFD D300,D308", "SINTAXE", "XFD sem instrucao");

    // ---- XM: a sessao ----
    checkOut(mon, "XM C100", "SESSAO XM EM C100", "XM abre a sessao");
    check(mon.Prompt() == "XM C100> " && mon.editing(), "XM: o prompt mostra o endereco");
    checkOut(mon, "LD A,5", "C100  3E 05", "XM monta e grava uma instrucao");
    check(mon.Prompt() == "XM C102> ", "XM: o endereco avanca sozinho");
    checkOut(mon, "LD HL,1234H", "C102  21 34 12", "XM: LD HL,nn");
    checkOut(mon, ".CD,4DH,00", "C105  CD 4D 00", "XM: dados de 1 byte");
    checkOut(mon, ":1234", "C108  34 12", "XM: dados de 2 bytes (little-endian)");
    checkOut(mon, "\"OLA", "C10A  4F 4C 41", "XM: texto cru");
    checkOut(mon, "DAA", "C10D  27", "XM: DAA sozinho e' a instrucao, nao o endereco 0DAA");
    checkOut(mon, "C200", "", "XM: um endereco sozinho pula para ele");
    check(mon.Prompt() == "XM C200> ", "XM: o prompt acompanha o pulo");
    checkOut(mon, "NOP", "C200  00", "XM: grava no novo endereco");
    checkOut(mon, "C210 .1,2,3", "C210  01 02 03", "XM: endereco seguido de dados");
    checkOut(mon, "I 2", "C213  00", "XM: I lista as proximas instrucoes (do endereco atual)");
    checkOut(mon, "I 2", "C214  00", "XM: I mostra 2 linhas");
    checkOut(mon, "FOO BAR", "ERRO", "XM: instrucao invalida");
    Run(mon, "C100 ");
    checkOut(mon, "I 1", "C100  3E 05", "XM: I a partir do endereco");
    checkOut(mon, "QUIT", "VOLTA AO MONITOR", "XM: QUIT volta ao monitor");
    check(mon.Prompt() == "MON(off)> " && !mon.editing(), "XM: depois do QUIT o prompt e' do monitor");
    checkOut(mon, "XM", "SESSAO XM EM", "XM sem endereco reabre onde ficou");
    {
        bool quit = false;
        mon.Execute("BA", &quit);
        check(quit && mon.Prompt() == "MON(off)> ", "XM: BA sai de tudo");
    }
    checkOut(mon, "XM C100#3-1", "SESSAO XM EM C100#3-1", "XM num sub-slot");
    checkOut(mon, "LD A,6", "C100  3E 06", "XM no sub-slot");
    Run(mon, "QUIT");
    checkOut(mon, "XD C100#3-1,C101", "C100 3E 06", "o sub-slot recebeu o byte");
    checkOut(mon, "XD C100,C101", "C100 3E 05", "e o PAGE nao");

    // ---- XH ----
    Run(mon, "M C300,FF,81,81,81,81,81,81,FF");
    checkOut(mon, "XH C300", "0123456789ABCDEF", "XH mostra a grade");
    checkOut(mon, "XH C300", "00000000--------  C300 : FF:00 0", "XH: pixels acesos como 0 e a coluna do byte");
    checkOut(mon, "XH C300", "C301 : 81:00 1", "XH: o byte da linha 1");

    // ---- XRG / XGO / XTR ----
    Run(mon, "M C800,3E,05,06,03,3C,3C,C9");
    checkOut(mon, "XRG SP,F000", "SP=F000", "XRG: SP");
    checkOut(mon, "XRG BP,C804", "BP=C804", "XRG: breakpoint");
    checkOut(mon, "XGO C800", "PARADO NO BREAKPOINT (C804)", "XGO para no BP");
    checkOut(mon, "XRG", "AF=05", "o A depois do LD A,5");
    checkOut(mon, "XRG", "BC=03", "o B depois do LD B,3");
    checkOut(mon, "XGO", "PROGRAMA TERMINOU (RET NO TOPO DA PILHA)", "XGO sem BP1: roda livre ate' o RET");
    checkOut(mon, "XRG", "AF=07", "o A depois dos dois INC A");
    checkOut(mon, "XRG A,99", "AF=99", "XRG: edita um registrador de 1 byte");
    checkOut(mon, "XRG AF',1234", "AF'=1234", "XRG: o par alternado");
    checkOut(mon, "XRG IXH,AB", "IX=AB", "XRG: meio registrador");
    checkOut(mon, "XRG *", "AF=0000 BC=0000", "XRG *: limpa");
    checkOut(mon, "XRG *", "SP=F002", "XRG *: a pilha fica (o RET do XGO deixou SP=F002)");
    checkOut(mon, "XRG +", "SP=0000", "XRG +: so' a pilha");
    checkOut(mon, "XRG BP,-", "BP=----", "XRG BP,-: limpa o breakpoint");
    checkOut(mon, "XRG ZZ,1", "SINTAXE", "XRG: registrador desconhecido");
    Run(mon, "XRG SP,F000");
    checkOut(mon, "XTR C800", "C800  3E 05", "XTR: o primeiro passo");
    checkOut(mon, "XTR", "C802  06 03", "XTR sem argumento continua do PC");
    checkOut(mon, "XTR ,2", "C805  3C", "XTR ,n executa n passos");
    Run(mon, "M C900,76");
    Run(mon, "XRG SP,F000");
    checkOut(mon, "XGO C900", "CPU HALTADA (HALT) EM", "XGO para no HALT");
    Run(mon, "M CA00,18,FE");
    checkOut(mon, "XGO CA00#3", "LIMITE DE 2000000", "XGO: o teto de instrucoes de um laco infinito");

    // ---- XPP / XPI / XPO e as portas do Z80 ----
    checkOut(mon, "XPI 60", "PORTA 60 = FF", "XPI cria a porta com Saida FF");
    checkOut(mon, "XPO 61,5A", "PORTA 61 <- 5A", "XPO");
    checkOut(mon, "XPP", "61*    5A      FF", "XPP: Entrada, Saida e a marca de alterada");
    Run(mon, "M CB00,DB,60,D3,62,76");
    Run(mon, "XRG SP,F000");
    Run(mon, "XGO CB00");
    checkOut(mon, "XRG", "AF=FF", "IN A,(60) leu a Saida (FF)");
    checkOut(mon, "XPP", "62*    FF      FF", "OUT (62),A criou a porta e marcou como alterada");
    checkOut(mon, "XPI 5G", "SINTAXE", "XPI invalido");

    // ---- XCO ----
    checkOut(mon, "XCO 15,4,7", "FRENTE: 15 BRANCO   FUNDO: 4 AZUL ESCURO   BORDA: 7 CIANO", "XCO: as tres cores");
    checkOut(mon, "XCO ,,2", "FRENTE: 15 BRANCO   FUNDO: 4 AZUL ESCURO   BORDA: 2 VERDE", "XCO: vazio mantem o valor");
    checkOut(mon, "XCO 16", "SINTAXE", "XCO: so' ate' 15");

    // ---- XSD ----
    checkOut(mon, "XSD " + t + "sd.asm,C000,C005", "GRAVADO", "XSD: listagem assembly");
    {
        const std::vector<uint8_t> b = ReadAll(t + "sd.asm");
        const std::string s(b.begin(), b.end());
        check(has(s, "ORG C000H") && has(s, "LD A,05") && has(s, "CALL"), "XSD: ORG e as instrucoes decodificadas");
    }
    Run(mon, "XSD " + t + "sdb.asm,C000,C005,B");
    {
        const std::vector<uint8_t> b = ReadAll(t + "sdb.asm");
        check(has(std::string(b.begin(), b.end()), "DEFB 3EH,05H,CDH,A2H,00H,C9H"), "XSD,B: DEFB");
    }
    Run(mon, "XSD " + t + "sdd.bas,C000,C005,D");
    {
        const std::vector<uint8_t> b = ReadAll(t + "sdd.bas");
        const std::string s(b.begin(), b.end());
        check(has(s, "10000 DATA &H3E,&H05,&HCD,&HA2,&H00,&HC9") && has(s, "10010 FOR I=&HC000 TO &HC005:READ A:POKE I,A:NEXT I"), "XSD,D: DATA e o laco de carga");
    }
    Run(mon, "XSD " + t + "sdx.bas,C000,C005,X");
    {
        const std::vector<uint8_t> b = ReadAll(t + "sdx.bas");
        check(has(std::string(b.begin(), b.end()), "10000 '#&H3E,&H05"), "XSD,X: dados do X-BASIC");
    }
    checkOut(mon, "XSD " + t + "sdv.bas,0000#V,0003,D", "NAO SERVE PARA VRAM", "XSD,D recusa a VRAM");

    // ---- XSV / XLD / XS# / XL# ----
    checkOut(mon, "XSV " + t + "sv.bin,C000,C005", "EXEC C000", "XSV: cabecalho do BSAVE");
    {
        const std::vector<uint8_t> b = ReadAll(t + "sv.bin");
        check(b.size() == 13 && b[0] == 0xFE && b[1] == 0x00 && b[2] == 0xC0 && b[3] == 0x05 && b[4] == 0xC0 && b[5] == 0x00 && b[6] == 0xC0 && b[7] == 0x3E, "XSV: FE ini fim exec + os bytes");
    }
    checkOut(mon, "XLD " + t + "sv.bin,E100", "EM E100", "XLD com desloc carrega em outro endereco");
    checkOut(mon, "DM E100", "E100  3E 05 CD A2 00 C9", "XLD: os bytes chegaram");
    checkOut(mon, "@B", "@B = E100", "XLD preenche @B");
    checkOut(mon, "@E", "@E = E105", "XLD preenche @E");
    checkOut(mon, "@S", "@S = E100", "XLD preenche @S com o EXEC relocado");
    checkOut(mon, "XSV " + t + "sv2.bin,C000,C005,C000,1000", "CABECALHO DESLOCADO 1000", "XSV: desloc so' no cabecalho");
    {
        const std::vector<uint8_t> b = ReadAll(t + "sv2.bin");
        check(b.size() == 13 && b[1] == 0x00 && b[2] == 0xD0 && b[4] == 0xD0 && b[6] == 0xD0, "XSV: enderecos do cabecalho deslocados");
    }
    checkOut(mon, "XSV " + t + "sv3.bin,0000#V,0003", "NUNCA SALVA DA VRAM", "XSV nao salva VRAM");
    {
        std::ofstream o(t + "plain.bin", std::ios::binary);
        o << "nao e' bsave";
    }
    checkOut(mon, "XLD " + t + "plain.bin", "ARQUIVO INVALIDO", "XLD recusa o que nao comeca com FE");
    checkOut(mon, "XS# " + t + "raw.bin,C000,C005", "TAMANHO 0006", "XS#");
    check(ReadAll(t + "raw.bin").size() == 6, "XS#: so' os bytes, sem cabecalho");
    checkOut(mon, "XL# " + t + "raw.bin,E200", "EM E200", "XL#");
    checkOut(mon, "DM E200", "E200  3E 05 CD A2 00 C9", "XL#: os bytes chegaram");
    checkOut(mon, "XL# " + t + "raw.bin,0100#V", "EM 00100", "XL# na VRAM");
    checkOut(mon, "XL# " + t + "raw.bin,FFFE", "NAO CABE", "XL# nao da' a volta");

    // ---- disco: XDK XFS XCI XTP XL% XS% ----
    checkOut(mon, "XFS", "NENHUM DISCO CORRENTE", "XFS sem disco corrente");
    {
        std::ofstream o(t + "hello.txt", std::ios::binary);
        o << "linha1\r\nlinha2\r\n\tcom tab\r\n";
        o.put('\x1A');
        o << "depois do fim";
    }
    {
        msxdisk::DiskImage img = msxdisk::DiskImage::CreateBlank();
        msxdisk::DiskError de = msxdisk::DiskError::None;
        check(img.AddFile(t + "hello.txt", "HELLO.TXT", msxdisk::kAttrArchive, &de), "disco de teste: AddFile");
        check(img.Save(t + "disk.dsk", &de), "disco de teste: Save");
    }
    checkOut(mon, "XDK " + t + "naoexiste.dsk", "NAO E' UMA IMAGEM", "XDK recusa o que nao e' disco");
    checkOut(mon, "XDK " + t + "disk.dsk", "DISCO: " + std::string("mamute_sx_disk.dsk"), "XDK escolhe o disco corrente");
    checkOut(mon, "XDK", "DISCO: mamute_sx_disk.dsk", "XDK sem argumento mostra o corrente");
    checkOut(mon, "XFS", "HELLO.TXT", "XFS lista o diretorio");
    checkOut(mon, "XCI", "CLUSTERS LIVRES", "XCI");
    checkOut(mon, "XTP HELLO.TXT", "linha2", "XTP mostra o texto do arquivo do disco");
    checkOut(mon, "XTP HELLO.TXT", "        com tab", "XTP expande o TAB");
    {
        const std::string out = Run(mon, "XTP HELLO.TXT");
        check(!has(out, "depois do fim"), "XTP para no 1A (fim de texto)");
    }
    checkOut(mon, "XTP NOPE.TXT", "NAO ENCONTRADO", "XTP de arquivo que nao existe");
    Run(mon, "M D600,DE,AD,BE,EF");
    checkOut(mon, "XS% 20,D600", "GRAVADOS 1 SETORES (20-20)", "XS% grava um setor");
    checkOut(mon, "XL% 20,D800", "LIDOS 1 SETORES (20-20)", "XL% le um setor");
    checkOut(mon, "DM D800", "D800  DE AD BE EF", "XL%: os bytes do setor chegaram");
    checkOut(mon, "XS% 21,22,D600", "GRAVADOS 2 SETORES", "XS% de dois setores");
    checkOut(mon, "XL% 5000,D800", "ALEM DO FIM DO DISCO", "XL% alem do disco");
    checkOut(mon, "XL% 0,FF00", "NAO CABEM", "XL% que nao cabe na memoria");
    checkOut(mon, "ZAP " + t + "disk.dsk,20", "000  DE AD BE EF", "o ZAP enxerga o que o XS% gravou");

    // ---- notas ----
    checkOut(mon, "XIC 1234", "NOTA NAO ENCONTRADA", "XIC sem notas");
    checkOut(mon, "XIM 1234,1,4,Teste de porta", "NOTA ADICIONADA EM 1234 (1 NA MEMORIA)", "XIM");
    checkOut(mon, "XIM 1234,2,1,Outra no mesmo endereco", "(2 NA MEMORIA)", "XIM: duas notas no mesmo endereco");
    checkOut(mon, "XIC 1234", "1234 [MAIN/PORT] Teste de porta", "XIC mostra a primeira");
    checkOut(mon, "XIC 1234", "1234 [SUB/BIOS] Outra no mesmo endereco", "XIC mostra todas");
    checkOut(mon, "XIM 1234,9,1,x", "SINTAXE", "XIM: slot fora de 0-4");
    checkOut(mon, "XIM 1234,1,8,x", "SINTAXE", "XIM: tipo fora de 0-7");
    checkOut(mon, "XIS " + t + "n.notas", "GRAVADO", "XIS");
    checkOut(mon, "XIL " + t + "n.notas", "(2 NOTAS)", "XIL recarrega o que o XIS gravou");
#ifdef FWMSX_SOURCE_DIR
    {
        const std::string notes = std::string(FWMSX_SOURCE_DIR) + "/dist/SUPER-X-PT.notas";
        std::ifstream probe(notes);
        if (probe) {
            checkOut(mon, "XIL " + notes, "(471 NOTAS)", "XIL: o arquivo de notas traduzido do SUPER-X (471 notas)");
            checkOut(mon, "XIC 0010", "Extrai um caractere", "XIC: nota da BIOS (RST 10H)");
            checkOut(mon, "XIR extrai", "0010 [GERAL/BIOS]", "XIR <texto> filtra as notas no console");
            mamute::SetConfigDir(std::string(FWMSX_SOURCE_DIR) + "/dist");
            checkOut(mon, "XIL", "(471 NOTAS)", "XIL sem argumento le o arquivo ao lado do executavel");
            mamute::SetConfigDir("");
        } else {
            std::printf("[SKIP] notas do SUPER-X nao encontradas em '%s'\n", notes.c_str());
        }
    }
#endif

    // ---- os paineis (sem terminal: as teclas por nome) ----
    Run(mon, "@panels on");
    {
        std::string r = Run(mon, "XD C400");
        check(has(r, "action=show panel=mamute title=XD") && has(r, "XD - MEMORIA"), "painel XD abre");
        Run(mon, "@key c:1");
        r = Run(mon, "@key c:2");
        check(has(r, "C400  12 "), "painel XD: dois digitos gravam o byte e o cursor avanca");
        r = Run(mon, "@key c:\"");
        check(has(r, "DIGITANDO ASCII"), "painel XD: aspas ligam a digitacao ASCII");
        Run(mon, "@key c:H");
        Run(mon, "@key c:i");
        Run(mon, "@key esc");
        r = Run(mon, "@key c:@");
        Run(mon, "@key esc");
        r = Run(mon, "@key esc");
        check(has(r, "action=close"), "painel XD: ESC fecha");
    }
    Run(mon, "@panels off");
    checkOut(mon, "DM C400", "C400  12 48 69", "painel XD: 12 gravado em hexa, 'H' e 'i' digitados em ASCII");
    Run(mon, "@panels on");
    {
        Run(mon, "XA C500");
        Run(mon, "@key c:O");
        Run(mon, "@key c:K");
        Run(mon, "@key space");
        Run(mon, "@key c:!");
        const std::string r = Run(mon, "@key esc");
        check(has(r, "action=close"), "painel XA: ESC fecha");
    }
    Run(mon, "@panels off");
    checkOut(mon, "DM C500", "C500  4F 4B 20 21", "painel XA: as teclas viram bytes crus");
    Run(mon, "@panels on");
    {
        Run(mon, "XH C600");
        Run(mon, "@key space");   // pixel (0,0): bit 7 do byte 0
        Run(mon, "@key right");
        Run(mon, "@key space");   // pixel (1,0): bit 6
        Run(mon, "@key down");
        Run(mon, "@key c:i");     // inverte o caractere sob o cursor
        Run(mon, "@key esc");
    }
    Run(mon, "@panels off");
    checkOut(mon, "DM C600", "C600  3F FF FF FF FF FF FF FF", "painel XH: o pixel 0 e 1 acesos e depois invertidos (3F), o resto invertido (FF)");
    Run(mon, "@panels on");
    {
        Run(mon, "XH C600");
        Run(mon, "@key c:b");     // apaga os 4 caracteres
        Run(mon, "@key esc");
    }
    Run(mon, "@panels off");
    checkOut(mon, "DM C600", "C600  00 00 00 00 00 00 00 00", "painel XH: B apaga os 4 caracteres");
    Run(mon, "@panels on");
    {
        std::string r = Run(mon, "XI C000");
        check(has(r, "title=XI") && has(r, "C000  3E 05"), "painel XI abre com o disassembly");
        r = Run(mon, "@key down");
        check(has(r, "C001  05"), "painel XI: baixo anda 1 byte (acha o alinhamento)");
        r = Run(mon, "@key esc");
        check(has(r, "action=close"), "painel XI: ESC fecha");
    }
    {
        std::string r = Run(mon, "XPP");
        check(has(r, "title=XPP") && has(r, "62*"), "painel XPP abre com as portas");
        Run(mon, "@key c:a");
        Run(mon, "@key c:7");
        Run(mon, "@key c:0");
        r = Run(mon, "@key enter");
        check(has(r, "70 "), "painel XPP: A adiciona uma porta");
        Run(mon, "@key c:s");
        Run(mon, "@key c:5");
        Run(mon, "@key c:A");
        Run(mon, "@key enter");
        r = Run(mon, "@key esc");
        check(has(r, "action=close"), "painel XPP: ESC fecha");
    }
    Run(mon, "@panels off");
    {
        const std::string p = Run(mon, "XPP");
        check(has(p, "60     00      5A"), "painel XPP: S definiu a Saida");
    }
    Run(mon, "M CC00,DB,60,76");
    Run(mon, "XRG SP,F000");
    Run(mon, "XGO CC00");
    checkOut(mon, "XRG", "AF=5A", "IN A,(60) leu a Saida que o painel definiu");
    Run(mon, "@panels on");
    {
        Run(mon, "XIL " + t + "n.notas");
        std::string r = Run(mon, "XIR 1234");
        check(has(r, "title=XIR") && has(r, "ENDERECO: 1234"), "painel XIR abre na nota do endereco");
        r = Run(mon, "@key c:/");
        Run(mon, "@key c:o");
        Run(mon, "@key c:u");
        Run(mon, "@key c:t");
        Run(mon, "@key c:r");
        Run(mon, "@key c:a");
        r = Run(mon, "@key enter");
        check(has(r, "Outra no mesmo endereco") || has(r, "NAO ENCONTRADO"), "painel XIR: a busca roda");
        r = Run(mon, "@key esc");
        r = Run(mon, "@key esc");
        check(has(r, "action=close"), "painel XIR: ESC fecha");
    }
    {
        std::string r = Run(mon, "XTP HELLO.TXT");
        check(has(r, "title=XTP") && has(r, "linha1"), "painel XTP abre com o texto");
        r = Run(mon, "@key esc");
        check(has(r, "action=close"), "painel XTP: ESC fecha");
    }
    {
        Run(mon, "XRG SP,F000");
        std::string r = Run(mon, "XTR C800");
        check(has(r, "title=XTR") && has(r, "C800  3E 05"), "painel XTR abre e ja' executa o primeiro passo");
        r = Run(mon, "@key enter");
        check(has(r, "C802  06 03"), "painel XTR: ENTER executa a proxima instrucao");
        r = Run(mon, "@key esc");
        check(has(r, "action=close"), "painel XTR: ESC fecha");
    }
    Run(mon, "@panels off");
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0); // se algo abortar, o log ate aqui nao se perde
    TestOffline();
    TestEdit();
    TestScreens();
    TestSuperX();
    TestTape();
    TestOnline();
    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
