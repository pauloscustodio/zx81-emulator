//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <chrono>
#include <cstdint>

typedef uint64_t Cycles;
typedef uint64_t Milliseconds;

class Clock {
public:
    Clock();

    Cycles get_ticks() const;
    void add_ticks(Cycles delta);
    Milliseconds get_elapsed_ms() const;

private:
    Cycles ticks = 0;
    std::chrono::steady_clock::time_point start;
};
