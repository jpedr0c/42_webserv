"""
-> How to activate venv in Ubuntu
    - In the directory cgi, rum the command:
        source ./venv/bin/activate
"""

import cgi
from http import cookies

uploaded_cookies = cgi.FieldStorage()
key = uploaded_cookies.getvalue("key")
value = uploaded_cookies.getvalue("value")
response_headers = ["Content-type: text/html;charset=utf-8\n"]

try:
    if isinstance(key, str) and isinstance(value, str):
        cookie = cookies.SimpleCookie()
        cookie[key] = value
        response_headers.append(cookie.output())
        response_headers.insert(0, "HTTP/1.1 204 No Content")
    else:
        response_headers.insert(0, "HTTP/1.1 400 Bad Request")
        response_body = "Invalid key or value"
except Exception as e:
    response_headers.insert(0, "HTTP/1.1 500 Internal Server Error")
    response_body = str(e)

[print(i) for i in response_headers]
if "response_body" in locals():
    print(response_body)
