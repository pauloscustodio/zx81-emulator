//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "breakpoints.h"
#include "zx81.h"

class Memory {
public:
    explicit Memory(const Breakpoints& breakpoints);

    void add_rom(const byte_t* data, int size);
    void set_ram_limit(int limit);

    byte_t fetch8(word_t addr) const;
    byte_t read8(word_t addr) const;
    word_t read16(word_t addr) const;
    byte_t force_read8(word_t addr) const;
    word_t force_read16(word_t addr) const;

    void write8(word_t addr, byte_t value);
    void write16(word_t addr, word_t value);

    const byte_t* get_mem() const;
    byte_t* get_mem();
    size_t size() const;

    bool hit_read_breakpoint() const;
    bool hit_write_breakpoint() const;
    word_t get_breakpoint_addr() const;
    void clear_breakpoint_flags();

private:
    const Breakpoints& breakpoints;
    byte_t mem[MemSize] = { 0 };
    int rom_size = 0;
    int ram_limit = MemSize;

    // flags to indicate if a read or write breakpoint was hit during the last memory access
    mutable bool hit_read_breakpoint_ = false;
    mutable bool hit_write_breakpoint_ = false;
    mutable word_t breakpoint_addr_ = 0;

    bool is_rom(word_t addr) const;
    bool is_ram(word_t addr) const;
    bool is_video_mirror(word_t addr) const;
};
