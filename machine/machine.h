//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "clock.h"
#include "cpu.h"
#include "memory.h"
#include "ports.h"
#include "traps.h"

class Machine {
public:
    Machine();

    Memory& memory();
    Ports& ports();
    Clock& clock();
    Traps& traps();
    Cpu& cpu();

private:
    Memory memory_;
    Ports ports_;
    Clock clock_;
    Traps traps_;
    Cpu cpu_;
};
