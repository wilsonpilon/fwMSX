//
// fwMSX - ponto de entrada do projeto (linguagem: C++).
//
// Created by barney on 27/09/2026.
//
// Este e o "main" do sistema.
//
// Sem argumento nenhum, abre a maquina MSX1 completa em janela (os mesmos
// padroes de "--msx" -- ver doc/SPEC.md, secao 5.1, decisao registrada em
// 2026-09-29 e implementada quando o core de emulacao passou a existir de
// verdade).
//
// Com argumentos que nao batem com nenhum modo conhecido (ex.:
// "fwMSX.exe NomeDoProduto 1 2 3"), cai no esqueleto historico do projeto:
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
// "fwmsx --msxdisk <resto dos argumentos>" repassa direto para o
// utilitario de disco embutido (msxdisk::RunEntryPoint -- os mesmos
// pontos de entrada de CLI/shell/TUI/GUI do msxdisk.exe standalone). Ver
// doc/SPEC.md, secao 5.1, e doc/msxdisk-spec.md, Fase 5c.
//
// "fwmsx --msx" liga a maquina MSX1 completa numa janela com teclado do
// host (ou sem janela, com --frames/--shot) -- ver doc/machine-spec.md.
//
// "fwmsx --z80dbg" abre o REPL de depuracao do nucleo Z80 (RAM plana de
// teste, sem maquina MSX ainda) -- ver doc/z80-core-spec.md, Fase 4.
// "fwmsx --z80dbg --slots" liga o mapa de memoria de verdade (slots/
// subslots) em vez da RAM plana -- ver doc/memory-map-spec.md, Fase 1.
//

#include "romdb/cli.h"
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "version.h"

#include "init_cpp.h"
#include "init_c.h"
#include "init_asm.h"
#include "init_fortran.h"

#include "msxdisk/entry.h"
#include "machine/cli.h"
#include "z80/debug/z80_debug_shell.h"

namespace {

void print_signature(const char* label, std::uint16_t signature) {
    std::cout << "  " << std::left << std::setw(10) << label
               << std::right << "0x" << std::hex << std::uppercase << std::setfill('0')
               << std::setw(4) << signature
               << std::nouppercase << std::dec << std::setfill(' ') << std::endl;
}

} // namespace

int main(int argc, char* argv[]) {
    // Sem argumento nenhum, com o core de emulacao (Z80+VDP+PSG+mapa de
    // memoria+maquina) ja existindo de verdade, abre a maquina completa em
    // janela com os padroes de "--msx" (BIOS MSX1 ao lado do executavel,
    // sem cartucho/disco) -- decisao registrada em doc/SPEC.md, secao 5.1.
    // O esqueleto dos quatro modulos (C++/C/Assembly/Fortran) abaixo so e'
    // alcancado quando ha argumentos explicitos que nao sao um dos modos
    // conhecidos (ex.: "fwMSX.exe NomeDoProduto 1 2 3").
    if (argc == 1) {
        return machine::RunMachineCommand({}, argv[0]);
    }
    if (argc > 1 && std::string(argv[1]) == "--msxdisk") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return msxdisk::RunEntryPoint(tokens);
    }
    if (argc > 1 && std::string(argv[1]) == "--msx") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return machine::RunMachineCommand(tokens, argv[0]);
    }
    if (argc > 1 && std::string(argv[1]) == "--romdb") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return romdb::RunRomDbCommand(tokens, argv[0]);
    }
    if (argc > 1 && std::string(argv[1]) == "--z80dbg") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return z80::debug::RunZ80DebugShell(tokens);
    }

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
