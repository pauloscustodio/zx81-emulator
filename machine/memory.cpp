//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "memory.h"
#include "zx81.h"
#include <cstring>

void Memory::add_rom(const byte_t *data, int size)
{
    memcpy(mem, data, size);
    rom_size = size;
}

void Memory::set_ram_limit(int limit)
{
    ram_limit = limit;
}

byte_t Memory::fetch8(word_t addr) const
{
    if (is_video_mirror(addr)) {
        byte_t b = mem[addr & 0x7fff];
        return (b & 0x40) != 0 ? b : 0;
    }

    return read8(addr);
}

byte_t Memory::read8(word_t addr) const {
    if (is_video_mirror(addr)) {
        return mem[addr & 0x7fff];
    }

    if (is_rom(addr) || is_ram(addr)) {
        return mem[addr];
    }

    return 0;
}

word_t Memory::read16(word_t addr) const {
    return read8(addr) | (read8(addr + 1) << 8);
}

void Memory::write8(word_t addr, byte_t value) {
    // don't modify ROM
    if (is_ram(addr)) {
        mem[addr] = value;
    }
}

void Memory::write16(word_t addr, word_t value) {
    write8(addr, value & 0xFF);
    write8(addr + 1, (value >> 8) & 0xFF);
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
