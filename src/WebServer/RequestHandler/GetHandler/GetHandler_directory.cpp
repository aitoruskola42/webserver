/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler_directory.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/22 13:57:38 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/02 22:06:38 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetHandler.hpp"
#include "HttpGenerator/HttpGenerator.hpp"

/**
 * @brief Handles the request for a directory.
 *
 * This function checks if an index file exists in the specified directory.
 * If an index file is found, it sends the file to the client. If not,
 * it checks if autoindexing is enabled in the configuration. If autoindexing
 * is enabled, it sends a directory listing. If neither condition is met,
 * it sends a 403 Forbidden error response.
 *
 * @param client_socket The socket connected to the client.
 * @param file_path The path of the directory to be handled.
 * @param request_path The original request path from the client.
 * @param keep_alive Indicates whether to keep the connection alive after the response.
 * @param config The location configuration containing settings for the directory handling.
 * @return SocketResult indicating the result of the operation.
 */

SocketResult GetHandler::handleDirectory(int                   client_socket,
                                         const std::string    &file_path,
                                         const std::string    &request_path,
                                         bool                  keep_alive,
                                         const LocationConfig &config) {
  std::string index_file = HttpUtils::findIndexFile(file_path, config.index_files);
  if (!index_file.empty()) {
    return HttpUtils::sendFile(client_socket, index_file, keep_alive, config, 200);
  } else if (config.autoindex) {
    return sendDirectoryListing(client_socket, file_path, request_path, keep_alive, config);
  } else {
    return HttpUtils::sendErrorResponse(client_socket, 403, keep_alive, config);
  }
}

/**
 * @brief Handles the request for a directory.
 *
 * This function checks if an index file exists in the specified directory.
 * If an index file is found, it sends the file to the client. If not,
 * it checks if autoindexing is enabled in the configuration. If autoindexing
 * is enabled, it sends a directory listing. If neither condition is met,
 * it sends a 403 Forbidden error response.
 *
 * @param client_socket The socket connected to the client.
 * @param file_path The path of the directory to be handled.
 * @param request_path The original request path from the client.
 * @param keep_alive Indicates whether to keep the connection alive after the response.
 * @param config The location configuration containing settings for the directory handling.
 * @return SocketResult indicating the result of the operation.
 */

SocketResult GetHandler::sendDirectoryListing(int                   client_socket,
                                              const std::string    &dir_path,
                                              const std::string    &request_path,
                                              bool                  keep_alive,
                                              const LocationConfig &config) {
  bool        delete_allowed = HttpUtils::isMethodAllowed(config.allowed_methods, "DELETE");
  std::string response       = HttpGenerator::generate_HTMLHeader2(request_path, delete_allowed);

  DIR *dir = opendir(dir_path.c_str());
  if (dir == NULL) {
    LOG_ERROR("Could not open directory: " << dir_path);
    return HttpUtils::sendErrorResponse(client_socket, 500, keep_alive, LocationConfig());
  }

  try {
    response += generateDirectoryContent(dir, dir_path, request_path, config, delete_allowed);

  } catch (const std::exception &e) {
    closedir(dir);
    LOG_ERROR("Exception while generating directory listing: " << e.what());
    return HttpUtils::sendErrorResponse(client_socket, 500, keep_alive, LocationConfig());
  }
  closedir(dir);

  response += HttpGenerator::generate_HTMLFooter();

  return HttpUtils::sendResponse(client_socket, "text/html", response, 200, keep_alive);
}

