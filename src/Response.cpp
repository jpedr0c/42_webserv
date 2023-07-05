#include "../inc/Response.hpp"

Response::Response() {
  targetFile = "";
  body.clear();
  bodyLength = 0;
  responseContent = "";
  responseBody = "";
  location = "";
  code = 0;
  cgi = 0;
  cgiResLength = 0;
  autoIndex = 0;
  mimeTypes[".png"] = "image/png";
  mimeTypes[".html"] = "text/html";
  mimeTypes[".jpg"] = "image/jpg";
  mimeTypes[".jpeg"] = "image/jpeg";
  mimeTypes[".ico"] = "image/x-icon";
  mimeTypes[".svg"] = "image/svg+xml";
  mimeTypes[".bmp"] = "image/bmp";
  mimeTypes["default"] = "text/html";
}

Response::~Response() {}

Response::Response(Request &req) : request(req) {
  targetFile = "";
  body.clear();
  bodyLength = 0;
  responseContent = "";
  responseBody = "";
  location = "";
  code = 0;
  cgi = 0;
  cgiResLength = 0;
  autoIndex = 0;
}

bool Response::hasMimeType(std::string &extension) const {
  return mimeTypes.find(extension) != mimeTypes.end();
}

std::string Response::getMimeType(std::string extension) const {
  std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);

  if (it != mimeTypes.end()) {
    return it->second;
  }
  return mimeTypes.find("default")->second;
}

void Response::contentType() {
  responseContent.append("Content-Type: ");

  std::string::size_type dotPos = targetFile.rfind(".");
  std::string extension = (dotPos != std::string::npos) ? targetFile.substr(dotPos) : "";
  std::string mimeType = (dotPos != std::string::npos && code == 200) ? this->getMimeType(extension) : this->getMimeType("default");

  responseContent.append(mimeType);
  responseContent.append("\r\n");
}

void Response::contentLength() {
  std::stringstream ss;
  ss << responseBody.length();
  responseContent.append("Content-Length: ");
  responseContent.append(ss.str());
  responseContent.append("\r\n");
}

void Response::connection() {
  if (request.getHeader("connection") == "keep-alive")
    responseContent.append("Connection: keep-alive\r\n");
}

void Response::server() {
  responseContent.append("Server: Cheetahs\r\n");
}

void Response::locations() {
  if (location.length())
    responseContent.append("Location: " + location + "\r\n");
}

void Response::date() {
  char date[1000];
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
  responseContent.append("Date: ");
  responseContent.append(date);
  responseContent.append("\r\n");
}

void Response::setHeaders() {
  contentType();
  contentLength();
  connection();
  server();
  locations();
  date();

  responseContent.append("\r\n");
}

static bool fileExists(const std::string &f) {
  std::ifstream file(f.c_str());
  return (file.good());
}

static bool isDirectory(std::string path) {
  struct stat file_stat;
  if (stat(path.c_str(), &file_stat) != 0)
    return (false);

  return (S_ISDIR(file_stat.st_mode));
}

static bool isAllowedMethod(HttpMethod &method, Location &location, short &code) {
  std::vector<short> methods = location.getMethods();
  if ((method == GET && !methods[0]) || (method == POST && !methods[1]) ||
      (method == DELETE && !methods[2])) {
    code = 405;
    return (1);
  }
  return (0);
}

static bool checkReturn(Location &loc, short &code, std::string &location) {
  if (loc.getReturn().empty())
    return (0);
  code = 301;
  location = loc.getReturn();
  if (location[0] != '/')
    location.insert(location.begin(), '/');
  return (1);
}

static std::string combinePaths(std::string p1, std::string p2, std::string p3) {
  if (p1[p1.length() - 1] == '/' && (!p2.empty() && p2[0] == '/'))
    p2.erase(0, 1);
  if (p1[p1.length() - 1] != '/' && (!p2.empty() && p2[0] != '/'))
    p1.insert(p1.end(), '/');
  if (p2[p2.length() - 1] == '/' && (!p3.empty() && p3[0] == '/'))
    p3.erase(0, 1);
  if (p2[p2.length() - 1] != '/' && (!p3.empty() && p3[0] != '/'))
    p2.insert(p1.end(), '/');
  return (p1 + p2 + p3);
}

