#ifndef LogService_HPP
#define LogService_HPP

#include "Webserv.hpp"

#define RED_BOLD "\e[31;1m"
#define GREEN_BOLD "\e[32;1m"
#define YELLOW "\e[33;10m"
#define ORANGE "\e[38;5;208m"
#define BLUE "\e[34;10m"
#define PURPLE "\e[35;10m"
#define WHITE "\e[38;10m"
#define RESET "\e[0m"

enum ExitStatus {
  FAILURE,
  SUCCESS,
};

class Request;
class Server;
class LogService {
 public:
  static std::string getCurrentDateTime();
  static void printLog(const char* color, ExitStatus status, const char* str, ...);
  static void printErrorCodeLog(const char* color, short& errorCode, short code, const char* str, ...);
  static void logStartServer(Server& server);
};

#endif