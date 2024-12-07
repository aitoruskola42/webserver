/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpUtils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 14:15:05 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/05 12:07:33 by xamayuel         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "HttpUtils.hpp"

/**
 * @brief Finds the index file in the given directory.
 *
 * @param dir_path The directory path to search in.
 * @param index_files A vector of index files to search for.
 * @return The path to the index file if found, an empty string otherwise.
 */
std::string HttpUtils::findIndexFile(const std::string &dir_path, const std::vector<std::string> &index_files) {
  for (std::vector<std::string>::const_iterator it = index_files.begin(); it != index_files.end(); ++it) {
    std::string full_path = dir_path + "/" + *it;
    if (fileExists(full_path)) {
      return full_path;
    }
  }
  return "";
}

/**
 * @brief Checks if the given request path is valid.
 *
 * @param request_path The request path to check.
 * @return true if the request path is valid, false otherwise.
 */
bool HttpUtils::isValidRequest(const std::string &request_path) {
  if (request_path.empty() || request_path[0] != '/') {
    return false;
  }
  for (std::string::const_iterator it = request_path.begin(); it != request_path.end(); ++it) {
    if (!isalnum(*it) && *it != '/' && *it != '-' && *it != '_' && *it != '.' && *it != '~' && *it != '?' &&
        *it != '#' && *it != ' ') {
      LOG_WARNING("Invalid character in request path: " << *it);
      return false;
    }
  }
  size_t pos = 0;
  while ((pos = request_path.find("..", pos)) != std::string::npos) {
    if (pos == 0 || request_path[pos - 1] == '/') {
      size_t next_pos = pos + 2;
      if (next_pos == request_path.length() || request_path[next_pos] == '/') {
        return false;
      }
    }
    pos += 2;
  }
  return true;
}

std::string HttpUtils::checkRedirect(const std::string &request_path, const LocationConfig &config) {
  std::map<std::string, std::string>::const_iterator it;
  for (it = config.redirects.begin(); it != config.redirects.end(); ++it) {
    const std::string &from = it->first;
    const std::string &to   = it->second;
    if (request_path == from ||
        (from.length() > 0 && from[from.length() - 1] == '/' && request_path.compare(0, from.length(), from) == 0)) {
      if (from == request_path) {
        return to;
      } else {
        return to + request_path.substr(from.length());
      }
    }
  }
  return "";
}

// Cambiar la definición del miembro estático
std::map<int, FileState *> HttpUtils::file_states;

SocketResult HttpUtils::sendChunkedFileNonBlocking(int client_socket, bool keep_alive, const LocationConfig &config) {
  

  if (!hasFileState(client_socket)) {
    return initializeFileState(client_socket, config.root_path + "/file.txt", keep_alive, config);
  }

  FileState &state = getFileState(client_socket);

  if (!state.headers_sent) {
    LOG_DEBUG("Sending headers for socket: " << client_socket);
    std::string headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Type: " + getContentType(state.filename) + "\r\n";
    headers += "Transfer-Encoding: chunked\r\n";
    headers += keep_alive ? "Connection: keep-alive\r\n" : "Connection: close\r\n";
    headers += "\r\n";

    ssize_t sent = send(client_socket, headers.c_str(), headers.length(), MSG_DONTWAIT | MSG_NOSIGNAL);
    if (sent == -1) {
      LOG_ERROR("Failed to send headers for socket: " << client_socket);
      removeFileState(client_socket);
      return SOCKET_ERROR;
    }
    state.headers_sent = true;
    LOG_DEBUG("Headers sent successfully for socket: " << client_socket);
    return SOCKET_WOULD_BLOCK;
  }

  if (state.bytes_sent < state.file_size) {
    if (state.buffer_pos >= state.buffer_len) {
      LOG_DEBUG("Reading new block of file for socket: " << client_socket);
      state.file->read(state.buffer, sizeof(state.buffer));
      state.buffer_len = state.file->gcount();
      state.buffer_pos = 0;
      LOG_DEBUG("Reading " << state.buffer_len << " bytes from file");
    }

    size_t remaining = state.buffer_len - state.buffer_pos;
    LOG_DEBUG("Sending chunk of " << remaining << " bytes for socket: " << client_socket);
    SocketResult result = sendChunk(client_socket, state.buffer + state.buffer_pos, remaining);

    if (result == SOCKET_WOULD_BLOCK) {
      LOG_WARNING("Chunk not sent completely, WOULD_BLOCK");
      return SOCKET_WOULD_BLOCK;
    } else if (result != SOCKET_OK) {
      LOG_ERROR("Error sending chunk for socket: " << client_socket);
      removeFileState(client_socket);
      return result;
    }

    state.buffer_pos += remaining;
    state.bytes_sent += remaining;
    LOG_DEBUG("Chunk sent successfully. Total sent: " << state.bytes_sent << " of " << state.file_size);
    return SOCKET_WOULD_BLOCK;
  }

  LOG_DEBUG("Sending final chunk for socket: " << client_socket);
  SocketResult result = sendChunk(client_socket, NULL, 0);
  if (result == SOCKET_WOULD_BLOCK) {
    LOG_DEBUG("Final chunk not sent completely, WOULD_BLOCK");
    return SOCKET_WOULD_BLOCK;
  }
  LOG_DEBUG("Transfer complete for socket: " << client_socket);
  removeFileState(client_socket);
  return result;
}

