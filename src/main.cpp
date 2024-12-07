#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "ConfigFileParse/ConfigurationManager.hpp"
#include "Logger/includes/Logger.hpp"
#include "WebServer/WebServer.hpp"

void setupLogger(ConfigurationManager &configuration, int argc, char *argv[]);
void runWebServer(ConfigurationManager &configuration);
int  main(int argc, char *argv[]) {
   if (argc < 2) {
     std::cerr << KRED "Usage: " << argv[0] << " <filename>" << std::endl;
     return 1;
  }
   std::string filename = argv[1];
   if (filename.substr(filename.find_last_of(".") + 1) != "conf") {
       LOG_CRITICAL("File " << filename << " must have a .conf extension.");
       return 1;
   }

   std::ifstream file(filename.c_str());
   if (!file.is_open()) {
       LOG_CRITICAL("File " << filename << " not found.");
       return 1;
   }
   Logger::getInstance().printLogo();

   ConfigurationManager configuration;

   if (!configuration.parseFile(argv[1])) {
     LOG_CRITICAL("Error parsing configuration file: " << argv[1]);
     return 1;
  }

   setupLogger(configuration, argc, argv);
   std::cout << configuration;

   try {
     runWebServer(configuration);
  } catch (const std::exception &e) {
     LOG_CRITICAL(e.what());
     return 1;
  }

   return 0;
}

/**
 * @brief Setup the logger for the server.
 *
 * @param configuration The configuration parser.
 * @param argc The number of arguments.
 * @param argv The arguments.
 */
void setupLogger(ConfigurationManager &configuration, int argc, char *argv[]) {
  Logger::getInstance().setLogFile(configuration.get_debug_file());
  std::string level = configuration.get_log_level();

  if (level == "DEBUG") {
    Logger::getInstance().setLogLevel(Logger::DEBUG);
  } else if (level == "INFO") {
    Logger::getInstance().setLogLevel(Logger::INFO);
  } else if (level == "WARNING") {
    Logger::getInstance().setLogLevel(Logger::WARNING);
  } else if (level == "ERROR") {
    Logger::getInstance().setLogLevel(Logger::ERROR);
  } else if (level == "CRITICAL") {
    Logger::getInstance().setLogLevel(Logger::CRITICAL);
  } else {
    Logger::getInstance().setLogLevel(Logger::INFO); // Default level
  }

  if (argc == 3 && std::string(argv[2]) == "--logfile") {
    LOG_SUCCESS("Enabling file logging");
    Logger::getInstance().setFileLoggingEnabled(true);
  } else {
    Logger::getInstance().setFileLoggingEnabled(false);
  }
}

/**
 * @brief Run the web server.
 *
 * @param configuration The configuration parser.
 */
void runWebServer(ConfigurationManager &configuration) {
  WebServer           server(configuration);
  std::vector<Server> servers = configuration.get_servers();

  for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
    std::ostringstream logMsg;

    if (it->getType() == 0) {
      server.addPort(it->getListen(), it->getIp().c_str());
    }
  }
  server.run();
}