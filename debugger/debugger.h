//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef _WIN32
using socket_t = SOCKET;
using socket_size_t = int;
constexpr socket_t invalid_socket = INVALID_SOCKET;
#else
using socket_t = int;
using socket_size_t = ssize_t;
constexpr socket_t invalid_socket = -1;
#endif

class IDebugger {
public:
    virtual ~IDebugger() = default;
    virtual std::string debugger_command(const std::string& command) = 0;
};

class DebugServer {
public:
    void start(uint16_t port);
    void poll(IDebugger& debugger);
    void send(const std::string& text);

private:
    socket_t listen_fd = invalid_socket;
    socket_t client_fd = invalid_socket;

    void make_socket_non_blocking(socket_t sock);
};
