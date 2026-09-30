CC = gcc
CFLAGS = -Wall -Wextra -fno-stack-protector -no-pie -g
SRC = main.c
TARGET = oopsie
ARCHIVE = Oopsie.tar.gz
DIRECTORY = Oopsie

all: player

server: $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $< 2>/dev/null

player: $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $<
	mkdir -p $(DIRECTORY)
	cp $(TARGET) $(SRC) challenge.txt $(DIRECTORY)
	tar -czf $(ARCHIVE) $(DIRECTORY)
	$(MAKE) clean

clean:
	rm -rf $(DIRECTORY) $(TARGET)

.PHONY: all clean server player
