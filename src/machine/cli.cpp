#include "cli.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "../vdp/cpp/ppm_writer.h"
#include "gui/emu_window.h"
#include "machine.h"

namespace machine {

namespace {

namespace fs = std::filesystem;

bool ParseMapper(const std::string &s, MemMapMapperType &out) {
    if (s == "gen8") { out = MEMMAP_MAPPER_GEN8; return true; }
    if (s == "gen16") { out = MEMMAP_MAPPER_GEN16; return true; }
    if (s == "konami5") { out = MEMMAP_MAPPER_KONAMI5; return true; }
    if (s == "konami4") { out = MEMMAP_MAPPER_KONAMI4; return true; }
    if (s == "ascii8") { out = MEMMAP_MAPPER_ASCII8; return true; }
    if (s == "ascii16") { out = MEMMAP_MAPPER_ASCII16; return true; }
    return false;
}

// BIOS padrao: resource/fMSX/ROMs/MSX.ROM a partir do diretorio de trabalho,
// do diretorio do executavel ou de um dos pais deles (dist/ fica dentro do
// repositorio); ou um MSX.ROM solto ao lado do executavel.
std::string FindDefaultBios(const std::string &argv0, const char *rom_name) {
    std::vector<fs::path> roots;
    std::error_code ec;
    roots.push_back(fs::current_path(ec));
    const fs::path exe_dir = fs::absolute(fs::path(argv0), ec).parent_path();
    roots.push_back(exe_dir);
    for (fs::path p = exe_dir; p.has_parent_path() && p != p.parent_path(); p = p.parent_path()) roots.push_back(p.parent_path());

    for (const fs::path &root : roots) {
        for (const std::string &rel : {std::string("resource/fMSX/ROMs/") + rom_name, std::string(rom_name)}) {
            const fs::path candidate = root / rel;
            if (fs::is_regular_file(candidate, ec)) return candidate.string();
        }
    }
    return "";
}

} // namespace

int RunMachineCommand(const std::vector<std::string> &args, const std::string &argv0) {
    MachineConfig config;
    int frames = 0;
    std::string shot_path;
    std::string keys;
    bool mute = false;
    bool vdplog = false;
    int wait_frames = 60;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string &a = args[i];
        auto need = [&](const char *what) -> const std::string * {
            if (i + 1 >= args.size()) {
                std::cerr << "fwmsx --msx: " << a << " requer " << what << std::endl;
                return nullptr;
            }
            return &args[++i];
        };
        if (a == "--bios") {
            const std::string *v = need("um arquivo");
            if (!v) return 2;
            config.bios_path = *v;
        } else if (a == "--msx2") {
            config.model = Model::MSX2;
        } else if (a == "--ext") {
            const std::string *v = need("um arquivo");
            if (!v) return 2;
            config.ext_rom_path = *v;
        } else if (a == "--cart") {
            const std::string *v = need("um arquivo");
            if (!v) return 2;
            config.cart_path = *v;
            if (i + 1 < args.size() && args[i + 1].rfind("--", 0) != 0) {
                if (args[i + 1] == "auto") {
                    ++i; // detecta sozinho (o padrao)
                } else if (!ParseMapper(args[i + 1], config.cart_mapper)) {
                    std::cerr << "fwmsx --msx: mapper desconhecido: '" << args[i + 1]
                              << "' (use auto, gen8, gen16, konami5, konami4, ascii8 ou ascii16)" << std::endl;
                    return 2;
                } else {
                    ++i;
                }
            }
        } else if (a == "--frames") {
            const std::string *v = need("um numero");
            if (!v) return 2;
            frames = std::atoi(v->c_str());
            if (frames <= 0) {
                std::cerr << "fwmsx --msx: --frames precisa de um numero positivo" << std::endl;
                return 2;
            }
        } else if (a == "--shot") {
            const std::string *v = need("um arquivo .ppm");
            if (!v) return 2;
            shot_path = *v;
        } else if (a == "--disk" || a == "--diska") {
            const std::string *v = need("um arquivo .dsk");
            if (!v) return 2;
            config.disk_a = *v;
        } else if (a == "--diskb") {
            const std::string *v = need("um arquivo .dsk");
            if (!v) return 2;
            config.disk_b = *v;
        } else if (a == "--disk-interface") {
            config.disk_interface = true;
        } else if (a == "--diskrom") {
            const std::string *v = need("um arquivo");
            if (!v) return 2;
            config.disk_rom_path = *v;
        } else if (a == "--wait") {
            const std::string *v = need("um numero de quadros");
            if (!v) return 2;
            wait_frames = std::atoi(v->c_str());
        } else if (a == "--vdplog") {
            vdplog = true;
        } else if (a == "--mute") {
            mute = true;
        } else if (a == "--keys") {
            const std::string *v = need("um texto");
            if (!v) return 2;
            keys = *v;
        } else {
            std::cerr << "fwmsx --msx: argumento desconhecido: '" << a << "'" << std::endl;
            return 2;
        }
    }

