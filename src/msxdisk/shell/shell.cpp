//
// msxdisk (fwMSX): shell interativo (REPL) -- Fase 3.
//
// Fase 3a: loop basico com replxx, tudo delegado ao Dispatch one-shot.
// Fase 3b: sessao com imagem carregada em memoria ("load"/"save"/
// "saveas", como um cliente FTP -- comandos de imagem passam a operar
// direto na sessao, sem repetir o caminho do .dsk) e comandos locais de
// filesystem (ls/cd/md/rm/pwd) pra navegar o host e decidir o que
// adicionar.
//

#include "shell.h"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <replxx.hxx>

#include "../cli/app.h"
#include "../gui/app.h"
#include "../tui/app.h"
#include "local_fs.h"
#include "session.h"

namespace msxdisk::shell {

namespace {

// Tokeniza uma linha respeitando aspas simples/duplas (para caminhos com
// espaco, ex.: add "meu arquivo.txt"). Nao suporta escape de aspas dentro
// de aspas (fora do escopo da Fase 3).
std::vector<std::string> Tokenize(const std::string &line) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;
    char quote_char = '"';

    for (char c : line) {
        if (in_quotes) {
            if (c == quote_char) {
                in_quotes = false;
            } else {
                current += c;
            }
            continue;
        }
        if (c == '"' || c == '\'') {
            in_quotes = true;
            quote_char = c;
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

// Caminho do arquivo de historico do replxx: HOME/.msxdisk_history (ou
// USERPROFILE no Windows), pra persistir entre sessoes independente do
// diretorio atual.
std::string HistoryFilePath() {
    const char *home =
#if defined(_WIN32)
        std::getenv("USERPROFILE");
#else
        std::getenv("HOME");
#endif
    if (home == nullptr || *home == '\0') return ".msxdisk_history";
    return std::string(home) + "/.msxdisk_history";
}

// Comandos de imagem que, com uma sessao carregada, passam a operar nela
// direto (sem o argumento de caminho da imagem); sem sessao carregada,
// caem no modo CLI one-shot de sempre (Dispatch).
bool IsSessionImageCommand(const std::string &cmd) {
    static const std::set<std::string> kCommands = {"list", "dir",  "add",   "extract", "delete",
                                                       "rename", "ren", "mkdir", "rmdir",   "info"};
    return kCommands.count(cmd) > 0;
}

std::string BuildPrompt(const Session &session) {
    if (!session.HasImage()) return "msxdisk> ";
    const std::filesystem::path p(session.ImagePath());
    return "msxdisk [" + p.filename().string() + (session.IsDirty() ? "*" : "") + "]> ";
}

void PrintUsage(const char *command, const char *usage) {
    std::cerr << command << ": uso: " << usage << std::endl;
}

bool HasWildcardChars(const std::string &s) {
    return s.find('*') != std::string::npos || s.find('?') != std::string::npos;
}

// Comandos de imagem operando direto na sessao (parsing posicional +
// flags simples, espelhando as opcoes do CLI11 equivalente).
void DispatchSessionCommand(Session &session, const std::string &cmd, const std::vector<std::string> &tokens) {
    if (cmd == "list" || cmd == "dir") {
        std::string dir_path, pattern;
        bool tree = false;
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i] == "--dir" && i + 1 < tokens.size()) {
                dir_path = tokens[++i];
            } else if (tokens[i] == "-r" || tokens[i] == "--tree") {
                tree = true;
            } else if (pattern.empty()) {
                pattern = tokens[i];
            }
        }
        session.CmdList(dir_path, pattern, tree);
    } else if (cmd == "add") {
        std::string host_path, msx_path;
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i] == "--as" && i + 1 < tokens.size()) {
                msx_path = tokens[++i];
            } else if (host_path.empty()) {
                host_path = tokens[i];
            }
        }
        if (host_path.empty()) {
            PrintUsage("add", "add <arquivo_local> [--as <caminho_msx>]");
        } else {
            session.CmdAdd(host_path, msx_path);
        }
    } else if (cmd == "extract") {
        std::string pattern, dir_path, dest = ".";
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i] == "--dir" && i + 1 < tokens.size()) {
                dir_path = tokens[++i];
            } else if ((tokens[i] == "-d" || tokens[i] == "--dest") && i + 1 < tokens.size()) {
                dest = tokens[++i];
            } else if (pattern.empty()) {
                pattern = tokens[i];
            }
        }
        if (pattern.empty()) {
            PrintUsage("extract", "extract <padrao> [--dir <subdir>] [-d <destino>]");
        } else {
            session.CmdExtract(dir_path, pattern, dest);
        }
    } else if (cmd == "delete") {
        if (tokens.size() < 2) {
            PrintUsage("delete", "delete <arquivo>");
        } else {
            session.CmdDelete(tokens[1]);
        }
    } else if (cmd == "rename" || cmd == "ren") {
        if (tokens.size() < 3) {
            PrintUsage("rename", "rename <arquivo> <novo_nome>");
        } else {
            session.CmdRename(tokens[1], tokens[2]);
        }
    } else if (cmd == "mkdir") {
        if (tokens.size() < 2) {
            PrintUsage("mkdir", "mkdir <diretorio>");
        } else {
            session.CmdMkdir(tokens[1]);
        }
    } else if (cmd == "rmdir") {
        if (tokens.size() < 2) {
            PrintUsage("rmdir", "rmdir <diretorio>");
        } else {
            session.CmdRmdir(tokens[1]);
        }
    } else if (cmd == "info") {
        session.CmdInfo();
    }
}

