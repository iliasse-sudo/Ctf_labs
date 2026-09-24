CC = gcc
CFLAGS = -fno-stack-protector -no-pie -Wall -Wextra
TARGET = returnal

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean