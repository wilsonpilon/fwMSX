// fwMSX -- Basic Dignified: o pipeline (ler -> lexer -> parser -> gerar o .amx -> tokenizar o .bmx), os argumentos do badig.py, as remtags `##BB:`
// do proprio fonte, o arquivo de configuracao INI e o comando de linha. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

#include "badig.h"
#include "badig_core.h"

namespace badig {

namespace fs = std::filesystem;

namespace {

bool IsTrueWord(const std::string &v, bool &out) {
    const std::string u = PyUpper(PyStripWs(v));
    if (u == "TRUE" || u == "YES" || u == "ON" || u == "1") {
        out = true;
        return true;
    }
    if (u == "FALSE" || u == "NO" || u == "OFF" || u == "0") {
        out = false;
        return true;
    }
    return false;
}

std::string StripExt(const std::string &p) {
    const std::string base = BaseName(p);
    const size_t dot = base.rfind('.');
    if (dot == std::string::npos || dot == 0) return p;
    return p.substr(0, p.size() - (base.size() - dot));
}

std::string ExtOf(const std::string &p) {
    const std::string base = BaseName(p);
    const size_t dot = base.rfind('.');
    return (dot == std::string::npos || dot == 0) ? std::string() : base.substr(dot);
}

bool ParseInt(const std::string &s, int &v) {
    if (s.empty()) return false;
    char *end = nullptr;
    const long r = std::strtol(s.c_str(), &end, 10);
    if (*end != '\0') return false;
    v = static_cast<int>(r);
    return true;
}

std::vector<std::string> SplitWs(const std::string &s) {
    std::vector<std::string> r;
    std::string cur;
    for (const char c : s) {
        if (IsPySpace(static_cast<unsigned char>(c))) {
            if (!cur.empty()) r.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) r.push_back(cur);
    return r;
}

// new_save_file do original: onde vai o .amx
std::string NewSaveFile(const std::string &file_load, const std::string &old_save_file) {
    std::string load_file = BaseName(file_load);
    load_file = StripExt(load_file);
    const std::string load_path = DirName(file_load);

    std::string save_file = BaseName(old_save_file);
    std::string save_path = DirName(old_save_file);
    while (!save_path.empty() && (save_path[0] == '\\' || save_path[0] == '/')) save_path.erase(0, 1);

    std::string result;
    if (old_save_file.empty()) {
        result = JoinPath(load_path, load_file);
    } else {
        if (!IsAbsPath(save_path)) {
            std::error_code ec;
            save_path = fs::absolute(fs::path(JoinPath(load_path, save_path)), ec).lexically_normal().string();
            if (!save_path.empty() && (save_path.back() == '/' || save_path.back() == '\\')) save_path.pop_back();
        }
        if (!save_path.empty() && !save_file.empty()) result = JoinPath(save_path, save_file);
        else if (!save_file.empty()) result = JoinPath(load_path, save_file);
        else result = JoinPath(save_path, load_file);
    }
    if (ExtOf(result).empty()) result += ".amx";
    return result;
}

bool WriteFile(const std::string &path, const std::string &data, std::string &err) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) {
        err = "Save folder not found: " + path;
        return false;
    }
    f.write(data.data(), static_cast<std::streamsize>(data.size()));
    return true;
}

std::string TokenRepr(const Token &t, bool utf8_input) {
    char b[32];
    std::snprintf(b, sizeof b, "%02d | %02d | ", t.pos ? t.pos->lin : 0, t.pos ? DisplayCol(*t.pos) : 0);
    return std::string(b) + (t.tok.empty() ? std::string("None") : t.tok) + " | " + PyRepr(t.val, utf8_input);
}

// extensao de saida do BASIC tokenizado e da listagem
const char *kAmxExt = ".amx";
const char *kBmxExt = ".bmx";
const char *kLmxExt = ".lmx";

} // namespace

// ---------------------------------------------------------------------------------------------------------------
// argumentos
// ---------------------------------------------------------------------------------------------------------------

