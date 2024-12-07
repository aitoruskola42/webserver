/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser_utils.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:43:38 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/02 12:02:03 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

/**
 * @brief Checks if a string consists solely of digits.
 *
 * This function iterates through each character of the string and checks
 * if all are digits. If a character that is not a digit is found,
 * the function returns false. If all characters are digits,
 * it returns true.
 *
 * @param str The string to check.
 * @return true If the string consists solely of digits.
 * @return false If the string contains non-numeric characters.
 */
bool is_numeric(const std::string &str) {
  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (!isdigit(*it))
      return false;
  }
  return true;
}

/**
 * @brief Checks if a character is valid in a URI.
 *
 * This function checks if a character is valid in a URI according to the
 * URI standard.
 *
 * @param ch The character to check.
 * @return true If the character is valid in a URI.
 * @return false If the character is not valid in a URI.
 */
bool isValidUriChar(char ch) {
  return (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~' || ch == '/' || ch == '?' || ch == '&' ||
          ch == '=' || ch == '%');
}

/**
 * @brief Builds a query string from the parsed queries.
 *
 * This function constructs a query string from the parsed queries stored in the
 * RequestParser object. It concatenates the key-value pairs with '&' and returns
 * the resulting string.
 *
 * @return The constructed query string.
 */
std::string RequestParser::buildQueryString() const {
  std::string query_string;

  const std::map<std::string, std::string> &queries = getQueries();
  for (std::map<std::string, std::string>::const_iterator it = queries.begin(); it != queries.end(); ++it) {
    if (it != queries.begin()) {
      query_string += "&";
    }
    query_string += it->first + "=" + it->second;
  }
  const std::map<std::string, std::string>          &headers         = getHeaders();
  std::map<std::string, std::string>::const_iterator content_type_it = headers.find("Content-Type");

  if (content_type_it != headers.end()) {
    if (content_type_it->second.find("application/x-www-form") != std::string::npos) {
      if (!query_string.empty()) {
        query_string += "?";
      }
      query_string += getBody();
    } else if (content_type_it->second.find("multipart/form-data") == 0) {
      query_string += parseMultipartFormData(content_type_it->second);
    }
  }

  return query_string;
}

std::string RequestParser::parseMultipartFormData(const std::string &content_type) const {
  std::string query_string;
  std::string body     = getBody();
  std::string boundary = content_type.substr(content_type.find("boundary=") + 9);
  size_t      pos      = 0;

  while ((pos = body.find(boundary, pos)) != std::string::npos) {
    size_t start = body.find("Content-Disposition: form-data; name=\"", pos);
    if (start == std::string::npos)
      break;
    start += 38; // Longitud de "Content-Disposition: form-data; name=\""
    size_t      end  = body.find("\"", start);
    std::string name = body.substr(start, end - start);

    start             = body.find("\r\n\r\n", end) + 4;
    end               = body.find("\r\n--" + boundary, start);
    std::string value = body.substr(start, end - start);

    if (!query_string.empty()) {
      query_string += "&";
    }
    query_string += name + "=" + value;

    pos = end;
  }

  return query_string;
}
