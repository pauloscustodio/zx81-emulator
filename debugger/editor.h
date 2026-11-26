//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"
#include <string>
#include <unordered_map>
#include <vector>

class EditorClient {
public:
    EditorClient();

    void add(word_t addr, const std::string& filename, int line_num);
    void navigate_to(word_t addr) const;
    void read_map_file(const std::string& filename);

private:
    struct Entry {
        size_t filename_idx;
        int line_num;
    };
    std::unordered_map<word_t, Entry> entries;
    std::vector<std::string> filenames;

    void open_file(const std::string& filename, int line_num) const;
};
