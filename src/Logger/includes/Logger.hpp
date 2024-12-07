/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 19:19:20 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/27 12:01:25 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_H
#define LOGGER_H

// CONFIG
#include "CommonDefinitions.hpp"
#include "Logger_config.hpp"
// INCLUDES
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// Logger Class: Implements a logging system for the application
//
// This class follows the Singleton pattern to ensure a single global instance.
// It allows logging messages with different severity levels, both to the
// console and to a log file.
//
// Main features:
// - Log levels: DEBUG, INFO, WARNING, ERROR, SUCCESS, CRITICAL
// - Formatted output with timestamp, level, function name, and file name
// - Ability to change the log file at runtime
// - Colorful messages in console output for better visibility
//
// Typical usage:
//   Logger::getInstance().log(Logger::INFO, "Information message",
//   __FUNCTION__, __FILE__);
//
// Note: This class is not thread-safe. For use in multi-threaded environments,
// it is recommended to implement additional synchronization mechanisms.

class Logger {
 public:
  enum LogLevel { DEBUG, INFO, WARNING, SUCCESS, ERROR, CRITICAL };

 private:
  std::string        m_filename;
  std::ofstream      m_file;
  LogLevel           m_level;
  bool               m_fileLoggingEnabled;
  static std::string centerString(const std::string &str, int width) {
    int padding  = width - str.length();
    int padLeft  = padding / 2;
    int padRight = padding - padLeft;
    return std::string(padLeft, ' ') + str + std::string(padRight, ' ');
  }

  // CONSTRUCTOR, ASSIGN AND DESTRUCTOR
 private:
  Logger();
  ~Logger();
  Logger(const Logger &);
  Logger &operator=(const Logger &);

 private:
  std::string getLevelString(LogLevel level);
  std::string getCurrentTime();
  std::string getColorCode(LogLevel level);
  // OUTPUT TO CONSOLE AND FILE
  void outputToConsole(LogLevel level, const std::string &consoleMessage);
  void outputToFile(LogLevel level, const std::string &fileMessage);
  void writeToFile(const std::string &message);
  // FORMAT MESSAGES
  void        printHeader();
  std::string formatFileMessage(const std::string &timestamp,
                                const std::string &levelStr,
                                const std::string &message,
                                const char        *functionName,
                                const char        *fileName);
  std::string formatConsoleMessage(LogLevel           level,
                                   const std::string &timestamp,
                                   const std::string &levelStr,
                                   const std::string &message);
  // CLOSE AND OPEN FILES
  void closeCurrentFile();
  int  openNewFile();
  // PUBLIC METHODS
 public:
  static Logger &getInstance();
  void           log(LogLevel                  level,
                     const std::ostringstream &messageStream,
                     const char               *functionName,
                     const char               *fileName);
  void           setLogFile(const std::string &filename);
  void           setLogLevel(LogLevel level);
  void           setFileLoggingEnabled(bool enable);
  void           printLogo();
};

// MACROS
#define LOG_DEBUG(message)                                                 \
  {                                                                        \
    std::ostringstream oss;                                                \
    oss << message;                                                        \
    Logger::getInstance().log(Logger::DEBUG, oss, __FUNCTION__, __FILE__); \
  }
#define LOG_INFO(message)                                                 \
  {                                                                       \
    std::ostringstream oss;                                               \
    oss << message;                                                       \
    Logger::getInstance().log(Logger::INFO, oss, __FUNCTION__, __FILE__); \
  }
#define LOG_WARNING(message)                                                 \
  {                                                                          \
    std::ostringstream oss;                                                  \
    oss << message;                                                          \
    Logger::getInstance().log(Logger::WARNING, oss, __FUNCTION__, __FILE__); \
  }
#define LOG_ERROR(message)                                                 \
  {                                                                        \
    std::ostringstream oss;                                                \
    oss << message;                                                        \
    Logger::getInstance().log(Logger::ERROR, oss, __FUNCTION__, __FILE__); \
  }
#define LOG_CRITICAL(message)                                                 \
  {                                                                           \
    std::ostringstream oss;                                                   \
    oss << message;                                                           \
    Logger::getInstance().log(Logger::CRITICAL, oss, __FUNCTION__, __FILE__); \
  }
#define LOG_SUCCESS(message)                                                 \
  {                                                                          \
    std::ostringstream oss;                                                  \
    oss << message;                                                          \
    Logger::getInstance().log(Logger::SUCCESS, oss, __FUNCTION__, __FILE__); \
  }
#endif // LOGGER_H