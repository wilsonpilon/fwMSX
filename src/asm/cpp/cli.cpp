// fwMSX -- montador (fwasm): linha de comando. Codigo ORIGINAL do fwMSX (BSD-3-Clause); opcoes conforme o manual
// do asMSX 1.2.0 (secao 1.5.1).
#include "cli.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>

#include "assembler.h"

namespace fwasm {

namespace {

bool ReadAll(const std::string &path, std::vector<uint8_t> &data) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    data.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    return true;
}

bool WriteAll(const std::string &path, const std::vector<uint8_t> &data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    return static_cast<bool>(out);
}

size_t LastSlash(const std::string &s) { return s.find_last_of("/\\"); }

// Includes e incbin sao relativos ao diretorio do fonte (o -r do asMSX ficou obsoleto).
class DiskFiles : public FileProvider {
public:
    explicit DiskFiles(std::string dir) : dir_(std::move(dir)) {}
    // O montador ja' manda o caminho com o diretorio do fonte na frente (ou so' o nome, relativo ao diretorio atual, como o asMSX aceita);
    // prefixar de novo o diretorio do fonte quebrava todo include de um fonte em subpasta.
    bool Read(const std::string &path, std::vector<uint8_t> &data) override { return ReadAll(path, data) || ReadAll(dir_ + path, data); }

private:
    std::string dir_;
};

// Tira a extensao do nome-base, se houver (so' do ultimo componente).
std::string StripExt(const std::string &p) {
    const size_t slash = LastSlash(p), dot = p.rfind('.');
    return (dot != std::string::npos && (slash == std::string::npos || dot > slash)) ? p.substr(0, dot) : p;
}

} // namespace

int RunAsmCommand(const std::vector<std::string> &tokens) {
    AsmOptions opts;
    opts.silent = false;
    std::string source, out_arg;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string &t = tokens[i];
        if (t == "-z") opts.zilog = true;
        else if (t == "-s") opts.silent = true;
        else if (t == "-vv") opts.verbose = true;
        else if (t == "-r" || t == "-d") { /* -r obsoleto; -d so' existe no asMSX compilado com YYDEBUG */ }
        else if (t == "-o") {
            if (++i >= tokens.size()) {
                std::cerr << "fwmsx --asm: -o precisa de um caminho\n";
                return 2;
            }
            out_arg = tokens[i];
        } else if (!t.empty() && t[0] == '-') {
            std::cerr << "fwmsx --asm: opcao desconhecida: " << t << "\n";
            return 2;
        } else if (source.empty()) {
            source = t;
        } else {
            std::cerr << "fwmsx --asm: so' um arquivo fonte por vez\n";
            return 2;
        }
    }
    if (source.empty()) {
        std::cerr << "uso: fwmsx --asm [-z] [-s] [-vv] [-o <caminho>] <arquivo.asm>\n"
                     "  -z   sintaxe Zilog padrao (sem precisar da diretiva .zilog)\n"
                     "  -s   modo silencioso (sem mensagens de progresso)\n"
                     "  -vv  mensagens detalhadas\n"
                     "  -o   saida: nome-base (sem extensao), ou pasta terminada em / ou \\ (usa o nome do fonte)\n";
        return 2;
    }

    std::vector<uint8_t> raw;
    if (!ReadAll(source, raw)) {
        std::cerr << "fwmsx --asm: nao foi possivel ler '" << source << "'\n";
        return 2;
    }
    const size_t slash = LastSlash(source);
    const std::string dir = slash == std::string::npos ? "" : source.substr(0, slash + 1);
    const std::string file = slash == std::string::npos ? source : source.substr(slash + 1);

    // Nome-base das saidas: padrao = fonte sem extensao; -o troca (arquivo) ou escolhe a pasta.
    std::string base = StripExt(source);
    if (!out_arg.empty()) {
        if (out_arg.back() == '/' || out_arg.back() == '\\') base = out_arg + StripExt(file);
        else base = StripExt(out_arg);
    }
    if (!out_arg.empty()) opts.out_base = base; // sem -o o .FILENAME do programa decide o nome

    DiskFiles files(dir);
    const AsmResult r = Assemble(source, std::string(raw.begin(), raw.end()), opts, &files);
    for (const std::string &m : r.msgs) std::cerr << m << "\n";
    if (r.rc != 0) return r.rc;

    for (const auto &kv : r.outs) {
        // A chave e' a extensao ("rom", "sym"...) ou, quando o programa pede outro nome (.filename), um caminho relativo.
        const std::string path = kv.first.find('.') == std::string::npos ? base + "." + kv.first : dir + kv.first;
        if (!WriteAll(path, kv.second)) {
            std::cerr << "fwmsx --asm: nao foi possivel gravar '" << path << "'\n";
            return 2;
        }
        if (!opts.silent) std::cout << "gerado: " << path << " (" << kv.second.size() << " bytes)\n";
    }
    return 0;
}

} // namespace fwasm
