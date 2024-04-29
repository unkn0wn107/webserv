# Makefile for HTTP Server Project in C++ 98

NAME = http_server
CC = g++
CFLAGS = -Wall -Wextra -Werror -std=c++98

# Source files
SRC = Server.cpp ConfigLoader.cpp ErrorHandler.cpp HTTP1_1.cpp HTTP2.cpp \
      HTTPRequest.cpp HTTPResponse.cpp FileManager.cpp CGIHandler.cpp \
      Router.cpp FileHandler.cpp Logger.cpp SessionManager.cpp CookieManager.cpp

# Object files
OBJ = $(SRC:.cpp=.o)

# Default rule
all: $(NAME)

# Linking the program
$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compiling source files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Cleaning up object files
clean:
	rm -f $(OBJ)

# Full clean (objects and executable)
fclean: clean
	rm -f $(NAME)

# Rebuild everything
re: fclean all

.PHONY: all clean fclean re
