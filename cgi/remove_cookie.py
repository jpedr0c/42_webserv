"""
-> How to activate venv in Ubuntu
    - In the directory cgi, rum the command:
        source ./venv/bin/activate
"""

import os, cgi
from http import cookies
from datetime import datetime, timedelta

uploaded_cookies = cgi.FieldStorage()
key = uploaded_cookies.getvalue("key")
response_body = ["HTTP1.1 200 OK", "Content-type: text/html;charset=utf-8\n"]

try:
    if isinstance(key, str):
        cookie = cookies.SimpleCookie()
        if "HTTP_COOKIE" in os.environ:
            cookie.load(os.environ["HTTP_COOKIE"])
        if key in cookie:
            cookie[key] = ""
            expiration_cookie = datetime.now() - timedelta(days=1)
            cookie[key]['expires'] = expiration_cookie.strftime('%a, %d-%b-%Y %H:%M:%S GMT')
            response_body.append(cookie.output())
        else:
            response_body.append("Cookie was not found!")
    else:
        response_body.append("Invalid key provided!")
except Exception as e:
    print(f"HTTP/1.1 500 Internal Server Error\n {e}")

[print(i) for i in response_body]
