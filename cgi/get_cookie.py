"""
-> How to activate venv in Ubuntu
    - In the directory cgi, rum the command:
        source ./venv/bin/activate
"""

import os, cgi
from http import cookies

uploaded_cookies = cgi.FieldStorage()
key = uploaded_cookies.getvalue("key")
value = uploaded_cookies.getvalue("value")
response_body = ["HTTP1.1 200 OK", "Content-type: text/html;charset=utf-8\n"]

try:
    if isinstance(key, str) and isinstance(value, str):
        cookie = cookies.SimpleCookie()
        if "HTTP_COOKIE" in os.environ:
            cookie.load(os.environ["HTTP_COOKIE"])
        if key in cookie:
            response_body.append(f"The value of cookie {key} is {cookie[key].value}")
    else:
        response_body.append("Cookie was not found!")
except Exception as e:
    print(f"HTTP/1.1 500 Internal Server Error\n {e}")

[print(i) for i in response_body]
