// fwMSX -- janela do emulador. Ver emu_window.h e doc/machine-spec.md.
#include "emu_window.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>
#include "../../audio/audio_output.h"
#include "../../common/version.h"
#include "../../msxdisk/gui/file_dialog.h"
#include "../../msxdisk/gui/style.h"
#include "video_filters.h"
#include "../../psg/cpp/psg_device.h"
#include "../../psg/core/psg_state.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace machine::gui {

namespace {

// Teclado do host -> tecla do MSX (nome de ppi_key_name()). Mapeamento
// POSICIONAL (layout US): a tecla fisica que fica no lugar da tecla MSX.
// Devolve nullptr para teclas sem equivalente.
const char *HostKeyToMsx(int key) {
    static const char *const kLetters[26] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
                                              "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"};
    static const char *const kDigits[10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    static const char *const kPad[10] = {"pad0", "pad1", "pad2", "pad3", "pad4", "pad5", "pad6", "pad7", "pad8", "pad9"};

    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) return kLetters[key - GLFW_KEY_A];
    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) return kDigits[key - GLFW_KEY_0];
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) return kPad[key - GLFW_KEY_KP_0];
    switch (key) {
    case GLFW_KEY_MINUS: return "-";
    case GLFW_KEY_EQUAL: return "=";
    case GLFW_KEY_BACKSLASH: return "\\";
    case GLFW_KEY_LEFT_BRACKET: return "[";
    case GLFW_KEY_RIGHT_BRACKET: return "]";
    case GLFW_KEY_SEMICOLON: return ";";
    case GLFW_KEY_APOSTROPHE: return "'";
    case GLFW_KEY_GRAVE_ACCENT: return "`";
    case GLFW_KEY_COMMA: return ",";
    case GLFW_KEY_PERIOD: return ".";
    case GLFW_KEY_SLASH: return "/";
    case GLFW_KEY_SPACE: return "space";
    case GLFW_KEY_ENTER:
    case GLFW_KEY_KP_ENTER: return "enter";
    case GLFW_KEY_BACKSPACE: return "bs";
    case GLFW_KEY_TAB: return "tab";
    case GLFW_KEY_ESCAPE: return "esc";
    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_RIGHT_SHIFT: return "shift";
    case GLFW_KEY_LEFT_CONTROL:
    case GLFW_KEY_RIGHT_CONTROL: return "ctrl";
    case GLFW_KEY_LEFT_ALT: return "graph";
    case GLFW_KEY_RIGHT_ALT: return "code";
    case GLFW_KEY_CAPS_LOCK: return "caps";
    case GLFW_KEY_F1: return "f1";
    case GLFW_KEY_F2: return "f2";
    case GLFW_KEY_F3: return "f3";
    case GLFW_KEY_F4: return "f4";
    case GLFW_KEY_F5: return "f5";
    case GLFW_KEY_HOME: return "home";
    case GLFW_KEY_INSERT: return "ins";
    case GLFW_KEY_DELETE: return "del";
    case GLFW_KEY_END: return "select";
    case GLFW_KEY_PAUSE: return "stop";
    case GLFW_KEY_LEFT: return "left";
    case GLFW_KEY_UP: return "up";
    case GLFW_KEY_DOWN: return "down";
    case GLFW_KEY_RIGHT: return "right";
    case GLFW_KEY_KP_MULTIPLY: return "pad*";
    case GLFW_KEY_KP_ADD: return "pad+";
    case GLFW_KEY_KP_DIVIDE: return "pad/";
    case GLFW_KEY_KP_SUBTRACT: return "pad-";
    case GLFW_KEY_KP_DECIMAL: return "pad.";
    default: return nullptr;
    }
}

// Teclado do host -> bit do joystick da porta A (alem de continuar valendo como
// tecla do MSX: as setas sao o cursor e Espaco a barra de espaco, e os jogos
// que leem so' um dos dois ignoram o outro). 0 = a tecla nao e' do joystick.
uint8_t HostKeyToJoy(int key) {
    switch (key) {
    case GLFW_KEY_UP: return PSG_JOY_UP;
    case GLFW_KEY_DOWN: return PSG_JOY_DOWN;
    case GLFW_KEY_LEFT: return PSG_JOY_LEFT;
    case GLFW_KEY_RIGHT: return PSG_JOY_RIGHT;
    case GLFW_KEY_SPACE:
    case GLFW_KEY_Z: return PSG_JOY_FIRE_A;
    case GLFW_KEY_X: return PSG_JOY_FIRE_B;
    default: return 0;
    }
}

// Gamepad do GLFW (d-pad ou analogico esquerdo + botoes) -> bits do joystick.
uint8_t PadToJoy(const GLFWgamepadstate &s) {
    uint8_t bits = 0;
    if (s.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] || s.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] < -0.5f) bits |= PSG_JOY_UP;
    if (s.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] || s.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] > 0.5f) bits |= PSG_JOY_DOWN;
    if (s.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] || s.axes[GLFW_GAMEPAD_AXIS_LEFT_X] < -0.5f) bits |= PSG_JOY_LEFT;
    if (s.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] || s.axes[GLFW_GAMEPAD_AXIS_LEFT_X] > 0.5f) bits |= PSG_JOY_RIGHT;
    if (s.buttons[GLFW_GAMEPAD_BUTTON_A] || s.buttons[GLFW_GAMEPAD_BUTTON_X]) bits |= PSG_JOY_FIRE_A;
    if (s.buttons[GLFW_GAMEPAD_BUTTON_B] || s.buttons[GLFW_GAMEPAD_BUTTON_Y]) bits |= PSG_JOY_FIRE_B;
    return bits;
}

