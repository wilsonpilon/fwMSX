// fwMSX -- relogio de tempo real do MSX2 (RP5C01, portas B4h/B5h) com a
// memoria CMOS de configuracao. Codigo ORIGINAL do fwMSX (BSD-3-Clause); o
// comportamento segue RTCIn() e os casos B4h/B5h de MSX.c do fMSX, incluindo
// os valores padrao da CMOS (RTCInit). Ver doc/msx2-spec.md.
#pragma once

#include <cstdint>
#include <ctime>
#include <functional>

#include "../z80/cpp/z80_bus.h"

namespace rtc {

// B4h escolhe o registrador (0-15); B5h le/escreve nele. O registrador 13 e'
// o de MODO: seus 2 bits baixos escolhem o BANCO (0-3). O banco 0 e' o relogio
// (segundos, minutos, horas... lidos do relogio do host); os bancos 1-3 sao
// memoria CMOS de 13 nibbles -- o banco 2 guarda as preferencias da BIOS
// (cores da tela, largura, ...), por isso os valores padrao importam: sem eles
// o BASIC do MSX2 sobe com cores zeradas.
class RtcDevice : public z80::IBus {
public:
    using Clock = std::function<std::time_t()>;

    RtcDevice() { Reset(); }

    uint8_t read(uint16_t) override { return 0; }
    void write(uint16_t, uint8_t) override {}

    uint8_t in(uint16_t port) override { return (port & 0xFF) == 0xB5 ? ReadRegister(reg_) : 0xFF; }

    void out(uint16_t port, uint8_t value) override {
        switch (port & 0xFF) {
        case 0xB4: reg_ = value & 0x0F; break;
        case 0xB5: WriteRegister(value); break;
        default: break;
        }
    }

    // Padrao da CMOS: RTCInit[] do fMSX (so' o banco 2 tem algo: ajuste de tela,
    // largura 40, cores 15/4/4...).
    void Reset() {
        reg_ = 0;
        mode_ = 0;
        for (auto &bank : cmos_)
            for (auto &r : bank) r = 0;
        static const uint8_t kBank2[13] = {0, 0, 0, 0, 40, 80, 15, 4, 4, 0, 0, 0, 0};
        for (int i = 0; i < 13; ++i) cmos_[2][i] = kBank2[i];
    }

    // Relogio injetavel (testes); sem ele usa o do sistema.
    void SetClock(Clock clock) { clock_ = std::move(clock); }

    uint8_t mode() const { return mode_; }
    uint8_t cmos(int bank, int reg) const { return cmos_[bank & 3][reg % 13]; }

private:
    uint8_t ReadRegister(uint8_t r) {
        r &= 0x0F;
        const int bank = mode_ & 0x03;
        uint8_t j;
        if (r > 12) {
            j = r == 13 ? mode_ : 0xFF;
        } else if (bank) {
            j = cmos_[bank][r];
        } else {
            const std::time_t now = clock_ ? clock_() : std::time(nullptr);
            std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm, &now);
#else
            localtime_r(&now, &tm);
#endif
            switch (r) {
            case 0: j = static_cast<uint8_t>(tm.tm_sec % 10); break;
            case 1: j = static_cast<uint8_t>(tm.tm_sec / 10); break;
            case 2: j = static_cast<uint8_t>(tm.tm_min % 10); break;
            case 3: j = static_cast<uint8_t>(tm.tm_min / 10); break;
            case 4: j = static_cast<uint8_t>(tm.tm_hour % 10); break;
            case 5: j = static_cast<uint8_t>(tm.tm_hour / 10); break;
            case 6: j = static_cast<uint8_t>(tm.tm_wday); break;
            case 7: j = static_cast<uint8_t>(tm.tm_mday % 10); break;
            case 8: j = static_cast<uint8_t>(tm.tm_mday / 10); break;
            case 9: j = static_cast<uint8_t>((tm.tm_mon + 1) % 10); break;
            case 10: j = static_cast<uint8_t>((tm.tm_mon + 1) / 10); break;
            case 11: j = static_cast<uint8_t>((tm.tm_year - 80) % 10); break;
            case 12: j = static_cast<uint8_t>(((tm.tm_year - 80) / 10) % 10); break;
            default: j = 0x0F; break;
            }
        }
        return static_cast<uint8_t>(j | 0xF0); // os 4 bits altos sempre leem 1
    }

    void WriteRegister(uint8_t value) {
        if (reg_ < 13) {
            cmos_[mode_ & 0x03][reg_] = value; // o banco 0 e' o relogio: a escrita nao muda a hora lida
            return;
        }
        if (reg_ == 13) mode_ = value;
    }

    uint8_t reg_ = 0;
    uint8_t mode_ = 0;
    uint8_t cmos_[4][13] = {};
    Clock clock_;
};

} // namespace rtc
