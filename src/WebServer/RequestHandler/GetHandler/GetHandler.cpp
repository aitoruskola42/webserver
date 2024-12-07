/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 15:27:46 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/03 20:13:40 by xamayuel         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "GetHandler.hpp"
#include "../../HttpUtils/HttpUtils.hpp"

//-----------------------------------------------------------------------------
//                               CONSTRUCTOR / DESTRUCTOR
//-----------------------------------------------------------------------------
GetHandler::GetHandler(ConfigurationManager &config) : config(config) {
  LOG_DEBUG("GetHandler constructor called");
}

GetHandler::~GetHandler() {
  LOG_DEBUG("GetHandler destructor called");
}

//-----------------------------------------------------------------------------
//                               METHODS
//-----------------------------------------------------------------------------
/**
 * @brief Handle the GET request
 *
 * @param client_socket
 * @param request_path
 * @param config
 * @param keep_alive
 * @return SocketResult
 */
SocketResult
GetHandler::handle_get(int client_socket, const RequestParser &parser, const LocationConfig &config, bool keep_alive) {
  if (!HttpUtils::isMethodAllowed(config.allowed_methods, "GET")) {
    return HttpUtils::sendErrorResponse(client_socket, 405, keep_alive, config);
  }

  if (!config.return_code_path.empty()) {
    short int   return_code = config.return_code_path.begin()->first;
    std::string return_path = config.return_code_path.begin()->second;
    LOG_DEBUG("return_code: " << return_code << " return_path: " << return_path);
    return HttpUtils::sendRedirectResponse(client_socket, return_path, return_code, keep_alive);
  }
  std::string file_path = HttpUtils::constructFilePath(config.root_path, config.location_path, parser.getPath());
  LOG_DEBUG("Full file path: " << file_path);

  if (!HttpUtils::isValidRequest(parser.getPath())) {
    LOG_WARNING("Invalid request path: " << parser.getPath());
    return HttpUtils::sendErrorResponse(client_socket, 400, keep_alive, config);
  }

  std::string redirect_url = HttpUtils::checkRedirect(parser.getPath(), config);
  if (!redirect_url.empty()) {
    return HttpUtils::sendRedirectResponse(client_socket, redirect_url, 301, keep_alive);
  }

  if (access(file_path.c_str(), F_OK) != 0) {
    return HttpUtils::sendErrorResponse(client_socket, 404, keep_alive, config); 
  }
  if (access(file_path.c_str(), R_OK) != 0) {
    return HttpUtils::sendErrorResponse(client_socket, 403, keep_alive, config); 
  }

  if (HttpUtils::isDirectory(file_path)) {
    return handleDirectory(client_socket, file_path, parser.getPath(), keep_alive, config);
  }
  if (HttpUtils::isCgiScript(file_path, config)) {
    LOG_DEBUG("Executing CGI script");
    return HttpUtils::executeCgiScript(client_socket, file_path, parser, keep_alive, config);
  } else {
    return handleFile(client_socket, file_path, keep_alive, config);
  }
}
