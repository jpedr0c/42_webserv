#include "../inc/CgiController.hpp"

CgiController::CgiController() {
  this->cgiPid = -1;
  this->exitStatus = 0;
  this->cgiPath = "";
  this->chEnv = NULL;
  this->argv = NULL;
}

CgiController::CgiController(std::string path) {
  this->cgiPid = -1;
  this->exitStatus = 0;
  this->cgiPath = path;
  this->chEnv = NULL;
  this->argv = NULL;
}

CgiController::~CgiController() {
  clear();
}

CgiController::CgiController(const CgiController &copy) {
  this->env = copy.env;
  this->chEnv = copy.chEnv;
  this->argv = copy.argv;
  this->cgiPath = copy.cgiPath;
  this->cgiPid = copy.cgiPid;
  this->exitStatus = copy.exitStatus;
}

CgiController &CgiController::operator=(const CgiController &copy) {
  this->env = copy.env;
  this->chEnv = copy.chEnv;
  this->argv = copy.argv;
  this->cgiPath = copy.cgiPath;
  this->cgiPid = copy.cgiPid;
  this->exitStatus = copy.exitStatus;
  return (*this);
}

void CgiController::setCgiPath(const std::string &cgiPath) {
  this->cgiPath = cgiPath;
}

const pid_t &CgiController::getCgiPid() const {
  return (this->cgiPid);
}

const std::string &CgiController::getCgiPath() const {
  return (this->cgiPath);
}

void CgiController::setContentLength(int length) {
  std::stringstream out;
  out << length;
  env["CONTENT_LENGTH"] = out.str();
}

void CgiController::setContentType(const std::string &contentType) {
  env["CONTENT_TYPE"] = contentType;
}

void CgiController::setDefaultEnvValues(Request &req, std::string cgiExec) {
  env["GATEWAY_INTERFACE"] = "CGI/1.1";
  env["SCRIPT_NAME"] = cgiExec;
  env["SCRIPT_FILENAME"] = cgiPath;
  env["PATH_INFO"] = cgiPath;
  env["PATH_TRANSLATED"] = cgiPath;
  env["REQUEST_URI"] = cgiPath;
  env["SERVER_NAME"] = req.getHeader("host");
  env["SERVER_PORT"] = "8002";
  env["REQUEST_METHOD"] = req.getMethodStr();
  env["SERVER_PROTOCOL"] = "HTTP/1.1";
  env["REDIRECT_STATUS"] = "200";
  env["SERVER_SOFTWARE"] = "CHEETAHS";
}

void CgiController::setRequestHeaders(Request &req) {
  std::map<std::string, std::string> requestHeaders = req.getHeaders();
  for (std::map<std::string, std::string>::iterator it = requestHeaders.begin(); it != requestHeaders.end(); ++it) {
    std::string name = it->first;
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    std::string key = "HTTP_" + name;
    env[key] = it->second;
  }
}

void CgiController::createChEnv() {
  chEnv = new char *[env.size() + 1];
  int i = 0;
  for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it) {
    std::string tmp = it->first + "=" + it->second;
    chEnv[i] = strdup(tmp.c_str());
    i++;
  }
  chEnv[i] = NULL;
}

void CgiController::createArgv(const std::string &cgiExec) {
  argv = new char *[3];
  argv[0] = strdup(cgiExec.c_str());
  argv[1] = strdup(cgiPath.c_str());
  argv[2] = NULL;
}

void CgiController::initEnvCgi(Request &req, const std::vector<Location>::iterator itLocation) {
  std::string cgiExec = ("cgi/" + itLocation->getCgiPath()[0]).c_str();
  char *cwd = getcwd(NULL, 0);
  if (cgiPath[0] != '/') {
    std::string tmp(cwd);
    tmp.append("/");
    if (!cgiPath.empty())
      cgiPath.insert(0, tmp);
  }

  if (req.getHttpMethod() == POST) {
    setContentLength(req.getBody().length());
    setContentType(req.getHeader("content-type"));
  }

  setDefaultEnvValues(req, cgiExec);
  setRequestHeaders(req);

  createChEnv();
  createArgv(cgiExec);

  delete[] cwd;
}

