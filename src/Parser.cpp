#include "../inc/Parser.hpp"

Parser::Parser() {
  this->numberOfServers = 0;
}

Parser::~Parser() {}

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
      throw ErrorException("Wrong character out of server scope{}");
  }
  if (!content[i])
    return (start);
  if (content.compare(i, 6, "server") != 0)
    throw ErrorException("Wrong character out of server scope{}");
  i += 6;
  while (content[i] && isspace(content[i]))
    i++;
  if (content[i] == '{')
    return (i);
  throw ErrorException("Wrong character out of server scope{}");
}

const std::string Parser::extractServersConfig(std::string &fileContent) {
  std::string contentWithoutComments = this->removeComments(fileContent);
  std::string contentWithoutSpaces = this->removeSpaces(contentWithoutComments);
  this->splitServers(contentWithoutSpaces);

  return contentWithoutSpaces;
}

/* checking and read config file, split servers to strings and creating vector of servers */
int Parser::createCluster(const std::string &filePath) {
  ConfigFile file(filePath);

  if (file.getTypePath(filePath) != 1)
    throw Error("File is invalid");

  int isFileReadable = file.checkAccessFile(filePath, R_OK);

  if (isFileReadable == -1)
    throw Error("File is not accessible");

  std::string fileContent = file.readFile(filePath);

  if (fileContent.empty())
    throw Error("File is empty");

  extractServersConfig(fileContent);

  if (this->_server_config.size() != this->numberOfServers)
    throw Error("Something with size");

  for (size_t i = 0; i < this->numberOfServers; i++) {
    Server server;
    createServer(this->_server_config[i], server);
    this->serv.push_back(server);
  }

  if (this->numberOfServers > 1)
    checkServers();

  return (0);
}

/* deleting whitespaces in the start, end and in the content if more than one */

/* spliting servers on separetly strings in vector */
void Parser::splitServers(std::string &content) {
  size_t start = 0;
  size_t end = 1;

  if (content.find("server", 0) == std::string::npos)
    throw ErrorException("Server did not find");
  while (start != end && start < content.length()) {
    start = findStartServer(start, content);
    end = findEndServer(start, content);
    if (start == end)
      throw ErrorException("problem with scope");
    this->_server_config.push_back(content.substr(start, end - start + 1));
    this->numberOfServers++;
    start = end + 1;
  }
}

/* finding a server end and return the index of } end of server */
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

/* spliting line by separator */
std::vector<std::string> splitParametrs(std::string line, std::string sep) {
  std::vector<std::string> str;
  std::string::size_type start, end;

  start = end = 0;
  while (1) {
    end = line.find_first_of(sep, start);
    if (end == std::string::npos)
      break;
    std::string tmp = line.substr(start, end - start);
    str.push_back(tmp);
    start = line.find_first_not_of(sep, end);
    if (start == std::string::npos)
      break;
  }
  return (str);
}

