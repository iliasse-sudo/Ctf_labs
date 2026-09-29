================================================================================
BUFFER OVERFLOW FUNDAMENTALS — A HANDS-ON INTRODUCTION
================================================================================
Level      : Beginner
Topic      : Memory Corruption / Stack Buffer Overflow
Binary     : oopsie (compiled from main.c)
================================================================================

[1. WHY THIS MATTERS]
--------------------------------------------------------------------------------
In high-level languages (Python, JavaScript, Java), arrays and strings manage
their own memory. Try to access index 100 of a 5-element list? You get an
exception. Append to a string? It grows automatically.

In C, memory is manual. If you declare `char buffer[32]` and write 100 bytes
into it, the extra 68 bytes don't vanish — they spill into adjacent memory.
This is a **stack buffer overflow**, the vulnerability that launched the entire
field of binary exploitation.

This guide uses a tiny program (`oopsie`) to show exactly what happens on the
stack when input exceeds its buffer.

[2. THE VULNERABLE CODE]
--------------------------------------------------------------------------------
File: main.c

    #include <stdio.h>
    #include <string.h>

    void vulnerable(void)
    {
        char    buffer[32];

        printf("Enter your name: ");
        memset(buffer, 0, 32);
        gets(buffer);
        printf("Hello %s\n", buffer);
    }

    int main(void)
    {
        setvbuf(stdout, NULL, _IONBF, 0);
        vulnerable();
        return 0;
    }

Key observations:
  1. `buffer` is exactly 32 bytes on the stack.
  2. `gets(buffer)` reads from stdin until newline or EOF — **with no limit**.
  3. `memset` zeroes the buffer first, but that doesn't protect against overflow.

`gets()` was removed from the C standard (C11) because it is fundamentally
unsafe. It remains in many codebases for backward compatibility.

[3. STACK FRAME LAYOUT]
--------------------------------------------------------------------------------
When `vulnerable()` is called, the CPU builds a stack frame:

    Higher Addresses (growing "down" on x86-64)
    +---------------------------+
    | Saved RIP (return addr)  |  8 bytes  <-- Overwritten at offset 40
    +---------------------------+
    | Saved RBP (base pointer) |  8 bytes  <-- Overwritten at offset 32
    +---------------------------+
    | buffer[0]                |  \
    | buffer[1]                |   |  32 bytes total
    | ...                      |   |  (buffer[0] at lowest address)
    | buffer[31]               |  /
    +---------------------------+
    Lower Addresses

The stack grows toward lower addresses. `gets()` writes upward from
`buffer[0]` toward higher addresses — directly toward saved RBP and RIP.

[4. WHAT HAPPENS WITH DIFFERENT INPUTS]
--------------------------------------------------------------------------------
Input: "Alice" (5 chars + null = 6 bytes)
  - Fits in buffer. RBP and RIP untouched. Program returns normally.

Input: 32 'A's
  - Fills buffer exactly. RBP and RIP still intact. Returns normally.

Input: 33 'A's
  - Buffer full. 1 byte spills into saved RBP (lowest byte).

Input: 40 'A's (32 + 8)
  - Buffer full. Saved RBP completely overwritten with 0x4141414141414141.
  - Saved RIP still intact. Function returns to correct address, but RBP
    is corrupted — may cause issues in caller.

Input: 48 'A's (32 + 8 + 8)
  - Buffer full. Saved RBP overwritten. **Saved RIP overwritten with
    0x4141414141414141.**
  - When `vulnerable()` executes `ret`, CPU pops this value into RIP.
  - CPU tries to execute at address 0x4141414141414141 — not mapped.
  - **Segmentation fault (SIGSEGV).**

Input: 60+ 'A's
  - Same crash, more overwritten data past RIP.

[5. HANDS-ON INVESTIGATION]
--------------------------------------------------------------------------------
Compile and run:
    $ gcc -o oopsie main.c
    $ ./oopsie

