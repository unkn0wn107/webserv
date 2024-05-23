/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 14:24:50 by mchenava          #+#    #+#             */
/*   Updated: 2024/05/23 12:14:09 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"

Worker::Worker(Config& config):
	_config(config),
	_log(Logger::getInstance()),
	_maxConnections(config.worker_connections),
	_currentConnections(0)
{
	if (pthread_create(&_thread, NULL, _workerRoutine, this) != 0)
	{
		_log.error("WORKER : Failed to create thread proc");
	}
}

Worker::~Worker()
{
	pthread_join(_thread, NULL);
}

int		Worker::_setupEpoll()
{
	int	socket = epoll_create1(0);
	if (socket == -1)
	{
		_log.error("WORKER: Failed \"epoll_create1\"");
		exit(EXIT_FAILURE);
	}
	return socket;
}

void	Worker::assignConnection(int clientSocket)
{
	struct epoll_event event;
    event.data.fd = clientSocket;
    event.events = EPOLLIN | EPOLLOUT;

	if (_currentConnections >= _maxConnections) {
        _log.error("Nombre maximum de connexions atteint");
        close(clientSocket);
    } else {
		if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
        _log.error("Erreur lors de l'ajout du socket à epoll");
        close(clientSocket);
    	}
        _currentConnections++;
	}
}

void Worker::_runEventLoop() {
    struct epoll_event events[MAX_EVENTS];
    int nfds, fd;

    while (true) {
        nfds = epoll_wait(_epollSocket, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            _log.error("Erreur lors de l'attente des événements epoll");
            break;
        }

        for (int n = 0; n < nfds; ++n) {
            fd = events[n].data.fd;
            if (events[n].events & EPOLLIN) {
                _handleIncomingConnection(fd);
            }
            if (events[n].events & EPOLLOUT) {
                _handleOutgoingData(fd);
            }
        }
    }
}

void Worker::_handleIncomingConnection(int fd) {
    // Gérer une nouvelle connexion entrante
    int clientSocket = accept(fd, NULL, NULL);
    if (clientSocket == -1) {
        _log.error("Erreur lors de l'acceptation de la connexion");
        return;
    }
    _log.info("Nouvelle connexion entrante");
}

void	Worker::_handleOutgoingData(int fd) {
    _log.info("Données sortantes" + Utils::to_string(fd));
}

void*	Worker::_workerRoutine(void *ref)
{
	Worker* worker = static_cast<Worker*>(ref);
	worker->_epollSocket = worker->_setupEpoll();
	worker->_runEventLoop();
	return NULL;
}
