/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 16:11:39 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/29 13:32:21 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
std::string joinStrings(const std::vector<std::string> &strings, const std::string &delimiter);
//------------------------------------------------------------------------------
//                   CONSTRUCTOR
//------------------------------------------------------------------------------
Server::Server() : _type(0) {
  _ip = "127.0.0.1";
  addAllowedMethod("GET");
  addAllowedMethod("POST");
  addAllowedMethod("DELETE");
  _autoindex = false;
  setClientMaxBodySize(1000001);
}

//------------------------------------------------------------------------------
//                   SETTERS
//------------------------------------------------------------------------------

void Server::setIp(const std::string &ip) {
  _ip = ip;
}
void Server::addServerName(const std::string &serverName) {
  _server_names.push_back(serverName);
}
void Server::setRootPath(const std::string &rootPath) {
  _root_path = rootPath;
}
void Server::set_addIndex(const std::string &index) {
  _index.push_back(index);
}
void Server::setAddErrorPage(int code, const std::string &errorPath) {
  _error_pages[code] = errorPath;
}
void Server::setType(int type) {
  _type = type;
}
void Server::setClientMaxBodySize(unsigned int size) {
  _client_max_body_size = size;
}

void Server::setAutoindex(bool autoindex) {
  _autoindex = autoindex;
}
void Server::setCgiHandler(const std::string &extension, const std::string &path) {
  _cgi_handler[extension] = path;
}
void Server::setUploadPath(const std::string &uploadPath) {
  _upload_path = uploadPath;
}
void Server::setReturnCodePath(const int code, const std::string path) {
  _return_code_path[code] = path;
}

void Server::setListen(int port) {
  _listen = port;
}
void Server::setLocationPath(const std::string &locationPath) {
  _locationPath = locationPath;
}
void Server::clearAllowedMethods() {
  _allowed_methods.clear();
}
void Server::addAllowedMethod(const std::string &method) {
  _allowed_methods.push_back(method);
}

//------------------------------------------------------------------------------
//                                 GETTERS
//------------------------------------------------------------------------------

std::string Server::getIP() const {
  return _ip;
}
std::string Server::getIp() const {
  return _ip;
}
std::vector<std::string> Server::getServerNames() const {
  return _server_names;
}
int Server::getListen() const {
  return _listen;
}
std::string Server::getRootPath() const {
  return _root_path;
}
std::vector<std::string> Server::getIndex() const {
  return _index;
}
std::map<int, std::string> Server::getErrorPages() const {
  return _error_pages;
}
int Server::getType() const {
  return _type;
}
unsigned int Server::getClientMaxBodySize() const {
  return _client_max_body_size;
}
bool Server::getAutoindex() const {
  return _autoindex;
}
std::map<std::string, std::string> Server::getCgiHandler() const {
  return _cgi_handler;
}
std::string Server::getUploadPath() const {
  return _upload_path;
}
std::map<short int, std::string> Server::getReturnCodePath() const {
  return _return_code_path;
}
/* @brief: check if the server name is in the server names vector
 * @param: hostname: the hostname to check
 * @return: true if the hostname is in the server names vector, false otherwise
 */
bool Server::matchServerName(const std::string &hostname) const {
  for (std::vector<std::string>::const_iterator it = _server_names.begin(); it != _server_names.end(); ++it) {
    if (*it == hostname) {
      return true;
    }
  }
  return false;
}
std::string Server::getLocationPath() const {
  return _locationPath;
}

std::vector<std::string> Server::getAllowedMethods() const {
  return _allowed_methods;
}
std::map<std::string, std::string> Server::getLocationCgiHandler() const {
  return _cgi_handler;
}

bool Server::isMethodAllowed(const std::string &method) const {
  for (std::vector<std::string>::const_iterator it = _allowed_methods.begin(); it != _allowed_methods.end(); ++it) {
    if (*it == method) {
      return true;
    }
  }
  return false;
}
//------------------------------------------------------------------------------
//                          OPERATORS
//------------------------------------------------------------------------------
// Auxiliar function to print the map
template <typename MapType>
void printMap(const std::string &spaces, const std::string &label, const MapType &map) {
  LOG_INFO(spaces << label << ":");
  if (map.empty()) {
    LOG_DEBUG(spaces << "\t\tNo " << label);
  } else {
    for (typename MapType::const_iterator it = map.begin(); it != map.end(); ++it) {
      LOG_INFO(spaces << "\t\t" << it->first << " " << it->second);
    }
  }
}
std::ostream &operator<<(std::ostream &o, const Server &i) {
  std::string spaces = "   ";
  spaces += "    ";

  LOG_INFO((i.getType() == 0 ? "Server" : "\tLocation " + std::string(i.getLocationPath())));
  spaces += (i.getType() == 0 ? "" : "\t");
  LOG_INFO(spaces << "Listen:\t\t" << i.getListen());
  LOG_INFO(spaces << "Ip:\t\t" << i.getIp());
  LOG_INFO(spaces << "Server_names:\t" << joinStrings(i.getServerNames(), " "));
  LOG_INFO(spaces << "Methods:\t" << joinStrings(i.getAllowedMethods(), " "));
  LOG_INFO(spaces << "Root_path:\t" << i.getRootPath());
  LOG_INFO(spaces << "Index:\t\t" << joinStrings(i.getIndex(), " "));
  LOG_INFO(spaces << "Max_body_size:\t" << i.getClientMaxBodySize());
  LOG_INFO(spaces << "Autoindex:\t" << (i.getAutoindex() ? "ON" : "OFF"));
  LOG_INFO(spaces << "Upload_path:\t" << (i.getUploadPath().empty() ? "No upload path" : i.getUploadPath()));
  printMap(spaces, "Error_pages", i.getErrorPages());
  printMap(spaces, "Cgi_handler", i.getCgiHandler());
  printMap(spaces, "Return_path", i.getReturnCodePath());

  return o;
}

//------------------------------------------------------------------------------
//                   JOIN STRINGS
//------------------------------------------------------------------------------
std::string joinStrings(const std::vector<std::string> &strings, const std::string &delimiter) {
  std::string result;
  for (size_t i = 0; i < strings.size(); ++i) {
    result += strings[i];
    if (i < strings.size() - 1) {
      result += delimiter;
    }
  }
  return result;
}

//----------------------------------END---------------------------------------