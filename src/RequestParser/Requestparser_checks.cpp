/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Requestparser_checks.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:40:24 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 18:30:40 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

/**
 * @brief Checks the validity of a domain name in HTTP requests.
 *
 * This function verifies if a domain name complies with established standards,
 * ensuring it only contains allowed characters and that the labels are properly
 * formed.
 *
 * @param host The domain name to check.
 * @return true If the domain name is valid.
 * @return false If the domain name is not valid.
 */
bool check_host_valid_domain_name(const std::string &host) {
  if (host.empty())
    return false;

  std::string::const_iterator it          = host.begin();
  std::string::const_iterator label_start = it;

  while (it != host.end()) {
    if (!isalnum(*it) && *it != '-' && *it != '.') {
      return false; // Solo letras, dígitos, guiones y puntos son permitidos
    }

    // El guion no puede estar al principio o final de una etiqueta o antes de punto!
    if (*it == '-' && (it == label_start || (it + 1 == host.end() || *(it + 1) == '.'))) {
      return false;
    }

    // Cada parte debe terminar en un carácter alfanumérico
    if (*it == '.') {
      if (it == label_start || it - 1 == label_start || !isalnum(*(it - 1))) {
        return false;
      }
      label_start = it + 1; // Reinicia el inicio de la siguiente etiqueta
    }
    ++it;
  }

  // La última etiqueta debe terminar en alfanumérico
  if (!isalnum(*(it - 1)))
    return false;

  return true;
}

/**
 * @brief Checks the validity of an IP address (simplified IPv4) in HTTP requests.
 *
 * This function verifies if an IP address complies with established standards,
 * ensuring that each segment of the address is within the allowed range (0-255)
 * and contains only digits.
 *
 * @param host The IP address to check.
 * @return true If the IP address is valid.
 * @return false If the IP address is not valid.
 */
bool check_host_valid_ipv4(const std::string &host) {
  int         segments = 0;
  std::string segment;
  for (std::string::const_iterator it = host.begin(); it != host.end(); ++it) {
    if (*it == '.') {
      if (segment.empty() || !is_numeric(segment) || std::atoi(segment.c_str()) > 255) {
        return false;
      }
      segments++;
      segment.clear();
    } else {
      if (!isdigit(*it))
        return false;
      segment += *it;
    }
  }

  // Último segmento
  if (segment.empty() || !is_numeric(segment) || std::atoi(segment.c_str()) > 255 || segments != 3) {
    return false;
  }

  return true;
}

/**
 * @brief Checks the validity of a host header in HTTP requests.
 *
 * This function verifies if a host header complies with established standards,
 * ensuring it only contains allowed characters and that the labels are properly
 * formed.
 *
 * @param host_header The host header to check.
 * @return true If the host header is valid.
 * @return false If the host header is not valid.
 */
bool check_valid_host(const std::string &host_header) {
  if (host_header.empty())
    return false;

  std::string host, port;
  std::size_t colon_pos = host_header.find(':');
  if (colon_pos != std::string::npos) {
    host = host_header.substr(0, colon_pos);
    port = host_header.substr(colon_pos + 1);

    if (!port.empty() && !is_numeric(port)) {
      return false;
    }
  } else {
    host = host_header;
  }

  if (host == "localhost" || check_host_valid_ipv4(host) || check_host_valid_domain_name(host)) {
    return true;
  }

  return false;
}

/**
 * @brief Checks the validity of headers in HTTP requests.
 *
 * This function verifies if the headers in an HTTP request comply with established
 * standards, ensuring they are properly formed and contain the necessary information.
 *
 * @param headers The headers to check.
 * @return true If the headers are valid.
 * @return false If the headers are not valid.
 */
bool RequestParser::check_headers() {
  if (!check_valid_host(getHeader("Host"))) {
    LOG_ERROR("HOST HEADER NOT PRESENT OR INVALID");
    _ecode = e_http_errorcodes(BAD_REQUEST);
    return false;
  }
  return true;
}

/**
 * @brief Checks the validity of the body in HTTP requests.
 *
 * This function verifies if the body in an HTTP request complies with established
 * standards, ensuring it is properly formed and contains the necessary information.
 *
 * @param body The body to check.
 * @return true If the body is valid.
 * @return false If the body is not valid.
 */
bool RequestParser::check_body() {
  // validacion junto al header content-length
  std::string content_length = getHeader("Content-Length");
  std::string ismultipart    = getHeader("Content-Type");

  if (!getHeader("Transfer-encoding").empty())
    return true;
  calculateTotalSize();
  if (!is_numeric(content_length)) {
    LOG_ERROR("CONTENT-LENGTH HEADER INVALID");
    _ecode = e_http_errorcodes(BAD_REQUEST);
    return false;
  }
  return true;
}

/**
 * @brief Checks the validity of the query string in HTTP requests.
 *
 * This function verifies if the query string in an HTTP request complies with established
 * standards, ensuring it is properly formed and contains the necessary information.
 *
 * @param query The query string to check.
 * @return true If the query string is valid.
 * @return false If the query string is not valid.
 */
bool RequestParser::check_query_string(const std::string &query) {
  size_t start   = 0;
  size_t amp_pos = 0;
  while (start < query.length()) {
    amp_pos       = query.find('&', start);
    size_t eq_pos = query.find('=', start);

    if (eq_pos == std::string::npos || (amp_pos != std::string::npos && eq_pos > amp_pos)) {
      LOG_ERROR("WRONG FORMAT QUERIES");
      _ecode = e_http_errorcodes(BAD_REQUEST);
      return false;
    }
    std::string key = query.substr(start, eq_pos - start);

    size_t value_length;
    if (amp_pos == std::string::npos)
      value_length = query.length();
    else
      value_length = amp_pos;

    std::string value = query.substr(eq_pos + 1, value_length - eq_pos - 1);
    _queries[key]     = value;
    if (amp_pos == std::string::npos)
      start = query.length();
    else
      start = amp_pos + 1;
  }

  return true;
}

