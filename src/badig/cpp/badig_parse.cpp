// fwMSX -- Basic Dignified: o parser em 5 passadas e a geracao do codigo classico (porte do Parser do badig.py e do parser do modulo MSX).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Os tokens sao COMPARTILHADOS entre as listas (como os objetos do Python): varias passadas alteram o
// `val`/`pos` de um token que ja' esta' em outra lista, e o resultado depende disso.
#include "badig_core.h"

#include <algorithm>
#include <cstdint>

#include "badig_trans.inc"

namespace badig {

namespace {

// Python: s[:n] / s[n:] sem estourar
std::string Head(const std::string &s, size_t n) { return s.substr(0, std::min(n, s.size())); }

bool IsAlnumAscii(const std::string &s) {
    if (s.empty()) return false;
    return std::all_of(s.begin(), s.end(), [](unsigned char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); });
}

bool AllDigits(const std::string &s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return c >= '0' && c <= '9'; });
}

// ^[a-zA-z][a-zA-Z0-9]?$ (o intervalo A-z do original inclui [ \ ] ^ _ `)
bool IsShortVarName(const std::string &s) {
    if (s.empty() || s.size() > 2) return false;
    const unsigned char a = static_cast<unsigned char>(s[0]);
    if (!((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'z'))) return false;
    if (s.size() == 2) {
        const unsigned char b = static_cast<unsigned char>(s[1]);
        if (!((b >= 'a' && b <= 'z') || (b >= 'A' && b <= 'Z') || (b >= '0' && b <= '9'))) return false;
    }
    return true;
}

bool IsLitTok(const std::string &t) { return t == "C_LITREM" || t == "C_LIBREM" || t == "C_LITQUO" || t == "C_LITDAT"; }

bool HasLong(const std::vector<TokPtr> &v, const TokPtr &t) { return std::find(v.begin(), v.end(), t) != v.end(); }

std::vector<TokPtr> Reversed(std::vector<TokPtr> v) {
    std::reverse(v.begin(), v.end());
    return v;
}

// decodifica UTF-8 em pontos de codigo (bytes invalidos viram o proprio byte)
std::vector<uint32_t> DecodeUtf8(const std::string &s) {
    std::vector<uint32_t> r;
    for (size_t i = 0; i < s.size();) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        int n = 0;
        uint32_t cp = 0;
        if (c < 0x80) {
            cp = c;
            n = 1;
        } else if ((c & 0xE0) == 0xC0) {
            cp = c & 0x1F;
            n = 2;
        } else if ((c & 0xF0) == 0xE0) {
            cp = c & 0x0F;
            n = 3;
        } else if ((c & 0xF8) == 0xF0) {
            cp = c & 0x07;
            n = 4;
        } else {
            cp = c;
            n = 1;
        }
        if (i + static_cast<size_t>(n) > s.size()) n = 1, cp = c;
        for (int k = 1; k < n; ++k) cp = (cp << 6) | (static_cast<unsigned char>(s[i + static_cast<size_t>(k)]) & 0x3F);
        r.push_back(cp);
        i += static_cast<size_t>(n);
    }
    return r;
}

