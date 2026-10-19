// fwMSX -- Mamute: configuracao inicial (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md.
#include "mamute_config.h"

#include <cctype>
#include <cstdlib>
#include <fstream>

namespace mamute {

namespace {

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

// "S.P" -> slot e pagina (0-3 cada); false se nao e' desse formato.
bool ParseSlotPage(const std::string &s, int &slot, int &page) {
    if (s.size() != 3 || s[1] != '.' || s[0] < '0' || s[0] > '3' || s[2] < '0' || s[2] > '3') return false;
    slot = s[0] - '0';
    page = s[2] - '0';
    return true;
}

} // namespace

const char *CellName(CellType t) {
    switch (t) {
    case CellType::Empty: return "VAZIO";
    case CellType::Ram: return "RAM";
    case CellType::Rom: return "ROM";
    case CellType::Basic: return "BASIC";
    default: return "?";
    }
}

bool ParseCellType(const std::string &s, CellType &t) {
    const std::string k = Lower(s);
    if (k == "vazio" || k == "empty") t = CellType::Empty;
    else if (k == "ram") t = CellType::Ram;
    else if (k == "rom") t = CellType::Rom;
    else if (k == "basic") t = CellType::Basic;
    else return false;
    return true;
}

Config::Config() {
    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            cell[s][p] = s == 3 ? CellType::Ram : CellType::Empty;
            half[s][p] = false;
        }
    }
}

std::string ConfigPathFor(const std::string &exe_dir) {
    if (exe_dir.empty()) return "mamute.cfg";
    const char last = exe_dir.back();
    return (last == '/' || last == '\\') ? exe_dir + "mamute.cfg" : exe_dir + "/mamute.cfg";
}

namespace {
std::string g_config_dir;
} // namespace

void SetConfigDir(const std::string &exe_dir) { g_config_dir = exe_dir; }
std::string DefaultConfigPath() { return ConfigPathFor(g_config_dir); }

Config LoadConfig(const std::string &path) {
    Config cfg;
    std::ifstream f(path);
    if (!f) return cfg;
    // se o arquivo fala de celulas, ele manda em TODAS (o padrao "slot 3 = RAM" so' vale sem configuracao)
    bool cells_reset = false;
    std::string line;
    while (std::getline(f, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = Lower(Trim(line.substr(0, eq)));
        const std::string val = Trim(line.substr(eq + 1));
        if (key == "online") {
            cfg.start_online = val == "1";
        } else if (key == "vram") {
            const int v = std::atoi(val.c_str());
            if (v == 16 || v == 128 || v == 192) cfg.vram_kb = v;
        } else if (key == "dump") {
            const int v = std::atoi(val.c_str());
            if (v >= 0 && v <= 3) cfg.dump_mode = v;
        } else if (key.compare(0, 5, "cell.") == 0 || key.compare(0, 5, "file.") == 0 || key.compare(0, 5, "half.") == 0) {
            int s = 0, p = 0;
            if (!ParseSlotPage(key.substr(5), s, p)) continue;
            if (key[0] == 'c') {
                if (!cells_reset) {
                    for (int a = 0; a < 4; ++a)
                        for (int b = 0; b < 4; ++b) cfg.cell[a][b] = CellType::Empty;
                    cells_reset = true;
                }
                CellType t;
                if (ParseCellType(val, t)) cfg.cell[s][p] = t;
            } else if (key[0] == 'f') {
                cfg.file[s][p] = val;
            } else {
                cfg.half[s][p] = val == "1";
            }
        }
    }
    return cfg;
}

bool SaveConfig(const std::string &path, const Config &cfg) {
    std::ofstream f(path, std::ios::trunc);
    if (!f) return false;
    f << "# Mamute (monitor do fwMSX) -- ver doc/mamute-spec.md\n";
    f << "online=" << (cfg.start_online ? 1 : 0) << "\n";
    f << "vram=" << cfg.vram_kb << "\n";
    f << "dump=" << cfg.dump_mode << "\n";
    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            std::string name = CellName(cfg.cell[s][p]);
            for (char &c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            f << "cell." << s << "." << p << "=" << name << "\n";
            if (!cfg.file[s][p].empty()) f << "file." << s << "." << p << "=" << cfg.file[s][p] << "\n";
            if (cfg.half[s][p]) f << "half." << s << "." << p << "=1\n";
        }
    }
    return static_cast<bool>(f);
}

} // namespace mamute
