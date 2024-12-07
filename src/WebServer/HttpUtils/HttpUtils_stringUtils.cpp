/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpUtils_stringUtils.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 14:37:58 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/03 19:54:34 by xamayuel         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "HttpUtils.hpp"

/**
 * @file HttpUtils_stringUtils.cpp
 * @brief This file contains utility functions for handling strings
 *        related to HTTP operations, such as content type determination
 *        and status message retrieval.
 *
 * This file is part of the HttpUtils module, which provides various
 * functionalities to support HTTP server operations. The functions
 * defined here are used to convert integers to strings, determine
 * the content type based on file extensions, and retrieve status
 * messages for HTTP responses.
 *
 */

/**
 * @brief Determines the content type based on the file extension.
 *
 * @param filename The name of the file including its extension.
 * @return std::string The MIME type corresponding to the file extension.
 */
std::string HttpUtils::getContentType(const std::string &filename) {
  size_t dot_pos = filename.find_last_of('.');
  if (dot_pos != std::string::npos) {
    std::string extension = filename.substr(dot_pos);
    if (extension == ".html" || extension == ".htm")
      return "text/html";
    if (extension == ".txt")
      return "text/plain";
    if (extension == ".css")
      return "text/css";
    if (extension == ".js")
      return "application/javascript";
    if (extension == ".jpg" || extension == ".jpeg")
      return "image/jpeg";
    if (extension == ".png")
      return "image/png";
    if (extension == ".gif")
      return "image/gif";
    if (extension == ".pdf")
      return "application/pdf";
    if (extension == ".mp4")
      return "video/mp4";
  }
  return "application/octet-stream";
}

/**
 * @brief Retrieves the status message corresponding to an HTTP status code.
 *
 * @param status_code The HTTP status code.
 * @return std::string The corresponding status message.
 */
std::string HttpUtils::getStatusMessage(int status_code) {
  switch (status_code) {
    case 200:
      return "OK";
    case 204:
      return "No Content";
    case 301:
      return "Moved Permanently";
    case 302:
      return "Found";
    case 303:
      return "See Other";
    case 307:
      return "Temporary Redirect";
    case 308:
      return "Permanent Redirect";
    case 400:
      return "Bad Request";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    case 405:
      return "Method Not Allowed";
    case 408:
      return "Request Timeout";
    case 411:
      return "Length Required";
    case 413:
      return "Payload Too Large";
    case 414:
      return "URI Too Long";
    case 500:
      return "Internal Server Error";
    case 501:
      return "Not Implemented";
    case 505:
      return "HTTP Version Not Supported";
    default:
      return "Unknown Status";
  }
}

/**
 * @brief Gets the current date and time in the format required for HTTP headers.
 *
 * @return std::string The current date and time in the format "Day, DD Mon YYYY HH:MM:SS GMT".
 */
std::string HttpUtils::getCurrentDate() {
  time_t    now = time(0);
  struct tm tm  = *gmtime(&now);
  char      buf[100];
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
  return std::string(buf);
}

/**
 * @brief Constructs the file path based on the root path, location path, and request path.
 *
 * @param root_path The root path of the server.
 * @param location_path The location path of the server.
 * @param request_path The request path of the client.
 * @return std::string The constructed file path.
 */
std::string HttpUtils::constructFilePath(const std::string &root_path,
                                         const std::string &location_path,
                                         const std::string &request_path) {
  std::string file_path = root_path;
 
  if (!file_path.empty() && file_path[file_path.length() - 1] != '/') {
    file_path += '/';
  }
  std::string relative_path = request_path;
  if (request_path.compare(0, location_path.length(), location_path) == 0) {
    relative_path = request_path.substr(location_path.length());
  }

  if (!relative_path.empty() && relative_path[0] == '/') {
    relative_path = relative_path.substr(1);
  }
  if (!relative_path.empty() && relative_path[relative_path.length() - 1] == '/' && relative_path.find('.') == std::string::npos) {
    relative_path = relative_path.substr(0, relative_path.length() - 1);
  }
 
  file_path += relative_path;
  char resolved_path[PATH_MAX];
  if (realpath(file_path.c_str(), resolved_path) != NULL) {
    file_path = resolved_path;
  }

  LOG_DEBUG("Constructed file path: " << file_path);
  return file_path;
}
