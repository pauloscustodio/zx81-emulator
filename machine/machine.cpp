//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "machine.h"
#include "memory.h"
#include "ports.h"
#include "clock.h"
#include "traps.h"
#include "cpu.h"

Machine::Machine()
    : cpu_(memory_, ports_, clock_, traps_) {}

Memory& Machine::memory() {
    return memory_;
}

Ports& Machine::ports() {
    return ports_;
}

Clock& Machine::clock() {
    return clock_;
}

Traps& Machine::traps() {
    return traps_;
}

Cpu& Machine::cpu() {
    return cpu_;
}
