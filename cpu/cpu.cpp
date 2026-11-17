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
#include <cstdint>
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

/*read byte from memory <addr> -- called when RD & MREQ goes active.
m1_state will be 1 if M1 signal is active*/
static byte_t z80ex_mread(Z80EX_CONTEXT* cpu, word_t addr, int m1_state,
                          void* user_data) {
    Cpu* self = static_cast<Cpu*>(user_data);
    if (m1_state) {
        return self->machine().memory().fetch8(addr);
    }
    else {
        return self->machine().memory().read8(addr);
    }
}

/*write <value> to memory <addr> -- called when WR & MREQ goes active*/
static void z80ex_mwrite(Z80EX_CONTEXT* cpu, word_t addr, byte_t value,
                         void* user_data) {
    Cpu* self = static_cast<Cpu*>(user_data);
    self->machine().memory().write8(addr, value);
}

/*read byte from <port> -- called when RD & IORQ goes active*/
static byte_t z80ex_pread(Z80EX_CONTEXT* cpu, word_t port, void* user_data) {
    Cpu* self = static_cast<Cpu*>(user_data);
    return self->machine().ports().read8(port);
}

/*write <value> to <port> -- called when WR & IORQ goes active*/
static void z80ex_pwrite(Z80EX_CONTEXT* cpu, word_t port, byte_t value,
                         void* user_data) {
    Cpu* self = static_cast<Cpu*>(user_data);
    self->machine().ports().write8(port, value);
}

Cpu::Cpu(Machine& machine)
    : machine_(machine) {
    z80ctx_ = z80ex_create(z80ex_mread, this,
                           z80ex_mwrite, this,
                           z80ex_pread, this,
                           z80ex_pwrite, this,
                           nullptr, nullptr);
    if (!z80ctx_) {
        fatal_error("Failed to create Z80EX context");
    }
}

Cpu::~Cpu() {
    z80ex_destroy(static_cast<Z80EX_CONTEXT*>(z80ctx_));
}

void Cpu::reset() {
    z80ex_reset(static_cast<Z80EX_CONTEXT*>(z80ctx_));
}

void Cpu::step() {
    // handle traps before executing the next instruction
    Registers regs = get_registers();
    if (Traps::TrapHandler* trap = machine_.traps().find(regs.pc)) {
        (*trap)(machine_);
    }

    // then execute the next instruction
    int tstates = z80ex_step(static_cast<Z80EX_CONTEXT*>(z80ctx_));
    machine_.clock().tstates += tstates;
}

void Cpu::run_cycles(int cycles) {
    auto& clock = machine_.clock();
    uint64_t start_tstates = clock.tstates;
    while (clock.tstates - start_tstates < static_cast<uint64_t>(cycles)) {
        step();
    }
}

void Cpu::exec_ret() {
    auto regs = get_registers();
    regs.pc = machine_.memory().read16(regs.sp);
    regs.sp += 2;
    set_registers(regs);
}

Registers Cpu::get_registers() const {
    Registers regs;
    Z80EX_CONTEXT* cpu = static_cast<Z80EX_CONTEXT*>(z80ctx_);
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
    Z80EX_CONTEXT* cpu = static_cast<Z80EX_CONTEXT*>(z80ctx_);
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

