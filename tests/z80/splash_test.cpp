// Teste do splash (src/splash), do leitor de PNG e das configuracoes gravadas em arquivo
// (src/common/settings) -- ver doc/splash-spec.md.
#include <miniz.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/common/settings.h"
#include "../../src/splash/png_reader.h"
#include "../../src/splash/splash.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

std::string TempPath(const char *name) {
    const char *t = std::getenv("TEMP");
    return (t ? std::string(t) : std::string("/tmp")) + "/" + name;
}

// PNG de verdade (RGB, 8 bits) gerado pelo miniz, para o leitor ler de volta.
std::vector<uint8_t> MakePng(int w, int h, const std::vector<uint8_t> &rgb) {
    size_t len = 0;
    void *png = tdefl_write_image_to_png_file_in_memory(rgb.data(), w, h, 3, &len);
    std::vector<uint8_t> out(static_cast<const uint8_t *>(png), static_cast<const uint8_t *>(png) + len);
    mz_free(png);
    return out;
}

} // namespace

int main() {
    using splash::Flag;
    using splash::Mode;

    // ---- politica: quando mostrar ------------------------------------------------------------------
    check(splash::ShouldShow(Mode::Window, Flag::Default, true), "janela: mostra por padrao (configuracao ligada)");
    check(!splash::ShouldShow(Mode::Window, Flag::Default, false), "janela: nao mostra se a configuracao desligou");
    check(splash::ShouldShow(Mode::Window, Flag::ForceOn, false), "janela: --splash mostra mesmo com a configuracao desligada");
    check(!splash::ShouldShow(Mode::Window, Flag::ForceOff, true), "janela: --no-splash nao mostra");
    check(!splash::ShouldShow(Mode::Terminal, Flag::Default, true), "terminal (--cli/--tui/--term): nao mostra por padrao, nem com a configuracao ligada");
    check(splash::ShouldShow(Mode::Terminal, Flag::ForceOn, false), "terminal: so' com --splash");
    check(!splash::ShouldShow(Mode::Terminal, Flag::ForceOff, true), "terminal: --no-splash nao mostra");
    check(!splash::ShouldShow(Mode::Command, Flag::Default, true) && !splash::ShouldShow(Mode::Command, Flag::ForceOn, true),
          "--cmd: nunca mostra, nem com --splash");

    // ---- --splash / --no-splash saem da lista de argumentos -----------------------------------------
    {
        std::vector<std::string> a = {"--msx2", "--splash", "--cart", "x.rom"};
        const Flag f = splash::ExtractFlag(a);
        check(f == Flag::ForceOn && a == std::vector<std::string>({"--msx2", "--cart", "x.rom"}), "ExtractFlag: --splash e' retirado");
        std::vector<std::string> b = {"--no-splash", "--mute"};
        check(splash::ExtractFlag(b) == Flag::ForceOff && b == std::vector<std::string>({"--mute"}), "ExtractFlag: --no-splash e' retirado");
        std::vector<std::string> c = {"--splash", "--no-splash"};
        check(splash::ExtractFlag(c) == Flag::ForceOff && c.empty(), "ExtractFlag: --no-splash vence --splash");
        std::vector<std::string> d = {"--mute"};
        check(splash::ExtractFlag(d) == Flag::Default && d.size() == 1, "ExtractFlag: sem nenhum dos dois = padrao");
    }

    // ---- leitor de PNG ------------------------------------------------------------------------------
    {
        const int w = 5, h = 3;
        std::vector<uint8_t> rgb;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                rgb.push_back(static_cast<uint8_t>(x * 40));
                rgb.push_back(static_cast<uint8_t>(y * 80));
                rgb.push_back(static_cast<uint8_t>(255 - x * y * 10));
            }
        splash::Image img;
        std::string err;
        check(splash::DecodePng(MakePng(w, h, rgb), img, err) && img.width == w && img.height == h && img.rgba.size() == 5u * 3u * 4u,
              "DecodePng: dimensoes de um PNG RGB (" + err + ")");
        bool same = true;
        for (int i = 0; i < w * h; ++i) {
            same = same && img.rgba[i * 4] == rgb[i * 3] && img.rgba[i * 4 + 1] == rgb[i * 3 + 1] && img.rgba[i * 4 + 2] == rgb[i * 3 + 2] &&
                   img.rgba[i * 4 + 3] == 255;
        }
        check(same, "DecodePng: os pixels voltam iguais, com alfa 255");
        std::vector<uint8_t> bad = {1, 2, 3};
        check(!splash::DecodePng(bad, img, err) && !err.empty(), "DecodePng: lixo e' recusado com mensagem");
        std::vector<uint8_t> cut = MakePng(w, h, rgb);
        cut.resize(cut.size() / 2);
        check(!splash::DecodePng(cut, img, err), "DecodePng: PNG truncado e' recusado");
    }

    // ---- o splash de verdade (so' se o arquivo existir ao lado do executavel / no repositorio) --------
