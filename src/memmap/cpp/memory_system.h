// fwMSX -- orquestracao do mapa de memoria MSX (slots/subslots). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause) -- nao existe no fMSX (que nao tem
// depurador com essa granularidade de inspecao). Ver
// doc/memory-map-spec.md, secao 3.3.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../common/memmap_types.h"
#include "../core/slot_state.h"

namespace memmap {

struct SlotDescriptor {
    MemMapKind kind = MEMMAP_KIND_EMPTY;
    std::size_t size = 0;
    // Nome do tipo de mapper (ex.: "ASCII8"), vazio para ROM plana
    // (MEMMAP_MAPPER_NONE) -- Fase 3, ver doc/memory-map-spec.md, secao 6.
    const char *mapper_name = "";
    // CRC32 dos bytes da ROM (Fase 2, ver doc/memory-map-spec.md, secao
    // 3.4) -- valido SO quando kind == MEMMAP_KIND_ROM; 0 (nao "nenhum
    // CRC calculado") para Empty/Ram, onde o campo nao tem sentido.
    // Conveniencia do depurador para identificar qual imagem exata esta
    // carregada -- NAO e' deteccao de mapper/cartucho (isso usaria SHA1
    // contra resource/fMSX/ROMs/CARTS.SHA, como o fMSX faz -- ver a nota
    // em src/memmap/fortran/rom_checksum.f90).
    uint32_t crc32 = 0;
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

// Dona do SlotState (C) e dos buffers de RAM/ROM alocados via
// AllocateRam()/LoadRom(). Bank-switch (mappers MegaROM) -- Fase 3, ver
// doc/memory-map-spec.md, secao 6 -- cobre so a troca de banco de ROM de
// MAP_GEN8/GEN16/KONAMI5/KONAMI4/ASCII8/ASCII16 (sem SCC/SRAM/GMASTER2/
// FMPAC/MAP_GUESS, ver a justificativa no design doc).
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

    // Carrega uma imagem de ROM plana (SEM bank-switch -- isso e' Fase 3,
    // ver doc/memory-map-spec.md, secao 6) na combinacao (primary,
    // secondary). `size` deve ser multiplo de MEMMAP_CHUNK_SIZE (8KB) e
    // estar entre 8KB e 64KB (0x2000..0x10000) -- tamanhos reais de ROM
    // MSX (8/16/24/32/48/64KB) sempre respeitam isso, entao a checagem e'
    // generica, nao uma lista de tamanhos aceitos. A ROM sempre comeca no
    // pedaco 0 da combinacao (endereco relativo 0x0000 do slot) -- mesma
    // convencao ja usada por AllocateRam()/memmap_attach() desde a Fase 1;
    // pedacos de 8KB alem do tamanho da imagem ficam vazios (ver
    // memmap_attach() em slot_state.c). Copia `data` para um buffer
    // proprio (o buffer do chamador pode ser liberado logo depois da
    // chamada) e marca todos os pedacos como NAO-graviaveis -- uma
    // escrita ali e' descartada em silencio, mesma regra de um slot vazio
    // (ver PokeSlot/SlotMemoryBus::write). Calcula o CRC32 da imagem (via
    // Fortran, src/memmap/fortran/rom_checksum.f90) para Describe()
    // reportar. Devolve false e preenche `error` (se nao-nulo) em caso de
    // tamanho invalido ou combinacao de slot invalida; nunca lanca.
    // `mapper` (Fase 3, ver doc/memory-map-spec.md, secao 6): MEMMAP_MAPPER_NONE
    // (default) preserva o comportamento da Fase 2 exatamente -- ROM plana,
    // sempre comecando no pedaco 0, tamanho entre 8KB e 64KB. Qualquer
    // outro valor liga bank-switch: `data` pode entao ter ATE 256 bancos
    // de 8KB (2MB, limite do uint8_t que guarda a mascara de banco -- ver
    // slot_state.h), e so os 4 pedacos enderecaveis por bank-switch
    // (4000h-BFFFh) sao ocupados -- 0000h-3FFFh e C000h-FFFFh ficam
    // vazios, igual um cartucho MSX classico de verdade (que so responde
    // em 4000h-BFFFh). Estado inicial: todos os 4 quartos mostram o banco
    // 0 (ver memmap_attach_megarom() em slot_state.c).
    bool LoadRom(int primary, int secondary, const uint8_t *data, std::size_t size, std::string *error = nullptr,
                 MemMapMapperType mapper = MEMMAP_MAPPER_NONE);

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
    // CRC32 por combinacao de slot (so relevante para MEMMAP_KIND_ROM) --
    // vive aqui em vez de em SlotState (C) porque e' metadado de
    // depurador, nao algo que o motor de troca de slot precisa conhecer.
    uint32_t rom_crc32_[MEMMAP_PRIMARY_SLOTS][MEMMAP_SECONDARY_SLOTS] = {};
};

} // namespace memmap
