// fwMSX -- entrada e saida de fita por EXTENSAO (.cas, .tsx/.tzx, .wav) e acesso aos arquivos logicos de uma imagem.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/tape-spec.md, secao 12.
//
// Antes disto cada chamador escolhia o leitor/escritor por conta propria (o motor, o `fwmsx --cas`, a janela). Agora os tres
// formatos passam por aqui, e o Mamute (que nao tem Z80 da fita nem motor) usa as mesmas funcoes para ler e gravar arquivos.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "tape_image.h"

namespace tape {

// Os formatos que as funcoes abaixo sabem, pela extensao (sem diferenciar maiusculas): "cas", "tsx" (tambem .tzx), "wav".
// Devolve "" se a extensao nao e' de fita.
std::string TapeExtension(const std::string &path);

// Le .cas, .tsx/.tzx ou .wav para `out`, pronto para os dois modos de carregamento. O .wav passa pelo ripper (so' o bloco KCS do
// MSX) e a imagem sai com pulsos sintetizados, como a de um .cas. Qualquer outra extensao e' tratada como .cas (o que o motor
// ja' fazia). Devolve false com `error` se nao abrir ou se o .wav nao tiver nenhum bloco.
bool LoadAnyTape(const std::string &path, TapeImage &out, std::string &error);

// Grava `fast_bytes`/`marks` em .cas (bytes crus), .tsx/.tzx (um bloco #4B por marca) ou .wav (PCM 16 bits mono, 44100 Hz, com os
// pulsos do MSX: piloto + KCS a 1200/2400 baud). `marks` vazio com bytes presentes e' recalculado varrendo os cabecalhos.
bool SaveAnyTape(const std::string &path, const std::vector<uint8_t> &fast_bytes, const std::vector<TapeMark> &marks, std::string &error);

// Escreve `pulses` (meios-periodos em T-states do Z80 a 3,579545 MHz) como .wav 16 bits mono 44100 Hz, com meio segundo de
// silencio antes e depois (o ripper precisa de silencio para achar o piloto).
bool WriteWavFromPulses(const std::string &path, const std::vector<uint32_t> &pulses, std::string &error);

// Regenera os pulsos exatos (piloto + KCS) de cada marca de `bytes`, SEM o enchimento de alinhamento entre os blocos, e atualiza
// `marks[i].pulse_index`.
void PulsesFromMarks(const std::vector<uint8_t> &bytes, std::vector<TapeMark> &marks, std::vector<uint32_t> &pulses);

// O conteudo de um arquivo logico da imagem (`index` de TapeImage::files).
struct TapeFileData {
    TapeFileType type = TapeFileType::Unknown;
    std::string name;
    std::vector<uint8_t> data; // Binary: os 6 bytes (ini, fim, exec) + o programa; Basic: o programa; Ascii: o texto inteiro
    // Binary: preenchidos a partir do cabecalho de 6 bytes do conteudo
    uint16_t bin_start = 0;
    uint16_t bin_end = 0;
    uint16_t bin_exec = 0;
};

// Extrai o arquivo `index`. Um .BIN (0xD0) devolve exatamente (fim - inicio + 7) bytes (descarta o enchimento do final); um
// .BAS (0xD3) o primeiro bloco de dados; um ASCII (0xEA) todos os blocos de dados ate' o proximo arquivo.
bool ReadTapeFile(const TapeImage &img, std::size_t index, TapeFileData &out, std::string &error);

// Acrescenta um arquivo (cabecalho com nome + bloco de dados) ao FIM da imagem, gerando tambem os pulsos para o modo normal.
// `type_id` e' kCasIdBinary/kCasIdBasic/kCasIdAscii (cas_format.h). Nao grava em disco.
void AppendTapeFile(TapeImage &img, uint8_t type_id, const std::string &name, const std::vector<uint8_t> &data);

// Monta o conteudo de um arquivo Binary: (ini, fim, exec) em little endian + os bytes (fim = ini + n - 1).
std::vector<uint8_t> MakeBinaryContent(uint16_t start, uint16_t exec, const std::vector<uint8_t> &program);

// Corta a imagem a partir do arquivo logico `index` (apaga ele e tudo depois, como regravar a fita a partir dali).
void TruncateAtFile(TapeImage &img, std::size_t index);

} // namespace tape
