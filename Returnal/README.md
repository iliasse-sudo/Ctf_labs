# RETURNAL

**Level:** Medium
**Category:** Pwn / Binary Exploitation
**Technique:** Stack Buffer Overflow / Return Address Overwrite
**Difficulty:** ⭐⭐☆☆☆

## Description

Returnal is a beginner-level binary exploitation challenge designed to introduce
players to controlling program execution through a **stack-based buffer
overflow**.

The binary contains a vulnerable function that reads more data into a local
stack buffer than the buffer can hold. This allows the player to overwrite
data stored further up the stack, including the function's saved return
address.

The challenge does not provide the source code to the player. They must
reverse-engineer enough of the binary to identify the vulnerability and locate
the target function.

## Intended Learning Objectives

Players should learn how to:

* Identify a buffer overflow vulnerability from a binary.
* Inspect a binary using tools such as `file`, `objdump`, `readelf`, and `gdb`.
* Identify interesting functions inside a binary.
* Understand the basic layout of a stack frame.
* Determine the offset between a local buffer and the saved return address.
* Understand how return addresses control program execution.
* Construct a basic ret2win-style payload.
* Handle little-endian addresses when constructing a payload.

## Vulnerability

The vulnerable function effectively performs the equivalent of:

```c
void vulnerable(void)
{
    char buffer[64];

    read(0, buffer, 256);
}
```

The destination buffer is only **64 bytes**, while `read()` allows up to
**256 bytes** to be written.

This allows input to overflow the buffer and overwrite values stored in the
stack frame.

The intended target is the hidden `win()` function:

```c
void win(void)
{
    printf("You found the secret!\n");
    system("/bin/bash -i");
}
```

The function is present in the binary but is not called during normal program
execution.

## Intended Solution

The intended exploitation path is:

1. Inspect the binary and identify the available functions.
2. Find the address of `win()`.
3. Determine the number of bytes required to reach the saved return address.
4. Construct a payload containing:

   * padding up to the saved return address
   * the address of `win()`
5. Send the payload to the program.
6. When `vulnerable()` returns, execution is redirected to `win()`.
7. `win()` spawns a shell.

This is essentially a **ret2win** challenge.

## Mitigations

The challenge is intentionally configured to keep the exploitation process
simple for beginners.

Expected relevant properties include:

* No stack canary
* No PIE
* NX may remain enabled, since code injection is not required
* The target function has a predictable address

The exact compilation options should be kept consistent between the challenge
binary and the deployed server binary.

## Server

The challenge is exposed through a TCP service.

```bash
nc localhost 1339
```

The server should execute the vulnerable binary and provide the player's input
to it.

## Files

```text
Returnal/
├── challenge.txt
├── flag.txt
├── README.md
├── main.c
├── returnal
├── Dockerfile
├── Makefile
└── run.sh
```

> `main.c` may be kept in the repository for challenge development and
> documentation, but it should **not** be included in the player package.

## Flag

The flag is provided after successfully reaching the intended shell/function
and completing the challenge.

Flag format:

```text
0XATTACK{...}
```

## Author Notes

This challenge is intended to follow the introductory buffer-overflow
challenge **Oopsie**.

Where Oopsie focuses on understanding the basics of overflowing a stack
buffer, Returnal introduces the next step: using the overflow to **control
the saved return address and redirect execution to an existing function**.

The challenge should therefore avoid unnecessary complications such as
stack canaries, PIE, or requiring shellcode. The intended technique is a
straightforward ret2win exploit.