// Estado compartilhado com os callbacks do GLFW (que nao carregam ponteiro
// de usuario por padrao aqui -- uma unica janela por processo).
struct HostInput {
    std::vector<std::pair<const char *, bool>> events; // (tecla MSX, pressionada?)
    uint8_t joy_keys = 0;    // bits do joystick com a tecla do host segurada agora
    uint8_t joy_latched = 0; // bits pressionados em algum momento desde o ultimo quadro
    bool lost_focus = false;
    bool toggle_fullscreen = false;
};
HostInput g_input;

void KeyCallback(GLFWwindow *, int key, int, int action, int) {
    if (action == GLFW_REPEAT) return; // o MSX faz o proprio auto-repeat
    if (key == GLFW_KEY_F11) {
        if (action == GLFW_PRESS) g_input.toggle_fullscreen = true;
        return;
    }
    if (const uint8_t bit = HostKeyToJoy(key)) {
        if (action == GLFW_PRESS) {
            g_input.joy_keys |= bit;
            g_input.joy_latched |= bit; // um toque mais curto que um quadro ainda vale 1 quadro
        } else {
            g_input.joy_keys &= static_cast<uint8_t>(~bit);
        }
    }
    if (const char *name = HostKeyToMsx(key)) g_input.events.emplace_back(name, action == GLFW_PRESS);
}

void FocusCallback(GLFWwindow *, int focused) {
    if (!focused) {
        g_input.lost_focus = true;
        g_input.joy_keys = 0;
        g_input.joy_latched = 0;
    }
}

void GlfwErrorCallback(int error, const char *description) {
    std::fprintf(stderr, "fwmsx: erro do GLFW %d: %s\n", error, description);
}

// Aplica os eventos de teclado acumulados. Um toque mais curto que um quadro
// (press+release no mesmo lote) teria sumido para o BIOS, que so' le o teclado
// uma vez por quadro: o press vale agora e o release fica para depois do
// proximo quadro (`deferred`).
void ApplyKeyEvents(Machine &m, std::vector<std::string> &deferred) {
    std::vector<std::string> pressed_in_batch;
    for (const auto &[name, down] : g_input.events) {
        const std::string key = name;
        if (down) {
            deferred.erase(std::remove(deferred.begin(), deferred.end(), key), deferred.end());
            m.KeyDown(key);
            pressed_in_batch.push_back(key);
        } else if (std::find(pressed_in_batch.begin(), pressed_in_batch.end(), key) != pressed_in_batch.end()) {
            deferred.push_back(key);
        } else {
            m.KeyUp(key);
        }
    }
    g_input.events.clear();
    if (g_input.lost_focus) {
        m.ReleaseAllKeys();
        deferred.clear();
        g_input.lost_focus = false;
    }
}

// BIOS padrao de cada modelo, ao lado da BIOS atual (mesma pasta de ROMs).
// FMPAC.ROM fica ao lado das ROMs do fMSX: resource/fMSX/ROMs/ -> resource/fMSX/FMPAC.ROM.
// Vazio se nao existir.
std::string FmpacNear(const std::string &bios_path) {
    std::error_code ec;
    const std::filesystem::path candidate =
        std::filesystem::path(bios_path).parent_path().parent_path() / "FMPAC.ROM";
    return std::filesystem::is_regular_file(candidate, ec) ? candidate.string() : std::string();
}

std::string BiosFor(const std::string &current_bios, Model model) {
    const char *name = model == Model::MSX2P ? "MSX2P.ROM" : model == Model::MSX2 ? "MSX2.ROM" : "MSX.ROM";
    const size_t slash = current_bios.find_last_of("/\\");
    const std::string dir = slash == std::string::npos ? std::string() : current_bios.substr(0, slash + 1);
    return dir + name;
}

} // namespace

