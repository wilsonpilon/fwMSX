// fwMSX -- despachante de comandos da ponte de controle. Ver command.h e doc/control-spec.md.
#include "command.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>

#include "../common/version.h"

namespace control {

namespace {

Result Ok(std::string text = std::string()) { return Result{true, std::move(text)}; }
Result Err(std::string text) { return Result{false, std::move(text)}; }

std::string Hex(unsigned value, int digits) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*X", digits, value);
    return buf;
}

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// "A"/"B" (qualquer caixa) -> 0/1; -1 se invalido.
int DriveIndex(const std::string &s) {
    const std::string l = Lower(s);
    if (l == "a" || l == "a:") return 0;
    if (l == "b" || l == "b:") return 1;
    return -1;
}

const char *kHelp =
    "comandos: help version status reset pause resume step [n] type <texto> peek <end> [n] "
    "poke <end> <v>... regs cart <arq|-> [mapper] disk <A|B> <arq> eject <A|B> tape <arq|eject|rewind> "
    "state <save|load> <arq> screenshot <arq> quit";

} // namespace

bool Commander::ParseNumber(const std::string &text, uint32_t &value) {
    if (text.empty()) return false;
    std::string s = text;
    int base = 10;
    if (s[0] == '$') {
        base = 16;
        s.erase(0, 1);
    } else if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16;
        s.erase(0, 2);
    } else if (s.size() > 1 && (s.back() == 'h' || s.back() == 'H')) {
        base = 16;
        s.pop_back();
    }
    if (s.empty()) return false;
    char *end = nullptr;
    const unsigned long v = std::strtoul(s.c_str(), &end, base);
    if (*end != '\0' || v > 0xFFFFFFFFul) return false;
    value = static_cast<uint32_t>(v);
    return true;
}

std::vector<std::string> Commander::Tokenize(const std::string &line, std::string &error) {
    std::vector<std::string> tokens;
    std::string cur;
    bool in_token = false;
    bool in_quotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (in_quotes) {
            if (c == '"') {
                in_quotes = false;
            } else if (c == '\\' && i + 1 < line.size() && line[i + 1] == '"') {
                // So' \" e' escape: o resto fica literal, senao "C:\temp" viraria "C:<tab>emp".
                cur += '"';
                ++i;
            } else {
                cur += c;
            }
        } else if (c == '"') {
            in_quotes = true;
            in_token = true;
        } else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (in_token) {
                tokens.push_back(cur);
                cur.clear();
                in_token = false;
            }
        } else {
            cur += c;
            in_token = true;
        }
    }
    if (in_quotes) {
        error = "aspas sem fechar";
        return {};
    }
    if (in_token) tokens.push_back(cur);
    return tokens;
}

