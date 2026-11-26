//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "chars.h"
#include "clock.h"
#include "cpu.h"
#include "keyboard.h"
#include "machine.h"
#include "memory.h"
#include "rom.h"
#include "system.h"
#include "traps.h"
#include "utils.h"
#include "zx81.h"
#include <string>

Machine::Machine(Clock& clock)
    : clock(clock),
      memory(breakpoints),
      ports(keyboard),
      cpu(memory, ports, clock, traps, breakpoints),
      files(cpu, memory) {

    // init rom
    memory.add_rom(rom, sizeof(rom));

    // make display routines return immediatelly
    auto return_trap = [this](Cpu&) {
        cpu.exec_ret();
    };
    traps.add(DISPLAY_5, return_trap);

    // exit emulation if EXIT_HOOK is called
    traps.add(EXIT_HOOK, [this](Cpu & cpu) {
        running = false;
        cpu.exec_ret();
    });

    // dump state if DUMP_HOOK is called
    traps.add(DUMP_HOOK, [this](Cpu & cpu) {
        files.save_dump(DumpFilename.data());
        cpu.exec_ret();
    });

    // patch FAST and SLOW routines
    traps.add(FAST, [this](Cpu & cpu) {
        basic_slow_mode = false;
        cpu.exec_ret();
    });

    traps.add(SLOW, [this](Cpu & cpu) {
        basic_slow_mode = true;
        cpu.exec_ret();
    });

    basic_slow_mode = master_slow_mode;

    // detect when the ROM finished initialization
    auto init_finished_trap = [this](Cpu & cpu) {
        if (!init_finished) {
            init_finished = true;
        }
    };
    traps.add(DISPLAY_6, init_finished_trap);
    traps.add(SLOW_DISP, init_finished_trap);

    // detect PAUSE routine
    traps.add(PAUSE1, [this](Cpu & cpu) {
        Registers regs = cpu.get_registers();
        in_pause = true;
        pause_start_ms = this->clock.get_elapsed_ms();
        pause_duration_ms = INT(regs.bc) * 1000 / RefreshHz;
        cpu.exec_ret();
    });

    // SAVE and LOAD hooks
    traps.add(NEXT_PROG, [this](Cpu & cpu) {
        // LOAD routine after file name was parsed
        files.load_hook(cpu);
    });
    traps.add(SAVE1, [this](Cpu & cpu) {
        // SAVE routine after file name was parsed
        files.save_hook(cpu);
    });

    // LPRINT traps
    auto lprint_trap = [this](Cpu & cpu) {
        // LPRINT trap; A has the character to print
        if (printer_active()) {
            Registers regs = cpu.get_registers();
            byte_t ch = HIGH_BYTE(regs.af);
            printer.print(ch);
            cpu.exec_ret();
        }
    };
    traps.add(PRINT_A, lprint_trap);
    traps.add(PRINT_CH, lprint_trap);
    traps.add(PRINT_SP, lprint_trap);

    // COPY trap
    traps.add(COPY, [this](Cpu & cpu) {
        // print the screen to the printer
        std::string dump = get_screen_dump();
        printer.print(dump);
        cpu.exec_ret();
    });

    // reset the CPU
    cpu.reset();
}

Breakpoints& Machine::get_breakpoints() {
    return breakpoints;
}

Cpu& Machine::get_cpu() {
    return cpu;
}

