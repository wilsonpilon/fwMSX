#include "tape_deck.h"

#include <algorithm>
#include <cctype>
#include <cstdio>

#include "cas_format.h"

namespace tape {

namespace {

std::string UpperCopy(std::string s) {
    for (char &c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

} // namespace

bool TapeDeck::Insert(const std::string &path, std::string &error) {
    TapeImage img;
    if (!LoadAnyTape(path, img, error)) return false;
    // Reinserir a MESMA fita (ex.: para atualizar depois de editar por fora) nao trava de novo uma fita que o usuario ja' tinha
    // destravado -- so' uma fita DIFERENTE e' protegida por padrao. Sem isso, o "destravar" se perdia em silencio e o proximo
    // CSAVE dava "Device I/O error" sem pista nenhuma.
    const bool same_tape = inserted_ && path_ == path;
    image_ = std::move(img);
    path_ = path;
    inserted_ = true;
    if (!same_tape) read_only_ = true;
    write_mode_ = TapeWriteMode::AppendAtEnd;
    marked_file_ = kNoMark;
    OnReplaced();
    return true;
}

bool TapeDeck::NewBlank(const std::string &path, std::string &error) {
    image_ = TapeImage{};
    path_ = path;
    inserted_ = true;
    read_only_ = false; // fita nova: destravada (e' o motivo de criar uma)
    write_mode_ = TapeWriteMode::AppendAtEnd;
    marked_file_ = kNoMark;
    OnReplaced();
    return SaveAnyTape(path, image_.fast_bytes, image_.marks, error);
}

void TapeDeck::Eject() {
    inserted_ = false;
    path_.clear();
    image_ = TapeImage{};
    read_only_ = true;
    marked_file_ = kNoMark;
    OnReplaced();
}

bool TapeDeck::SeekToFile(std::size_t index) {
    if (index >= image_.files.size()) return false;
    marked_file_ = index;
    return true;
}

void TapeDeck::CutForWrite() {
    switch (write_mode_) {
    case TapeWriteMode::NewTape:
        image_.fast_bytes.clear();
        image_.pulses.clear();
        image_.marks.clear();
        image_.files.clear();
        marked_file_ = kNoMark;
        break;
    case TapeWriteMode::OverwriteAtPoint:
        if (marked_file_ != kNoMark && marked_file_ < image_.files.size()) TruncateAtFile(image_, marked_file_);
        marked_file_ = kNoMark;
        break;
    case TapeWriteMode::AppendAtEnd:
        break;
    }
    write_mode_ = TapeWriteMode::AppendAtEnd;
}

bool TapeDeck::AppendFile(uint8_t type_id, const std::string &name, const std::vector<uint8_t> &data, std::string &error) {
    if (!inserted_) {
        error = "nao ha' fita inserida (use tape <arquivo> ou tape new <arquivo>)";
        return false;
    }
    if (read_only_) {
        error = "fita protegida contra gravacao (tape protect off)";
        return false;
    }
    CutForWrite();
    AppendTapeFile(image_, type_id, name, data);
    OnPulsesChanged();
    Persist();
    return true;
}

bool TapeDeck::ReadFile(std::size_t index, TapeFileData &out, std::string &error) const {
    if (!inserted_) {
        error = "nao ha' fita inserida";
        return false;
    }
    return ReadTapeFile(image_, index, out, error);
}

bool TapeDeck::FindFile(const std::string &name_or_index, std::size_t from, std::size_t &index) const {
    if (name_or_index.empty()) return false;
    if (std::all_of(name_or_index.begin(), name_or_index.end(), [](char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; })) {
        const std::size_t n = static_cast<std::size_t>(std::stoul(name_or_index));
        if (n >= image_.files.size()) return false;
        index = n;
        return true;
    }
    const std::string want = UpperCopy(name_or_index);
    for (std::size_t i = from; i < image_.files.size(); ++i) {
        if (UpperCopy(image_.files[i].name) == want) {
            index = i;
            return true;
        }
    }
    return false;
}

bool TapeDeck::SaveAs(const std::string &path, std::string &error) const {
    if (!inserted_) {
        error = "nao ha' fita inserida";
        return false;
    }
    return SaveAnyTape(path, image_.fast_bytes, image_.marks, error);
}

void TapeDeck::Persist() {
    if (path_.empty()) return;
    std::string err;
    SaveAnyTape(path_, image_.fast_bytes, image_.marks, err);
}

const char *FileTypeName(TapeFileType type) {
    switch (type) {
    case TapeFileType::Binary: return "bin";
    case TapeFileType::Basic: return "bas";
    case TapeFileType::Ascii: return "asc";
    default: return "?";
    }
}

namespace {

std::string Hex4(unsigned v) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%04X", v & 0xFFFF);
    return buf;
}

} // namespace

std::string DescribeDeck(const TapeDeck &deck) {
    if (!deck.inserted()) return "fita: nenhuma inserida";
    const char *mode = deck.write_mode() == TapeWriteMode::AppendAtEnd ? "acrescentar" : deck.write_mode() == TapeWriteMode::OverwriteAtPoint ? "sobrescrever no ponto" : "fita nova";
    std::string s = "fita: \"" + deck.path() + "\" (" + TapeExtension(deck.path()) + ") " + std::to_string(deck.files().size()) + " arquivo(s), " +
                    (deck.read_only() ? "protegida" : "gravavel") + ", gravacao: " + mode + ", ponto: ";
    if (deck.marked_file() != kNoMark && deck.marked_file() < deck.files().size())
        s += std::to_string(deck.marked_file()) + " " + deck.files()[deck.marked_file()].name;
    else
        s += "inicio";
    if (!deck.skipped_blocks().empty()) s += ", " + std::to_string(deck.skipped_blocks().size()) + " bloco(s) TZX ignorado(s)";
    return s;
}

std::string ListDeck(const TapeDeck &deck) {
    if (!deck.inserted()) return "fita: nenhuma inserida";
    if (deck.files().empty()) return "fita vazia (nenhum arquivo)";
    std::string out;
    for (std::size_t i = 0; i < deck.files().size(); ++i) {
        const TapeFileEntry &f = deck.files()[i];
        std::string line = (i == deck.marked_file() ? "> " : "  ") + std::to_string(i) + " " + FileTypeName(f.type) + "  " + f.name;
        line.resize(std::max<std::size_t>(line.size(), 20), ' ');
        TapeFileData d;
        std::string err;
        if (f.type == TapeFileType::Binary && deck.ReadFile(i, d, err))
            line += " " + Hex4(d.bin_start) + "-" + Hex4(d.bin_end) + " exec " + Hex4(d.bin_exec) + "  " + std::to_string(d.data.size() - 6) + " bytes";
        else if (deck.ReadFile(i, d, err))
            line += " " + std::to_string(d.data.size()) + " bytes";
        else
            line += " ?";
        out += line;
        if (i + 1 < deck.files().size()) out += "\n";
    }
    return out;
}

} // namespace tape
