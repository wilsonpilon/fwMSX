// fwMSX -- formatos de disquete suportados pelo MSX: 180 KB, 360 KB e 720 KB, escolhidos
// pela configuracao do drive (5 1/4 ou 3 1/2, face simples ou dupla, densidade simples
// ou dupla). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/fdc-spec.md, secao 6.
//
// Densidade: o WD2793 gravado no MSX usa FM (simples) ou MFM (dupla). O emulador le
// setores de 512 bytes nos dois casos; a densidade escolhida define so' qual formato
// (e qual tamanho de imagem) o drive aceita.
#pragma once

#include <cstddef>

#include "../core/fdc_state.h"

namespace fdc {

enum class DiskFormat {
    Auto = 0,     // aceita qualquer imagem de 180, 360 ou 720 KB (padrao)
    Ss525Sd180,   // 5 1/4, face simples, densidade simples: 40 trilhas x 1 lado x 9 setores = 180 KB
    Ds525Dd360,   // 5 1/4, face dupla, densidade dupla: 40 x 2 x 9 = 360 KB
    Ss35Dd360,    // 3 1/2, face simples, densidade dupla: 80 x 1 x 9 = 360 KB
    Ds35Dd720,    // 3 1/2, face dupla, densidade dupla: 80 x 2 x 9 = 720 KB
};

struct FormatSpec {
    DiskFormat format;
    const char *name;
    int tracks;
    int sides;
    int sectors;
    size_t bytes;
};

inline const FormatSpec *SpecFor(DiskFormat format) {
    static const FormatSpec kSpecs[] = {
        {DiskFormat::Ss525Sd180, "5 1/4, face simples, densidade simples (180 KB)", 40, 1, 9, 184320},
        {DiskFormat::Ds525Dd360, "5 1/4, face dupla, densidade dupla (360 KB)", 40, 2, 9, 368640},
        {DiskFormat::Ss35Dd360, "3 1/2, face simples, densidade dupla (360 KB)", 80, 1, 9, 368640},
        {DiskFormat::Ds35Dd720, "3 1/2, face dupla, densidade dupla (720 KB)", 80, 2, 9, 737280},
    };
    for (const FormatSpec &s : kSpecs) {
        if (s.format == format) return &s;
    }
    return nullptr;
}

// Nome para a interface (inclui o modo automatico).
inline const char *FormatName(DiskFormat format) {
    if (format == DiskFormat::Auto) return "automatico (180, 360 ou 720 KB)";
    const FormatSpec *s = SpecFor(format);
    return s ? s->name : "?";
}

// O drive aceita uma imagem deste tamanho? Auto aceita os tres tamanhos do MSX.
inline bool SizeAllowed(DiskFormat format, size_t bytes) {
    if (format == DiskFormat::Auto) return bytes == 184320 || bytes == 368640 || bytes == 737280;
    const FormatSpec *s = SpecFor(format);
    return s != nullptr && s->bytes == bytes;
}

// Formato correspondente a uma escolha de drive. false se a combinacao nao existe no MSX
// (ex.: 5 1/4 de face dupla com densidade simples).
inline bool FormatFromDrive(bool inch35, int sides, bool double_density, DiskFormat &out) {
    if (!inch35 && sides == 1 && !double_density) out = DiskFormat::Ss525Sd180;
    else if (!inch35 && sides == 2 && double_density) out = DiskFormat::Ds525Dd360;
    else if (inch35 && sides == 1 && double_density) out = DiskFormat::Ss35Dd360;
    else if (inch35 && sides == 2 && double_density) out = DiskFormat::Ds35Dd720;
    else return false;
    return true;
}

// Impõe a geometria do formato a imagem (trilhas/lados/setores), sobre o que o tamanho sugeriu.
// Um 360 KB de 3 1/2 (80 x 1) e' lido como 80 trilhas de 1 lado, nao 40 x 2.
inline void ApplyFormat(FdcDisk *disk, const FormatSpec &spec) {
    disk->tracks = spec.tracks;
    disk->sides = spec.sides;
    disk->sectors = spec.sectors;
    disk->sec_size = 512;
}

} // namespace fdc
