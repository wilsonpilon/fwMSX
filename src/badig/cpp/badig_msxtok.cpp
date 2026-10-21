// fwMSX -- Basic Dignified: o tokenizador do MSX BASIC (porte do msxbatoken.py). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Converte o BASIC ASCII (linhas numeradas) no formato do arquivo .BAS do MSX: 0xFF, depois cada linha (ponteiro da proxima, numero, tokens, 0)
// e 0000. Fica identico ao original, inclusive nos casos de borda: o original monta TUDO como texto hexadecimal e so' no fim vira bytes; aqui a
// montagem tambem e' em texto hexadecimal, para manter a mesma aritmetica de digitos.
#include "badig.h"
#include "badig_core.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <regex>

extern "C" {
#include "../c/badig_tok.h"
#include "../fortran/badig_num.h"
}

namespace badig {

namespace {

constexpr int kBase = 0x8001;

struct TokError {
    Diag d;
};

std::string Hex(unsigned v, int width) {
    char b[24];
    std::snprintf(b, sizeof b, "%0*x", width, v);
    return b;
}
std::string H2(long long v) { return Hex(static_cast<unsigned>(v), 2); }
std::string H4(long long v) { return Hex(static_cast<unsigned>(v), 4); }

std::string Repeat(const std::string &s, long long n) {
    std::string r;
    for (long long i = 0; i < n; ++i) r += s;
    return r;
}

// bytes (hex "ab12") -> texto de 2 caracteres por byte invertido "low high" (hexa[2:] + hexa[:-2] do original)
std::string Swap4(const std::string &hex) { return hex.substr(2) + hex.substr(0, hex.size() - 2); }

std::string LStripCh(const std::string &s, char c) {
    size_t i = 0;
    while (i < s.size() && s[i] == c) ++i;
    return s.substr(i);
}
std::string RStripCh(const std::string &s, char c) {
    size_t i = s.size();
    while (i > 0 && s[i - 1] == c) --i;
    return s.substr(0, i);
}

// int("000123") em texto: sem zeros a esquerda, "0" se vazio
std::string DecNorm(const std::string &s) {
    std::string r = LStripCh(s, '0');
    return r.empty() ? std::string("0") : r;
}
int DecCmp(const std::string &a_in, const std::string &b_in) {
    const std::string a = DecNorm(a_in), b = DecNorm(b_in);
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    return a.compare(b) < 0 ? -1 : (a == b ? 0 : 1);
}
const std::string k63Nines(63, '9');
bool IsDigitStr(const std::string &s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return c >= '0' && c <= '9'; });
}
bool IsSpaceCh(unsigned char c) { return c == ' ' || (c >= 9 && c <= 13); }

class Tokenizer {
public:
    Tokenizer(const std::string &file, int list_bytes, Logger &log) : file_(file), list_(list_bytes), log_(log) {
        const int bw = std::min(std::abs(list_bytes), 32);
        width_byte_ = bw * 2;
        width_line_ = bw * 3 + 7;
    }

    struct ListData {
        std::vector<std::string> core;
        std::string hexa;
        long long adrs = 0;
        size_t lines = 0;
    };

    std::vector<std::string> Tok(const std::vector<std::string> &ascii_code, ListData &ld);
    int width_line() const { return width_line_; }

private:
    [[noreturn]] void Fatal(const std::string &desc) {
        Diag d;
        d.level = 1;
        d.file = file_;
        d.line = log_n_;
        d.col = log_col_;
        d.text = desc;
        const int col = log_col_ - 1;
        d.formatted = "*** " + BaseName(file_) + ": (" + std::to_string(log_n_) + "," + std::to_string(log_col_) + "): " + desc + "\n    " +
                      PyStripWs(log_text_) + "\n    " + std::string(static_cast<size_t>(std::max(col, 0)), '-') + "^" + "\n    Conversion stopped.";
        log_.lines.push_back(d.formatted);
        throw TokError{d};
    }
    void ShowSteps() {
        if (log_.level >= 5) log_.lines.push_back("    " + line_compiled_ + " | " + PyRStripWs(line_source_));
    }
    void UpdateLines() {
        if (line_source_.size() > 2) {
            line_source_ = line_source_.substr(std::min(static_cast<size_t>(source_), line_source_.size()));
            line_compiled_ += compiled_;
            ShowSteps();
        }
    }
    std::string ParseNumericBases(std::string &nugget_comp, const std::string &token, int base);
    std::string ParseSgnDbl(const std::string &header, int precision, std::string &nugget_integer, const std::string &nugget_fractional,
                            const std::string &g1, const std::string &nugget_number);
    std::vector<std::string> MakeListLine(long long base_prev, std::string line_base);
    bool Match(const std::regex &re, std::smatch &m) const {
        return std::regex_search(line_source_.cbegin(), line_source_.cend(), m, re, std::regex_constants::match_continuous);
    }

