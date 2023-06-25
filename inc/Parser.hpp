#ifndef Parser_HPP
#define Parser_HPP

#include "Webserv.hpp"

class Server;

class Parser {
 private:
  std::vector<Server> serv;
  std::vector<std::string> _server_config;
  size_t numberOfServers;

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
  void createServer(std::string &config, Server &server);
  void checkServers();
  std::vector<Server> getServers();
  int stringCompare(std::string str1, std::string str2, size_t pos);

  int print();

 public:
  class ErrorException : public std::exception {
   private:
    std::string _message;

   public:
    ErrorException(std::string message) throw() {
      _message = "CONFIG PARSER ERROR: " + message;
    }
    virtual const char *what() const throw() {
      return (_message.c_str());
    }
    virtual ~ErrorException() throw() {}
  };
};

#endif