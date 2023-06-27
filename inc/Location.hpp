#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Webserv.hpp"

class Location {
 private:
  std::string path;
  std::string root;
  bool autoIndex;
  std::string index;
  std::vector<short> methods;
  std::string returns;
  std::string alias;
  std::vector<std::string> cgiPath;
  std::vector<std::string> cgiExt;
  unsigned long maxBodySize;

 public:
  std::map<std::string, std::string> extPath;

  Location();
  Location(const Location &copy);
  Location &operator=(const Location &copy);
  ~Location();

  void setPath(std::string param);
  void setRootLocation(std::string param);
  void setMethods(std::vector<std::string> methods);
  void setAutoindex(std::string param);
  void setIndexLocation(std::string param);
  void setReturn(std::string param);
  void setAlias(std::string param);
  void setCgiPath(std::vector<std::string> path);
  void setCgiExtension(std::vector<std::string> extension);
  void setMaxBodySize(std::string param);
  void setMaxBodySize(unsigned long param);

  const std::string &getPath() const;
  const std::string &getRootLocation() const;
  const std::vector<short> &getMethods() const;
  const bool &getAutoindex() const;
  const std::string &getIndexLocation() const;
  const std::string &getReturn() const;
  const std::string &getAlias() const;
  const std::vector<std::string> &getCgiPath() const;
  const std::vector<std::string> &getCgiExtension() const;
  const std::map<std::string, std::string> &getExtensionPath() const;
  const unsigned long &getMaxBodySize() const;
};

#endif
