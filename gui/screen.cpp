//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "memory.h"
#include "screen.h"
#include "sdl.h"
#include "zx81.h"
#include <string>

Screen::Screen() {
    // initialize SDL
    xSDL_Init(SDL_INIT_VIDEO);

    // create the main window and renderer
    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    main_window = SDL_CreateWindow(
                      "ZX81 Emulator",
                      int(512 * scale),
                      int(384 * scale),
                      SDL_WINDOW_RESIZABLE);
    main_renderer = xSDL_CreateRenderer(main_window, nullptr);
    zx81_screen = xSDL_CreateSurface(512, 384, SDL_PIXELFORMAT_ARGB8888);
    screen_texture = xSDL_CreateTexture(main_renderer, SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING, 512, 384);

    // read the keyboard.png file from the same directory as the executable
    const char* base = SDL_GetBasePath();
    std::string filename = std::string(base) + KeyboardFilename.data();
    keyboard_surface = xIMG_Load(filename.c_str());

    // create the keyboard overlay window, renderer and texture
    keyboard_window = SDL_CreateWindow(
                          "ZX81 Keyboard",
                          keyboard_surface->w / 4,
                          keyboard_surface->h / 4,
                          0);
    keyboard_renderer = xSDL_CreateRenderer(keyboard_window, nullptr);
    keyboard_texture = xSDL_CreateTextureFromSurface(keyboard_renderer,
                       keyboard_surface);

    SDL_HideWindow(keyboard_window);
    keyboard_visible = false;

    // move the keyboard window to below the main window
    snap_windows();

    // create the debugger window
    debug_window = SDL_CreateWindow(
                       "ZX81 Debugger",
                       int(512 * scale),
                       int(384 * scale),
                       SDL_WINDOW_RESIZABLE);
    debug_renderer = xSDL_CreateRenderer(debug_window, nullptr);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(debug_window, debug_renderer);
    ImGui_ImplSDLRenderer3_Init(debug_renderer);

    // initialize the last_screen array to 0xff
    // so that the first update will redraw the entire screen
    memset(last_screen, 0xff, sizeof(last_screen));
    screen_dirty = true;
}

Screen::~Screen() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroySurface(zx81_screen);
    SDL_DestroySurface(zx81_charset);

    SDL_DestroyTexture(keyboard_texture);
    SDL_DestroySurface(keyboard_surface);

    SDL_DestroyRenderer(keyboard_renderer);
    SDL_DestroyWindow(keyboard_window);

    SDL_DestroyRenderer(debug_renderer);
    SDL_DestroyWindow(debug_window);

    SDL_DestroyTexture(screen_texture);
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);

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
        xSDL_RenderTexture(main_renderer,
                           screen_texture,
                           nullptr,
                           &dest);
        SDL_RenderPresent(main_renderer);

        // draw the keyboard overlay
        SDL_RenderClear(keyboard_renderer);
        xSDL_RenderTexture(
            keyboard_renderer,
            keyboard_texture,
            nullptr,
            nullptr);
        SDL_RenderPresent(keyboard_renderer);

        screen_dirty = false;
    }
}

void Screen::set_dirty() {
    screen_dirty = true;
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
        main_y + top + main_h + bottom - DockOverlap
    );

    SDL_SetWindowPosition(
        debug_window,
        main_x + right + main_w + right - DockOverlap,
        main_y
    );
}

SDL_Window* Screen::get_main_window() const {
    return main_window;
}

SDL_Window* Screen::get_debug_window() const {
    return debug_window;
}

SDL_Renderer* Screen::get_debug_renderer() const {
    return debug_renderer;
}

SDL_Window* Screen::get_keyboard_window() const {
    return keyboard_window;
}

// creates a sdl surface with the zx81 character set
void Screen::update_charset(const Memory& memory, word_t charset_addr) {
    // create a rgb surface to hold 256 8x8 characters
    SDL_Surface* charset_rgb = xSDL_CreateSurface(4096, 16,
                               SDL_PIXELFORMAT_ARGB8888);

    // map black and white colors
    const SDL_PixelFormatDetails* format_details =
        xSDL_GetPixelFormatDetails(charset_rgb->format);
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
    if (zx81_charset) {
        SDL_DestroySurface(zx81_charset);
    }
    zx81_charset = xSDL_ConvertSurface(charset_rgb, SDL_PIXELFORMAT_ARGB8888);
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
                SDL_BlitSurface(zx81_charset, &source, zx81_screen, &dest);
            }

            dest.x += 16;
        }

        addr++;        // skip the 0x76 at the end of the line
        dest.y += 16;
    }
}
