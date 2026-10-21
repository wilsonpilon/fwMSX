// fwMSX -- o comando `project` da ponte de comandos: o .msxproject (SQLite) e as copias soltas. Ver doc/project-spec.md e `man project`.
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>

#include "../project/cpp/project.h"
#include "command.h"

namespace control {

namespace {

namespace fs = std::filesystem;

Result Ok(std::string text = std::string()) { return Result{true, std::move(text)}; }
Result Err(std::string text) { return Result{false, std::move(text)}; }

std::string Join(const std::vector<std::string> &t, size_t from) {
    std::string s;
    for (size_t i = from; i < t.size(); ++i) s += (s.empty() ? "" : " ") + t[i];
    return s;
}

std::string Size(int64_t n) {
    if (n < 10 * 1024) return std::to_string(n) + " B";
    if (n < 10 * 1024 * 1024) return std::to_string(n / 1024) + " KB";
    return std::to_string(n / (1024 * 1024)) + " MB";
}

} // namespace

Result Commander::ProjectCmd(const std::vector<std::string> &t) {
    static const char *kUse =
        "project: use new <arq.msxproject> [nome] | open <arq> | close | info | ls [prefixo] | add <arq> [caminho] | get <caminho> <destino> | rm <caminho> | "
        "set [chave [valor...]] | root [pasta] | status [pasta] | sync [--prune] [pasta] | pull <caminho> | push <caminho> | export <pasta> | import <pasta> <novo.msxproject>";
    if (t.size() < 2) return Err(kUse);
    std::string sub = t[1];
    std::transform(sub.begin(), sub.end(), sub.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::string err;

    const auto need_open = [&]() { return project_ && project_->IsOpen(); };
    const auto root_of = [&](const std::string &given) {
        if (!given.empty()) return given;
        return project_root_.empty() ? project_->DefaultRoot() : project_root_;
    };
    const auto open_msg = [&]() { return "projeto " + project_->Name() + " (" + project_->Path() + "); pasta das copias soltas: " + root_of(""); };

    if (sub == "new") {
        if (t.size() < 3) return Err("project new: use project new <arq.msxproject> [nome]");
        auto p = std::make_unique<prj::Project>();
        if (!p->Create(t[2], t.size() > 3 ? Join(t, 3) : std::string(), err)) return Err("project new: " + err);
        project_ = std::move(p);
        project_root_.clear();
        return Ok("criado " + open_msg());
    }
    if (sub == "open") {
        if (t.size() < 3) return Err("project open: use project open <arq.msxproject>");
        auto p = std::make_unique<prj::Project>();
        if (!p->Open(t[2], err)) return Err("project open: " + err);
        project_ = std::move(p);
        project_root_.clear();
        return Ok("aberto " + open_msg() + "; " + std::to_string(project_->List().size()) + " arquivo(s)");
    }
    if (sub == "import") {
        if (t.size() < 4) return Err("project import: use project import <pasta> <novo.msxproject>");
        auto p = std::make_unique<prj::Project>();
        if (!p->Create(t[3], std::string(), err)) return Err("project import: " + err);
        if (!p->ImportFolder(t[2], err)) {
            const std::string file = t[3];
            p.reset();
            std::error_code ec;
            fs::remove(file, ec); // nao deixa um projeto pela metade
            return Err("project import: " + err);
        }
        project_ = std::move(p);
        project_root_ = fs::absolute(t[2]).string();
        return Ok("importado " + open_msg() + "; " + std::to_string(project_->List().size()) + " arquivo(s)");
    }
    if (sub == "close") {
        if (!need_open()) return Err("project close: nenhum projeto aberto");
        project_.reset();
        project_root_.clear();
        return Ok("projeto fechado");
    }
    if (!need_open()) return Err("project " + sub + ": nenhum projeto aberto (project new / project open)");

    if (sub == "info") {
        int64_t total = 0;
        const auto files = project_->List();
        for (const prj::FileInfo &f : files) total += f.size;
        std::string out = "nome:    " + project_->Name() + "\nid:      " + project_->Id() + "\narquivo: " + project_->Path() + "\npasta:   " + root_of("") + "\n" +
                          std::to_string(files.size()) + " arquivo(s), " + Size(total);
        for (const auto &kv : project_->Config()) out += "\n  " + kv.first + " = " + kv.second;
        return Ok(out);
    }
    if (sub == "ls") {
        std::string out;
        for (const prj::FileInfo &f : project_->List(t.size() > 2 ? t[2] : std::string())) {
            char line[64];
            std::snprintf(line, sizeof(line), "%-10s %9s  ", f.kind.c_str(), Size(f.size).c_str());
            out += (out.empty() ? "" : "\n") + std::string(line) + f.path;
        }
        return Ok(out);
    }
    if (sub == "add") {
        if (t.size() < 3) return Err("project add: use project add <arquivo> [caminho-no-projeto]");
        std::ifstream f(t[2], std::ios::binary);
        if (!f) return Err("project add: nao foi possivel ler " + t[2]);
        const std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        const std::string root = root_of("");
        std::error_code ec;
        std::string path;
        if (t.size() > 3) {
            path = t[3];
        } else {
            const fs::path rel = fs::relative(fs::absolute(t[2]), fs::absolute(root), ec); // dentro da pasta: mantem o caminho; fora: so' o nome
            path = (!ec && !rel.empty() && *rel.begin() != "..") ? rel.generic_string() : fs::path(t[2]).filename().generic_string();
        }
        if (!project_->Save(path, data, root, err)) return Err("project add: " + err);
        return Ok("adicionado " + prj::NormalizePath(path) + " (" + Size(static_cast<int64_t>(data.size())) + ")");
    }
    if (sub == "get") {
        if (t.size() < 4) return Err("project get: use project get <caminho> <destino>");
        std::vector<uint8_t> data;
        if (!project_->Get(t[2], data)) return Err("project get: nao esta' no projeto: " + t[2]);
        std::ofstream f(t[3], std::ios::binary | std::ios::trunc);
        f.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!f) return Err("project get: nao foi possivel gravar " + t[3]);
        return Ok(t[3] + " (" + Size(static_cast<int64_t>(data.size())) + ")");
    }
    if (sub == "rm") {
        if (t.size() < 3) return Err("project rm: use project rm <caminho>");
        const std::string root = root_of("");
        const std::string path = prj::NormalizePath(t[2]);
        if (!project_->Drop(path, root, err)) return Err("project rm: " + err);
        std::error_code ec;
        const bool had_copy = prj::PathOk(path) && fs::remove(fs::path(root) / path, ec); // a copia solta sai junto: senao voltaria no proximo sync
        return Ok("removido " + path + (had_copy ? " (e a copia solta)" : ""));
    }
    if (sub == "set") {
        if (t.size() == 2) {
            std::string out;
            for (const auto &kv : project_->Config()) out += (out.empty() ? "" : "\n") + kv.first + " = " + kv.second;
            return Ok(out);
        }
        if (t[2] == "-d" && t.size() > 3) return project_->DelConfig(t[3]) ? Ok("removida " + t[3]) : Err("project set: nao existe: " + t[3]);
        if (t.size() == 3) {
            const std::string v = project_->GetConfig(t[2], "\x01");
            return v == "\x01" ? Err("project set: nao existe: " + t[2]) : Ok(t[2] + " = " + v);
        }
        if (!project_->SetConfig(t[2], Join(t, 3), err)) return Err("project set: " + err);
        return Ok(t[2] + " = " + Join(t, 3));
    }
    if (sub == "root") {
        if (t.size() > 2) project_root_ = fs::absolute(t[2]).string();
        return Ok("pasta das copias soltas: " + root_of(""));
    }
    if (sub == "status") {
        const std::string root = root_of(t.size() > 2 ? t[2] : std::string());
        std::vector<prj::SyncItem> items;
        if (!project_->Status(root, items, err)) return Err("project status: " + err);
        std::string out;
        int same = 0;
        for (const prj::SyncItem &i : items) {
            if (i.state == prj::State::InSync) {
                ++same;
                continue;
            }
            std::string line = std::string(prj::StateName(i.state)) + ": " + i.path + "  -> " + prj::ActionName(i.suggest);
            if (i.state == prj::State::Conflict) line += " (o mais novo e' " + std::string(i.newer == prj::Action::Pull ? "o projeto" : "a pasta") + ")";
            out += (out.empty() ? "" : "\n") + line;
        }
        out += (out.empty() ? "" : "\n") + std::to_string(same) + " igual(is), " + std::to_string(items.size() - static_cast<size_t>(same)) + " diferente(s) em " + root;
        return Ok(out);
    }
    if (sub == "sync") {
        bool prune = false;
        std::string folder;
        for (size_t i = 2; i < t.size(); ++i) {
            if (t[i] == "--prune") prune = true;
            else folder = t[i];
        }
        const std::string root = root_of(folder);
        prj::SyncSummary s;
        if (!project_->Sync(root, prune, s, err)) return Err("project sync: " + err);
        std::string out = std::to_string(s.pulled) + " projeto->pasta, " + std::to_string(s.pushed) + " pasta->projeto, " + std::to_string(s.same) + " igual(is)";
        if (s.dropped) out += ", " + std::to_string(s.dropped) + " tirado(s) do projeto";
        for (const prj::SyncItem &i : s.deleted) out += "\nsumiu da pasta (use --prune para tirar do projeto, ou project pull para restaurar): " + i.path;
        for (const prj::SyncItem &i : s.conflicts) {
            out += "\nCONFLITO " + i.path + ": os dois mudaram; o mais novo e' " + (i.newer == prj::Action::Pull ? "o projeto" : "a pasta") +
                   " -> project pull " + i.path + " (fica o projeto) ou project push " + i.path + " (fica a pasta)";
        }
        for (const std::string &e : s.errors) out += "\nERRO " + e;
        // conflitos e erros nao sao "falha" do comando: ficam no texto para o usuario decidir
        return s.errors.empty() ? Ok(out) : Err(out);
    }
    if (sub == "pull" || sub == "push") {
        if (t.size() < 3) return Err("project " + sub + ": use project " + sub + " <caminho>");
        const bool ok = sub == "pull" ? project_->Pull(t[2], root_of(""), err) : project_->Push(t[2], root_of(""), err);
        if (!ok) return Err("project " + sub + ": " + err);
        return Ok(sub == "pull" ? "projeto->pasta: " + prj::NormalizePath(t[2]) : "pasta->projeto: " + prj::NormalizePath(t[2]));
    }
    if (sub == "export") {
        if (t.size() < 3) return Err("project export: use project export <pasta>");
        if (!project_->Export(t[2], err)) return Err("project export: " + err);
        project_root_ = fs::absolute(t[2]).string();
        return Ok(std::to_string(project_->List().size()) + " arquivo(s) em " + project_root_ + " (com o manifesto msxproject.txt)");
    }
    return Err(kUse);
}

} // namespace control
