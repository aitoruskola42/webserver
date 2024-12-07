/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/01 17:38:03 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 18:03:05 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DELETE_HANDLER
#define DELETE_HANDLER

// -------------  Webserver includes  -----------------------------------------
#include "RequestParser/RequestParser.hpp"
#include "WebServer/HttpUtils/HttpUtils.hpp"
#include "WebServer/RequestHandler/GetHandler/GetHandler.hpp"
//-----------------------------------------------------------------------------
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

class DeleteHandler {
  // ------------------CONSTRUCTOR-----------------------------------------------
 public:
  DeleteHandler(int                   client_socket,
                const LocationConfig &loc_config,
                RequestParser        &request,
                bool                  keep_alive);
  // ------------------METHODS---------------------------------------------------
 public:
  SocketResult handle_delete_request();

 private:
  int remove_directory(const char *path) const;

  // ------------------ATTRIBUTES------------------------------------------------
 private:
  int                   _del_client_socket;
  const LocationConfig &_del_location_config;
  RequestParser        &_del_request;
  bool                  _del_keep_alive;
};

#endif
