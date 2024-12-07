/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpGenerator.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/22 14:00:30 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 18:15:57 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_GENERATOR_HPP
#define HTTP_GENERATOR_HPP

#include <sys/stat.h>
#include <sstream>
#include <string>

class HttpGenerator {
 public:
  static std::string generate_HTMLHeader2(const std::string &request_path,
                                          bool               delete_allowed);
  static std::string generateEntryHtml(const std::string &dir_path,
                                       const std::string &request_path,
                                       const std::string &name,
                                       bool               is_dir,
                                       const std::string &location_path,
                                       bool               delete_allowed);
  static std::string getFileIconClass(const std::string &name, bool is_dir);
  static std::string generateLinkHtml(const std::string &request_path,
                                      const std::string &name);
  static std::string generateHref(const std::string &request_path,
                                  const std::string &name);
  static std::string generateFileInfoHtml(const std::string &full_path,
                                          bool               is_dir);
  static std::string generate_HTMLFooter();
};

#endif
