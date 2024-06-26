# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/12/15 15:51:13 by agaley            #+#    #+#              #
#    Updated: 2024/06/27 01:03:46 by agaley           ###   ########lyon.fr    #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -std=c++98
DEBUGFLAGS = -g3

SRC_DIR = src
OBJ_DIR = .obj
DEBUG_OBJ_DIR = .obj_debug
LOG_DIR = ./logs

LOG_FILE_EXT = .log

export MY_GID ?= $(id -g)
export BUILD_TYPE ?= production
export MY_UID ?= $(id -u)
export NGINX_PORT_1 ?= 8000
export NGINX_PORT_2 ?= 8001
export CONTAINER ?= webserv-production
export PORT ?= 8080

SRC = $(SRC_DIR)/Server.cpp \
		$(SRC_DIR)/Config.cpp $(SRC_DIR)/ConfigManager.cpp $(SRC_DIR)/ConfigParser.cpp \
		$(SRC_DIR)/FileManager.cpp \
		$(SRC_DIR)/ConnectionHandler.cpp $(SRC_DIR)/CacheHandler.cpp \
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

all: $(NAME) setup

setup:
	NUM=1; \
	for dir in /var/www/html/static_templates/* ; do \
		if [ "$$(basename "$$dir")" != "images" ] && [ "$$(basename "$$dir")" != "README.md"  ]; then \
			echo "$$dir"; \
			mv "$$dir" /var/www/html/static_templates/static$$NUM || true; \
			NUM=$$((NUM + 1)); \
			if [ "$$NUM" -eq 5 ]; then \
				break; \
			fi; \
		fi; \
	done; \
	NUM=1; \
	for dir in /mnt/e/webserv/html-website-templates/* ; do \
		if [ "$$(basename "$$dir")" != "images" ] && [ "$$(basename "$$dir")" != "README.md"  ]; then \
			echo "$$dir"; \
			mv "$$dir" /home/mchenava/webserv/html-website-templates/static$$NUM || true; \
			NUM=$$((NUM + 1)); \
			if [ "$$NUM" -eq 5 ]; then \
				break; \
			fi; \
		fi; \
	done

$(NAME): $(OBJ) update_gitignore
	$(CXX) $(CXXFLAGS) $(OBJ) -lpthread -o $(NAME)
	mkdir -p $(LOG_DIR)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: $(DEBUG_OBJ)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) $(DEBUG_OBJ) -lpthread -o $(NAME)

$(DEBUG_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(DEBUG_OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c $< -o $@

run: daemon
	$(MAKE) wait-for-healthy
	docker compose exec -it webserv make
	docker compose exec -it webserv bash -c "kill 1"
	sleep 1
	@make logs

daemon:
	BUILD_TYPE=production docker compose up --build -d webserv

watch:
	while true; do \
		$(MAKE); \
		inotifywait -qre close_write /app/src; \
	done

dev:
	export BUILD_TYPE=debug
	docker compose up --build -d webserv-dev
	docker compose exec -it webserv-dev make debug
	docker compose exec -it webserv-dev bash -c "./webserv"

valgrind:
	export BUILD_TYPE=debug
	docker compose up --build -d webserv-dev
	docker compose exec -it webserv-dev make debug
	docker compose exec -it webserv-dev bash -c "ulimit -n 1024 && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv"

logs:
	docker compose logs -f

stop:
	docker compose stop

test: stop daemon
	$(MAKE) wait-for-healthy
	./test.sh
	@make clean

test-compare: stop daemon
	@$(MAKE) nginxd
	$(MAKE) wait-for-healthy
	$(MAKE) wait-for-nginx-healthy
	./test_compare.sh
	@make clean

siege: stop daemon
	$(MAKE) wait-for-healthy
	echo "init" > siege.log
	docker compose up siege
	# make logs
	@make clean
	cat siege.log

siege-nginx: stop nginxd
	$(MAKE) wait-for-nginx-healthy
	CONTAINER=nginx docker compose up siege
	@make clean
	cat siege.log

run_tests:
	@./test.sh
	@./test_compare.sh

wait-for-healthy:
	@echo "Waiting for webserv docker to be healthy..."
	@while ! docker inspect --format='{{json .State.Health.Status}}' webserv-production | grep -q '"healthy"'; do \
		echo "Waiting for webserv to become healthy..."; \
		sleep 2; \
	done

wait-for-nginx-healthy:
	@echo "Waiting for nginx docker to be healthy..."
	@while ! docker inspect --format='{{json .State.Health.Status}}' nginx | grep -q '"healthy"'; do \
		echo "Waiting for nginx to become healthy..."; \
		sleep 2; \
	done

update_gitignore:
	@if ! grep -q "$(LOG_FILE_EXT)" .gitignore; then \
		echo "$(LOG_FILE_EXT)" >> .gitignore; \
		echo "Added $(LOG_FILE_EXT) to .gitignore"; \
	else \
		echo "$(LOG_FILE_EXT) already in .gitignore"; \
	fi

nginx:
	NGINX_PORT_1=$(NGINX_PORT_1) NGINX_PORT_2=$(NGINX_PORT_2) docker compose up nginx --build

nginxd:
	NGINX_PORT_1=$(NGINX_PORT_1) NGINX_PORT_2=$(NGINX_PORT_2) docker compose up -d nginx --build

docker-clean:
	docker compose down --rmi all

docker-fclean:
	docker system prune --all --volumes -f

-include $(DEPS)
-include $(DEBUG_DEPS)

clean: docker-stop docker-clean
	rm -f $(OBJ) $(DEBUG_OBJ)
	rm -f $(DEPS) $(DEBUG_DEPS)

fclean: clean docker-fclean
	rm -f $(NAME)
	rm -rf $(OBJ_DIR) $(DEBUG_OBJ_DIR)

re: fclean all
debug_re: fclean debug

.PHONY: all clean fclean re debug debug_re update_gitignore
.PHONY: run daemon dev logs stop
.PHONY: test test-compare wait-for-healthy wait-for-nginx-healthy
.PHONY: nginx nginxd docker-stop docker-fclean run_tests
