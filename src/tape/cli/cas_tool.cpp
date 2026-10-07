#include "cas_tool.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "../cpp/cas_format.h"
#include "../cpp/cas_pack.h"
#include "../cpp/cas_reader.h"
#include "../cpp/tsx_writer.h"
#include "../cpp/tzx_reader.h"
#include "../cpp/wav_reader.h"
#include "../cpp/wav_ripper.h"

namespace tape {
namespace {

bool EndsWithCi(const std::string &s, const char *suffix) {
    const std::size_t n = std::strlen(suffix);
    if (s.size() < n) return false;
    return std::equal(s.end() - static_cast<std::ptrdiff_t>(n), s.end(), suffix,
                       [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == b; });
}

bool ReadFile(const std::string &path, std::vector<uint8_t> &out, std::string &error) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }
    const std::streamsize size = f.tellg();
    f.seekg(0);
    out.resize(static_cast<std::size_t>(size));
    if (size > 0 && !f.read(reinterpret_cast<char *>(out.data()), size)) {
        error = "nao foi possivel ler '" + path + "'";
        return false;
    }
    return true;
}

bool WriteRaw(const std::string &path, const std::vector<uint8_t> &bytes, std::string &error) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f || !f.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) {
        error = "nao foi possivel gravar '" + path + "'";
        return false;
    }
    return true;
}

bool LoadAny(const std::string &path, TapeImage &out, std::string &error) {
    const bool is_tsx = EndsWithCi(path, ".tsx") || EndsWithCi(path, ".tzx");
    return is_tsx ? LoadTzxImage(path, out, error) : LoadCasImage(path, out, error);
}

bool SaveAny(const std::string &path, const std::vector<uint8_t> &fast_bytes, const std::vector<TapeMark> &marks,
             std::string &error) {
    const bool is_tsx = EndsWithCi(path, ".tsx") || EndsWithCi(path, ".tzx");
    return is_tsx ? WriteTsxFromCas(fast_bytes, marks, path, error) : WriteRaw(path, fast_bytes, error);
}

// Valor de uma opcao "--nome valor" (a primeira ocorrencia); vazia se faltar.
std::string Option(const std::vector<std::string> &args, const std::string &flag) {
    for (std::size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == flag) return args[i + 1];
    }
    return "";
}

// Argumentos sem "--opcao valor" (pulando o valor de cada opcao conhecida),
// a partir de `from` (1 para pular o nome do subcomando).
std::vector<std::string> Positionals(const std::vector<std::string> &args, std::size_t from) {
    static const std::vector<std::string> kFlagsWithValue = {"--tipo", "--nome", "--inicio", "--fim", "--exec",
                                                               "--anexar", "--tolerancia"};
    std::vector<std::string> out;
    for (std::size_t i = from; i < args.size(); ++i) {
        if (!args[i].empty() && args[i].rfind("--", 0) == 0) {
            if (std::find(kFlagsWithValue.begin(), kFlagsWithValue.end(), args[i]) != kFlagsWithValue.end()) ++i;
            continue;
        }
        out.push_back(args[i]);
    }
    return out;
}

bool ParseHex16(const std::string &s, uint16_t &out) {
    if (s.empty()) return false;
    try {
        std::size_t pos = 0;
        const unsigned long v = std::stoul(s, &pos, 0); // base 0: aceita "0x..", "0" e decimal
        if (pos != s.size() || v > 0xFFFF) return false;
        out = static_cast<uint16_t>(v);
        return true;
    } catch (...) {
        return false;
    }
}

// Maiusculas (convencao do MSX) e truncado para os 6 bytes do .CAS, com aviso.
std::string NormalizeName(std::string name) {
    for (char &c : name) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if (name.size() > kCasFileNameBytes) {
        std::cerr << "fwmsx --cas: aviso: nome '" << name << "' tem mais de " << kCasFileNameBytes
                  << " caracteres, truncado para '" << name.substr(0, kCasFileNameBytes) << "'" << std::endl;
        name = name.substr(0, kCasFileNameBytes);
    }
    return name;
}

int Fail(const std::string &message) {
    std::cerr << "fwmsx --cas: " << message << std::endl;
    return 1;
}

