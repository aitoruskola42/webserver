#include "RequestHandler.hpp"

#include "../../ConfigFileParse/ConfigurationManager.hpp"
#include "../../RequestParser/RequestParser.hpp"
#include "../WebServer.hpp"
#include "CommonDefinitions.hpp"

RequestHandler::RequestHandler(ConfigurationManager &config) : config(config), getHandler(config) {
  LOG_DEBUG("RequestHandler initialized");
}

SocketResult RequestHandler::handle_request(int         client_socket,
                                            const char *request_data,
                                            size_t      request_size,
                                            int         server_port,
                                            int         client_id,
                                            size_t     *bytes_read) {
  if (bytes_read == NULL) {
    LOG_ERROR("bytes_read pointer is null");
    return SOCKET_ERROR;
  }

  LOG_DEBUG("Handling request on socket: " << client_socket << ", client ID: " << client_id
                                          << ", server_port : " << server_port << ", request size: " << request_size);

  RequestParser parser;
  parser.parseRequest(std::string(request_data, request_size));

  std::string request_method = parser.getMethod();
  std::string request_path   = parser.getPath();
  std::string hostname       = get_hostname(parser.getHeader("Host"));

  LocationConfig loc_config;
  Server        *server = config.get_server(hostname, server_port, request_path);
  loc_config            = create_location_config(server);

  if (parser.getErrorCode() && !parser.isComplete()) {
    LOG_ERROR("Parsing error or incomplete request");
    return HttpUtils::sendErrorResponse(client_socket, parser.getErrorCode(), true, loc_config);
  }
  if (parser.getBody().size() > loc_config.client_max_body_size) {
    LOG_WARNING("Client maximun size exceeded.");
    return HttpUtils::sendErrorResponse(client_socket, 413, true, loc_config);
  }

// PARSER DEL REQUEST
  //save_request_to_file(std::string(request_data, request_size), "", 0);
  //save_parsing_request_to_file(parser);


  SocketResult method_result;

  LOG_DEBUG("Received request method: " << request_method);

  if (request_method == "GET") {
    method_result = handle_get_request(client_socket, parser, loc_config, true);
  } else if (request_method == "POST") {
    method_result = handle_post_request(client_socket, loc_config, parser, true);
  } else if (request_method == "DELETE") {
    method_result = handle_delete_request(client_socket, loc_config, parser, true);
  } else if (request_method == "PUT") {
    method_result = HttpUtils::sendErrorResponse(client_socket, 405, true, loc_config);
  } else if (request_method == "HEAD") {
    method_result = HttpUtils::sendErrorResponse(client_socket, 405, true, loc_config);
  } else {
    method_result = handle_unsupported_method(client_socket, true);
  }

  *bytes_read = request_size;
  return method_result;
}



SocketResult
RequestHandler::read_and_parse_request(int client_socket, std::string &request, bool &keep_alive, size_t *bytes_read) {
  char    buffer[4096];
  ssize_t bytes_read_now = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

  if (bytes_read_now > 0) {
    buffer[bytes_read_now] = '\0';
    request.append(buffer, bytes_read_now);
    *bytes_read += bytes_read_now;

    keep_alive = (request.find("Connection: keep-alive") != std::string::npos);

    LOG_INFO("Received " << bytes_read_now << " bytes. Total: " << *bytes_read);

    return SOCKET_OK;
  } else if (bytes_read_now == 0) {
    LOG_INFO("Client closed the connection");
    return SOCKET_CLOSED;
  } else {
    if (bytes_read_now < 0) {
        LOG_ERROR("Error reading from socket");
        return SOCKET_ERROR;
    }
   
  }
 return SOCKET_WOULD_BLOCK;
}

std::string RequestHandler::get_hostname(const std::string &host_header) {
  size_t colonPos = host_header.find(':');
  return (colonPos != std::string::npos) ? host_header.substr(0, colonPos) : host_header;
}

