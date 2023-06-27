#pragma once
#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iomanip>
#include <iostream>
// # include <fstream>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <sstream>
#include <string>
// # include <bits/stdc++.h>
#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>

/* STL Containers */
#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <vector>

/* System */
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
// # include <machine/types.h>
#include <signal.h>

/* Network */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "CgiController.hpp"
#include "ConfigFile.hpp"
#include "Location.hpp"
#include "LogService.hpp"
#include "Mime.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Server.hpp"

#define CONNECTION_TIMEOUT 60  // Time in seconds before client get kicked out if no data was sent.
#ifdef TESTER
#define MESSAGE_BUFFER 40000
#else
#define MESSAGE_BUFFER 40000
#endif

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

/* Utils.c */

std::string statusCodeString(short);
std::string getErrorPage(short);
int buildHtmlIndex(std::string &, std::vector<uint8_t> &, size_t &);
int ft_stoi(std::string str);
unsigned int fromHexToDec(const std::string &nb);

#endif