//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "sdl.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>

void xSDL_Init(SDL_InitFlags flags) {
    if (!SDL_Init(flags)) {
        die("SDL_Init failed: " + std::string(SDL_GetError()));
    }
}

SDL_Renderer* xSDL_CreateRenderer(SDL_Window* window, const char* name) {
    SDL_Renderer* renderer = SDL_CreateRenderer(window, name);
    if (renderer == nullptr) {
        die("Unable to create renderer: " + std::string(SDL_GetError()));
    }
    return renderer;
}

SDL_Surface* xSDL_CreateSurface(int width, int height, SDL_PixelFormat format) {
    SDL_Surface* surface = SDL_CreateSurface(width, height, format);
    if (surface == nullptr) {
        die("Unable to create surface: " + std::string(SDL_GetError()));
    }
    return surface;
}

SDL_Texture* xSDL_CreateTexture(SDL_Renderer* renderer, SDL_PixelFormat format,
                                SDL_TextureAccess access, int w, int h) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, format, access, w, h);
    if (texture == nullptr) {
        die("Unable to create texture: " + std::string(SDL_GetError()));
    }
    return texture;
}

SDL_Surface* xIMG_Load(const char* file) {
    SDL_Surface* surface = IMG_Load(file);
    if (surface == nullptr) {
        die("Unable to load image: " + std::string(SDL_GetError()));
    }
    return surface;
}

SDL_Texture* xSDL_CreateTextureFromSurface(SDL_Renderer* renderer,
        SDL_Surface* surface) {
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        die("Unable to create texture from surface: " + std::string(SDL_GetError()));
    }
    return texture;
}

void xSDL_RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture,
                        const SDL_FRect* srcrect, const SDL_FRect* dstrect) {
    if (!SDL_RenderTexture(renderer, texture, srcrect, dstrect)) {
        die("Unable to render texture: " + std::string(SDL_GetError()));
    }
}

const SDL_PixelFormatDetails* xSDL_GetPixelFormatDetails(
    SDL_PixelFormat format) {
    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(format);
    if (details == nullptr) {
        die("Unable to get pixel format details: " + std::string(SDL_GetError()));
    }
    return details;
}

SDL_Surface* xSDL_ConvertSurface(SDL_Surface* surface, SDL_PixelFormat format) {
    SDL_Surface* new_surface = SDL_ConvertSurface(surface, format);
    if (new_surface == nullptr) {
        die("Unable to convert surface: " + std::string(SDL_GetError()));
    }
    return new_surface;
}

