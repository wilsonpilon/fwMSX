// Teste do modelo da TUI de menus (src/tui/tui_model): menus, assistentes, navegador de arquivos e linha
// de comando -- ver doc/tui-spec.md. Sem terminal: as teclas entram como eventos abstratos e o teste
// confere as LINHAS DE COMANDO que saem.
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/tui/tui_model.h"

namespace {

namespace fs = std::filesystem;
using tui::Key;
using tui::TuiModel;

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

struct Rig {
    std::vector<std::string> sent;
    TuiModel model;
    explicit Rig(const std::string &dir) : model([this](const std::string &l) { sent.push_back(l); }, dir) {}

    void K(Key::Type t) {
        Key k;
        k.type = t;
        model.HandleKey(k);
    }
    void Type(const std::string &s) {
        for (const char c : s) {
            Key k;
            k.type = Key::Char;
            k.text = std::string(1, c);
            model.HandleKey(k);
        }
    }
    // Abre o menu `menu` (0-based) e escolhe o item pelo ROTULO, navegando com as setas como um usuario
    // (os separadores sao pulados pelo proprio modelo).
    void PickL(int menu, const std::string &label) {
        K(Key::F10);
        for (int i = 0; i < menu; ++i) K(Key::Right);
        for (int guard = 0; guard < 40; ++guard) {
            const auto menus = model.BuildMenus();
            if (menus[static_cast<size_t>(menu)].items[static_cast<size_t>(model.item_index())].label == label) break;
            K(Key::Down);
        }
        K(Key::Enter);
    }
    // Por posicao na lista de itens (separadores contam).
    void Pick(int menu, int item) {
        const auto menus = model.BuildMenus();
        PickL(menu, menus[static_cast<size_t>(menu)].items[static_cast<size_t>(item)].label);
    }
};

std::string TempDir() {
    const char *t = std::getenv("TEMP");
    return (t ? std::string(t) : std::string("/tmp")) + "/fwmsx_tui_test";
}

} // namespace

