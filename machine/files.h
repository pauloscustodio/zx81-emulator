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

    // save all memory and CPU state to a file
    void save_dump(const std::string& filename) const;
    void load_dump(const std::string& filename);

private:
    Cpu& cpu;
    Memory& memory;
    Keyboard& keyboard;
};