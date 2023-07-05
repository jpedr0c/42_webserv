#include "../inc/Request.hpp"

Request::Request() {
  httpMethodStr[::GET] = "GET";
  httpMethodStr[::POST] = "POST";
  httpMethodStr[::DELETE] = "DELETE";
  path = "";
  query = "";
  bodyStr = "";
  errorCode = 0;
  chunkLength = 0;
  httpMethod = NONE;
  httpMethod_index = 1;
  parsingStatus = Request_Line;
  fieldsDoneFlag = false;
  bodyFlag = false;
  bodyDoneFlag = false;
  chunkedFlag = false;
  bodyLength = 0;
  storage = "";
  keyStorage = "";
  multiformFlag = false;
  boundary = "";
}

Request::~Request() {}

bool Request::isValidUriPosition(std::string path) {
  std::string tmp(path);
  char *res = strtok((char *)tmp.c_str(), "/");
  int pos = 0;
  while (res != NULL) {
    if (!strcmp(res, ".."))
      pos--;
    else
      pos++;
    if (pos < 0)
      return (true);
    res = strtok(NULL, "/");
  }
  return (false);
}

bool Request::isValidURIChar(uint8_t ch) {
  if ((ch >= '#' && ch <= ';') || (ch >= '?' && ch <= '[') || (ch >= 'a' && ch <= 'z') ||
      ch == '!' || ch == '=' || ch == ']' || ch == '_' || ch == '~')
    return (true);
  return (false);
}

bool Request::isValidTokenChar(uint8_t ch) {
  if (ch == '!' || (ch >= '#' && ch <= '\'') || ch == '*' || ch == '+' || ch == '-' || ch == '.' ||
      (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= '^' && ch <= '`') ||
      (ch >= 'a' && ch <= 'z') || ch == '|')
    return (true);
  return (false);
}

void Request::removeLeadingTrailingWhitespace(std::string &str) {
  static const char *spaces = " \t";
  str.erase(0, str.find_first_not_of(spaces));
  str.erase(str.find_last_not_of(spaces) + 1);
}

void Request::convertToLowerCase(std::string &str) {
  for (size_t i = 0; i < str.length(); ++i)
    str[i] = std::tolower(str[i]);
}