void PrintLocalHelp() {
    std::cout << std::endl;
    std::cout << "Modo:" << std::endl;
    std::cout << "  tui / call tui     abre a interface em modo texto (TUI), sem sair do programa" << std::endl;
    std::cout << "  gui / call gui     abre a interface grafica (GUI), sem sair do programa" << std::endl;
    std::cout << std::endl;
    std::cout << "Comandos locais (sistema de arquivos do host, como o lado 'local' de um FTP):" << std::endl;
    std::cout << "  ls [padrao]        lista o diretorio atual (aceita coringas: ls *.bas)" << std::endl;
    std::cout << "  cd <caminho>       muda de diretorio (cd .., cd D: no Windows)" << std::endl;
    std::cout << "  md <caminho>       cria um diretorio local" << std::endl;
    std::cout << "  rm <arquivo>       remove um arquivo local (nunca diretorios)" << std::endl;
    std::cout << "  pwd                mostra o diretorio atual" << std::endl;
    std::cout << std::endl;
    std::cout << "Sessao de imagem (como abrir uma conexao FTP):" << std::endl;
    std::cout << "  load <imagem.dsk>  carrega a imagem em memoria; comandos abaixo passam a" << std::endl;
    std::cout << "                     operar nela sem repetir o caminho" << std::endl;
    std::cout << "  save               grava a sessao carregada de volta no mesmo arquivo" << std::endl;
    std::cout << "  saveas <destino>   grava a sessao carregada num novo arquivo" << std::endl;
    std::cout << "  (create tambem carrega a imagem recem-criada na sessao automaticamente)" << std::endl;
    std::cout << std::endl;
    std::cout << "Transferencia estilo FTP (com sessao carregada):" << std::endl;
    std::cout << "  put <local> [nome_msx]     envia 1 arquivo local (sem coringa)" << std::endl;
    std::cout << "  get <msx> [destino_local]  recebe 1 arquivo da imagem (sem coringa)" << std::endl;
    std::cout << "  mput <padrao_local>        envia varios (coringa: mput *.bas)" << std::endl;
    std::cout << "  mget <padrao_msx>          recebe varios (coringa: mget *.bin)" << std::endl;
    std::cout << "  prompt                     liga/desliga a confirmacao arquivo a arquivo" << std::endl;
    std::cout << "                             do mput/mget (default: ligada)" << std::endl;
}

} // namespace

