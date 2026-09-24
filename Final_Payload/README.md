# Challenge 06: Final_Payload - Real-World CTF Exploitation

**Difficulty**: ★★★★★ Expert  
**Binary**: `final_payload`  
**Port**: 1342  
**Category**: Full-Chain Binary Exploitation / Realistic Application

## Description

A realistic "Secure Notes Manager" application with menu-driven interface. No source code provided in real scenario. Full protections enabled. Custom libc/ld provided for exact environment matching.

## Binary Protections

```bash
$ checksec --file=final_payload
Arch:     amd64-64-little
RELRO:    Full RELRO
Stack:    Canary found
NX:       NX enabled
PIE:      PIE enabled
```

- **Stack Canary** (-fstack-protector-strong)
- **PIE** (-pie -fPIE) - Binary base randomized
- **Full RELRO** (-Wl,-z,relro,-z,now) - GOT read-only
- **NX** - Stack/heap not executable
- **ASLR** - All bases randomized

## Provided Files

- `final_payload` - The stripped binary
- `libc.so.6` - Exact glibc from Ubuntu 24.04 (glibc 2.39)
- `ld-linux-x86-64.so.2` - Dynamic linker
- Binary patched with `patchelf` to use provided libc/ld

## Vulnerability Analysis

The `edit_note()` function has a heap overflow:
```c
void edit_note(void)
{
    // ... index validation ...
    printf("Enter new content: ");
    read_bytes(notes[idx].content, 256);  // Reads 256 bytes regardless of allocation size!
}
```

Notes are allocated with `malloc(size)` where size ≤ 128 (NOTE_SIZE).
But `edit_note` reads 256 bytes into the chunk → **heap overflow**.

Additional attack surface:
- Use-after-free in `delete_note()` (doesn't zero pointer)
- Heap metadata corruption
- Potential for arbitrary write via heap manipulation

## Professional Exploitation Workflow

### 1. File Recon
```bash
$ file final_payload
$ checksec --file=final_payload
```

### 2. Strings & Behavior
```bash
$ strings -n 6 final_payload
$ ./final_payload  # Interact with menu
```

### 3. Reverse Engineering (Ghidra/IDA/GDB)
- Map all menu options
- Find where user input is stored
- Identify unbounded reads
- Locate heap management

### 4. Vulnerability Discovery
- Heap overflow in `edit_note()` (256-byte read into ≤128-byte chunk)
- Use-after-free in `delete_note()`
- No win() function, no obvious leaks

### 5. Exploit Strategy (Heap Exploitation)

**Phase 1: Leak libc address**
- Use UAF to read freed chunk metadata (fd/bk pointers in unsorted bin)
- Or overflow into adjacent chunk to leak pointers

**Phase 2: Leak heap address**
- Use UAF or overflow to read heap pointers

**Phase 3: Arbitrary write**
- House of Force / House of Einherjar / Tcache poisoning
- Overwrite `__free_hook` or `__malloc_hook` (if not RELRO)
- With Full RELRO: ROP via stack pivot or `__exit_funcs`

**Phase 4: ROP to system("/bin/sh")**
- Calculate libc base from leak
- Build ROP chain with provided libc gadgets
- Pivot stack if needed

### 6. Environment Matching

```bash
# Already done via patchelf in Makefile
patchelf --set-interpreter ./ld-linux-x86-64.so.2 --set-rpath \$ORIGIN ./final_payload

# Verify
$ ldd final_payload
	linux-vdso.so.1 =>  (0x00007ff...)
	libc.so.6 => ./libc.so.6 (0x00007f...)
	./ld-linux-x86-64.so.2 => ./ld-linux-x86-64.so.2 (0x00007f...)
```

### 7. Exploit Development (High-Level)

```python
from pwn import *

# Use provided libc
libc = ELF('./libc.so.6')
elf = ELF('./final_payload')

p = process('./final_payload')
# p = remote('localhost', 1342)

# Helper functions for menu interaction
def create(size, content):
    p.sendlineafter(b'Choice: ', b'1')
    p.sendlineafter(b'Enter note size', str(size).encode())
    p.sendafter(b'Enter note content: ', content)

def view(idx):
    p.sendlineafter(b'Choice: ', b'2')
    p.sendlineafter(b'Enter note index', str(idx).encode())
    p.recvuntil(b'Note ')
    return p.recvline()

def edit(idx, content):
    p.sendlineafter(b'Choice: ', b'3')
    p.sendlineafter(b'Enter note index', str(idx).encode())
    p.sendafter(b'Enter new content: ', content)

def delete(idx):
    p.sendlineafter(b'Choice: ', b'4')
    p.sendlineafter(b'Enter note index', str(idx).encode())

# Exploit steps:
# 1. Create notes to arrange heap
# 2. Trigger UAF or overflow to leak libc/heap
# 3. Calculate bases
# 4. Achieve arbitrary write
# 5. Execute ROP chain

p.interactive()
```

## Key Techniques for This Challenge

- **Heap Feng Shui**: Arrange chunks for controlled overflow/UAF
- **Unsorted Bin Attack**: Leak libc via fd/bk pointers
- **Tcache Poisoning**: Arbitrary write via tcache fd pointer
- **House of Force**: Extend top chunk for arbitrary allocation
- **Stack Pivot**: Move RSP to controlled heap area for ROP
- **ROP with Full RELRO**: Use `__libc_csu_init` or ret2dlresolve

## Flag

Successful exploit gives interactive shell. Read flag.txt.

## Progressive Hints

1. Start with reconnaissance. Run `checksec`. What protections are enabled?
2. Interact with the menu. Which option has unbounded input?
3. The binary uses heap (malloc/free). What heap vulnerabilities exist?
4. Use the provided libc/ld - patch the binary to match remote exactly.
5. Leak libc first (unsorted bin), then heap, then build arbitrary write.
6. With Full RELRO, GOT is read-only. Target `__free_hook` (if available) or ROP.
7. Remember 16-byte stack alignment for `system()` calls.

## Resources

- [How2Heap](https://github.com/shellphish/how2heap) - Heap exploitation techniques
- [Heap Exploitation](https://heap-exploitation.dhavalkapil.com/)
- [pwntools ROP](https://docs.pwntools.com/en/stable/rop/rop.html)