// Teste do disco rigido (src/hd): formato MBR + FAT16 (C/Fortran/Assembly), imagem em arquivo, disco ATA e interface Sunrise IDE.
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/hd/cpp/cli.h"
#include "../../src/hd/cpp/hd_image.h"
#include "../../src/hd/cpp/ide_disk.h"
#include "../../src/hd/cpp/megasd.h"
#include "../../src/hd/cpp/sd_card.h"
#include "../../src/hd/cpp/sunrise_ide.h"

namespace fs = std::filesystem;

static int g_failures = 0;

static void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

static uint32_t Le32(const uint8_t *p) { return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24); }
static uint32_t Le16(const uint8_t *p) { return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8); }

// Manda um comando SPI de 6 bytes e devolve a resposta R1 (procura o 1o byte != FFh nos 16 seguintes, como os drivers).
static uint8_t SdCmd(hd::SdCard &c, uint8_t cmd, uint32_t arg, uint8_t crc = 0x95) {
    c.Transfer(0xFF);
    c.Transfer(static_cast<uint8_t>(0x40 | cmd));
    c.Transfer(static_cast<uint8_t>(arg >> 24));
    c.Transfer(static_cast<uint8_t>(arg >> 16));
    c.Transfer(static_cast<uint8_t>(arg >> 8));
    c.Transfer(static_cast<uint8_t>(arg));
    c.Transfer(crc);
    for (int i = 0; i < 16; ++i) {
        const uint8_t r = c.Transfer(0xFF);
        if (r != 0xFF) return r;
    }
    return 0xFF;
}

// A capacidade em setores que o CSD diz (o mesmo calculo do driver do MegaSD).
static uint64_t CsdSectors(const uint8_t *csd) {
    auto bits = [&](int hi, int lo) {
        uint64_t v = 0;
        for (int b = hi; b >= lo; --b) v = (v << 1) | ((csd[15 - b / 8] >> (b % 8)) & 1u);
        return v;
    };
    if (bits(127, 126) == 1) return (bits(69, 48) + 1) * 1024;
    return (bits(73, 62) + 1) << (bits(49, 47) + 2 + bits(83, 80) - 9);
}

// Operacoes de registrador do ATA como o driver do Nextor faria.
static void Cmd(hd::IdeDisk &d, uint8_t cmd, uint32_t lba, int count) {
    d.WriteReg(2, static_cast<uint8_t>(count));
    d.WriteReg(3, static_cast<uint8_t>(lba));
    d.WriteReg(4, static_cast<uint8_t>(lba >> 8));
    d.WriteReg(5, static_cast<uint8_t>(lba >> 16));
    d.WriteReg(6, static_cast<uint8_t>(0xE0 | ((lba >> 24) & 0x0F)));
    d.WriteReg(7, cmd);
}

