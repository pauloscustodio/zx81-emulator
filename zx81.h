//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>

using byte_t = uint8_t;
using word_t = uint16_t;

static inline constexpr int MEM_SIZE = 0x10000;

static inline constexpr int ERRNO = 0x4000;

static inline constexpr int LabelWidth = 16;

#if 0
#include "z80ex.h"
#include <cstdint>
#include <functional>
#include <string>

class CpuZ80 {
public:
    void reset();
    void run_cycles(int cycles);
    void step();

    Registers get_registers() const;
    void set_registers(const Registers& regs);

private:
    Z80EX_CONTEXT* cpu_;
};

class ICpu {
public:
    virtual void reset() = 0;

    virtual void run_cycles(int cycles) = 0;
    virtual void step() = 0;

    virtual Registers registers() const = 0;
    virtual void set_registers(const Registers&) = 0;

    virtual ~ICpu() = default;
};

struct DecodedInstruction {
    uint16_t address;

    uint8_t bytes[4];
    int size;

    std::string mnemonic;
    std::string operands;
};

class IDisassembler {
public:
    virtual DecodedInstruction
    decode(uint16_t addr) const = 0;
};

struct CpuEvent {
    uint16_t pc;
    uint8_t opcode;
};

using ExecuteCallback = std::function<void(const CpuEvent&)>;

class BreakpointManager {
public:
    void add(uint16_t addr);
    void remove(uint16_t addr);

    bool is_breakpoint(uint16_t addr) const;
};

enum class WatchType {
    Read,
    Write,
    ReadWrite
};

class Emulator {
public:
    uint64_t cycles() const {
        return cycles_;
    }

private:
    CpuZ80 cpu_;
    IMemory* memory_;

    uint64_t cycles_ = 0;
};

class Emulator {
    uint64_t total_cycles_ = 0;
    uint64_t frame_cycles_ = 0;
    uint64_t instructions_ = 0;
};
class Machine {
public:
    Machine()
        : cpu(memory, ports) {
    }

private:
    Memory memory;
    Ports ports;
    Cpu cpu;
};
#endif
