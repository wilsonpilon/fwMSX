// Leitor dos arquivos golden do montador (formato em tools/asmdiff/golden.py): cada caso tem o fonte e o
// que o asMSX de verdade produziu (codigo de saida, mensagens e arquivos de saida em hexadecimal).
#pragma once

#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace asmtest {

struct GoldenCase {
    std::string name;
    std::string source;
    int rc = 0;
    std::vector<std::string> msgs;
    std::map<std::string, std::string> outs; // extensao -> hex
    std::map<std::string, std::pair<size_t, unsigned>> outs_crc; // arquivos grandes: extensao -> (tamanho, CRC-32)
    // Arquivos auxiliares para .include / .incbin ("@file nome hex" no golden).
    std::map<std::string, std::string> files;
    // Argumentos de linha de comando ("@args -z ..."), se houver.
    std::vector<std::string> args;
};

inline std::vector<GoldenCase> ReadGolden(const std::string &path) {
    std::vector<GoldenCase> cases;
    std::ifstream in(path, std::ios::binary);
    std::string line;
    GoldenCase cur;
    bool in_src = false, open = false;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!in_src && line.rfind("@case ", 0) == 0) {
            cur = GoldenCase{};
            cur.name = line.substr(6);
            open = true;
            continue;
        }
        if (!open) continue;
        if (in_src) {
            if (line.rfind("@rc ", 0) == 0) {
                in_src = false;
                cur.rc = std::atoi(line.c_str() + 4);
            } else {
                cur.source += line + "\n";
            }
            continue;
        }
        if (line == "@src") {
            in_src = true;
        } else if (line.rfind("@msg ", 0) == 0) {
            cur.msgs.push_back(line.substr(5));
        } else if (line.rfind("@out ", 0) == 0) {
            const size_t sp = line.find(' ', 5);
            std::string name = line.substr(5, sp - 5);
            for (size_t p = name.find("%20"); p != std::string::npos; p = name.find("%20")) name.replace(p, 3, " ");
            cur.outs[name] = sp == std::string::npos ? "" : line.substr(sp + 1);
        } else if (line.rfind("@outcrc ", 0) == 0) {
            // @outcrc <arquivo> <tamanho> <crc32 hex>
            const size_t a = line.find(' ', 8), b = a == std::string::npos ? a : line.find(' ', a + 1);
            if (a != std::string::npos && b != std::string::npos) {
                std::string name = line.substr(8, a - 8);
                for (size_t p = name.find("%20"); p != std::string::npos; p = name.find("%20")) name.replace(p, 3, " ");
                cur.outs_crc[name] = {static_cast<size_t>(std::strtoull(line.c_str() + a + 1, nullptr, 10)),
                                      static_cast<unsigned>(std::strtoul(line.c_str() + b + 1, nullptr, 16))};
            }
        } else if (line.rfind("@file ", 0) == 0) {
            const size_t sp = line.find(' ', 6);
            cur.files[line.substr(6, sp - 6)] = sp == std::string::npos ? "" : line.substr(sp + 1);
        } else if (line.rfind("@args ", 0) == 0) {
            std::string rest = line.substr(6);
            size_t p = 0;
            while (p < rest.size()) {
                const size_t sp = rest.find(' ', p);
                cur.args.push_back(rest.substr(p, sp == std::string::npos ? std::string::npos : sp - p));
                if (sp == std::string::npos) break;
                p = sp + 1;
            }
        } else if (line == "@end") {
            cases.push_back(cur);
            open = false;
        }
    }
    return cases;
}

} // namespace asmtest
