//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>

struct Options {
    int debug_port = -1;
    bool slow = false;
};

extern Options options;
extern std::vector<std::string> args;

void parse_cmd_line(int argc, char* argv[]);
[[noreturn]] void exit_usage(void);
