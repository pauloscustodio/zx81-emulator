//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "cpu.h"
#include "keyboard.h"
#include "memory.h"
#include <string>

class Files {
public:
    explicit Files(Cpu& cpu, Memory& memory, Keyboard& keyboard);

    // save and load .p files
    void save_p(const std::string& filename) const;
    void load_p(const std::string& filename);

    // save all memory and CPU state to a file
    void save_dump(const std::string& filename) const;
    void load_dump(const std::string& filename);

    // LOAD and SAVE hooks
    void save_hook(Cpu& cpu);
    void load_hook(Cpu& cpu);

    // accept a filename to be loaded on the next LOAD ""
    void set_next_load_filename(const std::string& filename);

private:
    Cpu& cpu;
    Memory& memory;
    Keyboard& keyboard;
    std::string next_load_filename;

    // read string from memory at addr until an inverted character
    std::string read_string(int addr) const;
    std::string read_filename(int addr) const;
};
