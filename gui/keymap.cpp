//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "keyboard.h"
#include "keymap.h"
#include <SDL3/SDL.h>

static kbd_state_t sdlk2kbd_base(SDL_Keycode sym) {
    switch (sym) {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
        return K_SHIFT;
    case SDLK_Z:
        return K_Z;
    case SDLK_X:
        return K_X;
    case SDLK_C:
        return K_C;
    case SDLK_V:
        return K_V;
    case SDLK_A:
        return K_A;
    case SDLK_S:
        return K_S;
    case SDLK_D:
        return K_D;
    case SDLK_F:
        return K_F;
    case SDLK_G:
        return K_G;
    case SDLK_Q:
        return K_Q;
    case SDLK_W:
        return K_W;
    case SDLK_E:
        return K_E;
    case SDLK_R:
        return K_R;
    case SDLK_T:
        return K_T;
    case SDLK_1:
        return K_1;
    case SDLK_2:
        return K_2;
    case SDLK_3:
        return K_3;
    case SDLK_4:
        return K_4;
    case SDLK_5:
        return K_5;
    case SDLK_0:
        return K_0;
    case SDLK_9:
        return K_9;
    case SDLK_8:
        return K_8;
    case SDLK_7:
        return K_7;
    case SDLK_6:
        return K_6;
    case SDLK_P:
        return K_P;
    case SDLK_O:
        return K_O;
    case SDLK_I:
        return K_I;
    case SDLK_U:
        return K_U;
    case SDLK_Y:
        return K_Y;
    case SDLK_RETURN:
        return K_RETURN;
    case SDLK_L:
        return K_L;
    case SDLK_K:
        return K_K;
    case SDLK_J:
        return K_J;
    case SDLK_H:
        return K_H;
    case SDLK_SPACE:
        return K_SPACE;
    case SDLK_PERIOD:
        return K_PERIOD;
    case SDLK_M:
        return K_M;
    case SDLK_N:
        return K_N;
    case SDLK_B:
        return K_B;
    case SDLK_BACKSPACE:
        return K_BACKSPACE;
    case SDLK_MINUS:
        return K_MINUS;
    case SDLK_EQUALS:
        return K_EQ;
    case SDLK_LEFTBRACKET:
        return K_LPAREN;
    case SDLK_RIGHTBRACKET:
        return K_RPAREN;
    case SDLK_SEMICOLON:
        return K_SEMICOLON;
    case SDLK_APOSTROPHE:
        return K_QUOTE;
    case SDLK_COMMA:
        return K_COMMA;
    case SDLK_SLASH:
        return K_DIVIDE;
    case SDLK_UP:
        return K_UP;
    case SDLK_DOWN:
        return K_DOWN;
    case SDLK_LEFT:
        return K_LEFT;
    case SDLK_RIGHT:
        return K_RIGHT;
    default:
        return 0;
    }
}

static kbd_state_t sdlk2kbd_shift(SDL_Keycode sym) {
    switch (sym) {
    case SDLK_EQUALS:
        return K_PLUS;
    case SDLK_SEMICOLON:
        return K_COLON;
    case SDLK_COMMA:
        return K_LT;
    case SDLK_PERIOD:
        return K_GT;
    case SDLK_SLASH:
        return K_QUESTION;
    default:
        return sdlk2kbd_base(sym);
    }
}

static kbd_state_t sdlk2kbd(SDL_Keycode sym, bool shifted) {
    return shifted ? sdlk2kbd_shift(sym) : sdlk2kbd_base(sym);
}

kbd_state_t sdlk2kbd(SDL_Keycode sym, SDL_Keymod mod) {
    bool shifted = (mod & SDL_KMOD_SHIFT) != 0;
    return sdlk2kbd(sym, shifted);
}
