//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "cpu.h"
#include "files.h"
#include "keyboard.h"
#include "memory.h"
#include "zx81.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

Files::Files(Cpu& cpu, Memory& memory, Keyboard& keyboard)
    : cpu(cpu), memory(memory), keyboard(keyboard) {
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
