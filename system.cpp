//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "clock.h"
#include "system.h"
#include "utils.h"
#include <chrono>
#include <cstring>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

void sleep_ms(Milliseconds ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::string find_file_case(const std::string& path) {
    // Split path into directory and filename
    std::string dirname = get_directory(path);
    std::string basename = get_filename(path);

#ifdef _WIN32

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA((dirname + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return "";
    }

    do {
        if (str_case_cmp(fd.cFileName, basename) == 0) {
            FindClose(h);
            return dirname + "\\" + fd.cFileName;
        }
    }
    while (FindNextFileA(h, &fd));

    FindClose(h);

#else

    DIR* dir = opendir(dirname.c_str());
    if (!dir) {
        return "";
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        if (str_case_cmp(entry->d_name, basename) == 0) {
            std::string result =
                dirname + "/" + entry->d_name;

            closedir(dir);
            return result;
        }
    }

    closedir(dir);

#endif

    return "";
}

