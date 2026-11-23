//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "utils.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

[[noreturn]]
void die(const std::string& msg) {
    std::cerr << "fatal error: " << msg << std::endl;
    exit(EXIT_FAILURE);
}

std::string fmt_hex(int value, int digits, const std::string& prefix) {
    std::ostringstream oss;
    oss << prefix << std::setw(digits) << std::setfill('0')
        << std::hex << std::nouppercase << value;
    return oss.str();
}

std::string fmt_with_commas(uint64_t value) {
    std::ostringstream oss;
    oss.imbue(std::locale(""));
    oss << std::fixed << value;
    return oss.str();
}

std::string str_tolower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](char c) {
        return std::tolower(c);
    });
    return str;
}

std::string str_replace_all(std::string str, const std::string& from,
                            const std::string& to) {
    std::string::size_type n = 0;
    while ((n = str.find(from, n)) != std::string::npos) {
        str.replace(n, from.size(), to);
        n += to.size();
    }
    return str;
}

bool str_ends_with(const std::string& str, const std::string& ending) {
    if (str.length() >= ending.length()) {
        return (0 == str.compare(str.length() - ending.length(), ending.length(),
                                 ending));
    }
    else {
        return false;
    }
}

bool str_starts_with(const std::string& str, const std::string& starting) {
    if (str.length() >= starting.length()) {
        return (0 == str.compare(0, starting.length(), starting));
    }
    else {
        return false;
    }
}

std::string str_trim(std::string str) {
    while (!str.empty() && isspace(str.back())) {
        str.pop_back();
    }
    return str;
}

int str_case_cmp(const std::string& str1, const std::string& str2) {
    std::string s1 = str_tolower(str1);
    std::string s2 = str_tolower(str2);
    if (s1 < s2) {
        return -1;
    }
    if (s1 > s2) {
        return 1;
    }
    return 0;
}

std::string get_directory(const std::string& path) {
    std::string::size_type pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(0, pos);
    }
    return ".";
}

std::string get_filename(const std::string& path) {
    std::string::size_type pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}
