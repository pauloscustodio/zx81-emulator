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

Cpu::Cpu(Memory& memory, Ports& ports, Clock& clock, Traps& traps)
    : memory(memory), ports(ports), clock(clock), traps(traps) {
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
}

void Cpu::step() {
    // handle traps before executing the next instruction
    Registers regs = get_registers();
    if (auto trap = traps.find(regs.pc)) {
        trap(*this);
    }

    // then execute the next instruction
    int ticks = z80ex_step(static_cast<Z80EX_CONTEXT*>(z80ctx));
    clock.ticks += ticks;
}

void Cpu::exec_ret() {
    auto regs = get_registers();
    regs.pc = memory.read16(regs.sp);
    regs.sp += 2;
    set_registers(regs);
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
