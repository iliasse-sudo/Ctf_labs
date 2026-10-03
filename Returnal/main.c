#include <stdio.h>
#include <unistd.h>

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