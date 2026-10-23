// fwMSX -- um cartao SD (ou SDHC) em modo SPI sobre uma imagem HdImage. Codigo ORIGINAL do fwMSX (BSD-3-Clause), escrito a partir da
// especificacao SD (Physical Layer Simplified): comandos de 6 bytes, respostas R1/R3/R7, leitura e escrita de blocos de 512 bytes, CSD v1/v2.
// Tudo termina na hora: cada byte trocado (Transfer) ja' devolve o byte seguinte do cartao. Usado pelo MegaSD (megasd.h). Ver doc/hd-spec.md.
#pragma once

#include <cstdint>
#include <vector>

#include "hd_image.h"

namespace hd {

class SdCard {
public:
    // `image` fica com o dono; nullptr = sem cartao no soquete (Transfer devolve FFh).
    explicit SdCard(HdImage *image = nullptr);

    bool present() const { return image_ != nullptr; }
    // Imagens de ate' 2 GB sao SDSC (enderecos em bytes); acima disso SDHC (enderecos em setores).
    bool sdhc() const { return sdhc_; }

    // /CS do cartao: false = selecionado (nivel baixo). Desselecionar reinicia o analisador de comandos.
    void Select(bool selected);
    // Um byte no barramento SPI: `out` vai ao cartao e o byte que o cartao esta' enviando volta.
    uint8_t Transfer(uint8_t out);

    // Ultimo comando que o cartao nao conhece (0 = nenhum) e contadores, para o diagnostico.
    uint8_t last_unsupported() const { return last_unsupported_; }
    uint32_t sectors_read() const { return sectors_read_; }
    uint32_t sectors_written() const { return sectors_written_; }

private:
    enum class State { Command, WriteToken, WriteData, MultiWriteToken };

    void Execute();
    void Respond(std::vector<uint8_t> bytes, int lead = 2);
    void QueueReadBlock(uint32_t sector);
    void BuildCsd(uint8_t *csd) const;
    uint32_t SectorFromArg(uint32_t arg) const { return sdhc_ ? arg : arg / HD_SECTOR_SIZE; }

    HdImage *image_ = nullptr;
    bool selected_ = false;
    bool sdhc_ = false;
    bool idle_ = true;       // ainda nao saiu do estado "idle" (CMD0 -> ACMD41)
    int init_polls_ = 0;     // quantos ACMD41 ja' vieram
    bool app_cmd_ = false;   // o ultimo comando foi CMD55
    State state_ = State::Command;
    uint8_t cmd_[6] = {};
    int cmd_len_ = 0;
    std::vector<uint8_t> out_;  // bytes a enviar (resposta, dados)
    size_t out_pos_ = 0;
    bool multi_read_ = false;
    bool multi_write_ = false;
    uint32_t next_sector_ = 0;
    uint32_t write_sector_ = 0;
    std::vector<uint8_t> wbuf_;
    uint8_t last_unsupported_ = 0;
    uint32_t sectors_read_ = 0, sectors_written_ = 0;
};

} // namespace hd
