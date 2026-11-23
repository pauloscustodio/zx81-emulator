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
#include "traps.h"
#include "zx81.h"
#include <string>

struct DebuggerSnapshot {
    Registers regs;
    word_t data_addr;
    std::vector<word_t> watchpoints;
    Cycles cycles;
    Frames frames_count;
    Cycles cycles_per_frame;
    bool slow;
};

class Machine {
public:
    explicit Machine(Clock& clock);

    void run_frame();
    void key_down(kbd_state_t key);
    void key_up(kbd_state_t key);
    void paste(const std::string& text);
    void paste_file(const std::string& filename);
    void save_dump(const std::string& filename) const;
    void load_dump(const std::string& filename);

    Milliseconds frame_budget_ms() const;
    std::string get_screen_dump() const;
    const byte_t* get_frame_buffer() const;
    const byte_t* get_charset_buffer() const;
    bool is_chr128_mode() const;
    void set_master_slow_mode(bool enable);
    bool is_running() const;
    Registers get_registers() const;
    const Memory& get_memory() const;
    bool slow_mode() const;
    Cycles get_cycles_per_frame() const;
    DebuggerSnapshot get_debugger_snapshot() const;

private:
    Clock& clock;
    Memory memory;
    Keyboard keyboard;
    Ports ports;
    Printer printer;
    Traps traps;
    Cpu cpu;
    Files files;
    Frames frames_count = 0;

    // slow mode from -s option and from SLOW/FAST commands
    bool master_slow_mode = false;
    bool basic_slow_mode = false;

    // true when the machine is running, false when EXIT_HOOK is called
    bool running = true;

    // detect when ROM finished initialization
    bool init_finished = false;

    // detect PAUSE mode
    bool in_pause = false;
    Milliseconds pause_start_ms = 0;
    Milliseconds pause_duration_ms = 0;

    bool printer_active() const;
};
