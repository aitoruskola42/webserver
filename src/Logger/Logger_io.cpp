/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger_io.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:30:24 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/21 11:44:12 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Logger.hpp"

/**
 * @brief Output a message to the console
 *
 * @param level The log level
 * @param message The message to log
 */
void Logger::outputToConsole(LogLevel level, const std::string &message) {
  if (level >= DEBUG && level < ERROR)
    std::cout << message;
  else if (level >= ERROR)
    std::cerr << message;
}

/**
 * @brief Write a message to the file
 *
 * @param message The message to log
 */
void Logger::writeToFile(const std::string &message) {
  m_file << message;
  m_file.flush();
}