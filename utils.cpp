//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "utils.h"
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

[[noreturn]]
void fatal_error(const char* msg) {
    std::cerr << "fatal error: " << msg << std::endl;
    exit(EXIT_FAILURE);
}

std::string fmt_hex(int value, int digits, const std::string& prefix) {
    std::ostringstream oss;
    oss << prefix << std::setw(digits) << std::setfill('0')
        << std::hex << std::uppercase << value;
    return oss.str();
}

