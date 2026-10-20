// fwMSX -- editor de textos: ligacoes com o resto do programa. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "editor_host.h"

#include <algorithm>

#include "../../man/cpp/man.h"

namespace ed {

void AttachManHelp(EditorApp &app) {
    app.SetHelpHook([](const std::string &word) -> std::string {
        std::string w = word;
        std::transform(w.begin(), w.end(), w.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const man::Topic *t = man::Resolve(w);
        if (!t && !w.empty() && w.back() != '$') t = man::Resolve(w + "$"); // LEFT -> LEFT$ nao existe sem o cifrao, mas o contrario sim
        return t ? man::Render(*t, 66) : std::string();
    });
}

} // namespace ed