SocketResult RequestHandler::handle_get_request(int                   client_socket,
                                                const RequestParser  &parser,
                                                const LocationConfig &config,
                                                bool                  keep_alive) {
  return getHandler.handle_get(client_socket, parser, config, keep_alive);
}

SocketResult RequestHandler::handle_head_request(int                   client_socket,
                                                 RequestParser        &parser,
                                                 const LocationConfig &config,
                                                 bool                  keep_alive) {
  HeadHandler head(client_socket, config, parser, keep_alive);
  return head.processHead();
}

SocketResult RequestHandler::handle_post_request(int                   client_socket,
                                                 const LocationConfig &config,
                                                 RequestParser        &request,
                                                 bool                  keep_alive) {
  PostHandler post(client_socket, config, request, keep_alive);
  return post.processPost();
}

SocketResult RequestHandler::handle_delete_request(int                   client_socket,
                                                   const LocationConfig &config,
                                                   RequestParser        &request,
                                                   bool                  keep_alive) {
  DeleteHandler delete_obj(client_socket, config, request, keep_alive);
  return delete_obj.handle_delete_request();
}

SocketResult RequestHandler::handle_unsupported_method(int client_socket, bool keep_alive) {
  std::string error_message = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
  return send_response(client_socket, "text/html", error_message, 405, keep_alive);
}

SocketResult RequestHandler::read_request(int client_socket, std::string &request, size_t *bytes_read) {
  char    buffer[4096];
  ssize_t bytes_read_now = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

  if (bytes_read_now > 0) {
    buffer[bytes_read_now] = '\0';
    request.append(buffer, bytes_read_now);
    *bytes_read += bytes_read_now;

    LOG_INFO("Received " << bytes_read_now << " bytes. Total: " << *bytes_read);

    return SOCKET_OK;
  } else if (bytes_read_now == 0) {
    LOG_INFO("Client closed the connection");
    return SOCKET_CLOSED;
  } else {
    LOG_ERROR("Error reading from socket");
    return SOCKET_ERROR;
  }
}

