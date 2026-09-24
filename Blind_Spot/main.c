#include <stdio.h>
#include <unistd.h>

void win(void)
{
    system("/bin/sh");
}

void leak(void)
{
    printf("Here is something interesting: %p\n", (void*)win);
}

void vulnerable(void)
{
    char buffer[64];
    read(0, buffer, 256);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    leak();
    vulnerable();
    return 0;
}