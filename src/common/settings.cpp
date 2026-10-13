// fwMSX -- configuracoes do usuario gravadas em arquivo. Ver settings.h.
#include "settings.h"

#include <cstdlib>
#include <fstream>

namespace settings {

namespace {

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r')) --b;
    return s.substr(a, b - a);
}

int Clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

} // namespace

std::string PathFor(const std::string &exe_dir) {
    if (exe_dir.empty()) return "fwmsx.cfg";
    const char last = exe_dir.back();
    return exe_dir + ((last == '/' || last == '\\') ? "" : "/") + "fwmsx.cfg";
}

Settings Load(const std::string &path) {
    Settings s;
    std::ifstream in(path);
    if (!in) return s;
    std::string line;
    while (std::getline(in, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = Trim(line.substr(0, eq));
        const int value = std::atoi(Trim(line.substr(eq + 1)).c_str());
        if (key == "splash") s.splash = value != 0;
        else if (key == "theme") s.theme = Clamp(value, 0, 1);
        else if (key == "font") s.font = Clamp(value, 0, 2);
        else if (key == "frame") s.frame = value != 0;
    }
    return s;
}

bool Save(const std::string &path, const Settings &s) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;
    out << "# fwMSX -- configuracoes (gravado pelo proprio programa; pode editar a mao)\n"
        << "splash=" << (s.splash ? 1 : 0) << "\n"
        << "theme=" << s.theme << "\n"
        << "font=" << s.font << "\n"
        << "frame=" << (s.frame ? 1 : 0) << "\n";
    return static_cast<bool>(out);
}

} // namespace settings
