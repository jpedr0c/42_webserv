#include "../inc/Server.hpp"

Server::Server() {
  this->port = 0;
  this->host = 0;
  this->serverName = "";
  this->root = "";
  this->maxBodySize = MAX_CONTENT_LENGTH;
  this->index = "";
  this->listenFd = 0;
  this->autoIndex = false;
  this->initErrorPages();
}

Server::~Server() {}

Server::Server(const Server& copy) {
  this->serverName = copy.serverName;
  this->root = copy.root;
  this->host = copy.host;
  this->port = copy.port;
  this->maxBodySize = copy.maxBodySize;
  this->index = copy.index;
  this->errorPages = copy.errorPages;
  this->locations = copy.locations;
  this->listenFd = copy.listenFd;
  this->autoIndex = copy.autoIndex;
  this->serverAddress = copy.serverAddress;
}

Server& Server::operator=(const Server& copy) {
  this->serverName = copy.serverName;
  this->root = copy.root;
  this->port = copy.port;
  this->host = copy.host;
  this->maxBodySize = copy.maxBodySize;
  this->index = copy.index;
  this->errorPages = copy.errorPages;
  this->locations = copy.locations;
  this->listenFd = copy.listenFd;
  this->autoIndex = copy.autoIndex;
  this->serverAddress = copy.serverAddress;
  return (*this);
}

void Server::initErrorPages(void) {
  errorPages[301] = "";
  errorPages[302] = "";
  errorPages[400] = "";
  errorPages[401] = "";
  errorPages[402] = "";
  errorPages[403] = "";
  errorPages[404] = "";
  errorPages[405] = "";
  errorPages[406] = "";
  errorPages[500] = "";
  errorPages[501] = "";
  errorPages[502] = "";
  errorPages[503] = "";
  errorPages[505] = "";
  errorPages[505] = "";
}

void Server::setServerName(std::string serverName) {
  checkToken(serverName);
  this->serverName = serverName;
}

void Server::setHost(std::string param) {
  checkToken(param);
  if (param == "localhost")
    param = "127.0.0.1";
  if (isValidHost(param))
    this->host = inet_addr(param.data());
  else
    throw Error("Invalid host syntax. Please provide a valid host address");
}

void Server::setRoot(std::string root) {
  checkToken(root);
  char dir[1024];
  getcwd(dir, 1024);
  if (ConfigFile::getTypePath(root) != 2)
    ;
  else {
    this->root = root;
    return;
  }
  std::string full_root = dir + root;
  if (ConfigFile::getTypePath(full_root) == 2)
    this->root = full_root;
  else
    throw Error("Invalid root directory. Please provide a valid directory path");
}

void Server::setPort(std::string param) {
  unsigned int port = 0;
  int isDigits = 0;

  checkToken(param);
  for (size_t i = 0; i < param.length(); i++) {
    if (std::isdigit(param[i]))
      isDigits = 1;
    else
      throw Error("Invalid port syntax. The port should be a numeric value");
  }
  if (isDigits) {
    port = std::atoi((param.c_str()));
    if (port < 1 || port > 65636)
      throw Error("Invalid port range. The port number should be between 1 and 65636");
    this->port = (uint16_t)port;
  }
}

void Server::setClientMaxBodySize(std::string param) {
  unsigned long bodySize = 0;
  try {
    checkToken(param);
    bodySize = std::strtoul(param.c_str(), NULL, 10);
    if (errno == ERANGE || bodySize > INT_MAX)
      Error("Invalid body size. The specified value exceeds the maximum limit");
  } catch (const std::exception& e) {
    Error("Invalid syntax: client_max_body_size");
  }
  this->maxBodySize = bodySize;
}

void Server::setIndex(std::string index) {
  checkToken(index);
  this->index = index;
}

void Server::setAutoindex(std::string autoindex) {
  checkToken(autoindex);
  if (autoindex != "on" && autoindex != "off")
    throw Error("Invalid syntax: autoindex. Please specify 'on' or 'off'");
  if (autoindex == "on")
    this->autoIndex = true;
}

