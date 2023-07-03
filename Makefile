SHELL = /bin/bash

NAME = webserv
INCS = inc/
SRC_DIR = src/
OBJ_DIR = obj/
RM = rm -rf
CPP = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -g3

# Cores
SET_COLOR = \033[0m
WHITE = \033[0;37m
WHITE_BOLD = \033[1;37m
GREEN = \033[0;32m
BLUE = \033[0;34m
CYAN = \033[0;36m
YELLOW = \033[0;33m
MAGENTA = \033[0;95m
BG_GREEN = \033[42;1;37m

# Fontes
FILES = CgiController Client ConfigFile Location LogService main ManagerServ Parser Request Response Server Utils
SRC_FILES = $(addprefix $(SRC_DIR),$(FILES))
SRC = $(addsuffix .cpp, $(SRC_FILES))
OBJ = $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC))

###

OBJF = .cache_exists

start:
	@echo -e "$(CYAN)[🖥️  WEBSERV]$(SET_COLOR)$(WHITE_BOLD)$$(date +"%T")$(SET_COLOR)"
	@make all

all: $(NAME)

$(NAME): $(OBJ)
	@$(CPP) $(CPPFLAGS) $(OBJ) -o $(NAME)
	@echo -e "$(CYAN)[🖥️  WEBSERV]$(SET_COLOR)$(BLUE)Compilado com sucesso.$(SET_COLOR)"

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJF)
	@$(CPP) $(CPPFLAGS) -I $(INCS) -c $< -o $@
	@echo -e -n "$(CYAN)[🖥️  WEBSERV]$(SET_COLOR)$(BLUE)Compilando $(notdir $<) $(SET_COLOR)"
	@for i in {1..10}; do \
		sleep 0.05; \
		echo -ne "$(GREEN)✔$(SET_COLOR)"; \
	done
	@echo -e ""

$(OBJF):
	@mkdir -p $(OBJ_DIR)
	@touch $(OBJF)

clean:
	@$(RM) -r $(OBJ_DIR)
	@$(RM) $(OBJF)
	@echo -e -n "$(CYAN)[🖥️  WEBSERV]$(SET_COLOR)$(BLUE)Object files$(SET_COLOR)$(GREEN) => Cleaned$(SET_COLOR)"
	@echo -e "$(GREEN)🗑️$(SET_COLOR)"

fclean: clean
	@$(RM) $(NAME)
	@$(RM) ./cgi/tmp/
	@echo -e -n "$(CYAN)[🖥️  WEBSERV]$(SET_COLOR)$(BLUE)Executable file$(SET_COLOR)$(GREEN) => Cleaned$(SET_COLOR)"
	@echo -e "$(GREEN)🗑️$(SET_COLOR)"

re: fclean start
	@echo -e "\n$(BG_GREEN)=> Cleaned and rebuilt everything <=$(SET_COLOR)"

.PHONY: all clean fclean re
