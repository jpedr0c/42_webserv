#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include "Webserv.hpp"

class Response {
 public:
  Response();
  Response(Request &);
  ~Response();
  std::string getRes();
  size_t getLen() const;
  int getCode() const;
  void setRequest(Request &);
  void setServer(Server &);
  void buildResponse();
  void clear();
  void handleCgi(Request &);
  void cutRes(size_t);
  int getCgiState();
  void setCgiState(int);
  void setErrorResponse(short code);
  bool isValidPath(std::string &path, std::string &locationKey, size_t &pos);
  bool isValidExtension(std::string &path);
  bool isValidFileType(std::string &path);
  bool isFileAllowed(Request &request, std::string &locationKey);
  bool initializeCgi(std::string &path, std::string &locationKey);
  std::string getErrorPage(short);
  int buildHtmlIndex(std::string &dirName, std::vector<uint8_t> &body, size_t &bodyLen);
  std::string extractFilename(const std::string &line);
  bool isBoundaryLine(const std::string &line, const std::string &boundary);
  bool isEndBoundaryLine(const std::string &line, const std::string &boundary);
  std::string removeBoundary(std::string &body, const std::string &boundary);
  bool isMethodNotAllowed(Location &location);
  bool isRequestBodySizeExceeded(const std::string &body, const Location &location);
  bool checkLocationReturn(Location &location);
  bool isCgiPath(const std::string &path);
  bool isCgiExtension(const std::string &targetFile, const Location &location);
  bool handleIndexLocation(const std::string &indexLocation, bool autoindex);
  bool handleNonLocation(const std::string &root, Request &request);
  bool isBodySizeExceeded();
  bool handleTgt();
  bool handleGetMethod();
  bool handlePostMethod();
  bool handleDeleteMethod();
  std::string responseContent;
  bool hasMimeType(std::string &extension) const;
  std::string getMimeType(std::string extension) const;

  CgiController cgiObj;
  Request request;

 private:
  Server serv;
  std::string targetFile;
  std::vector<uint8_t> body;
  size_t bodyLength;
  std::string responseBody;
  std::string location;
  short code;
  char *res;
  int cgi;
  int cgiFd[2];
  size_t cgiResLength;
  bool autoIndex;
  std::map<std::string, std::string> mimeTypes;

  int buildBody();
  void setStatusLine();
  void setHeaders();
  void setServerDefaultErrorPages();
  int readFile();
  void contentType();
  void contentLength();
  void connection();
  void server();
  void locations();
  void date();
  int handleTarget();
  void buildErrorBody();
  bool reqError();
  int handleCgi(std::string &);
  int controllerCgiTemp(std::string &);
};

#endif