    std::string file_;
    int list_;
    Logger &log_;
    int width_byte_ = 0, width_line_ = 0;
    int log_n_ = 0, log_col_ = 1;
    std::string log_text_;

    int source_ = 0;
    std::string compiled_, line_compiled_, line_source_;
};

std::string Tokenizer::ParseNumericBases(std::string &nugget_comp, const std::string &token, int base) {
    std::string hexa;
    if (nugget_comp.empty()) {
        hexa = "0000";
    } else {
        unsigned long long v = 0;
        bool overflow = false;
        for (const char c : nugget_comp) {
            const int d = (c >= '0' && c <= '9') ? c - '0' : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : c - 'A' + 10;
            v = v * static_cast<unsigned>(base) + static_cast<unsigned>(d);
            if (v > 65535) {
                overflow = true;
                break;
            }
        }
        if (overflow) Fatal("Number overflow: " + nugget_comp);
        hexa = H4(static_cast<long long>(v));
    }
    return token + Swap4(hexa);
}

std::string Tokenizer::ParseSgnDbl(const std::string &header, int precision, std::string &nugget_integer, const std::string &nugget_fractional,
                                   const std::string &g1, const std::string &nugget_number) {
    std::string stripped = LStripCh(nugget_integer, '0');
    std::string hexa_precision;
    if (stripped.empty()) {
        const bool zero_frac = nugget_fractional.empty() || std::all_of(nugget_fractional.begin() + 1, nugget_fractional.end(), [](char c) { return c == '0'; });
        if (zero_frac) {
            stripped = "0";
            hexa_precision = "00";
        } else {
            nugget_integer = g1;
            if (nugget_fractional[1] == '0') {
                const std::string zeros = RStripCh(nugget_fractional.substr(1), '0');
                hexa_precision = H2(64 - static_cast<long long>(zeros.size() - LStripCh(zeros, '0').size()));
            } else {
                hexa_precision = "40";
            }
        }
    } else {
        hexa_precision = H2(static_cast<long long>(stripped.size()) + 64);
    }
    std::string hexa = header + hexa_precision;

    const std::string cropped = DecNorm(nugget_number); // str(int(nugget_number))
    char out[80];
    const int n = bd_round_digits(cropped.data(), static_cast<int>(cropped.size()), precision, out, static_cast<int>(sizeof out));
    hexa.append(out, static_cast<size_t>(n));
    return hexa;
}

std::vector<std::string> Tokenizer::MakeListLine(long long base_prev, std::string line_base) {
    long long line_inc = 12;
    std::vector<std::string> line_temp;
    std::string next_addr = line_compiled_.substr(0, 4);
    std::string curr_line = line_compiled_.size() > 4 ? line_compiled_.substr(4, 4) : std::string();
    const std::string line_byte = line_compiled_.size() > 8 ? line_compiled_.substr(8) : std::string();
    for (size_t pos = 0; pos < line_byte.size(); pos += static_cast<size_t>(width_byte_)) {
        const std::string line = line_byte.substr(pos, static_cast<size_t>(width_byte_));
        const std::string curr_addr = Hex(static_cast<unsigned>(base_prev), 1);
        std::string byte_splt;
        for (size_t i = 0; i < line.size(); i += 2) {
            if (i) byte_splt.push_back(' ');
            byte_splt += line.substr(i, 2);
        }
        const long long pad = width_line_ - static_cast<long long>(byte_splt.size());
        line_temp.push_back(curr_addr + ": " + next_addr + " " + curr_line + " " + byte_splt + std::string(static_cast<size_t>(std::max(pad, 0LL)), ' ') +
                            PyRStripWs(line_base));
        next_addr = "        ";
        curr_line.clear();
        line_base.clear();
        base_prev += line_inc;
        line_inc = static_cast<long long>(line.size()) / 2;
    }
    return line_temp;
}

