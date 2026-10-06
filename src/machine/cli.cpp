#include "cli.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "../vdp/cpp/ppm_writer.h"
#include "../vdp/core/vdp_state.h"
#include "../fm/cpp/fm_device.h"
#include "../psg/cpp/wav_writer.h"
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

// FMPAC.ROM (ROM de 16KB do FM-PAC, em resource/fMSX/): mesma busca da BIOS.
std::string FindDefaultFmpac(const std::string &argv0) {
    std::vector<fs::path> roots;
    std::error_code ec;
    roots.push_back(fs::current_path(ec));
    const fs::path exe_dir = fs::absolute(fs::path(argv0), ec).parent_path();
    roots.push_back(exe_dir);
    for (fs::path p = exe_dir; p.has_parent_path() && p != p.parent_path(); p = p.parent_path()) roots.push_back(p.parent_path());

    for (const fs::path &root : roots) {
        for (const std::string &rel : {std::string("resource/fMSX/FMPAC.ROM"), std::string("FMPAC.ROM")}) {
            const fs::path candidate = root / rel;
            if (fs::is_regular_file(candidate, ec)) return candidate.string();
        }
    }
    return "";
}

} // namespace

int RunMachineCommand(const std::vector<std::string> &args, const std::string &argv0) {
    MachineConfig config;
    config.fmpac_rom_path = "auto"; // FM-PAC ligado por padrao (--no-fmpac desliga)
    int frames = 0;
    std::string shot_path;
    std::string keys;
    bool mute = false;
    bool vdplog = false;
    bool dump_text = false;
    bool fm_stat = false;
    bool fmpac_explicit = false;
    std::string wav_path;
    std::vector<int16_t> wav_samples;
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
        } else if (a == "--msx2p") {
            config.model = Model::MSX2P;
        } else if (a == "--no-fmpac") {
            // Sem FM-PAC (o padrao liga o FM-PAC se o FMPAC.ROM for encontrado).
            config.fmpac_rom_path.clear();
            fmpac_explicit = false;
        } else if (a == "--fmpac") {
            // FM-PAC no slot 2:0. Sem arquivo, usa resource/fMSX/FMPAC.ROM.
            fmpac_explicit = true;
            if (i + 1 < args.size() && args[i + 1].rfind("--", 0) != 0) {
                config.fmpac_rom_path = args[++i];
            } else {
                config.fmpac_rom_path = "auto";
            }
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
        } else if (a == "--disk-ro") {
            config.disk_read_only = true;
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
        } else if (a == "--text") {
            dump_text = true;
        } else if (a == "--fmstat") {
            fm_stat = true;
        } else if (a == "--wav") {
            const std::string *v = need("um arquivo .wav");
            if (!v) return 2;
            wav_path = *v;
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

    if (config.fmpac_rom_path == "auto") {
        config.fmpac_rom_path = FindDefaultFmpac(argv0);
        if (config.fmpac_rom_path.empty() && !fmpac_explicit) {
            // Padrao: sem o FMPAC.ROM, a maquina sobe sem FM-PAC.
        } else if (config.fmpac_rom_path.empty()) {
            std::cerr << "fwmsx --msx: FMPAC.ROM nao encontrada (esperava resource/fMSX/FMPAC.ROM); use --fmpac <arquivo>" << std::endl;
            return 1;
        }
    }

    if (config.bios_path.empty()) {
        const char *rom_name = config.model == Model::MSX2P ? "MSX2P.ROM" : config.model == Model::MSX2 ? "MSX2.ROM" : "MSX.ROM";
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
            if (!wav_path.empty()) m->TakeLiveAudio(wav_samples);
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
        // --wav: a mistura de audio (PSG + SCC + FM) e' gravada quadro a quadro.
        if (!wav_path.empty()) m->EnableLiveAudio(true);
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
        if (!wav_path.empty()) {
            std::string werr;
            if (!psg::WriteWav(wav_path, wav_samples, psg::kSampleRate, werr)) {
                std::cerr << "fwmsx --msx: " << werr << std::endl;
                return 1;
            }
            std::cout << "audio salvo em " << wav_path << " (" << wav_samples.size() << " amostras)" << std::endl;
        }
        std::string save_error;
        if (!m->SaveSram(save_error)) std::cerr << "fwmsx --msx: " << save_error << std::endl;
        if (fm_stat) {
            // Estado do chip FM: canal por canal, a nota ligada e o timbre.
            const Ym2413State &fs = m->fm().state();
            std::cout << "FM ritmo: " << (((fs.reg[0x0E] >> 5) & 1) ? "LIGADO" : "desligado")
                      << " teclas " << static_cast<int>(fs.drum_keys) << std::endl;
            for (int c = 0; c < YM2413_CHANNELS; ++c) {
                const Ym2413Ch &ch = fs.ch[c];
                std::cout << "FM canal " << (c + 1) << ": nota " << (ch.key ? "LIGADA" : "parada")
                          << " timbre " << static_cast<int>(ch.inst) << " fnum " << ch.fnum
                          << " bloco " << static_cast<int>(ch.block) << " volume " << static_cast<int>(ch.vol)
                          << " env " << ch.op[1].env << std::endl;
            }
        }
        if (dump_text) {
            // Tela de texto (SCREEN 0: 40 colunas; SCREEN 1: 32), so' os caracteres imprimiveis.
            const VdpState &vs = m->vdp_state();
            const int cols = vs.scr_mode == 1 ? 32 : 40;
            for (int row = 0; row < 24; ++row) {
                std::string line;
                for (int col = 0; col < cols; ++col) {
                    const uint8_t c = vs.vram[(vs.chr_tab + row * cols + col) & (VDP_VRAM_SIZE - 1)];
                    line += (c >= 0x20 && c < 0x7F) ? static_cast<char>(c) : ' ';
                }
                while (!line.empty() && line.back() == ' ') line.pop_back();
                std::cout << line << std::endl;
            }
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
