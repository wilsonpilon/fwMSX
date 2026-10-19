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

class IExec {
public:
    virtual ~IExec() = default;
    virtual Z80State &State() = 0;
    // G: roda a partir de `addr` (se has_addr; senao continua do PC) ate um dos breakpoints `bps` (parando
    // ANTES da instrucao que esta' nele), HALT ou o teto de instrucoes. Devolve o texto para o log.
    virtual std::string Go(bool has_addr, uint16_t addr, const std::vector<uint16_t> &bps) = 0;
};

// CPU privada do modo offline. Portas de E/S: IN le FFh e OUT e' ignorado (o painel de portas do SUPER-X
// entra numa fase futura).
class OfflineExec : public IExec {
public:
    explicit OfflineExec(IMemory &mem);
    Z80State &State() override { return cpu_.state(); }
    std::string Go(bool has_addr, uint16_t addr, const std::vector<uint16_t> &bps) override;

    static constexpr int kMaxInstructions = 5000000;

private:
    class Bus : public z80::IBus {
    public:
        explicit Bus(IMemory &m) : mem_(m) {}
        uint8_t read(uint16_t a) override { return mem_.Read(a); }
        void write(uint16_t a, uint8_t v) override { mem_.Write(a, v); }
        uint8_t in(uint16_t) override { return 0xFF; }
        void out(uint16_t, uint8_t) override {}

    private:
        IMemory &mem_;
    };
    Bus bus_;
    z80::Z80Cpu cpu_;
};

} // namespace mamute
