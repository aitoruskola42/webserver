// CommonDefinitions.hpp
#ifndef COMMON_DEFINITIONS_HPP
#define COMMON_DEFINITIONS_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>

#define AJXWEBSERVER_VERSION "1.1.1"

// -----------------------------------------------------------------------------
// SocketResult
// -----------------------------------------------------------------------------
enum SocketResult
{
    SOCKET_OK,
    SOCKET_CLOSED,
    SOCKET_ERROR,
    SOCKET_WOULD_BLOCK
};

// -----------------------------------------------------------------------------
// HTTP Error Codes
// -----------------------------------------------------------------------------
enum e_http_errorcodes {
  BAD_REQUEST                = 400,
  UNAUTHORIZED               = 401,
  FORBIDDEN                  = 403,
  NOT_FOUND                  = 404,
  METHOD_NOT_ALLOWED         = 405,
  REQUEST_TIMEOUT            = 408,
  LENGTH_REQUIRED            = 411,
  PAYLOAD_TOO_LONG           = 413,
  URI_TOO_LONG               = 414,
  UNSUPPORTED_MEDIA_TYPE     = 415,
  HEADER_FILES_TOO_LARGE     = 431,
  INTERNAL_SERVER_ERROR      = 500,
  METHOD_NOT_IMPLEMENTED     = 501,
  BAD_GATEWAY                = 502,
  SERVICE_UNAVAILABLE        = 503,
  GATEWAY_TIMEOUT            = 504,
  HTTP_VERSION_NOT_SUPPORTED = 505
};

// -----------------------------------------------------------------------------
//  LOCAL CONFIG
// -----------------------------------------------------------------------------

struct LocationConfig {
    bool                                autoindex;
    unsigned int                        client_max_body_size;
    std::string                         location_path;  
    std::string                         root_path;
    std::string                         upload_path;
    std::vector<std::string>            allowed_methods;
    std::vector<std::string>            index_files;
    std::map<int, std::string>          error_pages;
    std::map<short int, std::string>    return_code_path;
    std::map<std::string, std::string>  cgi_extensions;
    std::map<std::string, std::string>  redirects;
};

struct FileState {
  std::ifstream *file;
  size_t         file_size;
  size_t         bytes_sent;
  bool           headers_sent;
  char           buffer[4096];
  size_t         buffer_pos;
  size_t         buffer_len;
  bool           last_chunk_sent;
  std::string    filename;

  FileState() : file(NULL), file_size(0), bytes_sent(0), headers_sent(false), buffer_pos(0), buffer_len(0), last_chunk_sent(false) {}

  ~FileState() {
    if (file) {
      file->close();
      delete file;
    }
  }

private:
  // Prohibir la copia
  FileState(const FileState &);
  FileState &operator=(const FileState &);
};

struct ClientInfo {
  int            socket;
  int            id;
  int            port;
  bool           ready_for_read;
  bool           ready_for_write;
  bool           waiting_to_write;
  std::string    pending_response;
  std::string    partial_request;
  bool           request_complete;
  size_t         content_length;
  size_t         bytes_received;
  time_t         last_activity;

  ClientInfo(int s, int i, int p)
      : socket(s)
      , id(i)
      , port(p)
      , ready_for_read(false)
      , ready_for_write(false)
      , waiting_to_write(false)
      , request_complete(false)
      , content_length(0)
      , bytes_received(0)
      , last_activity(time(NULL)) {}
};

// -----------------------------------------------------------------------------
// LOG FORMAT CONFIG
// -----------------------------------------------------------------------------
// Default log file name
#define LOG_FILE "log.txt"
// Width of the function column
#define FUNCTION_WIDTH 26
// Width of the file name column
#define FILE_NAME_WIDTH 30
// Width of the PID column
#define PID_WIDTH 8
// Width of the log level column
#define LOG_LEVEL_WIDTH 10

// -----------------------------------------------------------------------------
// COLORS CONFIG
// -----------------------------------------------------------------------------
// COLORS
#define KRED "\033[0;31m"
#define KGRE "\033[0;32m"
#define KYEL "\033[0;33m"
#define LBLU "\033[0;34m"
#define KPUR "\033[0;35m"
#define KNOR "\033[0;37m"
#define KCYA "\033[0;36m"

#define KBLA "\033[0;30m"
#define KBLU "\x1B[34m"
#define SUBR "\033[0;31m\033[4m"
#define NEGR "\033[0;31m\033[1m"

// Colores adicionales
#define KORA "\033[0;33m"
#define KPIN "\033[0;35m"
#define KGRA "\033[0;90m"
#define KBRO "\033[0;33m"
#define KLIM "\033[0;92m"

// Colores brillantes
#define KBRED "\033[1;31m"
#define KBGRE "\033[1;32m"
#define KBYEL "\033[1;33m"
#define KBBLU "\033[1;34m"
#define KBPUR "\033[1;35m"
#define KBCYA "\033[1;36m"
#define KBWHI "\033[1;37m"

// Colores de fondo
#define BGRED "\033[41m"
#define BGGRE "\033[42m"
#define BGYEL "\033[43m"
#define BGBLU "\033[44m"
#define BGPUR "\033[45m"
#define BGCYA "\033[46m"
#define BGWHI "\033[47m"

#define MAX_URI_LENGTH  512 //32768 //32 KB como nginx ( 4 buffers de 8kb si es large uri)

#endif // COMMON_DEFINITIONS_HPP