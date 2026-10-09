// fwMSX -- TUI do emulador. Ver tui_app.h e doc/tui-spec.md.
#include "tui_app.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <deque>
#include <filesystem>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>

#include "../audio/audio_output.h"
#include "../common/version.h"
#include "../control/command.h"
#include "../control/server.h"
#include "../diskfmt/cpp/disk_creator.h"
#include "../machine/screenshot.h"
#include "../psg/core/psg_state.h"
#include "../psg/cpp/psg_device.h"
#include "../vdp/core/vdp_state.h"

namespace tui {

namespace {

using namespace ftxui;
using machine::FrameSize;
using machine::Machine;
using machine::MachineConfig;

// Tudo o que a interface precisa para desenhar um quadro, copiado pela thread da maquina.
struct Snapshot {
    std::vector<uint32_t> rgba; // R,G,B,A (uint32 little-endian), fs.width x fs.height
    FrameSize fs;
    bool text = false;          // SCREEN 0/1/13: desenha os CARACTERES de verdade
    int cols = 40;
    int rows = 24;
    std::vector<std::string> text_rows;
    uint8_t fg[3] = {255, 255, 255};
    uint8_t bg[3] = {0, 0, 128};
    int scr_mode = 0;
    uint64_t frame = 0;
    bool paused = false;
    double fps = 0.0;
    bool valid = false;
};

// Quanto tempo (em quadros) uma tecla do terminal vale como "pressionada". Terminais nao mandam o
// evento de soltar; o auto-repeat do teclado renova o prazo enquanto a tecla esta' segura.
constexpr int kKeyHoldFrames = 6;

class Runner : public control::Host {
public:
    explicit Runner(const TuiOptions &options) : opts_(options), commander_(*this), audio_(psg::kSampleRate) {}
    ~Runner() override { Stop(); }

    bool Start(std::string &error) {
        config_ = opts_.machine;
        machine_ = Machine::Create(config_, error);
        if (!machine_) return false;
        if (opts_.audio) {
            std::string audio_error;
            audio_ok_ = audio_.Start(audio_error);
            if (audio_ok_) machine_->EnableLiveAudio(true);
        }
        if (opts_.ctl_port >= 0) {
            std::string ctl_error;
            if (server_.Start(opts_.ctl_port, ctl_error)) {
                std::vector<std::string> paths = {"fwmsx.port"};
                if (!opts_.exe_dir.empty()) paths.push_back((std::filesystem::path(opts_.exe_dir) / "fwmsx.port").string());
                for (const std::string &p : paths) {
                    if (std::FILE *f = std::fopen(p.c_str(), "w")) {
                        std::fprintf(f, "%d\n", server_.port());
                        std::fclose(f);
                        port_files_.push_back(p);
                    }
                }
            } else {
                ctl_message_ = ctl_error;
            }
        }
        running_ = true;
        thread_ = std::thread(&Runner::Loop, this);
        return true;
    }

    void Stop() {
        if (running_.exchange(false)) {
            if (thread_.joinable()) thread_.join();
        }
        server_.Stop();
        if (audio_ok_) {
            audio_.Stop();
            audio_ok_ = false;
        }
        for (const std::string &p : port_files_) std::remove(p.c_str());
        port_files_.clear();
        // Quem ainda esperava uma resposta nao fica pendurado.
        std::lock_guard<std::mutex> lock(jobs_mutex_);
        for (Job &j : jobs_) j.reply.set_value("err emulador encerrado");
        jobs_.clear();
    }

    // ---- chamadas da thread da interface -------------------------------------------------------

    // Executa um comando na thread da maquina e devolve a linha de resposta ("ok ..." / "err ...").
    std::string Execute(const std::string &line) {
        Job job;
        job.line = line;
        std::future<std::string> fut = job.reply.get_future();
        {
            std::lock_guard<std::mutex> lock(jobs_mutex_);
            if (!running_) return "err emulador encerrado";
            jobs_.push_back(std::move(job));
        }
        return fut.get();
    }

    // Tecla do PC ja' traduzida para o nome da tecla do MSX (e se precisa de SHIFT).
    void PressKey(const std::string &key, bool shift) {
        std::lock_guard<std::mutex> lock(keys_mutex_);
        pending_keys_.push_back({key, shift});
    }

    Snapshot Snap() {
        std::lock_guard<std::mutex> lock(snap_mutex_);
        return snapshot_;
    }

