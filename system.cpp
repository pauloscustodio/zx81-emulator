//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "clock.h"
#include "system.h"
#include <chrono>
#include <thread>

void sleep_ms(Milliseconds ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
