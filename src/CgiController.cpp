#include "../inc/CgiController.hpp"

/* Constructor */
CgiController::CgiController() {
  this->_cgi_pid = -1;
  this->_exit_status = 0;
  this->cgiPath = "";
  this->_ch_env = NULL;
  this->_argv = NULL;
}

CgiController::CgiController(std::string path) {
  this->_cgi_pid = -1;
  this->_exit_status = 0;
  this->cgiPath = path;
  this->_ch_env = NULL;
  this->_argv = NULL;
}

CgiController::~CgiController() {
  if (this->_ch_env) {
    for (int i = 0; this->_ch_env[i]; i++)
      free(this->_ch_env[i]);
    free(this->_ch_env);
  }
  if (this->_argv) {
    for (int i = 0; this->_argv[i]; i++)
      free(_argv[i]);
    free(_argv);
  }
  this->_env.clear();
}

CgiController::CgiController(const CgiController &other) {
  this->_env = other._env;
  this->_ch_env = other._ch_env;
  this->_argv = other._argv;
  this->cgiPath = other.cgiPath;
  this->_cgi_pid = other._cgi_pid;
  this->_exit_status = other._exit_status;
}

CgiController &CgiController::operator=(const CgiController &rhs) {
  if (this != &rhs) {
    this->_env = rhs._env;
    this->_ch_env = rhs._ch_env;
    this->_argv = rhs._argv;
    this->cgiPath = rhs.cgiPath;
    this->_cgi_pid = rhs._cgi_pid;
    this->_exit_status = rhs._exit_status;
  }
  return (*this);
}

/*Set functions */
void CgiController::setCgiPid(pid_t cgi_pid) {
  this->_cgi_pid = cgi_pid;
}

void CgiController::setCgiPath(const std::string &cgi_path) {
  this->cgiPath = cgi_path;
}

/* Get functions */
const std::map<std::string, std::string> &CgiController::getEnv() const {
  return (this->_env);
}

const pid_t &CgiController::getCgiPid() const {
  return (this->_cgi_pid);
}

const std::string &CgiController::getCgiPath() const {
  return (this->cgiPath);
}

void CgiController::initEnvCgi(Request &req, const std::vector<Location>::iterator it_loc) {
  std::string cgi_exec = ("cgi/" + it_loc->getCgiPath()[0]).c_str();
  char *cwd = getcwd(NULL, 0);
  if (cgiPath[0] != '/') {
    std::string tmp(cwd);
    tmp.append("/");
    if (cgiPath.length() > 0)
      cgiPath.insert(0, tmp);
  }
  if (req.getMethod() == POST) {
    std::stringstream out;
    out << req.getBody().length();
    this->_env["CONTENT_LENGTH"] = out.str();
    this->_env["CONTENT_TYPE"] = req.getHeader("content-type");
  }

  this->_env["GATEWAY_INTERFACE"] = std::string("CGI/1.1");
  this->_env["SCRIPT_NAME"] = cgi_exec;  //
  this->_env["SCRIPT_FILENAME"] = this->cgiPath;
  this->_env["PATH_INFO"] = this->cgiPath;        //
  this->_env["PATH_TRANSLATED"] = this->cgiPath;  //
  this->_env["REQUEST_URI"] = this->cgiPath;      //
  this->_env["SERVER_NAME"] = req.getHeader("host");
  this->_env["SERVER_PORT"] = "8002";
  this->_env["REQUEST_METHOD"] = req.getMethodStr();
  this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
  this->_env["REDIRECT_STATUS"] = "200";
  this->_env["SERVER_SOFTWARE"] = "AMANIX";

  std::map<std::string, std::string> request_headers = req.getHeaders();
  for (std::map<std::string, std::string>::iterator it = request_headers.begin();
       it != request_headers.end(); ++it) {
    std::string name = it->first;
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    std::string key = "HTTP_" + name;
    _env[key] = it->second;
  }
  this->_ch_env = (char **)calloc(sizeof(char *), this->_env.size() + 1);
  std::map<std::string, std::string>::const_iterator it = this->_env.begin();
  for (int i = 0; it != this->_env.end(); it++, i++) {
    std::string tmp = it->first + "=" + it->second;
    this->_ch_env[i] = strdup(tmp.c_str());
  }
  this->_argv = (char **)malloc(sizeof(char *) * 3);
  this->_argv[0] = strdup(cgi_exec.c_str());
  this->_argv[1] = strdup(this->cgiPath.c_str());
  this->_argv[2] = NULL;
}

