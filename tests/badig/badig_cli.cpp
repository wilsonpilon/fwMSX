// fwMSX -- conversor Basic Dignified em linha de comando (so' a biblioteca), usado para conferir contra o badig.py original.
// BADIG_LIB=<pasta> aponta a biblioteca (nbasic.dmx) como o executavel principal faz com badig/.
#include <cstdlib>
#include <string>
#include <vector>

#include "../../src/badig/cpp/badig.h"

int main(int argc, char **argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    if (const char *lib = std::getenv("BADIG_LIB")) badig::SetLibraryDir(lib);
    return badig::RunBadigCommand(args, "");
}
