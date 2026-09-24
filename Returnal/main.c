#include <stdio.h>
#include <unistd.h>

void win(void)
{
    puts("You found the secret!");
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