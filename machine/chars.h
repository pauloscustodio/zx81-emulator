//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "keyboard.h"
#include <string_view>

// convert byte to string using ZX81 character set
extern const std::string_view zx81chars[256];

// mapping of BASIC keywords to keys
enum { KL_MODE, FN_MODE, GR_MODE };

typedef struct {
    std::string_view keyword;
    int mode;
    kbd_state_t key;
} keyword_map_t;

extern const keyword_map_t keyword_map[256];
