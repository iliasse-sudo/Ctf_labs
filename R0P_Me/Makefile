CC = gcc
CFLAGS = -fno-stack-protector -no-pie -Wall -Wextra
TARGET = r0p_me
SRC = main.c
ARCHIVE = R0P_Me.tar.gz
Directory = R0P_Me

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
	rm -rf $(Directory)

.PHONY: all player server clean