    bool quit_requested() const { return quit_; }
    bool audio_ok() const { return audio_ok_; }
    int ctl_port() { return server_.port(); }
    const std::string &ctl_message() const { return ctl_message_; }

    // ---- control::Host (so' chamados na thread da maquina) --------------------------------------
    Machine &machine() override { return *machine_; }
    bool paused() const override { return paused_; }
    void set_paused(bool p) override { paused_ = p; }
    void quit() override { quit_ = true; }
    bool LoadCartridge(const std::string &path, const std::string &mapper_name, std::string &error) override {
        MemMapMapperType kind = MEMMAP_MAPPER_NONE;
        if (!machine::ParseMapperName(mapper_name, kind)) {
            error = "mapper desconhecido '" + mapper_name + "'";
            return false;
        }
        MachineConfig next = config_;
        machine::SetCartridge(next, path, kind);
        std::string sram_error;
        machine_->SaveSram(sram_error);
        std::unique_ptr<Machine> fresh = Machine::Create(next, error);
        if (!fresh) return false;
        machine_ = std::move(fresh);
        config_ = next;
        machine_->EnableLiveAudio(audio_ok_);
        return true;
    }
    bool SaveScreenshot(const std::string &path, std::string &error) override {
        return !machine::SaveScreenshotPng(*machine_, error, path).empty();
    }

private:
    struct Job {
        std::string line;
        std::promise<std::string> reply;
    };
    struct PendingKey {
        std::string key;
        bool shift;
    };
    struct Held {
        int frames;
    };

    void Press(const std::string &key) {
        const auto it = held_.find(key);
        if (it == held_.end()) {
            machine_->KeyDown(key);
            held_[key] = Held{kKeyHoldFrames};
        } else {
            it->second.frames = kKeyHoldFrames;
        }
    }

    void ApplyPendingKeys() {
        std::vector<PendingKey> keys;
        {
            std::lock_guard<std::mutex> lock(keys_mutex_);
            keys.swap(pending_keys_);
        }
        for (const PendingKey &k : keys) {
            if (k.shift) Press("shift");
            Press(k.key);
        }
    }

