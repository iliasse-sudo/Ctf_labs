CC = gcc
CFLAGS = -fno-stack-protector -no-pie -Wall -Wextra
TARGET = house_of_calls

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean