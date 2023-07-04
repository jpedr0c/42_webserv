#include "../inc/Parser.hpp"

Parser::Parser() {
  this->numberOfServers = 0;
}

Parser::~Parser() {}

std::vector<Server> Parser::getServers() {
  return (this->serverList);
}

const std::string Parser::removeComments(std::string &string) {
  int hashtagIndex = string.find('#');

  while (hashtagIndex >= 0) {
    int newLineIndex = string.find('\n', hashtagIndex);
    string.erase(hashtagIndex, newLineIndex - hashtagIndex);
    hashtagIndex = string.find('#');
  }

  return string;
}

const std::string Parser::removeSpaces(std::string &string) {
  size_t start = 0;

  while (start < string.size() && std::isspace(string[start]))
    start++;

  string.erase(0, start);
  size_t end = string.size() - 1;

  while (end > 0 && std::isspace(string[end]))
    end--;

  string.erase(end + 1);
  return string;
}

size_t Parser::findStartServer(size_t start, std::string &content) {
  size_t i;

  for (i = start; content[i]; i++) {
    if (content[i] == 's')
      break;
    if (!isspace(content[i]))
      throw Error("Encountered an unexpected character outside of server scope '{}'");
  }
  if (!content[i])
    return (start);
  if (content.compare(i, 6, "server") != 0)
    throw Error("Unexpected character found outside of server scope '{}'");
  i += 6;
  while (content[i] && isspace(content[i]))
    i++;
  if (content[i] == '{')
    return (i);
  throw Error("Unexpected character found outside of server scope '{}'");
}

size_t Parser::findEndServer(size_t start, std::string &content) {
  size_t i;
  size_t scope;

  scope = 0;
  for (i = start + 1; content[i]; i++) {
    if (content[i] == '{')
      scope++;
    if (content[i] == '}') {
      if (!scope)
        return (i);
      scope--;
    }
  }
  return (start);
}

void Parser::splitServers(std::string &content) {
  size_t start = 0;
  size_t end = 1;

  if (content.find("server", 0) == std::string::npos)
    throw Error("Server not found");
  while (start != end && start < content.length()) {
    start = findStartServer(start, content);
    end = findEndServer(start, content);
    if (start == end)
      throw Error("Scope problem encountered. Start and end positions are the same");
    this->serverConfig.push_back(content.substr(start, end - start + 1));
    this->numberOfServers++;
    start = end + 1;
  }
}

const std::string Parser::extractServersConfig(std::string &fileContent) {
  std::string contentWithoutComments = this->removeComments(fileContent);
  std::string contentWithoutSpaces = this->removeSpaces(contentWithoutComments);
  this->splitServers(contentWithoutSpaces);

  return contentWithoutSpaces;
}

bool Parser::areServersDuplicate(Server &currentServer, Server &nextServer) {
  bool isPortDuplicate = (currentServer.getPort() == nextServer.getPort());
  bool isHostDuplicate = (currentServer.getHost() == nextServer.getHost());
  bool isNameDuplicate = (currentServer.getServerName() == nextServer.getServerName());

  return (isPortDuplicate && isHostDuplicate && isNameDuplicate);
}

void Parser::checkDuplicateServerConfigurations() {
  std::vector<Server>::iterator currentServer;
  std::vector<Server>::iterator nextServer;

  for (currentServer = serverList.begin(); currentServer != serverList.end() - 1; currentServer++) {
    nextServer = currentServer;
    ++nextServer;

    while (nextServer != serverList.end()) {
      if (areServersDuplicate(*currentServer, *nextServer))
        throw Error("Duplicate server configuration detected. Servers must have unique combinations of port, host, and server name.");

      ++nextServer;
    }
  }
}

std::vector<std::string> Parser::splitParameters(std::string inputStr, std::string delimeter) {
  std::vector<std::string> parameterList;
  std::string::size_type startPosition = 0;
  std::string::size_type endPosition = inputStr.find_first_of(delimeter, startPosition);

  while (endPosition != std::string::npos) {
    std::string currentSegment = inputStr.substr(startPosition, endPosition - startPosition);
    parameterList.push_back(currentSegment);
    startPosition = inputStr.find_first_not_of(delimeter, endPosition);
    if (startPosition == std::string::npos)
      break;
    endPosition = inputStr.find_first_of(delimeter, startPosition);
  }

  return (parameterList);
}

void Parser::validateServerParametersSize(const std::vector<std::string> &parameters) {
  if (parameters.size() < 3)
    throw Error("Server validation failed. Insufficient number of parameters");
}

void Parser::validateDuplicatePort(Server &server) {
  if (server.getPort())
    throw Error("Duplicate port detected. Port is already assigned");
}

void Parser::validateServerScopeCharacter(const std::string &parameter) {
  if (parameter == "{" || parameter == "}")
    throw Error("Invalid character found in server scope '{}'");
}

std::vector<std::string> Parser::parseLocationCodes(const std::vector<std::string> &parameters, size_t &i) {
  std::vector<std::string> codes;
  if (parameters[++i] != "{")
    throw Error("Unexpected character encountered in server scope '{}'");
  i++;
  while (i < parameters.size() && parameters[i] != "}")
    codes.push_back(parameters[i++]);
  return codes;
}

void Parser::validateClosingBracket(const std::vector<std::string> &parameter, size_t &i) {
  if (i < parameter.size() && parameter[i] != "}")
    throw Error("Invalid character in server scope. Expected '}', but found: '" + parameter[i] + "'");
}

void Parser::validateDuplicateHost(Server &server) {
  if (server.getHost())
    throw Error("Duplicate host detected. Host is already assigned");
}

