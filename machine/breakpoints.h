//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"
#include <bitset>

class Breakpoints {
public:
    void clear();

    bool has_exec(word_t addr) const;
    bool has_read(word_t addr) const;
    bool has_write(word_t addr) const;

    void add_exec(word_t addr);
    void add_read(word_t addr);
    void add_write(word_t addr);

    void remove_exec(word_t addr);
    void remove_read(word_t addr);
    void remove_write(word_t addr);

private:
    std::bitset<MemSize> exec_breaks;
    std::bitset<MemSize> write_breaks;
    std::bitset<MemSize> read_breaks;
};