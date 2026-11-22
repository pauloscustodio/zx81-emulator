//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "app.h"
#include "clock.h"
#include "getopt.h"
#include "machine.h"
#include "system.h"
#include "utils.h"
#include <cstdlib>
#include <iostream>
#include <string>

App::App()
    : machine(clock),
      screen(clock, machine, debug_server) {
}

void App::parse_cmd_line(int argc, char* argv[]) {
    while (true) {
        int c = mygetopt(argc, argv, "d:s");
        if (c == -1) {
            break;
        }
        switch (c) {
        case 'd':
            debug_port = std::stoi(optarg);
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
    machine.set_master_slow_mode(slow_mode);

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
            machine.paste(command);
        }
        else if (str_ends_with(filename, ".dump")) {
            // load dump file
            machine.load_dump(filename);
        }
        else if (str_ends_with(filename, ".bas")) {
            // push file to keyboard queue
            machine.paste_file(filename);
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
    while (screen.is_running() && machine.is_running()) {
        // Measure the start of the host video frame.
        // In SLOW mode we use wall-clock synchronization so that one
        // emulated frame takes exactly 1/RefreshHz seconds in real time.
        Milliseconds start = clock.get_elapsed_ms();

        screen.handle_events();
        machine.run_frame();
        debug_server.poll(machine);
        screen.render();

        // sleep if needed to maintain the frame rate in SLOW mode
        Milliseconds budget = machine.frame_budget_ms();
        if (budget > 0) {
            Milliseconds elapsed = clock.get_elapsed_ms() - start;
            if (elapsed < budget) {
                sleep_ms(budget - elapsed);
            }
        }
    }
}

bool App::is_slow_mode() const {
    return slow_mode;
}

bool App::is_debug_mode() const {
    return debug_port != -1;
}

int App::get_debug_port() const {
    return debug_port;
}

const std::vector<std::string>& App::get_args() const {
    return args;
}

void App::exit_usage() {
    std::cerr << "Usage: zx81 [-d] [-s]" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -d <port>     Start debugger listening on the specified port" <<
              std::endl;
    std::cerr << "  -s            Slow mode" << std::endl;
    exit(EXIT_FAILURE);
}
