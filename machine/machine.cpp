//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "rom.h"
#include "screen.h"
#include "traps.h"
#include <zx81.h>

Machine::Machine()
    : cpu(memory, ports, clock, traps) {

    // init rom
    memory.add_rom(rom, sizeof(rom));

    // make DISPLAY_5 return immediatelly 
    // so that it does not execute the display file
    traps.add(DISPLAY_5, trap_return);

    // patch CDFLAG for SLOW mode
    memory.write8(CDFLAG, SlowMode);

    // reset the CPU
    cpu.reset();
}

void Machine::run() {
    Screen screen;

    // run until stopped
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        if (!paused) {
            emulate();
        }

        render(screen);
    }
}

void Machine::emulate() {
    /*
    timestamp ticks start
    while elapsed < frame-ticks
        step
    */
}

void Machine::render(Screen& screen) {
    word_t d_file = memory.read16(D_FILE);
    Registers regs = cpu.get_registers();
    word_t charset_addr = regs.i << 8;
    screen.render(memory, d_file, charset_addr);
}

void Machine::trap_return(Cpu &cpu)
{
    cpu.exec_ret();
}
