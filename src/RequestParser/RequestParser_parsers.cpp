/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser_parsers.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:57:47 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/02 12:03:13 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

/**
 * @brief Clears all data in the RequestParser object.
 *
 * This function resets all member variables to their default states,
 * preparing the object for parsing a new request.
 */
void RequestParser::clear() {
  _method.clear();
  _path.clear();
  _version.clear();
  _headers.clear();
  _queries.clear();
  _body.clear();
  _isComplete = false;
  _ecode      = 0;
  _totalsize  = 0;
}

/**
 * @brief Parses an entire HTTP request.
 *
 * @param request The complete HTTP request as a string.
 *
 * This function processes the request line, headers, and body of an HTTP request.
 * It sets the appropriate member variables and performs validation checks.
 */
void RequestParser::parseRequest(const std::string &request) {
  clear(); // Clear any previous data

  size_t pos    = 0;
  size_t endPos = request.find("\r\n");

  // Parse the request line
  if (endPos != std::string::npos) {
    parseRequestLine(request.substr(pos, endPos - pos));
    if (_ecode)
      return;
    pos = endPos + 2;
  } else {
    LOG_ERROR("MALFORMED REQUEST LINE");
    _ecode = e_http_errorcodes(BAD_REQUEST);
    return; // Error in the request (no request line)
  }
  if (!check_line_method() || !check_line_URI() || !check_line_protocol())
    return;
  // Parse the headers
  while ((endPos = request.find("\r\n", pos)) != std::string::npos && endPos != pos) {
    parseHeaderLine(request.substr(pos, endPos - pos));
    if (_ecode)
      return;
    pos = endPos + 2;
  }
  if (!check_headers())
    return;
  // Skip the blank line between headers and body
  if (request.substr(pos, 2) == "\r\n") {
    pos += 2;
  } else {
    LOG_ERROR("NO BLANK LINE AFTER HEADERS")
    _ecode = e_http_errorcodes(BAD_REQUEST); // Error si no hay línea en blanco
    return;
  }

  // Parsear el cuerpo (Body) si existe
  if (pos < request.size()) {
    parseBody(request.substr(pos));
    if (!check_body())
      return;
  }
  _isComplete = true;
}

/**
 * @brief Parses the request line of an HTTP request.
 *
 * @param line The request line to parse.
 *
 * This function extracts the method, path, and HTTP version from the request line.
 * It sets the corresponding member variables or sets an error code if malformed.
 */
void RequestParser::parseRequestLine(const std::string &line) {
  size_t methodEnd = line.find(' ');
  size_t pathEnd   = line.find(' ', methodEnd + 1);

  if (methodEnd != std::string::npos && pathEnd != std::string::npos) {
    _method  = line.substr(0, methodEnd);
    _path    = line.substr(methodEnd + 1, pathEnd - methodEnd - 1);
    _version = line.substr(pathEnd + 1);
  } else {
    LOG_ERROR("MALFORMED REQUEST LINE");
    _ecode = e_http_errorcodes(BAD_REQUEST); // Malformed request line
  }
}

/**
 * @brief Parses a single header line from an HTTP request.
 *
 * @param line The header line to parse.
 *
 * This function extracts the header name and value, adding them to the _headers map.
 * It sets an error code if the header line is malformed.
 */
void RequestParser::parseHeaderLine(const std::string &line) {
  size_t separatorPos = line.find(": ");
  if (separatorPos != std::string::npos) {
    std::string headerName  = line.substr(0, separatorPos);
    std::string headerValue = line.substr(separatorPos + 2);
    _headers[headerName]    = headerValue;
  } else {
    LOG_ERROR("BAD FORMATTED HEADERS");
    _ecode = e_http_errorcodes(BAD_REQUEST);
  }
}

/**
 * @brief Stores the body of an HTTP request.
 *
 * @param body The body content of the HTTP request.
 *
 * This function simply stores the provided body content in the _body member variable.
 */
void RequestParser::parseBody(const std::string &body) {
  _body = body;
}