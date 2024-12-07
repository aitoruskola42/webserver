/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpUtils_fileUtils.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 14:25:56 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 17:44:14 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpUtils.hpp"

/**
 * @brief Checks if the provided path is a directory.
 *
 * This function uses the `stat` structure to determine if the specified
 * path corresponds to a directory in the file system.
 *
 * @param path The path of the file or directory to check.
 * @return true If the path is a directory.
 * @return false If the path is not a directory or if an error occurs
 * while trying to access the path.
 */
bool HttpUtils::isDirectory(const std::string &path) {
  struct stat statbuf;
  if (stat(path.c_str(), &statbuf) != 0) {
    return false;
  }
  return S_ISDIR(statbuf.st_mode);
}
/**
 * @brief Checks if a file exists.
 *
 * This function uses the `ifstream` class to check if the specified file
 * exists in the file system.
 *
 * @param filename The name of the file to check.
 * @return true If the file exists.
 * @return false If the file does not exist or if an error occurs while
 * trying to access the file.
 */
bool HttpUtils::fileExists(const std::string &filename) {
  std::ifstream file(filename.c_str());
  return file.good();
}
