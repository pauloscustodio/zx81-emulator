//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string_view>

using byte_t = uint8_t;
using word_t = uint16_t;

// machine constants
static inline constexpr std::string_view WindowTitle = "ZX81 Emulator";
static inline constexpr int MEM_SIZE = 0x10000;
static inline constexpr int NumRows = 24;
static inline constexpr int NumCols = 32;

// system variables
static inline constexpr int SAVE_ADDR = 0x4009;

static inline constexpr int ERR_NO  = 0x4000;
static inline constexpr int FLAGS   = 0x4001;
static inline constexpr int ERR_SP  = 0x4002;
static inline constexpr int RAMTOP  = 0x4004;
static inline constexpr int MODE    = 0x4006;
static inline constexpr int PPC     = 0x4007;
static inline constexpr int VERSN   = 0x4009;
static inline constexpr int E_PPC   = 0x400a;
static inline constexpr int D_FILE  = 0x400c;
static inline constexpr int DF_CC   = 0x400e;
static inline constexpr int VARS    = 0x4010;
static inline constexpr int DEST    = 0x4012;
static inline constexpr int E_LINE  = 0x4014;
static inline constexpr int CH_ADD  = 0x4016;
static inline constexpr int X_PTR   = 0x4018;
static inline constexpr int STKBOT  = 0x401a;
static inline constexpr int STKEND  = 0x401c;
static inline constexpr int BREG    = 0x401e;
static inline constexpr int MEM     = 0x401f;
static inline constexpr int FREE1   = 0x4021;
static inline constexpr int DF_SZ   = 0x4022;
static inline constexpr int S_TOP   = 0x4023;
static inline constexpr int LAST_K  = 0x4025;
static inline constexpr int DBOUNCE = 0x4027;
static inline constexpr int MARGIN  = 0x4028;
static inline constexpr int NXTLIN  = 0x4029;
static inline constexpr int OLDPPC  = 0x402b;
static inline constexpr int FLAGX   = 0x402d;
static inline constexpr int STRLEN  = 0x402e;
static inline constexpr int T_ADDR  = 0x4030;
static inline constexpr int SEED    = 0x4032;
static inline constexpr int FRAMES  = 0x4034;
static inline constexpr int COORDS  = 0x4036;
static inline constexpr int COORDX  = 0x4036;
static inline constexpr int COORDY  = 0x4037;
static inline constexpr int PR_CC   = 0x4038;
static inline constexpr int S_POSN  = 0x4039;
static inline constexpr int S_POSNC = 0x4039;
static inline constexpr int S_POSNL = 0x403a;
static inline constexpr int CDFLAG  = 0x403b;
static inline constexpr int PRBUFF  = 0x403c;
static inline constexpr int MEMBOT  = 0x405d;
static inline constexpr int FREE2   = 0x407b;


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

while (running) {
    // SDL events
    processSDLEvents();

    // Debugger commands
    DebugCommand cmd;

    while (popCommand(cmd)) {
        debugger.execute(cmd);
    }

    // Emulation
    if (!debugger.isPaused()) {
        zx81.executeFrame();
    }

    // Rendering
    renderZX81();
}

void consoleThread() {
    std::string line;

    while (std::getline(std::cin, line)) {
        DebugCommand cmd = parseCommand(line);
        pushCommand(cmd);
    }
}

#endif
