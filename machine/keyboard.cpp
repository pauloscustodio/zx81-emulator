//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "keyboard.h"
#include "zx81.h"
#include <SDL.h>

//-----------------------------------------------------------------------------
// mapping of SDLK_xxx to kbd_state_t
//-----------------------------------------------------------------------------
static kbd_state_t sdlk2kbd_base(SDL_Keycode sym) {
    switch (sym) {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
        return K_SHIFT;
    case SDLK_z:
        return K_Z;
    case SDLK_x:
        return K_X;
    case SDLK_c:
        return K_C;
    case SDLK_v:
        return K_V;
    case SDLK_a:
        return K_A;
    case SDLK_s:
        return K_S;
    case SDLK_d:
        return K_D;
    case SDLK_f:
        return K_F;
    case SDLK_g:
        return K_G;
    case SDLK_q:
        return K_Q;
    case SDLK_w:
        return K_W;
    case SDLK_e:
        return K_E;
    case SDLK_r:
        return K_R;
    case SDLK_t:
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
    case SDLK_p:
        return K_P;
    case SDLK_o:
        return K_O;
    case SDLK_i:
        return K_I;
    case SDLK_u:
        return K_U;
    case SDLK_y:
        return K_Y;
    case SDLK_RETURN:
        return K_RETURN;
    case SDLK_l:
        return K_L;
    case SDLK_k:
        return K_K;
    case SDLK_j:
        return K_J;
    case SDLK_h:
        return K_H;
    case SDLK_SPACE:
        return K_SPACE;
    case SDLK_PERIOD:
        return K_PERIOD;
    case SDLK_m:
        return K_M;
    case SDLK_n:
        return K_N;
    case SDLK_b:
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
    case SDLK_QUOTE:
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

//-----------------------------------------------------------------------------
// Keyboard class
//-----------------------------------------------------------------------------
void Keyboard::clear() {
    key_state = 0;
    shift_state = 0;
}

void Keyboard::keydown(kbd_state_t key) {
    key_state |= key;
    if ((key & K_SHIFT) != 0) {
        shift_state = K_SHIFT;
    }
}

void Keyboard::keyup(kbd_state_t key) {
    key_state &= ~key;
    if ((key & K_SHIFT) != 0) {
        shift_state = 0;
    }
}

void Keyboard::handle_sdl_event(const SDL_Event* event) {
    SDL_Keycode sym = event->key.keysym.sym;
    bool shifted = (event->key.keysym.mod & KMOD_SHIFT) != 0;
    kbd_state_t key = sdlk2kbd(sym, shifted);

    switch (event->type) {
    case SDL_KEYDOWN:
        keydown(key);
        break;
    case SDL_KEYUP:
        keyup(key);
        break;
    }
}

byte_t Keyboard::in(word_t addr) {
    // high byte of the address has the selected row with bit=0
    byte_t row = (byte_t)((addr >> 8) & 0xFF);

    // Note: key_state is 1 for key pressed, ROM is 0 for key pressed
    switch (row) {
    case 0xFE:
        return (byte_t)(~key_state & 0xff);
    case 0xFD:
        return (byte_t)((~key_state >> 8) & 0xff);
    case 0xFB:
        return (byte_t)((~key_state >> 16) & 0xff);
    case 0xF7:
        return (byte_t)((~key_state >> 24) & 0xff);
    case 0xEF:
        return (byte_t)((~key_state >> 32) & 0xff);
    case 0xDF:
        return (byte_t)((~key_state >> 40) & 0xff);
    case 0xBF:
        return (byte_t)((~key_state >> 48) & 0xff);
    case 0x7F:
        return (byte_t)((~key_state >> 56) & 0xff);
    default:
        return 0xff;
    }
}
