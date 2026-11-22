//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "system.h"
#include <cstdint>
#include <string>

class Machine;

class DebugServer {
public:
    void start(uint16_t port);
    void poll(Machine& debugger);
    void send(const std::string& text);

private:
    socket_t listen_fd = invalid_socket;
    socket_t client_fd = invalid_socket;
};
