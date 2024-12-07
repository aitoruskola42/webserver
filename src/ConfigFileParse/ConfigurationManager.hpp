/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigurationManager.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/01 18:10:14 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 18:10:48 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGURATIONMANAGER_HPP
#define CONFIGURATIONMANAGER_HPP

//------------------------------------------------------------------------------
#include "Logger/includes/Logger.hpp"
#include "Server/Server.hpp"
//------------------------------------------------------------------------------
#include <arpa/inet.h>
#include <sys/stat.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class ConfigurationManager {
  //------------------------ATTRIBUTES------------------------------------------
 private:
  std::map<std::string, std::string> _configMap;
  std::vector<Server>                _servers;

  //------------------------GETTERS---------------------------------------------
 public:
  int                 get_max_clients();
  int                 get_keep_alive_timeout();
  int                 get_serverCount();
  std::string         get_log_level();
  std::string         get_debug_file();
  std::vector<Server> get_servers();

  //------------------------SETTERS---------------------------------------------
  void set_max_clients(std::string max_clients);
  void set_keep_alive_timeout(std::string keep_alive_timeout);
  void set_debug_file(std::string debug_file);
  void setConfig(const std::string &key, const std::string &value);
  void addServer();
  bool validateAndSetPort(const std::string &portStr);

  //------------------------PARSING------------------------------------------
  bool parseFile(const std::string &file_path);
  bool parseLine_new(const std::string &line, int depth);
  bool parseConfig(std::istringstream &iss, std::string &token, int depth);
  bool parseServerConfig(std::istringstream &iss, std::string &token, int depth);
  bool parseListen(const std::string &value);
  bool parseServerName(const std::string &value);
  bool parseRootPath(const std::string &value);
  bool parseIndex(const std::string &value);
  bool parseErrorPage(const std::string &value);
  bool parseClientMaxBodySize(const std::string &value);
  bool parseAutoindex(const std::string &value);
  bool parseCgiExt(const std::string &value);
  bool parseUploadPath(const std::string &value);
  bool parseReturn(const std::string &value);
  bool parseLocation(const std::string &value);
  bool parseAllowedMethods(const std::string &value);

  //------------------------UTILS------------------------------------------
  int         getDepth(const std::string &line) const;
  bool        isGlobalConfigToken(const std::string &token);
  bool        isValidLocationPath(const std::string &path);
  std::string trim(const std::string &line) const;
  Server     *get_server(const std::string &hostname,
                         int                server_port,
                         std::string        request_path);
};

std::ostream &operator<<(std::ostream &o, ConfigurationManager &i);

#endif