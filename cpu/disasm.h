//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"
#include <string>

class Memory;

struct DisInstruction {
    word_t addr = 0;
    int size = 0;
    std::string text;
    std::string hex_data;
};

class Disassembler {
public:
    explicit Disassembler(const Memory& memory);
    virtual ~Disassembler() = default;

    DisInstruction disassemble(word_t addr);

private:
    const Memory& memory_;

    static byte_t read_byte(word_t addr, void* user_data);
};
