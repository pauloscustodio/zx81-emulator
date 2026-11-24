//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "clock.h"
#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "ports.h"
#include "traps.h"
#include "utils.h"
#include "z80ex.h"
#include "zx81.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static constexpr int StoppedCycles = 4;

std::string Registers::to_string() const {
    std::ostringstream oss;
    oss << "AF:  " << fmt_hex(af, 4)
        << "  BC:  " << fmt_hex(bc, 4)
        << "  DE:  " << fmt_hex(de, 4)
        << "  HL:  " << fmt_hex(hl, 4)
        << std::endl;
    oss << "AF': " << fmt_hex(af_, 4)
        << "  BC': " << fmt_hex(bc_, 4)
        << "  DE': " << fmt_hex(de_, 4)
        << "  HL': " << fmt_hex(hl_, 4)
        << std::endl;
    oss << "IX:  " << fmt_hex(ix, 4)
        << "  IY:  " << fmt_hex(iy, 4)
        << "  PC:  " << fmt_hex(pc, 4)
        << "  SP:  " << fmt_hex(sp, 4)
        << std::endl;
    oss << "I: " << fmt_hex(i, 2)
        << "  R: " << fmt_hex(r, 2)
        << "  R7: " << fmt_hex(r7, 2)
        << "  IM: " << static_cast<int>(im)
        << "  IFF1: " << static_cast<int>(iff1)
        << "  IFF2: " << static_cast<int>(iff2)
        << std::endl;
    return oss.str();
}

std::vector<byte_t> Registers::to_bytes() const {
    std::vector<byte_t> bytes;
    bytes.push_back(LOW_BYTE(af));
    bytes.push_back(HIGH_BYTE(af));
    bytes.push_back(LOW_BYTE(bc));
    bytes.push_back(HIGH_BYTE(bc));
    bytes.push_back(LOW_BYTE(de));
    bytes.push_back(HIGH_BYTE(de));
    bytes.push_back(LOW_BYTE(hl));
    bytes.push_back(HIGH_BYTE(hl));
    bytes.push_back(LOW_BYTE(af_));
    bytes.push_back(HIGH_BYTE(af_));
    bytes.push_back(LOW_BYTE(bc_));
    bytes.push_back(HIGH_BYTE(bc_));
    bytes.push_back(LOW_BYTE(de_));
    bytes.push_back(HIGH_BYTE(de_));
    bytes.push_back(LOW_BYTE(hl_));
    bytes.push_back(HIGH_BYTE(hl_));
    bytes.push_back(LOW_BYTE(ix));
    bytes.push_back(HIGH_BYTE(ix));
    bytes.push_back(LOW_BYTE(iy));
    bytes.push_back(HIGH_BYTE(iy));
    bytes.push_back(LOW_BYTE(pc));
    bytes.push_back(HIGH_BYTE(pc));
    bytes.push_back(LOW_BYTE(sp));
    bytes.push_back(HIGH_BYTE(sp));
    bytes.push_back(i);
    bytes.push_back(r);
    bytes.push_back(r7);
    bytes.push_back(im);
    bytes.push_back(iff1);
    bytes.push_back(iff2);
    return bytes;
}

void Registers::from_bytes(const std::vector<byte_t>& bytes) {
    if (bytes.size() != RegistersSize) {
        std::cerr << "Error: Invalid byte vector size for Registers" << std::endl;
        return;
    }

    size_t index = 0;
    byte_t high, low;
    low = bytes[index++];
    high = bytes[index++];
    af = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    bc = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    de = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    hl = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    af_ = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    bc_ = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    de_ = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    hl_ = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    ix = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    iy = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    pc = TO_WORD(high, low);
    low = bytes[index++];
    high = bytes[index++];
    sp = TO_WORD(high, low);
    i = bytes[index++];
    r = bytes[index++];
    r7 = bytes[index++];
    im = bytes[index++];
    iff1 = bytes[index++];
    iff2 = bytes[index++];
}

