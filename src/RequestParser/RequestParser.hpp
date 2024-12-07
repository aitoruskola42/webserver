/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:39:38 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/02 11:58:36 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

//------------------------------------------------------------------------------
#include "../Logger/includes/Logger.hpp"
#include "CommonDefinitions.hpp"
//------------------------------------------------------------------------------
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
//------------------------------------------------------------------------------
bool is_numeric(const std::string &str);
bool isValidUriChar(char ch);

//------------------------------------------------------------------------------
class RequestParser {
  // ---------------ATTRIBUTES-------------------------------------------------
 private:
  std::string                        _method;
  std::string                        _path;
  std::string                        _version;
  std::map<std::string, std::string> _headers;
  std::string                        _body;
  unsigned short                     _ecode;
  bool                               _isComplete;
  std::map<std::string, std::string> _httpMethods;
  std::map<std::string, std::string> _queries;
  size_t                             _totalsize;

  // ---------------CONSTRUCTORS-----------------------------------------------
 public:
  RequestParser();
  ~RequestParser();
  RequestParser(const RequestParser &other);

  RequestParser &operator=(const RequestParser &other);

  // -------------------- GETTERS -------------------------------------------
 public:
  bool           isComplete() const;
  unsigned short getErrorCode() const;
  int            getTotalSize() const;
  std::string    getMethod() const;
  std::string    getPath() const;
  std::string    getVersion() const;
  std::string    getHeader(const std::string &name) const;
  std::string    getQuery(const std::string &name) const;
  std::string    getBody() const;
  const std::map<std::string, std::string> &getHeaders() const;
  const std::map<std::string, std::string> &getQueries() const;
  const std::map<std::string, std::string> &getHttpMethods() const;
  // -------------------- UTILS -------------------------------------------
 public:
  void        clear();
  std::string buildQueryString() const;
  std::string parseMultipartFormData(const std::string &content_type) const;

 private:
  void        calculateTotalSize();
  std::string decode_percent_encoding(const std::string &uri);

  // --------------------- PARSERS  ---------------------------------------
 private:
  void parseRequestLine(const std::string &line);
  void parseHeaderLine(const std::string &line);
  void parseBody(const std::string &body);

 public:
  void parseRequest(const std::string &request);

  // ----------------------- CHECKS -----------------------------------------
 private:
  bool check_directory_traversal(const std::string &uri);
  bool check_query_string(const std::string &query);
  bool check_line_method();
  bool check_line_URI();
  bool check_line_protocol();
  bool check_headers();
  bool check_body();
};

std::ostream &operator<<(std::ostream &os, const RequestParser &rp);

#endif
