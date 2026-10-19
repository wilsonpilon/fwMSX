// fwMSX -- Mamute: o lado ONLINE (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "mamute_online.h"

#include <cstdio>

namespace mamute {

bool MachineMemory::Write(uint16_t addr, uint8_t value) {
    const auto view = m_.memory().CurrentView();
    if (!view[addr >> 14].writable) return false; // ROM/vazio: nada muda, como num MSX de verdade
    m_.WriteMemory(addr, value);
    return true;
}

bool MachineMemory::WriteSlot(int slot, uint16_t addr, uint8_t value) {
    const int sec = Sec(slot);
    m_.memory().PokeSlot(slot & 3, sec, addr, value);
    return m_.memory().PeekSlot(slot & 3, sec, addr) == value; // PokeSlot ignora o que nao e' gravavel
}

bool MachineMemory::WriteSub(int slot, int sub, uint16_t addr, uint8_t value) {
    const int sec = sub ? (sub & 3) : Sec(slot);
    m_.memory().PokeSlot(slot & 3, sec, addr, value);
    return m_.memory().PeekSlot(slot & 3, sec, addr) == value; // ROM/vazio ignoram a escrita: a leitura nao muda
}

bool MachineMemory::WriteVram(uint32_t addr, uint8_t value) { return m_.PokeVram(addr, value); }

CellType MachineMemory::Cell(int slot, int page) {
    const auto view = m_.memory().CurrentView();
    const auto &pv = view[static_cast<size_t>(page & 3)];
    if (pv.primary != (slot & 3)) return CellType::Unknown; // fora de vista a maquina nao diz o que ha'
    return pv.writable ? CellType::Ram : CellType::Rom;
}

std::array<int, 4> MachineMemory::PageMap() {
    const auto view = m_.memory().CurrentView();
    return {{view[0].primary, view[1].primary, view[2].primary, view[3].primary}};
}

size_t MachineMemory::VramSize() { return sizeof(m_.vdp_state().vram); }

uint8_t MachineMemory::ReadVram(uint32_t addr) { return addr < VramSize() ? m_.vdp_state().vram[addr] : 0xFF; }

std::string MachineExec::Go(bool has_addr, uint16_t addr, const std::vector<uint16_t> &bps) {
    if (has_addr) m_.cpu().set_pc(addr);
    std::string ids;
    for (const uint16_t b : bps) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%s#%d", ids.empty() ? "" : " ", m_.debugger().AddBreak(b));
        ids += buf;
    }
    m_.debugger().Resume();
    return "EXECUTANDO NA MAQUINA" + (ids.empty() ? std::string() : " - BREAKPOINTS " + ids) + " (use STOP/regs no console para ver onde parou)";
}

} // namespace mamute
