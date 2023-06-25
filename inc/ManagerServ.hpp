#ifndef ManagerServ_HPP
#define ManagerServ_HPP

#include "Client.hpp"
#include "Response.hpp"
#include "Webserv.hpp"

class ManagerServ {
 public:
  ManagerServ();
  ~ManagerServ();
  void setupServers(std::vector<Server>);
  void processServerRequests();

 private:
  std::vector<Server> serv;
  std::map<int, Server> servsDict;
  std::map<int, Client> clientsDict;
  fd_set receiveFdSet;
  fd_set writeFdSet;
  int largestFd;

  void acceptNewConnection(Server &);
  void handleClientTimeout();
  void initializeSets();
  void readAndProcessRequest(const int &, Client &);
  void handleReqBody(Client &);
  void sendResponse(const int &, Client &);
  void sendCgiBody(Client &, CgiController &);
  void readCgiResponse(Client &, CgiController &);
  void closeConnection(const int);
  void assignServer(Client &);
  void addToSet(const int, fd_set &);
  void removeFromSet(const int, fd_set &);

  bool checkServ(Client &client, std::vector<Server>::iterator it);
};

#endif