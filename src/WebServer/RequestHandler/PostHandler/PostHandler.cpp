// PostHandler.cpp
#include "PostHandler.hpp"

PostHandler::PostHandler(int client_socket, const LocationConfig &loc_config, RequestParser &request, bool keep_alive)
    : _post_client_socket(client_socket)
    , _post_location_config(loc_config)
    , _post_request(request)
    , _post_keep_alive(keep_alive) {
  LOG_DEBUG("PostHandler constructor called");
}

PostHandler::~PostHandler() {
  LOG_DEBUG("PostHandler destructor called");
}

SocketResult PostHandler::processPost() {
  if (!HttpUtils::isMethodAllowed(_post_location_config.allowed_methods, "POST")) {
    return HttpUtils::sendErrorResponse(_post_client_socket, 405, _post_keep_alive, _post_location_config);
  }
  std::string file_path = HttpUtils::constructFilePath(
      _post_location_config.root_path, _post_location_config.location_path, _post_request.getPath());
  
  if (HttpUtils::isCgiScript(file_path, _post_location_config)) {
    LOG_DEBUG("Executing CGI script");
    return HttpUtils::executeCgiScript(
        _post_client_socket, file_path, _post_request, _post_keep_alive, _post_location_config);
  }

  // METER REDIRECCIONES?
  std::string contentType = _post_request.getHeader("Content-Type");

  if (contentType.empty()) {
    LOG_ERROR("POST ERROR - NO CONTENT-TYPE");
    return HttpUtils::sendErrorResponse(_post_client_socket, 400, _post_keep_alive, _post_location_config);
  }
  if (contentType.find("multipart/form-data") != std::string::npos) {
    std::string boundary = extractBoundary(contentType);
    if (boundary.empty()) {
      LOG_ERROR("POST ERROR - No boundary found in multipart request");
      return HttpUtils::sendErrorResponse(_post_client_socket, 400, _post_keep_alive, _post_location_config);
    }
    return processMultipartData(_post_client_socket, _post_request.getBody(), boundary);
  } else {
    LOG_ERROR("POST ERROR - UNSUPPORTED CONTENT-TYPE: " << contentType);
    std::string errorMessage =
        "Unsupported Content-Type. This server only accepts POST requests with "
        "multipart/form-data.";
    return HttpUtils::sendResponse(_post_client_socket, "text/plain", errorMessage, 415, _post_keep_alive);
  }
}

SocketResult PostHandler::processMultipartData(int clientSocket, const std::string &data, const std::string &boundary) {
  std::vector<std::string> parts = splitMultipartBody(data, boundary);

  bool                               fileUploaded = false;
  std::map<std::string, std::string> formFields;
  unsigned int                       max_body_size = _post_location_config.client_max_body_size;

  if (data.size() > max_body_size) {
    return HttpUtils::sendErrorResponse(clientSocket, 413, _post_keep_alive, _post_location_config);
  }

  for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it) {
    if (isFilePart(*it)) {
      std::string filename = extractFilename(*it);
      std::string content  = extractContent(*it);

      if (!filename.empty() && !content.empty()) {
        if (saveUploadedFile(filename, content)) {
          fileUploaded = true;
        }
        formFields["File name"] = filename;
      }
    } else {
      std::pair<std::string, std::string> field = processTextPart(*it);
      if (!field.first.empty()) {
        formFields[field.first] = field.second;
      }
    }
    LOG_DEBUG("File uploaded: " << fileUploaded);
  }

  std::string responseMessage = generateResponseMessage(formFields);
  return HttpUtils::sendResponse(clientSocket, "text/html", responseMessage, 200, _post_keep_alive);
}

std::string PostHandler::extractBoundary(const std::string &contentType) {
  size_t boundaryPos = contentType.find("boundary=");
  if (boundaryPos != std::string::npos) {
    return "--" + contentType.substr(boundaryPos + 9);
  }
  return "";
}

std::vector<std::string> PostHandler::splitMultipartBody(const std::string &body, const std::string &boundary) {
  std::vector<std::string> parts;
  size_t                   pos         = 0;
  size_t                   boundaryLen = boundary.length();

  while ((pos = body.find(boundary, pos)) != std::string::npos) {
    size_t start = pos + boundaryLen + 2; // +2 for CRLF
    size_t end   = body.find(boundary, start);

    if (end == std::string::npos)
      break;

    parts.push_back(body.substr(start, end - start - 2)); // -2 to remove CRLF
    pos = end;
  }

  return parts;
}

bool PostHandler::isFilePart(const std::string &part) {
  return part.find("filename=") != std::string::npos;
}

