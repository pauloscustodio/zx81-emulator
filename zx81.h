//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string_view>

using byte_t = uint8_t;
using word_t = uint16_t;

#define TO_WORD(high, low) (static_cast<word_t>(((high) << 8) | (low)))
#define HIGH_BYTE(word) (static_cast<byte_t>(((word) >> 8) & 0xFF))
#define LOW_BYTE(word) (static_cast<byte_t>((word) & 0xFF))

#define BYTE(x) (static_cast<byte_t>(x))
#define WORD(x) (static_cast<word_t>(x))
#define INT(x) (static_cast<int>(x))

// App name and window title
static inline constexpr std::string_view WindowTitle = "ZX81 Emulator";

// name of files
static inline constexpr std::string_view DumpFilename 	= "zx81.dump";
static inline constexpr std::string_view LPrintFilename = "zx81.txt";

// machine constants
static inline constexpr int MemSize  = 0x10000;
static inline constexpr int NumRows  = 24;
static inline constexpr int NumCols  = 32;
static inline constexpr int SlowMode = 0x40;
static inline constexpr int FastMode = 0x00;

// clock and interrups
static inline constexpr int ClockHz   = 3250000;
static inline constexpr int RefreshHz = 50;
static inline constexpr int ClocksPerFrameFast = ClockHz / RefreshHz;
static inline constexpr int ClocksPerFrameSlow = ClocksPerFrameFast / 4;

// system variables
static inline constexpr int SAVE_ADDR = 0x4009;

static inline constexpr int ERR_NO  = 0x4000;
static inline constexpr int FLAGS   = 0x4001;
static inline constexpr int ERR_SP  = 0x4002;
static inline constexpr int RAMTOP  = 0x4004;
static inline constexpr int MODE    = 0x4006;
static inline constexpr int PPC     = 0x4007;
static inline constexpr int VERSN   = 0x4009;
static inline constexpr int E_PPC   = 0x400a;
static inline constexpr int D_FILE  = 0x400c;
static inline constexpr int DF_CC   = 0x400e;
static inline constexpr int VARS    = 0x4010;
static inline constexpr int DEST    = 0x4012;
static inline constexpr int E_LINE  = 0x4014;
static inline constexpr int CH_ADD  = 0x4016;
static inline constexpr int X_PTR   = 0x4018;
static inline constexpr int STKBOT  = 0x401a;
static inline constexpr int STKEND  = 0x401c;
static inline constexpr int BREG    = 0x401e;
static inline constexpr int MEM     = 0x401f;
static inline constexpr int FREE1   = 0x4021;
static inline constexpr int DF_SZ   = 0x4022;
static inline constexpr int S_TOP   = 0x4023;
static inline constexpr int LAST_K  = 0x4025;
static inline constexpr int DBOUNCE = 0x4027;
static inline constexpr int MARGIN  = 0x4028;
static inline constexpr int NXTLIN  = 0x4029;
static inline constexpr int OLDPPC  = 0x402b;
static inline constexpr int FLAGX   = 0x402d;
static inline constexpr int STRLEN  = 0x402e;
static inline constexpr int T_ADDR  = 0x4030;
static inline constexpr int SEED    = 0x4032;
static inline constexpr int FRAMES  = 0x4034;
static inline constexpr int COORDS  = 0x4036;
static inline constexpr int COORDX  = 0x4036;
static inline constexpr int COORDY  = 0x4037;
static inline constexpr int PR_CC   = 0x4038;
static inline constexpr int S_POSN  = 0x4039;
static inline constexpr int S_POSNC = 0x4039;
static inline constexpr int S_POSNL = 0x403a;
static inline constexpr int CDFLAG  = 0x403b;
static inline constexpr int PRBUFF  = 0x403c;
static inline constexpr int MEMBOT  = 0x405d;
static inline constexpr int FREE2   = 0x407b;

// Hook to exit the emulator
static inline constexpr int EXIT_HOOK = 8191;

// Hook to dump memory and z80 state to a file
static inline constexpr int DUMP_HOOK = 8190;

// ROM routines
static inline constexpr int  PRINT_A   = 0x0010;
static inline constexpr int  PRINT_CH  = 0x07f1;
static inline constexpr int  PRINT_SP  = 0x07f5;
static inline constexpr int  DISPLAY_5 = 0x02b5;
static inline constexpr int  KEYBOARD  = 0x02bb;

// SAVE "FILE" after name has been parsed
static inline constexpr int  SAVE1     = 0x02fc;

static inline constexpr int  NEXT_PROG = 0x0347;
static inline constexpr int  DISPLAY_6 = 0x04c1;
static inline constexpr int  SLOW_DISP = 0x04cf;
static inline constexpr int  FAST      = 0x0f23;
static inline constexpr int  SLOW      = 0x0f2b;

// PAUSE after delay has been parsed and is in BC
static inline constexpr int  PAUSE1    = 0x0f35;

static inline constexpr int  SCROLL    = 0x0c0e;
static inline constexpr int  COPY	   = 0x0869;
static inline constexpr int  CHARSET   = 0x1e00;
