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
#include "../../src/hd/cpp/sunrise_ide.h"

namespace fs = std::filesystem;

static int g_failures = 0;

static void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

static uint32_t Le32(const uint8_t *p) { return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24); }
static uint32_t Le16(const uint8_t *p) { return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8); }

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
