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
//   --msx2p                 maquina MSX2+ (VDP V9958: SCREEN 10-12 YJK/YAE, scroll
//                           em SCREEN 12); a BIOS padrao vira MSX2P.ROM e a sub-ROM MSX2PEXT.ROM
//   --msx2                  maquina MSX2 (VDP V9938, RAM de 128KB com mapper, RTC);
//                           a BIOS padrao vira MSX2.ROM e a sub-ROM MSX2EXT.ROM
//   --ext <arq>             sub-ROM do MSX2 (padrao: MSX2EXT.ROM ao lado da BIOS)
//   --bios <arq>            BIOS (padrao: resource/fMSX/ROMs/MSX.ROM, ou MSX2.ROM
//                           com --msx2)
//   --cart <arq> [mapper]   cartucho no slot 1: sem mapper ("auto", o padrao),
//                           consulta o banco de ROMs pelo SHA-1 (fwmsx --romdb
//                           cartsha, ver doc/romdb-spec.md) antes de cair na
//                           heuristica de sempre (ate 32KB = ROM plana; acima,
//                           MegaROM detectada pelo conteudo); ou escolhido a
//                           dedo (gen8 gen16 konami5 konami4 ascii8 ascii16)
//   --disk <arq.dsk>        disquete em A: (liga a interface de disco: DISK.ROM
//                           ao lado da BIOS, no slot 3:1); --diskb para B:;
//                           --disk-interface liga so' a interface; --diskrom
//                           <arq> escolhe outra DISK.ROM. As escritas do
//                           MSX-DOS vao direto para o arquivo da imagem, a menos
//                           que se use --disk-ro (discos somente leitura: o
//                           MSX-DOS le, recusa gravar e o arquivo nao muda).
//   --mute                  nao abre dispositivo de audio (emulador mudo)
//   --frames <n>            roda n quadros e sai; sem --shot/--keys abre a
//                           janela e fecha sozinha apos n quadros
//   --shot <arq.ppm>        sem janela: roda --frames quadros e salva a tela
//   --wait <n>              quadros a esperar depois das teclas (padrao 60)
//   --vdplog                sem janela: imprime as mudancas dos registradores do VDP
//                           a cada quadro (depuracao de modos de tela)
//   --keys <texto>          sem janela: digita o texto (letras, numeros, espaco e
//                           a pontuacao comum, com SHIFT quando preciso; '|' =
//                           ENTER) antes de salvar a tela
int RunMachineCommand(const std::vector<std::string> &args, const std::string &argv0);

} // namespace machine
