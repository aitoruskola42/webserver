#ifndef POST_HANDLER_HPP
#define POST_HANDLER_HPP

#include <sys/stat.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "CommonDefinitions.hpp"
#include "RequestParser/RequestParser.hpp"
#include "WebServer/HttpUtils/HttpUtils.hpp"
#include "WebServer/RequestHandler/GetHandler/GetHandler.hpp"

class PostHandler {
 private:
  int                   _post_client_socket;
  const LocationConfig &_post_location_config;
  RequestParser        &_post_request;
  bool                  _post_keep_alive;

 public:
  PostHandler(int                   client_socket,
              const LocationConfig &loc_config,
              RequestParser        &request,
              bool                  keep_alive);
  ~PostHandler();

  SocketResult processPost();

 private:
  void ValidatePost();
  void HeaderandBody();

  SocketResult handleFileUpload(int clientSocket);
  SocketResult processMultipartData(int                clientSocket,
                                    const std::string &data,
                                    const std::string &boundary);
  bool saveUploadedFile(const std::string &filename, const std::string &content);

  // Nuevas funciones para manejar multipart/form-data
  std::string              extractBoundary(const std::string &contentType);
  std::vector<std::string> splitMultipartBody(const std::string &body,
                                              const std::string &boundary);
  bool                     isFilePart(const std::string &part);
  std::string              extractFilename(const std::string &part);
  std::string              extractFieldName(const std::string &part);
  std::string              extractContent(const std::string &part);
  std::pair<std::string, std::string> processTextPart(const std::string &part);
  std::string                         generateResponseMessage(
                              const std::map<std::string, std::string> &formFields);
};

#endif // POST_HANDLER_HPP