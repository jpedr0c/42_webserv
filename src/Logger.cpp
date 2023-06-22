#include "../inc/Logger.hpp"

std::string Logger::file_name = "logfile.txt";
LogPrio Logger::prio = ERROR;
L_State Logger::state = ON;

std::map<LogPrio, std::string> Logger::prio_str = initMap();

std::map<LogPrio, std::string> Logger::initMap() {
  std::map<LogPrio, std::string> p_map;

  // p_map[DEBUG] = "[DEBUG]   ";
  p_map[DEBUG] = "[INFO]    ";
  p_map[INFO] = "[DEBUG]    ";
  p_map[ERROR] = "[ERROR]   ";
  return p_map;
}

void Logger::logMsg(const char* color, Mode m, const char* msg, ...) {
  char output[8192];
  va_list args;
  int n;

  if (state == ON) {
    va_start(args, msg);
    n = vsnprintf(output, 8192, msg, args);
    std::string date = getCurrentDateTime();
    if (m == FILE_OUTPUT) {
      if (mkdir("logs", 0777) < 0 && errno != EEXIST) {
        std::cerr << "mkdir() Error: " << strerror(errno) << std::endl;
        return;
      }
      int fd = open(("logs/" + file_name).c_str(), O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR);
      std::cout << "fd is " << fd << "And errno is :" << strerror(errno) << std::endl;
      write(fd, date.c_str(), date.length());
      write(fd, "   ", 3);
      write(fd, output, n);
      write(fd, "\n", 1);
      close(fd);
    } else if (m == CONSOLE_OUTPUT) {
      std::cout << color << getCurrentDateTime() << output << RESET << std::endl;
    }
    va_end(args);
  }
}

std::string Logger::getCurrentDateTime() {
  std::time_t now = std::time(NULL);
  std::tm* time_info = std::localtime(&now);
  char date[100];
  std::strftime(date, sizeof(date), "[%Y-%d-%m %H:%M:%S] ", time_info);
  return std::string(date, date + std::strlen(date));
}

void Logger::setPrio(LogPrio p) {
  Logger::prio = p;
}

void Logger::setFilenName(std::string name) {
  Logger::file_name = name;
}

void Logger::setState(L_State s) {
  Logger::state = s;
}
