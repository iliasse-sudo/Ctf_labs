CC = gcc
SRC = main.c
TARGET = oopsie
ARCHIVE = Oopsie.tar.gz
DIRECTORY = Oopsie

all: player

server: $(SRC)
	$(CC) -o $(TARGET) $< 2>/dev/null

player: $(SRC)
	$(CC) -o $(TARGET) $<
	mkdir -p $(DIRECTORY)
	cp $(TARGET) $(SRC) challenge.txt $(DIRECTORY)
	tar -czf $(ARCHIVE) $(DIRECTORY)
	make clean

clean:
	rm -rf $(DIRECTORY) $(TARGET)

.PHONY: all clean server player