/**
 * @brief Checks the validity of the URI in HTTP requests.
 *
 * This function verifies if the URI in an HTTP request complies with established
 * standards, ensuring it is properly formed and contains the necessary information.
 *
 * @param uri The URI to check.
 * @return true If the URI is valid.
 * @return false If the URI is not valid.
 */
bool RequestParser::check_line_URI() {
  for (size_t i = 0; i < _path.size(); ++i) {
    if (!isValidUriChar(_path[i])) {
      LOG_ERROR("INVALID CHARACTER IN URI");
      _ecode = e_http_errorcodes(BAD_REQUEST);
      return false;
    }
  }
  std::string decode_path = decode_percent_encoding(_path);
  if (decode_path.empty()) {
    return false;
  } else
    _path = decode_path;
  size_t      query_pos = _path.find('?');
  std::string path      = _path.substr(0, query_pos);
  std::string query     = (query_pos != std::string::npos) ? _path.substr(query_pos + 1) : "";

  if (!check_directory_traversal(path)) {
    _ecode = e_http_errorcodes(FORBIDDEN);
    return false;
  }
  if (!query.empty() && !check_query_string(query)) {
    LOG_ERROR("QUERY STRING IS NOT VALID");
    _ecode = e_http_errorcodes(BAD_REQUEST);
    return false;
  }
  if (_path.size() > static_cast<size_t>(MAX_URI_LENGTH)){
    LOG_ERROR("URI TOO LONG");
    _ecode = e_http_errorcodes(URI_TOO_LONG);
    return false;
  }
  return true;
}
/**
 * @brief Checks the validity of the protocol in HTTP requests.
 *
 * This function verifies if the protocol in an HTTP request complies with established
 * standards, ensuring it is properly formed and contains the necessary information.
 *
 * @param protocol The protocol to check.
 * @return true If the protocol is valid.
 * @return false If the protocol is not valid.
 */
bool RequestParser::check_line_protocol() {
  std::string version = "HTTP/";

  for (size_t i = 0; i < 4; i++) {
    if (_version[i] != version[i]) {
      LOG_ERROR("MALFORMED REQUEST LINE");
      _ecode = e_http_errorcodes(BAD_REQUEST);
      return false;
    }
  }

  if (_version == "HTTP/1.1")
    return true;
  LOG_ERROR("HTTP VERSION NOT SUPPORTED");
  _ecode = e_http_errorcodes(HTTP_VERSION_NOT_SUPPORTED);
  return false;
}

/**
 * @brief Checks for directory traversal in a given URI.
 *
 * This function verifies if a given URI complies with established standards,
 * ensuring it is properly formed and contains the necessary information.
 *
 * @param uri The URI to check.
 * @return true If the URI is valid.
 * @return false If the URI is not valid.
 */

bool RequestParser::check_directory_traversal(const std::string &uri) {
  std::vector<std::string> segments;
  std::string              segment;
  size_t                   start           = 0, end;
  bool                     ends_with_slash = !uri.empty() && uri[uri.length() - 1] == '/';

  while ((end = uri.find('/', start)) != std::string::npos) {
    segment = uri.substr(start, end - start);
    start   = end + 1;

    if (segment == "..") {
      if (!segments.empty())
        segments.pop_back();
      else {
        LOG_ERROR("Directory traversal attempt detected");
        _ecode = e_http_errorcodes(FORBIDDEN);
        return false;
      }
    } else if (segment != "." && !segment.empty()) {
      segments.push_back(segment);
    }
  }

  segment = uri.substr(start);
  if (!segment.empty()) {
    if (segment == "..") {
      if (!segments.empty())
        segments.pop_back();
      else {
        LOG_ERROR("Directory traversal attempt detected");
        _ecode = e_http_errorcodes(FORBIDDEN);
        return false;
      }
    } else if (segment != ".") {
      segments.push_back(segment);
    }
  }

  std::string normalized_uri = "/";
  for (size_t i = 0; i < segments.size(); ++i) {
    if (i > 0)
      normalized_uri += "/";
    normalized_uri += segments[i];
  }

  if (ends_with_slash && normalized_uri[normalized_uri.length() - 1] != '/') {
    normalized_uri += '/';
  }

  _path = normalized_uri;
  return true;
}

/**
 * @brief Checks the validity of the method in HTTP requests.
 *
 * This function verifies if the method in an HTTP request complies with established
 * standards, ensuring it is properly formed and contains the necessary information.
 *
 * @param method The method to check.
 * @return true If the method is valid.
 * @return false If the method is not valid.
 */
bool RequestParser::check_line_method() {
  std::map<std::string, std::string>::iterator it = _httpMethods.find(_method);

  if (it != _httpMethods.end()) {
    if (it->second == "NOT ALLOWED") {
      LOG_ERROR("METHOD NOT ALLOWED");
      _ecode = e_http_errorcodes(METHOD_NOT_ALLOWED);
      return false;
    }
    if (it->second == "NOT IMPLEMENTED") {
      LOG_ERROR("METHOD NOT IMPLEMENTED");
      _ecode = e_http_errorcodes(METHOD_NOT_IMPLEMENTED);
      return false;
    }
    return true;
  } else {
    LOG_ERROR("INVALID METHOD");
    _ecode = e_http_errorcodes(BAD_REQUEST);
    return false;
  }
}