std::vector<std::string> Tokenizer::Tok(const std::vector<std::string> &ascii_code, ListData &ld) {
    long long line_address = kBase;
    long long line_order = 0;
    std::vector<std::string> tokenized_code = {"ff"};
    std::string last_hexa;

    static const std::regex reLine(R"(\s*\d+\s?)");
    static const std::regex reAs(R"((\s*)(\d{1,2}))");
    static const std::regex reJump(R"((\s*)(\d+|,+))");
    static const std::regex reA(R"((\d*)\s*([^\n])\s*([^\n]?))");
    static const std::regex reB(R"((\d*)\s*([^\n])\s*(\d*)\s*([^\n])\s*([^\n]?))");
    static const std::regex reE(R"(\d*\s*[^\n]\s*\d*\s*[^\n]\s*(\+|-)\s*(\d+))");

    int n = 0;
    for (const std::string &src_line : ascii_code) {
        ++n;
        line_source_ = src_line;
        log_n_ = n;
        log_text_ = src_line;
        log_col_ = 1;

        const std::string line_base = line_source_;
        line_compiled_.clear();
        if (line_source_.empty()) continue;

        const unsigned char first = static_cast<unsigned char>(line_source_[0]);
        if (first < 48 || first > 57) {
            if (first == 26) continue; // o ^Z do fim de alguns arquivos
            Fatal("Line not starting with number.");
        }
        ShowSteps();

        // numero da linha
        std::smatch m;
        Match(reLine, m);
        const std::string nugget = m.str();
        const std::string line_number = PyStripWs(nugget);
        if (DecCmp(line_number, std::to_string(line_order)) <= 0) Fatal("Line number out of order: " + line_number);
        if (DecCmp(line_number, "65529") > 0) Fatal("Line number too high: " + line_number);
        line_order = std::stoll(DecNorm(line_number));
        line_source_ = line_source_.substr(nugget.size());
        {
            const std::string hexa = H4(line_order);
            line_compiled_ += Swap4(hexa);
        }
        ShowSteps();

        while (line_source_.size() > 2) {
            log_col_ = static_cast<int>(line_source_.size());

            const int ci = bd_tok_find(line_source_.data(), static_cast<int>(line_source_.size()));
            if (ci >= 0) {
                const std::string command = bd_tok_name(ci);
                compiled_ = bd_tok_hex(ci);
                source_ = static_cast<int>(command.size());
                UpdateLines();

                if (command == "AS") {
                    if (Match(reAs, m)) {
                        const std::string spaces = m[1].str(), digits = m[2].str();
                        if (digits.size() != 1) Fatal("Internal error: ord() expected a character on AS");
                        compiled_ = Repeat("20", static_cast<long long>(spaces.size())) + H2(static_cast<unsigned char>(digits[0]));
                        source_ = static_cast<int>(spaces.size() + digits.size());
                        UpdateLines();
                    }
                }

                // instrucao de desvio: numeros de linha (ou varias virgulas) depois dela
                if (bd_tok_is_jump(ci)) {
                    while (Match(reJump, m)) {
                        const std::string spaces = m[1].str(), what = m[2].str();
                        if (IsDigitStr(what)) {
                            if (DecCmp(what, "65529") > 0) Fatal("Line number jump too high: " + what);
                            const std::string hexa = H4(std::stoll(DecNorm(what)));
                            compiled_ = Repeat("20", static_cast<long long>(spaces.size())) + "0e" + Swap4(hexa);
                        } else {
                            compiled_ = Repeat("20", static_cast<long long>(spaces.size())) + Repeat("2c", static_cast<long long>(what.size()));
                        }
                        source_ = static_cast<int>(spaces.size() + what.size());
                        UpdateLines();
                    }
                }

                // instrucao com dados literais depois dela
                if (command == "DATA" || command == "REM" || command == "'" || command == "CALL" || command == "_") {
                    while (true) {
                        unsigned char character = static_cast<unsigned char>(line_source_[0]);
                        if (command == "CALL" || command == "_") {
                            if (character >= 'a' && character <= 'z') character = static_cast<unsigned char>(character - 32);
                            else if (character >= 0xE0 && character <= 0xFE && character != 0xF7) character = static_cast<unsigned char>(character - 32);
                        }
                        compiled_ = H2(character);
                        source_ = 1;
                        UpdateLines();
                        const char c0 = line_source_.empty() ? '\0' : line_source_[0];
                        if (line_source_.size() <= 2 || (command == "DATA" && c0 == ':') || (command == "_" && (c0 == ':' || c0 == '(')) ||
                            (command == "CALL" && (c0 == ':' || c0 == '('))) {
                            break;
                        }
                    }
                }
                continue;
            }

            // nenhuma palavra: caractere a caractere
            const unsigned char c0 = static_cast<unsigned char>(line_source_[0]);
            const unsigned char c0u = (c0 >= 'a' && c0 <= 'z') ? static_cast<unsigned char>(c0 - 32) : c0;
            bool is_int = false;

            if ((c0u >= '0' && c0u <= '9') || c0u == '.') {
                // numero
                Match(reA, m);
                std::string nugget_number = m[1].str();
                std::string nugget_integer = m[1].str();
                std::string nugget_fractional;
                std::string nugget_signal = m[2].str();
                std::string nugget_notif = m[3].str();
                std::string g1 = m[1].str();

                if (nugget_signal == ".") {
                    Match(reB, m);
                    g1 = m[1].str();
                    const std::string group1 = m[1].str().empty() ? std::string("0") : m[1].str();
                    nugget_number = group1 + m[3].str();
                    nugget_integer = group1;
                    nugget_fractional = "." + m[3].str();
                    nugget_signal = m[4].str();
                    nugget_notif = m[5].str();
                }

                const std::string sig_l = PyLower(nugget_signal);
                std::string hexa;
                if (nugget_signal == "%") {
                    nugget_number = nugget_integer;
                    is_int = true;
                    if (DecCmp(nugget_number, "32768") >= 0) Fatal("Integer overflow: " + nugget_number);
                } else if (nugget_signal != "%" && nugget_signal != "!" && nugget_signal != "#" &&
                           ((sig_l != "e" && sig_l != "d") || (nugget_notif != "-" && nugget_notif != "+"))) {
                    nugget_signal.clear();
                    if (nugget_fractional.empty()) is_int = true;
                }

                const std::string sig2 = PyLower(nugget_signal);
                if ((sig2 == "e" || sig2 == "d") && (nugget_notif == "-" || nugget_notif == "+")) {
                    // notacao cientifica
                    Match(reE, m);
                    const std::string exp_sign = m[1].str(), exp_digits = m[2].str();
                    const long long int_len = static_cast<long long>(LStripCh(nugget_integer, '0').size());
                    const long long exp_val = std::stoll(exp_digits) * (exp_sign == "-" ? -1 : 1);
                    const long long nugget_exp_size = int_len + exp_val;
                    const long long nugget_man_size = nugget_exp_size - static_cast<long long>(nugget_fractional.size() > 0 ? nugget_fractional.size() - 1 : 0) - int_len;

                    if (nugget_exp_size > 63 || nugget_exp_size < -64) Fatal("Float overflow: " + nugget_number);

                    const int fractional = nugget_man_size < 0 ? static_cast<int>(-nugget_man_size) : 0;
                    // '%.*f' % (fractional, int(nugget_number) * (10 ** nugget_man_size)): o original usa float em Python
                    double value;
                    if (nugget_man_size < 0) {
                        value = std::strtod(DecNorm(nugget_number).c_str(), nullptr) * std::pow(10.0, static_cast<double>(nugget_man_size));
                    } else {
                        value = std::strtod((DecNorm(nugget_number) + std::string(static_cast<size_t>(nugget_man_size), '0')).c_str(), nullptr);
                    }
                    std::vector<char> buf(400);
                    std::snprintf(buf.data(), buf.size(), "%.*f", fractional, value);
                    const std::string notation = buf.data();
                    const size_t dot = notation.find('.');
                    const std::string notation_integer = dot == std::string::npos ? notation : notation.substr(0, dot);
                    const std::string notation_fractional = dot == std::string::npos ? std::string() : notation.substr(dot);
                    std::string notation_number = notation;
                    notation_number.erase(std::remove(notation_number.begin(), notation_number.end(), '.'), notation_number.end());
                    const std::string notation_size = LStripCh(nugget_number, '0');

                    std::string ni = notation_integer;
                    if (sig2 == "e" && notation_size.size() < 7) {
                        hexa = ParseSgnDbl("1d", 6, ni, notation_fractional, g1, notation_number);
                        hexa += Repeat("0", 10 - static_cast<long long>(hexa.size()));
                    } else {
                        hexa = ParseSgnDbl("1f", 14, ni, notation_fractional, g1, notation_number);
                        hexa += Repeat("0", 18 - static_cast<long long>(hexa.size()));
                        hexa = hexa.substr(0, 18);
                    }
                    if (LStripCh(nugget_integer, '0').empty()) nugget_integer = g1;
                    nugget_signal += exp_sign + exp_digits;
                } else if ((DecCmp(nugget_number, "32768") >= 0 && DecCmp(nugget_number, "999999") <= 0 && nugget_signal != "#") ||
                           (nugget_signal == "!" && DecCmp(nugget_number, k63Nines) <= 0) ||
                           (!is_int && DecCmp(nugget_number, "999999") <= 0 && nugget_signal != "#")) {
                    // precisao simples
                    hexa = ParseSgnDbl("1d", 6, nugget_integer, nugget_fractional, g1, nugget_number);
                    hexa += Repeat("0", 10 - static_cast<long long>(hexa.size()));
                } else if ((DecCmp(nugget_number, "1000000") >= 0 && DecCmp(nugget_number, k63Nines) <= 0) ||
                           (nugget_signal == "#" && DecCmp(nugget_number, k63Nines) <= 0) || (!is_int && DecCmp(nugget_number, k63Nines) <= 0)) {
                    // precisao dupla
                    hexa = ParseSgnDbl("1f", 14, nugget_integer, nugget_fractional, g1, nugget_number);
                    hexa += Repeat("0", 18 - static_cast<long long>(hexa.size()));
                    hexa = hexa.substr(0, 18);
                } else if (DecCmp(nugget_number, "9") <= 0) {
                    hexa = H2(std::stoll(DecNorm(nugget_number)) + 17); // 0..9 -> 11..1A
                } else if (DecCmp(nugget_number, "255") <= 0) {
                    hexa = "0f" + H2(std::stoll(DecNorm(nugget_number)));
                } else if (DecCmp(nugget_number, "32767") <= 0) {
                    hexa = "1c" + Swap4(H4(std::stoll(DecNorm(nugget_number))));
                } else {
                    Fatal("Number too high: " + LStripCh(nugget_number, '0'));
                }

                compiled_ = hexa;
                source_ = static_cast<int>(nugget_integer.size() + nugget_fractional.size() + nugget_signal.size());
                UpdateLines();
            } else if (c0u == '&') {
                // outras bases
                const std::string two = PyUpper(line_source_.substr(0, 2));
                std::string hexa, nugget_comp, nug;
                const std::string tail = line_source_.size() > 2 ? line_source_.substr(2) : std::string();
                if (two == "&H") {
                    nug = two;
                    for (const char c : PyLower(tail)) {
                        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) break;
                        nugget_comp.push_back(c);
                    }
                    hexa = ParseNumericBases(nugget_comp, "0c", 16);
                } else if (two == "&O") {
                    nug = two;
                    for (const char c : tail) {
                        if (c < '0' || c > '7') break;
                        nugget_comp.push_back(c);
                    }
                    hexa = ParseNumericBases(nugget_comp, "0b", 8);
                } else if (two == "&B") {
                    nug = two;
                    for (const char c : tail) {
                        if (c != '0' && c != '1') break;
                        nugget_comp.push_back(c);
                    }
                    hexa = "2642";
                    for (const char c : nugget_comp) hexa += H2(static_cast<unsigned char>(c));
                } else {
                    nug = "&";
                    hexa = "26";
                }
                compiled_ = hexa;
                source_ = static_cast<int>(nug.size() + nugget_comp.size());
                UpdateLines();
            } else if (c0u == '"') {
                int num_quotes = 0;
                while (true) {
                    if (line_source_[0] == '"') ++num_quotes;
                    compiled_ = H2(static_cast<unsigned char>(line_source_[0]));
                    source_ = 1;
                    UpdateLines();
                    if (num_quotes > 1 || line_source_.size() <= 2) break;
                }
            } else if (c0u >= 65 && c0u <= 90) {
                // nome de variavel: letras e digitos ate' aparecer uma palavra
                bool is_var = true;
                while (true) {
                    const unsigned char ch = static_cast<unsigned char>(line_source_[0]);
                    const unsigned char chu = (ch >= 'a' && ch <= 'z') ? static_cast<unsigned char>(ch - 32) : ch;
                    if (bd_tok_find(line_source_.data(), static_cast<int>(line_source_.size())) >= 0) is_var = false;
                    if (((chu < 48 || chu > 57) && (chu < 65 || chu > 90)) || !is_var) break;
                    compiled_ = H2(chu);
                    source_ = 1;
                    UpdateLines();
                }
            } else {
                unsigned char u = c0;
                if (u >= 'a' && u <= 'z') u = static_cast<unsigned char>(u - 32);
                else if (u >= 0xE0 && u <= 0xFE && u != 0xF7) u = static_cast<unsigned char>(u - 32);
                compiled_ = H2(u);
                source_ = 1;
                UpdateLines();
            }
        }

        const long long base_prev = line_address;
        line_address += (static_cast<long long>(line_compiled_.size()) + 6) / 2;
        last_hexa = H4(line_address);
        line_compiled_ = Swap4(last_hexa) + line_compiled_;
        line_compiled_ += "00";
        tokenized_code.push_back(line_compiled_);

        if (list_) {
            auto l = MakeListLine(base_prev, line_base);
            ld.core.insert(ld.core.end(), l.begin(), l.end());
        }
    }
    tokenized_code.push_back("0000");
    ld.hexa = last_hexa;
    ld.adrs = line_address;
    ld.lines = ascii_code.size();
    return tokenized_code;
}

} // namespace