/**
 * @brief Sends a chunk to the client.
 *
 * @param client_socket The socket to send the chunk to.
 * @param data The data to send.
 * @param length The length of the data to send.
 * @return The result of the send operation.
 */
SocketResult HttpUtils::sendChunk(int client_socket, const char *data, size_t length) {
 

  std::ostringstream chunk_header;
  chunk_header << std::hex << length << "\r\n";
  std::string header = chunk_header.str();


  if (length > 0) {
    LOG_DEBUG("Sending chunk data of length: " << length);
    ssize_t sent = send(client_socket, data, length, MSG_DONTWAIT | MSG_NOSIGNAL);


    if (sent == -1) {
      LOG_ERROR("Failed to send chunk data for socket: " << client_socket);
      return SOCKET_ERROR;
    }
    LOG_DEBUG("Sent " << sent << " bytes of chunk data");
  }

  LOG_DEBUG("Sending chunk trailer");
  if (send(client_socket, "", 0, MSG_DONTWAIT | MSG_NOSIGNAL) == -1) {
    LOG_ERROR("Failed to send chunk trailer for socket: " << client_socket);
    return SOCKET_ERROR;
  }

  LOG_DEBUG("Chunk sent successfully for socket: " << client_socket);
  return SOCKET_OK;
}

SocketResult HttpUtils::initializeFileState(int                   client_socket,
                                            const std::string    &filename,
                                            bool                  keep_alive,
                                            const LocationConfig &config) {
  LOG_DEBUG("Initializing FileState for socket: " << client_socket << " with file: " << filename);

  FileState *state = new FileState();
  state->file      = new std::ifstream(filename.c_str(), std::ios::binary);
  if (!state->file->is_open()) {
    LOG_ERROR("Failed to open file: " << filename << " for socket: " << client_socket);
    delete state;
    return sendErrorResponse(client_socket, 500, keep_alive, config);
  }

  state->file->seekg(0, std::ios::end);
  state->file_size = state->file->tellg();
  state->file->seekg(0, std::ios::beg);

  state->bytes_sent   = 0;
  state->headers_sent = false;
  state->buffer_pos   = 0;
  state->buffer_len   = 0;
  state->filename     = filename;

  file_states[client_socket] = state;

  LOG_DEBUG("FileState initialized for socket: " << client_socket << ". File size: " << state->file_size);
  return SOCKET_OK;
}

bool HttpUtils::hasFileState(int client_socket) {
  return file_states.find(client_socket) != file_states.end();
}

FileState &HttpUtils::getFileState(int client_socket) {
  return *file_states[client_socket];
}

void HttpUtils::removeFileState(int client_socket) {
  LOG_DEBUG("Removing FileState for socket: " << client_socket);
  std::map<int, FileState *>::iterator it = file_states.find(client_socket);
  if (it != file_states.end()) {
    delete it->second;
    file_states.erase(it);
    LOG_DEBUG("FileState removed successfully for socket: " << client_socket);
  } else {
    LOG_DEBUG("FILE STATE NOT FOUND: " << client_socket);
  }
}

bool HttpUtils::isMethodAllowed(const std::vector<std::string> &allowed_methods, const std::string &method) {
  for (std::vector<std::string>::const_iterator it = allowed_methods.begin(); it != allowed_methods.end(); ++it) {
    if (*it == method)
      return true;
  }
  return false;
}