void Parser::validateDuplicateRoot(Server &server) {
  if (!server.getRoot().empty())
    throw Error("Duplicate root directory detected. Root directory is already assigned");
}

void Parser::processErrorCodes(const std::vector<std::string> &parameters, size_t &i, std::vector<std::string> &errorCodes) {
  while (++i < parameters.size()) {
    errorCodes.push_back(parameters[i]);
    if (parameters[i].find(';') != std::string::npos)
      break;
    if (i + 1 >= parameters.size())
      throw Error("Invalid syntax: Unexpected character outside server scope. Found: '" + parameters[i] + "'");
  }
}

void Parser::validateDuplicateMaxBodySize(bool isMaxSizeSet) {
  if (isMaxSizeSet)
    throw Error("Invalid syntax: Duplicate declaration of 'client_max_body_size'");
}

void Parser::validateDuplicateServerName(Server &server) {
  if (!server.getServerName().empty())
    throw Error("Invalid syntax: Duplicate declaration of 'server_name'");
}

void Parser::validateDuplicateIndex(Server &server) {
  if (!server.getIndex().empty())
    throw Error("Invalid syntax: Duplicate declaration of 'index'");
}

void Parser::validateDuplicateAutoindex(bool flag_autoindex) {
  if (flag_autoindex)
    throw Error("Invalid syntax: Duplicate declaration of 'autoindex'");
}

void Parser::setDefaultServerValues(Server &server) {
  if (server.getRoot().empty())
    server.setRoot("/;");
  if (!server.getHost())
    server.setHost("localhost;");
  if (server.getIndex().empty())
    server.setIndex("index.html;");
}

void Parser::performServerValidations(Server &server) {
  if (ConfigFile::isFileExistAndReadable(server.getRoot(), server.getIndex()))
    throw Error("The index file specified in the config file was not found or is unreadable");
  if (server.checkLocation())
    throw Error("Duplicate location found in the server configuration");
  if (!server.getPort())
    throw Error("Port number is missing in the server configuration");
  if (!server.isValidErrorPages())
    throw Error("Incorrect error page path or invalid number of error pages specified");
}

void Parser::createServerFromConfig(std::string &configString, Server &server) {
  std::vector<std::string> parameterList;
  std::vector<std::string> errorCode;
  int locationFlag = 1;
  bool isAutoindexEnable = false;
  bool isMaxSizeSet = false;

  parameterList = splitParameters(configString += ' ', std::string(" \n\t"));
  validateServerParametersSize(parameterList);

  for (size_t i = 0; i < parameterList.size(); i++) {
    if (parameterList[i] == "listen" && (i + 1) < parameterList.size() && locationFlag) {
      validateDuplicatePort(server);
      server.setPort(parameterList[++i]);
    } else if (parameterList[i] == "location" && (i + 1) < parameterList.size()) {
      validateServerScopeCharacter(parameterList[++i]);
      std::string path = parameterList[i];
      std::vector<std::string> codes = parseLocationCodes(parameterList, i);
      server.setLocation(path, codes);
      validateClosingBracket(parameterList, i);
      locationFlag = 0;
    } else if (parameterList[i] == "host" && (i + 1) < parameterList.size() && locationFlag) {
      validateDuplicateHost(server);
      server.setHost(parameterList[++i]);
    } else if (parameterList[i] == "root" && (i + 1) < parameterList.size() && locationFlag) {
      validateDuplicateRoot(server);
      server.setRoot(parameterList[++i]);
    } else if (parameterList[i] == "error_page" && (i + 1) < parameterList.size() && locationFlag) {
      processErrorCodes(parameterList, i, errorCode);
    } else if (parameterList[i] == "client_max_body_size" && (i + 1) < parameterList.size() && locationFlag) {
      validateDuplicateMaxBodySize(isMaxSizeSet);
      server.setClientMaxBodySize(parameterList[++i]);
      isMaxSizeSet = true;
    } else if (parameterList[i] == "server_name" && (i + 1) < parameterList.size() && locationFlag) {
      validateDuplicateServerName(server);
      server.setServerName(parameterList[++i]);
    } else if (parameterList[i] == "index" && (i + 1) < parameterList.size() && locationFlag) {
      validateDuplicateIndex(server);
      server.setIndex(parameterList[++i]);
    } else if (parameterList[i] == "autoindex" && (i + 1) < parameterList.size() && locationFlag) {
      validateDuplicateAutoindex(isAutoindexEnable);
      server.setAutoindex(parameterList[++i]);
      isAutoindexEnable = true;
    } else if (parameterList[i] != "}" && parameterList[i] != "{") {
      if (!locationFlag)
        throw Error("Unexpected parameters found after the 'location' directive");
      throw Error("Unsupported directive encountered");
    }
  }
  setDefaultServerValues(server);
  performServerValidations(server);
  server.setErrorPages(errorCode);
}

int Parser::parseServerConfigFile(const std::string &filePath) {
  ConfigFile configFile;
  int fileType = configFile.getTypePath(filePath);

  if (fileType == INVALID_TYPE)
    throw Error("Invalid file type detected");

  int isFileReadable = configFile.checkAccessFile(filePath, R_OK);

  if (isFileReadable == -1)
    throw Error("File is not accessible or cannot be read");

  std::string fileContent = configFile.readFile(filePath);

  if (fileContent.empty())
    throw Error("Empty file encountered");

  extractServersConfig(fileContent);

  if (this->serverConfig.size() != this->numberOfServers)
    throw Error("Inconsistent size detected");

  for (size_t i = 0; i < this->numberOfServers; i++) {
    Server server;
    createServerFromConfig(this->serverConfig[i], server);
    this->serverList.push_back(server);
  }

  if (this->numberOfServers > 1)
    checkDuplicateServerConfigurations();

  return (0);
}