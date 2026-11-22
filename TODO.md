TODO
====
- run a headless emulator
- allow single-stepping of BASIC program, 
  inspection and change of BASIC variables
- add a line editing function
- pressing the real keyboard aborts queue push
- very long edit lines loose keys during paste
- accept extended SAVE and LOAD commands from ZXpand

+----------------+
| SDL Frontend   |
+----------------+
        |
        v
+----------------+
| Machine        |
|  CPU           |
|  Memory        |
|  ULA           |
|  ROM           |
+----------------+
        ^
        |
+----------------+
| IDebugTarget   |
+----------------+
        ^
        |
+----------------+
| Debugger       |
|  command parser|
|  formatting    |
|  breakpoints   |
+----------------+
        ^
        |
+----------------+
| DebugServer    |
| TCP socket     |
+----------------+

TerminalView
    |
    +--> machine.state()

+----------------------+
| SDL Frontend         |
| windows              |
| keyboard overlay     |
| menu                 |
+----------------------+

+----------------------+
| Debug Frontend       |
| command parser       |
| terminal rendering   |
| network protocol     |
+----------------------+

+----------------------+
| Emulator Core        |
| Machine              |
| CPU                  |
| Memory               |
| ULA                  |
| ROM                  |
+----------------------+

int main()
{
    Machine machine;
    DebugServer debugger(machine);
    Screen screen(machine);

    Application app(machine, screen, debugger);

    app.run();
}

class IMachineUI
{
public:
    virtual const uint8_t* framebuffer() const = 0;

    virtual void key_down(ZX81Key k) = 0;
    virtual void key_up(ZX81Key k) = 0;

    virtual ~IMachineUI() = default;
};

class Machine :
    public IMachineUI,
    public IDebugTarget
{
};


class Machine
{
public:
    void execute_tstates(int count);

    FrameBuffer const& frame_buffer();

    void press_key(...);
    void release_key(...);
};

class Screen
{
public:
    void run()
    {
        while(running)
        {
            handle_events();

            machine.execute_tstates(frame_budget);

            render();
        }
    }
};

class Machine
{
public:
    void key_down(...);
    void key_up(...);

    void execute_frame();

    const FrameBuffer& framebuffer() const;
};

class Screen
{
public:
    explicit Screen(Machine& machine);
};



int main()
{
    Machine machine;
    DebugServer debugger(machine);
    Screen screen(machine);

    screen.run();
}



// exported machine state for debugger
struct MachineState {
    Registers regs;
    uint64_t ticks = 0;

    std::vector<std::string> disassembly;
    std::vector<uint16_t> stack;
};

// events for the SDL event loop
enum class MachineEventType {
    KeyPressed,
    KeyReleased,
    Quit,
    Reset,
};

struct MachineEvent {
    MachineEventType type;
    int key;
};



// interface implemented by the debug target (the machine)
class IDebugTarget {
public:
    virtual Registers get_registers() const = 0;
    virtual uint8_t read_memory8(word_t addr) const = 0;
    virtual uint16_t read_memory16(word_t addr) const = 0;
    virtual void write_memory8(word_t addr, uint8_t value) = 0;
    virtual void write_memory16(word_t addr, uint16_t value) = 0;

    virtual void step() = 0;
    virtual void continue_execution() = 0;
    virtual void break_execution() = 0;

    virtual ~IDebugTarget() = default;
};

// breakpoint manager that interacts with the debug target
class BreakpointManager {
};

// debugger class that interacts with the debug target
class Debugger {
public:
    explicit Debugger(IDebugTarget& target);

    std::string execute(const std::string& command);

private:
    IDebugTarget& target;
};


