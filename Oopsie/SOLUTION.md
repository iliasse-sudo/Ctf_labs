# OOPSIE — SOLUTION

**Difficulty:** Beginner
**Category:** Pwn / Binary Exploitation
**Goal:** Understand and trigger a stack buffer overflow

---

## 1. What are we trying to do?

This challenge is about a **buffer overflow**.

You are given a program that asks for your name:

```text
Enter your name:
```

The program stores your input in a small buffer.

The problem is that the program uses `gets()`, which does **not** check how much data you enter.

So our goal is simple:

> **Enter more data than the buffer can hold and see what happens.**

This is one of the most basic ideas in binary exploitation.

---

## 2. Looking at the source code

Open `main.c`:

```c
void vulnerable(void)
{
    char buffer[32];

    printf("Enter your name: ");
    memset(buffer, 0, 32);
    gets(buffer);
    printf("Hello %s\n", buffer);
}
```

There are two important lines here:

```c
char buffer[32];
```

and:

```c
gets(buffer);
```

### What does `buffer[32]` mean?

It creates a space in memory that can hold **32 bytes**.

Think of it like a box that can hold 32 characters:

```text
+--------------------------------+
|                                |
|          32 bytes              |
|                                |
+--------------------------------+
```

If we put 10 characters inside, everything is fine.

But what happens if we try to put 100 characters inside?

There isn't enough space.

---

## 3. Why is `gets()` dangerous?

Normally, when reading input, we want to tell the program how much data it is allowed to read.

For example:

```c
fgets(buffer, sizeof(buffer), stdin);
```

This tells `fgets()` that `buffer` is only 32 bytes.

`gets()` doesn't do this.

```c
gets(buffer);
```

It keeps reading until it reaches a newline or EOF.

So if we give it:

```text
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
```

the program will happily keep writing the `A`s even after the 32-byte buffer is full.

Those extra bytes start overwriting other data in memory.

That's our **buffer overflow**.

---

# 4. Let's try it

First, compile the program:

```bash
gcc -o oopsie main.c
```

Then run it:

```bash
./oopsie
```

Try a normal input:

```text
Enter your name: Alice
Hello Alice
```

Nothing interesting happens.

Now let's send a lot of `A`s:

```bash
python3 -c "print('A' * 60)" | ./oopsie
```

You may see:

```text
Segmentation fault
```

That's interesting.

Our input was able to corrupt something important.

---

# 5. What exactly did we overwrite?

To understand this, we need to know a little about the **stack**.

When `vulnerable()` runs, the program creates a small area of memory called a **stack frame**.

A simplified version looks like this:

```text
Higher memory addresses
        |
        v

+-------------------------+
| Return address (RIP)    |  <-- Where to go after vulnerable()
+-------------------------+
| Saved RBP               |
+-------------------------+
|                         |
|       buffer[32]        |
|                         |
+-------------------------+

        ^
        |
Lower memory addresses
```

The important part is that the buffer is close to information that the CPU needs to return from the function.

If we write too much into the buffer, we can overwrite that information.

---

# 6. Why does the program crash?

Remember our input:

```text
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
```

Each `A` is one byte:

```text
A = 0x41
```

So if we overwrite an 8-byte value with `A`s, it becomes:

```text
41 41 41 41 41 41 41 41
```

Which is represented as:

```text
0x4141414141414141
```

If we overwrite the function's return address with this value, the CPU eventually tries to return to:

```text
0x4141414141414141
```

That isn't a valid address for our program.

The CPU can't continue there, so the program crashes with a **segmentation fault**.

---

# 7. Let's see it in GDB

GDB is a debugger.

It lets us pause a program and inspect what is happening inside it.

Start GDB:

```bash
gdb ./oopsie
```

Then run the program:

```text
(gdb) run
```

When it asks for your name, give it a long string of `A`s:

```text
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
```

When the program crashes, run:

```text
(gdb) info registers
```

You may see something similar to:

```text
rip    0x4141414141414141
rbp    0x4141414141414141
```

Don't worry if this looks confusing.

The important thing is:

```text
0x4141414141414141
```

is just our `A`s.

Remember:

```text
A = 0x41
```

So:

```text
AAAAAAAA
```

becomes:

```text
0x4141414141414141
```

This proves that our input reached places it wasn't supposed to reach.

---

# 8. What is RIP?

You will see `RIP` a lot when doing x86-64 pwn challenges.

**RIP** is the CPU's **instruction pointer**.

It tells the CPU:

> "The next instruction you should execute is here."

For example:

```text
RIP = 0x401234
```

means the CPU is currently executing instructions around that address.

If we manage to overwrite RIP:

```text
RIP = 0x4141414141414141
```

the CPU tries to execute code at that address.

There is no valid code there, so the program crashes.

This is why controlling RIP is such an important concept in binary exploitation.

---

# 9. Finding the overflow

We can experiment with different input lengths.

Try:

```bash
python3 -c "print('A' * 32)" | ./oopsie
```

Then:

```bash
python3 -c "print('A' * 40)" | ./oopsie
```

And:

```bash
python3 -c "print('A' * 48)" | ./oopsie
```

The exact behavior can depend on how the binary was compiled and which protections are enabled.

For example, you might encounter:

```text
*** stack smashing detected ***
```

followed by:

```text
Aborted
```

This means the compiler added a **stack canary**.

---

# 10. What is a stack canary?

A stack canary is a security mechanism designed to detect stack overflows.

The compiler can place a special value between local variables and important control data:

```text
+-------------------------+
| Return address          |
+-------------------------+
| Saved RBP               |
+-------------------------+
| Stack canary            |  <-- Security check
+-------------------------+
| buffer[32]              |
+-------------------------+
```

When the function finishes, the program checks whether the canary changed.

If our overflow overwrote it, the program knows something went wrong and can stop the program instead of returning normally.

That's why you may see:

```text
stack smashing detected
```

instead of immediately seeing RIP become `0x4141414141414141`.

---

# 11. Looking at the stack

GDB can also show us the memory around the stack pointer.

Try:

```text
(gdb) x/20gx $rsp
```

Let's break that command down:

* `x` = examine memory
* `20` = show 20 values
* `g` = display 8-byte values
* `x` = display them in hexadecimal
* `$rsp` = start at the address stored in RSP

You don't need to memorize this command yet.

The important idea is that GDB lets us **look directly at memory**.

You can also examine individual bytes:

```text
(gdb) x/24xb $rsp
```

Here:

* `b` = byte
* `x` = hexadecimal

This is useful when learning how your input is actually stored in memory.

---

# 12. What did we learn?

The important chain is:

```text
Our input
    ↓
buffer[32]
    ↓
too much data
    ↓
data spills into nearby memory
    ↓
important stack data gets corrupted
    ↓
program crashes
```

That's the basic idea behind a stack buffer overflow.

---

# 13. Why does this matter in a CTF?

In this challenge, making the program crash is enough to demonstrate the vulnerability.

In a real pwn challenge, the goal is often to go further.

Instead of simply crashing:

```text
AAAAAAA...
       ↓
     CRASH
```

we may eventually want to control where the program goes:

```text
AAAAAAA...
       ↓
overwrite RIP
       ↓
choose a new address
       ↓
execute something useful
       ↓
get the flag
```

This is the foundation for techniques such as:

* **ret2win**
* **ROP**
* **ret2libc**
* **shellcode**
* and other binary exploitation techniques.

You don't need to know these yet. The important thing is understanding **why overwriting RIP gives an attacker control over the program's execution.**

---

# 14. Bonus: Fixing the vulnerability

Replace:

```c
gets(buffer);
```

with:

```c
fgets(buffer, sizeof(buffer), stdin);
```

Now the program knows the size of the buffer.

Try entering 100 `A`s again:

```bash
python3 -c "print('A' * 100)" | ./oopsie
```

The program should no longer overflow the buffer.

---

# 15. Key Takeaways

If you remember only a few things from this challenge, remember these:

### 1. A buffer is just a piece of memory

```c
char buffer[32];
```

means we have space for 32 bytes.

### 2. `gets()` doesn't check the size

```c
gets(buffer);
```

can write more data than the buffer can hold.

### 3. Extra data can overwrite nearby memory

That's the **buffer overflow**.

### 4. RIP controls where the CPU executes

If an attacker can overwrite RIP, they may be able to control the program's execution.

### 5. GDB helps us see what's happening

Useful commands from this challenge:

```text
run
info registers
x/20gx $rsp
x/24xb $rsp
```

Don't worry about memorizing everything. In pwn, a big part of the learning process is getting comfortable with **looking at memory and registers and asking what the values mean.**

---

## Further Reading

* **CWE-121** — Stack-based Buffer Overflow
* **Smashing the Stack for Fun and Profit** — Aleph One
* **Computer Systems: A Programmer's Perspective** — Chapter 3
