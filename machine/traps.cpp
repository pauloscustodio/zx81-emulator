//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "traps.h"
#include "cpu.h"

Traps::TrapHandler Traps::find(word_t addr) {
    return traps[addr];
}

void Traps::add(word_t addr, TrapHandler handler) {
    traps[addr] = handler;
}
