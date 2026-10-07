// Teste do modulo de fita (src/tape/) -- leitor de .CAS e de .TSX/.TZX,
// cursor de pulsos (modo normal), gancho de BIOS (modo rapido) e a
// entrada de cassete no PSG (R14, bit 7). Ver doc/tape-spec.md.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/memmap/cpp/memory_system.h"
#include "../../src/memmap/cpp/slot_memory_bus.h"
#include "../../src/psg/core/psg_state.h"
#include "../../src/tape/core/kcs_codec.h"
#include "../../src/tape/core/tape_pulse.h"
#include "../../src/tape/cpp/cas_format.h"
#include "../../src/tape/cpp/cas_reader.h"
#include "../../src/tape/cpp/tape_device.h"
#include "../../src/tape/cpp/tzx_reader.h"
#include "../../src/z80/common/z80_state.h"
#include "../../src/z80/cpp/z80_cpu.h"

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

void AppendU16(std::vector<uint8_t> &v, uint16_t x) {
    v.push_back(static_cast<uint8_t>(x & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
}
void AppendU32(std::vector<uint8_t> &v, uint32_t x) {
    v.push_back(static_cast<uint8_t>(x & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 16) & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 24) & 0xFF));
}

// Monta um bloco #4B (Kansas City Standard) com os parametros padrao do
// MSX (bitcfg 0x24, bytecfg 0x54 -- ver resource/makeTSX/TZX_Blocks.h).
void AppendBlock4B(std::vector<uint8_t> &v, const std::vector<uint8_t> &data) {
    v.push_back(0x4B);
    AppendU32(v, static_cast<uint32_t>(12 + data.size()));
    AppendU16(v, 0);                                   // pausa
    AppendU16(v, tape::kMsxPilotTStates);               // piloto
    AppendU16(v, 2000);                                 // numero de pulsos do piloto
    AppendU16(v, static_cast<uint16_t>(tape::kMsxZeroTStates));
    AppendU16(v, static_cast<uint16_t>(tape::kMsxOneTStates));
    v.push_back(0x24); // bitcfg: 2 pulsos/bit0, 4 pulsos/bit1
    v.push_back(0x54); // bytecfg: 1 bit de inicio=0, 2 de fim=1, LSb primeiro
    v.insert(v.end(), data.begin(), data.end());
}

} // namespace

