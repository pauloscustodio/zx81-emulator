//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "app.h"
#include <cstdio>
#include <memory>

int main(int argc, char* argv[]) {
    // avoid that the windows subsystem buffers stdout and stderr,
    // which causes the output to be delayed
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // too large for the stack, need to allocate on the heap
    auto app = std::make_unique<App>();
    app->parse_cmd_line(argc, argv);
    app->configure();
    app->run();
}
