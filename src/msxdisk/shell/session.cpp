//
// msxdisk (fwMSX): sessao do shell interativo -- ver session.h.
//

#include "session.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>

#include "../core/dir_entry.h"
#include "../cli/format.h"
#include "local_fs.h"

namespace msxdisk::shell {

namespace {

// Confirmacao "y/n" (aceita tambem "s"/"sim") lida direto de std::cin --
// nao usa o replxx aqui (nao precisa de historico/edicao pra uma
// pergunta de uma letra so).
bool ConfirmYesNo(const std::string &message) {
    std::cout << message << " (s/n) ";
    std::string response;
    if (!std::getline(std::cin, response) || response.empty()) return false;
    const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(response[0])));
    return c == 's' || c == 'y';
}

// Resolve 'path' para absoluto USANDO O DIRETORIO ATUAL NESTE INSTANTE.
// Essencial pra guardar em image_path_: sem isso, um caminho relativo
// guardado como veio (ex.: "teste.dsk") passaria a apontar pra outro
// arquivo se o 'cd' local mudasse o diretorio do processo depois do
// load/create -- 'save' reabriria "teste.dsk" relativo ao diretorio
// ATUAL (errado), nao ao diretorio de quando foi carregado (correto).
std::string ResolveForStorage(const std::string &path) {
    std::error_code ec;
    const auto abs = std::filesystem::absolute(path, ec);
    return ec ? path : abs.string();
}

} // namespace

void Session::Attach(DiskImage image, std::string path) {
    image_ = std::move(image);
    image_path_ = ResolveForStorage(path);
    dirty_ = false;
}

void Session::ReportNoImage(const char *command_name) const {
    std::cerr << "msxdisk: nenhuma imagem carregada -- use 'load <imagem.dsk>' antes de '" << command_name << "'"
               << std::endl;
}

bool Session::CmdLoad(const std::string &path) {
    if (image_.has_value() && dirty_) {
        std::cout << "Aviso: a imagem '" << image_path_
                   << "' tem alteracoes nao salvas (use 'save' antes de trocar de imagem)." << std::endl;
    }

    DiskError error = DiskError::None;
    auto image = DiskImage::Load(path, &error);
    if (!image) {
        std::cerr << "load: nao foi possivel abrir '" << path << "': " << cli::DiskErrorMessage(error) << std::endl;
        return false;
    }

    Attach(std::move(*image), path);
    std::cout << "Imagem carregada: " << path << std::endl;
    return true;
}

bool Session::CmdSave() {
    if (!HasImage()) {
        ReportNoImage("save");
        return false;
    }
    if (!cli::SaveOrReport(*image_, image_path_)) return false;

    dirty_ = false;
    std::cout << "Salvo: " << image_path_ << std::endl;
    return true;
}

bool Session::CmdSaveAs(const std::string &new_path) {
    if (!HasImage()) {
        ReportNoImage("saveas");
        return false;
    }
    if (!cli::SaveOrReport(*image_, new_path)) return false;

    image_path_ = ResolveForStorage(new_path);
    dirty_ = false;
    std::cout << "Salvo como: " << new_path << std::endl;
    return true;
}

void Session::CmdList(const std::string &dir_path, const std::string &pattern, bool tree) const {
    if (!HasImage()) {
        ReportNoImage("list");
        return;
    }

    if (tree) {
        const int total = cli::PrintTree(*image_, dir_path, 0);
        std::cout << total << " entrada(s)." << std::endl;
        return;
    }

    DiskError error = DiskError::None;
    const auto entries = image_->ListDirectory(dir_path, pattern, &error);
    if (error != DiskError::None) {
        std::cerr << "list: falha ao listar '" << (dir_path.empty() ? "\\" : dir_path)
                   << "': " << cli::DiskErrorMessage(error) << std::endl;
        return;
    }
    if (entries.empty()) {
        std::cout << "(nenhuma entrada)" << std::endl;
        return;
    }
    for (const auto &e : entries) cli::PrintEntry(e, 0);
    std::cout << entries.size() << " entrada(s)." << std::endl;
}

