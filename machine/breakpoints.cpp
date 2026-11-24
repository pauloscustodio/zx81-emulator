//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "breakpoints.h"

void Breakpoints::clear() {
    exec_breaks.reset();
    write_breaks.reset();
    read_breaks.reset();
}

bool Breakpoints::has_exec(word_t addr) const {
    return exec_breaks.test(addr);
}

bool Breakpoints::has_read(word_t addr) const {
    return read_breaks.test(addr);
}

bool Breakpoints::has_write(word_t addr) const {
    return write_breaks.test(addr);
}

void Breakpoints::add_exec(word_t addr) {
    exec_breaks.set(addr);
}

void Breakpoints::add_read(word_t addr) {
    read_breaks.set(addr);
}

void Breakpoints::add_write(word_t addr) {
    write_breaks.set(addr);
}

void Breakpoints::remove_exec(word_t addr) {
    exec_breaks.reset(addr);
}

void Breakpoints::remove_read(word_t addr) {
    read_breaks.reset(addr);
}

void Breakpoints::remove_write(word_t addr) {
    write_breaks.reset(addr);
}
