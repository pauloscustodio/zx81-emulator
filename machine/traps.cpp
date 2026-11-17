//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "traps.h"
#include "zx81.h"

Traps::TrapHandler* Traps::find(word_t addr) {
    auto it = traps_.find(addr);
    if (it != traps_.end()) {
        return &it->second;
    }
    return nullptr;
}
