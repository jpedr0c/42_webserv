#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Webserv.hpp"

class ServerConfig;

class ConfigParser {
 private:
  std::vector<ServerConfig> _servers;
  std::vector<std::string> _server_config;
  size_t numberOfServers;

 public:
  ConfigParser();
  ~ConfigParser();

  int createCluster(const std::string &config_file);

  void splitServers(std::string &content);
  const std::string removeComments(std::string &string);
  const std::string removeSpaces(std::string &string);
  const std::string extractServersConfig(std::string &fileContent);
  size_t findStartServer(size_t start, std::string &content);
  bool areServersDuplicate(ServerConfig &currentServer, ServerConfig &nextServer);
  size_t findEndServer(size_t start, std::string &content);
  void createServer(std::string &config, ServerConfig &server);
  void checkServers();
  std::vector<ServerConfig> getServers();
  int stringCompare(std::string str1, std::string str2, size_t pos);

  int print();
};

#endif