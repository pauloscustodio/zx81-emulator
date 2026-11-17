//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "cpu.h"
#include "machine.h"
#include <iostream>

Machine mach;

int main() {
    mach.cpu().reset();
    Registers regs = mach.cpu().get_registers();
    std::cout << "Initial CPU state:\n" << regs.to_string() << std::endl;

    mach.cpu().step();
    regs = mach.cpu().get_registers();
    std::cout << "CPU state after one step:\n" << regs.to_string() << std::endl;
}
