#include "PostHandler.hpp"

SocketResult PostHandler::handleFileUpload(int clientSocket) {
  std::string contentType = _post_request.getHeader("Content-Type");
  if (contentType.empty()) {
    LOG_ERROR("POST ERROR - NO CONTENT-TYPE");
    return HttpUtils::sendErrorResponse(clientSocket, 400, _post_keep_alive, _post_location_config);
  }

  std::string transferEncoding = _post_request.getHeader("Transfer-Encoding");

  std::string boundary;
  if (contentType.find("multipart/form-data") != std::string::npos) {
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos != std::string::npos && boundaryPos + 9 < contentType.size()) {
      boundary = contentType.substr(boundaryPos + 9);
    }
    if (boundary.empty()) {
      LOG_ERROR("POST ERROR -  NO boundary");
      return HttpUtils::sendErrorResponse(clientSocket, 400, _post_keep_alive, _post_location_config);
    }
  }

  // El cuerpo de la solicitud ya está en _post_request.getBody()
  std::string requestBody = _post_request.getBody();

  // Procesa el cuerpo directamente sin intentar leer más datos
  return processMultipartData(clientSocket, requestBody, boundary);
}