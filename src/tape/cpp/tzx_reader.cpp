#include "tzx_reader.h"

#include <algorithm>
#include <cstring>
#include <fstream>

#include "../core/kcs_codec.h"
#include "cas_format.h"
#include "cas_reader.h"

namespace tape {

namespace {

constexpr uint32_t kZ80Hz = 3579545;

void SinkAppend(void *ctx, uint32_t t) { static_cast<std::vector<uint32_t> *>(ctx)->push_back(t); }

// Cursor com limites -- os campos de cada bloco do TZX (ver
// resource/makeTSX/docs/TZX_format.md) tem tamanho fixo ou dado por um
// campo anterior; `fail()` fica ligado se qualquer leitura passar do fim
// do arquivo, e o chamador para' o parser nesse caso.
class Cursor {
public:
    Cursor(const uint8_t *p, std::size_t n) : p_(p), n_(n) {}

    bool ok(std::size_t need) const { return pos_ + need <= n_; }
    bool fail() const { return fail_; }
    bool eof() const { return pos_ >= n_; }
    std::size_t pos() const { return pos_; }

    uint8_t u8() {
        if (!ok(1)) { fail_ = true; return 0; }
        return p_[pos_++];
    }
    uint16_t u16() {
        if (!ok(2)) { fail_ = true; return 0; }
        const uint16_t v = static_cast<uint16_t>(p_[pos_] | (p_[pos_ + 1] << 8));
        pos_ += 2;
        return v;
    }
    uint32_t u24() {
        if (!ok(3)) { fail_ = true; return 0; }
        const uint32_t v = static_cast<uint32_t>(p_[pos_] | (p_[pos_ + 1] << 8) | (p_[pos_ + 2] << 16));
        pos_ += 3;
        return v;
    }
    uint32_t u32() {
        if (!ok(4)) { fail_ = true; return 0; }
        const uint32_t v = static_cast<uint32_t>(p_[pos_] | (p_[pos_ + 1] << 8) | (p_[pos_ + 2] << 16) | (static_cast<uint32_t>(p_[pos_ + 3]) << 24));
        pos_ += 4;
        return v;
    }
    const uint8_t *ptr(std::size_t len) {
        if (!ok(len)) { fail_ = true; return nullptr; }
        const uint8_t *r = p_ + pos_;
        pos_ += len;
        return r;
    }
    void skip(std::size_t len) {
        if (!ok(len)) { fail_ = true; pos_ = n_; return; }
        pos_ += len;
    }

private:
    const uint8_t *p_;
    std::size_t n_;
    std::size_t pos_ = 0;
    bool fail_ = false;
};

// Pausa entre blocos (TZX_format.md, secao 2, "Pause block"): um pulso
// curto no nivel oposto para terminar a borda, depois silencio (nivel
// baixo) pelo resto do tempo. `ms`==0 e' ignorado (nao muda o nivel).
void EmitPauseMs(std::vector<uint32_t> &pulses, uint32_t ms) {
    if (ms == 0) return;
    constexpr uint32_t kEdgeFinishTStates = 3580; // ~1ms a 3.58 MHz
    const uint64_t total = (static_cast<uint64_t>(ms) * kZ80Hz) / 1000;
    pulses.push_back(kEdgeFinishTStates);
    if (total > kEdgeFinishTStates) pulses.push_back(static_cast<uint32_t>(total - kEdgeFinishTStates));
}

// Blocos #10/#11/#14 (convencao ZX: piloto + 2 pulsos de sync opcionais +
// dados com 1 periodo completo -- 2 pulsos -- por bit, MSb primeiro, sem
// bits de inicio/fim). usedBitsLastByte trunca o ultimo byte.
void EmitZxDataBlock(uint16_t pilot_len, uint32_t pilot_count, uint16_t sync1, uint16_t sync2, uint16_t zero_len,
                      uint16_t one_len, uint8_t used_bits_last_byte, const uint8_t *data, std::size_t size,
                      std::vector<uint32_t> &pulses) {
    for (uint32_t i = 0; i < pilot_count; ++i) pulses.push_back(pilot_len);
    if (sync1) pulses.push_back(sync1);
    if (sync2) pulses.push_back(sync2);
    const KcsByteFraming cfg{0, 0, 0, 0, 1, 2, 2, zero_len, one_len};
    for (std::size_t i = 0; i + 1 < size; ++i) kcs_emit_byte(&cfg, data[i], &SinkAppend, &pulses);
    if (size > 0) {
        const uint8_t last = data[size - 1];
        const int used = (used_bits_last_byte == 0 || used_bits_last_byte > 8) ? 8 : used_bits_last_byte;
        for (int b = 0; b < used; ++b) {
            const int bit = (last >> (7 - b)) & 1;
            pulses.push_back(bit ? one_len : zero_len);
            pulses.push_back(bit ? one_len : zero_len);
        }
    }
}

// Deslocamento relativo (salto/chamada/selecao, ver TZX_format.md IDs
// 23/26/28): WORD armazenado como inteiro COM SINAL (complemento de 2).
int16_t ReadRelativeOffset(Cursor &cur) { return static_cast<int16_t>(cur.u16()); }

void MarkSkipped(std::vector<std::string> &skipped_blocks, const std::string &what) {
    if (std::find(skipped_blocks.begin(), skipped_blocks.end(), what) == skipped_blocks.end())
        skipped_blocks.push_back(what);
}

// Consome o CORPO de um bloco (depois do byte de ID), sem gerar pulsos
// nem guardar conteudo -- so' para a 1a passada descobrir onde cada
// bloco comeca (navegar blocos de controle, abaixo, precisa saber os
// limites de TODOS os blocos ANTES de executar, porque os saltos/lacos/
// chamadas se referem a blocos pelo NUMERO DE ORDEM, nao pelo
// deslocamento em bytes -- ver TZX_format.md, IDs 23/24/26/28). Os
// mesmos campos que ExecuteDataBlock()/o loop de navegacao leem de
// verdade na 2a passada -- manter os dois em sincronia se um formato
// novo for adicionado.
bool SkipOneBlock(Cursor &cur, uint8_t id) {
    switch (id) {
    case 0x10: cur.u16(); cur.skip(cur.u16()); break;
    case 0x11: cur.skip(12); cur.u8(); cur.skip(2); cur.skip(cur.u24()); break;
    case 0x12: cur.skip(4); break;
    case 0x13: cur.skip(static_cast<std::size_t>(cur.u8()) * 2); break;
    case 0x14: cur.skip(7); cur.skip(cur.u24()); break; // zero(2)+um(2)+bits usados(1)+pausa(2)
    case 0x15: cur.skip(5); cur.skip(cur.u24()); break;
    case 0x18: cur.skip(cur.u32()); break;
    case 0x19: cur.skip(cur.u32()); break;
    case 0x20: cur.skip(2); break;
    case 0x21: cur.skip(cur.u8()); break;
    case 0x22: break;
    case 0x23: cur.skip(2); break;
    case 0x24: cur.skip(2); break;
    case 0x25: break;
    case 0x26: cur.skip(static_cast<std::size_t>(cur.u16()) * 2); break;
    case 0x27: break;
    case 0x28: cur.skip(cur.u16()); break;
    case 0x2A: cur.skip(cur.u32()); break;
    case 0x2B: cur.skip(cur.u32()); break;
    case 0x30: cur.skip(cur.u8()); break;
    case 0x31: cur.u8(); cur.skip(cur.u8()); break;
    case 0x32: cur.skip(cur.u16()); break;
    case 0x33: cur.skip(static_cast<std::size_t>(cur.u8()) * 3); break;
    case 0x35: cur.skip(16); cur.skip(cur.u32()); break;
    case 0x4B: cur.skip(cur.u32()); break; // tamanho = 12+N, inclui o cabecalho (ver ExecuteDataBlock)
    case 0x5A: cur.skip(9); break;
    default: return false;
    }
    return true;
}

// Blocos "passivos" (nao mudam a ordem de execucao) -- a MESMA logica de
// antes desta secao navegar os de controle (23-28), so' fatorada numa
// funcao para o loop de navegacao em LoadTzxImage() poder chamar so' um
// bloco por vez, pelo PC (indice), sem se importar com o que vem depois.
bool ExecuteDataBlock(uint8_t id, Cursor &cur, TapeImage &out, std::vector<TapeMark> &marks,
                      std::vector<std::string> &skipped_blocks, const std::string &path, std::string &error) {
    switch (id) {
    case 0x10: { // Standard Speed Data Block
        const uint16_t pause = cur.u16();
        const uint16_t len = cur.u16();
        const uint8_t *data = cur.ptr(len);
        if (cur.fail() || !data) break;
        const uint32_t pilot_count = (len > 0 && data[0] < 128) ? 8063 : 3223;
        EmitZxDataBlock(2168, pilot_count, 667, 735, 855, 1710, 8, data, len, out.pulses);
        EmitPauseMs(out.pulses, pause);
        break;
    }
    case 0x11: { // Turbo Speed Data Block
        const uint16_t pilot = cur.u16();
        const uint16_t sync1 = cur.u16();
        const uint16_t sync2 = cur.u16();
        const uint16_t zero = cur.u16();
        const uint16_t one = cur.u16();
        const uint16_t pilot_count = cur.u16();
        const uint8_t used_bits = cur.u8();
        const uint16_t pause = cur.u16();
        const uint32_t len = cur.u24();
        const uint8_t *data = cur.ptr(len);
        if (cur.fail() || !data) break;
        EmitZxDataBlock(pilot, pilot_count, sync1, sync2, zero, one, used_bits, data, len, out.pulses);
        EmitPauseMs(out.pulses, pause);
        break;
    }
    case 0x12: { // Pure Tone
        const uint16_t pulse_len = cur.u16();
        const uint16_t count = cur.u16();
        for (uint16_t i = 0; i < count; ++i) out.pulses.push_back(pulse_len);
        break;
    }
    case 0x13: { // Pulse sequence
        const uint8_t count = cur.u8();
        for (uint8_t i = 0; i < count; ++i) out.pulses.push_back(cur.u16());
        break;
    }
    case 0x14: { // Pure Data Block
        const uint16_t zero = cur.u16();
        const uint16_t one = cur.u16();
        const uint8_t used_bits = cur.u8();
        const uint16_t pause = cur.u16();
        const uint32_t len = cur.u24();
        const uint8_t *data = cur.ptr(len);
        if (cur.fail() || !data) break;
        EmitZxDataBlock(0, 0, 0, 0, zero, one, used_bits, data, len, out.pulses);
        EmitPauseMs(out.pulses, pause);
        break;
    }
    case 0x15: // Direct recording -- nao reproduzido (ver doc/tape-spec.md, "Limites")
        cur.skip(5); // T-states/amostra (2) + pausa (2) + bits usados (1)
        cur.skip(cur.u24());
        MarkSkipped(skipped_blocks, "15 (gravacao direta)");
        break;
    case 0x18: // CSW recording
        cur.skip(cur.u32());
        MarkSkipped(skipped_blocks, "18 (CSW)");
        break;
    case 0x19: // Generalized Data Block
        cur.skip(cur.u32());
        MarkSkipped(skipped_blocks, "19 (bloco generalizado)");
        break;
    case 0x20: { // Pause/Stop
        const uint16_t ms = cur.u16();
        if (ms != 0) EmitPauseMs(out.pulses, ms);
        break;
    }
    case 0x2A: // Stop tape if 48K
        cur.skip(cur.u32());
        break;
    case 0x2B: // Set signal level
        cur.skip(cur.u32());
        MarkSkipped(skipped_blocks, "2B (nivel de sinal)");
        break;
    case 0x30: // Text description
        cur.skip(cur.u8());
        break;
    case 0x31: // Message block
        cur.u8();
        cur.skip(cur.u8());
        break;
    case 0x32: // Archive info
        cur.skip(cur.u16());
        break;
    case 0x33: // Hardware type
        cur.skip(static_cast<std::size_t>(cur.u8()) * 3);
        break;
    case 0x35: // Custom info block
        // BUG corrigido em 2026-10-07: o texto do TZX_format.md escreve o
        // deslocamento do campo seguinte em HEXADECIMAL ("[10,11,12,13]+14"),
        // e o proprio "CHAR[10]" da string de identificacao usa a MESMA
        // notacao -- 0x10 = 16 bytes, nao 10. Confirmado contra um .TSX
        // real (resource/fmsxgo/media/*.tsx), cujo 1o bloco e' #35 com
        // "TSX.RIPPER" + 6 espacos de preenchimento = 16 bytes exatos.
        cur.skip(16);
        cur.skip(cur.u32());
        break;
    case 0x4B: { // Kansas City Standard (MSX) -- ver doc/tape-spec.md
        const uint32_t block_len = cur.u32();
        if (block_len < 12) { cur.skip(block_len); break; }
        const uint16_t pause = cur.u16();
        const uint16_t pilot = cur.u16();
        const uint16_t pilot_count = cur.u16();
        const uint16_t zero = cur.u16();
        const uint16_t one = cur.u16();
        const uint8_t bit_cfg = cur.u8();
        const uint8_t byte_cfg = cur.u8();
        const uint32_t n = block_len - 12;
        const uint8_t *data = cur.ptr(n);
        if (cur.fail() || !data) break;

        // Decodificacao dos bits de configuracao -- conferida contra
        // resource/openMSX_TSXadv/Contrib/tsx/TsxParser.cc (so' leitura,
        // GPL, nenhum codigo copiado).
        auto decode = [](uint8_t x) { return x ? x : 16; };
        KcsByteFraming cfg{};
        cfg.zero_pulses = decode(static_cast<uint8_t>(bit_cfg >> 4));
        cfg.one_pulses = decode(static_cast<uint8_t>(bit_cfg & 0x0F));
        cfg.start_bits = (byte_cfg & 0xC0) >> 6;
        cfg.start_value = (byte_cfg & 0x20) >> 5;
        cfg.stop_bits = (byte_cfg & 0x18) >> 3;
        cfg.stop_value = (byte_cfg & 0x04) >> 2;
        cfg.msb_first = byte_cfg & 0x01;
        cfg.zero_len = zero;
        cfg.one_len = one;

        const std::size_t pulse_mark = out.pulses.size();
        for (uint16_t i = 0; i < pilot_count; ++i) out.pulses.push_back(pilot);
        for (uint32_t i = 0; i < n; ++i) kcs_emit_byte(&cfg, data[i], &SinkAppend, &out.pulses);
        EmitPauseMs(out.pulses, pause);

        // fast_bytes: reconstroi o equivalente a um .CAS (cabecalho de 8
        // bytes + os MESMOS dados, ver cas_format.h) -- o bloco #4B ja'
        // guarda `data[N]` como bytes puros, nao pulsos (bitCfg/byteCfg
        // so' dizem como GERAR os pulsos de reproducao).
        //
        // IMPORTANTE (bug encontrado em 2026-10-07 com um .TSX real, ver
        // doc/tape-spec.md): antes de CADA cabecalho de 8 bytes, o
        // TAPION/TAPOON de verdade (fMSX, Patch.c) alinha a posicao a um
        // multiplo de 8 -- e' assim que um .CAS de verdade sai do CSAVE
        // (TAPOON pula para o proximo multiplo de 8 antes de escrever o
        // cabecalho). Um .TSX tem blocos de tamanho QUALQUER (a maioria
        // das vezes nao multiplo de 8): sem este preenchimento, o
        // TAPION do bloco SEGUINTE desalinha e nunca mais acha cabecalho
        // nenhum (ate' rebobinar) -- e' exatamente o "Device I/O error"
        // de um BLOAD"CAS:" depois do primeiro.
        if (out.fast_bytes.size() % kCasHeader.size()) {
            out.fast_bytes.resize(out.fast_bytes.size() + (kCasHeader.size() - out.fast_bytes.size() % kCasHeader.size()), 0);
        }
        marks.push_back({out.fast_bytes.size(), pulse_mark, n});
        out.fast_bytes.insert(out.fast_bytes.end(), kCasHeader.begin(), kCasHeader.end());
        out.fast_bytes.insert(out.fast_bytes.end(), data, data + n);
        break;
    }
    case 0x5A: // "Glue" block (90 dec, 'Z') -- 9 bytes fixos
        cur.skip(9);
        break;
    default:
        error = "'" + path + "': bloco TZX desconhecido (ID " + std::to_string(static_cast<int>(id)) +
                "h) -- fora da lista do TZX 1.20, sem como saber o tamanho dele";
        return false;
    }
    return true;
}

} // namespace

bool LoadTzxImage(const std::string &path, TapeImage &out, std::string &error) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }
    const std::streamsize size = f.tellg();
    f.seekg(0);
    std::vector<uint8_t> file(static_cast<std::size_t>(size));
    if (size > 0 && !f.read(reinterpret_cast<char *>(file.data()), size)) {
        error = "nao foi possivel ler '" + path + "'";
        return false;
    }

    static const char kSignature[8] = {'Z', 'X', 'T', 'a', 'p', 'e', '!', 0x1A};
    if (file.size() < 10 || std::memcmp(file.data(), kSignature, 8) != 0) {
        error = "'" + path + "' nao e' um TZX/TSX valido (assinatura 'ZXTape!' ausente)";
        return false;
    }

    out = TapeImage{};
    out.from_tsx = true;

    // 1a passada: so' indexa onde cada bloco COMECA (byte do ID), sem
    // gerar pulso nem conteudo nenhum -- os blocos de controle (23-28,
    // abaixo) se referem a outros blocos pelo NUMERO DE ORDEM, nao pelo
    // deslocamento em bytes, entao precisamos saber os limites de TODOS
    // os blocos antes de poder navegar (um salto para a FRENTE, por
    // exemplo, teria que "adivinhar" o tamanho dos blocos no meio do
    // caminho se feito numa passada so').
    struct BlockMeta {
        std::size_t offset; // posicao do byte de ID no arquivo
        uint8_t id;
    };
    std::vector<BlockMeta> blocks;
    {
        Cursor idx(file.data(), file.size());
        idx.skip(10); // cabecalho (assinatura + versao)
        while (!idx.eof() && !idx.fail()) {
            const std::size_t offset = idx.pos();
            const uint8_t id = idx.u8();
            if (idx.fail()) break;
            blocks.push_back({offset, id});
            if (!SkipOneBlock(idx, id)) {
                error = "'" + path + "': bloco TZX desconhecido (ID " + std::to_string(static_cast<int>(id)) +
                        "h) -- fora da lista do TZX 1.20, sem como saber o tamanho dele";
                return false;
            }
        }
        if (idx.fail()) {
            error = "'" + path + "': arquivo truncado ou corrompido (leitura alem do fim)";
            return false;
        }
    }

    // 2a passada: executa de verdade, navegando pelos blocos de
    // controle (ver TZX_format.md, IDs 21-28) em vez de so' pular --
    // "pc" e' o INDICE do bloco atual em `blocks` (nao um deslocamento
    // em bytes). Lacos (24/25) nao se encaixam por especificacao
    // ("don't nest loop blocks"), mas usamos uma pilha mesmo assim por
    // seguranca contra um arquivo malformado; chamadas (26/27) tambem
    // guardam uma pilha (podem aninhar com lacos, segundo a mesma
    // especificacao: "you can use CALL blocks in LOOP sequences and
    // vice versa").
    struct LoopFrame {
        std::size_t start_pc; // 1o bloco do corpo do laco
        uint32_t total;       // repeticoes pedidas pelo #24
        uint32_t done;        // quantas ja' tocaram
    };
    struct CallFrame {
        std::size_t call_pc;            // o proprio bloco #26
        std::vector<int16_t> offsets;   // lista de chamadas (deslocamentos relativos)
        std::size_t next_index;         // qual da lista estamos tocando agora
    };
    std::vector<LoopFrame> loop_stack;
    std::vector<CallFrame> call_stack;
    std::vector<TapeMark> marks; // uma por bloco #4B (ver TapeMark em tape_image.h)

    // Protecao contra um TZX malformado/malicioso com um salto/laco que
    // nunca termina (ex.: "Jump 0" -- a propria especificacao avisa que
    // "isso nunca deveria acontecer", mas nao diz o que fazer se
    // acontecer) -- um limite generoso (bem mais que o numero de
    // blocos) deixa qualquer arquivo valido passar, so' pega um laco de
    // verdade infinito.
    const std::size_t step_limit = (blocks.size() + 16) * 1000;
    std::size_t steps = 0;
    std::size_t pc = 0;

    while (pc < blocks.size()) {
        if (++steps > step_limit) {
            error = "'" + path + "': possivel laco infinito nos blocos de controle do TZX (IDs 20h-28h)";
            return false;
        }
        const BlockMeta &bm = blocks[pc];
        Cursor cur(file.data(), file.size());
        cur.skip(bm.offset);
        cur.u8(); // == bm.id, ja sabido pela indexacao

        switch (bm.id) {
        case 0x21: // Group start -- so' um marcador (nome do grupo), sem efeito na ordem
        case 0x22: // Group end
            pc += 1;
            break;
        case 0x23: { // Jump to block -- deslocamento relativo ao PROPRIO bloco #23 (ver TZX_format.md)
            const int16_t value = ReadRelativeOffset(cur);
            const long target = static_cast<long>(pc) + value;
            if (target < 0 || static_cast<std::size_t>(target) >= blocks.size()) {
                error = "'" + path + "': bloco 23h (salto) aponta para fora dos limites do arquivo";
                return false;
            }
            pc = static_cast<std::size_t>(target);
            break;
        }
        case 0x24: { // Loop start
            const uint16_t repeat = cur.u16();
            if (repeat == 0) {
                // Sem nenhuma repeticao: pula o corpo todo, ate' o #25
                // correspondente (o PROXIMO #25 encontrado, ja' que a
                // especificacao nao permite lacos encadeados).
                std::size_t end = pc + 1;
                while (end < blocks.size() && blocks[end].id != 0x25) ++end;
                pc = (end < blocks.size()) ? end + 1 : blocks.size();
            } else {
                loop_stack.push_back({pc + 1, repeat, 0});
                pc += 1;
            }
            break;
        }
        case 0x25: { // Loop end
            if (loop_stack.empty()) { pc += 1; break; } // #25 sem #24 (malformado) -- so' continua
            LoopFrame &top = loop_stack.back();
            top.done += 1;
            if (top.done < top.total) {
                pc = top.start_pc; // mais uma volta
            } else {
                loop_stack.pop_back();
                pc += 1;
            }
            break;
        }
        case 0x26: { // Call sequence -- lista de chamadas, executadas uma depois da outra
            const uint16_t n = cur.u16();
            std::vector<int16_t> offsets(n);
            for (uint16_t i = 0; i < n; ++i) offsets[i] = ReadRelativeOffset(cur);
            if (n == 0) { pc += 1; break; }
            const long target = static_cast<long>(pc) + offsets[0];
            if (target < 0 || static_cast<std::size_t>(target) >= blocks.size()) {
                error = "'" + path + "': bloco 26h (chamada) aponta para fora dos limites do arquivo";
                return false;
            }
            call_stack.push_back({pc, std::move(offsets), 0});
            pc = static_cast<std::size_t>(target);
            break;
        }
        case 0x27: { // Return from sequence
            if (call_stack.empty()) { pc += 1; break; } // #27 sem #26 (malformado) -- so' continua
            CallFrame &top = call_stack.back();
            top.next_index += 1;
            if (top.next_index < top.offsets.size()) {
                // Mais uma chamada da MESMA lista do #26 original.
                const long target = static_cast<long>(top.call_pc) + top.offsets[top.next_index];
                if (target < 0 || static_cast<std::size_t>(target) >= blocks.size()) {
                    error = "'" + path + "': bloco 26h (chamada) aponta para fora dos limites do arquivo";
                    return false;
                }
                pc = static_cast<std::size_t>(target);
            } else {
                // Lista esgotada: continua depois do #26 original.
                const std::size_t after = top.call_pc + 1;
                call_stack.pop_back();
                pc = after;
            }
            break;
        }
        case 0x28: { // Select block -- sem como mostrar um menu de verdade aqui (ferramenta batch,
                     // sem interacao possivel): escolhe sempre a 1a opcao da lista, por padrao.
            cur.u16(); // tamanho do bloco inteiro -- nao precisa, ja sabido pela indexacao
            const uint8_t n = cur.u8();
            if (n == 0) { pc += 1; break; }
            const int16_t first_offset = ReadRelativeOffset(cur);
            const long target = static_cast<long>(pc) + first_offset;
            if (target < 0 || static_cast<std::size_t>(target) >= blocks.size()) {
                error = "'" + path + "': bloco 28h (selecao) aponta para fora dos limites do arquivo";
                return false;
            }
            pc = static_cast<std::size_t>(target);
            break;
        }
        default:
            if (!ExecuteDataBlock(bm.id, cur, out, marks, out.skipped_blocks, path, error)) return false;
            pc += 1;
            break;
        }
        if (cur.fail()) {
            error = "'" + path + "': arquivo truncado ou corrompido (leitura alem do fim)";
            return false;
        }
    }

    out.marks = marks;
    out.files = ScanCasFiles(out.fast_bytes, marks);
    return true;
}

} // namespace tape
