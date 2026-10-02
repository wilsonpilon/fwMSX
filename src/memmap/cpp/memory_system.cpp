#include "memory_system.h"

#include <algorithm>
#include <cstdint>

// Implementado em src/memmap/fortran/rom_checksum.f90. Nome exposto via
// bind(c, name=...), entao nao ha' name mangling a considerar aqui (mesmo
// padrao ja usado para z80_build_flag_tables em src/z80/core/z80_tables.c).
extern "C" void rom_crc32(const uint8_t *data, int32_t length, uint32_t *crc_out);

namespace memmap {

namespace {
bool ValidSlotIndex(int primary, int secondary) {
    return primary >= 0 && primary < MEMMAP_PRIMARY_SLOTS && secondary >= 0 && secondary < MEMMAP_SECONDARY_SLOTS;
}

// Nome legivel do tipo de mapper (Fase 3) -- usado por Describe()/'slots'.
// "" para MEMMAP_MAPPER_NONE (ROM plana, Fase 2) e para Empty/Ram, onde o
// campo nao tem sentido.
const char *MapperName(MemMapMapperType mapper) {
    switch (mapper) {
        case MEMMAP_MAPPER_GEN8: return "Gen8";
        case MEMMAP_MAPPER_GEN16: return "Gen16";
        case MEMMAP_MAPPER_KONAMI5: return "Konami5";
        case MEMMAP_MAPPER_KONAMI4: return "Konami4";
        case MEMMAP_MAPPER_ASCII8: return "ASCII8";
        case MEMMAP_MAPPER_ASCII16: return "ASCII16";
        case MEMMAP_MAPPER_NONE:
        default:
            return "";
    }
}
} // namespace

MemorySystem::MemorySystem() { memmap_init(&state_); }

void MemorySystem::AllocateRam(int primary, int secondary, std::size_t size) {
    const std::size_t max_size = static_cast<std::size_t>(MEMMAP_PAGES) * MEMMAP_PAGE_SIZE;
    if (size > max_size) size = max_size;
    const std::size_t rounded = ((size + MEMMAP_CHUNK_SIZE - 1) / MEMMAP_CHUNK_SIZE) * MEMMAP_CHUNK_SIZE;

    auto buffer = std::make_unique<uint8_t[]>(rounded);
    std::fill(buffer.get(), buffer.get() + rounded, uint8_t{0});

    memmap_attach(&state_, primary, secondary, buffer.get(), rounded, MEMMAP_KIND_RAM, /*writable=*/1);
    owned_buffers_.push_back(std::move(buffer));
}

void MemorySystem::AllocateMapperRam(int primary, int secondary, int segments) {
    if (!ValidSlotIndex(primary, secondary)) return;
    int n = 4;
    while (n < segments && n < 256) n <<= 1;
    const std::size_t total = static_cast<std::size_t>(n) * 0x4000;

    auto buffer = std::make_unique<uint8_t[]>(total);
    std::fill(buffer.get(), buffer.get() + total, uint8_t{0});

    // Anexa como RAM de 64KB (os 4 segmentos iniciais) e depois remapeia as
    // paginas para os segmentos 3,2,1,0 do fMSX.
    memmap_attach(&state_, primary, secondary, buffer.get(), 0x10000, MEMMAP_KIND_RAM, /*writable=*/1);
    mapper_base_[primary][secondary] = buffer.get();
    mapper_segments_[primary][secondary] = n;
    owned_buffers_.push_back(std::move(buffer));
    for (int page = 0; page < 4; ++page) SetMapperSegment(primary, secondary, page, 3 - page);
}

void MemorySystem::SetMapperSegment(int primary, int secondary, int page, int segment) {
    if (!ValidSlotIndex(primary, secondary) || page < 0 || page > 3) return;
    const int n = mapper_segments_[primary][secondary];
    if (n == 0) return;
    uint8_t *base = mapper_base_[primary][secondary] + static_cast<std::size_t>(segment & (n - 1)) * 0x4000;
    memmap_remap_ram_chunk(&state_, primary, secondary, page * 2, base);
    memmap_remap_ram_chunk(&state_, primary, secondary, page * 2 + 1, base + 0x2000);
}

int MemorySystem::MapperSegments(int primary, int secondary) const {
    return ValidSlotIndex(primary, secondary) ? mapper_segments_[primary][secondary] : 0;
}

bool MemorySystem::LoadRom(int primary, int secondary, const uint8_t *data, std::size_t size, std::string *error,
                            MemMapMapperType mapper) {
    if (!ValidSlotIndex(primary, secondary)) {
        if (error) *error = "combinacao de slot invalida: " + std::to_string(primary) + ":" + std::to_string(secondary);
        return false;
    }

    // Tamanho maximo depende do modo: ROM plana (Fase 2) cabe inteira no
    // espaco de enderecos do Z80 (64KB); MegaROM (Fase 3) pode ter varios
    // bancos de 8KB num buffer maior, mas a mascara de banco
    // (rom_bank_mask, um uint8_t em SlotState) so representa ate 256
    // bancos = 2MB -- ver slot_state.h.
    const std::size_t plain_max = static_cast<std::size_t>(MEMMAP_PAGES) * MEMMAP_PAGE_SIZE;
    const std::size_t megarom_max = static_cast<std::size_t>(256) * MEMMAP_CHUNK_SIZE;
    const std::size_t max_size = (mapper == MEMMAP_MAPPER_NONE) ? plain_max : megarom_max;

    if (size < MEMMAP_CHUNK_SIZE || size > max_size || (size % MEMMAP_CHUNK_SIZE) != 0) {
        if (error) {
            *error = "tamanho de ROM invalido (" + std::to_string(size) + " bytes) -- precisa ser multiplo de " +
                      std::to_string(MEMMAP_CHUNK_SIZE) + " entre " + std::to_string(MEMMAP_CHUNK_SIZE) + " e " +
                      std::to_string(max_size);
        }
        return false;
    }

    // CRC32 calculado ANTES de tocar em chunk[]/owned_buffers_ -- se algo
    // desse errado aqui (nao da, rom_crc32 nao falha, mas por seguranca
    // contra mudanca futura), a combinacao de slot fica intacta.
    uint32_t crc = 0;
    rom_crc32(data, static_cast<int32_t>(size), &crc);

    auto buffer = std::make_unique<uint8_t[]>(size);
    std::copy(data, data + size, buffer.get());

    if (mapper == MEMMAP_MAPPER_NONE) {
        // ROM plana (Fase 2, comportamento inalterado): sempre comeca no
        // pedaco 0 da combinacao (endereco relativo 0x0000 do slot) --
        // mesma convencao de AllocateRam()/memmap_attach() desde a Fase 1
        // (ver o comentario em LoadRom() no .h).
        memmap_attach(&state_, primary, secondary, buffer.get(), size, MEMMAP_KIND_ROM, /*writable=*/0);
    } else {
        // MegaROM (Fase 3): so os 4 pedacos enderecaveis por bank-switch
        // (4000h-BFFFh) sao ocupados, com todos os quartos comecando no
        // banco 0 -- ver memmap_attach_megarom() em slot_state.c.
        memmap_attach_megarom(&state_, primary, secondary, buffer.get(), size, mapper);
    }
    owned_buffers_.push_back(std::move(buffer));
    rom_crc32_[primary][secondary] = crc;
    return true;
}

uint8_t MemorySystem::PeekSlot(int primary, int secondary, uint16_t addr) const {
    return memmap_peek_slot(&state_, primary, secondary, addr);
}

void MemorySystem::PokeSlot(int primary, int secondary, uint16_t addr, uint8_t value) {
    memmap_poke_slot(&state_, primary, secondary, addr, value);
}

SlotDescriptor MemorySystem::Describe(int primary, int secondary) const {
    SlotDescriptor desc;
    if (primary < 0 || primary >= MEMMAP_PRIMARY_SLOTS || secondary < 0 || secondary >= MEMMAP_SECONDARY_SLOTS) {
        return desc;
    }
    desc.kind = state_.slot_kind[primary][secondary];
    desc.size = state_.slot_size[primary][secondary];
    if (desc.kind == MEMMAP_KIND_ROM) {
        desc.crc32 = rom_crc32_[primary][secondary];
        desc.mapper_name = MapperName(state_.slot_mapper[primary][secondary]);
    }
    return desc;
}

std::array<PageView, MEMMAP_PAGES> MemorySystem::CurrentView() const {
    std::array<PageView, MEMMAP_PAGES> view{};
    for (int page = 0; page < MEMMAP_PAGES; ++page) {
        view[page].primary = state_.psl[page];
        view[page].secondary = state_.ssl[page];
        view[page].writable = state_.active_writable[page * 2] != 0 && state_.active_writable[page * 2 + 1] != 0;
    }
    return view;
}

} // namespace memmap
