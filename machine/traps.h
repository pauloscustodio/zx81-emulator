//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"
#include <array>
#include <functional>

class Cpu;

class Machine;

class Traps {
public:
    using TrapHandler = std::function<void(Cpu&)>;

    TrapHandler find(word_t addr);
    void add(word_t addr, TrapHandler handler);

private:
    std::array<TrapHandler, MemSize> traps = { 0 };
};
