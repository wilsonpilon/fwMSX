// fwMSX -- configuracoes do usuario gravadas em arquivo (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/splash-spec.md.
//
// Um arquivo de texto `chave=valor` (fwmsx.cfg) ao lado do executavel: o tema da janela, o tamanho da letra,
// a moldura da tela e se o splash aparece ao abrir. Arquivo ausente ou chave desconhecida = padrao.
#pragma once

#include <string>

namespace settings {

struct Settings {
    bool splash = true; // mostra a imagem de abertura antes do emulador (so' na janela)
    int theme = 0;      // 0 = escuro, 1 = claro
    int font = 1;       // 0 = normal, 1 = grande, 2 = muito grande
    bool frame = true;  // borda e sombra em volta da tela do MSX
};

// Caminho do arquivo de configuracao: `exe_dir`/fwmsx.cfg (ou ./fwmsx.cfg sem pasta).
std::string PathFor(const std::string &exe_dir);

// Le o arquivo; devolve os padroes se nao existir ou estiver ilegivel.
Settings Load(const std::string &path);

// Grava o arquivo. false se nao for possivel (pasta somente leitura, por exemplo).
bool Save(const std::string &path, const Settings &settings);

} // namespace settings
