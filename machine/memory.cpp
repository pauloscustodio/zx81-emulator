//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "memory.h"
#include "zx81.h"
#include <cstring>

Memory::Memory(const Breakpoints& breakpoints)
    : breakpoints(breakpoints)
{}

void Memory::add_rom(const byte_t* data, int size) {
    memcpy(mem, data, size);
    rom_size = size;
}

void Memory::set_ram_limit(int limit) {
    ram_limit = limit;
}

byte_t Memory::fetch8(word_t addr) const {
    if (is_video_mirror(addr)) {
        byte_t b = mem[addr & 0x7fff];
        return (b & 0x40) != 0 ? b : 0;
    }

    if (is_rom(addr) || is_ram(addr)) {
        return mem[addr];
    }

    return 0;
}

byte_t Memory::read8(word_t addr) const {
    // check for breakpoints
    if (breakpoints.has_read(addr)) {
        hit_read_breakpoint_ = true;
        breakpoint_addr_ = addr;
    }

    if (is_video_mirror(addr)) {
        return mem[addr & 0x7fff];
    }

    return force_read8(addr);
}

word_t Memory::read16(word_t addr) const {
    byte_t low = read8(addr);
    byte_t high = read8(addr + 1);
    return TO_WORD(high, low);
}

byte_t Memory::force_read8(word_t addr) const {
    if (is_rom(addr) || is_ram(addr)) {
        return mem[addr];
    }

    return 0;
}

word_t Memory::force_read16(word_t addr) const {
    byte_t low = force_read8(addr);
    byte_t high = force_read8(addr + 1);
    return TO_WORD(high, low);
}

void Memory::write8(word_t addr, byte_t value) {
    // check for breakpoints
    if (breakpoints.has_write(addr)) {
        hit_write_breakpoint_ = true;
        breakpoint_addr_ = addr;
    }

    // don't modify ROM
    if (is_ram(addr)) {
        mem[addr] = value;
    }
}

void Memory::write16(word_t addr, word_t value) {
    write8(addr, LOW_BYTE(value));
    write8(addr + 1, HIGH_BYTE(value));
}

const byte_t* Memory::get_mem() const {
    return mem;
}

byte_t* Memory::get_mem() {
    return mem;
}

size_t Memory::size() const {
    return sizeof(mem);
}

bool Memory::hit_read_breakpoint() const {
    return hit_read_breakpoint_;
}

bool Memory::hit_write_breakpoint() const {
    return hit_write_breakpoint_;
}

word_t Memory::get_breakpoint_addr() const {
    return breakpoint_addr_;
}

void Memory::clear_breakpoint_flags() {
    hit_read_breakpoint_ = false;
    hit_write_breakpoint_ = false;
    breakpoint_addr_ = 0;
}

bool Memory::is_rom(word_t addr) const {
    return INT(addr) < rom_size;
}

bool Memory::is_ram(word_t addr) const {
    return INT(addr) >= rom_size && INT(addr) < ram_limit && INT(addr) < MemSize;
}

bool Memory::is_video_mirror(word_t addr) const {
    return (addr & 0x8000) != 0 && is_ram(addr & 0x7fff);
}
