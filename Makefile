# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/12/15 15:51:13 by agaley            #+#    #+#              #
#    Updated: 2024/05/04 02:30:27 by agaley           ###   ########lyon.fr    #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -MMD -std=c++98
DEBUGFLAGS = -g3

SRC_DIR = src
OBJ_DIR = obj
DEBUG_OBJ_DIR = obj_debug

SRC = $(SRC_DIR)/Server.cpp $(SRC_DIR)/ConfigLoader.cpp $(SRC_DIR)/FileManager.cpp \
      $(SRC_DIR)/ConnectionHandler.cpp \
      $(SRC_DIR)/ErrorHandler.cpp $(SRC_DIR)/Logger.cpp $(SRC_DIR)/Utils.cpp \
      $(SRC_DIR)/HTTP1_1.cpp \
      $(SRC_DIR)/HTTPRequest.cpp $(SRC_DIR)/HTTPResponse.cpp \
      $(SRC_DIR)/CGIHandler.cpp $(SRC_DIR)/FileHandler.cpp \
      $(SRC_DIR)/main.cpp

OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))
DEBUG_OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_OBJ_DIR)/%.o, $(SRC))
DEPS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(SRC))
DEBUG_DEPS = $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_OBJ_DIR)/%.d, $(SRC))

VPATH = $(SRC_DIR)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(NAME)
	@$(MAKE) run_webserv &
	@$(MAKE) run_tests
	@kill $$(pgrep -f './webserv')

run_webserv:
	@./webserv

run_tests:
	@./test.sh

debug: $(DEBUG_OBJ)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) $(DEBUG_OBJ) -o $(NAME)

$(DEBUG_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(DEBUG_OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c $< -o $@

-include $(DEPS)
-include $(DEBUG_DEPS)

clean:
	rm -f $(OBJ) $(DEBUG_OBJ)
	rm -f $(DEPS) $(DEBUG_DEPS)

fclean: clean
	rm -f $(NAME)
	rm -rf $(OBJ_DIR) $(DEBUG_OBJ_DIR)

re: fclean all
debug_re: fclean debug

.PHONY: all clean fclean re debug debug_re
