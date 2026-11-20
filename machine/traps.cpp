//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "traps.h"
#include "cpu.h"

Traps::TrapHandler Traps::find(word_t addr)
{
    auto it = traps.find(addr);
    if (it != traps.end()) {
        return it->second;
    }
    return nullptr;
}

void Traps::add(word_t addr, TrapHandler handler)
{
    traps[addr] = handler;
}
