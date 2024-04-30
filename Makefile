# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/12/15 15:51:13 by agaley            #+#    #+#              #
#    Updated: 2024/04/30 16:49:27 by agaley           ###   ########lyon.fr    #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -MMD -std=c++98

SRC_DIR = src
OBJ_DIR = obj

SRC = $(SRC_DIR)/Server.cpp $(SRC_DIR)/ConfigLoader.cpp $(SRC_DIR)/FileManager.cpp \
	  $(SRC_DIR)/ConnectionHandler.cpp \
	  $(SRC_DIR)/ErrorHandler.cpp $(SRC_DIR)/Logger.cpp $(SRC_DIR)/Utils.cpp \
      $(SRC_DIR)/HTTP1_1.cpp \
      $(SRC_DIR)/HTTPRequest.cpp $(SRC_DIR)/HTTPResponse.cpp \
      $(SRC_DIR)/CGIHandler.cpp $(SRC_DIR)/FileHandler.cpp \

OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))
DEPS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(SRC))

VPATH = $(SRC_DIR)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJ)
	rm -f $(DEPS)

fclean: clean
	rm -f $(NAME)
	rm -rf $(OBJ_DIR)

re: fclean all

.PHONY: all clean fclean re
