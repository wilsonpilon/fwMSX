//
// msxdisk (fwMSX): estilo visual moderno da GUI -- ver style.h.
//

#include "style.h"

#include <filesystem>

#include <imgui.h>

namespace msxdisk::gui {

namespace {

ImVec4 Rgb(int r, int g, int b, float a = 1.0f) {
    return ImVec4(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, a);
}

// Acento (azul) igual nos dois temas -- e o que muda entre claro/escuro e
// o fundo/contraste em volta dele.
constexpr int kAccentR = 66, kAccentG = 133, kAccentB = 244;

void ApplySpacing(ImGuiStyle &style) {
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 6.0f;

    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
    style.IndentSpacing = 18.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;

    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
}

} // namespace

void ApplyModernStyle(bool dark) {
    ImGuiStyle &style = ImGui::GetStyle();
    ApplySpacing(style);
    ImVec4 *colors = style.Colors;

    const ImVec4 accent = Rgb(kAccentR, kAccentG, kAccentB);
    const ImVec4 accent_hover = Rgb(kAccentR + 20, kAccentG + 20, 255);
    const ImVec4 accent_active = Rgb(kAccentR - 20, kAccentG - 20, kAccentB - 10);

    if (dark) {
        colors[ImGuiCol_WindowBg] = Rgb(18, 18, 23);
        colors[ImGuiCol_ChildBg] = Rgb(24, 24, 30);
        colors[ImGuiCol_PopupBg] = Rgb(28, 28, 35);
        colors[ImGuiCol_Border] = Rgb(50, 50, 60);
        colors[ImGuiCol_BorderShadow] = Rgb(0, 0, 0, 0);

        colors[ImGuiCol_FrameBg] = Rgb(35, 35, 43);
        colors[ImGuiCol_FrameBgHovered] = Rgb(45, 45, 55);
        colors[ImGuiCol_FrameBgActive] = Rgb(55, 55, 68);

        colors[ImGuiCol_TitleBg] = Rgb(18, 18, 23);
        colors[ImGuiCol_TitleBgActive] = Rgb(24, 24, 30);
        colors[ImGuiCol_TitleBgCollapsed] = Rgb(18, 18, 23);
        colors[ImGuiCol_MenuBarBg] = Rgb(22, 22, 28);

        colors[ImGuiCol_ScrollbarBg] = Rgb(18, 18, 23);
        colors[ImGuiCol_ScrollbarGrab] = Rgb(60, 60, 70);
        colors[ImGuiCol_ScrollbarGrabHovered] = Rgb(75, 75, 88);
        colors[ImGuiCol_ScrollbarGrabActive] = Rgb(90, 90, 105);

        colors[ImGuiCol_CheckMark] = accent_hover;
        colors[ImGuiCol_SliderGrab] = accent;
        colors[ImGuiCol_SliderGrabActive] = accent_active;

        colors[ImGuiCol_Button] = Rgb(45, 45, 55);
        colors[ImGuiCol_ButtonHovered] = accent;
        colors[ImGuiCol_ButtonActive] = accent_active;

        colors[ImGuiCol_Header] = Rgb(kAccentR, kAccentG, kAccentB, 0.55f);
        colors[ImGuiCol_HeaderHovered] = Rgb(kAccentR, kAccentG, kAccentB, 0.75f);
        colors[ImGuiCol_HeaderActive] = accent;

        colors[ImGuiCol_Separator] = Rgb(50, 50, 60);
        colors[ImGuiCol_SeparatorHovered] = accent;
        colors[ImGuiCol_SeparatorActive] = accent_active;

        colors[ImGuiCol_Tab] = Rgb(28, 28, 35);
        colors[ImGuiCol_TabHovered] = accent;
        colors[ImGuiCol_TabActive] = Rgb(40, 40, 50);

        colors[ImGuiCol_Text] = Rgb(228, 228, 232);
        colors[ImGuiCol_TextDisabled] = Rgb(130, 130, 140);
        colors[ImGuiCol_TextSelectedBg] = Rgb(kAccentR, kAccentG, kAccentB, 0.4f);

        colors[ImGuiCol_TableHeaderBg] = Rgb(30, 30, 38);
        colors[ImGuiCol_TableRowBg] = Rgb(24, 24, 30);
        colors[ImGuiCol_TableRowBgAlt] = Rgb(28, 28, 35);
        colors[ImGuiCol_TableBorderStrong] = Rgb(50, 50, 60);
        colors[ImGuiCol_TableBorderLight] = Rgb(40, 40, 48);
    } else {
        colors[ImGuiCol_WindowBg] = Rgb(246, 246, 248);
        colors[ImGuiCol_ChildBg] = Rgb(255, 255, 255);
        colors[ImGuiCol_PopupBg] = Rgb(255, 255, 255);
        colors[ImGuiCol_Border] = Rgb(222, 222, 226);
        colors[ImGuiCol_BorderShadow] = Rgb(0, 0, 0, 0);

        colors[ImGuiCol_FrameBg] = Rgb(233, 233, 237);
        colors[ImGuiCol_FrameBgHovered] = Rgb(222, 230, 250);
        colors[ImGuiCol_FrameBgActive] = Rgb(205, 220, 250);

        colors[ImGuiCol_TitleBg] = Rgb(255, 255, 255);
        colors[ImGuiCol_TitleBgActive] = Rgb(240, 240, 244);
        colors[ImGuiCol_TitleBgCollapsed] = Rgb(255, 255, 255);
        colors[ImGuiCol_MenuBarBg] = Rgb(240, 240, 244);

        colors[ImGuiCol_ScrollbarBg] = Rgb(246, 246, 248);
        colors[ImGuiCol_ScrollbarGrab] = Rgb(200, 200, 208);
        colors[ImGuiCol_ScrollbarGrabHovered] = Rgb(180, 180, 190);
        colors[ImGuiCol_ScrollbarGrabActive] = Rgb(160, 160, 172);

        colors[ImGuiCol_CheckMark] = accent;
        colors[ImGuiCol_SliderGrab] = accent;
        colors[ImGuiCol_SliderGrabActive] = accent_active;

        colors[ImGuiCol_Button] = Rgb(233, 233, 237);
        colors[ImGuiCol_ButtonHovered] = accent_hover;
        colors[ImGuiCol_ButtonActive] = accent_active;

        colors[ImGuiCol_Header] = Rgb(kAccentR, kAccentG, kAccentB, 0.30f);
        colors[ImGuiCol_HeaderHovered] = Rgb(kAccentR, kAccentG, kAccentB, 0.45f);
        colors[ImGuiCol_HeaderActive] = accent;

        colors[ImGuiCol_Separator] = Rgb(222, 222, 226);
        colors[ImGuiCol_SeparatorHovered] = accent;
        colors[ImGuiCol_SeparatorActive] = accent_active;

        colors[ImGuiCol_Tab] = Rgb(233, 233, 237);
        colors[ImGuiCol_TabHovered] = accent_hover;
        colors[ImGuiCol_TabActive] = Rgb(210, 225, 250);

        colors[ImGuiCol_Text] = Rgb(30, 30, 34);
        colors[ImGuiCol_TextDisabled] = Rgb(150, 150, 155);
        colors[ImGuiCol_TextSelectedBg] = Rgb(kAccentR, kAccentG, kAccentB, 0.35f);

        colors[ImGuiCol_TableHeaderBg] = Rgb(236, 236, 240);
        colors[ImGuiCol_TableRowBg] = Rgb(255, 255, 255);
        colors[ImGuiCol_TableRowBgAlt] = Rgb(246, 246, 249);
        colors[ImGuiCol_TableBorderStrong] = Rgb(222, 222, 226);
        colors[ImGuiCol_TableBorderLight] = Rgb(235, 235, 238);
    }
}

void ModernClearColor(bool dark, float *r, float *g, float *b) {
    if (dark) {
        *r = 18.0f / 255.0f;
        *g = 18.0f / 255.0f;
        *b = 23.0f / 255.0f;
    } else {
        *r = 246.0f / 255.0f;
        *g = 246.0f / 255.0f;
        *b = 248.0f / 255.0f;
    }
}

void LoadModernFont(ImGuiIO &io) {
    static const char *kCandidates[] = {
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\calibri.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
    };
    for (const char *path : kCandidates) {
        std::error_code ec;
        if (std::filesystem::exists(path, ec)) {
            if (io.Fonts->AddFontFromFileTTF(path, 18.0f) != nullptr) return;
        }
    }
    io.Fonts->AddFontDefault();
}

} // namespace msxdisk::gui
