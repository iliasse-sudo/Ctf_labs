NAME = Oopsie

CC = cc
CFLAGS = -Wall -Wextra -Werror -fno-stack-protector -z execstack -no-pie -fomit-frame-pointer -D_GNU_SOURCE

SRCS = main.c

all: $(NAME)

$(NAME): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)

clean:
	rm -f $(NAME)

re: clean all

.PHONY: all clean re