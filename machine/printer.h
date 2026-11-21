//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"
#include <fstream>
#include <string>

class Printer {
public:
    void print(const std::string& text);
    void print(byte_t b);

private:
    bool is_open = false;
    std::ofstream ofs;

    void open_file();
};
