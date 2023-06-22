#ifndef LogService_HPP
#define LogService_HPP

#include "Webserv.hpp"

#define GRAY "\e[30;10m"
#define GRAY_BOLD "\e[30;1m"
#define GRAY_LIGHT "\e[30;2m"
#define GRAY_ITALIC "\e[30;3m"
#define GRAY_UNDERLINE "\e[30;4m"
#define GRAY_BACKGROUND "\e[30;7m"
#define GRAY_STRIKE "\e[30;9m"
#define RED "\033[31m"
#define RED_BOLD "\e[31;1m"
#define RED_LIGHT "\e[31;2m"
#define RED_ITALIC "\e[31;3m"
#define RED_UNDERLINE "\e[31;4m"
#define RED_BACKGROUND "\e[31;7m"
#define RED_STRIKE "\e[31;9m"
#define GREEN "\e[32;10m"
#define GREEN_BOLD "\e[32;1m"
#define GREEN_LIGHT "\e[32;2m"
#define GREEN_ITALIC "\e[32;3m"
#define GREEN_UNDERLINE "\e[32;4m"
#define GREEN_BACKGROUND "\e[32;7m"
#define GREEN_STRIKE "\e[32;9m"
#define YELLOW "\e[33;10m"
#define YELLOW_BOLD "\e[33;1m"
#define YELLOW_LIGHT "\e[33;2m"
#define YELLOW_ITALIC "\e[33;3m"
#define YELLOW_UNDERLINE "\e[33;4m"
#define YELLOW_BACKGROUND "\e[33;7m"
#define YELLOW_STRIKE "\e[33;9m"
#define BLUE "\e[34;10m"
#define BLUE_BOLD "\e[34;1m"
#define BLUE_LIGHT "\e[34;2m"
#define BLUE_ITALIC "\e[34;3m"
#define BLUE_UNDERLINE "\e[34;4m"
#define BLUE_BACKGROUND "\e[34;7m"
#define BLUE_STRIKE "\e[34;9m"
#define PURPLE "\e[35;10m"
#define PURPLE_BOLD "\e[35;1m"
#define PURPLE_LIGHT "\e[35;2m"
#define PURPLE_ITALIC "\e[35;3m"
#define PURPLE_UNDERLINE "\e[35;4m"
#define PURPLE_BACKGROUND "\e[35;7m"
#define PURPLE_STRIKE "\e[35;9m"
#define CYAN "\e[36;10m"
#define CYAN_BOLD "\e[36;1m"
#define CYAN_LIGHT "\e[36;2m"
#define CYAN_ITALIC "\e[36;3m"
#define CYAN_UNDERLINE "\e[36;4m"
#define CYAN_BACKGROUND "\e[36;7m"
#define CYAN_STRIKE "\e[36;9m"
#define WHITE "\e[38;10m"
#define WHITE_BOLD "\e[38;1m"
#define WHITE_LIGHT "\e[38;2m"
#define WHITE_ITALIC "\e[38;3m"
#define WHITE_UNDERLINE "\e[38;4m"
#define WHITE_BACKGROUND "\e[38;7m"
#define WHITE_STRIKE "\e[38;9m"
#define RESET "\e[0m"

enum ExitStatus {
  FAILURE,
  SUCCESS,
};

class ServerConfig;
class LogService {
 public:
  static std::string getCurrentDateTime();
  static void printLog(const char* color, ExitStatus status, const char* str, ...);
  static void logStartServer(ServerConfig& server);
};

#endif