#ifdef FWMSX_SOURCE_DIR
    {
        const std::string file = std::string(FWMSX_SOURCE_DIR) + "/dist/fwMSX-splash.png";
        if (std::filesystem::exists(file)) {
            splash::Image img;
            std::string err;
            check(splash::LoadPngFile(file, img, err) && img.width == 1376 && img.height == 768, "splash real: fwMSX-splash.png decodifica 1376x768 (" + err + ")");
            const std::string frame = splash::RenderHalfBlocks(img, 100, 30);
            check(!frame.empty() && frame.find("\x1b[38;2;") != std::string::npos && frame.find("\xE2\x96\x80") != std::string::npos,
                  "terminal: o splash vira meios blocos com cor de 24 bits");
            int lines = 0;
            for (char c : frame) lines += c == '\n';
            check(lines <= 30 && lines >= 10, "terminal: cabe em 30 linhas (" + std::to_string(lines) + ")");
        } else {
            std::printf("[SKIP] %s nao encontrado\n", file.c_str());
        }
        check(!splash::FindSplashFile(std::string(FWMSX_SOURCE_DIR) + "/dist").empty(), "FindSplashFile acha o arquivo ao lado do executavel");
    }
#endif
    {
        // Imagem sintetica: metade de cima vermelha, metade de baixo azul -> uma celula com as duas cores.
        splash::Image img;
        img.width = 4;
        img.height = 4;
        img.rgba.assign(4 * 4 * 4, 255);
        for (int y = 0; y < 4; ++y)
            for (int x = 0; x < 4; ++x) {
                uint8_t *p = &img.rgba[(y * 4 + x) * 4];
                p[0] = y < 2 ? 255 : 0;
                p[1] = 0;
                p[2] = y < 2 ? 0 : 255;
            }
        const std::string s = splash::RenderHalfBlocks(img, 10, 2);
        check(s.find("38;2;255;0;0") != std::string::npos && s.find("48;2;0;0;255") != std::string::npos,
              "RenderHalfBlocks: cima vermelha (frente) e baixo azul (fundo)");
        check(splash::RenderHalfBlocks(img, 2, 1).empty(), "RenderHalfBlocks: terminal pequeno demais devolve vazio");
    }

    // ---- configuracoes -------------------------------------------------------------------------------
    {
        const std::string path = TempPath("fwmsx_test.cfg");
        std::filesystem::remove(path);
        settings::Settings def = settings::Load(path);
        check(def.splash && def.theme == 0 && def.font == 1 && def.frame, "settings: arquivo ausente = padroes (splash ligado)");
        settings::Settings s;
        s.splash = false;
        s.theme = 1;
        s.font = 2;
        s.frame = false;
        check(settings::Save(path, s), "settings: grava");
        const settings::Settings r = settings::Load(path);
        check(!r.splash && r.theme == 1 && r.font == 2 && !r.frame, "settings: le de volta o que gravou");
        {
            std::ofstream o(path);
            o << "# comentario\nsplash = 0\ntheme=99\nfont=-5\nlixo\nfrase=sem importancia\n";
        }
        const settings::Settings c = settings::Load(path);
        check(!c.splash && c.theme == 1 && c.font == 0, "settings: espacos, comentarios e lixo ignorados; valores fora da faixa viram o limite");
        check(settings::PathFor("C:/fw") == "C:/fw/fwmsx.cfg" && settings::PathFor("C:/fw/") == "C:/fw/fwmsx.cfg" && settings::PathFor("") == "fwmsx.cfg",
              "settings: caminho do arquivo ao lado do executavel");
        std::filesystem::remove(path);
    }

    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