/* creating Server from string and fill the value */
void Parser::createServer(std::string &config, Server &server) {
  std::vector<std::string> parametrs;
  std::vector<std::string> error_codes;
  int flag_loc = 1;
  bool flag_autoindex = false;
  bool flag_max_size = false;

  parametrs = splitParametrs(config += ' ', std::string(" \n\t"));
  if (parametrs.size() < 3)
    throw ErrorException("Failed server validation");
  for (size_t i = 0; i < parametrs.size(); i++) {
    if (parametrs[i] == "listen" && (i + 1) < parametrs.size() && flag_loc) {
      if (server.getPort())
        throw ErrorException("Port is duplicated");
      server.setPort(parametrs[++i]);
    } else if (parametrs[i] == "location" && (i + 1) < parametrs.size()) {
      std::string path;
      i++;
      if (parametrs[i] == "{" || parametrs[i] == "}")
        throw ErrorException("Wrong character in server scope{}");
      path = parametrs[i];
      std::vector<std::string> codes;
      if (parametrs[++i] != "{")
        throw ErrorException("Wrong character in server scope{}");
      i++;
      while (i < parametrs.size() && parametrs[i] != "}")
        codes.push_back(parametrs[i++]);
      server.setLocation(path, codes);
      if (i < parametrs.size() && parametrs[i] != "}")
        throw ErrorException("Wrong character in server scope{}");
      flag_loc = 0;
    } else if (parametrs[i] == "host" && (i + 1) < parametrs.size() && flag_loc) {
      if (server.getHost())
        throw ErrorException("Host is duplicated");
      server.setHost(parametrs[++i]);
    } else if (parametrs[i] == "root" && (i + 1) < parametrs.size() && flag_loc) {
      if (!server.getRoot().empty())
        throw ErrorException("Root is duplicated");
      server.setRoot(parametrs[++i]);
    } else if (parametrs[i] == "error_page" && (i + 1) < parametrs.size() && flag_loc) {
      while (++i < parametrs.size()) {
        error_codes.push_back(parametrs[i]);
        if (parametrs[i].find(';') != std::string::npos)
          break;
        if (i + 1 >= parametrs.size())
          throw ErrorException("Wrong character out of server scope{}");
      }
    } else if (parametrs[i] == "client_max_body_size" && (i + 1) < parametrs.size() && flag_loc) {
      if (flag_max_size)
        throw ErrorException("Client_max_body_size is duplicated");
      server.setClientMaxBodySize(parametrs[++i]);
      flag_max_size = true;
    } else if (parametrs[i] == "server_name" && (i + 1) < parametrs.size() && flag_loc) {
      if (!server.getServerName().empty())
        throw ErrorException("Server_name is duplicated");
      server.setServerName(parametrs[++i]);
    } else if (parametrs[i] == "index" && (i + 1) < parametrs.size() && flag_loc) {
      if (!server.getIndex().empty())
        throw ErrorException("Index is duplicated");
      server.setIndex(parametrs[++i]);
    } else if (parametrs[i] == "autoindex" && (i + 1) < parametrs.size() && flag_loc) {
      if (flag_autoindex)
        throw ErrorException("Autoindex of server is duplicated");
      server.setAutoindex(parametrs[++i]);
      flag_autoindex = true;
    } else if (parametrs[i] != "}" && parametrs[i] != "{") {
      if (!flag_loc)
        throw ErrorException("Parametrs after location");
      else
        throw ErrorException("Unsupported directive");
    }
  }
  if (server.getRoot().empty())
    server.setRoot("/;");
  if (server.getHost() == 0)
    server.setHost("localhost;");
  if (server.getIndex().empty())
    server.setIndex("index.html;");
  if (ConfigFile::isFileExistAndReadable(server.getRoot(), server.getIndex()))
    throw ErrorException("Index from config file not found or unreadable");
  if (server.checkLocaitons())
    throw ErrorException("Locaition is duplicated");
  if (!server.getPort())
    throw ErrorException("Port not found");
  server.setErrorPages(error_codes);
  if (!server.isValidErrorPages())
    throw ErrorException("Incorrect path for error page or number of error");
}

/* comparing strings from position */
int Parser::stringCompare(std::string str1, std::string str2, size_t pos) {
  size_t i;

  i = 0;
  while (pos < str1.length() && i < str2.length() && str1[pos] == str2[i]) {
    pos++;
    i++;
  }
  if (i == str2.length() && pos <= str1.length() && (str1.length() == pos || isspace(str1[pos])))
    return (0);
  return (1);
}

/* checking repeat and mandatory parametrs*/
void Parser::checkServers() {
  std::vector<Server>::iterator it1;
  std::vector<Server>::iterator it2;

  for (it1 = this->serv.begin(); it1 != this->serv.end() - 1; it1++) {
    for (it2 = it1 + 1; it2 != this->serv.end(); it2++) {
      if (it1->getPort() == it2->getPort() && it1->getHost() == it2->getHost() && it1->getServerName() == it2->getServerName())
        throw ErrorException("Failed server validation");
    }
  }
}

std::vector<Server> Parser::getServers() {
  return (this->serv);
}