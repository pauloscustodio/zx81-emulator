//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "app.h"
#include "clock.h"
#include "debugger.h"
#include "getopt.h"
#include "machine.h"
#include "screen.h"
#include "symbols.h"
#include "system.h"
#include "utils.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

App::App() {
    machine = std::make_unique<Machine>(clock);
    debug_server = std::make_unique<DebugServer>();
    symtab = std::make_unique<SymbolTable>();
    disasm = std::make_unique<Disassembler>(machine->get_memory(), *symtab);
    editor_client = std::make_unique<EditorClient>();
    debugger = std::make_unique<Debugger>(*machine, machine->get_memory(),
                                          *debug_server, *symtab, *disasm, *editor_client);
}

void App::parse_cmd_line(int argc, char* argv[]) {
    while (true) {
        int c = mygetopt(argc, argv, "d:ehm:s");
        if (c == -1) {
            break;
        }
        switch (c) {
        case 'd':
            debug_port = std::stoi(optarg);
            break;
        case 'e':
            editor_mode = true;
            break;
        case 'h':
            headless_mode = true;
            break;
        case 'm':
            map_filename = optarg;
            break;
        case 's':
            slow_mode = true;
            break;
        default:
            exit_usage();
        }
    }

    for (int i = optind; i < argc; i++) {
        args.push_back(argv[i]);
    }
}

void App::configure() {
    if (!headless_mode) {
        screen = std::make_unique<Screen>(*machine);
    }

    machine->set_master_slow_mode(slow_mode);
    debugger->set_editor_mode(editor_mode);
    if (!map_filename.empty()) {
        load_map_symbols(map_filename);
    }

    if (args.size() > 1) {
        std::cerr << "Error: too many arguments" << std::endl;
        exit_usage();
    }
    else if (args.size() == 1) {
        auto& filename = args[0];
        if (filename.empty()) {
            std::cerr << "Error: empty filename" << std::endl;
            exit_usage();
        }
        else if (str_ends_with(filename, ".p")) {
            // push LOAD "filename" to keyboard queue
            std::string command = "LOAD \"" +
                                  filename.substr(0, filename.size() - 2) + "\"\n";
            machine->paste(command);
        }
        else if (str_ends_with(filename, ".dump")) {
            // load dump file
            machine->load_dump(filename);
        }
        else if (str_ends_with(filename, ".bas")) {
            // push file to keyboard queue
            machine->paste_file(filename);
        }
        else {
            std::cerr << "Error: Invalid file extension: " << filename << std::endl;
            exit_usage();
        }
    }

    // start debugger server if requested
    if (is_debug_mode()) {
        debug_server->start(debug_port);
    }
}

void App::run() {
    // stop CPU if debugger is enabled, otherwise resume CPU
    if (is_debug_mode()) {
        std::cout << "Debugger server listening on port " << debug_port << std::endl;
        machine->get_cpu().set_state(Cpu::State::Stopped);
    }
    else {
        machine->get_cpu().set_state(Cpu::State::Running);
    }

    // run the main loop
    if (headless_mode) {
        run_headless();
    }
    else {
        run_with_screen();
    }
}

bool App::is_slow_mode() const {
    return slow_mode;
}

bool App::is_debug_mode() const {
    return debug_port != -1;
}

bool App::is_headless_mode() const {
    return headless_mode;
}

int App::get_debug_port() const {
    return debug_port;
}

const std::vector<std::string>& App::get_args() const {
    return args;
}

void App::run_headless() {
    while (machine->is_running() && debugger->is_running()) {
        Milliseconds start = clock.get_elapsed_ms();
        run_frame();
        throttle(start);
    }
}

void App::run_with_screen() {
    while (screen->is_running() && machine->is_running()
            && debugger->is_running()) {
        Milliseconds start = clock.get_elapsed_ms();
        screen->handle_events();
        run_frame();
        screen->render();
        throttle(start);
    }
}

void App::run_frame() {
    machine->run_frame();
    if (is_debug_mode()) {
        debugger->dump_status();
    }
    debug_server->poll(*debugger);
}

void App::throttle(Milliseconds start) {
    // In SLOW mode we use wall-clock synchronization so that one
    // emulated frame takes exactly 1/RefreshHz seconds in real time.
    Milliseconds budget = machine->frame_budget_ms();
    if (budget > 0) {
        Milliseconds elapsed = clock.get_elapsed_ms() - start;
        if (elapsed < budget) {
            sleep_ms(budget - elapsed);
        }
    }
}

void App::exit_usage() {
    std::cerr <<
              "Usage: zx81 [-d port] [-e] [-h] [-m file.map] [-s] [file.p|file.dump|file.bas]"
              << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -d <port>     Start debugger listening on the specified port"
              << std::endl;
    std::cerr << "  -e            Open editor on source line" << std::endl;
    std::cerr << "  -h            Headless mode" << std::endl;
    std::cerr << "  -m file.map   Load symbols and source locations" << std::endl;
    std::cerr << "  -s            Slow mode" << std::endl;
    exit(EXIT_FAILURE);
}

std::vector<App::MapEntry> App::read_map_file(const std::string& filename) {
    std::vector<MapEntry> entries;

    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::cerr << "open file " << filename << " failed" << std::endl;
    }

    std::string text;
    while (std::getline(ifs, text)) {
        // get symbol name
        const char* p = text.c_str();
        if (!isalpha(*p) && *p != '_') {
            continue;
        }
        const char* p0 = p;
        while (isalnum(*p) || *p == '_') {
            p++;
        }
        std::string name = std::string(p0, p);

        // skip to address
        while (isspace(*p)) {
            p++;
        }
        if (*p == '=') {
            p++;
        }
        else {
            continue;    // ignore line
        }
        while (isspace(*p)) {
            p++;
        }
        if (*p == '$') {
            p++;
        }
        else {
            continue;    // ignore line
        }

        // get address
        p0 = p;
        while (isxdigit(*p)) {
            p++;
        }
        if (p == p0) {
            continue;    // ignore line
        }
        int addr = static_cast<int>(strtol(p0, NULL, 16));

        // skip to filename
        bool matched = true;
        for (int commas = 0; commas < 5; commas++) {
            while (*p && *p != ',') {
                p++;
            }
            if (*p != ',') {
                matched = false;
                break;
            }
            p++;    // skip comma
        }
        if (!matched) {
            continue;    // ignore line
        }
        while (isspace(*p)) {
            p++;
        }

        // get filename
        p0 = p;
        while (isprint(*p) && *p != ':') {
            p++;
        }
        if (*p != ':') {
            continue;    // ignore line
        }
        std::string filename = std::string(p0, p);
        p++;    // skip colon

        // get line number
        p0 = p;
        while (isdigit(*p)) {
            p++;
        }
        if (p == p0) {
            continue;    // ignore line
        }
        int line_num = static_cast<int>(strtol(p0, NULL, 10));

        MapEntry entry{name, addr, filename, line_num};
        entries.push_back(std::move(entry));
    }

    return entries;
}

void App::load_map_symbols(const std::string& filename) {
    std::vector<MapEntry> entries = read_map_file(filename);
    for (auto& entry : entries) {
        symtab->add(entry.name, entry.value, entry.filename, entry.line_num);
        editor_client->add(entry.value, entry.filename, entry.line_num);
    }
}
