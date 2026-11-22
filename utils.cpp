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

std::string str_trim(std::string str) {
    while (!str.empty() && isspace(str.back())) {
        str.pop_back();
    }
    return str;
}
