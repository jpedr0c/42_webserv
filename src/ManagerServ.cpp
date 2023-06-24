#include "../inc/ManagerServ.hpp"

ManagerServ::ManagerServ() {}
ManagerServ::~ManagerServ() {}

/**
 * Start all servers on ports specified in the config file
 */
void ManagerServ::setupServers(std::vector<Server> servers) {
  std::cout << "Initializing  Servers...\n";
  _servers = servers;

  for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
    bool isSameServ = false;
    for (std::vector<Server>::iterator existingServerIt = _servers.begin(); existingServerIt != it; ++existingServerIt) {
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

/**
 * Runs main loop that goes through all file descriptors from 0 till the biggest fd in the set.
 * - check file descriptors returend from select():
 *      if server fd --> accept new client
 *      if client fd in read_set --> read message from client
 *      if client fd in write_set:
 *          1- If it's a CGI response and Body still not sent to CGI child process --> Send request body to CGI child process.
 *          2- If it's a CGI response and Body was sent to CGI child process --> Read outupt from CGI child process.
 *          3- If it's a normal response --> Send response to client.
 * - servers and clients sockets will be added to _recv_set_pool initially,
 *   after that, when a request is fully parsed, socket will be moved to _write_set_pool
 */
// FIXME: verificar as funções que chama o response porém que são do manager
void ManagerServ::runServers() {
  fd_set receivedFdCopy;
  fd_set writeFdCopy;
  int selectStatus;

  _biggest_fd = 0;
  initializeSets();
  struct timeval timer;
  while (true) {
    timer.tv_sec = 1;
    timer.tv_usec = 0;
    receivedFdCopy = _recv_fd_pool;
    writeFdCopy = _write_fd_pool;

    if ((selectStatus = select(_biggest_fd + 1, &receivedFdCopy, &writeFdCopy, NULL, &timer)) < 0)
      LogService::printLog(RED, FAILURE, "webserv: select error %s   Closing ....", strerror(errno));
    for (int i = 0; i <= _biggest_fd; ++i) {
      if (FD_ISSET(i, &receivedFdCopy) && _servers_map.count(i))
        acceptNewConnection(_servers_map.find(i)->second);
      else if (FD_ISSET(i, &receivedFdCopy) && _clients_map.count(i))
        readRequest(i, _clients_map[i]);
      else if (FD_ISSET(i, &writeFdCopy) && _clients_map.count(i)) {
        int cgi_state = _clients_map[i].response.getCgiState();
        if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_in[1], &writeFdCopy))
          sendCgiBody(_clients_map[i], _clients_map[i].response._cgi_obj);
        else if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_out[0], &receivedFdCopy))
          readCgiResponse(_clients_map[i], _clients_map[i].response._cgi_obj);
        else if ((cgi_state == 0 || cgi_state == 2) && FD_ISSET(i, &writeFdCopy))
          sendResponse(i, _clients_map[i]);
      }
    }
    checkTimeout();
  }
}

// void ManagerServ::runServers() {
//   fd_set receivedFdCopy;
//   fd_set writeFdCopy;
//   int select_ret;

//   _biggest_fd = 0;
//   initializeSets();
//   struct timeval timer;
//   while (true) {
//     timer.tv_sec = 1;
//     timer.tv_usec = 0;
//     receivedFdCopy = _recv_fd_pool;
//     writeFdCopy = _write_fd_pool;

//     if ((select_ret = select(_biggest_fd + 1, &receivedFdCopy, &writeFdCopy, NULL, &timer)) < 0) {
//       LogService::printLog(RED, SUCCESS, "webserv: select error %s   Closing ....", strerror(errno));
//       exit(1);
//       continue;
//     }
//     for (int i = 0; i <= _biggest_fd; ++i) {
//       if (FD_ISSET(i, &receivedFdCopy) && _servers_map.count(i))
//         acceptNewConnection(_servers_map.find(i)->second);
//       else if (FD_ISSET(i, &receivedFdCopy) && _clients_map.count(i))
//         readRequest(i, _clients_map[i]);
//       else if (FD_ISSET(i, &writeFdCopy) && _clients_map.count(i)) {
//         int cgi_state = _clients_map[i].response.getCgiState();  // 0->NoCGI 1->CGI write/read to/from script 2-CGI read/write done
//         if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_in[1], &writeFdCopy))
//           sendCgiBody(_clients_map[i], _clients_map[i].response._cgi_obj);
//         else if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_out[0], &receivedFdCopy))
//           readCgiResponse(_clients_map[i], _clients_map[i].response._cgi_obj);
//         else if ((cgi_state == 0 || cgi_state == 2) && FD_ISSET(i, &writeFdCopy))
//           sendResponse(i, _clients_map[i]);
//       }
//     }
//     checkTimeout();
//   }
// }

