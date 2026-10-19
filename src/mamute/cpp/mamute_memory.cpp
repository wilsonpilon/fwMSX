// fwMSX -- Mamute: memoria privada do modo offline (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "mamute_memory.h"

#include <algorithm>
#include <fstream>
#include <iterator>

namespace mamute {

OfflineMemory::OfflineMemory(const Config &cfg) { Reconfigure(cfg); }

std::string OfflineMemory::Reconfigure(const Config &cfg) {
    std::string warnings;
    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            blocks_[s][p].assign(0x4000, 0);
            types_[s][p] = cfg.cell[s][p];
            const CellType t = types_[s][p];
            if ((t == CellType::Rom || t == CellType::Basic) && !cfg.file[s][p].empty()) {
                std::ifstream f(cfg.file[s][p], std::ios::binary);
                if (!f) {
                    warnings += "AVISO: nao abriu " + cfg.file[s][p] + " (slot " + std::to_string(s) + " pagina " + std::to_string(p) + ")\n";
                    continue;
                }
                std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                // "half": o arquivo de 32 KB com BIOS+BASIC lado a lado; esta celula pega os ultimos 16 KB
                const size_t from = (cfg.half[s][p] && data.size() >= 0x8000) ? 0x4000 : 0;
                if (data.size() > from) {
                    const size_t n = std::min<size_t>(0x4000, data.size() - from);
                    std::copy(data.begin() + static_cast<std::ptrdiff_t>(from), data.begin() + static_cast<std::ptrdiff_t>(from + n), blocks_[s][p].begin());
                }
            }
        }
    }
    vram_.assign(static_cast<size_t>(cfg.vram_kb) * 1024, 0);
    Boot();
    return warnings;
}

void OfflineMemory::Boot() {
    int slot = 0;
    bool found = false;
    for (int s = 0; s < 4 && !found; ++s)
        for (int p = 0; p < 4 && !found; ++p)
            if (types_[s][p] == CellType::Ram) {
                slot = s;
                found = true;
            }
    map_ = {{slot, slot, slot, slot}};
}

bool OfflineMemory::Write(uint16_t addr, uint8_t value) {
    const int page = addr >> 14;
    if (types_[map_[page]][page] != CellType::Ram) return false;
    blocks_[map_[page]][page][addr & 0x3FFF] = value;
    return true;
}

bool OfflineMemory::WriteSlot(int slot, uint16_t addr, uint8_t value) {
    blocks_[slot & 3][(addr >> 14) & 3][addr & 0x3FFF] = value;
    return true;
}

bool OfflineMemory::SetPageMap(const std::array<int, 4> &map) {
    for (int s : map)
        if (s < 0 || s > 3) return false;
    map_ = map;
    return true;
}

bool OfflineMemory::WriteVram(uint32_t addr, uint8_t value) {
    if (addr >= vram_.size()) return false;
    vram_[addr] = value;
    return true;
}

} // namespace mamute