void PrintHelp() {
    std::cout << "fwmsx --cas <comando> [opcoes]\n"
                 "\n"
                 "pack --tipo bin|bas --nome NOME [opcoes] <entrada> <saida.tsx|.cas>\n"
                 "  Empacota um arquivo solto (ja no formato BINARIO/tokenizado do MSX) num\n"
                 "  .TSX (ou .CAS) valido, pronto pra BLOAD/CLOAD \"CAS:\" -- sem passar pelo\n"
                 "  emulador. O nome vai pro cabecalho de 6 caracteres (maiusculas, truncado\n"
                 "  se for maior).\n"
                 "\n"
                 "  --tipo bin                 BLOAD \"CAS:\" -- precisa de --inicio/--fim/--exec\n"
                 "  --tipo bas                 CLOAD \"CAS:\" -- a entrada tem que ja estar\n"
                 "                             tokenizada (os bytes exatos que o CSAVE gravaria,\n"
                 "                             ex.: extraidos com BSAVE dentro do emulador)\n"
                 "  --inicio/--fim/--exec N    enderecos do bloco binario em hexadecimal\n"
                 "                             (0xNNNN), so' para --tipo bin\n"
                 "  --anexar <arquivo>         carrega uma fita existente e acrescenta o novo\n"
                 "                             arquivo no final, em vez de criar uma fita so'\n"
                 "                             com ele (a saida pode ser o MESMO arquivo)\n"
                 "\n"
                 "rip [--tolerancia N] [--anexar <arquivo>] <entrada.wav> <saida.tsx|.cas>\n"
                 "  \"Ripa\" uma gravacao real de fita (.wav PCM mono, 8 ou 16 bits) para um\n"
                 "  .TSX valido -- detecta o piloto e decodifica os bytes do bloco #4B (Kansas\n"
                 "  City Standard), igual o makeTSX (resource/makeTSX/, MIT). So' funciona com\n"
                 "  o formato KCS fixo do MSX (ver doc/tape-spec.md, secao 9 para os limites).\n"
                 "\n"
                 "  --tolerancia N (1-90, padrao 25)   tolerancia (%) no casamento dos pulsos --\n"
                 "                                     suba para gravacoes mais ruidosas\n"
                 "  --anexar <arquivo>                 acrescenta no final de uma fita existente\n"
                 "\n"
                 "list <arquivo.tsx|.tzx|.cas>\n"
                 "  Lista os arquivos de uma fita (indice, tipo, nome, tamanho dos dados) --\n"
                 "  util pra confirmar o resultado do pack/rip sem abrir o emulador.\n"
              << std::endl;
}

int RunPack(const std::vector<std::string> &args) {
    const std::string tipo = Option(args, "--tipo");
    const std::string nome_raw = Option(args, "--nome");
    const std::string anexar = Option(args, "--anexar");
    const std::vector<std::string> pos = Positionals(args, 1);

    if (tipo != "bin" && tipo != "bas") return Fail("--tipo precisa ser 'bin' ou 'bas' (ver 'fwmsx --cas help')");
    if (nome_raw.empty()) return Fail("--nome e' obrigatorio");
    if (pos.size() != 2) return Fail("uso: pack --tipo <bin|bas> --nome <NOME> [opcoes] <entrada> <saida>");

    const std::string &input_path = pos[0];
    const std::string &output_path = pos[1];
    const std::string name = NormalizeName(nome_raw);

    std::vector<uint8_t> payload;
    std::string error;
    if (!ReadFile(input_path, payload, error)) return Fail(error);

    uint8_t type_id = kCasIdBasic;
    std::vector<uint8_t> data;
    if (tipo == "bin") {
        type_id = kCasIdBinary;
        uint16_t inicio = 0, fim = 0, exec = 0;
        if (!ParseHex16(Option(args, "--inicio"), inicio) || !ParseHex16(Option(args, "--fim"), fim) ||
            !ParseHex16(Option(args, "--exec"), exec)) {
            return Fail("--tipo bin precisa de --inicio/--fim/--exec (enderecos em hexadecimal, ex.: 0x8000)");
        }
        data = {static_cast<uint8_t>(inicio & 0xFF), static_cast<uint8_t>(inicio >> 8),
                static_cast<uint8_t>(fim & 0xFF), static_cast<uint8_t>(fim >> 8),
                static_cast<uint8_t>(exec & 0xFF), static_cast<uint8_t>(exec >> 8)};
        data.insert(data.end(), payload.begin(), payload.end());
    } else {
        data = std::move(payload);
    }

    std::vector<uint8_t> fast_bytes;
    std::vector<TapeMark> marks;
    if (!anexar.empty()) {
        TapeImage existing;
        if (!LoadAny(anexar, existing, error)) return Fail("--anexar: " + error);
        fast_bytes = std::move(existing.fast_bytes);
        marks = std::move(existing.marks);
    }

    AppendCasFile(fast_bytes, marks, type_id, name, data);

    if (!SaveAny(output_path, fast_bytes, marks, error)) return Fail(error);

    std::cout << "gravado '" << output_path << "': " << name << " (" << (tipo == "bin" ? "BLOAD" : "CLOAD") << ", "
              << data.size() << " bytes de dados)" << std::endl;
    return 0;
}

