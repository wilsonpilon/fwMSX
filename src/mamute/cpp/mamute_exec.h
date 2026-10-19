// fwMSX -- Mamute: execucao e registradores (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md.
//
// IExec e' o "Z80" que o monitor usa nos comandos X e G. Offline e' uma CPU privada do nucleo Z80 do fwMSX
// sobre a memoria privada (OfflineExec); online e' a CPU da maquina emulada (OnlineExec, mamute_online.h).
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../../z80/cpp/z80_bus.h"
#include "../../z80/cpp/z80_cpu.h"
#include "mamute_memory.h"

namespace mamute {

// As portas de E/S monitoradas (painel XPP do SUPER-X). `in` e' a "Entrada" (o ultimo byte que o programa mandou com OUT) e `out` e' a
// "Saida" (o byte que um IN vai ler). Nao ha' hardware simulado atras: quem controla o que o IN le' e' o usuario (XPP/XPO).
struct PortTable {
    bool mon[256]{};
    bool changed[256]{}; // sofreu um OUT (ou XPO) desde a ultima limpeza das marcas
    uint8_t in[256]{};
    uint8_t out[256]{};

    void Add(int port) {
        if (!mon[port & 255]) {
            mon[port & 255] = true;
            out[port & 255] = 0xFF; // barramento flutuante ate' o usuario definir outra coisa
        }
    }
    uint8_t IoIn(int port) const { return mon[port & 255] ? out[port & 255] : 0xFF; }
    void IoOut(int port, uint8_t v) {
        Add(port);
        in[port & 255] = v;
        changed[port & 255] = true;
    }
};

class IExec {
public:
    virtual ~IExec() = default;
    virtual Z80State &State() = 0;
    // Executa UMA instrucao (XTR) e se a CPU esta' em HALT.
    virtual void StepOne() = 0;
    bool Halted() { return (State().iff & Z80_IFF_HALT) != 0; }
    // G: roda a partir de `addr` (se has_addr; senao continua do PC) ate um dos breakpoints `bps` (parando
    // ANTES da instrucao que esta' nele), HALT ou o teto de instrucoes. Devolve o texto para o log.
    virtual std::string Go(bool has_addr, uint16_t addr, const std::vector<uint16_t> &bps) = 0;
};

// CPU privada do modo offline. Portas de E/S: IN le o que o painel de portas (XPP) tem em "Saida", ou FFh; OUT grava em "Entrada".
class OfflineExec : public IExec {
public:
    explicit OfflineExec(IMemory &mem);
    Z80State &State() override { return cpu_.state(); }
    void StepOne() override { cpu_.run(1); }
    std::string Go(bool has_addr, uint16_t addr, const std::vector<uint16_t> &bps) override;
    // O painel de portas (XPP); sem ele, IN le' FFh e OUT e' ignorado.
    void SetPorts(PortTable *p) { bus_.ports = p; }

    static constexpr int kMaxInstructions = 5000000;

private:
    class Bus : public z80::IBus {
    public:
        explicit Bus(IMemory &m) : mem_(m) {}
        uint8_t read(uint16_t a) override { return mem_.Read(a); }
        void write(uint16_t a, uint8_t v) override { mem_.Write(a, v); }
        uint8_t in(uint16_t p) override { return ports ? ports->IoIn(p & 255) : 0xFF; }
        void out(uint16_t p, uint8_t v) override {
            if (ports) ports->IoOut(p & 255, v);
        }
        PortTable *ports = nullptr;

    private:
        IMemory &mem_;
    };
    Bus bus_;
    z80::Z80Cpu cpu_;
};

} // namespace mamute
