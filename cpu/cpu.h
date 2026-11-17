//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"
#include <string>

class Machine;

struct Registers
{
    word_t af=0xffff, bc= 0xffff, de= 0xffff, hl= 0xffff;
    word_t af_= 0xffff, bc_= 0xffff, de_= 0xffff, hl_= 0xffff;
    word_t ix= 0xffff, iy= 0xffff;
    word_t pc=0, sp= 0xffff;
    byte_t i=0, r=0, r7=0;
    byte_t im=0;
    byte_t iff1=0, iff2=0;

    std::string to_string() const;
};

class Cpu {
public:
    Cpu(Machine &machine);
    virtual ~Cpu();
    Cpu(const Cpu&) = delete;
    Cpu& operator=(const Cpu&) = delete;

    void reset();
    void step();
    void run_cycles(int cycles);
    void exec_ret();

    Registers get_registers() const;
    void set_registers(const Registers &regs);

    Machine& machine() { return machine_; }

private:
    Machine &machine_;
    void* z80ctx_;
};
