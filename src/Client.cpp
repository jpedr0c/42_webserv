#include "../inc/Client.hpp"

Client::Client() {
  lastMessageTime = time(NULL);
}

Client::~Client() {}

Client::Client(const Client &copy) {
  this->clientSocket = copy.clientSocket;
  this->clientSocketAddr = copy.clientSocketAddr;
  this->request = copy.request;
  this->response = copy.response;
  this->server = copy.server;
  this->lastMessageTime = copy.lastMessageTime;
}

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

void Client::setServer(Server &server) {
  response.setServer(server);
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