//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "debugger.h"
#include "machine.h"
#include "memory.h"
#include "zx81.h"
#include <SDL3/SDL.h>

class Screen {
public:
    explicit Screen(Clock& clock, Machine& machine, DebugServer& debug_server);
    virtual ~Screen();

    void run();

private:
    static inline constexpr int DockOverlap = 11;

    Clock& clock;
    Machine& machine;
    DebugServer& debug_server;
    bool running = true;

    // SDL window, renderer and texture for the main screen
    SDL_Window* main_window = nullptr;
    SDL_Renderer* main_renderer = nullptr;
    SDL_Texture* screen_texture = nullptr;

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

    // last contents of the charset
    byte_t last_charset[NumChars128 * 8];

    // last contents of the display file
    byte_t last_screen[NumRows][NumCols];
    bool screen_dirty = true;

    void handle_events();
    void render();
    void update_charset(const byte_t* charset_buffer, bool chr128_mode);
    void update_screen(const byte_t* frame_buffer);
    void snap_windows();
    void toggle_keyboard_overlay();
};
