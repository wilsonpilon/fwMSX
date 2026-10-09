// fwMSX -- "fwmsx --disknew". Ver cli.h.
#include "cli.h"

#include <iostream>

#include "disk_creator.h"

namespace diskfmt {

namespace {

void PrintUsage() {
    std::cout << "fwmsx --disknew <arquivo.dsk> <formato>\n\nFormatos:\n";
    for (int i = 0; i < DISKFMT_COUNT; ++i) {
        const DiskFmtSpec *s = diskfmt_spec(static_cast<DiskFmtId>(i));
        std::cout << "  " << s->key << "\t" << s->name << "\n";
    }
    std::cout << "\nO MSX nao suporta 3 1/2 de 1,44 MB (dupla face / alta densidade).\n";
}

} // namespace

int RunDiskNewCommand(const std::vector<std::string> &args) {
    if (args.size() != 2) {
        PrintUsage();
        return args.empty() ? 0 : 2;
    }
    const DiskFmtSpec *spec = diskfmt_spec_by_key(args[1].c_str());
    if (!spec) {
        std::cerr << "fwmsx --disknew: formato desconhecido '" << args[1] << "'\n\n";
        PrintUsage();
        return 2;
    }
    std::string error;
    if (!CreateBlankDisk(args[0], spec, error)) {
        std::cerr << "fwmsx --disknew: " << error << std::endl;
        return 1;
    }
    std::cout << "disco criado: " << args[0] << " (" << spec->name << ", " << diskfmt_image_size(spec) / 1024 << " KB)" << std::endl;
    return 0;
}

} // namespace diskfmt
