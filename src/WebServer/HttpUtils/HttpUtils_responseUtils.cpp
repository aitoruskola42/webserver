
#include "HttpUtils.hpp"
/**
 * Sends an HTTP response to the client.
 *
 * @param client_socket The socket connected to the client.
 * @param content_type The MIME type of the content.
 * @param content The body of the response.
 * @param status_code The HTTP status code.
 * @param keep_alive Whether to keep the connection alive.
 * @return SOCKET_OK if successful, SOCKET_ERROR otherwise.
 */

SocketResult HttpUtils::sendResponse(int                client_socket,
                                     const std::string &content_type,
                                     const std::string &content,
                                     int                status_code,
                                     bool               keep_alive) {
  LOG_DEBUG("Starting to send response on socket: " << client_socket << ", status: " << status_code);

  std::string headers       = generateResponseHeaders(content_type, content.length(), status_code, keep_alive);
  std::string full_response = headers + content;

  size_t total_sent = 0;
  size_t bytes_left = full_response.length();

  while (total_sent < full_response.length()) {
    ssize_t bytes_sent = send(client_socket, full_response.c_str() + total_sent, bytes_left, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (bytes_sent > 0) {
      total_sent += bytes_sent;
      bytes_left -= bytes_sent;
    } else if (bytes_sent == 0) {
      LOG_INFO("Connection closed while sending response on socket: " << client_socket);
      return SOCKET_CLOSED;
    } else {
      LOG_ERROR("Error sending response on socket " << client_socket);
      return SOCKET_ERROR;
    }
  }
  LOG_SUCCESS("Finished sending response on socket: " << client_socket << " with status: " << status_code);
  
  return SOCKET_OK;
}

/**
 * Sends a file as an HTTP response to the client.
 *
 * @param client_socket The socket connected to the client.
 * @param filename The path to the file to be sent.
 * @param keep_alive Whether to keep the connection alive.
 * @param config The location configuration.
 * @return SOCKET_OK if successful, SOCKET_ERROR or error response otherwise.
 */

void HttpUtils::setFileState(int client_socket, FileState *state) {
  file_states[client_socket] = state;
}

SocketResult
HttpUtils::sendFile(int client_socket, const std::string &filename, bool keep_alive, const LocationConfig &config, int status_code) {
  LOG_DEBUG("Sending file on socket: " << client_socket << " (write), file: " << filename);

  // Comprobar si ya existe un estado para este socket
  if (hasFileState(client_socket)) {
    return sendFileContent(client_socket);
  }

  // Si no existe un estado, inicializar uno nuevo
  std::ifstream *file = new std::ifstream(filename.c_str(), std::ios::binary);
  if (!file->is_open()) {
    LOG_ERROR("Failed to open file: " << filename);
    delete file;
    return sendErrorResponse(client_socket, 500, keep_alive, config);
  }

  file->seekg(0, std::ios::end);
  size_t file_size = file->tellg();
  file->seekg(0, std::ios::beg);

  std::string content_type = getContentType(filename);
  std::string headers      = generateResponseHeaders(content_type, file_size, status_code, keep_alive);

  FileState *state    = new FileState();
  state->file         = file;
  state->file_size    = file_size;
  state->bytes_sent   = 0;
  state->headers_sent = false;

  setFileState(client_socket, state);

  // Enviar los headers
  if (!sendData(client_socket, headers.c_str(), headers.length())) {
    LOG_ERROR("Failed to send headers for file: " << filename);
    removeFileState(client_socket);
    return SOCKET_ERROR;
  }

  state->headers_sent = true;

  // Intentar enviar el contenido del archivo
  return sendFileContent(client_socket);
}

/**
 * Sends an error response to the client.
 *
 * @param client_socket The socket connected to the client.
 * @param status_code The HTTP error status code.
 * @param keep_alive Whether to keep the connection alive.
 * @param config The location configuration.
 * @return SOCKET_OK if successful, SOCKET_ERROR otherwise.
 */
SocketResult
HttpUtils::sendErrorResponse(int client_socket, int status_code, bool keep_alive, const LocationConfig &config) {
  std::map<int, std::string>::const_iterator it = config.error_pages.find(status_code);

  std::ifstream test_file(it->second.c_str());

  if (it != config.error_pages.end() && it->second.substr(it->second.find_last_of('.') + 1) == "html" && test_file.is_open()) {
    return sendFile(client_socket, it->second, keep_alive, config, status_code);
  } else {
    std::string error_message;
    error_message = "<html><body><h1>";
    error_message += getStatusMessage(status_code);
    error_message += "</h1></body></html>";
    return sendResponse(client_socket, "text/html", error_message, status_code, keep_alive);
  }
}
/**
 * Sends a redirect response to the client.
 *
 * @param client_socket The socket connected to the client.
 * @param redirect_url The URL to redirect to.
 * @param status_code The HTTP redirect status code.
 * @param keep_alive Whether to keep the connection alive.
 * @return SOCKET_OK if successful, SOCKET_ERROR otherwise.
 */
SocketResult
HttpUtils::sendRedirectResponse(int client_socket, const std::string &redirect_url, int status_code, bool keep_alive) {
  std::ostringstream oss;
  oss << status_code;
  std::string status_code_str = oss.str();
  std::string headers         = "HTTP/1.1 " + status_code_str + " " + getStatusMessage(status_code) + "\r\n";
  headers += "Location: " + redirect_url + "\r\n";
  headers += "Content-Length: 0\r\n";
  headers += keep_alive ? "Connection: keep-alive\r\n" : "Connection: close\r\n";
  headers += "\r\n";

  if (!sendData(client_socket, headers.c_str(), headers.length())) {
    LOG_ERROR("Error sending redirect response");
    return SOCKET_ERROR;
  }

  return SOCKET_OK;
}

/**
 * Generates HTTP response headers.
 *
 * @param content_type The MIME type of the content.
 * @param content_length The length of the content in bytes.
 * @param status_code The HTTP status code.
 * @param keep_alive Whether to keep the connection alive.
 * @return A string containing the generated headers.
 */
std::string HttpUtils::generateResponseHeaders(const std::string &content_type,
                                               size_t             content_length,
                                               int                status_code,
                                               bool               keep_alive) {
  std::ostringstream headers;
  headers << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
  headers << "Content-Type: " << content_type << "\r\n";
  headers << "Content-Length: " << content_length << "\r\n";
  headers << "Server: AJX Server/" << AJXWEBSERVER_VERSION << "\r\n";
  headers << "Date: " << getCurrentDate() << "\r\n";
  headers << "Connection: " << (keep_alive ? "keep-alive" : "close") << "\r\n";
  headers << "\r\n"; // Línea vacía para separar los headers del cuerpo
  return headers.str();
}

/**
 * Sends data over a socket.
 *
 * @param client_socket The socket to send data over.
 * @param data The data to send.
 * @param length The length of the data in bytes.
 * @return true if all data was sent successfully, false otherwise.
 */
bool HttpUtils::sendData(int client_socket, const char *data, size_t length) {
  size_t total_sent = 0;
  while (total_sent < length) {
    ssize_t sent = send(client_socket, data + total_sent, length - total_sent, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (sent < 0) {
      LOG_ERROR("Error sending data");
      return false;
    }
    total_sent += sent;
  }
  return true;
}

/**
 * Sends the content of a file over a socket.
 *
 * @param client_socket The socket to send data over.
 * @param file_state The state of the file being sent.
 * @return SOCKET_OK if successful, SOCKET_ERROR or SOCKET_CLOSED otherwise.
 */
SocketResult HttpUtils::sendFileContent(int client_socket) {
  FileState &state = getFileState(client_socket);

  char buffer[4096];
  state.file->read(buffer, sizeof(buffer));
  std::streamsize bytes_read = state.file->gcount();

  if (bytes_read > 0) {
    ssize_t sent = send(client_socket, buffer, bytes_read, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (sent > 0) {
      state.bytes_sent += sent;
      LOG_DEBUG("Sent " << sent << " bytes. Total sent: " << state.bytes_sent << " / " << state.file_size);

      if (state.bytes_sent >= state.file_size) {
        LOG_DEBUG("File sending completed for socket: " << client_socket);
        removeFileState(client_socket);
        return SOCKET_OK;
      }
      return SOCKET_WOULD_BLOCK;
    } else if (sent == 0) {
      LOG_ERROR("Connection closed while sending file content on socket: " << client_socket);
      removeFileState(client_socket);
      return SOCKET_CLOSED;
    } else {
      LOG_ERROR("Error sending file content on socket " << client_socket << ": ");
      removeFileState(client_socket);
      return SOCKET_ERROR;
    }
  }

  if (state.file->eof()) {
    if (state.bytes_sent < state.file_size) {
      LOG_ERROR("Unexpected EOF. Bytes sent: " << state.bytes_sent << ", File size: " << state.file_size);
    }
    removeFileState(client_socket);
    return SOCKET_OK;
  }

  return SOCKET_WOULD_BLOCK;
}

SocketResult
HttpUtils::sendHead(int client_socket, const std::string &filename, bool keep_alive, const LocationConfig &config) {
  LOG_DEBUG("Sending file on socket: " << client_socket << " (write), file: " << filename);

  std::ifstream file(filename.c_str(), std::ios::binary);
  if (!file.is_open()) {
    LOG_ERROR("Failed to open file: " << filename);
    return sendErrorResponse(client_socket, 500, keep_alive, config);
  }

  file.seekg(0, std::ios::end);
  size_t file_size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::string content_type = getContentType(filename);
  std::string headers      = generateResponseHeaders(content_type, file_size, 200, keep_alive);

  if (!sendData(client_socket, headers.c_str(), headers.length())) {
    LOG_ERROR("Failed to send headers for file: " << filename);
    return SOCKET_ERROR;
  }

  LOG_SUCCESS("Headers sent successfully for HEAD request on socket: " << client_socket);

  return SOCKET_OK;
}
