#pragma once
//
// fwMSX - Informacoes de versao
//
// Este arquivo e a UNICA fonte de verdade para o numero de versao padrao
// do projeto (usado quando o executavel roda sem argumentos). Ele e
// atualizado manualmente a cada fase do trabalho, seguindo a politica de
// versionamento descrita em doc/SPEC.md:
//
//   X (major) -> sobe quando um conjunto grande de mudancas esta' praticamente todo feito.
//                Recebe o nome de uma EMPRESA de MSX (1 = Konami, 2 = ASCII, 3 = Compile...).
//   Y (minor) -> bloco maior de funcionalidade publicado como release. Recebe o nome de um
//                JOGO NOVO de MSX (nunca repetido). Zera o patch.
//   Z (patch) -> feature pequena, ajuste ou correcao. Leva o nome do jogo do Y mais um
//                SUBTITULO ("Nemesis: Ponte de controle"). X.Y.0 leva so' o nome do jogo.
//
// Ao subir o Y: trocar FWMSX_CODENAME por um jogo novo e esvaziar o subtitulo. Ao subir o X:
// trocar FWMSX_COMPANY. Lista de jogos e empresas reservados: CLAUDE.md, regra 1.
//
#define FWMSX_NAME "fwMSX"

#define FWMSX_VERSION_MAJOR 1
#define FWMSX_VERSION_MINOR 32
#define FWMSX_VERSION_PATCH 0

#define FWMSX_COMPANY   "Konami"
#define FWMSX_CODENAME  "Nemesis"
#define FWMSX_SUBTITLE  ""  // so' nas versoes X.Y.Z com Z > 0
