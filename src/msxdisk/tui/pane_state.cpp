//
// msxdisk-tui (fwMSX): estado/navegacao dos paineis -- ver pane_state.h.
//

#include "pane_state.h"

#include <algorithm>

#include "../core/dir_entry.h"

namespace msxdisk::tui {

namespace {

void ClampCursor(int &cursor, size_t count) {
    if (count == 0) {
        cursor = 0;
    } else if (cursor >= static_cast<int>(count)) {
        cursor = static_cast<int>(count) - 1;
    } else if (cursor < 0) {
        cursor = 0;
    }
}

void SortDirsThenFiles(std::vector<PaneEntry> &entries) {
    std::vector<PaneEntry> dirs, files;
    for (auto &e : entries) (e.is_dir ? dirs : files).push_back(std::move(e));

    const auto by_name = [](const PaneEntry &a, const PaneEntry &b) { return a.name < b.name; };
    std::sort(dirs.begin(), dirs.end(), by_name);
    std::sort(files.begin(), files.end(), by_name);

    entries.clear();
    for (auto &d : dirs) entries.push_back(std::move(d));
    for (auto &f : files) entries.push_back(std::move(f));
}

} // namespace

void RefreshLocalEntries(LocalPaneState &pane) {
    std::vector<PaneEntry> listed;

    std::error_code ec;
    for (const auto &entry : std::filesystem::directory_iterator(
             pane.cwd, std::filesystem::directory_options::skip_permission_denied, ec)) {
        PaneEntry pe;
        pe.name = entry.path().filename().string();
        pe.is_dir = entry.is_directory();
        if (!pe.is_dir) {
            std::error_code size_ec;
            pe.size = entry.file_size(size_ec);
        }
        listed.push_back(std::move(pe));
    }
    SortDirsThenFiles(listed);

    pane.entries.clear();
    if (pane.cwd.has_parent_path() && pane.cwd != pane.cwd.root_path()) {
        PaneEntry up;
        up.name = "..";
        up.is_dir = true;
        up.is_parent = true;
        pane.entries.push_back(up);
    }
    for (auto &e : listed) pane.entries.push_back(std::move(e));

    ClampCursor(pane.cursor, pane.entries.size());
}

void RefreshDiskEntries(DiskPaneState &pane) {
    pane.entries.clear();
    if (!pane.image.has_value()) {
        pane.cursor = 0;
        return;
    }

    if (!pane.current_dir.empty()) {
        PaneEntry up;
        up.name = "..";
        up.is_dir = true;
        up.is_parent = true;
        pane.entries.push_back(up);
    }

    std::vector<PaneEntry> listed;
    for (const auto &fe : pane.image->ListDirectory(pane.current_dir, "")) {
        PaneEntry pe;
        pe.name = fe.name;
        pe.is_dir = (fe.attr & msxdisk::kAttrDirectory) != 0;
        pe.size = fe.size;
        listed.push_back(std::move(pe));
    }
    SortDirsThenFiles(listed);
    for (auto &e : listed) pane.entries.push_back(std::move(e));

    ClampCursor(pane.cursor, pane.entries.size());
}

void NavigateLocal(LocalPaneState &pane, const PaneEntry &entry) {
    if (!entry.is_dir) return;

    pane.cwd = entry.is_parent ? pane.cwd.parent_path() : (pane.cwd / entry.name);
    pane.cursor = 0;
    RefreshLocalEntries(pane);
}

void NavigateDisk(DiskPaneState &pane, const PaneEntry &entry) {
    if (!entry.is_dir || !pane.image.has_value()) return;

    if (entry.is_parent) {
        const auto pos = pane.current_dir.find_last_of('\\');
        pane.current_dir = (pos == std::string::npos) ? "" : pane.current_dir.substr(0, pos);
    } else {
        pane.current_dir = pane.current_dir.empty() ? entry.name : (pane.current_dir + "\\" + entry.name);
    }
    pane.cursor = 0;
    RefreshDiskEntries(pane);
}

} // namespace msxdisk::tui
