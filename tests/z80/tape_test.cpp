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

        // Regressao (2026-10-07): bloco #35 (Custom info) com a string de
        // identificacao de 16 bytes (nao 10 -- o TZX_format.md escreve o
        // deslocamento do campo seguinte em HEXADECIMAL, "0x10"), igual ao
        // que .TSX reais tem (ex.: makeTSX grava "TSX.RIPPER" ali). Um .TSX
        // com esse bloco ANTES do #4B tinha a leitura inteira corrompida.
        std::vector<uint8_t> tsx35 = {'Z', 'X', 'T', 'a', 'p', 'e', '!', 0x1A, 1, 20};
        tsx35.push_back(0x35);
        const char ident[16] = {'T', 'S', 'X', '.', 'R', 'I', 'P', 'P', 'E', 'R', ' ', ' ', ' ', ' ', ' ', ' '};
        tsx35.insert(tsx35.end(), ident, ident + 16);
        const std::vector<uint8_t> custom_info = {'m', 'a', 'k', 'e', 'T', 'S', 'X'};
        AppendU32(tsx35, static_cast<uint32_t>(custom_info.size()));
        tsx35.insert(tsx35.end(), custom_info.begin(), custom_info.end());
        const std::vector<uint8_t> data_chunk2 = {0x01, 0x02};
        AppendBlock4B(tsx35, data_chunk2);
        const std::string tsx35_path = TempPath("fwmsx_tape_test_35.tsx");
        WriteFile(tsx35_path, tsx35);

        tape::TapeImage img35;
        std::string error35;
        check(tape::LoadTzxImage(tsx35_path, img35, error35), "LoadTzxImage com bloco #35 antes do #4B (" + error35 + ")");
        check(!img35.pulses.empty() && !img35.fast_bytes.empty(), "bloco #35 nao corrompe a leitura do #4B seguinte");
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

        // Regressao (2026-10-07, bug real com um .TSX real baixado pelo
        // usuario): dois blocos #4B consecutivos cujo tamanho combinado NAO
        // e' multiplo de 8 (o caso comum -- so' por acaso cai num
        // multiplo). Sem o preenchimento ate' o proximo multiplo de 8 antes
        // do SEGUNDO cabecalho (ver tzx_reader.cpp), o segundo TAPION
        // (equivalente a um 2o BLOAD"CAS:" na mesma fita) desalinhava e
        // nunca mais achava cabecalho nenhum -- "Device I/O error".
        std::vector<uint8_t> tsx_align = {'Z', 'X', 'T', 'a', 'p', 'e', '!', 0x1A, 1, 20};
        const std::vector<uint8_t> odd_data(167, 0xAB); // tamanho NAO multiplo de 8
        AppendBlock4B(tsx_align, odd_data);
        const std::vector<uint8_t> second_data = {0xCD, 0xEF, 0x12};
        AppendBlock4B(tsx_align, second_data);
        const std::string tsx_align_path = TempPath("fwmsx_tape_test_align.tsx");
        WriteFile(tsx_align_path, tsx_align);

        check(engine.Insert(tsx_align_path, tape_error), "fita de alinhamento inserida (" + tape_error + ")");

        call_vector(0x00E1); // TAPION: acha o 1o cabecalho
        check((cpu.af() & Z80_C_FLAG) == 0, "alinhamento: TAPION acha o 1o cabecalho");
        for (int i = 0; i < 167; ++i) call_vector(0x00E4); // consome os 167 bytes do 1o bloco
        check((cpu.af() & Z80_C_FLAG) == 0, "alinhamento: le os 167 bytes do 1o bloco sem erro");

        call_vector(0x00E1); // TAPION: acha o 2o cabecalho (bloco anterior com tamanho impar)
        check((cpu.af() & Z80_C_FLAG) == 0, "alinhamento: TAPION acha o 2o cabecalho mesmo sem multiplo de 8");

        call_vector(0x00E4); // TAPIN: 1o byte do 2o bloco
        check(((cpu.af() >> 8) & 0xFF) == 0xCD, "alinhamento: 1o byte do 2o bloco e' o esperado");
        check((cpu.af() & Z80_C_FLAG) == 0, "alinhamento: sucesso");
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

    // --- 7. Gravacao (TAPOON/TAPOUT/TAPOOF), fita nova, proteção e modos ----
    {
        memmap::MemorySystem mem;
        std::vector<uint8_t> bios(0x4000, 0);
        std::string load_error;
        check(mem.LoadRom(0, 0, bios.data(), bios.size(), &load_error), "ROM de teste carregada em 0:0 p/ gravacao (" + load_error + ")");
        mem.AllocateRamTop(1, 0, 0x4000);
        memmap_switch_primary(&mem.state(), 0x40);
        mem.PokeSlot(1, 0, 0xC000, 0x76);

        memmap::SlotMemoryBus bus(mem);
        tape::TapeEngine engine(mem);
        bus.AttachTapeHook(&engine);
        engine.SetMode(tape::TapeMode::Fast);

        z80::Z80Cpu cpu(bus);
        cpu.reset();

        auto call_vector = [&](uint16_t addr, int a_reg = -1) {
            if (a_reg >= 0) cpu.set_af(static_cast<uint16_t>((a_reg << 8) | (cpu.af() & 0x00FF)));
            cpu.set_sp(0xFFFC);
            mem.PokeSlot(1, 0, 0xFFFC, 0x00);
            mem.PokeSlot(1, 0, 0xFFFD, 0xC0);
            cpu.set_pc(addr);
            cpu.run(40);
        };

        std::string tape_error;

        // --- fita nova, destravada, e um round-trip simples -----------------
        const std::string blank_path = TempPath("fwmsx_tape_test_new.tsx");
        check(engine.NewBlank(blank_path, tape_error), "fita nova criada (" + tape_error + ")");
        check(engine.inserted() && !engine.read_only(), "fita nova: inserida e destravada");
        check(engine.files().empty(), "fita nova: sem arquivos");

        call_vector(0x00EA); // TAPOON
        check((cpu.af() & Z80_C_FLAG) == 0, "TAPOON (fita nova): sucesso");
        const uint8_t written[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
        bool out_ok = true;
        for (uint8_t b : written) {
            call_vector(0x00ED, b); // TAPOUT
            if (cpu.af() & Z80_C_FLAG) out_ok = false;
        }
        check(out_ok, "TAPOUT: os 5 bytes escritos com sucesso");
        call_vector(0x00F0); // TAPOOF
        check((cpu.af() & Z80_C_FLAG) == 0, "TAPOOF: sucesso");

        engine.Rewind();
        call_vector(0x00E1); // TAPION
        check((cpu.af() & Z80_C_FLAG) == 0, "TAPION depois de gravar: acha o cabecalho escrito");
        bool all_match = true;
        for (uint8_t expect : written) {
            call_vector(0x00E4);
            if (((cpu.af() >> 8) & 0xFF) != expect) all_match = false;
        }
        check(all_match, "TAPIN depois de gravar: le os mesmos 5 bytes escritos (round-trip)");

        tape::TapeImage reread;
        std::string reread_error;
        check(tape::LoadTzxImage(blank_path, reread, reread_error), "o .tsx gravado no disco e' valido (" + reread_error + ")");
        check(reread.fast_bytes.size() >= tape::kCasHeader.size() + 5 &&
                  std::equal(written, written + 5, reread.fast_bytes.end() - 5),
              "o .tsx no disco tem os 5 bytes gravados (persistencia)");

        // Regressao (2026-10-08, bug identificado apos o relato do usuario):
        // o preenchimento de alinhamento (zeros que o proprio TAPOON insere
        // antes do PROXIMO cabecalho, ver OnTapoon()) nao pode ser
        // codificado em pulso como se fosse dado de verdade. O 1o TAPOON/
        // TAPOUT/TAPOOF acima escreveu 5 bytes (nao multiplo de 8) --
        // qualquer gravacao SEGUINTE (modo "incluir no final") precisa de
        // preenchimento antes do seu proprio cabecalho. Grava mais 3 bytes
        // e confere que os pulsos desse 2o bloco tem EXATAMENTE a mesma
        // contagem que esses 3 bytes dariam isolados -- nada de pulsos
        // extras vindos do preenchimento do bloco anterior.
        engine.SetWriteMode(tape::TapeWriteMode::AppendAtEnd);
        call_vector(0x00EA);
        const uint8_t second_write[3] = {0xAA, 0xBB, 0xCC};
        for (uint8_t b : second_write) call_vector(0x00ED, b);
        call_vector(0x00F0);

        tape::TapeImage reread2;
        std::string reread2_error;
        check(tape::LoadTzxImage(blank_path, reread2, reread2_error), "2a gravacao: o .tsx persistido abre");
        std::vector<uint32_t> expected_pulses;
        const KcsByteFraming expect_cfg{1, 0, 2, 1, 0, tape::kMsxZeroPulsesPerBit, tape::kMsxOnePulsesPerBit,
                                        tape::kMsxZeroTStates, tape::kMsxOneTStates};
        for (uint8_t b : second_write) {
            kcs_emit_byte(&expect_cfg, b, +[](void *ctx, uint32_t t) { static_cast<std::vector<uint32_t> *>(ctx)->push_back(t); },
                          &expected_pulses);
        }
        check(reread2.marks.size() >= 2, "2a gravacao: os dois blocos tem marca (byte/pulso) registrada");
        if (reread2.marks.size() >= 2) {
            const std::size_t second_block_pulse_start = reread2.marks.back().pulse_index + tape::kMsxPilotPulses;
            // So' os pulsos de DADOS dos 3 bytes -- o bloco ainda tem a
            // pausa (EmitPauseMs) depois, que nao faz parte desta conta.
            const std::vector<uint32_t> actual_pulses(
                reread2.pulses.begin() + static_cast<std::ptrdiff_t>(second_block_pulse_start),
                reread2.pulses.begin() + static_cast<std::ptrdiff_t>(second_block_pulse_start + expected_pulses.size()));
            check(actual_pulses == expected_pulses,
                  "pulsos do 2o bloco sao EXATAMENTE os dos 3 bytes gravados, sem o preenchimento do 1o bloco misturado");
        }

        // Regressao (2026-10-08, bug real relatado pelo usuario): uma fita
        // GRAVADA por este emulador carregava certo no modo rapido (nao usa
        // pulso nenhum) mas nunca no modo normal -- so' o "chiado", nunca
        // achava o programa. Causa dupla: (1) os pulsos de ZERO e UM
        // estavam TROCADOS (a convencao do #4B do MSX e' zero = 2x o pulso
        // de um, e o piloto tem a MESMA duracao do um -- ver
        // resource/makeTSX/rippers/MSX4B_Ripper.h/.cpp, "bit0len =
        // bit1len*2"; a nota antiga do SPEC.md vinha dos defaults
        // GENERICOS de ZX Spectrum, nao do #4B do MSX); (2) o piloto era
        // curto demais (2000 pulsos, ~0.48s) para a calibracao da BIOS de
        // verdade -- confirmado empiricamente contra o arquivo real do
        // usuario e contra um .TSX comercial (Dinamic), cujo primeiro
        // cabecalho da fita tem piloto de varios segundos.
        check(tape::kMsxZeroTStates == tape::kMsxOneTStates * 2,
              "pulso de ZERO e' o DOBRO do pulso de UM (convencao do #4B do MSX, nao a generica de ZX)");
        check(tape::kMsxPilotTStates == tape::kMsxOneTStates, "piloto tem a MESMA duracao do pulso de UM");
        check(tape::kMsxPilotPulses * tape::kMsxPilotTStates >= 3579545ULL,
              "piloto dura pelo menos ~1s (pulsos curtos demais nao calibram a BIOS de verdade)");

        // --- protecao contra gravacao (fita de arquivo comeca travada) -----
        const std::string ro_path = TempPath("fwmsx_tape_test_ro.cas");
        {
            std::ifstream in(cas_path, std::ios::binary);
            std::ofstream out(ro_path, std::ios::binary);
            out << in.rdbuf();
        }
        check(engine.Insert(ro_path, tape_error), "fita de arquivo inserida (copia dedicada, " + tape_error + ")");
        check(engine.read_only(), "fita de arquivo: comeca travada (so' leitura)");
        call_vector(0x00EA);
        check((cpu.af() & Z80_C_FLAG) != 0, "TAPOON com a fita travada: falha (carry ligado)");
        engine.SetReadOnly(false);
        call_vector(0x00EA);
        check((cpu.af() & Z80_C_FLAG) == 0, "TAPOON depois de destravar: sucesso");
        call_vector(0x00F0);

        // --- modo "nova fita" (ao gravar, limpa tudo) -----------------------
        engine.SetWriteMode(tape::TapeWriteMode::NewTape);
        call_vector(0x00EA);
        call_vector(0x00ED, 0x7A);
        call_vector(0x00F0);
        tape::TapeImage reread_new;
        check(tape::LoadCasImage(ro_path, reread_new, reread_error), "modo NewTape: o arquivo persistido abre (" + reread_error + ")");
        check(reread_new.fast_bytes.size() == tape::kCasHeader.size() + 1 && reread_new.fast_bytes.back() == 0x7A,
              "modo NewTape: so' sobra o que foi gravado agora (o resto foi apagado)");

        // --- modo "incluir no final" (o padrao) -----------------------------
        engine.SetWriteMode(tape::TapeWriteMode::AppendAtEnd);
        const std::size_t size_before_append = reread_new.fast_bytes.size();
        call_vector(0x00EA);
        call_vector(0x00ED, 0x7B);
        call_vector(0x00F0);
        tape::TapeImage reread_append;
        check(tape::LoadCasImage(ro_path, reread_append, reread_error), "modo AppendAtEnd: o arquivo persistido abre");
        check(reread_append.fast_bytes.size() > size_before_append && reread_append.fast_bytes.back() == 0x7B,
              "modo AppendAtEnd: o que ja' existia continua, o novo byte vai depois");

        // --- marcar um arquivo (SeekToFile) e sobrescrever a partir dali ----
        check(!engine.SeekToFile(999), "SeekToFile com indice invalido devolve false");
        std::vector<uint8_t> two_files;
        two_files.insert(two_files.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
        two_files.insert(two_files.end(), 10, tape::kCasIdBasic);
        two_files.insert(two_files.end(), {'A', ' ', ' ', ' ', ' ', ' '});
        two_files.insert(two_files.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
        two_files.insert(two_files.end(), {0x01, 0x02});
        two_files.insert(two_files.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
        two_files.insert(two_files.end(), 10, tape::kCasIdBasic);
        two_files.insert(two_files.end(), {'B', ' ', ' ', ' ', ' ', ' '});
        two_files.insert(two_files.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
        two_files.insert(two_files.end(), {0x03, 0x04, 0x05});
        const std::string two_files_path = TempPath("fwmsx_tape_test_two.cas");
        WriteFile(two_files_path, two_files);

        check(engine.Insert(two_files_path, tape_error), "fita com 2 arquivos inserida");
        check(engine.files().size() == 2, "fita com 2 arquivos: os 2 foram achados");
        engine.SetReadOnly(false);
        check(engine.SeekToFile(1), "marca o 2o arquivo (indice 1, 'B')");
        check(engine.marked_file() == 1, "marked_file() reflete a marcacao");

        engine.SetWriteMode(tape::TapeWriteMode::OverwriteAtPoint);
        call_vector(0x00EA);
        check((cpu.af() & Z80_C_FLAG) == 0, "TAPOON (sobrescrever o ponto): sucesso");
        // Escreve um cabecalho de arquivo completo (16 bytes: 10x ID + 6 de
        // nome) no lugar de 'B', para ScanCasFiles() reconhecer o resultado
        // como um arquivo novo ('C') -- so' o ultimo byte (0xFF) nao seria
        // reconhecido como arquivo nenhum, so' como lixo no fim da fita.
        for (uint8_t b : {tape::kCasIdAscii, tape::kCasIdAscii, tape::kCasIdAscii, tape::kCasIdAscii, tape::kCasIdAscii,
                           tape::kCasIdAscii, tape::kCasIdAscii, tape::kCasIdAscii, tape::kCasIdAscii, tape::kCasIdAscii,
                           uint8_t('C'), uint8_t(' '), uint8_t(' '), uint8_t(' '), uint8_t(' '), uint8_t(' ')}) {
            call_vector(0x00ED, b);
        }
        call_vector(0x00F0);
        tape::TapeImage reread_overwrite;
        check(tape::LoadCasImage(two_files_path, reread_overwrite, reread_error), "modo OverwriteAtPoint: o arquivo persistido abre");
        check(reread_overwrite.files.size() == 2, "modo OverwriteAtPoint: 'A' continua, e 'C' aparece no lugar de 'B'");
        if (reread_overwrite.files.size() == 2) {
            check(reread_overwrite.files[0].name == "A", "o 1o arquivo ('A') nao foi tocado");
            check(reread_overwrite.files[1].name == "C" && reread_overwrite.files[1].type == tape::TapeFileType::Ascii,
                  "o 2o arquivo agora e' 'C' (ASCII), no lugar de 'B'");
        }

        // Regressao (2026-10-08, bug real relatado pelo usuario): um CSAVE
        // de verdade chama TAPOON/TAPOOF DUAS vezes (um bloco so' para o
        // cabecalho com o nome, outro so' para os dados do programa) -- o
        // teste acima usa UM bloco so' (simplificado). Sem o auto-retorno
        // para AppendAtEnd em OnTapoon(), a 2a chamada desta MESMA gravacao
        // cortava de NOVO, so' que agora "o ponto marcado" e' o cabecalho
        // com nome que a 1a chamada tinha acabado de escrever -- apagando-o
        // e deixando so' os dados, sem nome nenhum. Resultado: o programa
        // "desaparecia" por completo (nem o antigo nem o novo apareciam na
        // lista), exatamente como o usuario relatou: "e' como se o programa
        // sumisse, perdeu o anterior e o novo no ponto salvo".
        check(engine.Insert(two_files_path, tape_error), "CSAVE de 2 blocos: fita com 'A'/'C' reinserida");
        check(engine.files().size() == 2, "CSAVE de 2 blocos: 'A' e 'C' achados de novo");
        engine.SetReadOnly(false);
        check(engine.SeekToFile(1), "CSAVE de 2 blocos: marca o 2o arquivo ('C')");
        engine.SetWriteMode(tape::TapeWriteMode::OverwriteAtPoint);

        call_vector(0x00EA); // TAPOON -- bloco 1: cabecalho com o nome "NOVO"
        check((cpu.af() & Z80_C_FLAG) == 0, "CSAVE de 2 blocos, bloco 1 (nome): TAPOON com sucesso");
        for (uint8_t b : {tape::kCasIdBasic, tape::kCasIdBasic, tape::kCasIdBasic, tape::kCasIdBasic, tape::kCasIdBasic,
                           tape::kCasIdBasic, tape::kCasIdBasic, tape::kCasIdBasic, tape::kCasIdBasic, tape::kCasIdBasic,
                           uint8_t('N'), uint8_t('O'), uint8_t('V'), uint8_t('O'), uint8_t(' '), uint8_t(' ')}) {
            call_vector(0x00ED, b);
        }
        call_vector(0x00F0); // TAPOOF -- fecha o bloco 1
        check(engine.write_mode() == tape::TapeWriteMode::AppendAtEnd,
              "CSAVE de 2 blocos: depois do bloco 1, o modo volta sozinho para AppendAtEnd");

        call_vector(0x00EA); // TAPOON -- bloco 2: dados do programa
        check((cpu.af() & Z80_C_FLAG) == 0, "CSAVE de 2 blocos, bloco 2 (dados): TAPOON com sucesso (nao corta de novo)");
        const uint8_t program_data[3] = {0x09, 0x08, 0x07};
        for (uint8_t b : program_data) call_vector(0x00ED, b);
        call_vector(0x00F0); // TAPOOF -- fecha o bloco 2 e persiste

        tape::TapeImage reread_two_block;
        check(tape::LoadCasImage(two_files_path, reread_two_block, reread_error),
              "CSAVE de 2 blocos: o arquivo persistido abre (" + reread_error + ")");
        check(reread_two_block.files.size() == 2, "CSAVE de 2 blocos: 'A' continua, e 'NOVO' aparece no lugar de 'C' (nao desaparece)");
        if (reread_two_block.files.size() == 2) {
            check(reread_two_block.files[0].name == "A", "CSAVE de 2 blocos: o 1o arquivo ('A') nao foi tocado");
            check(reread_two_block.files[1].name == "NOVO" && reread_two_block.files[1].type == tape::TapeFileType::Basic,
                  "CSAVE de 2 blocos: o 2o arquivo agora e' 'NOVO' (BASIC), com o nome intacto");
            check(reread_two_block.files[1].data_bytes == sizeof(program_data),
                  "CSAVE de 2 blocos: os dados do 2o bloco (3 bytes) estao la', separados do nome");
        }

        // --- contagiros (odometro) ------------------------------------------
        engine.SetMode(tape::TapeMode::Normal);
        engine.Rewind();
        check(engine.odometer() == 0, "contagiros comeca em 0 apos rebobinar");
        engine.SetMotor(true);
        // O contagiros e' so' ~15 "giros" por segundo de fita (ver
        // TapeEngine::odometer()): avanca o equivalente a ~1s de CPU para
        // garantir uma leitura > 0 (100 pulsos de piloto, por exemplo, so'
        // seriam ~0.05s -- pouco demais para arredondar acima de zero).
        engine.Advance(3579545);
        check(engine.odometer() > 0, "contagiros avanca conforme a fita roda");
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes de fita passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) de fita falharam.\n", g_failures);
    return 1;
}