void Session::CmdAdd(const std::string &host_path, const std::string &msx_path) {
    if (!HasImage()) {
        ReportNoImage("add");
        return;
    }
    if (!cli::CheckLocalFileReadable(host_path, "add")) return;

    const std::string name = msx_path.empty() ? std::filesystem::path(host_path).filename().string() : msx_path;

    DiskError error = DiskError::None;
    if (!image_->AddFile(host_path, name, msxdisk::kAttrArchive, &error)) {
        std::cerr << "add: falha ao adicionar '" << host_path << "': " << cli::DiskErrorMessage(error) << std::endl;
        return;
    }
    dirty_ = true;
    std::cout << "Adicionado (em memoria -- use 'save' para gravar): " << name << std::endl;
}

void Session::CmdExtract(const std::string &dir_path, const std::string &pattern,
                          const std::string &dest_dir) const {
    if (!HasImage()) {
        ReportNoImage("extract");
        return;
    }

    DiskError error = DiskError::None;
    const int count = image_->ExtractMatching(dir_path, pattern, dest_dir, &error);
    if (count == 0) {
        std::cerr << "extract: falha ao extrair '" << pattern << "': " << cli::DiskErrorMessage(error) << std::endl;
        return;
    }
    std::cout << count << " arquivo(s) extraido(s) para " << dest_dir << std::endl;
}

void Session::CmdGet(const std::string &msx_path, const std::string &host_dest) const {
    if (!HasImage()) {
        ReportNoImage("get");
        return;
    }

    std::string dest = host_dest;
    if (dest.empty()) {
        const auto pos = msx_path.find_last_of("\\/");
        dest = (pos == std::string::npos) ? msx_path : msx_path.substr(pos + 1);
    }

    DiskError error = DiskError::None;
    if (!image_->ExtractFile(msx_path, dest, &error)) {
        std::cerr << "get: falha ao receber '" << msx_path << "': " << cli::DiskErrorMessage(error) << std::endl;
        return;
    }
    std::cout << "Recebido: " << msx_path << " -> " << dest << std::endl;
}

void Session::CmdMput(const std::string &pattern, bool confirm_each) {
    if (!HasImage()) {
        ReportNoImage("mput");
        return;
    }

    namespace fs = std::filesystem;
    std::error_code ec;
    std::vector<std::string> matches;
    for (const auto &entry : fs::directory_iterator(fs::current_path(), ec)) {
        if (!entry.is_regular_file()) continue;
        if (LocalWildcardMatch(pattern, entry.path().filename().string())) {
            matches.push_back(entry.path().filename().string());
        }
    }
    std::sort(matches.begin(), matches.end());

    if (matches.empty()) {
        std::cout << "mput: nenhum arquivo local combina com '" << pattern << "'" << std::endl;
        return;
    }

    int sent = 0;
    for (const auto &name : matches) {
        if (confirm_each && !ConfirmYesNo("Enviar '" + name + "'?")) continue;

        DiskError error = DiskError::None;
        if (!image_->AddFile(name, name, msxdisk::kAttrArchive, &error)) {
            std::cerr << "mput: falha ao enviar '" << name << "': " << cli::DiskErrorMessage(error) << std::endl;
            continue;
        }
        dirty_ = true;
        std::cout << "Enviado (em memoria): " << name << std::endl;
        ++sent;
    }
    std::cout << sent << " de " << matches.size() << " arquivo(s) enviado(s)." << std::endl;
}

