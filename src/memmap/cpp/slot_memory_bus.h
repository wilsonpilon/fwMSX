// fwMSX -- adaptador de MemorySystem para z80::IBus. Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). Ver doc/memory-map-spec.md, secao 3.3.
#pragma once

#include <cstdint>

#include "../../z80/cpp/z80_bus.h"
#include "memory_system.h"

namespace memmap {

// Dispositivo mapeado em memoria DENTRO de um slot: quando o slot do
// dispositivo esta visivel na pagina do endereco, leituras/escritas em
// certos enderecos vao para ele em vez da ROM/RAM do slot (ex.: a controladora
// de disquete em 7FF8h-7FFFh do slot do DiskROM, ver src/fdc/). Cada metodo
// devolve true se tratou o acesso, false para cair na memoria normal.
class SlotMmio {
public:
    virtual ~SlotMmio() = default;
    virtual bool MmioRead(uint16_t addr, uint8_t &value) = 0;
    virtual bool MmioWrite(uint16_t addr, uint8_t value) = 0;
};

// Liga um MemorySystem ao nucleo Z80: leitura/escrita normais vao para a
// vista ativa (memmap_read/memmap_write); a porta A8h e o endereco FFFFh
// recebem o tratamento especial de troca de slot primario/secundario que
// o fMSX da' dentro do proprio RdZ80/WrZ80/InZ80/OutZ80 -- aqui fica
// isolado nesta classe porque MemorySystem/SlotState nao sabem nada sobre
// "que endereco/porta aciona o que" (ver doc/memory-map-spec.md, secao
// 3.2 e 3.3: essa e' a fronteira deliberada entre o motor em C e a
// integracao especifica de MSX em C++).
class SlotMemoryBus : public z80::IBus {
public:
    explicit SlotMemoryBus(MemorySystem &memory) : memory_(memory) {}

    uint8_t read(uint16_t addr) override {
        if (addr == 0xFFFF) {
            // Leitura do registrador de slot secundario devolve o
            // COMPLEMENTO do valor -- mesmo comportamento de hardware que
            // o fMSX reproduz em RdZ80 (`~SSLReg[PSL[3]]`), inclusive o
            // detalhe de que e' indexado pelo slot primario que ocupa a
            // pagina 3 (C000h-FFFFh), nao um registrador global unico.
            const SlotState &s = memory_.state();
            return static_cast<uint8_t>(~s.ssl_reg[s.psl[3]]);
        }
        if (mmio_ && (addr & 0x3F80) == 0x3F80 && MmioVisible(addr)) {
            uint8_t value;
            if (mmio_->MmioRead(addr, value)) return value;
        }
        return memmap_read(&memory_.state(), addr);
    }

    void write(uint16_t addr, uint8_t value) override {
        if (addr == 0xFFFF) {
            memmap_switch_secondary(&memory_.state(), value);
            return;
        }
        if (mmio_ && (addr & 0x3F80) == 0x3F80 && MmioVisible(addr) && mmio_->MmioWrite(addr, value)) return;
        memmap_write(&memory_.state(), addr, value);
    }

    // Liga um dispositivo mapeado em memoria ao slot (primary, secondary) --
    // um so' por barramento (suficiente para o DiskROM). nullptr desliga.
    void AttachMmio(int primary, int secondary, SlotMmio *device) {
        mmio_ = device;
        mmio_primary_ = primary;
        mmio_secondary_ = secondary;
    }

    uint8_t in(uint16_t port) override {
        // So a porta A8h (estado do slot primario) tem sentido por
        // enquanto -- nenhum outro dispositivo de I/O existe ainda (PPI,
        // VDP, PSG etc. sao fora do escopo desta fase).
        if ((port & 0xFF) == 0xA8) return memory_.state().psl_reg;
        return 0;
    }

    void out(uint16_t port, uint8_t value) override {
        if ((port & 0xFF) == 0xA8) memmap_switch_primary(&memory_.state(), value);
    }

    // Caminho rapido de LDIR/LDDR (Fase 3 do nucleo Z80) -- so devolve
    // ponteiro quando o intervalo cai INTEIRO dentro de um unico pedaco
    // de 8KB da vista ativa, nunca atravessando fronteira de chunk (ver
    // doc/memory-map-spec.md, secao 3.3).
    //
    // IMPORTANTE, achado ao implementar (nao estava explicito no design
    // doc): o core do Z80 chama ram_ptr() tanto para a ORIGEM quanto para
    // o DESTINO de um LDIR/LDDR (ver src/z80/core/opcodes_ed.h), e
    // ESCREVE no ponteiro de destino diretamente via
    // z80_fast_block_move() -- sem passar por memmap_write(). Se este
    // metodo devolvesse um ponteiro para um pedaco NAO gravavel (ex.: uma
    // futura ROM), o caminho rapido corromperia memoria que
    // memmap_write() teria corretamente recusado a escrever. Por isso
    // ram_ptr() so devolve ponteiro para pedacos com active_writable!=0
    // -- mais conservador que o necessario quando o intervalo e' so
    // ORIGEM de leitura (LDIR lendo de uma ROM nunca usa o caminho
    // rapido, cai no loop lento), mas e' a unica forma segura de manter a
    // garantia "corretude nunca depende do caminho rapido" com uma unica
    // funcao servindo os dois papeis.
    uint8_t *ram_ptr(uint16_t addr, uint16_t len) override {
        if (len == 0) return nullptr;
        const uint32_t last = static_cast<uint32_t>(addr) + len - 1;
        if (last > 0xFFFF) return nullptr;

        const int chunk_idx = addr >> 13;
        if ((static_cast<uint16_t>(last) >> 13) != chunk_idx) return nullptr;

        const SlotState &s = memory_.state();
        if (!s.active_writable[chunk_idx]) return nullptr;
        return s.active_view[chunk_idx] + (addr & (MEMMAP_CHUNK_SIZE - 1));
    }

private:
    // O slot do dispositivo esta visivel na pagina de `addr` agora?
    bool MmioVisible(uint16_t addr) const {
        const SlotState &s = memory_.state();
        const int page = addr >> 14;
        return s.psl[page] == mmio_primary_ && s.ssl[page] == mmio_secondary_;
    }

    MemorySystem &memory_;
    SlotMmio *mmio_ = nullptr;
    int mmio_primary_ = 0;
    int mmio_secondary_ = 0;
};

} // namespace memmap
