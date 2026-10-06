// fwMSX -- leitura de indices de pastas. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "listing.h"

#include <cctype>
#include <cstdlib>

#include "web.h"

namespace romdb {
namespace {

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

// Entidades HTML comuns em nomes de arquivo: &amp; &lt; &gt; &quot; &#39; e &#NNN;.
std::string HtmlDecode(const std::string &text) {
    std::string out;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '&') {
            const size_t semi = text.find(';', i);
            if (semi != std::string::npos && semi - i <= 8) {
                const std::string ent = text.substr(i + 1, semi - i - 1);
                if (ent == "amp") { out += '&'; i = semi; continue; }
                if (ent == "lt") { out += '<'; i = semi; continue; }
                if (ent == "gt") { out += '>'; i = semi; continue; }
                if (ent == "quot") { out += '"'; i = semi; continue; }
                if (ent == "apos") { out += '\''; i = semi; continue; }
                if (ent.size() > 1 && ent[0] == '#') {
                    const long code = std::atol(ent.c_str() + 1);
                    if (code > 0 && code < 128) {
                        out += static_cast<char>(code);
                        i = semi;
                        continue;
                    }
                }
            }
        }
        out += text[i];
    }
    return out;
}

// Texto entre as tags (sem tags), ex.: "<TD> 32.06 MB </TD>" -> "32.06 MB".
std::string StripTags(const std::string &s) {
    std::string out;
    bool in_tag = false;
    for (char c : s) {
        if (c == '<') in_tag = true;
        else if (c == '>') in_tag = false;
        else if (!in_tag) out += c;
    }
    return Trim(out);
}

// Conteudo de cada celula <TD>...</TD> de uma linha.
std::vector<std::string> Cells(const std::string &row_lower, const std::string &row) {
    std::vector<std::string> cells;
    size_t pos = 0;
    while ((pos = row_lower.find("<td", pos)) != std::string::npos) {
        const size_t open_end = row_lower.find('>', pos);
        if (open_end == std::string::npos) break;
        const size_t close = row_lower.find("</td", open_end);
        const size_t end = close == std::string::npos ? row.size() : close;
        cells.push_back(StripTags(row.substr(open_end + 1, end - open_end - 1)));
        pos = close == std::string::npos ? row.size() : close + 4;
    }
    return cells;
}

} // namespace

std::vector<ListingEntry> ParseListing(const std::string &html, const std::string &base_url) {
    std::vector<ListingEntry> entries;
    const std::string lower = Lower(html);
    size_t pos = 0;
    while ((pos = lower.find("<tr", pos)) != std::string::npos) {
        size_t row_end = lower.find("</tr", pos);
        if (row_end == std::string::npos) row_end = html.size();
        const std::string row = html.substr(pos, row_end - pos);
        const std::string row_lower = lower.substr(pos, row_end - pos);
        pos = row_end;

        const size_t href_pos = row_lower.find("href=\"");
        if (href_pos == std::string::npos) continue;
        const size_t href_start = href_pos + 6;
        const size_t href_end = row.find('"', href_start);
        if (href_end == std::string::npos) continue;
        // O HTML codifica o link (ROM&#39;s = ROM's); a URL depois usa %XX.
        const std::string href = HtmlDecode(row.substr(href_start, href_end - href_start));
        if (href == "../" || href == "..") continue;
        if (href.rfind("http", 0) == 0) continue;  // links externos (rodape do site)

        ListingEntry e;
        e.is_dir = !href.empty() && href.back() == '/';
        e.name = UrlDecode(e.is_dir ? href.substr(0, href.size() - 1) : href);
        e.url = base_url + href;
        const std::vector<std::string> cells = Cells(row_lower, row);
        if (cells.size() >= 2) e.size = cells[1];
        if (cells.size() >= 3) e.type = cells[2];
        if (e.is_dir) e.size.clear();
        entries.push_back(e);
    }
    return entries;
}

bool ParseFullSetDate(const std::string &name, int &year, int &month, int &day) {
    // Procura "DD-MM-AAAA" (10 caracteres) no nome.
    for (size_t i = 0; i + 10 <= name.size(); ++i) {
        bool ok = true;
        for (size_t k = 0; k < 10 && ok; ++k) {
            const char c = name[i + k];
            if (k == 2 || k == 5) ok = c == '-';
            else ok = std::isdigit(static_cast<unsigned char>(c)) != 0;
        }
        if (!ok) continue;
        day = std::atoi(name.substr(i, 2).c_str());
        month = std::atoi(name.substr(i + 3, 2).c_str());
        year = std::atoi(name.substr(i + 6, 4).c_str());
        return true;
    }
    return false;
}

const ListingEntry *LatestFullSet(const std::vector<ListingEntry> &entries) {
    const ListingEntry *best = nullptr;
    long best_key = -1;
    for (const ListingEntry &e : entries) {
        if (e.is_dir || e.name.find("Full Set System ROMs") == std::string::npos) continue;
        int y = 0, m = 0, d = 0;
        if (!ParseFullSetDate(e.name, y, m, d)) continue;
        const long key = static_cast<long>(y) * 10000 + m * 100 + d;  // AAAAMMDD
        if (key > best_key) {
            best_key = key;
            best = &e;
        }
    }
    return best;
}

std::string LatestFmsxWindowsZip(const std::string &html, const std::string &base_url) {
    // Procura "fMSXNN-Windows-bin.zip" (ou "fMSXNN.zip" sem Windows) e fica com o maior NN.
    std::string best;
    int best_version = -1;
    const std::string lower = Lower(html);
    size_t pos = 0;
    while ((pos = lower.find("href=\"", pos)) != std::string::npos) {
        pos += 6;
        const size_t end = html.find('"', pos);
        if (end == std::string::npos) break;
        const std::string href = html.substr(pos, end - pos);
        pos = end;
        const std::string h = Lower(href);
        if (h.find("fmsx") == std::string::npos || h.find("windows-bin.zip") == std::string::npos) continue;
        size_t digits = h.find("fmsx");
        int version = std::atoi(href.c_str() + digits + 4);
        if (version > best_version) {
            best_version = version;
            best = base_url + href;
        }
    }
    return best;
}

} // namespace romdb
