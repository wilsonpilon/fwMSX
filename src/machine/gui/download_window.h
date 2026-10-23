// fwMSX -- janela "Downloads": o File Hunter (navegar e procurar pelo indice) e os utilitarios de sites especificos, item por item.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/download-spec.md.
#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../../download/cpp/download.h"

namespace machine::gui {

// A barra do download em curso (nome, MB baixados / total, porcentagem); nao desenha nada fora de um download. Usada pela janela Downloads e pela de tarefas do banco de ROMs.
void DrawDownloadProgress();

class DownloadWindow {
public:
    DownloadWindow();
    ~DownloadWindow();
    DownloadWindow(const DownloadWindow &) = delete;
    DownloadWindow &operator=(const DownloadWindow &) = delete;

    void Open() { open_ = true; need_load_ = true; }
    void Draw();  // uma vez por quadro

private:
    struct Job {
        std::string title;
        std::string url, dest, install;
        bool overwrite = false;
    };
    struct Row {
        bool folder = false;
        size_t index = 0;      // do indice (arquivo)
        std::string name;      // o que mostrar
    };

    void Start(std::vector<Job> jobs, bool refresh_index = false);
    void Join();
    void Log(const std::string &line);
    void LoadIndexNow();
    void Rebuild();            // refaz a lista da pasta atual ou do resultado da busca
    void DrawFileHunter();
    void DrawUtilities();
    bool Downloaded(const std::string &rel) const;

    dl::Paths paths_;
    bool open_ = false;
    bool need_load_ = false;
    dl::Index index_;
    bool index_loaded_ = false;
    std::string index_error_;
    std::string dir_;                  // pasta atual no indice ("" = raiz)
    char query_[200] = {};
    std::string applied_query_;
    std::vector<Row> rows_;
    bool truncated_ = false;

    std::thread worker_;
    std::atomic<bool> busy_{false};
    std::mutex mutex_;
    std::string status_;               // o que a tarefa de fundo esta' fazendo
    std::vector<std::string> log_;
    bool reload_index_ = false;
};

} // namespace machine::gui
