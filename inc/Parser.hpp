#ifndef Parser_HPP
#define Parser_HPP

#include "Webserv.hpp"

class Server;

class Parser {
 public:
  Parser();
  ~Parser();

  int createCluster(const std::string &config_file);

  void splitServers(std::string &content);
  const std::string removeComments(std::string &string);
  const std::string removeSpaces(std::string &string);
  const std::string extractServersConfig(std::string &fileContent);
  size_t findStartServer(size_t start, std::string &content);
  size_t findEndServer(size_t start, std::string &content);
  bool areServersDuplicate(Server &currentServer, Server &nextServer);
  void createServer(std::string &config, Server &server);
  void checkServers();
  std::vector<Server> getServers();
  int stringCompare(std::string str1, std::string str2, size_t pos);

  void validateServerParametersSize(const std::vector<std::string> &parameters);
  void validateDuplicatePort(Server &server);

 private:
  std::vector<Server> serverList;
  std::vector<std::string> serverConfig;
  size_t numberOfServers;
};

#endif