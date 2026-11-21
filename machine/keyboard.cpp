//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "keyboard.h"
#include "memory.h"
#include "zx81.h"
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <SDL.h>
#include <string>

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
// mapping of BASIC keywords to keys
//-----------------------------------------------------------------------------
enum { KL_MODE, FN_MODE, GR_MODE };

typedef struct {
    const char* keyword;
    int mode;
    kbd_state_t key;
} keyword_map_t;

static const keyword_map_t keyword_map[256] = {
    /* 0x00 */{ " ", KL_MODE, K_SPACE },
    /* 0x01 */{ "\\' ", GR_MODE, K_SHIFT | K_1 },
    /* 0x02 */{ "\\ '", GR_MODE, K_SHIFT | K_2 },
    /* 0x03 */{ "\\''", GR_MODE, K_SHIFT | K_7 },
    /* 0x04 */{ "\\. ", GR_MODE, K_SHIFT | K_4 },
    /* 0x05 */{ "\\: ", GR_MODE, K_SHIFT | K_5 },
    /* 0x06 */{ "\\.'", GR_MODE, K_SHIFT | K_T },
    /* 0x07 */{ "\\:'", GR_MODE, K_SHIFT | K_E },
    /* 0x08 */{ "\\##", GR_MODE, K_SHIFT | K_A },
    /* 0x09 */{ "\\,,", GR_MODE, K_SHIFT | K_D },
    /* 0x0A */{ "\\~~", GR_MODE, K_SHIFT | K_S },
    /* 0x0B */{ "\"", KL_MODE, K_QUOTE },
    /* 0x0C */{ "#", KL_MODE, K_POUND },
    /* 0x0D */{ "$", KL_MODE, K_DOLLAR },
    /* 0x0E */{ ":", KL_MODE, K_COLON },
    /* 0x0F */{ "?", KL_MODE, K_QUESTION },
    /* 0x10 */{ "(", KL_MODE, K_LPAREN },
    /* 0x11 */{ ")", KL_MODE, K_RPAREN },
    /* 0x12 */{ ">", KL_MODE, K_GT },
    /* 0x13 */{ "<", KL_MODE, K_LT },
    /* 0x14 */{ "=", KL_MODE, K_EQ },
    /* 0x15 */{ "+", KL_MODE, K_PLUS },
    /* 0x16 */{ "-", KL_MODE, K_MINUS },
    /* 0x17 */{ "*", KL_MODE, K_MULTIPLY },
    /* 0x18 */{ "/", KL_MODE, K_DIVIDE },
    /* 0x19 */{ ";", KL_MODE, K_SEMICOLON },
    /* 0x1A */{ ",", KL_MODE, K_COMMA },
    /* 0x1B */{ ".", KL_MODE, K_PERIOD },
    /* 0x1C */{ "0", KL_MODE, K_0 },
    /* 0x1D */{ "1", KL_MODE, K_1 },
    /* 0x1E */{ "2", KL_MODE, K_2 },
    /* 0x1F */{ "3", KL_MODE, K_3 },
    /* 0x20 */{ "4", KL_MODE, K_4 },
    /* 0x21 */{ "5", KL_MODE, K_5 },
    /* 0x22 */{ "6", KL_MODE, K_6 },
    /* 0x23 */{ "7", KL_MODE, K_7 },
    /* 0x24 */{ "8", KL_MODE, K_8 },
    /* 0x25 */{ "9", KL_MODE, K_9 },
    /* 0x26 */{ "A", KL_MODE, K_A },
    /* 0x27 */{ "B", KL_MODE, K_B },
    /* 0x28 */{ "C", KL_MODE, K_C },
    /* 0x29 */{ "D", KL_MODE, K_D },
    /* 0x2A */{ "E", KL_MODE, K_E },
    /* 0x2B */{ "F", KL_MODE, K_F },
    /* 0x2C */{ "G", KL_MODE, K_G },
    /* 0x2D */{ "H", KL_MODE, K_H },
    /* 0x2E */{ "I", KL_MODE, K_I },
    /* 0x2F */{ "J", KL_MODE, K_J },
    /* 0x30 */{ "K", KL_MODE, K_K },
    /* 0x31 */{ "L", KL_MODE, K_L },
    /* 0x32 */{ "M", KL_MODE, K_M },
    /* 0x33 */{ "N", KL_MODE, K_N },
    /* 0x34 */{ "O", KL_MODE, K_O },
    /* 0x35 */{ "P", KL_MODE, K_P },
    /* 0x36 */{ "Q", KL_MODE, K_Q },
    /* 0x37 */{ "R", KL_MODE, K_R },
    /* 0x38 */{ "S", KL_MODE, K_S },
    /* 0x39 */{ "T", KL_MODE, K_T },
    /* 0x3A */{ "U", KL_MODE, K_U },
    /* 0x3B */{ "V", KL_MODE, K_V },
    /* 0x3C */{ "W", KL_MODE, K_W },
    /* 0x3D */{ "X", KL_MODE, K_X },
    /* 0x3E */{ "Y", KL_MODE, K_Y },
    /* 0x3F */{ "Z", KL_MODE, K_Z },
    /* 0x40 */{ "RND", FN_MODE, FN_RND },
    /* 0x41 */{ "INKEY$", FN_MODE, FN_INKEY_DLR },
    /* 0x42 */{ "PI", FN_MODE, FN_PI },
    /* 0x43 */{ nullptr, 0, 0 },
    /* 0x44 */{ nullptr, 0, 0 },
    /* 0x45 */{ nullptr, 0, 0 },
    /* 0x46 */{ nullptr, 0, 0 },
    /* 0x47 */{ nullptr, 0, 0 },
    /* 0x48 */{ nullptr, 0, 0 },
    /* 0x49 */{ nullptr, 0, 0 },
    /* 0x4A */{ nullptr, 0, 0 },
    /* 0x4B */{ nullptr, 0, 0 },
    /* 0x4C */{ nullptr, 0, 0 },
    /* 0x4D */{ nullptr, 0, 0 },
    /* 0x4E */{ nullptr, 0, 0 },
    /* 0x4F */{ nullptr, 0, 0 },
    /* 0x50 */{ nullptr, 0, 0 },
    /* 0x51 */{ nullptr, 0, 0 },
    /* 0x52 */{ nullptr, 0, 0 },
    /* 0x53 */{ nullptr, 0, 0 },
    /* 0x54 */{ nullptr, 0, 0 },
    /* 0x55 */{ nullptr, 0, 0 },
    /* 0x56 */{ nullptr, 0, 0 },
    /* 0x57 */{ nullptr, 0, 0 },
    /* 0x58 */{ nullptr, 0, 0 },
    /* 0x59 */{ nullptr, 0, 0 },
    /* 0x5A */{ nullptr, 0, 0 },
    /* 0x5B */{ nullptr, 0, 0 },
    /* 0x5C */{ nullptr, 0, 0 },
    /* 0x5D */{ nullptr, 0, 0 },
    /* 0x5E */{ nullptr, 0, 0 },
    /* 0x5F */{ nullptr, 0, 0 },
    /* 0x60 */{ nullptr, 0, 0 },
    /* 0x61 */{ nullptr, 0, 0 },
    /* 0x62 */{ nullptr, 0, 0 },
    /* 0x63 */{ nullptr, 0, 0 },
    /* 0x64 */{ nullptr, 0, 0 },
    /* 0x65 */{ nullptr, 0, 0 },
    /* 0x66 */{ nullptr, 0, 0 },
    /* 0x67 */{ nullptr, 0, 0 },
    /* 0x68 */{ nullptr, 0, 0 },
    /* 0x69 */{ nullptr, 0, 0 },
    /* 0x6A */{ nullptr, 0, 0 },
    /* 0x6B */{ nullptr, 0, 0 },
    /* 0x6C */{ nullptr, 0, 0 },
    /* 0x6D */{ nullptr, 0, 0 },
    /* 0x6E */{ nullptr, 0, 0 },
    /* 0x6F */{ nullptr, 0, 0 },
    /* 0x70 */{ nullptr, 0, 0 },
    /* 0x71 */{ nullptr, 0, 0 },
    /* 0x72 */{ nullptr, 0, 0 },
    /* 0x73 */{ nullptr, 0, 0 },
    /* 0x74 */{ nullptr, 0, 0 },
    /* 0x75 */{ nullptr, 0, 0 },
    /* 0x76 */{ nullptr, 0, 0 },
    /* 0x77 */{ nullptr, 0, 0 },
    /* 0x78 */{ nullptr, 0, 0 },
    /* 0x79 */{ nullptr, 0, 0 },
    /* 0x7A */{ nullptr, 0, 0 },
    /* 0x7B */{ nullptr, 0, 0 },
    /* 0x7C */{ nullptr, 0, 0 },
    /* 0x7D */{ nullptr, 0, 0 },
    /* 0x7E */{ nullptr, 0, 0 },
    /* 0x7F */{ nullptr, 0, 0 },
    /* 0x80 */{ "\\::", GR_MODE, K_SPACE },
    /* 0x81 */{ "\\.:", GR_MODE, K_SHIFT | K_Q },
    /* 0x82 */{ "\\:.", GR_MODE, K_SHIFT | K_W },
    /* 0x83 */{ "\\..", GR_MODE, K_SHIFT | K_6 },
    /* 0x84 */{ "\\':", GR_MODE, K_SHIFT | K_R },
    /* 0x85 */{ "\\ :", GR_MODE, K_SHIFT | K_8 },
    /* 0x86 */{ "\\'.", GR_MODE, K_SHIFT | K_Y },
    /* 0x87 */{ "\\ .", GR_MODE, K_SHIFT | K_3 },
    /* 0x88 */{ "\\@@", GR_MODE, K_SHIFT | K_H },
    /* 0x89 */{ "\\;;", GR_MODE, K_SHIFT | K_G },
    /* 0x8A */{ "\\!!", GR_MODE, K_SHIFT | K_F },
    /* 0x8B */{ "%\"", GR_MODE, K_QUOTE },
    /* 0x8C */{ "%#", GR_MODE, K_POUND },
    /* 0x8D */{ "%$", GR_MODE, K_DOLLAR },
    /* 0x8E */{ "%:", GR_MODE, K_COLON },
    /* 0x8F */{ "%?", GR_MODE, K_QUESTION },
    /* 0x90 */{ "%(", GR_MODE, K_LPAREN },
    /* 0x91 */{ "%)", GR_MODE, K_RPAREN },
    /* 0x92 */{ "%>", GR_MODE, K_GT },
    /* 0x93 */{ "%<", GR_MODE, K_LT },
    /* 0x94 */{ "%=", GR_MODE, K_EQ },
    /* 0x95 */{ "%+", GR_MODE, K_PLUS },
    /* 0x96 */{ "%-", GR_MODE, K_MINUS },
    /* 0x97 */{ "%*", GR_MODE, K_MULTIPLY },
    /* 0x98 */{ "%/", GR_MODE, K_DIVIDE },
    /* 0x99 */{ "%;", GR_MODE, K_SEMICOLON },
    /* 0x9A */{ "%,", GR_MODE, K_COMMA },
    /* 0x9B */{ "%.", GR_MODE, K_PERIOD },
    /* 0x9C */{ "%0", GR_MODE, K_0 },
    /* 0x9D */{ "%1", GR_MODE, K_1 },
    /* 0x9E */{ "%2", GR_MODE, K_2 },
    /* 0x9F */{ "%3", GR_MODE, K_3 },
    /* 0xA0 */{ "%4", GR_MODE, K_4 },
    /* 0xA1 */{ "%5", GR_MODE, K_5 },
    /* 0xA2 */{ "%6", GR_MODE, K_6 },
    /* 0xA3 */{ "%7", GR_MODE, K_7 },
    /* 0xA4 */{ "%8", GR_MODE, K_8 },
    /* 0xA5 */{ "%9", GR_MODE, K_9 },
    /* 0xA6 */{ "%A", GR_MODE, K_A },
    /* 0xA7 */{ "%B", GR_MODE, K_B },
    /* 0xA8 */{ "%C", GR_MODE, K_C },
    /* 0xA9 */{ "%D", GR_MODE, K_D },
    /* 0xAA */{ "%E", GR_MODE, K_E },
    /* 0xAB */{ "%F", GR_MODE, K_F },
    /* 0xAC */{ "%G", GR_MODE, K_G },
    /* 0xAD */{ "%H", GR_MODE, K_H },
    /* 0xAE */{ "%I", GR_MODE, K_I },
    /* 0xAF */{ "%J", GR_MODE, K_J },
    /* 0xB0 */{ "%K", GR_MODE, K_K },
    /* 0xB1 */{ "%L", GR_MODE, K_L },
    /* 0xB2 */{ "%M", GR_MODE, K_M },
    /* 0xB3 */{ "%N", GR_MODE, K_N },
    /* 0xB4 */{ "%O", GR_MODE, K_O },
    /* 0xB5 */{ "%P", GR_MODE, K_P },
    /* 0xB6 */{ "%Q", GR_MODE, K_Q },
    /* 0xB7 */{ "%R", GR_MODE, K_R },
    /* 0xB8 */{ "%S", GR_MODE, K_S },
    /* 0xB9 */{ "%T", GR_MODE, K_T },
    /* 0xBA */{ "%U", GR_MODE, K_U },
    /* 0xBB */{ "%V", GR_MODE, K_V },
    /* 0xBC */{ "%W", GR_MODE, K_W },
    /* 0xBD */{ "%X", GR_MODE, K_X },
    /* 0xBE */{ "%Y", GR_MODE, K_Y },
    /* 0xBF */{ "%Z", GR_MODE, K_Z },
    /* 0xC0 */{ "\\\"", KL_MODE, K_DQUOTE },
    /* 0xC1 */{ "AT", FN_MODE, FN_AT },
    /* 0xC2 */{ "TAB", FN_MODE, FN_TAB },
    /* 0xC3 */{ nullptr, 0, 0 },
    /* 0xC4 */{ "CODE", FN_MODE, FN_CODE },
    /* 0xC5 */{ "VAL", FN_MODE, FN_VAL },
    /* 0xC6 */{ "LEN", FN_MODE, FN_LEN },
    /* 0xC7 */{ "SIN", FN_MODE, FN_SIN },
    /* 0xC8 */{ "COS", FN_MODE, FN_COS },
    /* 0xC9 */{ "TAN", FN_MODE, FN_TAN },
    /* 0xCA */{ "ASN", FN_MODE, FN_ARCSIN },
    /* 0xCB */{ "ACS", FN_MODE, FN_ARCCOS },
    /* 0xCC */{ "ATN", FN_MODE, FN_ARCTAN },
    /* 0xCD */{ "LN", FN_MODE, FN_LN },
    /* 0xCE */{ "EXP", FN_MODE, FN_EXP },
    /* 0xCF */{ "INT", FN_MODE, FN_INT },
    /* 0xD0 */{ "SQR", FN_MODE, FN_SQR },
    /* 0xD1 */{ "SGN", FN_MODE, FN_SGN },
    /* 0xD2 */{ "ABS", FN_MODE, FN_ABS },
    /* 0xD3 */{ "PEEK", FN_MODE, FN_PEEK },
    /* 0xD4 */{ "USR", FN_MODE, FN_USR },
    /* 0xD5 */{ "STR$", FN_MODE, FN_STR_DLR },
    /* 0xD6 */{ "CHR$", FN_MODE, FN_CHR_DLR },
    /* 0xD7 */{ "NOT", FN_MODE, FN_NOT },
    /* 0xD8 */{ "**", KL_MODE, K_POWER },
    /* 0xD9 */{ "OR", KL_MODE, K_OR },
    /* 0xDA */{ "AND", KL_MODE, K_AND },
    /* 0xDB */{ "<=", KL_MODE, K_LE },
    /* 0xDC */{ ">=", KL_MODE, K_GE },
    /* 0xDD */{ "<>", KL_MODE, K_NE },
    /* 0xDE */{ "THEN", KL_MODE, K_THEN },
    /* 0xDF */{ "TO", KL_MODE, K_TO },
    /* 0xE0 */{ "STEP", KL_MODE, K_STEP },
    /* 0xE1 */{ "LPRINT", KL_MODE, K_LPRINT },
    /* 0xE2 */{ "LLIST", KL_MODE, K_LLIST },
    /* 0xE3 */{ "STOP", KL_MODE, K_STOP },
    /* 0xE4 */{ "SLOW", KL_MODE, K_SLOW },
    /* 0xE5 */{ "FAST", KL_MODE, K_FAST },
    /* 0xE6 */{ "NEW", KL_MODE, KW_NEW },
    /* 0xE7 */{ "SCROLL", KL_MODE, KW_SCROLL },
    /* 0xE8 */{ "CONT", KL_MODE, KW_CONT },
    /* 0xE9 */{ "DIM", KL_MODE, KW_DIM },
    /* 0xEA */{ "REM", KL_MODE, KW_REM },
    /* 0xEB */{ "FOR", KL_MODE, KW_FOR },
    /* 0xEC */{ "GOTO", KL_MODE, KW_GOTO },
    /* 0xED */{ "GOSUB", KL_MODE, KW_GOSUB },
    /* 0xEE */{ "INPUT", KL_MODE, KW_INPUT },
    /* 0xEF */{ "LOAD", KL_MODE, KW_LOAD },
    /* 0xF0 */{ "LIST", KL_MODE, KW_LIST },
    /* 0xF1 */{ "LET", KL_MODE, KW_LET },
    /* 0xF2 */{ "PAUSE", KL_MODE, KW_PAUSE },
    /* 0xF3 */{ "NEXT", KL_MODE, KW_NEXT },
    /* 0xF4 */{ "POKE", KL_MODE, KW_POKE },
    /* 0xF5 */{ "PRINT", KL_MODE, KW_PRINT },
    /* 0xF6 */{ "PLOT", KL_MODE, KW_PLOT },
    /* 0xF7 */{ "RUN", KL_MODE, KW_RUN },
    /* 0xF8 */{ "SAVE", KL_MODE, KW_SAVE },
    /* 0xF9 */{ "RAND", KL_MODE, KW_RAND },
    /* 0xFA */{ "IF", KL_MODE, KW_IF },
    /* 0xFB */{ "CLS", KL_MODE, KW_CLS },
    /* 0xFC */{ "UNPLOT", KL_MODE, KW_UNPLOT },
    /* 0xFD */{ "CLEAR", KL_MODE, KW_CLEAR },
    /* 0xFE */{ "RETURN", KL_MODE, KW_RETURN },
    /* 0xFF */{ "COPY", KL_MODE, KW_COPY }
};

