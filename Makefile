# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/12/15 15:51:13 by agaley            #+#    #+#              #
#    Updated: 2024/06/19 23:36:06 by agaley           ###   ########lyon.fr    #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -std=c++98
DEBUGFLAGS = -g3 -fsanitize=address

SRC_DIR = src
OBJ_DIR = obj
DEBUG_OBJ_DIR = obj_debug
LOG_DIR = ./logs

LOG_FILE_EXT = .log

NGINX_PORT_1 = 8000
NGINX_PORT_2 = 8001

SRC = $(SRC_DIR)/Server.cpp \
		$(SRC_DIR)/ConfigManager.cpp $(SRC_DIR)/ConfigParser.cpp \
		$(SRC_DIR)/FileManager.cpp \
		$(SRC_DIR)/ConnectionHandler.cpp \
		$(SRC_DIR)/Worker.cpp \
		$(SRC_DIR)/HTTPRequest.cpp $(SRC_DIR)/HTTPResponse.cpp $(SRC_DIR)/URI.cpp \
		$(SRC_DIR)/CGIHandler.cpp $(SRC_DIR)/FileHandler.cpp \
		$(SRC_DIR)/VirtualServer.cpp $(SRC_DIR)/Common.cpp \
		$(SRC_DIR)/Exception.cpp $(SRC_DIR)/HTTPMethods.cpp \
		$(SRC_DIR)/ErrorHandler.cpp $(SRC_DIR)/Logger.cpp $(SRC_DIR)/Utils.cpp \
		$(SRC_DIR)/main.cpp

OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))
DEBUG_OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_OBJ_DIR)/%.o, $(SRC))
DEPS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(SRC))
DEBUG_DEPS = $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_OBJ_DIR)/%.d, $(SRC))

VPATH = $(SRC_DIR)

all: $(NAME)

$(NAME): $(OBJ) update_gitignore
	$(CXX) $(CXXFLAGS) $(OBJ) -lpthread -o $(NAME)
	mkdir -p $(LOG_DIR)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: run-only
	sleep 1
	@make logs

run-only:
	MY_UID=$(id -u) MY_GID=$(id -g) BUILD_TYPE=production docker compose up --build -d
	@make build

run-debug:
	MY_UID=$(id -u) MY_GID=$(id -g) BUILD_TYPE=debug docker compose up --build -d
	@make build-debug

dev:
	MY_UID=$(id -u) MY_GID=$(id -g) BUILD_TYPE=production docker compose up --build -d webserv-dev
	MY_UID=$(id -u) MY_GID=$(id -g) BUILD_TYPE=production docker compose exec -it webserv-dev make
	MY_UID=$(id -u) MY_GID=$(id -g) BUILD_TYPE=production docker compose exec -it webserv-dev ash -c "./webserv"

build:
	docker compose exec webserv* make -C /app/ 2>&1 | grep -v "WARN\[0000\]" | grep -v "level=warning"
	-docker compose exec webserv* kill 1 2>&1 | grep -v "WARN\[0000\]" | grep -v "level=warning"

build-debug:
	docker compose exec webserv* make -C debug /app/ 2>&1 | grep -v "WARN\[0000\]" | grep -v "level=warning"
	-docker compose exec webserv* kill 1 2>&1 | grep -v "WARN\[0000\]" | grep -v "level=warning"

logs:
	docker compose logs -f

stop:
	docker compose stop

test:
	MY_UID=$(id -u) MY_GID=$(id -g) BUILD_TYPE=production docker compose up --build -d webserv
	sleep 2
	./test.sh
	$(MAKE) stop

test-compare: docker-stop
	@$(MAKE) run-only
	@$(MAKE) nginxd
	sleep 1
	./test_compare.sh
	@$(MAKE) docker-stop > /dev/null 2>&1

update_gitignore:
	@if ! grep -q "$(LOG_FILE_EXT)" .gitignore; then \
		echo "$(LOG_FILE_EXT)" >> .gitignore; \
		echo "Added $(LOG_FILE_EXT) to .gitignore"; \
	else \
		echo "$(LOG_FILE_EXT) already in .gitignore"; \
	fi

nginx-build:
	docker build -t nginx nginx/

nginx: nginx-build
	docker run --rm --name nginx -p $(NGINX_PORT_1):8080 -p $(NGINX_PORT_2):8081 \
		-v ./default.conf:/etc/nginx/conf.d/default.conf:ro \
		-v ./site:/var/www/html nginx

nginxd: nginx-build
	docker compose run --rm --build --name webserv -d -p 8080:8080 -p 8081:8081 \
		-v ./src/:/app/src/ \
		-v ./default.conf:/app/default.conf \
		-v ./site:/var/www/html webserv

docker-stop:
	-docker stop nginx & docker compose stop webserv*

docker-fclean:
	docker system prune --all --volumes -f

run_tests:
	@./test.sh
	@./test_compare.sh

debug: $(DEBUG_OBJ)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) $(DEBUG_OBJ) -o $(NAME)

$(DEBUG_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(DEBUG_OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c $< -o $@

-include $(DEPS)
-include $(DEBUG_DEPS)

clean: docker-stop
	rm -f $(OBJ) $(DEBUG_OBJ)
	rm -f $(DEPS) $(DEBUG_DEPS)

fclean: clean docker-fclean
	rm -f $(NAME)
	rm -rf $(OBJ_DIR) $(DEBUG_OBJ_DIR)

re: fclean all
debug_re: fclean debug

.PHONY: all clean fclean re debug debug_re update_gitignore logs nginx-build nginx docker-fclean run_tests test
