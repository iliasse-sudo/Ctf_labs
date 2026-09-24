CC = gcc
CFLAGS = -fstack-protector-strong -pie -fPIE -Wall -Wextra -Wformat-security
LDFLAGS = -Wl,-z,relro,-z,now
TARGET = final_payload

all: $(TARGET) patch

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

patch: $(TARGET) libc.so.6 ld-linux-x86-64.so.2
	patchelf --set-interpreter ./ld-linux-x86-64.so.2 --set-rpath \$ORIGIN ./$(TARGET)

libc.so.6 ld-linux-x86-64.so.2:
	# These are copied by Dockerfile from the base image

clean:
	rm -f $(TARGET)

.PHONY: all patch clean