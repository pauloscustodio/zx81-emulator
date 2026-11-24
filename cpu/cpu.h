//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "breakpoints.h"
#include "zx81.h"
#include <bitset>
#include <string>
#include <vector>

class Memory;
class Ports;
class Clock;
class Traps;
struct _z80_cpu_context; // aka Z80EX_CONTEXT;

static constexpr int RegistersSize =
    4 * 2 + 4 * 2 + 2 * 2 + 2 * 2 + 3 + 1 + 2;
struct Registers {
    word_t af = 0xffff, bc = 0xffff, de = 0xffff, hl = 0xffff;
    word_t af_ = 0xffff, bc_ = 0xffff, de_ = 0xffff, hl_ = 0xffff;
    word_t ix = 0xffff, iy = 0xffff;
    word_t pc = 0, sp = 0xffff;
    byte_t i = 0, r = 0, r7 = 0;
    byte_t im = 0;
    byte_t iff1 = 0, iff2 = 0;

    std::string to_string() const;

    // pack in little-endian order
    std::vector<byte_t> to_bytes() const;
    void from_bytes(const std::vector<byte_t>& bytes);
};

class Cpu {
public:
    enum class State { Stopped, Running };
    enum class StopReason { None, Request, Breakpoint, MemoryRead, MemoryWrite };

    explicit Cpu(Memory& memory, Ports& ports, Clock& clock, Traps& traps,
                 const Breakpoints& breakpoints);
    virtual ~Cpu();

    void reset();
    void step();

    // ignore breakpoints and traps, for debugging
    void force_step();

    void exec_ret();

    Registers get_registers() const;
    void set_registers(const Registers& regs);

    // breakpoints
    State get_state() const;
    StopReason get_stop_reason() const;
    word_t get_stop_addr() const;

    void set_state(State new_state);

    // adds a breakpoint that is removed when reached
    void add_temp_breakpoint(word_t addr);

    word_t previous_instr(word_t addr, int n = 1) const;
    word_t next_instr(word_t addr, int n = 1) const;

private:
    Memory& memory;
    Ports& ports;
    Clock& clock;
    Traps& traps;
    const Breakpoints& breakpoints;
    void* z80ctx = nullptr;
    std::bitset<MemSize> start_instr;
    std::bitset<MemSize> temp_breakpoints;

    State state = State::Stopped;
    StopReason stop_reason = StopReason::None;
    word_t stop_addr = 0;

    void stop(StopReason reason, word_t addr);
    void step(bool check_breakpoints);

    static byte_t mread(struct _z80_cpu_context* cpu, word_t addr, int m1_state,
                        void* user_data);
    static void mwrite(struct _z80_cpu_context* cpu, word_t addr, byte_t value,
                       void* user_data);
    static byte_t pread(struct _z80_cpu_context* cpu, word_t port, void* user_data);
    static void pwrite(struct _z80_cpu_context* cpu, word_t port, byte_t value,
                       void* user_data);
};
