#include <stdio.h>
#include <unistd.h>

void __attribute__((naked)) gadget_pop_rdi(void)
{
    asm("pop %rdi; ret");
}

void vulnerable(void)
{
    char buffer[64];
    
    // Leak puts@GOT
    printf("Leak: %p\n", (void*)puts);
    
    read(0, buffer, 256);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    vulnerable();
    return 0;
}