int main() {
    const fs::path tmp = fs::temp_directory_path() / "fwmsx_hdtest";
    std::error_code ec;
    fs::remove_all(tmp, ec);
    fs::create_directories(tmp);
    std::string error;

    // --- tamanhos ---------------------------------------------------------------------------------------------------------------------
    uint32_t mb = 0;
    check(hd::ParseSize("512", mb, error) && mb == 512 && hd::ParseSize("512M", mb, error) && mb == 512 && hd::ParseSize("2g", mb, error) && mb == 2048 &&
              !hd::ParseSize("abc", mb, error) && !hd::ParseSize("2T", mb, error) && !hd::ParseSize("0", mb, error),
          "ParseSize: 512, 512M, 2g; recusa texto, unidade desconhecida e zero");
    check(hd::ValidatePartitionSizes({2048, 2048}, error) && hd::ValidatePartitionSizes({8}, error), "particoes validas: 2 x 2048 MB e 8 MB");
    check(!hd::ValidatePartitionSizes({}, error) && !hd::ValidatePartitionSizes({4}, error) && !hd::ValidatePartitionSizes({2049}, error) &&
              !hd::ValidatePartitionSizes({2048, 2048, 8}, error) && !hd::ValidatePartitionSizes({8, 8, 8, 8, 8}, error),
          "recusa: nenhuma, 4 MB, 2049 MB, soma de 4104 MB, 5 particoes");

    // --- o plano do FAT16 (C + Fortran) -----------------------------------------------------------------------------------------------
    for (const uint32_t size_mb : {8u, 100u, 512u, 2047u, 2048u}) {
        HdFat16Plan plan;
        const bool ok = hd_fat16_plan(size_mb * 2048u, &plan) == 0;
        const uint32_t root_sectors = plan.root_entries * 32u / 512u;
        const bool fits = ok && plan.clusters >= 4085 && plan.clusters <= 65524 &&
                          plan.reserved_sectors + 2u * plan.fat_sectors + root_sectors + plan.clusters * plan.sectors_per_cluster <= plan.total_sectors &&
                          (plan.clusters + 2u) * 2u <= plan.fat_sectors * 512u;
        check(fits, "plano FAT16 de " + std::to_string(size_mb) + " MB: cluster de " + std::to_string(plan.sectors_per_cluster * 512u) + " bytes, " + std::to_string(plan.clusters) +
                        " clusters, FAT de " + std::to_string(plan.fat_sectors) + " setores");
    }
    {
        HdFat16Plan plan;
        check(hd_fat16_plan(2048u * 2048u, &plan) == 0 && plan.sectors_per_cluster == 64, "2048 MB usa cluster de 32 KB (o maximo do FAT16 do Nextor)");
        check(hd_fat16_plan(7u * 2048u, &plan) != 0 && hd_fat16_plan(2049u * 2048u, &plan) != 0, "7 MB e 2049 MB ficam fora do plano");
    }

    // --- a imagem ----------------------------------------------------------------------------------------------------------------------
    const std::string img = (tmp / "disco.hdd").string();
    check(hd::CreateHdImage(img, {16, 32}, error), "CreateHdImage: 16 MB + 32 MB (" + error + ")");
    check(fs::file_size(img) == (1ull + 16 + 32) * 1024 * 1024, "o arquivo tem 1 MiB (area do MBR) + 48 MB");
    std::vector<uint8_t> raw;
    {
        std::ifstream f(img, std::ios::binary);
        raw.resize(2049 * 512);
        f.read(reinterpret_cast<char *>(raw.data()), static_cast<std::streamsize>(raw.size()));
    }
    check(raw[510] == 0x55 && raw[511] == 0xAA, "MBR: assinatura 55AAh");
    const uint8_t *e0 = raw.data() + 446;
    const uint8_t *e1 = raw.data() + 462;
    check(e0[0] == 0x80 && e0[4] == 0x06 && Le32(e0 + 8) == 2048 && Le32(e0 + 12) == 16 * 2048, "MBR: 1a particao ativa, tipo 06h, comeca em 2048, 16 MB");
    check(e1[0] == 0x00 && e1[4] == 0x06 && Le32(e1 + 8) == 2048 + 16 * 2048 && Le32(e1 + 12) == 32 * 2048, "MBR: 2a particao logo depois, 32 MB");
    check(raw[446 + 32 + 4] == 0 && raw[446 + 48 + 4] == 0, "MBR: 3a e 4a entradas vazias");
    const uint8_t *bs = raw.data() + 2048 * 512;
    check(bs[510] == 0x55 && bs[511] == 0xAA && Le16(bs + 11) == 512 && bs[16] == 2 && Le16(bs + 17) == 512 && bs[21] == 0xF8, "setor de boot: BPB com 512 bytes/setor, 2 FATs, 512 entradas na raiz, media F8h");
    check(Le32(bs + 28) == 2048, "setor de boot: setores escondidos = inicio da particao");
    {
        HdFat16Plan plan;
        hd_fat16_plan(16 * 2048u, &plan);
        check(bs[13] == plan.sectors_per_cluster && Le16(bs + 22) == plan.fat_sectors && Le16(bs + 19) == 16 * 2048u && Le32(bs + 32) == 0, "setor de boot: cluster e FAT iguais ao plano; total de 16 bits (< 65536 setores)");
        // a FAT: media + fim de cadeia, e a segunda copia igual
        hd::HdImage open;
        check(open.Open(img, true, error) && open.sectors() == 2048 + 48 * 2048u, "HdImage::Open le o tamanho em setores");
        uint8_t fat1[512], fat2[512];
        open.ReadSector(2048 + 1, fat1);
        open.ReadSector(2048 + 1 + plan.fat_sectors, fat2);
        check(fat1[0] == 0xF8 && fat1[1] == 0xFF && fat1[2] == 0xFF && fat1[3] == 0xFF && std::memcmp(fat1, fat2, 512) == 0, "FAT: F8 FF FF FF e a 2a copia igual");
        std::vector<hd::PartitionInfo> parts;
        check(hd::ReadPartitions(open, parts, error) && parts.size() == 2 && parts[0].start == 2048 && parts[1].sectors == 32 * 2048u, "ReadPartitions: 2 particoes");
        std::string text;
        check(hd::DescribeImage(img, text, error) && text.find("particao 2") != std::string::npos, "DescribeImage lista as particoes");
        uint8_t dummy[512] = {};
        check(!open.WriteSector(0, dummy), "imagem aberta so' para leitura recusa gravar");
        check(!open.ReadSector(open.sectors(), dummy), "ler alem do fim falha");
    }
    {
        std::ofstream bad(tmp / "torto.hdd", std::ios::binary);
        bad << "123";
    }
    hd::HdImage badimg;
    check(!badimg.Open((tmp / "torto.hdd").string(), false, error) && !badimg.Open((tmp / "nao_existe.hdd").string(), false, error), "imagem que nao e' multiplo de 512 bytes ou que nao existe: erro claro");
    check(!hd::CreateHdImage((tmp / "x.hdd").string(), {2049}, error) && !fs::exists(tmp / "x.hdd"), "CreateHdImage recusa tamanho invalido e nao deixa arquivo");

    // --- o disco ATA --------------------------------------------------------------------------------------------------------------------
    hd::HdImage rw;
    check(rw.Open(img, false, error), "imagem aberta para leitura e escrita");
    hd::IdeDisk disk(&rw);
    check(disk.ReadReg(7) == 0x50 && disk.ReadReg(1) == 0x01, "reset: estado DRDY|DSC (50h) e diagnostico 01h");
    Cmd(disk, 0xEC, 0, 0);
    check((disk.ReadReg(7) & hd::IdeDisk::kDrq) != 0, "IDENTIFY DEVICE: DRQ ligado");
    std::vector<uint16_t> id(256);
    for (auto &w : id) w = disk.ReadData();
    check((disk.ReadReg(7) & hd::IdeDisk::kDrq) == 0 && disk.ReadReg(7) == 0x50, "IDENTIFY: DRQ cai depois das 256 palavras");
    check((id[49] & 0x0200) != 0 && (static_cast<uint32_t>(id[61]) << 16 | id[60]) == rw.sectors(), "IDENTIFY: LBA suportado e o total de setores bate com a imagem");
    std::string model;
    for (int i = 27; i < 47; ++i) {
        model += static_cast<char>(id[i] >> 8);
        model += static_cast<char>(id[i] & 0xFF);
    }
    check(model.compare(0, 15, "fwMSX hard disk") == 0, "IDENTIFY: o modelo vem com os pares de bytes trocados (Assembly) - \"" + model.substr(0, 15) + "\"");
    check(id[3] == 16 && id[6] == 63, "IDENTIFY: 16 cabecas, 63 setores por trilha");

    Cmd(disk, 0x20, 0, 1);
    check(disk.ReadReg(7) == 0x58, "READ SECTORS: DRQ ligado (58h)");
    std::vector<uint8_t> sector0;
    for (int i = 0; i < 256; ++i) {
        const uint16_t w = disk.ReadData();
        sector0.push_back(static_cast<uint8_t>(w & 0xFF));
        sector0.push_back(static_cast<uint8_t>(w >> 8));
    }
    check(std::equal(sector0.begin(), sector0.end(), raw.begin()), "READ SECTORS: o setor 0 lido pelo ATA e' o MBR do arquivo");
    check(disk.ReadReg(7) == 0x50, "READ SECTORS: estado volta a 50h");

    // varios setores de uma vez
    Cmd(disk, 0x20, 2048, 2);
    int words = 0;
    while (disk.ReadReg(7) & hd::IdeDisk::kDrq) {
        disk.ReadData();
        ++words;
        if (words > 1000) break;
    }
    check(words == 512, "READ SECTORS com contagem 2: 512 palavras seguidas");

    // escrita
    Cmd(disk, 0x30, 5000, 1);
    check(disk.ReadReg(7) == 0x58, "WRITE SECTORS: DRQ ligado");
    for (int i = 0; i < 256; ++i) disk.WriteData(static_cast<uint16_t>(0xA500 | i));
    check(disk.ReadReg(7) == 0x50 && disk.sectors_written() == 1, "WRITE SECTORS: gravou 1 setor e voltou a 50h");
    Cmd(disk, 0x20, 5000, 1);
    bool same = true;
    for (int i = 0; i < 256; ++i) same = same && disk.ReadData() == (0xA500 | i);
    check(same, "o setor gravado volta igual pela leitura");
    {
        std::ifstream f(img, std::ios::binary);
        f.seekg(5000 * 512);
        uint8_t b[4] = {};
        f.read(reinterpret_cast<char *>(b), 4);
        check(b[0] == 0x00 && b[1] == 0xA5 && b[2] == 0x01 && b[3] == 0xA5, "o arquivo .hdd tem os bytes gravados (a escrita vai direto ao arquivo)");
    }

    // erros
    Cmd(disk, 0x20, rw.sectors() - 1, 2);
    check((disk.ReadReg(7) & hd::IdeDisk::kErr) != 0 && (disk.ReadReg(1) & hd::IdeDisk::kIdnf) != 0, "READ alem do fim: ERR + IDNF");
    Cmd(disk, 0xC4, 0, 1);
    check((disk.ReadReg(7) & hd::IdeDisk::kErr) != 0 && (disk.ReadReg(1) & hd::IdeDisk::kAbrt) != 0 && disk.last_unsupported() == 0xC4, "comando desconhecido: ERR + ABRT e fica registrado");
    Cmd(disk, 0x91, 0, 0);
    check(disk.ReadReg(7) == 0x50, "INITIALIZE DEVICE PARAMETERS: aceito");
    // modo CHS
    disk.WriteReg(2, 1);
    disk.WriteReg(3, 1);
    disk.WriteReg(4, 0);
    disk.WriteReg(5, 0);
    disk.WriteReg(6, 0xA0); // CHS, cabeca 0
    disk.WriteReg(7, 0x20);
    std::vector<uint8_t> chs0;
    for (int i = 0; i < 256; ++i) {
        const uint16_t w = disk.ReadData();
        chs0.push_back(static_cast<uint8_t>(w & 0xFF));
        chs0.push_back(static_cast<uint8_t>(w >> 8));
    }
    check(chs0 == sector0, "CHS (cilindro 0, cabeca 0, setor 1) = LBA 0");
    {
        hd::HdImage ro;
        ro.Open(img, true, error);
        hd::IdeDisk rod(&ro);
        Cmd(rod, 0x30, 10, 1);
        check((rod.ReadReg(7) & hd::IdeDisk::kErr) != 0, "imagem somente leitura: WRITE SECTORS recusado");
    }

    // --- a interface Sunrise ------------------------------------------------------------------------------------------------------------
    std::vector<uint8_t> rom(0x20000);
    for (size_t i = 0; i < rom.size(); ++i) rom[i] = static_cast<uint8_t>((i >> 14) * 16 + (i & 0x0F)); // alto = banco
    hd::HdImage master;
    master.Open(img, false, error);
    hd::SunriseIde ide(rom, &master, nullptr); // so' o mestre
    uint8_t v = 0;
    check(ide.CartRead(0x4000, MEMMAP_MAPPER_NONE, v) && v == 0x70 && ide.bank() == 7, "power-on: o controle vale FFh, banco 7 da ROM em 4000h");
    check(ide.CartWrite(0x4104, 0x40, MEMMAP_MAPPER_NONE) && ide.bank() == 2 && !ide.regs_enabled(), "controle 40h em 4104h: banco 2 (bits do banco invertidos) e registradores IDE desligados");
    check(ide.CartRead(0x7E07, MEMMAP_MAPPER_NONE, v) && v == static_cast<uint8_t>(2 * 16 + 7), "com os registradores desligados, 7E07h le' a ROM do banco 2");
    check(ide.CartWrite(0x4104, 0xF9, MEMMAP_MAPPER_NONE) && ide.bank() == 7 && ide.regs_enabled(), "controle F9h: banco 7 e registradores IDE ligados");
    check(ide.CartRead(0x7E07, MEMMAP_MAPPER_NONE, v) && v == 0x50, "7E07h: estado do ATA (50h)");
    check(!ide.CartRead(0x8000, MEMMAP_MAPPER_NONE, v), "8000h: nao e' do cartucho (cai na memoria)");
    // IDENTIFY pelo mapa de memoria, com o latch de dados
    ide.CartWrite(0x7E06, 0xE0, MEMMAP_MAPPER_NONE);
    ide.CartWrite(0x7E07, 0xEC, MEMMAP_MAPPER_NONE);
    check(ide.CartRead(0x7E07, MEMMAP_MAPPER_NONE, v) && (v & 0x08) != 0, "IDENTIFY pela interface: DRQ");
    uint16_t w0 = 0, w1 = 0;
    ide.CartRead(0x7C00, MEMMAP_MAPPER_NONE, v);
    w0 = v;
    ide.CartRead(0x7C01, MEMMAP_MAPPER_NONE, v);
    w0 |= static_cast<uint16_t>(v << 8);
    ide.CartRead(0x7C00, MEMMAP_MAPPER_NONE, v);
    w1 = v;
    ide.CartRead(0x7C01, MEMMAP_MAPPER_NONE, v);
    w1 |= static_cast<uint16_t>(v << 8);
    check(w0 == 0x0040, "dados: byte par = baixo da palavra, impar = alto (trava): palavra 0 = 0040h");
    check(w1 != 0x7F7F, "a segunda palavra vem do disco");
    // escrita de setor por palavras (par grava o baixo na trava, impar completa)
    ide.CartWrite(0x7E02, 1, MEMMAP_MAPPER_NONE);
    ide.CartWrite(0x7E03, 0x40, MEMMAP_MAPPER_NONE);
    ide.CartWrite(0x7E04, 0x1F, MEMMAP_MAPPER_NONE);
    ide.CartWrite(0x7E05, 0, MEMMAP_MAPPER_NONE);
    ide.CartWrite(0x7E06, 0xE0, MEMMAP_MAPPER_NONE);
    ide.CartWrite(0x7E07, 0x30, MEMMAP_MAPPER_NONE);
    for (int i = 0; i < 256; ++i) {
        ide.CartWrite(0x7C00, static_cast<uint8_t>(i), MEMMAP_MAPPER_NONE);
        ide.CartWrite(0x7C01, 0x5A, MEMMAP_MAPPER_NONE);
    }
    check(ide.disk(0).sectors_written() == 1, "escrita pela interface: 1 setor gravado");
    uint8_t chk[512];
    master.ReadSector(0x1F40, chk);
    check(chk[0] == 0 && chk[1] == 0x5A && chk[2] == 1 && chk[3] == 0x5A && chk[510] == 0xFF && chk[511] == 0x5A, "o setor 1F40h do arquivo tem a palavra (i, 5Ah) em cada posicao");
    // escravo ausente: o barramento flutua em 7Fh
    ide.CartWrite(0x7E06, 0xF0, MEMMAP_MAPPER_NONE);
    check(ide.CartRead(0x7E07, MEMMAP_MAPPER_NONE, v) && v == 0x7F, "disco escravo ausente: o estado le' 7Fh");
    ide.CartWrite(0x7E06, 0xE0, MEMMAP_MAPPER_NONE);
    check(ide.CartRead(0x7E07, MEMMAP_MAPPER_NONE, v) && v == 0x50, "voltando ao mestre: estado 50h");
    // reset por software (SRST em 7E0E)
    ide.CartWrite(0x7E0E, 0x04, MEMMAP_MAPPER_NONE);
    check(ide.CartRead(0x7E07, MEMMAP_MAPPER_NONE, v) && v == 0xFF, "SRST ligado: estado ocupado (FFh)");
    ide.CartWrite(0x7E0E, 0x00, MEMMAP_MAPPER_NONE);
    check(ide.CartRead(0x7E07, MEMMAP_MAPPER_NONE, v) && v == 0x50, "SRST desligado: de volta a 50h");
    ide.Reset();
    check(ide.bank() == 7 && ide.regs_enabled(), "Reset da interface: controle FFh de novo");

    // --- o cartao SD em modo SPI --------------------------------------------------------------------------------------------------------
    {
        hd::HdImage simg;
        simg.Open(img, false, error);  // 48 MB + 1 MiB: SDSC
        hd::SdCard card(&simg);
        check(!card.sdhc(), "imagem pequena = cartao SDSC");
        check(card.Transfer(0x40) == 0xFF && SdCmd(card, 0, 0) == 0xFF, "/CS alto: o cartao nao responde");
        card.Select(true);
        check(SdCmd(card, 0, 0) == 0x01, "CMD0: R1 = 01h (idle)");
        check(SdCmd(card, 8, 0x1AA, 0x87) == 0x01, "CMD8: R1 = 01h e o eco do padrao vem nos 4 bytes seguintes");
        uint8_t echo[4];
        for (auto &e : echo) e = card.Transfer(0xFF);
        check(echo[2] == 0x01 && echo[3] == 0xAA, "CMD8: eco 000001AAh");
        int polls = 0;
        uint8_t r = 0x01;
        while (r == 0x01 && polls < 10) {
            SdCmd(card, 55, 0);
            r = SdCmd(card, 41, 0x40000000);
            ++polls;
        }
        check(r == 0x00 && polls >= 2, "ACMD41 sai do idle depois de algumas consultas (" + std::to_string(polls) + ")");
        check(SdCmd(card, 58, 0) == 0x00, "CMD58: R1 = 00h");
        uint8_t ocr[4];
        for (auto &o : ocr) o = card.Transfer(0xFF);
        check((ocr[0] & 0x80) != 0 && (ocr[0] & 0x40) == 0, "OCR: pronto, SDSC (bit 30 = 0)");
        check(SdCmd(card, 9, 0) == 0x00, "CMD9: R1");
        uint8_t tok = 0xFF;
        for (int i = 0; i < 8 && tok == 0xFF; ++i) tok = card.Transfer(0xFF);
        uint8_t csd[16];
        for (auto &c : csd) c = card.Transfer(0xFF);
        card.Transfer(0xFF);
        card.Transfer(0xFF);
        check(tok == 0xFE && CsdSectors(csd) == simg.sectors(), "CSD v1: token FEh e a capacidade (" + std::to_string(CsdSectors(csd)) + ") bate com a imagem (" + std::to_string(simg.sectors()) + ")");

        // leitura de um bloco
        check(SdCmd(card, 17, 0) == 0x00, "CMD17: R1");
        tok = 0xFF;
        for (int i = 0; i < 8 && tok == 0xFF; ++i) tok = card.Transfer(0xFF);
        std::vector<uint8_t> blk;
        for (int i = 0; i < 512; ++i) blk.push_back(card.Transfer(0xFF));
        card.Transfer(0xFF);
        card.Transfer(0xFF);
        check(tok == 0xFE && std::equal(blk.begin(), blk.end(), raw.begin()), "CMD17 (endereco em bytes, SDSC): o bloco 0 e' o MBR");
        check(SdCmd(card, 17, 512 * 5000) == 0x00, "CMD17 em 5000 * 512");
        tok = 0xFF;
        for (int i = 0; i < 8 && tok == 0xFF; ++i) tok = card.Transfer(0xFF);
        check(tok == 0xFE, "o bloco 5000 vem com o token FEh");
        for (int i = 0; i < 514; ++i) card.Transfer(0xFF);

        // leitura multipla + CMD12
        check(SdCmd(card, 18, 0) == 0x00, "CMD18: R1");
        tok = 0xFF;
        for (int i = 0; i < 8 && tok == 0xFF; ++i) tok = card.Transfer(0xFF);
        for (int i = 0; i < 514; ++i) card.Transfer(0xFF);
        tok = 0xFF;
        for (int i = 0; i < 8 && tok == 0xFF; ++i) tok = card.Transfer(0xFF);
        check(tok == 0xFE, "CMD18: o 2o bloco segue sem novo comando");
        for (int i = 0; i < 514; ++i) card.Transfer(0xFF);
        SdCmd(card, 12, 0);
        for (int i = 0; i < 4; ++i) card.Transfer(0xFF);
        check(card.sectors_read() >= 5, "contador de setores lidos");

        // escrita de um bloco
        check(SdCmd(card, 24, 512 * 7000) == 0x00, "CMD24: R1");
        card.Transfer(0xFF);
        card.Transfer(0xFE);
        for (int i = 0; i < 512; ++i) card.Transfer(static_cast<uint8_t>(i ^ 0x3C));
        card.Transfer(0xFF);
        card.Transfer(0xFF);
        uint8_t dr = 0xFF;
        for (int i = 0; i < 4 && dr == 0xFF; ++i) dr = card.Transfer(0xFF);
        check((dr & 0x1F) == 0x05, "CMD24: resposta de dados 'aceito' (xxx00101b)");
        for (int i = 0; i < 4; ++i) card.Transfer(0xFF);
        uint8_t chk[512];
        simg.ReadSector(7000, chk);
        check(chk[0] == 0x3C && chk[1] == 0x3D && chk[511] == static_cast<uint8_t>(511 ^ 0x3C), "o bloco gravado esta' no arquivo");
        check(card.sectors_written() == 1, "contador de setores gravados");

        // erros
        check(SdCmd(card, 17, 512u * (simg.sectors() + 10)) == 0x40, "CMD17 alem do fim: erro de parametro (40h)");
        check(SdCmd(card, 63, 0) == 0x04 && card.last_unsupported() == 63, "comando desconhecido: R1 = 04h (ilegal) e fica registrado");
        card.Select(false);
        check(card.Transfer(0x00) == 0xFF, "/CS alto de novo: FFh");
    }
    {
        // cartao SDHC (acima de 2 GB): enderecos em setores e CSD v2
        const std::string big = (tmp / "sdhc.hdd").string();
        check(hd::CreateHdImage(big, {2048, 1024}, error), "imagem de 3 GB criada (" + error + ")");
        hd::HdImage himg;
        himg.Open(big, false, error);
        hd::SdCard card(&himg);
        check(card.sdhc(), "imagem de 3 GB = cartao SDHC");
        card.Select(true);
        SdCmd(card, 0, 0);
        SdCmd(card, 8, 0x1AA, 0x87);
        for (int i = 0; i < 4; ++i) card.Transfer(0xFF);
        for (int i = 0; i < 4; ++i) {
            SdCmd(card, 55, 0);
            SdCmd(card, 41, 0x40000000);
        }
        SdCmd(card, 58, 0);
        uint8_t ocr[4];
        for (auto &o : ocr) o = card.Transfer(0xFF);
        check((ocr[0] & 0xC0) == 0xC0, "OCR do SDHC: pronto e CCS (bit 30) ligado");
        SdCmd(card, 9, 0);
        uint8_t tok = 0xFF;
        for (int i = 0; i < 8 && tok == 0xFF; ++i) tok = card.Transfer(0xFF);
        uint8_t csd[16];
        for (auto &c : csd) c = card.Transfer(0xFF);
        check(tok == 0xFE && CsdSectors(csd) == himg.sectors() - himg.sectors() % 1024, "CSD v2: capacidade em unidades de 512 KB (" + std::to_string(CsdSectors(csd)) + ")");
        for (int i = 0; i < 4; ++i) card.Transfer(0xFF);
        check(SdCmd(card, 17, 2048) == 0x00, "CMD17 SDHC: o argumento e' o numero do setor");
        tok = 0xFF;
        for (int i = 0; i < 8 && tok == 0xFF; ++i) tok = card.Transfer(0xFF);
        uint8_t bs0[512];
        for (auto &b : bs0) b = card.Transfer(0xFF);
        check(tok == 0xFE && bs0[510] == 0x55 && bs0[511] == 0xAA, "o setor 2048 e' o setor de boot da 1a particao");
    }

    // --- o MegaSD (mapper ESE-RAM de 1 MB com o cartao SD na janela 0) --------------------------------------------------------------------
    {
        std::vector<uint8_t> mrom(0x8000);
        for (size_t i = 0; i < mrom.size(); ++i) mrom[i] = static_cast<uint8_t>((i >> 13) * 16 + (i & 0x0F));  // alto = banco de 8 KB
        hd::HdImage simg;
        simg.Open(img, false, error);
        hd::SdCard card(&simg);
        hd::MegaSd mega(mrom, &card);
        uint8_t v = 0;
        check(mega.CartRead(0x4000, MEMMAP_MAPPER_NONE, v) && v == 0x00 && mega.CartRead(0x6000, MEMMAP_MAPPER_NONE, v) && v == 0x00, "reset: todas as janelas mostram o banco 0");
        mega.CartWrite(0x6000, 3, MEMMAP_MAPPER_NONE);   // janela 0 (4000h) = banco 3
        mega.CartWrite(0x6800, 2, MEMMAP_MAPPER_NONE);   // janela 1 (6000h) = banco 2
        mega.CartWrite(0x7000, 1, MEMMAP_MAPPER_NONE);   // janela 2 (8000h) = banco 1
        mega.CartWrite(0x7800, 0x80 | 3, MEMMAP_MAPPER_NONE);   // janela 3 (A000h) = banco 3, gravavel
        check(mega.CartRead(0x4005, MEMMAP_MAPPER_NONE, v) && v == 0x35 && mega.CartRead(0x6005, MEMMAP_MAPPER_NONE, v) && v == 0x25 &&
                  mega.CartRead(0x8005, MEMMAP_MAPPER_NONE, v) && v == 0x15,
              "registradores 6000h/6800h/7000h: as janelas de 8 KB mostram os bancos pedidos");
        mega.CartWrite(0xA000, 0x77, MEMMAP_MAPPER_NONE);
        mega.CartWrite(0x8000, 0x11, MEMMAP_MAPPER_NONE);   // janela 2 sem o bit 7: ROM, ignora
        check(mega.CartRead(0xA000, MEMMAP_MAPPER_NONE, v) && v == 0x77 && mega.CartRead(0x8000, MEMMAP_MAPPER_NONE, v) && v == 0x10, "bit 7 do registrador = RAM gravavel; sem ele a janela e' ROM");
        // modo SD: registrador 0 = 40h
        mega.CartWrite(0x6000, 0x40, MEMMAP_MAPPER_NONE);
        check(mega.sd_mode(), "registrador 0 = 40h: a janela 0 vira o cartao SD");
        check(mega.CartRead(0x5800, MEMMAP_MAPPER_NONE, v) && v == 0x01, "5800h (leitura): estado, bit 0 = acabou de ligar");
        mega.CartWrite(0x5800, 0x00, MEMMAP_MAPPER_NONE);   // modo: soquete 0 ligado
        check(mega.CartRead(0x5800, MEMMAP_MAPPER_NONE, v) && v == 0x00, "depois de escrever o modo o bit 'acabou de ligar' cai");
        // /CS baixo (4000h), CMD0, resposta atrasada de um byte (pipeline da leitura)
        const uint8_t cmd0[6] = {0x40, 0, 0, 0, 0, 0x95};
        mega.CartRead(0x4000, MEMMAP_MAPPER_NONE, v);
        for (uint8_t b : cmd0) mega.CartWrite(0x4000, b, MEMMAP_MAPPER_NONE);
        uint8_t r1 = 0xFF;
        for (int i = 0; i < 16 && r1 == 0xFF; ++i) mega.CartRead(0x4000, MEMMAP_MAPPER_NONE, r1);
        check(r1 == 0x01, "CMD0 pelo registrador de dados: R1 = 01h");
        mega.CartRead(0x5000, MEMMAP_MAPPER_NONE, v);   // solta o /CS
        // modo desligado: a janela volta a ser memoria
        mega.CartWrite(0x6000, 0x00, MEMMAP_MAPPER_NONE);
        check(!mega.sd_mode() && mega.CartRead(0x4005, MEMMAP_MAPPER_NONE, v) && v == 0x05, "registrador 0 = 00h: a janela 0 volta a ser a ROM");
        mega.Reset();
        check(mega.bank(0) == 0 && mega.bank(3) == 0, "Reset zera os bancos");
    }

    // CLI
    const std::string cli_img = (tmp / "cli.hdd").string();
    check(hd::RunHdNewCommand({cli_img, "8M", "16M"}) == 0 && fs::file_size(cli_img) == (1ull + 24) * 1024 * 1024, "fwmsx --hdnew <arq> 8M 16M cria a imagem");
    check(hd::RunHdNewCommand({"--split", "64M", "2", (tmp / "split.hdd").string()}) == 0 && fs::file_size(tmp / "split.hdd") == (1ull + 64) * 1024 * 1024, "--split 64M 2 divide em 2 de 32 MB");
    check(hd::RunHdNewCommand({"--split", "100M", "3", (tmp / "s3.hdd").string()}) == 2 && hd::RunHdNewCommand({(tmp / "z.hdd").string(), "4M"}) == 1, "--split que nao divide e tamanho fora da faixa: erro");
    check(hd::RunHdNewCommand({"--info", cli_img}) == 0 && hd::RunHdNewCommand({}) == 2, "--info e uso");

    fs::remove_all(tmp, ec);
    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
