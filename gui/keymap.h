//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "keyboard.h"
#include <SDL3/SDL.h>

kbd_state_t sdlk2kbd(SDL_Keycode sym, SDL_Keymod mod);
