#include "../inc/Location.hpp"

Location::Location() {
  this->path = "";
  this->root = "";
  this->autoIndex = false;
  this->index = "";
  this->returns = "";
  this->alias = "";
  this->maxBodySize = MAX_CONTENT_LENGTH;
  this->methods.reserve(5);
  this->methods.push_back(1);
  this->methods.push_back(0);
  this->methods.push_back(0);
  this->methods.push_back(0);
  this->methods.push_back(0);
}

Location::Location(const Location &copy) {
  this->path = copy.path;
  this->root = copy.root;
  this->autoIndex = copy.autoIndex;
  this->index = copy.index;
  this->cgiPath = copy.cgiPath;
  this->cgiExt = copy.cgiExt;
  this->returns = copy.returns;
  this->alias = copy.alias;
  this->methods = copy.methods;
  this->extPath = copy.extPath;
  this->maxBodySize = copy.maxBodySize;
}

Location &Location::operator=(const Location &copy) {
  this->path = copy.path;
  this->root = copy.root;
  this->autoIndex = copy.autoIndex;
  this->index = copy.index;
  this->cgiPath = copy.cgiPath;
  this->cgiExt = copy.cgiExt;
  this->returns = copy.returns;
  this->alias = copy.alias;
  this->methods = copy.methods;
  this->extPath = copy.extPath;
  this->maxBodySize = copy.maxBodySize;

  return (*this);
}

Location::~Location() {}

void Location::setPath(std::string param) {
  this->path = param;
}

void Location::setRootLocation(std::string param) {
  if (ConfigFile::getTypePath(param) == 2)
    this->root = param;
  else
    throw Error("Error: Invalid root location");
}

// FIXME: change methods
void Location::setMethods(std::vector<std::string> methods) {
  this->methods[0] = 0;
  this->methods[1] = 0;
  this->methods[2] = 0;
  this->methods[3] = 0;
  this->methods[4] = 0;

  for (size_t i = 0; i < methods.size(); i++) {
    if (methods[i] == "GET")
      this->methods[0] = 1;
    else if (methods[i] == "POST")
      this->methods[1] = 1;
    else if (methods[i] == "DELETE")
      this->methods[2] = 1;
    else if (methods[i] == "PUT")
      this->methods[3] = 1;
    else if (methods[i] == "HEAD")
      this->methods[4] = 1;
    else
      throw Error("Error: Unsupported HTTP method: " + methods[i]);
  }
}

void Location::setAutoindex(std::string param) {
  if (param == "on" || param == "off")
    this->autoIndex = (param == "on");
  else
    throw Error("Error: Incorrect autoindex value");
}

void Location::setIndexLocation(std::string param) {
  this->index = param;
}

void Location::setReturn(std::string param) {
  this->returns = param;
}

void Location::setAlias(std::string param) {
  this->alias = param;
}

void Location::setCgiPath(std::vector<std::string> path) {
  this->cgiPath = path;
}

void Location::setCgiExtension(std::vector<std::string> extension) {
  this->cgiExt = extension;
}

void Location::setMaxBodySize(std::string param) {
  unsigned long bodySize = 0;

  for (size_t i = 0; i < param.length(); i++) {
    if (param[i] < '0' || param[i] > '9')
      throw Error("Error: Incorrect syntax: client_max_body_size");
  }
  if (!atoi(param.c_str()))
    throw Error("Error: Incorrect syntax: client_max_body_size");
  bodySize = atoi(param.c_str());
  this->maxBodySize = bodySize;
}

void Location::setMaxBodySize(unsigned long param) {
  this->maxBodySize = param;
}

const std::string &Location::getPath() const {
  return (this->path);
}

const std::string &Location::getRootLocation() const {
  return (this->root);
}

const std::string &Location::getIndexLocation() const {
  return (this->index);
}

const std::vector<short> &Location::getMethods() const {
  return (this->methods);
}

const std::vector<std::string> &Location::getCgiPath() const {
  return (this->cgiPath);
}

const std::vector<std::string> &Location::getCgiExtension() const {
  return (this->cgiExt);
}

const bool &Location::getAutoindex() const {
  return (this->autoIndex);
}

const std::string &Location::getReturn() const {
  return (this->returns);
}

const std::string &Location::getAlias() const {
  return (this->alias);
}

const std::map<std::string, std::string> &Location::getExtensionPath() const {
  return (this->extPath);
}

const unsigned long &Location::getMaxBodySize() const {
  return (this->maxBodySize);
}

// FIXME: deletar após refatorar
std::string Location::getPrintMethods() const {
  std::string res;
  if (methods[4])
    res.insert(0, "HEAD");
  if (methods[3]) {
    if (!res.empty())
      res.insert(0, ", ");
    res.insert(0, "PUT");
  }
  if (methods[2]) {
    if (!res.empty())
      res.insert(0, ", ");
    res.insert(0, "DELETE");
  }
  if (methods[1]) {
    if (!res.empty())
      res.insert(0, ", ");
    res.insert(0, "POST");
  }
  if (methods[0]) {
    if (!res.empty())
      res.insert(0, ", ");
    res.insert(0, "GET");
  }
  return (res);
}