SocketResult RequestHandler::send_response(int                client_socket,
                                           const std::string &content_type,
                                           const std::string &content,
                                           int                status_code,
                                           bool               keep_alive) {
  std::string status_line;
  switch (status_code) {
    case 200:
      status_line = "HTTP/1.1 200 OK";
      break;
    case 404:
      status_line = "HTTP/1.1 404 Not Found";
      break;
    case 405:
      status_line = "HTTP/1.1 405 Method Not Allowed";
      break;
    default:
      status_line = "HTTP/1.1 500 Internal Server Error";
  }

  LOG_INFO(status_line);
  std::string response = status_line + "\r\n";
  response += "Content-Type: " + content_type + "\r\n";

  std::ostringstream content_length_stream;
  content_length_stream << content.length();
  response += "Content-Length: " + content_length_stream.str() + "\r\n";

  if (keep_alive) {
    response += "Connection: keep-alive\r\n";
    response += "Keep-Alive: timeout=5, max=1000\r\n";
  } else {
    response += "Connection: close\r\n";
  }

  response += "\r\n";
  response += content;

  size_t total_sent = 0;
  size_t bytes_left = response.length();

  while (total_sent < response.length()) {
    ssize_t bytes_sent = send(client_socket, response.c_str() + total_sent, bytes_left, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (bytes_sent > 0) {
      total_sent += bytes_sent;
      bytes_left -= bytes_sent;
    } else if (bytes_sent == 0) {
      return SOCKET_CLOSED;
    } else {
      LOG_ERROR("Error sending response");
      return SOCKET_ERROR;
    }
  }

  return SOCKET_OK;
}

void RequestHandler::send_file(int client_socket, const std::string &filename, bool keep_alive) {
  std::ifstream file(filename.c_str(), std::ios::binary);
  if (file) {
    std::string        content_type = HttpUtils::getContentType(filename);
    std::ostringstream file_contents;
    file_contents << file.rdbuf();
    SocketResult result = send_response(client_socket, content_type, file_contents.str(), 200, keep_alive);
    if (result != SOCKET_OK) {
      LOG_ERROR("Error sending file: " << filename);
    }
  } else {
    std::string  error_message = "<html><body><h1>404 Not Found</h1></body></html>";
    SocketResult result        = send_response(client_socket, "text/html", error_message, 404, keep_alive);
    if (result != SOCKET_OK) {
      LOG_ERROR("Error sending 404 response");
    }
  }
}



std::string RequestHandler::get_current_time() {
  time_t    now = time(0);
  struct tm tstruct;
  char      buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
  return std::string(buf);
}

void RequestHandler::save_request_to_file(const std::string &request, const char *client_ip, int client_port) {
  std::string filename = "requests.txt";
  (void)client_ip;
  (void)client_port;
  // Open the file in append mode
  std::ofstream file(filename.c_str(), std::ios::app);
  LOG_WARNING("Saving request to file");
  if (file.is_open()) {
    // Add a separator and metadata before each new request
    file << "\n----- New Request -----\n";
    file << request << "\n";
    file << "----- End of Request -----\n\n";

    file.close();
    LOG_INFO("Request appended to file: " << filename);
  } else {
    LOG_ERROR("Unable to open file for appending request: " << filename);
  }
}

void RequestHandler::save_parsing_request_to_file(const RequestParser &request) {
  std::string filename = "parsed_requests.txt";

  // Open the file in append mode
  std::ofstream file(filename.c_str(), std::ios::app);
  LOG_WARNING("Saving parsed request to file");
  if (file.is_open()) {
    // Add a separator and metadata before each new request
    file << "\n----- New Request -----\n";
    file << request << "\n";
    file << "----- End of Request -----\n\n";

    file.close();
    LOG_INFO("Request appended to file: " << filename);
  } else {
    LOG_ERROR("Unable to open file for appending request: " << filename);
  }
}

std::string RequestHandler::get_root_path(int server_port) {
  std::vector<Server> servers = config.get_servers();
  for (size_t i = 0; i < servers.size(); ++i) {
    if (servers[i].getListen() == server_port) {
      return servers[i].getRootPath();
    }
  }
  return servers[0].getRootPath();
}

std::string RequestHandler::join_strings(const std::vector<std::string> &strings, const std::string &delimiter) {
  std::string result;
  for (size_t i = 0; i < strings.size(); ++i) {
    if (i > 0)
      result += delimiter;
    result += strings[i];
  }
  return result;
}

LocationConfig RequestHandler::create_location_config(const Server *server) {
  LocationConfig config;

  config.location_path        = server->getLocationPath();
  config.root_path            = server->getRootPath();
  config.autoindex            = server->getAutoindex();
  config.client_max_body_size = server->getClientMaxBodySize();
  config.allowed_methods      = server->getAllowedMethods();
  config.error_pages          = server->getErrorPages();
  config.index_files          = server->getIndex();
  config.cgi_extensions       = server->getLocationCgiHandler();
  config.upload_path          = server->getUploadPath();
  config.return_code_path     = server->getReturnCodePath();
  LOG_DEBUG("Using location-specific configuration for path: " << server->getLocationPath());
  return config;
}

bool RequestHandler::isRequestComplete(const ClientInfo &client) const {
    size_t header_end = client.partial_request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return false;
    }

    std::string headers = client.partial_request.substr(0, header_end);
    size_t content_length_pos = headers.find("Content-Length: ");
    if (content_length_pos != std::string::npos) {
        size_t content_length_end = headers.find("\r\n", content_length_pos);
        std::string content_length_str = headers.substr(content_length_pos + 16, content_length_end - (content_length_pos + 16));
        size_t content_length = std::atoi(content_length_str.c_str());
        return client.bytes_received >= (header_end + 4 + content_length);
    }

    // Si no hay Content-Length, asumimos que la solicitud está completa si tenemos los encabezados
    return true;
}