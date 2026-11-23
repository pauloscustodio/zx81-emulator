//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "clock.h"
#include "cpu.h"
#include "disasm.h"
#include "memory.h"
#include "status.h"
#include "utils.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

static constexpr int RegistersRow = 0;
static constexpr int RegistersCol = 0;
static constexpr int RegistersWidth = 19;
static constexpr int RegistersHeight = 11;
static constexpr int StackRow = RegistersRow;
static constexpr int StackCol = RegistersCol + RegistersWidth - 1;
static constexpr int StackWidth = 13;
static constexpr int StackHeight = RegistersHeight;
static constexpr int SysvarsRow = RegistersRow;
static constexpr int SysvarsCol = StackCol + StackWidth - 1;
static constexpr int SysvarsWidth = 50;
static constexpr int SysvarsHeight = RegistersHeight;
static constexpr int DataRow = RegistersRow + RegistersHeight;
static constexpr int DataCol = 0;
static constexpr int DataWidth = 31;
static constexpr int DataHeight = 8;
static constexpr int CodeRow = DataRow;
static constexpr int CodeCol = DataCol + DataWidth - 1;
static constexpr int CodeWidth = 50;
static constexpr int CodeHeight = DataHeight;
static constexpr int WatchRow = DataRow + DataHeight;
static constexpr int WatchCol = 0;
static constexpr int WatchWidth = 31;
static constexpr int WatchHeight = 5;
static constexpr int StatusRow = WatchRow;
static constexpr int StatusCol = WatchCol + WatchWidth - 1;
static constexpr int StatusWidth = 50;
static constexpr int StatusHeight = WatchHeight;

AsciiScreen::AsciiScreen() {
    memset(cells, ' ', sizeof(cells));
    for (int r = 0; r < NumRows; ++r) {
        cells[r][NumCols] = '\0';
    }
}

void AsciiScreen::put(int r, int c, const char* text) {
    release_assert(r >= 0 && r < NumRows);
    release_assert(c >= 0 && c + strlen(text) <= NumCols);
    for (int i = 0; i < INT(strlen(text)); ++i) {
        cells[r][c + i] = text[i];
    }
}

void AsciiScreen::putf(int r, int c, const char* fmt, ...) {
    char buffer[NumCols + 1];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    put(r, c, buffer);
}

void AsciiScreen::draw_horizontal_line(int r, int c, int w, char ch) {
    release_assert(r >= 0 && r < NumRows);
    release_assert(c >= 0 && c + w < NumCols);
    for (int i = 0; i < w; ++i) {
        cells[r][c + i] = ch;
    }
}

void AsciiScreen::draw_vertical_line(int r, int c, int h, char ch) {
    release_assert(r >= 0 && r + h < NumRows);
    release_assert(c >= 0 && c < NumCols);
    for (int i = 0; i < h; ++i) {
        cells[r + i][c] = ch;
    }
}

void AsciiScreen::draw_box(int r, int c, int w, int h, const char* title) {
    put(r, c, "+");
    draw_horizontal_line(r, c + 1, w - 2, '=');
    put(r, c + w - 1, "+");
    draw_vertical_line(r + 1, c, h - 2, '|');
    draw_vertical_line(r + 1, c + w - 1, h - 2, '|');
    put(r + h - 1, c, "+");
    draw_horizontal_line(r + h - 1, c + 1, w - 2, '-');
    put(r + h - 1, c + w - 1, "+");
    put(r, c + 2, title);
}

std::string AsciiScreen::get_image() const {
    std::string result;
    for (int r = 0; r < NumRows; ++r) {
        result += cells[r];
        result += '\n';
    }
    return result;
}

StatusScreen::StatusScreen(const Clock& clock, const Memory& memory,
                           const SymbolTable& symtab, const Disassembler& disasm)
    : clock(clock), memory(memory), symtab(symtab), disasm(disasm) {
}

void StatusScreen::update(const Registers& regs, word_t data_addr,
                          const std::vector<word_t>& watchpoints, Cycles cycles_per_frame, bool slow) {
    update_registers(regs);
    update_stack(regs.sp);
    update_sysvars();
    update_memory(data_addr);
    update_disassembly(regs.pc);
    update_watchpoints(watchpoints);
    update_status(cycles_per_frame, slow);
}

