//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "app.h"
#include "system.h"

int main(int argc, char* argv[]) {
    init_terminal();

    App app;
    app.parse_cmd_line(argc, argv);
    app.configure();
    app.run();
}
