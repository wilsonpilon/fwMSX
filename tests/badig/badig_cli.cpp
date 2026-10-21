// fwMSX -- conversor Basic Dignified em linha de comando (so' a biblioteca), usado para conferir contra o badig.py original.
#include <string>
#include <vector>

#include "../../src/badig/cpp/badig.h"

int main(int argc, char **argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    return badig::RunBadigCommand(args, "");
}
