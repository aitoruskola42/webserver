/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler_file.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/22 13:52:58 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/02 22:06:57 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer/RequestHandler/GetHandler/GetHandler.hpp"

/**
 * Handles the file transfer for a given file path.
 *
 * This function sends the specified file to the client over the provided socket.
 * It checks if the file exists and is accessible, and then sends the file content
 * along with the appropriate HTTP headers. If the file cannot be sent, it returns
 * an error status.
 *
 * @param client_socket The socket connected to the client.
 * @param file_path The path of the file to be sent.
 * @param keep_alive Indicates whether to keep the connection alive after the response.
 * @param config The location configuration containing settings for the file transfer.
 * @return SOCKET_OK if the file was sent successfully, SOCKET_ERROR otherwise.
 */

SocketResult
GetHandler::handleFile(int client_socket, const std::string &file_path, bool keep_alive, const LocationConfig &config) {
  return HttpUtils::sendFile(client_socket, file_path, keep_alive, config, 200);
}
