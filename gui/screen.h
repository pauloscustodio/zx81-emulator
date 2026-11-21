//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "memory.h"
#include "sdl.h"
#include "zx81.h"

class Screen {
public:
    Screen();
    virtual ~Screen();

    void render(const Memory& memory, word_t d_file, word_t charset_addr);
    void set_dirty();
    void toggle_keyboard_overlay();
    void snap_windows();

    SDL_Window* get_main_window() const;
    SDL_Window* get_debug_window() const;
    SDL_Renderer* get_debug_renderer() const;
    SDL_Window* get_keyboard_window() const;

private:
    static inline constexpr int DockOverlap = 11;

    // SDL window, renderer and texture for the main screen
    SDL_Window* main_window = nullptr;
    SDL_Renderer* main_renderer = nullptr;
    SDL_Texture* screen_texture = nullptr;

    // debugger window and renderer
    SDL_Window* debug_window = nullptr;
    SDL_Renderer* debug_renderer = nullptr;

    // SDL window, renderer and texture for the keyboard overlay
    SDL_Window* keyboard_window = nullptr;
    SDL_Renderer* keyboard_renderer = nullptr;

    // SDL surface and texture for the keyboard overlay
    SDL_Surface* keyboard_surface = nullptr;
    SDL_Texture* keyboard_texture = nullptr;

    // zx81 screen surface and charset surface
    SDL_Surface* zx81_screen = nullptr;
    SDL_Surface* zx81_charset = nullptr;

    // is keyboard visible?
    bool keyboard_visible = false;

    // last contents of the display file
    byte_t last_screen[NumRows][NumCols];
    bool screen_dirty = true;
    word_t last_charset_addr = 0xffff;

    void update_charset(const Memory& memory, word_t charset_addr);
    void update_screen(const Memory& memory, word_t d_file);
};
