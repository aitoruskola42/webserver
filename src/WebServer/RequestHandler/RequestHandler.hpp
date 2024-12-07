/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/01 17:32:53 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/02 12:30:47 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "CommonDefinitions.hpp"
#include "ConfigFileParse/ConfigurationManager.hpp"
#include "DeleteHandler/DeleteHandler.hpp"
#include "GetHandler/GetHandler.hpp"
#include "HeadHandler/HeadHandler.hpp"
#include "Logger/includes/Logger.hpp"
#include "PostHandler/PostHandler.hpp"
#include "WebServer/WebServer.hpp"

class RequestHandler {
  // ---------------ATTRIBUTES-------------------------------------------------
 private:
  ConfigurationManager &config;
  GetHandler            getHandler;

  // ---------------CONSTRUCTORS-------------------------------------------------
 public:
  RequestHandler(ConfigurationManager &config);

  // ---------------METHODS------------------------------------------------------
 public:
  bool         isRequestComplete(const ClientInfo &client) const;
  std::string  get_root_path(int server_port);
  SocketResult handle_request(int         client_socket,
                              const char *request_data,
                              size_t      request_size,
                              int         server_port,
                              int         client_id,
                              size_t     *bytes_read);

 private:
  SocketResult read_request(int          client_socket,
                            std::string &request,
                            size_t      *bytes_read);
  SocketResult send_response(int                client_socket,
                             const std::string &content_type,
                             const std::string &content,
                             int                status_code,
                             bool               keep_alive);
  void send_file(int client_socket, const std::string &filename, bool keep_alive);
  void        log_request(const std::string &message, const std::string &url);
  std::string get_current_time();
  void        save_parsing_request_to_file(const RequestParser &request);
  void        save_request_to_file(const std::string &request,
                                   const char        *client_ip,
                                   int                client_port);
  void        log_server_config(const Server &server, size_t server_num);
  void        log_error_pages(const std::map<int, std::string> &error_pages);

  void log_return_codes(const std::map<short int, std::string> &return_codes);
  std::string join_strings(const std::vector<std::string> &strings,
                           const std::string              &delimiter);
  void log_cgi_handler(const std::map<std::string, std::string> &cgi_handler);
  SocketResult  read_and_parse_request(int          client_socket,
                                       std::string &request,
                                       bool        &keep_alive,
                                       size_t      *bytes_read);
  std::string   get_hostname(const std::string &host_header);
  const Server *get_server(const std::string &hostname, int server_port);

  SocketResult handle_get_request(int                   client_socket,
                                  const RequestParser  &parser,
                                  const LocationConfig &config,
                                  bool                  keep_alive);
  SocketResult handle_head_request(int                   client_socket,
                                   RequestParser        &parser,
                                   const LocationConfig &config,
                                   bool                  keep_alive);
  SocketResult handle_post_request(int                   client_socket,
                                   const LocationConfig &config,
                                   RequestParser        &request,
                                   bool                  keep_alive);
  SocketResult handle_delete_request(int                   client_socket,
                                     const LocationConfig &config,
                                     RequestParser        &request,
                                     bool                  keep_alive);
  SocketResult handle_unsupported_method(int client_socket, bool keep_alive);

  // ---------------UTILS------------------------------------------------------
  LocationConfig create_location_config(const Server *server);
};

#endif // REQUEST_HANDLER_HPP