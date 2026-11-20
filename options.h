//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>

struct Options {
    bool debug = false;
    bool slow = false;
};

extern Options options;
extern std::vector<std::string> args;

void parse_cmd_line(int argc, char* argv[]);
