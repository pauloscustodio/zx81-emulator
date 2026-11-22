//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "machine.h"
#include "options.h"
#include "utils.h"
#include <cstdio>
#include <iostream>
#include <string>

// SDL.h defines its own main; remove the definition
#undef main

int main(int argc, char* argv[]) {
    static Machine machine;
    static DebugServer debug_server;

    // avoid that the windows subsystem buffers stdout and stderr,
    // which causes the output to be delayed
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // parse command line arguments
    parse_cmd_line(argc, argv);
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
            std::string command = "LOAD \"" + filename.substr(0,
                                  filename.size() - 2) + "\"\n";
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
    if (options.debug_port != -1) {
        debug_server.start(options.debug_port);
    }

    // run emulation
    machine.run(debug_server);
}
