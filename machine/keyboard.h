//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "zx81.h"
#include <cstdint>
#include <queue>
#include <SDL.h>

// bit mask with 1 at each selected column of the 8 keyboard rows
using  kbd_state_t = uint64_t;

// keyboard masks
static inline constexpr kbd_state_t K_SHIFT = (1ull << (0 * 8));
static inline constexpr kbd_state_t K_Z = ( 2ull << (0 * 8));
static inline constexpr kbd_state_t K_X = ( 4ull << (0 * 8));
static inline constexpr kbd_state_t K_C = ( 8ull << (0 * 8));
static inline constexpr kbd_state_t K_V = (16ull << (0 * 8));
static inline constexpr kbd_state_t K_A = ( 1ull << (1 * 8));
static inline constexpr kbd_state_t K_S = ( 2ull << (1 * 8));
static inline constexpr kbd_state_t K_D = ( 4ull << (1 * 8));
static inline constexpr kbd_state_t K_F = ( 8ull << (1 * 8));
static inline constexpr kbd_state_t K_G = (16ull << (1 * 8));
static inline constexpr kbd_state_t K_Q = ( 1ull << (2 * 8));
static inline constexpr kbd_state_t K_W = ( 2ull << (2 * 8));
static inline constexpr kbd_state_t K_E = ( 4ull << (2 * 8));
static inline constexpr kbd_state_t K_R = ( 8ull << (2 * 8));
static inline constexpr kbd_state_t K_T = (16ull << (2 * 8));
static inline constexpr kbd_state_t K_1 = ( 1ull << (3 * 8));
static inline constexpr kbd_state_t K_2 = ( 2ull << (3 * 8));
static inline constexpr kbd_state_t K_3 = ( 4ull << (3 * 8));
static inline constexpr kbd_state_t K_4 = ( 8ull << (3 * 8));
static inline constexpr kbd_state_t K_5 = (16ull << (3 * 8));
static inline constexpr kbd_state_t K_0 = ( 1ull << (4 * 8));
static inline constexpr kbd_state_t K_9 = ( 2ull << (4 * 8));
static inline constexpr kbd_state_t K_8 = ( 4ull << (4 * 8));
static inline constexpr kbd_state_t K_7 = ( 8ull << (4 * 8));
static inline constexpr kbd_state_t K_6 = (16ull << (4 * 8));
static inline constexpr kbd_state_t K_P = ( 1ull << (5 * 8));
static inline constexpr kbd_state_t K_O = ( 2ull << (5 * 8));
static inline constexpr kbd_state_t K_I = ( 4ull << (5 * 8));
static inline constexpr kbd_state_t K_U = ( 8ull << (5 * 8));
static inline constexpr kbd_state_t K_Y = (16ull << (5 * 8));
static inline constexpr kbd_state_t K_RETURN = ( 1ull << (6 * 8));
static inline constexpr kbd_state_t K_L = ( 2ull << (6 * 8));
static inline constexpr kbd_state_t K_K = ( 4ull << (6 * 8));
static inline constexpr kbd_state_t K_J = ( 8ull << (6 * 8));
static inline constexpr kbd_state_t K_H = (16ull << (6 * 8));
static inline constexpr kbd_state_t K_SPACE = ( 1ull << (7 * 8));
static inline constexpr kbd_state_t K_PERIOD = ( 2ull << (7 * 8));
static inline constexpr kbd_state_t K_M = ( 4ull << (7 * 8));
static inline constexpr kbd_state_t K_N = ( 8ull << (7 * 8));
static inline constexpr kbd_state_t K_B = (16ull << (7 * 8));

static inline constexpr kbd_state_t K_EDIT = (K_SHIFT | K_1);
static inline constexpr kbd_state_t K_AND = (K_SHIFT | K_2);
static inline constexpr kbd_state_t K_THEN = (K_SHIFT | K_3);
static inline constexpr kbd_state_t K_TO = (K_SHIFT | K_4);
static inline constexpr kbd_state_t K_LEFT = (K_SHIFT | K_5);
static inline constexpr kbd_state_t K_DOWN = (K_SHIFT | K_6);
static inline constexpr kbd_state_t K_UP = (K_SHIFT | K_7);
static inline constexpr kbd_state_t K_RIGHT = (K_SHIFT | K_8);
static inline constexpr kbd_state_t K_GRAPHICS = (K_SHIFT | K_9);
static inline constexpr kbd_state_t K_BACKSPACE = (K_SHIFT | K_0);
static inline constexpr kbd_state_t K_DQUOTE = (K_SHIFT | K_Q);
static inline constexpr kbd_state_t K_OR = (K_SHIFT | K_W);
static inline constexpr kbd_state_t K_STEP = (K_SHIFT | K_E);
static inline constexpr kbd_state_t K_LE = (K_SHIFT | K_R);
static inline constexpr kbd_state_t K_NE = (K_SHIFT | K_T);
static inline constexpr kbd_state_t K_GE = (K_SHIFT | K_Y);
static inline constexpr kbd_state_t K_DOLLAR = (K_SHIFT | K_U);
static inline constexpr kbd_state_t K_LPAREN = (K_SHIFT | K_I);
static inline constexpr kbd_state_t K_RPAREN = (K_SHIFT | K_O);
static inline constexpr kbd_state_t K_QUOTE = (K_SHIFT | K_P);
static inline constexpr kbd_state_t K_STOP = (K_SHIFT | K_A);
static inline constexpr kbd_state_t K_LPRINT = (K_SHIFT | K_S);
static inline constexpr kbd_state_t K_SLOW = (K_SHIFT | K_D);
static inline constexpr kbd_state_t K_FAST = (K_SHIFT | K_F);
static inline constexpr kbd_state_t K_LLIST = (K_SHIFT | K_G);
static inline constexpr kbd_state_t K_POWER = (K_SHIFT | K_H);
static inline constexpr kbd_state_t K_MINUS = (K_SHIFT | K_J);
static inline constexpr kbd_state_t K_PLUS = (K_SHIFT | K_K);
static inline constexpr kbd_state_t K_EQ = (K_SHIFT | K_L);
static inline constexpr kbd_state_t K_FUNCTION = (K_SHIFT | K_RETURN);
static inline constexpr kbd_state_t K_COLON = (K_SHIFT | K_Z);
static inline constexpr kbd_state_t K_SEMICOLON = (K_SHIFT | K_X);
static inline constexpr kbd_state_t K_QUESTION = (K_SHIFT | K_C);
static inline constexpr kbd_state_t K_DIVIDE = (K_SHIFT | K_V);
static inline constexpr kbd_state_t K_MULTIPLY = (K_SHIFT | K_B);
static inline constexpr kbd_state_t K_LT = (K_SHIFT | K_N);
static inline constexpr kbd_state_t K_GT = (K_SHIFT | K_M);
static inline constexpr kbd_state_t K_COMMA = (K_SHIFT | K_PERIOD);
static inline constexpr kbd_state_t K_POUND = (K_SHIFT | K_SPACE);

