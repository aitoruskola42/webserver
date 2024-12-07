/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/01 18:38:15 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 18:38:16 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "Logger/includes/Logger.hpp"

class Server {
  //------------------------CONSTRUCTOR--------------------------------------
 public:
  Server();
  virtual ~Server(){};

  //------------------------GETTERS------------------------------------------
 public:
  bool                               getAutoindex() const;
  unsigned int                       getClientMaxBodySize() const;
  int                                getListen() const;
  int                                getType() const;
  std::string                        getName() const;
  std::string                        getIP() const;
  std::string                        getRootPath() const;
  std::string                        getUploadPath() const;
  std::string                        getIp() const;
  std::string                        getLocationPath() const;
  std::vector<std::string>           getIndex() const;
  std::vector<std::string>           getAllowedMethods() const;
  std::vector<std::string>           getServerNames() const;
  std::map<std::string, std::string> getCgiHandler() const;
  std::map<short int, std::string>   getReturnCodePath() const;
  std::map<int, std::string>         getErrorPages() const;
  std::map<std::string, std::string> getLocationCgiHandler() const;

  //------------------------MATCHERS------------------------------------------
 public:
  bool matchServerName(const std::string &hostname) const;
  bool isMethodAllowed(const std::string &method) const;

  //------------------------SETTERS------------------------------------------
 public:
  void setIp(const std::string &ip);
  void setRootPath(const std::string &rootPath);
  void addServerName(const std::string &serverName);
  void set_addIndex(const std::string &index);
  void setAddErrorPage(int code, const std::string &errorPath);
  void setType(int type);
  void setClientMaxBodySize(unsigned int size);
  void setAutoindex(bool autoindex);
  void setCgiHandler(const std::string &extension, const std::string &path);
  void setUploadPath(const std::string &uploadPath);
  void setReturnCodePath(const int code, const std::string path);
  void setListen(int port);
  void setLocationPath(const std::string &locationPath);
  void addAllowedMethod(const std::string &method);
  void clearAllowedMethods();

  //------------------------ATTRIBUTES----------------------------------------
 private:
  int                                _listen;
  int                                _type;
  bool                               _autoindex;
  unsigned int                       _client_max_body_size;
  std::string                        _ip;
  std::string                        _root_path;
  std::string                        _upload_path;
  std::string                        _locationPath;
  std::vector<std::string>           _server_names;
  std::vector<std::string>           _allowed_methods;
  std::vector<std::string>           _index;
  std::map<int, std::string>         _error_pages;
  std::map<std::string, std::string> _cgi_handler;
  std::map<short int, std::string>   _return_code_path;
};

std::ostream &operator<<(std::ostream &o, const Server &i);

#endif // SERVER_HPP
