//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "keyboard.h"
#include "zx81.h"

class Ports {
public:
    explicit Ports(Keyboard& keyboard);

    byte_t read8(word_t addr) const;
    void write8(word_t addr, byte_t value);

private:
    Keyboard& keyboard;
};
