#include "memory_system.h"

#include <algorithm>

namespace memmap {

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