std::string Server::statusCodeString(short statusCode) {
  std::map<short, std::string> statusCodes;

  statusCodes[100] = "Continue";
  statusCodes[101] = "Switching Protocol";
  statusCodes[200] = "OK";
  statusCodes[201] = "Created";
  statusCodes[202] = "Accepted";
  statusCodes[203] = "Non-Authoritative Information";
  statusCodes[204] = "No Content";
  statusCodes[205] = "Reset Content";
  statusCodes[206] = "Partial Content";
  statusCodes[300] = "Multiple Choice";
  statusCodes[301] = "Moved Permanently";
  statusCodes[302] = "Moved Temporarily";
  statusCodes[303] = "See Other";
  statusCodes[304] = "Not Modified";
  statusCodes[307] = "Temporary Redirect";
  statusCodes[308] = "Permanent Redirect";
  statusCodes[400] = "Bad Request";
  statusCodes[401] = "Unauthorized";
  statusCodes[403] = "Forbidden";
  statusCodes[404] = "Not Found";
  statusCodes[405] = "Method Not Allowed";
  statusCodes[406] = "Not Acceptable";
  statusCodes[407] = "Proxy Authentication Required";
  statusCodes[408] = "Request Timeout";
  statusCodes[409] = "Conflict";
  statusCodes[410] = "Gone";
  statusCodes[411] = "Length Required";
  statusCodes[412] = "Precondition Failed";
  statusCodes[413] = "Payload Too Large";
  statusCodes[414] = "URI Too Long";
  statusCodes[415] = "Unsupported Media Type";
  statusCodes[416] = "Requested Range Not Satisfiable";
  statusCodes[417] = "Expectation Failed";
  statusCodes[418] = "I'm a teapot";
  statusCodes[421] = "Misdirected Request";
  statusCodes[425] = "Too Early";
  statusCodes[426] = "Upgrade Required";
  statusCodes[428] = "Precondition Required";
  statusCodes[429] = "Too Many Requests";
  statusCodes[431] = "Request Header Fields Too Large";
  statusCodes[451] = "Unavailable for Legal Reasons";
  statusCodes[500] = "Internal Server Error";
  statusCodes[501] = "Not Implemented";
  statusCodes[502] = "Bad Gateway";
  statusCodes[503] = "Service Unavailable";
  statusCodes[504] = "Gateway Timeout";
  statusCodes[505] = "HTTP Version Not Supported";
  statusCodes[506] = "Variant Also Negotiates";
  statusCodes[507] = "Insufficient Storage";
  statusCodes[510] = "Not Extended";
  statusCodes[511] = "Network Authentication Required";

  std::map<short, std::string>::iterator it = statusCodes.find(statusCode);
  if (it != statusCodes.end()) {
    return it->second;
  } else {
    return "Undefined";
  }
}

void Server::setErrorPages(std::vector<std::string>& param) {
  if (param.empty())
    return;
  if (param.size() % 2 != 0)
    throw Error("Error page initialization failed. Missing error code-path pairs");

  for (size_t i = 0; i < param.size() - 1; i += 2) {
    std::string codeStr = param[i];
    std::string path = param[i + 1];

    if (codeStr.size() != 3 || !std::isdigit(codeStr[0]) || !std::isdigit(codeStr[1]) || !std::isdigit(codeStr[2]))
      throw Error("Error code is invalid");

    short codeError = static_cast<short>(std::atoi(codeStr.c_str()));

    if (statusCodeString(codeError) == "Undefined" || codeError < 400)
      throw Error("Incorrect error code: " + codeStr);

    checkToken(path);

    std::string fullPath = this->root + path;
    int type = ConfigFile::getTypePath(fullPath);
    if (type != 2) {
      if (ConfigFile::getTypePath(fullPath) != 1)
        throw Error("Incorrect path for error page file: " + fullPath);

      if (ConfigFile::checkAccessFile(fullPath, 0) == -1 || ConfigFile::checkAccessFile(fullPath, 4) == -1)
        throw Error("Error page file: " + fullPath + " is not accessible");
    }

    this->errorPages[codeError] = path;
  }
}

