#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include "Webserv.hpp"

enum FileType {
  FILE_TYPE = 1,
  DIRECTORY_TYPE = 2,
  OTHER_TYPE = 3,
  INVALID_TYPE = -1,
};

class ConfigFile {
 private:
  std::string path;

 public:
  ConfigFile();
  ConfigFile(std::string const path);
  ~ConfigFile();

  static int getTypePath(std::string const path);
  static int checkAccessFile(std::string const path, int mode);
  std::string readFile(std::string path);
  static int isFileExistAndReadable(std::string const path, std::string const index);
};

#endif
