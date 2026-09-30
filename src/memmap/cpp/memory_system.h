// fwMSX -- orquestracao do mapa de memoria MSX (slots/subslots). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause) -- nao existe no fMSX (que nao tem
// depurador com essa granularidade de inspecao). Ver
// doc/memory-map-spec.md, secao 3.3.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "../common/memmap_types.h"
#include "../core/slot_state.h"

namespace memmap {

struct SlotDescriptor {
    MemMapKind kind = MEMMAP_KIND_EMPTY;
    std::size_t size = 0;
    // Nome do tipo de mapper (ex.: "ASCII8"), vazio ate a Fase 3 (bank
    // switch) existir -- campo presente desde ja para nao mudar a forma
    // do struct depois (ver doc/memory-map-spec.md, secao 3.3).
    const char *mapper_name = "";
};

struct PageView {
    int primary = 0;
    int secondary = 0;
    // true so quando os DOIS pedacos de 8KB da pagina sao gravaveis --
    // visao combinada por pagina de 16KB, ainda que o motor por baixo
    // (SlotState) trabalhe com granularidade de 8KB. Ver
    // doc/memory-map-spec.md, secao 3.2/3.3.
    bool writable = false;
};

// Dona do SlotState (C) e dos buffers de RAM alocados via AllocateRam().
// Fase 1: so RAM (sem carregamento de ROM/mapper -- isso e' Fase 2/3, ver
// doc/memory-map-spec.md, secao 6).
class MemorySystem {
public:
    MemorySystem();

    // Aloca `size` bytes (arredondados PARA CIMA para multiplo de
    // MEMMAP_CHUNK_SIZE, ate no maximo MEMMAP_PAGES*MEMMAP_PAGE_SIZE =
    // 0x10000 por combinacao -- um slot nao pode ter mais que o espaco de
    // enderecos inteiro do Z80) de RAM zerada e conecta na combinacao
    // (primary, secondary). O buffer fica vivo enquanto o MemorySystem
    // existir. Chamar de novo na mesma combinacao substitui o que havia
    // antes (o buffer antigo e' liberado).
    void AllocateRam(int primary, int secondary, std::size_t size);

    // Le/escreve numa combinacao de slot especifica, independente do que
    // esta na vista ativa da CPU agora -- a API de inspecao "por fora"
    // que o depurador usa (requisito vital do autor, ver
    // doc/memory-map-spec.md, secao 1/3.3).
    uint8_t PeekSlot(int primary, int secondary, uint16_t addr) const;
    void PokeSlot(int primary, int secondary, uint16_t addr, uint8_t value);

    SlotDescriptor Describe(int primary, int secondary) const;
    std::array<PageView, MEMMAP_PAGES> CurrentView() const;

    SlotState &state() { return state_; }
    const SlotState &state() const { return state_; }

private:
    SlotState state_{};
    // Mantem vivos os buffers passados para memmap_attach() -- SlotState
    // em C so guarda ponteiros crus, nao possui memoria.
    std::vector<std::unique_ptr<uint8_t[]>> owned_buffers_;
};

} // namespace memmap
