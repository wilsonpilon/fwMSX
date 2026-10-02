// fwMSX -- stub da janela do emulador para builds com FWMSX_MSXDISK_GUI=OFF
// -- ver emu_window.h.
#include "emu_window.h"

#include <iostream>

namespace machine::gui {

int RunEmulatorWindow(const WindowOptions & /*options*/) {
    std::cerr << "fwmsx: a janela do emulador nao foi compilada nesta build "
                 "(recompile com -DFWMSX_MSXDISK_GUI=ON). Use --frames N --shot arq.ppm para rodar sem janela."
              << std::endl;
    return 1;
}

} // namespace machine::gui
