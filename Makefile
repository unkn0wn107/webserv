# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/24 12:57:57 by mchenava          #+#    #+#              #
#    Updated: 2024/05/03 00:07:38 by  mchenava        ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Nom du programme
NAME = webserv

# Compilateur
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g3

# Dossiers
SRC_DIR = ./src
BUILD_DIR = ./.build
LOG_DIR = /home/mchenava/log

# Fichiers sources et objets
SRCS =	main.cpp \
		Server.cpp \
		ConnectionHandler.cpp \
		HTTP1_1.cpp \
		Request.cpp \
		Response.cpp \
		Logger.cpp \
		ServerException.cpp

SRCS := $(addprefix $(SRC_DIR)/,$(SRCS))
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Règle par défaut
all: $(NAME)

# Création du dossier .build et compilation du programme
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	mkdir -p $(LOG_DIR)

# Compilation des fichiers objets dans le dossier .build
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle pour nettoyer les fichiers objets
clean:
	rm -rf $(BUILD_DIR)

# Règle pour nettoyer tout ce qui peut être régénéré
fclean: clean
	rm -f $(NAME)

# Règle pour recompiler le programme
re: fclean all

# Empêcher les règles implicites
.PHONY: all clean fclean re