static inline constexpr kbd_state_t KW_PLOT = (K_Q);
static inline constexpr kbd_state_t KW_UNPLOT = (K_W);
static inline constexpr kbd_state_t KW_REM = (K_E);
static inline constexpr kbd_state_t KW_RUN = (K_R);
static inline constexpr kbd_state_t KW_RAND = (K_T);
static inline constexpr kbd_state_t KW_RETURN = (K_Y);
static inline constexpr kbd_state_t KW_IF = (K_U);
static inline constexpr kbd_state_t KW_INPUT = (K_I);
static inline constexpr kbd_state_t KW_POKE = (K_O);
static inline constexpr kbd_state_t KW_PRINT = (K_P);
static inline constexpr kbd_state_t KW_NEW = (K_A);
static inline constexpr kbd_state_t KW_SAVE = (K_S);
static inline constexpr kbd_state_t KW_DIM = (K_D);
static inline constexpr kbd_state_t KW_FOR = (K_F);
static inline constexpr kbd_state_t KW_GOTO = (K_G);
static inline constexpr kbd_state_t KW_GOSUB = (K_H);
static inline constexpr kbd_state_t KW_LOAD = (K_J);
static inline constexpr kbd_state_t KW_LIST = (K_K);
static inline constexpr kbd_state_t KW_LET = (K_L);
static inline constexpr kbd_state_t KW_COPY = (K_Z);
static inline constexpr kbd_state_t KW_CLEAR = (K_X);
static inline constexpr kbd_state_t KW_CONT = (K_C);
static inline constexpr kbd_state_t KW_CLS = (K_V);
static inline constexpr kbd_state_t KW_SCROLL = (K_B);
static inline constexpr kbd_state_t KW_NEXT = (K_N);
static inline constexpr kbd_state_t KW_PAUSE = (K_M);

static inline constexpr kbd_state_t FN_SIN = (K_Q);
static inline constexpr kbd_state_t FN_COS = (K_W);
static inline constexpr kbd_state_t FN_TAN = (K_E);
static inline constexpr kbd_state_t FN_INT = (K_R);
static inline constexpr kbd_state_t FN_RND = (K_T);
static inline constexpr kbd_state_t FN_STR_DLR = (K_Y);
static inline constexpr kbd_state_t FN_CHR_DLR = (K_U);
static inline constexpr kbd_state_t FN_CODE = (K_I);
static inline constexpr kbd_state_t FN_PEEK = (K_O);
static inline constexpr kbd_state_t FN_TAB = (K_P);
static inline constexpr kbd_state_t FN_ARCSIN = (K_A);
static inline constexpr kbd_state_t FN_ARCCOS = (K_S);
static inline constexpr kbd_state_t FN_ARCTAN = (K_D);
static inline constexpr kbd_state_t FN_SGN = (K_F);
static inline constexpr kbd_state_t FN_ABS = (K_G);
static inline constexpr kbd_state_t FN_SQR = (K_H);
static inline constexpr kbd_state_t FN_VAL = (K_J);
static inline constexpr kbd_state_t FN_LEN = (K_K);
static inline constexpr kbd_state_t FN_USR = (K_L);
static inline constexpr kbd_state_t FN_LN = (K_Z);
static inline constexpr kbd_state_t FN_EXP = (K_X);
static inline constexpr kbd_state_t FN_AT = (K_C);
static inline constexpr kbd_state_t FN_INKEY_DLR = (K_B);
static inline constexpr kbd_state_t FN_NOT = (K_N);
static inline constexpr kbd_state_t FN_PI = (K_M);

class Keyboard {
public:
    void clear();
    void keydown(kbd_state_t key);
    void keyup(kbd_state_t key);
    void handle_sdl_event(const SDL_Event* event);
    byte_t in(word_t addr);

private:
    kbd_state_t key_state;          // current state of the keys (pressed or not)
    kbd_state_t
    shift_state;        // current state of the shift key (pressed or not)
    std::queue<kbd_state_t> queue;  // keyboard queue for pasting
};
