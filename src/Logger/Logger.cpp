/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 19:19:08 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/21 11:37:55 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Logger.hpp"

/**
 * @brief Log a message with a specific log level
 *
 * @param level The log level
 * @param messageStream The message to log
 * @param functionName The name of the function where the log is called
 * @param fileName The name of the file where the log is called
 */
void Logger::log(LogLevel level, const std::ostringstream &messageStream, const char *functionName, const char *fileName) {
  std::string message   = messageStream.str();
  std::string levelStr  = getLevelString(level);
  std::string timestamp = getCurrentTime();

  if (level >= m_level) {
    if (m_fileLoggingEnabled) {
      // Format file output
      std::string fileMessage = formatFileMessage(timestamp, levelStr, message, functionName, fileName);

      writeToFile(fileMessage);
    }
    // Format console output
    std::string consoleMessage = formatConsoleMessage(level, timestamp, levelStr, message);

    // Output to console based on log level
    outputToConsole(level, consoleMessage);
  }
}

/**
 * @brief Get the singleton instance of the Logger
 *
 * @return Logger& Reference to the Logger instance
 */
Logger &Logger::getInstance() {
  static Logger instance;
  return instance;
}