void Request::parseHTTPRequestData(char *data, size_t size) {
  u_int8_t character;
  static std::stringstream s;

  for (size_t i = 0; i < size; ++i) {
    character = data[i];
    switch (parsingStatus) {
      case Request_Line: {
        if (character == 'G')
          httpMethod = GET;
        else if (character == 'P') {
          parsingStatus = Request_Line_Post_Put;
          break;
        } else if (character == 'D')
          httpMethod = DELETE;
        else if (character == 'H') {
          errorCode = 501;
          LogService::printLog(ORANGE, SUCCESS, "Unsupported method <%s>", "HEAD");
          return;
        } else if (character == 'O') {
          errorCode = 501;
          LogService::printLog(ORANGE, SUCCESS, "Unsupported method <%s>", "OPTIONS");
          return;
        } else {
          errorCode = 501;
          LogService::printLog(ORANGE, SUCCESS, "Invalid character \"%s\"", character);
          return;
        }
        parsingStatus = Request_Line_Method;
        break;
      }
      case Request_Line_Post_Put: {
        if (character == 'O')
          httpMethod = POST;
        else if (character == 'U') {
          errorCode = 501;
          LogService::printLog(ORANGE, SUCCESS, "Unsupported method <%s>", "PUT");
          return;
        } else if (character == 'A') {
          errorCode = 501;
          LogService::printLog(ORANGE, SUCCESS, "Unsupported method <%s>", "PATCH");
          return;
        } else {
          errorCode = 501;
          LogService::printLog(ORANGE, SUCCESS, "Invalid character \"%s\"", character);
          return;
        }
        httpMethod_index++;
        parsingStatus = Request_Line_Method;
        break;
      }
      case Request_Line_Method: {
        if (character == httpMethodStr[httpMethod][httpMethod_index])
          httpMethod_index++;
        else {
          errorCode = 501;
          std::cout << "Method Error Request_Line and Character is = " << character << std::endl;
          return;
        }

        if ((size_t)httpMethod_index == httpMethodStr[httpMethod].length())
          parsingStatus = Request_Line_First_Space;
        break;
      }
      case Request_Line_First_Space: {
        if (character != ' ') {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_First_Space)" << std::endl;
          return;
        }
        parsingStatus = Request_Line_URI_Path_Slash;
        continue;
      }
      case Request_Line_URI_Path_Slash: {
        if (character == '/') {
          parsingStatus = Request_Line_URI_Path;
          storage.clear();
        } else {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_URI_Path_Slash)" << std::endl;
          return;
        }
        break;
      }
      case Request_Line_URI_Path: {
        if (character == ' ') {
          parsingStatus = Request_Line_Ver;
          path.append(storage);
          storage.clear();
          continue;
        } else if (character == '?') {
          parsingStatus = Request_Line_URI_Query;
          path.append(storage);
          storage.clear();
          continue;
        } else if (character == '#') {
          parsingStatus = Request_Line_URI_Fragment;
          path.append(storage);
          storage.clear();
          continue;
        } else if (!isValidURIChar(character)) {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_URI_Path)" << std::endl;
          return;
        } else if (i > MAX_URI_LENGTH) {
          errorCode = 414;
          std::cout << "URI Too Long (Request_Line_URI_Path)" << std::endl;
          return;
        }
        break;
      }
      case Request_Line_URI_Query: {
        if (character == ' ') {
          parsingStatus = Request_Line_Ver;
          query.append(storage);
          storage.clear();
          continue;
        } else if (character == '#') {
          parsingStatus = Request_Line_URI_Fragment;
          query.append(storage);
          storage.clear();
          continue;
        } else if (!isValidURIChar(character)) {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_URI_Query)" << std::endl;
          return;
        } else if (i > MAX_URI_LENGTH) {
          errorCode = 414;
          std::cout << "URI Too Long (Request_Line_URI_Path)" << std::endl;
          return;
        }
        break;
      }
      case Request_Line_URI_Fragment: {
        if (character == ' ') {
          parsingStatus = Request_Line_Ver;
          storage.clear();
          continue;
        } else if (!isValidURIChar(character)) {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_URI_Fragment)" << std::endl;
          return;
        } else if (i > MAX_URI_LENGTH) {
          errorCode = 414;
          std::cout << "URI Too Long (Request_Line_URI_Path)" << std::endl;
          return;
        }
        break;
      }
      case Request_Line_Ver: {
        if (isValidUriPosition(path)) {
          errorCode = 400;
          std::cout << "Request URI ERROR: goes before root !!" << std::endl;
          return;
        }
        if (character != 'H') {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_Ver)" << std::endl;
          return;
        }
        parsingStatus = Request_Line_HT;
        break;
      }
      case Request_Line_HT: {
        if (character != 'T') {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_HT)" << std::endl;
          return;
        }
        parsingStatus = Request_Line_HTT;
        break;
      }
      case Request_Line_HTT: {
        if (character != 'T') {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_HTT)" << std::endl;
          return;
        }
        parsingStatus = Request_Line_HTTP;
        break;
      }
      case Request_Line_HTTP: {
        if (character != 'P') {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_HTTP)" << std::endl;
          return;
        }
        parsingStatus = Request_Line_HTTP_Slash;
        break;
      }
      case Request_Line_HTTP_Slash: {
        if (character != '/') {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_HTTP_Slash)" << std::endl;
          return;
        }
        parsingStatus = Request_Line_Major;
        break;
      }
      case Request_Line_Major: {
        if (!isdigit(character)) {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_Major)" << std::endl;
          return;
        }
        verMajor = character;

        parsingStatus = Request_Line_Dot;
        break;
      }
      case Request_Line_Dot: {
        if (character != '.') {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_Dot)" << std::endl;
          return;
        }
        parsingStatus = Request_Line_Minor;
        break;
      }
      case Request_Line_Minor: {
        if (!isdigit(character)) {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_Minor)" << std::endl;
          return;
        }
        verMinor = character;
        parsingStatus = Request_Line_CR;
        break;
      }
      case Request_Line_CR: {
        if (character != '\r') {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_CR)" << std::endl;
          return;
        }
        parsingStatus = Request_Line_LF;
        break;
      }
      case Request_Line_LF: {
        if (character != '\n') {
          errorCode = 400;
          std::cout << "Bad Character (Request_Line_LF)" << std::endl;
          return;
        }
        parsingStatus = Field_Name_Start;
        storage.clear();
        continue;
      }
      case Field_Name_Start: {
        if (character == '\r')
          parsingStatus = Fields_End;
        else if (isValidTokenChar(character))
          parsingStatus = Field_Name;
        else {
          errorCode = 400;
          std::cout << "Bad Character (Field_Name_Start)" << std::endl;
          return;
        }
        break;
      }
      case Fields_End: {
        if (character == '\n') {
          storage.clear();
          fieldsDoneFlag = true;
          _handle_headers();
          if (bodyFlag == 1) {
            if (chunkedFlag == true)
              parsingStatus = Chunked_Length_Begin;
            else {
              parsingStatus = Message_Body;
            }
          } else {
            parsingStatus = Parsing_Done;
          }
          continue;
        } else {
          errorCode = 400;
          std::cout << "Bad Character (Fields_End)" << std::endl;
          return;
        }
        break;
      }
      case Field_Name: {
        if (character == ':') {
          keyStorage = storage;
          storage.clear();
          parsingStatus = Field_Value;
          continue;
        } else if (!isValidTokenChar(character)) {
          errorCode = 400;
          std::cout << "Bad Character (Field_Name)" << std::endl;
          return;
        }
        break;
      }
      case Field_Value: {
        if (character == '\r') {
          setHeader(keyStorage, storage);
          keyStorage.clear();
          storage.clear();
          parsingStatus = Field_Value_End;
          continue;
        }
        break;
      }
      case Field_Value_End: {
        if (character == '\n') {
          parsingStatus = Field_Name_Start;
          continue;
        } else {
          errorCode = 400;
          std::cout << "Bad Character (Field_Value_End)" << std::endl;
          return;
        }
        break;
      }
      case Chunked_Length_Begin: {
        if (isxdigit(character) == 0) {
          errorCode = 400;
          std::cout << "Bad Character (Chunked_Length_Begin)" << std::endl;
          return;
        }
        s.str("");
        s.clear();
        s << character;
        s >> std::hex >> chunkLength;
        if (chunkLength == 0)
          parsingStatus = Chunked_Length_CR;
        else
          parsingStatus = Chunked_Length;
        continue;
      }
      case Chunked_Length: {
        if (isxdigit(character) != 0) {
          int temp_len = 0;
          s.str("");
          s.clear();
          s << character;
          s >> std::hex >> temp_len;
          chunkLength *= 16;
          chunkLength += temp_len;
        } else if (character == '\r')
          parsingStatus = Chunked_Length_LF;
        else
          parsingStatus = Chunked_Ignore;
        continue;
      }
      case Chunked_Length_CR: {
        if (character == '\r')
          parsingStatus = Chunked_Length_LF;
        else {
          errorCode = 400;
          std::cout << "Bad Character (Chunked_Length_CR)" << std::endl;
          return;
        }
        continue;
      }
      case Chunked_Length_LF: {
        if (character == '\n') {
          if (chunkLength == 0)
            parsingStatus = Chunked_End_CR;
          else
            parsingStatus = Chunked_Data;
        } else {
          errorCode = 400;
          std::cout << "Bad Character (Chunked_Length_LF)" << std::endl;
          return;
        }
        continue;
      }
      case Chunked_Ignore: {
        if (character == '\r')
          parsingStatus = Chunked_Length_LF;
        continue;
      }
      case Chunked_Data: {
        body.push_back(character);
        --chunkLength;
        if (chunkLength == 0)
          parsingStatus = Chunked_Data_CR;
        continue;
      }
      case Chunked_Data_CR: {
        if (character == '\r')
          parsingStatus = Chunked_Data_LF;
        else {
          errorCode = 400;
          std::cout << "Bad Character (Chunked_Data_CR)" << std::endl;
          return;
        }
        continue;
      }
      case Chunked_Data_LF: {
        if (character == '\n')
          parsingStatus = Chunked_Length_Begin;
        else {
          errorCode = 400;
          std::cout << "Bad Character (Chunked_Data_LF)" << std::endl;
          return;
        }
        continue;
      }
      case Chunked_End_CR: {
        if (character != '\r') {
          errorCode = 400;
          std::cout << "Bad Character (Chunked_End_CR)" << std::endl;
          return;
        }
        parsingStatus = Chunked_End_LF;
        continue;
      }
      case Chunked_End_LF: {
        if (character != '\n') {
          errorCode = 400;
          std::cout << "Bad Character (Chunked_End_LF)" << std::endl;
          return;
        }
        bodyDoneFlag = true;
        parsingStatus = Parsing_Done;
        continue;
      }
      case Message_Body: {
        if (body.size() < bodyLength)
          body.push_back(character);
        if (body.size() == bodyLength) {
          bodyDoneFlag = true;
          parsingStatus = Parsing_Done;
        }
        break;
      }
      case Parsing_Done: {
        return;
      }
    }
    storage += character;
  }
  if (parsingStatus == Parsing_Done) {
    bodyStr.append((char *)body.data(), body.size());
  }
}

