#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NOTES 10
#define NOTE_SIZE 128

typedef struct {
    char *content;
    size_t size;
} Note;

Note notes[MAX_NOTES];
int note_count = 0;

void setup(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

void print_menu(void)
{
    puts("\n=== Secure Notes Manager ===");
    puts("1. Create note");
    puts("2. View note");
    puts("3. Edit note");
    puts("4. Delete note");
    puts("5. Exit");
    printf("Choice: ");
}

int read_int(void)
{
    char buf[16];
    read(0, buf, sizeof(buf) - 1);
    return atoi(buf);
}

void read_bytes(char *buf, size_t len)
{
    read(0, buf, len);
}

void create_note(void)
{
    if (note_count >= MAX_NOTES) {
        puts("Maximum notes reached!");
        return;
    }

    printf("Enter note size (max %d): ", NOTE_SIZE);
    size_t size = read_int();
    
    if (size > NOTE_SIZE || size == 0) {
        puts("Invalid size!");
        return;
    }

    notes[note_count].content = malloc(size);
    if (!notes[note_count].content) {
        puts("Allocation failed!");
        return;
    }
    notes[note_count].size = size;

    printf("Enter note content: ");
    read_bytes(notes[note_count].content, size);
    
    note_count++;
    puts("Note created!");
}

void view_note(void)
{
    printf("Enter note index (0-%d): ", note_count - 1);
    int idx = read_int();
    
    if (idx < 0 || idx >= note_count || !notes[idx].content) {
        puts("Invalid index!");
        return;
    }

    printf("Note %d: %s\n", idx, notes[idx].content);
}

void edit_note(void)
{
    printf("Enter note index (0-%d): ", note_count - 1);
    int idx = read_int();
    
    if (idx < 0 || idx >= note_count || !notes[idx].content) {
        puts("Invalid index!");
        return;
    }

    printf("Enter new content: ");
    // VULNERABILITY: No bounds check! Uses note size but reads more
    read_bytes(notes[idx].content, 256);  // Overflow!
    
    puts("Note updated!");
}

void delete_note(void)
{
    printf("Enter note index (0-%d): ", note_count - 1);
    int idx = read_int();
    
    if (idx < 0 || idx >= note_count || !notes[idx].content) {
        puts("Invalid index!");
        return;
    }

    free(notes[idx].content);
    notes[idx].content = NULL;
    notes[idx].size = 0;
    puts("Note deleted!");
}

int main(void)
{
    setup();
    
    puts("Welcome to Secure Notes Manager v1.0");
    
    while (1) {
        print_menu();
        int choice = read_int();
        
        switch (choice) {
            case 1: create_note(); break;
            case 2: view_note(); break;
            case 3: edit_note(); break;
            case 4: delete_note(); break;
            case 5: 
                puts("Goodbye!");
                return 0;
            default:
                puts("Invalid choice!");
        }
    }
    
    return 0;
}