#pragma once
//
// Modulo C++ do fwMSX.
//
// Created by barney on 27/09/2026.
//
#include <cstdint>

// Carrega o modulo C++: imprime "Loading module... CPP [v MAJOR.MINOR.PATCH]"
// e retorna a assinatura hexadecimal do modulo (0x0001).
std::uint16_t init_cpp(int major, int minor, int patch);
