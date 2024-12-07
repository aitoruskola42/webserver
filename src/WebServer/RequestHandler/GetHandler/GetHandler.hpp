/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/01 17:37:17 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 21:03:21 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_HANDLER_HPP
#define GET_HANDLER_HPP

//------------------------------------------------------------------------------
#include "CommonDefinitions.hpp"
#include "ConfigFileParse/ConfigurationManager.hpp"
#include "RequestParser/RequestParser.hpp"
#include "WebServer/HttpUtils/HttpUtils.hpp"
//------------------------------------------------------------------------------
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>

class GetHandler {
 private:
  ConfigurationManager &config;

 public:
  GetHandler(ConfigurationManager &config);
  ~GetHandler();

  SocketResult handle_get(int                   client_socket,
                          const RequestParser  &parser,
                          const LocationConfig &config,
                          bool                  keep_alive);

 private:
  SocketResult handleDirectory(int                   client_socket,
                               const std::string    &file_path,
                               const std::string    &request_path,
                               bool                  keep_alive,
                               const LocationConfig &config);
  SocketResult handleFile(int                   client_socket,
                          const std::string    &file_path,
                          bool                  keep_alive,
                          const LocationConfig &config);
  SocketResult sendDirectoryListing(int                   client_socket,
                                    const std::string    &dir_path,
                                    const std::string    &request_path,
                                    bool                  keep_alive,
                                    const LocationConfig &config);

 public:
 private:
  std::string generateDirectoryContent(DIR                  *dir,
                                       const std::string    &dir_path,
                                       const std::string    &request_path,
                                       const LocationConfig &config,
                                       bool                  delete_allowed);

 private:
  bool isCgiScript(const std::string &filepath, const LocationConfig &config);

  void        collectDirectoryEntries(DIR                      *dir,
                                      const std::string        &dir_path,
                                      std::vector<std::string> &directories,
                                      std::vector<std::string> &files);
  std::string generateSortedEntries(const std::vector<std::string> &entries,
                                    const std::string              &dir_path,
                                    const std::string    &request_path,
                                    const LocationConfig &config,
                                    bool                  is_directory,
                                    size_t                max_length,
                                    bool                  delete_allowed);

  SocketResult executeCgiChild(int                  pipefd[2],
                               const std::string   &filepath,
                               const RequestParser &parser,
                               const std::string   &cgi_path,
                               const std::string   &program_name);
  bool         findCgiExecutable(const std::string    &filepath,
                                 const LocationConfig &config,
                                 std::string          &cgi_path,
                                 std::string          &program_name);

  SocketResult executeCgiParent(int                   client_socket,
                                int                   pipefd[2],
                                pid_t                 pid,
                                bool                  keep_alive,
                                const LocationConfig &config);
};

#endif // GET_HANDLER_HPP