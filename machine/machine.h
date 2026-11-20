//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "clock.h"
#include "cpu.h"
#include "keyboard.h"
#include "memory.h"
#include "ports.h"
#include "screen.h"
#include "traps.h"
#include <cstdint>

class Machine {
public:
    explicit Machine();

    void run();

private:
    Memory memory;
    Keyboard keyboard;
    Ports ports;
    Clock clock;
    Traps traps;
    Cpu cpu;

    // true while emulation is running; false to exit
    bool running = true;

    // detect when ROM finished initialization
    bool init_finished = false;
    uint64_t init_finished_ms = 0;

    // detect PAUSE mode
    bool in_pause = false;
    uint64_t pause_start_ms = 0;
    uint64_t pause_duration_ms = 0;

    void run_frame();
    void render(Screen& screen);
    bool slow_mode() const;
};
