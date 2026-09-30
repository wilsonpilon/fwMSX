#pragma once
//
// fwMSX - Informacoes de versao
//
// Este arquivo e a UNICA fonte de verdade para o numero de versao padrao
// do projeto (usado quando o executavel roda sem argumentos). Ele e
// atualizado manualmente a cada fase do trabalho, seguindo a politica de
// versionamento descrita em doc/SPEC.md:
//
//   X (major) -> sobe quando um grupo de mudancas fecha uma base estavel.
//   Y (minor) -> sobe a cada feature nova incorporada ao projeto.
//   Z (patch) -> sobe a cada compilacao/build gerado.
//
// Cada versao tambem recebe o nome de um jogo classico de MSX + um
// subtitulo curto indicando em que ponto do projeto estamos. O historico
// completo de versoes fica em doc/RELEASE.md e doc/CHANGELOG.md.

#define FWMSX_NAME "fwMSX"

#define FWMSX_VERSION_MAJOR 1
#define FWMSX_VERSION_MINOR 4
#define FWMSX_VERSION_PATCH 1

#define FWMSX_CODENAME  "Salamander"
#define FWMSX_SUBTITLE  "Compilando em Linux"
