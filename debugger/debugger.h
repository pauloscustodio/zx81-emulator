//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "clock.h"
#include "disasm.h"
#include "memory.h"
#include "symbols.h"
#include "system.h"
#include "zx81.h"
#include <cstdint>
#include <string>
#include <vector>

class Machine;
class DebugServer;

class Debugger {
public:
    explicit Debugger(Machine& machine, const Memory& memory,
                      DebugServer& debug_server,
                      const SymbolTable& symtab, const Disassembler& disasm);

    void poll();
    bool is_running() const;
    std::string execute_command(const std::string& command);
    void dump_status();

private:
    Machine& machine;
    const Memory& memory;
    DebugServer& debug_server;
    const SymbolTable& symtab;
    const Disassembler& disasm;
    bool running = true;
    word_t data_addr = 0;
    std::vector<word_t> watchpoints;
    std::string last_status_dump;


};

class DebugServer {
public:
    void start(uint16_t port);
    void poll(Debugger& debugger);
    void send(const std::string& text);

private:
    socket_t listen_fd = invalid_socket;
    socket_t client_fd = invalid_socket;
};

