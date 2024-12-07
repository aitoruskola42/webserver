/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigurationManager.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 15:46:38 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/03 20:03:01 by xamayuel         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "ConfigurationManager.hpp"
//------------------------------------------------------------------------------
//                                  PARSING
//------------------------------------------------------------------------------
/**
 * @brief Parse the configuration file
 * @param file_path The path to the configuration file
 * @return True if the file was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseFile(const std::string &file_path) {
  std::ifstream file(file_path.c_str());
  if (!file.is_open()) {
    LOG_ERROR("Error opening file: " << file_path);
    return false;
  }
  LOG_SUCCESS("File opened successfully: " << file_path);
  std::string line;
  while (std::getline(file, line)) {
    // Si la línea solo contiene espacios o tabulaciones, continuar con el bucle
    if (line.find_first_not_of(" \t") == std::string::npos) {
      continue;
    }
    std::string::size_type commentPos = line.find('#');
    if (commentPos != std::string::npos) {
      line.erase(commentPos);
    }
    if (line.empty()) {
      continue;
    }
    if (!parseLine_new(line, getDepth(line))) {
      LOG_ERROR("Error parsing line: " << line);
      return false;
    }
  }
  file.close();
  return true;
}
/**
 * @brief Parse a line of the configuration file
 * @param line The line to parse
 * @param depth The depth of the line
 * @return True if the line was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseLine_new(const std::string &line, int depth) {
  std::string        trimmedLine = trim(line);
  std::istringstream iss(trimmedLine);
  std::string        token;

  if (!(iss >> token))
    return false;
  if (depth == 0) {
    return parseConfig(iss, token, depth);
  } else {
    return parseServerConfig(iss, token, depth);
  }
  return true;
}
/**
 * @brief Parse the global configuration
 * @param iss The stream to parse
 * @param token The token to parse
 * @param depth The depth of the token
 * @return True if the token was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseConfig(std::istringstream &iss, std::string &token, int depth) {
  if (isGlobalConfigToken(token)) {
    if (depth == 0 and token != "server") {
      std::string iss_str((std::istreambuf_iterator<char>(iss)), std::istreambuf_iterator<char>());
      setConfig(token, trim(iss_str));
      return true;
    }
    if (token == "server") {
      addServer();
      return true;
    }
    return false;
  }
  return false;
}
/**
 * @brief Parse the server configuration
 * @param iss The stream to parse
 * @param token The token to parse
 * @param depth The depth of the token
 * @return True if the token was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseServerConfig(std::istringstream &iss, std::string &token, int depth) {
  std::string value;
  std::getline(iss, value);
  value = trim(value);

  if (token == "listen" and depth == 1)
    return parseListen(value);
  else if (token == "server_name" and depth == 1)
    return parseServerName(value);
  else if (token == "root_path" and (depth == 1 or depth == 2))
    return parseRootPath(value);
  else if (token == "index" and (depth == 1 or depth == 2))
    return parseIndex(value);
  else if (token == "error_page" and (depth == 1 or depth == 2))
    return parseErrorPage(value);
  else if (token == "client_max_body_size" and (depth == 1 or depth == 2))
    return parseClientMaxBodySize(value);
  else if (token == "autoindex" and (depth == 1 or depth == 2))
    return parseAutoindex(value);
  else if (token == "allowed_methods" and (depth == 1 or depth == 2))
    return parseAllowedMethods(value);
  else if (token == "cgi_ext" and (depth == 1 or depth == 2))
    return parseCgiExt(value);
  else if (token == "upload_path" and (depth == 1 or depth == 2))
    return parseUploadPath(value);
  else if (token == "location" and depth == 1) {
    return parseLocation(value);
  } else if (token == "return" and depth == 2)
    return parseReturn(value);
  return false;
}
/**
 * @brief Parse the location configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseLocation(const std::string &value) {
  Server      newServer;
  bool        foundServer   = false;
  std::string modifiedValue = value;
  if (isValidLocationPath(value)) {
    if (modifiedValue.length() > 1 && modifiedValue[modifiedValue.length() - 1] == '/') {
      modifiedValue = modifiedValue.substr(0, modifiedValue.length() - 1);
    }

    for (std::vector<Server>::reverse_iterator it = _servers.rbegin(); it != _servers.rend(); ++it) {
      if (it->getType() == 0) {
        newServer   = *it;
        foundServer = true;
        break;
      }
    }

    if (!foundServer) {
      LOG_CRITICAL("Location without server");
      return false;
    }
  } else {
    LOG_ERROR("Invalid location path: " << value);
    return false;
  }
  _servers.push_back(newServer);
  _servers.back().setType(1);
  _servers.back().setLocationPath(modifiedValue);
  return true;
}
/**
 * @brief Check if the location path is valid
 * @param path The path to check
 * @return True if the path is valid, false otherwise
 */
