//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "debugger.h"
#include <iostream>

Debugger::Debugger(Machine& machine)
    : machine(machine) {
}

void Debugger::start() {
    running = true;
    debugger_thread = std::thread(&Debugger::run, this);
}

void Debugger::stop() {
    running = false;
    if (debugger_thread.joinable()) {
        debugger_thread.join();
    }
}

void Debugger::run() {
    std::string line;

    std::cout << WindowTitle << " - debugger" << std::endl << "> ";
    while (running && std::getline(std::cin, line)) {
        if (line == "pause")
            machine.push_command({ DebugCommandType::Pause });

        else if (line == "continue")
            machine.push_command({ DebugCommandType::Resume });

        else if (line == "step")
            machine.push_command({ DebugCommandType::Step });

        else if (line == "regs")
            machine.push_command({ DebugCommandType::Registers });

        else if (line == "quit") {
            machine.push_command({ DebugCommandType::Quit });
            running = false;
        }
        else {
            std::cout << "Invalid command: " << line << std::endl;
        }

        std::string response;
        while (machine.pop_response(response)) {
            std::cout << response;
        }

        std::cout << "> ";
    }
}
