//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"

class Memory {
public:
    void add_rom(const byte_t* data, int size);
    void set_ram_limit(int limit);

    byte_t fetch8(word_t addr) const;
    byte_t read8(word_t addr) const;
    word_t read16(word_t addr) const;

    void write8(word_t addr, byte_t value);
    void write16(word_t addr, word_t value);

    const byte_t* get_mem() const;
    byte_t* get_mem();
    size_t size() const;

private:
    byte_t mem[MemSize] = { 0 };
    int rom_size = 0;
    int ram_limit = MemSize;

    bool is_rom(word_t addr) const;
    bool is_ram(word_t addr) const;
    bool is_video_mirror(word_t addr) const;
};