bool ConfigurationManager::isValidLocationPath(const std::string &path) {
  std::string::const_iterator start = path.begin();

  while (start != path.end() && *start == ' ') {
    ++start;
  }
  if (path.empty() || path[0] != '/') {
    return false;
  }
  for (std::string::const_iterator it = path.begin(); it != path.end(); ++it) {
    if (*it == ' ' || *it == '.') {
      return false;
    }
  }
  return true;
}
/**
 * @brief Parse the return configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseReturn(const std::string &value) {
  std::istringstream iss(value);
  std::string        firstWord, secondWord;
  int                statusCode;

  if (!(iss >> firstWord >> secondWord) || iss >> std::ws) {
    return false;
  }

  try {
    statusCode = std::atoi(firstWord.c_str());
  } catch (const std::invalid_argument &e) {
    LOG_ERROR("Return first argument is not a number: " << firstWord);
    return false;
  }

  if (statusCode != 301 && statusCode != 302 && statusCode != 303 && statusCode != 307 && statusCode != 308) {
    LOG_ERROR("Return first argument should be 301, 302, 303, 307 or 308: " << statusCode);
    return false;
  }
  if (secondWord.substr(0, 7) != "http://" && secondWord.substr(0, 8) != "https://") {
    LOG_ERROR("Not a valid HTTP redirection URL: " << secondWord);
    return false;
  }
  if (!_servers.empty()) {
    _servers.back().setReturnCodePath(statusCode, secondWord);
  } else {
    LOG_CRITICAL("No server to assign " << value);
    return false;
  }
  return true;
}
/**
 * @brief Parse the upload path configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseUploadPath(const std::string &value) {
  struct stat buffer;

  if (value.empty() || stat(value.c_str(), &buffer) != 0 || !(buffer.st_mode & S_IWUSR)) {
    if (stat(value.c_str(), &buffer) != 0) {
      if (mkdir(value.c_str(), 0755) != 0) {
        LOG_ERROR("Invalid upload path or no write permissions: " << value);
        return false;
      } else {
        LOG_INFO("Folder created: " << value);
      }
    }
  }
  if (!_servers.empty()) {
    _servers.back().setUploadPath(value);
  } else {
    LOG_CRITICAL("Server not found for upload_path: " << value);
    return false;
  }
  return true;
}
/**
 * @brief Parse the CGI extension configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseCgiExt(const std::string &value) {
  std::istringstream iss(value);
  std::string        extension, path;
  if (!(iss >> extension >> path) || iss >> std::ws) {
    LOG_ERROR("Incorrect CGI format: " << value);
    return false;
  }
  if (extension.empty() || extension[0] != '.') {
    LOG_ERROR("Not valid extension for CGI: " << extension);
    return false;
  }

  struct stat buffer;
  if (stat(path.c_str(), &buffer) != 0) {
    LOG_ERROR("Not valid path: " << path);
    return false;
  }

  if (!_servers.empty()) {
    _servers.back().setCgiHandler(extension, path);

  } else {
    LOG_CRITICAL("No server to assign " << value);
    return false;
  }
  return true;
}
/**
 * @brief Parse the autoindex configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseAutoindex(const std::string &value) {
  if (value.size() == 2 && std::tolower(value[0]) == 'o' && std::tolower(value[1]) == 'n') {
    if (!_servers.empty()) {
      _servers.back().setAutoindex(true);
    } else {
      LOG_CRITICAL("No server to assign " << value);
      return false;
    }
    return true;
  } else if (value.size() == 3 && std::tolower(value[0]) == 'o' && std::tolower(value[1]) == 'f' &&
             std::tolower(value[2]) == 'f') {
    if (!_servers.empty()) {
      _servers.back().setAutoindex(false);
    } else {
      LOG_CRITICAL("No server to assign " << value);
      return false;
    }

    return true;
  }
  return false;
}
/**
 * @brief Parse the client max body size configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseClientMaxBodySize(const std::string &value) {
  unsigned int size          = 0;
  char         suffix        = value[value.length() - 1];
  std::string  modifiedValue = value.substr(0, value.length() - 1); // Eliminar el último carácter

  size_t start = modifiedValue.find_first_not_of(" \t"); // Encontrar el primer carácter no espacio
  if (start != std::string::npos) {
    modifiedValue = modifiedValue.substr(start); // Eliminar espacios o tabuladores iniciales
  } else {
    modifiedValue = ""; // Si solo había espacios, asignar una cadena vacía
  }
  if (modifiedValue.empty()) {
    LOG_ERROR("Invalid client max body size: " << value);
    return false;
  }

  // Comprobar si todos los caracteres son dígitos
  for (size_t i = 0; i < modifiedValue.size(); ++i) {
    if (!isdigit(modifiedValue[i])) {
      LOG_ERROR("Invalid client max body size: " << value);
      return false;
    }
  }
  size = static_cast<unsigned int>(atoi(modifiedValue.c_str())); // Convertir a unsigned int

  switch (suffix) {
    case 'k':
    case 'K':
      size *= 1024;
      break;
    case 'm':
    case 'M':
      size *= 1024 * 1024;
      break;
    case 'g':
    case 'G':
      size *= 1024 * 1024 * 1024;
      break;
    case '0':
      break;
    default:
      LOG_ERROR("Invalid suffix for client max body size: " << suffix);

      return false;
  }
  if (!_servers.empty()) {
    _servers.back().setClientMaxBodySize(size);
  } else {
    LOG_CRITICAL("No server to assign " << value);
    return false;
  }
  return true;
}
/**
 * @brief Parse the error page configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseErrorPage(const std::string &value) {
  std::istringstream errorStream(value);
  std::string        errorCode;
  std::string        errorPath;
  if (errorStream >> errorCode >> errorPath) {
    int              code = std::atoi(errorCode.c_str());
    std::vector<int> validErrorCodes;
    validErrorCodes.push_back(400);
    validErrorCodes.push_back(401);
    validErrorCodes.push_back(403);
    validErrorCodes.push_back(404);
    validErrorCodes.push_back(405);
    validErrorCodes.push_back(408);
    validErrorCodes.push_back(413);
    validErrorCodes.push_back(414);
    validErrorCodes.push_back(415);
    validErrorCodes.push_back(429);
    validErrorCodes.push_back(500);
    validErrorCodes.push_back(501);
    validErrorCodes.push_back(502);
    validErrorCodes.push_back(503);
    validErrorCodes.push_back(504);

    std::vector<int>::const_iterator it = std::find(validErrorCodes.begin(), validErrorCodes.end(), code);
    if (it != validErrorCodes.end()) {
      if (!_servers.empty()) {
        _servers.back().setAddErrorPage(code, errorPath);
      } else {
        LOG_CRITICAL("No server to assign " << value);
        return false;
      }
    } else {
      LOG_ERROR("Invalid error code: " << errorCode);
      return false;
    }
  } else {
    LOG_ERROR("Format error: " << value);
    return false;
  }
  return true;
}
/**
 * @brief Parse the index configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseIndex(const std::string &value) {
  std::istringstream iss(value);
  std::string        indexValue;
  while (iss >> indexValue) {
    if (!_servers.empty()) {
      _servers.back().set_addIndex(indexValue);
    } else {
      LOG_CRITICAL("No server to assign " << value);
      return false;
    }
  }
  return true;
}
/**
 * @brief Parse the listen configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseListen(const std::string &value) {
  size_t colonPos = value.find(':');

  if (colonPos != std::string::npos) {
    std::string ip      = value.substr(0, colonPos);
    std::string portStr = value.substr(colonPos + 1);

    struct in_addr addr;
    if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
      LOG_ERROR("Dirección IP inválida: " << ip);

      return false;
    }

    if (!_servers.empty()) {
      _servers.back().setIp(ip);
    } else {
      LOG_ERROR("No server to assign IP: " << ip);
      return false;
    }

    return validateAndSetPort(portStr);
  } else {
    if (!_servers.empty()) {
      _servers.back().setIp("127.0.0.1");
    } else {
      LOG_ERROR("No server to assign ");
      return false;
    }
    return validateAndSetPort(value);
  }
}
/**
 * @brief Validate and set the port
 * @param portStr The port to validate and set
 * @return True if the port was validated and set successfully, false otherwise
 */
