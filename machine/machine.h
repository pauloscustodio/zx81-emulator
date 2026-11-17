//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "cpu.h"
#include "memory.h"
#include "ports.h"
#include "clock.h"

class Machine {
public:
    Machine();

    Memory& memory() { return memory_; }
    Ports& ports() { return ports_; }
    Clock& clock() { return clock_; }
    Cpu& cpu() { return cpu_; }

private:
    Memory memory_;
    Ports ports_;
    Clock clock_;
    Cpu cpu_;
};
