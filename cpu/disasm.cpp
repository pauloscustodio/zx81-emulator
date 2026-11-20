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
#include <algorithm>
#include <regex>
#include <string>

Disassembler::Disassembler(const Memory& memory)
    : memory_(memory) {}

DisInstruction Disassembler::disassemble(int addr) {
    char buffer[64] = { 0 };
    int t_states = 0, t_states2 = 0;
    int size = z80ex_dasm(buffer, sizeof(buffer), /*flags*/0,
                          &t_states, &t_states2, read_byte, addr, this);

    DisInstruction instr;
    instr.addr = addr;
    instr.size = size;
    instr.text = buffer;

    // format the instruction text: replace '#' with '$' for hex numbers, and convert to lowercase
    instr.text = str_replace_all(instr.text, "#", "$");
    instr.text = str_tolower(instr.text);

    // add the hex representation of the instruction bytes to the output
    instr.hex_data = fmt_hex(addr, 4, "") + " ";
    for (int i = 0; i < size; ++i) {
        byte_t b = memory_.fetch8(addr + i);
        instr.hex_data += " " + fmt_hex(b, 2, "");
    }

    // pad to 4 bytes
    instr.hex_data += std::string(std::max(0, (4 - size) * 3), ' ');

    // space before instruction text
    instr.hex_data += "  ";

    return instr;
}

byte_t Disassembler::read_byte(word_t addr, void* user_data) {
    Disassembler* self = static_cast<Disassembler*>(user_data);
    return self->memory_.fetch8(addr);
}

static std::string replace_labels(const std::string& text,
                                  const SymbolTable& symtab) {
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
        std::vector<const Symbol*> symbols = symtab.get(value);
        if (!symbols.empty()) {
            // Replace with the first symbol name
            result += symbols.front()->name;
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

static std::string replace_iy_references(const std::string& text,
        const SymbolTable& symtab) {
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
            std::vector<const Symbol*> symbols = symtab.get(ERR_NO + value);
            if (!symbols.empty()) {
                // Replace with the first symbol name
                result += "iy+" + symbols.front()->name + "-IY0";
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

std::string DisInstruction::format(const SymbolTable& symtab) {
    std::vector<const Symbol*> labels = symtab.get(addr);

    // hex address and bytes
    std::string result = hex_data;

    // label if any
    if (!labels.empty()) {
        std::string label = labels.front()->name + ":";
        int padding = LabelWidth - static_cast<int>(label.size());
        label += std::string(std::max(0, padding), ' ');
        result += label;
    }
    else {
        result += std::string(LabelWidth, ' ');
    }

    // add opcode with numbers and (iy+xx) replaced by labels
    std::string opcode = replace_labels(text, symtab);
    opcode = replace_iy_references(opcode, symtab);
    result += opcode;

    return result;
}
