// fwMSX -- o editor de textos como PROGRAMA de janela (standalone): `fwMSX.exe --edit [arquivo [arquivo2]]` / `--view arquivo`.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver editor_window.h. Uma janela do sistema (GLFW + OpenGL3) com o tema e a fonte do fwMSX,
// ocupada pelo `EditorPanel` (o mesmo componente que o menu "Editor" do emulador abre como janela flutuante).
#include "editor_window.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../../msxdisk/gui/style.h"
#include "editor_panel.h"

namespace ed {

namespace {

void GlfwError(int code, const char *desc) {
    std::fprintf(stderr, "fwmsx editor: GLFW erro %d: %s\n", code, desc);
}

} // namespace

int RunEditorWindow(const std::vector<std::string> &args, const std::string &argv0, bool as_view) {
    namespace fs = std::filesystem;
    std::vector<std::string> files;
    bool view = as_view;
    std::string cfg_dir;
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string &a = args[i];
        if (a == "--view") view = true;
        else if (a == "--config" && i + 1 < args.size()) cfg_dir = fs::path(args[++i]).parent_path().string();
        else if (a == "--keys" || a == "--size") ++i; // opcoes do --dump (so' terminal)
        else if (!a.empty() && a[0] != '-') files.push_back(a);
    }

    glfwSetErrorCallback(GlfwError);
    if (!glfwInit()) {
        std::fprintf(stderr, "fwmsx: falha ao inicializar o GLFW\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    int start_w = 1100, start_h = 720;
    int wx = 0, wy = 0, ww = 0, wh = 0;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &wx, &wy, &ww, &wh);
    if (ww > 0 && wh > 0) {
        start_w = std::min(start_w, ww * 9 / 10);
        start_h = std::min(start_h, wh * 85 / 100);
    }
    GLFWwindow *window = glfwCreateWindow(start_w, start_h, "fwEdit", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "fwmsx: falha ao criar a janela (GLFW)\n");
        glfwTerminate();
        return 1;
    }
    if (ww > 0) glfwSetWindowPos(window, wx + (ww - start_w) / 2, wy + 50);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    msxdisk::gui::LoadModernFont(io);     // a fonte da interface, a mesma do fwMSX
    gui::EditorPanel panel;
    panel.LoadFont(io, 17.0f);            // a fonte de largura fixa do texto
    msxdisk::gui::ApplyModernStyle(true); // o tema do fwMSX (escuro)
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    std::error_code ec;
    const std::string exe_dir = fs::absolute(fs::path(argv0), ec).parent_path().string();
    panel.Configure(cfg_dir.empty() ? exe_dir : cfg_dir, fs::current_path(ec).string());
    std::string err;
    for (const std::string &f : files) {
        if (!panel.app().OpenTab(f, err, view)) {
            if (view) {
                std::fprintf(stderr, "fwmsx --view: %s\n", err.c_str());
                return 1;
            }
            panel.app().NewTab(); // arquivo que ainda nao existe: abre vazio com esse nome
            panel.app().Ed().SetName(f);
        }
    }
    if (files.size() >= 2) panel.app().ToggleSplit(); // dois arquivos: lado a lado, como a "2nd window" do NE

    std::string last_title;
    while (!panel.app().QuitRequested()) {
        glfwWaitEventsTimeout(0.25); // acorda para piscar o cursor
        if (glfwWindowShouldClose(window)) {
            glfwSetWindowShouldClose(window, GLFW_FALSE);
            panel.app().RequestExit();   // pergunta por cada arquivo alterado
            if (panel.app().QuitRequested()) break;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        panel.DrawFullscreen();

        const Editor &e = panel.app().Ed();
        std::string title = "fwEdit - " + (e.Name().empty() ? std::string("Sem nome") : fs::path(e.Name()).filename().string());
        if (e.Modified()) title += " *";
        if (e.ReadOnly()) title += " [somente leitura]";
        if (title != last_title) {
            glfwSetWindowTitle(window, title.c_str());
            last_title = title;
        }

        ImGui::Render();
        int dw = 0, dh = 0;
        glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        float r = 0.1f, g = 0.1f, b = 0.12f;
        msxdisk::gui::ModernClearColor(true, &r, &g, &b);
        glClearColor(r, g, b, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

} // namespace ed
