//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "memory.h"
#include "zx81.h"
#include <SDL.h>

class Screen {
public:
    Screen();
    virtual ~Screen();

    void render(const Memory& memory, word_t d_file, word_t charset_addr);

private:
    // SDL window and renderer
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Surface* screen = nullptr;
    SDL_Texture* screen_texture = nullptr;

    // the surface that holds the zx81 charset
    SDL_Surface* charset = nullptr;

    // last contents of the display file
    byte_t last_screen[NumRows][NumCols];
    bool screen_dirty = true;
    word_t last_charset_addr = 0xffff;

    void update_charset(const Memory& memory, word_t charset_addr);
    void update_screen(const Memory& memory, word_t d_file);
};