    void AgeKeys() {
        for (auto it = held_.begin(); it != held_.end();) {
            if (--it->second.frames <= 0) {
                machine_->KeyUp(it->first);
                it = held_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Joystick da porta A a partir das teclas seguradas (mesmo mapeamento da janela).
    void UpdateJoystick() {
        uint8_t bits = 0;
        auto down = [&](const char *k) { return held_.count(k) != 0; };
        if (down("up")) bits |= PSG_JOY_UP;
        if (down("down")) bits |= PSG_JOY_DOWN;
        if (down("left")) bits |= PSG_JOY_LEFT;
        if (down("right")) bits |= PSG_JOY_RIGHT;
        if (down("space") || down("z")) bits |= PSG_JOY_FIRE_A;
        if (down("x")) bits |= PSG_JOY_FIRE_B;
        machine_->SetJoystick(0, bits);
    }

    void DrainJobs() {
        std::deque<Job> work;
        {
            std::lock_guard<std::mutex> lock(jobs_mutex_);
            work.swap(jobs_);
        }
        for (Job &j : work) j.reply.set_value(commander_.Execute(j.line).Line());
    }

    void TakeSnapshot(double fps) {
        Snapshot s;
        s.fs = machine_->RenderFrame(s.rgba);
        const VdpState &vs = machine_->vdp_state();
        s.scr_mode = vs.scr_mode;
        s.frame = machine_->frame_count();
        s.paused = paused_;
        s.fps = fps;
        s.valid = true;
        if (vs.scr_mode == 0 || vs.scr_mode == 1 || vs.scr_mode == 13) {
            s.text = true;
            s.cols = vs.scr_mode == 1 ? 32 : (vs.scr_mode == 13 ? 80 : 40);
            s.rows = 24;
            for (int row = 0; row < s.rows; ++row) {
                std::string line;
                for (int col = 0; col < s.cols; ++col) {
                    const uint8_t c = vs.vram[(vs.chr_tab + static_cast<uint32_t>(row * s.cols + col)) & (VDP_VRAM_SIZE - 1)];
                    line += (c >= 0x20 && c < 0x7F) ? static_cast<char>(c) : ' ';
                }
                s.text_rows.push_back(line);
            }
            const int fg = vs.regs[7] >> 4;
            const int bg = vs.regs[7] & 0x0F;
            s.fg[0] = vs.palette_r[fg];
            s.fg[1] = vs.palette_g[fg];
            s.fg[2] = vs.palette_b[fg];
            s.bg[0] = vs.palette_r[bg];
            s.bg[1] = vs.palette_g[bg];
            s.bg[2] = vs.palette_b[bg];
        }
        std::lock_guard<std::mutex> lock(snap_mutex_);
        snapshot_ = std::move(s);
    }

    void Loop() {
        using clock = std::chrono::steady_clock;
        const double frame_dt = 1.0 / Machine::kFrameRate;
        auto last = clock::now();
        auto last_snap = last;
        auto fps_start = last;
        int fps_frames = 0;
        double fps = 0.0;
        double acc = 0.0;
        std::vector<int16_t> live;
        while (running_) {
            DrainJobs();
            if (server_.port() != 0) server_.Poll(commander_);
            const auto now = clock::now();
            acc += std::chrono::duration<double>(now - last).count();
            last = now;
            if (acc > 0.25) acc = 0.25;

            ApplyPendingKeys();
            UpdateJoystick();
            int ran = 0;
            while (!paused_ && acc >= frame_dt && ran < 4) {
                machine_->RunFrame();
                commander_.Tick();
                AgeKeys();
                acc -= frame_dt;
                ++ran;
                ++fps_frames;
            }
            if (paused_) acc = 0.0;
            if (audio_ok_) {
                live.clear();
                machine_->TakeLiveAudio(live);
                audio_.Push(live.data(), live.size());
            }
            if (std::chrono::duration<double>(now - fps_start).count() >= 0.5) {
                fps = fps_frames / std::chrono::duration<double>(now - fps_start).count();
                fps_start = now;
                fps_frames = 0;
            }
            if (std::chrono::duration<double>(now - last_snap).count() >= 0.04) { // ~25 quadros/s para o terminal
                TakeSnapshot(fps);
                last_snap = now;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    TuiOptions opts_;
    MachineConfig config_;
    std::unique_ptr<Machine> machine_;
    control::Commander commander_;
    control::Server server_;
    audio::AudioOutput audio_;
    bool audio_ok_ = false;
    std::string ctl_message_;
    std::vector<std::string> port_files_;

    std::atomic<bool> running_{false};
    std::atomic<bool> paused_{false};
    std::atomic<bool> quit_{false};
    std::thread thread_;

    std::mutex jobs_mutex_;
    std::deque<Job> jobs_;
    std::mutex keys_mutex_;
    std::vector<PendingKey> pending_keys_;
    std::map<std::string, Held> held_;
    std::mutex snap_mutex_;
    Snapshot snapshot_;
};

// ---- desenho da tela do MSX -------------------------------------------------------------------

class MsxView : public Node {
public:
    MsxView(Snapshot snap, bool four_three) : snap_(std::move(snap)), four_three_(four_three) {}

    void ComputeRequirement() override {
        requirement_.min_x = 20;
        requirement_.min_y = 6;
        requirement_.flex_grow_x = 1;
        requirement_.flex_grow_y = 1;
    }

    void Render(Screen &screen) override {
        const int w = box_.x_max - box_.x_min + 1;
        const int h = box_.y_max - box_.y_min + 1;
        if (w <= 0 || h <= 0) return;
        if (!snap_.valid) {
            Put(screen, box_.x_min + 1, box_.y_min, "(iniciando a maquina...)", Color::GrayLight, Color::Default);
            return;
        }
        if (snap_.text) RenderText(screen, w, h);
        else RenderPixels(screen, w, h);
    }

private:
    static void Put(Screen &screen, int x, int y, const std::string &s, Color fg, Color bg) {
        for (size_t i = 0; i < s.size(); ++i) {
            if (x + static_cast<int>(i) > screen.dimx() - 1) break;
            Cell &c = screen.CellAt(x + static_cast<int>(i), y);
            c.character = std::string(1, s[i]);
            c.foreground_color = fg;
            c.background_color = bg;
        }
    }

    // SCREEN 0/1: o texto de verdade (nitido), nao pixels reduzidos.
    void RenderText(Screen &screen, int w, int h) {
        const Color fg = Color::RGB(snap_.fg[0], snap_.fg[1], snap_.fg[2]);
        const Color bg = Color::RGB(snap_.bg[0], snap_.bg[1], snap_.bg[2]);
        const int draw_w = std::min(w, snap_.cols);
        const int draw_h = std::min(h, snap_.rows);
        const int x0 = box_.x_min + (w - draw_w) / 2;
        const int y0 = box_.y_min + (h - draw_h) / 2;
        for (int r = 0; r < draw_h; ++r) {
            for (int c = 0; c < draw_w; ++c) {
                Cell &cell = screen.CellAt(x0 + c, y0 + r);
                cell.character = std::string(1, snap_.text_rows[static_cast<size_t>(r)][static_cast<size_t>(c)]);
                cell.foreground_color = fg;
                cell.background_color = bg;
            }
        }
    }

    // Graficos: cada celula do terminal mostra 2 pixels (meio bloco ▀: cima = frente, baixo = fundo),
    // reduzidos por media para caber na area disponivel.
    void RenderPixels(Screen &screen, int w, int h) {
        const int img_w = snap_.fs.width;
        const int img_h = snap_.fs.height * snap_.fs.y_scale;
        // A area util em "pixels quadrados": largura w celulas e altura 2*h (cada celula = 2 meios blocos;
        // uma celula de terminal e' ~1:2, entao o meio bloco e' ~quadrado).
        const int avail_h = 2 * h;
        int out_w;
        int out_h_px;
        if (four_three_) {
            // Proporcao de tela 4:3 (a de uma TV/monitor do MSX), qualquer que seja o formato do quadro
            // (272x228 no MSX1, 512 de largura no MSX2): a imagem inteira e' esticada para 4:3.
            if (w * 3 <= avail_h * 4) {
                out_w = w;
                out_h_px = w * 3 / 4;
            } else {
                out_h_px = avail_h;
                out_w = avail_h * 4 / 3;
            }
        } else {
            // Pixels originais (quadrados), so' reduzidos para caber.
            const double scale = std::min(static_cast<double>(w) / img_w, static_cast<double>(avail_h) / img_h);
            out_w = static_cast<int>(img_w * scale);
            out_h_px = static_cast<int>(img_h * scale);
        }
        out_w = std::max(1, out_w);
        out_h_px = std::max(2, out_h_px & ~1);
        const int x0 = box_.x_min + (w - out_w) / 2;
        const int y0 = box_.y_min + (h - out_h_px / 2) / 2;
        const double step_x = static_cast<double>(img_w) / out_w; // pixels de origem por pixel de destino
        const double step_y = static_cast<double>(img_h) / out_h_px;
        auto sample = [&](int dx, int dy) {
            const int sx0 = static_cast<int>(dx * step_x);
            const int sy0 = static_cast<int>(dy * step_y);
            const int sx1 = std::max(sx0 + 1, static_cast<int>((dx + 1) * step_x));
            const int sy1 = std::max(sy0 + 1, static_cast<int>((dy + 1) * step_y));
            unsigned r = 0, g = 0, b = 0, n = 0;
            for (int sy = sy0; sy < sy1 && sy < img_h; ++sy) {
                const uint32_t *row = snap_.rgba.data() + static_cast<size_t>(sy / snap_.fs.y_scale) * img_w;
                for (int sx = sx0; sx < sx1 && sx < img_w; ++sx) {
                    const uint32_t p = row[sx];
                    r += p & 0xFF;
                    g += (p >> 8) & 0xFF;
                    b += (p >> 16) & 0xFF;
                    ++n;
                }
            }
            if (n == 0) return Color::RGB(0, 0, 0);
            return Color::RGB(static_cast<uint8_t>(r / n), static_cast<uint8_t>(g / n), static_cast<uint8_t>(b / n));
        };
        for (int cy = 0; cy < out_h_px / 2; ++cy) {
            for (int cx = 0; cx < out_w; ++cx) {
                Cell &cell = screen.CellAt(x0 + cx, y0 + cy);
                cell.character = "\xE2\x96\x80"; // ▀
                cell.foreground_color = sample(cx, cy * 2);
                cell.background_color = sample(cx, cy * 2 + 1);
            }
        }
    }

    Snapshot snap_;
    bool four_three_;
};

// ---- traducao de teclas do terminal para o MSX -----------------------------------------------------

// Devolve true e preenche key/shift se o evento vira uma tecla do MSX.
bool EventToMsxKey(const Event &e, std::string &key, bool &shift) {
    shift = false;
    if (e == Event::Return) { key = "enter"; return true; }
    if (e == Event::Backspace) { key = "bs"; return true; }
    if (e == Event::Tab) { key = "tab"; return true; }
    if (e == Event::Escape) { key = "esc"; return true; }
    if (e == Event::ArrowUp) { key = "up"; return true; }
    if (e == Event::ArrowDown) { key = "down"; return true; }
    if (e == Event::ArrowLeft) { key = "left"; return true; }
    if (e == Event::ArrowRight) { key = "right"; return true; }
    if (e == Event::Delete) { key = "del"; return true; }
    if (e == Event::Insert) { key = "ins"; return true; }
    if (e == Event::Home) { key = "home"; return true; }
    if (e == Event::End) { key = "select"; return true; }
    if (e == Event::F1) { key = "f1"; return true; }
    if (e == Event::F2) { key = "f2"; return true; }
    if (e == Event::F3) { key = "f3"; return true; }
    if (e == Event::F4) { key = "f4"; return true; }
    if (e == Event::F5) { key = "f5"; return true; }
    const std::string &in = e.input();
    if (in.size() == 1) {
        const unsigned char c = static_cast<unsigned char>(in[0]);
        if (c == 3) { key = "stop"; shift = false; return true; } // Ctrl+C = STOP (CTRL+STOP abaixo)
        if (c >= 1 && c <= 26 && c != 8 && c != 9 && c != 10 && c != 13) { // Ctrl+letra
            key = std::string(1, static_cast<char>('a' + c - 1));
            return true;
        }
        if (c >= 0x20 && c < 0x7F) return Machine::KeysForChar(static_cast<char>(c), key, shift);
    }
    return false;
}

const char *const kHelpText[] = {
    "Comandos (os mesmos da ponte de controle e do console):",
    "  help version status reset pause resume step [n] type <texto> peek <end> [n] poke <end> <v>...",
    "  regs cart <arq|-> [mapper] disk <A|B> <arq> eject <A|B> tape <arq|eject|rewind>",
    "  state <save|load> <arq> screenshot <arq> quit   e   newdisk <arq> <ss525|ds525|ss35|ds35>",
    "Teclas: F6 salvar estado  F7 carregar  F8/F9 slot  F10 menu  F11 linha de comando  F12 captura",
    "  Esc fecha o menu/linha de comando. No MSX: Ctrl+C = STOP, End = SELECT, setas = joystick.",
};

} // namespace

int RunTui(const TuiOptions &options) {
    Runner runner(options);
    std::string error;
    if (!runner.Start(error)) {
        std::fprintf(stderr, "fwmsx --tui: %s\n", error.c_str());
        return 1;
    }

    enum class Mode { Msx, Menu, Command };
    Mode mode = Mode::Msx;
    int slot = 1;
    std::string toast;
    std::chrono::steady_clock::time_point toast_until{};
    std::string input;                       // linha de comando em edicao
    std::vector<std::string> history;        // comandos digitados
    int history_pos = 0;
    std::deque<std::string> log;             // ultimas respostas / ajuda
    int menu_index = 0;
    bool four_three = true;                  // graficos na proporcao 4:3 (menu: alterna com pixels originais)

    auto set_toast = [&](const std::string &t) {
        toast = t;
        toast_until = std::chrono::steady_clock::now() + std::chrono::seconds(4);
    };
    auto add_log = [&](const std::string &l) {
        log.push_back(l);
        while (log.size() > 8) log.pop_front();
    };
    auto slot_path = [&]() { return "fwmsx-estado-" + std::to_string(slot) + ".sst"; };
    auto shot_command = [&]() {
        const std::time_t now = std::time(nullptr);
        char name[64];
        std::strftime(name, sizeof(name), "fwmsx-%Y%m%d-%H%M%S.png", std::localtime(&now));
        return std::string("screenshot ") + name;
    };

    // Roda uma linha como a TUI entende: comandos locais (help, newdisk) ou o Commander.
    auto run_line = [&](const std::string &line) -> std::string {
        std::string err;
        const std::vector<std::string> t = control::Commander::Tokenize(line, err);
        if (!err.empty()) return "err " + err;
        if (t.empty()) return "ok";
        if (t[0] == "help") {
            for (const char *l : kHelpText) add_log(l);
            return "ok";
        }
        if (t[0] == "newdisk") {
            if (t.size() != 3) return "err use: newdisk <arq.dsk> <ss525|ds525|ss35|ds35>";
            const DiskFmtSpec *spec = diskfmt_spec_by_key(t[2].c_str());
            std::string derr;
            if (!spec) return "err formato desconhecido (ss525, ds525, ss35, ds35)";
            if (!diskfmt::CreateBlankDisk(t[1], spec, derr)) return "err " + derr;
            return std::string("ok disco criado: ") + t[1];
        }
        return runner.Execute(line);
    };
    auto run_and_report = [&](const std::string &line) {
        const std::string reply = run_line(line);
        const std::string shown = reply.rfind("ok ", 0) == 0 ? reply.substr(3) : (reply == "ok" ? "ok" : (reply.rfind("err ", 0) == 0 ? "erro: " + reply.substr(4) : reply));
        set_toast(shown);
        return shown;
    };

    struct MenuEntry {
        std::string label;
        std::function<void()> action;
    };
    auto build_menu = [&]() {
        std::vector<MenuEntry> m;
        const bool paused = runner.Snap().paused;
        m.push_back({"Reiniciar", [&] { run_and_report("reset"); }});
        m.push_back({paused ? "Retomar" : "Pausar", [&, paused] { run_and_report(paused ? "resume" : "pause"); }});
        m.push_back({"Salvar estado no slot " + std::to_string(slot) + "  (F6)", [&] { run_and_report("state save " + slot_path()); }});
        m.push_back({"Carregar estado do slot " + std::to_string(slot) + "  (F7)", [&] { run_and_report("state load " + slot_path()); }});
        m.push_back({"Slot de estado anterior  (F8)", [&] { slot = (slot + 7) % 9 + 1; set_toast("slot " + std::to_string(slot)); }});
        m.push_back({"Proximo slot de estado  (F9)", [&] { slot = slot % 9 + 1; set_toast("slot " + std::to_string(slot)); }});
        m.push_back({"Capturar tela (PNG)  (F12)", [&] { run_and_report(shot_command()); }});
        m.push_back({"Inserir cartucho...", [&] { input = "cart "; mode = Mode::Command; }});
        m.push_back({"Retirar cartucho", [&] { run_and_report("cart -"); }});
        m.push_back({"Inserir disco em A:...", [&] { input = "disk A "; mode = Mode::Command; }});
        m.push_back({"Inserir disco em B:...", [&] { input = "disk B "; mode = Mode::Command; }});
        m.push_back({"Ejetar disco A:", [&] { run_and_report("eject A"); }});
        m.push_back({"Ejetar disco B:", [&] { run_and_report("eject B"); }});
        m.push_back({"Novo disco em branco...", [&] { input = "newdisk novo.dsk ds35"; mode = Mode::Command; }});
        m.push_back({"Inserir fita...", [&] { input = "tape "; mode = Mode::Command; }});
        m.push_back({"Ejetar fita", [&] { run_and_report("tape eject"); }});
        m.push_back({"Rebobinar fita", [&] { run_and_report("tape rewind"); }});
        m.push_back({four_three ? "Proporcao: 4:3 (alternar para pixels originais)" : "Proporcao: pixels originais (alternar para 4:3)",
                     [&] { four_three = !four_three; set_toast(four_three ? "proporcao 4:3" : "pixels originais"); }});
        m.push_back({"Linha de comando  (F11)", [&] { mode = Mode::Command; }});
        m.push_back({"Ajuda", [&] { run_line("help"); mode = Mode::Command; }});
        m.push_back({"Sair", [&] { run_and_report("quit"); }});
        return m;
    };

    auto screen = ScreenInteractive::Fullscreen();
    screen.TrackMouse(false);
    screen.ForceHandleCtrlC(false);

    auto renderer = Renderer([&] {
        const Snapshot snap = runner.Snap();
        Elements rows;
        rows.push_back(text(" fwMSX TUI  F10 Menu  F11 Comando  F6/F7 Estado  F8/F9 Slot  F12 Captura  Ctrl+C = STOP ") | inverted);
        Element view = std::make_shared<MsxView>(snap, four_three);
        rows.push_back(view | flex);

        std::string status = " " + std::string(FWMSX_COMPANY) + ": " + FWMSX_CODENAME + " | SCREEN " + std::to_string(snap.scr_mode) + " | " +
                             std::to_string(static_cast<int>(snap.fps + 0.5)) + " fps | quadro " + std::to_string(snap.frame) + " | slot " +
                             std::to_string(slot) + (snap.paused ? " | PAUSADO" : "") + (runner.audio_ok() ? "" : " | mudo");
        if (runner.ctl_port() != 0) status += " | controle: porta " + std::to_string(runner.ctl_port());
        if (!toast.empty() && std::chrono::steady_clock::now() < toast_until) status += " | " + toast;
        rows.push_back(text(status) | inverted);

        if (mode == Mode::Command) {
            for (const std::string &l : log) rows.push_back(text(" " + l) | color(Color::GrayLight));
            rows.push_back(text(" > " + input + "_") | bold);
        }
        Element base = vbox(rows);
        if (mode == Mode::Menu) {
            const auto entries = build_menu();
            Elements items;
            for (size_t i = 0; i < entries.size(); ++i) {
                Element line = text(" " + entries[i].label + " ");
                if (static_cast<int>(i) == menu_index) line = line | inverted;
                items.push_back(line);
            }
            Element popup = vbox(items) | border | clear_under;
            base = dbox({base, vbox({filler(), hbox({filler(), popup, filler()}), filler()})});
        }
        return base;
    });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Custom) {
            if (runner.quit_requested()) screen.Exit();
            return true;
        }
        if (e.is_mouse()) return true;

        // ---- menu ----
        if (mode == Mode::Menu) {
            const auto entries = build_menu();
            if (e == Event::Escape || e == Event::F10) { mode = Mode::Msx; return true; }
            if (e == Event::ArrowUp) { menu_index = (menu_index + static_cast<int>(entries.size()) - 1) % static_cast<int>(entries.size()); return true; }
            if (e == Event::ArrowDown) { menu_index = (menu_index + 1) % static_cast<int>(entries.size()); return true; }
            if (e == Event::Return) {
                mode = Mode::Msx; // a acao pode abrir a linha de comando
                entries[static_cast<size_t>(menu_index)].action();
                return true;
            }
            return true;
        }

        // ---- linha de comando ----
        if (mode == Mode::Command) {
            if (e == Event::Escape || e == Event::F11) { mode = Mode::Msx; return true; }
            if (e == Event::F10) { mode = Mode::Menu; return true; }
            if (e == Event::Return) {
                if (!input.empty()) {
                    history.push_back(input);
                    history_pos = static_cast<int>(history.size());
                    add_log("> " + input);
                    const std::string shown = run_and_report(input);
                    if (!shown.empty() && shown != "ok") add_log(shown);
                }
                input.clear();
                return true;
            }
            if (e == Event::Backspace) { if (!input.empty()) input.pop_back(); return true; }
            if (e == Event::ArrowUp) {
                if (!history.empty() && history_pos > 0) input = history[static_cast<size_t>(--history_pos)];
                return true;
            }
            if (e == Event::ArrowDown) {
                if (history_pos + 1 < static_cast<int>(history.size())) input = history[static_cast<size_t>(++history_pos)];
                else { history_pos = static_cast<int>(history.size()); input.clear(); }
                return true;
            }
            if (e.is_character()) { input += e.character(); return true; }
            return true;
        }

        // ---- teclado do MSX ----
        if (e == Event::F6) { run_and_report("state save " + slot_path()); return true; }
        if (e == Event::F7) { run_and_report("state load " + slot_path()); return true; }
        if (e == Event::F8) { slot = (slot + 7) % 9 + 1; set_toast("slot " + std::to_string(slot)); return true; }
        if (e == Event::F9) { slot = slot % 9 + 1; set_toast("slot " + std::to_string(slot)); return true; }
        if (e == Event::F10) { mode = Mode::Menu; menu_index = 0; return true; }
        if (e == Event::F11) { mode = Mode::Command; return true; }
        if (e == Event::F12) { run_and_report(shot_command()); return true; }

        std::string key;
        bool shift = false;
        if (EventToMsxKey(e, key, shift)) {
            if (e.input().size() == 1 && static_cast<unsigned char>(e.input()[0]) == 3) {
                // Ctrl+C: CTRL+STOP do MSX
                runner.PressKey("ctrl", false);
            }
            runner.PressKey(key, shift);
        }
        return true;
    });

    // Atualiza a tela ~25 vezes por segundo (e percebe o pedido de saida vindo de `quit`).
    std::atomic<bool> ui_running{true};
    std::thread refresher([&] {
        while (ui_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
            screen.PostEvent(Event::Custom);
        }
    });
    screen.Loop(component);
    ui_running = false;
    refresher.join();
    runner.Stop();
    return 0;
}

} // namespace tui