void Server::setLocation(std::string path, std::vector<std::string> param) {
  if (param.empty())
    return;
  if (param.size() % 2 != 0)
    throw Error("Error page initialization failed");

  Location newLocation;
  std::vector<std::string> methods;
  bool flagMethods = false;
  bool flagAutoIndex = false;
  bool flagMaxSize = false;
  int valid;

  newLocation.setPath(path);
  for (size_t i = 0; i < param.size(); i++) {
    if (param[i] == "root" && (i + 1) < param.size()) {
      handleRootLocation(param, i, newLocation);
    } else if ((param[i] == "allow_methods" || param[i] == "methods") && (i + 1) < param.size()) {
      handleAllowMethods(param, i, newLocation, flagMethods);
    } else if (param[i] == "autoindex" && (i + 1) < param.size()) {
      handleAutoindex(param, i, path, newLocation, flagAutoIndex);
    } else if (param[i] == "index" && (i + 1) < param.size()) {
      handleIndexLocation(param, i, newLocation);
    } else if (param[i] == "return" && (i + 1) < param.size()) {
      handleReturn(param, i, path, newLocation);
    } else if (param[i] == "alias" && (i + 1) < param.size()) {
      handleAlias(param, i, path, newLocation);
    } else if (param[i] == "cgi_ext" && (i + 1) < param.size()) {
      handleCgiExtension(param, i, newLocation);
    } else if (param[i] == "cgi_path" && (i + 1) < param.size()) {
      handleCgiPath(param, i, newLocation);
    } else if (param[i] == "client_max_body_size" && (i + 1) < param.size()) {
      handleMaxBodySize(param, i, newLocation, flagMaxSize);
    } else if (i < param.size()) {
      throw Error("param in a location is invalid");
    }
  }

  handleLocationDefaults(newLocation, flagMaxSize);

  valid = isValidLocation(newLocation);
  handleLocationValidation(valid);

  this->locations.push_back(newLocation);
}

void Server::handleRootLocation(std::vector<std::string>& param, size_t& i, Location& newLocation) {
  if (!newLocation.getRootLocation().empty())
    throw Error("Root of location is duplicated");
  checkToken(param[++i]);
  std::string rootLocation = ConfigFile::getTypePath(param[i]) == 2 ? param[i] : (this->root + param[i]);
  newLocation.setRootLocation(rootLocation);
}

void Server::handleAllowMethods(std::vector<std::string>& param, size_t& i, Location& newLocation, bool& flagMethods) {
  if (flagMethods)
    throw Error("Allow_methods of location is duplicated");
  std::vector<std::string> methods;
  while (++i < param.size()) {
    if (param[i].find(";") != std::string::npos) {
      checkToken(param[i]);
      methods.push_back(param[i]);
      break;
    } else {
      methods.push_back(param[i]);
      if (i + 1 >= param.size())
        throw Error("Token is invalid");
    }
  }
  newLocation.setMethods(methods);
  flagMethods = true;
}

void Server::handleAutoindex(std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation, bool& flagAutoIndex) {
  if (path == "/cgi")
    throw Error("param autoindex not allowed for CGI");
  if (flagAutoIndex)
    throw Error("Autoindex of location is duplicated");
  checkToken(param[++i]);
  newLocation.setAutoindex(param[i]);
  flagAutoIndex = true;
}

void Server::handleIndexLocation(std::vector<std::string>& param, size_t& i, Location& newLocation) {
  if (!newLocation.getIndexLocation().empty())
    throw Error("Index of location is duplicated");
  checkToken(param[++i]);
  newLocation.setIndexLocation(param[i]);
}

void Server::handleReturn(std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation) {
  if (path == "/cgi")
    throw Error("param return not allowed for CGI");
  if (!newLocation.getReturn().empty())
    throw Error("Return of location is duplicated");
  checkToken(param[++i]);
  newLocation.setReturn(param[i]);
}

