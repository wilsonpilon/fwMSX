// Teste do msxdisk aprendendo FAT16 + particoes de HD/SD (1.43.27) -- ver doc/msxdisk-spec.md
// (Fase 6) e doc/hd-spec.md. O nucleo do msxdisk (src/msxdisk/core) continua sem depender do
// modulo hd; aqui SIM usamos hd::CreateHdImage/hd::ReadPartitions, mas so' para construir e
// inspecionar a imagem de fixture -- o alvo de teste e' isolado dos outros que linkam
// MSXDISK_CORE_SOURCES (controltest, mamutetest...), que continuam sem essa dependencia.
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/hd/cpp/hd_image.h"
#include "../../src/msxdisk/core/disk_image.h"
#include "../../src/msxdisk/fat16_c/fat16.h"

namespace fs = std::filesystem;
using msxdisk::DiskError;
using msxdisk::DiskImage;

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

std::vector<uint8_t> ReadWholeFile(const fs::path &path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    const auto size = static_cast<size_t>(in.tellg());
    in.seekg(0);
    std::vector<uint8_t> data(size);
    in.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(size));
    return data;
}

void TestFat16Entries() {
    std::vector<uint8_t> fat(200000, 0xCC);

    msxdisk_fat16_set(fat.data(), 0, 0x0FF8);
    msxdisk_fat16_set(fat.data(), 1, 0xFFFF);
    msxdisk_fat16_set(fat.data(), 5000, 5001);
    msxdisk_fat16_set(fat.data(), 5001, 0xFFFF);
    msxdisk_fat16_set(fat.data(), 65523, 2);

    check(msxdisk_fat16_get(fat.data(), 0) == 0x0FF8, "FAT16: entrada 0 (media descriptor) bate");
    check(msxdisk_fat16_get(fat.data(), 1) == 0xFFFF, "FAT16: entrada 1 bate");
    check(msxdisk_fat16_get(fat.data(), 5000) == 5001, "FAT16: cadeia em cluster alto (5000 -> 5001)");
    check(msxdisk_fat16_get(fat.data(), 5001) == 0xFFFF, "FAT16: fim de cadeia em cluster alto");
    check(msxdisk_fat16_get(fat.data(), 65523) == 2, "FAT16: maior cluster possivel (65523) bate");
    // Entradas vizinhas nao tocadas continuam com o valor de "sujeira" original (sem
    // empacotamento de bits cruzando para a entrada ao lado, diferente da FAT12).
    check(msxdisk_fat16_get(fat.data(), 5002) == 0xCCCC, "FAT16: entrada vizinha nao e' afetada (sem empacotamento de bits)");
}

void TestGeometryDetectsFat16(const fs::path &hdd, const std::vector<hd::PartitionInfo> &parts) {
    std::ifstream in(hdd, std::ios::binary);
    in.seekg(static_cast<std::streamoff>(parts[0].start) * 512);
    uint8_t boot[512];
    in.read(reinterpret_cast<char *>(boot), sizeof(boot));

    const auto geometry = msxdisk::Geometry::ParseFromBootSector(boot);
    check(geometry.fat_bits == 16, "Geometry::ParseFromBootSector detecta FAT16 pelo rotulo \"FAT16\" do BPB");
    check(geometry.total_sectors == parts[0].sectors,
          "Geometry::ParseFromBootSector le o tamanho certo (campo estendido de 32 bits do BPB)");
    check(geometry.bytes_per_sector == 512 && geometry.fat_count == 2, "Geometry::ParseFromBootSector: BPB basico ok");
}

