// fwMSX -- stub da janela do editor para builds com FWMSX_MSXDISK_GUI=OFF -- ver editor_window.h.
#include "editor_window.h"

#include <iostream>

namespace ed {

int RunEditorWindow(const std::vector<std::string> & /*args*/, const std::string & /*argv0*/, bool /*as_view*/) {
    std::cerr << "fwmsx: a janela do editor nao foi compilada nesta build (recompile com -DFWMSX_MSXDISK_GUI=ON). "
                 "Use o editor no terminal: fwmsx --edit --tui arquivo."
              << std::endl;
    return 1;
}

} // namespace ed