bool Tokenize(const std::string &ascii, const std::string &file_name, int list_bytes, std::vector<uint8_t> &out, std::string &lmx, Diag &err) {
    Logger log;
    log.level = 3;
    // IO.load_file do tokenizador: pula as linhas em branco e as so' de digitos; cada linha vira  texto.strip() + CRLF
    std::vector<std::string> ascii_code;
    size_t pos = 0;
    while (pos < ascii.size()) {
        size_t nl = ascii.find_first_of("\r\n", pos);
        if (nl == std::string::npos) nl = ascii.size();
        const std::string line = PyStripWs(ascii.substr(pos, nl - pos));
        if (!(line.empty() || IsDigitStr(line))) ascii_code.push_back(line + "\r\n");
        if (nl < ascii.size() && ascii[nl] == '\r' && nl + 1 < ascii.size() && ascii[nl + 1] == '\n') ++nl;
        pos = nl + 1;
    }

    Tokenizer tk(file_name, list_bytes, log);
    Tokenizer::ListData ld;
    std::vector<std::string> hex;
    try {
        hex = tk.Tok(ascii_code, ld);
    } catch (const TokError &e) {
        err = e.d;
        return false;
    }

    out.clear();
    for (const std::string &line : hex) {
        for (size_t i = 0; i + 1 < line.size(); i += 2) out.push_back(static_cast<uint8_t>(std::stoi(line.substr(i, 2), nullptr, 16)));
    }

    lmx.clear();
    if (list_bytes > 0) {
        const int width_line = tk.width_line();
        std::time_t t = std::time(nullptr);
        char date[32];
        std::strftime(date, sizeof date, "%Y-%m-%d %H:%M:%S", std::localtime(&t));
        std::vector<std::string> list_code = {"' -------------------------------------", "' MSX Basic Tokenizer: \"" + BaseName(file_name) + "\"",
                                              std::string("' Date: ") + date, "' -------------------------------------", ""};
        list_code.push_back(Hex(kBase - 1, 1) + ": ff" + std::string(static_cast<size_t>(width_line + 8), ' ') + "start");
        list_code.insert(list_code.end(), ld.core.begin(), ld.core.end());
        list_code.push_back(ld.hexa + ": 0000" + std::string(static_cast<size_t>(width_line + 6), ' ') + "end");
        for (const char *s : {"", "' -------------------------------------", "' Statistics", "' -------------------------------------", ""}) list_code.push_back(s);
        list_code.push_back("lines " + std::to_string(ld.lines));
        list_code.push_back("start &h" + Hex(kBase - 1, 4) + " | " + std::to_string(kBase - 1));
        list_code.push_back("end   &h" + Hex(static_cast<unsigned>(ld.adrs + 1), 4) + " | " + std::to_string(ld.adrs + 1));
        list_code.push_back("size  &h" + Hex(static_cast<unsigned>(ld.adrs - kBase + 3), 4) + " | " + std::to_string(ld.adrs - kBase + 3));
        for (const std::string &l : list_code) lmx += l + "\r\n";
    }
    return true;
}

} // namespace badig