bool Request::isParsingDone() {
  return (parsingStatus == Parsing_Done);
}

HttpMethod &Request::getHttpMethod() {
  return (httpMethod);
}

std::string &Request::getPath() {
  return (path);
}

std::string &Request::getQuery() {
  return (query);
}

std::string Request::getHeader(std::string const &name) {
  return (headerList[name]);
}

const std::map<std::string, std::string> &Request::getHeaders() const {
  return (this->headerList);
}

std::string Request::getMethodStr() {
  return (httpMethodStr[httpMethod]);
}

std::string &Request::getBody() {
  return (bodyStr);
}

std::string Request::getServerName() {
  return (this->serverName);
}

bool Request::getMultiformFlag() {
  return (this->multiformFlag);
}

std::string &Request::getBoundary() {
  return (this->boundary);
}

void Request::setBody(std::string body) {
  body.clear();
  body.insert(body.begin(), body.begin(), body.end());
  bodyStr = body;
}

void Request::setHeader(std::string &name, std::string &value) {
  removeLeadingTrailingWhitespace(value);
  convertToLowerCase(name);
  headerList[name] = value;
}

void Request::setMaxBodySize(size_t size) {
  maxBodySize = size;
}

void Request::_handle_headers() {
  std::stringstream ss;

  if (headerList.count("content-length")) {
    bodyFlag = true;
    ss << headerList["content-length"];
    ss >> bodyLength;
  }
  if (headerList.count("transfer-encoding")) {
    if (headerList["transfer-encoding"].find_first_of("chunked") != std::string::npos)
      chunkedFlag = true;
    bodyFlag = true;
  }
  if (headerList.count("host")) {
    size_t pos = headerList["host"].find_first_of(':');
    serverName = headerList["host"].substr(0, pos);
  }
  if (headerList.count("content-type") && headerList["content-type"].find("multipart/form-data") != std::string::npos) {
    size_t pos = headerList["content-type"].find("boundary=", 0);
    if (pos != std::string::npos)
      this->boundary = headerList["content-type"].substr(pos + 9, headerList["content-type"].size());
    this->multiformFlag = true;
  }
}

short Request::errorCodes() {
  return (this->errorCode);
}

void Request::setErrorCode(short status) {
  this->errorCode = status;
}

void Request::clear() {
  path.clear();
  errorCode = 0;
  query.clear();
  httpMethod = NONE;
  httpMethod_index = 1;
  parsingStatus = Request_Line;
  bodyLength = 0;
  chunkLength = 0x0;
  storage.clear();
  bodyStr = "";
  keyStorage.clear();
  headerList.clear();
  serverName.clear();
  body.clear();
  boundary.clear();
  fieldsDoneFlag = false;
  bodyFlag = false;
  bodyDoneFlag = false;
  completeFlag = false;
  chunkedFlag = false;
  multiformFlag = false;
}

bool Request::keepAlive() {
  if (headerList.count("connection")) {
    if (headerList["connection"].find("close", 0) != std::string::npos)
      return (false);
  }
  return (true);
}