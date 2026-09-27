#include <stdlib.h>
#include <stdio.h>


int main(void)
{
    int i;
    setvbuf(stdout, NULL, _IONBF, 0);
    system("cc main.c -o .a.out 2>/dev/null");
    i = system("./.a.out");
    if (i == 34304)
        printf("Congrats, you overflowed the buffer, try on the server now to get the flag.\n");
    system("rm .a.out");
    return (0);
}