void StatusScreen::update_registers(const Registers& regs) {
    auto format_flags = [](word_t af) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%c%c%c%c%c%c%c%c",
                 (af & 0x80) ? 'S' : '.',
                 (af & 0x40) ? 'Z' : '.',
                 (af & 0x20) ? '5' : '.',
                 (af & 0x10) ? 'H' : '.',
                 (af & 0x08) ? '3' : '.',
                 (af & 0x04) ? 'P' : '.',
                 (af & 0x02) ? 'N' : '.',
                 (af & 0x01) ? 'C' : '.');
        std::string flags(buffer);
        return flags;
    };
    draw_box(RegistersRow, RegistersCol, RegistersWidth, RegistersHeight,
             "Registers");
    putf(RegistersRow + 1, RegistersCol + 1, "AF  %04x|AF' %04x", regs.af,
         regs.af_);
    putf(RegistersRow + 2, RegistersCol + 1, "%s|%s",
         format_flags(regs.af).c_str(), format_flags(regs.af_).c_str());
    putf(RegistersRow + 3, RegistersCol + 1, "BC  %04x|BC' %04x", regs.bc,
         regs.bc_);
    putf(RegistersRow + 4, RegistersCol + 1, "DE  %04x|DE' %04x", regs.de,
         regs.de_);
    putf(RegistersRow + 5, RegistersCol + 1, "HL  %04x|HL' %04x", regs.hl,
         regs.hl_);
    putf(RegistersRow + 6, RegistersCol + 1, "IX  %04x|I     %02x", regs.ix,
         regs.i);
    putf(RegistersRow + 7, RegistersCol + 1, "IY  %04x|R     %02x", regs.iy,
         regs.r);
    putf(RegistersRow + 8, RegistersCol + 1, "PC  %04x|IM     %d", regs.pc,
         regs.im);
    putf(RegistersRow + 9, RegistersCol + 1, "SP  %04x|IFF  %d %d", regs.sp,
         regs.iff1, regs.iff2);
}

void StatusScreen::update_stack(word_t sp) {
    draw_box(StackRow, StackCol, StackWidth, StackHeight, "Stack");
    for (int i = 0; i < StackHeight - 2; i++) {
        word_t addr = WORD(sp - 2 * i);
        putf(StackRow + StackHeight - 2 - i, StackCol + 1, "%04x: %c%04x",
             addr, addr == sp ? '>' : ' ', memory.read16(addr));
    }
}

void StatusScreen::update_sysvars() {
    auto byte = [&](int addr) {
        return memory.read8(addr);
    };
    auto word = [&](int addr) {
        return memory.read16(addr);
    };
    draw_box(SysvarsRow, SysvarsCol, SysvarsWidth, SysvarsHeight,
             "System Variables");
    putf(SysvarsRow + 1, SysvarsCol + 1,
         "ERR_NR   %02x|DF_CC  %04x|MEM    %04x|STRLEN %04x",
         byte(ERR_NO), word(DF_CC), word(MEM), word(STRLEN));
    putf(SysvarsRow + 2, SysvarsCol + 1,
         "FLAGS    %02x|VARS   %04x|DF_SZ    %02x|T_ADDR %04x",
         byte(FLAGS), word(VARS), byte(DF_SZ), word(T_ADDR));
    putf(SysvarsRow + 3, SysvarsCol + 1,
         "ERR_SP %04x|DEST   %04x|S_TOP  %04x|SEED   %04x",
         word(ERR_SP), word(DEST), word(S_TOP), word(SEED));
    putf(SysvarsRow + 4, SysvarsCol + 1,
         "RAMTOP %04x|E_LINE %04x|LAST_K %04x|FRAMES %04x",
         word(RAMTOP), word(E_LINE), word(LAST_K), word(FRAMES));
    putf(SysvarsRow + 5, SysvarsCol + 1,
         "MODE     %02x|CH_ADD %04x|DB_ST    %02x|COORDS %04x",
         byte(MODE), word(CH_ADD), byte(DB_ST), word(COORDS));
    putf(SysvarsRow + 6, SysvarsCol + 1,
         "PPC    %04x|X_PTR  %04x|MARGIN   %02x|PR_CC    %02x",
         word(PPC), word(X_PTR), byte(MARGIN), byte(PR_CC));
    putf(SysvarsRow + 7, SysvarsCol + 1,
         "VERSN    %02x|STKBOT %04x|NXTLIN %04x|S_POSN %04x",
         byte(VERSN), word(STKBOT), word(NXTLIN), word(S_POSN));
    putf(SysvarsRow + 8, SysvarsCol + 1,
         "E_PPC  %04x|STKEND %04x|OLDPPC %04x|CDFLAG   %02x",
         word(E_PPC), word(STKEND), word(OLDPPC), byte(CDFLAG));
    putf(SysvarsRow + 9, SysvarsCol + 1,
         "D_FILE %04x|BREG     %02x|FLAGX    %02x|           ",
         word(D_FILE), byte(BREG), byte(FLAGX));
}

