#ifndef HEAD_HANDLER_HPP
#define HEAD_HANDLER_HPP

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "CommonDefinitions.hpp"
#include "RequestParser/RequestParser.hpp"
#include "WebServer/HttpUtils/HttpUtils.hpp"
#include "WebServer/RequestHandler/GetHandler/GetHandler.hpp"

class HeadHandler {
 private:
  int                   _head_client_socket;
  const LocationConfig &_head_location_config;
  RequestParser        &_head_request;
  bool                  _head_keep_alive;

 public:
  HeadHandler(int                   client_socket,
              const LocationConfig &loc_config,
              RequestParser        &request,
              bool                  keep_alive);
  ~HeadHandler();

  SocketResult processHead();
};

#endif // HEAD_HANDLER_HPP
