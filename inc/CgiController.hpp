#ifndef CgiController_HPP
#define CgiController_HPP

#include "Webserv.hpp"

class Location;
class Request;
class CgiController {
 private:
  std::map<std::string, std::string> env;
  char **chEnv;
  char **argv;
  int exitStatus;
  std::string cgiPath;
  pid_t cgiPid;

 public:
  int pipeIn[2];
  int pipeOut[2];

  CgiController();
  CgiController(std::string path);
  ~CgiController();
  CgiController(CgiController const &other);
  CgiController &operator=(CgiController const &rhs);
  void initEnv(Request &req, const std::vector<Location>::iterator it_loc);
  void initEnvCgi(Request &req, const std::vector<Location>::iterator it_loc);
  void execute(short &error_code);
  void clear();
  void setContentLength(int length);
  void setContentType(const std::string &contentType);
  void setDefaultEnvValues(Request &req, std::string cgiExec);
  void setRequestHeaders(Request &req);
  void createChEnv();
  void createArgv(const std::string &cgiExec);
  unsigned int fromHexToDec(const std::string &nb);
  void closePipes();
  void setCgiPath(const std::string &cgiPath);
  const pid_t &getCgiPid() const;
  const std::string &getCgiPath() const;
  std::string getPathInfo(std::string &path, std::vector<std::string> extensions);
  int findStart(const std::string path, const std::string delim);
  std::string decode(std::string &path);
};

#endif
