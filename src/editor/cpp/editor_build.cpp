// fwMSX -- editor de textos: menu Build (fase E6) -- montar o fonte do buffer, mensagens que levam a' linha, executar no MSX e a ajuda da
// palavra sob o cursor. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md.
//
// O editor monta o TEXTO DO BUFFER (nao o arquivo do disco): as mensagens refletem o que esta' na tela, mesmo sem salvar. Quem sabe rodar
// na maquina e quem sabe o manual (`man`) entram por ganchos (SetRunHook/SetHelpHook), para o modelo continuar testavel sem emulador.
#include "editor_ui.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <unordered_set>

extern "C" {
#include "../asm/ed_hash.h"
#include "../c/ed_syntax.h"
}
#include "../../asm/cpp/assembler.h"
#include "../../obi/cpp/obi.h"

namespace fs = std::filesystem;

namespace ed {

namespace {

// Includes e incbin saem da pasta do fonte (o mesmo criterio do `asmrun`). Com um projeto aberto, o banco `.msxproject` vale ANTES do disco: e' a
// fonte da verdade, e a copia da pasta pode estar atrasada (outro PC, Sync ainda nao feito). `proj` devolve false para o que o projeto nao tem.
using ProjectGet = std::function<bool(const std::string &abs_path, std::vector<uint8_t> &data)>;

class DiskFiles : public fwasm::FileProvider {
public:
    DiskFiles(std::string dir, ProjectGet proj) : dir_(std::move(dir)), proj_(std::move(proj)) {}
    bool Read(const std::string &path, std::vector<uint8_t> &data) override {
        if (proj_ && (proj_(dir_ + path, data) || proj_(path, data))) return true;
        return ReadAll(path, data) || ReadAll(dir_ + path, data);
    }

private:
    static bool ReadAll(const std::string &p, std::vector<uint8_t> &data) {
        std::ifstream f(p, std::ios::binary);
        if (!f) return false;
        data.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        return true;
    }
    std::string dir_;
    ProjectGet proj_;
};

// Poe o leitor do projeto no Badig durante uma conversao e tira ao sair (qualquer retorno), para nao vazar para outro uso (badigcli, testes).
struct BadigReaderScope {
    explicit BadigReaderScope(badig::SourceReader r) { badig::SetSourceReader(std::move(r)); }
    ~BadigReaderScope() { badig::SetSourceReader({}); }
};

bool IsBasicFamily(const Lang *l) {
    if (!l) return false;
    const std::string &id = l->Id();
    return id.find("bas") != std::string::npos || id == "dignified"; // o $ de LEFT$ faz parte do nome
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------
// a palavra sob o cursor
// ---------------------------------------------------------------------------------------------------------------

std::string Editor::WordAtCursor() const {
    const std::string &s = lines_[static_cast<size_t>(cur_.line)];
    const int len = static_cast<int>(s.size());
    const int st = ed_word_start(s.data(), len, cur_.col);
    const int end = ed_hl_ident_end(s.data(), len, st, IsBasicFamily(Language()) ? ED_ID_SUFFIX : 0);
    if (end <= st) return {};
    return s.substr(static_cast<size_t>(st), static_cast<size_t>(end - st));
}

// ---------------------------------------------------------------------------------------------------------------
// montar
// ---------------------------------------------------------------------------------------------------------------

// Basic Dignified: converte o texto do buffer (.dmx) em .amx e .bmx pelo clone do badig.py (src/badig). Os erros e avisos do Badig viram as mesmas
// mensagens do montador (F8 / Shift-F8 / Messages...); o erro de um .include abre a aba do arquivo.
badig::SourceReader EditorApp::ProjectSourceReader() const {
    if (!project_.IsOpen()) return {};
    return [this](const std::string &p, std::string &data) {
        const std::string rel = ProjectRel(p);
        std::vector<uint8_t> b;
        if (rel.empty() || !project_.Get(rel, b)) return false;
        data.assign(b.begin(), b.end());
        return true;
    };
}

bool EditorApp::BuildDignified() {
    Editor &e = Ed();
    const std::string path = e.Name();
    const bool unnamed = path.empty() || (path.rfind("noname", 0) == 0 && fs::path(path).extension().empty());

    badig::Options o = badig::LoadDefaults();
    o.source = unnamed ? std::string("noname.dmx") : path;
    const std::string text = e.Text();
    BadigReaderScope scope(ProjectSourceReader());
    const badig::Result r = badig::Convert(o, {}, &text, !unnamed); // sem nome: so' confere (nada e' gravado)

    build_msgs_.clear();
    build_cur_ = -1;
    build_src_ = path;
    build_name_ = o.source;
    basic_map_ = r.line_map;
    int errors = 0, warnings = 0;
    for (const badig::Diag &d : r.diags) {
        BuildMsg b;
        b.file = d.file;
        b.line = d.line;
        b.warning = d.level == 2;
        b.text = d.text;
        if (!d.file.empty() && d.file != o.source) b.path = d.file; // um .include
        (b.warning ? warnings : errors)++;
        build_msgs_.push_back(b);
    }

    if (!r.ok) {
        if (errors == 0) { // falha sem posicao (argumento invalido...): a primeira linha do registro
            BuildMsg b;
            b.text = r.log.empty() ? std::string("conversion failed") : r.log.front();
            build_msgs_.push_back(b);
            errors = 1;
        }
        size_t first = 0;
        while (first < build_msgs_.size() && build_msgs_[first].warning) ++first;
        if (first >= build_msgs_.size()) first = 0;
        BuildGoto(static_cast<int>(first));
        Msg("Convert: " + std::to_string(errors) + " error(s), " + std::to_string(warnings) + " warning(s)  [" + std::to_string(first + 1) + "/" +
            std::to_string(build_msgs_.size()) + "] " + build_msgs_[first].text);
        return false;
    }
    std::string done = "Converted: " + std::to_string(r.line_count) + " lines";
    if (!r.bmx.empty()) done += ", " + std::to_string(r.bmx.size()) + " bytes tokenized";
    if (unnamed) done += " (not saved: the file has no name)";
    else if (!r.bmx_path.empty()) done += " -> " + fs::path(r.bmx_path).filename().string();
    else done += " -> " + fs::path(r.amx_path).filename().string();
    if (warnings) done += ", " + std::to_string(warnings) + " warning(s)";
    Msg(done);
    return true;
}

bool EditorApp::BuildAssemble() {
    Editor &e = Ed();
    if (e.Partial()) {
        Msg("Assemble: the file is only partly in memory (File > Top of file / Load more)");
        return false;
    }
    if (e.Language() && e.Language()->Id() == "dignified") return BuildDignified();
    if (!e.Language() || e.Language()->Id() != "z80asm") {
        Msg("Assemble: this is not a Z80 assembly or Basic Dignified source (View > Language)");
        return false;
    }
    const std::string path = e.Name();
    const std::string name = path.empty() ? std::string("noname.asm") : fs::path(path).filename().string();
    std::string dir;
    if (!path.empty() && fs::path(path).has_parent_path()) dir = fs::path(path).parent_path().string() + "/";

    DiskFiles files(dir, [this](const std::string &p, std::vector<uint8_t> &d) {
        const std::string rel = ProjectRel(p);
        return !rel.empty() && project_.Get(rel, d);
    });
    fwasm::AsmOptions opts;
    const fwasm::AsmResult r = fwasm::Assemble(name, e.Text(), opts, &files);

    build_msgs_.clear();
    build_cur_ = -1;
    build_src_ = path;
    build_name_ = name;
    std::unordered_set<unsigned> seen; // o montador repete a mesma mensagem em varias passadas
    int errors = 0, warnings = 0;
    for (const std::string &m : r.msgs) {
        const unsigned h = ed_hash_ci(reinterpret_cast<const unsigned char *>(m.data()), static_cast<int>(m.size()));
        if (!seen.insert(h).second) continue;
        BuildMsg b;
        int name_end = 0, line = 0, kind = 0;
        if (ed_build_msg(m.data(), static_cast<int>(m.size()), &name_end, &line, &kind)) {
            b.file = m.substr(0, static_cast<size_t>(name_end));
            b.line = line;
            b.warning = kind == 2;
            const size_t colon = m.find(':', static_cast<size_t>(name_end));
            b.text = colon == std::string::npos ? m : m.substr(colon + 1);
        } else {
            b.text = m;
            b.warning = m.rfind("Warning", 0) == 0;
        }
        while (!b.text.empty() && b.text.front() == ' ') b.text.erase(b.text.begin());
        if (b.warning && b.text.rfind("Warning:", 0) == 0) {
            b.text.erase(0, 8);
            while (!b.text.empty() && b.text.front() == ' ') b.text.erase(b.text.begin());
        }
        (b.warning ? warnings : errors)++;
        build_msgs_.push_back(b);
    }

    if (r.rc != 0 || errors > 0) {
        size_t first = 0;
        while (first < build_msgs_.size() && build_msgs_[first].warning) ++first;
        if (first >= build_msgs_.size()) first = 0;
        if (!build_msgs_.empty()) BuildGoto(static_cast<int>(first));
        Msg("Assemble: " + std::to_string(errors) + " error(s), " + std::to_string(warnings) + " warning(s)  [" + std::to_string(first + 1) + "/" +
            std::to_string(build_msgs_.size()) + "] " + (build_msgs_.empty() ? std::string() : build_msgs_[first].text));
        return false;
    }
    size_t bytes = 0;
    std::string kind = r.format.empty() ? "bin" : r.format;
    for (const char *ext : {"rom", "com", "bin", "cas", "z80"}) {
        const auto it = r.outs.find(ext);
        if (it != r.outs.end()) {
            bytes = it->second.size();
            break;
        }
    }
    Msg("Assemble OK: " + kind + ", " + std::to_string(bytes) + " bytes" + (warnings ? ", " + std::to_string(warnings) + " warning(s)" : std::string()));
    return true;
}

bool EditorApp::BuildRun() {
    if (!run_hook_) {
        Msg("Run on MSX: only inside the fwMSX emulator window");
        return false;
    }
    if (!BuildAssemble()) return false; // as mensagens e a linha do erro ja' estao na tela
    Editor &e = Ed();
    if (e.Name().empty() || e.Name().rfind("noname", 0) == 0) {
        Msg("Run on MSX: save the file first (File > Save As)");
        return false;
    }
    if (e.Modified()) { // o `asmrun` le' o arquivo do disco: o que esta' na tela precisa estar gravado
        std::string err;
        if (!e.Save(err, false)) {
            Msg("Run on MSX: " + err);
            return false;
        }
    }
    const std::string res = run_hook_(e.Name());
    if (!res.empty()) {
        Msg(res);
        return false;
    }
    Msg("Running on the MSX: " + fs::path(e.Name()).filename().string());
    if (embedded_) hide_request_ = true; // a janela do editor sai da frente para ver o programa rodar
    std::string ext = fs::path(e.Name()).extension().string();
    for (char &c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    watch_ = (screen_hook_ && !basic_map_.empty() && (ext == ".dmx" || ext == ".bad")) ? 1 : 0;
    watch_start_ = -1;
    watch_line_ = 0;
    return true;
}

// Depois do F5 num .dmx: o BASIC pode parar com erro de execucao. A tela do MSX diz "Syntax error in 130" e o prompt "Ok"; lendo-a de tempos em tempos
// o editor leva o cursor sozinho para a linha do .dmx (o mesmo caminho do Run > BASIC line), sem o usuario digitar o numero. A espera tem prazo e
// o primeiro olhar vem so' depois do boot (a reinicializacao ainda pode estar mostrando a tela anterior); o erro precisa aparecer em duas leituras
// seguidas, para nao pegar a tela no meio de uma troca.
bool EditorApp::RunWatchTick(double now) {
    if (watch_ <= 0 || !screen_hook_) return false;
    if (watch_start_ < 0) {
        watch_start_ = now;
        watch_next_ = now + 2.0;
    }
    if (now - watch_start_ > 180.0) {
        watch_ = 0;
        return false;
    }
    if (now < watch_next_) return false;
    watch_next_ = now + 0.3;
    std::string message;
    const int line = badig::FindBasicError(screen_hook_(), &message);
    if (line <= 0 || line != watch_line_) {
        watch_line_ = line;
        return false;
    }
    watch_ = 0;
    BasicLineGoto(message);
    show_request_ = true;
    return true;
}

// Run > Build disk (F6): a receita (Obifile) monta o .dsk com o .dmx, o .asm e os dados. Qual receita: o proprio arquivo aberto, se for um Obifile;
// senao o do projeto (raiz); senao o da pasta do arquivo aberto. Os fontes vem, nesta ordem, dos buffers abertos (o que esta' na tela, salvo ou nao),
// do banco do projeto e da pasta.
bool EditorApp::BuildDisk() {
    Editor &e = Ed();
    const std::string name = e.Name();
    std::string ext = fs::path(name).extension().string();
    for (char &c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    std::string recipe;
    if (!name.empty() && (fs::path(name).filename() == "Obifile" || ext == ".obi")) recipe = name;
    if (recipe.empty() && project_.IsOpen()) recipe = obi::FindObifile(project_root_);
    if (recipe.empty()) {
        std::error_code ec;
        const std::string where = !name.empty() && fs::path(name).has_parent_path() ? fs::path(name).parent_path().string() : (cwd_.empty() ? std::string(".") : cwd_);
        if (fs::is_directory(where, ec)) recipe = obi::FindObifile(where);
    }
    if (recipe.empty()) {
        Msg("Build disk: no Obifile here (see man obi)");
        return false;
    }
    std::error_code ec;
    recipe = fs::absolute(recipe, ec).lexically_normal().string();

    obi::BuildOptions o;
    o.obifile = recipe;
    o.reader = [this](const std::string &p, std::vector<uint8_t> &data) {
        std::error_code e2;
        for (int w = 0; w < WindowCount(); ++w) { // o buffer aberto vale mais que qualquer copia
            const std::string &n = Win(w).Name();
            if (n.empty() || !(fs::path(n).lexically_normal() == fs::path(p).lexically_normal())) continue;
            const std::string t = Win(w).Text();
            data.assign(t.begin(), t.end());
            return true;
        }
        const std::string rel = ProjectRel(p);
        return project_.IsOpen() && !rel.empty() && project_.Get(rel, data);
    };
    const obi::BuildResult r = obi::Build(o);

    build_msgs_.clear();
    build_cur_ = -1;
    build_src_ = recipe;
    build_name_ = fs::path(recipe).filename().string();
    int errors = 0, warnings = 0;
    for (const obi::Message &m : r.msgs) {
        BuildMsg b;
        b.file = m.file == recipe ? build_name_ : (m.file.empty() ? std::string() : fs::path(m.file).filename().string());
        b.line = m.line;
        b.warning = m.warning;
        b.text = m.text;
        if (!m.file.empty() && m.file != recipe) b.path = m.file;
        (b.warning ? warnings : errors)++;
        build_msgs_.push_back(b);
    }
    if (!r.ok) {
        if (errors == 0) {
            BuildMsg b;
            b.text = "build failed";
            build_msgs_.push_back(b);
            errors = 1;
        }
        size_t first = 0;
        while (first < build_msgs_.size() && build_msgs_[first].warning) ++first;
        if (first >= build_msgs_.size()) first = 0;
        BuildGoto(static_cast<int>(first));
        Msg("Build disk: " + std::to_string(errors) + " error(s), " + std::to_string(warnings) + " warning(s)  [" + std::to_string(first + 1) + "/" +
            std::to_string(build_msgs_.size()) + "] " + build_msgs_[first].text);
        return false;
    }
    Msg("Disk built: " + fs::path(r.dsk).filename().string() + ", " + std::to_string(r.files.size()) + " files, " + std::to_string(r.free_bytes / 1024) +
        " KiB free" + (warnings ? ", " + std::to_string(warnings) + " warning(s)" : std::string()));
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// mensagens
// ---------------------------------------------------------------------------------------------------------------

void EditorApp::BuildGoto(int i) {
    if (i < 0 || i >= static_cast<int>(build_msgs_.size())) return;
    build_cur_ = i;
    const BuildMsg &m = build_msgs_[static_cast<size_t>(i)];
    const std::string tag = "[" + std::to_string(i + 1) + "/" + std::to_string(build_msgs_.size()) + "] " + (m.warning ? "Warning: " : "") + m.text;
    if (m.line <= 0) {
        Msg(tag);
        return;
    }
    const fs::path src(build_src_);
    int target = -1;
    if (m.file.empty() || m.file == build_name_) {
        for (int w = 0; w < WindowCount(); ++w) {
            if (!build_src_.empty() && Win(w).Name() == build_src_) target = w;
        }
        if (target < 0) target = active_;
    } else { // um .include: abre a aba se ainda nao esta' aberta (so' na janela grafica; na TUI as duas janelas sao do usuario)
        const std::string inc = !m.path.empty() ? m.path : (src.has_parent_path() ? src.parent_path() / m.file : fs::path(m.file)).string();
        for (int w = 0; w < WindowCount(); ++w) {
            std::error_code ec;
            if (Win(w).Name() == inc || (!Win(w).Name().empty() && fs::equivalent(Win(w).Name(), inc, ec))) target = w;
        }
        if (target < 0 && gui_mode_) {
            std::string err;
            if (OpenTab(inc, err)) target = active_;
        }
        if (target < 0) {
            Msg(m.file + ":" + std::to_string(m.line) + " " + tag);
            return;
        }
    }
    SetActive(target);
    Editor &e = Ed();
    e.SetCursor({std::max(0, std::min(m.line - 1, e.LineCount() - 1)), 0});
    Msg(tag);
}

// O BASIC rodou e parou com "Syntax error in 130": a tabela de linhas da ultima conversao diz de qual linha do .dmx (ou .include) veio a 130.
// Vira uma mensagem de Build de uma linha so', para o BuildGoto abrir a aba e posicionar o cursor.
void EditorApp::BasicLineGoto(const std::string &text) {
    const int basic = badig::ParseBasicErrorLine(text);
    if (basic <= 0) {
        Msg("BASIC line: no line number in \"" + text + "\"");
        return;
    }
    badig::Result r;
    r.line_map = basic_map_;
    badig::LineMapEntry m;
    if (!badig::MapBasicLine(r, basic, m)) {
        Msg("BASIC line " + std::to_string(basic) + ": the converted program has no such line");
        return;
    }
    if (m.line <= 0) {
        Msg("BASIC line " + std::to_string(basic) + ": generated by the Badig (no source line)");
        return;
    }
    BuildMsg b;
    b.file = m.file;
    b.line = m.line;
    b.text = "BASIC line " + std::to_string(basic) + " -> source line " + std::to_string(m.line);
    if (!m.file.empty() && m.file != build_name_) b.path = m.file;
    build_msgs_.assign(1, b);
    BuildGoto(0);
}

void EditorApp::BuildStep(int dir) {
    if (build_msgs_.empty()) {
        Msg("No messages (Run > Assemble)");
        return;
    }
    BuildGoto(ed_cycle(static_cast<int>(build_msgs_.size()), build_cur_, dir));
}

void EditorApp::BuildErrors() {
    if (build_msgs_.empty()) {
        Msg("No messages (Run > Assemble)");
        return;
    }
    ShowDialog(Dialog::Errors);
}

// ---------------------------------------------------------------------------------------------------------------
// ajuda da palavra
// ---------------------------------------------------------------------------------------------------------------

bool EditorApp::HelpOnWord() {
    if (!help_hook_) {
        Msg("Help: the manual is not attached");
        return false;
    }
    const std::string word = Ed().WordAtCursor();
    if (word.empty()) {
        Msg("Help: put the cursor on a word");
        return false;
    }
    const std::string text = help_hook_(word);
    if (text.empty()) {
        Msg("No manual page for '" + word + "'");
        return false;
    }
    BuildReference(word, text);
    return true;
}

} // namespace ed