/* initialization environment variable */
void CgiController::initEnv(Request &req, const std::vector<Location>::iterator it_loc) {
  int poz;
  std::string extension;
  std::string extPath;

  extension = this->cgiPath.substr(this->cgiPath.find("."));
  std::map<std::string, std::string>::iterator it_path = it_loc->extPath.find(extension);
  if (it_path == it_loc->extPath.end())
    return;
  extPath = it_loc->extPath[extension];

  this->_env["AUTH_TYPE"] = "Basic";
  this->_env["CONTENT_LENGTH"] = req.getHeader("content-length");
  this->_env["CONTENT_TYPE"] = req.getHeader("content-type");
  this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
  poz = findStart(this->cgiPath, "cgi/");
  this->_env["SCRIPT_NAME"] = this->cgiPath;
  this->_env["SCRIPT_FILENAME"] = ((poz < 0 || (size_t)(poz + 8) > this->cgiPath.size()) ? "" : this->cgiPath.substr(poz + 8, this->cgiPath.size()));  // check dif cases after put right parametr from the response
  this->_env["PATH_INFO"] = getPathInfo(req.getPath(), it_loc->getCgiExtension());
  this->_env["PATH_TRANSLATED"] = it_loc->getRootLocation() + (this->_env["PATH_INFO"] == "" ? "/" : this->_env["PATH_INFO"]);
  this->_env["QUERY_STRING"] = decode(req.getQuery());
  this->_env["REMOTE_ADDR"] = req.getHeader("host");
  poz = findStart(req.getHeader("host"), ":");
  this->_env["SERVER_NAME"] = (poz > 0 ? req.getHeader("host").substr(0, poz) : "");
  this->_env["SERVER_PORT"] = (poz > 0 ? req.getHeader("host").substr(poz + 1, req.getHeader("host").size()) : "");
  this->_env["REQUEST_METHOD"] = req.getMethodStr();
  this->_env["HTTP_COOKIE"] = req.getHeader("cookie");
  this->_env["DOCUMENT_ROOT"] = it_loc->getRootLocation();
  this->_env["REQUEST_URI"] = req.getPath() + req.getQuery();
  this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
  this->_env["REDIRECT_STATUS"] = "200";
  this->_env["SERVER_SOFTWARE"] = "AMANIX";

  this->_ch_env = (char **)calloc(sizeof(char *), this->_env.size() + 1);
  std::map<std::string, std::string>::const_iterator it = this->_env.begin();
  for (int i = 0; it != this->_env.end(); it++, i++) {
    std::string tmp = it->first + "=" + it->second;
    this->_ch_env[i] = strdup(tmp.c_str());
  }
  this->_argv = (char **)malloc(sizeof(char *) * 3);
  this->_argv[0] = strdup(extPath.c_str());
  this->_argv[1] = strdup(this->cgiPath.c_str());
  this->_argv[2] = NULL;
}

/* Pipe and execute CGI */
void CgiController::execute(short &error_code) {
  if (this->_argv[0] == NULL || this->_argv[1] == NULL) {
    error_code = 500;
    return;
  }
  if (pipe(pipe_in) < 0) {
    LogService::printLog(RED, SUCCESS, "pipe() failed");

    error_code = 500;
    return;
  }
  if (pipe(pipe_out) < 0) {
    LogService::printLog(RED, SUCCESS, "pipe() failed");

    close(pipe_in[0]);
    close(pipe_in[1]);
    error_code = 500;
    return;
  }
  this->_cgi_pid = fork();
  if (this->_cgi_pid == 0) {
    dup2(pipe_in[0], STDIN_FILENO);
    dup2(pipe_out[1], STDOUT_FILENO);
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);
    this->_exit_status = execve(this->_argv[0], this->_argv, this->_ch_env);
    exit(this->_exit_status);
  } else if (this->_cgi_pid > 0) {
  } else {
    std::cout << "Fork failed" << std::endl;
    error_code = 500;
  }
}

int CgiController::findStart(const std::string path, const std::string delim) {
  if (path.empty())
    return (-1);
  size_t poz = path.find(delim);
  if (poz != std::string::npos)
    return (poz);
  else
    return (-1);
}

/* Translation of parameters for QUERY_STRING environment variable */
std::string CgiController::decode(std::string &path) {
  size_t token = path.find("%");
  while (token != std::string::npos) {
    if (path.length() < token + 2)
      break;
    char decimal = fromHexToDec(path.substr(token + 1, 2));
    path.replace(token, 3, toString(decimal));
    token = path.find("%");
  }
  return (path);
}

/* Isolation PATH_INFO environment variable */
std::string CgiController::getPathInfo(std::string &path, std::vector<std::string> extensions) {
  std::string tmp;
  size_t start, end;

  for (std::vector<std::string>::iterator it_ext = extensions.begin(); it_ext != extensions.end(); it_ext++) {
    start = path.find(*it_ext);
    if (start != std::string::npos)
      break;
  }
  if (start == std::string::npos)
    return "";
  if (start + 3 >= path.size())
    return "";
  tmp = path.substr(start + 3, path.size());
  if (!tmp[0] || tmp[0] != '/')
    return "";
  end = tmp.find("?");
  return (end == std::string::npos ? tmp : tmp.substr(0, end));
}

void CgiController::clear() {
  this->_cgi_pid = -1;
  this->_exit_status = 0;
  this->cgiPath = "";
  this->_ch_env = NULL;
  this->_argv = NULL;
  this->_env.clear();
}