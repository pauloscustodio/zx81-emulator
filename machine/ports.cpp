//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "keyboard.h"
#include "ports.h"

Ports::Ports(Keyboard& keyboard)
    : keyboard(keyboard) {
}

byte_t Ports::read8(word_t addr) const {
    // any read where the 0th bit of the port is zero reads from the keyboard
    if ((addr & 1) == 0) {
        return keyboard.in(addr);
    }

    return 0xff;
}

void Ports::write8(word_t addr, byte_t value) {
    (void)addr;
    (void)value;
}
