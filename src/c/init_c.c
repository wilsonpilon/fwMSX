//
// Modulo C do fwMSX.
//
// Created by barney on 27/09/2026.
//

#include "init_c.h"

#include <stdio.h>

uint16_t init_c(int major, int minor, int patch) {
    printf("Loading module...C [v %d.%d.%d]\n", major, minor, patch);

    /* Assinatura hexadecimal do modulo C. */
    return 0x0002;
}
