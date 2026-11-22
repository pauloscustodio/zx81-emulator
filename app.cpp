//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "app.h"
#include "clock.h"
#include "getopt.h"
#include "machine.h"
#include "screen.h"
#include "system.h"
#include "utils.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

App::App() {
    machine = std::make_unique<Machine>(clock);
}

void App::parse_cmd_line(int argc, char* argv[]) {
    while (true) {
        int c = mygetopt(argc, argv, "d:hs");
        if (c == -1) {
            break;
        }
        switch (c) {
        case 'd':
            debug_port = std::stoi(optarg);
            break;
        case 'h':
            headless_mode = true;
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
        screen = std::make_unique<Screen>(clock, *machine, debug_server);
    }

    machine->set_master_slow_mode(slow_mode);

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
    if (debug_port != -1) {
        debug_server.start(debug_port);
    }
}

void App::run() {
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
    while (machine->is_running()) {
        Milliseconds start = clock.get_elapsed_ms();
        run_frame();
        throttle(start);
    }
}

void App::run_with_screen() {
    while (screen->is_running() && machine->is_running()) {
        Milliseconds start = clock.get_elapsed_ms();
        screen->handle_events();
        run_frame();
        screen->render();
        throttle(start);
    }
}

void App::run_frame() {
    machine->run_frame();
    debug_server.poll(*machine);
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
    std::cerr << "Usage: zx81 [-d] [-s]" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -d <port>     Start debugger listening on the specified port"
              << std::endl;
    std::cerr << "  -h            Headless mode" << std::endl;
    std::cerr << "  -s            Slow mode" << std::endl;
    exit(EXIT_FAILURE);
}
