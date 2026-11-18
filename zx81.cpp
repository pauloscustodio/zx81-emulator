//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "cpu.h"
#include "disasm.h"
#include "machine.h"
#include "symbols.h"
#include <iostream>

static Machine mach;
static Disassembler disasm(mach.memory());
static SymbolTable symtab;

int main() {
    mach.cpu().reset();

    Registers regs = mach.cpu().get_registers();
    std::cout << "Initial CPU state:\n" << regs.to_string() << std::endl;

    while (regs.pc != 0x04c1) {     // run until DISPLAY_6 (idle loop) is reached
        auto instr = disasm.disassemble(regs.pc);
        std::cout << instr.format(symtab) << std::endl;

        mach.cpu().step();
        regs = mach.cpu().get_registers();
    }

    std::cout << "Final CPU state:\n" << regs.to_string() << std::endl;
}
