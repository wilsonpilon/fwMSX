// fwMSX -- hashes das ROMs do banco (SHA-1 e CRC32). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/romdb-spec.md.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace romdb {

// SHA-1 em hexadecimal minusculo (40 caracteres), como usado pelo CARTS.SHA do
// fMSX e pelo banco do Vampier.
std::string Sha1Hex(const uint8_t *data, size_t size);

// CRC32 (poligono IEEE 0xEDB88320) em hexadecimal maiusculo de 8 caracteres.
std::string Crc32Hex(const uint8_t *data, size_t size);

} // namespace romdb
