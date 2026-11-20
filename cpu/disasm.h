//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "symbols.h"
#include "zx81.h"
#include <string>

class Memory;

static inline constexpr int LabelWidth = 16;

struct DisInstruction {
    int addr = 0;
    int size = 0;
    std::string text;
    std::string hex_data;

    std::string format(const SymbolTable& symtab);
};

class Disassembler {
public:
    explicit Disassembler(const Memory& memory);

    DisInstruction disassemble(int addr);

private:
    const Memory& memory_;

    static byte_t read_byte(word_t addr, void* user_data);
};