void Server::handleAlias(std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation) {
  if (path == "/cgi")
    throw Error("param alias not allowed for CGI");
  if (!newLocation.getAlias().empty())
    throw Error("Alias of location is duplicated");
  checkToken(param[++i]);
  newLocation.setAlias(param[i]);
}

void Server::handleCgiExtension(std::vector<std::string>& param, size_t& i, Location& newLocation) {
  std::vector<std::string> extension;
  while (++i < param.size()) {
    if (param[i].find(";") != std::string::npos) {
      checkToken(param[i]);
      extension.push_back(param[i]);
      break;
    } else {
      extension.push_back(param[i]);
      if (i + 1 >= param.size())
        throw Error("Invalid token");
    }
  }
  newLocation.setCgiExtension(extension);
}

void Server::handleCgiPath(std::vector<std::string>& param, size_t& i, Location& newLocation) {
  std::vector<std::string> path;
  while (++i < param.size()) {
    if (param[i].find(";") != std::string::npos) {
      checkToken(param[i]);
      path.push_back(param[i]);
      break;
    } else {
      path.push_back(param[i]);
      if (i + 1 >= param.size())
        throw Error("Invalid token");
    }
    if (param[i].find("/python") == std::string::npos)
      throw Error("Invalid cgi path");
  }
  newLocation.setCgiPath(path);
}

void Server::handleMaxBodySize(std::vector<std::string>& param, size_t& i, Location& newLocation, bool& flagMaxSize) {
  if (flagMaxSize)
    throw Error("Max_body_size of location is duplicated");
  checkToken(param[++i]);
  newLocation.setMaxBodySize(param[i]);
  flagMaxSize = true;
}

void Server::handleLocationDefaults(Location& newLocation, bool flagMaxSize) {
  if (newLocation.getPath() != "/cgi" && newLocation.getIndexLocation().empty())
    newLocation.setIndexLocation(this->index);
  if (!flagMaxSize)
    newLocation.setMaxBodySize(this->maxBodySize);
}

void Server::handleLocationValidation(int valid) {
  if (valid == 1)
    throw Error("Invalid CGI configuration. CGI validation failed");
  else if (valid == 2)
    throw Error("Invalid path configuration. Path in location validation failed");
  else if (valid == 3)
    throw Error("Invalid redirection file. Redirection file in location validation failed");
  else if (valid == 4)
    throw Error("Invalid alias file. Alias file in location validation failed");
}

void Server::setFd(int fd) {
  this->listenFd = fd;
}

bool Server::isValidHost(std::string host) const {
  struct sockaddr_in sockaddr;
  return (inet_pton(AF_INET, host.c_str(), &(sockaddr.sin_addr)) ? true : false);
}

bool Server::isValidErrorPages() {
  std::map<short, std::string>::const_iterator it;
  for (it = this->errorPages.begin(); it != this->errorPages.end(); it++) {
    if (it->first < 100 || it->first > 599)
      return (false);
    if (ConfigFile::checkAccessFile(getRoot() + it->second, 0) < 0 || ConfigFile::checkAccessFile(getRoot() + it->second, 4) < 0)
      return (false);
  }
  return (true);
}

int Server::isValidLocation(Location& location) const {
  if (location.getPath() == "/cgi") {
    return isValidCgiLocation(location);
  } else {
    return isValidRegularLocation(location);
  }
}

