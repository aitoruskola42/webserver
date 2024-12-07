
#include "WebServer.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include "../Logger/includes/Logger.hpp"
#include "RequestHandler/RequestHandler.hpp"
#include <fcntl.h>
volatile sig_atomic_t g_shutdownRequested = 0;

extern "C" void signalHandler(int signum) {
  if (signum == SIGINT || signum == SIGTERM) {
    g_shutdownRequested = 1;
  }
}

WebServer::WebServer(ConfigurationManager& config)
    : config(config), next_client_id(1), active_connections(0) {
  request_handler = new RequestHandler(config);
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

void WebServer::addPort(int port, const char* ip) {
  std::ostringstream logMsg;
  logMsg << "Adding port " << port << " with IP " << ip;
  LOG_INFO(logMsg.str());

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

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
  if (!initializeSockets()) {
    LOG_ERROR("Failed to initialize sockets");
    return;
  }

  fd_set master_set, read_fds, write_fds;
  int max_fd = initializeFdSets(master_set);

  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  while (!g_shutdownRequested) {
    if (!handleSelect(master_set, read_fds, write_fds, max_fd, timeout)) {
      continue;
    }
    handleNewConnections(master_set, read_fds, max_fd);
    handleExistingConnections(master_set);
    check_idle_connections(master_set);
  }

  cleanupConnections();
}

bool WebServer::initializeSockets() {
  for (size_t i = 0; i < ports.size(); ++i) {
    create_socket(i);
    int opt = 1;
    if (setsockopt(server_fds[i], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
  