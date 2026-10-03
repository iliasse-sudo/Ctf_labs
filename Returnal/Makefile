CC = gcc
CFLAGS = -fno-stack-protector -no-pie -Wall -Wextra
TARGET = returnal
SRC = main.c
ARCHIVE = Returnal.tar.gz
Directory = Returnal

all: player

player: $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $<
	mkdir $(Directory)
	cp $(TARGET) challenge.txt $(Directory)
	tar -cvf $(ARCHIVE) $(Directory)
	$(MAKE) clean

server: $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $<

clean:
	rm -rf $(TARGET) $(Directory)

.PHONY: all player server clean