std::string Response::getErrorPage(short statusCode) {
  return ("<html>\r\n<head><title>" + toString(statusCode) + " " + Server::statusCodeString(statusCode) + " </title></head>\r\n" + "<body>\r\n" + "<center><h1>" + toString(statusCode) + " " + Server::statusCodeString(statusCode) + "</h1></center>\r\n");
}

static void replaceAlias(Location &location, Request &request, std::string &targetFile) {
  targetFile = combinePaths(location.getAlias(), request.getPath().substr(location.getPath().length()), "");
}

static void appendRoot(Location &location, Request &request, std::string &targetFile) {
  targetFile = combinePaths(location.getRootLocation(), request.getPath(), "");
}

int Response::controllerCgiTemp(std::string &locationKey) {
  std::string path;
  path = targetFile;
  cgiObj.clear();
  cgiObj.setCgiPath(path);
  cgi = 1;
  if (pipe(cgiFd) < 0) {
    code = 500;
    return (1);
  }
  cgiObj.initEnvCgi(request, serv.getLocationKey(locationKey));
  cgiObj.execute(this->code);
  return (0);
}

bool Response::isValidPath(std::string &path, std::string &locationKey, size_t &pos) {
  if (!path.empty() && path[0] == '/')
    path.erase(0, 1);

  if (path == "cgi")
    path += "/" + serv.getLocationKey(locationKey)->getIndexLocation();
  else if (path == "cgi/")
    path.append(serv.getLocationKey(locationKey)->getIndexLocation());

  pos = path.find(".");
  if (pos == std::string::npos) {
    code = 501;
    return false;
  }
  return true;
}

bool Response::isValidExtension(std::string &path) {
  std::string exten = path.substr(path.find("."));
  if (exten != ".py") {
    code = 501;
    return false;
  }
  return true;
}

bool Response::isValidFileType(std::string &path) {
  if (ConfigFile::getTypePath(path) != 1) {
    code = 404;
    return false;
  }

  if (ConfigFile::checkAccessFile(path, 1) == -1 || ConfigFile::checkAccessFile(path, 3) == -1) {
    code = 403;
    return false;
  }
  return true;
}

bool Response::isFileAllowed(Request &request, std::string &locationKey) {
  if (isAllowedMethod(request.getHttpMethod(), *serv.getLocationKey(locationKey), code))
    return false;
  return true;
}

bool Response::initializeCgi(std::string &path, std::string &locationKey) {
  cgiObj.clear();
  cgiObj.setCgiPath(path);
  cgi = 1;
  if (pipe(cgiFd) < 0) {
    code = 500;
    return (true);
  }
  cgiObj.initEnv(request, serv.getLocationKey(locationKey));
  cgiObj.execute(this->code);
  return (false);
}

int Response::handleCgi(std::string &locationKey) {
  std::string path = this->request.getPath();
  std::string exten;
  size_t pos = 0;

  if (!isValidPath(path, locationKey, pos))
    return (1);
  if (!isValidExtension(path))
    return (1);
  if (!isValidFileType(path))
    return (1);
  if (!isFileAllowed(request, locationKey))
    return (1);
  if (!(initializeCgi(path, locationKey)))
    return (1);
  return (0);
}

static void getLocationMatch(std::string &path, std::vector<Location> locations, std::string &locationKey) {
  size_t bigMatch = 0;

  for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
    if (path.find(it->getPath()) == 0) {
      if (it->getPath() == "/" || path.length() == it->getPath().length() || path[it->getPath().length()] == '/') {
        if (it->getPath().length() > bigMatch) {
          bigMatch = it->getPath().length();
          locationKey = it->getPath();
        }
      }
    }
  }
}

bool Response::isMethodNotAllowed(Location &location) {
  return isAllowedMethod(request.getHttpMethod(), location, code);
}

bool Response::isRequestBodySizeExceeded(const std::string &body, const Location &location) {
  return (body.length() > location.getMaxBodySize());
}

