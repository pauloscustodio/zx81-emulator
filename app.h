//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "clock.h"
#include "debugger.h"
#include "machine.h"
#include "screen.h"
#include <string>
#include <vector>

class App {
public:
    App();
    virtual ~App() = default;

    void parse_cmd_line(int argc, char* argv[]);
    void configure();
    void run();

    bool is_slow_mode() const;
    bool is_debug_mode() const;
    int get_debug_port() const;
    const std::vector<std::string>& get_args() const;

private:
    Clock clock;
    Machine machine;
    Screen screen;
    DebugServer debug_server;

    int debug_port = -1;
    bool slow_mode = false;
    std::vector<std::string> args;

    static void exit_usage();
};