int main() {
    const std::string dir = TempDir();
    fs::remove_all(dir);
    fs::create_directories(dir + "/jogos/sub");
    for (const char *f : {"/jogos/a.rom", "/jogos/b.ROM", "/jogos/leia.txt", "/jogos/d.dsk", "/jogos/sub/c.rom"}) std::ofstream(dir + f) << "x";

    // ---- estrutura dos menus ----
    {
        Rig r(dir);
        const auto menus = r.model.BuildMenus();
        std::vector<std::string> titles;
        for (const auto &m : menus) titles.push_back(m.title);
        check(titles == std::vector<std::string>({"Emulador", "Arquivo", "Maquina", "Midia", "Ferramentas", "Ajuda"}), "barra de menus: 6 menus");
        const auto spans = r.model.TitleSpans();
        check(spans.size() == 6 && spans[0].first == 0 && spans[1].first == spans[0].second, "TitleSpans: titulos contiguos");
    }

    // ---- F10, setas e comandos diretos ----
    {
        Rig r(dir);
        check(r.model.mode() == TuiModel::Mode::Normal, "comeca no modo normal");
        r.K(Key::F10);
        check(r.model.mode() == TuiModel::Mode::Menu && r.model.open_menu() == 0, "F10 abre o menu Emulador");
        r.K(Key::Right);
        check(r.model.open_menu() == 1, "seta direita vai para o menu Arquivo");
        r.K(Key::Left);
        r.K(Key::Left);
        check(r.model.open_menu() == 5, "seta esquerda no primeiro menu volta ao ultimo (Ajuda)");
        r.K(Key::Escape);
        check(r.model.mode() == TuiModel::Mode::Normal, "Esc fecha o menu");

        r.Pick(0, 2); // Emulador > Iniciar MSX2+
        check(r.sent.size() == 1 && r.sent[0] == "emu start --msx2p", "Emulador > Iniciar MSX2+ -> 'emu start --msx2p'");
        r.Pick(2, 0); // Maquina > Reiniciar
        check(r.sent.back() == "reset", "Maquina > Reiniciar -> 'reset'");
        r.Pick(2, 1);
        check(r.sent.back() == "pause", "Maquina > Pausar -> 'pause'");
        tui::EmuStatus st;
        st.connected = true;
        st.paused = true;
        r.model.SetStatus(st);
        r.Pick(2, 1);
        check(r.sent.back() == "resume", "com o emulador pausado o item vira Retomar -> 'resume'");
        r.Pick(3, 1); // Midia > Retirar cartucho
        check(r.sent.back() == "cart -", "Midia > Retirar cartucho -> 'cart -'");
    }

    // ---- itens separadores sao pulados ----
    {
        Rig r(dir);
        r.K(Key::F10);
        r.K(Key::Right); // Arquivo: ... Slot, Proximo slot, ---, Salvar estado em arquivo
        for (int i = 0; i < 4; ++i) r.K(Key::Down);
        const auto menus = r.model.BuildMenus();
        check(!menus[1].items[static_cast<size_t>(r.model.item_index())].label.empty(), "setas pulam o separador");
    }

    // ---- slot de estado ----
    {
        Rig r(dir);
        r.Pick(1, 0);
        check(r.sent.back() == "state save fwmsx-estado-1.sst", "Arquivo > Salvar estado -> slot 1");
        r.Pick(1, 3); // Proximo slot
        check(r.model.slot() == 2, "Proximo slot -> 2");
        r.Pick(1, 1);
        check(r.sent.back() == "state load fwmsx-estado-2.sst", "Carregar estado usa o slot 2");
        for (int i = 0; i < 8; ++i) r.Pick(1, 3);
        check(r.model.slot() == 1, "slots dao a volta (1..9)");
        r.Pick(1, 2); // anterior
        check(r.model.slot() == 9, "slot anterior de 1 vai para 9");
    }

    // ---- assistente de texto (varios passos) ----
    {
        Rig r(dir);
        r.Pick(0, 3); // Iniciar com opcoes...
        check(r.model.mode() == TuiModel::Mode::Prompt && r.model.prompt_step_count() == 1, "assistente: abre um prompt");
        r.Type("--msx2p --cart x.rom");
        r.K(Key::Backspace);
        r.Type("m");
        r.K(Key::Enter);
        check(r.model.mode() == TuiModel::Mode::Normal && r.sent.back() == "emu start --msx2p --cart x.rom", "assistente: monta o comando -> " + r.sent.back());

        r.sent.clear();
        r.Pick(2, 6); // Ler memoria... (2 passos, com valores padrao)
        check(r.model.prompt_step_count() == 2 && r.model.prompt_value() == "0x0000", "assistente de 2 passos com valor padrao");
        for (int i = 0; i < 6; ++i) r.K(Key::Backspace);
        r.Type("0xC000");
        r.K(Key::Enter);
        check(r.model.mode() == TuiModel::Mode::Prompt && r.model.prompt_step_number() == 2 && r.model.prompt_value() == "16", "segundo passo");
        r.K(Key::Enter);
        check(r.sent.size() == 1 && r.sent[0] == "peek 0xC000 16", "peek montado -> " + (r.sent.empty() ? "" : r.sent[0]));

        r.sent.clear();
        r.Pick(2, 8); // Digitar texto...
        r.Type("print \"oi\"");
        r.K(Key::Enter);
        check(r.sent.size() == 1 && r.sent[0] == "type \"print \\\"oi\\\"\\n\"", "texto digitado escapa aspas e termina com Enter -> " + (r.sent.empty() ? "" : r.sent[0]));

        r.sent.clear();
        r.Pick(0, 3);
        r.Type("abc");
        r.K(Key::Escape);
        check(r.model.mode() == TuiModel::Mode::Normal && r.sent.empty(), "Esc cancela o assistente sem enviar nada");
    }

    // ---- navegador de arquivos ----
    {
        Rig r(dir + "/jogos");
        r.Pick(3, 0); // Midia > Inserir cartucho...
        check(r.model.mode() == TuiModel::Mode::Browser, "passo de arquivo abre o navegador");
        std::vector<std::string> names;
        for (const auto &e : r.model.browser_entries()) names.push_back(e.name);
        check(names == std::vector<std::string>({"..", "sub", "a.rom", "b.ROM"}), "navegador: .., pastas, e so' arquivos .rom/.mx1/.mx2/.bin");
        // desce na subpasta
        r.K(Key::Down);
        r.K(Key::Enter);
        check(r.model.browser_dir().find("sub") != std::string::npos && r.model.browser_entries().size() == 2, "Enter numa pasta entra nela");
        r.K(Key::Backspace);
        check(r.model.browser_dir().find("sub") == std::string::npos, "Backspace sobe uma pasta");
        // escolhe a.rom
        r.K(Key::Down);
        r.K(Key::Down);
        r.K(Key::Enter);
        check(r.model.mode() == TuiModel::Mode::Prompt && r.model.prompt_step_number() == 2 && r.model.prompt_value() == "auto",
              "depois do arquivo vem o passo do mapper (padrao auto)");
        r.K(Key::Enter);
        check(r.sent.size() == 1 && r.sent[0].rfind("cart \"", 0) == 0 && r.sent[0].find("a.rom\" auto") != std::string::npos,
              "cart montado com o caminho entre aspas e o mapper -> " + (r.sent.empty() ? "" : r.sent[0]));

        // digitar o caminho a mao ('t')
        r.sent.clear();
        r.Pick(3, 3); // Inserir disco em A:...
        check(r.model.mode() == TuiModel::Mode::Browser, "disco: navegador");
        Key t;
        t.type = Key::Char;
        t.text = "t";
        r.model.HandleKey(t);
        check(r.model.mode() == TuiModel::Mode::Prompt, "'t' passa a digitar o caminho");
        r.Type("x.dsk");
        r.K(Key::Enter);
        check(r.sent.size() == 1 && r.sent[0].rfind("disk A \"", 0) == 0 && r.sent[0].find("x.dsk\"") != std::string::npos, "disk A montado -> " + (r.sent.empty() ? "" : r.sent[0]));

        r.sent.clear();
        r.Pick(3, 3);
        r.K(Key::Escape);
        check(r.model.mode() == TuiModel::Mode::Normal && r.sent.empty(), "Esc fecha o navegador");
    }

    // ---- varios comandos de uma vez (novo disco e inserir) ----
    {
        Rig r(dir);
        r.Pick(3, 8); // Novo disco e inserir em A:...
        r.K(Key::Enter); // arquivo (padrao novo.dsk)
        r.K(Key::Enter); // formato (padrao ds35)
        check(r.sent.size() == 2 && r.sent[0] == "newdisk \"novo.dsk\" ds35" && r.sent[1] == "disk A \"novo.dsk\"", "novo disco + inserir envia dois comandos");
    }

    // ---- ferramentas ----
    {
        Rig r(dir);
        r.Pick(4, 0);
        r.Type("nemesis");
        r.K(Key::Enter);
        check(r.sent.back() == "romdb search \"nemesis\"", "Ferramentas > Banco de ROMs: buscar -> " + r.sent.back());
        r.Pick(4, 2);
        check(r.sent.back() == "romdb verify", "romdb verify");
    }

    // ---- linha de comando ----
    {
        Rig r(dir);
        r.Type("peek 0 4");
        r.K(Key::Enter);
        check(r.sent.size() == 1 && r.sent[0] == "peek 0 4" && r.model.input().empty(), "Enter envia a linha digitada e limpa a entrada");
        r.K(Key::Up);
        check(r.model.input() == "peek 0 4", "seta cima recupera o historico");
        r.K(Key::Left);
        r.K(Key::Left);
        r.K(Key::Left);
        r.Type("X");
        check(r.model.input() == "peek 0X 4" || r.model.input() == "peek 0 X4" || r.model.input().find('X') != std::string::npos, "edicao no meio da linha (cursor)");
        r.K(Key::Home);
        r.K(Key::Delete);
        check(r.model.input().rfind("eek", 0) == 0, "Home + Delete apaga o primeiro caractere");
        r.K(Key::Down);
        check(r.model.input().empty(), "seta baixo no fim do historico limpa a linha");

        r.Type("emu s");
        r.K(Key::Tab);
        check(r.model.input() == "emu st", "Tab completa o prefixo comum (emu start/status/stop -> 'emu st')");
        for (int i = 0; i < 6; ++i) r.K(Key::Backspace);
        r.Type("rese");
        r.K(Key::Tab);
        check(r.model.input() == "reset", "Tab completa 'reset'");

        r.sent.clear();
        r.K(Key::Enter);
        r.Type("clear");
        r.K(Key::Enter);
        check(r.model.log().empty() && r.sent.size() == 1, "'clear' limpa o historico sem enviar ao emulador");
        r.Type("quit");
        r.K(Key::Enter);
        check(r.model.quit_requested() && r.sent.size() == 1, "'quit' sai da TUI sem enviar nada");
    }

    // ---- Sair, ajuda e rolagem ----
    {
        Rig r(dir);
        r.PickL(1, "Sair da TUI");
        check(r.model.quit_requested(), "Arquivo > Sair da TUI");
        Rig h(dir);
        h.K(Key::F1);
        check(h.model.mode() == TuiModel::Mode::Help, "F1 abre a ajuda");
        h.K(Key::Enter);
        check(h.model.mode() == TuiModel::Mode::Normal, "qualquer tecla fecha a ajuda");
        for (int i = 0; i < 30; ++i) h.model.AddLog("linha " + std::to_string(i));
        h.K(Key::PageUp);
        check(h.model.log_scroll() == 5, "PgUp rola o historico");
        h.model.AddLog("nova");
        check(h.model.log_scroll() == 6, "quem rolou para cima nao e' puxado para baixo por uma linha nova");
        h.K(Key::PageDown);
        h.K(Key::PageDown);
        check(h.model.log_scroll() == 0, "PgDn volta ao fim");
    }

    // ---- mouse ----
    {
        Rig r(dir);
        const auto spans = r.model.TitleSpans();
        r.model.Click(spans[2].first + 1, 0);
        check(r.model.mode() == TuiModel::Mode::Menu && r.model.open_menu() == 2, "clique no titulo abre o menu");
        r.model.Click(spans[2].first + 1, 0);
        check(r.model.mode() == TuiModel::Mode::Normal, "clique de novo no mesmo titulo fecha");
        r.model.Click(spans[2].first + 1, 0);
        r.model.Click(spans[2].first + 2, 2); // primeiro item do menu Maquina = Reiniciar
        check(r.sent.size() == 1 && r.sent[0] == "reset", "clique no item executa o comando -> " + (r.sent.empty() ? "" : r.sent[0]));
        r.model.Click(spans[2].first + 1, 0);
        r.model.Click(70, 15);
        check(r.model.mode() == TuiModel::Mode::Normal, "clique fora fecha o menu");
    }

    fs::remove_all(dir);
    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
