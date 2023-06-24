#include "../inc/Server.hpp"

Server::Server() {
  this->_port = 0;
  this->_host = 0;
  this->_server_name = "";
  this->root = "";
  this->maxBodySize = MAX_CONTENT_LENGTH;
  this->index = "";
  this->_listen_fd = 0;
  this->autoIndex = false;
  this->initErrorPages();
}

Server::~Server() {}

/* copy constructor */
Server::Server(const Server &other) {
  if (this != &other) {
    this->_server_name = other._server_name;
    this->root = other.root;
    this->_host = other._host;
    this->_port = other._port;
    this->maxBodySize = other.maxBodySize;
    this->index = other.index;
    this->_error_pages = other._error_pages;
    this->_locations = other._locations;
    this->_listen_fd = other._listen_fd;
    this->autoIndex = other.autoIndex;
    this->_server_address = other._server_address;
  }
  return;
}

/* assinment operator */
Server &Server::operator=(const Server &rhs) {
  if (this != &rhs) {
    this->_server_name = rhs._server_name;
    this->root = rhs.root;
    this->_port = rhs._port;
    this->_host = rhs._host;
    this->maxBodySize = rhs.maxBodySize;
    this->index = rhs.index;
    this->_error_pages = rhs._error_pages;
    this->_locations = rhs._locations;
    this->_listen_fd = rhs._listen_fd;
    this->autoIndex = rhs.autoIndex;
    this->_server_address = rhs._server_address;
  }
  return (*this);
}

/* init error page by default */
void Server::initErrorPages(void) {
  _error_pages[301] = "";
  _error_pages[302] = "";
  _error_pages[400] = "";
  _error_pages[401] = "";
  _error_pages[402] = "";
  _error_pages[403] = "";
  _error_pages[404] = "";
  _error_pages[405] = "";
  _error_pages[406] = "";
  _error_pages[500] = "";
  _error_pages[501] = "";
  _error_pages[502] = "";
  _error_pages[503] = "";
  _error_pages[505] = "";
  _error_pages[505] = "";
}

/* Set functions */
void Server::setServerName(std::string server_name) {
  checkToken(server_name);
  this->_server_name = server_name;
}

void Server::setHost(std::string parametr) {
  checkToken(parametr);
  if (parametr == "localhost")
    parametr = "127.0.0.1";
  if (!isValidHost(parametr))
    throw ErrorException("Wrong syntax: host");
  this->_host = inet_addr(parametr.data());
}

void Server::setRoot(std::string root) {
  checkToken(root);
  if (ConfigFile::getTypePath(root) == 2) {
    this->root = root;
    return;
  }
  char dir[1024];
  getcwd(dir, 1024);
  std::string full_root = dir + root;
  if (ConfigFile::getTypePath(full_root) != 2)
    throw ErrorException("Wrong syntax: root");
  this->root = full_root;
}

void Server::setPort(std::string parametr) {
  unsigned int port;

  port = 0;
  checkToken(parametr);
  for (size_t i = 0; i < parametr.length(); i++) {
    if (!std::isdigit(parametr[i]))
      throw ErrorException("Wrong syntax: port");
  }
  port = ft_stoi((parametr));
  if (port < 1 || port > 65636)
    throw ErrorException("Wrong syntax: port");
  this->_port = (uint16_t)port;
}

void Server::setClientMaxBodySize(std::string parametr) {
  unsigned long body_size;

  body_size = 0;
  checkToken(parametr);
  for (size_t i = 0; i < parametr.length(); i++) {
    if (parametr[i] < '0' || parametr[i] > '9')
      throw ErrorException("Wrong syntax: client_max_body_size");
  }
  if (!ft_stoi(parametr))
    throw ErrorException("Wrong syntax: client_max_body_size");
  body_size = ft_stoi(parametr);
  this->maxBodySize = body_size;
}

void Server::setIndex(std::string index) {
  checkToken(index);
  this->index = index;
}

void Server::setAutoindex(std::string autoindex) {
  checkToken(autoindex);
  if (autoindex != "on" && autoindex != "off")
    throw ErrorException("Wrong syntax: autoindex");
  if (autoindex == "on")
    this->autoIndex = true;
}