int main() {
    // --- 1. Cursor de pulsos (C) -------------------------------------------
    {
        const uint32_t pulses[3] = {100, 200, 300};
        TapePulseCursor c{};
        tape_cursor_set(&c, pulses, 3);
        check(c.level == 0, "cursor comeca no nivel 0");
        check(tape_cursor_advance(&c, 50) == 0, "dentro do 1o pulso: nivel nao mudou");
        check(tape_cursor_advance(&c, 50) == 1, "fim do 1o pulso: nivel vira 1");
        check(tape_cursor_advance(&c, 200) == 0, "fim do 2o pulso: nivel volta a 0");
        // 3 pulsos = numero IMPAR de transicoes a partir do nivel 0 -> fica em 1.
        check(tape_cursor_advance(&c, 1000) == 1 && c.finished, "depois do ultimo pulso: fica parado (silencio)");
    }

    // --- 2. Framing KCS (C) -------------------------------------------------
    {
        std::vector<uint32_t> out;
        const KcsByteFraming cfg{1, 0, 2, 1, 0, 2, 4, 855, 1710};
        kcs_emit_byte(&cfg, 0x01, +[](void *ctx, uint32_t t) { static_cast<std::vector<uint32_t> *>(ctx)->push_back(t); }, &out);
        // 1 bit de inicio (valor 0 -> 2 pulsos de 855) + 8 bits de dados (bit0=1,
        // LSb primeiro -> 4 pulsos de 1710; os outros 7 bits=0 -> 2 pulsos de 855
        // cada) + 2 bits de fim (valor 1 -> 4 pulsos de 1710 cada).
        const size_t expected = 2 + (4 + 7 * 2) + 2 * 4;
        check(out.size() == expected, "byte 0x01 (LSb primeiro) gera a quantidade certa de pulsos");
        check(out[0] == 855 && out[1] == 855, "bits de inicio (valor 0) usam o pulso de zero");
        check(out[2] == 1710 && out[3] == 1710 && out[4] == 1710 && out[5] == 1710, "bit 0 (LSb de 0x01, =1) usa o pulso de um");
    }

    // --- 3. Leitor de .CAS ---------------------------------------------------
    const std::string cas_path = TempPath("fwmsx_tape_test.cas");
    {
        std::vector<uint8_t> cas;
        cas.insert(cas.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
        cas.insert(cas.end(), 10, tape::kCasIdBasic);
        const char name[6] = {'T', 'E', 'S', 'T', ' ', ' '};
        cas.insert(cas.end(), name, name + 6);
        cas.insert(cas.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
        const std::vector<uint8_t> content = {0x10, 0x20, 0x30, 0x40};
        cas.insert(cas.end(), content.begin(), content.end());
        WriteFile(cas_path, cas);

        tape::TapeImage img;
        std::string error;
        check(tape::LoadCasImage(cas_path, img, error), "LoadCasImage: le o arquivo (" + error + ")");
        check(img.fast_bytes == cas, "fast_bytes do .CAS e' o proprio arquivo");
        check(img.files.size() == 1, "um arquivo logico encontrado");
        if (!img.files.empty()) {
            check(img.files[0].type == tape::TapeFileType::Basic, "tipo BASIC (0xD3) reconhecido");
            check(img.files[0].name == "TEST", "nome sem espacos a direita");
            check(img.files[0].data_bytes == content.size(), "tamanho dos dados do arquivo");
        }
        check(!img.pulses.empty(), ".CAS sintetiza pulsos (piloto + bytes)");
    }

    // --- 4. Leitor de .TSX ---------------------------------------------------
    const std::string tsx_path = TempPath("fwmsx_tape_test.tsx");
    std::vector<uint8_t> tsx_fast_expected;
    {
        std::vector<uint8_t> tsx = {'Z', 'X', 'T', 'a', 'p', 'e', '!', 0x1A, 1, 20};
        const std::vector<uint8_t> header_chunk(16, tape::kCasIdBinary); // 10 ID + 6 nome
        AppendBlock4B(tsx, header_chunk);
        const std::vector<uint8_t> data_chunk = {0xAA, 0xBB, 0xCC};
        AppendBlock4B(tsx, data_chunk);
        // Bloco de controle (grupo), so' para conferir que e' pulado com
        // seguranca e nao estraga a leitura do resto do arquivo.
        tsx.push_back(0x21);
        tsx.push_back(4);
        tsx.insert(tsx.end(), {'T', 'e', 's', 't'});
        tsx.push_back(0x22); // fim do grupo
        WriteFile(tsx_path, tsx);

        tsx_fast_expected.insert(tsx_fast_expected.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
        tsx_fast_expected.insert(tsx_fast_expected.end(), header_chunk.begin(), header_chunk.end());
        tsx_fast_expected.insert(tsx_fast_expected.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
        tsx_fast_expected.insert(tsx_fast_expected.end(), data_chunk.begin(), data_chunk.end());

        tape::TapeImage img;
        std::string error;
        check(tape::LoadTzxImage(tsx_path, img, error), "LoadTzxImage: le o arquivo (" + error + ")");
        check(img.fast_bytes == tsx_fast_expected, "fast_bytes reconstroi o equivalente a .CAS a partir dos blocos #4B");
        check(img.files.size() == 1, "o bloco de cabecalho (16 bytes) e' reconhecido como arquivo");
        if (!img.files.empty()) {
            check(img.files[0].type == tape::TapeFileType::Binary, "tipo binario (0xD0) reconhecido no .TSX");
            check(img.files[0].data_bytes == data_chunk.size(), "tamanho dos dados no .TSX");
        }
        check(!img.pulses.empty(), ".TSX gera pulsos a partir do #4B");

        // Bloco desconhecido: tem que falhar com erro, nao travar nem ler lixo.
        std::vector<uint8_t> bad = {'Z', 'X', 'T', 'a', 'p', 'e', '!', 0x1A, 1, 20, 0xEE};
        const std::string bad_path = TempPath("fwmsx_tape_test_bad.tsx");
        WriteFile(bad_path, bad);
        tape::TapeImage bad_img;
        std::string bad_error;
        check(!tape::LoadTzxImage(bad_path, bad_img, bad_error) && !bad_error.empty(), "bloco TZX desconhecido: recusado com erro");
    }

    // --- 5. Gancho de BIOS (modo rapido) -------------------------------------
    {
        memmap::MemorySystem mem;
        std::vector<uint8_t> bios(0x4000, 0);
        std::string load_error;
        check(mem.LoadRom(0, 0, bios.data(), bios.size(), &load_error), "ROM de teste carregada em 0:0 (" + load_error + ")");
        mem.AllocateRamTop(1, 0, 0x4000); // RAM de 16KB em 1:0, para a pilha
        memmap_switch_primary(&mem.state(), 0x40); // pagina 3 (C000h-FFFFh) mostra o slot 1
        mem.PokeSlot(1, 0, 0xC000, 0x76); // HALT: ponto de chegada do RET, para' ali de verdade

        memmap::SlotMemoryBus bus(mem);
        tape::TapeEngine engine(mem);
        bus.AttachTapeHook(&engine);
        engine.SetMode(tape::TapeMode::Fast);

        std::string tape_error;
        check(engine.Insert(cas_path, tape_error), "fita (.CAS) inserida em modo rapido (" + tape_error + ")");

        z80::Z80Cpu cpu(bus);
        cpu.reset();

        auto call_vector = [&](uint16_t addr) {
            cpu.set_sp(0xFFFC);
            mem.PokeSlot(1, 0, 0xFFFC, 0x00);
            mem.PokeSlot(1, 0, 0xFFFD, 0xC0);
            cpu.set_pc(addr);
            cpu.run(40); // ED FE + o RET manual chegam ao HALT em 0xC000 e ficam ali
        };

        call_vector(0x00E1); // TAPION
        check(cpu.pc() == 0xC000, "TAPION: fez o RET para o endereco empilhado (parou no HALT)");
        check(cpu.sp() == 0xFFFE, "TAPION: SP avancou 2 (RET)");
        check((cpu.af() & Z80_C_FLAG) == 0, "TAPION: achou o cabecalho (carry limpo)");

        // TAPION so' achou o PRIMEIRO cabecalho de 8 bytes (antes do bloco de
        // 10 bytes de ID + 6 de nome) -- os proximos bytes do fluxo sao esse
        // bloco (10x 0xD3, o ID do BASIC), nao o conteudo (que vem depois de
        // um SEGUNDO cabecalho, achado por outro TAPION).
        call_vector(0x00E4); // TAPIN (1o byte)
        check(((cpu.af() >> 8) & 0xFF) == tape::kCasIdBasic, "TAPIN: 1o byte lido e' o esperado (ID do cabecalho)");
        check((cpu.af() & Z80_C_FLAG) == 0, "TAPIN: sucesso (carry limpo)");

        call_vector(0x00E4); // TAPIN (2o byte)
        check(((cpu.af() >> 8) & 0xFF) == tape::kCasIdBasic, "TAPIN: 2o byte lido em sequencia");

        call_vector(0x00E7); // TAPIOF
        check((cpu.af() & Z80_C_FLAG) == 0, "TAPIOF: sempre sucesso");

        // Sem fita: TAPION tem que falhar (carry ligado), como o fMSX
        // (Patch.c) sem CasStream.
        engine.Eject();
        call_vector(0x00E1);
        check((cpu.af() & Z80_C_FLAG) != 0, "TAPION sem fita: carry ligado (erro)");
    }

    // --- 6. Modo normal (pulsos de verdade) e entrada de cassete do PSG -----
    {
        memmap::MemorySystem mem;
        tape::TapeEngine engine(mem);
        std::string tape_error;
        check(engine.Insert(cas_path, tape_error), "fita inserida para o teste de modo normal");
        engine.SetMode(tape::TapeMode::Normal);
        check(engine.CassetteInLevel() == 0, "nivel inicial e' 0 (silencio)");

        engine.SetMotor(false);
        engine.Advance(static_cast<int>(tape::kMsxPilotTStates) * 2);
        check(engine.CassetteInLevel() == 0, "motor desligado: o cursor nao avanca");

        engine.SetMotor(true);
        engine.Advance(static_cast<int>(tape::kMsxPilotTStates) - 1);
        check(engine.CassetteInLevel() == 0, "dentro do 1o pulso do piloto: nivel ainda 0");
        engine.Advance(2);
        check(engine.CassetteInLevel() == 1, "fim do 1o pulso do piloto: nivel vira 1");

        check(engine.duration_seconds() > 0.0, "duracao da fita e' positiva");
        check(engine.position_seconds() > 0.0 && engine.position_seconds() <= engine.duration_seconds(),
              "posicao avancou e nao passa da duracao");

        PsgState psg{};
        psg_reset(&psg);
        psg.latch = 14;
        check((psg_read_data(&psg) & 0x80) == 0, "PSG R14: bit 7 comeca em 0");
        psg_set_cassette_in(&psg, engine.CassetteInLevel());
        check((psg_read_data(&psg) & 0x80) != 0, "PSG R14: bit 7 reflete o nivel da fita (1)");
        psg_set_cassette_in(&psg, 0);
        check((psg_read_data(&psg) & 0x80) == 0, "PSG R14: bit 7 reflete o nivel da fita (0)");
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes de fita passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) de fita falharam.\n", g_failures);
    return 1;
}
