#include "../inc/ManagerServ.hpp"

ManagerServ::ManagerServ() {}
ManagerServ::~ManagerServ() {}

void ManagerServ::setupServers(std::vector<Server> servers) {
  std::cout << "Initializing servers...\n";
  serv = servers;

  for (std::vector<Server>::iterator it = serv.begin(); it != serv.end(); ++it) {
    bool isSameServ = false;
    for (std::vector<Server>::iterator existingServerIt = serv.begin(); existingServerIt != it; ++existingServerIt) {
      if (existingServerIt->getHost() == it->getHost() && existingServerIt->getPort() == it->getPort()) {
        it->setFd(existingServerIt->getFd());
        isSameServ = true;
      }
    }
    if (!isSameServ)
      it->setupServer();
    LogService::logStartServer(*it);
  }
}

void ManagerServ::processServerRequests() {
  fd_set receivedFdCopy;
  fd_set writeFdCopy;
  int selectStatus;

  largestFd = 0;
  initializeSets();
  struct timeval timer;
  while (true) {
    timer.tv_sec = 1;
    timer.tv_usec = 0;
    receivedFdCopy = receiveFdSet;
    writeFdCopy = writeFdSet;

    if ((selectStatus = select(largestFd + 1, &receivedFdCopy, &writeFdCopy, NULL, &timer)) < 0)
      LogService::printLog(RED, FAILURE, "webserv: Select error: %s. Closing the connection...", strerror(errno));
    for (int i = 0; i <= largestFd; ++i) {
      if (FD_ISSET(i, &receivedFdCopy) && servsDict.count(i))
        acceptNewConnection(servsDict.find(i)->second);
      else if (FD_ISSET(i, &receivedFdCopy) && clientsDict.count(i))
        readAndProcessRequest(i, clientsDict[i]);
      else if (FD_ISSET(i, &writeFdCopy) && clientsDict.count(i)) {
        int cgi_state = clientsDict[i].response.getCgiState();
        if (cgi_state == 1 && FD_ISSET(clientsDict[i].response._cgi_obj.pipe_in[1], &writeFdCopy))
          sendCgiBody(clientsDict[i], clientsDict[i].response._cgi_obj);
        else if (cgi_state == 1 && FD_ISSET(clientsDict[i].response._cgi_obj.pipe_out[0], &receivedFdCopy))
          readCgiResponse(clientsDict[i], clientsDict[i].response._cgi_obj);
        else if ((cgi_state == 0 || cgi_state == 2) && FD_ISSET(i, &writeFdCopy))
          sendResponse(i, clientsDict[i]);
      }
    }
    handleClientTimeout();
  }
}

void ManagerServ::handleClientTimeout() {
  for (std::map<int, Client>::iterator it = clientsDict.begin(); it != clientsDict.end(); ++it) {
    if (time(NULL) - it->second.getLastMessageTime() > 30) {
      LogService::printLog(YELLOW, SUCCESS, "Client %d timed out. Closing connection...", it->first);
      closeConnection(it->first);
      return;
    }
  }
}

void ManagerServ::initializeSets() {
  FD_ZERO(&receiveFdSet);
  FD_ZERO(&writeFdSet);

  for (std::vector<Server>::iterator it = serv.begin(); it != serv.end(); ++it) {
    if (listen(it->getFd(), 512) == -1)
      LogService::printLog(RED, FAILURE, "webserv: failed to listen: %s. Closing the connection...", strerror(errno));
    if (fcntl(it->getFd(), F_SETFL, O_NONBLOCK) < 0)
      LogService::printLog(RED, FAILURE, "webserv: fcntl error: %s. Closing the connection...", strerror(errno));
    addToSet(it->getFd(), receiveFdSet);
    servsDict.insert(std::make_pair(it->getFd(), *it));
  }
  largestFd = serv.back().getFd();
}

