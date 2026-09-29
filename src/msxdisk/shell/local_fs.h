//
// msxdisk (fwMSX): comandos locais do shell interativo (Fase 3b) --
// navegam o sistema de arquivos do HOST, nao a imagem MSX. Existem pra
// ajudar a achar o que passar pro 'add', igual ao lado local de um
// cliente FTP (aqui sem o prefixo 'l' de lftp/ftp: ls/cd/md/rm/pwd, ja
// que os nomes dos comandos do lado da imagem MSX sao outros --
// list/mkdir/rmdir/delete -- entao nao ha colisao dentro do shell).
//
#pragma once

#include <string>

namespace msxdisk::shell {

// Casamento de coringas generico (qualquer tamanho de string, case
// insensitive) -- usado por 'ls'/'mput'. Nao e o mesmo algoritmo do
// msxdisk_name_match em Assembly, que so entende os 11 bytes fixos do
// formato 8.3 do MSX-DOS.
bool LocalWildcardMatch(const std::string &pattern, const std::string &text);

// 'pattern' vazio lista tudo; aceita coringas '*'/'?' (comparacao local,
// sem limite de 8.3 -- diferente do casamento de nome MSX-DOS).
void CmdLocalList(const std::string &pattern);

// Muda o diretorio atual do processo (aceita "..", caminho absoluto, e no
// Windows "D:" para trocar de unidade).
void CmdLocalChangeDir(const std::string &path);

// Cria um diretorio local (e os pais que faltarem).
void CmdLocalMakeDir(const std::string &path);

// Remove um UNICO arquivo local (nunca diretorios, por seguranca -- ver
// doc/msxdisk-spec.md).
void CmdLocalRemoveFile(const std::string &path);

void CmdLocalPrintWorkingDir();

} // namespace msxdisk::shell
