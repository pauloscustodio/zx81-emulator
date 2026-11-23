//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "chars.h"
#include "cpu.h"
#include "files.h"
#include "keyboard.h"
#include "memory.h"
#include "system.h"
#include "utils.h"
#include "zx81.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

Files::Files(Cpu& cpu, Memory& memory)
    : cpu(cpu), memory(memory) {
}

void Files::save_p(const std::string& filename) const {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Error: Unable to open file for writing: "
                  << filename << std::endl;
        return;
    }

    size_t start_addr = SAVE_ADDR;
    size_t end_addr = memory.read16(E_LINE);
    ofs.write(reinterpret_cast<const char*>(memory.get_mem() + start_addr),
              end_addr - start_addr);
}

void Files::load_p(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        std::cerr << "Error: Unable to open file for reading: "
                  << filename << std::endl;
        return;
    }

    size_t max_size = MemSize - SAVE_ADDR;
    ifs.read(reinterpret_cast<char*>(memory.get_mem() + SAVE_ADDR), max_size);
}

void Files::save_dump(const std::string& filename) const {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Error: Unable to open file for writing: "
                  << filename << std::endl;
        return;
    }

    // write 64k of memory to the file
    ofs.write(reinterpret_cast<const char*>(memory.get_mem()), memory.size());

    // write CPU registers to the file
    std::vector<byte_t> regs_bytes = cpu.get_registers().to_bytes();
    ofs.write(reinterpret_cast<const char*>(regs_bytes.data()), regs_bytes.size());
}

void Files::load_dump(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        std::cerr << "Error: Unable to open file for reading: "
                  << filename << std::endl;
        return;
    }

    // read 64k of memory from the file
    ifs.read(reinterpret_cast<char*>(memory.get_mem()), memory.size());

    // read CPU registers from the file
    std::vector<byte_t> regs_bytes(RegistersSize);
    ifs.read(reinterpret_cast<char*>(regs_bytes.data()), regs_bytes.size());
    Registers regs;
    regs.from_bytes(regs_bytes);
    cpu.set_registers(regs);
}

void Files::save_hook(Cpu& cpu) {
    // collect file name
    Registers regs = cpu.get_registers();
    word_t addr = regs.hl;  // address of the program to save
    std::string filename = read_filename(addr);

    // save the program to a .p file
    save_p(filename);

    // jump to RET
    cpu.exec_ret();
}

void Files::load_hook(Cpu& cpu) {
    Registers regs = cpu.get_registers();
    int addr = regs.de;

    bool is_load_qq = (addr & 0x8000) != 0;     // LOAD ""
    if (is_load_qq) {
        // ignore LOAD ""
    }
    else {
        // LOAD "FILE"
        std::string filename = read_filename(addr);
        std::string found_filename = find_file_case(filename);
        if (found_filename.empty()) {
            std::cerr << "Error: File not found: " << filename << std::endl;
        }
        else {
            load_p(found_filename);
        }
    }

    // jump to RET
    cpu.exec_ret();
}

std::string Files::read_string(int addr) const {
    std::string str;

    // convert to string using zx81chars
    for (int i = 0; ; i++) {
        byte_t b = memory.read8(addr + i);
        str += zx81chars[b & 0x7f];
        if ((b & 0x80) != 0) {
            break;
        }
    }

    return str;
}

std::string Files::read_filename(int addr) const {
    std::string filename = read_string(addr);
    filename = str_tolower(filename);
    filename += ".p";
    return filename;
}
