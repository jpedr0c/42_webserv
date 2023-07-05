#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "Webserv.hpp"

enum HttpMethod {
  GET,
  POST,
  DELETE,
  NONE
};

enum ParsingState {
  REQUEST_LINE,
  REQUEST_LINE_POST_PUT,
  REQUEST_LINE_METHOD,
  REQUEST_LINE_FIRST_SPACE,
  REQUEST_LINE_URI_PATH_SLASH,
  REQUEST_LINE_URI_PATH,
  REQUEST_LINE_URI_QUERY,
  REQUEST_LINE_URI_FRAGMENT,
  REQUEST_LINE_VER,
  REQUEST_LINE_HT,
  REQUEST_LINE_HTT,
  REQUEST_LINE_HTTP,
  REQUEST_LINE_HTTP_SLASH,
  REQUEST_LINE_MAJOR,
  REQUEST_LINE_DOT,
  REQUEST_LINE_MINOR,
  REQUEST_LINE_CR,
  REQUEST_LINE_LF,
  FIELD_NAME_START,
  FIELDS_END,
  FIELD_NAME,
  FIELD_VALUE,
  FIELD_VALUE_END,
  CHUNKED_LENGTH_BEGIN,
  CHUNKED_LENGTH,
  CHUNKED_IGNORE,
  CHUNKED_LENGTH_CR,
  CHUNKED_LENGTH_LF,
  CHUNKED_DATA,
  CHUNKED_DATA_CR,
  CHUNKED_DATA_LF,
  CHUNKED_END_CR,
  CHUNKED_END_LF,
  MESSAGE_BODY,
  PARSING_DONE
};

class Request {
 public:
  Request();
  ~Request();

  bool isValidUriPosition(std::string path);
  bool isValidURIChar(uint8_t ch);
  bool isValidTokenChar(uint8_t ch);
  void removeLeadingTrailingWhitespace(std::string &str);
  void convertToLowerCase(std::string &str);

  HttpMethod &getHttpMethod();
  std::string &getPath();
  std::string &getQuery();
  std::string getHeader(std::string const &);
  const std::map<std::string, std::string> &getHeaders() const;
  std::string getMethodStr();
  std::string &getBody();
  std::string getServerName();
  std::string &getBoundary();
  bool getMultiformFlag();

  void setHeader(std::string &, std::string &);
  void setMaxBodySize(size_t);
  void setBody(std::string name);

  void parseHTTPRequestData(char *data, size_t size);
  bool isParsingDone();
  void clear();
  short errorCodes();
  void setErrorCode(short status);
  bool isConnectionKeepAlive();

 private:
  std::string path;
  std::string query;
  std::map<std::string, std::string> headerList;
  std::vector<u_int8_t> body;
  std::string boundary;
  HttpMethod httpMethod;
  std::map<u_int8_t, std::string> httpMethodStr;
  ParsingState parsingStatus;
  size_t maxBodySize;
  size_t bodyLength;
  short errorCode;
  size_t chunkLength;
  std::string storage;
  std::string keyStorage;
  short httpMethod_index;
  u_int8_t verMajor;
  u_int8_t verMinor;
  std::string serverName;
  std::string bodyStr;
  bool fieldsDoneFlag;
  bool bodyFlag;
  bool bodyDoneFlag;
  bool completeFlag;
  bool chunkedFlag;
  bool multiformFlag;

  void extractRequestHeaders();
};

#endif