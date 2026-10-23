// fwMSX -- teste do clone do Basic Dignified Suite.
//
// 1. Os arquivos de referencia (tests/badig/golden) foram gerados pelo badig.py ORIGINAL (v2.2.1, resource/basic-dignified) com
//    tools/badig_compare/compare.py: cada pasta tem o .amx, o .bmx, o .lmx, os relatorios (-lex -par -lnr -var -lbr) ou a mensagem de erro do original.
//    O clone precisa sair IGUAL, byte a byte.
// 2. Testes de unidade do resto: argumentos, configuracao, remtags, mensagens com linha/coluna para o editor, execucao sem gravar em disco.
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/badig/cpp/badig.h"

namespace fs = std::filesystem;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, const std::string &what) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::printf("[FAIL] %s\n", what.c_str());
    }
}

std::string slurp(const fs::path &p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

void spit(const fs::path &p, const std::string &d) {
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f.write(d.data(), static_cast<std::streamsize>(d.size()));
}

std::string NormLmx(std::string s) {
    const size_t a = s.find("' Date: ");
    if (a != std::string::npos) {
        const size_t b = s.find_first_of("\r\n", a);
        s.replace(a, b == std::string::npos ? std::string::npos : b - a, "' Date: -");
    }
    return s;
}

std::vector<std::string> SplitWs(const std::string &s) {
    std::vector<std::string> r;
    std::istringstream in(s);
    std::string t;
    while (in >> t) r.push_back(t);
    return r;
}

// as linhas de mensagem de erro ("***" e as indentadas), sem o diretorio do fonte
std::vector<std::string> ErrLines(const std::vector<std::string> &log, const std::string &dir) {
    std::vector<std::string> out;
    for (const std::string &entry : log) {
        std::istringstream in(entry);
        std::string l;
        while (std::getline(in, l)) {
            if (l.rfind("***", 0) != 0 && l.rfind("    ", 0) != 0) continue;
            for (const std::string &sep : {std::string("\\"), std::string("/")}) {
                const std::string d = dir + sep;
                for (size_t p; (p = l.find(d)) != std::string::npos;) l.erase(p, d.size());
            }
            out.push_back(l);
        }
    }
    return out;
}

fs::path FindSource(const fs::path &data, const std::string &stem) {
    for (const auto &e : fs::recursive_directory_iterator(data)) {
        if (!e.is_regular_file()) continue;
        const std::string name = e.path().filename().string();
        if (name == stem + ".dmx" || name == stem + ".amx") return e.path();
    }
    return {};
}

void test_golden(const fs::path &root) {
    const fs::path golden = root / "golden", data = root / "data";
    int cases = 0;
    for (const auto &d : fs::directory_iterator(golden)) {
        if (!d.is_directory()) continue;
        const std::string stem = d.path().filename().string();
        const fs::path src = FindSource(data, stem);
        if (src.empty()) {
            check(false, stem + ": fonte nao encontrado");
            continue;
        }
        const bool asc = src.extension() == ".amx";
        std::vector<std::string> args = {src.string(), "-lex", "-par", "-lnr", "-var", "-lbr", "--tk_tokenize", "--tk_list", "16"};
        if (fs::exists(d.path() / "args.txt")) {
            for (const std::string &t : SplitWs(slurp(d.path() / "args.txt"))) args.push_back(t);
        }
        // os relatorios -lex/-par sao do modo Dignified
        if (asc) args = {src.string(), "-asc", "--tk_tokenize", "--tk_list", "16"};

        badig::Options base;
        const badig::Result r = badig::Convert(base, args, nullptr, false);
        ++cases;

        const fs::path gerr = d.path() / ("golden_" + stem + ".err");
        if (fs::exists(gerr)) {
            std::vector<std::string> want;
            {
                std::istringstream in(slurp(gerr));
                std::string l;
                while (std::getline(in, l)) want.push_back(l);
            }
            const std::vector<std::string> got = ErrLines(r.log, src.parent_path().string());
            check(!r.ok, stem + ": devia falhar");
            check(got == want, stem + ": mensagem de erro igual a do original");
            if (got != want) {
                for (const std::string &l : got) std::printf("   got : %s\n", l.c_str());
                for (const std::string &l : want) std::printf("   want: %s\n", l.c_str());
            }
        } else {
            check(r.ok, stem + ": converte");
            if (!r.ok) {
                for (const std::string &l : r.log) std::printf("   %s\n", l.c_str());
                continue;
            }
        }
        if (fs::exists(d.path() / ("golden_" + stem + ".amx"))) {
            check(r.amx == slurp(d.path() / ("golden_" + stem + ".amx")), stem + ": .amx igual ao do original");
        }
        if (fs::exists(d.path() / ("golden_" + stem + ".bmx"))) {
            const std::string want = slurp(d.path() / ("golden_" + stem + ".bmx"));
            check(std::string(r.bmx.begin(), r.bmx.end()) == want, stem + ": .bmx igual ao do original");
        }
        if (fs::exists(d.path() / ("golden_" + stem + ".lmx"))) {
            check(NormLmx(r.lmx) == NormLmx(slurp(d.path() / ("golden_" + stem + ".lmx"))), stem + ": .lmx igual ao do original");
        }
        for (const auto &f : fs::directory_iterator(d.path())) {
            const std::string name = f.path().filename().string();
            if (name.size() < 12 || name.compare(0, 7, "golden_") != 0 || name.substr(name.size() - 4) != ".txt") continue;
            bool found = false;
            for (const auto &rep : r.reports) {
                if (fs::path(rep.first).filename().string() == name.substr(7)) {
                    found = true;
                    check(rep.second == slurp(f.path()), stem + ": relatorio " + name.substr(7) + " igual ao do original");
                }
            }
            check(found, stem + ": relatorio " + name.substr(7) + " gerado");
        }
    }
    check(cases >= 40, "casos de referencia encontrados (" + std::to_string(cases) + ")");
}

void test_units() {
    // argumentos
    badig::Options o;
    std::string err;
    check(badig::ApplyArgs({"-ss", "-ca", "-ls", "100", "-lp", "5", "a.dmx", "b.amx"}, o, err) && o.strip_spaces && o.capitalise_all && o.line_start == 100 &&
              o.line_step == 5 && o.source == "a.dmx" && o.dest == "b.amx",
          "argumentos do badig.py");
    check(!badig::ApplyArgs({"-zz"}, o, err) && err.find("unrecognized") != std::string::npos, "opcao desconhecida");
    check(!badig::ApplyArgs({"-cp", "x"}, o, err), "-cp so' aceita ? ou p");
    check(!badig::ApplyArgs({"-id", "coco"}, o, err), "so' o MSX existe");
    check(badig::ApplyArgs({"--tk_list", "--tk_del_ascii", "x.dmx"}, o, err) && o.tk_list == 16 && o.tk_del_ascii, "--tk_list sem valor = 16");

    // configuracao em arquivo
    const fs::path dir = fs::temp_directory_path() / "fwbadig_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    badig::Options c;
    c.line_start = 20;
    c.line_step = 2;
    c.strip_spaces = true;
    c.convert_print = "?";
    c.strip_then_goto = "g";
    c.tk_list = 8;
    c.tokenize = false;
    check(badig::SaveConfig((dir / "badig.cfg").string(), c, err), "grava o badig.cfg");
    badig::Options l;
    check(badig::LoadConfig((dir / "badig.cfg").string(), l) && l.line_start == 20 && l.line_step == 2 && l.strip_spaces && l.convert_print == "?" &&
              l.strip_then_goto == "g" && l.tk_list == 8 && !l.tokenize,
          "le o badig.cfg de volta");

    // converter um texto sem gravar em disco (o buffer do editor)
    badig::Options base;
    base.source = (dir / "x.dmx").string();
    const std::string text = "print \"hi\"\n{a}\nx = 1\ngoto {a}\n";
    badig::Result r = badig::Convert(base, {}, &text, false);
    check(r.ok && r.amx.find("30 print \"hi\"") != std::string::npos && r.amx.find("goto 40") != std::string::npos, "converte o texto do buffer");
    check(!fs::exists(dir / "x.amx") && !fs::exists(dir / "x.bmx"), "sem write_files nada e' gravado");
    check(!r.bmx.empty() && r.bmx[0] == 0xFF && r.bmx.size() > 8, "o .bmx tokenizado comeca por FF");
    check(r.bmx[r.bmx.size() - 1] == 0 && r.bmx[r.bmx.size() - 2] == 0, "e termina com 0000");

    // erro de execucao do BASIC -> linha do fonte (a tabela de linhas vem sempre, sem -lnr)
    {
        badig::LineMapEntry m;
        check(badig::MapBasicLine(r, 30, m) && m.line == 1 && fs::path(m.file).filename() == "x.dmx", "BASIC 30 = linha 1 do .dmx");
        check(badig::MapBasicLine(r, 50, m) && m.line == 4, "BASIC 50 = linha 4 do .dmx (o goto)");
        check(!badig::MapBasicLine(r, 55, m) && !badig::MapBasicLine(r, 0, m), "linha BASIC que nao existe");
        check(badig::ParseBasicErrorLine("Syntax error in 130") == 130 && badig::ParseBasicErrorLine("Type mismatch in 40\r\nOk") == 40 &&
                  badig::ParseBasicErrorLine("130") == 130 && badig::ParseBasicErrorLine("in 10 in 20") == 20 && badig::ParseBasicErrorLine("Syntax error") == 0 &&
                  badig::ParseBasicErrorLine("Error in 70000") == 0,
              "extrai o numero da linha da mensagem do BASIC");
        std::string msg;
        check(badig::FindBasicError("Syntax error in 130\nOk\n", &msg) == 130 && msg == "Syntax error in 130" &&
                  badig::FindBasicError("RUN\nType mismatch in 40   \r\n\nOk\n") == 40 && badig::FindBasicError("Break in 20\nOk") == 20 &&
                  badig::FindBasicError("Hello in 5\nOk\n") == 0 && badig::FindBasicError("Syntax error in 130\nrunning\n") == 0 &&
                  badig::FindBasicError("Syntax error\nOk\n") == 0 && badig::FindBasicError("Error in 70000\nOk\n") == 0 && badig::FindBasicError("") == 0,
              "acha o erro de execucao e o prompt Ok na tela do MSX (sem confundir a saida do programa)");
    }

    // gravando
    r = badig::Convert(base, {}, &text, true);
    check(r.ok && fs::exists(dir / "x.amx") && fs::exists(dir / "x.bmx"), "write_files grava .amx e .bmx ao lado do fonte");
    check(slurp(dir / "x.amx") == r.amx, "o .amx gravado e' o devolvido");
    {
        std::ifstream f(dir / "x.bmx", std::ios::binary);
        const std::string b((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        check(b == std::string(r.bmx.begin(), r.bmx.end()), "o .bmx gravado e' o devolvido");
    }

    // erros com posicao para o editor
    const std::string bad = "print 1\ngoto {nowhere}\n";
    r = badig::Convert(base, {}, &bad, false);
    check(!r.ok && !r.diags.empty() && r.diags.back().level == 1 && r.diags.back().line == 2 && r.diags.back().text.find("Label does not exist") == 0 &&
              fs::path(r.diags.back().file).filename() == "x.dmx",
          "erro com arquivo e linha");
    const std::string bad2 = "a = 1\nx = 99999999999999999999999999999999999999999999999999999999999999999999999\n";
    r = badig::Convert(base, {}, &bad2, false);
    check(!r.ok && !r.diags.empty() && r.diags.back().text.find("Number too high") == 0 && r.diags.back().line == 2, "erro do tokenizador volta para a linha do .dmx");
    const std::string bad3 = "exit\n";
    r = badig::Convert(base, {}, &bad3, false);
    check(!r.ok && r.diags.back().text.find("Exit without a loop label") == 0, "exit fora de um laco avisa (o original quebra)");

    // remtags do proprio fonte
    const std::string rt = "##BB:arguments=-ss -ls 100\n##BB:tk_list=8\n##BB:foo=1\nprint \"a b\"\n";
    r = badig::Convert(base, {}, &rt, false);
    check(r.ok && r.amx.find("100'Converted") != std::string::npos, "remtag ARGUMENTS");
    check(!r.lmx.empty(), "remtag TK_LIST gera a listagem");
    bool warned = false;
    for (const std::string &s : r.log) warned = warned || s.find("Remtag not available: FOO") != std::string::npos;
    check(warned, "remtag desconhecida avisa");

    // so' o ASCII -> tokenizado
    std::vector<uint8_t> out;
    std::string lmx;
    badig::Diag d;
    check(badig::Tokenize("10 PRINT \"A\"\r\n", "x.amx", 0, out, lmx, d), "tokeniza BASIC ASCII");
    // FF | proximo=800B | linha 10 | PRINT=91 | espaco | "A" | 00 | 0000
    const std::vector<uint8_t> want = {0xFF, 0x0B, 0x80, 0x0A, 0x00, 0x91, 0x20, 0x22, 0x41, 0x22, 0x00, 0x00, 0x00};
    check(out == want, "bytes do 10 PRINT \"A\"");
    check(!badig::Tokenize("PRINT 1\r\n", "x.amx", 0, out, lmx, d) && d.text == "Line not starting with number." && d.line == 1, "linha sem numero");
    check(!badig::Tokenize("20 A=1\r\n10 B=2\r\n", "x.amx", 0, out, lmx, d) && d.text.find("Line number out of order") == 0, "linhas fora de ordem");

    // opcoes que mudam a saida
    badig::Options so;
    so.source = (dir / "y.dmx").string();
    const std::string t2 = "x = true\ny = x + 1\nx++\nprint x\n";
    r = badig::Convert(so, {"-rh", "-ls", "1", "-lp", "1"}, &t2, false);
    check(r.ok && r.amx.rfind("1 ", 0) == 0 && r.amx.find("-1") != std::string::npos, "-rh tira o cabecalho, -ls/-lp");

    // NestorBASIC: o include "nbasic.dmx" sai da biblioteca (nao ha' um na pasta do fonte) e o NBASIC.BIN vai para o disco
    // (o NBASIC.BIN de verdade nao vem no repositorio nem no pacote: baixa-se pela janela Downloads; aqui vale um arquivo de mentira ao lado do nbasic.dmx)
    const fs::path src_lib = fs::path(BADIG_TEST_DIR).parent_path().parent_path() / "src" / "badig" / "lib";
    const fs::path lib = dir / "lib";
    fs::create_directories(lib);
    fs::copy_file(src_lib / "nbasic.dmx", lib / "nbasic.dmx", fs::copy_options::overwrite_existing);
    spit(lib / "NBASIC.BIN", "NBASIC-DE-MENTIRA");
    badig::SetLibraryDir(lib.string());
    badig::Options nb;
    nb.source = (dir / "nb.dmx").string();
    const std::string nbt = "include \"nbasic.dmx\"\nx = .NB_ReadByte(1, 100)\n";
    r = badig::Convert(nb, {}, &nbt, false);
    check(r.ok && r.amx.find("USR(2)") != std::string::npos || r.amx.find("usr(2)") != std::string::npos, "include \"nbasic.dmx\" resolve na biblioteca: .NB_ReadByte -> usr(2)");
    check(r.includes.size() == 1 && fs::path(r.includes[0]).filename() == "nbasic.dmx", "Result::includes lista o que foi incluido");
    check(r.amx.find("usr(1)") == std::string::npos && r.amx.find("'---") == std::string::npos && r.amx.size() < 600,
          "da biblioteca so' ficam as funcoes chamadas, sem os comentarios (" + std::to_string(r.amx.size()) + " bytes)");
    std::vector<badig::DiskFile> files;
    check(badig::CollectDiskFiles(r, files, err) && files.size() == 1 && files[0].msx == "NBASIC.BIN", "incluir o nbasic.dmx leva o NBASIC.BIN ao disco");

    // arquivos de apoio: nomes e mascaras da pasta do fonte
    spit(dir / "dados.bin", "x");
    spit(dir / "mapa.dat", "y");
    spit(dir / "Logo.BIN", "z");
    spit(dir / "nome muito longo.txt", "w");
    badig::Options df;
    df.source = (dir / "df.dmx").string();
    df.disk_files = "*.bin mapa.dat";
    const std::string dft = "print 1\n";
    r = badig::Convert(df, {}, &dft, true);
    files.clear();
    check(r.ok && badig::CollectDiskFiles(r, files, err) && files.size() == 3, "disk_files: duas mascaras e um nome");
    bool has_logo = false, has_map = false;
    for (const auto &f : files) {
        has_logo = has_logo || f.msx == "LOGO.BIN";
        has_map = has_map || f.msx == "MAPA.DAT";
    }
    check(has_logo && has_map, "nomes em maiusculas no disco (8.3)");
    df.disk_files = "nome muito longo.txt";
    r.options = df;
    files.clear();
    check(!badig::CollectDiskFiles(r, files, err), "nome que nao e' 8.3 e' recusado");
    df.disk_files = "nao_existe.bin";
    r.options = df;
    check(!badig::CollectDiskFiles(r, files, err) && err.find("not found") != std::string::npos, "arquivo que nao existe e' recusado");
    const std::string rtx = "##BB:disk_files=mapa.dat\n##BB:nbasic=true\nprint 1\n";
    badig::Options plain;
    plain.source = (dir / "rt.dmx").string();
    r = badig::Convert(plain, {}, &rtx, false);
    files.clear();
    check(r.ok && badig::CollectDiskFiles(r, files, err) && files.size() == 2, "remtags DISK_FILES e NBASIC");
    badig::SetLibraryDir("");
    r = badig::Convert(plain, {}, &rtx, false);
    check(!badig::CollectDiskFiles(r, files, err) && err.find("NBASIC.BIN not found") == 0, "sem o NBASIC.BIN avisa");
    check(badig::ApplyArgs({"--disk_files", "a.bin b.bin", "--nbasic"}, plain, err) && plain.disk_files == "a.bin b.bin" && plain.nbasic, "argumentos --disk_files e --nbasic");
    badig::Options sv = plain;
    check(badig::SaveConfig((dir / "n.cfg").string(), sv, err), "grava");
    badig::Options ld;
    check(badig::LoadConfig((dir / "n.cfg").string(), ld) && ld.disk_files == "a.bin b.bin" && ld.nbasic, "o badig.cfg guarda os arquivos de apoio e o NestorBASIC");

    fs::remove_all(dir);
}

} // namespace

int main(int argc, char **argv) {
    const fs::path root = argc > 1 ? fs::path(argv[1]) : fs::path(BADIG_TEST_DIR);
    test_golden(root);
    test_units();
    std::printf("%d verificacoes, %d falhas\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
