/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger_privates.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 19:19:08 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/21 11:48:18 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Logger.hpp"

/**
 * @brief Construct a new Logger object
 *
 * @param filename The name of the log file
 * @throws std::runtime_error if the log file cannot be opened
 */
Logger::Logger() {
  m_level = WARNING;
}

/**
 * @brief Destroy the Logger object
 *
 */
Logger::~Logger() {
  if (m_file.is_open()) {
    std::ostringstream logMessage;
    logMessage << "Closing log file: " << m_filename;
    log(DEBUG, logMessage, __FUNCTION__, __FILE__);
    m_file.close();
  }
}

/**
 * @brief Attempts to open a new log file. If it fails, it keeps the current
 * file.
 *
 * @param filename The name of the new log file
 */
int Logger::openNewFile() {
  m_file.open(m_filename.c_str(), std::ios::in | std::ios::out | std::ios::app);
  if (!m_file.is_open()) {
    std::cerr << "Error: No se pudo abrir el archivo de log: " << m_filename << std::endl;
    return 1;
  }

  m_file.seekp(0, std::ios::end);
  if (m_file.tellp() == 0) {
    m_file << " ----------------------------------------------------------------" << std::endl;
    m_file << "      TIMESTAMP         |";
    m_file << std::setw(PID_WIDTH) << "PID";
    m_file << "│ ";
    m_file << centerString("FUNCTION2", FUNCTION_WIDTH);
    m_file << " │ ";
    m_file << std::setw(FILE_NAME_WIDTH) << "SRC FILE";
    m_file << "│";
    m_file << std::setw(LOG_LEVEL_WIDTH) << "LOG LEVEL";
    m_file << "│ DESCRIPTION";
    m_file << std::endl;
  }
  m_file.seekp(0, std::ios::end);
  return 0;
}

/**
 * @brief Close the current log file
 */
void Logger::closeCurrentFile() {
  if (m_file.is_open()) {
    m_file.close();
  }
}
