//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "debugger.h"
#include "keymap.h"
#include "machine.h"
#include "memory.h"
#include "screen.h"
#include "system.h"
#include "utils.h"
#include "zx81.h"
#include <algorithm>
#include <SDL3_image/SDL_image.h>
#include <string>

// these macros that are defined by windows.h and cause conflicts with
// std::min and std::max
#undef min
#undef max

Screen::Screen(Clock& clock, Machine& machine, DebugServer& debug_server)
    : clock(clock), machine(machine), debug_server(debug_server) {
    // initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        die("SDL_Init failed: " + std::string(SDL_GetError()));
    }

    // create the main window and renderer
    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    main_window = SDL_CreateWindow(
                      WindowTitle.data(),
                      int(512 * scale),
                      int(384 * scale),
                      SDL_WINDOW_RESIZABLE);

    main_renderer = SDL_CreateRenderer(main_window, nullptr);
    if (main_renderer == nullptr) {
        die("Unable to create renderer: " + std::string(SDL_GetError()));
    }

    zx81_screen = SDL_CreateSurface(512, 384, SDL_PIXELFORMAT_ARGB8888);
    if (zx81_screen == nullptr) {
        die("Unable to create screen surface: " + std::string(SDL_GetError()));
    }

    screen_texture = SDL_CreateTexture(main_renderer, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_STREAMING, 512, 384);
    if (screen_texture == nullptr) {
        die("Unable to create screen texture: " + std::string(SDL_GetError()));
    }

    // read the keyboard.png file from the same directory as the executable
    const char* base = SDL_GetBasePath();
    std::string filename = std::string(base) + KeyboardFilename.data();
    keyboard_surface = IMG_Load(filename.c_str());
    if (!keyboard_surface) {
        die(SDL_GetError());
    }

    // create the keyboard overlay window, renderer and texture
    keyboard_window =
        SDL_CreateWindow(
            "ZX81 Keyboard",
            keyboard_surface->w / 4,
            keyboard_surface->h / 4,
            0);

    keyboard_renderer =
        SDL_CreateRenderer(keyboard_window, nullptr);
    if (keyboard_renderer == nullptr) {
        die("Unable to create renderer: " + std::string(SDL_GetError()));
    }

    keyboard_texture =
        SDL_CreateTextureFromSurface(keyboard_renderer, keyboard_surface);
    if (!keyboard_texture) {
        die(SDL_GetError());
    }

    SDL_HideWindow(keyboard_window);
    keyboard_visible = false;

    // move the keyboard window to below the main window
    snap_windows();

    // initialize the last_screen array to 0xff
    // so that the first update will redraw the entire screen
    memset(last_screen, 0xff, sizeof(last_screen));
    screen_dirty = true;
}

Screen::~Screen() {
    SDL_DestroySurface(zx81_screen);
    SDL_DestroySurface(zx81_charset);

    SDL_DestroyTexture(keyboard_texture);
    SDL_DestroySurface(keyboard_surface);

    SDL_DestroyRenderer(keyboard_renderer);
    SDL_DestroyWindow(keyboard_window);

    SDL_DestroyTexture(screen_texture);
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);

    SDL_Quit();
}

bool Screen::is_running() const {
    return running;
}

