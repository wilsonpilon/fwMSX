//
// msxdisk (fwMSX): formatacao/mensagens compartilhadas entre o modo CLI
// one-shot (commands.cpp) e a sessao do shell interativo (Fase 3b,
// src/msxdisk/shell/session.cpp) -- para nao duplicar a mesma logica de
// apresentacao nos dois lugares.
//
#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "../core/disk_image.h"

namespace msxdisk::cli {

const char *DiskErrorMessage(DiskError error);

std::optional<DiskImage> LoadOrReport(const std::string &image_path, DiskError *error_out = nullptr);
bool SaveOrReport(const DiskImage &image, const std::string &image_path);

// Confere que 'host_path' existe e e um arquivo comum ANTES de tentar
// abri-lo (usado antes de AddFile). Existe porque o erro generico de
// AddFile ("erro de E/S (arquivo/imagem inacessivel)") nao deixa claro
// se o problema e o arquivo LOCAL ou a imagem -- aqui ja sabemos que e o
// arquivo local, entao a mensagem aponta pwd/ls como proximo passo em vez
// de deixar parecer que a imagem carregada e que esta com problema.
bool CheckLocalFileReadable(const std::string &host_path, const char *command_name);

void PrintEntry(const FileEntry &e, int indent);
std::string JoinMsxPath(const std::string &dir_path, const std::string &name);

// Lista 'dir_path' recursivamente (estilo TREE), imprimindo cada entrada
// indentada. Devolve o total de entradas impressas.
int PrintTree(const DiskImage &image, const std::string &dir_path, int indent);

} // namespace msxdisk::cli
