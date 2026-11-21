//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "memory.h"
#include "screen.h"
#include "utils.h"
#include "zx81.h"
#include <string>

Screen::Screen() {
    // initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        die("SDL_Init failed: " + std::string(SDL_GetError()));
    }

    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    window = SDL_CreateWindow(
                 WindowTitle.data(),
                 int(512 * scale),
                 int(384 * scale),
                 SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        die("Unable to create renderer: " + std::string(SDL_GetError()));
    }

    screen = SDL_CreateSurface(512, 384, SDL_PIXELFORMAT_ARGB8888);
    if (screen == nullptr) {
        die("Unable to create screen surface: " + std::string(SDL_GetError()));
    }

    screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_STREAMING, 512, 384);
    if (screen_texture == nullptr) {
        die("Unable to create screen texture: " + std::string(SDL_GetError()));
    }

    // initialize the last_screen array to 0xff
    // so that the first update will redraw the entire screen
    memset(last_screen, 0xff, sizeof(last_screen));
    screen_dirty = true;
}

Screen::~Screen() {
    SDL_DestroySurface(charset);
    SDL_DestroyTexture(screen_texture);
    SDL_DestroySurface(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Screen::render(const Memory& memory, word_t d_file, word_t charset_addr) {
    // update  charset if address change is detected
    if (last_charset_addr != charset_addr) {

        update_charset(memory, charset_addr);
        last_charset_addr = charset_addr;

        // initialize the last_screen array to 0xff
        // so that the next update will redraw the entire screen
        memset(last_screen, 0xff, sizeof(last_screen));
        screen_dirty = true;
    }

    // update screen
    update_screen(memory, d_file);
}

// creates a sdl surface with the zx81 character set
void Screen::update_charset(const Memory& memory, word_t charset_addr) {
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
    // bit 0 of I is 1 for CHR$128 mode, where bit 7 of the char selects a new pattern
    bool is_chr128 = (charset_addr & 0x0100) != 0;
    word_t addr = charset_addr & ~0x0100;
    int num_chars = is_chr128 ? 128 : 64;

    // the ammount of uint32s to add to go up/down on line
    int pitch = charset_rgb->pitch / 4;

    // create the 128 characters (64 normal + 64 blanks + 64 inverted)
    // or in CHR$128 mode (first 64 + 64 blanks + second 64)
    for (int i = 0; i < num_chars; i++) {
        int slot;
        if (!is_chr128) {
            slot = i;
        }
        else {
            slot = (i < 64) ? i : (i + 64);
        }

        uint32_t* char_pixel = base + slot * 16;

        for (int row = 0; row < 8; row++) {
            uint32_t* pixel = char_pixel + row * pitch * 2;
            byte_t b = memory.read8(addr++);

            for (int col = 0; col < 8; col++) {
                bool bit_set = (b & 128) != 0;
                if (is_chr128 && i >= 64) {
                    bit_set = !bit_set; // invert the second 64 characters
                }

                if (bit_set) {
                    pixel[0] = black;
                    pixel[1] = black;
                    pixel[pitch + 0] = black;
                    pixel[pitch + 1] = black;

                    if (!is_chr128) {
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

                    if (!is_chr128) {
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
    if (charset) {
        SDL_DestroySurface(charset);
    }
    charset = SDL_ConvertSurface(charset_rgb, SDL_PIXELFORMAT_ARGB8888);
    if (charset == nullptr) {
        SDL_DestroySurface(charset_rgb);
        die("Unable to convert charset surface: " +
            std::string(SDL_GetError()));
    }

    SDL_DestroySurface(charset_rgb);
}

void Screen::update_screen(const Memory& memory, word_t d_file) {
    // a pointer to the display file
    word_t addr = d_file;

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
            byte_t b = memory.read8(addr);
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
                SDL_BlitSurface(charset, &source, screen, &dest);
            }

            dest.x += 16;
        }

        addr++;        // skip the 0x76 at the end of the line
        dest.y += 16;
    }

    if (screen_dirty) {
        SDL_UpdateTexture(screen_texture, nullptr, screen->pixels, screen->pitch);
        SDL_RenderClear(renderer);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        // scale to multiples of 256x192
        int scale = std::max(1, std::min(w / 256, h / 192));

        SDL_FRect dest;
        dest.w = float(scale * 256);
        dest.h = float(scale * 192);
        dest.x = (w - dest.w) / 2.0f;
        dest.y = (h - dest.h) / 2.0f;

        SDL_RenderTexture(renderer,
                          screen_texture,
                          nullptr,
                          &dest);

        SDL_RenderPresent(renderer);

        screen_dirty = false;
    }
}
