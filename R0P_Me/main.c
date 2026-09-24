#include <stdio.h>
#include <unistd.h>

void win(unsigned long magic)
{
    if (magic != 0x13371337)
        return;

    puts("Correct!");
    system("/bin/sh");
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