bool Response::checkLocationReturn(Location &location) {
  return checkReturn(location, code, this->location);
}

bool Response::isCgiPath(const std::string &path) {
  return (path.find("cgi") != std::string::npos);
}

bool Response::isCgiExtension(const std::string &targetFile, const Location &location) {
  return (targetFile.rfind(location.getCgiExtension()[0]) != std::string::npos);
}

bool Response::handleIndexLocation(const std::string &indexLocation, bool autoindex) {
  if (!indexLocation.empty())
    targetFile += indexLocation;
  else
    targetFile += serv.getIndex();

  if (!fileExists(targetFile)) {
    if (autoindex) {
      targetFile.erase(targetFile.find_last_of('/') + 1);
      autoIndex = true;
      return false;
    } else {
      code = 403;
      return true;
    }
  }

  if (isDirectory(targetFile)) {
    code = 301;
    if (!indexLocation.empty())
      location = combinePaths(request.getPath(), indexLocation, "");
    else
      location = combinePaths(request.getPath(), serv.getIndex(), "");
    if (location[location.length() - 1] != '/')
      location.insert(location.end(), '/');

    return true;
  }

  return false;
}

bool Response::handleNonLocation(const std::string &root, Request &request) {
  targetFile = combinePaths(root, request.getPath(), "");
  if (isDirectory(targetFile)) {
    if (targetFile[targetFile.length() - 1] != '/') {
      code = 301;
      location = request.getPath() + "/";
      return true;
    }
    targetFile += serv.getIndex();
    if (!fileExists(targetFile)) {
      code = 403;
      return true;
    }
    if (isDirectory(targetFile)) {
      code = 301;
      location = combinePaths(request.getPath(), serv.getIndex(), "");
      if (location[location.length() - 1] != '/')
        location.insert(location.end(), '/');
      return true;
    }
  }

  return false;
}

int Response::handleTarget() {
  std::string locationKey;
  getLocationMatch(request.getPath(), serv.getLocations(), locationKey);

  if (!locationKey.empty()) {
    Location targetLocation = *serv.getLocationKey(locationKey);

    if (isMethodNotAllowed(targetLocation)) {
      std::cout << "METHOD NOT ALLOWED \n";
      return 1;
    }

    if (isRequestBodySizeExceeded(request.getBody(), targetLocation)) {
      code = 413;
      return 1;
    }

    if (checkLocationReturn(targetLocation))
      return 1;

    if (isCgiPath(targetLocation.getPath())) {
      return handleCgi(locationKey);
    }

    if (!targetLocation.getAlias().empty()) {
      replaceAlias(targetLocation, request, targetFile);
    } else {
      appendRoot(targetLocation, request, targetFile);
    }

    if (!targetLocation.getCgiExtension().empty()) {
      if (isCgiExtension(targetFile, targetLocation)) {
        return controllerCgiTemp(locationKey);
      }
    }

    if (isDirectory(targetFile)) {
      if (targetFile[targetFile.length() - 1] != '/') {
        code = 301;
        location = request.getPath() + "/";
        return 1;
      }

      return handleIndexLocation(targetLocation.getIndexLocation(), targetLocation.getAutoindex());
    }
  } else {
    return handleNonLocation(serv.getRoot(), request);
  }

  return 0;
}

bool Response::reqError() {
  if (request.errorCodes()) {
    code = request.errorCodes();
    return (1);
  }
  return (0);
}

void Response::setServerDefaultErrorPages() {
  responseBody = getErrorPage(code);
}

void Response::buildErrorBody() {
  if (!serv.getErrorPages().count(code) || serv.getErrorPages().at(code).empty() ||
      request.getHttpMethod() == DELETE || request.getHttpMethod() == POST) {
    setServerDefaultErrorPages();
  } else {
    if (code >= 400 && code < 500) {
      location = serv.getErrorPages().at(code);
      if (location[0] != '/')
        location.insert(location.begin(), '/');
      code = 302;
    }

    targetFile = serv.getRoot() + serv.getErrorPages().at(code);
    short oldcode = code;
    if (readFile()) {
      code = oldcode;
      responseBody = getErrorPage(code);
    }
  }
}