std::string GetHandler::generateDirectoryContent(DIR                  *dir,
                                                 const std::string    &dir_path,
                                                 const std::string    &request_path,
                                                 const LocationConfig &config,
                                                 bool                  delete_allowed) {
  std::string              content;
  std::vector<std::string> directories;
  std::vector<std::string> files;
   std::string new_request_path;
  const size_t             MAX_CONTENT_LENGTH = 1000000; // ESTO???
  // mirar si quitar
  if (request_path.length() > 1 && request_path[request_path.length() - 1] == '/') {
    new_request_path = request_path.substr(0, request_path.length() - 1);
  }
  else {
    new_request_path = request_path;
  }
  content += HttpGenerator::generateEntryHtml(dir_path, new_request_path, "..", true, config.location_path, delete_allowed);

  collectDirectoryEntries(dir, dir_path, directories, files);
  content +=
      generateSortedEntries(directories, dir_path, request_path, config, true, MAX_CONTENT_LENGTH, delete_allowed);
  content += generateSortedEntries(files, dir_path, request_path, config, false, MAX_CONTENT_LENGTH, delete_allowed);

  content += "<script>\n";
  content += "function deleteFile(path) {\n";
  content += "  console.log('Intentando eliminar el archivo:', path); // Depuración\n";
  content +=
      "  if (confirm('\\u00BFEst\\u00E1s seguro de que quieres eliminar este "
      "archivo?')) {\n";
  content += "    var xhr = new XMLHttpRequest();\n";
  content += "    xhr.open('DELETE', path, true);\n";
  content += "    xhr.onreadystatechange = function() {\n";
  content += "      if (xhr.readyState === 4) {\n";
  content += "        if (xhr.status === 204) {\n";
  content += "          location.reload();\n";
  content += "        } else {\n";
  content += "          alert('Error al eliminar el archivo');\n";
  content += "        }\n";
  content += "      }\n";
  content += "    };\n";
  content += "    xhr.send();\n";
  content += "  }\n";
  content += "}\n";
  content += "</script>\n";

  return content;
}

/**
 * @brief Collects directory entries and separates them into directories and files.
 *
 * This function reads the entries of a directory using the provided DIR pointer.
 * It iterates over each entry and classifies them as either a directory or a file.
 * The classified entries are stored in the respective 'directories' and 'files' vectors.
 *
 * @param dir The DIR pointer representing the directory to collect entries from.
 * @param dir_path The path of the directory being processed.
 * @param directories A reference to the vector where directory entries will be stored.
 * @param files A reference to the vector where file entries will be stored.
 */

void GetHandler::collectDirectoryEntries(DIR                      *dir,
                                         const std::string        &dir_path,
                                         std::vector<std::string> &directories,
                                         std::vector<std::string> &files) {
  struct dirent *ent;
  while ((ent = readdir(dir)) != NULL) {
    std::string name = ent->d_name;
    if (name != "." && name != "..") {
      std::string full_path = dir_path + "/" + name;
      if (HttpUtils::isDirectory(full_path)) {
        directories.push_back(name);
      } else {
        files.push_back(name);
      }
    }
  }
}

/**
 * @brief Collects directory entries and separates them into directories and files.
 *
 * This function reads the entries of a directory using the provided DIR pointer.
 * It iterates over each entry and classifies them as either a directory or a file.
 * The classified entries are stored in the respective 'directories' and 'files' vectors.
 *
 * @param dir The DIR pointer representing the directory to collect entries from.
 * @param dir_path The path of the directory being processed.
 * @param directories A reference to the vector where directory entries will be stored.
 * @param files A reference to the vector where file entries will be stored.
 */

std::string GetHandler::generateSortedEntries(const std::vector<std::string> &entries,
                                              const std::string              &dir_path,
                                              const std::string              &request_path,
                                              const LocationConfig           &config,
                                              bool                            is_directory,
                                              size_t                          max_length,
                                              bool                            delete_allowed) {
  std::string              content;
  std::vector<std::string> sorted_entries = entries;
  std::sort(sorted_entries.begin(), sorted_entries.end());

  for (std::vector<std::string>::const_iterator it = sorted_entries.begin(); it != sorted_entries.end(); ++it) {
    if (content.length() + it->length() > max_length) {
      content +=
          "<tr><td colspan=\"4\">... (Listing truncated due to "
          "size)</td></tr>\n";
      break;
    }

    content += HttpGenerator::generateEntryHtml(
        dir_path, request_path, *it, is_directory, config.location_path, delete_allowed);
  }

  return content;
}
