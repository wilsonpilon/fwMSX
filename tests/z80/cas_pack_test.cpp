// Teste do empacotador "fwmsx --cas" (src/tape/cpp/cas_pack.*,
// src/tape/cli/cas_tool.*): empacota um .BIN/.BAS solto num .TSX/.CAS
// valido sem passar pelo emulador, e lista o conteudo de uma fita. Sem
// Z80/mapa de memoria -- so' leitura/escrita de arquivo e os blocos
// .CAS. Ver doc/tape-spec.md, secao 8.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/tape/cli/cas_tool.h"
#include "../../src/tape/cpp/cas_format.h"
#include "../../src/tape/cpp/cas_pack.h"
#include "../../src/tape/cpp/tzx_reader.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    if (cond) {
        std::printf("[PASS] %s\n", what.c_str());
    } else {
        std::printf("[FAIL] %s\n", what.c_str());
        ++g_failures;
    }
}

std::string TempPath(const char *name) {
    const char *t = std::getenv("TEMP");
    return (t ? std::string(t) : std::string("/tmp")) + "/" + name;
}

void WriteFile(const std::string &path, const std::vector<uint8_t> &bytes) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

} // namespace

int main() {
    // --- AppendCasBlock/AppendCasFile (nivel baixo, sem CLI) ----------------
    {
        std::vector<uint8_t> bytes;
        std::vector<tape::TapeMark> marks;
        // "ABCDEFGH" tem mais de 6 letras -- AppendCasFile() trunca por conta
        // propria (so' o tool CLI faz maiusculas, nao esta funcao de baixo nivel).
        tape::AppendCasFile(bytes, marks, tape::kCasIdBinary, "ABCDEFGH", {0x11, 0x22});
        check(marks.size() == 2, "AppendCasFile: 2 marcas (cabecalho + dados)");
        check(bytes.size() >= tape::kCasHeader.size() * 2 + tape::kCasFileIdBytes + tape::kCasFileNameBytes + 2,
              "AppendCasFile: cabecalho(8+16) + dados(8+2) presentes");
        // A 2a chamada precisa alinhar a 8 bytes antes do proximo cabecalho.
        const std::size_t before = bytes.size();
        tape::AppendCasFile(bytes, marks, tape::kCasIdBasic, "X", {0xAA});
        check(marks.size() == 4, "AppendCasFile (2a vez): mais 2 marcas");
        check(marks[2].fast_byte_offset >= before && marks[2].fast_byte_offset % tape::kCasHeader.size() == 0,
              "AppendCasFile (2a vez): o 3o cabecalho fica alinhado a 8 bytes");
    }

    // --- pack --tipo bin -----------------------------------------------------
    const std::string bin_input = TempPath("fwmsx_cas_pack_test.bin");
    const std::vector<uint8_t> bin_payload = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03};
    WriteFile(bin_input, bin_payload);
    const std::string bin_output = TempPath("fwmsx_cas_pack_test_bin.tsx");
    {
        const int rc = tape::RunCasToolCommand(
            {"pack", "--tipo", "bin", "--nome", "jogo", "--inicio", "0x8000", "--fim", "0x8006", "--exec", "0x8000",
             bin_input, bin_output},
            "fwmsx");
        check(rc == 0, "pack --tipo bin: sucesso (codigo 0)");

        tape::TapeImage img;
        std::string error;
        check(tape::LoadTzxImage(bin_output, img, error), "pack --tipo bin: o .tsx gerado abre (" + error + ")");
        check(img.files.size() == 1, "pack --tipo bin: 1 arquivo no resultado");
        if (img.files.size() == 1) {
            const tape::TapeFileEntry &f = img.files[0];
            check(f.type == tape::TapeFileType::Binary, "pack --tipo bin: tipo Binary");
            check(f.name == "JOGO", "pack --tipo bin: nome em maiusculas ('jogo' -> 'JOGO')");
            check(f.data_bytes == 6 + bin_payload.size(), "pack --tipo bin: dados = 6 bytes de endereco + payload");
            const std::size_t data_start = f.fast_byte_offset + tape::kCasHeader.size() + tape::kCasFileIdBytes +
                                            tape::kCasFileNameBytes + tape::kCasHeader.size();
            const std::vector<uint8_t> expect_addrs = {0x00, 0x80, 0x06, 0x80, 0x00, 0x80}; // inicio/fim/exec, little-endian
            check(data_start + 6 <= img.fast_bytes.size() &&
                      std::equal(expect_addrs.begin(), expect_addrs.end(), img.fast_bytes.begin() + static_cast<std::ptrdiff_t>(data_start)),
                  "pack --tipo bin: os 6 bytes de endereco (inicio/fim/exec) vem certos e em little-endian");
            check(std::equal(bin_payload.begin(), bin_payload.end(),
                              img.fast_bytes.begin() + static_cast<std::ptrdiff_t>(data_start + 6)),
                  "pack --tipo bin: o payload vem depois dos 6 bytes de endereco, intacto");
        }
    }

    // --- pack --tipo bas (bytes ja tokenizados, copiados sem alteracao) -----
    const std::string bas_input = TempPath("fwmsx_cas_pack_test.bas");
    const std::vector<uint8_t> bas_payload = {0x01, 0x02, 'A', 'B', 'C', 0x00, 0x00, 0x00};
    WriteFile(bas_input, bas_payload);
    const std::string bas_output = TempPath("fwmsx_cas_pack_test_bas.tsx");
    {
        const int rc =
            tape::RunCasToolCommand({"pack", "--tipo", "bas", "--nome", "TESTE", bas_input, bas_output}, "fwmsx");
        check(rc == 0, "pack --tipo bas: sucesso (codigo 0)");

        tape::TapeImage img;
        std::string error;
        check(tape::LoadTzxImage(bas_output, img, error), "pack --tipo bas: o .tsx gerado abre");
        check(img.files.size() == 1 && img.files[0].type == tape::TapeFileType::Basic && img.files[0].name == "TESTE",
              "pack --tipo bas: 1 arquivo, tipo Basic, nome 'TESTE'");
        if (img.files.size() == 1) {
            check(img.files[0].data_bytes == bas_payload.size(), "pack --tipo bas: tamanho dos dados == payload (sem os 6 bytes de endereco)");
            const std::size_t data_start = img.files[0].fast_byte_offset + tape::kCasHeader.size() + tape::kCasFileIdBytes +
                                            tape::kCasFileNameBytes + tape::kCasHeader.size();
            check(data_start + bas_payload.size() <= img.fast_bytes.size() &&
                      std::equal(bas_payload.begin(), bas_payload.end(), img.fast_bytes.begin() + static_cast<std::ptrdiff_t>(data_start)),
                  "pack --tipo bas: os bytes tokenizados vao direto, sem endereco na frente");
        }
    }

    // --- --anexar: acrescenta no final de uma fita existente ----------------
    {
        const std::string anexo_output = TempPath("fwmsx_cas_pack_test_anexo.tsx");
        const int rc1 = tape::RunCasToolCommand({"pack", "--tipo", "bas", "--nome", "UM", bas_input, anexo_output}, "fwmsx");
        check(rc1 == 0, "--anexar: 1a gravacao (fita com 'UM') com sucesso");
        const int rc2 = tape::RunCasToolCommand(
            {"pack", "--tipo", "bin", "--nome", "DOIS", "--inicio", "0x9000", "--fim", "0x9006", "--exec", "0x9000",
             "--anexar", anexo_output, bin_input, anexo_output},
            "fwmsx");
        check(rc2 == 0, "--anexar: 2a gravacao (acrescenta 'DOIS') com sucesso");

        tape::TapeImage img;
        std::string error;
        check(tape::LoadTzxImage(anexo_output, img, error), "--anexar: o .tsx final abre");
        check(img.files.size() == 2, "--anexar: 2 arquivos na fita final");
        if (img.files.size() == 2) {
            check(img.files[0].name == "UM" && img.files[0].type == tape::TapeFileType::Basic,
                  "--anexar: 'UM' (1a gravacao) continua intacto");
            check(img.files[1].name == "DOIS" && img.files[1].type == tape::TapeFileType::Binary,
                  "--anexar: 'DOIS' (2a gravacao) foi acrescentado no final");
        }
    }

    // --- list ------------------------------------------------------------
    check(tape::RunCasToolCommand({"list", bin_output}, "fwmsx") == 0, "list: fita valida devolve codigo 0");
    check(tape::RunCasToolCommand({"list", TempPath("fwmsx_cas_pack_test_nao_existe.tsx")}, "fwmsx") == 1,
          "list: arquivo inexistente devolve codigo 1");

    // --- validacao de argumentos ------------------------------------------
    check(tape::RunCasToolCommand({"help"}, "fwmsx") == 0, "help: codigo 0");
    check(tape::RunCasToolCommand({}, "fwmsx") == 0, "sem argumento nenhum: mostra ajuda, codigo 0");
    check(tape::RunCasToolCommand({"comando-invalido"}, "fwmsx") == 1, "comando desconhecido: codigo 1");
    check(tape::RunCasToolCommand({"pack", "--nome", "X", bin_input, bin_output}, "fwmsx") == 1,
          "pack sem --tipo: codigo 1");
    check(tape::RunCasToolCommand({"pack", "--tipo", "bin", bin_input, bin_output}, "fwmsx") == 1,
          "pack sem --nome: codigo 1");
    check(tape::RunCasToolCommand({"pack", "--tipo", "bin", "--nome", "X", "--inicio", "xyz", "--fim", "0x8006",
                                    "--exec", "0x8000", bin_input, bin_output},
                                   "fwmsx") == 1,
          "pack --tipo bin com endereco invalido: codigo 1");
    check(tape::RunCasToolCommand({"pack", "--tipo", "bas", "--nome", "X", bas_input}, "fwmsx") == 1,
          "pack sem o arquivo de saida: codigo 1");

    if (g_failures == 0) {
        std::printf("\nTodos os testes do empacotador .CAS passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) do empacotador .CAS falharam.\n", g_failures);
    return 1;
}
