// fwMSX -- fwmsx --hdnew. Ver cli.h e doc/hd-spec.md.
#include "cli.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "hd_image.h"

namespace hd {

namespace {

void Usage() {
    std::cout << "uso: fwmsx --hdnew <arquivo.hdd> <tamanho> [tamanho...]\n"
                 "     fwmsx --hdnew --split <total> <n> <arquivo.hdd>\n"
                 "     fwmsx --hdnew --info <arquivo.hdd>\n"
                 "  Cria uma imagem de disco rigido (ou de cartao SD) com a tabela de particoes que o Nextor le e uma particao FAT16 vazia\n"
                 "  para cada tamanho. Tamanhos: 512 ou 512M (megabytes), 2G (gigabytes). Cada particao de 8M a 2G (o limite do Nextor);\n"
                 "  ate' 4 particoes e 4G ao todo. Exemplos:\n"
                 "    fwmsx --hdnew disco.hdd 512M              um disco de 512 MB com 1 particao\n"
                 "    fwmsx --hdnew sd4g.hdd 2G 2G              4 GB em 2 particoes de 2 GB\n"
                 "    fwmsx --hdnew --split 4G 2 sd4g.hdd       o mesmo\n"
                 "  Use com: fwmsx --msx --msx2 --ide disco.hdd   (a ROM do Nextor vem do banco de ROMs, ou --nextor <rom>)\n"
              << std::endl;
}

} // namespace

bool DescribeImage(const std::string &path, std::string &text, std::string &error) {
    HdImage img;
    if (!img.Open(path, true, error)) return false;
    text = path + ": " + std::to_string(static_cast<uint64_t>(img.sectors()) / 2048) + " MB (" + std::to_string(img.sectors()) + " setores)";
    std::vector<PartitionInfo> parts;
    std::string perr;
    if (!ReadPartitions(img, parts, perr)) {
        text += "\n  " + perr;
        return true;
    }
    int n = 0;
    for (const PartitionInfo &p : parts) {
        char line[160];
        std::snprintf(line, sizeof(line), "\n  particao %d: inicio %u, %u MB, tipo %02Xh", ++n, p.start, p.sectors / 2048, p.type);
        text += line;
    }
    return true;
}

int RunHdNewCommand(const std::vector<std::string> &args) {
    if (args.empty() || args[0] == "-h" || args[0] == "--help") {
        Usage();
        return args.empty() ? 2 : 0;
    }
    std::string error;
    if (args[0] == "--info") {
        if (args.size() != 2) {
            Usage();
            return 2;
        }
        std::string text;
        if (!DescribeImage(args[1], text, error)) {
            std::cerr << "fwmsx --hdnew: " << error << std::endl;
            return 1;
        }
        std::cout << text << std::endl;
        return 0;
    }
    std::string path;
    std::vector<uint32_t> sizes;
    if (args[0] == "--split") {
        if (args.size() != 4) {
            Usage();
            return 2;
        }
        uint32_t total = 0;
        if (!ParseSize(args[1], total, error)) {
            std::cerr << "fwmsx --hdnew: " << error << std::endl;
            return 2;
        }
        const int n = std::atoi(args[2].c_str());
        if (n < 1 || n > HD_MAX_PARTITIONS || total % static_cast<uint32_t>(n) != 0) {
            std::cerr << "fwmsx --hdnew: --split: o total tem de dividir em 1 a 4 partes iguais" << std::endl;
            return 2;
        }
        sizes.assign(static_cast<size_t>(n), total / static_cast<uint32_t>(n));
        path = args[3];
    } else {
        path = args[0];
        for (size_t i = 1; i < args.size(); ++i) {
            uint32_t mb = 0;
            if (!ParseSize(args[i], mb, error)) {
                std::cerr << "fwmsx --hdnew: " << error << std::endl;
                return 2;
            }
            sizes.push_back(mb);
        }
        if (sizes.empty()) {
            Usage();
            return 2;
        }
    }
    if (!CreateHdImage(path, sizes, error)) {
        std::cerr << "fwmsx --hdnew: " << error << std::endl;
        return 1;
    }
    std::string text;
    if (DescribeImage(path, text, error)) std::cout << text << std::endl;
    return 0;
}

} // namespace hd
