#include "rom_guess.h"

namespace memmap {

MemMapMapperType GuessMapper(const uint8_t *data, std::size_t size) {
    // Na ordem do enum (empate -> menor indice): Gen8, Gen16, Konami5, Konami4, ASCII8, ASCII16.
    enum { kGen8, kGen16, kKonami5, kKonami4, kAscii8, kAscii16, kCount };
    int count[kCount];
    for (int &c : count) c = 1;
    count[kGen8] += 1;   // 8KB generico e' o padrao
    count[kAscii8] -= 1; // ASCII 16KB preferido ao 8KB

    for (std::size_t i = 0; i + 2 < size; ++i) {
        // LD (nnnn),A = 32h lo hi (o fMSX compara os 3 bytes juntos; aqui, opcode + endereco).
        if (data[i] != 0x32) continue;
        const unsigned addr = static_cast<unsigned>(data[i + 1]) | (static_cast<unsigned>(data[i + 2]) << 8);
        switch (addr) {
        case 0x5000: case 0x9000: case 0xB000: count[kKonami5]++; break;
        case 0x4000: case 0x8000: case 0xA000: count[kKonami4]++; break;
        case 0x6800: case 0x7800: count[kAscii8]++; break;
        case 0x6000:
            count[kKonami4]++;
            count[kAscii8]++;
            count[kAscii16]++;
            break;
        case 0x7000:
            count[kKonami5]++;
            count[kAscii8]++;
            count[kAscii16]++;
            break;
        case 0x77FF: count[kAscii16]++; break;
        default: break;
        }
    }

    int best = 0;
    for (int i = 1; i < kCount; ++i)
        if (count[i] > count[best]) best = i;

    switch (best) {
    case kGen16: return MEMMAP_MAPPER_GEN16;
    case kKonami5: return MEMMAP_MAPPER_KONAMI5;
    case kKonami4: return MEMMAP_MAPPER_KONAMI4;
    case kAscii8: return MEMMAP_MAPPER_ASCII8;
    case kAscii16: return MEMMAP_MAPPER_ASCII16;
    default: return MEMMAP_MAPPER_GEN8;
    }
}

} // namespace memmap
