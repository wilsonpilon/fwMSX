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

bool MemorySystem::LoadRom(int primary, int secondary, const uint8_t *data, std::size_t size, std::string *error) {
    if (!ValidSlotIndex(primary, secondary)) {
        if (error) *error = "combinacao de slot invalida: " + std::to_string(primary) + ":" + std::to_string(secondary);
        return false;
    }
    const std::size_t max_size = static_cast<std::size_t>(MEMMAP_PAGES) * MEMMAP_PAGE_SIZE;
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

    // ROM sempre comeca no pedaco 0 da combinacao (endereco relativo
    // 0x0000 do slot) -- mesma convencao de AllocateRam()/memmap_attach()
    // desde a Fase 1 (ver o comentario em LoadRom() no .h).
    memmap_attach(&state_, primary, secondary, buffer.get(), size, MEMMAP_KIND_ROM, /*writable=*/0);
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
    if (desc.kind == MEMMAP_KIND_ROM) desc.crc32 = rom_crc32_[primary][secondary];
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
