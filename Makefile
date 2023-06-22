NAME = webserv

CC = c++
CPPFLAGS = -Wall -Werror -Wextra -std=c++98 -g3

SRCS = src/*.cpp

all: $(NAME)

$(NAME):
	$(CC) $(CPPFLAGS) $(SRCS) -o $(NAME)

fclean:
	rm -rf $(NAME)

re: fclean all

.PHONY: fclean, all, re
