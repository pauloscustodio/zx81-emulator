//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "memory.h"
#include "zx81.h"

byte_t Memory::fetch8(word_t addr) const
{
    return mem_[addr];
}

byte_t Memory::read8(word_t addr) const
{
    return mem_[addr];
}

word_t Memory::read16(word_t addr) const
{
    return read8(addr) | (read8(addr + 1) << 8);
}

void Memory::write8(word_t addr, byte_t value)
{
    mem_[addr] = value;
}

void Memory::write16(word_t addr, word_t value)
{
    write8(addr, value & 0xFF);
    write8(addr + 1, (value >> 8) & 0xFF);
}
