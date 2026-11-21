//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "clock.h"
#include "cpu.h"
#include "files.h"
#include "keyboard.h"
#include "memory.h"
#include "ports.h"
#include "printer.h"
#include "screen.h"
#include "traps.h"
#include <cstdint>
#include <string>

class Machine {
public:
    explicit Machine();

    void run();
    void paste(const std::string& text);
    void paste_file(const std::string& filename);
    void load_dump(const std::string& filename);

private:
    Memory memory;
    Keyboard keyboard;
    Ports ports;
    Printer printer;
    Clock clock;
    Traps traps;
    Cpu cpu;
    Files files;

    // true while emulation is running; false to exit
    bool running = true;

    // detect when ROM finished initialization
    bool init_finished = false;
    uint64_t init_finished_ms = 0;

    // detect PAUSE mode
    bool in_pause = false;
    uint64_t pause_start_ms = 0;
    uint64_t pause_duration_ms = 0;

    void handle_events();
    void run_frame();
    void render(Screen& screen);
    bool slow_mode() const;
    bool printer_active() const;
};