void ManagerServ::acceptNewConnection(Server &serv) {
  struct sockaddr_in clientSocketAddr;
  long clientSocketAddrSize = sizeof(clientSocketAddr);
  int clientSocket;
  Client newClient(serv);
  char buf[INET_ADDRSTRLEN];

  if ((clientSocket = accept(serv.getFd(), (struct sockaddr *)&clientSocketAddr,
                             (socklen_t *)&clientSocketAddrSize)) == -1) {
    LogService::printLog(RED, SUCCESS, "webserv: Accept error encountered: %s", strerror(errno));
    return;
  }
  LogService::printLog(YELLOW, SUCCESS, "New connection established from %s, assigned socket %d", inet_ntop(AF_INET, &clientSocketAddr, buf, INET_ADDRSTRLEN), clientSocket);

  addToSet(clientSocket, receiveFdSet);

  if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) {
    LogService::printLog(RED, FAILURE, "webserv: fcntl error: %s. Closing the connection...", strerror(errno));
    removeFromSet(clientSocket, receiveFdSet);
    close(clientSocket);
    return;
  }

  newClient.setSocket(clientSocket);
  if (clientsDict.count(clientSocket) != 0)
    clientsDict.erase(clientSocket);
  clientsDict.insert(std::make_pair(clientSocket, newClient));
}

void ManagerServ::closeConnection(const int fd) {
  if (FD_ISSET(fd, &writeFdSet))
    removeFromSet(fd, writeFdSet);
  if (FD_ISSET(fd, &receiveFdSet))
    removeFromSet(fd, receiveFdSet);
  close(fd);
  clientsDict.erase(fd);
}

void ManagerServ::sendResponse(const int &i, Client &c) {
  int bytesSent;
  std::string response = c.response.getRes();
  if (response.length() >= 40000)
    bytesSent = write(i, response.c_str(), 40000);
  else
    bytesSent = write(i, response.c_str(), response.length());

  if (bytesSent < 0) {
    LogService::printLog(RED, SUCCESS, "sendResponse(): error sending : %s", strerror(errno));
    closeConnection(i);
  } else if (bytesSent == 0 || (size_t)bytesSent == response.length()) {
    LogService::printLog(CYAN, SUCCESS, "Response Sent To Socket %d, Stats=<%d>", i, c.response.getCode());
    if (c.request.keepAlive() == false || c.request.errorCode() || c.response.getCgiState()) {
      LogService::printLog(YELLOW, SUCCESS, "Client %d: Connection Closed.", i);
      closeConnection(i);
    } else {
      removeFromSet(i, writeFdSet);
      addToSet(i, receiveFdSet);
      c.clearClient();
    }
  } else {
    c.updateLastMessageTime();
    c.response.cutRes(bytesSent);
  }
}

bool ManagerServ::checkServ(Client &client, std::vector<Server>::iterator it) {
  bool s = client.request.getServerName() == it->getServerName();
  bool h = client.server.getHost() == it->getHost();
  bool p = client.server.getPort() == it->getPort();
  return s && h && p;
}

void ManagerServ::assignServer(Client &client) {
  for (std::vector<Server>::iterator it = serv.begin();
       it != serv.end(); ++it) {
    if (checkServ(client, it))
      client.setServer(*it);
  }
}

void ManagerServ::readAndProcessRequest(const int &i, Client &client) {
  char buffer[40000];
  int bytesRead = read(i, buffer, 40000);
  if (bytesRead == 0) {
    LogService::printLog(YELLOW, SUCCESS, "webserv: Connection closed for client %d", i);
    closeConnection(i);
    return;
  }
  if (bytesRead < 0) {
    LogService::printLog(RED, SUCCESS, "webserv: Error reading from file descriptor %d: %s", i, strerror(errno));
    closeConnection(i);
    return;
  }
  if (bytesRead) {
    client.updateLastMessageTime();
    client.request.feed(buffer, bytesRead);
    memset(buffer, 0, sizeof(buffer));
  }

  if (client.request.parsingCompleted() || client.request.errorCode()) {
    assignServer(client);
    LogService::printLog(CYAN, SUCCESS, "Request received from Socket %d, Method=<%s>, URI=<%s>", i, client.request.getMethodStr().c_str(), client.request.getPath().c_str());
    client.buildResponse();
    if (client.response.getCgiState()) {
      handleReqBody(client);
      addToSet(client.response._cgi_obj.pipe_in[1], writeFdSet);
      addToSet(client.response._cgi_obj.pipe_out[0], receiveFdSet);
    }
    removeFromSet(i, receiveFdSet);
    addToSet(i, writeFdSet);
  }
}

