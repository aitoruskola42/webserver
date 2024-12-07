/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger_formatting.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:27:04 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/21 11:48:38 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Logger.hpp"

/**
 * @brief Format a message for file output
 *
 * @param timestamp The timestamp of the log
 * @param levelStr The log level string
 * @param message The message to log
 * @param functionName The name of the function where the log is called
 * @param fileName The name of the file where the log is called
 * @return std::string The formatted message
 */
std::string Logger::formatFileMessage(const std::string &timestamp,
                                      const std::string &levelStr,
                                      const std::string &message,
                                      const char        *functionName,
                                      const char        *fileName) {
  std::ostringstream functionNameStream;
  functionNameStream << std::setw(FUNCTION_WIDTH) << std::internal << functionName << "()";

  std::string fileNameStr = fileName;
  size_t      pos         = fileNameStr.rfind("/");
  if (pos != std::string::npos) {
    fileNameStr = fileNameStr.substr(pos + 1);
  }
  std::ostringstream fileNameStream;
  fileNameStream << std::setw(FILE_NAME_WIDTH) << std::internal << fileNameStr;

  pid_t              pid = getpid();
  std::ostringstream pidStream;
  pidStream << std::setw(PID_WIDTH) << std::internal << pid;

  return timestamp + " │" + pidStream.str() + " │" + functionNameStream.str() + " │" + fileNameStream.str() + " │" +
         levelStr + " │" + message + "\n";
}

/**
 * @brief Format a message for console output
 *
 * @param level The log level
 * @param timestamp The timestamp of the log
 * @param levelStr The log level string
 * @param message The message to log
 * @return std::string The formatted message
 */
std::string Logger::formatConsoleMessage(LogLevel           level,
                                         const std::string &timestamp,
                                         const std::string &levelStr,
                                         const std::string &message) {
  std::string colorCode = getColorCode(level);
  return LBLU + timestamp + KNOR + " │" + colorCode + levelStr + KNOR + "│ " + message + "\n";
}

/**
 * @brief Print the header of the log file
 */
void Logger::printHeader() {
  m_file << "──────────────────────────────────────────────────────────────";
  m_file << "──────────────────────────────────────────────────────────────";
  m_file << "─────────────────────";
  m_file << std::endl;
  m_file << "      TIMESTAMP         |";
  m_file << centerString("PID", PID_WIDTH);
  m_file << " │ ";
  m_file << centerString("FUNCTION", FUNCTION_WIDTH);
  m_file << "  │ ";
  m_file << centerString("SRC FILE", FILE_NAME_WIDTH);
  m_file << "│  ";
  m_file << centerString("LOG LEVEL", LOG_LEVEL_WIDTH);
  m_file << "   │ DESCRIPTION";
  m_file << std::endl;
  m_file << "──────────────────────────────────────────────────────────────";
  m_file << "──────────────────────────────────────────────────────────────";
  m_file << "─────────────────────";
  m_file << std::endl;
}
