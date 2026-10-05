// fwMSX -- janela do emulador. Ver emu_window.h e doc/machine-spec.md.
#include "emu_window.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>
#include "../../audio/audio_output.h"
#include "../../msxdisk/gui/file_dialog.h"
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

    // Janela inicial: 3x o MSX1 (768x576) ou 2x o MSX2 (512x384 -> 1024x768).
    const int start_w = machine->is_msx2() ? 1024 : Machine::kFrameWidth * 3;
    const int start_h = (machine->is_msx2() ? 768 : Machine::kFrameHeight * 3) + 20;
    GLFWwindow *window = glfwCreateWindow(start_w, start_h, "fwMSX", nullptr, nullptr);
    if (window == nullptr) {
        std::fprintf(stderr, "fwmsx: falha ao criar a janela (GLFW)\n");
        glfwTerminate();
        return 1;
    }
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
    ImGui::StyleColorsDark();
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
            if (fsz.width != frame_size.width || fsz.height != frame_size.height) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fsz.width, fsz.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            } else {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fsz.width, fsz.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Maquina")) {
                if (ImGui::MenuItem("Reset")) machine->Reset();
                if (ImGui::MenuItem("Pausar", nullptr, &paused) && paused) audio_out.Flush();
                if (ImGui::MenuItem("Soltar todas as teclas")) {
                    machine->ReleaseAllKeys();
                    deferred_releases.clear();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Sair")) glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Exibir")) {
                ImGui::MenuItem("Escala inteira", nullptr, &integer_scale);
                if (ImGui::MenuItem("Tela cheia", "F11", fullscreen)) g_input.toggle_fullscreen = true;
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
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Disco")) {
                if (!machine->has_disk_interface()) {
                    ImGui::TextDisabled("Sem interface de disquete.");
                    ImGui::TextDisabled("Inicie com --disk <arq.dsk> ou --disk-interface.");
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
                    ImGui::TextDisabled("As gravacoes do MSX-DOS vao direto para o arquivo.");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Joystick")) {
                ImGui::TextDisabled("Porta A: setas + Z/Espaco (A) + X (B)");
                for (int port = 0; port < 2; ++port) {
                    ImGui::Text("Gamepad -> porta %c: %s", 'A' + port, pad_names[port].empty() ? "(nenhum)" : pad_names[port].c_str());
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Ajuda")) {
                ImGui::MenuItem("Teclado", nullptr, &show_keys);
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
            const float disp_w = static_cast<float>(frame_size.width);
            const float disp_h = static_cast<float>(frame_size.height * frame_size.y_scale);
            float scale = std::min(avail.x / disp_w, avail.y / disp_h);
            if (integer_scale && scale >= 1.0f) scale = std::floor(scale);
            if (scale < 0.1f) scale = 0.1f;
            const ImVec2 size(disp_w * scale, disp_h * scale);
            ImGui::SetCursorPos(ImVec2((avail.x - size.x) * 0.5f, (avail.y - size.y) * 0.5f));
            ImGui::Image(static_cast<ImTextureID>(texture), size);
        }
        ImGui::End();
        ImGui::PopStyleVar(2);

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
        glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
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
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return exit_code;
}

} // namespace machine::gui
