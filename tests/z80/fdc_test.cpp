// Teste da controladora de disquete WD2793 (src/fdc/) -- ver doc/fdc-spec.md.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/fdc/cpp/disk_image.h"
#include "../../src/fdc/cpp/disk_format.h"
#include "../../src/fdc/cpp/fdc_device.h"
#include "../../src/fdc/cpp/fdc_port.h"
#include "../../src/fdc/core/fdc_state.h"
#include "../../src/memmap/cpp/memory_system.h"
#include "../../src/memmap/cpp/slot_memory_bus.h"

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

// Imagem em memoria com 720KB: o primeiro byte de cada setor = numero logico do setor (modulo 256).
void Fill(fdc::DiskImage &img) {
    FdcDisk *d = img.disk();
    for (size_t s = 0; s < d->size / 512; ++s)
        for (size_t i = 0; i < 512; ++i) d->data[s * 512 + i] = static_cast<uint8_t>(i == 0 ? s : (s * 7 + i));
}

} // namespace

int main() {
    // --- 1. Geometria ----------------------------------------------------------
    {
        struct Case { size_t size; int sides, tracks, sectors; };
        const Case cases[] = {{163840, 1, 40, 8}, {184320, 1, 40, 9}, {327680, 2, 40, 8}, {368640, 2, 40, 9}, {737280, 2, 80, 9}, {1474560, 2, 80, 18}};
        for (const Case &c : cases) {
            FdcDisk d{};
            std::vector<uint8_t> blank(c.size, 0);
            const bool ok = fdc_disk_detect_geometry(&d, blank.data(), blank.size());
            check(ok && d.sides == c.sides && d.tracks == c.tracks && d.sectors == c.sectors && d.sec_size == 512,
                  "geometria por tamanho: " + std::to_string(c.size) + " bytes = " + std::to_string(c.tracks) + "x" + std::to_string(c.sides) + "x" +
                      std::to_string(c.sectors));
        }
        FdcDisk d{};
        std::vector<uint8_t> odd(12345, 0);
        check(!fdc_disk_detect_geometry(&d, odd.data(), odd.size()), "tamanho que nao e' de disquete: rejeitado");

        // BPB do setor de boot manda sobre o tamanho (360KB de 80 trilhas, 1 lado).
        std::vector<uint8_t> bpb(368640, 0);
        bpb[0x0B] = 0x00; bpb[0x0C] = 0x02;       // 512 bytes/setor
        bpb[0x13] = 0xD0; bpb[0x14] = 0x02;       // 720 setores
        bpb[0x18] = 9;    bpb[0x19] = 0;          // 9 setores/trilha
        bpb[0x1A] = 1;    bpb[0x1B] = 0;          // 1 lado
        FdcDisk e{};
        check(fdc_disk_detect_geometry(&e, bpb.data(), bpb.size()) && e.sides == 1 && e.tracks == 80 && e.sectors == 9,
              "BPB do boot: 360KB com 1 lado = 80 trilhas (nao 40x2, como o tamanho sozinho diria)");
    }

    // --- 2. Comandos tipo 1 ----------------------------------------------------
    {
        fdc::DiskImage img;
        std::string error;
        img.CreateBlank(737280, error);
        Fdc f;
        fdc_reset(&f);
        fdc_attach(&f, 0, img.disk());

        fdc_write(&f, FDC_REG_COMMAND, 0x00); // RESTORE
        uint8_t st = fdc_read(&f, FDC_REG_STATUS);
        check((st & FDC_F_TRACK0) && !(st & FDC_F_BUSY) && f.track[0] == 0 && f.r[1] == 0, "RESTORE: trilha 0, TRACK0 no status, sem BUSY");
        check((fdc_read(&f, FDC_REG_READY) & FDC_IRQ) != 0, "RESTORE: gera IRQ");

        fdc_write(&f, FDC_REG_DATA, 17); // trilha desejada
        fdc_write(&f, FDC_REG_COMMAND, 0x10); // SEEK
        st = fdc_read(&f, FDC_REG_STATUS);
        check(f.track[0] == 17 && f.r[1] == 17 && !(st & FDC_F_TRACK0), "SEEK 17: trilha fisica e registrador de trilha = 17");

        fdc_write(&f, FDC_REG_COMMAND, 0x5A); // STEP-IN-AND-UPDATE
        check(f.track[0] == 18 && f.r[1] == 18, "STEP-IN com update: 18");
        fdc_write(&f, FDC_REG_COMMAND, 0x7A); // STEP-OUT-AND-UPDATE
        fdc_write(&f, FDC_REG_COMMAND, 0x7A);
        check(f.track[0] == 16 && f.r[1] == 16, "STEP-OUT x2 com update: 16");
        fdc_write(&f, FDC_REG_COMMAND, 0x20); // STEP (repete a ultima direcao: para fora), sem update
        check(f.track[0] == 15 && f.r[1] == 16, "STEP sem update repete a direcao e deixa o registrador de trilha");

        // INDEX fica alternando a cada leitura do status (o disco gira).
        const uint8_t a = fdc_read(&f, FDC_REG_STATUS) & FDC_F_INDEX;
        const uint8_t b = fdc_read(&f, FDC_REG_STATUS) & FDC_F_INDEX;
        check(a != b, "o bit INDEX alterna entre leituras do status");
    }

    // --- 3. Leitura de setor ---------------------------------------------------
    {
        fdc::DiskImage img;
        std::string error;
        img.CreateBlank(737280, error);
        Fill(img);
        Fdc f;
        fdc_reset(&f);
        fdc_attach(&f, 0, img.disk());

        // Trilha 0, lado 0, setor 3: logico 2.
        fdc_write(&f, FDC_REG_SYSTEM, FDC_S_DENSITY | 0); // drive 0, lado 0 (S_SIDE=0 => lado 1? ver abaixo)
        fdc_write(&f, FDC_REG_SYSTEM, FDC_S_DENSITY | FDC_S_SIDE); // S_SIDE ligado = lado 0
        check(f.side == 0 && f.drive == 0, "registrador de sistema: S_SIDE ligado = lado 0, drive 0");
        fdc_write(&f, FDC_REG_COMMAND, 0x00);
        fdc_write(&f, FDC_REG_SECTOR, 3);
        fdc_write(&f, FDC_REG_COMMAND, 0x80); // READ SECTOR
        uint8_t st = fdc_read(&f, FDC_REG_STATUS);
        check((st & FDC_F_BUSY) && (st & FDC_F_DRQ), "READ SECTOR: BUSY + DRQ");
        check(fdc_read(&f, FDC_REG_READY) == FDC_DRQ, "READY mostra DRQ (40h) durante a transferencia");
        std::vector<uint8_t> got;
        for (int i = 0; i < 512; ++i) got.push_back(fdc_read(&f, FDC_REG_DATA));
        check(got[0] == 2 && got[1] == static_cast<uint8_t>(2 * 7 + 1) && got[511] == static_cast<uint8_t>(2 * 7 + 511), "READ SECTOR 3: devolve os 512 bytes do setor logico 2");
        st = fdc_read(&f, FDC_REG_STATUS);
        check(!(st & (FDC_F_BUSY | FDC_F_DRQ)) && fdc_read(&f, FDC_REG_READY) == FDC_IRQ, "fim da leitura: sem BUSY/DRQ, IRQ (80h)");

        // Multi-setor: 7 setores restantes na trilha a partir do 3 (9 por trilha).
        fdc_write(&f, FDC_REG_SECTOR, 3);
        fdc_write(&f, FDC_REG_COMMAND, 0x90); // READ SECTORS (multiplos)
        int total = 0;
        while (fdc_read(&f, FDC_REG_READY) == FDC_DRQ) {
            fdc_read(&f, FDC_REG_DATA);
            ++total;
        }
        check(total == 7 * 512 && f.r[2] == 9, "READ SECTORS multiplo: ate' o fim da trilha (7 setores = 3584 bytes), registrador de setor avanca (" + std::to_string(total) + ")");

        // Lado 1 (S_SIDE desligado), trilha 1, setor 1: logico = (1*2+1)*9 + 0 = 27
        fdc_write(&f, FDC_REG_SYSTEM, FDC_S_DENSITY); // S_SIDE=0 => lado 1
        fdc_write(&f, FDC_REG_DATA, 1);
        fdc_write(&f, FDC_REG_COMMAND, 0x10); // SEEK 1
        fdc_write(&f, FDC_REG_SECTOR, 1);
        fdc_write(&f, FDC_REG_COMMAND, 0x80);
        check(f.side == 1 && fdc_read(&f, FDC_REG_DATA) == 27, "lado 1, trilha 1, setor 1 = setor logico 27 (ordem trilha/lado/setor)");
    }

    // --- 4. Erros -----------------------------------------------------------------
    {
        fdc::DiskImage img;
        std::string error;
        img.CreateBlank(184320, error); // 180KB: 1 lado
        Fdc f;
        fdc_reset(&f);
        fdc_attach(&f, 0, img.disk());
        fdc_write(&f, FDC_REG_SYSTEM, FDC_S_DENSITY | FDC_S_SIDE);
        fdc_write(&f, FDC_REG_SECTOR, 10); // so' ha' 9 setores
        fdc_write(&f, FDC_REG_COMMAND, 0x80);
        uint8_t st = fdc_read(&f, FDC_REG_STATUS);
        check((st & FDC_F_NOTFOUND) && !(st & FDC_F_BUSY), "setor 10 em trilha de 9: NOT FOUND");
        check(fdc_read(&f, FDC_REG_READY) == FDC_IRQ, "erro de leitura gera IRQ");

        fdc_write(&f, FDC_REG_SYSTEM, FDC_S_DENSITY); // lado 1 num disco de 1 lado
        fdc_write(&f, FDC_REG_SECTOR, 1);
        fdc_write(&f, FDC_REG_COMMAND, 0x80);
        check((fdc_read(&f, FDC_REG_STATUS) & FDC_F_NOTFOUND) != 0, "lado 1 num disco de 1 lado: NOT FOUND");

        fdc_write(&f, FDC_REG_SYSTEM, FDC_S_DENSITY | 1 | FDC_S_SIDE); // drive 1: sem disco
        check((fdc_read(&f, FDC_REG_STATUS) & FDC_F_NOTREADY) != 0, "drive sem disco: NOT READY");
        fdc_write(&f, FDC_REG_SECTOR, 1);
        fdc_write(&f, FDC_REG_COMMAND, 0x80);
        check((fdc_read(&f, FDC_REG_STATUS) & FDC_F_NOTFOUND) != 0, "READ SECTOR sem disco: NOT FOUND");

        // FORCE INTERRUPT aborta uma leitura em andamento.
        fdc_write(&f, FDC_REG_SYSTEM, FDC_S_DENSITY | FDC_S_SIDE);
        fdc_write(&f, FDC_REG_SECTOR, 1);
        fdc_write(&f, FDC_REG_COMMAND, 0x80);
        fdc_read(&f, FDC_REG_DATA);
        fdc_write(&f, FDC_REG_COMMAND, 0xD8); // FORCE INTERRUPT com IRQ imediato
        check(f.rd_length == 0 && !(f.r[0] & FDC_F_BUSY) && f.irq == FDC_IRQ, "FORCE INTERRUPT: aborta a leitura e gera IRQ");

        // Comando durante BUSY e' ignorado; watchdog: ler READY 255 vezes sem tocar nos dados aborta (dado perdido).
        fdc_write(&f, FDC_REG_COMMAND, 0x80);
        fdc_write(&f, FDC_REG_COMMAND, 0x00); // ignorado (BUSY)
        check(f.cmd == 0x80, "comando enquanto BUSY e' ignorado");
        for (int i = 0; i < 255; ++i) fdc_read(&f, FDC_REG_READY);
        check((f.r[0] & FDC_F_LOSTDATA) && f.rd_length == 0 && f.irq == FDC_IRQ, "watchdog: 255 leituras de READY sem consumir dados = LOST DATA + IRQ");
    }

    // --- 5. Escrita de setor + persistencia -----------------------------------------
    {
        const std::string path = TempPath("fwmsx_fdc_test.dsk");
        {
            std::ofstream o(path, std::ios::binary);
            std::vector<uint8_t> blank(737280, 0xAA);
            o.write(reinterpret_cast<const char *>(blank.data()), static_cast<std::streamsize>(blank.size()));
        }
        fdc::DiskImage img;
        std::string error;
        check(img.Load(path, error) && img.loaded() && !img.disk()->write_protected, "Load(): imagem de 720KB carregada e gravavel (" + error + ")");
        Fdc f;
        fdc_reset(&f);
        fdc_attach(&f, 0, img.disk());
        fdc_write(&f, FDC_REG_SYSTEM, FDC_S_DENSITY | FDC_S_SIDE);
        fdc_write(&f, FDC_REG_SECTOR, 5);
        fdc_write(&f, FDC_REG_COMMAND, 0xA0); // WRITE SECTOR
        check((fdc_read(&f, FDC_REG_STATUS) & FDC_F_DRQ) != 0, "WRITE SECTOR: DRQ");
        for (int i = 0; i < 512; ++i) fdc_write(&f, FDC_REG_DATA, static_cast<uint8_t>(i ^ 0x5A));
        check(!(f.r[0] & FDC_F_BUSY) && f.irq == FDC_IRQ && img.sectors_written() == 1, "fim da escrita: sem BUSY, IRQ, 1 setor persistido");
        std::ifstream in(path, std::ios::binary);
        std::vector<char> on_disk(737280);
        in.read(on_disk.data(), static_cast<std::streamsize>(on_disk.size()));
        bool ok = true;
        for (int i = 0; i < 512; ++i) ok = ok && static_cast<uint8_t>(on_disk[4 * 512 + static_cast<size_t>(i)]) == static_cast<uint8_t>(i ^ 0x5A);
        check(ok && static_cast<uint8_t>(on_disk[3 * 512 + 511]) == 0xAA && static_cast<uint8_t>(on_disk[5 * 512]) == 0xAA,
              "o setor 5 foi gravado no ARQUIVO e os vizinhos ficaram intactos");

        // Escrita multi-setor: 2 setores => 2 persistencias.
        fdc_write(&f, FDC_REG_SECTOR, 1);
        fdc_write(&f, FDC_REG_COMMAND, 0xB0);
        for (int i = 0; i < 9 * 512; ++i) fdc_write(&f, FDC_REG_DATA, 0x11);
        check(img.sectors_written() == 1 + 9, "WRITE SECTORS multiplo: um setor persistido por vez (" + std::to_string(img.sectors_written()) + ")");

        // Protegido contra gravacao.
        img.disk()->write_protected = 1;
        fdc_write(&f, FDC_REG_SECTOR, 2);
        fdc_write(&f, FDC_REG_COMMAND, 0xA0);
        check((f.r[0] & 0x40) && f.wr_length == 0 && f.irq == FDC_IRQ, "disco protegido: WRITE SECTOR recusado (bit 40h no status)");
        std::remove(path.c_str());

        std::string bad_error;
        fdc::DiskImage none;
        check(!none.Load("/nao/existe.dsk", bad_error) && !bad_error.empty() && !none.loaded(), "Load() de arquivo inexistente: erro");
    }

    // --- 6. READ ADDRESS -----------------------------------------------------------
    {
        fdc::DiskImage img;
        std::string error;
        img.CreateBlank(737280, error);
        Fdc f;
        fdc_reset(&f);
        fdc_attach(&f, 0, img.disk());
        fdc_write(&f, FDC_REG_SYSTEM, FDC_S_DENSITY | FDC_S_SIDE);
        fdc_write(&f, FDC_REG_DATA, 7);
        fdc_write(&f, FDC_REG_COMMAND, 0x10);
        fdc_write(&f, FDC_REG_COMMAND, 0xC0); // READ ADDRESS
        uint8_t hdr[6];
        for (uint8_t &b : hdr) b = fdc_read(&f, FDC_REG_DATA);
        check(hdr[0] == 7 && hdr[1] == 0 && hdr[2] == 1 && hdr[3] == 2, "READ ADDRESS: trilha 7, lado 0, setor 1, codigo de tamanho 2 (512 bytes)");
    }

    // --- 7. Dispositivo MMIO no barramento de slots ---------------------------------
    {
        memmap::MemorySystem mem;
        std::vector<uint8_t> rom(0x8000, 0x77);        // "ROM de disco" em 4000h-7FFFh do slot 3:1
        mem.AllocateRam(0, 0, 0x10000);
        std::string error;
        check(mem.LoadRom(3, 1, rom.data(), rom.size(), &error), "LoadRom(3:1)");
        memmap::SlotMemoryBus bus(mem);
        fdc::FdcDevice dev;
        fdc::DiskImage img;
        img.CreateBlank(737280, error);
        Fill(img);
        fdc_attach(&dev.fdc(), 0, img.disk());
        bus.AttachMmio(3, 1, &dev);

        // Sem o slot 3:1 visivel, 7FF8h e' memoria normal -- o MMIO nao responde.
        bus.write(0x7FFC, 0x01);
        check(dev.fdc().side == 0, "fora do slot 3:1 o MMIO nao responde (7FFCh nao muda o lado)");

        // Seleciona 3:1 na pagina 1. O registrador secundario (FFFFh) e' do slot que
        // ocupa a pagina 3, entao paginas 1 e 3 vao para o slot primario 3 (A8h = CCh).
        bus.out(0xA8, 0xCC);
        bus.write(0xFFFF, 0x04);   // secundario do slot 3: pagina 1 = 1 (bits 2-3)
        check(bus.read(0x4000) == 0x77, "slot 3:1 visivel na pagina 1 (a ROM de disco)");
        check(bus.read(0x7000) == 0x77, "fora dos registradores a leitura e' a ROM normal");

        bus.write(0x7FFC, 0x00); // lado 1... (bit 0 = 0 => S_SIDE ligado = lado 0)
        check(dev.fdc().side == 0, "7FFCh: bit 0 = 0 escolhe o lado 0");
        bus.write(0x7FFC, 0x01);
        check(dev.fdc().side == 1, "7FFCh: bit 0 = 1 escolhe o lado 1");
        bus.write(0x7FFC, 0x00);
        bus.write(0x7FFD, 0x01);
        check(dev.fdc().drive == 1, "7FFDh: bit 0 = 1 escolhe o drive B");
        bus.write(0x7FFD, 0x00);

        bus.write(0x7FF8, 0x00);   // RESTORE
        check((bus.read(0x7FF8) & FDC_F_TRACK0) != 0, "7FF8h: comando RESTORE e leitura de status");
        bus.write(0x7FFA, 4);      // setor 4
        bus.write(0x7FF8, 0x80);   // READ SECTOR
        check(bus.read(0x7FFF) == FDC_DRQ, "7FFFh: DRQ");
        check(bus.read(0x7FFB) == 3, "7FFBh: primeiro byte do setor 4 (logico 3)");
        bus.write(0x7FFA, 5);      // a trilha/setor sao tocados enquanto BUSY: ignorado
        check(bus.read(0x7FFA) != 5, "7FFAh nao muda enquanto BUSY");

        // O espelho em BFF8h-BFFFh (MSX-DOS BDOS) tambem responde se o slot estiver na pagina 2.
        bus.out(0xA8, 0xF0);       // paginas 2 e 3 = slot primario 3
        bus.write(0xFFFF, 0x10);   // secundario: pagina 2 = 1
        check(bus.read(0xBFFF) == FDC_DRQ, "espelho em BFFFh (pagina 2): mesmo DRQ");

        // Outro slot visivel: o MMIO fica quieto.
        bus.out(0xA8, 0x00);
        bus.write(0x7FFC, 0x01);
        check(dev.fdc().side == 0, "com outro slot na pagina 1, 7FFCh e' memoria normal (nao muda o lado)");
    }

    // --- Formatos (180, 360, 720 KB) e acesso por portas (doc/fdc-spec.md, secao 6) -----
    {
        using fdc::DiskFormat;
        check(fdc::SizeAllowed(DiskFormat::Auto, 184320) && fdc::SizeAllowed(DiskFormat::Auto, 368640) &&
                  fdc::SizeAllowed(DiskFormat::Auto, 737280) && !fdc::SizeAllowed(DiskFormat::Auto, 163840),
              "formato automatico: aceita 180, 360 e 720 KB e recusa 160 KB");
        check(fdc::SizeAllowed(DiskFormat::Ss525Sd180, 184320) && !fdc::SizeAllowed(DiskFormat::Ss525Sd180, 368640),
              "5 1/4 face simples (180 KB): aceita so' 180 KB");
        check(fdc::SizeAllowed(DiskFormat::Ds35Dd720, 737280) && !fdc::SizeAllowed(DiskFormat::Ds35Dd720, 368640),
              "3 1/2 face dupla (720 KB): aceita so' 720 KB");
        fdc::DiskFormat f = DiskFormat::Auto;
        check(fdc::FormatFromDrive(false, 1, false, f) && f == DiskFormat::Ss525Sd180, "escolha 5 1/4 / 1 face / simples = 180 KB");
        check(fdc::FormatFromDrive(false, 2, true, f) && f == DiskFormat::Ds525Dd360, "escolha 5 1/4 / 2 faces / dupla = 360 KB");
        check(fdc::FormatFromDrive(true, 1, true, f) && f == DiskFormat::Ss35Dd360, "escolha 3 1/2 / 1 face / dupla = 360 KB");
        check(fdc::FormatFromDrive(true, 2, true, f) && f == DiskFormat::Ds35Dd720, "escolha 3 1/2 / 2 faces / dupla = 720 KB");
        check(!fdc::FormatFromDrive(false, 2, false, f), "5 1/4 de face dupla com densidade simples nao existe no MSX");

        // Um 360 KB de 3 1/2 (80 trilhas x 1 lado) e' lido com a geometria do formato, nao a do tamanho.
        fdc::DiskImage img;
        std::string error;
        img.CreateBlank(368640, error);
        fdc::ApplyFormat(img.disk(), *fdc::SpecFor(DiskFormat::Ss35Dd360));
        check(img.disk()->tracks == 80 && img.disk()->sides == 1 && img.disk()->sectors == 9,
              "3 1/2 SS 360 KB: geometria 80 trilhas x 1 lado x 9 setores");
    }

    {
        // Controladora por portas e por memoria: mesmo motor, mesmo setor, mesmos bytes.
        fdc::DiskImage img;
        std::string error;
        img.CreateBlank(737280, error);
        Fill(img);

        fdc::FdcDevice mem;
        fdc_attach(&mem.fdc(), 0, img.disk());
        fdc::PortFdcDevice port(0xD0);
        fdc_attach(&port.fdc(), 0, img.disk());

        // Setor 2 da trilha 1, lado 0: leitura pela porta, byte a byte.
        port.out(0xD1, 1);                   // trilha
        port.out(0xD2, 2);                   // setor
        port.out(0xD0, 0x80);                // READ SECTOR
        fdc_write(&mem.fdc(), FDC_REG_TRACK, 1);   // o mesmo comando, pela memoria
        fdc_write(&mem.fdc(), FDC_REG_SECTOR, 2);
        fdc_write(&mem.fdc(), FDC_REG_COMMAND, 0x80);
        bool same = true;
        for (int i = 0; i < 512; ++i) {
            const uint8_t pv = port.in(0xD3);
            const uint8_t mv = fdc_read(&mem.fdc(), FDC_REG_DATA); // leitura pela memoria, em paralelo
            if (pv != mv) same = false;
        }
        check(same, "porta e memoria leem o mesmo setor (512 bytes iguais)");

        // Registradores pela porta e pela memoria apontam para o mesmo estado.
        check(port.in(0xD1) == 1 && port.in(0xD2) == 2, "registradores de trilha e setor respondem pela porta");

        // base+4: bit 0 = lado (0 = lado 1), bit 1 = drive (1 = B), como o 7FFCh/7FFDh.
        port.out(0xD4, 0x12);                // drive B (bit 1), lado 1 (bit 4): convencao do Microsol
        check(port.fdc().drive == 1 && port.fdc().side == 1, "base+4 = 12h: drive B e lado 1 (bits 1 e 4, como MicrosolFDC do openMSX)");
        port.out(0xD4, 0x01);                // drive A (bit 0), lado 0
        check(port.fdc().drive == 0 && port.fdc().side == 0, "base+4 = 01h: drive A e lado 0");
        port.out(0xD4, 0x20);                // so' motor (bit 5): nao muda drive nem lado
        check(port.fdc().drive == 0 && port.fdc().side == 0, "base+4 = 20h (motor): drive e lado continuam");

        // Fora da faixa de portas nao responde (0xD5 nao e' da controladora).
        check(port.in(0xD5) == 0xFF, "porta fora da base+0..base+4 nao responde (FFh)");
        check(fdc::PortFdcDevice::kPorts == 5, "a controladora ocupa 5 portas");
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
