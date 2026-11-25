# ZX81 Emulator

A lightweight ZX81 emulator designed for fast startup and for use as a test backend in BASIC and decompilation workflows.

## Project intent and origin

This project is based on the ZX81 emulation approach described in:

https://leiradel.github.io/2011/06/11/How-to-Emulate.html

It was later adapted and simplified for a practical goal: run a ZX81 program quickly, without the overhead and extra hardware features of a full-blown emulator such as EightyOne.

The current development focus is intentionally narrow:

- fast startup and low overhead
- easy automation with BASIC files and state dumps
- sufficient compatibility for ZX81 text-mode programs and decompiler testing
- no attempt to emulate every hardware quirk of the original display system

## License

This project is released under the GNU General Public License, version 2 or later (GPL-2.0-or-later), as indicated in the source headers and project files.

## Command-line usage

```bash
zx81 [-d port] [-h] [-s] [file.p|file.dump|file.bas]
```

Options:

- `-d port`: starts a TCP listener on the given port; on the client side connect
  for example with `nc` and type `help`:
```
    $ ./zx81 -d 8112
    $ nc localhost 8112
    Welcome to the ZX81 Emulator Debugger!
    > help
    Available commands:
    [h]elp   - Show this help message
    [q]uit   - Quit the debugger
    [t]ype   - Type text on the machine
    [c]ont   - Continue execution
    [S]top   - Stop execution
    [s]tep   - Step one instruction
    [n]next  - Step over one instruction
    [r]eturn - Step out of current function
    [b]reak $xx|dd|NAME - Add breakpoint
    [R]ead  $xx|dd|NAME - Add break on read
    [W]rite $xx|dd|NAME - Add break on write
    [d]el   $xx|dd|NAME - Delete breakpoints
    [w]atch $xx|dd|NAME - Add watchpoint
    [m]em   $xx|dd|NAME - Show memory contents
```
- `-h`: run in headless mode, i.e. the emulator runs but the main window
  is not created. Usefull for batch execution and CI runs.
- `-s`: run at normal ZX81 speed when the emulator is not in FAST mode.
- `file.p`: load a BASIC program file on startup, as if `LOAD "file"` had been typed.
- `file.dump`: restore emulator state from a saved dump.
- `file.bas`: paste the file as keystrokes into the BASIC editor.

Examples:

```bash
zx81 hello.p
zx81 -s hello.p
zx81 saved.dump
zx81 input.bas
```

## Commands during usage

When the emulator is running, these host-side controls are available:

- `F1`: show a keyboard overlay below the main window
- `F6`: save emulator state to `zx81.dump`
- `F7`: restore emulator state from `zx81.dump`
- `Esc`: save the current state to `zx81.dump` and exit immediately
- `Ctrl+C`: copy the screen image to the clipboard
- `Ctrl+V`: paste clipboard text into the emulator as BASIC input

Inside the emulated ZX81 environment:

- `USR 8191`: exit the emulator
- `USR 8190`: dump memory and CPU state to `zx81.dump`

## Features

- very fast startup compared with heavier desktop emulators
- BASIC program loading from `.p` files and state restoration from `.dump` files
- save/restore snapshots using `F6` and `F7`
- `SAVE "X"` writes a `.p` file
- `LOAD "X"` reads a `.p` file
- `PAUSE` supports timed frame delays
- `LPRINT`, `LLIST`, and `COPY` write output to `zx81.txt`
- optional slow-mode emulation with `-s`
- dynamic font handling when register `I` changes
- `CHR$128` support for alternate character pages
- basic clipboard integration for copying and pasting text

## Scope and limitations

This emulator focuses on the default text-based ZX81 screen and quick program execution. It does not aim to reproduce the full hardware display system in every detail.

## Technology

C++17, SDL3, SDL_Image

## Further work

See the issue tracker for planned improvements and extensions:

https://github.com/pauloscustodio/Simple-ZX81-Emulator/issues/
