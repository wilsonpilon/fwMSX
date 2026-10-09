// fwMSX -- utilitario de build (Windows): copia um .exe trocando o campo "Subsystem" do cabecalho PE
// (2 = janela/GUI, 3 = console). E' o que gera o fwMSXc.exe (o mesmo programa, mas do subsistema
// console) a partir do fwMSX.exe (subsistema janela): um programa de janela devolve o prompt do
// terminal na hora e briga com o shell pela entrada do teclado, o que quebra o console interativo
// (fwmsx --cli); o de console faz o shell ESPERAR. Equivale a `editbin /SUBSYSTEM`.
//
// Uso: pe_subsystem <entrada.exe> <saida.exe> <2|3>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

int main(int argc, char **argv) {
    if (argc != 4) {
        std::fprintf(stderr, "uso: pe_subsystem <entrada.exe> <saida.exe> <2|3>\n");
        return 2;
    }
    const int subsystem = std::atoi(argv[3]);
    if (subsystem != 2 && subsystem != 3) {
        std::fprintf(stderr, "pe_subsystem: o subsistema deve ser 2 (janela) ou 3 (console)\n");
        return 2;
    }
    std::ifstream in(argv[1], std::ios::binary);
    if (!in) {
        std::fprintf(stderr, "pe_subsystem: nao foi possivel abrir '%s'\n", argv[1]);
        return 1;
    }
    std::vector<unsigned char> d((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    if (d.size() < 0x40 || d[0] != 'M' || d[1] != 'Z') {
        std::fprintf(stderr, "pe_subsystem: '%s' nao e' um executavel PE\n", argv[1]);
        return 1;
    }
    uint32_t pe = 0;
    std::memcpy(&pe, &d[0x3C], 4);
    // "PE\0\0" (4) + IMAGE_FILE_HEADER (20) + 68 bytes ate' o campo Subsystem do optional header.
    const size_t field = static_cast<size_t>(pe) + 4 + 20 + 68;
    if (static_cast<size_t>(pe) + 4 > d.size() || field + 2 > d.size() || std::memcmp(&d[pe], "PE\0\0", 4) != 0) {
        std::fprintf(stderr, "pe_subsystem: cabecalho PE invalido\n");
        return 1;
    }
    d[field] = static_cast<unsigned char>(subsystem);
    d[field + 1] = 0;
    std::ofstream out(argv[2], std::ios::binary | std::ios::trunc);
    if (!out || !out.write(reinterpret_cast<const char *>(d.data()), static_cast<std::streamsize>(d.size()))) {
        std::fprintf(stderr, "pe_subsystem: nao foi possivel gravar '%s'\n", argv[2]);
        return 1;
    }
    return 0;
}
