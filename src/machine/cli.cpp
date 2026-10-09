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
#include "../term/term_app.h"
#include "../romdb/service.h"
#include "../romdb/core/hash.h"
#include "../romdb/store/romdb.h"
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
    if (s == "msxdos2") { out = MEMMAP_MAPPER_MSXDOS2; return true; }
    return false;
}

// Numero do mapper do fMSX original (CARTS.SHA/cart_mappers do romdb, 0-5 --
// ver resource/fMSX/fMSX/MSX.h, MAP_GEN8..MAP_ASCII16) para o enum publico
// MemMapMapperType (que tem MEMMAP_MAPPER_NONE=0 antes de GEN8): soma 1.
// Qualquer outro valor (desconhecido, ou um mapper que o fwMSX nao suporta)
// devolve false -- o chamador cai no fallback por tamanho/heuristica de
// sempre (machine.cpp::LoadRomCell -> memmap::GuessMapper()), sem mudanca.
bool FmsxMapperToMemMap(int n, MemMapMapperType &out) {
    if (n < 0 || n > 5) return false;
    out = static_cast<MemMapMapperType>(n + 1);
    return true;
}

// Se o usuario NAO escolheu um mapper a dedo para --cart, consulta o banco
// de ROMs (fwmsx --romdb cartsha, ja' importado previamente) pelo SHA-1 do
// arquivo -- so' uma CAMADA OPCIONAL antes da heuristica por tamanho/
// conteudo de sempre (ver doc/romdb-spec.md, secao 8: "o emulador ainda nao
// usa o banco para escolher o mapper"). Falha em qualquer etapa (sem banco,
// sem arquivo, SHA-1 desconhecido) e' SILENCIOSA -- isto e' um atalho, nao
// um requisito; o fallback de sempre continua intacto em machine.cpp.
void TryMapperFromRomDb(MachineConfig &config, const std::string &argv0) {
    const romdb::RomPaths paths = romdb::DefaultRomPaths(argv0);
    romdb::RomDb db;
    std::string db_error;
    if (!romdb::OpenRomDb(paths, db, db_error)) return;
    std::vector<uint8_t> cart_bytes;
    if (!romdb::ReadWholeFile(config.cart_path, cart_bytes)) return;
    const std::string sha1 = romdb::Sha1Hex(cart_bytes.data(), cart_bytes.size());
    const int fmsx_mapper = db.CartMapper(sha1);
    MemMapMapperType mapper{};
    if (FmsxMapperToMemMap(fmsx_mapper, mapper)) {
        config.cart_mapper = mapper;
        std::cout << "fwmsx --msx: mapper do cartucho pelo banco de ROMs (CARTS.SHA)" << std::endl;
    }
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

// --slot P:S=tipo[:arg] (ver doc/slots-spec.md, secao 4): tipo = empty, rom, sub,
// ram, mapper, disk, fmpac. Para rom/sub/disk/fmpac o argumento e' o arquivo; para
// ram/mapper, o tamanho em KB. Devolve false com a razao.
bool ApplySlotSpec(const std::string &spec, machine::SlotLayout &layout, std::string &error) {
    const size_t eq = spec.find('=');
    const size_t colon = spec.find(':');
    if (eq == std::string::npos || colon == std::string::npos || colon > eq) {
        error = "--slot: use P:S=tipo[:arg], ex.: --slot 3:0=ram:16";
        return false;
    }
    const int p = std::atoi(spec.substr(0, colon).c_str());
    const int sec = std::atoi(spec.substr(colon + 1, eq - colon - 1).c_str());
    if (p < 0 || p > 3 || sec < 0 || sec > 3) {
        error = "--slot: slot " + spec.substr(0, eq) + " invalido (primario 0-3, secundario 0-3)";
        return false;
    }
    const std::string rest = spec.substr(eq + 1);
    const size_t c2 = rest.find(':');
    const std::string kind = rest.substr(0, c2);
    const std::string arg = c2 == std::string::npos ? std::string() : rest.substr(c2 + 1);

    machine::SlotItem &item = layout.cell[p][sec];
    item = machine::SlotItem{};
    if (kind == "empty") {
        // celula vazia
    } else if (kind == "rom") {
        item.kind = machine::SlotKind::Rom;
        item.path = arg;
        item.page = (p == 0 && sec == 0) ? 0 : 1;
    } else if (kind == "sub") {
        item.kind = machine::SlotKind::SubRom;
        item.path = arg;
    } else if (kind == "ram") {
        item.kind = machine::SlotKind::Ram;
        item.size_kb = std::atoi(arg.c_str());
    } else if (kind == "mapper") {
        item.kind = machine::SlotKind::Mapper;
        item.size_kb = std::atoi(arg.c_str());
    } else if (kind == "disk") {
        item.kind = machine::SlotKind::Disk;
        item.path = arg;
    } else if (kind == "fmpac") {
        item.kind = machine::SlotKind::FmPac;
        item.path = arg;
    } else {
        error = "--slot: tipo desconhecido '" + kind + "' (use empty, rom, sub, ram, mapper, disk ou fmpac)";
        return false;
    }
    return true;
}

} // namespace

