//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "disasm.h"
#include "memory.h"
#include "utils.h"
#include "z80ex_dasm.h"
#include "zx81.h"
#include <algorithm>
#include <cstdio>
#include <regex>
#include <string>
#include <vector>

Disassembler::Disassembler(const Memory& memory, const SymbolTable& symtab)
    : memory(memory), symtab(symtab) {}

AsmInstr Disassembler::disassemble(word_t addr) const {
    char buffer[64] = { 0 };
    int t_states = 0, t_states2 = 0;
    int size = z80ex_dasm(buffer, sizeof(buffer), /*flags*/0,
                          &t_states, &t_states2, read_byte, addr,
                          reinterpret_cast<void*>(const_cast<Disassembler*>(this)));

    AsmInstr instr;
    instr.addr = addr;
    instr.size = size;
    instr.text = buffer;
    return instr;
}

std::string Disassembler::format(const AsmInstr& instr, word_t pc) const {
    char buffer[64];

    // create the hex representation of the instruction bytes to the output
    snprintf(buffer, sizeof(buffer), "%04x:", instr.addr);
    std::string result = buffer;
    for (int i = 0; i < instr.size; i++) {
        snprintf(buffer, sizeof(buffer), " %02x", memory.read8(WORD(instr.addr + i)));
        result += buffer;
    }
    if (instr.size < 4) {
        result += std::string(3 * (4 - instr.size), ' ');
    }
    snprintf(buffer, sizeof(buffer), "%c", INT(instr.addr) == pc ? '>' : ' ');
    result += buffer;

    // add the label if any
    const Symbol* label = symtab.get_first(WORD(instr.addr));
    if (label == nullptr) {
        result += std::string(10, ' ');
    }
    else {
        snprintf(buffer, sizeof(buffer), "%-10.10s", label->name.c_str());
        result += buffer;
    }

    // format the instruction text: replace '#' with '$' for hex numbers, and convert to lowercase
    std::string opcode = instr.text;
    opcode = str_replace_all(opcode, "#", "$");
    opcode = str_tolower(opcode);

    // replace numbers and (iy+xx) by labels
    opcode = replace_labels(opcode);
    opcode = replace_iy_references(opcode);

    // fill with spaces and trim if too long
    snprintf(buffer, sizeof(buffer), "%-20.20s", opcode.c_str());
    opcode = buffer;

    result += opcode;
    return result;
}

byte_t Disassembler::read_byte(word_t addr, void* user_data) {
    Disassembler* self = static_cast<Disassembler*>(user_data);
    return self->memory.fetch8(addr);
}

std::string Disassembler::replace_labels(const std::string& text) const {
    // Match: $ followed by 4 hexadecimal digits
    static const std::regex pattern(R"(\$([0-9a-f]{4}))", std::regex::icase);

    std::string result;
    std::sregex_iterator it(text.begin(), text.end(), pattern);
    std::sregex_iterator end;

    size_t last_pos = 0;
    for (; it != end; ++it) {
        const std::smatch& match = *it;

        // Preserve text preceding the match
        result.append(text, last_pos, match.position() - last_pos);

        // Convert captured hexadecimal value to integer
        int value = static_cast<int>(std::stoull(match[1].str(), nullptr, 16));

        // Look up the symbol table
        const Symbol* symbol = symtab.get_first(value);
        if (symbol != nullptr) {
            // Replace with the first symbol name
            result += symbol->name;
        }
        else {
            // Leave original text unchanged
            result += match.str();
        }

        last_pos = match.position() + match.length();
    }

    // Append remaining text
    result.append(text, last_pos, std::string::npos);
    return result;
}

std::string Disassembler::replace_iy_references(const std::string& text) const {
    // Match: iy+$ followed 2 hexadecimal digits
    static const std::regex pattern(R"(iy\+\$([0-9a-f]{2}))", std::regex::icase);

    std::string result;
    std::sregex_iterator it(text.begin(), text.end(), pattern);
    std::sregex_iterator end;

    size_t last_pos = 0;
    for (; it != end; ++it) {
        const std::smatch& match = *it;

        // Preserve text preceding the match
        result.append(text, last_pos, match.position() - last_pos);

        // Convert captured hexadecimal value to integer
        int value = static_cast<int>(std::stoull(match[1].str(), nullptr, 16));

        // special case for ERRNO, as there are two symbols with the same 0x4000 value
        if (value == 0) {
            result += "iy+ERRNO-IY0";
        }
        else {
            // Look up the symbol table
            const Symbol* symbol = symtab.get_first(ERR_NO + value);
            if (symbol != nullptr) {
                // Replace with the first symbol name
                result += "iy+" + symbol->name + "-IY0";
            }
            else {
                // Leave original text unchanged
                result += match.str();
            }
        }

        last_pos = match.position() + match.length();
    }

    // Append remaining text
    result.append(text, last_pos, std::string::npos);
    return result;
}
