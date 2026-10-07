// Teste do empacotador e do "ripper" de .WAV de "fwmsx --cas"
// (src/tape/cpp/cas_pack.*, wav_reader.*, wav_ripper.*,
// src/tape/cli/cas_tool.*): empacota um .BIN/.BAS solto num .TSX/.CAS
// sem passar pelo emulador, ripa uma gravacao .WAV de volta para
// .TSX, e lista o conteudo de uma fita. Sem Z80/mapa de memoria --
// so' leitura/escrita de arquivo e os blocos .CAS. Ver
// doc/tape-spec.md, secoes 8-9.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/tape/cli/cas_tool.h"
#include "../../src/tape/core/kcs_codec.h"
#include "../../src/tape/cpp/cas_format.h"
#include "../../src/tape/cpp/cas_pack.h"
#include "../../src/tape/cpp/cas_reader.h"
#include "../../src/tape/cpp/tzx_reader.h"
#include "../../src/tape/cpp/wav_reader.h"
#include "../../src/tape/cpp/wav_ripper.h"

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

constexpr int kZ80Clock = 3579545;

void AppendPulseSink(void *ctx, uint32_t t_states) { static_cast<std::vector<uint32_t> *>(ctx)->push_back(t_states); }

// Sintetiza os pulsos (em T-states, a MESMA unidade de kcs_emit_byte)
// de UM bloco #4B completo: piloto + os bytes de `content`, igual uma
// gravacao de verdade teria (sem pausa nenhuma depois -- de proposito,
// para testar que o "ripper" nao confunde o piloto do PROXIMO bloco
// com mais dados do bloco atual, ver o comentario de kcs_decode_byte()
// em kcs_codec.h/.c).
void AppendModulatedBlock(std::vector<uint32_t> &pulses, const std::vector<uint8_t> &content) {
    const KcsByteFraming cfg{1, 0, 2, 1, 0, tape::kMsxZeroPulsesPerBit, tape::kMsxOnePulsesPerBit,
                              tape::kMsxZeroTStates, tape::kMsxOneTStates};
    for (uint32_t i = 0; i < tape::kMsxPilotPulses; ++i) pulses.push_back(tape::kMsxPilotTStates);
    for (uint8_t byte : content) kcs_emit_byte(&cfg, byte, &AppendPulseSink, &pulses);
}

