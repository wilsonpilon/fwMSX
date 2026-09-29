//
// msxdisk (fwMSX): sessao do shell interativo (Fase 3b) -- uma imagem
// carregada em memoria sobre a qual os comandos operam direto, como a
// "conexao aberta" de um cliente FTP: 'load'/'create' abrem a sessao,
// os comandos seguintes atuam nela sem repetir o caminho do .dsk, e
// nada e gravado em disco ate 'save'/'saveas'.
//
#pragma once

#include <optional>
#include <string>

#include "../core/disk_image.h"

namespace msxdisk::shell {

class Session {
public:
    bool HasImage() const { return image_.has_value(); }
    bool IsDirty() const { return dirty_; }
    const std::string &ImagePath() const { return image_path_; }

    // Assume um DiskImage ja carregado (usado por CmdLoad e pelo 'create'
    // que auto-carrega a imagem recem-criada na sessao).
    void Attach(DiskImage image, std::string path);

    bool CmdLoad(const std::string &path);
    bool CmdSave();
    bool CmdSaveAs(const std::string &new_path);

    void CmdList(const std::string &dir_path, const std::string &pattern, bool tree) const;
    void CmdAdd(const std::string &host_path, const std::string &msx_path);
    void CmdExtract(const std::string &dir_path, const std::string &pattern, const std::string &dest_dir) const;

    // 'get' estilo FTP: um unico arquivo exato (sem coringa), destino
    // local opcional (default: o proprio nome do arquivo, no diretorio
    // atual). Diferente de CmdExtract, o destino aqui e o CAMINHO exato
    // do arquivo local, nao um diretorio.
    void CmdGet(const std::string &msx_path, const std::string &host_dest) const;

    // 'put' estilo FTP e apenas outro nome para CmdAdd (mesma semantica:
    // nome MSX opcional, default = nome do arquivo local).

    // mput/mget: varios arquivos de uma vez com coringa, confirmando cada
    // um (a menos que 'confirm_each' seja false -- comando 'prompt' do
    // shell liga/desliga isso).
    void CmdMput(const std::string &pattern, bool confirm_each);
    void CmdMget(const std::string &dir_path, const std::string &pattern, const std::string &dest_dir,
                  bool confirm_each) const;

    void CmdDelete(const std::string &msx_path);
    void CmdRename(const std::string &msx_path, const std::string &new_name);
    void CmdMkdir(const std::string &msx_path);
    void CmdRmdir(const std::string &msx_path);
    void CmdInfo() const;

private:
    void ReportNoImage(const char *command_name) const;

    std::optional<DiskImage> image_;
    std::string image_path_;
    bool dirty_ = false;
};

} // namespace msxdisk::shell
