//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "ports.h"

byte_t Ports::read8(word_t addr) const
{
    return 0;
}

void Ports::write8(word_t addr, byte_t value)
{
    (void)addr;
    (void)value;
}