bool ConfigurationManager::validateAndSetPort(const std::string &portStr) {
  std::istringstream iss(portStr);
  int                port;

  if (!(iss >> port) || !iss.eof()) {
    LOG_ERROR("Invalid port: " << portStr);
    return false;
  }

  if (port <= 0 || port >= 65536) {
    LOG_ERROR("Invalid port: " << port);
    return false;
  }
  if (!_servers.empty()) {
    _servers.back().setListen(port);
  } else {
    LOG_CRITICAL("No server to assign " << portStr);
    return false;
  }
  return true;
}
/**
 * @brief Parse the server name configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseServerName(const std::string &value) {
  std::istringstream valueStream(value);
  std::string        serverName;
  while (valueStream >> serverName) {
    for (std::vector<Server>::const_iterator serverIt = _servers.begin(); serverIt != _servers.end(); ++serverIt) {
      const std::vector<std::string> &serverNames = serverIt->getServerNames();
      for (std::vector<std::string>::const_iterator nameIt = serverNames.begin(); nameIt != serverNames.end();
           ++nameIt) {
        if (*nameIt == serverName && serverIt->getListen() == _servers.back().getListen()) {
          LOG_ERROR("IGUALES:: Server Name: " << *nameIt << " en IP: " << serverIt->getListen());
          return false;
        }
      }
    }
    if (!_servers.empty()) {
      _servers.back().addServerName(serverName);
    } else {
      LOG_CRITICAL("No server to assign server_name: " << serverName);
      return false;
    }
  }
  return true;
}
/**
 * @brief Parse the root path configuration
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseRootPath(const std::string &value) {
  if (std::ifstream(value.c_str())) {
    if (!_servers.empty()) {
      _servers.back().setRootPath(value);
    } else {
      LOG_CRITICAL("No server to assign " << value);
      return false;
    }
  } else {
    LOG_ERROR("Root path invalid: " << value);

    return false;
  }
  return true;
}
/**
 * @brief Parse the allowed methods
 * @param value The value to parse
 * @return True if the value was parsed successfully, false otherwise
 */
