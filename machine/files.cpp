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
    save_data(filename, SAVE_ADDR, memory.read16(E_LINE) - SAVE_ADDR);
}

void Files::load_p(const std::string& filename) {
    load_data(filename, SAVE_ADDR);
}

void Files::save_data(const std::string& filename, int addr, int size) const {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Error: Unable to open file for writing: "
                  << filename << std::endl;
        return;
    }

    size_t start_addr = addr;
    size_t end_addr = addr + size;
    ofs.write(reinterpret_cast<const char*>(memory.get_mem() + start_addr),
              end_addr - start_addr);
}

void Files::load_data(const std::string& filename, int addr) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        std::cerr << "Error: Unable to open file for reading: "
                  << filename << std::endl;
        return;
    }

    size_t max_size = MemSize - addr;
    ifs.read(reinterpret_cast<char*>(memory.get_mem() + addr), max_size);
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
    word_t filename_addr = regs.hl;  // address of the program to save
    int data_addr = -1, data_size = -1;
    std::string filename = read_filename(filename_addr, data_addr, data_size);

    if (data_addr >= 0 && data_size >= 0) {
        // SAVE "FILE;start,length"
        save_data(filename, data_addr, data_size);
    }
    else {
        // SAVE "FILE" - save the program to a .p file
        save_p(filename);
    }

    // jump to RET
    cpu.exec_ret();
}

void Files::load_hook(Cpu& cpu) {
    Registers regs = cpu.get_registers();
    int filename_addr = regs.de;

    bool is_load_qq = (filename_addr & 0x8000) != 0;     // LOAD ""
    if (is_load_qq) {
        // ignore LOAD ""
    }
    else {
        // LOAD "FILE"
        int data_addr = -1, data_size = -1;
        std::string filename = read_filename(filename_addr, data_addr, data_size);
        std::string found_filename = find_file_case(filename);
        if (found_filename.empty()) {
            std::cerr << "Error: File not found: " << filename << std::endl;
        }
        else if (data_addr >= 0) {
            // LOAD "FILE;addr"
            load_data(filename, data_addr);
        }
        else {
            // LOAD "FILE"
            load_p(found_filename);
        }
    }

    // jump to RET
    cpu.exec_ret();
}

std::string Files::read_string(int filename_addr) const {
    std::string str;

    // convert to string using zx81chars
    for (int i = 0; ; i++) {
        byte_t b = memory.read8(filename_addr + i);
        str += zx81chars[b & 0x7f];
        if ((b & 0x80) != 0) {
            break;
        }
    }

    return str;
}

std::string Files::read_filename(int filename_addr, int& data_addr,
                                 int& size) const {
    data_addr = -1;
    size = -1;
    std::string arg = str_tolower(read_string(filename_addr));
    auto semicolon_pos = arg.find(';');
    if (semicolon_pos == std::string::npos) {
        std::string filename = arg + ".p";
        return filename;
    }
    else {
        // extract filename
        std::string filename = arg.substr(0, semicolon_pos);
        arg = arg.substr(semicolon_pos + 1);
        auto comma_pos = arg.find(",");
        if (comma_pos == std::string::npos) {
            // extract only data_addr
            data_addr = static_cast<int>(strtol(arg.c_str(), NULL, 10));
            return filename;
        }
        else {
            // extract data_addr and size
            data_addr = static_cast<int>(strtol(arg.substr(0, comma_pos).c_str(), NULL,
                                                10));
            size = static_cast<int>(strtol(arg.substr(comma_pos + 1).c_str(), NULL, 10));
            return filename;
        }
    }
}
