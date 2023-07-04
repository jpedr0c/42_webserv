#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Webserv.hpp"

class Client {
 public:
  Client();
  Client(const Client &other);
  Client(Server &);
  Client &operator=(const Client &rhs);
  ~Client();

  const time_t &getLastMessageTime() const;

  void setSocket(int &);
  void setServer(Server &);
  void buildResponse();
  void updateLastMessageTime();

  void clearClient();
  Response response;
  Request request;
  Server server;

 private:
  int clientSocket;
  struct sockaddr_in clientSocketAddr;
  time_t lastMessageTime;
};

#endif