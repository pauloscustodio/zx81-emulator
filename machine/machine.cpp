//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "options.h"
#include "rom.h"
#include "screen.h"
#include "traps.h"
#include "utils.h"
#include "zx81.h"
#include <SDL.h>

Machine::Machine()
    : ports(keyboard),
      cpu(memory, ports, clock, traps),
      files(cpu, memory) {

    // init rom
    memory.add_rom(rom, sizeof(rom));

    // make DISPLAY_5 return immediatelly
    // so that it does not execute the display file
    traps.add(DISPLAY_5, [](Cpu & cpu) {
        cpu.exec_ret();
    });

    // exit emulation if EXIT_HOOK is called
    traps.add(EXIT_HOOK, [this](Cpu & cpu) {
        running = false;
    });

    // dump state if DUMP_HOOK is called
    traps.add(DUMP_HOOK, [this](Cpu & cpu) {
        files.save_dump(DumpFilename.data());
        cpu.exec_ret();
    });

    // patch CDFLAG for SLOW mode
    memory.write8(CDFLAG, SlowMode);

    // patch FAST and SLOW routines
    traps.add(FAST, [this](Cpu & cpu) {
        memory.write8(CDFLAG, FastMode);
        cpu.exec_ret();
    });

    traps.add(SLOW, [this](Cpu & cpu) {
        memory.write8(CDFLAG, SlowMode);
        cpu.exec_ret();
    });

    // detect when the ROM finished initialization
    auto init_finished_trap = [this](Cpu & cpu) {
        if (!init_finished) {
            init_finished = true;
            init_finished_ms = SDL_GetTicks64();
        }
    };
    traps.add(DISPLAY_6, init_finished_trap);
    traps.add(SLOW_DISP, init_finished_trap);

    // detect PAUSE routine
    traps.add(PAUSE1, [this](Cpu & cpu) {
        Registers regs = cpu.get_registers();
        in_pause = true;
        pause_start_ms = SDL_GetTicks64();
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

    // reset the CPU
    cpu.reset();
}

void Machine::run() {
    // creates window
    Screen screen;

    // run until stopped
    while (running) {
        handle_events();
        run_frame();
        render(screen);
    }

    // destroys window when screen goes out of scope
}

void Machine::paste(const std::string& text) {
    keyboard.paste(text);
}

void Machine::paste_file(const std::string& filename) {
    keyboard.paste_file(filename);
}

void Machine::load_dump(const std::string& filename) {
    files.load_dump(filename);
}

void Machine::handle_events() {
    SDL_Event event;
    while (running && SDL_PollEvent(&event)) {
        bool control = (event.key.keysym.mod & (KMOD_CTRL | KMOD_GUI)) != 0;
        switch (event.type) {
        case SDL_KEYDOWN:
            // exit pause on any key press
            in_pause = false;

            // Paste
            if (control && event.key.keysym.sym == SDLK_v) {
                char* text = SDL_GetClipboardText();
                if (text) {
                    keyboard.paste(text);
                    SDL_free(text);
                }
                break;
            }

            // dump state and exit
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                files.save_dump(DumpFilename.data());
                running = false;
                break;
            }

            // dump state to file
            if (event.key.keysym.sym == SDLK_F6) {
                files.save_dump(DumpFilename.data());
                break;
            }

            // load state from file
            if (event.key.keysym.sym == SDLK_F7) {
                files.load_dump(DumpFilename.data());
                break;
            }


            // key pressed, update matrix
            keyboard.handle_sdl_event(&event);
            break;

        case SDL_KEYUP:
            // key released, update matrix
            keyboard.handle_sdl_event(&event);
            break;

        case SDL_QUIT:
            // quit the emulation
            running = false;
            break;
        }
    }
}

// run a single screen-frame of emulation
void Machine::run_frame() {
    // Measure the start of the host video frame.
    // In SLOW mode we use wall-clock synchronization so that one
    // emulated frame takes exactly 1/RefreshHz seconds in real time.
    uint64_t start_frame_ms = SDL_GetTicks64();

    // Execute one frame worth of Z80 clock cycles.
    // In SLOW mode this matches the real ZX81 timing.
    // In FAST mode a larger value may be used to accelerate execution.
    uint64_t ticks_per_frame = slow_mode() && options.slow ?
                               ClocksPerFrameSlow :
                               ClocksPerFrameFast;

    uint64_t start_ticks = clock.ticks;

    while (running && clock.ticks - start_ticks < ticks_per_frame) {

        // Handle the ZX81 PAUSE command.
        //
        // PAUSE is based on wall-clock time rather than emulated CPU
        // time. This is important because the emulator may execute
        // significantly faster than real hardware, especially in FAST
        // mode. Using host time guarantees that a PAUSE of N milliseconds
        // lasts for N milliseconds of real time regardless of emulation
        // speed.
        if (in_pause) {
            uint64_t elapsed_ms = SDL_GetTicks64() - pause_start_ms;

            if (elapsed_ms < pause_duration_ms) {

                uint64_t remaining_pause_ms =
                    pause_duration_ms - elapsed_ms;

                // Convert the remaining wall-clock pause time into the
                // equivalent number of emulated clock cycles.
                uint64_t remaining_pause_ticks =
                    remaining_pause_ms * ClockHz / 1000;

                // Number of cycles remaining in the current video frame.
                uint64_t remaining_frame_ticks =
                    ticks_per_frame - (clock.ticks - start_ticks);

                if (remaining_pause_ticks < remaining_frame_ticks) {

                    // The pause will finish before the end of the current
                    // frame. Advance the emulated clock by the equivalent
                    // paused duration, wait for the remaining wall-clock
                    // time, then resume execution within this frame.
                    clock.ticks += remaining_pause_ticks;
                    SDL_Delay64(remaining_pause_ms);
                    in_pause = false;
                }
                else {

                    // The pause extends beyond the end of this frame.
                    // Advance only to the end of the frame and leave the
                    // pause active so that the next frame will continue
                    // waiting.
                    clock.ticks += remaining_frame_ticks;
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

    // decrement FRAMES
    word_t frames = memory.read16(FRAMES);
    frames--;
    frames |= 0x8000; // bit 15 must be set
    memory.write16(FRAMES, frames);

    // scan keyboard after computer initialized
    if (init_finished) {
        keyboard.scan(memory);
    }

    // In SLOW mode, synchronize emulation to real time by delaying
    // until a complete video frame period has elapsed.
    if (slow_mode() && options.slow) {
        uint64_t elapsed_ms = SDL_GetTicks64() - start_frame_ms;

        if (elapsed_ms < 1000 / RefreshHz) {
            uint64_t delay = 1000 / RefreshHz - elapsed_ms;
            SDL_Delay64(delay);
        }
    }
}

void Machine::render(Screen& screen) {
    word_t d_file = memory.read16(D_FILE);
    Registers regs = cpu.get_registers();
    word_t charset_addr = TO_WORD(regs.i, 0x00);
    screen.render(memory, d_file, charset_addr);
}

bool Machine::slow_mode() const {
    return (memory.read8(CDFLAG) & SlowMode) != 0;
}
