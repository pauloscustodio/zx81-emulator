//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "clock.h"
#include "cpu.h"
#include "memory.h"
#include "ports.h"
#include "screen.h"
#include "traps.h"
#include <atomic>
#include <mutex>
#include <queue>
#include <string>

// debug command queue
enum class DebugCommandType {
    Pause,
    Resume,
    Step,
    Registers,
    Quit
};

struct DebugCommand {
    DebugCommandType type;
};


class Machine {
public:
    explicit Machine();

    void run();

    void push_command(DebugCommand cmd);
    void push_response(const std::string& response);
    bool pop_response(std::string& response);

private:
    std::atomic<bool> running = true;
    std::atomic<bool> paused = false;

    std::queue<DebugCommand> commands;
    std::queue<std::string> responses;

    std::mutex command_mutex;
    std::mutex response_mutex;

    Memory memory;
    Ports ports;
    Clock clock;
    Traps traps;
    Cpu cpu;

    void process_commands();
    void process_commands(std::queue<DebugCommand>& commands);
    void emulate();
    void render(Screen& screen);
};
