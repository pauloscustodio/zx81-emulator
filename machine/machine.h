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
#include "screen.h"
#include "traps.h"

class Machine {
public:
    explicit Machine();

    void run();

private:
    bool running = true;
    bool paused = false;

    Memory memory;
    Ports ports;
    Clock clock;
    Traps traps;
    Cpu cpu;

    void emulate();
    void render(Screen& screen);

    // CPU traps
    static void trap_return(Cpu& cpu);
};
