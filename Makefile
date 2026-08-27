NAME	= codexion
CC		= gcc
CFLAGS	= -Wall -Wextra -Werror -pthread
SRC		= main.c parsing.c init.c coder.c dongle.c heap.c monitor.c utils.c
OBJ		= $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
