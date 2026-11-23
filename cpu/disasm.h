//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "memory.h"
#include "symbols.h"
#include "zx81.h"
#include <string>

struct AsmInstr {
    word_t addr = 0;
    word_t size = 0;
    std::string text;
};

class Disassembler {
public:
    explicit Disassembler(const Memory& memory, const SymbolTable& symtab);

    AsmInstr disassemble(word_t addr) const;
    std::string format(const AsmInstr& instr, word_t pc) const;

private:
    const Memory& memory;
    const SymbolTable& symtab;

    static byte_t read_byte(word_t addr, void* user_data);
    std::string replace_labels(const std::string& text) const;
    std::string replace_iy_references(const std::string& text) const;
};
