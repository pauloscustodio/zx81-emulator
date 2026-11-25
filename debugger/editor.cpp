//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "editor.h"
#include "editor_locs.h"
#include "zx81.h"
#include <cstdlib>
#include <filesystem>

EditorClient::EditorClient() {
    for (const auto& entry : editor_locs) {
        add(entry.addr, entry.filename, entry.line_num);
    }
}

void EditorClient::add(word_t addr, const std::string& filename, int line_num) {
    Entry entry{ filename, line_num };
    entries[addr] = entry;
}

void EditorClient::navigate_to(word_t addr) const {
    auto it = entries.find(addr);
    if (it != entries.end()) {
        open_file(it->second.filename, it->second.line_num);
    }
}

void EditorClient::open_file(const std::string& filename, int line_num) const {
    static std::string last_filename;
    static int last_line_num = 0;

    if (!std::filesystem::exists(filename)) {
        return;
    }

    if (filename != last_filename || line_num != last_line_num) {
        last_filename = filename;
        last_line_num = line_num;
        // Visual Studio Code command to open a file at a specific line number
        std::string command = "code --goto \"" + filename + ":" + std::to_string(
                                  line_num) + "\"";
        std::system(command.c_str());
    }
}