// run a single screen-frame of emulation
void Machine::run_frame() {
    // Execute one frame worth of Z80 clock cycles.
    // In SLOW mode this matches the real ZX81 timing.
    // In FAST mode a larger value may be used to accelerate execution.
    Cycles cycles_per_frame = get_cycles_per_frame();
    Cycles start_cycles = clock.get_cycles();

    while (running && clock.get_cycles() - start_cycles < cycles_per_frame) {

        // Handle the ZX81 PAUSE command.
        //
        // PAUSE is based on wall-clock time rather than emulated CPU
        // time. This is important because the emulator may execute
        // significantly faster than real hardware, especially in FAST
        // mode. Using host time guarantees that a PAUSE of N milliseconds
        // lasts for N milliseconds of real time regardless of emulation
        // speed.
        if (in_pause) {
            Milliseconds elapsed_ms = clock.get_elapsed_ms() - pause_start_ms;

            if (elapsed_ms < pause_duration_ms) {

                Milliseconds remaining_pause_ms =
                    pause_duration_ms - elapsed_ms;

                // Convert the remaining wall-clock pause time into the
                // equivalent number of emulated clock cycles.
                Cycles remaining_pause_ticks =
                    remaining_pause_ms * ClockHz / 1000;

                // Number of cycles remaining in the current video frame.
                Cycles remaining_frame_cycles =
                    cycles_per_frame - (clock.get_cycles() - start_cycles);

                if (remaining_pause_ticks < remaining_frame_cycles) {

                    // The pause will finish before the end of the current
                    // frame. Advance the emulated clock by the equivalent
                    // paused duration, wait for the remaining wall-clock
                    // time, then resume execution within this frame.
                    clock.add_cycles(remaining_pause_ticks);
                    sleep_ms(remaining_pause_ms);
                    in_pause = false;
                }
                else {

                    // The pause extends beyond the end of this frame.
                    // Advance only to the end of the frame and leave the
                    // pause active so that the next frame will continue
                    // waiting.
                    clock.add_cycles(remaining_frame_cycles);
                    break;
                }
            }
            else {

                // Pause duration already elapsed.
                in_pause = false;
            }
        }

        // Execute the next Z80 instruction.
        // cpu.step() updates clock.ticks by the instruction duration.
        cpu.step();
    }

    Milliseconds now_ms = clock.get_elapsed_ms();
    if (now_ms > last_frame_ms + 1000 / RefreshHz) {
        last_frame_ms = now_ms;

        // decrement FRAMES
        if (get_cpu().get_state() == Cpu::State::Running) {
            word_t frames = memory.read16(FRAMES);
            frames--;
            frames |= 0x8000; // bit 15 must be set
            memory.write16(FRAMES, frames);
        }

        // scan keyboard after computer initialized
        if (init_finished) {
            keyboard.scan(clock.get_elapsed_ms(), memory);
        }

        // count one frame
        frames_count++;
    }
}

Milliseconds Machine::frame_budget_ms() const {
    if (slow_mode()) {
        return 1000 / RefreshHz;
    }

    return 0;
}

std::string Machine::get_screen_dump() const {
    std::string dump;
    word_t addr = memory.read16(D_FILE) + 1;
    for (int row = 0; row < NumRows; row++) {
        for (int col = 0; col < NumCols + 1; col++) {
            byte_t ch = memory.read8(addr++);
            dump += zx81chars[ch];
            if (ch == 0x76) {
                break;
            }
        }
    }
    return dump;
}

const byte_t* Machine::get_frame_buffer() const {
    word_t d_file = memory.read16(D_FILE);
    return memory.get_mem() + d_file;
}

const byte_t* Machine::get_charset_buffer() const {
    // bit 0 of I is 1 for CHR$128 mode, where bit 7 of the char selects a new pattern
    Registers regs = cpu.get_registers();
    word_t charset_addr = TO_WORD(regs.i & 0xfe, 0x00);
    return memory.get_mem() + charset_addr;
}

bool Machine::is_chr128_mode() const {
    // bit 0 of I is 1 for CHR$128 mode, where bit 7 of the char selects a new pattern
    Registers regs = cpu.get_registers();
    return (regs.i & 0x01) != 0;
}

void Machine::key_down(kbd_state_t key) {
    // keyboad press ends pause mode
    in_pause = false;
    keyboard.key_down(key);
}

void Machine::key_up(kbd_state_t key) {
    keyboard.key_up(key);
}

void Machine::paste(const std::string& text) {
    keyboard.paste(text);
}

void Machine::paste_file(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::cerr << "Error: open file " << filename << " failed" << std::endl;
        return;
    }

    std::string text;
    while (std::getline(ifs, text)) {
        keyboard.paste(text + "\n");
    }
}

void Machine::save_dump(const std::string& filename) const {
    files.save_dump(filename);
}

void Machine::load_dump(const std::string& filename) {
    files.load_dump(filename);
}

void Machine::set_master_slow_mode(bool enable) {
    master_slow_mode = enable;
}

bool Machine::is_running() const {
    return running;
}

const Memory& Machine::get_memory() const {
    return memory;
}

bool Machine::slow_mode() const {
    return basic_slow_mode && master_slow_mode;
}

Cycles Machine::get_cycles_per_frame() const {
    return slow_mode() ?
           ClocksPerFrameSlow :
           ClocksPerFrameFast;
}

DebuggerSnapshot Machine::get_debugger_snapshot() const {
    DebuggerSnapshot snapshot;
    snapshot.regs = cpu.get_registers();
    snapshot.cycles = clock.get_cycles();
    snapshot.frames_count = frames_count;
    snapshot.cycles_per_frame = get_cycles_per_frame();
    snapshot.slow = slow_mode();
    snapshot.cpu_state = cpu.get_state();
    snapshot.cpu_stop_reason = cpu.get_stop_reason();
    snapshot.cpu_stop_addr = cpu.get_stop_addr();
    return snapshot;
}

bool Machine::printer_active() const {
    return (memory.read8(FLAGS) & 2) != 0;
}
