//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <string>

#define release_assert(expr) do { \
        if (!(expr)) { \
            std::cerr << "internal error: " << #expr << std::endl \
                      << "file: " << __FILE__ << std::endl \
                      << "line: " << __LINE__ << std::endl \
                      << "please report this error" << std::endl; \
        } \
    } while (0)

[[noreturn]] void die(const std::string& msg);

std::string fmt_hex(int value, int digits = 2,
                    const std::string& prefix = "$");

std::string str_tolower(std::string str);
std::string str_replace_all(std::string str, const std::string& from,
                            const std::string& to);
bool str_ends_with(const std::string& str, const std::string& ending);
bool str_starts_with(const std::string& str, const std::string& starting);
std::string str_trim(std::string str);
int str_case_cmp(const std::string& str1, const std::string& str2);

std::string get_directory(const std::string& path);
std::string get_filename(const std::string& path);
