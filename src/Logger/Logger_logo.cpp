/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger_logo.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:44:20 by xamayuel          #+#    #+#             */
/*   Updated: 2024/08/27 12:02:07 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Logger.hpp"

/**
 * @brief Static class for printing the web server logo.
 *
 * This class provides a static method to print a colorful ASCII logo
 * representing the web server. The logo includes information
 * about the authors and the current process PID.
 */

void Logger::printLogo() {
  std::cout << KRED;
  std::cout << "                          (            (              " KNOR;
  std::cout << std::endl;
  std::cout << KRED << "   (  (              (    )\\ )         )\\ )   " KNOR << std::endl;
  std::cout << KRED;
  std::cout << "   )\\))(   ' (     ( )\\  (()/(   (    (()/(   (   ( ";
  std::cout << KGRE;
  std::cout << "            Authors: " << std::endl;
  std::cout << KRED;
  std::cout << "  ((_)()\\ )  )\\    )((_)  /(_))  )\\    /(_))  )\\  )\\  ";
  std::cout << KPUR;
  std::cout << "          Aitor Uskola Mendieta ";
  std::cout << std::endl;
  std::cout << LBLU;
  std::cout << "  _" KRED "(())\\" LBLU "_" KRED ")()((" LBLU "_" KRED;
  std::cout << ")  ((_)_  (_))   ((_)  (_))   ((_)((_) " KPUR;
  std::cout << "          Javier García-Arango Vázquez-Prada" KNOR << std::endl;
  std::cout << LBLU;
  std::cout << "  \\ \\" KRED "((_)" LBLU;
  std::cout << "/ /| __|  | _ ) / __|  | __| | _ \\  \\ \\ / /  " KPUR;
  std::cout << "          Xabier Unai Amayuelas Urruticoechea " KNOR;
  std::cout << std::endl;
  std::cout << LBLU;
  std::cout << "   \\ \\/\\/ / | _|   | _ \\ \\__ \\  | _|  |   /   \\ V /  ";
  std::cout << KNOR;
  std::cout << std::endl;
  std::cout << LBLU;
  std::cout << "    \\_/\\_/  |___|  |___/ |___/  |___| |_|_\\    \\_/  ";
  std::cout << KCYA;
  std::cout << "            PID:";
  std::cout << KNOR;
  std::cout << getpid();
  std::cout << KCYA;
  std::cout << "            VERSION: " KNOR << AJXWEBSERVER_VERSION << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "      TIMESTAMP         |   TYPE       |  Log";
  std::cout << std::endl;
  std::cout << "────────────────────────┼──────────────┼──────────────────";
  std::cout << "─────────────────────────────────────────────────";
  std::cout << std::endl;
}