std::string PostHandler::extractFilename(const std::string &part) {
  size_t filenamePos = part.find("filename=\"");
  if (filenamePos != std::string::npos) {
    size_t filenameEnd = part.find("\"", filenamePos + 10);
    return part.substr(filenamePos + 10, filenameEnd - (filenamePos + 10));
  }
  return "";
}

std::string PostHandler::extractFieldName(const std::string &part) {
  size_t namePos = part.find("name=\"");
  if (namePos != std::string::npos) {
    size_t nameEnd = part.find("\"", namePos + 6);
    return part.substr(namePos + 6, nameEnd - (namePos + 6));
  }
  return "";
}

std::string PostHandler::extractContent(const std::string &part) {
  size_t headerEnd = part.find("\r\n\r\n");
  if (headerEnd != std::string::npos) {
    return part.substr(headerEnd + 4);
  }
  return "";
}

std::pair<std::string, std::string> PostHandler::processTextPart(const std::string &part) {
  std::string name  = extractFieldName(part);
  std::string value = extractContent(part);
  return std::make_pair(name, value);
}

std::string PostHandler::generateResponseMessage(const std::map<std::string, std::string> &formFields) {
    std::ostringstream oss;
    oss << "<!DOCTYPE html>\n"
        << "<html lang=\"en\">\n"
        << "<head>\n"
        << "    <meta charset=\"UTF-8\">\n"
        << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        << "    <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n"
        << "    <title>File Upload Result</title>\n"
        << "    <!-- Bulma Version 1-->\n"
        << "    <link rel=\"stylesheet\" href=\"https://unpkg.com/bulma@1.0.2/css/bulma.min.css\" />\n"
        << "    <script defer src=\"https://use.fontawesome.com/releases/v5.0.10/js/all.js\"></script>\n"
        << "    <style>\n"
        << "        footer { position: fixed; left: 0; bottom: 0; width: 100%; background-color: #333; color: #fff; text-align: center; font-size: 14px; padding: 10px 0; }\n"
        << "        .small-text { font-size: 0.8em; }\n"
        << "        .icon-column { width: 40px; text-align: center; }\n"
        << "        .icon-column i { font-size: 24px; }\n"
        << "    </style>\n"
        << "    <script>\n"
        << "        setTimeout(function() {\n"
        << "            window.location.href = 'upload.html';\n"
        << "        }, 3000);\n"
        << "    </script>\n"
        << "</head>\n"
        << "<body>\n"
        << "    <nav class=\"navbar is-dark\">\n"
        << "        <div class=\"navbar-brand\">\n"
        << "            <a class=\"navbar-item\" href=\"https://www.42urduliz.com/\">\n"
        << "                <img src=\"/images/logo.png\" alt=\"42 \" width=\"112\" height=\"28\">\n"
        << "            </a>\n"
        << "            <div class=\"navbar-burger burger\" data-target=\"navbarExampleTransparentExample\">\n"
        << "                <span></span>\n"
        << "                <span></span>\n"
        << "                <span></span>\n"
        << "            </div>\n"
        << "        </div>\n"
        << "        <div id=\"navbarExampleTransparentExample\" class=\"navbar-menu\">\n"
        << "            <div class=\"navbar-start\">\n"
        << "                <a class=\"navbar-item\" href=\"/\">\n"
        << "                    Home\n"
        << "                </a>\n"
        << "                <a class=\"navbar-item\" href=\"/files\">\n"
        << "                    Directory list\n"
        << "                </a>\n"
        << "                <a class=\"navbar-item\" href=\"/upload.html\">\n"
        << "                    Upload\n"
        << "                </a>\n"
        << "                <div class=\"navbar-item has-dropdown is-hoverable\">\n"
        << "                    <a class=\"navbar-link\" href=\"/cgi-bin/\">\n"
        << "                        CGI Demonstrations\n"
        << "                    </a>\n"
        << "                    <div class=\"navbar-dropdown is-boxed\">\n"
        << "                        <a class=\"navbar-item\" href=\"/cgi-bin/Python/miCalculadora.html\">\n"
        << "                            Calculator\n"
        << "                        </a>\n"
        << "                        <a class=\"navbar-item\" href=\"/cgi-bin/Perl/passwordGenerator.pl\">\n"
        << "                            Password Generator\n"
        << "                        </a>\n"
        << "                        <a class=\"navbar-item\" href=\"/cgi-bin/Python/contador.py\">\n"
        << "                            Visitors Counter\n"
        << "                        </a>\n"
        << "                    </div>\n"
        << "                </div>\n"
        << "                <div class=\"navbar-item has-dropdown is-hoverable\">\n"
        << "                    <a class=\"navbar-link\" href=\"/documentation/overview/start/\">\n"
        << "                        Redirections\n"
        << "                    </a>\n"
        << "                    <div class=\"navbar-dropdown is-boxed\">\n"
        << "                        <a class=\"navbar-item\" href=\"/buscador\">\n"
        << "                            Redirect 301\n"
        << "                        </a>\n"
        << "                        <a class=\"navbar-item\" href=\"/buscador2\">\n"
        << "                            Redirect 302\n"
        << "                        </a>\n"
        << "                        <a class=\"navbar-item\" href=\"/buscador3\">\n"
        << "                            Redirect 303\n"
        << "                        </a>\n"
        << "                        <a class=\"navbar-item\" href=\"/buscador4\">\n"
        << "                            Redirect 307\n"
        << "                        </a>\n"
        << "                    </div>\n"
        << "                </div>\n"
        << "            </div>\n"
        << "            <a class=\"navbar-item\" href=\"http://localhost:8081\">\n"
        << "                Server 2\n"
        << "            </a>\n"
        << "        </div>\n"
        << "    </nav>\n"
        << "    <section class=\"hero\">\n"
        << "        <h1 class=\"title has-text-centered\">File Upload Result</h1>\n"
        << "        <div class=\"card\">\n"
        << "            <div class=\"card-content\">\n"
        << "                <h2 style=\"color: #4CAF50;\">File uploaded successfully</h2>\n";
    if (!formFields.empty()) {
        oss << "                <p>Form fields received:</p>\n"
            << "                <ul style=\"list-style-type: none; padding: 0;\">\n";
        for (std::map<std::string, std::string>::const_iterator it = formFields.begin(); it != formFields.end(); ++it) {
            oss << "                    <li><strong>" << it->first << ":</strong> " << it->second << "</li>\n";
        }
        oss << "                </ul>\n";
    }
    oss << "                <p>You will be redirected to the upload page in 3 seconds...</p>\n"
        << "            </div>\n"
        << "        </div>\n"
        << "    </section>\n"
        << "    <footer>\n"
        << "        <p style=\"margin: 0;\">Powered by AJXWebserver &copy; 2024</p>\n"
        << "        <p style=\"font-size: 12px; margin: 5px 0 0 0;\">Built with <span style=\"color: #e25555; font-size: 16px; display: inline-block; transform: scale(1.5, 1.3); margin: 0 3px;\">&hearts;</span> by auskola- / javigarc / xamayuel</p>\n"
        << "    </footer>\n"
        << "    <script>\n"
        << "        (function() {\n"
        << "            var burger = document.querySelector('.burger');\n"
        << "            var menu = document.querySelector('#'+burger.dataset.target);\n"
        << "            burger.addEventListener('click', function() {\n"
        << "                burger.classList.toggle('is-active');\n"
        << "                menu.classList.toggle('is-active');\n"
        << "            });\n"
        << "        })();\n"
        << "    </script>\n"
        << "</body>\n"
        << "</html>\n";
    return oss.str();
}

