NAME = webserv

CC = c++
CPPFLAGS = -Wall -Werror -Wextra -std=c++98 -g3

SRCS = src/*.cpp

all: $(NAME)
  $(NAME):
	$(CC) $(CPPFLAGS) $(SRCS) -o $(NAME)
	@echo webserv compiled 🌐

# clean:
# 	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)
	rm -rf ./cgi/tmp

re: fclean all

.PHONY: all clean fclean re
