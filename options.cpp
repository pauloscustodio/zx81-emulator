//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "getopt.h"
#include "options.h"
#include <iostream>
#include <string>
#include <vector>

Options options;
std::vector<std::string> args;

[[noreturn]] void exit_usage(void) {
    std::cerr << "Usage: zx81 [-d] [-s]" << std::endl;
    std::cerr << "Options:" << std::endl;
    //std::cerr << "  -d    Start debugger" << std::endl;
    std::cerr << "  -s    Slow mode" << std::endl;
    exit(EXIT_FAILURE);
}

void parse_cmd_line(int argc, char* argv[]) {
    while (true) {
        int c = mygetopt(argc, argv, "d:s");
        if (c == -1) {
            break;
        }
        switch (c) {
        case 'd':
            options.debug_port = std::stoi(optarg);
            break;
        case 's':
            options.slow = true;
            break;
        default:
            exit_usage();
        }
    }

    for (int i = optind; i < argc; i++) {
        args.push_back(argv[i]);
    }
}
