//
// fwMSX - ponto de entrada do projeto (linguagem: C++).
//
// Created by barney on 27/09/2026.
//
// Este e o "main" do sistema. Ele:
//   1. Imprime o aviso de copyright;
//   2. Le (ou assume os valores padrao de version.h para) o nome do
//      produto e a versao (major.minor.patch), recebidos como parametros
//      de main();
//   3. Aciona, em sequencia, o "carregamento" de um modulo escrito em
//      cada uma das quatro linguagens do projeto -- C++, C, Assembly e
//      Fortran --, cada um imprimindo sua propria mensagem com a versao
//      entre colchetes e devolvendo uma assinatura hexadecimal propria;
//   4. Imprime o resumo final com as assinaturas coletadas.
//
// Ver doc/SPEC.md para a especificacao completa do projeto e
// doc/MANUAL.md para como compilar e executar.
//

#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include "version.h"

#include "init_cpp.h"
#include "init_c.h"
#include "init_asm.h"
#include "init_fortran.h"

namespace {

void print_signature(const char* label, std::uint16_t signature) {
    std::cout << "  " << std::left << std::setw(10) << label
               << std::right << "0x" << std::hex << std::uppercase << std::setfill('0')
               << std::setw(4) << signature
               << std::nouppercase << std::dec << std::setfill(' ') << std::endl;
}

} // namespace

int main(int argc, char* argv[]) {
    std::cout << "Copyright (c) 1972-2026 Cybernostra, Inc." << std::endl;

    // main() recebe o nome do produto e os tres inteiros da versao como
    // parametros de linha de comando (argv[1..4]); na ausencia deles,
    // usamos os valores padrao definidos em src/common/version.h.
    const char* project_name = (argc > 1) ? argv[1] : FWMSX_NAME;
    const int major = (argc > 2) ? std::atoi(argv[2]) : FWMSX_VERSION_MAJOR;
    const int minor = (argc > 3) ? std::atoi(argv[3]) : FWMSX_VERSION_MINOR;
    const int patch = (argc > 4) ? std::atoi(argv[4]) : FWMSX_VERSION_PATCH;

    std::cout << project_name << " [v " << major << "." << minor << "." << patch << "]"
               << std::endl;
    std::cout << std::string(40, '-') << std::endl;

    // Cada modulo recebe a versao corrente, imprime sua propria linha de
    // "Loading module..." com a versao entre colchetes, e devolve sua
    // assinatura hexadecimal (0x0001..0x0004).
    const std::uint16_t sig_cpp = init_cpp(major, minor, patch);
    const std::uint16_t sig_c = init_c(major, minor, patch);
    const std::uint16_t sig_asm = init_asm(major, minor, patch);
    const std::uint16_t sig_fortran = init_fortran(major, minor, patch);

    std::cout << std::string(40, '-') << std::endl;
    std::cout << "Assinaturas dos modulos:" << std::endl;
    print_signature("C++", sig_cpp);
    print_signature("C", sig_c);
    print_signature("Assembly", sig_asm);
    print_signature("Fortran", sig_fortran);

    return 0;
}
