/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpGenerator.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/22 14:09:18 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/03 20:05:07 by xamayuel         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "HttpGenerator.hpp"
#include "Logger/includes/Logger.hpp"
#include "WebServer/RequestHandler/GetHandler/GetHandler.hpp"
#include "CommonDefinitions.hpp"
std::string HttpGenerator::generate_HTMLHeader2(const std::string &request_path, bool delete_allowed) {
  LOG_DEBUG("Delete allowed: " << delete_allowed);
  std::string header =
      "<html>\n"
      "<head>\n"
      "    <title>Directory Explorer</title>\n"
      "    <link rel=\"stylesheet\" "
      "href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.3/css/"
      "all.min.css\">\n"
      "    <style>\n"
      "        body { font-family: Arial, sans-serif; margin: 0; padding: 20px "
      "20px 100px; background-color: #f0f0f0; }\n"
      "        h1 { color: #333; border-bottom: 2px solid #333; "
      "padding-bottom: 10px; }\n"
      "        table { width: 100%; border-collapse: collapse; "
      "background-color: white; margin-bottom: 20px; }\n"
      "        th, td { border: 1px solid #ddd; padding: 8px; text-align: "
      "left; }\n"
      "        th { background-color: #f2f2f2; }\n"
      "        a { color: #0066cc; text-decoration: none; }\n"
      "        a:hover { text-decoration: underline; }\n"
      "        footer { position: fixed; left: 0; bottom: 0; width: 100%; "
      "background-color: #333; color: #fff; text-align: center; font-size: "
      "14px; padding: 10px 0; }\n"
      " .small-text { font-size: 0.8em; }\n"
      "        .icon-column { width: 40px; text-align: center; }\n" // Nueva clase para la columna de iconos
      "        .icon-column i { font-size: 24px; }\n"               // Aumentar el tamaño de los iconos
      "    </style>\n"
      "</head>\n"
      "<body>\n"
      "    <h1>Directory contents: ";
  header += request_path + "</h1>\n";
  header +=
      "    <table>\n"
      "        <tr>\n"
      "            <th class=\"icon-column\"></th>\n"
      "            <th>Name</th>\n"
      "            <th>Type</th>\n"
      "            <th>Size <br><span "
      "class=\"smalll-text\">[bytes]</span></th>\n"
      "            <th>Last Modified <br><span "
      "class=\"small-text\">[YYYY-MM-DD HH:MM:SS]</span></th>\n";

  if (delete_allowed) {
    header += "            <th></th>\n";
  }
  header += "        </tr>\n";
  return header;
}

std::string HttpGenerator::generateEntryHtml(const std::string &dir_path,
                                             const std::string &request_path,
                                             const std::string &name,
                                             bool               is_dir,
                                             const std::string &location_path,
                                             bool               delete_allowed) {
  if (request_path == location_path) {
    if (request_path.substr(request_path.size() - location_path.size()) == location_path) {
      if (name == "..") {
        return "";
      }
      LOG_DEBUG("Location path: " + location_path);
      LOG_DEBUG("Name: " + name);
      LOG_DEBUG("Dir path: " + dir_path);
      LOG_DEBUG("Request path: " + request_path);
    }
  }

  std::string entry = "<tr>";

  // Añadir icono
  std::string icon_class = getFileIconClass(name, is_dir);
  entry += "<td class='icon-column'><i class='" + icon_class + "'></i></td>";

  // Generar el enlace
  entry += generateLinkHtml(request_path, name);

  // tipo de archivo
  entry += "<td>" + std::string(is_dir ? "Directory" : "File") + "</td>";

  // Tamaño y fecha de modificación
  if (name != "..") {
    entry += generateFileInfoHtml(dir_path + "/" + name, is_dir);
  } else {
    entry += "<td>-</td><td>-</td>";
  }
  if (delete_allowed && name != "..") {
    std::string deletefile = generateHref(request_path, name);
    entry += "<td><button onclick=\"deleteFile('" + deletefile +
             "')\" style=\"background: none; border: none; cursor: pointer;\"><i "
             "class=\"fas fa-trash-alt\" style=\"color: red;\"></i></button></td>";
  }

  return entry + "</tr>\n";
}

std::string HttpGenerator::getFileIconClass(const std::string &name, bool is_dir) {
  if (is_dir) {
    return "fas fa-folder";
  }

  size_t dot_pos = name.find_last_of('.');
  if (dot_pos != std::string::npos) {
    std::string ext = name.substr(dot_pos + 1);
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "gif") {
      return "fas fa-image";
    } else if (ext == "html" || ext == "htm") {
      return "fab fa-html5";
    } else if (ext == "txt") {
      return "fas fa-file-alt";
    } else if (ext == "pdf") {
      return "fas fa-file-pdf";
    }
  }
  return "fas fa-file"; // Icono por defecto para archivos desconocidos
}

std::string HttpGenerator::generateLinkHtml(const std::string &request_path, const std::string &name) {
  std::string href         = generateHref(request_path, name);
  std::string display_name = (name == "..") ? "Parent Directory" : name;

  return "<td><a href=\"" + href + "\">" + display_name + "</a></td>";
}

std::string HttpGenerator::generateHref(const std::string &request_path, const std::string &name) {
  if (name == "..") {
    size_t last_slash = request_path.find_last_of('/');
    return (last_slash != std::string::npos && last_slash > 0) ? request_path.substr(0, last_slash) : "/";
  }

  std::string href = (request_path[request_path.length() - 1] != '/') ? request_path + "/" + name : request_path + name;
  return href;
}

std::string HttpGenerator::generateFileInfoHtml(const std::string &full_path, bool is_dir) {
  struct stat st;
  if (stat(full_path.c_str(), &st) != 0) {
    return "<td>-</td><td>-</td>";
  }

  std::string size_html;
  if (is_dir) {
    size_html = "-";
  } else {
    std::ostringstream oss;
    oss << st.st_size;
    size_html = oss.str();
  }

  char date_buf[64];
  strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));

  return "<td>" + size_html + "</td><td>" + std::string(date_buf) + "</td>";
}

std::string HttpGenerator::generate_HTMLFooter() {
  std::string footer =
      "</table>\n"
      "<footer>\n"
      "    <p style=\"margin: 0;\">Powered by AJXWebserver &copy; 2024</p>\n"
      "    <p style=\"margin: 5px 0;\">\n"
      "        <a href=\"https://github.com/unamayu/ft_webserver\" "
      "style=\"color: #4CAF50; text-decoration: none;\">GitHub</a> | \n"
      "        <a href=\"mailto:xamayuel@proton.me\" style=\"color: #4CAF50; "
      "text-decoration: none;\">Support</a>\n"
      "    </p>\n"
      "      <p style=\"font-size: 12px; margin: 5px 0 0 0;\">Version " +
      std::string(AJXWEBSERVER_VERSION) +
      " | "
      "Built with <span style=\"color: #e25555; font-size: 16px; "
      "display: inline-block; transform: scale(1.5, 1.3); margin: 0 "
      "3px;\">&hearts;</span> "
      "by auskola- / javigarc / xamayuel</p>\n"
      "</footer>\n"
      "</body>\n"
      "</html>";
  return footer;
}