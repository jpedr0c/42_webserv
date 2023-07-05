#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "Webserv.hpp"

enum HttpMethod {
  GET,
  POST,
  DELETE,
  PUT,
  HEAD,
  NONE
};

enum ParsingState {
  Request_Line,
  Request_Line_Post_Put,
  Request_Line_Method,
  Request_Line_First_Space,
  Request_Line_URI_Path_Slash,
  Request_Line_URI_Path,
  Request_Line_URI_Query,
  Request_Line_URI_Fragment,
  Request_Line_Ver,
  Request_Line_HT,
  Request_Line_HTT,
  Request_Line_HTTP,
  Request_Line_HTTP_Slash,
  Request_Line_Major,
  Request_Line_Dot,
  Request_Line_Minor,
  Request_Line_CR,
  Request_Line_LF,
  Field_Name_Start,
  Fields_End,
  Field_Name,
  Field_Value,
  Field_Value_End,
  Chunked_Length_Begin,
  Chunked_Length,
  Chunked_Ignore,
  Chunked_Length_CR,
  Chunked_Length_LF,
  Chunked_Data,
  Chunked_Data_CR,
  Chunked_Data_LF,
  Chunked_End_CR,
  Chunked_End_LF,
  Message_Body,
  Parsing_Done
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
  short errorCode();
  bool keepAlive();

 private:
  std::string path;
  std::string query;
  std::map<std::string, std::string> headerList;
  std::vector<u_int8_t> _body;
  std::string _boundary;
  HttpMethod httpMethod;
  std::map<u_int8_t, std::string> httpMethod_str;
  ParsingState parsingStatus;
  size_t maxBodySize;
  size_t _body_length;
  short _error_code;
  size_t _chunk_length;
  std::string _storage;
  std::string _key_storage;
  short httpMethod_index;
  u_int8_t _ver_major;
  u_int8_t _ver_minor;
  std::string _server_name;
  std::string _body_str;
  bool _fields_done_flag;
  bool _body_flag;
  bool _body_done_flag;
  bool _complete_flag;
  bool _chunked_flag;
  bool _multiform_flag;

  void _handle_headers();
};

#endif