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
#include "screen.h"
#include "screen.h"
#include "traps.h"
#include <zx81.h>

Machine::Machine()
    : cpu(memory, ports, clock, traps) {}

void Machine::run() {
    Screen screen;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        process_commands();

        if (!paused) {
            emulate();
        }

        render(screen);
    }
}

void Machine::push_command(DebugCommand cmd) {
    std::lock_guard<std::mutex> lock(command_mutex);
    commands.push(cmd);
}

void Machine::push_response(const std::string& response) {
    std::lock_guard<std::mutex> lock(response_mutex);
    responses.push(response);
}

bool Machine::pop_response(std::string& response) {
    std::lock_guard<std::mutex> lock(response_mutex);
    if (responses.empty()) {
        return false;
    }

    response = responses.front();
    responses.pop();
    return true;
}

void Machine::process_commands() {
    // copy commands to a local queue to keep the mutex released
    // during execution
    std::queue<DebugCommand> local_commands;
    {
        std::lock_guard<std::mutex> lock(command_mutex);
        while (!commands.empty()) {
            auto cmd = commands.front();
            commands.pop();
            local_commands.push(cmd);
        }
    }

    // mutex is released, now execute commands
    process_commands(local_commands);
}

void Machine::process_commands(std::queue<DebugCommand>& commands) {
    while (!commands.empty()) {
        auto cmd = commands.front();
        commands.pop();

        switch (cmd.type) {
        case DebugCommandType::Pause:
            paused = true;
            break;

        case DebugCommandType::Resume:
            paused = false;
            break;

        case DebugCommandType::Step:
            paused = true;
            emulate();
            break;

        case DebugCommandType::Registers:
            // Read CPU registers here.
            break;

        case DebugCommandType::Quit:
            running = false;
            break;
        }
    }
}

void Machine::emulate() {
}

void Machine::render(Screen& screen) {
    word_t d_file = memory.read16(D_FILE);
    Registers regs = cpu.get_registers();
    word_t charset_addr = regs.i << 8;
    screen.render(memory, d_file, charset_addr);
}
