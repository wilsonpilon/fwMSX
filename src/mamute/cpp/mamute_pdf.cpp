// fwMSX -- Mamute: listagens em PDF (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "mamute_pdf.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>

namespace mamute {

namespace {

constexpr int kPageW = 595, kPageH = 842; // A4 em pontos
constexpr int kMargin = 40;
constexpr int kFont = 9, kLeading = 11;
constexpr int kLinesPerPage = (kPageH - 2 * kMargin - 2 * kLeading) / kLeading; // sobra espaco para o cabecalho

std::string Escape(const std::string &s) {
    std::string out;
    for (const char c : s) {
        const unsigned char u = static_cast<unsigned char>(c);
        if (c == '(' || c == ')' || c == '\\') {
            out += '\\';
            out += c;
        } else if (u < 0x20 || u > 0x7E) {
            out += u == '\t' ? ' ' : '?';
        } else {
            out += c;
        }
    }
    return out;
}

} // namespace

bool IsPdfName(const std::string &path) {
    if (path.size() < 4) return false;
    std::string e = path.substr(path.size() - 4);
    for (char &c : e) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return e == ".pdf";
}

std::string BuildPdf(const std::string &title, const std::vector<std::string> &lines) {
    const size_t pages = std::max<size_t>(1, (lines.size() + kLinesPerPage - 1) / kLinesPerPage);
    // objetos: 1 catalogo, 2 paginas, 3 fonte, depois (conteudo, pagina) de cada folha
    std::vector<std::string> obj(3 + pages * 2);
    obj[0] = "<< /Type /Catalog /Pages 2 0 R >>";
    std::string kids;
    for (size_t p = 0; p < pages; ++p) kids += std::to_string(5 + p * 2) + " 0 R ";
    obj[1] = "<< /Type /Pages /Kids [" + kids + "] /Count " + std::to_string(pages) + " >>";
    obj[2] = "<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>";
    for (size_t p = 0; p < pages; ++p) {
        std::string stream = "BT /F1 " + std::to_string(kFont) + " Tf " + std::to_string(kLeading) + " TL " + std::to_string(kMargin) + " " +
                             std::to_string(kPageH - kMargin) + " Td\n";
        stream += "(" + Escape(title) + "  -  pagina " + std::to_string(p + 1) + "/" + std::to_string(pages) + ") Tj T*\nT*\n";
        for (size_t i = p * kLinesPerPage; i < lines.size() && i < (p + 1) * kLinesPerPage; ++i) stream += "(" + Escape(lines[i]) + ") Tj T*\n";
        stream += "ET";
        obj[3 + p * 2] = "<< /Length " + std::to_string(stream.size()) + " >>\nstream\n" + stream + "\nendstream";
        obj[4 + p * 2] = "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 " + std::to_string(kPageW) + " " + std::to_string(kPageH) +
                         "] /Resources << /Font << /F1 3 0 R >> >> /Contents " + std::to_string(4 + p * 2) + " 0 R >>";
    }
    std::string pdf = "%PDF-1.4\n";
    std::vector<size_t> offset;
    for (size_t i = 0; i < obj.size(); ++i) {
        offset.push_back(pdf.size());
        pdf += std::to_string(i + 1) + " 0 obj\n" + obj[i] + "\nendobj\n";
    }
    const size_t xref = pdf.size();
    pdf += "xref\n0 " + std::to_string(obj.size() + 1) + "\n0000000000 65535 f \n";
    for (const size_t o : offset) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%010zu 00000 n \n", o);
        pdf += buf;
    }
    pdf += "trailer\n<< /Size " + std::to_string(obj.size() + 1) + " /Root 1 0 R >>\nstartxref\n" + std::to_string(xref) + "\n%%EOF\n";
    return pdf;
}

bool WritePdf(const std::string &path, const std::string &title, const std::vector<std::string> &lines, std::string &error) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) {
        error = "NAO FOI POSSIVEL GRAVAR " + path;
        return false;
    }
    const std::string pdf = BuildPdf(title, lines);
    f.write(pdf.data(), static_cast<std::streamsize>(pdf.size()));
    return static_cast<bool>(f);
}

} // namespace mamute
