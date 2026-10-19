// fwMSX -- Mamute: a fita cassete (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md, "Fita".
//
// TAPE, R, TLOAD, TSAVE e o LOAD/SAVE de .cas/.tsx/.tzx/.wav. Online o toca-fitas e' o da MAQUINA (a mesma fita que o CLOAD e o CSAVE do MSX
// em execucao usam: o que o Mamute grava, o BASIC enxerga, e a posicao e' uma so'); offline e' um deck proprio do monitor, sem Z80 nem BIOS.
// Quem faz o trabalho de formato e' src/tape (tape_io, tape_deck): aqui so' ha' a sintaxe dos comandos e a colocacao do bloco na memoria.
#include <algorithm>
#include <cctype>

#include "../../tape/cpp/cas_format.h"
#include "../../tape/cpp/tape_device.h"
#include "mamute_monitor.h"
#include "mamute_util.h"

namespace mamute {

using util::Hex;
using util::ParseHexN;
using util::Trim;
using util::Upper;

namespace {

// Maiusculas em tudo, MENOS o que esta' entre aspas (o caminho do arquivo: no Linux a caixa conta).
std::string UpperOutsideQuotes(const std::string &s) {
    std::string out = s;
    bool in_quotes = false;
    for (char &c : out) {
        if (c == '"') in_quotes = !in_quotes;
        else if (!in_quotes) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return out;
}

// O nome de 6 letras que a fita guarda: o nome do arquivo sem pasta nem extensao, em maiusculas.
std::string TapeNameOf(const std::string &path) {
    const size_t slash = path.find_last_of("/\\");
    std::string n = slash == std::string::npos ? path : path.substr(slash + 1);
    const size_t dot = n.rfind('.');
    if (dot != std::string::npos && dot > 0) n = n.substr(0, dot);
    n = Upper(n);
    if (n.size() > tape::kCasFileNameBytes) n.resize(tape::kCasFileNameBytes);
    return n.empty() ? "MAMUTE" : n;
}

bool ParseAddr16(const std::string &s, unsigned &out) {
    uint32_t v = 0;
    if (!ParseHexN(s, v, 4)) return false;
    out = v;
    return true;
}

} // namespace

std::string Monitor::CmdTape(const std::string &rest) {
    tape::TapeDeck &d = Deck();
    const std::string r = Trim(rest);
    const std::string where = Mem().Tape() ? " [FITA DA MAQUINA]" : " [FITA DO MONITOR]";
    if (r.empty()) return UpperOutsideQuotes(tape::DescribeDeck(d)) + where;

    const size_t sp = r.find_first_of(" \t");
    const std::string sub = Upper(r.substr(0, sp));
    const std::string arg = sp == std::string::npos ? std::string() : Trim(r.substr(sp));
    std::string err;

    if (sub == "EJECT" || sub == "EJ") {
        d.Eject();
        tape_next_ = 0;
        return "FITA EJETADA";
    }
    if (sub == "REW" || sub == "REWIND") {
        d.ClearMark();
        d.Rewind();
        tape_next_ = 0;
        return "FITA REBOBINADA";
    }
    if (sub == "LIST" || sub == "L") return UpperOutsideQuotes(tape::ListDeck(d));
    if (sub == "NEW") {
        if (arg.empty()) return "SINTAXE: TAPE NEW <arquivo.cas|.tsx|.wav>";
        if (!d.NewBlank(arg, err)) return "NAO FOI POSSIVEL CRIAR A FITA: " + err;
        tape_next_ = 0;
        return UpperOutsideQuotes(tape::DescribeDeck(d)) + where;
    }
    if (sub == "SEEK") {
        if (arg.empty()) return "SINTAXE: TAPE SEEK <n|nome|->";
        if (!d.inserted()) return "NENHUMA FITA (TAPE arq)";
        if (arg == "-") {
            d.ClearMark();
            d.Rewind();
            tape_next_ = 0;
            return "PONTO: INICIO DA FITA";
        }
        size_t index = 0;
        if (!d.FindFile(arg, 0, index)) return "A FITA NAO TEM O ARQUIVO '" + arg + "' (TAPE LIST)";
        d.SeekToFile(index);
        tape_next_ = index;
        return "PONTO: " + std::to_string(index) + " " + Upper(d.files()[index].name);
    }
    if (sub == "PROT" || sub == "PROTECT") {
        const std::string v = Upper(arg);
        if (v != "ON" && v != "OFF") return "SINTAXE: TAPE PROT ON|OFF";
        d.SetReadOnly(v == "ON");
        return v == "ON" ? "FITA PROTEGIDA CONTRA GRAVACAO" : "FITA LIBERADA PARA GRAVACAO";
    }
    if (sub == "WRITE") {
        const std::string v = Upper(arg);
        if (v == "APPEND") d.SetWriteMode(tape::TapeWriteMode::AppendAtEnd);
        else if (v == "OVER" || v == "OVERWRITE") d.SetWriteMode(tape::TapeWriteMode::OverwriteAtPoint);
        else if (v == "NEW") d.SetWriteMode(tape::TapeWriteMode::NewTape);
        else return "SINTAXE: TAPE WRITE APPEND|OVER|NEW";
        return "PROXIMA GRAVACAO: " + std::string(v == "APPEND" ? "ACRESCENTA NO FIM" : v == "NEW" ? "APAGA A FITA E COMECA DO ZERO" : "CORTA NO PONTO MARCADO (TAPE SEEK)");
    }
    if (sub == "SAVE") {
        if (arg.empty()) return "SINTAXE: TAPE SAVE <arquivo.cas|.tsx|.wav>";
        if (!d.SaveAs(arg, err)) return "NAO FOI POSSIVEL GRAVAR: " + err;
        return "FITA GRAVADA EM " + arg;
    }
    if (sub == "MODE") {
        tape::TapeEngine *engine = dynamic_cast<tape::TapeEngine *>(Mem().Tape());
        if (!engine) return "TAPE MODE SO' VALE ONLINE (A MAQUINA EMULADA E' QUEM LE A FITA)";
        const std::string v = Upper(arg);
        if (v != "FAST" && v != "NORMAL") return "SINTAXE: TAPE MODE FAST|NORMAL";
        engine->SetMode(v == "NORMAL" ? tape::TapeMode::Normal : tape::TapeMode::Fast);
        return v == "NORMAL" ? "CARREGAMENTO NORMAL (PULSOS DE VERDADE, COM SOM)" : "CARREGAMENTO RAPIDO (GANCHO DE BIOS)";
    }
    // qualquer outra palavra e' o caminho de uma fita (TAPE INSERT arq tambem serve para um arquivo com nome de comando)
    const std::string path = sub == "INSERT" ? arg : r;
    if (path.empty()) return "SINTAXE: TAPE INSERT <arquivo>";
    if (!d.Insert(path, err)) return "NAO FOI POSSIVEL ABRIR A FITA: " + err;
    tape_next_ = 0;
    return UpperOutsideQuotes(tape::DescribeDeck(d)) + where;
}

// Coloca o conteudo de um arquivo da fita na memoria. Binario: no endereco do cabecalho + `displacement` (ou em `end_arg`); o resto
// (BASIC, ASCII) so' com um endereco explicito, porque nao tem para onde ir sozinho.
std::string Monitor::PlaceTapeFile(const tape::TapeFileData &d, const std::string &slot_arg, const std::string &end_arg, unsigned displacement, bool &ok) {
    ok = false;
    std::string err;
    unsigned addr = 0;
    size_t skip = 0;
    if (!end_arg.empty()) {
        if (!ParseAddr16(end_arg, addr)) return "ENDERECO INVALIDO: " + end_arg;
        if (d.type == tape::TapeFileType::Binary) skip = 6; // com endereco explicito vai so' o programa, sem o inicio/fim/exec
    } else if (d.type == tape::TapeFileType::Binary) {
        addr = (static_cast<unsigned>(d.bin_start) + displacement) & 0xFFFF;
        skip = 6;
    } else {
        return "'" + Upper(d.name) + "' E' " + Upper(tape::FileTypeName(d.type)) + ": INFORME O ENDERECO  (TLOAD n,slot,end)";
    }
    const size_t n = d.data.size() - skip;
    if (addr + n > 0x10000) return "O ARQUIVO PASSA DE FFFF";
    const int slot = ChooseSlot(slot_arg, addr, err);
    if (slot < 0) return err;
    std::string out = PlaceBlock(slot, addr, d.data.data() + skip, n, CellType::Ram);
    out += "\nARQUIVO '" + Upper(d.name) + "' (" + Upper(tape::FileTypeName(d.type)) + ")";
    if (d.type == tape::TapeFileType::Binary) out += " EXEC " + Hex((static_cast<unsigned>(d.bin_exec) + displacement) & 0xFFFF, 4);
    ok = true;
    return out;
}

std::string Monitor::CmdTapeR(const std::vector<std::string> &f) {
    tape::TapeDeck &d = Deck();
    if (!d.inserted()) return "NENHUMA FITA (TAPE arq)";
    unsigned disp = 0;
    if (!f.empty() && !f[0].empty() && !ParseAddr16(f[0], disp)) return "SINTAXE: R [<desloc>]";
    size_t i = tape_next_;
    while (i < d.files().size() && d.files()[i].type != tape::TapeFileType::Binary) ++i;
    if (i >= d.files().size()) return "FIM DA FITA: NENHUM ARQUIVO BINARIO DEPOIS DO " + std::to_string(tape_next_) + " (TAPE REW volta ao inicio)";
    tape::TapeFileData data;
    std::string err;
    if (!d.ReadFile(i, data, err)) return "ERRO NA FITA: " + err;
    bool ok = false;
    std::string out = PlaceTapeFile(data, std::string(), std::string(), disp, ok);
    if (ok) {
        tape_next_ = i + 1;
        if (tape_next_ < d.files().size()) d.SeekToFile(tape_next_); // a fita da maquina continua dali (o CLOAD le o proximo)
    }
    return out;
}

std::string Monitor::CmdTload(const std::vector<std::string> &f) {
    if (f.empty() || f[0].empty()) return "SINTAXE: TLOAD <n|nome>[,<slot 0-3>[,<end>]]";
    tape::TapeDeck &d = Deck();
    if (!d.inserted()) return "NENHUMA FITA (TAPE arq)";
    size_t index = 0;
    if (!d.FindFile(f[0], 0, index)) return "A FITA NAO TEM O ARQUIVO '" + f[0] + "' (TAPE LIST)";
    tape::TapeFileData data;
    std::string err;
    if (!d.ReadFile(index, data, err)) return "ERRO NA FITA: " + err;
    bool ok = false;
    std::string out = PlaceTapeFile(data, f.size() > 1 ? f[1] : std::string(), f.size() > 2 ? f[2] : std::string(), 0, ok);
    if (ok) {
        tape_next_ = index + 1;
        if (tape_next_ < d.files().size()) d.SeekToFile(tape_next_);
    }
    return out;
}

std::string Monitor::CmdTsave(const std::vector<std::string> &f) {
    unsigned a = 0, b = 0, e = 0;
    if (f.size() < 3 || f[0].empty() || !ParseAddr16(f[1], a) || !ParseAddr16(f[2], b) || b < a) return "SINTAXE: TSAVE <nome>,<ini>,<fim>[,<exec>]";
    e = a;
    if (f.size() > 3 && !f[3].empty() && !ParseAddr16(f[3], e)) return "SINTAXE: TSAVE <nome>,<ini>,<fim>[,<exec>]";
    tape::TapeDeck &d = Deck();
    std::vector<uint8_t> program;
    for (unsigned x = a; x <= b; ++x) program.push_back(Mem().Read(static_cast<uint16_t>(x)));
    std::string err;
    const std::string name = TapeNameOf(f[0]);
    if (!d.AppendFile(tape::kCasIdBinary, name, tape::MakeBinaryContent(static_cast<uint16_t>(a), static_cast<uint16_t>(e), program), err))
        return "NAO FOI POSSIVEL GRAVAR NA FITA: " + Upper(err);
    tape_next_ = 0;
    return "GRAVADO NA FITA '" + name + "' - " + Hex(a, 4) + "-" + Hex(b, 4) + " - TAMANHO " + Hex(b - a + 1, 4) + " - EXEC " + Hex(e, 4);
}

// LOAD arq.cas|.tsx|.tzx|.wav: o primeiro arquivo binario do arquivo de fita (ou, com <end>, o primeiro arquivo qualquer). Nao mexe na fita inserida.
std::string Monitor::CmdLoadTape(const std::vector<std::string> &f) {
    tape::TapeImage img;
    std::string err;
    if (!tape::LoadAnyTape(f[0], img, err)) return "NAO FOI POSSIVEL ABRIR A FITA: " + err;
    const bool any = f.size() > 2 && !f[2].empty();
    size_t index = 0;
    while (index < img.files.size() && !any && img.files[index].type != tape::TapeFileType::Binary) ++index;
    if (index >= img.files.size()) return any ? "A FITA ESTA' VAZIA" : "A FITA NAO TEM NENHUM ARQUIVO BINARIO (LOAD arq,slot,end carrega o primeiro, seja qual for)";
    tape::TapeFileData data;
    if (!tape::ReadTapeFile(img, index, data, err)) return "ERRO NA FITA: " + err;
    bool ok = false;
    return PlaceTapeFile(data, f.size() > 1 ? f[1] : std::string(), f.size() > 2 ? f[2] : std::string(), 0, ok);
}

// SAVE arq.cas|.tsx|.tzx|.wav,ini,fim[,exec]: cria o arquivo de fita com UM arquivo binario (o nome de 6 letras sai do nome do arquivo).
std::string Monitor::CmdSaveTape(const std::string &path, unsigned a, unsigned b, unsigned e) {
    std::vector<uint8_t> program;
    for (unsigned x = a; x <= b; ++x) program.push_back(Mem().Read(static_cast<uint16_t>(x)));
    tape::TapeImage img;
    const std::string name = TapeNameOf(path);
    tape::AppendTapeFile(img, tape::kCasIdBinary, name, tape::MakeBinaryContent(static_cast<uint16_t>(a), static_cast<uint16_t>(e), program));
    std::string err;
    if (!tape::SaveAnyTape(path, img.fast_bytes, img.marks, err)) return "NAO FOI POSSIVEL GRAVAR " + path + ": " + err;
    return "SALVO NA FITA \"" + path + "\" - '" + name + "' - " + Hex(a, 4) + "-" + Hex(b, 4) + " - TAMANHO " + Hex(b - a + 1, 4) + " - FORMATO " + Upper(tape::TapeExtension(path));
}

// A opcao T do comando A: o codigo-objeto montado vai para a fita. Com `tape_file` (A OT>jogo.cas) cria esse arquivo; sem ele, acrescenta na fita inserida.
std::string Monitor::StoreOnTape(const std::string &name, const std::string &tape_file, const AsmProduct &p) {
    if (!p.has_code) return "T: NAO HA' CODIGO-OBJETO PARA GRAVAR";
    std::vector<uint8_t> program;
    for (int a = p.start; a <= p.end; ++a) {
        const auto it = p.image.find(a);
        program.push_back(it == p.image.end() ? 0 : it->second);
    }
    const std::vector<uint8_t> content = tape::MakeBinaryContent(static_cast<uint16_t>(p.start), static_cast<uint16_t>(p.exec), program);
    const std::string tname = TapeNameOf(name);
    std::string err;
    if (!tape_file.empty()) {
        tape::TapeImage img;
        tape::AppendTapeFile(img, tape::kCasIdBinary, tname, content);
        if (!tape::SaveAnyTape(tape_file, img.fast_bytes, img.marks, err)) return "T: NAO FOI POSSIVEL GRAVAR " + tape_file + ": " + err;
        return "GRAVADO " + tape_file + " (FITA, '" + tname + "' " + Hex(static_cast<unsigned>(p.start) & 0xFFFF, 4) + "-" + Hex(static_cast<unsigned>(p.end) & 0xFFFF, 4) + ")";
    }
    tape::TapeDeck &d = Deck();
    if (!d.AppendFile(tape::kCasIdBinary, tname, content, err)) return "T: " + Upper(err) + " (ou use A OT>arquivo.cas)";
    tape_next_ = 0;
    return "GRAVADO NA FITA '" + tname + "' (" + Hex(static_cast<unsigned>(p.start) & 0xFFFF, 4) + "-" + Hex(static_cast<unsigned>(p.end) & 0xFFFF, 4) + ")";
}

} // namespace mamute
