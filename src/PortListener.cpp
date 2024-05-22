/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PortListener.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 17:40:37 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/21 16:00:07 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PortListener.hpp"
#include "Utils.hpp"

PortListener::PortListener(int port, std::vector<ServerConfig> configs) :
	_port(port),
	_server_fd(-1),
	_epoll_fd(-1),
	_keepRunning(false),
	_serverConfigs(configs),
	_connectionHandler(new ConnectionHandler(_serverConfigs)),
    _log(Logger::getInstance())
{
    _log.info("New port listener on :" + Utils::to_string(_port));
}

PortListener::~PortListener() {
    stopListening();
}

void PortListener::_setupListeningSocket() {
	struct sockaddr_in6 address;
  int                 opt = 1;
  int                 ipv6only = 0;
	_listeningSocket = socket(AF_INET6, SOCK_STREAM, 0);
	if (_listeningSocket == 0) {
		throw std::runtime_error("Failed to create listening socket");
	}
	if (setsockopt(_listeningSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		throw std::runtime_error("Failed to set socket options");
	}
	if (setsockopt(_listeningSocket, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6only, sizeof(ipv6only)) == -1) {
		throw std::runtime_error("Failed to set socket options");
	}
	address.sin6_family = AF_INET6;
	address.sin6_addr = in6addr_any;
	address.sin6_port = htons(_port);
	if (bind(_listeningSocket, (struct sockaddr *)&address, sizeof(address)) == -1) {
		throw std::runtime_error("Failed to bind listening socket");
	}
	if (listen(_listeningSocket, SOMAXCONN) == -1) {
		throw std::runtime_error("Failed to listen on listening socket");
	}
	int flags = fcntl(_listeningSocket, F_GETFL, 0);
	if (fcntl(_listeningSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
		throw std::runtime_error("Failed to set non-blocking mode for listening socket");
	}
}

void PortListener::_setupEpoll() {
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd == -1) {
        std::cerr << "Failed to create epoll file descriptor" << std::endl;
        return;
    }

    struct epoll_event event;
    event.data.fd = _listeningSocket;
    event.events = EPOLLIN | EPOLLET | EPOLLOUT;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _listeningSocket, &event) == -1) {
        std::cerr << "Failed to add server socket to epoll" << std::endl;
        close(_epoll_fd);
    }
}

void PortListener::startListening() {
    _keepRunning = true;
        if (pthread_create(&_listeningThread, NULL, &PortListener::listenThreadStatic, this)) {
        std::cerr << "Erreur lors de la création du thread" << std::endl;
        return; // Assurez-vous que le type de retour est void si vous ne retournez pas de valeur
    }
}

void PortListener::stopListening() {
    if (_keepRunning) {
        _keepRunning = false;
        pthread_join(_listeningThread, NULL);
        if (_server_fd != -1) {
            close(_server_fd);
        }
        if (_epoll_fd != -1) {
            close(_epoll_fd);
        }
    }
}

void* PortListener::listenThreadStatic(void* arg) {
    PortListener* self = static_cast<PortListener*>(arg);
    self->_listenThread();
    return NULL;
}

void PortListener::_listenThread() {
    _log.info("Listenig on thread");
    _setupListeningSocket();
    _setupEpoll();
    const int MAX_EVENTS = 10;
    struct epoll_event events[MAX_EVENTS];

    while (_keepRunning) {
        int event_count = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == _listeningSocket) {
                _acceptConnection();
            } else {
                if (events[i].events & EPOLLIN) {
                    _log.info("EPOLLIN evt");
                }
                if (events[i].events & EPOLLOUT) {
                    _log.info("EPPOLOUT evt");
                }
            }
        }
    }
}

void PortListener::_acceptConnection() {
    struct sockaddr_in address;
    socklen_t          addrlen = sizeof(address);
    int                new_socket;

    _log.info("accepting conn");
    while ((new_socket = accept(_listeningSocket, (struct sockaddr*)&address,
                              &addrlen)) != -1) {
    int flags = fcntl(new_socket, F_GETFL, 0);
    if (flags == -1 || fcntl(new_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
      ErrorHandler::log("Failed to set new socket to non-blocking");
      _log.error("(PortListener) Failed to set new socket to non-blocking");
      continue;
    }
    _client_sockets.insert(new_socket);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET | EPOLLOUT;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, new_socket, &event) == -1) {
      ErrorHandler::log("Failed to add new socket to epoll");
      _log.error("(PortListener) Failed to add new socket to epoll");
      close(new_socket);
      _client_sockets.erase(new_socket);
    }
    else
    {
        _log.info("NEW CON TO HANDLE");
        _connectionHandler->handleConnection(new_socket);
    }
    // else
        // std::thread(&ConnectionHandler::handleConnection, _connectionHandler, new_socket).detach();
  } // Close the socket or handle it appropriately
}
