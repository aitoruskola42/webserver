/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/01 17:35:55 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/05 12:08:11 by xamayuel         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "WebServer.hpp"
#include "Logger/includes/Logger.hpp"
#include "WebServer/RequestHandler/RequestHandler.hpp"

volatile sig_atomic_t g_shutdownRequested = 0;

extern "C" void signalHandler(int signum) {
  if (signum == SIGINT || signum == SIGTERM) {
    g_shutdownRequested = 1;
  }
}

WebServer::WebServer(ConfigurationManager &config) : config(config), next_client_id(1), active_connections(0) {
  request_handler = new RequestHandler(config);
  if (request_handler == NULL) {
    LOG_ERROR("Failed to create RequestHandler");
    // Maneja el error apropiadamente
  }
}

WebServer::~WebServer() {
  LOG_DEBUG("Shutting down WebServer");

  delete request_handler;

  for (size_t i = 0; i < server_fds.size(); ++i) {
    if (server_fds[i] != -1) {
      close(server_fds[i]);
      LOG_DEBUG("Closed server socket on port: " << ports[i]);
    }
  }
}

void WebServer::addPort(int port, const char *ip) {
  std::ostringstream logMsg;
  logMsg << "Adding port " << port << " with IP " << ip;
  LOG_INFO(logMsg.str());

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);

  if (inet_pton(AF_INET, ip, &(addr.sin_addr)) <= 0) {
    LOG_ERROR("Invalid address: " << ip);
    return;
  }

  ports.push_back(port);
  server_fds.push_back(-1);
  addresses.push_back(addr);
}

void WebServer::run() {
  struct sigaction sa;
  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  const int MAX_RESTART_ATTEMPTS = 10;
  int       restartAttempts      = 0;
  bool      shouldRestart        = false;
  bool      shouldShutdown       = false;

  do {
    shouldRestart  = false;
    shouldShutdown = false;

    if (!initializeSockets()) {
      LOG_DEBUG("Failed to initialize sockets");
      break;
    }

    fd_set master_set, read_fds, write_fds;
    int    max_fd = initializeFdSets(master_set);

    struct timeval timeout;
    timeout.tv_sec  = 5;
    timeout.tv_usec = 0;

    while (!g_shutdownRequested && !shouldRestart && !shouldShutdown) {
      int selectResult = handleSelect(master_set, read_fds, write_fds, max_fd, timeout);

      switch (selectResult) {
        case 0: // SELECT_OK
          handleNewConnections(master_set, read_fds, max_fd);
          handleExistingConnections(master_set);
          break;
        case 1: // SELECT_TIMEOUT
          // No necesita hacer nada, ya se manejan las conexiones inactivas en handleSelect
          break;
        case 2: // SELECT_ERROR
          cleanupConnectionsRestart();
          restartAttempts++;
          shouldRestart = true;
          break;
        case 3: // SELECT_SHUTDOWN
          shouldShutdown = true;
          break;
      }
    }

    if (shouldRestart) {
      // Espera un poco antes de reiniciar
      sleep(1);
      LOG_DEBUG("Restarting... (" << restartAttempts << " of " << MAX_RESTART_ATTEMPTS << ")");
    }

  } while (shouldRestart && restartAttempts < MAX_RESTART_ATTEMPTS && !g_shutdownRequested);

  if (restartAttempts >= MAX_RESTART_ATTEMPTS) {
    LOG_CRITICAL("Restart attempts exceeded. Shutting down.");
  }

  cleanupConnections();
}

