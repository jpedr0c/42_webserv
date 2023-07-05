#pragma once
#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iomanip>
#include <iostream>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <climits>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include "CgiController.hpp"
#include "ConfigFile.hpp"
#include "Location.hpp"
#include "LogService.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Server.hpp"

#define MAX_URI_LENGTH 4096
#define MAX_CONTENT_LENGTH 30000000

template <typename T>
std::string toString(const T val) {
  std::stringstream stream;
  stream << val;
  return stream.str();
}

class Error : public std::runtime_error {
 public:
  Error(const std::string &errorMessage) : std::runtime_error(formatErrorMessage(errorMessage)) {}

 private:
  std::string formatErrorMessage(const std::string &errorMessage) const {
    if (errorMessage.empty())
      return "";
    return "\033[31m" + errorMessage + "\033[0m";
  }
};

#endif