// Converte pulsos em T-states para um .WAV PCM mono de 16 bits de
// verdade (RIFF/WAVE), em onda quadrada alternando amplitude a cada
// pulso -- so' para testar o "ripper" com dados CONHECIDOS, sem
// precisar de uma gravacao real.
void WriteSyntheticWav(const std::string &path, const std::vector<uint32_t> &pulses_tstates, uint32_t sample_rate) {
    std::vector<int16_t> samples;
    bool high = false;
    double carry = 0.0; // amostras fracionarias acumuladas, para nao perder precisao em pulsos longos
    for (uint32_t t : pulses_tstates) {
        const double exact = static_cast<double>(t) * sample_rate / kZ80Clock + carry;
        const long n = std::lround(exact);
        carry = exact - static_cast<double>(n);
        for (long i = 0; i < n; ++i) samples.push_back(high ? static_cast<int16_t>(10000) : static_cast<int16_t>(-10000));
        high = !high;
    }

    std::vector<uint8_t> wav;
    const auto appendU32 = [&](uint32_t v) {
        wav.push_back(static_cast<uint8_t>(v & 0xFF));
        wav.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        wav.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
        wav.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    };
    const auto appendU16 = [&](uint16_t v) {
        wav.push_back(static_cast<uint8_t>(v & 0xFF));
        wav.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    };
    const auto appendTag = [&](const char *s) { wav.insert(wav.end(), s, s + 4); };

    const uint32_t data_size = static_cast<uint32_t>(samples.size() * 2);
    appendTag("RIFF");
    appendU32(36 + data_size);
    appendTag("WAVE");
    appendTag("fmt ");
    appendU32(16);
    appendU16(1);               // PCM
    appendU16(1);               // mono
    appendU32(sample_rate);
    appendU32(sample_rate * 2); // bytes/seg (16 bits mono)
    appendU16(2);               // block align
    appendU16(16);              // bits por amostra
    appendTag("data");
    appendU32(data_size);
    for (int16_t s : samples) {
        wav.push_back(static_cast<uint8_t>(s & 0xFF));
        wav.push_back(static_cast<uint8_t>((s >> 8) & 0xFF));
    }
    WriteFile(path, wav);
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

    // --- rip: WAV sintetico com dados CONHECIDOS (sem gravacao real) -------
    const std::vector<uint8_t> header_block = []() {
        std::vector<uint8_t> v(tape::kCasFileIdBytes, tape::kCasIdBasic);
        const char name[tape::kCasFileNameBytes] = {'R', 'A', 'D', 'I', 'O', ' '};
        v.insert(v.end(), name, name + tape::kCasFileNameBytes);
        return v;
    }();
    const std::vector<uint8_t> data_block = {0x01, 0x02, 0xAA, 0xBB, 0x00, 0x00, 0x00};
    std::vector<uint32_t> wav_pulses;
    AppendModulatedBlock(wav_pulses, header_block); // SEM pausa entre os blocos, de proposito
    AppendModulatedBlock(wav_pulses, data_block);
    const std::string wav_path = TempPath("fwmsx_cas_pack_test.wav");
    WriteSyntheticWav(wav_path, wav_pulses, 44100);

    {
        tape::WavSamples wav;
        std::string error;
        check(tape::LoadWav(wav_path, wav, error), "LoadWav: o .wav sintetico abre (" + error + ")");
        check(wav.sample_rate == 44100, "LoadWav: taxa de amostragem correta");

        std::vector<uint8_t> fast_bytes;
        std::vector<tape::TapeMark> marks;
        tape::RipStats stats;
        check(tape::RipWavToCas(wav, fast_bytes, marks, 25, stats, error),
              "RipWavToCas: decodifica o .wav sintetico (" + error + ")");
        check(stats.blocks_found == 2, "RipWavToCas: 2 blocos encontrados (cabecalho + dados), sem pausa entre eles");
        check(stats.bytes_decoded == header_block.size() + data_block.size(), "RipWavToCas: total de bytes decodificados bate");

        const std::vector<tape::TapeFileEntry> files = tape::ScanCasFiles(fast_bytes, marks);
        check(files.size() == 1, "RipWavToCas: 1 arquivo reconhecido nos bytes ripados");
        if (files.size() == 1) {
            check(files[0].name == "RADIO" && files[0].type == tape::TapeFileType::Basic,
                  "RipWavToCas: nome e tipo corretos ('RADIO', Basic)");
            check(files[0].data_bytes == data_block.size(), "RipWavToCas: tamanho dos dados correto");
            const std::size_t data_start = files[0].fast_byte_offset + tape::kCasHeader.size() + tape::kCasFileIdBytes +
                                            tape::kCasFileNameBytes + tape::kCasHeader.size();
            check(data_start + data_block.size() <= fast_bytes.size() &&
                      std::equal(data_block.begin(), data_block.end(), fast_bytes.begin() + static_cast<std::ptrdiff_t>(data_start)),
                  "RipWavToCas: os bytes de dados decodificados sao EXATAMENTE os modulados (sem lixo do piloto seguinte)");
        }
    }

    // --- rip via CLI (fwmsx --cas rip), fim a fim at'e um .tsx novo ---------
    const std::string rip_output = TempPath("fwmsx_cas_pack_test_rip.tsx");
    check(tape::RunCasToolCommand({"rip", wav_path, rip_output}, "fwmsx") == 0, "cas rip: sucesso (codigo 0)");
    {
        tape::TapeImage img;
        std::string error;
        check(tape::LoadTzxImage(rip_output, img, error), "cas rip: o .tsx gerado abre");
        check(img.files.size() == 1 && img.files[0].name == "RADIO" && img.files[0].type == tape::TapeFileType::Basic,
              "cas rip: o .tsx gerado reconhece 'RADIO' (Basic)");
    }

    // --- rip: validacao de argumentos e erros -------------------------------
    check(tape::RunCasToolCommand({"rip", wav_path}, "fwmsx") == 1, "cas rip sem saida: codigo 1");
    check(tape::RunCasToolCommand({"rip", "--tolerancia", "abc", wav_path, rip_output}, "fwmsx") == 1,
          "cas rip --tolerancia invalida: codigo 1");
    check(tape::RunCasToolCommand({"rip", "--tolerancia", "99", wav_path, rip_output}, "fwmsx") == 1,
          "cas rip --tolerancia fora do intervalo (1-90): codigo 1");
    check(tape::RunCasToolCommand({"rip", TempPath("fwmsx_cas_pack_test_nao_existe.wav"), rip_output}, "fwmsx") == 1,
          "cas rip com .wav inexistente: codigo 1");
    {
        // .wav so' com silencio -- nenhum pulso detectavel, nenhum bloco.
        const std::string silence_path = TempPath("fwmsx_cas_pack_test_silence.wav");
        WriteSyntheticWav(silence_path, std::vector<uint32_t>(100, 2000), 44100);
        check(tape::RunCasToolCommand({"rip", silence_path, rip_output}, "fwmsx") == 1,
              "cas rip com .wav sem piloto nenhum: codigo 1");
    }
    {
        // .wav estereo: LoadWav rejeita (so' mono e' suportado).
        std::vector<uint8_t> stereo_wav = {'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E',
                                            'f', 'm', 't', ' ', 16, 0, 0, 0, 1, 0, 2, 0,
                                            0x44, 0xAC, 0, 0, 0, 0, 0, 0, 4, 0, 16, 0,
                                            'd', 'a', 't', 'a', 0, 0, 0, 0};
        const std::string stereo_path = TempPath("fwmsx_cas_pack_test_stereo.wav");
        WriteFile(stereo_path, stereo_wav);
        tape::WavSamples wav;
        std::string error;
        check(!tape::LoadWav(stereo_path, wav, error), "LoadWav: .wav estereo e' rejeitado");
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
