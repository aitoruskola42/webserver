/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser_getters.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:57:39 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/21 14:00:40 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

/**
 * @file RequestParser_getters.cpp
 * @brief Implementation of the getter methods of the RequestParser class.
 *
 * This file contains the definitions of the methods that allow access to
 * the private attributes of the RequestParser class. These methods are used
 * to obtain information about the HTTP request being processed,
 * including the method, path, version, headers, queries, body, and error code.
 *
 * Methods:
 * - isComplete: Checks if the request is complete.
 * - getMethod: Retrieves the HTTP method of the request.
 * - getPath: Retrieves the path of the request.
 * - getVersion: Retrieves the version of the HTTP protocol.
 * - getHeader: Retrieves the value of a specific header.
 * - getQuery: Retrieves the value of a specific query.
 * - getBody: Retrieves the body of the request.
 * - getErrorCode: Retrieves the error code associated with the request.
 * - getTotalSize: Retrieves the total size of the request.
 * - getHeaders: Retrieves all headers of the request.
 * - getQueries: Retrieves all queries of the request.
 */

/**
 * @brief Checks if the request is complete.
 * @return true if the request is complete, false otherwise.
 */
bool RequestParser::isComplete() const {
  return _isComplete;
}

/**
 * @brief Gets the HTTP method of the request.
 * @return A string representing the HTTP method.
 */
std::string RequestParser::getMethod() const {
  return _method;
}

/**
 * @brief Gets the path of the request.
 * @return A string representing the request path.
 */
std::string RequestParser::getPath() const {
  return _path;
}

/**
 * @brief Gets the version of the HTTP protocol.
 * @return A string representing the HTTP protocol version.
 */
std::string RequestParser::getVersion() const {
  return _version;
}

/**
 * @brief Gets the value of a specific header.
 * @param name The name of the header.
 * @return The value of the header if it exists, or an empty string if not found.
 */
std::string RequestParser::getHeader(const std::string &name) const {
  std::map<std::string, std::string>::const_iterator it = _headers.find(name);
  if (it != _headers.end())
    return it->second;
  return "";
}

/**
 * @brief Gets the value of a specific query.
 * @param name The name of the query.
 * @return The value of the query if it exists, or an empty string if not found.
 */
std::string RequestParser::getQuery(const std::string &name) const {
  std::map<std::string, std::string>::const_iterator it = _queries.find(name);
  if (it != _queries.end())
    return it->second;
  return "";
}

/**
 * @brief Gets the body of the request.
 * @return A string representing the request body.
 */
std::string RequestParser::getBody() const {
  return _body;
}

/**
 * @brief Gets the error code associated with the request.
 * @return The error code as an unsigned 16-bit integer.
 */
unsigned short RequestParser::getErrorCode() const {
  return _ecode;
}

/**
 * @brief Gets the total size of the request.
 * @return The total size of the request as an integer.
 */
int RequestParser::getTotalSize() const {
  return _totalsize;
}

/**
 * @brief Gets all headers of the request.
 * @return A constant reference to the map of headers.
 */
const std::map<std::string, std::string> &RequestParser::getHeaders() const {
  return _headers;
}

/**
 * @brief Gets all queries of the request.
 * @return A constant reference to the map of queries.
 */
const std::map<std::string, std::string> &RequestParser::getQueries() const {
  return _queries;
}

/**
 * @brief Gets all supported HTTP methods.
 * @return A constant reference to the map of HTTP methods.
 */
const std::map<std::string, std::string> &RequestParser::getHttpMethods() const {
  return _httpMethods;
}