Test increasing input sizes:
    $ python3 -c "print('A'*20)" | ./oopsie
    $ python3 -c "print('A'*32)" | ./oopsie
    $ python3 -c "print('A'*33)" | ./oopsie
    $ python3 -c "print('A'*40)" | ./oopsie
    $ python3 -c "print('A'*48)" | ./oopsie
    $ python3 -c "print('A'*60)" | ./oopsie
    Segmentation fault (core dumped)

Inspect the crash with GDB:
    $ gdb ./oopsie
    (gdb) run
    Enter your name: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    Program received signal SIGSEGV, Segmentation fault.
    0x0000000000401234 in vulnerable ()

    (gdb) info registers
    rip            0x4141414141414141    0x4141414141414141
    rbp            0x4141414141414141    0x4141414141414141

Notice:
  - RIP = 0x4141414141414141 ('AAAAAAAA' in little-endian)
  - RBP = 0x4141414141414141
  - The crash happens because the return address was corrupted.

Examine the stack:
    (gdb) x/20gx $rsp
    (shows 20 quadwords from current stack pointer)

[6. CORE CONCEPTS]
--------------------------------------------------------------------------------
1. Stack Memory
   Local variables, saved registers, and return addresses share the same
   contiguous memory region. No hardware boundary separates them.

2. Buffer Boundaries
   `char buffer[32]` allocates exactly 32 bytes. The compiler emits no
   runtime checks. Writing past `buffer[31]` is undefined behavior.

3. Segmentation Fault
   Not a C error — a hardware exception (page fault) delivered by the OS
   when the CPU accesses unmapped or protected memory. Here, corrupted RIP
   points to invalid address 0x4141414141414141.

4. Unchecked Input
   `gets()` has no length parameter. It cannot be used safely. Modern
   replacements: `fgets(buffer, sizeof(buffer), stdin)`.

5. Stack Frame Structure
   Caller pushes return address. Callee pushes RBP, sets new RBP = RSP.
   Local variables at negative offsets from RBP. Return via `leave; ret`.

6. Little-Endian Byte Order
   0x4141414141414141 in memory appears as bytes: 41 41 41 41 41 41 41 41
   (least significant byte at lowest address). 'A' = 0x41.

[7. PROGRESSIVE EXPERIMENTS]
--------------------------------------------------------------------------------
Experiment 1: Find the exact crash threshold
  - Try 32, 33, 40, 41, 47, 48, 49 bytes.
  - At which length does the program segfault?

Experiment 2: Observe register corruption in GDB
  - Run with 48 'A's.
  - Check `info registers` — which registers hold 0x41...?

Experiment 3: Visualize the stack
  - In GDB: `x/24xb buffer` (examine 24 bytes from buffer start)
  - Then `x/24xb buffer+32` (examine saved RBP and RIP)

Experiment 4: Try non-crashing overflow
  - 40 'A's overwrites RBP but not RIP.
  - Program may appear to work but caller's frame is corrupted.

Experiment 5: Replace gets() with fgets()
  - Edit main.c: `fgets(buffer, sizeof(buffer), stdin);`
  - Recompile. Test with 100 'A's. No crash.

[8. KEY TAKEAWAYS]
--------------------------------------------------------------------------------
- Fixed-size buffers + unbounded input = overflow.
- The stack stores both data (buffer) and control flow (RIP) adjacently.
- Overwriting RIP redirects execution — the basis of code execution exploits.
- Modern mitigations (stack canaries, ASLR, NX, PIE) make exploitation
  harder, but the root cause remains: unchecked writes to fixed buffers.
- Safe coding: always use bounded functions (`fgets`, `snprintf`, `read`
  with explicit length).

================================================================================
FURTHER READING
================================================================----------------
- "Smashing the Stack for Fun and Profit" — Aleph One (Phrack 49)
- Computer Systems: A Programmer's Perspective (CS:APP) — Ch. 3
- CWE-121: Stack-based Buffer Overflow
================================================================================