void ManagerServ::handleReqBody(Client &client) {
  if (client.request.getBody().length() == 0) {
    std::string tmp;
    std::fstream file;
    (client.response._cgi_obj.getCgiPath().c_str());
    std::stringstream ss;
    ss << file.rdbuf();
    tmp = ss.str();
    client.request.setBody(tmp);
  }
}

void ManagerServ::sendCgiBody(Client &client, CgiController &cgi) {
  int bytesSent;
  std::string &req_body = client.request.getBody();

  if (req_body.empty())
    bytesSent = 0;
  else if (req_body.length() >= 40000)
    bytesSent = write(cgi.pipe_in[1], req_body.c_str(), 40000);
  else
    bytesSent = write(cgi.pipe_in[1], req_body.c_str(), req_body.length());

  if (bytesSent < 0) {
    LogService::printLog(RED, SUCCESS, "sendCgiBody() Error Sending: %s", strerror(errno));
    removeFromSet(cgi.pipe_in[1], writeFdSet);
    close(cgi.pipe_in[1]);
    close(cgi.pipe_out[1]);
    client.response.setErrorResponse(500);
  } else if (bytesSent == 0 || static_cast<size_t>(bytesSent) == req_body.length()) {
    removeFromSet(cgi.pipe_in[1], writeFdSet);
    close(cgi.pipe_in[1]);
    close(cgi.pipe_out[1]);
  } else {
    client.updateLastMessageTime();
    req_body = req_body.substr(bytesSent);
  }
}

void ManagerServ::readCgiResponse(Client &client, CgiController &cgi) {
  char buffer[40000 * 2];
  int bytesRead = 0;
  bytesRead = read(cgi.pipe_out[0], buffer, 40000 * 2);

  if (bytesRead == 0) {
    removeFromSet(cgi.pipe_out[0], receiveFdSet);
    close(cgi.pipe_in[0]);
    close(cgi.pipe_out[0]);
    int status;
    waitpid(cgi.getCgiPid(), &status, 0);
    if (WEXITSTATUS(status) != 0)
      client.response.setErrorResponse(502);

    client.response.setCgiState(2);
    if (client.response._response_content.find("HTTP/1.1") == std::string::npos)
      client.response._response_content.insert(0, "HTTP/1.1 200 OK\r\n");
    return;
  } else if (bytesRead < 0) {
    LogService::printLog(RED, SUCCESS, "readCgiResponse() Error Reading From CGI Script: ", strerror(errno));
    removeFromSet(cgi.pipe_out[0], receiveFdSet);
    close(cgi.pipe_in[0]);
    close(cgi.pipe_out[0]);
    client.response.setCgiState(2);
    client.response.setErrorResponse(500);
    return;
  } else {
    client.updateLastMessageTime();
    client.response._response_content.append(buffer, bytesRead);
    memset(buffer, 0, sizeof(buffer));
  }
}

void ManagerServ::addToSet(const int fd, fd_set &set) {
  FD_SET(fd, &set);
  if (fd > largestFd)
    largestFd = fd;
}

void ManagerServ::removeFromSet(const int fd, fd_set &fdSet) {
  FD_CLR(fd, &fdSet);
  if (fd == largestFd)
    largestFd--;
}
