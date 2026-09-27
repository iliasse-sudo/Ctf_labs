CC = gcc
SRC = main.c detector.c
TARGET = oopsie

all: $(TARGET)

server: main.c clean
	$(CC) -o $(TARGET) $< 2>/dev/null

$(TARGET): $(SRC) clean
	$(CC) -o $@ detector.c

clean:
	rm -f $(TARGET)

.PHONY: all clean server