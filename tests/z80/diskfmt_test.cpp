// Teste do disquete novo formatado (src/diskfmt: C + Assembly + C++) e do WRITE TRACK do WD2793
// (src/fdc) -- ver doc/diskfmt-spec.md.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "../../src/diskfmt/core/diskfmt.h"
#include "../../src/diskfmt/cpp/disk_creator.h"
#include "../../src/fdc/core/fdc_state.h"
#include "../../src/msxdisk/core/msxdos1_boot.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

std::string TempPath(const char *name) {
    const char *t = std::getenv("TEMP");
    return (t ? std::string(t) : std::string("/tmp")) + "/" + name;
}

unsigned Le16(const uint8_t *p) { return p[0] | (p[1] << 8); }

// Entrada n da FAT12 (leitura).
unsigned Fat12Get(const uint8_t *fat, unsigned n) {
    const uint8_t *p = fat + (n * 3) / 2;
    return (n & 1) ? ((p[0] >> 4) | (p[1] << 4)) & 0xFFF : (p[0] | ((p[1] & 0x0F) << 8)) & 0xFFF;
}

void TestFormats() {
    struct Expect {
        DiskFmtId id;
        size_t bytes;
        unsigned media, tracks, sides;
    };
    const Expect table[] = {
        {DISKFMT_SS525_180, 184320, 0xFC, 40, 1},
        {DISKFMT_DS525_360, 368640, 0xFD, 40, 2},
        {DISKFMT_SS35_360, 368640, 0xF8, 80, 1},
        {DISKFMT_DS35_720, 737280, 0xF9, 80, 2},
    };
    for (const Expect &e : table) {
        const DiskFmtSpec *spec = diskfmt_spec(e.id);
        const std::string name = spec ? spec->key : "?";
        check(spec && diskfmt_image_size(spec) == e.bytes, name + ": tamanho da imagem");
        if (!spec) continue;
        std::vector<uint8_t> img(e.bytes, 0x77); // sujo de proposito
        check(diskfmt_build(spec, img.data(), img.size(), msxdisk::kMsxDos1BootSector720KB) == 0, name + ": diskfmt_build()");

        const uint8_t *b = img.data();
        check(Le16(b + 0x0B) == 512 && b[0x15] == e.media && b[0x10] == 2 && Le16(b + 0x0E) == 1,
              name + ": BPB (512 bytes/setor, media descriptor, 2 FATs, 1 reservado)");
        check(Le16(b + 0x13) == e.bytes / 512 && Le16(b + 0x18) == 9 && Le16(b + 0x1A) == e.sides,
              name + ": BPB (total de setores, 9 setores/trilha, lados)");
        check(b[0] == msxdisk::kMsxDos1BootSector720KB[0] &&
                  std::memcmp(b + 0x1E, msxdisk::kMsxDos1BootSector720KB + 0x1E, 0x40) == 0,
              name + ": o bootstrap Z80 do MSX-DOS foi preservado");

        const unsigned spf = Le16(b + 0x16);
        const uint8_t *fat1 = b + 512;
        const uint8_t *fat2 = b + 512 + spf * 512;
        check(Fat12Get(fat1, 0) == (0xF00u | e.media) && Fat12Get(fat1, 1) == 0xFFF && Fat12Get(fat1, 2) == 0,
              name + ": FAT com media descriptor, entrada 1 reservada e o resto livre");
        check(std::memcmp(fat1, fat2, spf * 512) == 0, name + ": as duas copias da FAT sao identicas");
        const unsigned root = 1 + 2 * spf;
        const unsigned root_secs = (Le16(b + 0x11) * 32 + 511) / 512;
        bool root_empty = true;
        for (unsigned i = 0; i < root_secs * 512; ++i) root_empty &= (b[root * 512 + i] == 0);
        check(root_empty, name + ": diretorio raiz vazio");
        const unsigned data = root + root_secs;
        check(b[data * 512] == 0xE5 && b[e.bytes - 1] == 0xE5, name + ": area de dados preenchida com E5h (Assembly)");

        FdcDisk d{};
        check(fdc_disk_detect_geometry(&d, img.data(), img.size()) && d.tracks == static_cast<int>(e.tracks) &&
                  d.sides == static_cast<int>(e.sides) && d.sectors == 9,
              name + ": o FDC reconhece a geometria pelo BPB");
    }
    check(diskfmt_spec_by_key("ds35hd") == nullptr && diskfmt_spec(static_cast<DiskFmtId>(DISKFMT_COUNT)) == nullptr,
          "nao existe formato 3 1/2 de 1,44 MB");

    std::string err;
    const std::string path = TempPath("fwmsx_diskfmt_test.dsk");
    check(diskfmt::CreateBlankDisk(path, diskfmt_spec(DISKFMT_DS35_720), err), "CreateBlankDisk(): grava o arquivo (" + err + ")");
    {
        std::ifstream in(path, std::ios::binary);
        const std::vector<char> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        check(bytes.size() == 737280 && static_cast<uint8_t>(bytes[0x15]) == 0xF9, "arquivo de 720 KB com media F9h");
    }
    std::remove(path.c_str());
}

// ---- WRITE TRACK ---------------------------------------------------------------------------

void Put(std::vector<uint8_t> &s, uint8_t v, int n = 1) { s.insert(s.end(), static_cast<size_t>(n), v); }

// Fluxo de formatacao de uma trilha MFM como um driver do MSX monta: lacunas, F5 F5 F5 FE + ID +
// F7, F5 F5 F5 FB + dados + F7. `fill(sector)` da' o byte de enchimento de cada setor.
std::vector<uint8_t> TrackStream(int track, int side, const std::vector<int> &sector_order, int size_code,
                                 uint8_t (*fill)(int)) {
    std::vector<uint8_t> s;
    Put(s, 0x4E, 80);
    Put(s, 0x00, 12);
    Put(s, 0xF6, 3);
    Put(s, 0xFC);
    Put(s, 0x4E, 50);
    for (int sec : sector_order) {
        Put(s, 0x00, 12);
        Put(s, 0xF5, 3);
        Put(s, 0xFE);
        Put(s, static_cast<uint8_t>(track));
        Put(s, static_cast<uint8_t>(side));
        Put(s, static_cast<uint8_t>(sec));
        Put(s, static_cast<uint8_t>(size_code));
        Put(s, 0xF7);
        Put(s, 0x4E, 22);
        Put(s, 0x00, 12);
        Put(s, 0xF5, 3);
        Put(s, 0xFB);
        Put(s, fill(sec), 128 << size_code);
        Put(s, 0xF7);
        Put(s, 0x4E, 24);
    }
    while (s.size() < FDC_TRACK_BYTES + 100) Put(s, 0x4E); // o driver ainda manda lacuna ate' o INTRQ
    return s;
}

uint8_t FillBySector(int sec) { return static_cast<uint8_t>(0xA0 + sec); }

void CountWrite(void *user, size_t, size_t) { ++*static_cast<int *>(user); }

void TestWriteTrack() {
    std::vector<uint8_t> img(737280, 0x00);
    FdcDisk disk{};
    disk.data = img.data();
    disk.size = img.size();
    disk.sides = 2;
    disk.tracks = 80;
    disk.sectors = 9;
    disk.sec_size = 512;
    int callbacks = 0;
    disk.write_cb = CountWrite;
    disk.write_user = &callbacks;

    Fdc f{};
    fdc_reset(&f);
    fdc_attach(&f, 0, &disk);
    fdc_write(&f, FDC_REG_SYSTEM, 0x10); // drive 0, lado 0

    // trilha 3, lado 0, setores entrelacados 1,3,5,7,9,2,4,6,8
    f.track[0] = 3;
    fdc_write(&f, FDC_REG_COMMAND, 0xF0);
    check((f.r[0] & FDC_F_BUSY) && (f.r[0] & FDC_F_DRQ) && f.irq == FDC_DRQ, "WRITE TRACK: BUSY + DRQ depois do comando");

    const std::vector<int> order = {1, 3, 5, 7, 9, 2, 4, 6, 8};
    const std::vector<uint8_t> stream = TrackStream(3, 0, order, 2, FillBySector);
    size_t sent = 0;
    while (f.trk_left > 0 && sent < stream.size()) fdc_write(&f, FDC_REG_DATA, stream[sent++]);
    check(f.trk_left == 0 && !(f.r[0] & FDC_F_BUSY) && f.irq == FDC_IRQ, "WRITE TRACK: termina com INTRQ depois de uma trilha inteira");

    bool all_ok = true;
    for (int sec = 1; sec <= 9; ++sec) {
        const size_t off = (static_cast<size_t>(3 * 2 + 0) * 9 + static_cast<size_t>(sec - 1)) * 512;
        for (size_t i = 0; i < 512; ++i) all_ok &= (img[off + i] == FillBySector(sec));
    }
    check(all_ok, "WRITE TRACK: cada setor recebeu os dados do seu proprio ID (entrelacamento respeitado)");
    check(callbacks == 9, "WRITE TRACK: a gravacao foi avisada ao dono da imagem (9 setores)");
    bool others_untouched = true;
    for (size_t i = 0; i < img.size(); ++i) {
        const size_t sector = i / 512;
        if (sector >= 54 && sector < 63) continue; // trilha 3, lado 0 = setores logicos 54..62
        others_untouched &= (img[i] == 0x00);
    }
    check(others_untouched, "WRITE TRACK: nenhum outro setor da imagem foi tocado");

    // lado 1 da trilha 5, com um ID de setor inexistente (10) no meio: ignorado, sem estragar o resto
    fdc_write(&f, FDC_REG_SYSTEM, 0x00); // lado 1
    f.track[0] = 5;
    f.r[0] = 0;
    fdc_write(&f, FDC_REG_COMMAND, 0xF4);
    const std::vector<uint8_t> s2 = TrackStream(5, 1, {1, 10, 2}, 2, FillBySector);
    sent = 0;
    while (f.trk_left > 0 && sent < s2.size()) fdc_write(&f, FDC_REG_DATA, s2[sent++]);
    const size_t base = (static_cast<size_t>(5 * 2 + 1) * 9) * 512;
    check(img[base] == FillBySector(1) && img[base + 512] == FillBySector(2) && img[base + 2 * 512] == 0x00,
          "WRITE TRACK: lado 1; ID de setor inexistente (10) e' ignorado");

    // protegido contra gravacao
    disk.write_protected = 1;
    f.r[0] = 0;
    fdc_write(&f, FDC_REG_COMMAND, 0xF0);
    check(f.trk_left == 0 && (f.r[0] & 0x40) && f.irq == FDC_IRQ, "WRITE TRACK: disco protegido responde Write Protect e nao formata");

    // sem disco
    disk.write_protected = 0;
    fdc_attach(&f, 0, nullptr);
    f.r[0] = 0;
    fdc_write(&f, FDC_REG_COMMAND, 0xF0);
    check(f.trk_left == 0 && (f.r[0] & FDC_F_NOTREADY), "WRITE TRACK: sem disco no drive = Not Ready");
}

} // namespace

int main() {
    TestFormats();
    TestWriteTrack();
    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
