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
#include "zx81.h"
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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

    if (command == "help" || command == "h") {
        return "Available commands:\n"
               "  [h]elp   - Show this help message\n"
               "  [q]uit   - Quit the debugger\n"
               "  [r]egs   - Show CPU registers\n"
               "  [t]ype   - Type text on the machine\n"
               "  [c]ont   - Continue execution\n"
               "  [S]top   - Stop execution\n"
               "  [s]tep   - Step one instruction\n"
               "  [b]reak $xx|dd|NAME - Add breakpoint\n"
               "  [R]ead  $xx|dd|NAME - Add break on read\n"
               "  [W]rite $xx|dd|NAME - Add break on write\n"
               "  [d]el   $xx|dd|NAME - Delete breakpoints\n"
               "  [w]atch $xx|dd|NAME - Add watchpoint\n"
               "  [m]em   $xx|dd|NAME - Show memory contents\n"
               ;
    }

    if (command == "quit" || command == "q") {
        running = false;
        return "Quitting emulator";
    }

    if (command == "regs" || command == "r") {
        Registers regs = machine.get_registers();
        return regs.to_string();
    }

    if (str_starts_with(command, "type ") || str_starts_with(command, "t ")) {
        std::string text = command.substr(command.find(' ') + 1);
        machine.paste(text + "\n");
        return "Typed text: " + text;
    }

    if (command == "cont" || command == "c") {
        machine.get_cpu().set_state(Cpu::State::Running);

        // execute one instruction to avoid stopping at the same breakpoint again
        machine.get_cpu().force_step();

        return "Continue execution";
    }

    if (command == "stop" || command == "S") {
        machine.get_cpu().set_state(Cpu::State::Stopped);
        return "Stop execution";
    }

    if (command == "step" || command == "s") {
        machine.get_cpu().set_state(Cpu::State::Stopped);
        machine.get_cpu().force_step();
        return "Stop execution";
    }

    if (str_starts_with(command, "break ") || str_starts_with(command, "b ")) {
        std::string arg = command.substr(command.find(' ') + 1);
        word_t address;
        if (parse_address(arg, address)) {
            machine.get_breakpoints().add_exec(address);
            return fmt_hex(address, 4, "") + " added to breakpoints";
        }
        else {
            return "Invalid address: '" + arg + "'";
        }
    }

    if (str_starts_with(command, "read ") || str_starts_with(command, "R ")) {
        std::string arg = command.substr(command.find(' ') + 1);
        word_t address;
        if (parse_address(arg, address)) {
            machine.get_breakpoints().add_read(address);
            return fmt_hex(address, 4, "") + " added to read watchpoints";
        }
        else {
            return "Invalid address: '" + arg + "'";
        }
    }

    if (str_starts_with(command, "write ") || str_starts_with(command, "W ")) {
        std::string arg = command.substr(command.find(' ') + 1);
        word_t address;
        if (parse_address(arg, address)) {
            machine.get_breakpoints().add_write(address);
            return fmt_hex(address, 4, "") + " added to write watchpoints";
        }
        else {
            return "Invalid address: '" + arg + "'";
        }
    }

    if (str_starts_with(command, "del ") || str_starts_with(command, "d ")) {
        std::string arg = command.substr(command.find(' ') + 1);
        word_t address;
        if (parse_address(arg, address)) {
            machine.get_breakpoints().remove_exec(address);
            machine.get_breakpoints().remove_read(address);
            machine.get_breakpoints().remove_write(address);
            return fmt_hex(address, 4, "") + " deleted from breakpoints";
        }
        else {
            return "Invalid address: '" + arg + "'";
        }
    }

    if (str_starts_with(command, "watch ") || str_starts_with(command, "w ")) {
        std::string arg = command.substr(command.find(' ') + 1);
        word_t address;
        if (parse_address(arg, address)) {
            for (auto it = watchpoints.begin(); it != watchpoints.end(); ++it) {
                if (*it == address) {
                    watchpoints.erase(it);
                    break;
                }
            }
            watchpoints.insert(watchpoints.begin(), address);
            return fmt_hex(address, 4, "") + " added to watchpoints";
        }
        else {
            return "Invalid address: '" + arg + "'";
        }
    }

    if (str_starts_with(command, "mem ") || str_starts_with(command, "m ")) {
        std::string arg = command.substr(command.find(' ') + 1);
        word_t address;
        if (parse_address(arg, address)) {
            data_addr = address;
            return "Dumping memory at address: " + fmt_hex(address, 4, "");
        }
        else {
            return "Invalid address: '" + arg + "'";
        }
    }

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
    StatusScreen screen(machine.get_cpu(), memory, symtab, disasm);
    screen.update(snapshot);
    std::string status_dump = screen.get_image();
    if (status_dump != last_status_dump) {
        last_status_dump = status_dump;
        printf("%s%s", HOME, status_dump.c_str());
    }
}

bool Debugger::parse_address(std::string arg, word_t& address) const {
    arg = str_trim(arg);
    while (!arg.empty() && std::isspace(arg.front())) {
        arg.erase(arg.begin());
    }

    if (arg.empty()) {
        return false;
    }

    if (arg.front() == '$' && arg.size() > 1 && isxdigit(arg[1])) {
        // hex
        arg.erase(arg.begin());
        if (arg.empty()) {
            return false;
        }
        address = WORD(strtol(arg.c_str(), NULL, 16));
        return true;
    }
    else if (std::isdigit(arg.front())) {
        // decimal
        address = WORD(strtol(arg.c_str(), NULL, 10));
        return true;
    }
    else {
        // symbol
        const Symbol* symbol = symtab.get(arg);
        if (symbol != nullptr) {
            address = WORD(symbol->value);
            return true;
        }
    }
    return false;
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