void Screen::handle_events() {
    SDL_Event event;
    while (running && SDL_PollEvent(&event)) {
        bool control = (event.key.mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) != 0;
        switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_MOVED:
            snap_windows();
            screen_dirty = true;
            break;

        case SDL_EVENT_KEY_DOWN:
            // Copy
            if (control && event.key.key == SDLK_C) {
                std::string dump = machine.get_screen_dump();
                SDL_SetClipboardText(dump.c_str());
                break;
            }

            // Paste
            if (control && event.key.key == SDLK_V) {
                char* text = SDL_GetClipboardText();
                if (text) {
                    machine.paste(text);
                    SDL_free(text);
                }
                break;
            }

            // dump state and exit
            if (event.key.key == SDLK_ESCAPE) {
                machine.save_dump(DumpFilename.data());
                running = false;
                break;
            }

            // dump state to file
            if (event.key.key == SDLK_F6) {
                machine.save_dump(DumpFilename.data());
                break;
            }

            // load state from file
            if (event.key.key == SDLK_F7) {
                machine.load_dump(DumpFilename.data());
                break;
            }

            // F1: toggle keyboard overlay
            if (event.key.key == SDLK_F1) {
                toggle_keyboard_overlay();
                break;
            }

            // key pressed, update matrix
            machine.key_down(sdlk2kbd(event.key.key, event.key.mod));
            break;

        case SDL_EVENT_KEY_UP:
            // key released, update matrix
            machine.key_up(sdlk2kbd(event.key.key, event.key.mod));
            break;

        case SDL_EVENT_QUIT:
            running = false;
            break;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            // quit the emulation
            if (event.window.windowID == SDL_GetWindowID(keyboard_window)) {
                toggle_keyboard_overlay();
            }
            else if (event.window.windowID == SDL_GetWindowID(main_window)) {
                running = false;
            }
            break;
        }
    }
}

void Screen::render() {
    const byte_t* frame_buffer = machine.get_frame_buffer();
    const byte_t* charset_buffer = machine.get_charset_buffer();
    bool chr128_mode = machine.is_chr128_mode();

    // update charset if content change is detected
    if (memcmp(last_charset, charset_buffer, sizeof(last_charset)) != 0) {
        memcpy(last_charset, charset_buffer, sizeof(last_charset));

        update_charset(charset_buffer, chr128_mode);

        // initialize the last_screen array to 0xff
        // so that the next update will redraw the entire screen
        memset(last_screen, 0xff, sizeof(last_screen));
        screen_dirty = true;
    }

    // update screen, detecting changes in the display file
    update_screen(frame_buffer);

    // display if dirty
    if (screen_dirty) {
        // display the main screen
        SDL_UpdateTexture(screen_texture, nullptr, zx81_screen->pixels,
                          zx81_screen->pitch);
        SDL_RenderClear(main_renderer);

        int w, h;
        SDL_GetWindowSize(main_window, &w, &h);

        // scale to multiples of 256x192
        int scale = std::max(1, std::min(w / 256, h / 192));

        SDL_FRect dest;
        dest.w = float(scale * 256);
        dest.h = float(scale * 192);
        dest.x = (w - dest.w) / 2.0f;
        dest.y = (h - dest.h) / 2.0f;

        SDL_RenderTexture(main_renderer,
                          screen_texture,
                          nullptr,
                          &dest);
        SDL_RenderPresent(main_renderer);

        // draw the keyboard overlay
        SDL_RenderClear(keyboard_renderer);
        SDL_RenderTexture(
            keyboard_renderer,
            keyboard_texture,
            nullptr,
            nullptr);
        SDL_RenderPresent(keyboard_renderer);

        screen_dirty = false;
    }

}