int Server::isValidCgiLocation(Location& location) const {
  if (location.getCgiPath().empty() || location.getCgiExtension().empty() || location.getIndexLocation().empty()) {
    return 1;
  }

  if (ConfigFile::checkAccessFile(location.getIndexLocation(), 4) < 0) {
    std::string path = location.getRootLocation() + location.getPath() + "/" + location.getIndexLocation();
    if (ConfigFile::getTypePath(path) != 1) {
      std::string root = getcwd(NULL, 0);
      location.setRootLocation(root);
      path = root + location.getPath() + "/" + location.getIndexLocation();
    }
    if (path.empty() || ConfigFile::getTypePath(path) != 1 || ConfigFile::checkAccessFile(path, 4) < 0) {
      return 1;
    }
  }

  if (location.getCgiPath().size() != location.getCgiExtension().size()) {
    return 1;
  }

  std::vector<std::string>::const_iterator itPath;
  for (itPath = location.getCgiPath().begin(); itPath != location.getCgiPath().end(); ++itPath) {
    const std::string& cgiPath = *itPath;
    if (ConfigFile::getTypePath(cgiPath) < 0) {
      return 1;
    }
  }

  std::vector<std::string>::const_iterator itExtension;
  for (itExtension = location.getCgiExtension().begin(); itExtension != location.getCgiExtension().end(); ++itExtension) {
    const std::string& cgiExtension = *itExtension;
    if (cgiExtension != ".py" && cgiExtension != "*.py") {
      return 1;
    }

    std::vector<std::string>::const_iterator itPath;
    for (itPath = location.getCgiPath().begin(); itPath != location.getCgiPath().end(); ++itPath) {
      const std::string& cgiPath = *itPath;
      if ((cgiExtension == ".py" || cgiExtension == "*.py") && cgiPath.find("python") != std::string::npos) {
        location.extPath.insert(std::make_pair(".py", cgiPath));
      }
    }
  }

  if (location.getCgiPath().size() != location.getExtensionPath().size()) {
    return 1;
  }

  return 0;
}

int Server::isValidRegularLocation(Location& location) const {
  if (location.getPath()[0] != '/') {
    return 2;
  }

  if (location.getRootLocation().empty()) {
    location.setRootLocation(this->root);
  }

  if (ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPath() + "/", location.getIndexLocation())) {
    return 5;
  }

  if (!location.getReturn().empty() && ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getReturn())) {
    return 3;
  }

  if (!location.getAlias().empty() && ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getAlias())) {
    return 4;
  }

  return 0;
}

const std::string& Server::getServerName() {
  return (this->serverName);
}

const std::string& Server::getRoot() {
  return (this->root);
}

const bool& Server::getAutoindex() {
  return (this->autoIndex);
}

const in_addr_t& Server::getHost() {
  return (this->host);
}

const uint16_t& Server::getPort() {
  return (this->port);
}

const size_t& Server::getClientMaxBodySize() {
  return (this->maxBodySize);
}

const std::vector<Location>& Server::getLocations() {
  return (this->locations);
}

const std::map<short, std::string>& Server::getErrorPages() {
  return (this->errorPages);
}

const std::string& Server::getIndex() {
  return (this->index);
}

int Server::getFd() {
  return (this->listenFd);
}

const std::string& Server::getPathErrorPage(short key) {
  std::map<short, std::string>::iterator it = this->errorPages.find(key);
  if (it == this->errorPages.end())
    throw Error("Specified error page does not exist");
  return (it->second);
}

const std::vector<Location>::iterator Server::getLocationKey(std::string key) {
  std::vector<Location>::iterator it;
  for (it = this->locations.begin(); it != this->locations.end(); it++) {
    if (it->getPath() == key)
      return (it);
  }
  throw Error("Path to location not found: " + key);
}

void Server::checkToken(std::string& param) {
  size_t pos = param.rfind(';');
  if (pos != param.size() - 1)
    throw Error("Invalid token. Token must end with a semicolon");
  param.erase(pos);
}

bool Server::checkLocation() const {
  if (this->locations.size() < 2)
    return (false);
  std::vector<Location>::const_iterator it1;
  std::vector<Location>::const_iterator it2;
  for (it1 = this->locations.begin(); it1 != this->locations.end() - 1; it1++) {
    for (it2 = it1 + 1; it2 != this->locations.end(); it2++) {
      if (it1->getPath() == it2->getPath())
        return (true);
    }
  }
  return (false);
}

void Server::setupServer(void) {
  if ((listenFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    LogService::printLog(RED_BOLD, FAILURE, "Failed to create socket: %s. Closing...", strerror(errno));

  int option_value = 1;
  setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = host;
  serverAddress.sin_port = htons(port);
  if (bind(listenFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
    LogService::printLog(RED_BOLD, FAILURE, "Failed to bind: %s. Closing...", strerror(errno));
}