#include "../inc/LogService.hpp"

std::string LogService::getCurrentDateTime() {
  std::time_t now = std::time(NULL);
  std::tm* time_info = std::localtime(&now);
  char date[100];
  std::strftime(date, sizeof(date), "[%d-%m-%Y %H:%M:%S] ", time_info);
  return std::string(date, date + std::strlen(date));
}

void LogService::printLog(const char* color, ExitStatus status, const char* str, ...) {
  char output[8192];
  va_list args;

  va_start(args, str);
  vsnprintf(output, sizeof(output), str, args);
  va_end(args);

  std::string errorAndExit = color + LogService::getCurrentDateTime() + output + RESET;

  if (status == FAILURE)
    throw Error(errorAndExit);
  std::cout << color << LogService::getCurrentDateTime() << output << RESET << std::endl;
}

void LogService::printErrorCodeLog(const char* color, short& errorCode, short code, const char* str, ...) {
  char output[8192];
  va_list args;

  va_start(args, str);
  vsnprintf(output, sizeof(output), str, args);
  va_end(args);

  errorCode = code;
  std::cout << color << LogService::getCurrentDateTime() << output << RESET << std::endl;
  return;
}

void LogService::logStartServer(Server& server) {
  char buf[INET_ADDRSTRLEN];

  const char* name = server.getServerName().c_str();
  const char* host = inet_ntop(AF_INET, &server.getHost(), buf, INET_ADDRSTRLEN);
  int port = server.getPort();

  std::stringstream p;
  p << port;
  std::string portStr = p.str();

  std::string serverAddress = "http://" + std::string(host) + ":" + portStr;

  std::cout << PURPLE;
  std::cout << "┌─────────────────────────────────────────────────┐" << std::endl;
  std::cout << "│   Server Address: " << serverAddress << std::setw(33 - serverAddress.size()) << "│" << std::right << std::endl;
  std::cout << "│   " << name << ": " << LogService::getCurrentDateTime() << std::setw(25 - std::string(name).size()) << "│" << std::right << std::endl;
  std::cout << "└─────────────────────────────────────────────────┘" << std::endl;
  std::cout << RESET;
}