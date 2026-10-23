// fwMSX -- assinaturas do openMSX. Ver sig.h.
#include "sig.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

extern "C" {
#include "xml_scan.h"
}

namespace fs = std::filesystem;

namespace romdb {
namespace {

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool IsSha1(const std::string &s) {
    return s.size() == 40 && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isxdigit(c) != 0; });
}

std::string Entities(std::string s) {
    const std::pair<const char *, const char *> table[] = {{"&amp;", "&"}, {"&lt;", "<"}, {"&gt;", ">"}, {"&quot;", "\""}, {"&apos;", "'"}};
    for (const auto &e : table) {
        size_t pos = 0;
        while ((pos = s.find(e.first, pos)) != std::string::npos) {
            s.replace(pos, std::strlen(e.first), e.second);
            pos += std::strlen(e.second);
        }
    }
    return s;
}

}  // namespace

void ParseOpenmsxXml(const std::string &xml, const std::string &stem, bool extension, std::vector<SigRow> &out) {
    XmlScan scan{xml.data(), xml.data() + xml.size(), nullptr, 0};
    XmlTok tok;
    struct Open {
        std::string name, id;
    };
    std::vector<Open> stack;
    std::string manufacturer, code, name, type;
    std::string field;           // a tag de texto aberta agora (manufacturer, code, name, type, sha1, filename)
    bool in_rom = false;          // dentro de <rom>...</rom> (minusculo: a lista de arquivos; <ROM id=...> maiusculo e' o dispositivo)
    std::string rom_role;         // o id do dispositivo que contem o <rom>
    std::vector<std::string> rom_hashes;
    std::string rom_file;
    size_t first_row = out.size();

    while (xml_next(&scan, &tok) != XML_END) {
        const std::string tag(tok.name ? tok.name : "", static_cast<size_t>(tok.name_len));
        if (tok.kind == XML_OPEN) {
            char id[256];
            if (xml_attr(tok.attrs, tok.attrs_len, "id", id, sizeof(id)) < 0) id[0] = '\0';
            if (tag == "rom") {
                in_rom = true;
                rom_role = stack.empty() ? std::string() : stack.back().id;
                rom_hashes.clear();
                rom_file.clear();
            }
            stack.push_back({tag, id});
            field = (tag == "manufacturer" || tag == "code" || tag == "name" || tag == "type" || tag == "sha1" || tag == "filename") ? tag : std::string();
        } else if (tok.kind == XML_TEXT) {
            const std::string text = Entities(std::string(tok.text, static_cast<size_t>(tok.text_len)));
            // <info> do topo do arquivo (nao o <name> de outros blocos: so' vale quando o pai e' <info>)
            const bool in_info = stack.size() >= 2 && stack[stack.size() - 2].name == "info";
            if (field == "manufacturer" && in_info) manufacturer = text;
            else if (field == "code" && in_info) code = text;
            else if (field == "name" && in_info) name = text;
            else if (field == "type" && in_info) type = text;
            else if (in_rom && field == "sha1") {
                const std::string h = Lower(text);
                if (IsSha1(h)) rom_hashes.push_back(h);
            } else if (in_rom && field == "filename") {
                rom_file = text;
            }
        } else if (tok.kind == XML_CLOSE) {
            field.clear();
            if (tag == "rom" && in_rom) {
                for (const std::string &h : rom_hashes) {
                    SigRow r;
                    r.sha1 = h;
                    r.filename = rom_file;
                    r.role = rom_role;
                    r.source = extension ? "extension" : "machine";
                    out.push_back(r);
                }
                in_rom = false;
            }
            if (!stack.empty()) stack.pop_back();
        }
    }

    // O hardware e a familia so' sao conhecidos no fim (o <info> vem antes das ROMs, mas nao se depende da ordem).
    std::string hardware;
    if (extension) {
        hardware = !name.empty() ? name : stem;
    } else {
        hardware = manufacturer;
        if (!code.empty()) hardware += (hardware.empty() ? "" : " ") + code;
        if (hardware.empty()) hardware = stem;
    }
    for (size_t i = first_row; i < out.size(); ++i) {
        out[i].hardware = hardware;
        out[i].family = type;
        // Extensao: o "papel" mais util e' o nome da extensao ("MSX-DOS 2"), nao o id interno do dispositivo.
        if (extension && !name.empty()) out[i].role = name;
        if (out[i].role.empty()) out[i].role = type;
    }
}

bool ReadOpenmsxShare(const std::string &share_dir, std::vector<SigRow> &rows, int &files, std::string &error) {
    files = 0;
    bool any_dir = false;
    for (const char *sub : {"machines", "extensions"}) {
        const fs::path dir = fs::path(share_dir) / sub;
        std::error_code ec;
        if (!fs::is_directory(dir, ec)) continue;
        any_dir = true;
        std::vector<fs::path> list;
        for (const auto &de : fs::directory_iterator(dir, ec)) {
            if (de.is_regular_file() && de.path().extension() == ".xml") list.push_back(de.path());
        }
        std::sort(list.begin(), list.end());
        for (const fs::path &p : list) {
            std::ifstream in(p, std::ios::binary);
            if (!in) continue;
            std::stringstream ss;
            ss << in.rdbuf();
            ParseOpenmsxXml(ss.str(), p.stem().string(), std::string(sub) == "extensions", rows);
            ++files;
        }
    }
    if (!any_dir) {
        error = "'" + share_dir + "' nao tem machines/ nem extensions/ (use a pasta share do openMSX)";
        return false;
    }
    return true;
}

}  // namespace romdb
