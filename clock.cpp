//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "clock.h"
#include <chrono>

Clock::Clock()
    : start(std::chrono::steady_clock::now()) {
}

Cycles Clock::get_cycles() const {
    return cycles;
}

void Clock::add_cycles(Cycles delta) {
    cycles += delta;
}

Frames Clock::get_frames() const {
    return frames;
}

void Clock::add_frame() {
    frames += 1;
}

Milliseconds Clock::get_elapsed_ms() const {
    auto elapsed = std::chrono::steady_clock::now() - start;

    return std::chrono::duration_cast <
           std::chrono::milliseconds > (elapsed).count();
}
