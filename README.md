# CTF Pwn Labs - Progressive Binary Exploitation

A structured learning path for binary exploitation, from basic buffer overflows to full-chain CTF exploitation.

## Challenges Overview

| # | Challenge | Binary | Port | Difficulty | Key Concept |
|---|-----------|--------|------|------------|-------------|
| 01 | Oopsie | `oopsie` | 1337 | Beginner | Stack Buffer Overflow (`gets`) |
| 02 | Returnal | `returnal` | 1338 | Easy | ret2win (Control Flow Hijacking) |
| 03 | R0P_Me | `r0p_me` | 1339 | Medium | ROP with Arguments (`pop rdi`) |
| 04 | Blind_Spot | `blind_spot` | 1340 | Medium | Information Leak & ASLR Bypass |
| 05 | House_of_Calls | `house_of_calls` | 1341 | Hard | ret2libc (`system("/bin/sh")`) |
| 06 | Final_Payload | `final_payload` | 1342 | Expert | Full-Chain Real-World CTF |

## Quick Start

```bash
# Build and start all challenges
docker-compose up -d --build

# Connect to a challenge (e.g., Oopsie)
nc localhost 1337

# Or run a specific challenge locally
cd Oopsie && make && ./oopsie

# View logs
docker-compose logs -f oopsie

# Stop all
docker-compose down
```

## Prerequisites

- Docker & Docker Compose
- `pwntools` (for exploit development): `pip3 install pwntools`
- `ROPgadget` / `ropper`: `pip3 install ROPgadget ropper`
- GDB with pwndbg/GEF: `gdb ./binary`

## Learning Path

### Challenge 01: Oopsie - Buffer Overflow Basics
- Understand stack layout and buffer boundaries
- Crash the program with oversized input
- Flag printed on segmentation fault

### Challenge 02: Returnal - ret2win
- Overwrite saved RIP to jump to `win()`
- Learn little-endian encoding
- Static addresses (no PIE)

### Challenge 03: R0P_Me - ROP with Arguments
- x86-64 calling convention (RDI = 1st arg)
- Find and use `pop rdi; ret` gadget
- Pass `0x13371337` to `win()`

### Challenge 04: Blind_Spot - ASLR Bypass
- Address Space Layout Randomization
- Parse leaked function pointer
- Calculate runtime base address

### Challenge 05: House_of_Calls - ret2libc
- No `win()` function - use libc
- Leak libc address via GOT
- 16-byte stack alignment for `system()`

### Challenge 06: Final_Payload - Real CTF
- Black-box analysis (stripped binary)
- Provided libc/ld for exact environment matching
- Full exploit chain: recon → leak → ROP → shell

## Tools Reference

```bash
# Binary analysis
file ./binary
checksec --file=./binary
strings -n 6 ./binary

# GDB debugging
gdb ./binary
(gdb) pattern create 100
(gdb) pattern offset $rsp

# ROP gadgets
ROPgadget --binary ./binary | grep "pop rdi"
ropper --file ./binary --search "pop rdi"

# Libc analysis
readelf -s libc.so.6 | grep system
```

## Flag Format

Each challenge has a `flag.txt` file. The flag is plain text (no `flag{}` wrapper).

## Directory Structure

```
Ctf_labs/
├── docker-compose.yml
├── README.md
├── Oopsie/
├── Returnal/
├── R0P_Me/
├── Blind_Spot/
├── House_of_Calls/
└── Final_Payload/
```

Each challenge directory contains:
- `main.c` - Vulnerable source code
- `Makefile` - Build configuration
- `Dockerfile` - Container definition
- `run.sh` - Entry script
- `flag.txt` - Challenge flag
- `README.md` - Challenge-specific documentation