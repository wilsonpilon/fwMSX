//
// msxdisk (fwMSX): roteamento comum dos 4 modos -- ver entry.h.
//

#include "entry.h"

#include "cli/app.h"
#include "gui/app.h"
#include "shell/shell.h"
#include "tui/app.h"

namespace msxdisk {

int RunEntryPoint(const std::vector<std::string> &tokens) {
    if (!tokens.empty() && tokens[0] == "--tui") {
        return tui::LaunchTui(tokens.size() > 1 ? tokens[1] : "");
    }
    if (!tokens.empty() && tokens[0] == "--gui") {
        return gui::LaunchGui(tokens.size() > 1 ? tokens[1] : "");
    }
    if (tokens.empty() || tokens[0] == "shell" || tokens[0] == "--cli") {
        return shell::RunShell();
    }
    return cli::Dispatch(tokens);
}

} // namespace msxdisk
