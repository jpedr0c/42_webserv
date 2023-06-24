#include "../inc/Client.hpp"

Client::Client() {
  lastMessageTime = time(NULL);
}

Client::~Client() {}

/* Copy constructor */
Client::Client(const Client &copy) {
  this->clientSocket = copy.clientSocket;
  this->clientSocketAddr = copy.clientSocketAddr;
  this->request = copy.request;
  this->response = copy.response;
  this->server = copy.server;
  this->lastMessageTime = copy.lastMessageTime;
}

/* Assinment operator */
Client &Client::operator=(const Client &copy) {
  this->clientSocket = copy.clientSocket;
  this->clientSocketAddr = copy.clientSocketAddr;
  this->request = copy.request;
  this->response = copy.response;
  this->server = copy.server;
  this->lastMessageTime = copy.lastMessageTime;
  return (*this);
}

Client::Client(Server &server) {
  setServer(server);
  request.setMaxBodySize(server.getClientMaxBodySize());
  lastMessageTime = time(NULL);
}

void Client::setSocket(int &socket) {
  clientSocket = socket;
}

void Client::setAddress(sockaddr_in &address) {
  clientSocketAddr = address;
}

void Client::setServer(Server &server) {
  response.setServer(server);
}

const int &Client::getSocket() const {
  return (clientSocket);
}

const Request &Client::getRequest() const {
  return (request);
}

const struct sockaddr_in &Client::getAddress() const {
  return (clientSocketAddr);
}

const time_t &Client::getLastMessageTime() const {
  return (lastMessageTime);
}

void Client::buildResponse() {
  response.setRequest(this->request);
  response.buildResponse();
}

void Client::updateLastMessageTime() {
  lastMessageTime = time(NULL);
}

void Client::clearClient() {
  response.clear();
  request.clear();
}