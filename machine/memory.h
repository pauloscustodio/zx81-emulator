//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"

class Memory {
public:
    Memory();
    virtual ~Memory() = default;

    byte_t fetch8(word_t addr) const;
    byte_t read8(word_t addr) const;
    word_t read16(word_t addr) const;

    void write8(word_t addr, byte_t value);
    void write16(word_t addr, word_t value);

private:
    byte_t mem_[MEM_SIZE] = { 0 };
    word_t rom_size_ = 0;
};
