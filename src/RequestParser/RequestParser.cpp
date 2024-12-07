/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:40:35 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 18:22:29 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

//------------------------------------------------------------------------------
//                            CONSTRUCTORS / DESTRUCTOR
//------------------------------------------------------------------------------

RequestParser::RequestParser() : _method(""), _path(""), _version(""), _body(""), _isComplete(false), _totalsize(0) {
  LOG_DEBUG("RequestParser constructor called");
  _httpMethods["GET"]     = "ALLOWED";
  _httpMethods["POST"]    = "ALLOWED";
  _httpMethods["DELETE"]  = "ALLOWED";
  _httpMethods["PUT"]     = "NOT IMPLEMENTED";
  _httpMethods["PATCH"]   = "NOT IMPLEMENTED";
  _httpMethods["OPTIONS"] = "NOT IMPLEMENTED";
  _httpMethods["HEAD"]    = "ALLOWED";
  _httpMethods["CONNECT"] = "NOT IMPLEMENTED";
  _httpMethods["TRACE"]   = "NOT IMPLEMENTED";
}

RequestParser::~RequestParser() {
  LOG_DEBUG("RequestParser destructor called");
}

RequestParser::RequestParser(const RequestParser &other) {
  *this = other;
  LOG_DEBUG("RequestParser copy constructor called");
}

//------------------------------------------------------------------------------
//                            OPERATORS
//------------------------------------------------------------------------------

RequestParser &RequestParser::operator=(const RequestParser &other) {
  if (this != &other) {
    this->_body       = other._body;
    this->_headers    = other._headers;
    this->_isComplete = other._isComplete;
    this->_method     = other._method;
    this->_path       = other._path;
    this->_version    = other._version;
    this->_ecode      = other._ecode;
  }
  return *this;
}

/**
 * @brief Decodes percent-encoded strings in URIs.
 *
 * This function decodes percent-encoded strings in URIs, converting them back to their original characters.
 *
 * @param uri The URI string containing percent-encoded characters.
 * @return std::string The decoded string.
 */
std::string RequestParser::decode_percent_encoding(const std::string &uri) {
  std::string decoded;
  char        hex[2] = {0};

  for (size_t i = 0; i < uri.length(); ++i) {
    if (uri[i] == '%' && i + 2 < uri.length()) {
      hex[0] = uri[i + 1];
      hex[1] = uri[i + 2];

      // Comprobar si son caracteres hexadecimales válidos
      if (isxdigit(hex[0]) && isxdigit(hex[1])) {
        int value = 0;
        if (hex[0] >= '0' && hex[0] <= '9')
          value += (hex[0] - '0') * 16;
        else
          value += (tolower(hex[0]) - 'a' + 10) * 16;
        if (hex[1] >= '0' && hex[1] <= '9')
          value += (hex[1] - '0');
        else
          value += (tolower(hex[1]) - 'a' + 10);

        decoded += static_cast<char>(value); // Convertir de hexadecimal a char
        i += 2;                              // Saltar los caracteres del porcentaje
                                             // std::cout << "DECODED : " << decoded << "" << std::endl;
      } else {
        // Secuencia inválida
        LOG_ERROR("WRONG ENCODED URI");
        _ecode = e_http_errorcodes(BAD_REQUEST);
        return "";
      }
    } else
      decoded += uri[i];
  }
  // std::cout << "ENCODED" << std::endl;
  return decoded;
}

void RequestParser::calculateTotalSize() {
  // Inicializa el tamaño total
  _totalsize = 0;

  // 1. Sumar el tamaño de las cabeceras
  for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
    _totalsize += it->first.size() + 2;  // Tamaño de la clave + ": "
    _totalsize += it->second.size() + 2; // Tamaño del valor + "\r\n"
  }
  _totalsize += 2; // Una línea en blanco después de las cabeceras ("\r\n")

  // 2. Sumar el tamaño del body
  _totalsize += _body.size();

  // 3. Si es multipart/form-data, sumar los boundaries y las cabeceras de cada parte
  std::string contentType = getHeader("Content-Type");
  if (contentType.find("multipart/form-data") != std::string::npos) {
    // Supongamos que el boundary está definido en el Content-Type
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos != std::string::npos) {
      std::string boundary = "--" + contentType.substr(boundaryPos + 9); // boundary=
      // Sumar el tamaño de cada boundary (antes y después de cada parte)
      _totalsize += (boundary.size() + 4); // "--boundary\r\n" para cada parte
      // Aquí también puedes sumar las cabeceras de cada parte si las tienes
    }
  }
}

std::ostream &operator<<(std::ostream &os, const RequestParser &rp) {
  os << "Method: " << rp.getMethod() << std::endl;
  os << "Path: " << rp.getPath() << std::endl;
  os << "Version: " << rp.getVersion() << std::endl;
  os << "Headers: \n";

  std::map<std::string, std::string>::const_iterator headerIt;
  for (headerIt = rp.getHeaders().begin(); headerIt != rp.getHeaders().end(); ++headerIt)
    os << "    " << headerIt->first << ": " << headerIt->second << std::endl;

  os << "Queries: \n";
  std::map<std::string, std::string>::const_iterator queryIt;
  for (queryIt = rp.getQueries().begin(); queryIt != rp.getQueries().end(); ++queryIt)
    os << "    " << queryIt->first << ": " << queryIt->second << std::endl;

  std::string contentType = rp.getHeader("ContentType");
  if (contentType.find("text") != std::string::npos || contentType.find("json") != std::string::npos) {
    os << "Body: " << rp.getBody() << std::endl;
  } else {
    os << "Body contains binary data (size: " << rp.getBody().size() << " bytes)" << std::endl;
  }
  os << "Error Code: " << rp.getErrorCode() << std::endl;
  os << "Is Complete: " << (rp.isComplete() ? "true" : "false") << std::endl;
  os << "Total Size: " << rp.getTotalSize() << std::endl;
  return os;
}