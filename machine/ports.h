//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"

class Ports
{
public:
    byte_t read8(word_t addr) const;
    void write8(word_t addr, byte_t value);
};