bool WebServer::initializeSockets() {
  static int sockets_open = 0;
  for (size_t i = 0; i < ports.size(); ++i) {
    create_socket(i);
    int opt = 1;
    if (setsockopt(server_fds[i], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      LOG_ERROR("setsockopt(SO_REUSEADDR) failed for port " << ports[i]);
    }
    sockets_open++;
    sockets_open += bind_socket(i);
    if (server_fds[i] != -1) {
      listen_socket(i);
    }
    
  }
  if (sockets_open == 0) {
    LOG_ERROR("No sockets opened. Sending exit signal");
    return false;
  }
  return true;
}

int WebServer::initializeFdSets(fd_set &master_set) {
  int max_fd = -1;
  FD_ZERO(&master_set);
  for (size_t i = 0; i < server_fds.size(); ++i) {
    FD_SET(server_fds[i], &master_set);
    if (server_fds[i] > max_fd) {
      max_fd = server_fds[i];
    }
  }
  return max_fd;
}

int WebServer::handleSelect(fd_set &master_set, fd_set &read_fds, fd_set &write_fds, int max_fd, struct timeval &timeout) {
    read_fds = master_set;
    FD_ZERO(&write_fds);

    for (std::vector<ClientInfo>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->waiting_to_write || HttpUtils::hasFileState(it->socket)) {
            FD_SET(it->socket, &write_fds);
        }
    }

    struct timeval tmp_timeout = timeout;
    int            activity    = select(max_fd + 1, &read_fds, &write_fds, NULL, &tmp_timeout);

    if (activity < 0) {
        if (g_shutdownRequested) {
            std::cout << "\033[2J\033[1;1H"; // Borrar la pantalla
            LOG_SUCCESS("Received exit signal, shutting down gracefully");
            return 3;
        }
        return 2;
    } else if (activity == 0) {
        //LOG_INFO("Tiempo de espera de select agotado - realizando tareas periódicas");
        checkIdleConnections();
        return 1;
    }

    // Actualizar el estado de lectura/escritura de cada cliente
    for (std::vector<ClientInfo>::iterator it = clients.begin(); it != clients.end(); ++it) {
        it->ready_for_read  = FD_ISSET(it->socket, &read_fds);
        it->ready_for_write = FD_ISSET(it->socket, &write_fds);
    }

    return 0;
}

void WebServer::handleNewConnections(fd_set &master_set, const fd_set &read_fds, int &max_fd) {
  for (size_t i = 0; i < server_fds.size(); ++i) {
    if (FD_ISSET(server_fds[i], &read_fds)) {
      sockaddr_in client_addr;
      socklen_t   addrlen    = sizeof(client_addr);
      int         new_socket = accept(server_fds[i], (struct sockaddr *)&client_addr, &addrlen);

      if (new_socket < 0) {
        LOG_ERROR("Error accepting connection on port " << ports[i]);
        continue;
      }

      if (getActiveConnections() >= config.get_max_clients()) {
        LOG_WARNING("Server overloaded. Rejecting new connection. Active connections: " << getActiveConnections());
        send(new_socket, SERVER_BUSY_RESPONSE, strlen(SERVER_BUSY_RESPONSE), MSG_DONTWAIT | MSG_NOSIGNAL);
        close(new_socket);
        continue;
      }

      // Verificar si ya existe una conexión para este socket
      bool socket_exists = false;
      for (std::vector<ClientInfo>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->socket == new_socket) {
          socket_exists = true;
          break;
        }
      }

      if (!socket_exists) {
        LOG_SUCCESS("New connection accepted on socket: " << new_socket << ", on port " << ports[i]
                                                       << ", client ID: " << next_client_id);
        configureClientSocket(new_socket);
        FD_SET(new_socket, &master_set);
        if (new_socket > max_fd) {
          max_fd = new_socket;
        }
        clients.push_back(ClientInfo(new_socket, next_client_id++, ports[i]));
        update_last_activity(new_socket);
        incrementActiveConnections();
        LOG_INFO("Active connections: " << getActiveConnections());
        usleep(1); 
      } else {
        LOG_WARNING("Attempted to accept a connection on an existing socket: " << new_socket);
        close(new_socket);
      }
    }
  }
}


