// fwMSX -- depurador basico do emulador. Ver debugger.h e doc/debug-spec.md.
#include "debugger.h"

#include <algorithm>
#include <cstdio>

#include "../asm/dbg_range.h"

namespace dbg {

namespace {
// A fila nao cresce sem fim quando ninguem consome (modo terminal, execucao sem cliente).
constexpr size_t kMaxEvents = 256;

std::string Hex(unsigned v, int digits) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*X", digits, v);
    return buf;
}
} // namespace

int Debugger::AddBreak(uint16_t addr) {
    for (const Breakpoint &b : breaks_) {
        if (b.addr == addr) return b.id;
    }
    Breakpoint b;
    b.id = next_id_++;
    b.addr = addr;
    breaks_.push_back(b);
    return b.id;
}

bool Debugger::RemoveBreak(uint16_t addr) {
    const auto it = std::remove_if(breaks_.begin(), breaks_.end(), [&](const Breakpoint &b) { return b.addr == addr; });
    const bool found = it != breaks_.end();
    breaks_.erase(it, breaks_.end());
    return found;
}

int Debugger::AddWatch(uint16_t addr, uint16_t len, bool on_read, bool on_write) {
    for (Watchpoint &w : watches_) {
        if (w.addr == addr) {
            w.len = len;
            w.on_read = on_read;
            w.on_write = on_write;
            return w.id;
        }
    }
    Watchpoint w;
    w.id = next_id_++;
    w.addr = addr;
    w.len = len;
    w.on_read = on_read;
    w.on_write = on_write;
    watches_.push_back(w);
    return w.id;
}

bool Debugger::RemoveWatch(uint16_t addr) {
    const auto it = std::remove_if(watches_.begin(), watches_.end(), [&](const Watchpoint &w) { return w.addr == addr; });
    const bool found = it != watches_.end();
    watches_.erase(it, watches_.end());
    return found;
}

bool Debugger::CheckPc(uint16_t pc) {
    if (skip_once_) {
        skip_once_ = false;
        return false;
    }
    if (until_active_ && pc == until_addr_) {
        stopped_ = true;
        until_active_ = false;
        last_stop_ = "until";
        PostEvent("! until pc=" + Hex(pc, 4));
        return true;
    }
    for (Breakpoint &b : breaks_) {
        if (b.addr == pc) {
            ++b.hits;
            stopped_ = true;
            until_active_ = false;
            last_stop_ = "break";
            PostEvent("! break id=" + std::to_string(b.id) + " pc=" + Hex(pc, 4) + " hits=" + std::to_string(b.hits));
            return true;
        }
    }
    return false;
}

void Debugger::OnAccess(uint16_t addr, uint8_t value, bool write) {
    if (watch_hit_) return;
    for (Watchpoint &w : watches_) {
        if (write ? !w.on_write : !w.on_read) continue;
        if (!dbg_addr_in_range(addr, w.addr, w.len)) continue;
        ++w.hits;
        watch_hit_ = true;
        watch_id_ = w.id;
        watch_addr_ = addr;
        watch_value_ = value;
        watch_write_ = write;
        watch_pc_ = inst_pc_;
        return;
    }
}

bool Debugger::TakeWatchHit(uint16_t pc_after) {
    if (!watch_hit_) return false;
    watch_hit_ = false;
    stopped_ = true;
    until_active_ = false;
    last_stop_ = "watch";
    PostEvent("! watch id=" + std::to_string(watch_id_) + " addr=" + Hex(watch_addr_, 4) + " rw=" + (watch_write_ ? "w" : "r") +
              " value=" + Hex(watch_value_, 2) + " pc=" + Hex(watch_pc_, 4) + " next=" + Hex(pc_after, 4));
    return true;
}

void Debugger::PostEvent(const std::string &line) {
    if (events_.size() >= kMaxEvents) events_.erase(events_.begin());
    events_.push_back(line);
}

std::vector<std::string> Debugger::TakeEvents() {
    std::vector<std::string> out;
    out.swap(events_);
    return out;
}

void Debugger::Reset() {
    breaks_.clear();
    watches_.clear();
    stopped_ = false;
    skip_once_ = false;
    until_active_ = false;
    watch_hit_ = false;
    events_.clear();
    last_stop_.clear();
}

} // namespace dbg
