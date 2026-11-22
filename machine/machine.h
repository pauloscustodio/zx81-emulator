//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "clock.h"
#include "cpu.h"
#include "debugger.h"
#include "files.h"
#include "keyboard.h"
#include "memory.h"
#include "ports.h"
#include "printer.h"
#include "traps.h"
#include <string>
#include "zx81.h"

class Machine {
public:
    explicit Machine(Clock& clock);

    void run_frame();
    Milliseconds frame_budget_ms() const;
    std::string get_screen_dump() const;
    const byte_t* get_frame_buffer() const;
    const byte_t* get_charset_buffer() const;
    bool is_chr128_mode() const;
    void key_down(kbd_state_t key);
    void key_up(kbd_state_t key);
    void paste(const std::string& text);
    void paste_file(const std::string& filename);
    void save_dump(const std::string& filename) const;
    void load_dump(const std::string& filename);
    void set_master_slow_mode(bool enable);
    bool is_running() const;
    std::string debugger_command(const std::string& command);

private:
    Clock& clock;

    Memory memory;
    Keyboard keyboard;
    Ports ports;
    Printer printer;
    Traps traps;
    Cpu cpu;
    Files files;

    // detect when ROM finished initialization
    bool init_finished = false;

    // detect PAUSE mode
    bool in_pause = false;
    Milliseconds pause_start_ms = 0;
    Milliseconds pause_duration_ms = 0;

    // machine state
    bool running = true;

    // current SLOW/FAST mode based on CDFLAG
    bool slow_mode() const;

    // master slow mode based on the command line option -s
    bool master_slow_mode = false;

    bool printer_active() const;
};
