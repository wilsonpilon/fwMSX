// fwMSX -- Basic Dignified: o lexer (porte do Lexer do badig.py e do lexer do modulo MSX). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// O original le' caractere a caractere e cresce o prefixo enquanto ALGUM grupo ainda casa o prefixo inteiro; o token e' o ultimo grupo que casou.
// Aqui `Description::Classify` faz o papel do `commands.match`.
#include "badig_core.h"

#include <algorithm>

namespace badig {

namespace {

constexpr char kNl = '\r'; // newline_str do original: cada linha lida termina com um '\r' de sentinela

std::string PySliceAllButLast(const std::string &s, size_t n) { return s.size() > n ? s.substr(0, s.size() - n) : std::string(); }
std::string PyLastN(const std::string &s, size_t n) { return s.size() > n ? s.substr(s.size() - n) : s; }

} // namespace

Lexer::Lexer(const Cfg &cfg, CodePtr code, Logger &log) : cfg_(cfg), log_(log), code_(std::move(code)) {
    // O original repete a primeira linha (PROGRAM) no fim "porque advance() ignora a ultima": essa linha vira o identificador PROGRAM antes do EOF.
    code_->push_back((*code_)[0]);
    pos_ = std::make_shared<Position>(code_, 0, -1);
}

char Lexer::Lookahead() const {
    return pos_->col >= 0 && pos_->col < static_cast<int>(pos_->text.size()) ? pos_->text[static_cast<size_t>(pos_->col)] : kNl;
}

std::optional<std::string> Lexer::AdvanceLine() {
    pos_->lin += 1;
    pos_->col = 0;
    if (pos_->lin <= pos_->code_len - 1) {
        pos_->text = PyRStripWs((*code_)[static_cast<size_t>(pos_->lin)].text) + kNl;
        return pos_->text;
    }
    return std::nullopt;
}

std::optional<char> Lexer::Advance() {
    const char c = Lookahead();
    pos_->col += 1;
    if (pos_->col >= static_cast<int>(pos_->text.size())) {
        if (!AdvanceLine()) return std::nullopt;
    }
    return c;
}

TokPtr Lexer::GetToken() {
    std::string partial;
    std::string match_group;
    PosPtr last_pos = pos_->Copy();
    while (true) {
        const char next_char = Lookahead();
        const char *g = Description::Classify(partial + next_char);
        if (!*g) {
            std::string group = match_group;
            if (partial.empty()) {
                group.clear();
                partial = std::string(1, next_char);
            }
            return std::make_shared<Token>(group, PyStrip(partial, " "), last_pos);
        }
        const auto adv = Advance();
        if (!adv) return std::make_shared<Token>("EOF", "EOF", last_pos);
        partial.push_back(*adv);
        match_group = g;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// literais
// ---------------------------------------------------------------------------------------------------------------

std::optional<std::vector<TokPtr>> Lexer::GetLitBlock(const TokPtr &tk) {
    std::string partial;
    std::vector<TokPtr> rem_block = {tk};
    const size_t val_len = tk->val.size();

    if (pos_->col + 1 >= static_cast<int>(pos_->text.size())) {
        AdvanceLine();
        pos_->col -= 1;
    }
    PosPtr last_pos_lit = pos_->Copy(std::nullopt, std::max(pos_->col, 0));

    while (true) {
        const auto adv = Advance();
        if (!adv) return std::nullopt;
        partial.push_back(*adv);
        const char next_char = Lookahead();
        PosPtr last_pos_close = pos_->Copy(std::nullopt, pos_->col - static_cast<int>(val_len));

        if (next_char == kNl) {
            // re.match(tk.val, partial[-val_len:]): o abridor ("###" ou "''") fecha o bloco quando aparece no fim da linha
            const bool g = PyLastN(partial, val_len) == tk->val;
            if (g) partial = PySliceAllButLast(partial, val_len);
            partial = PyStrip(partial, std::string(1, kNl));

            // se nao esta' sozinho numa linha com o indicador do bloco
            if (!(PyStripWs(PySliceAllButLast(partial, val_len)).empty() && g)) {
                rem_block.push_back(std::make_shared<Token>("C_LIBREM", partial, last_pos_lit));
            }
            if (g) {
                rem_block.push_back(std::make_shared<Token>("C_C_BREM", tk->val, last_pos_close));
                break;
            }
            if (pos_->lin + 1 >= pos_->code_len) return std::nullopt;
            last_pos_lit = pos_->Copy(pos_->lin + 1, 0);
            partial.clear();
        }
    }
    if (tk->tok == "D_O_EREM") rem_block.clear();
    return rem_block;
}

TokPtr Lexer::GetLitLine(const std::string &tok, char end, const std::string &part_in, PosPtr l_pos) {
    std::string part = part_in;
    if (Lookahead() == kNl) return nullptr;
    while (true) {
        const auto last_chr = Advance();
        if (!last_chr) return nullptr;
        part.push_back(*last_chr);
        const char next_chr = Lookahead();
        if (next_chr == end || next_chr == kNl) {
            if (next_chr != kNl) {
                const auto a = Advance();
                if (a) part.push_back(*a);
            }
            break;
        }
    }
    return std::make_shared<Token>(tok, part, std::move(l_pos));
}

// O DATA tem regras proprias: o que vem depois e' texto literal ate' o ':' ou o fim da linha, a virgula separa os itens e um '_' no fim une a
// proxima linha.
std::vector<TokPtr> Lexer::GetDataLine(bool &has) {
    has = false;
    std::string part, join;
    std::vector<TokPtr> data_line;
    PosPtr l_pos = pos_->Copy(std::nullopt, pos_->col + 1);

    while (Lookahead() == ' ') Advance();
    if (Lookahead() == ':') return data_line;

    char last_chr = 0;
    while (true) {
        const auto adv = Advance();
        if (!adv) return data_line;
        last_chr = *adv;
        part.push_back(last_chr);

        if (PyStripWs(part).empty()) {
            part.clear();
            continue;
        }

        if (part == "\"") {
            while (true) {
                const auto a2 = Advance();
                if (!a2) return data_line;
                last_chr = *a2;
                part.push_back(last_chr);
                const char nc = Lookahead();
                if (last_chr == '"' || nc == kNl) break;
            }
        }

        const char next_char = Lookahead();
        const bool l = last_chr == ',';
        const bool n = next_char == ':' || next_char == kNl;
        if (l || n) {
            if (l) {
                while (!part.empty() && part.back() == ',') part.pop_back();
            }
            data_line.push_back(std::make_shared<Token>("C_LITDAT", join + part, l_pos));

            if (l) {
                PosPtr new_pos = pos_->Copy(std::nullopt, pos_->col - 1);
                data_line.push_back(std::make_shared<Token>("C_SYMBOL", ",", new_pos));
                l_pos = pos_->Copy();
                part.clear();
                join.clear();
            }

            if (n) {
                if (next_char == ':' || part.empty()) break;
                if (part.back() != '_') break;
                // linha terminada em '_': une com a proxima
                std::string v = data_line.back()->val;
                data_line.pop_back();
                while (!v.empty() && v.back() == '_') v.pop_back();
                join = v;
                part.clear();
                Advance();
            }
        }
    }
    has = true;
    return data_line;
}

// ---------------------------------------------------------------------------------------------------------------
// o laco principal
// ---------------------------------------------------------------------------------------------------------------

std::vector<TokPtr> Lexer::Lex() {
    tk_ = std::make_shared<Token>("NEWLINE", std::string(1, kNl), pos_->Copy());
    lexer_ = {std::make_shared<Token>("PROGRAM", "PROGRAM", pos_->Copy()), tk_};
    last_tok_ = tk_;

    AdvanceLine();

    while (true) {
        last_tok_ = lexer_.back();
        tk_ = GetToken();

        if (!tk_->Truthy()) continue;

        // Erros ----------------------------------------------------------
        if (tk_->tok.empty()) {
            log_.Fatal("Character not recognized in this context: " + tk_->val, tk_);
        } else if (tk_->tok == "C_PARTLS" || tk_->tok == "D_PARTLS") {
            log_.Fatal("Token incomplete: " + tk_->val, tk_);
        }

        const std::string last_kind = lexer_.back()->tok;
        if (tk_->tok == "D_TGLREM" && last_kind != "NEWLINE" && last_kind != "D_TGLREM" && last_kind != "D_INSTRC") {
            // um toggle fora do inicio da linha: volta a ser '#' e um identificador/numero
            const std::string hash = "#";
            const std::string rest = tk_->val.substr(1);
            lexer_.push_back(std::make_shared<Token>(Description::Classify(hash), hash, pos_->Copy()));
            tk_ = std::make_shared<Token>(Description::Classify(rest), rest, pos_->Copy());
        }
        // Blocos literais ------------------------------------------------
        else if ((tk_->tok == "C_O_BREM" || tk_->tok == "D_O_EREM") && last_tok_->tok == "NEWLINE") {
            const TokPtr last_tok = tk_->Copy();
            auto rem_block = GetLitBlock(tk_);
            if (!rem_block) log_.Fatal("Block not closed from: " + last_tok->val, last_tok);
            lexer_.insert(lexer_.end(), rem_block->begin(), rem_block->end());
            continue;
        }
        // Linhas literais ------------------------------------------------
        else if (tk_->tok == "D_LINREM") { // comentario exclusivo do Dignified (##)
            pos_->col = static_cast<int>(pos_->text.size()) - 1;
            continue;
        } else if (tk_->tok == "C_O_BREM" && last_tok_->tok != "NEWLINE") {
            lexer_.push_back(std::make_shared<Token>("C_LINREM", "'", pos_->Copy(std::nullopt, pos_->col - 2)));
            tk_ = GetLitLine("C_LITREM", kNl, "'", pos_->Copy(std::nullopt, pos_->col - 1));
            if (!tk_) continue;
        } else if (tk_->tok == "C_LINREM") {
            lexer_.push_back(tk_);
            TokPtr lit_rem = GetLitLine("C_LITREM", kNl, "", pos_->Copy(std::nullopt, pos_->col));
            if (!lit_rem) continue;
            tk_ = lit_rem;
        } else if (tk_->tok == "C_QUOTES") {
            pos_->col -= 1;
            tk_ = GetLitLine("C_LITQUO", '"', "", pos_->Copy(std::nullopt, pos_->col));
            if (!tk_) continue;
        }
        // Modulo classico: o DATA --------------------------------------
        else if (tk_->tok == "C_DATALN") {
            lexer_.push_back(tk_);
            bool has = false;
            std::vector<TokPtr> data_line = GetDataLine(has);
            lexer_.insert(lexer_.end(), data_line.begin(), data_line.end());
            continue;
        }

        lexer_.push_back(tk_);
        if (tk_->tok == "EOF") break;
    }
    return lexer_;
}

// ---------------------------------------------------------------------------------------------------------------
// tokens a partir de um texto solto
// ---------------------------------------------------------------------------------------------------------------

std::vector<TokPtr> TokStr(const Description &, const std::string &text_in, PosPtr pos) {
    std::vector<TokPtr> tokens;
    std::string text = text_in;
    while (true) {
        std::string partial, match_group;
        std::string text_buf = text + " ";
        TokPtr token;
        while (true) {
            const char next_char = text_buf.empty() ? ' ' : text_buf[0];
            const char *g = Description::Classify(partial + next_char);
            if (!*g) {
                std::string group = match_group;
                if (partial.empty()) {
                    group.clear();
                    partial = std::string(1, next_char);
                }
                token = std::make_shared<Token>(group, PyStrip(partial, " "), pos);
                break;
            }
            partial.push_back(next_char);
            if (!text_buf.empty()) text_buf.erase(0, 1);
            match_group = g;
        }
        tokens.push_back(token);
        const size_t cut = std::min(token->val.size(), text.size());
        text = PyStripWs(text.substr(cut));
        if (text.empty()) return tokens;
        if (token->val.empty()) return tokens; // nada consumido: evita laco infinito (o original travaria)
    }
}

} // namespace badig