int Response::buildHtmlIndex(std::string &dirName, std::vector<uint8_t> &body, size_t &bodyLen) {
  struct dirent *entityStruct;
  DIR *directory;
  std::string dirListPage;

  directory = opendir(dirName.c_str());
  if (directory == NULL) {
    std::cerr << "opendir failed" << std::endl;
    return (1);
  }
  dirListPage.append("<html>\n");
  dirListPage.append("<head>\n");
  dirListPage.append("<title> Index of");
  dirListPage.append(dirName);
  dirListPage.append("</title>\n");
  dirListPage.append("</head>\n");
  dirListPage.append("<body >\n");
  dirListPage.append("<h1> Index of " + dirName + "</h1>\n");
  dirListPage.append("<table style=\"width:80%; font-size: 15px\">\n");
  dirListPage.append("<hr>\n");
  dirListPage.append("<th style=\"text-align:left\"> File Name </th>\n");
  dirListPage.append("<th style=\"text-align:left\"> Last Modification  </th>\n");
  dirListPage.append("<th style=\"text-align:left\"> File Size </th>\n");

  struct stat fileStat;
  std::string filePath;

  while ((entityStruct = readdir(directory)) != NULL) {
    if (strcmp(entityStruct->d_name, ".") == 0)
      continue;
    filePath = dirName + entityStruct->d_name;
    stat(filePath.c_str(), &fileStat);
    dirListPage.append("<tr>\n");
    dirListPage.append("<td>\n");
    dirListPage.append("<a href=\"");
    dirListPage.append(entityStruct->d_name);
    if (S_ISDIR(fileStat.st_mode))
      dirListPage.append("/");
    dirListPage.append("\">");
    dirListPage.append(entityStruct->d_name);
    if (S_ISDIR(fileStat.st_mode))
      dirListPage.append("/");
    dirListPage.append("</a>\n");
    dirListPage.append("</td>\n");
    dirListPage.append("<td>\n");
    dirListPage.append(ctime(&fileStat.st_mtime));
    dirListPage.append("</td>\n");
    dirListPage.append("<td>\n");
    if (!S_ISDIR(fileStat.st_mode))
      dirListPage.append(toString(fileStat.st_size));
    dirListPage.append("</td>\n");
    dirListPage.append("</tr>\n");
  }
  dirListPage.append("</table>\n");
  dirListPage.append("<hr>\n");

  dirListPage.append("</body>\n");
  dirListPage.append("</html>\n");

  body.insert(body.begin(), dirListPage.begin(), dirListPage.end());
  bodyLen = body.size();
  return (0);
}

void Response::buildResponse() {
  if (reqError() || buildBody())
    buildErrorBody();
  if (cgi)
    return;
  else if (autoIndex) {
    std::cout << "Auto index " << std::endl;
    if (buildHtmlIndex(targetFile, body, bodyLength)) {
      code = 500;
      buildErrorBody();
    } else
      code = 200;
    responseBody.insert(responseBody.begin(), body.begin(), body.end());
  }
  setStatusLine();
  setHeaders();
  if (request.getHttpMethod() == GET || code != 200)
    responseContent.append(responseBody);
}

void Response::setErrorResponse(short code) {
  responseContent = "";
  this->code = code;
  responseBody = "";
  buildErrorBody();
  setStatusLine();
  setHeaders();
  responseContent.append(responseBody);
}

std::string Response::getRes() {
  return (responseContent);
}

size_t Response::getLen() const {
  return (responseContent.length());
}

void Response::setStatusLine() {
  responseContent.append("HTTP/1.1 " + toString(code) + " ");
  responseContent.append(Server::statusCodeString(code));
  responseContent.append("\r\n");
}

bool Response::isBodySizeExceeded() {
  return (request.getBody().length() > serv.getClientMaxBodySize());
}

bool Response::handleTgt() {
  if (cgi || autoIndex) {
    return true;
  }
  return false;
}

bool Response::handleGetMethod() {
  if (readFile()) {
    return true;
  }
  return false;
}

