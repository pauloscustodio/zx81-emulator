//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "memory.h"
#include "rom.h"
#include "zx81.h"
#include <cstring>

Memory::Memory() {
	memcpy(mem_, rom, sizeof(rom));
	rom_size_ = sizeof(rom);
}

byte_t Memory::fetch8(word_t addr) const
{
    byte_t b = mem_[addr];
	if ((addr & 0x8000) == 0) {
		return b;
	}
	
	// fetch from high-copy of display file
	b = mem_[addr & 0x7fff];
	return (b & 0x40) != 0 ? b : 0;
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
	// don't modify ROM
	if (addr >= rom_size_) {
		mem_[addr] = value;
	}
}

void Memory::write16(word_t addr, word_t value)
{
    write8(addr, value & 0xFF);
    write8(addr + 1, (value >> 8) & 0xFF);
}
