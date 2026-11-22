//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "debugger.h"
#include "machine.h"
#include "utils.h"
#include <iostream>
#include <string>

void DebugServer::start(uint16_t port) {
    // setup socket server for debugger
    init_socket_runtime();

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // localhost
    addr.sin_port = htons(port);

    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 1);

    // make it non-blocking
    set_socket_non_blocking(listen_fd);

    std::cout << "Debugger server listening on port " << port << std::endl;
}

void DebugServer::poll(Machine& debugger) {
    if (listen_fd == invalid_socket) {
        return;
    }

    char buffer[1024];

    if (client_fd == invalid_socket) {
        client_fd = accept(listen_fd, nullptr, nullptr);

        if (client_fd != invalid_socket) {
            // make it non-blocking
            set_socket_non_blocking(client_fd);
            send("Welcome to the ZX81 Emulator Debugger!\n> ");
        }
    }

    if (client_fd != invalid_socket) {
        socket_size_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';

            // show what we recieved
            for (int i = 0; i < bytes_received; ++i) {
                std::cout << fmt_hex(buffer[i], 2, "") << " ";
            }
            std::cout << std::endl;

            std::string response = debugger.debugger_command(buffer);
            if (!response.empty()) {
                send(response + "\n> ");
            }
        }
        else if (bytes_received == 0) {
            std::cout << "Debugger disconnected\n";
            close_socket(client_fd);
            client_fd = invalid_socket;
        }
    }
}

void DebugServer::send(const std::string& text) {
    if (client_fd == invalid_socket) {
        return;
    }

    size_t total = 0;
    while (total < text.size()) {
        int n = ::send(
                    client_fd,
                    text.data() + total,
                    static_cast<int>(text.size() - total),
                    0);

        if (n <= 0) {
            break;
        }

        total += n;
    }
}

