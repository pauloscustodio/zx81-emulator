//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "debugger.h"
#include "getopt.h"
#include "machine.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>

// SDL.h defines its own main; remove the definition
#undef main

static void exit_usage(void) {
    std::cerr << "Usage: zx81 [-d]" << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    // avoid that the windows subsystem buffers stdout and stderr,
    // which causes the output to be delayed
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // parse command line arguments
    bool debug = false;
    while (true) {
        int c = getopt(argc, argv, "d");
        if (c == -1) {
            break;
        }
        switch (c) {
        case 'd':
            debug = true;
            break;
        default:
            exit_usage();
        }
    }

    // make these static to avoid large stack warning
    static Machine machine;
    static Debugger debugger(machine);

    // start the debugger thread
    if (debug) {
        debugger.start();
    }

    // SDL and ZX81 run on the main thread.
    machine.run();

    // stop the debugger thread
    if (debug) {
        debugger.stop();
    }
}