void TestPartitionRoundtripAndIsolation(const fs::path &hdd, const std::vector<hd::PartitionInfo> &parts) {
    const std::vector<uint8_t> before = ReadWholeFile(hdd);

    const std::string target = hdd.string() + ":1"; // particao 1 (1-based)
    DiskError error = DiskError::None;
    auto image = DiskImage::Load(target, &error);
    check(image.has_value(), "DiskImage::Load(\"img.hdd:1\") abre a particao 1 (" +
                                  std::string(image ? "ok" : "falhou") + ")");
    if (!image) return;

    check(image->geometry().fat_bits == 16, "particao 1 carregada como FAT16");

    // Conteudo sintetico (NAO e' MSXDOS2.SYS/COMMAND2.COM reais -- o msxdisk nunca redistribui
    // arquivos de sistema do MSX-DOS, mesma regra de create --dos1/--dos2, doc/msxdisk-spec.md).
    const fs::path host_file = hdd.parent_path() / "fixture_cmd2.bin";
    {
        std::ofstream out(host_file, std::ios::binary);
        std::vector<char> content(3000, 'Z'); // mais de um cluster, exercita cadeia FAT16
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
    }

    const bool added = image->AddFile(host_file, "COMMAND2.COM", msxdisk::kAttrArchive, &error);
    check(added, "AddFile grava um arquivo multi-cluster na particao FAT16");

    const bool saved = image->Save(target, &error);
    check(saved, "Save grava so' a particao 1 de volta no .hdd");

    // Recarrega do zero (novo DiskImage::Load) e confere que o que foi gravado persistiu.
    auto reloaded = DiskImage::Load(target, &error);
    check(reloaded.has_value(), "recarrega a particao 1 depois do Save");
    if (reloaded) {
        const auto entries = reloaded->ListDirectory("", "", &error);
        bool found = false;
        for (const auto &e : entries) found |= (e.name == "COMMAND2.COM" && e.size == 3000);
        check(found, "COMMAND2.COM aparece na listagem da particao 1 com o tamanho certo");

        const fs::path extracted = hdd.parent_path() / "extracted_cmd2.bin";
        check(reloaded->ExtractFile("COMMAND2.COM", extracted, &error) == true, "ExtractFile le o arquivo de volta");
        const auto extracted_bytes = ReadWholeFile(extracted);
        check(extracted_bytes.size() == 3000 && extracted_bytes[0] == 'Z' && extracted_bytes[2999] == 'Z',
              "conteudo extraido bate byte a byte");
    }

    // Prova de isolamento: o MBR e a particao 2 continuam EXATAMENTE como antes do Save --
    // a escrita parcial nao vazou para fora da faixa de bytes da particao 1.
    const std::vector<uint8_t> after = ReadWholeFile(hdd);
    bool mbr_untouched = std::memcmp(before.data(), after.data(), 512) == 0;
    check(mbr_untouched, "Save na particao 1 NAO alterou o MBR (setor 0)");

    const size_t p2_offset = static_cast<size_t>(parts[1].start) * 512;
    const size_t p2_bytes = static_cast<size_t>(parts[1].sectors) * 512;
    bool p2_untouched = std::memcmp(before.data() + p2_offset, after.data() + p2_offset, p2_bytes) == 0;
    check(p2_untouched, "Save na particao 1 NAO alterou a particao 2 vizinha");
}

void TestWholeFileFat12Unaffected() {
    // Regressao: sem sufixo ":N", comportamento idêntico ao de sempre (disquete FAT12
    // inteiro) -- inclusive com um caminho absoluto do Windows (letra de unidade "C:\...",
    // que tem ':' na posicao 1 e NAO pode ser confundida com um sufixo de particao).
    const fs::path tmp = fs::temp_directory_path() / "fwmsx_msxdisktest";
    std::error_code ec;
    fs::create_directories(tmp, ec);
    const fs::path dsk = tmp / "floppy.dsk";

    DiskImage image = DiskImage::CreateBlank();
    check(image.geometry().fat_bits == 12, "DiskImage::CreateBlank() continua FAT12 por padrao");

    DiskError error = DiskError::None;
    check(image.Save(dsk, &error), "Save sem sufixo grava o disquete inteiro, como sempre");

    auto reloaded = DiskImage::Load(dsk, &error);
    check(reloaded.has_value() && reloaded->geometry().fat_bits == 12,
          "Load sem sufixo le o disquete inteiro como FAT12, como sempre (caminho absoluto com letra de unidade)");
}

} // namespace

int main() {
    const fs::path tmp = fs::temp_directory_path() / "fwmsx_msxdisktest";
    std::error_code ec;
    fs::create_directories(tmp, ec);
    const fs::path hdd = tmp / "fixture.hdd";

    std::string hd_error;
    const bool created = hd::CreateHdImage(hdd.string(), {8, 8}, hd_error); // 2 particoes de 8 MB
    check(created, "fixture: hd::CreateHdImage(8 MB + 8 MB) (" + hd_error + ")");
    if (!created) {
        std::printf("\nFALHOU (nao foi possivel construir a fixture)\n");
        return 1;
    }

    hd::HdImage opened;
    std::vector<hd::PartitionInfo> parts;
    check(opened.Open(hdd.string(), /*read_only=*/true, hd_error), "fixture: abre a imagem para ler a tabela de particoes");
    check(hd::ReadPartitions(opened, parts, hd_error) && parts.size() == 2, "fixture: 2 particoes no MBR");
    if (parts.size() != 2) {
        std::printf("\nFALHOU (fixture sem as 2 particoes esperadas)\n");
        return 1;
    }

    TestFat16Entries();
    TestGeometryDetectsFat16(hdd, parts);
    TestPartitionRoundtripAndIsolation(hdd, parts);
    TestWholeFileFat12Unaffected();

    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
