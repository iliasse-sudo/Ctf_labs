# Challenge 03: R0P_Me — Return-Oriented Programming Basics

**Difficulty:** ★★☆☆☆ Medium  
**Binary:** `r0p_me`  
**Port:** 1340  
**Category:** Pwn / Stack Buffer Overflow / Return-Oriented Programming

## Description

R0P_Me introduces the next step in stack-based binary exploitation: controlling not only the program's execution flow, but also the arguments passed to a function.

The binary contains a vulnerable function that reads more data into a stack buffer than it can hold. By exploiting this vulnerability, players can overwrite the saved return address and redirect execution to an existing function.

However, there is a catch: the target function expects a specific argument. Reaching it without satisfying that requirement will not be enough.

The challenge introduces the fundamentals of **ret2win exploitation with register control**, a stepping stone toward Return-Oriented Programming (ROP).

## Intended Learning Objectives

Players should learn how to:

- Identify a stack buffer overflow in a compiled binary.
- Inspect a binary using `file`, `objdump`, `readelf`, and `gdb`.
- Locate useful functions and gadgets.
- Determine the offset to the saved return address.
- Understand the x86-64 System V calling convention.
- Understand how function arguments are passed through registers.
- Construct a payload that controls execution and prepares the required argument.
- Understand how a `pop rdi; ret` gadget can be used in a ROP chain.
- Interact with a shell spawned by a remotely exploited process.

## Vulnerability

The vulnerable function effectively performs the following operation:

```c
void vulnerable(void)
{
    char buffer[64];
    read(0, buffer, 256);
}
```

The destination buffer is only **64 bytes**, while `read()` allows up to **256 bytes** to be written.

Because the input length is not restricted to the buffer's capacity, an attacker can overwrite adjacent stack data, including the saved return address.

## Target Function

The binary contains a function that requires a specific argument:

```c
void win(unsigned long magic)
{
    if (magic != 0x13371337)
        return;

    puts("Correct!");
    system("/bin/bash -i");
}
```

The function checks whether its argument matches `0x13371337`. If the value is incorrect, the function returns without spawning a shell.

The function is not called during normal execution, so the player must redirect execution to it through the overflow.

## The Extra Challenge: Function Arguments

On Linux x86-64, the System V AMD64 calling convention specifies that the first integer or pointer argument is passed in the `RDI` register.

Consequently, overwriting the saved return address to point directly to `win()` is insufficient. The required argument must also be prepared before the function executes.

The binary includes a useful instruction sequence:

```asm
pop rdi
ret
```

This sequence can be used to load a chosen value from the stack into `RDI`, then continue execution using the next return address.

A successful payload must therefore account for:

1. The padding needed to reach the saved return address.
2. The address of a suitable gadget.
3. The required argument value.
4. The address of the target function.

The exact addresses and offsets should be determined from the provided binary rather than assumed.

## Intended Exploitation Path

The intended solution follows these steps:

1. Inspect the binary and identify interesting functions.
2. Locate `win()` and the available instruction sequences.
3. Determine the offset to the saved return address.
4. Identify a suitable gadget for controlling the first function argument.
5. Construct a ROP chain that places `0x13371337` in `RDI`.
6. Redirect execution to `win()`.
7. Verify that the program prints `Correct!` and provides an interactive shell.

This challenge builds on a basic ret2win exploit by introducing **argument control through a ROP gadget**.

## Binary Protections

The challenge is intended to be beginner-friendly, with predictable code addresses and no stack canary.

Expected relevant properties include:

- **No stack canary:** stack corruption is not detected by a canary check.
- **No PIE:** executable code addresses remain predictable.
- **NX enabled:** code injection is unnecessary because the exploit reuses existing instructions.
- **A predictable target function:** `win()` can be located through binary analysis.

Verify the actual protections of the distributed binary with:

```bash
checksec --file=r0p_me
```

The deployed binary should match the version used during testing.

## Server

The challenge is exposed through a TCP service on port `1340`.

Connect using:

```bash
nc localhost 1340
```

If the exploit successfully spawns an interactive shell, the player's input stream must remain open. For a locally generated payload, a pattern such as the following keeps standard input available:

```bash
(your_payload; cat) | nc localhost 1340
```

Replace `your_payload` with the command that generates the actual exploit input.

The trailing `cat` is important because the shell needs to continue receiving input after the initial payload has been sent.

## Files

The player package contains:

```text
R0P_Me/
├── challenge.txt
└── r0p_me
```

## Flag

The flag is provided by the challenge environment after successful exploitation.

Flag format:

```text
0XATTACK{...}
```

## Author Notes

R0P_Me is intended to follow **Returnal**, which introduces overwriting a saved return address to reach an existing function.

The progression is deliberate:

- **Oopsie:** Understand the fundamentals of stack buffer overflows.
- **Returnal:** Redirect execution by overwriting the saved return address.
- **R0P_Me:** Control a function argument using a ROP gadget before redirecting execution.

The challenge should not require shellcode or advanced ROP techniques. Its central lesson is that controlling the instruction pointer alone does not guarantee a successful exploit: the target function must also receive the arguments it expects.

The `pop rdi; ret` gadget is intentionally included to make register control accessible to beginners while introducing a technique that will be useful in more advanced binary exploitation challenges.