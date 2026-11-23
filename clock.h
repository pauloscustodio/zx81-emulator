//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <chrono>
#include <cstdint>

using Cycles = uint64_t;
using Milliseconds = uint64_t;
using Frames = uint64_t;

class Clock {
public:
    Clock();

    Cycles get_cycles() const;
    void add_cycles(Cycles delta);

    Milliseconds get_elapsed_ms() const;

private:
    Cycles cycles = 0;

    std::chrono::steady_clock::time_point start;
};