void Session::CmdMget(const std::string &dir_path, const std::string &pattern, const std::string &dest_dir,
                       bool confirm_each) const {
    if (!HasImage()) {
        ReportNoImage("mget");
        return;
    }

    DiskError error = DiskError::None;
    const auto entries = image_->ListDirectory(dir_path, pattern, &error);
    if (error != DiskError::None) {
        std::cerr << "mget: falha ao listar '" << (dir_path.empty() ? "\\" : dir_path)
                   << "': " << cli::DiskErrorMessage(error) << std::endl;
        return;
    }
    if (entries.empty()) {
        std::cout << "mget: nenhum arquivo na imagem combina com '" << pattern << "'" << std::endl;
        return;
    }

    std::error_code ec;
    std::filesystem::create_directories(dest_dir, ec);

    int received = 0;
    int candidates = 0;
    for (const auto &fe : entries) {
        if (fe.attr & msxdisk::kAttrDirectory) continue; // mget nao desce em subdiretorios
        ++candidates;
        if (confirm_each && !ConfirmYesNo("Receber '" + fe.name + "'?")) continue;

        DiskError file_error = DiskError::None;
        const std::string full = cli::JoinMsxPath(dir_path, fe.name);
        const auto local_dest = std::filesystem::path(dest_dir) / fe.name;
        if (!image_->ExtractFile(full, local_dest, &file_error)) {
            std::cerr << "mget: falha ao receber '" << fe.name << "': " << cli::DiskErrorMessage(file_error)
                       << std::endl;
            continue;
        }
        std::cout << "Recebido: " << fe.name << std::endl;
        ++received;
    }
    std::cout << received << " de " << candidates << " arquivo(s) recebido(s)." << std::endl;
}

void Session::CmdDelete(const std::string &msx_path) {
    if (!HasImage()) {
        ReportNoImage("delete");
        return;
    }

    DiskError error = DiskError::None;
    if (!image_->DeleteFile(msx_path, &error)) {
        std::cerr << "delete: falha ao excluir '" << msx_path << "': " << cli::DiskErrorMessage(error) << std::endl;
        return;
    }
    dirty_ = true;
    std::cout << "Excluido (em memoria -- use 'save' para gravar): " << msx_path << std::endl;
}

void Session::CmdRename(const std::string &msx_path, const std::string &new_name) {
    if (!HasImage()) {
        ReportNoImage("rename");
        return;
    }

    DiskError error = DiskError::None;
    if (!image_->RenameFile(msx_path, new_name, &error)) {
        std::cerr << "rename: falha ao renomear '" << msx_path << "': " << cli::DiskErrorMessage(error) << std::endl;
        return;
    }
    dirty_ = true;
    std::cout << "Renomeado (em memoria -- use 'save' para gravar): " << msx_path << " -> " << new_name << std::endl;
}

void Session::CmdMkdir(const std::string &msx_path) {
    if (!HasImage()) {
        ReportNoImage("mkdir");
        return;
    }

    DiskError error = DiskError::None;
    if (!image_->MakeDirectory(msx_path, &error)) {
        std::cerr << "mkdir: falha ao criar diretorio '" << msx_path << "': " << cli::DiskErrorMessage(error)
                   << std::endl;
        return;
    }
    dirty_ = true;
    std::cout << "Diretorio criado (em memoria -- use 'save' para gravar): " << msx_path << std::endl;
}

void Session::CmdRmdir(const std::string &msx_path) {
    if (!HasImage()) {
        ReportNoImage("rmdir");
        return;
    }

    DiskError error = DiskError::None;
    if (!image_->RemoveDirectory(msx_path, &error)) {
        std::cerr << "rmdir: falha ao remover diretorio '" << msx_path << "': " << cli::DiskErrorMessage(error)
                   << std::endl;
        return;
    }
    dirty_ = true;
    std::cout << "Diretorio removido (em memoria -- use 'save' para gravar): " << msx_path << std::endl;
}

void Session::CmdInfo() const {
    if (!HasImage()) {
        ReportNoImage("info");
        return;
    }

    const auto &g = image_->geometry();
    int64_t free_bytes = 0;
    float free_percent = 0.0f;
    image_->CapacityStats(&free_bytes, &free_percent);

    std::cout << "Imagem: " << image_path_ << (dirty_ ? " (alteracoes nao salvas)" : "") << std::endl;
    std::cout << "  Tamanho total: " << g.ImageSizeBytes() << " bytes" << std::endl;
    std::cout << "  Setores/cluster: " << static_cast<int>(g.sectors_per_cluster) << "  Clusters de dados: "
               << g.TotalClusters() << std::endl;
    std::cout << "  Espaco livre: " << free_bytes << " bytes (" << std::fixed << std::setprecision(1)
               << free_percent << "%)" << std::endl;
    std::cout << "  Entradas no diretorio raiz: " << g.root_dir_entries << std::endl;
}

} // namespace msxdisk::shell