int RunShell() {
    replxx::Replxx rx;
    const std::string history_path = HistoryFilePath();
    rx.history_load(history_path);

    Session session;
    bool prompt_enabled = true;

    std::cout << "msxdisk - shell interativo." << std::endl;
    std::cout << "Comandos de imagem: create, load, save, saveas, list/dir, add, extract, delete,"
              << std::endl;
    std::cout << "                    rename, mkdir, rmdir, info, copy, put, get, mput, mget, prompt."
              << std::endl;
    std::cout << "Comandos locais (filesystem, como um FTP): ls, cd, md, rm, pwd." << std::endl;
    std::cout << "'tui'/'call tui' abre a TUI, 'gui'/'call gui' abre a GUI, sem sair do programa."
              << std::endl;
    std::cout << "Digite 'help' para a lista completa, 'exit' ou Ctrl-D para sair." << std::endl;

    while (true) {
        const std::string prompt = BuildPrompt(session);
        const char *raw_line = rx.input(prompt.c_str());
        if (raw_line == nullptr) {
            std::cout << std::endl;
            break;
        }

        const std::string line(raw_line);
        if (line.empty()) continue;
        rx.history_add(line);

        const auto tokens = Tokenize(line);
        if (tokens.empty()) continue;
        const std::string &cmd = tokens[0];

        if (cmd == "exit" || cmd == "quit") break;

        if (cmd == "help" || cmd == "?") {
            cli::Dispatch({"--help"});
            PrintLocalHelp();
            continue;
        }

        // Troca pro modo TUI sem sair do processo (mesmo executavel --
        // pedido do autor). "call tui" existe como sinonimo, no espirito
        // do CALL do MSX-BASIC. Se ja houver uma imagem carregada na
        // sessao, a TUI abre com ela; ao voltar, a sessao recarrega do
        // disco pra pegar o que a TUI tiver salvo (Fase 4c).
        if (cmd == "tui" || (cmd == "call" && tokens.size() > 1 && tokens[1] == "tui")) {
            const std::string image_arg = session.HasImage() ? session.ImagePath() : std::string();
            msxdisk::tui::LaunchTui(image_arg);
            if (session.HasImage()) session.CmdLoad(session.ImagePath());
            continue;
        }
        if (cmd == "gui" || (cmd == "call" && tokens.size() > 1 && tokens[1] == "gui")) {
            const std::string image_arg = session.HasImage() ? session.ImagePath() : std::string();
            msxdisk::gui::LaunchGui(image_arg);
            if (session.HasImage()) session.CmdLoad(session.ImagePath());
            continue;
        }

        // Comandos locais (filesystem do host) -- sempre disponiveis,
        // independente de haver uma imagem carregada.
        if (cmd == "ls") {
            CmdLocalList(tokens.size() > 1 ? tokens[1] : "");
            continue;
        }
        if (cmd == "cd") {
            if (tokens.size() < 2) {
                PrintUsage("cd", "cd <diretorio>");
            } else {
                CmdLocalChangeDir(tokens[1]);
            }
            continue;
        }
        if (cmd == "md") {
            if (tokens.size() < 2) {
                PrintUsage("md", "md <diretorio>");
            } else {
                CmdLocalMakeDir(tokens[1]);
            }
            continue;
        }
        if (cmd == "rm") {
            if (tokens.size() < 2) {
                PrintUsage("rm", "rm <arquivo>");
            } else {
                CmdLocalRemoveFile(tokens[1]);
            }
            continue;
        }
        if (cmd == "pwd") {
            CmdLocalPrintWorkingDir();
            continue;
        }

        // Sessao (load/save/saveas) -- nunca vao pelo Dispatch one-shot.
        if (cmd == "load") {
            if (tokens.size() < 2) {
                PrintUsage("load", "load <imagem.dsk>");
            } else {
                session.CmdLoad(tokens[1]);
            }
            continue;
        }
        if (cmd == "save") {
            session.CmdSave();
            continue;
        }
        if (cmd == "saveas" && session.HasImage()) {
            if (tokens.size() < 2) {
                PrintUsage("saveas", "saveas <novo_caminho.dsk>");
            } else {
                session.CmdSaveAs(tokens[1]);
            }
            continue;
        }

        // Transferencia estilo FTP -- sempre tratados aqui (nunca via
        // Dispatch, nao existem no modo one-shot); a propria Session
        // reporta "nenhuma imagem carregada" se for o caso.
        if (cmd == "prompt") {
            prompt_enabled = !prompt_enabled;
            std::cout << "Confirmacao interativa (mput/mget): " << (prompt_enabled ? "ligada" : "desligada")
                       << std::endl;
            continue;
        }
        if (cmd == "put") {
            if (tokens.size() < 2) {
                PrintUsage("put", "put <arquivo_local> [nome_msx]");
            } else if (HasWildcardChars(tokens[1])) {
                std::cerr << "put: '" << tokens[1] << "' tem coringa -- use 'mput' para varios arquivos"
                           << std::endl;
            } else {
                session.CmdAdd(tokens[1], tokens.size() > 2 ? tokens[2] : "");
            }
            continue;
        }
        if (cmd == "get") {
            if (tokens.size() < 2) {
                PrintUsage("get", "get <caminho_msx> [destino_local]");
            } else if (HasWildcardChars(tokens[1])) {
                std::cerr << "get: '" << tokens[1] << "' tem coringa -- use 'mget' para varios arquivos"
                           << std::endl;
            } else {
                session.CmdGet(tokens[1], tokens.size() > 2 ? tokens[2] : "");
            }
            continue;
        }
        if (cmd == "mput") {
            if (tokens.size() < 2) {
                PrintUsage("mput", "mput <padrao_local> (ex.: mput *.bas)");
            } else {
                session.CmdMput(tokens[1], prompt_enabled);
            }
            continue;
        }
        if (cmd == "mget") {
            if (tokens.size() < 2) {
                PrintUsage("mget", "mget <padrao_msx> [--dir <subdir>] [-d <destino_local>]");
            } else {
                std::string pattern, dir_path, dest = ".";
                for (size_t i = 1; i < tokens.size(); ++i) {
                    if (tokens[i] == "--dir" && i + 1 < tokens.size()) {
                        dir_path = tokens[++i];
                    } else if ((tokens[i] == "-d" || tokens[i] == "--dest") && i + 1 < tokens.size()) {
                        dest = tokens[++i];
                    } else if (pattern.empty()) {
                        pattern = tokens[i];
                    }
                }
                session.CmdMget(dir_path, pattern, dest, prompt_enabled);
            }
            continue;
        }

        // Com sessao carregada, os comandos de imagem operam nela direto.
        if (session.HasImage() && IsSessionImageCommand(cmd)) {
            DispatchSessionCommand(session, cmd, tokens);
            continue;
        }

        // 'create' sempre via Dispatch (preserva --dos1/--dos2/etc.); se
        // der certo, carrega a imagem recem-criada na sessao (como um
        // 'load' automatico), pra ja poder trabalhar nela direto.
        if (cmd == "create") {
            const int rc = cli::Dispatch(tokens);
            if (rc == 0 && tokens.size() >= 2) {
                session.CmdLoad(tokens[1]);
            }
            continue;
        }

        // Tudo o resto ('copy', comandos de imagem sem sessao carregada,
        // ou um comando desconhecido) cai no modo CLI one-shot de sempre.
        cli::Dispatch(tokens);
    }

    rx.history_save(history_path);
    std::cout << "Ate mais!" << std::endl;
    return 0;
}

} // namespace msxdisk::shell
