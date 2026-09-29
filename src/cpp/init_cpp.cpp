//
// Modulo C++ do fwMSX.
//
// Created by barney on 27/09/2026.
//

#include "init_cpp.h"

#include <iostream>

std::uint16_t init_cpp(int major, int minor, int patch) {
    std::cout << "Loading module... CPP [v " << major << "." << minor << "." << patch << "]"
               << std::endl;

    // Assinatura hexadecimal do modulo C++.
    return 0x0001;
}