/*std::string PostHandler::generateResponseMessage(const std::map<std::string, std::string> &formFields) {
  std::ostringstream oss;
  oss << "<!DOCTYPE html>\n"
      << "<html>\n"
      << "<head>\n"
      << "    <meta charset=\"UTF-8\">\n"
      << "    <script>\n"
      << "        setTimeout(function() {\n"
      << "            window.location.href = 'upload.html';\n"
      << "        }, 3000);\n"
      << "    </script>\n"
      << "</head>\n"
      << "            <h2 style=\"color: #4CAF50;\">File uploaded "
         "successfully</h2>\n";
  if (!formFields.empty()) {
    oss << "            <p>Form fields received:</p>\n"
        << "            <ul style=\"list-style-type: none; padding: 0;\">\n";
    for (std::map<std::string, std::string>::const_iterator it = formFields.begin(); it != formFields.end(); ++it) {
      oss << "                <li><strong>" << it->first << ":</strong> " << it->second << "</li>\n";
    }
    oss << "            </ul>\n";
  }
  oss << "            <p>You will be redirected to the upload page in 3 "
         "seconds...</p>\n"
      << "</body>\n"
      << "</html>\n";
  return oss.str();
}*/

bool PostHandler::saveUploadedFile(const std::string &filename, const std::string &content) {
  std::string uploadPath = _post_location_config.upload_path;
  if (uploadPath.empty()) {
    uploadPath = "./uploads/";
  }

  mkdir(uploadPath.c_str(), 0777);

  std::string   fullPath = uploadPath + "/" + filename;
  std::ofstream file(fullPath.c_str(), std::ios::binary);
  if (!file) {
    LOG_ERROR("Failed to open file for writing: " << fullPath);
    return false;
  }

  file.write(content.c_str(), content.length());
  file.close();

  if (!file) {
    LOG_ERROR("Failed to write to file: " << fullPath);
    return false;
  }

  LOG_INFO("File saved successfully: " << fullPath);
  return true;
}
