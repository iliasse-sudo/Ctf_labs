#include <stdio.h>
#include <string.h>


void vulnerable(void)
{
    char    buffer[32];

    printf("Enter your name: ");
    memset(buffer, 0, 32);
    gets(buffer);
    printf("Hello %s\n", buffer);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    vulnerable();
    return 0;
}