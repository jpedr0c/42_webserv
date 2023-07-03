#include "../inc/Location.hpp"

Location::Location() {
  this->path = "";
  this->root = "";
  this->autoIndex = false;
  this->index = "";
  this->returns = "";
  this->alias = "";
  this->maxBodySize = MAX_CONTENT_LENGTH;
  this->methods.reserve(3);
  this->methods.push_back(1);
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
    throw Error("Invalid root location. Please provide a valid path for the root location.");
}

void Location::setMethods(std::vector<std::string> methods) {
  this->methods[0] = 0;
  this->methods[1] = 0;
  this->methods[2] = 0;

  for (size_t i = 0; i < methods.size(); i++) {
    if (methods[i] == "GET")
      this->methods[0] = 1;
    else if (methods[i] == "POST")
      this->methods[1] = 1;
    else if (methods[i] == "DELETE")
      this->methods[2] = 1;
    else
      throw Error("Unsupported HTTP method: " + methods[i] + ". Only GET, POST, and DELETE methods are supported.");
  }
}

void Location::setAutoindex(std::string param) {
  if (param == "on" || param == "off")
    this->autoIndex = (param == "on");
  else
    throw Error("Incorrect autoindex value. Please specify 'on' or 'off' for the autoindex parameter.");
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
      throw Error("Incorrect syntax: client_max_body_size. Only numerical digits are allowed.");
  }
  if (!atoi(param.c_str()))
    throw Error("Incorrect syntax: client_max_body_size. The parameter should be a positive integer.");
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
