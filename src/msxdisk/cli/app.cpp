//
// msxdisk (fwMSX): definicao da CLI11 App e despacho de comandos.
//

#include "app.h"

#include <CLI/CLI.hpp>

#include "commands.h"

namespace msxdisk::cli {

int Dispatch(const std::vector<std::string> &tokens) {
    CLI::App app{"msxdisk - utilitario de imagens de disco MSX (.dsk)"};
    app.require_subcommand(1);

    CreateOptions create_opts;
    auto *create_cmd = app.add_subcommand("create", "Cria uma nova imagem de disco MSX em branco");
    create_cmd->add_option("imagem", create_opts.image_path, "Caminho do .dsk a criar")->required();
    create_cmd->add_flag("--dos1", create_opts.with_dos1,
                          "Inclui MSXDOS.SYS/COMMAND.COM (arquivos fornecidos pelo usuario, nao redistribuidos)");
    create_cmd->add_flag("--dos2", create_opts.with_dos2,
                          "Inclui MSXDOS.SYS/MSXDOS2.SYS (arquivos fornecidos pelo usuario, nao redistribuidos)");
    create_cmd->add_option("--sys", create_opts.msxdos_sys_path, "Caminho local do MSXDOS.SYS (com --dos1/--dos2)");
    create_cmd->add_option("--sys2", create_opts.msxdos2_sys_path, "Caminho local do MSXDOS2.SYS (com --dos2)");
    create_cmd->add_option("--command", create_opts.command_com_path, "Caminho local do COMMAND.COM (com --dos1)");

    std::string list_image;
    std::string list_dir;
    std::string list_pattern;
    bool list_tree = false;
    auto *list_cmd = app.add_subcommand("list", "Lista os arquivos/diretorios de uma imagem");
    list_cmd->alias("dir");
    list_cmd->add_option("imagem", list_image, "Caminho do .dsk")->required();
    list_cmd->add_option("padrao", list_pattern, "Filtro com coringas MSX-DOS (ex.: *.BAS)");
    list_cmd->add_option("--dir", list_dir, "Subdiretorio a listar (default: raiz)");
    list_cmd->add_flag("-r,--tree", list_tree, "Lista recursivamente (estilo TREE)");

    std::string add_image;
    std::string add_host;
    std::string add_path;
    auto *add_cmd = app.add_subcommand("add", "Adiciona um arquivo local na imagem");
    add_cmd->add_option("imagem", add_image, "Caminho do .dsk")->required();
    add_cmd->add_option("arquivo_local", add_host, "Arquivo local a adicionar")->required();
    add_cmd->add_option("--as", add_path,
                         "Caminho/nome MSX a usar, ex. DIR\\ARQ.BAS (default: nome do arquivo local na raiz)");

    std::string extract_image;
    std::string extract_dir;
    std::string extract_pattern;
    std::string extract_dest = ".";
    auto *extract_cmd = app.add_subcommand("extract", "Extrai arquivo(s) da imagem");
    extract_cmd->add_option("imagem", extract_image, "Caminho do .dsk")->required();
    extract_cmd->add_option("padrao", extract_pattern, "Nome exato ou padrao com coringas (ex.: *.BAS)")
        ->required();
    extract_cmd->add_option("--dir", extract_dir, "Subdiretorio de origem (default: raiz)");
    extract_cmd->add_option("-d,--dest", extract_dest, "Diretorio de destino (default: diretorio atual)");

    std::string delete_image;
    std::string delete_path;
    auto *delete_cmd = app.add_subcommand("delete", "Remove um arquivo da imagem");
    delete_cmd->add_option("imagem", delete_image, "Caminho do .dsk")->required();
    delete_cmd->add_option("arquivo", delete_path, "Caminho MSX do arquivo a remover")->required();

    std::string rename_image;
    std::string rename_path;
    std::string rename_new_name;
    auto *rename_cmd = app.add_subcommand("rename", "Renomeia um arquivo/diretorio (nao move entre pastas)");
    rename_cmd->alias("ren");
    rename_cmd->add_option("imagem", rename_image, "Caminho do .dsk")->required();
    rename_cmd->add_option("arquivo", rename_path, "Caminho MSX atual")->required();
    rename_cmd->add_option("novo_nome", rename_new_name, "Novo nome (sem separadores de diretorio)")->required();

    std::string mkdir_image;
    std::string mkdir_path;
    auto *mkdir_cmd = app.add_subcommand("mkdir", "Cria um subdiretorio (MSX-DOS 2)");
    mkdir_cmd->add_option("imagem", mkdir_image, "Caminho do .dsk")->required();
    mkdir_cmd->add_option("diretorio", mkdir_path, "Caminho MSX do novo diretorio")->required();

    std::string rmdir_image;
    std::string rmdir_path;
    auto *rmdir_cmd = app.add_subcommand("rmdir", "Remove um subdiretorio vazio (MSX-DOS 2)");
    rmdir_cmd->add_option("imagem", rmdir_image, "Caminho do .dsk")->required();
    rmdir_cmd->add_option("diretorio", rmdir_path, "Caminho MSX do diretorio a remover")->required();

    std::string copy_src;
    std::string copy_dest;
    auto *copy_cmd = app.add_subcommand("copy", "Clona uma imagem de disco inteira");
    copy_cmd->add_option("origem", copy_src, "Imagem .dsk de origem")->required();
    copy_cmd->add_option("destino", copy_dest, "Caminho do novo .dsk")->required();

    std::string saveas_image;
    std::string saveas_dest;
    auto *saveas_cmd = app.add_subcommand("saveas", "Salva a imagem carregada em outro caminho");
    saveas_cmd->add_option("imagem", saveas_image, "Caminho do .dsk de origem")->required();
    saveas_cmd->add_option("destino", saveas_dest, "Novo caminho do .dsk")->required();

    std::string info_image;
    auto *info_cmd = app.add_subcommand("info", "Mostra geometria e espaco livre da imagem");
    info_cmd->add_option("imagem", info_image, "Caminho do .dsk")->required();

    std::vector<std::string> argv_strings;
    argv_strings.reserve(tokens.size() + 1);
    argv_strings.emplace_back("msxdisk");
    for (const auto &t : tokens) argv_strings.push_back(t);

    std::vector<char *> argv_ptrs;
    argv_ptrs.reserve(argv_strings.size());
    for (auto &s : argv_strings) argv_ptrs.push_back(s.data());

    try {
        app.parse(static_cast<int>(argv_ptrs.size()), argv_ptrs.data());
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    if (*create_cmd) return CmdCreate(create_opts);
    if (*list_cmd) return CmdList(list_image, list_dir, list_pattern, list_tree);
    if (*add_cmd) return CmdAdd(add_image, add_host, add_path);
    if (*extract_cmd) return CmdExtract(extract_image, extract_dir, extract_pattern, extract_dest);
    if (*delete_cmd) return CmdDelete(delete_image, delete_path);
    if (*rename_cmd) return CmdRename(rename_image, rename_path, rename_new_name);
    if (*mkdir_cmd) return CmdMkdir(mkdir_image, mkdir_path);
    if (*rmdir_cmd) return CmdRmdir(rmdir_image, rmdir_path);
    if (*copy_cmd) return CmdCopy(copy_src, copy_dest);
    if (*saveas_cmd) return CmdSaveAs(saveas_image, saveas_dest);
    if (*info_cmd) return CmdInfo(info_image);

    return 1;
}

} // namespace msxdisk::cli
