//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "chars.h"
#include "printer.h"
#include "zx81.h"
#include <fstream>
#include <iostream>

void Printer::print(const std::string& text) {
    open_file();
    ofs << text;
    ofs.flush();
}

void Printer::print(byte_t b) {
    open_file();
    ofs << zx81chars[b];
    ofs.flush();
}

void Printer::open_file() {
    if (!is_open) {
        ofs.open(LPrintFilename.data(), std::ios::app);
        if (!ofs.is_open()) {
            std::cerr << "Error: failed to open printer output file" << std::endl;
            return;
        }
        is_open = true;
    }
}