void Screen::update_charset(const byte_t* charset_buffer, bool chr128_mode) {
    // create a rgb surface to hold 256 8x8 characters
    SDL_Surface* charset_rgb = SDL_CreateSurface(4096, 16,
                               SDL_PIXELFORMAT_ARGB8888);
    if (charset_rgb == nullptr) {
        die("Unable to create RGB surface: " + std::string(SDL_GetError()));
    }

    // map black and white colors
    const SDL_PixelFormatDetails* format_details = SDL_GetPixelFormatDetails(
                charset_rgb->format);
    if (format_details == nullptr) {
        SDL_DestroySurface(charset_rgb);
        die("Unable to get pixel format details: " + std::string(SDL_GetError()));
    }
    uint32_t black = SDL_MapRGB(format_details, nullptr, 0, 0, 0);
    uint32_t white = SDL_MapRGB(format_details, nullptr, 255, 255, 255);

    // base points to the top-left pixel of the surface
    uint32_t* base = (uint32_t*)charset_rgb->pixels;

    // addr points to the start of the characters bits
    int addr = 0;
    int num_chars = chr128_mode ? NumChars128 : NumChars64;

    // the ammount of uint32s to add to go up/down on line
    int pitch = charset_rgb->pitch / 4;

    // create the 128 characters (64 normal + 64 blanks + 64 inverted)
    // or in CHR$128 mode (first 64 + 64 blanks + second 64)
    for (int i = 0; i < num_chars; i++) {
        int slot;
        if (!chr128_mode) {
            slot = i;
        }
        else {
            slot = (i < 64) ? i : (i + 64);
        }

        uint32_t* char_pixel = base + slot * 16;

        for (int row = 0; row < 8; row++) {
            uint32_t* pixel = char_pixel + row * pitch * 2;
            byte_t b = charset_buffer[addr++];

            for (int col = 0; col < 8; col++) {
                bool bit_set = (b & 128) != 0;
                if (chr128_mode && i >= 64) {
                    bit_set = !bit_set; // invert the second 64 characters
                }

                if (bit_set) {
                    pixel[0] = black;
                    pixel[1] = black;
                    pixel[pitch + 0] = black;
                    pixel[pitch + 1] = black;

                    if (!chr128_mode) {
                        pixel[2048] = white;
                        pixel[2049] = white;
                        pixel[pitch + 2048] = white;
                        pixel[pitch + 2049] = white;
                    }
                }
                else {
                    pixel[0] = white;
                    pixel[1] = white;
                    pixel[pitch + 0] = white;
                    pixel[pitch + 1] = white;

                    if (!chr128_mode) {
                        pixel[2048] = black;
                        pixel[2049] = black;
                        pixel[pitch + 2048] = black;
                        pixel[pitch + 2049] = black;
                    }
                }
                // advance pixel to the right
                pixel += 2;
                b <<= 1;
            }

            // advance pixel to the start of the next line
            pixel += pitch * 2 - 16;
        }
    }

    // convert the rgb surface to a known format used by the screen surface
    if (zx81_charset) {
        SDL_DestroySurface(zx81_charset);
    }
    zx81_charset = SDL_ConvertSurface(charset_rgb, SDL_PIXELFORMAT_ARGB8888);
    if (zx81_charset == nullptr) {
        SDL_DestroySurface(charset_rgb);
        die("Unable to convert charset surface: " +
            std::string(SDL_GetError()));
    }

    SDL_DestroySurface(charset_rgb);
}

void Screen::update_screen(const byte_t* frame_buffer) {
    // a pointer to the display file
    int addr = 0;

    // rects to blit from the charset to the screen
    SDL_Rect source = { 0 }, dest = { 0 };

    // counters to redraw the screen
    int row = 0, col = 0;

    // setup invariants of the rect to address characters in the charset
    source.y = 0;
    source.w = 16;
    source.h = 16;

    // redraw the screen
    dest.y = 0;

    addr++; // skip the 0x76 at the start of the display
    for (row = 0; row < NumRows; row++) {
        dest.x = 0;

        for (col = 0; col < NumCols; col++) {
            byte_t b = frame_buffer[addr];
            if (b == 0x76) {
                b = 0; // the 0x76 is a space
            }
            else {
                addr++; // advance to the next byte in the display file
            }

            if (last_screen[row][col] != b) {
                last_screen[row][col] = b;
                screen_dirty = true;

                // blit the character from the charset to the screen
                source.x = b * 16;
                SDL_BlitSurface(zx81_charset, &source, zx81_screen, &dest);
            }

            dest.x += 16;
        }

        addr++;        // skip the 0x76 at the end of the line
        dest.y += 16;
    }
}

void Screen::toggle_keyboard_overlay() {
    if (keyboard_visible) {
        SDL_HideWindow(keyboard_window);
        keyboard_visible = false;
    }
    else {
        SDL_ShowWindow(keyboard_window);
        keyboard_visible = true;
    }

    screen_dirty = true;
}

void Screen::snap_windows() {
    int main_x, main_y;
    SDL_GetWindowPosition(main_window, &main_x, &main_y);

    int main_w, main_h;
    SDL_GetWindowSize(main_window, &main_w, &main_h);

    int top, left, bottom, right;
    SDL_GetWindowBordersSize(
        main_window,
        &top,
        &left,
        &bottom,
        &right);

    SDL_SetWindowPosition(
        keyboard_window,
        main_x,
        main_y + top + +main_h + bottom - DockOverlap
    );
}