int RunRip(const std::vector<std::string> &args) {
    const std::string tolerancia_raw = Option(args, "--tolerancia");
    const std::string anexar = Option(args, "--anexar");
    const std::vector<std::string> pos = Positionals(args, 1);
    if (pos.size() != 2) return Fail("uso: rip [--tolerancia N] [--anexar <arquivo>] <entrada.wav> <saida.tsx|.cas>");

    int tolerance_percent = 25;
    if (!tolerancia_raw.empty()) {
        try {
            std::size_t consumed = 0;
            tolerance_percent = std::stoi(tolerancia_raw, &consumed);
            if (consumed != tolerancia_raw.size()) throw std::invalid_argument(tolerancia_raw);
        } catch (...) {
            return Fail("--tolerancia precisa ser um numero inteiro (percentual, ex.: 25)");
        }
        if (tolerance_percent < 1 || tolerance_percent > 90) return Fail("--tolerancia precisa estar entre 1 e 90");
    }

    const std::string &input_path = pos[0];
    const std::string &output_path = pos[1];

    WavSamples wav;
    std::string error;
    if (!LoadWav(input_path, wav, error)) return Fail(error);

    std::vector<uint8_t> fast_bytes;
    std::vector<TapeMark> marks;
    if (!anexar.empty()) {
        TapeImage existing;
        if (!LoadAny(anexar, existing, error)) return Fail("--anexar: " + error);
        fast_bytes = std::move(existing.fast_bytes);
        marks = std::move(existing.marks);
    }

    RipStats stats;
    if (!RipWavToCas(wav, fast_bytes, marks, tolerance_percent, stats, error)) return Fail(error);
    for (const std::string &warning : stats.warnings) {
        std::cerr << "fwmsx --cas: aviso: " << warning << std::endl;
    }

    if (!SaveAny(output_path, fast_bytes, marks, error)) return Fail(error);

    std::cout << "gravado '" << output_path << "': " << stats.blocks_found << " bloco(s), " << stats.bytes_decoded
              << " bytes decodificados" << std::endl;
    return 0;
}

int RunList(const std::vector<std::string> &args) {
    const std::vector<std::string> pos = Positionals(args, 1);
    if (pos.size() != 1) return Fail("uso: list <arquivo.tsx|.tzx|.cas>");

    TapeImage img;
    std::string error;
    if (!LoadAny(pos[0], img, error)) return Fail(error);

    if (img.files.empty()) {
        std::cout << "(nenhum arquivo encontrado em '" << pos[0] << "')" << std::endl;
        return 0;
    }
    for (std::size_t i = 0; i < img.files.size(); ++i) {
        const TapeFileEntry &f = img.files[i];
        const char *type = f.type == TapeFileType::Binary ? "BIN" : f.type == TapeFileType::Basic ? "BAS" : "ASC";
        std::cout << i << "\t" << type << "\t" << f.name << "\t" << f.data_bytes << " bytes" << std::endl;
    }
    return 0;
}

} // namespace

int RunCasToolCommand(const std::vector<std::string> &args, const std::string & /*argv0*/) {
    if (args.empty() || args[0] == "help" || args[0] == "--help") {
        PrintHelp();
        return 0;
    }
    if (args[0] == "pack") return RunPack(args);
    if (args[0] == "rip") return RunRip(args);
    if (args[0] == "list") return RunList(args);
    return Fail("comando desconhecido '" + args[0] + "' (ver 'fwmsx --cas help')");
}

} // namespace tape
