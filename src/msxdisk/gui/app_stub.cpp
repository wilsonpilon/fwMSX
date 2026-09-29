//
// msxdisk (fwMSX): stub de GUI para builds com FWMSX_MSXDISK_GUI=OFF --
// ver app.h.
//

#include "app.h"

#include <iostream>

namespace msxdisk::gui {

int LaunchGui(const std::string & /*image_path*/) {
    std::cerr << "msxdisk: GUI nao foi compilada nesta build "
                 "(recompile com -DFWMSX_MSXDISK_GUI=ON -- ver doc/msxdisk-spec.md, Fase 5)."
              << std::endl;
    return 1;
}

} // namespace msxdisk::gui
