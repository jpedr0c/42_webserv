#ifndef Server_HPP
#define Server_HPP

#include "Webserv.hpp"

class Location;

class Server {
 private:
  uint16_t port;
  in_addr_t host;
  std::string serverName;
  std::string root;
  unsigned long maxBodySize;
  std::string index;
  bool autoIndex;
  std::map<short, std::string> errorPages;
  std::vector<Location> locations;
  struct sockaddr_in serverAddress;
  int listenFd;

 public:
  Server();
  ~Server();
  Server(const Server &other);
  Server &operator=(const Server &rhs);

  void initErrorPages(void);

  void setServerName(std::string server_name);
  void setHost(std::string parametr);
  void setRoot(std::string root);
  void setFd(int);
  void setPort(std::string parametr);
  void setClientMaxBodySize(std::string parametr);
  void setErrorPages(std::vector<std::string> &parametr);
  void setIndex(std::string index);
  void setLocation(std::string nameLocation, std::vector<std::string> parametr);
  void setAutoindex(std::string autoindex);
  static std::string statusCodeString(short);
  bool isValidHost(std::string host) const;
  bool isValidErrorPages();
  int isValidLocation(Location &location) const;

  const std::string &getServerName();
  const uint16_t &getPort();
  const in_addr_t &getHost();
  const size_t &getClientMaxBodySize();
  const std::vector<Location> &getLocations();
  const std::string &getRoot();
  const std::map<short, std::string> &getErrorPages();
  const std::string &getIndex();
  const bool &getAutoindex();
  const std::string &getPathErrorPage(short key);
  const std::vector<Location>::iterator getLocationKey(std::string key);

  void handleRootLocation(std::vector<std::string> &param, size_t &i, Location &newLocation);
  void handleAllowMethods(std::vector<std::string> &param, size_t &i, Location &newLocation, bool &flagMethods);
  void handleIndexLocation(std::vector<std::string> &param, size_t &i, Location &newLocation);
  void handleAutoindex(std::vector<std::string> &param, size_t &i, const std::string &path, Location &newLocation, bool &flagAutoIndex);
  void handleReturn(std::vector<std::string> &param, size_t &i, const std::string &path, Location &newLocation);
  void handleAlias(std::vector<std::string> &param, size_t &i, const std::string &path, Location &newLocation);
  void handleCgiExtension(std::vector<std::string> &param, size_t &i, Location &newLocation);
  void handleCgiPath(std::vector<std::string> &param, size_t &i, Location &newLocation);
  void handleMaxBodySize(std::vector<std::string> &param, size_t &i, Location &newLocation, bool &flagMaxSize);
  void handleLocationDefaults(Location &newLocation, bool flagMaxSize);
  void handleLocationValidation(int valid);

  int isValidRegularLocation(Location &location) const;
  int isValidCgiLocation(Location &location) const;
  static void checkToken(std::string &parametr);
  bool checkLocation() const;

  void setupServer();
  int getFd();
};

#endif