void StatusScreen::update_memory(word_t data_addr) {
    auto byte = [&](int addr) {
        return memory.read8(addr);
    };
    draw_box(DataRow, DataCol, DataWidth, DataHeight, "Memory");
    for (size_t i = 0; i < 8 * (DataHeight - 2); i += 8) {
        putf(DataRow + 1 + INT(i) / 8, DataCol + 1,
             "%04x: %02x %02x %02x %02x %02x %02x %02x %02x",
             WORD(data_addr + i),
             byte(WORD(data_addr + i)),
             byte(WORD(data_addr + i + 1)),
             byte(WORD(data_addr + i + 2)),
             byte(WORD(data_addr + i + 3)),
             byte(WORD(data_addr + i + 4)),
             byte(WORD(data_addr + i + 5)),
             byte(WORD(data_addr + i + 6)),
             byte(WORD(data_addr + i + 7)));
    }
}

void StatusScreen::update_disassembly(word_t pc) {
    draw_box(CodeRow, CodeCol, CodeWidth, CodeHeight, "Disassembly");
    word_t addr = pc;
    for (int i = 0; i < CodeHeight - 2; ++i) {
        AsmInstr instr = disasm.disassemble(addr);
        std::string formatted = disasm.format(instr, pc);
        put(CodeRow + 1 + i, CodeCol + 1, formatted.c_str());
        addr += instr.size;
    }
}

void StatusScreen::update_watchpoints(const std::vector<word_t>& watchpoints) {
    char buffer_label[16], buffer[64];

    draw_box(WatchRow, WatchCol, WatchWidth, WatchHeight, "Watchpoints");
    draw_vertical_line(WatchRow + 1, WatchCol + 15, WatchHeight - 2, '|');
    int num_rows = WatchHeight - 2;
    for (int i = 0; i < INT(watchpoints.size()) && i < 2 * num_rows; ++i) {
        int row = WatchRow + 1 + (i % num_rows);
        int col = WatchCol + 1 + (i / num_rows) * 15;

        word_t addr = watchpoints[i];
        word_t value = memory.read16(addr);
        const Symbol* symbol = symtab.get_first(addr);
        if (symbol != nullptr) {
            snprintf(buffer_label, sizeof(buffer_label), "%-10.10s", symbol->name.c_str());
        }
        else {
            snprintf(buffer_label, sizeof(buffer_label), "%04x      ", addr);
        }
        snprintf(buffer, sizeof(buffer), "%s%04x", buffer_label, value);
        putf(row, col, "%s", buffer);
    }
}

void StatusScreen::update_status(Cycles cycles_per_frame, bool slow) {
    draw_box(StatusRow, StatusCol, StatusWidth, StatusHeight, "Status");
    std::string cycles_str = fmt_with_commas(clock.get_cycles());
    std::string frames_str = fmt_with_commas(clock.get_frames());
    std::string cycles_per_frame_str = fmt_with_commas(cycles_per_frame);
    putf(StatusRow + 1, StatusCol + 1, "Cycles%18s|", cycles_str.c_str());
    putf(StatusRow + 2, StatusCol + 1, "Frames%18s|", frames_str.c_str());
    putf(StatusRow + 3, StatusCol + 1, "Cycles/Frame%12s|",
         cycles_per_frame_str.c_str());
    putf(StatusRow + 1, StatusCol + 26, "Mode%19s|", slow ? "SLOW" : "FAST");
}