/* Checks time passed for clients since last message, If more than CONNECTION_TIMEOUT, close connection */
void ManagerServ::checkTimeout() {
  for (std::map<int, Client>::iterator it = _clients_map.begin(); it != _clients_map.end(); ++it) {
    if (time(NULL) - it->second.getLastTime() > 30) {
      LogService::printLog(YELLOW, SUCCESS, "Client %d Timeout, Closing Connection..", it->first);
      closeConnection(it->first);
      return;
    }
  }
}

/* initialize recv+write fd_sets and add all server listening sockets to _recv_fd_pool. */
void ManagerServ::initializeSets() {
  FD_ZERO(&_recv_fd_pool);
  FD_ZERO(&_write_fd_pool);

  // adds servers sockets to _recv_fd_pool set
  for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
    if (listen(it->getFd(), 512) == -1)
      LogService::printLog(RED, FAILURE, "webserv: listen error: %s   Closing....", strerror(errno));
    if (fcntl(it->getFd(), F_SETFL, O_NONBLOCK) < 0)
      LogService::printLog(RED, FAILURE, "webserv: fcntl error: %s   Closing....", strerror(errno));
    addToSet(it->getFd(), _recv_fd_pool);
    _servers_map.insert(std::make_pair(it->getFd(), *it));
  }
  _biggest_fd = _servers.back().getFd();
}

/**
 * Accept new incomming connection.
 * Create new Client object and add it to _client_map
 * Add client socket to _recv_fd_pool
 */
void ManagerServ::acceptNewConnection(Server &serv) {
  struct sockaddr_in client_address;
  long client_address_size = sizeof(client_address);
  int client_sock;
  Client new_client(serv);
  char buf[INET_ADDRSTRLEN];

  if ((client_sock = accept(serv.getFd(), (struct sockaddr *)&client_address,
                            (socklen_t *)&client_address_size)) == -1) {
    LogService::printLog(RED, SUCCESS, "webserv: accept error %s", strerror(errno));
    return;
  }
  LogService::printLog(YELLOW, SUCCESS, "New Connection From %s, Assigned Socket %d", inet_ntop(AF_INET, &client_address, buf, INET_ADDRSTRLEN), client_sock);

  addToSet(client_sock, _recv_fd_pool);

  if (fcntl(client_sock, F_SETFL, O_NONBLOCK) < 0) {
    LogService::printLog(RED, SUCCESS, "webserv: fcntl error %s", strerror(errno));
    removeFromSet(client_sock, _recv_fd_pool);
    close(client_sock);
    return;
  }

  new_client.setSocket(client_sock);
  if (_clients_map.count(client_sock) != 0)
    _clients_map.erase(client_sock);
  _clients_map.insert(std::make_pair(client_sock, new_client));
}

/* Closes connection from fd i and remove associated client object from _clients_map */
void ManagerServ::closeConnection(const int fd) {
  if (FD_ISSET(fd, &_write_fd_pool))
    removeFromSet(fd, _write_fd_pool);
  if (FD_ISSET(fd, &_recv_fd_pool))
    removeFromSet(fd, _recv_fd_pool);
  close(fd);
  _clients_map.erase(fd);
}

/**
 * Build the response and send it to client.
 * If no error was found in request and Connection header value is keep-alive,
 * connection is kept, otherwise connection will be closed.
 */
// FIXME: verificar o que for relacionado ao request
void ManagerServ::sendResponse(const int &i, Client &c) {
  int bytes_sent;
  std::string response = c.response.getRes();
  if (response.length() >= 40000)
    bytes_sent = write(i, response.c_str(), 40000);
  else
    bytes_sent = write(i, response.c_str(), response.length());

  if (bytes_sent < 0) {
    LogService::printLog(RED, SUCCESS, "sendResponse(): error sending : %s", strerror(errno));
    closeConnection(i);
  } else if (bytes_sent == 0 || (size_t)bytes_sent == response.length()) {
    LogService::printLog(CYAN, SUCCESS, "Response Sent To Socket %d, Stats=<%d>", i, c.response.getCode());
    if (c.request.keepAlive() == false || c.request.errorCode() || c.response.getCgiState()) {
      LogService::printLog(YELLOW, SUCCESS, "Client %d: Connection Closed.", i);
      closeConnection(i);
    } else {
      removeFromSet(i, _write_fd_pool);
      addToSet(i, _recv_fd_pool);
      c.clearClient();
    }
  } else {
    c.updateTime();
    c.response.cutRes(bytes_sent);
  }
}

/* Assigen server_block configuration to a client based on Host Header in request and server_name*/
void ManagerServ::assignServer(Client &c) {
  for (std::vector<Server>::iterator it = _servers.begin();
       it != _servers.end(); ++it) {
    if (c.server.getHost() == it->getHost() &&
        c.server.getPort() == it->getPort() &&
        c.request.getServerName() == it->getServerName()) {
      c.setServer(*it);
      return;
    }
  }
}

/**
 * - Reads data from client and feed it to the parser.
 * Once parser is done or an error was found in the request,
 * socket will be moved from _recv_fd_pool to _write_fd_pool
 * and response will be sent on the next iteration of select().
 */
