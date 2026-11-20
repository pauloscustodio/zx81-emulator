//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "debugger.h"
#include "machine.h"
#include "options.h"
#include <cstdio>

// SDL.h defines its own main; remove the definition
#undef main

int main(int argc, char* argv[]) {
    // avoid that the windows subsystem buffers stdout and stderr,
    // which causes the output to be delayed
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // parse command line arguments
    parse_cmd_line(argc, argv);

    // make machine static to avoid large stack warning
    static Machine machine;
    machine.run();
}
