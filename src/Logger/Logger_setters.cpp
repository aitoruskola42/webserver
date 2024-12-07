/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger_setters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:49:54 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/21 11:50:49 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Logger.hpp"

/**
 * @brief Attempts to open a new log file. If it fails, it keeps the current
 * file.
 *
 * @param filename The name of the new log file
 */
void Logger::setLogFile(const std::string &filename) {
  if (m_filename != filename) {
    std::ofstream newFile(filename.c_str(), std::ios::in | std::ios::out | std::ios::app);
    if (!newFile.is_open()) {
      std::ostringstream logMessage;
      logMessage << "The log file could not be opened: " << filename << ". Keeping the current file: " << m_filename;
      log(WARNING, logMessage, __FUNCTION__, __FILE__);
      return;
    }

    closeCurrentFile();
    m_filename = filename;
    m_file.close();
    m_file.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::app);

    if (m_file.tellp() == 0) {
      printHeader();
    }

    std::ostringstream logMessage;
    logMessage << "Log file changed to: " << m_filename;
    log(SUCCESS, logMessage, __FUNCTION__, __FILE__);
  }
}

/**
 * @brief Sets the log level of the Logger
 *
 * @param level The log level to set
 *
 * This function sets the log level of the Logger. Messages with a level lower than
 * the set level will not be logged. The available log levels are:
 * - DEBUG: Detailed debug messages
 * - INFO: General informational messages
 * - WARNING: Warning messages
 * - ERROR: Error messages
 * - SUCCESS: Success messages
 * - CRITICAL: Critical messages
 *
 * After setting the log level, a success message is logged indicating the new level
 * that has been set.
 */
void Logger::setLogLevel(LogLevel level) {
  m_level = level;
}

/**
 * @brief Activa o desactiva la escritura en el archivo de log
 *
 * @param enable True para activar la escritura, False para desactivarla
 */
void Logger::setFileLoggingEnabled(bool enable) {
  m_fileLoggingEnabled = enable;
  std::ostringstream logMessage;
  logMessage << "Log file logging  " << (enable ? "enabled" : "disabled");
  log(INFO, logMessage, __FUNCTION__, __FILE__);
}