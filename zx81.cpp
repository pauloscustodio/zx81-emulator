//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "cpu.h"
#include "disasm.h"
#include "machine.h"
#include <iostream>

Machine mach;
Disassembler disasm(mach.memory());

int main() {
    mach.cpu().reset();
    Registers regs = mach.cpu().get_registers();
    std::cout << "Initial CPU state:\n" << regs.to_string() << std::endl;

    while (regs.pc != 0x04c1) {     // run until DISPLAY_6 (idle loop) is reached
        auto instr = disasm.disassemble(regs.pc);
        std::cout << instr.hex_data << instr.text << std::endl;

        mach.cpu().step();
        regs = mach.cpu().get_registers();
    }

    std::cout << "Final CPU state:\n" << regs.to_string() << std::endl;
}