bool ConfigurationManager::parseAllowedMethods(const std::string &value) {
  std::istringstream methodStream(value);
  std::string        method;

  if (!_servers.empty()) {
    _servers.back().clearAllowedMethods();
  } else {
    LOG_ERROR("No server to assign " << value);
    LOG_ERROR("CLEAR METHODS");
    return false;
  }

  std::vector<std::string> validMethods;
  validMethods.push_back("GET");
  validMethods.push_back("POST");
  validMethods.push_back("DELETE");
  // validMethods.push_back("HEAD");

  while (methodStream >> method) {
    if (std::find(validMethods.begin(), validMethods.end(), method) == validMethods.end()) {
      LOG_ERROR("Method NOT valid found in: " << value);
      return false;
    } else {
      if (!_servers.empty()) {
        _servers.back().addAllowedMethod(method);
      } else {
        LOG_ERROR("No server to assign " << value);
        return false;
      }
    }
  }
  return true;
}
//------------------------------------------------------------------------------
//                                GETTERS
//------------------------------------------------------------------------------
int ConfigurationManager::get_max_clients() {
  return atoi(_configMap["max_clients"].c_str());
}
int ConfigurationManager::get_keep_alive_timeout() {
  return atoi(_configMap["keep_alive_timeout"].c_str());
}

std::string ConfigurationManager::get_debug_file() {
  return _configMap["debug_file"];
}

int ConfigurationManager::get_serverCount() {
  return _servers.size();
}

std::vector<Server> ConfigurationManager::get_servers() {
  return _servers;
}
std::string ConfigurationManager::get_log_level() {
  return _configMap["log_level"];
}
//------------------------------------------------------------------------------
//                                SETTERS
//------------------------------------------------------------------------------
void ConfigurationManager::set_max_clients(std::string max_clients) {
  _configMap["max_clients"] = max_clients;
}

void ConfigurationManager::set_keep_alive_timeout(std::string keep_alive_timeout) {
  _configMap["keep_alive_timeout"] = keep_alive_timeout;
}

void ConfigurationManager::set_debug_file(std::string debug_file) {
  _configMap["debug_file"] = debug_file;
}

void ConfigurationManager::addServer() {
  _servers.push_back(Server());

  _servers.back().setLocationPath("/");
}
void ConfigurationManager::setConfig(const std::string &key, const std::string &value) {
  _configMap[key] = value;
}

//------------------------------------------------------------------------------
//                                UTILS
//------------------------------------------------------------------------------
/**
 * @brief Trim the line
 * @param line The line to trim
 * @return The trimmed line
 */
