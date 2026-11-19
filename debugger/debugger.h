//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "machine.h"
#include <atomic>
#include <thread>

class Debugger {
public:
    explicit Debugger(Machine& machine);

    void start();
    void stop();

private:
    Machine& machine;
    std::thread debugger_thread;
    std::atomic<bool> running = false;

    void run();
};