int RunMachineCommand(const std::vector<std::string> &args, const std::string &argv0) {
    MachineConfig config;
    config.fmpac_rom_path = "auto"; // FM-PAC ligado por padrao (--no-fmpac desliga)
    int frames = 0;
    int ctl_port = -1; // -1 = sem ponte de controle
    bool use_term = false;
    std::string shot_path;
    std::string keys;
    bool mute = false;
    bool vdplog = false;
    bool dump_text = false;
    bool fm_stat = false;
    bool fmpac_explicit = false;
    bool cart_mapper_explicit = false; // true so' quando o usuario ESCOLHEU um mapper (nao "auto"/omitido)
    std::vector<std::string> slot_specs;
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
        } else if (a == "--slot") {
            const std::string *v = need("P:S=tipo[:arg]");
            if (!v) return 2;
            slot_specs.push_back(*v);
        } else if (a == "--disk-acesso") {
            // Controladora de disco: pela memoria (padrao, DISK.ROM) ou pelas portas.
            const std::string *v = need("mem ou porta");
            if (!v) return 2;
            if (*v == "mem") config.disk_access = DiskAccess::Memory;
            else if (*v == "porta") config.disk_access = DiskAccess::Port;
            else {
                std::cerr << "fwmsx --msx: --disk-acesso aceita mem ou porta" << std::endl;
                return 2;
            }
        } else if (a == "--disk-porta") {
            const std::string *v = need("uma porta, ex.: D0h ou 208");
            if (!v) return 2;
            config.disk_port = static_cast<int>(std::strtol(v->c_str(), nullptr, 0));
            if (v->size() > 1 && (v->back() == 'h' || v->back() == 'H')) {
                config.disk_port = static_cast<int>(std::strtol(v->substr(0, v->size() - 1).c_str(), nullptr, 16));
            }
            if (config.disk_port < 0 || config.disk_port > 0xFB) {
                std::cerr << "fwmsx --msx: --disk-porta precisa estar entre 00h e FBh (4 portas livres)" << std::endl;
                return 2;
            }
        } else if (a == "--disk-formato") {
            // 180, 360 ou 720 KB: ss525 (5 1/4, face simples), ds525 (5 1/4, face dupla),
            // ss35 (3 1/2, face simples), ds35 (3 1/2, face dupla); auto aceita os tres tamanhos.
            const std::string *v = need("auto, ss525, ds525, ss35 ou ds35");
            if (!v) return 2;
            if (*v == "auto") config.disk_format = fdc::DiskFormat::Auto;
            else if (*v == "ss525") config.disk_format = fdc::DiskFormat::Ss525Sd180;
            else if (*v == "ds525") config.disk_format = fdc::DiskFormat::Ds525Dd360;
            else if (*v == "ss35") config.disk_format = fdc::DiskFormat::Ss35Dd360;
            else if (*v == "ds35") config.disk_format = fdc::DiskFormat::Ds35Dd720;
            else {
                std::cerr << "fwmsx --msx: --disk-formato aceita auto, ss525, ds525, ss35 ou ds35" << std::endl;
                return 2;
            }
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
                              << "' (use auto, gen8, gen16, konami5, konami4, ascii8, ascii16 ou msxdos2)" << std::endl;
                    return 2;
                } else {
                    cart_mapper_explicit = true;
                    ++i;
                }
            }
        } else if (a == "--term") {
            use_term = true; // maquina dentro do terminal (FTXUI) -- doc/term-spec.md
        } else if (a == "--ctl-port") {
            // Ponte de controle externo (doc/control-spec.md): 0 = o sistema escolhe a porta.
            const std::string *v = need("um numero de porta (0 = automatica)");
            if (!v) return 2;
            ctl_port = std::atoi(v->c_str());
            if (ctl_port < 0 || ctl_port > 65535) {
                std::cerr << "fwmsx --msx: --ctl-port precisa estar entre 0 e 65535" << std::endl;
                return 2;
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
        } else if (a == "--fita") {
            const std::string *v = need("um arquivo .cas, .tsx ou .tzx");
            if (!v) return 2;
            config.tape_path = *v;
        } else if (a == "--fita-modo") {
            // Rapido (gancho de BIOS, sem som, padrao) ou normal (pulsos de
            // verdade pela porta, com o barulho do carregamento) -- ver doc/tape-spec.md.
            const std::string *v = need("rapido ou normal");
            if (!v) return 2;
            if (*v == "rapido") config.tape_mode = tape::TapeMode::Fast;
            else if (*v == "normal") config.tape_mode = tape::TapeMode::Normal;
            else {
                std::cerr << "fwmsx --msx: --fita-modo aceita rapido ou normal" << std::endl;
                return 2;
            }
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

    if (!config.cart_path.empty() && config.cart_mapper == MEMMAP_MAPPER_NONE && !cart_mapper_explicit) {
        TryMapperFromRomDb(config, argv0);
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
    if (!slot_specs.empty()) {
        // Layout explicito pela linha de comando: parte do padrao e troca as celulas pedidas.
        config.layout = machine::DefaultLayout(config);
        config.layout_set = true;
        for (const std::string &spec : slot_specs) {
            std::string slot_error;
            if (!ApplySlotSpec(spec, config.layout, slot_error)) {
                std::cerr << "fwmsx --msx: " << slot_error << std::endl;
                return 2;
            }
        }
    }

    if (!shot_path.empty() || !keys.empty()) {
        std::string error;
        std::unique_ptr<Machine> m = Machine::Create(config, error);
        if (!m) {
            std::cerr << "fwmsx --msx: " << error << std::endl;
            return 1;
        }
        if (!m->cart_info().empty()) std::cout << "cartucho: " << m->cart_info() << std::endl;
        if (m->has_disk_interface()) {
            if (m->disk_access_is_port()) {
                std::cout << "disco: controladora por portas (base " << std::hex << config.disk_port << std::dec << "h), formato " << fdc::FormatName(m->disk_format());
            } else {
                std::cout << "disco: interface de disquete ligada (DISK.ROM em 3:1), formato " << fdc::FormatName(m->disk_format());
            }
            for (int d = 0; d < 2; ++d)
                if (m->disk(d).loaded()) std::cout << ", " << static_cast<char>('A' + d) << ": " << m->disk(d).path();
            std::cout << std::endl;
        }
        if (m->tape().inserted()) {
            std::cout << "fita: " << m->tape().path() << " (modo "
                      << (m->tape().mode() == tape::TapeMode::Fast ? "rapido" : "normal") << ")" << std::endl;
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

    if (use_term) {
        term::TermOptions topts;
        topts.machine = config;
        topts.audio = !mute;
        topts.ctl_port = ctl_port;
        std::error_code ec;
        topts.exe_dir = fs::absolute(fs::path(argv0), ec).parent_path().string();
        return term::RunTerm(topts);
    }

    gui::WindowOptions options;
    options.machine = config;
    options.rom_root = romdb::DefaultRomPaths(argv0).root;
    options.autoquit_frames = frames;
    options.audio = !mute;
    options.ctl_port = ctl_port;
    {
        std::error_code ec;
        options.exe_dir = fs::absolute(fs::path(argv0), ec).parent_path().string();
    }
    return gui::RunEmulatorWindow(options);
}

} // namespace machine