int RunEmulatorWindow(const WindowOptions &options) {
    std::string error;
    std::unique_ptr<Machine> machine = Machine::Create(options.machine, error);
    if (!machine) {
        std::fprintf(stderr, "fwmsx: %s\n", error.c_str());
        return 1;
    }

    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) {
        std::fprintf(stderr, "fwmsx: falha ao inicializar o GLFW\n");
        return 1;
    }
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Janela inicial: 3x o MSX1 ou 2x o MSX2 (quadro com borda, 544x228 com linhas dobradas).
    // Janela inicial em 3x, sem passar da area util do monitor principal.
    int start_w = std::max(980, machine->is_msx2() ? 544 * 3 : Machine::kFrameWidth * 3);
    int start_h = (machine->is_msx2() ? Machine::kFrameHeight * 2 * 3 : Machine::kFrameHeight * 3) + 40;
    int work_x = 0, work_y = 0, work_w = 0, work_h = 0;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &work_x, &work_y, &work_w, &work_h);
    if (work_w > 0 && work_h > 0) {
        start_w = std::min(start_w, work_w * 95 / 100);
        start_h = std::min(start_h, work_h * 80 / 100); // deixa espaco para a barra de titulo
    }
    GLFWwindow *window = glfwCreateWindow(start_w, start_h, "fwMSX", nullptr, nullptr);
    if (window == nullptr) {
        std::fprintf(stderr, "fwmsx: falha ao criar a janela (GLFW)\n");
        glfwTerminate();
        return 1;
    }
    if (work_w > 0) glfwSetWindowPos(window, work_x + (work_w - start_w) / 2, work_y + 60); // espaco para a barra de titulo
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Callbacks do emulador ANTES do ImGui: o backend do ImGui encadeia o
    // callback que ja' existe, entao os dois recebem as teclas.
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetWindowFocusCallback(window, FocusCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr; // sem imgui.ini solto na pasta de trabalho
    msxdisk::gui::LoadModernFont(io);
    msxdisk::gui::ApplyModernStyle(true);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    std::vector<uint32_t> pixels;
    // A imagem muda de tamanho com o modo de tela no MSX2 (512x192 ou 512x212,
    // linhas dobradas na exibicao): a textura e' recriada quando isso acontece.
    FrameSize frame_size = machine->RenderFrame(pixels);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_size.width, frame_size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Audio ao vivo: o PSG empurra as amostras de cada quadro para o buffer do
    // dispositivo. Sem dispositivo (ou --mute) o emulador segue mudo.
    audio::AudioOutput audio_out(psg::kSampleRate);
    std::string audio_status = "desligado (--mute)";
    bool audio_ok = false;
    if (options.audio) {
        std::string audio_error;
        audio_ok = audio_out.Start(audio_error);
        audio_status = audio_ok ? audio_out.device_name() : ("indisponivel: " + audio_error);
        if (audio_ok) machine->EnableLiveAudio(true);
    }
    float volume = 0.7f;
    bool muted = false;
    audio_out.SetGain(volume);
    std::vector<int16_t> live_samples;

    bool paused = false;
    bool integer_scale = true;
    bool show_keys = false;
    // Configuracao de slots (menu Maquina): o layout em edicao e a mensagem de erro dele.
    bool show_slots = false;
    SlotLayout slot_edit;
    std::string slot_message;
    bool fullscreen = false;
    int saved_x = 0, saved_y = 0, saved_w = 0, saved_h = 0;

    std::string pad_names[2];
    std::string disk_message; // ultimo erro ao inserir disco (menu Disco)
    std::vector<std::string> deferred_releases;
    const double frame_dt = 1.0 / Machine::kFrameRate;
    double last_time = glfwGetTime();
    double accumulator = 0.0;
    double fps_window_start = last_time;
    int fps_frames = 0;
    double fps = 0.0;
    int exit_code = 0;

    // Configuracao usada para criar a maquina atual: trocar modelo ou cartucho
    // recria a maquina a partir dela (os discos montados entram de novo).
    MachineConfig current = options.machine;
    auto snapshot = [&]() {
        MachineConfig next = current;
        next.disk_a = machine->disk(0).loaded() ? machine->disk(0).path() : std::string();
        next.disk_b = machine->disk(1).loaded() ? machine->disk(1).path() : std::string();
        return next;
    };
    auto reboot = [&](const MachineConfig &next) {
        // A SRAM do cartucho atual e' gravada antes de a maquina ser recriada.
        std::string save_error;
        if (!machine->SaveSram(save_error)) disk_message = save_error;
        std::string reboot_error;
        std::unique_ptr<Machine> fresh = Machine::Create(next, reboot_error);
        if (!fresh) {
            disk_message = reboot_error;
            return;
        }
        machine = std::move(fresh);
        current = next;
        machine->EnableLiveAudio(audio_ok);
        disk_message.clear();
    };
    auto LoadCartridgeFromDialog = [&]() {
        if (const auto chosen = msxdisk::gui::ShowOpenFileDialog(window, "Abrir cartucho MSX", "Cartuchos MSX",
                                                                  "*.rom;*.mx1;*.mx2;*.bin")) {
            MachineConfig next = snapshot();
            SetCartridge(next, *chosen);
            reboot(next);
        }
    };

    // Interface (menu Configuracoes -> Interface): tema, tamanho da letra e moldura da tela.
    int ui_theme = 0;      // 0 = escuro, 1 = claro
    int ui_font = 1;       // 0 = normal, 1 = grande (padrao), 2 = muito grande
    bool ui_frame = true; // borda e sombra em volta da tela do MSX
    int applied_theme = 0;
    int applied_font = 0;
    static const float kFontScales[3] = {1.0f, 1.25f, 1.5f};
    // Exibicao: zoom da janela (1x, 1.5x, 2x, 3x), proporcao da imagem e menu oculto em tela cheia.
    static const float kZooms[4] = {2.0f, 3.0f, 4.0f, 6.0f};
    static const char *kZoomNames[4] = {"2x", "3x", "4x", "6x"};
    // Filtros de video (menu Video): interpolacao, scanlines e filtro de cor.
    VideoFilterOptions video_opts;
    std::vector<uint32_t> filtered;
    int tex_w = frame_size.width;
    int tex_h = frame_size.height;
    int zoom_index = 1; // 3x
    int aspect_mode = 0; // 0 = original (pixels), 1 = 4:3 corrigido, 2 = 16:9 esticado
    bool resize_pending = false;
    bool menu_visible = true;
    // Tamanho da imagem em pixels de tela, antes do zoom, para a proporcao escolhida.
    auto display_size = [&](float &w, float &h) {
        h = static_cast<float>(frame_size.height * frame_size.y_scale);
        w = static_cast<float>(frame_size.width);
        if (aspect_mode == 1) w = h * 4.0f / 3.0f;
        else if (aspect_mode == 2) w = h * 16.0f / 9.0f;
    };
    // Ajusta a janela para o zoom escolhido (a altura inclui a barra de menu).
    auto apply_window_zoom = [&]() {
        float w = 0.0f, h = 0.0f;
        display_size(w, h);
        const int cw = static_cast<int>(std::lround(w * kZooms[zoom_index]));
        const int ch = static_cast<int>(std::lround(h * kZooms[zoom_index] + ImGui::GetFrameHeight()));
        glfwSetWindowSize(window, cw, ch);
    };
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (g_input.toggle_fullscreen) {
            g_input.toggle_fullscreen = false;
            if (!fullscreen) {
                glfwGetWindowPos(window, &saved_x, &saved_y);
                glfwGetWindowSize(window, &saved_w, &saved_h);
                GLFWmonitor *monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            } else {
                glfwSetWindowMonitor(window, nullptr, saved_x, saved_y, saved_w, saved_h, 0);
            }
            fullscreen = !fullscreen;
        }
        if (resize_pending && !fullscreen) {
            apply_window_zoom();
            resize_pending = false;
        }

        // Emulacao em tempo real: quantos quadros de 1/59.92 s ja' passaram.
        const double now = glfwGetTime();
        accumulator += std::min(now - last_time, 0.1);
        last_time = now;
        int ran = 0;
        ApplyKeyEvents(*machine, deferred_releases);
        // Joystick: teclado (porta A) + gamepads do GLFW (1o -> porta A, 2o -> porta B).
        pad_names[0].clear();
        pad_names[1].clear();
        uint8_t joy[2] = {static_cast<uint8_t>(g_input.joy_keys | g_input.joy_latched), 0};
        for (int pad = 0, port = 0; pad <= GLFW_JOYSTICK_LAST && port < 2; ++pad) {
            GLFWgamepadstate state;
            if (!glfwJoystickIsGamepad(pad) || !glfwGetGamepadState(pad, &state)) continue;
            joy[port] |= PadToJoy(state);
            pad_names[port] = glfwGetGamepadName(pad);
            ++port;
        }
        machine->SetJoystick(0, joy[0]);
        machine->SetJoystick(1, joy[1]);
        if (ran == 0 && accumulator >= frame_dt) g_input.joy_latched = 0;
        while (!paused && accumulator >= frame_dt && ran < 4) {
            machine->RunFrame();
            g_input.joy_latched = 0; // o toque ja' foi visto por este quadro
            accumulator -= frame_dt;
            ++ran;
            ++fps_frames;
            // Teclas soltas "rapido demais" so' soltam depois de um quadro inteiro.
            for (const std::string &k : deferred_releases) machine->KeyUp(k);
            deferred_releases.clear();
            if (ran < 4 && accumulator >= frame_dt) ApplyKeyEvents(*machine, deferred_releases);
        }
        if (ran == 4) accumulator = 0.0; // muito atrasado: descarta em vez de acelerar
        if (paused) accumulator = 0.0;
        if (audio_ok) {
            live_samples.clear();
            machine->TakeLiveAudio(live_samples);
            audio_out.Push(live_samples.data(), live_samples.size());
        }
        if (ran > 0) {
            const FrameSize fsz = machine->RenderFrame(pixels);
            glBindTexture(GL_TEXTURE_2D, texture);
            const std::vector<uint32_t> *upload = &pixels;
            int up_w = fsz.width;
            int up_h = fsz.height;
            if (VideoFiltersActive(video_opts)) {
                ApplyVideoFilters(pixels, fsz.width, fsz.height, video_opts, filtered, up_w, up_h);
                upload = &filtered;
            }
            if (up_w != tex_w || up_h != tex_h) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, up_w, up_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, upload->data());
                tex_w = up_w;
                tex_h = up_h;
            } else {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, up_w, up_h, GL_RGBA, GL_UNSIGNED_BYTE, upload->data());
            }
            frame_size = fsz;
        }

        if (now - fps_window_start >= 0.5) {
            fps = fps_frames / (now - fps_window_start);
            fps_window_start = now;
            fps_frames = 0;
            char title[96];
            std::snprintf(title, sizeof(title), "fwMSX - SCREEN %d - %.1f fps%s%s", machine->vdp_state().scr_mode, fps,
                          paused ? " (pausado)" : "", audio_ok ? "" : " (mudo)");
            glfwSetWindowTitle(window, title);
        }

        if (options.autoquit_frames > 0 && machine->frame_count() >= static_cast<uint64_t>(options.autoquit_frames)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if (ui_theme != applied_theme) {
            msxdisk::gui::ApplyModernStyle(ui_theme == 0);
            applied_theme = ui_theme;
        }
        if (ui_font != applied_font) {
            ImGui::GetIO().FontGlobalScale = kFontScales[ui_font];
            applied_font = ui_font;
        }
        // SRAM de cartucho: grava de tempos em tempos, so' se houve escrita (sem perder o save se a janela cair).
        if (machine->frame_count() % 300 == 0 && machine->sram_dirty()) {
            std::string save_error;
            if (!machine->SaveSram(save_error)) disk_message = save_error;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            const ImGuiIO &io_now = ImGui::GetIO();
            const float bar_h = ImGui::GetFrameHeight();
            const bool near_top = io_now.MousePos.y >= 0.0f && io_now.MousePos.y < (menu_visible ? bar_h + 8.0f : 6.0f);
            const bool submenu_open = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);
            menu_visible = !fullscreen || near_top || submenu_open;
        }
        if (menu_visible && ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Arquivo")) {
                if (ImGui::MenuItem("Carregar cartucho...")) LoadCartridgeFromDialog();
                ImGui::MenuItem("Salvar estado (em breve)", nullptr, false, false);
                ImGui::Separator();
                if (ImGui::MenuItem("Sair")) glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Maquina")) {
                if (ImGui::MenuItem("Reiniciar")) machine->Reset();
                if (ImGui::BeginMenu("Modelo")) {
                    const struct {
                        Model model;
                        const char *label;
                    } models[] = {{Model::MSX1, "MSX1 (TMS9918)"}, {Model::MSX2, "MSX2 (V9938)"}, {Model::MSX2P, "MSX2+ (V9958)"}};
                    for (const auto &entry : models) {
                        if (ImGui::MenuItem(entry.label, nullptr, current.model == entry.model) && current.model != entry.model) {
                            MachineConfig next = snapshot();
                            next.model = entry.model;
                            next.bios_path = BiosFor(current.bios_path, entry.model);
                            // Trocar o modelo volta o layout ao padrao; cartucho e FM-PAC continuam.
                            next.cart_path = CartridgePath(current);
                            next.cart_mapper = EffectiveLayout(current).cell[1][0].mapper;
                            next.fmpac_rom_path = FmPacPath(current);
                            next.layout_set = false;
                            next.ext_rom_path.clear();
                            reboot(next);
                        }
                    }
                    ImGui::Separator();
                    ImGui::TextDisabled("Trocar o modelo reinicia a maquina.");
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Configuracao de slots...")) {
                    show_slots = true;
                    slot_edit = EffectiveLayout(current);
                    slot_message.clear();
                }
                if (ImGui::BeginMenu("Video")) {
                    ImGui::MenuItem("NTSC (EUA/Japao)", nullptr, true);
                    ImGui::MenuItem("PAL (Europa) (em breve)", nullptr, false, false);
                    ImGui::EndMenu();
                }
                ImGui::MenuItem("Memoria principal (em breve)", nullptr, false, false);
                ImGui::MenuItem("Memoria de video (em breve)", nullptr, false, false);
                ImGui::Separator();
                if (ImGui::MenuItem("Pausar", nullptr, &paused) && paused) audio_out.Flush();
                if (ImGui::MenuItem("Soltar todas as teclas")) {
                    machine->ReleaseAllKeys();
                    deferred_releases.clear();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Exibir")) {
                ImGui::TextDisabled("Janela");
                for (int z = 0; z < 4; ++z) {
                    if (ImGui::MenuItem(kZoomNames[z], nullptr, !fullscreen && zoom_index == z)) {
                        zoom_index = z;
                        if (fullscreen) g_input.toggle_fullscreen = true;
                        resize_pending = true;
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Tela cheia", "F11", fullscreen)) g_input.toggle_fullscreen = true;
                ImGui::Separator();
                ImGui::TextDisabled("Proporcao");
                if (ImGui::MenuItem("Original (pixels)", nullptr, aspect_mode == 0)) aspect_mode = 0;
                if (ImGui::MenuItem("4:3 (corrigido)", nullptr, aspect_mode == 1)) aspect_mode = 1;
                if (ImGui::MenuItem("16:9 (esticado)", nullptr, aspect_mode == 2)) aspect_mode = 2;
                ImGui::Separator();
                ImGui::MenuItem("Escala inteira (so' em Original)", nullptr, &integer_scale);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Video")) {
                if (ImGui::BeginMenu("Interpolate Video")) {
                    const struct {
                        Interpolation mode;
                        const char *label;
                    } modes[] = {{Interpolation::Nearest, "Nearest Neighbor"},
                                 {Interpolation::Linear, "Linear Scaling"},
                                 {Interpolation::Epx, "EPX Scale 2x"},
                                 {Interpolation::Eagle, "Eagle Algorithm"},
                                 {Interpolation::Scale2x, "Scale 2x Algorithm"},
                                 {Interpolation::Sal2x, "2xSal Algorithm"}};
                    for (const auto &m : modes) {
                        if (ImGui::MenuItem(m.label, nullptr, video_opts.interp == m.mode)) video_opts.interp = m.mode;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Scanlines")) {
                    if (ImGui::MenuItem("Nenhum", nullptr, video_opts.scanlines == Scanlines::None)) video_opts.scanlines = Scanlines::None;
                    if (ImGui::MenuItem("TV", nullptr, video_opts.scanlines == Scanlines::Tv)) video_opts.scanlines = Scanlines::Tv;
                    if (ImGui::MenuItem("LCD", nullptr, video_opts.scanlines == Scanlines::Lcd)) video_opts.scanlines = Scanlines::Lcd;
                    if (ImGui::MenuItem("LCD Raster", nullptr, video_opts.scanlines == Scanlines::LcdRaster)) video_opts.scanlines = Scanlines::LcdRaster;
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Color Filter")) {
                    const struct {
                        ColorFilter filter;
                        const char *label;
                    } filters[] = {{ColorFilter::None, "Nenhum"},
                                   {ColorFilter::Monochrome, "Monochrome"},
                                   {ColorFilter::Sepia, "Sepia"},
                                   {ColorFilter::GreenCrt, "Green CRT"},
                                   {ColorFilter::AmberCrt, "Amber CRT"},
                                   {ColorFilter::CmyRaster, "CMY Raster"},
                                   {ColorFilter::RgbRaster, "RGB Raster"}};
                    for (const auto &f : filters) {
                        if (ImGui::MenuItem(f.label, nullptr, video_opts.color == f.filter)) video_opts.color = f.filter;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Som")) {
                ImGui::TextDisabled("%s", audio_ok ? "Saida:" : "Audio:");
                ImGui::TextUnformatted(audio_status.c_str());
                ImGui::Separator();
                if (audio_ok) {
                    bool changed = ImGui::MenuItem("Mudo", nullptr, &muted);
                    ImGui::SetNextItemWidth(160);
                    changed |= ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f, "%.2f");
                    if (changed) audio_out.SetGain(muted ? 0.0f : volume);
                }
                ImGui::Separator();
                ImGui::MenuItem("Gravar trilha de som (em breve)", nullptr, false, false);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Disco")) {
                if (!machine->has_disk_interface()) {
                    ImGui::TextDisabled("Sem interface de disquete.");
                    ImGui::TextDisabled("Use --disk <arq.dsk> ao iniciar, ou troque o modelo.");
                } else {
                    for (int d = 0; d < 2; ++d) {
                        const char letter = static_cast<char>('A' + d);
                        const fdc::DiskImage &img = machine->disk(d);
                        ImGui::Text("%c: %s", letter, img.loaded() ? img.path().c_str() : "(vazio)");
                        std::string label = std::string("Inserir em ") + letter + ":...";
                        if (ImGui::MenuItem(label.c_str())) {
                            if (const auto chosen = msxdisk::gui::ShowOpenDskDialog(window)) {
                                std::string disk_error;
                                if (!machine->InsertDisk(d, *chosen, disk_error)) disk_message = disk_error;
                                else disk_message.clear();
                            }
                        }
                        label = std::string("Ejetar ") + letter + ":";
                        if (ImGui::MenuItem(label.c_str(), nullptr, false, img.loaded())) machine->EjectDisk(d);
                    }
                    if (!disk_message.empty()) {
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "%s", disk_message.c_str());
                    }
                    ImGui::Separator();
                    ImGui::MenuItem("Novo disco... (em breve)", nullptr, false, false);
                    ImGui::TextDisabled("As gravacoes do MSX-DOS vao direto para o arquivo.");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Cartucho")) {
                const std::string cart = CartridgePath(current);
                ImGui::Text("Slot 1: %s", cart.empty() ? "(vazio)" : cart.c_str());
                if (!machine->cart_info().empty()) ImGui::TextDisabled("%s", machine->cart_info().c_str());
                ImGui::Separator();
                if (ImGui::MenuItem("Inserir cartucho...")) LoadCartridgeFromDialog();
                if (ImGui::MenuItem("Retirar cartucho", nullptr, false, !cart.empty())) {
                    MachineConfig next = snapshot();
                    SetCartridge(next, "");
                    reboot(next);
                }
                ImGui::Separator();
                // FM-PAC (OPLL + SRAM de 8KB) no slot 2:0 -- ver doc/fm-spec.md.
                if (!disk_message.empty()) ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "%s", disk_message.c_str());
                // FM-PAC (OPLL + SRAM de 8KB) no slot 2:0 -- ver doc/fm-spec.md.
                const bool fmpac_on = !FmPacPath(current).empty();
                if (ImGui::MenuItem("FM-PAC (Panasonic, slot 2:0)", nullptr, fmpac_on)) {
                    MachineConfig next = snapshot();
                    if (!fmpac_on) {
                        const std::string found = FmpacNear(current.bios_path);
                        if (found.empty()) {
                            disk_message = "FMPAC.ROM nao encontrada ao lado das ROMs do fMSX (resource/fMSX/)";
                        } else {
                            SetFmPac(next, found);
                            reboot(next);
                        }
                    } else {
                        SetFmPac(next, "");
                        reboot(next);
                    }
                }
                ImGui::MenuItem("Slot 2 (em breve)", nullptr, false, false);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Joystick")) {
                ImGui::TextDisabled("Porta A: setas + Z/Espaco (A) + X (B)");
                for (int port = 0; port < 2; ++port) {
                    ImGui::Text("Gamepad -> porta %c: %s", 'A' + port, pad_names[port].empty() ? "(nenhum)" : pad_names[port].c_str());
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Ferramentas")) {
                ImGui::MenuItem("Dispositivos de entrada (em breve)", nullptr, false, false);
                ImGui::MenuItem("Trapacas (em breve)", nullptr, false, false);
                ImGui::MenuItem("Buscar trapacas (em breve)", nullptr, false, false);
                ImGui::Separator();
                ImGui::MenuItem("Mostrar todos os sprites (em breve)", nullptr, false, false);
                ImGui::MenuItem("Patch da DiskROM (em breve)", nullptr, false, false);
                ImGui::MenuItem("Bateria MIDI (em breve)", nullptr, false, false);
                ImGui::Separator();
                ImGui::MenuItem("POKE &HFFFF,&HAA (em breve)", nullptr, false, false);
                ImGui::MenuItem("Rebobinar fita (em breve)", nullptr, false, false);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Configuracoes")) {
                if (ImGui::BeginMenu("Interface")) {
                    ImGui::TextDisabled("Tema");
                    if (ImGui::MenuItem("Escuro", nullptr, ui_theme == 0)) ui_theme = 0;
                    if (ImGui::MenuItem("Claro", nullptr, ui_theme == 1)) ui_theme = 1;
                    ImGui::Separator();
                    ImGui::TextDisabled("Tamanho da letra");
                    if (ImGui::MenuItem("Normal", nullptr, ui_font == 0)) ui_font = 0;
                    if (ImGui::MenuItem("Grande", nullptr, ui_font == 1)) ui_font = 1;
                    if (ImGui::MenuItem("Muito grande", nullptr, ui_font == 2)) ui_font = 2;
                    ImGui::Separator();
                    ImGui::MenuItem("Borda e sombra da tela", nullptr, &ui_frame);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Ajuda")) {
                ImGui::MenuItem("Teclado", nullptr, &show_keys);
                ImGui::Separator();
                ImGui::TextDisabled("fwMSX v%d.%d.%d (%s \"%s\")", FWMSX_VERSION_MAJOR, FWMSX_VERSION_MINOR,
                                    FWMSX_VERSION_PATCH, FWMSX_CODENAME, FWMSX_SUBTITLE);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // A tela do MSX, centralizada, ocupando o espaco abaixo do menu.
        const ImGuiViewport *vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("##tela", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground);
        {
            const ImVec2 avail = ImGui::GetContentRegionAvail();
            float disp_w = 0.0f, disp_h = 0.0f;
            display_size(disp_w, disp_h);
            float scale = std::min(avail.x / disp_w, avail.y / disp_h);
            if (aspect_mode == 0 && integer_scale && scale >= 1.0f) scale = std::floor(scale);
            if (scale < 0.1f) scale = 0.1f;
            const ImVec2 size(disp_w * scale, disp_h * scale);
            ImGui::SetCursorPos(ImVec2((avail.x - size.x) * 0.5f, (avail.y - size.y) * 0.5f));
            const ImVec2 p0 = ImGui::GetCursorScreenPos();
            const ImVec2 p1(p0.x + size.x, p0.y + size.y);
            ImDrawList *dl = ImGui::GetWindowDrawList();
            if (ui_frame) {
                // Sombra deslocada, atras da imagem.
                dl->AddRectFilled(ImVec2(p0.x + 6, p0.y + 8), ImVec2(p1.x + 6, p1.y + 8),
                                  IM_COL32(0, 0, 0, 120), 8.0f);
            }
            ImGui::Image(static_cast<ImTextureID>(texture), size);
            if (ui_frame) {
                dl->AddRect(ImVec2(p0.x - 3, p0.y - 3), ImVec2(p1.x + 3, p1.y + 3), IM_COL32(110, 110, 125, 255), 6.0f, 0, 2.0f);
            }
        }
        ImGui::End();
        ImGui::PopStyleVar(2);

        if (show_slots) {
            ImGui::SetNextWindowSize(ImVec2(820, 560), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Configuracao da maquina", &show_slots)) {
                ImGui::TextWrapped(
                    "Cada linha e' uma celula do layout (slot primario:secundario). A BIOS fica em 0:0: "
                    "um arquivo de 32KB ocupa a pagina 0 (BIOS, 16KB) e a pagina 1 (BASIC, 16KB); ou dois "
                    "arquivos de 16KB, um para cada pagina (use o botao BASIC). Aplicar reinicia a maquina.");
                const char *kind_names[] = {"Vazio", "ROM (BIOS, BASIC ou cartucho)", "Sub-ROM MSX2 (16KB)",
                                            "RAM", "RAM mapeada (mapper)", "Disco (DISK.ROM)", "FM-PAC"};
                if (ImGui::BeginTable("##slots", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableSetupColumn("Slot", ImGuiTableColumnFlags_WidthFixed, 48.0f);
                    ImGui::TableSetupColumn("Conteudo", ImGuiTableColumnFlags_WidthFixed, 230.0f);
                    ImGui::TableSetupColumn("Arquivo");
                    ImGui::TableSetupColumn("Arquivo 2", ImGuiTableColumnFlags_WidthFixed, 190.0f);
                    ImGui::TableSetupColumn("Opcoes", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                    ImGui::TableHeadersRow();
                    for (int p = 0; p < 4; ++p) {
                        for (int sec = 0; sec < 4; ++sec) {
                            SlotItem &it = slot_edit.cell[p][sec];
                            ImGui::PushID(p * 4 + sec);
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%d:%d", p, sec);
                            ImGui::TableSetColumnIndex(1);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            int kind = static_cast<int>(it.kind);
                            if (ImGui::Combo("##kind", &kind, kind_names, 7)) {
                                it = SlotItem{};
                                it.kind = static_cast<SlotKind>(kind);
                                if (it.kind == SlotKind::Rom) it.page = (p == 0 && sec == 0) ? 0 : 1;
                                if (it.kind == SlotKind::Ram) it.size_kb = 64;
                                if (it.kind == SlotKind::Mapper) it.size_kb = 128;
                            }
                            ImGui::TableSetColumnIndex(2);
                            const bool has_file = it.kind == SlotKind::Rom || it.kind == SlotKind::SubRom ||
                                                  it.kind == SlotKind::Disk || it.kind == SlotKind::FmPac;
                            if (has_file) {
                                ImGui::TextDisabled("%s", it.path.empty() ? "(nenhum)" : it.path.c_str());
                                if (ImGui::SmallButton("Procurar...")) {
                                    if (const auto chosen = msxdisk::gui::ShowOpenFileDialog(
                                            window, "Escolher ROM", "ROMs MSX", "*.rom;*.bin;*.mx1;*.mx2;*.dat")) {
                                        it.path = *chosen;
                                    }
                                }
                            }
                            ImGui::TableSetColumnIndex(3);
                            const bool second = (it.kind == SlotKind::Rom && p == 0 && sec == 0) || it.kind == SlotKind::Disk;
                            if (second) {
                                ImGui::TextDisabled("%s", it.path2.empty() ? "(nenhum)" : it.path2.c_str());
                                if (ImGui::SmallButton(it.kind == SlotKind::Disk ? "Sub-ROM MSX2..." : "BASIC...")) {
                                    if (const auto chosen = msxdisk::gui::ShowOpenFileDialog(
                                            window, "Escolher ROM", "ROMs MSX", "*.rom;*.bin;*.mx1;*.mx2;*.dat")) {
                                        it.path2 = *chosen;
                                    }
                                }
                                if (!it.path2.empty()) {
                                    ImGui::SameLine();
                                    if (ImGui::SmallButton("Limpar")) it.path2.clear();
                                }
                            }
                            ImGui::TableSetColumnIndex(4);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            if (it.kind == SlotKind::Rom) {
                                int page = it.page;
                                const char *pages[] = {"pagina 0 (0000h)", "pagina 1 (4000h)"};
                                if (ImGui::Combo("##page", &page, pages, 2)) it.page = page;
                            } else if (it.kind == SlotKind::Ram) {
                                int size = it.size_kb == 16 ? 0 : it.size_kb == 32 ? 1 : 2;
                                const char *sizes[] = {"16 KB", "32 KB", "64 KB"};
                                if (ImGui::Combo("##ram", &size, sizes, 3)) it.size_kb = size == 0 ? 16 : size == 1 ? 32 : 64;
                            } else if (it.kind == SlotKind::Mapper) {
                                static const int kMapperSizes[] = {64, 128, 256, 512, 1024};
                                static const char *kMapperNames[] = {"64 KB", "128 KB", "256 KB", "512 KB", "1024 KB"};
                                int index = 1;
                                for (int i = 0; i < 5; ++i)
                                    if (kMapperSizes[i] == it.size_kb) index = i;
                                if (ImGui::Combo("##mapper", &index, kMapperNames, 5)) it.size_kb = kMapperSizes[index];
                            }
                            ImGui::PopID();
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::Separator();
                if (ImGui::Button("Padrao")) {
                    slot_edit = DefaultLayout(current);
                    slot_message.clear();
                }
                ImGui::SameLine();
                if (ImGui::Button("Aplicar e reiniciar")) {
                    std::string check_error;
                    if (!ValidateLayout(slot_edit, check_error)) {
                        slot_message = check_error;
                    } else {
                        MachineConfig next = snapshot();
                        next.layout = slot_edit;
                        next.layout_set = true;
                        reboot(next);
                        slot_message = disk_message;
                    }
                }
                if (!slot_message.empty()) ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "%s", slot_message.c_str());
            }
            ImGui::End();
        }

        if (show_keys) {
            ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Teclado", &show_keys)) {
                ImGui::TextWrapped(
                    "Mapeamento posicional (layout US): letras, numeros e simbolos vao para a mesma tecla do MSX.\n\n"
                    "Shift = SHIFT   Ctrl = CTRL   Alt esq. = GRAPH   Alt dir. = CODE   Caps Lock = CAPS\n"
                    "Enter, Espaco, Backspace (BS), Tab, Esc, setas, Home, Ins, Del\n"
                    "End = SELECT   Pause = STOP   F1-F5 = F1-F5\n"
                    "Teclado numerico = teclado numerico do MSX\n"
                    "Joystick (porta A): setas, Z ou Espaco = botao A, X = botao B; gamepads: ver menu Joystick\n"
                    "F11 = tela cheia");
            }
            ImGui::End();
        }

        ImGui::Render();
        int display_w = 0, display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        float clear_r = 0.05f, clear_g = 0.05f, clear_b = 0.07f;
        msxdisk::gui::ModernClearColor(ui_theme == 0, &clear_r, &clear_g, &clear_b);
        glClearColor(clear_r, clear_g, clear_b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    if (options.autoquit_frames > 0) {
        // Modo de validacao (--frames): relata o que o dispositivo de audio fez.
        std::printf("audio: %s -- %llu amostras tocadas, %llu underruns, %llu descartadas\n", audio_status.c_str(),
                    static_cast<unsigned long long>(audio_out.consumed_samples()),
                    static_cast<unsigned long long>(audio_out.underruns()),
                    static_cast<unsigned long long>(audio_out.dropped_samples()));
    }
    audio_out.Stop();
    glDeleteTextures(1, &texture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    {
        std::string save_error;
        if (!machine->SaveSram(save_error)) std::fprintf(stderr, "fwmsx: %s\n", save_error.c_str());
    }
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return exit_code;
}

} // namespace machine::gui
