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
    checkOut(mon, "OFFLINE", "MODO OFFLINE", "OFFLINE volta para a memoria privada");
#endif
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0); // se algo abortar, o log ate aqui nao se perde
    TestOffline();
    TestEdit();
    TestScreens();
    TestOnline();
    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
