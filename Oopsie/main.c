#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern char *gets(char *str);

char flag[128];

void    vulnerable(void)
{
    char buffer[32];

    printf("Enter your name: ");
    gets(buffer);
    printf("Hello %s\n", buffer);
}

void win(void)
{
    puts("Congratulations!");
    puts(flag);
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    
    FILE *f = fopen("/home/ctf/flag.txt", "r");
    if (f) {
        fgets(flag, sizeof(flag), f);
        fclose(f);
    } else {
        strcpy(flag, "0XATTACK{FLAG_NOT_FOUND}\n");
    }
    
    vulnerable();
    return (0);
}