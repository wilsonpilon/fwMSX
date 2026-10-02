// fwMSX -- ponto de entrada de "fwmsx --msx ...": liga a maquina MSX1
// completa, em janela ou sem janela. Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/machine-spec.md.
#pragma once

#include <string>
#include <vector>

namespace machine {

// `args` sao os argumentos DEPOIS de "--msx"; `argv0` e' argv[0] (para achar a
// BIOS padrao ao lado do executavel).
//
//   --bios <arq>            BIOS MSX1 (padrao: resource/fMSX/ROMs/MSX.ROM)
//   --cart <arq> [mapper]   cartucho no slot 1: ROM plana (ate 32KB) ou, com
//                           mapper (gen8 gen16 konami5 konami4 ascii8
//                           ascii16), MegaROM
//   --mute                  nao abre dispositivo de audio (emulador mudo)
//   --frames <n>            roda n quadros e sai; sem --shot/--keys abre a
//                           janela e fecha sozinha apos n quadros
//   --shot <arq.ppm>        sem janela: roda --frames quadros e salva a tela
//   --keys <texto>          sem janela: digita o texto (so' letras/numeros/
//                           espaco; '|' = ENTER) antes de salvar a tela
int RunMachineCommand(const std::vector<std::string> &args, const std::string &argv0);

} // namespace machine
