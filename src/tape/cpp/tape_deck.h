// fwMSX -- o "toca-fitas" sem hardware: a fita inserida, a protecao contra gravacao, o modo de gravacao e o ponto marcado, com
// leitura e gravacao de ARQUIVOS logicos direto na imagem (sem BIOS, sem Z80). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/tape-spec.md, secao 12.
//
// O TapeEngine (tape_device.h) herda daqui e acrescenta o que e' do emulador (ganchos de BIOS, pulsos ao vivo, PPI/PSG). O Mamute
// OFFLINE usa um TapeDeck puro: o mesmo comportamento de protecao/corte/ponto, sem precisar de uma maquina emulada.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "tape_image.h"
#include "tape_io.h"

namespace tape {

// O que fazer no PROXIMO TAPOON (ou gravacao de arquivo) com sucesso: fitas de verdade sao lineares -- gravar a partir de um ponto
// destroi fisicamente o que vinha depois. NewTape/OverwriteAtPoint valem para UMA gravacao; depois o deck volta sozinho para
// AppendAtEnd (um CSAVE chama TAPOON varias vezes e so' a primeira pode cortar).
enum class TapeWriteMode {
    AppendAtEnd,      // grava depois do ultimo arquivo (o padrao: "ir enchendo a fita")
    OverwriteAtPoint, // trunca a partir do arquivo marcado (SeekToFile()) e grava ali
    NewTape,          // esquece tudo que havia e comeca do zero
};

// "Nenhum arquivo marcado" (SeekToFile()/marked_file()).
constexpr std::size_t kNoMark = static_cast<std::size_t>(-1);

class TapeDeck {
public:
    virtual ~TapeDeck() = default;

    // Insere a fita de um arquivo (.cas, .tsx/.tzx ou .wav pela extensao). Entra protegida, salvo reinserir a MESMA fita (um
    // "destravar" anterior nao se perde).
    bool Insert(const std::string &path, std::string &error);
    // Fita NOVA e vazia, destravada, gravada em `path` ja' (so' o cabecalho) para o caminho existir.
    bool NewBlank(const std::string &path, std::string &error);
    void Eject();

    bool inserted() const { return inserted_; }
    const std::string &path() const { return path_; }
    const TapeImage &image() const { return image_; }
    const std::vector<TapeFileEntry> &files() const { return image_.files; }
    const std::vector<std::string> &skipped_blocks() const { return image_.skipped_blocks; }

    bool read_only() const { return read_only_; }
    void SetReadOnly(bool on) { read_only_ = on; }
    void SetWriteMode(TapeWriteMode mode) { write_mode_ = mode; }
    TapeWriteMode write_mode() const { return write_mode_; }

    // Marca o arquivo `index` como o "ponto": onde o carregamento comeca e onde OverwriteAtPoint corta. false = indice invalido.
    virtual bool SeekToFile(std::size_t index);
    void ClearMark() { marked_file_ = kNoMark; }
    // Volta ao inicio da fita (o deck puro nao tem cabeca de leitura; o motor reposiciona os cursores).
    virtual void Rewind() {}
    std::size_t marked_file() const { return marked_file_; }

    // Acrescenta um arquivo (cabecalho com nome + dados) respeitando a protecao e o modo de gravacao, e grava no disco.
    // `type_id`: kCasIdBinary/kCasIdBasic/kCasIdAscii (cas_format.h).
    bool AppendFile(uint8_t type_id, const std::string &name, const std::vector<uint8_t> &data, std::string &error);
    // O conteudo do arquivo logico `index` (ver ReadTapeFile).
    bool ReadFile(std::size_t index, TapeFileData &out, std::string &error) const;
    // Acha um arquivo pelo nome (sem diferenciar maiusculas; o primeiro a partir de `from`) ou pelo numero. false = nao achou.
    bool FindFile(const std::string &name_or_index, std::size_t from, std::size_t &index) const;
    // Grava a imagem atual em OUTRO arquivo (a fita inserida continua a mesma).
    bool SaveAs(const std::string &path, std::string &error) const;
    // Grava a imagem no arquivo da fita inserida (melhor esforco: uma falha nao desfaz a memoria).
    void Persist();

protected:
    // O que e' feito DEPOIS de trocar a fita inteira (Insert/NewBlank/Eject) e depois de mudar so' os pulsos (gravacao/corte).
    // O motor religa os cursores aqui; o deck puro nao faz nada.
    virtual void OnReplaced() {}
    virtual void OnPulsesChanged() {}
    // Aplica o corte de UMA VEZ do modo de gravacao (NewTape/OverwriteAtPoint) e volta para AppendAtEnd.
    void CutForWrite();

    bool inserted_ = false;
    std::string path_;
    TapeImage image_;
    bool read_only_ = true;
    TapeWriteMode write_mode_ = TapeWriteMode::AppendAtEnd;
    std::size_t marked_file_ = kNoMark;
};

// Texto para o usuario (minusculo; o Mamute passa para maiusculas): o estado do deck numa linha e a lista de arquivos, um por
// linha ("> 1 BIN   NOME   4000-40FF exec 4000  256 bytes"; o ">" marca o ponto).
std::string DescribeDeck(const TapeDeck &deck);
std::string ListDeck(const TapeDeck &deck);
// "bin"/"bas"/"asc"/"?" -- o tipo do arquivo logico.
const char *FileTypeName(TapeFileType type);

} // namespace tape