static int strn_case_cmp(const char* s1, const char* s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        unsigned char c1 = (unsigned char)s1[i];
        unsigned char c2 = (unsigned char)s2[i];
        int lc1 = tolower(c1);
        int lc2 = tolower(c2);

        if (lc1 < lc2) {
            return -1;
        }
        if (lc1 > lc2) {
            return 1;
        }
    }

    return 0;
}

static const keyword_map_t* zx81_lookup_code(const char** p,
        bool allow_keyword) {
    if (p == nullptr || *p == nullptr) {
        return nullptr;
    }

    // search in reverse order to match ** instead of * *
    for (int i = sizeof(keyword_map) / sizeof(keyword_map[0]) - 1; i >= 0; i--) {
        const char* keyword = keyword_map[i].keyword;
        if (keyword == nullptr) {
            continue;
        }

        if (!allow_keyword && (i & 0x40) != 0) {
            continue; // skip function keywords if not allowed
        }

        size_t len = strlen(keyword);
        if (strn_case_cmp(*p, keyword, len) == 0) {
            *p += len; // advance the pointer
            return &keyword_map[i];
        }
    }

    return nullptr;
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

// called once per frame to update the keyboard queue and LAST_K
void Keyboard::scan(Memory& memory) {
    // only scan the keyboard if there are no pending events in the queue
    // avoid key losses during paste, as a 0xffff LAST_K is written over the
    // LAST_K from the pushed key
    if (!queue_busy()) {
        word_t last_k = rom_keyboard_scan();
        memory.write16(LAST_K, last_k);
    }
    else {
        // push the next keydown/keyup from the queue
        queue_tick();
    }
}

bool Keyboard::queue_busy() const {
    return !queue.empty() || q_state != Q_IDLE;
}

void Keyboard::queue_tick() {
    uint64_t now = SDL_GetTicks64();
    if (now - q_event_ms < QueueTimeMs) {
        return; // not enough time has passed
    }
    q_event_ms = now;

    if (q_state == Q_IDLE) {
        if (!queue.empty()) {
            last_key_pushed = queue.front();
            queue.pop();
            keydown(last_key_pushed);
            q_state = Q_DOWN;
        }
    }
    else if (q_state == Q_DOWN) {
        keyup(last_key_pushed);
        last_key_pushed = 0;
        q_state = Q_UP;
    }
    else if (q_state == Q_UP) {
        q_state = Q_IDLE;
    }
}

// C translation of the ZX81 KEYBOARD scanning routine
word_t Keyboard::rom_keyboard_scan() {
    // read the keyboard key_state using the ZX81 keyboard scanning routine
    byte_t h = 0xFF;
    byte_t l = 0xFF;
    byte_t row = 0xFE;

    byte_t a = in(0xFEFE);
    a |= 0x01;       /* ignore SHIFT initially */

    for (;;) {
        byte_t d = a | 0xE0;

        if ((~a & 0x1F) == 0) {
            l &= 0xFF;
        }
        else {
            l &= row;
        }

        h &= d;

        int done = (row & 0x80) == 0;

        row = (byte_t)((row << 1) | (row >> 7));
        a = in(((word_t)row << 8) | 0xFE);

        if (done) {
            break;
        }
    }

    /* incorporate SHIFT key */
    if (a & 0x01) {
        h = (byte_t)((h << 1) | 1);
    }
    else {
        h = (byte_t)(h << 1);
    }

    return ((word_t)h << 8) | l;
}

void Keyboard::push_one_token(const char** p, int& mode, bool allow_keywords) {
    if (**p == '\0') {
        return; // end of string
    }

    // lookup the keyword in the mapping table
    const keyword_map_t* kw_map = zx81_lookup_code(p, allow_keywords);
    if (kw_map == nullptr) {
        std::cerr << "Error: Unknown keyword in text: " << std::string(*p) << std::endl;
        (*p)++;
        return;
    }

    // change mode if needed
    switch (kw_map->mode) {
    case KL_MODE:
        if (mode == GR_MODE) {
            queue.push(K_GRAPHICS);
            mode = KL_MODE;
        }
        break;
    case FN_MODE:
        queue.push(K_FUNCTION);
        break;
    case GR_MODE:
        if (mode != GR_MODE) {
            queue.push(K_GRAPHICS);
            mode = GR_MODE;
        }
        break;
    default:
        std::cerr << "Error: Invalid mode in keyword mapping: " << kw_map->mode <<
                  std::endl;
        break;
    }

    // send the key to the keyboard queue
    queue.push(kw_map->key);
}

// paste text to keyboard queue
void Keyboard::paste(const std::string& text) {
    const char* p = text.c_str();
    int mode = KL_MODE;
    while (*p != '\0') {
        // newline
        if (*p == '\r' || *p == '\n') {
            queue.push(K_RETURN);
            mode = KL_MODE;
            p++;
            continue;
        }

        // white space
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }

        // quoted string
        if (*p == '"') {
            // extract quoted string from input
            std::string quoted_text;
            quoted_text.push_back('"');
            p++;

            while (*p != '\0' && *p != '"' && *p != '\r' && *p != '\n') {
                quoted_text.push_back(*p);
                p++;
            }

            if (*p == '"') {
                quoted_text.push_back('"');
                p++;
            }

            // push the quoted string to the keyboard queue
            const char* quoted_str = quoted_text.c_str();
            while (*quoted_str != '\0') {
                if (*quoted_str == '\\' && *(quoted_str + 1) == '"') {
                    if (mode == GR_MODE) {
                        queue.push(K_GRAPHICS); // return to KL_MODE after pasting
                        mode = KL_MODE;
                    }
                    queue.push(K_DQUOTE);
                    quoted_str += 2; // skip the escaped quote
                }
                else {
                    push_one_token(&quoted_str, mode, false);
                }
            }
            continue;
        }

        // allow keywords in BASIC code
        push_one_token(&p, mode, true);
    }

    if (mode == GR_MODE) {
        queue.push(K_GRAPHICS); // return to KL_MODE after pasting
    }
}

void Keyboard::paste_file(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::cerr << "Error: open file " << filename << " failed" << std::endl;
        return;
    }

    std::string text;
    while (std::getline(ifs, text)) {
        paste(text);
    }
}
