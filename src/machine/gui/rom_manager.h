// fwMSX -- interface do banco de ROMs (menu ROMs e janelas Banco de ROMs e Navegar
// file-hunter). Os downloads rodam numa thread de trabalho, com a propria conexao do
// banco. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/romdb-spec.md, secao 6.
#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../../romdb/net/listing.h"
#include "../../romdb/service.h"
#include "../../romdb/store/romdb.h"

struct GLFWwindow;

namespace machine::gui {

class RomManager {
public:
    explicit RomManager(const std::string &root);
    ~RomManager();
    RomManager(const RomManager &) = delete;
    RomManager &operator=(const RomManager &) = delete;

    // Itens do menu ROMs (chamar dentro de BeginMenu).
    void DrawMenu(GLFWwindow *window);
    // Janelas abertas (chamar a cada quadro).
    void DrawWindows(GLFWwindow *window);

private:
    using Work = std::function<bool(const romdb::RomPaths &, romdb::RomDb &, const romdb::Log &, std::string &)>;

    // Comeca uma tarefa de fundo. Recusa se ja houver uma rodando.
    void StartTask(const std::string &title, Work work);
    void FinishTask();
    void Refresh();  // le os resultados de busca no banco da interface
    void DrawStatus();
    void DrawDatabase(GLFWwindow *window);
    void DrawBrowser();

    romdb::RomPaths paths_;
    romdb::RomDb db_;      // conexao da interface (busca, edicao)
    std::string db_error_;

    // Tarefa de fundo.
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::mutex mutex_;
    std::string task_title_;
    std::vector<std::string> task_log_;
    std::string task_result_;  // "" = ok; texto = erro
    bool task_has_result_ = false;
    std::vector<romdb::ListingEntry> listing_;  // pasta lida pela tarefa da navegacao
    bool listing_ready_ = false;

    // Janelas.
    bool show_status_ = false;
    bool show_db_ = false;
    bool show_browser_ = false;
    char search_[256] = {};
    int category_ = 0;  // indice em kCategories
    std::vector<romdb::RomRecord> results_;
    bool results_dirty_ = true;
    int64_t selected_id_ = 0;
    romdb::RomRecord edit_;  // copia editavel da ROM selecionada
    bool edit_loaded_ = false;
    char vsearch_[256] = {};
    std::vector<romdb::VampierHit> vhits_;
    std::string db_message_;  // ultimo resultado da janela (salvar, excluir, adicionar)

    std::string browser_url_;  // pasta atual do file-hunter
    std::vector<romdb::ListingEntry> browser_entries_;
    bool browser_loaded_ = false;
};

} // namespace machine::gui
