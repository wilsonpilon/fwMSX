// fwMSX -- barramento composto: multiplexa I/O de porta entre varios
// dispositivos, enquanto leitura/escrita de MEMORIA vai sempre para um
// unico dispositivo designado (o MSX so tem UM espaco de enderecos do
// Z80; VDP e outros dispositivos futuros sao so-de-porta no hardware
// real, nao ha memoria mapeada em porta para desenhar aqui). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause), nao adaptado do fMSX. Ver
// doc/vdp-spec.md, secao 3.3 ("Fase 0.5").
#pragma once

#include <array>
#include <cstdint>

#include "z80_bus.h"

namespace z80 {

class CompositeBus : public IBus {
public:
    explicit CompositeBus(IBus &memory) : memory_(memory) {}

    // Registra `device` para atender a porta `port` (0-255; o Z80 so tem
    // 256 portas de I/O de 8 bits -- o byte baixo do numero de porta e'
    // o que importa, exatamente como os dispositivos individuais (ex.
    // SlotMemoryBus/VdpDevice) ja fazem ao mascarar `port & 0xFF`
    // internamente).
    void RegisterPort(uint16_t port, IBus *device) { port_map_[port & 0xFF] = device; }

    // Registra `device` para todas as portas de `first` a `last`,
    // inclusive (ambos mascarados com &0xFF).
    void RegisterPortRange(uint16_t first, uint16_t last, IBus *device) {
        for (uint32_t p = (first & 0xFF); p <= (last & 0xFF); ++p) port_map_[p] = device;
    }

    // Memoria: sempre o dispositivo designado -- nunca despachado por
    // porta (nao existe "porta de memoria" no MSX).
    uint8_t read(uint16_t addr) override { return memory_.read(addr); }
    void write(uint16_t addr, uint8_t value) override { memory_.write(addr, value); }
    void on_bios_patch(Z80Cpu &cpu) override { memory_.on_bios_patch(cpu); }
    uint8_t *ram_ptr(uint16_t addr, uint16_t len) override { return memory_.ram_ptr(addr, len); }
    void on_jump(uint16_t pc) override { memory_.on_jump(pc); }

    // Porta: despacha para o dispositivo registrado (o numero de porta
    // COMPLETO, nao mascarado, chega ao dispositivo -- cada dispositivo
    // ja mascara &0xFF internamente, exatamente como faria ligado
    // direto ao Z80Cpu sem o CompositeBus no meio). Sem dispositivo
    // registrado: leitura devolve 0, escrita e' descartada -- mesma
    // convencao de "porta inexistente" que os proprios dispositivos
    // usam para faixas que nao reconhecem.
    uint8_t in(uint16_t port) override {
        IBus *device = port_map_[port & 0xFF];
        return device ? device->in(port) : 0;
    }
    void out(uint16_t port, uint8_t value) override {
        IBus *device = port_map_[port & 0xFF];
        if (device) device->out(port, value);
    }

private:
    IBus &memory_;
    std::array<IBus *, 256> port_map_{};
};

} // namespace z80