/* checks if there is such a default error code. If there is, it overwrites the path to the file,
otherwise it creates a new pair: error code - path to the file */
void Server::setErrorPages(std::vector<std::string> &parametr) {
  if (parametr.empty())
    return;
  if (parametr.size() % 2 != 0)
    throw ErrorException("Error page initialization faled");
  for (size_t i = 0; i < parametr.size() - 1; i++) {
    for (size_t j = 0; j < parametr[i].size(); j++) {
      if (!std::isdigit(parametr[i][j]))
        throw ErrorException("Error code is invalid");
    }
    if (parametr[i].size() != 3)
      throw ErrorException("Error code is invalid");
    short code_error = ft_stoi(parametr[i]);
    if (statusCodeString(code_error) == "Undefined" || code_error < 400)
      throw ErrorException("Incorrect error code: " + parametr[i]);
    i++;
    std::string path = parametr[i];
    checkToken(path);
    if (ConfigFile::getTypePath(path) != 2) {
      if (ConfigFile::getTypePath(this->root + path) != 1)
        throw ErrorException("Incorrect path for error page file: " + this->root + path);
      if (ConfigFile::checkAccessFile(this->root + path, 0) == -1 || ConfigFile::checkAccessFile(this->root + path, 4) == -1)
        throw ErrorException("Error page file :" + this->root + path + " is not accessible");
    }
    std::map<short, std::string>::iterator it = this->_error_pages.find(code_error);
    if (it != _error_pages.end())
      this->_error_pages[code_error] = path;
    else
      this->_error_pages.insert(std::make_pair(code_error, path));
  }
}

/* parsing and set locations */
void Server::setLocation(std::string path, std::vector<std::string> parametr) {
  Location new_location;
  std::vector<std::string> methods;
  bool flag_methods = false;
  bool flag_autoindex = false;
  bool flag_max_size = false;
  int valid;

  new_location.setPath(path);
  for (size_t i = 0; i < parametr.size(); i++) {
    if (parametr[i] == "root" && (i + 1) < parametr.size()) {
      if (!new_location.getRootLocation().empty())
        throw ErrorException("Root of location is duplicated");
      checkToken(parametr[++i]);
      if (ConfigFile::getTypePath(parametr[i]) == 2)
        new_location.setRootLocation(parametr[i]);
      else
        new_location.setRootLocation(this->root + parametr[i]);
    } else if ((parametr[i] == "allow_methods" || parametr[i] == "methods") && (i + 1) < parametr.size()) {
      if (flag_methods)
        throw ErrorException("Allow_methods of location is duplicated");
      std::vector<std::string> methods;
      while (++i < parametr.size()) {
        if (parametr[i].find(";") != std::string::npos) {
          checkToken(parametr[i]);
          methods.push_back(parametr[i]);
          break;
        } else {
          methods.push_back(parametr[i]);
          if (i + 1 >= parametr.size())
            throw ErrorException("Token is invalid");
        }
      }
      new_location.setMethods(methods);
      flag_methods = true;
    } else if (parametr[i] == "autoindex" && (i + 1) < parametr.size()) {
      if (path == "/cgi")
        throw ErrorException("Parametr autoindex not allow for CGI");
      if (flag_autoindex)
        throw ErrorException("Autoindex of location is duplicated");
      checkToken(parametr[++i]);
      new_location.setAutoindex(parametr[i]);
      flag_autoindex = true;
    } else if (parametr[i] == "index" && (i + 1) < parametr.size()) {
      if (!new_location.getIndexLocation().empty())
        throw ErrorException("Index of location is duplicated");
      checkToken(parametr[++i]);
      new_location.setIndexLocation(parametr[i]);
    } else if (parametr[i] == "return" && (i + 1) < parametr.size()) {
      if (path == "/cgi")
        throw ErrorException("Parametr return not allow for CGI");
      if (!new_location.getReturn().empty())
        throw ErrorException("Return of location is duplicated");
      checkToken(parametr[++i]);
      new_location.setReturn(parametr[i]);
    } else if (parametr[i] == "alias" && (i + 1) < parametr.size()) {
      if (path == "/cgi")
        throw ErrorException("Parametr alias not allow for CGI");
      if (!new_location.getAlias().empty())
        throw ErrorException("Alias of location is duplicated");
      checkToken(parametr[++i]);
      new_location.setAlias(parametr[i]);
    } else if (parametr[i] == "cgi_ext" && (i + 1) < parametr.size()) {
      std::vector<std::string> extension;
      while (++i < parametr.size()) {
        if (parametr[i].find(";") != std::string::npos) {
          checkToken(parametr[i]);
          extension.push_back(parametr[i]);
          break;
        } else {
          extension.push_back(parametr[i]);
          if (i + 1 >= parametr.size())
            throw ErrorException("Token is invalid");
        }
      }
      new_location.setCgiExtension(extension);
    } else if (parametr[i] == "cgi_path" && (i + 1) < parametr.size()) {
      std::vector<std::string> path;
      while (++i < parametr.size()) {
        if (parametr[i].find(";") != std::string::npos) {
          checkToken(parametr[i]);
          path.push_back(parametr[i]);
          break;
        } else {
          path.push_back(parametr[i]);
          if (i + 1 >= parametr.size())
            throw ErrorException("Token is invalid");
        }
        if (parametr[i].find("/python") == std::string::npos && parametr[i].find("/bash") == std::string::npos)
          throw ErrorException("cgi_path is invalid");
      }
      new_location.setCgiPath(path);
    } else if (parametr[i] == "client_max_body_size" && (i + 1) < parametr.size()) {
      if (flag_max_size)
        throw ErrorException("Maxbody_size of location is duplicated");
      checkToken(parametr[++i]);
      new_location.setMaxBodySize(parametr[i]);
      flag_max_size = true;
    } else if (i < parametr.size())
      throw ErrorException("Parametr in a location is invalid");
  }
  if (new_location.getPath() != "/cgi" && new_location.getIndexLocation().empty())
    new_location.setIndexLocation(this->index);
  if (!flag_max_size)
    new_location.setMaxBodySize(this->maxBodySize);
  valid = isValidLocation(new_location);
  if (valid == 1)
    throw ErrorException("Failed CGI validation");
  else if (valid == 2)
    throw ErrorException("Failed path in locaition validation");
  else if (valid == 3)
    throw ErrorException("Failed redirection file in locaition validation");
  else if (valid == 4)
    throw ErrorException("Failed alias file in locaition validation");
  this->_locations.push_back(new_location);
}

