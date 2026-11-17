//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "disasm.h"
#include "memory.h"
#include "utils.h"
#include "z80ex_dasm.h"
#include <algorithm>
#include <string>

Disassembler::Disassembler(const Memory& memory)
    : memory_(memory) {}

DisInstruction Disassembler::disassemble(word_t addr) {
    char buffer[64] = { 0 };
    int t_states = 0, t_states2 = 0;
    int size = z80ex_dasm(buffer, sizeof(buffer), /*flags*/0,
                          &t_states, &t_states2, read_byte, addr, this);

    DisInstruction instr;
    instr.addr = addr;
    instr.size = size;
    instr.text = buffer;

    // format the instruction text: replace '#' with '$' for hex numbers, and convert to lowercase
    instr.text = str_replace_all(instr.text, "#",
                                 "$"); //replace '#' with '$' for hex numbers
    instr.text = str_tolower(instr.text);

    // add the hex representation of the instruction bytes to the output
    instr.hex_data = fmt_hex(addr, 4, "") + ": ";
    for (int i = 0; i < size; ++i) {
        byte_t b = memory_.fetch8(addr + i);
        instr.hex_data += " " + fmt_hex(b, 2, "");
    }
    instr.hex_data += std::string(std::max(0, (4 - size) * 3),
                                  ' '); // pad to 4 bytes
    instr.hex_data += "  "; // space before instruction text

    return instr;
}

byte_t Disassembler::read_byte(word_t addr, void* user_data) {
    Disassembler* self = static_cast<Disassembler*>(user_data);
    return self->memory_.fetch8(addr);
}
