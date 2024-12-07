/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeadHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/01 17:58:43 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 17:59:23 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeadHandler.hpp"

//-----------------------------------------------------------------------------
//                               CONSTRUCTOR / DESTRUCTOR
//-----------------------------------------------------------------------------
HeadHandler::HeadHandler(int client_socket, const LocationConfig &loc_config, RequestParser &request, bool keep_alive)
    : _head_client_socket(client_socket)
    , _head_location_config(loc_config)
    , _head_request(request)
    , _head_keep_alive(keep_alive) {}

HeadHandler::~HeadHandler() {}

//-----------------------------------------------------------------------------
//                               PROCESS HEAD
//-----------------------------------------------------------------------------
SocketResult HeadHandler::processHead() {
  if (!HttpUtils::isMethodAllowed(_head_location_config.allowed_methods, "HEAD")) {
    return HttpUtils::sendErrorResponse(_head_client_socket, 405, _head_keep_alive, _head_location_config);
  }

  return HttpUtils::sendHead(_head_client_socket, _head_request.getPath(), _head_keep_alive, _head_location_config);
}