    if (config.bios_path.empty()) {
        const char *rom_name = config.model == Model::MSX2 ? "MSX2.ROM" : "MSX.ROM";
        config.bios_path = FindDefaultBios(argv0, rom_name);
        if (config.bios_path.empty()) {
            std::cerr << "fwmsx --msx: BIOS nao encontrada (esperava resource/fMSX/ROMs/" << rom_name << "); use --bios <arquivo>" << std::endl;
            return 1;
        }
    }

    // Sem janela: roda os quadros pedidos, opcionalmente digita um texto, e
    // salva a tela.
    if (!shot_path.empty() || !keys.empty()) {
        std::string error;
        std::unique_ptr<Machine> m = Machine::Create(config, error);
        if (!m) {
            std::cerr << "fwmsx --msx: " << error << std::endl;
            return 1;
        }
        if (!m->cart_info().empty()) std::cout << "cartucho: " << m->cart_info() << std::endl;
        if (m->has_disk_interface()) {
            std::cout << "disco: interface de disquete ligada (DISK.ROM em 3:1)";
            for (int d = 0; d < 2; ++d)
                if (m->disk(d).loaded()) std::cout << ", " << static_cast<char>('A' + d) << ": " << m->disk(d).path();
            std::cout << std::endl;
        }
        // --vdplog: mostra, a cada quadro, os registradores do VDP que mudaram
        uint8_t last_regs[64] = {};
        auto run_frame = [&]() {
            m->RunFrame();
            if (!vdplog) return;
            const VdpState &vs = m->vdp_state();
            for (int r = 0; r < 47; ++r) {
                if (vs.regs[r] != last_regs[r]) {
                    std::printf("quadro %llu: R#%d %02X -> %02X (SCREEN %d, PC=%04X)\n", static_cast<unsigned long long>(m->frame_count()), r,
                                last_regs[r], vs.regs[r], vs.scr_mode, m->cpu().pc());
                    last_regs[r] = vs.regs[r];
                }
            }
        };
        const int boot_frames = frames > 0 ? frames : 400;
        for (int i = 0; i < boot_frames; ++i) run_frame();
        for (char c : keys) {
            std::string key;
            bool shift = false;
            if (!Machine::KeysForChar(c, key, shift)) {
                std::cerr << "fwmsx --msx: caractere nao suportado em --keys: '" << c << "'" << std::endl;
                return 2;
            }
            if (shift) m->KeyDown("shift");
            m->KeyDown(key);
            for (int i = 0; i < 6; ++i) run_frame();
            m->KeyUp(key);
            if (shift) m->KeyUp("shift");
            for (int i = 0; i < 6; ++i) run_frame();
        }
        if (!keys.empty()) {
            for (int i = 0; i < wait_frames; ++i) run_frame();
        }
        if (shot_path.empty()) return 0;

        std::vector<uint32_t> rgba;
        const FrameSize fs_size = m->RenderFrame(rgba);
        // Repete cada linha y_scale vezes para a imagem sair com a proporcao certa
        // (no MSX2 a imagem de 512 de largura tem pixels "altos").
        const int out_h = fs_size.height * fs_size.y_scale;
        std::vector<VdpRgb888> rgb(static_cast<size_t>(fs_size.width) * out_h);
        for (int y = 0; y < out_h; ++y) {
            const uint32_t *src = rgba.data() + static_cast<size_t>(y / fs_size.y_scale) * fs_size.width;
            VdpRgb888 *dst = rgb.data() + static_cast<size_t>(y) * fs_size.width;
            for (int x = 0; x < fs_size.width; ++x) {
                dst[x].r = static_cast<uint8_t>(src[x] & 0xFF);
                dst[x].g = static_cast<uint8_t>((src[x] >> 8) & 0xFF);
                dst[x].b = static_cast<uint8_t>((src[x] >> 16) & 0xFF);
            }
        }
        std::string werr;
        if (!vdp::WritePpm(shot_path, rgb.data(), fs_size.width, out_h, &werr)) {
            std::cerr << "fwmsx --msx: " << werr << std::endl;
            return 1;
        }
        std::cout << "tela salva em " << shot_path << " (" << m->frame_count() << " quadros, SCREEN " << static_cast<int>(m->vdp_state().scr_mode)
                  << ")" << std::endl;
        return 0;
    }

    gui::WindowOptions options;
    options.machine = config;
    options.autoquit_frames = frames;
    options.audio = !mute;
    return gui::RunEmulatorWindow(options);
}

} // namespace machine