bool Response::handlePostMethod() {
  if (fileExists(targetFile) && request.getHttpMethod() == POST) {
    code = 204;
    return false;
  }

  std::ofstream file(targetFile.c_str(), std::ios::binary);
  if (file.fail()) {
    code = 404;
    return true;
  }

  if (request.getMultiformFlag()) {
    std::string body = request.getBody();
    body = removeBoundary(body, request.getBoundary());
    file.write(body.c_str(), body.length());
  } else {
    file.write(request.getBody().c_str(), request.getBody().length());
  }

  return false;
}

bool Response::handleDeleteMethod() {
  if (!fileExists(targetFile)) {
    code = 404;
    return true;
  }
  if (remove(targetFile.c_str()) != 0) {
    code = 500;
    return true;
  }
  return false;
}

int Response::buildBody() {
  if (isBodySizeExceeded()) {
    code = 413;
    return 1;
  }

  if (handleTarget()) {
    return 1;
  }

  if (code) {
    return 0;
  }

  if (request.getHttpMethod() == GET) {
    if (handleGetMethod()) {
      return 1;
    }
  } else if (request.getHttpMethod() == POST) {
    if (handlePostMethod()) {
      return 1;
    }
  } else if (request.getHttpMethod() == DELETE) {
    if (handleDeleteMethod()) {
      return 1;
    }
  }
  code = 200;
  return 0;
}

int Response::readFile() {
  std::ifstream file(targetFile.c_str());

  if (file.fail()) {
    code = 404;
    return (1);
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  responseBody = ss.str();
  return (0);
}

void Response::setServer(Server &serv) {
  this->serv = serv;
}

void Response::setRequest(Request &req) {
  request = req;
}

void Response::cutRes(size_t i) {
  responseContent = responseContent.substr(i);
}

void Response::clear() {
  targetFile.clear();
  body.clear();
  bodyLength = 0;
  responseContent.clear();
  responseBody.clear();
  location.clear();
  code = 0;
  cgi = 0;
  cgiResLength = 0;
  autoIndex = 0;
}

int Response::getCode() const {
  return (code);
}

int Response::getCgiState() {
  return (cgi);
}

std::string Response::extractFilename(const std::string &line) {
  std::string filename;
  size_t start = line.find("filename=\"");
  if (start != std::string::npos) {
    size_t end = line.find("\"", start + 10);
    if (end != std::string::npos) {
      filename = line.substr(start + 10, end - (start + 10));
    }
  }
  return filename;
}

bool Response::isBoundaryLine(const std::string &line, const std::string &boundary) {
  return (line.compare("--" + boundary + "\r") == 0);
}

bool Response::isEndBoundaryLine(const std::string &line, const std::string &boundary) {
  return (line.compare("--" + boundary + "--\r") == 0);
}

std::string Response::removeBoundary(std::string &body, const std::string &boundary) {
  std::string newBody;
  std::string buffer;
  std::string filename;
  bool isBoundary = false;
  bool isContent = false;

  if (body.find("--" + boundary) != std::string::npos && body.find("--" + boundary + "--") != std::string::npos) {
    std::string::size_type pos = 0;
    std::string::size_type prevPos = 0;
    while ((pos = body.find("\n", pos)) != std::string::npos) {
      buffer = body.substr(prevPos, pos - prevPos);

      if (isEndBoundaryLine(buffer, boundary)) {
        isContent = true;
        isBoundary = false;
      }

      if (isBoundaryLine(buffer, boundary)) {
        isBoundary = true;
        filename = extractFilename(buffer);
      }

      if (isBoundary) {
        if (!filename.empty() && buffer.compare("\r") == 0) {
          isBoundary = false;
          isContent = true;
        }
      } else if (isContent) {
        if (isBoundaryLine(buffer, boundary)) {
          isBoundary = true;
        } else if (isEndBoundaryLine(buffer, boundary)) {
          break;
        } else {
          newBody += (buffer + "\n");
        }
      }

      prevPos = ++pos;
    }
  }

  body = newBody;
  return body;
}

void Response::setCgiState(int state) {
  cgi = state;
}