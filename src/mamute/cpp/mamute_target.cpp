// fwMSX -- Mamute: o enderecamento estendido do SUPER-X (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver mamute_target.h.
#include "mamute_target.h"

#include "mamute_util.h"

namespace mamute {

using util::Hex;
using util::ParseHexN;
using util::Trim;
using util::Upper;

Vars::Slot *Vars::Find(char name) {
    const char u = static_cast<char>(std::toupper(static_cast<unsigned char>(name)));
    if (u >= '0' && u <= '3') return &slots[u - '0'];
    if (u == 'B') return &slots[4];
    if (u == 'E') return &slots[5];
    if (u == 'S') return &slots[6];
    return nullptr;
}

void Vars::StoreBase(const SxAddr &a) {
    slots[0].has = true;
    slots[0].v = a;
}

std::string TargetSuffix(const Target &t) {
    if (t.vram) return "#V";
    if (!t.is_explicit) return "";
    return t.sub == 0 ? "#" + std::to_string(t.slot) : "#" + std::to_string(t.slot) + "-" + std::to_string(t.sub);
}

int TargetDigits(const Target &t) { return t.vram ? 5 : 4; }

uint32_t TargetMax(IMemory &mem, const Target &t) {
    if (t.vram) {
        const size_t n = mem.VramSize();
        return n ? static_cast<uint32_t>(n - 1) : 0;
    }
    return 0xFFFF;
}

uint32_t TargetWrap(IMemory &mem, const Target &t, int64_t addr) {
    const int64_t size = static_cast<int64_t>(TargetMax(mem, t)) + 1;
    int64_t m = addr % size;
    if (m < 0) m += size;
    return static_cast<uint32_t>(m);
}

uint8_t TRead(IMemory &mem, const Target &t, uint32_t addr) {
    if (t.vram) return mem.ReadVram(addr);
    if (!t.is_explicit) return mem.Read(static_cast<uint16_t>(addr & 0xFFFF));
    return mem.ReadSub(t.slot, t.sub, static_cast<uint16_t>(addr & 0xFFFF));
}

bool TWrite(IMemory &mem, const Target &t, uint32_t addr, uint8_t value) {
    if (t.vram) return mem.WriteVram(addr, value);
    if (!t.is_explicit) return mem.Write(static_cast<uint16_t>(addr & 0xFFFF), value);
    return mem.WriteSub(t.slot, t.sub, static_cast<uint16_t>(addr & 0xFFFF), value);
}

namespace {

// o que vem depois do "#": V/4, S/5, ou "slot" / "slot-sub"
bool ParseSuffix(const std::string &suffix, Target &t) {
    const std::string u = Upper(suffix);
    if (u == "V" || u == "4") {
        t.vram = true;
        return true;
    }
    if (u == "S" || u == "5") {
        t.is_explicit = false;
        return true;
    }
    const size_t dash = u.find('-');
    const std::string slot_tok = dash == std::string::npos ? u : u.substr(0, dash);
    const std::string sub_tok = dash == std::string::npos ? std::string() : u.substr(dash + 1);
    if (dash != std::string::npos && sub_tok.empty()) return false; // "3-" e' malformado, nao "sem sub-slot"
    if (slot_tok.size() != 1 || slot_tok[0] < '0' || slot_tok[0] > '3') return false;
    t.is_explicit = true;
    t.slot = slot_tok[0] - '0';
    if (!sub_tok.empty()) {
        if (sub_tok.size() != 1 || sub_tok[0] < '0' || sub_tok[0] > '3') return false;
        t.sub = sub_tok[0] - '0';
    } else {
        t.sub = 0; // sem sub-slot = sub-slot 0 (o Mamute nao simula o registrador de sub-slot ativo)
    }
    return true;
}

} // namespace

bool ParseSx(IMemory &mem, const Vars &vars, const std::string &token_in, SxAddr &out) {
    out = SxAddr{};
    const std::string token = Trim(token_in);
    if (token.empty()) return false;
    if (token[0] == '@') { // a variavel devolve o endereco e o alvo gravados nela
        if (token.size() != 2) return false;
        const Vars::Slot *s = vars.Find(token[1]);
        if (!s || !s->has) return false;
        out = s->v;
        return true;
    }
    const size_t hash = token.find('#');
    std::string addr_tok = token;
    if (hash != std::string::npos) {
        addr_tok = token.substr(0, hash);
        const std::string suffix = token.substr(hash + 1);
        if (suffix.empty() || !ParseSuffix(suffix, out.t)) return false;
    }
    if (out.t.vram) {
        if (!ParseHexN(addr_tok, out.addr, 5)) return false;
        return out.addr < mem.VramSize();
    }
    return ParseHexN(addr_tok, out.addr, 4);
}

bool ParsePure(IMemory &mem, const Target &t, const std::string &token, uint32_t &out) {
    if (!ParseHexN(token, out, TargetDigits(t))) return false;
    return out <= TargetMax(mem, t);
}

std::string FormatSx(const SxAddr &a) { return Hex(a.addr, TargetDigits(a.t)) + TargetSuffix(a.t); }

std::string FormatRange(const SxAddr &s, uint32_t end) { return Hex(s.addr, TargetDigits(s.t)) + "-" + Hex(end, TargetDigits(s.t)) + TargetSuffix(s.t); }

} // namespace mamute
