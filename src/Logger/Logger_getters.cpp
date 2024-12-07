/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger_getters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:26:23 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/21 11:44:02 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Logger.hpp"

/**
 * @brief Get the color code for a log level
 *
 * @param level The log level
 * @return std::string The color code
 */
std::string Logger::getColorCode(LogLevel level) {
  switch (level) {
    case DEBUG:
      return LBLU;
    case INFO:
      return KNOR;
    case WARNING:
      return KYEL;
    case ERROR:
      return KRED;
    case SUCCESS:
      return KBGRE;
    case CRITICAL:
      return KPUR;
    default:
      return KNOR;
  }
}

/**
 * @brief Get the string representation of a log level
 *
 * @param level The log level
 * @return std::string The string representation of the log level
 */
std::string Logger::getLevelString(LogLevel level) {
  switch (level) {
    case DEBUG:
      return "   [DEBUG]    ";
    case INFO:
      return "   [INFO]     ";
    case WARNING:
      return "   [WARNING]  ";
    case SUCCESS:
      return "   [SUCCESS]  ";
    case ERROR:
      return "   [ERROR]    ";
    case CRITICAL:
      return "   [CRITICAL] ";
    default:
      return "   UNKNOWN    ";
  }
}

/**
 * @brief Get the current time as a string with milliseconds
 *
 * @return std::string The current time as a string
 */
std::string Logger::getCurrentTime() {
  // Get current time with microsecond precision
  struct timeval tv;
  gettimeofday(&tv, NULL);

  // Convert seconds to local time structure
  time_t    now = tv.tv_sec;
  struct tm tstruct;
  localtime_r(&now, &tstruct);

  // Format date and time
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);

  // Add milliseconds to the formatted time
  char currentTimeWithMs[84];
  snprintf(currentTimeWithMs, sizeof(currentTimeWithMs), "%s.%03ld", buf, tv.tv_usec / 1000);

  return std::string(currentTimeWithMs);
}