void ManagerServ::readRequest(const int &i, Client &c) {
  char buffer[40000];
  int bytes_read = 0;
  bytes_read = read(i, buffer, 40000);
  if (bytes_read == 0) {
    LogService::printLog(YELLOW, SUCCESS, "webserv: Client %d Closed Connection", i);
    closeConnection(i);
    return;
  } else if (bytes_read < 0) {
    LogService::printLog(RED, SUCCESS, "webserv: fd %d read error %s", i, strerror(errno));
    closeConnection(i);
    return;
  } else if (bytes_read != 0) {
    c.updateTime();
    c.request.feed(buffer, bytes_read);
    memset(buffer, 0, sizeof(buffer));
  }

  if (c.request.parsingCompleted() || c.request.errorCode())  // 1 = parsing completed and we can work on the response.
  {
    assignServer(c);
    LogService::printLog(CYAN, SUCCESS, "Request Recived From Socket %d, Method=<%s>  URI=<%s>", i, c.request.getMethodStr().c_str(), c.request.getPath().c_str());
    c.buildResponse();
    if (c.response.getCgiState()) {
      handleReqBody(c);
      addToSet(c.response._cgi_obj.pipe_in[1], _write_fd_pool);
      addToSet(c.response._cgi_obj.pipe_out[0], _recv_fd_pool);
    }
    removeFromSet(i, _recv_fd_pool);
    addToSet(i, _write_fd_pool);
  }
}

void ManagerServ::handleReqBody(Client &c) {
  if (c.request.getBody().length() == 0) {
    std::string tmp;
    std::fstream file;
    (c.response._cgi_obj.getCgiPath().c_str());
    std::stringstream ss;
    ss << file.rdbuf();
    tmp = ss.str();
    c.request.setBody(tmp);
  }
}

// FIXME: essa função não foi criada no manager, só prcisa refatorar
/* Send request body to CGI script */
void ManagerServ::sendCgiBody(Client &c, CgiController &cgi) {
  int bytes_sent;
  std::string &req_body = c.request.getBody();

  if (req_body.length() == 0)
    bytes_sent = 0;
  else if (req_body.length() >= 40000)
    bytes_sent = write(cgi.pipe_in[1], req_body.c_str(), 40000);
  else
    bytes_sent = write(cgi.pipe_in[1], req_body.c_str(), req_body.length());

  if (bytes_sent < 0) {
    LogService::printLog(RED, SUCCESS, "sendCgiBody() Error Sending: %s", strerror(errno));
    removeFromSet(cgi.pipe_in[1], _write_fd_pool);
    close(cgi.pipe_in[1]);
    close(cgi.pipe_out[1]);
    c.response.setErrorResponse(500);
  } else if (bytes_sent == 0 || (size_t)bytes_sent == req_body.length()) {
    removeFromSet(cgi.pipe_in[1], _write_fd_pool);
    close(cgi.pipe_in[1]);
    close(cgi.pipe_out[1]);
  } else {
    c.updateTime();
    req_body = req_body.substr(bytes_sent);
  }
}

// FIXME: essa função não foi criada no manager, só precisa refatorar
/* Reads outpud produced by the CGI script */
void ManagerServ::readCgiResponse(Client &c, CgiController &cgi) {
  char buffer[40000 * 2];
  int bytes_read = 0;
  bytes_read = read(cgi.pipe_out[0], buffer, 40000 * 2);

  if (bytes_read == 0) {
    removeFromSet(cgi.pipe_out[0], _recv_fd_pool);
    close(cgi.pipe_in[0]);
    close(cgi.pipe_out[0]);
    int status;
    waitpid(cgi.getCgiPid(), &status, 0);
    if (WEXITSTATUS(status) != 0) {
      c.response.setErrorResponse(502);
    }
    c.response.setCgiState(2);
    if (c.response._response_content.find("HTTP/1.1") == std::string::npos)
      c.response._response_content.insert(0, "HTTP/1.1 200 OK\r\n");
    return;
  } else if (bytes_read < 0) {
    LogService::printLog(RED, SUCCESS, "readCgiResponse() Error Reading From CGI Script: ", strerror(errno));
    removeFromSet(cgi.pipe_out[0], _recv_fd_pool);
    close(cgi.pipe_in[0]);
    close(cgi.pipe_out[0]);
    c.response.setCgiState(2);
    c.response.setErrorResponse(500);
    return;
  } else {
    c.updateTime();
    c.response._response_content.append(buffer, bytes_read);
    memset(buffer, 0, sizeof(buffer));
  }
}

void ManagerServ::addToSet(const int fd, fd_set &set) {
  FD_SET(fd, &set);
  if (fd > _biggest_fd)
    _biggest_fd = fd;
}

void ManagerServ::removeFromSet(const int fd, fd_set &fdSet) {
  FD_CLR(fd, &fdSet);
  if (fd == _biggest_fd)
    _biggest_fd--;
}
