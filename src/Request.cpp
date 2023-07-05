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
  parsingStatus = REQUEST_LINE;
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
      case REQUEST_LINE: {
        if (character == 'G')
          httpMethod = GET;
        else if (character == 'P') {
          parsingStatus = REQUEST_LINE_POST_PUT;
          break;
        } else if (character == 'D')
          httpMethod = DELETE;
        else if (character == 'H') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 501, "Unsupported method <%s>", "HEAD");
          return;
        } else if (character == 'O') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 501, "Unsupported method <%s>", "OPTIONS");
          return;
        } else {
          LogService::printLog(ORANGE, SUCCESS, "Invalid character \"%s\"", character);
          return;
        }
        parsingStatus = REQUEST_LINE_METHOD;
        break;
      }
      case REQUEST_LINE_POST_PUT: {
        if (character == 'O')
          httpMethod = POST;
        else if (character == 'U') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 501, "Unsupported method <%s>", "PUT");
          return;
        } else if (character == 'A') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 501, "Unsupported method <%s>", "PATCH");
          return;
        } else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 501, "Invalid character \"%s\"", character);
          return;
        }
        httpMethod_index++;
        parsingStatus = REQUEST_LINE_METHOD;
        break;
      }
      case REQUEST_LINE_METHOD: {
        if (character == httpMethodStr[httpMethod][httpMethod_index])
          httpMethod_index++;
        else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 501, "Invalid character \"%s\"", character);
          return;
        }

        if ((size_t)httpMethod_index == httpMethodStr[httpMethod].length())
          parsingStatus = REQUEST_LINE_FIRST_SPACE;
        break;
      }
      case REQUEST_LINE_FIRST_SPACE: {
        if (character != ' ') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = REQUEST_LINE_URI_PATH_SLASH;
        continue;
      }
      case REQUEST_LINE_URI_PATH_SLASH: {
        if (character == '/') {
          parsingStatus = REQUEST_LINE_URI_PATH;
          storage.clear();
        } else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        break;
      }
      case REQUEST_LINE_URI_PATH: {
        if (character == ' ') {
          parsingStatus = REQUEST_LINE_VER;
          path.append(storage);
          storage.clear();
          continue;
        } else if (character == '?') {
          parsingStatus = REQUEST_LINE_URI_QUERY;
          path.append(storage);
          storage.clear();
          continue;
        } else if (character == '#') {
          parsingStatus = REQUEST_LINE_URI_FRAGMENT;
          path.append(storage);
          storage.clear();
          continue;
        } else if (!isValidURIChar(character)) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        } else if (i > MAX_URI_LENGTH) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 414, "URI exceeds maximum length: \"%s\"", character);
          return;
        }
        break;
      }
      case REQUEST_LINE_URI_QUERY: {
        if (character == ' ') {
          parsingStatus = REQUEST_LINE_VER;
          query.append(storage);
          storage.clear();
          continue;
        } else if (character == '#') {
          parsingStatus = REQUEST_LINE_URI_FRAGMENT;
          query.append(storage);
          storage.clear();
          continue;
        } else if (!isValidURIChar(character)) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        } else if (i > MAX_URI_LENGTH) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 414, "URI exceeds maximum length: \"%s\"", character);
          return;
        }
        break;
      }
      case REQUEST_LINE_URI_FRAGMENT: {
        if (character == ' ') {
          parsingStatus = REQUEST_LINE_VER;
          storage.clear();
          continue;
        } else if (!isValidURIChar(character)) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        } else if (i > MAX_URI_LENGTH) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 414, "URI exceeds maximum length: \"%s\"", character);
          return;
        }
        break;
      }
      case REQUEST_LINE_VER: {
        if (isValidUriPosition(path)) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        if (character != 'H') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = REQUEST_LINE_HT;
        break;
      }
      case REQUEST_LINE_HT: {
        if (character != 'T') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = REQUEST_LINE_HTT;
        break;
      }
      case REQUEST_LINE_HTT: {
        if (character != 'T') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = REQUEST_LINE_HTTP;
        break;
      }
      case REQUEST_LINE_HTTP: {
        if (character != 'P') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = REQUEST_LINE_HTTP_SLASH;
        break;
      }
      case REQUEST_LINE_HTTP_SLASH: {
        if (character != '/') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = REQUEST_LINE_MAJOR;
        break;
      }
      case REQUEST_LINE_MAJOR: {
        if (!isdigit(character)) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        verMajor = character;

        parsingStatus = REQUEST_LINE_DOT;
        break;
      }
      case REQUEST_LINE_DOT: {
        if (character != '.') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = REQUEST_LINE_MINOR;
        break;
      }
      case REQUEST_LINE_MINOR: {
        if (!isdigit(character)) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        verMinor = character;
        parsingStatus = REQUEST_LINE_CR;
        break;
      }
      case REQUEST_LINE_CR: {
        if (character != '\r') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = REQUEST_LINE_LF;
        break;
      }
      case REQUEST_LINE_LF: {
        if (character != '\n') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = FIELD_NAME_START;
        storage.clear();
        continue;
      }
      case FIELD_NAME_START: {
        if (character == '\r')
          parsingStatus = FIELDS_END;
        else if (isValidTokenChar(character))
          parsingStatus = FIELD_NAME;
        else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        break;
      }
      case FIELDS_END: {
        if (character == '\n') {
          storage.clear();
          fieldsDoneFlag = true;
          extractRequestHeaders();
          if (bodyFlag == 1) {
            if (chunkedFlag == true)
              parsingStatus = CHUNKED_LENGTH_BEGIN;
            else {
              parsingStatus = MESSAGE_BODY;
            }
          } else {
            parsingStatus = PARSING_DONE;
          }
          continue;
        } else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        break;
      }
      case FIELD_NAME: {
        if (character == ':') {
          keyStorage = storage;
          storage.clear();
          parsingStatus = FIELD_VALUE;
          continue;
        } else if (!isValidTokenChar(character)) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        break;
      }
      case FIELD_VALUE: {
        if (character == '\r') {
          setHeader(keyStorage, storage);
          keyStorage.clear();
          storage.clear();
          parsingStatus = FIELD_VALUE_END;
          continue;
        }
        break;
      }
      case FIELD_VALUE_END: {
        if (character == '\n') {
          parsingStatus = FIELD_NAME_START;
          continue;
        } else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        break;
      }
      case CHUNKED_LENGTH_BEGIN: {
        if (isxdigit(character) == 0) {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        s.str("");
        s.clear();
        s << character;
        s >> std::hex >> chunkLength;
        if (chunkLength == 0)
          parsingStatus = CHUNKED_LENGTH_CR;
        else
          parsingStatus = CHUNKED_LENGTH;
        continue;
      }
      case CHUNKED_LENGTH: {
        if (isxdigit(character) != 0) {
          int temp_len = 0;
          s.str("");
          s.clear();
          s << character;
          s >> std::hex >> temp_len;
          chunkLength *= 16;
          chunkLength += temp_len;
        } else if (character == '\r')
          parsingStatus = CHUNKED_END_LF;
        else
          parsingStatus = CHUNKED_IGNORE;
        continue;
      }
      case CHUNKED_LENGTH_CR: {
        if (character == '\r')
          parsingStatus = CHUNKED_END_LF;
        else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        continue;
      }
      case CHUNKED_LENGTH_LF: {
        if (character == '\n') {
          if (chunkLength == 0)
            parsingStatus = CHUNKED_END_CR;
          else
            parsingStatus = CHUNKED_DATA;
        } else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        continue;
      }
      case CHUNKED_IGNORE: {
        if (character == '\r')
          parsingStatus = CHUNKED_END_LF;
        continue;
      }
      case CHUNKED_DATA: {
        body.push_back(character);
        --chunkLength;
        if (chunkLength == 0)
          parsingStatus = CHUNKED_DATA_CR;
        continue;
      }
      case CHUNKED_DATA_CR: {
        if (character == '\r')
          parsingStatus = CHUNKED_DATA_LF;
        else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        continue;
      }
      case CHUNKED_DATA_LF: {
        if (character == '\n')
          parsingStatus = CHUNKED_LENGTH_BEGIN;
        else {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        continue;
      }
      case CHUNKED_END_CR: {
        if (character != '\r') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        parsingStatus = CHUNKED_END_LF;
        continue;
      }
      case CHUNKED_END_LF: {
        if (character != '\n') {
          LogService::printErrorCodeLog(ORANGE, errorCode, 400, "Unexpected character \"%s\" found", character);
          return;
        }
        bodyDoneFlag = true;
        parsingStatus = PARSING_DONE;
        continue;
      }
      case MESSAGE_BODY: {
        if (body.size() < bodyLength)
          body.push_back(character);
        if (body.size() == bodyLength) {
          bodyDoneFlag = true;
          parsingStatus = PARSING_DONE;
        }
        break;
      }
      case PARSING_DONE: {
        return;
      }
    }
    storage += character;
  }
  if (parsingStatus == PARSING_DONE) {
    bodyStr.append((char *)body.data(), body.size());
  }
}

bool Request::isParsingDone() {
  return (parsingStatus == PARSING_DONE);
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

void Request::extractRequestHeaders() {
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
  parsingStatus = REQUEST_LINE;
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

bool Request::isConnectionKeepAlive() {
  if (headerList.count("connection")) {
    if (headerList["connection"].find("close", 0) != std::string::npos)
      return (false);
  }
  return (true);
}