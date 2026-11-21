//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

void xSDL_Init(SDL_InitFlags flags);
SDL_Renderer* xSDL_CreateRenderer(SDL_Window* window, const char* name);
SDL_Surface* xSDL_CreateSurface(int width, int height, SDL_PixelFormat format);
SDL_Texture* xSDL_CreateTexture(SDL_Renderer* renderer, SDL_PixelFormat format,
                                SDL_TextureAccess access, int w, int h);
SDL_Surface* xIMG_Load(const char* file);
SDL_Texture* xSDL_CreateTextureFromSurface(SDL_Renderer* renderer,
        SDL_Surface* surface);
void xSDL_RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture,
                        const SDL_FRect* srcrect, const SDL_FRect* dstrect);
const SDL_PixelFormatDetails* xSDL_GetPixelFormatDetails(
    SDL_PixelFormat format);
SDL_Surface* xSDL_ConvertSurface(SDL_Surface* surface, SDL_PixelFormat format);

inline void SDL_Delay64(uint64_t delay) {
    SDL_Delay(static_cast<uint32_t>(delay));
}
