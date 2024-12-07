/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/01 17:38:12 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 18:00:20 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DeleteHandler.hpp"

//------------------------------------------------------------------------------
//                               CONSTRUCTOR
//------------------------------------------------------------------------------

DeleteHandler::DeleteHandler(int client_socket, const LocationConfig &loc_config, RequestParser &request, bool keep_alive)
    : _del_client_socket(client_socket)
    , _del_location_config(loc_config)
    , _del_request(request)
    , _del_keep_alive(keep_alive) {}

//------------------------------------------------------------------------------
//   METHODS
//------------------------------------------------------------------------------

/**
 * @brief Handles the DELETE HTTP request.
 *
 * This method processes a DELETE request by verifying if the DELETE method is allowed,
 * checking if the requested file or directory exists, and then attempting to remove it.
 * If the file or directory is successfully removed, a 204 No Content response is sent.
 * Otherwise, appropriate error responses are sent based on the encountered issue.
 *
 * @return SocketResult indicating the result of the operation.
 */
SocketResult DeleteHandler::handle_delete_request() {
  if (!HttpUtils::isMethodAllowed(_del_location_config.allowed_methods, "DELETE")) {
    return HttpUtils::sendErrorResponse(_del_client_socket, 405, _del_keep_alive, _del_location_config);
  }

  std::string path          = _del_request.getPath();
  std::string relative_path = path.substr(_del_location_config.location_path.length());
  std::string full_path     = _del_location_config.root_path + relative_path;

  if (!HttpUtils::fileExists(full_path)) {
    return HttpUtils::sendErrorResponse(_del_client_socket, 400, _del_keep_alive, _del_location_config);
  }
  if (remove_directory(full_path.c_str()) == 0) {
    return HttpUtils::sendResponse(_del_client_socket, "", "", 204, _del_keep_alive);
  } else {
    return HttpUtils::sendErrorResponse(_del_client_socket, 500, _del_keep_alive, _del_location_config);
  }
}

/**
 * @brief Removes a directory and its contents recursively.
 *
 * This method opens a directory, iterates through its contents, and removes each
 * file or subdirectory recursively. Finally, it removes the directory itself.
 *
 * @param path The path to the directory to be removed.
 * @return 0 on success, -1 on failure.
 */
int DeleteHandler::remove_directory(const char *path) const {
  DIR *dir = opendir(path);
  if (!dir) {
    return std::remove(path);
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    std::string full_path = std::string(path) + "/" + entry->d_name;

    struct stat statbuf;
    if (stat(full_path.c_str(), &statbuf) == 0) {
      if (S_ISDIR(statbuf.st_mode)) {
        if (remove_directory(full_path.c_str()) != 0) {
          closedir(dir);
          return -1;
        }
      } else {
        if (std::remove(full_path.c_str()) != 0) {
          closedir(dir);
          return -1;
        }
      }
    }
  }
  closedir(dir);
  return rmdir(path);
}
