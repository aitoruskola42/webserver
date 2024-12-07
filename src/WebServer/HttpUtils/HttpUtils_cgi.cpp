#include "HttpUtils.hpp"

bool HttpUtils::isCgiScript(const std::string &filepath, const LocationConfig &config) {
  
  LOG_DEBUG("filepath: " << filepath);
  // Verificar si hay un punto en el filepath
  size_t dot_pos = filepath.find_last_of('.');
  std::string extension = (dot_pos != std::string::npos) ? filepath.substr(dot_pos) : "";
  LOG_DEBUG("Extension: " << extension);
  std::map<std::string, std::string>::const_iterator it;
  for (it = config.cgi_extensions.begin(); it != config.cgi_extensions.end(); ++it) {
    if (extension == it->first) {
      LOG_DEBUG("Script CGI Extension: " + extension);
      return (true);
    }
  }

  LOG_DEBUG("File is NOT a CGI Script: " + extension);
  return (false);
}

bool HttpUtils::findCgiExecutable(const std::string    &filepath,
                                  const LocationConfig &config,
                                  std::string          &cgi_path,
                                  std::string          &program_name) {
  std::string                                        extension = filepath.substr(filepath.find_last_of('.'));
  std::map<std::string, std::string>::const_iterator it;
  for (it = config.cgi_extensions.begin(); it != config.cgi_extensions.end(); ++it) {
    if (extension == it->first) {
      cgi_path     = it->second;
      program_name = cgi_path.substr(cgi_path.find_last_of('/') + 1);
      return true;
    }
  }
  return false;
}

SocketResult HttpUtils::executeCgiScript(int                   client_socket,
                                         const std::string    &filepath,
                                         const RequestParser  &parser,
                                         bool                  keep_alive,
                                         const LocationConfig &config) {
  std::string cgi_path, program_name;

  // search for the cgi executable
  if (!HttpUtils::findCgiExecutable(filepath, config, cgi_path, program_name)) {
    return HttpUtils::sendErrorResponse(client_socket, 500, keep_alive, config);
  }

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    return HttpUtils::sendErrorResponse(client_socket, 500, keep_alive, config);
  }

  pid_t pid = fork();
  if (pid == -1) {
    close(pipefd[0]);
    close(pipefd[1]);
    return HttpUtils::sendErrorResponse(client_socket, 500, keep_alive, config);
  } else if (pid == 0) {
    return executeCgiChild(pipefd, filepath, parser, cgi_path, program_name);
  } else {
    return executeCgiParent(client_socket, pipefd, pid, keep_alive, config);
  }
}

SocketResult HttpUtils::executeCgiChild(int                  pipefd[2],
                                        const std::string   &filepath,
                                        const RequestParser &parser,
                                        const std::string   &cgi_path,
                                        const std::string   &program_name) {

  // Proceso hijo
  LOG_DEBUG("Child process created");
  close(pipefd[0]); // Cerrar extremo de lectura
  dup2(pipefd[1], STDOUT_FILENO);
  close(pipefd[1]);

  // Configurar variables de entorno para CGI
  std::string query_string = parser.buildQueryString(); // Llamada a la nueva función
  setenv("QUERY_STRING", query_string.c_str(), 1);
  setenv("SCRIPT_FILENAME", filepath.c_str(), 1);
  setenv("REQUEST_URI", parser.getPath().c_str(), 1);
  setenv("REQUEST_METHOD", "GET", 1);
  setenv("REMOTE_ADDR", "0.0.0.0", 1);
  // Añadir más variables de entorno según sea necesario

  // Ejecutar el script Python // CAMBIAR POR VALORES REALES
  execl(cgi_path.c_str(), program_name.c_str(), filepath.c_str(), NULL);

  // Si llegamos aquí, hubo un error en execl
  LOG_ERROR("CGI Execution Error"); // NO HAY QUE HACER ALGO MAS?
  exit(1);
}

SocketResult
HttpUtils::executeCgiParent(int client_socket, int pipefd[2], pid_t pid, bool keep_alive, const LocationConfig &config) {
  // Proceso padre
  LOG_DEBUG("Parent process waiting");
  close(pipefd[1]); // Cerrar extremo de escritura

  char        buffer[4096];
  std::string response;
  ssize_t     bytes_read;

  // Configurar tiempo límite
  const int TIMEOUT_SECONDS = 1;
  time_t    start_time      = time(NULL);

  while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
    response.append(buffer, bytes_read);

    if (difftime(time(NULL), start_time) > TIMEOUT_SECONDS) {
      LOG_WARNING("CGI Timeout reached");
      kill(pid, SIGTERM);
      return HttpUtils::sendErrorResponse(client_socket, 408, keep_alive, config);
    }
  }
  close(pipefd[0]);

  int   status;
  pid_t result = waitpid(pid, &status, WNOHANG);

  if (result == 0) {
    // El proceso hijo aún está en ejecución, lo terminamos
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    LOG_WARNING("CGI Execution Timeout");
    return HttpUtils::sendErrorResponse(client_socket, 408, keep_alive, config);
  } else if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
    // El script se ejecutó correctamente
    LOG_DEBUG("CGI Execution Success");
    return HttpUtils::sendResponse(client_socket, "text/html", response, 200, keep_alive);
  } else {
    // Hubo un error en la ejecución del script
    LOG_WARNING("CGI Execution Error");
    return HttpUtils::sendErrorResponse(client_socket, 500, keep_alive, config);
  }
}