std::string EncodeUtf8(uint32_t cp) {
    std::string r;
    if (cp < 0x80) r.push_back(static_cast<char>(cp));
    else if (cp < 0x800) {
        r.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        r.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        r.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        r.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        r.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        r.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        r.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        r.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        r.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return r;
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------
// construcao e navegacao
// ---------------------------------------------------------------------------------------------------------------

Parser::Parser(const Cfg &cfg, std::vector<TokPtr> lexed, Description &des, const IncludeVars &iv, Logger &log)
    : cfg_(cfg), log_(log), des_(des), in_(std::move(lexed)) {
    ResetPass();
    des_.hard_short_vars.insert(iv.shorts.begin(), iv.shorts.end());
    for (const TokPtr &t : iv.longs) {
        if (!HasLong(des_.hard_long_vars, t)) des_.hard_long_vars.push_back(t);
    }
    for (const auto &kv : iv.declares.items) des_.declares.Set(kv.first, kv.second);

    // [?](x,y) vira "locate x,y:?" -- o define padrao do modulo MSX
    auto def_rep = TokStrHere("locate VAR:?");
    def_rep[1]->tok = "D_DEFVAR";
    DefineGroup g;
    g.repl = def_rep;
    g.varv = TokStrHere("0,0");
    g.has_var = true;
    des_.defines["?"] = g;
}

void Parser::ResetPass() {
    len_ = in_.size();
    index_ = 0;
    tk_ = in_[0];
    tk_->pos->lin = 0;
    tk_->pos->col = 1;
    out_ = {tk_};
}

TokPtr Parser::NextTok() {
    if (index_ + 1 <= len_) index_ += 1;
    if (index_ >= len_) index_ = len_ - 1;
    tk_ = in_[index_];
    return tk_;
}

TokPtr Parser::PrevTok() {
    if (index_ > 1) index_ -= 1;
    tk_ = in_[index_];
    return tk_;
}

TokPtr Parser::PeekAhead(int offset) const {
    long long i = static_cast<long long>(index_) + offset;
    if (i < 0) i += static_cast<long long>(in_.size());
    i = std::max<long long>(0, std::min<long long>(i, static_cast<long long>(in_.size()) - 1));
    return in_[static_cast<size_t>(i)];
}

TokPtr Parser::LastTok(int offset) const {
    long long i = static_cast<long long>(out_.size()) + offset;
    i = std::max<long long>(0, std::min<long long>(i, static_cast<long long>(out_.size()) - 1));
    return out_[static_cast<size_t>(i)];
}

std::vector<TokPtr> Parser::TokStrHere(const std::string &text, PosPtr pos) {
    if (!pos) {
        pos = tk_->pos->Copy();
        pos->col -= 1;
    }
    return TokStr(des_, text, pos);
}

bool Parser::IsMainFile() const { return out_[0]->pos->file == cfg_.file_load; }

// ---------------------------------------------------------------------------------------------------------------
// defines
// ---------------------------------------------------------------------------------------------------------------

TokPtr Parser::GetInBracket(const std::string &delim, bool &blank) {
    blank = false;
    const std::string dop(1, delim[0]), dcl(1, delim[1]);
    if (tk_->val != dop) log_.Fatal("Expecting: " + dop, tk_);
    TokPtr content = NextTok();
    if (content->val == dcl) {
        blank = true;
        return nullptr;
    }
    if (NextTok()->val != dcl) log_.Fatal("Closing \"" + dcl + "\" not found.", tk_);
    NextTok();
    return content;
}

void Parser::GetDefines() {
    while (true) {
        NextTok();
        bool blank = false;
        TokPtr def_def = GetInBracket("[]", blank);
        if (!def_def) {
            PrevTok();
            log_.Fatal("Define definition blank.", tk_);
        }
        if (!Description::IsIdentf(def_def->val)) log_.Fatal("Invalid define name: " + def_def->val, def_def);

        DefContent rep = GetDefinedContent("[]", 2);
        DefineGroup grp;
        grp.repl = rep.part;
        if (!rep.var.empty()) {
            grp.has_var = true;
            grp.varv = rep.var[0].part;
        }
        if (des_.defines.count(def_def->LVal())) log_.Fatal("Define name duplicated: " + def_def->val, def_def);
        des_.defines[def_def->LVal()] = grp;

        TokPtr tk = NextTok();
        if (tk->val != ",") {
            if (tk->tok != "NEWLINE") log_.Fatal("Expecting: ,", tk_);
            break;
        }
    }
}

Parser::DefContent Parser::GetDefinedContent(const std::string &delim, int recur) {
    const std::string dop(1, delim[0]), dcl(1, delim[1]);
    if (tk_->val != dop) log_.Fatal("Expecting: " + dop, tk_);

    DefContent res;
    std::vector<std::string> balance = {dop};
    while (true) {
        TokPtr tk = NextTok();
        if (tk->tok == "NEWLINE" && !balance.empty()) {
            log_.Fatal("Unbalanced " + dop + dcl, tk_);
        } else if (tk->val == dop) {
            if (static_cast<int>(balance.size()) >= recur) log_.Fatal("Too many opened \"" + dop + "\"", tk_);
            balance.push_back(dop);
            PosPtr pos = tk_->pos;
            res.var.push_back(GetDefinedContent(delim, 1));
            res.part.push_back(std::make_shared<Token>("D_DEFVAR", "VAR", pos));
            PrevTok();
            continue;
        } else if (tk->val == dcl) {
            if (balance.empty()) log_.Fatal("Too many closed \"" + dcl + "\"", tk_);
            balance.pop_back();
            if (balance.empty()) return res;
            continue;
        }
        res.part.push_back(tk);
    }
}

std::vector<TokPtr> Parser::ReplaceDefine() {
    std::vector<TokPtr> define, def_var;
    std::vector<std::string> balance = {"("};

    bool blank = false;
    TokPtr def_def = GetInBracket("[]", blank);
    if (!def_def) log_.Fatal("Define blank.", tk_);

    const std::string lv = def_def->LVal();
    std::string no_snake = lv;
    no_snake.erase(std::remove(no_snake.begin(), no_snake.end(), '_'), no_snake.end());
    if (!IsAlnumAscii(no_snake) && lv != "?") log_.Fatal(def_def->val + " invalid define name.", def_def);
    if (!des_.defines.count(lv)) log_.Fatal(def_def->val + " define not defined.", def_def);

    if (tk_->tok == "C_SYMBOL" && tk_->val == "(") {
        while (true) {
            TokPtr tk = NextTok();
            if (tk->tok == "NEWLINE" && !balance.empty()) {
                log_.Fatal("Missing )", tk);
            } else if (tk->val == "[") { // um define dentro do argumento de outro
                std::vector<TokPtr> inner = ReplaceDefine();
                def_var.insert(def_var.end(), inner.begin(), inner.end());
                continue;
            } else if (tk->val == "(") {
                balance.push_back(tk->val);
            } else if (tk->val == ")") {
                balance.pop_back();
                if (balance.empty()) break;
            }
            def_var.push_back(tk);
        }
        NextTok();
    }

    DefineGroup &grp = des_.defines[lv];
    std::vector<TokPtr> insert_var = grp.varv;
    if (!def_var.empty()) insert_var = def_var;

    for (const TokPtr &d : grp.repl) {
        if (d->tok == "D_DEFVAR") {
            for (const TokPtr &v : insert_var) define.push_back(v->Copy());
            continue;
        }
        d->pos->lin = def_def->pos->lin;
        d->pos->col = def_def->pos->col;
        define.push_back(d->Copy());
    }
    PrevTok();
    return define;
}

// ---------------------------------------------------------------------------------------------------------------
// declares (nomes longos de variaveis)
// ---------------------------------------------------------------------------------------------------------------

void Parser::GetDeclares() {
    const size_t vc = 2;
    while (true) {
        TokPtr dec_long = NextTok();
        const std::string llow = dec_long->LVal();

        if (!Description::IsIdentf(llow)) log_.Fatal("Invalid declared variable: " + dec_long->val, dec_long);
        if (llow.size() == 1) log_.Fatal("Declared variable too short: " + llow, dec_long);
        if (Description::IsReserved(dec_long->UVal())) log_.Fatal("Variable is a reserved keyword: " + llow, dec_long);

        // sem atribuicao (:)
        if (PeekAhead()->val == "," || PeekAhead()->tok == "NEWLINE") {
            for (const auto &kv : des_.declares.items) {
                const std::string val_n_file = kv.first.substr(0, kv.first.find('@'));
                if (Head(llow, vc) == kv.second) log_.Warn("Declared variable conflict: " + llow + " " + val_n_file + ":" + kv.second, dec_long);
                if (dec_long->ValWithFile() == kv.first) {
                    log_.Fatal("Long variable already reserved: " + llow + " " + val_n_file + ":" + kv.second, dec_long);
                }
            }
            for (const TokPtr &var : des_.hard_long_vars) {
                if (Head(llow, vc) == Head(var->VarName(), vc)) log_.Warn("Reserved long variable conflict: " + llow + " " + var->VarName(), dec_long);
            }
            for (const std::string &var : des_.hard_short_vars) {
                if (Head(llow, vc) == var) log_.Warn("Reserved short variable conflict: " + llow + " " + var, dec_long);
            }
            if (llow.size() == vc) {
                des_.hard_short_vars.insert(llow);
            } else if (!HasLong(des_.hard_long_vars, dec_long)) {
                des_.hard_long_vars.push_back(dec_long);
            }
            if (PeekAhead()->tok == "NEWLINE") break;
            if (PeekAhead()->val == ",") {
                NextTok();
                continue;
            }
        }

        NextTok();

        // a variavel curta
        TokPtr dec_short = NextTok();
        const std::string slow = dec_short->LVal();
        if (!IsShortVarName(slow)) log_.Fatal("Invalid declared short variable: " + slow, dec_short);

        for (const auto &kv : des_.declares.items) {
            const std::string val_n_file = kv.first.substr(0, kv.first.find('@'));
            if (dec_long->ValWithFile() == kv.first) {
                log_.Fatal("Long variable already declared: " + llow + ":" + slow + " " + val_n_file + ":" + kv.second, dec_short);
            }
            if (slow == kv.second) {
                log_.Fatal("Short variable already declared: " + llow + ":" + slow + " " + val_n_file + ":" + kv.second, dec_short);
            }
        }
        for (const TokPtr &var : des_.hard_long_vars) {
            if (dec_long->ValWithFile() == var->ValWithFile()) {
                log_.Fatal("Long variable already reserved: " + llow + ":" + slow + " " + var->LVal(), dec_long);
            }
        }
        for (const TokPtr &var : des_.hard_long_vars) {
            if (slow == Head(var->VarName(), vc)) log_.Warn("Reserved long variable conflict: " + llow + ":" + slow + " " + var->VarName(), dec_short);
        }
        for (const std::string &var : des_.hard_short_vars) {
            if (slow == var) log_.Warn("Reserved short variable conflict: " + llow + ":" + slow + " " + var, dec_short);
        }

        des_.declares.Set(dec_long->ValWithFile(), slow);

        TokPtr tk = NextTok();
        if (tk->val != ",") {
            if (tk->tok != "NEWLINE") log_.Fatal("Expecting: ,", tk_);
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------
// labels
// ---------------------------------------------------------------------------------------------------------------

void Parser::GetLabelLines() {
    TokPtr label;
    bool is_none = true, blank = false;

    if (LastTok()->tok == "NEWLINE") { // label de linha: {nome}
        label = GetInBracket("{}", blank);
        is_none = false;
        PrevTok();
    } else if (LastTok(-2)->tok == "NEWLINE") { // label de laco: nome{
        label = LastTok();
        loop_labels_.push_back(label);
        out_.pop_back();
        is_none = false;
    }

    if (is_none) log_.Fatal("Label error.", tk_);
    if (!label) log_.Fatal("Label blank.", tk_);
    if (!Description::IsIdentf(label->val)) log_.Fatal("Invalid label name: " + label->val, label);

    tk_ = std::make_shared<Token>("D_LBLLIN", label->LVal(), tk_->pos);
}

void Parser::GetJumpLabels() {
    bool blank = false;
    TokPtr go = GetInBracket("{}", blank);
    if (!go) log_.Fatal("Label blank.", tk_);
    PrevTok();
    PosPtr tk_pos = PeekAhead(-1)->pos;
    tk_ = std::make_shared<Token>("D_LBLJMP", go->LVal(), tk_pos);
}

void Parser::GetLoopLabelReturn() {
    if (loop_labels_.empty()) log_.Fatal("Loop label close without open.", tk_);
    TokPtr label = loop_labels_.back();
    loop_labels_.pop_back();

    // ":" se o } nao esta' no inicio da linha
    if (LastTok()->tok != "NEWLINE" && LastTok()->tok != "C_INSTSP") {
        auto s = TokStrHere(":");
        out_.insert(out_.end(), s.begin(), s.end());
    }
    auto g = TokStrHere("goto");
    out_.insert(out_.end(), g.begin(), g.end());
    tk_ = std::make_shared<Token>("D_LBLRET", label->LVal(), tk_->pos);
}

void Parser::GetLabelsExit() {
    if (loop_labels_.empty()) log_.Fatal("Exit without a loop label.", tk_);
    auto g = TokStrHere("goto");
    out_.insert(out_.end(), g.begin(), g.end());
    tk_ = std::make_shared<Token>("D_LBLEXT", loop_labels_.back()->LVal(), tk_->pos);
}

// ---------------------------------------------------------------------------------------------------------------
// funcoes
// ---------------------------------------------------------------------------------------------------------------

std::vector<std::vector<TokPtr>> Parser::GetFuncArgs(const TokPtr &func) {
    std::vector<TokPtr> arg_tok;
    std::vector<std::string> balance = {"("};
    std::vector<std::vector<TokPtr>> func_args;

    if (func->tok != "D_FUNCNM") log_.Fatal("Invalid function name: " + func->val, func);

    TokPtr tk = NextTok();
    if (tk->val != "(") log_.Fatal("Missing (", tk_);

    if (PeekAhead()->val == ")") {
        NextTok();
        return {};
    }

    while (true) {
        tk = NextTok();
        if (tk->tok == "NEWLINE" && !balance.empty()) {
            log_.Fatal("Unbalanced ()", tk);
        } else if (tk->val == "(") {
            balance.push_back(tk->val);
        } else if (tk->val == "," && balance.size() == 1) {
            func_args.push_back(arg_tok);
            arg_tok.clear();
            continue;
        } else if (tk->val == ")") {
            balance.pop_back();
            if (balance.empty()) {
                func_args.push_back(arg_tok);
                break;
            }
        }
        arg_tok.push_back(tk);
    }
    return func_args;
}

void Parser::GetFuncDef() {
    TokPtr func = NextTok();
    if (in_func_) log_.Fatal("Already inside a function.", in_func_);
    if (des_.functions.count(func->LVal())) log_.Fatal("Function name duplicated: " + func->val, func);

    auto tmp = GetFuncArgs(func);
    func_def_args_.clear();
    func_def_equl_.clear();

    for (const auto &args : tmp) {
        std::vector<TokPtr> new_equ, new_arg = args;
        if (args.empty()) log_.Fatal("Function definition missing  argument: " + func->val, func);
        if (args[0]->tok != "D_IDNTTP") log_.Fatal("Function definition only takes variables: " + args[0]->val, args[0]);
        if (args.size() > 1) {
            if (args[1]->val != "=") log_.Fatal("Function definition only takes one variable: " + args[1]->val, args[1]);
            new_arg = {args[0]};
            new_equ.assign(args.begin() + 2, args.end());
        }
        func_def_args_.push_back(new_arg);
        func_def_equl_.push_back(new_equ);
    }
    for (const auto &a : func_def_args_) {
        if (a.empty()) log_.Fatal("Empty argument in function definition.", func);
    }

    tk_->pos->col = 1;
    tk_ = std::make_shared<Token>("D_FUNCDF", func->LVal(), tk_->pos->Copy());
    in_func_ = func;
}

void Parser::GetFuncRet() {
    std::vector<TokPtr> ret_tok;
    std::vector<std::vector<TokPtr>> ret_list;
    TokPtr last_ret_tok = tk_;

    if (!in_func_) log_.Fatal("Ret without function.", tk_);

    TokPtr tk = PeekAhead();
    if (tk->val != "'" && tk->tok != "C_INSTSP" && tk->tok != "NEWLINE") {
        while (true) {
            tk = NextTok();
            if (tk->val == ",") {
                ret_list.push_back(ret_tok);
                ret_tok.clear();
                continue;
            }
            if (tk->val == "'" || tk->tok == "C_INSTSP" || tk->tok == "NEWLINE") {
                ret_list.push_back(ret_tok);
                PrevTok();
                break;
            }
            ret_tok.push_back(tk);
        }
    }
    for (const auto &r : ret_list) {
        if (r.empty()) log_.Fatal("Function return missing variable.", last_ret_tok);
    }

    tk_ = TokStrHere("return")[0];
    FuncDef fd;
    fd.args = func_def_args_;
    fd.rets = ret_list;
    fd.equl = func_def_equl_;
    des_.functions[in_func_->LVal()] = fd;
    in_func_ = nullptr;
}

bool Parser::CompareFuncArgs(const std::vector<TokPtr> &a, const std::vector<TokPtr> &b) {
    auto join = [](const std::vector<TokPtr> &v) {
        std::string s;
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) s.push_back(' ');
            s += v[i]->val;
        }
        return s;
    };
    return join(a) == join(b);
}

std::vector<TokPtr> Parser::ReplaceFuncCalls() {
    TokPtr func = tk_;
    TokPtr equ_tok = TokStrHere("=")[0];
    TokPtr ins_tok = TokStrHere(":")[0];
    TokPtr fcl_tok = TokStrHere("gosub")[0];
    TokPtr fun_tok = std::make_shared<Token>("D_FUNCAL", func->LVal(), func->pos);

    if (!des_.functions.count(func->LVal())) log_.Fatal("Function not defined: " + func->val, func);

    std::vector<std::vector<TokPtr>> cal_vars;
    std::vector<TokPtr> var_tok;

    // chamada com variaveis de retorno:  a,b = .func(x)
    if (LastTok()->val == "=") {
        const size_t last_index = index_;
        PrevTok();
        while (true) {
            TokPtr tk = PrevTok();
            if (!out_.empty()) out_.pop_back();
            if (tk->val == ",") {
                cal_vars.push_back(Reversed(var_tok));
                var_tok.clear();
                continue;
            }
            if (tk_->tok == "NEWLINE" || tk_->val == ":" || Description::IsFuncStop(tk_->UVal())) {
                cal_vars.push_back(Reversed(var_tok));
                PrevTok();
                break;
            }
            var_tok.push_back(tk);
        }
        index_ = last_index;
        std::reverse(cal_vars.begin(), cal_vars.end());
    }

    for (const auto &var : cal_vars) {
        if (var.empty()) continue;
        if (var[0]->tok != "D_IDNTTP") log_.Fatal("Function definition only takes variables: " + var[0]->val, var[0]);
        if (var.size() > 1) log_.Fatal("Function definition only takes one variable: " + var[1]->val, var[1]);
    }

    std::vector<std::vector<TokPtr>> cal_args = GetFuncArgs(func);
    const FuncDef fd = des_.functions[func->LVal()];
    const auto &def_args = fd.args;
    const auto &def_rets = fd.rets;
    const auto &cal_equl = fd.equl;

    if (cal_args.size() > def_args.size()) log_.Fatal("Function arguments mismatch.", func);
    if (cal_vars.size() > def_rets.size()) log_.Fatal("Function variable mismatch.", func);

    std::vector<TokPtr> func_call;
    if (cal_args.empty()) cal_args = {{}};

    const size_t na = std::min({def_args.size(), cal_args.size(), cal_equl.size()});
    for (size_t i = 0; i < na; ++i) {
        const auto &d = def_args[i];
        const auto &c = cal_args[i];
        const auto &e = cal_equl[i];
        if (!CompareFuncArgs(d, c)) {
            if (!c.empty()) {
                func_call.push_back(d[0]->Copy());
                func_call.push_back(equ_tok);
                for (const TokPtr &ca : c) func_call.push_back(ca->Copy());
                func_call.push_back(ins_tok);
            } else if (!e.empty()) {
                func_call.push_back(d[0]->Copy());
                func_call.push_back(equ_tok);
                for (const TokPtr &eq : e) func_call.push_back(eq);
                func_call.push_back(ins_tok);
            }
        }
    }

    func_call.push_back(fcl_tok);
    func_call.push_back(fun_tok);

    const size_t nr = std::min(cal_vars.size(), def_rets.size());
    for (size_t i = 0; i < nr; ++i) {
        const auto &c = cal_vars[i];
        const auto &d = def_rets[i];
        if (!CompareFuncArgs(d, c)) {
            if (!c.empty()) {
                func_call.push_back(ins_tok);
                func_call.push_back(c[0]->Copy());
                func_call.push_back(equ_tok);
                for (const TokPtr &de : d) func_call.push_back(de->Copy());
            }
        }
    }
    return func_call;
}

// ---------------------------------------------------------------------------------------------------------------
// toggles, include, cabecalho
// ---------------------------------------------------------------------------------------------------------------

void Parser::GetKeeps() {
    while (true) {
        TokPtr keep = NextTok();
        if (keep->tok == "NEWLINE") break;
        if (keep->tok != "D_TGLREM") log_.Fatal("Invalid keep tag: " + keep->val, keep);
        if (std::find(des_.keeps.begin(), des_.keeps.end(), keep->val) != des_.keeps.end()) log_.Fatal("Keep tag duplicated: " + keep->val, keep);
        des_.keeps.push_back(keep->UVal());
    }
}

void Parser::ToggleLines() {
    auto has = [&](const std::string &k) { return std::find(des_.keeps.begin(), des_.keeps.end(), k) != des_.keeps.end(); };
    if ((has(tk_->UVal()) || has("#ALL")) && !has("#NONE")) return;

    bool tog_block = false;
    TokPtr toggle = tk_;
    if (PeekAhead()->tok == "NEWLINE") tog_block = true;

    while (true) {
        TokPtr last_tok = tk_;
        TokPtr tk = NextTok();
        if (tk->tok == "EOF") {
            log_.Fatal("Toggle rem not closed from " + toggle->val, toggle);
        } else if (tk->tok == "NEWLINE" && !tog_block) {
            break;
        } else if (tk->tok == "D_TGLREM" && tk->val == toggle->val && last_tok->tok == "NEWLINE" && PeekAhead()->tok == "NEWLINE") {
            break;
        }
    }
}

void Parser::IncludeFile() {
    TokPtr include = NextTok();
    std::string include_file = PyStripWs(PyStrip(include->val, "\""));

    if (include->tok != "C_LITQUO" || include_file.empty()) log_.Fatal("Include error: " + include->val, tk_);
    if (!IsAbsPath(include_file)) include_file = JoinPath(cfg_.file_path, include_file);

    CodePtr code = LoadFile(include_file, nullptr, cfg_.o.tab_length, log_, include.get());

    Lexer lex(cfg_, code, log_);
    std::vector<TokPtr> lexed = lex.Lex();

    IncludeVars iv;
    iv.shorts = des_.hard_short_vars;
    iv.longs = des_.hard_long_vars;
    iv.declares = des_.declares;

    Parser par(cfg_, lexed, lex.des, iv, log_);
    IncludeVars res;
    std::vector<TokPtr> parsed = par.Par(res);

    des_.hard_short_vars = res.shorts;
    des_.hard_long_vars = res.longs;
    des_.declares = res.declares;

    // parsed[2:-2]
    if (parsed.size() > 4) out_.insert(out_.end(), parsed.begin() + 2, parsed.end() - 2);
}

void Parser::InsertRemHeader(size_t idx, int /*lin*/, const std::string &text) {
    TokPtr tok_rem = TokStrHere("'", tk_->pos->Copy(std::nullopt, 0))[0];
    TokPtr tok_lit = std::make_shared<Token>("C_LINREM", text, tk_->pos->Copy(std::nullopt, 1));
    TokPtr tok_nl = TokStrHere("\r", tk_->pos->Copy(std::nullopt, static_cast<int>(text.size())))[0];

    in_.insert(in_.begin() + static_cast<long>(idx), tok_rem);
    in_.insert(in_.begin() + static_cast<long>(idx) + 1, tok_lit);
    in_.insert(in_.begin() + static_cast<long>(idx) + 2, tok_nl);
    len_ = in_.size();
}

// ---------------------------------------------------------------------------------------------------------------
// as passadas
// ---------------------------------------------------------------------------------------------------------------

std::vector<TokPtr> Parser::Par(IncludeVars &out) {
    Pass1();
    Pass2();
    Pass3();
    if (IsMainFile()) { // os includes nao passam das tres primeiras
        Pass4();
        Pass5();
    }
    out.shorts = des_.hard_short_vars;
    out.longs = des_.hard_long_vars;
    out.declares = des_.declares;
    return out_;
}

// Passada 1: preparacao, toggles, instrucoes Dignified e substituicao dos defines.
void Parser::Pass1() {
    log_.Log(5, "Pass 1.");
    in_func_ = nullptr;
    out_.push_back(NextTok());

    while (true) {
        NextTok();

        if (tk_->tok == "D_TGLREM" && LastTok()->tok == "NEWLINE") {
            ToggleLines();
            continue;
        } else if (tk_->tok == "D_INSTRC") {
            const bool first = LastTok()->tok == "NEWLINE";
            const std::string d_instr = tk_->UVal();
            if (d_instr == "EXIT") {
            } else if (d_instr == "INCLUDE") {
            } else if (d_instr == "ENDIF") {
                continue;
            } else if (d_instr == "DEFINE" && first) {
                GetDefines();
                continue;
            } else if (d_instr == "DECLARE" && first) {
                GetDeclares();
                continue;
            } else if (d_instr == "KEEP" && first) {
                GetKeeps();
                continue;
            } else if (d_instr == "FUNC" && first) {
                GetFuncDef();
            } else if (d_instr == "RET" && first) {
                GetFuncRet();
            } else {
                log_.Fatal(tk_->val + " must be at the start of a line.", tk_);
            }
        } else if (tk_->tok == "D_SYMBOL" && tk_->val == "[") {
            std::vector<TokPtr> d = ReplaceDefine();
            out_.insert(out_.end(), d.begin(), d.end());
            continue;
        } else {
            // modulo MSX: '_' no meio da linha e' CALL; o identificador depois dele e' o nome da chamada
            if (tk_->tok == "D_INSTSP" && in_[std::min(index_ + 1, len_ - 1)]->tok != "NEWLINE") {
                tk_->tok = "C_CALINS";
            } else if (tk_->tok == "D_IDNTTP" && out_.back()->tok == "C_CALINS") {
                tk_->tok = "C_CALIDF";
            }
        }

        out_.push_back(tk_);
        if (tk_->tok == "EOF") break;
    }
}

// Passada 2: o fluxo do codigo (chamadas de funcao e labels).
void Parser::Pass2() {
    log_.Log(5, "Pass 2.");
    in_ = out_;
    ResetPass();
    loop_labels_.clear();

    while (true) {
        NextTok();

        if (tk_->tok == "D_FUNCNM") {
            std::vector<TokPtr> calls = ReplaceFuncCalls();
            out_.insert(out_.end(), calls.begin(), calls.end());
            continue;
        } else if (tk_->tok == "D_SYMBOL") {
            if (LastTok()->tok != "NEWLINE" && PeekAhead(2)->val == "}") {
                GetJumpLabels();
            } else if (tk_->val == "{") {
                GetLabelLines();
            } else if (tk_->val == "}") {
                GetLoopLabelReturn();
            }
        } else if (tk_->tok == "D_INSTRC" && tk_->UVal() == "EXIT") {
            GetLabelsExit();
        }

        out_.push_back(tk_);
        if (tk_->tok == "EOF") break;
    }

    if (!loop_labels_.empty()) log_.Fatal("Loop label not closed from: " + loop_labels_.back()->val, loop_labels_.back());
    if (in_func_) log_.Fatal("Func without ret: " + in_func_->val, in_func_);
}

// Passada 3: includes, blocos de comentario, linhas em excesso, juncao de linhas e de strings.
void Parser::Pass3() {
    log_.Log(5, "Pass 3.");
    in_ = out_;
    ResetPass();

    while (true) {
        NextTok();
        const long long idx = static_cast<long long>(index_);
        const long long len = static_cast<long long>(len_);

        if (tk_->tok == "D_INSTRC" && tk_->UVal() == "INCLUDE") {
            IncludeFile();
            continue;
        }
        // modulo MSX: as variaveis curtas escritas no codigo
        if (tk_->tok == "D_IDNTTP") GetHardVariable(tk_);

        // blocos de comentario
        if (tk_->tok == "C_O_BREM") {
            NextTok();
            while (tk_->tok != "C_C_BREM") {
                PosPtr pos = tk_->pos->Copy(std::nullopt, static_cast<int>(tk_->val.size()));
                auto r = TokStrHere("'");
                out_.insert(out_.end(), r.begin(), r.end());
                out_.push_back(tk_);
                auto n = TokStrHere("\r", pos);
                out_.insert(out_.end(), n.begin(), n.end());
                NextTok();
                if (tk_->tok == "EOF") break;
            }
            continue;
        }

        if (tk_->tok == "NEWLINE") {
            tk_->SetStr("\r");
            if (LastTok()->tok == "NEWLINE") continue;
        }

        if (tk_->tok == "C_INSTSP" && LastTok()->tok == "C_INSTSP") {
            continue;
        } else if (tk_->tok == "NEWLINE" && LastTok()->tok == "D_INSTSP") {
            out_.pop_back();
            if (idx < len - 3) continue;
        } else if (tk_->tok == "NEWLINE" && LastTok()->tok == "C_INSTSP") {
            if (idx < len - 3) continue;
        } else if (tk_->tok == "C_INSTSP" && LastTok()->tok == "NEWLINE" && idx > 3) {
            out_.pop_back();
        } else if (tk_->tok == "NEWLINE" && LastTok()->tok == "D_LBLLIN") {
            continue;
        } else if (tk_->tok == "NEWLINE" && LastTok()->tok == "D_FUNCDF") {
            continue;
        } else if (tk_->tok == "C_LITQUO" && LastTok()->tok == "C_LITQUO") {
            // strings seguidas na mesma linha
            std::string a = LastTok()->val;
            while (!a.empty() && a.back() == '"') a.pop_back();
            std::string b = tk_->val;
            size_t k = 0;
            while (k < b.size() && b[k] == '"') ++k;
            tk_->SetStr(a + b.substr(k));
            out_.pop_back();
        } else if (tk_->tok == "C_LITQUO" && LastTok()->tok == "NEWLINE" && LastTok(-2)->tok == "C_LITQUO") {
            // strings em linhas seguidas
            std::string a = LastTok(-2)->val;
            while (!a.empty() && a.back() == '"') a.pop_back();
            std::string b = tk_->val;
            size_t k = 0;
            while (k < b.size() && b[k] == '"') ++k;
            tk_->SetStr(a + b.substr(k));
            out_.pop_back();
            out_.pop_back();
        }

        out_.push_back(tk_);
        if (tk_->tok == "EOF") break;
    }
}

// Passada 4: o fluxo vira numeros de linha (labels e funcoes) e o cabecalho entra.
void Parser::Pass4() {
    log_.Log(5, "Pass 4.");
    in_ = out_;
    ResetPass();

    long long line_number = static_cast<long long>(cfg_.o.line_start) - cfg_.o.line_step;
    line_report_.clear();
    label_report_.clear();

    struct LabelInfo {
        std::string label;
        size_t index;
        long long line;
    };
    std::map<std::string, long long> label_lines, llabels_ret, func_defs;
    std::vector<LabelInfo> label_jumps, llabels_exit, func_call;

    if (cfg_.o.rem_header) {
        InsertRemHeader(2, 1, cfg_.header1);
        InsertRemHeader(5, 2, cfg_.header2);
    }

    while (true) {
        NextTok();
        const std::string name_w_file = tk_->val + tk_->pos->file;

        if (tk_->tok == "D_LBLLIN") {
            label_lines[name_w_file] = line_number;
            label_report_[line_number].push_back("<" + tk_->val);
            continue;
        } else if (tk_->tok == "D_LBLJMP" || tk_->tok == "D_LBLRET") {
            label_report_[line_number].push_back(">" + tk_->val);
            if (tk_->tok == "D_LBLRET") llabels_ret[name_w_file] = line_number + cfg_.o.line_step;
            if (tk_->val == "@") { // o label especial que aponta para a propria linha
                tk_->SetInt(line_number);
            } else {
                label_jumps.push_back({name_w_file, out_.size(), line_number});
            }
        } else if (tk_->tok == "D_LBLEXT") {
            label_report_[line_number].push_back("*" + tk_->val);
            llabels_exit.push_back({name_w_file, out_.size(), line_number});
        }

        if (tk_->tok == "D_FUNCDF") {
            func_defs[name_w_file] = line_number;
            label_report_[line_number].push_back("<" + tk_->val);
            continue;
        } else if (tk_->tok == "D_FUNCAL") {
            func_call.push_back({name_w_file, out_.size(), line_number});
            label_report_[line_number].push_back(">" + tk_->val);
        } else if (tk_->tok == "NEWLINE" && static_cast<long long>(index_) < static_cast<long long>(len_) - 1) {
            line_report_.push_back({line_number, tk_->pos->lin, tk_->pos->text, tk_->pos->file});
            line_number += cfg_.o.line_step;
            TokPtr start_tok = PeekAhead();
            const int line = start_tok->pos->lin;

            if (AllDigits(start_tok->val)) log_.Fatal("Line starting with number: " + start_tok->val, start_tok);

            out_.push_back(tk_);
            out_.push_back(Token::MakeInt("C_LINENB", line_number, tk_->pos->Copy(line, 0)));
            continue;
        } else {
            // modulo MSX: troca os nomes longos de variavel pelos curtos
            if (tk_->tok == "D_IDNTTP") tk_ = ProcessVariable(tk_);
        }

        out_.push_back(tk_);
        if (tk_->tok == "EOF") break;
    }

    // os destinos dos desvios viram numeros de linha
    for (const LabelInfo &l : label_jumps) {
        const auto it = label_lines.find(l.label);
        if (it == label_lines.end()) {
            TokPtr error_tok = out_[l.index];
            log_.Fatal("Label does not exist: " + error_tok->val, error_tok);
        }
        out_[l.index]->SetInt(it->second);
    }
    for (const LabelInfo &l : llabels_exit) {
        const auto it = llabels_ret.find(l.label);
        if (it == llabels_ret.end() || it->second > line_number - cfg_.o.line_step) {
            TokPtr error_tok = out_[l.index];
            log_.Fatal("Loop exit past end of program.", error_tok);
        }
        out_[l.index]->SetInt(it->second);
    }
    for (const LabelInfo &l : func_call) {
        const auto it = func_defs.find(l.label);
        if (it != func_defs.end()) out_[l.index]->SetInt(it->second);
    }
}

// Passada 5: as alteracoes do BASIC classico (? / PRINT, THEN / GOTO, ++ e +=, true/false, traducao) e as maiusculas.
void Parser::Pass5() {
    log_.Log(5, "Pass 5.");
    in_ = out_;
    ResetPass();

    while (true) {
        NextTok();

        Ctk ctk = ClcPass5();
        if (ctk.k == K::None) continue;
        if (ctk.k == K::Tok) tk_ = ctk.tk;

        if (cfg_.o.capitalise_all && !tk_->is_int && !IsLitTok(tk_->tok)) tk_->SetStr(tk_->UVal());

        out_.push_back(tk_);
        if (tk_->tok == "EOF") break;
    }
}

Parser::Ctk Parser::ClcPass5() {
    const std::string cp = PyUpper(cfg_.o.convert_print);
    const std::string tg = PyUpper(cfg_.o.strip_then_goto);

    if (tk_->tok == "C_INSTRC" && (tk_->val == "?" || tk_->UVal() == "PRINT") && !cp.empty()) {
        tk_->SetStr(cp == "?" ? "?" : "print");
        return {K::Tok, tk_};
    } else if (tg == "T" && tk_->tok == "C_JUMPIN" && tk_->UVal() == "THEN" && in_[std::min(index_ + 1, len_ - 1)]->UVal() == "GOTO") {
        return {K::None, nullptr};
    } else if (tg == "G" && tk_->tok == "C_JUMPIN" && tk_->UVal() == "GOTO" && (out_.back()->UVal() == "THEN" || out_.back()->UVal() == "ELSE")) {
        return {K::None, nullptr};
    } else if (tk_->tok == "C_E_SYMB") {
        // x++ -> x=x+1 ; x+=5 -> x=x+5
        TokPtr operand_tok = out_.back();
        const std::string operator_val = tk_->val;
        auto eq = TokStrHere("=");
        out_.insert(out_.end(), eq.begin(), eq.end());
        out_.push_back(operand_tok);
        auto op = TokStrHere(std::string(1, operator_val[0]));
        out_.insert(out_.end(), op.begin(), op.end());
        if (static_cast<size_t>(std::count(operator_val.begin(), operator_val.end(), operator_val[0])) == operator_val.size()) {
            auto one = TokStrHere("1");
            out_.insert(out_.end(), one.begin(), one.end());
        }
        return {K::None, nullptr};
    } else if (tk_->tok == "D_OPERAT") {
        if (tk_->UVal() == "FALSE") {
            auto z = TokStrHere("0");
            out_.insert(out_.end(), z.begin(), z.end());
            return {K::None, nullptr};
        } else if (tk_->UVal() == "TRUE") {
            auto z = TokStrHere("-1");
            out_.insert(out_.end(), z.begin(), z.end());
            return {K::None, nullptr};
        }
    } else if (cfg_.o.translate && IsLitTok(tk_->tok)) {
        tk_->SetStr(TransChar(tk_->val, tk_));
        out_.push_back(tk_);
        return {K::None, nullptr};
    }
    return {K::False, nullptr};
}

// ---------------------------------------------------------------------------------------------------------------
// variaveis (modulo MSX)
// ---------------------------------------------------------------------------------------------------------------

void Parser::GetHardVariable(const TokPtr &tk) {
    const size_t vc = 2;
    const std::string vn = tk->VarName();

    if (LastTok()->val == "~") { // ~nome mantem o nome longo
        if (vn.size() <= vc) log_.Fatal("Can't use ~ on a short named variable: " + vn, tk);
        if (des_.declares.Has(tk->ValWithFile())) {
            log_.Fatal("Long variable already declared: " + tk->LVal() + " " + vn + ":" + des_.declares.At(tk->ValWithFile()), tk);
        }
        for (const TokPtr &var : des_.hard_long_vars) {
            const std::string vv = var->VarName();
            if (vn != vv && (vv.rfind(vn, 0) == 0 || vn.rfind(vv, 0) == 0)) log_.Warn("Reserved long variable conflict: " + vn + " " + vv, tk);
        }
        if (!out_.empty()) out_.pop_back();
        if (!HasLong(des_.hard_long_vars, tk)) des_.hard_long_vars.push_back(tk);
        return;
    }

    if (des_.declares.Has(tk->ValWithFile())) return;

    for (const std::string &var : des_.hard_short_vars) {
        if (vn.size() > vc && Head(vn, vc) == var) log_.Warn("Reserved short variable conflict: " + vn + " " + var, tk);
    }
    for (const TokPtr &var : des_.hard_long_vars) {
        const std::string vv = var->VarName();
        bool in_long = false;
        for (const TokPtr &l : des_.hard_long_vars) {
            if (l->VarName() == vn) in_long = true;
        }
        if (vn != vv && (vn == Head(vv, vc) || (in_long && vv.rfind(vn, 0) == 0))) log_.Warn("Reserved long variable conflict: " + vn + " " + vv, tk);
    }
    if (vn.size() <= vc) des_.hard_short_vars.insert(vn);
}

TokPtr Parser::ProcessVariable(const TokPtr &tk) {
    const size_t vc = 2;
    const std::string vn = tk->VarName();
    if (vn.size() <= vc) return tk;

    for (const TokPtr &var : des_.hard_long_vars) {
        if (vn == var->VarName() && tk->pos->file == var->pos->file) return tk;
    }

    if (des_.declares.Has(tk->ValWithFile())) {
        tk->SetStr(des_.declares.At(tk->ValWithFile()) + tk->VarType());
        return tk;
    }

    static const char kChr[] = "abcdefghijklmnopqrstuvwxyz";
    while (var_index_ > 0) {
        var_index_ -= 1;
        const int idx_h = var_index_ / 26;
        const int idx_l = var_index_ % 26;
        const std::string short_var = std::string(1, kChr[idx_h]) + kChr[idx_l];

        bool used_long = false;
        for (const TokPtr &var : des_.hard_long_vars) {
            if (Head(var->VarName(), vc) == short_var) used_long = true;
        }
        // (o original compara `short_var not in d_declares.items()`, que e' sempre verdadeiro: so' os conflitos abaixo contam)
        if (!des_.hard_short_vars.count(short_var) && !used_long) {
            des_.declares.Set(tk->ValWithFile(), short_var);
            tk->SetStr(short_var + tk->VarType());
            return tk;
        }
    }
    log_.Fatal("Too many variables used (max=676): " + tk_->val, tk_);
}

std::string Parser::TransChar(const std::string &text, TokPtr &tk) {
    // c_replacements: o caractere vira DOIS: 0x01 e uma letra (o prefixo dos graficos do MSX)
    static const std::map<uint32_t, uint32_t> rep = [] {
        std::map<uint32_t, uint32_t> m;
        for (size_t i = 0; i + 1 < sizeof kTransReplace / sizeof kTransReplace[0]; i += 2) m[kTransReplace[i]] = kTransReplace[i + 1];
        return m;
    }();
    static const std::map<uint32_t, uint32_t> orig = [] {
        std::map<uint32_t, uint32_t> m;
        for (size_t i = 0; i < sizeof kTransOriginal / sizeof kTransOriginal[0]; ++i) m[kTransOriginal[i]] = static_cast<uint32_t>(0x80 + i);
        return m;
    }();

    std::string out;
    for (uint32_t cp : DecodeUtf8(text)) {
        const auto r = rep.find(cp);
        if (r != rep.end()) {
            out.push_back('');
            out.push_back(static_cast<char>(r->second));
            continue;
        }
        const auto o = orig.find(cp);
        if (o != orig.end()) cp = o->second;
        if (cp <= 255) {
            out.push_back(static_cast<char>(cp));
        } else {
            if (!tk->bad_cp) tk->bad_cp = static_cast<int>(cp);
            out += EncodeUtf8(cp);
        }
    }
    return out;
}

// ---------------------------------------------------------------------------------------------------------------
// geracao do codigo classico
// ---------------------------------------------------------------------------------------------------------------

bool Parser::ClcGenerate(const TokPtr &token, size_t n) {
    // separa o X de um OR para nao virar XOR
    if (token->UVal() == "OR" && !c_line_.empty() && (c_line_.back() == 'X' || c_line_.back() == 'x')) {
        c_line_ += ' ';
    } else if (!token->UVal().empty() && std::string("ABCDEF").find(token->UVal()[0]) != std::string::npos &&
               line_[(n + line_.size() - 1) % line_.size()]->tok == "C_NUMBER") {
        // separa um hexadecimal de uma palavra que comeca por a-f
        c_line_ += ' ';
    }
    return false;
}

Generated Parser::Generate() {
    log_.Log(5, "Generating.");
    in_ = out_;
    ResetPass();
    NextTok();

    line_.clear();
    c_line_.clear();
    Generated gen;

    while (true) {
        NextTok();

        if (cfg_.o.translate && tk_->bad_cp) {
            log_.Fatal("Translate cannot encode character: " + EncodeUtf8(static_cast<uint32_t>(tk_->bad_cp)) + " (" + std::to_string(tk_->bad_cp) + ")", tk_);
        }

        if (tk_->Truthy()) line_.push_back(tk_);

        if (tk_->tok == "NEWLINE") {
            const long long line_number = line_.empty() ? 0 : line_[0]->ival;
            for (size_t n = 0; n < line_.size(); ++n) {
                const TokPtr &token = line_[n];
                ClcGenerate(token, n);

                const std::string uval = token->UVal();
                if (Description::IsReserved(uval) && (c_line_.empty() || c_line_.back() != ' ') &&
                    !Description::IsSymbComp(line_[(n + line_.size() - 1) % line_.size()]->val)) {
                    c_line_ += cfg_.general_spaces;
                }

                c_line_ += token->val;

                if (token->tok == "C_LINENB" ||
                    (Description::IsReserved(uval) && n + 1 < line_.size() && !Description::IsSymbComp(line_[n + 1]->val))) {
                    c_line_ += cfg_.general_spaces;
                }
            }

            if (cfg_.o.label_report) {
                const auto it = label_report_.find(line_number);
                if (it != label_report_.end()) {
                    std::string labels;
                    for (size_t i = 0; i < it->second.size(); ++i) {
                        if (i) labels.push_back(' ');
                        labels += it->second[i];
                    }
                    c_line_ = PyRStripWs(c_line_) + " '" + labels;
                }
            }

            c_line_ = PyRStripWs(c_line_);
            const size_t line_len = c_line_.size();
            if (line_len > 256) log_.Fatal("Line too long: " + std::to_string(line_len) + " characters", tk_);
            gen.c_code.push_back(c_line_ + "\r\n");

            c_line_.clear();
            line_.clear();
        }

        if (tk_->tok == "EOF") break;
    }

    gen.declares = des_.declares;
    gen.line_report = line_report_;
    return gen;
}

} // namespace badig
