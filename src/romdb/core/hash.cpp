// fwMSX -- hashes das ROMs do banco. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "hash.h"

#include <array>
#include <cstdio>
#include <vector>

namespace romdb {
namespace {

uint32_t Rol(uint32_t v, int n) { return (v << n) | (v >> (32 - n)); }

} // namespace

std::string Sha1Hex(const uint8_t *data, size_t size) {
    uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476, h4 = 0xC3D2E1F0;

    // Mensagem com o padding de SHA-1: bit 1, zeros ate 56 mod 64, tamanho em bits (64 bits, big-endian).
    std::vector<uint8_t> msg(data, data + size);
    msg.push_back(0x80);
    while (msg.size() % 64 != 56) msg.push_back(0);
    const uint64_t bits = static_cast<uint64_t>(size) * 8;
    for (int i = 7; i >= 0; --i) msg.push_back(static_cast<uint8_t>(bits >> (i * 8)));

    for (size_t off = 0; off < msg.size(); off += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i) {
            const size_t p = off + static_cast<size_t>(i) * 4;
            w[i] = (static_cast<uint32_t>(msg[p]) << 24) | (static_cast<uint32_t>(msg[p + 1]) << 16) |
                   (static_cast<uint32_t>(msg[p + 2]) << 8) | static_cast<uint32_t>(msg[p + 3]);
        }
        for (int i = 16; i < 80; ++i) w[i] = Rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if (i < 20) {
                f = (b & c) | (~b & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            const uint32_t temp = Rol(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = Rol(b, 30);
            b = a;
            a = temp;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    char out[41];
    std::snprintf(out, sizeof out, "%08x%08x%08x%08x%08x", h0, h1, h2, h3, h4);
    return std::string(out);
}

std::string Crc32Hex(const uint8_t *data, size_t size) {
    static const std::array<uint32_t, 256> table = [] {
        std::array<uint32_t, 256> t{};
        for (uint32_t n = 0; n < 256; ++n) {
            uint32_t c = n;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            t[n] = c;
        }
        return t;
    }();
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; ++i) crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    crc ^= 0xFFFFFFFFu;
    char out[9];
    std::snprintf(out, sizeof out, "%08X", crc);
    return std::string(out);
}

} // namespace romdb