Cpu::Cpu(Memory& memory, Ports& ports, Clock& clock, Traps& traps,
         const Breakpoints& breakpoints)
    : memory(memory), ports(ports), clock(clock), traps(traps),
      breakpoints(breakpoints) {
    z80ctx = z80ex_create(mread, this,
                          mwrite, this,
                          pread, this,
                          pwrite, this,
                          nullptr, nullptr);
    if (!z80ctx) {
        die("Failed to create Z80EX context");
    }
    reset();
}

Cpu::~Cpu() {
    z80ex_destroy(static_cast<Z80EX_CONTEXT*>(z80ctx));
}

void Cpu::reset() {
    z80ex_reset(static_cast<Z80EX_CONTEXT*>(z80ctx));
    state = State::Stopped;
    stop_reason = StopReason::None;
    stop_addr = 0;
}

void Cpu::step() {
    // check if the CPU is stopped due to a breakpoint or memory access
    if (state == State::Stopped) {
        clock.add_cycles(StoppedCycles); // advance machine time while CPU is stopped
        return;
    }

    // handle traps before executing the next instruction
    Registers regs = get_registers();
    if (auto trap = traps.find(regs.pc)) {
        trap(*this);
    }

    // next handle breakpoints
    regs = get_registers(); // refresh registers after trap execution
    if (breakpoints.has_exec(regs.pc)) {
        stop(StopReason::Breakpoint, regs.pc);
        clock.add_cycles(StoppedCycles); // advance machine time while CPU is stopped
        return;
    }

    // handle memory read/write breakpoints
    if (memory.hit_write_breakpoint()) {
        stop(StopReason::MemoryWrite, memory.get_breakpoint_addr());
        memory.clear_breakpoint_flags();
        clock.add_cycles(StoppedCycles); // advance machine time while CPU is stopped
        return;
    }

    if (memory.hit_read_breakpoint()) {
        stop(StopReason::MemoryRead, memory.get_breakpoint_addr());
        memory.clear_breakpoint_flags();
        clock.add_cycles(StoppedCycles); // advance machine time while CPU is stopped
        return;
    }

    // then execute the next instruction
    force_step();
}

void Cpu::force_step() {
    // define as executable address
    word_t pc = z80ex_get_reg(static_cast<Z80EX_CONTEXT*>(z80ctx), regPC);
    start_instr.set(pc);

    // z80ex_step() executes either prefix or opcode
    // z80ex_last_op_type() verifies if the instruction is complete
    do {
        int cycles = z80ex_step(static_cast<Z80EX_CONTEXT*>(z80ctx));
        clock.add_cycles(cycles);
    }
    while (z80ex_last_op_type(static_cast<Z80EX_CONTEXT*>(z80ctx)) != 0);
}

void Cpu::exec_ret() {
    auto regs = get_registers();
    regs.pc = memory.read16(regs.sp);
    regs.sp += 2;
    set_registers(regs);
}

word_t Cpu::previous_instr(word_t addr, int n)const {
    if (n < 1) {
        return addr;
    }
    else if (n == 1) {
        // find the previous instruction by scanning backwards for an executable address
        for (int i = 1; i <= 4 && addr >= i; ++i) {
            word_t prev_addr = WORD(addr - i);
            if (start_instr.test(prev_addr)) {
                return prev_addr;
            }
        }

        // not found, return the current address
        return addr;
    }
    else {
        // recursively find the previous instruction n times
        word_t prev_addr = previous_instr(addr);
        return previous_instr(prev_addr, n - 1);
    }
}