void CgiController::initEnv(Request &req, const std::vector<Location>::iterator itLocation) {
  int poz;
  std::string extension;
  std::string extPath;

  extension = this->cgiPath.substr(this->cgiPath.find("."));
  std::map<std::string, std::string>::iterator itPath = itLocation->extPath.find(extension);
  if (itPath == itLocation->extPath.end())
    return;
  extPath = itLocation->extPath[extension];

  this->env["AUTH_TYPE"] = "Basic";
  this->env["CONTENT_LENGTH"] = req.getHeader("content-length");
  this->env["CONTENT_TYPE"] = req.getHeader("content-type");
  this->env["GATEWAY_INTERFACE"] = "CGI/1.1";
  poz = findStart(this->cgiPath, "cgi/");
  this->env["SCRIPT_NAME"] = this->cgiPath;
  this->env["SCRIPT_FILENAME"] = ((poz < 0 || (size_t)(poz + 8) > this->cgiPath.size()) ? "" : this->cgiPath.substr(poz + 8, this->cgiPath.size()));
  this->env["PATH_INFO"] = getPathInfo(req.getPath(), itLocation->getCgiExtension());
  this->env["PATH_TRANSLATED"] = itLocation->getRootLocation() + (this->env["PATH_INFO"] == "" ? "/" : this->env["PATH_INFO"]);
  this->env["QUERY_STRING"] = decode(req.getQuery());
  this->env["REMOTE_ADDR"] = req.getHeader("host");
  poz = findStart(req.getHeader("host"), ":");
  this->env["SERVER_NAME"] = (poz > 0 ? req.getHeader("host").substr(0, poz) : "");
  this->env["SERVER_PORT"] = (poz > 0 ? req.getHeader("host").substr(poz + 1, req.getHeader("host").size()) : "");
  this->env["REQUEST_METHOD"] = req.getMethodStr();
  this->env["HTTP_COOKIE"] = req.getHeader("cookie");
  this->env["DOCUMENT_ROOT"] = itLocation->getRootLocation();
  this->env["REQUEST_URI"] = req.getPath() + req.getQuery();
  this->env["SERVER_PROTOCOL"] = "HTTP/1.1";
  this->env["REDIRECT_STATUS"] = "200";
  this->env["SERVER_SOFTWARE"] = "CHEETAHS";

  createChEnv();
  createArgv(extPath);
}

void CgiController::closePipes() {
  close(pipeIn[0]);
  close(pipeIn[1]);
  close(pipeOut[0]);
  close(pipeOut[1]);
}

void CgiController::execute(short &errorCode) {
  if (!this->argv[0] || !this->argv[1]) {
    errorCode = 500;
    return;
  }

  if (pipe(pipeIn) < 0 || pipe(pipeOut) < 0) {
    LogService::printLog(RED_BOLD, SUCCESS, "Failed to execute pipe() operation. Please check your system resources and try again.");
    closePipes();
    errorCode = 500;
    return;
  }

  this->cgiPid = fork();
  if (this->cgiPid == 0) {
    dup2(pipeIn[0], STDIN_FILENO);
    dup2(pipeOut[1], STDOUT_FILENO);
    closePipes();

    this->exitStatus = execve(this->argv[0], this->argv, this->chEnv);
    exit(this->exitStatus);

  } else if (this->cgiPid < 0) {
    LogService::printLog(RED_BOLD, SUCCESS, "Fork failed. Unable to create a new process. Please check the system resources and try again.");
    closePipes();
    errorCode = 500;
  }
}

int CgiController::findStart(const std::string path, const std::string delim) {
  if (path.empty())
    return (-1);
  return path.find(delim);
}

unsigned int CgiController::fromHexToDec(const std::string &nb) {
  unsigned int x;
  std::stringstream ss;
  ss << nb;
  ss >> std::hex >> x;
  return (x);
}

std::string CgiController::decode(std::string &path) {
  std::string decodedPath = path;
  size_t token = decodedPath.find("%");
  while (token != std::string::npos) {
    if (decodedPath.length() < token + 2)
      break;
    char decimal = fromHexToDec(decodedPath.substr(token + 1, 2));
    decodedPath.replace(token, 3, toString(decimal));
    token = decodedPath.find("%");
  }
  return decodedPath;
}

std::string CgiController::getPathInfo(std::string &path, std::vector<std::string> extensions) {
  std::string tmp;
  size_t start, end;

  for (std::vector<std::string>::iterator itNext = extensions.begin(); itNext != extensions.end(); itNext++) {
    start = path.find(*itNext);
    if (start != std::string::npos)
      break;
  }
  if (start == std::string::npos)
    return "";
  if (start + 3 >= path.size())
    return "";
  tmp = path.substr(start + 3, path.size());
  if (tmp.empty() || tmp[0] != '/')
    return "";
  end = tmp.find("?");
  return (end == std::string::npos ? tmp : tmp.substr(0, end));
}

void CgiController::clear() {
  cgiPid = -1;
  exitStatus = 0;
  cgiPath.clear();
  cgiPath = "";
  if (chEnv) {
    for (int i = 0; chEnv[i] != NULL; ++i)
      delete[] chEnv[i];
    delete[] chEnv;
    chEnv = NULL;
  }
  if (argv) {
    delete[] argv[0];
    delete[] argv[1];
    delete[] argv;
    argv = NULL;
  }
  env.clear();
}