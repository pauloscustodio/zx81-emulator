//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "debugger.h"
#include "machine.h"
#include <cstdio>

// SDL.h defines its own main; remove the definition
#undef main

int main() {
    // avoid that the windows subsystem buffers stdout and stderr,
    // which causes the output to be delayed
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);


    // make these static to avoid large stack warning
    static Machine machine;
    static Debugger debugger(machine);

    // start the debugger thread
    debugger.start();

    // SDL and ZX81 run on the main thread.
    machine.run();

    // stop the debugger thread
    debugger.stop();

    return 0;
}