std::string ConfigurationManager::trim(const std::string &line) const {
  size_t first = line.find_first_not_of(" \t");
  if (first == std::string::npos)
    return "";
  size_t last = line.find_last_not_of(" \t");
  return line.substr(first, (last - first + 1));
}
/**
 * @brief Get the depth of the line
 * @param line The line to get the depth
 * @return The depth of the line
 */
int ConfigurationManager::getDepth(const std::string &line) const {
  std::string tabLine = line;
  size_t      pos     = 0;

  while ((pos = tabLine.find("    ", pos)) != std::string::npos) {
    tabLine.replace(pos, 4, "\t");
    pos += 1;
  }
  int                         tabCount = 0;
  std::string::const_iterator it;
  for (it = tabLine.begin(); it != tabLine.end(); ++it) {
    if (*it == '\t') {
      ++tabCount;
    } else {
      break;
    }
  }
  return tabCount;
}
/**
 * @brief Check if the token is a global configuration token
 * @param token The token to check
 * @return True if the token is a global configuration token, false otherwise
 */
bool ConfigurationManager::isGlobalConfigToken(const std::string &token) {
  static const char *validTokens[] = {"server", "debug_file", "log_level", "max_clients", "keep_alive_timeout", NULL};

  for (int i = 0; validTokens[i] != NULL; ++i) {
    if (token == validTokens[i]) {
      return true;
    }
  }
  return false;
}
/**
 * @brief Get the server
 * @param hostname The hostname to get the server
 * @param server_port The server port
 * @param request_path The request path
 * @return The server
 */

Server *ConfigurationManager::get_server(const std::string &hostname, int server_port, std::string request_path) {
  LOG_DEBUG("Searching for server: hostname=" << hostname << ", port=" << server_port << ", path=" << request_path);

  Server     *exactMatch = NULL;
  Server     *portMatch  = NULL;
  Server     *bestMatch  = NULL;
  std::string bestMatchPath;

  for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
    LOG_DEBUG("Checking server: hostname=" << (it->getServerNames().empty() ? "N/A" : it->getServerNames()[0])
                                          << ", port=" << it->getListen() << ", location=" << it->getLocationPath());

    if (it->getListen() == server_port) {
      if (portMatch == NULL) {
        portMatch = &(*it);
        LOG_DEBUG("Port match found");
      }

      if (it->matchServerName(hostname)) {
        std::string locationPath = it->getLocationPath();

        if (locationPath == request_path) {
          LOG_DEBUG("Exact match found for path: " << request_path);
          exactMatch = &(*it);
          break; // end the search
        }

        // Buscar la mejor coincidencia parcial
        if (request_path.compare(0, locationPath.length(), locationPath) == 0) {
          if (bestMatch == NULL || locationPath.length() > bestMatchPath.length()) {
            bestMatch     = &(*it);
            bestMatchPath = locationPath;
            LOG_DEBUG("Better match found: " << locationPath);
          }
        }
      }
    } else {
      LOG_DEBUG("Port mismatch, skipping");
    }
  }

  // Priorizar la selección del servidor
  if (exactMatch != NULL) {
    LOG_DEBUG("Returning exact match");
    return exactMatch;
  }
  if (bestMatch != NULL) {
    LOG_DEBUG("Returning best partial match");
    return bestMatch;
  }
  if (portMatch != NULL) {
    LOG_DEBUG("Returning port match");
    return portMatch;
  }

  LOG_WARNING("No matching server found for request");
  // Si no se encuentra ningún servidor, devolver el primer servidor como fallback
  if (!_servers.empty()) {
    LOG_WARNING("Using first server as fallback");
    return &_servers[0];
  }

  LOG_ERROR("No servers available");
  return NULL;
}

//------------------------------------------------------------------------------
//                               OPERATORS
//------------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &o, ConfigurationManager &i) {
  std::string spaces = "";
  LOG_INFO("LOADED CONFIGURATION");

  LOG_INFO(spaces << "max_clients:\t\t" << i.get_max_clients());
  LOG_INFO(spaces << "keep_alive_timeout:\t" << i.get_keep_alive_timeout());
  LOG_INFO("debug_file:\t\t" << i.get_debug_file());
  LOG_INFO("log_level:\t\t" << i.get_log_level());
  std::vector<Server> servers = i.get_servers();
  for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
    o << *it;
  }
  return o;
}
