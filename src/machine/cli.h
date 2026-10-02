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
//   --cart <arq> [mapper]   cartucho no slot 1: ate 32KB = ROM plana; acima,
//                           MegaROM com mapper detectado sozinho ("auto", o
//                           padrao) ou escolhido (gen8 gen16 konami5 konami4
//                           ascii8 ascii16)
//   --disk <arq.dsk>        disquete em A: (liga a interface de disco: DISK.ROM
//                           ao lado da BIOS, no slot 3:1); --diskb para B:;
//                           --disk-interface liga so' a interface; --diskrom
//                           <arq> escolhe outra DISK.ROM. As escritas do
//                           MSX-DOS vao direto para o arquivo da imagem.
//   --mute                  nao abre dispositivo de audio (emulador mudo)
//   --frames <n>            roda n quadros e sai; sem --shot/--keys abre a
//                           janela e fecha sozinha apos n quadros
//   --shot <arq.ppm>        sem janela: roda --frames quadros e salva a tela
//   --wait <n>              quadros a esperar depois das teclas (padrao 60)
//   --keys <texto>          sem janela: digita o texto (so' letras/numeros/
//                           espaco; '|' = ENTER) antes de salvar a tela
int RunMachineCommand(const std::vector<std::string> &args, const std::string &argv0);

} // namespace machine
