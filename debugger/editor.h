//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"
#include <string>
#include <unordered_map>

class EditorClient {
public:
    EditorClient();

    void add(word_t addr, const std::string& filename, int line_num);
    void navigate_to(word_t addr) const;

private:
    struct Entry {
        std::string filename;
        int line_num;
    };
    std::unordered_map<word_t, Entry> entries;

    void open_file(const std::string& filename, int line_num) const;
};
