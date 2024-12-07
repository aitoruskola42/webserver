/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpUtils.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 14:13:45 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/02 22:06:12 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_UTILS_HPP
#define HTTP_UTILS_HPP

//------------------------------------------------------------------------------
#include "../src/Logger/includes/Logger.hpp"
#include "CommonDefinitions.hpp"
#include "RequestParser/RequestParser.hpp"
//------------------------------------------------------------------------------
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

class HttpUtils {
  //------------------------PUBLIC METHODS------------------------------------
 public:
  // VOID METHODS
  static void removeFileState(int client_socket);
  // -----------------------BOOLEAN METHODS-----------------------------------
  static bool isMethodAllowed(const std::vector<std::string> &allowed_methods,
                              const std::string              &method);
  static bool fileExists(const std::string &filename);
  static bool isDirectory(const std::string &path);
  static bool isValidRequest(const std::string &request_path);
  static bool hasFileState(int client_socket);
  static bool isCgiScript(const std::string    &filepath,
                          const LocationConfig &config);
  static bool findCgiExecutable(const std::string    &filepath,
                                const LocationConfig &config,
                                std::string          &cgi_path,
                                std::string          &program_name);
  static SocketResult executeCgiScript(int                   client_socket,
                                       const std::string    &filepath,
                                       const RequestParser  &parser,
                                       bool                  keep_alive,
                                       const LocationConfig &config);

 private:
  static SocketResult executeCgiParent(int                   client_socket,
                                       int                   pipefd[2],
                                       pid_t                 pid,
                                       bool                  keep_alive,
                                       const LocationConfig &config);
  static SocketResult executeCgiChild(int                  pipefd[2],
                                      const std::string   &filepath,
                                      const RequestParser &parser,
                                      const std::string   &cgi_path,
                                      const std::string   &program_name);
  // -----------------------STRING METHODS-------------------------------------
 public:
  static std::string constructFilePath(const std::string &root_path,
                                       const std::string &location_path,
                                       const std::string &request_path);
  static std::string findIndexFile(const std::string              &dir_path,
                                   const std::vector<std::string> &index_files);
  static std::string getContentType(const std::string &filename);
  static std::string getStatusMessage(int status_code);
  static std::string getCurrentDate();
  static std::string intToString(int number);
  static std::string checkRedirect(const std::string    &request_path,
                                   const LocationConfig &config);
  // -----------------------SOCKET METHODS-------------------------------------
  static SocketResult sendResponse(int                client_socket,
                                   const std::string &content_type,
                                   const std::string &content,
                                   int                status_code,
                                   bool               keep_alive);

  static void setFileState(int client_socket, FileState *state);

  static SocketResult sendFile(int                   client_socket,
                               const std::string    &filename,
                               bool                  keep_alive,
                               const LocationConfig &config, int status_code);
  static SocketResult sendHead(int                   client_socket,
                               const std::string    &filename,
                               bool                  keep_alive,
                               const LocationConfig &config);
  static SocketResult sendErrorResponse(int                   client_socket,
                                        int                   status_code,
                                        bool                  keep_alive,
                                        const LocationConfig &config);
  static SocketResult sendRedirectResponse(int                client_socket,
                                           const std::string &redirect_url,
                                           int                status_code,
                                           bool               keep_alive);

  static SocketResult sendFileContent(int client_socket);
  static SocketResult sendChunkedFileNonBlocking(int  client_socket,
                                                 bool keep_alive,
                                                 const LocationConfig &config);

  static FileState &getFileState(int client_socket);

 private:
  static bool sendData(int client_socket, const char *data, size_t length);
  static std::string generateResponseHeaders(const std::string &content_type,
                                             size_t             content_length,
                                             int                status_code,
                                             bool               keep_alive);

  static SocketResult sendChunk(int         client_socket,
                                const char *data,
                                size_t      length);
  static SocketResult initializeFileState(int                   client_socket,
                                          const std::string    &filename,
                                          bool                  keep_alive,
                                          const LocationConfig &config);

  //------------------------PRIVATE ATTRIBUTES--------------------------------
 private:
  static std::map<int, FileState *> file_states;
};

#endif // HTTP_UTILS_HPP