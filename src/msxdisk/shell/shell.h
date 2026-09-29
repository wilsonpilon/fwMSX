//
// msxdisk (fwMSX): shell interativo (REPL) -- Fase 3.
//
#pragma once

namespace msxdisk::shell {

// Abre o prompt interativo (replxx) e despacha cada linha digitada para
// msxdisk::cli::Dispatch, os mesmos comandos usados pelo modo CLI
// one-shot. Devolve o codigo de saida do processo.
int RunShell();

} // namespace msxdisk::shell
