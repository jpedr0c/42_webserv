#ifndef Parser_HPP
#define Parser_HPP

#include "Webserv.hpp"

class Server;

class Parser {
 public:
  Parser();
  ~Parser();
  std::vector<Server> getServers();
  const std::string removeComments(std::string &string);
  const std::string removeSpaces(std::string &string);
  size_t findStartServer(size_t start, std::string &content);
  size_t findEndServer(size_t start, std::string &content);
  void splitServers(std::string &content);
  const std::string extractServersConfig(std::string &fileContent);
  bool areServersDuplicate(Server &currentServer, Server &nextServer);
  void checkDuplicateServerConfigurations();
  std::vector<std::string> splitParameters(std::string inputStr, std::string delimeter);
  void validateServerParametersSize(const std::vector<std::string> &parameters);
  void validateDuplicatePort(Server &server);
  void validateServerScopeCharacter(const std::string &parameter);
  std::vector<std::string> parseLocationCodes(const std::vector<std::string> &parameters, size_t &i);
  void validateClosingBracket(const std::vector<std::string> &parameter, size_t &i);
  void validateDuplicateHost(Server &server);
  void validateDuplicateRoot(Server &server);
  void processErrorCodes(const std::vector<std::string> &parameters, size_t &i, std::vector<std::string> &errorCodes);
  void validateDuplicateMaxBodySize(bool isMaxSizeSet);
  void validateDuplicateServerName(Server &server);
  void validateDuplicateIndex(Server &server);
  void validateDuplicateAutoindex(bool flag_autoindex);
  void setDefaultServerValues(Server &server);
  void performServerValidations(Server &server);
  void createServerFromConfig(std::string &config, Server &server);
  int parseServerConfigFile(const std::string &config_file);

 private:
  std::vector<Server> serverList;
  std::vector<std::string> serverConfig;
  size_t numberOfServers;
};

#endif