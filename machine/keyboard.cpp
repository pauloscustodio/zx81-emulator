//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "chars.h"
#include "keyboard.h"
#include "memory.h"
#include "zx81.h"
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

//-----------------------------------------------------------------------------
// mapping of BASIC keywords to keys
//-----------------------------------------------------------------------------
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
        std::string_view keyword = keyword_map[i].keyword;
        if (keyword.empty()) {
            continue;
        }

        if (!allow_keyword && (i & 0x40) != 0) {
            continue; // skip function keywords if not allowed
        }

        size_t len = keyword.size();
        if (strn_case_cmp(*p, keyword.data(), len) == 0) {
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

void Keyboard::key_down(kbd_state_t key) {
    key_state |= key;
    if ((key & K_SHIFT) != 0) {
        shift_state = K_SHIFT;
    }
}

void Keyboard::key_up(kbd_state_t key) {
    key_state &= ~key;
    if ((key & K_SHIFT) != 0) {
        shift_state = 0;
    }
}

byte_t Keyboard::in(word_t addr) {
    // high byte of the address has the selected row with bit=0
    byte_t row = HIGH_BYTE(addr);

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
void Keyboard::scan(Milliseconds elapsed_ms, Memory& memory) {
    // only scan the keyboard if there are no pending events in the queue
    // avoid key losses during paste, as a 0xffff LAST_K is written over the
    // LAST_K from the pushed key
    if (!queue_busy()) {
        word_t last_k = rom_keyboard_scan();
        memory.write16(LAST_K, last_k);
    }
    else {
        // push the next key_down/key_up from the queue
        queue_tick(elapsed_ms);
    }
}

bool Keyboard::queue_busy() const {
    return !queue.empty() || q_state != Q_IDLE;
}

void Keyboard::queue_tick(Milliseconds elapsed_ms) {
    if (elapsed_ms - q_event_ms < QueueTimeMs) {
        return; // not enough time has passed
    }
    q_event_ms = elapsed_ms;

    if (q_state == Q_IDLE) {
        if (!queue.empty()) {
            last_key_pushed = queue.front();
            queue.pop();
            key_down(last_key_pushed);
            q_state = Q_DOWN;
        }
    }
    else if (q_state == Q_DOWN) {
        key_up(last_key_pushed);
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
        a = in(TO_WORD(row, 0xFE));

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

    return TO_WORD(h, l);
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