Registers Cpu::get_registers() const {
    Registers regs;
    Z80EX_CONTEXT* cpu = static_cast<Z80EX_CONTEXT*>(z80ctx);
    regs.af = z80ex_get_reg(cpu, regAF);
    regs.bc = z80ex_get_reg(cpu, regBC);
    regs.de = z80ex_get_reg(cpu, regDE);
    regs.hl = z80ex_get_reg(cpu, regHL);
    regs.af_ = z80ex_get_reg(cpu, regAF_);
    regs.bc_ = z80ex_get_reg(cpu, regBC_);
    regs.de_ = z80ex_get_reg(cpu, regDE_);
    regs.hl_ = z80ex_get_reg(cpu, regHL_);
    regs.ix = z80ex_get_reg(cpu, regIX);
    regs.iy = z80ex_get_reg(cpu, regIY);
    regs.pc = z80ex_get_reg(cpu, regPC);
    regs.sp = z80ex_get_reg(cpu, regSP);
    regs.i = static_cast<byte_t>(z80ex_get_reg(cpu, regI));
    regs.r = static_cast<byte_t>(z80ex_get_reg(cpu, regR));
    regs.r7 = static_cast<byte_t>(z80ex_get_reg(cpu, regR7));
    regs.im = static_cast<byte_t>(z80ex_get_reg(cpu, regIM));
    regs.iff1 = static_cast<byte_t>(z80ex_get_reg(cpu, regIFF1));
    regs.iff2 = static_cast<byte_t>(z80ex_get_reg(cpu, regIFF2));
    return regs;
}

void Cpu::set_registers(const Registers& regs) {
    Z80EX_CONTEXT* cpu = static_cast<Z80EX_CONTEXT*>(z80ctx);
    z80ex_set_reg(cpu, regAF, regs.af);
    z80ex_set_reg(cpu, regBC, regs.bc);
    z80ex_set_reg(cpu, regDE, regs.de);
    z80ex_set_reg(cpu, regHL, regs.hl);
    z80ex_set_reg(cpu, regAF_, regs.af_);
    z80ex_set_reg(cpu, regBC_, regs.bc_);
    z80ex_set_reg(cpu, regDE_, regs.de_);
    z80ex_set_reg(cpu, regHL_, regs.hl_);
    z80ex_set_reg(cpu, regIX, regs.ix);
    z80ex_set_reg(cpu, regIY, regs.iy);
    z80ex_set_reg(cpu, regPC, regs.pc);
    z80ex_set_reg(cpu, regSP, regs.sp);
    z80ex_set_reg(cpu, regI, regs.i);
    z80ex_set_reg(cpu, regR, regs.r);
    z80ex_set_reg(cpu, regR7, regs.r7);
    z80ex_set_reg(cpu, regIM, regs.im);
    z80ex_set_reg(cpu, regIFF1, regs.iff1);
    z80ex_set_reg(cpu, regIFF2, regs.iff2);
}

Cpu::State Cpu::get_state() const {
    return state;
}

Cpu::StopReason Cpu::get_stop_reason() const {
    return stop_reason;
}

word_t Cpu::get_stop_addr() const {
    return stop_addr;
}

void Cpu::set_state(State new_state) {
    if (state == new_state) {
        return;
    }

    state = new_state;
    if (state == State::Running) {
        stop_reason = StopReason::None;
        stop_addr = 0;
    }
    else {
        Registers regs = get_registers();
        state = State::Stopped;
        stop_reason = StopReason::Request;
        stop_addr = regs.pc;
    }
}

void Cpu::stop(StopReason reason, word_t addr) {
    state = State::Stopped;
    stop_reason = reason;
    stop_addr = addr;
}

byte_t Cpu::mread(Z80EX_CONTEXT* cpu, word_t addr, int m1_state,
                  void* user_data) {
    Cpu* self = static_cast<Cpu*>(user_data);
    if (m1_state) {
        return self->memory.fetch8(addr);
    }
    else {
        return self->memory.read8(addr);
    }
}

void Cpu::mwrite(Z80EX_CONTEXT* cpu, word_t addr, byte_t value,
                 void* user_data) {
    Cpu* self = static_cast<Cpu*>(user_data);
    self->memory.write8(addr, value);
}

byte_t Cpu::pread(Z80EX_CONTEXT* cpu, word_t port, void* user_data) {
    Cpu* self = static_cast<Cpu*>(user_data);
    return self->ports.read8(port);
}

void Cpu::pwrite(Z80EX_CONTEXT* cpu, word_t port, byte_t value,
                 void* user_data) {
    Cpu* self = static_cast<Cpu*>(user_data);
    self->ports.write8(port, value);
}