void Server::setFd(int fd) {
  this->_listen_fd = fd;
}

/* validation of parametrs */
bool Server::isValidHost(std::string host) const {
  struct sockaddr_in sockaddr;
  return (inet_pton(AF_INET, host.c_str(), &(sockaddr.sin_addr)) ? true : false);
}

bool Server::isValidErrorPages() {
  std::map<short, std::string>::const_iterator it;
  for (it = this->_error_pages.begin(); it != this->_error_pages.end(); it++) {
    if (it->first < 100 || it->first > 599)
      return (false);
    if (ConfigFile::checkAccessFile(getRoot() + it->second, 0) < 0 || ConfigFile::checkAccessFile(getRoot() + it->second, 4) < 0)
      return (false);
  }
  return (true);
}

/* check parametrs of location */
int Server::isValidLocation(Location &location) const {
  if (location.getPath() == "/cgi") {
    if (location.getCgiPath().empty() || location.getCgiExtension().empty() || location.getIndexLocation().empty())
      return (1);

    if (ConfigFile::checkAccessFile(location.getIndexLocation(), 4) < 0) {
      std::string path = location.getRootLocation() + location.getPath() + "/" + location.getIndexLocation();
      if (ConfigFile::getTypePath(path) != 1) {
        std::string root = getcwd(NULL, 0);
        location.setRootLocation(root);
        path = root + location.getPath() + "/" + location.getIndexLocation();
      }
      if (path.empty() || ConfigFile::getTypePath(path) != 1 || ConfigFile::checkAccessFile(path, 4) < 0)
        return (1);
    }
    if (location.getCgiPath().size() != location.getCgiExtension().size())
      return (1);
    std::vector<std::string>::const_iterator it;
    for (it = location.getCgiPath().begin(); it != location.getCgiPath().end(); ++it) {
      if (ConfigFile::getTypePath(*it) < 0)
        return (1);
    }
    std::vector<std::string>::const_iterator it_path;
    for (it = location.getCgiExtension().begin(); it != location.getCgiExtension().end(); ++it) {
      std::string tmp = *it;
      if (tmp != ".py" && tmp != ".sh" && tmp != "*.py" && tmp != "*.sh")
        return (1);
      for (it_path = location.getCgiPath().begin(); it_path != location.getCgiPath().end(); ++it_path) {
        std::string tmp_path = *it_path;
        if (tmp == ".py" || tmp == "*.py") {
          if (tmp_path.find("python") != std::string::npos)
            location.extPath.insert(std::make_pair(".py", tmp_path));
        } else if (tmp == ".sh" || tmp == "*.sh") {
          if (tmp_path.find("bash") != std::string::npos)
            location.extPath[".sh"] = tmp_path;
        }
      }
    }
    if (location.getCgiPath().size() != location.getExtensionPath().size())
      return (1);
  } else {
    if (location.getPath()[0] != '/')
      return (2);
    if (location.getRootLocation().empty()) {
      location.setRootLocation(this->root);
    }
    if (ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPath() + "/", location.getIndexLocation()))
      return (5);
    if (!location.getReturn().empty()) {
      if (ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getReturn()))
        return (3);
    }
    if (!location.getAlias().empty()) {
      if (ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getAlias()))
        return (4);
    }
  }
  return (0);
}