void WebServer::handleExistingConnections(fd_set &master_set) {
    time_t current_time = time(NULL);

    for (std::vector<ClientInfo>::iterator it = clients.begin(); it != clients.end();) {
        int client_socket = it->socket;
        int server_port   = it->port;

        bool should_close = false;

        // Comprobar si la conexión ha estado inactiva por demasiado tiempo
        if (difftime(current_time, it->last_activity) > config.get_keep_alive_timeout()) {
            LOG_INFO("Connection idle for too long on socket " << client_socket << ", client ID: " << it->id);
            should_close = true;
        }

        if (it->ready_for_read) {
            LOG_DEBUG("Activity on socket " << client_socket << " (read), client ID: " << it->id);

            char    buffer[4096];
            ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer), MSG_DONTWAIT);

            if (bytes_read > 0) {
                it->partial_request.append(buffer, bytes_read);
                it->bytes_received += bytes_read;
                it->last_activity = current_time;

                if (!it->request_complete && request_handler->isRequestComplete(*it)) {
                    it->request_complete = true;
                }

                if (it->request_complete) {
                    SocketResult result = request_handler->handle_request(client_socket,
                                                                          it->partial_request.c_str(),
                                                                          it->partial_request.size(),
                                                                          server_port,
                                                                          it->id,
                                                                          &it->bytes_received);

                    if (result == SOCKET_CLOSED) {
                        should_close = true;
                    } else if (result == SOCKET_ERROR) {
                        LOG_ERROR("Error handling request for client ID: " << it->id);
                        should_close = true;
                    } else {
                        // Reiniciar para la próxima solicitud
                        it->partial_request.clear();
                        it->request_complete = false;
                        it->content_length   = 0;
                        it->bytes_received   = 0;
                    }
                }
            } else if (bytes_read == 0) {
                LOG_SUCCESS("Client closed connection for client ID: " << it->id);
                should_close = true;
            } else if (bytes_read < 0) {
                LOG_ERROR("Error reading from socket for client ID: " << it->id);
                should_close = true;
            }
        }

        if (it->ready_for_write && !should_close) {
            LOG_DEBUG("Activity on socket " << client_socket << " (write), client ID: " << it->id);

            if (HttpUtils::hasFileState(client_socket)) {
                SocketResult result = HttpUtils::sendChunkedFileNonBlocking(client_socket, true, LocationConfig());
                if (result == SOCKET_OK) {
                    // Transferencia completa
                    it->waiting_to_write = false;
                } else if (result == SOCKET_WOULD_BLOCK) {
                    // Continuará en la próxima iteración
                    it->waiting_to_write = true;
                } else {
                    // Error, cerrar conexión
                    LOG_ERROR("Error sending file on socket " << client_socket);
                    should_close = true;
                }
            }
        }

        if (should_close) {
            LOG_DEBUG("Closing connection for client ID: " << it->id);
            FD_CLR(client_socket, &master_set);
            close(client_socket);
            HttpUtils::removeFileState(client_socket);
            it = clients.erase(it);
            decrementActiveConnections();
            LOG_INFO("Active connections: " << getActiveConnections());
        } else {
            ++it;
        }
    }
}

void WebServer::cleanupConnections() {
  std::cout << std::endl;
  LOG_INFO("Received exit signal, shutting down gracefully");

  for (std::vector<ClientInfo>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
    if (it->socket != -1) {
      close(it->socket);
      LOG_DEBUG("Closed client socket: " << it->socket << ", ID: " << it->id);
    }
  }
  clients.clear();
  for (size_t i = 0; i < server_fds.size(); ++i) {
    if (server_fds[i] != -1) {
      close(server_fds[i]);
      LOG_DEBUG("Closed server socket on port: " << ports[i]);
      server_fds[i] = -1;
    }
  }

  last_activity_map.clear();
  client_to_server_port.clear();

  LOG_SUCCESS("🧹 All cleaned up 🧹 . See you next time! 👋");
}

