#include "../inc/ManagerServ.hpp"
#include "../inc/Webserv.hpp"

int error(std::string errorMessage) {
  std::cerr << "\033[0;31m";
  std::cerr << errorMessage << std::endl;
  std::cerr << "\033[0m";
  return 1;
}

int main(int argc, char **argv) {
  try {
    if (argc != 2)
      throw std::invalid_argument("Invalid number of arguments. Usage: ./webserv <config_file>.conf.");
    signal(SIGPIPE, SIG_IGN);
    std::string configFile = (argv[1]);
    Parser serverParser;
    ManagerServ managerServ;
    serverParser.parseServerConfigFile(configFile);
    managerServ.setupServers(serverParser.getServers());
    managerServ.processServerRequests();
  } catch (std::exception &err) {
    return error(err.what());
  }
  return 0;
}