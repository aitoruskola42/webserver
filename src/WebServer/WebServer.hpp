/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/01 18:04:09 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/03 17:58:25 by xamayuel         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

// -----------------------------------------------------------------------------
#include "ConfigFileParse/ConfigurationManager.hpp"
// -----------------------------------------------------------------------------
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

class RequestHandler;

class WebServer {
  // -----------CONSTRUCTOR AND DESTRUCTOR--------------------------------
 public:
  WebServer(ConfigurationManager &config);
  ~WebServer();

  // -----------PUBLIC METHODS----------------------------------------------
 public:
  void addPort(int port, const char *ip = "0.0.0.0");
  void run();
  void cleanup();
  bool isShutdownRequested() const;
  void logConfiguration() const;

  // ----------------------ATTRIBUTES------------------------------------------
 private:
  std::vector<int>         ports;
  std::vector<int>         server_fds;
  std::vector<sockaddr_in> addresses;
  RequestHandler          *request_handler;
  std::map<int, time_t>    last_activity_map;
  ConfigurationManager    &config;
  std::map<int, int>       client_to_server_port;
  std::vector<ClientInfo>  clients;
  int                      next_client_id;
  int                      active_connections;
  

 private:
  bool   initializeSockets();
  int    handleSelect(fd_set         &master_set,
                      fd_set         &read_fds,
                      fd_set         &write_fds,
                      int             max_fd,
                      struct timeval &timeout);
  int    initializeFdSets(fd_set &master_set);
  int    create_socket(int index);
  int    get_client_server_port(int client_socket);
  int    accept_connection(int index);
  int    getActiveConnections() const;
  time_t get_last_activity(int client_socket);
  void   check_idle_connections(fd_set &master_set);
  void   add_client_server_port(int client_socket, int server_port);
  void   logServerConfig(const Server &server, size_t server_num) const;
  void   logErrorPages(const std::map<int, std::string> &error_pages,
                       int                               spaces) const;
  void logReturnCodes(const std::map<short int, std::string> &return_codes) const;
  std::string joinStrings(const std::vector<std::string> &strings,
                          const std::string              &delimiter) const;
  void logCgiExtensions(const std::map<std::string, std::string> &cgiExtensions,
                        int                                       spaces) const;
  void handleNewConnections(fd_set       &master_set,
                            const fd_set &read_fds,
                            int          &max_fd);
  void handleExistingConnections(fd_set &master_set);
  int bind_socket(int index);
  void listen_socket(int index);
  void update_last_activity(int client_socket);
  void cleanupConnections();
  void cleanupConnectionsRestart();
  void configureClientSocket(int client_socket);
  void incrementActiveConnections();
  void decrementActiveConnections();
  void checkIdleConnections();
  static const char SERVER_BUSY_RESPONSE[];

  std::map<int, std::pair<std::string, size_t> > pending_files;
  std::map<int, size_t>                          file_sizes;
};

#endif // WEBSERVER_HPP