bool ApplyArgs(const std::vector<std::string> &a, Options &o, std::string &err) {
    std::vector<std::string> pos;
    for (size_t i = 0; i < a.size(); ++i) {
        const std::string &t = a[i];
        auto value = [&](std::string &v) {
            if (i + 1 >= a.size()) {
                err = "argument " + t + ": expected one argument";
                return false;
            }
            v = a[++i];
            return true;
        };
        auto number = [&](int &v) {
            std::string s;
            if (!value(s)) return false;
            if (!ParseInt(s, v)) {
                err = "argument " + t + ": invalid int value: '" + s + "'";
                return false;
            }
            return true;
        };
        int n = 0;
        std::string s;

        if (t == "-ss") o.strip_spaces = true;
        else if (t == "-ca") o.capitalise_all = true;
        else if (t == "-tr") o.translate = true;
        else if (t == "-prr") o.print_report = true;
        else if (t == "-lbr") o.label_report = true;
        else if (t == "-lnr") o.line_report = true;
        else if (t == "-var") o.var_report = true;
        else if (t == "-lex") o.lexer_report = true;
        else if (t == "-par") o.parser_report = true;
        else if (t == "-asc") o.code_is_ascii = true;
        else if (t == "-rh") o.rem_header = false;
        else if (t == "-ini" || t == "-rtg") { /* tratados por quem chama */ }
        else if (t == "-tl") {
            if (!number(n)) return false;
            o.tab_length = std::abs(n);
        } else if (t == "-ls") {
            if (!number(n)) return false;
            o.line_start = std::abs(n);
        } else if (t == "-lp") {
            if (!number(n)) return false;
            o.line_step = std::abs(n);
        } else if (t == "-vb") {
            if (!number(n)) return false;
            o.verbose = n;
        } else if (t == "-cp") {
            if (!value(s)) return false;
            s = PyLower(s);
            if (s != "?" && s != "p") {
                err = "argument -cp: invalid choice: '" + s + "' (choose from '?', 'p')";
                return false;
            }
            o.convert_print = s;
        } else if (t == "-tg") {
            if (!value(s)) return false;
            s = PyLower(s);
            if (s != "t" && s != "g") {
                err = "argument -tg: invalid choice: '" + s + "' (choose from 't', 'g')";
                return false;
            }
            o.strip_then_goto = s;
        } else if (t == "-id") {
            if (i + 1 < a.size() && (a[i + 1].empty() || a[i + 1][0] != '-')) {
                s = a[++i];
                if (PyLower(s) != "msx") {
                    err = "only the MSX system is available in fwMSX (-id " + s + ")";
                    return false;
                }
            }
        } else if (t == "--tk_tokenize") o.tokenize = true;
        else if (t == "--no_tokenize") o.tokenize = false;
        else if (t == "--tk_del_ascii") o.tk_del_ascii = true;
        else if (t == "--tk_list") {
            o.tk_list = 16;
            if (i + 1 < a.size() && ParseInt(a[i + 1], n)) {
                ++i;
                o.tk_list = n;
            }
        } else if (t == "--tk_verbose") {
            if (!number(n)) return false;
            o.tk_verbose = n;
        } else if (t == "--em_run" || t == "--em_nothrottle" || t == "--em_monitor") { /* opcoes do openMSX: nao se aplicam */ }
        else if (t == "--em_setting" || t == "--em_machine" || t == "--em_extension" || t == "--em_verbose") {
            if (!value(s)) return false;
        } else if (!t.empty() && t[0] == '-' && t.size() > 1) {
            err = "unrecognized arguments: " + t;
            return false;
        } else {
            pos.push_back(t);
        }
    }
    if (pos.size() > 2) {
        err = "unrecognized arguments: " + pos[2];
        return false;
    }
    if (!pos.empty()) o.source = pos[0];
    if (pos.size() > 1) o.dest = pos[1];
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// configuracao em arquivo
// ---------------------------------------------------------------------------------------------------------------

bool LoadConfig(const std::string &path, Options &o) {
    std::ifstream f(path);
    if (!f) return false;
    std::string section, line;
    auto b = [](const std::string &v, bool &dst) {
        bool x;
        if (IsTrueWord(v, x)) dst = x;
    };
    auto n = [](const std::string &v, int &dst) {
        int x;
        if (ParseInt(PyStripWs(v), x)) dst = x;
    };
    while (std::getline(f, line)) {
        line = PyStripWs(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        if (line[0] == '[') {
            const size_t e = line.find(']');
            section = PyUpper(line.substr(1, e == std::string::npos ? std::string::npos : e - 1));
            continue;
        }
        size_t sep = line.find_first_of("=:");
        if (sep == std::string::npos) continue;
        const std::string key = PyLower(PyStripWs(line.substr(0, sep)));
        const std::string val = PyStripWs(line.substr(sep + 1));
        if (section == "CONFIGS") {
            if (key == "line_start") n(val, o.line_start);
            else if (key == "line_step") n(val, o.line_step);
            else if (key == "rem_header") b(val, o.rem_header);
            else if (key == "strip_spaces") b(val, o.strip_spaces);
            else if (key == "capitalize_all") b(val, o.capitalise_all);
            else if (key == "translate") b(val, o.translate);
            else if (key == "print_report") b(val, o.print_report);
            else if (key == "label_report") b(val, o.label_report);
            else if (key == "line_report") b(val, o.line_report);
            else if (key == "var_report") b(val, o.var_report);
            else if (key == "lexer_report") b(val, o.lexer_report);
            else if (key == "parser_report") b(val, o.parser_report);
            else if (key == "tab_lenght" || key == "tab_length") n(val, o.tab_length);
            else if (key == "verbose_level") n(val, o.verbose);
        } else if (section == "MSX") {
            if (key == "convert_print") o.convert_print = PyLower(val) == "p" || PyLower(val) == "print" ? "p" : (val == "?" ? "?" : "");
            else if (key == "strip_then_goto") {
                const std::string v = PyLower(val);
                o.strip_then_goto = (v == "t" || v == "then") ? "t" : (v == "g" || v == "goto") ? "g" : "";
            }
        } else if (section == "TOKENIZER") {
            if (key == "tokenize") b(val, o.tokenize);
            else if (key == "list") n(val, o.tk_list);
            else if (key == "del_ascii") b(val, o.tk_del_ascii);
            else if (key == "verbose") n(val, o.tk_verbose);
        }
    }
    return true;
}

bool SaveConfig(const std::string &path, const Options &o, std::string &err) {
    auto B = [](bool v) { return v ? "True" : "False"; };
    std::ostringstream s;
    s << "[CONFIGS]\n"
      << "system_id = msx\n"
      << "tab_lenght = " << o.tab_length << "\n"
      << "line_start = " << o.line_start << "\n"
      << "line_step = " << o.line_step << "\n"
      << "rem_header = " << B(o.rem_header) << "\n"
      << "strip_spaces = " << B(o.strip_spaces) << "\n"
      << "capitalize_all = " << B(o.capitalise_all) << "\n"
      << "translate = " << B(o.translate) << "\n"
      << "print_report = " << B(o.print_report) << "\n"
      << "label_report = " << B(o.label_report) << "\n"
      << "var_report = " << B(o.var_report) << "\n"
      << "line_report = " << B(o.line_report) << "\n"
      << "lexer_report = " << B(o.lexer_report) << "\n"
      << "parser_report = " << B(o.parser_report) << "\n"
      << "verbose_level = " << o.verbose << "\n\n"
      << "[MSX]\n"
      << "convert_print = " << o.convert_print << "\n"
      << "strip_then_goto = " << o.strip_then_goto << "\n\n"
      << "[TOKENIZER]\n"
      << "tokenize = " << B(o.tokenize) << "\n"
      << "list = " << o.tk_list << "\n"
      << "del_ascii = " << B(o.tk_del_ascii) << "\n"
      << "verbose = " << o.tk_verbose << "\n";
    return WriteFile(path, s.str(), err);
}

// ---------------------------------------------------------------------------------------------------------------
// o pipeline
// ---------------------------------------------------------------------------------------------------------------

namespace {

// ##BB:nome=valor
std::map<std::string, std::string> ReadRemtags(const std::string &text, std::vector<std::string> &unknown) {
    static const std::set<std::string> known = {"EXPORT_FILE", "ARGUMENTS", "HELP", "TK_TOKENIZE", "TK_LIST", "TK_DEL_ASCII", "TK_VERBOSE",
                                                "EM_RUN", "EM_SETTING", "EM_MACHINE", "EM_EXTENSION", "EM_NOTHROTTLE", "EM_MONITOR", "EM_VERBOSE"};
    std::map<std::string, std::string> r;
    size_t pos = 0;
    while (pos <= text.size()) {
        size_t nl = text.find_first_of("\r\n", pos);
        if (nl == std::string::npos) nl = text.size();
        const std::string line = PyLStripWs(text.substr(pos, nl - pos));
        if (line.size() > 5 && PyUpper(line.substr(0, 5)) == "##BB:") {
            size_t i = 5;
            while (i < line.size() && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_')) ++i;
            if (i > 5 && i < line.size() && line[i] == '=') {
                const std::string name = PyUpper(line.substr(5, i - 5));
                const std::string args = PyStripWs(line.substr(i + 1));
                if (!known.count(name)) unknown.push_back(name);
                r[name] = args;
            }
        }
        if (nl >= text.size()) break;
        pos = nl + 1;
        if (text[nl] == '\r' && pos < text.size() && text[pos] == '\n') ++pos;
    }
    return r;
}

void AddLog(Result &r, const std::vector<std::string> &lines) { r.log.insert(r.log.end(), lines.begin(), lines.end()); }

} // namespace

Result Convert(const Options &base, const std::vector<std::string> &cli_args, const std::string *text, bool write_files) {
    Result res;
    Logger log;
    Cfg cfg;
    Options o = base;
    std::string err;

    auto fail = [&](const std::string &msg) {
        Diag d;
        d.level = 1;
        d.text = msg;
        d.formatted = "*** " + msg;
        res.diags.push_back(d);
        res.log.push_back(d.formatted);
        res.ok = false;
        return res;
    };

    if (!ApplyArgs(cli_args, o, err)) return fail(err);
    if (o.source.empty()) return fail("File name not given.");

    // o fonte (para ler as remtags e o -asc)
    std::string source_text;
    if (text) {
        source_text = *text;
    } else {
        std::ifstream f(o.source, std::ios::binary);
        if (!f) return fail("File not found: " + o.source);
        source_text.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    }

    // remtags do proprio codigo: o ARGUMENTS reaplica os argumentos, os outros ajustam o destino e o tokenizador
    std::vector<std::string> unknown;
    const auto remtags = ReadRemtags(source_text, unknown);
    const auto rt = remtags.find("ARGUMENTS");
    if (rt != remtags.end() && !rt->second.empty()) {
        std::vector<std::string> more = cli_args;
        for (const std::string &x : SplitWs(rt->second)) more.push_back(x);
        o = base;
        if (!ApplyArgs(more, o, err)) return fail(err);
    }
    auto rbool = [&](const char *name, bool &dst) {
        const auto it = remtags.find(name);
        if (it == remtags.end()) return true;
        if (it->second.empty()) return true;
        bool x;
        if (!IsTrueWord(it->second, x) || (PyUpper(it->second) != "TRUE" && PyUpper(it->second) != "FALSE")) {
            err = std::string("Remtag must be true or false: ") + name + " = " + it->second;
            return false;
        }
        dst = x;
        return true;
    };
    if (!rbool("TK_TOKENIZE", o.tokenize) || !rbool("TK_DEL_ASCII", o.tk_del_ascii)) return fail(err);
    for (const char *name : {"TK_LIST", "TK_VERBOSE"}) {
        const auto it = remtags.find(name);
        if (it == remtags.end() || it->second.empty()) continue;
        int v;
        if (!ParseInt(it->second, v)) return fail(std::string("Remtag \"") + PyLower(name) + "\" must be a number: " + it->second);
        (std::string(name) == "TK_LIST" ? o.tk_list : o.tk_verbose) = v;
    }

    cfg.o = o;
    cfg.file_load = o.source;
    cfg.file_path = DirName(o.source);
    cfg.general_spaces = o.strip_spaces ? "" : " ";
    cfg.load_utf8 = o.translate;
    cfg.remtags = remtags;
    const auto ex = remtags.find("EXPORT_FILE");
    std::string out_arg = o.dest;
    if (ex != remtags.end() && !ex->second.empty()) out_arg = ex->second;
    cfg.file_save = o.code_is_ascii ? o.source : NewSaveFile(o.source, out_arg);
    SetLatin1Mode(!o.translate);

    log.level = o.verbose;
    for (const std::string &u : unknown) log.Log(2, "Remtag not available: " + u);

    const std::string amx_path = cfg.file_save;
    const std::string bmx_path = StripExt(amx_path) + kBmxExt;
    const std::string lmx_path = StripExt(amx_path) + kLmxExt;
    const std::string rep_base = amx_path;

    auto save = [&](const std::string &path, const std::string &data) {
        if (!write_files) return true;
        std::string e2;
        if (!WriteFile(path, data, e2)) {
            Diag d;
            d.level = 1;
            d.text = e2;
            d.formatted = "*** " + e2;
            res.diags.push_back(d);
            res.log.push_back(d.formatted);
            return false;
        }
        return true;
    };
    auto report = [&](const std::vector<std::string> &report_list, const std::string &header_text, const std::string &kind) {
        std::string body = header_text;
        for (const std::string &l : report_list) body += l;
        const std::string path = StripExt(rep_base) + "_" + kind + ".txt";
        if (o.print_report) {
            for (const std::string &l : report_list) log.lines.push_back(PyRStripWs(l));
        } else {
            res.reports.emplace_back(path, body);
            if (save(path, body)) log.Log(4, path + " saved.");
        }
    };

    Generated gen;
    try {
        std::string amx;
        if (o.code_is_ascii) {
            log.Log(3, "Basic Dignified: MSX");
            log.Log(3, "  Processing: " + BaseName(o.source));
            amx = source_text;
        } else {
            log.Log(3, "Basic Dignified: MSX");
            log.Log(3, "Converting: " + BaseName(o.source));

            log.Log(4, "Loading file: " + o.source);
            CodePtr d_code = LoadFile(o.source, &source_text, o.tab_length, log, nullptr);

            log.Log(5, "Lexing.");
            Lexer lex(cfg, d_code, log);
            std::vector<TokPtr> lexed = lex.Lex();
            log.Log(5, std::to_string(lexed.size()) + " tokens created.");

            if (o.lexer_report) {
                std::vector<std::string> list;
                for (const TokPtr &t : lexed) list.push_back(TokenRepr(*t, o.translate) + "\r");
                report(list, BaseName(o.source) + " lexer output\r" + std::to_string(lexed.size()) + " tokens\r\r", "lexer");
            }

            log.Log(5, "Parsing.");
            IncludeVars iv;
            iv.shorts = lex.des.hard_short_vars;
            iv.longs = lex.des.hard_long_vars;
            iv.declares = lex.des.declares;
            Parser par(cfg, lexed, lex.des, iv, log);
            IncludeVars iv_out;
            std::vector<TokPtr> parsed = par.Par(iv_out);
            log.Log(5, std::to_string(parsed.size()) + " tokens created.");

            if (o.parser_report) {
                std::vector<std::string> list;
                for (const TokPtr &t : parsed) {
                    // depois da passada 5 os literais ja' estao traduzidos: cada byte e' um caractere latin1
                    const bool literal = t->tok == "C_LITREM" || t->tok == "C_LIBREM" || t->tok == "C_LITQUO" || t->tok == "C_LITDAT";
                    list.push_back(TokenRepr(*t, o.translate && !literal) + "\r");
                }
                report(list, BaseName(o.source) + " parser output\r" + std::to_string(parsed.size()) + " tokens\r\r", "parser");
            }

            log.Log(5, "Generating Classic code.");
            gen = par.Generate();
            log.Log(5, std::to_string(gen.c_code.size()) + " lines created.");

            if (o.var_report) {
                std::vector<std::pair<std::string, std::string>> v(gen.declares.items.begin(), gen.declares.items.end());
                std::sort(v.begin(), v.end(), [](const auto &a, const auto &b) { return std::tie(a.second, a.first) > std::tie(b.second, b.first); });
                std::vector<std::string> list;
                for (const auto &kv : v) list.push_back(kv.second + ":" + kv.first.substr(0, kv.first.find('@')) + "\r\n");
                report(list, std::to_string(list.size()) + " variables assigned\r\r", "variables");
            }
            if (o.line_report) {
                std::vector<std::string> list;
                for (const auto &l : gen.line_report) {
                    list.push_back(std::to_string(l.line) + " - " + (l.dig_line == 0 ? std::string("Auto generated") : std::to_string(l.dig_line)) + "\r\n");
                }
                if (!list.empty()) list.erase(list.begin());
                report(list, std::to_string(list.size()) + " lines generated.\r(Classic - Dignified)\r\r", "lines");
            }

            for (const std::string &l : gen.c_code) amx += l;
            res.line_count = static_cast<int>(gen.c_code.size());
            log.Log(4, "Saving file: " + amx_path);
            if (!save(amx_path, amx)) {
                res.ok = false;
                res.amx = amx;
                AddLog(res, log.lines);
                res.diags.insert(res.diags.begin(), log.diags.begin(), log.diags.end());
                return res;
            }
        }
        res.amx = amx;
        res.amx_path = amx_path;

        if (o.tokenize) {
            log.Log(3, "MSX Basic Tokenizer");
            log.Log(3, "Tokenizing: " + BaseName(amx_path));
            Diag derr;
            std::vector<uint8_t> bmx;
            std::string lmx;
            if (!Tokenize(amx, amx_path, o.tk_list, bmx, lmx, derr)) {
                // o erro e' da linha do .amx: o numero BASIC da linha leva ao fonte pela tabela de linhas
                const size_t nth = static_cast<size_t>(std::max(derr.line, 1));
                const std::vector<std::string> alines = [&] {
                    std::vector<std::string> v;
                    std::istringstream in(amx);
                    std::string l;
                    while (std::getline(in, l)) {
                        l = PyStripWs(l);
                        if (!l.empty()) v.push_back(l);
                    }
                    return v;
                }();
                if (!o.code_is_ascii && nth - 1 < alines.size()) {
                    const long long basic = std::atoll(alines[nth - 1].c_str());
                    for (const auto &lr : gen.line_report) {
                        if (lr.line == basic) {
                            derr.file = lr.file;
                            derr.line = lr.dig_line;
                            break;
                        }
                    }
                }
                res.diags.insert(res.diags.end(), log.diags.begin(), log.diags.end());
                res.diags.push_back(derr);
                AddLog(res, log.lines);
                res.log.push_back(derr.formatted);
                res.ok = false;
                return res;
            }
            res.bmx = bmx;
            res.lmx = lmx;
            res.bmx_path = bmx_path;
            log.Log(4, "Saving file: " + bmx_path);
            if (!save(bmx_path, std::string(bmx.begin(), bmx.end()))) {
                res.ok = false;
                AddLog(res, log.lines);
                return res;
            }
            if (o.tk_list > 0) {
                res.lmx_path = lmx_path;
                log.Log(4, "Saving list: " + lmx_path);
                save(lmx_path, lmx);
            }
            if (o.tk_del_ascii && write_files && !o.code_is_ascii) {
                std::error_code ec;
                fs::remove(amx_path, ec);
                log.Log(4, "Deleting file: " + amx_path);
            }
        }
        res.ok = true;
    } catch (const BadigStop &) {
        res.ok = false;
    }

    // as mensagens: o que foi registrado (erros e avisos) e o que o original imprimiria
    std::vector<Diag> diags = log.diags;
    res.diags.insert(res.diags.begin(), diags.begin(), diags.end());
    AddLog(res, log.lines);
    return res;
}

// ---------------------------------------------------------------------------------------------------------------
// configuracao padrao
// ---------------------------------------------------------------------------------------------------------------

namespace {
std::string g_config_path;
} // namespace

void SetConfigPath(const std::string &path) { g_config_path = path; }
const std::string &ConfigPath() { return g_config_path; }

Options LoadDefaults() {
    Options o;
    if (!g_config_path.empty()) LoadConfig(g_config_path, o);
    return o;
}

// ---------------------------------------------------------------------------------------------------------------
// linha de comando
// ---------------------------------------------------------------------------------------------------------------

int RunBadigCommand(const std::vector<std::string> &args, const std::string &config_path) {
    Options o;
    if (!config_path.empty()) LoadConfig(config_path, o);

    bool want_ini = false, want_help = false;
    for (const std::string &a : args) {
        if (a == "-ini") want_ini = true;
        if (a == "-h" || a == "--help" || a == "-rtg") want_help = true;
    }
    std::string err;
    Options probe = o;
    if (!ApplyArgs(args, probe, err)) {
        std::cerr << "fwmsx --badig: " << err << "\n";
        return 2;
    }
    if (want_ini) {
        if (config_path.empty() || !SaveConfig(config_path, probe, err)) {
            std::cerr << "fwmsx --badig: " << (err.empty() ? "no configuration path" : err) << "\n";
            return 2;
        }
        std::cout << "--- .ini file written with current settings: " << config_path << "\n";
        return 0;
    }
    if (want_help || probe.source.empty()) {
        std::cerr << "uso: fwmsx --badig [opcoes] <fonte.dmx> [saida.amx]\n"
                     "  Converte o BASIC Dignified (.dmx) em BASIC ASCII (.amx) e em BASIC tokenizado (.bmx).\n"
                     "  -ls #  linha inicial (10)        -lp #  passo das linhas (10)     -rh  sem o cabecalho REM\n"
                     "  -ss    tira todos os espacos     -ca    maiusculas               -tr  traduz Unicode para o MSX (fonte UTF-8)\n"
                     "  -cp ?|p  PRINT <-> ?             -tg t|g  tira THEN antes do GOTO / GOTO depois do THEN\n"
                     "  -tl #  tamanho do TAB            -vb #  nivel das mensagens (0-5)\n"
                     "  -prr   mostra os relatorios      -lbr -lnr -var -lex -par  relatorios (labels, linhas, variaveis, lexer, parser)\n"
                     "  -asc   o arquivo ja' e' BASIC ASCII (so' tokeniza)\n"
                     "  --no_tokenize  so' gera o .amx   --tk_list [N]  gera o .lmx (N bytes por linha)   --tk_del_ascii  apaga o .amx\n"
                     "  -ini   grava as opcoes no badig.cfg\n"
                     "  Remtags no fonte:  ##BB:arguments=-ss -ca   ##BB:export_file=nome   ##BB:tk_list=16\n";
        return want_help ? 0 : 2;
    }

    Result r = Convert(o, args, nullptr, true);
    for (const std::string &l : r.log) std::cout << l << "\n";
    return r.ok ? 0 : 1;
}

} // namespace badig