/* Get functions */
const std::string &Server::getServerName() {
  return (this->_server_name);
}

const std::string &Server::getRoot() {
  return (this->root);
}

const bool &Server::getAutoindex() {
  return (this->autoIndex);
}

const in_addr_t &Server::getHost() {
  return (this->_host);
}

const uint16_t &Server::getPort() {
  return (this->_port);
}

const size_t &Server::getClientMaxBodySize() {
  return (this->maxBodySize);
}

const std::vector<Location> &Server::getLocations() {
  return (this->_locations);
}

const std::map<short, std::string> &Server::getErrorPages() {
  return (this->_error_pages);
}

const std::string &Server::getIndex() {
  return (this->index);
}

int Server::getFd() {
  return (this->_listen_fd);
}

/* the two functions below can be used later for response */
const std::string &Server::getPathErrorPage(short key) {
  std::map<short, std::string>::iterator it = this->_error_pages.find(key);
  if (it == this->_error_pages.end())
    throw ErrorException("Error_page does not exist");
  return (it->second);
}

/* find location by a name */  // do not using in parser, created for server manager
const std::vector<Location>::iterator Server::getLocationKey(std::string key) {
  std::vector<Location>::iterator it;
  for (it = this->_locations.begin(); it != this->_locations.end(); it++) {
    if (it->getPath() == key)
      return (it);
  }
  throw ErrorException("Error: path to location not found");
}

/* check is a properly end of parametr */
void Server::checkToken(std::string &parametr) {
  size_t pos = parametr.rfind(';');
  if (pos != parametr.size() - 1)
    throw ErrorException("Token is invalid");
  parametr.erase(pos);
}

/* check location for a dublicate */
bool Server::checkLocaitons() const {
  if (this->_locations.size() < 2)
    return (false);
  std::vector<Location>::const_iterator it1;
  std::vector<Location>::const_iterator it2;
  for (it1 = this->_locations.begin(); it1 != this->_locations.end() - 1; it1++) {
    for (it2 = it1 + 1; it2 != this->_locations.end(); it2++) {
      if (it1->getPath() == it2->getPath())
        return (true);
    }
  }
  return (false);
}

/* socket setup and binding */
void Server::setupServer(void) {
  if ((_listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    LogService::printLog(RED, FAILURE, "webserv: socket error %s   Closing ....", strerror(errno));

  int option_value = 1;
  setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
  memset(&_server_address, 0, sizeof(_server_address));
  _server_address.sin_family = AF_INET;
  _server_address.sin_addr.s_addr = _host;
  _server_address.sin_port = htons(_port);
  if (bind(_listen_fd, (struct sockaddr *)&_server_address, sizeof(_server_address)) == -1)
    LogService::printLog(RED, FAILURE, "webserv: bind error %s   Closing ....", strerror(errno));
}