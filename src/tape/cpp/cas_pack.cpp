#include "cas_pack.h"

#include "cas_format.h"

namespace tape {

void AppendCasBlock(std::vector<uint8_t> &bytes, const std::vector<uint8_t> &content, std::vector<TapeMark> &marks) {
    if (bytes.size() % kCasHeader.size()) {
        bytes.resize(bytes.size() + (kCasHeader.size() - bytes.size() % kCasHeader.size()), 0);
    }
    const std::size_t header_pos = bytes.size();
    bytes.insert(bytes.end(), kCasHeader.begin(), kCasHeader.end());
    marks.push_back({header_pos, 0, content.size()});
    bytes.insert(bytes.end(), content.begin(), content.end());
}

void AppendCasFile(std::vector<uint8_t> &bytes, std::vector<TapeMark> &marks, uint8_t type_id, const std::string &name,
                    const std::vector<uint8_t> &data) {
    std::vector<uint8_t> header_block(kCasFileIdBytes, type_id);
    std::string padded_name = name.substr(0, kCasFileNameBytes);
    padded_name.resize(kCasFileNameBytes, ' ');
    header_block.insert(header_block.end(), padded_name.begin(), padded_name.end());
    AppendCasBlock(bytes, header_block, marks);
    AppendCasBlock(bytes, data, marks);
}

} // namespace tape
