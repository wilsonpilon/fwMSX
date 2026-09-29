//
// msxdisk (fwMSX): comandos compartilhados entre a CLI one-shot (CLI11) e,
// nas proximas fases, o shell interativo (replxx) e a TUI (FTXUI) -- todos
// chamam as mesmas funcoes aqui, sobre o mesmo nucleo (src/msxdisk/core).
//
#pragma once

#include <string>

namespace msxdisk::cli {

struct CreateOptions {
    std::string image_path;

    bool with_dos1 = false;
    std::string command_com_path; // obrigatorio se with_dos1
    std::string msxdos_sys_path;  // obrigatorio se with_dos1 ou with_dos2

    bool with_dos2 = false;
    std::string msxdos2_sys_path; // obrigatorio se with_dos2
};

// Cada comando imprime o resultado/erro em stdout/stderr e devolve 0 em
// caso de sucesso, != 0 em caso de erro (para uso direto como codigo de
// saida do processo em modo CLI one-shot).
int CmdCreate(const CreateOptions &opts);
int CmdList(const std::string &image_path, const std::string &dir_path, const std::string &pattern, bool tree);
int CmdAdd(const std::string &image_path, const std::string &host_path, const std::string &msx_path);
int CmdExtract(const std::string &image_path, const std::string &dir_path, const std::string &pattern,
                const std::string &dest_dir);
int CmdDelete(const std::string &image_path, const std::string &msx_path);
int CmdRename(const std::string &image_path, const std::string &msx_path, const std::string &new_name);
int CmdMkdir(const std::string &image_path, const std::string &msx_path);
int CmdRmdir(const std::string &image_path, const std::string &msx_path);
int CmdCopy(const std::string &src_image_path, const std::string &dest_image_path);
int CmdSaveAs(const std::string &image_path, const std::string &dest_image_path);
int CmdInfo(const std::string &image_path);

} // namespace msxdisk::cli
