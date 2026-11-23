//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include "cpu.h"
#include "disasm.h"
#include "memory.h"
#include "zx81.h"
#include <string>
#include <vector>

/*
+=Registers=======+=Stack=====+=System Variables===============================+
|AF  1234|AF' 1234|7ffe:  0001|ERR_NR   34|DF_CC  1234|MEM    1234|STRLEN 1234 |
|SZ5H3PNC|SZ5H3PNC|7ffc:  0203|FLAGS    34|VARS   1234|DF_SZ    34|T_ADDR 1234 |
|BC  5678|BC' 5678|7ffa:  0405|ERR_SP 1234|DEST   1234|S_TOP  1234|SEED   1234 |
|DE  9abc|DE' 9abc|7ff8:  0607|RAMTOP 1234|E_LINE 1234|LAST_K 1234|FRAMES 1234 |
|HL  def0|HL' def0|7ff6:  0809|MODE     34|CH_ADD 1234|DB_ST    34|COORDS 1234 |
|IX  1234|I     12|7ff4:  0a0b|PPC    1234|X_PTR  1234|MARGIN   34|PR_CC    34 |
|IY  1234|R     34|7ff2:  0c0d|VERSN    34|STKBOT 1234|NXTLIN 1234|S_POSN 1234 |
|PC  5678|IM     2|7ff0:  0e0f|E_PPC  1234|STKEND 1234|OLDPPC 1234|CDFLAG   34 |
|SP  7ff0|IFF  0 0|7fee:> 0001|D_FILE 1234|BREG     34|FLAGX    12|            |
+-----------------+-----------+------------------------------------------------+
+=Memory======================+=Disassembly====================================+
|4000: 00 01 02 03 04 05 06 07|0000: d3 fd      >START     out (#fd),a         |
|4008: 00 01 02 03 04 05 06 07|0002: 01 ff 7f              ld bc,#7fff         |
|4010: 00 01 02 03 04 05 06 07|0005: c3 cb 03              jp RAM_CHECK        |
|4018: 00 01 02 03 04 05 06 07|0008: 2a 16 40              ld hl,(CH_ADD)      |
|4020: 00 01 02 03 04 05 06 07|000B: 22 18 40              ld (X_PTR),hl       |
|4028: 00 01 02 03 04 05 06 07|000E: 18 46                 jr ERROR_2          |
+-----------------------------+------------------------------------------------+
+=Watch=======================+=Status=========================================|
|D_FILE    1234|LIVES     1234|Cycles     2,147,483,648|Mode               FAST|
|4127      2345|6745      1234|Frames     2,147,483,648|STOP                   |
|4127      2345|6745      1234|Cycles/Frame      64,000|BREAK @ 0000           |
+-----------------------------+------------------------------------------------+
*/

class AsciiScreen {
public:
    AsciiScreen();

    static inline constexpr int NumRows = 24;
    static inline constexpr int NumCols = 80;

    void put(int r, int c, const char* text);
    void putf(int r, int c, const char* fmt, ...);
    void draw_horizontal_line(int r, int c, int w, char ch);
    void draw_vertical_line(int r, int c, int h, char ch);
    void draw_box(int r, int c, int w, int h, const char* title);
    std::string get_image() const;

private:
    char cells[NumRows][NumCols + 1];
};

class StatusScreen : public AsciiScreen {
public:
    StatusScreen(const Clock& clock, const Memory& memory,
                 const SymbolTable& symtab, const Disassembler& disasm);

    void update(const Registers& regs, word_t data_addr,
                const std::vector<word_t>& watchpoints, Cycles cycles_per_frame, bool slow);
    void update_registers(const Registers& regs);
    void update_stack(word_t sp);
    void update_sysvars();
    void update_memory(word_t data_addr);
    void update_disassembly(word_t pc);
    void update_watchpoints(const std::vector<word_t>& watchpoints);
    void update_status(Cycles cycles_per_frame, bool slow);

private:
    const Clock& clock;
    const Memory& memory;
    const SymbolTable& symtab;
    const Disassembler& disasm;
};