void WebServer::cleanupConnectionsRestart() {
  std::cout << std::endl;

  for (std::vector<ClientInfo>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
    if (it->socket != -1) {
      close(it->socket);
      LOG_DEBUG("Closed client socket: " << it->socket << ", ID: " << it->id);
    }
  }
  clients.clear();
  for (size_t i = 0; i < server_fds.size(); ++i) {
    if (server_fds[i] != -1) {
      close(server_fds[i]);
      LOG_DEBUG("Closed server socket on port: " << ports[i]);
      server_fds[i] = -1;
    }
  }

  last_activity_map.clear();
  client_to_server_port.clear();
}

void WebServer::configureClientSocket(int client_socket) {
  struct timeval client_timeout;
  client_timeout.tv_sec  = 5;
  client_timeout.tv_usec = 0;
  if (setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &client_timeout, sizeof(client_timeout)) < 0) {
    LOG_ERROR("Error setting socket send timeout." );
  }
  if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &client_timeout, sizeof(client_timeout)) < 0) {
    LOG_ERROR("Error setting socket receive timeout.");
  }
}

int WebServer::create_socket(int index) {
  server_fds[index] = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fds[index] < 0) {
    std::ostringstream errorMsg;
    errorMsg << "Failed to create socket: " << strerror(errno);
    LOG_ERROR(errorMsg.str());
    throw std::runtime_error(errorMsg.str());
  }
  std::ostringstream logMsg;
  logMsg << "Created socket for port " << ports[index];
  LOG_INFO(logMsg.str());
  return server_fds[index];
}

int WebServer::bind_socket(int index) {
  if (bind(server_fds[index], (struct sockaddr *)&addresses[index], sizeof(addresses[index])) < 0) {
    std::ostringstream errorMsg;
    errorMsg << "Failed to bind socket on port " << ports[index] << ": " << strerror(errno);
    LOG_ERROR(errorMsg.str());
    close(server_fds[index]);
    server_fds[index] = -1;
    return -1;
  } else {
    LOG_SUCCESS("Successfully bound socket on port " << ports[index]);
    return 0;
  }
}

void WebServer::listen_socket(int index) {
  if (listen(server_fds[index], 10) < 0) {
    throw std::runtime_error("Failed to listen on socket");
  }
}

void WebServer::update_last_activity(int client_socket) {
  last_activity_map[client_socket] = time(NULL);
}

time_t WebServer::get_last_activity(int client_socket) {
  std::map<int, time_t>::iterator it = last_activity_map.find(client_socket);
  if (it != last_activity_map.end()) {
    return it->second;
  }
  return 0;
}

void WebServer::check_idle_connections(fd_set &master_set) {
  time_t current_time       = time(NULL);
  int    keep_alive_timeout = config.get_keep_alive_timeout();

  std::vector<ClientInfo>::iterator it = clients.begin();
  while (it != clients.end()) {
    int    client_socket = it->socket;
    time_t last_activity = get_last_activity(client_socket);
    if (current_time - last_activity > keep_alive_timeout) {
      LOG_INFO("Closing idle connection on socket " << client_socket << ", client ID: " << it->id);
      FD_CLR(client_socket, &master_set);
      close(client_socket);
      last_activity_map.erase(client_socket);
      it = clients.erase(it);
      decrementActiveConnections();
      LOG_INFO("Idle connection closed. Active connections: " << getActiveConnections());
    } else {
      ++it;
    }
  }
}

void WebServer::incrementActiveConnections() {
  ++active_connections;
}

void WebServer::decrementActiveConnections() {
  --active_connections;
}

int WebServer::getActiveConnections() const {
  return active_connections;
}

const char WebServer::SERVER_BUSY_RESPONSE[] =
    "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 21\r\n\r\nServer is "
    "overloaded.";

void WebServer::checkIdleConnections() {
    time_t current_time = time(NULL);
    std::vector<ClientInfo>::iterator it = clients.begin();
    while (it != clients.end()) {
        if (difftime(current_time, it->last_activity) > config.get_keep_alive_timeout()) {
            LOG_INFO("Closing idle connection on socket " << it->socket << ", client ID: " << it->id);
            close(it->socket);
            it = clients.erase(it);
            decrementActiveConnections();
        } else {
            ++it;
        }
    }
}