Result Commander::Execute(const std::string &line) {
    std::string tokenize_error;
    const std::vector<std::string> t = Tokenize(line, tokenize_error);
    if (!tokenize_error.empty()) return Err(tokenize_error);
    if (t.empty()) return Ok();

    const std::string cmd = Lower(t[0]);
    machine::Machine &m = host_.machine();

    if (cmd == "help") return Ok(kHelp);

    if (cmd == "version") {
        return Ok(std::string(FWMSX_NAME) + " " + std::to_string(FWMSX_VERSION_MAJOR) + "." + std::to_string(FWMSX_VERSION_MINOR) +
                  "." + std::to_string(FWMSX_VERSION_PATCH));
    }

    if (cmd == "status") {
        std::string s = "frame=" + std::to_string(m.frame_count()) + " paused=" + (host_.paused() ? "1" : "0") +
                        " screen=" + std::to_string(m.vdp_state().scr_mode) + " pc=" + Hex(m.cpu().state().pc.w, 4) +
                        " typing=" + (typing() ? "1" : "0") + " cart=" + (m.cart_info().empty() ? "-" : "\"" + m.cart_info() + "\"");
        for (int d = 0; d < 2; ++d) {
            if (!m.has_disk_interface()) break;
            s += std::string(" disk") + static_cast<char>('A' + d) + "=" + (m.disk(d).loaded() ? "\"" + m.disk(d).path() + "\"" : "-");
        }
        s += std::string(" tape=") + (m.tape().inserted() ? "\"" + m.tape().path() + "\"" : "-");
        return Ok(s);
    }

    if (cmd == "reset") {
        m.Reset();
        return Ok();
    }
    if (cmd == "pause") {
        host_.set_paused(true);
        return Ok();
    }
    if (cmd == "resume") {
        host_.set_paused(false);
        return Ok();
    }

    if (cmd == "step") {
        uint32_t n = 1;
        if (t.size() > 1 && (!ParseNumber(t[1], n) || n == 0 || n > 3600)) return Err("step: use um numero de quadros entre 1 e 3600");
        if (!host_.paused()) return Err("step: so' com a maquina pausada (use pause antes)");
        for (uint32_t i = 0; i < n; ++i) {
            m.RunFrame();
            Tick();
        }
        return Ok("frame=" + std::to_string(m.frame_count()));
    }

    if (cmd == "type") {
        if (t.size() < 2) return Err("type: informe o texto (use \\n para ENTER)");
        std::string text;
        for (size_t i = 1; i < t.size(); ++i) text += (i > 1 ? " " : "") + t[i];
        // Aqui (e so' aqui, nos caminhos a barra e' literal): \n vira ENTER e \\ vira uma barra.
        for (size_t i = 0; i + 1 < text.size(); ++i) {
            if (text[i] != '\\') continue;
            if (text[i + 1] == 'n') text.replace(i, 2, "\n");
            else if (text[i + 1] == '\\') text.erase(i, 1);
        }
        std::vector<TypedKey> keys;
        for (const char c : text) {
            TypedKey k{};
            if (c == '\n') {
                k.key = "enter";
            } else if (!machine::Machine::KeysForChar(c, k.key, k.shift)) {
                return Err(std::string("type: o caractere '") + c + "' nao tem tecla no MSX");
            }
            keys.push_back(k);
        }
        for (const TypedKey &k : keys) typing_.push_back(k);
        return Ok(std::to_string(keys.size()) + " teclas");
    }

    if (cmd == "peek") {
        uint32_t addr = 0, count = 1;
        if (t.size() < 2 || !ParseNumber(t[1], addr) || addr > 0xFFFF) return Err("peek: use peek <endereco 0-FFFF> [quantidade]");
        if (t.size() > 2 && (!ParseNumber(t[2], count) || count == 0 || count > 4096)) return Err("peek: quantidade entre 1 e 4096");
        std::string out;
        for (uint32_t i = 0; i < count; ++i) {
            if (i) out += ' ';
            out += Hex(m.ReadMemory(static_cast<uint16_t>((addr + i) & 0xFFFF)), 2);
        }
        return Ok(out);
    }

    if (cmd == "poke") {
        uint32_t addr = 0;
        if (t.size() < 3 || !ParseNumber(t[1], addr) || addr > 0xFFFF) return Err("poke: use poke <endereco 0-FFFF> <valor> [valor...]");
        std::vector<uint8_t> values;
        for (size_t i = 2; i < t.size(); ++i) {
            uint32_t v = 0;
            if (!ParseNumber(t[i], v) || v > 0xFF) return Err("poke: valor invalido '" + t[i] + "' (0-FF)");
            values.push_back(static_cast<uint8_t>(v));
        }
        for (size_t i = 0; i < values.size(); ++i) m.WriteMemory(static_cast<uint16_t>((addr + i) & 0xFFFF), values[i]);
        return Ok();
    }

    if (cmd == "regs") {
        const Z80State &s = m.cpu().state();
        return Ok("AF=" + Hex(s.af.w, 4) + " BC=" + Hex(s.bc.w, 4) + " DE=" + Hex(s.de.w, 4) + " HL=" + Hex(s.hl.w, 4) +
                  " IX=" + Hex(s.ix.w, 4) + " IY=" + Hex(s.iy.w, 4) + " SP=" + Hex(s.sp.w, 4) + " PC=" + Hex(s.pc.w, 4));
    }

    if (cmd == "cart") {
        if (t.size() < 2) return Err("cart: use cart <arquivo|-> [mapper]");
        std::string error;
        const std::string path = (t[1] == "-") ? std::string() : t[1];
        if (!host_.LoadCartridge(path, t.size() > 2 ? t[2] : std::string(), error)) return Err(error);
        return Ok();
    }

    if (cmd == "disk" || cmd == "eject") {
        if (!m.has_disk_interface()) return Err("sem interface de disquete (inicie com --disk ou troque o layout)");
        const bool is_eject = cmd == "eject";
        if (t.size() != (is_eject ? 2u : 3u)) return Err(is_eject ? "eject: use eject <A|B>" : "disk: use disk <A|B> <arquivo>");
        const int drive = DriveIndex(t[1]);
        if (drive < 0) return Err("drive invalido (use A ou B)");
        if (is_eject) {
            m.EjectDisk(drive);
            return Ok();
        }
        std::string error;
        if (!m.InsertDisk(drive, t[2], error)) return Err(error);
        return Ok();
    }

    if (cmd == "tape") {
        if (t.size() != 2) return Err("tape: use tape <arquivo|eject|rewind>");
        if (t[1] == "eject") {
            m.EjectTape();
            return Ok();
        }
        if (t[1] == "rewind") {
            m.RewindTape();
            return Ok();
        }
        std::string error;
        if (!m.InsertTape(t[1], error)) return Err(error);
        return Ok();
    }

    if (cmd == "state") {
        if (t.size() != 3 || (t[1] != "save" && t[1] != "load")) return Err("state: use state <save|load> <arquivo>");
        std::string error;
        if (t[1] == "save") {
            if (!m.SaveState(t[2], error)) return Err(error);
            return Ok();
        }
        if (!m.LoadState(t[2], error)) return Err(error);
        return Ok(m.state_warning().empty() ? std::string() : "aviso: " + m.state_warning());
    }

    if (cmd == "screenshot") {
        if (t.size() != 2) return Err("screenshot: use screenshot <arquivo>");
        std::string error;
        if (!host_.SaveScreenshot(t[1], error)) return Err(error);
        return Ok(t[1]);
    }

    if (cmd == "quit") {
        host_.quit();
        return Ok();
    }

    return Err("comando desconhecido '" + t[0] + "' (use help)");
}

void Commander::Tick() {
    machine::Machine &m = host_.machine();
    if (key_phase_ == 0) {
        if (typing_.empty()) return;
        current_ = typing_.front();
        typing_.pop_front();
        if (current_.shift) m.KeyDown("shift");
        m.KeyDown(current_.key);
        key_phase_ = 1;
        key_ticks_ = 3;
        return;
    }
    if (--key_ticks_ > 0) return;
    if (key_phase_ == 1) {
        m.KeyUp(current_.key);
        if (current_.shift) m.KeyUp("shift");
        key_phase_ = 2;
        key_ticks_ = 3;
    } else {
        key_phase_ = 0;
    }
}

} // namespace control
