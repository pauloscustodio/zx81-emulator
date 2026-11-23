//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "clock.h"
#include "cpu.h"
#include "debugger.h"
#include "disasm.h"
#include "machine.h"
#include "status.h"
#include "symbols.h"
#include "utils.h"
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

static inline constexpr const char* CLEAR_SCREEN = "\033[2J\033[H";
static inline constexpr const char* HOME = "\033[H";

Debugger::Debugger(Machine& machine, const Memory& memory,
                   DebugServer& debug_server,
                   const SymbolTable& symtab, const Disassembler& disasm)
    : machine(machine), memory(memory),
      debug_server(debug_server),
      symtab(symtab), disasm(disasm) {
}

void Debugger::poll() {
    debug_server.poll(*this);
}

bool Debugger::is_running() const {
    return running;
}

std::string Debugger::execute_command(const std::string& command_) {
    std::string command = str_trim(command_);
    if (command.empty()) {
        return "";
    }

    if (command == "help") {
        return "Available commands:\n"
               "  help   - Show this help message\n"
               "  quit   - Quit the debugger\n"
               "  regs   - Show CPU registers\n"
               "  type   - Type text on the machine\n"
               ;
        //"  mem    - Dump memory contents\n"
        //"  screen - Dump screen contents\n"
    }

    if (command == "quit") {
        running = false;
        return "";
    }

    if (command == "regs") {
        Registers regs = machine.get_registers();
        return regs.to_string();
    }

    if (str_starts_with(command, "type ")) {
        std::string text = command.substr(5);
        machine.paste(text + "\n");
        return "";
    }

    /*
    else if (command == "mem") {
        return memory.dump();
    }
    else if (command == "screen") {
        return get_screen_dump();
    }
    else
    */
    return "Unknown command: '" + command + "'";
}

void Debugger::dump_status() {
    static bool first_time = true;
    if (first_time) {
        printf("%s", CLEAR_SCREEN);
        first_time = false;
    }

    // get snapshop
    DebuggerSnapshot snapshot = machine.get_debugger_snapshot();
    snapshot.data_addr = data_addr;
    snapshot.watchpoints = watchpoints;

    // render it and display it
    StatusScreen screen(memory, symtab, disasm);
    screen.update(snapshot);
    std::string status_dump = screen.get_image();
    if (status_dump != last_status_dump) {
        last_status_dump = status_dump;
        printf("%s%s", HOME, status_dump.c_str());
    }
}

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

void DebugServer::poll(Debugger& debugger) {
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

            std::string response = debugger.execute_command(buffer);
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
